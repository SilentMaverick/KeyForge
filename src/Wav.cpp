#include "Wav.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstring>

namespace {
constexpr uint32_t kTargetRate = 48000;
constexpr uint32_t kMaxFileBytes = 64u * 1024u * 1024u;
constexpr uint32_t kMaxPcmBytes = 32u * 1024u * 1024u;
constexpr uint32_t kMaxDurationSeconds = 10u;
uint32_t U32(const uint8_t* p){ return uint32_t(p[0]) | (uint32_t(p[1])<<8) | (uint32_t(p[2])<<16) | (uint32_t(p[3])<<24); }
uint16_t U16(const uint8_t* p){ return uint16_t(p[0]) | (uint16_t(p[1])<<8); }
bool Tag(const uint8_t* p,const char (&tag)[5]){ return std::memcmp(p,tag,4)==0; }
}

bool LoadPcmWav(const std::wstring& path, WavData& out, std::wstring& error) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if(!f){ error=L"The audio file could not be opened."; return false; }
    const std::streamoff fileSize=f.tellg();
    if(fileSize<=0 || fileSize>static_cast<std::streamoff>(kMaxFileBytes)){ error=L"The audio file is missing or exceeds the 64 MB safety limit."; return false; }
    f.seekg(0);
    std::vector<uint8_t> b(static_cast<size_t>(fileSize));
    if(!f.read(reinterpret_cast<char*>(b.data()), static_cast<std::streamsize>(b.size()))){ error=L"The audio file could not be read completely."; return false; }
    if(b.size()<12 || !Tag(b.data(),"RIFF") || !Tag(b.data()+8,"WAVE")){ error=L"Only PCM WAV files are supported in this build."; return false; }

    bool gotFmt=false, gotData=false; size_t pos=12; size_t dataPos=0; uint32_t dataSize=0;
    uint16_t channels=0,bits=0,blockAlign=0; uint32_t sampleRate=0;
    while(pos<=b.size() && b.size()-pos>=8){
        const uint32_t chunk=U32(b.data()+pos+4);
        const uint64_t rawNext=static_cast<uint64_t>(pos)+8ull+chunk+static_cast<uint64_t>(chunk&1u);
        if(rawNext>b.size()){ error=L"The WAV contains an invalid chunk size."; return false; }
        const size_t chunkData=pos+8;
        if(Tag(b.data()+pos,"fmt ")){
            if(chunk<16 || chunkData+16>b.size()){ error=L"The WAV format chunk is invalid."; return false; }
            const uint16_t tag=U16(b.data()+chunkData);
            channels=U16(b.data()+chunkData+2); sampleRate=U32(b.data()+chunkData+4);
            blockAlign=U16(b.data()+chunkData+12); bits=U16(b.data()+chunkData+14);
            if(tag!=WAVE_FORMAT_PCM || channels<1 || channels>2 || sampleRate<8000 || sampleRate>192000 || bits!=16 || blockAlign!=channels*2){ error=L"Unsupported WAV format. Use 16-bit PCM, 1–2 channels, 8–192 kHz."; return false; }
            gotFmt=true;
        } else if(Tag(b.data()+pos,"data")){
            if(chunk==0 || chunk>kMaxPcmBytes || chunkData+chunk>b.size()){ error=L"The WAV audio data is invalid or too large."; return false; }
            dataPos=chunkData; dataSize=chunk; gotData=true;
        }
        pos=static_cast<size_t>(rawNext);
    }
    if(!gotFmt || !gotData || dataSize==0 || dataSize%blockAlign!=0){ error=L"The WAV is missing valid PCM audio data."; return false; }

    const size_t inputFrames=dataSize/blockAlign;
    const uint64_t outputFrames64=(static_cast<uint64_t>(inputFrames)*kTargetRate + sampleRate-1)/sampleRate;
    if(outputFrames64==0 || outputFrames64>std::numeric_limits<size_t>::max()/4u || outputFrames64*4ull>kMaxPcmBytes){ error=L"The decoded WAV would exceed the 32 MB PCM safety limit."; return false; }
    const size_t outputFrames=static_cast<size_t>(outputFrames64);
    if(outputFrames>static_cast<size_t>(kTargetRate)*kMaxDurationSeconds){ error=L"The WAV exceeds the 10 second sound-duration safety limit."; return false; }
    out.pcm.resize(outputFrames*4u);

    const auto* src=reinterpret_cast<const int16_t*>(b.data()+dataPos);
    auto* dst=reinterpret_cast<int16_t*>(out.pcm.data());
    const double step=static_cast<double>(sampleRate)/kTargetRate;
    for(size_t i=0;i<outputFrames;++i){
        const double srcPos=std::min(static_cast<double>(inputFrames-1), i*step);
        const size_t i0=static_cast<size_t>(srcPos);
        const size_t i1=std::min(i0+1,inputFrames-1);
        const double frac=srcPos-i0;
        const int16_t l0=src[i0*channels];
        const int16_t l1=src[i1*channels];
        const int left=static_cast<int>(std::lround(l0+(l1-l0)*frac));
        int right=left;
        if(channels==2){
            const int16_t r0=src[i0*2+1], r1=src[i1*2+1];
            right=static_cast<int>(std::lround(r0+(r1-r0)*frac));
        }
        dst[i*2]=static_cast<int16_t>(std::clamp(left,-32768,32767));
        dst[i*2+1]=static_cast<int16_t>(std::clamp(right,-32768,32767));
    }
    out.format={WAVE_FORMAT_PCM,2,kTargetRate, kTargetRate*4,4,16,0};
    out.path=path;
    return true;
}
