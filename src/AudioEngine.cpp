#include "AudioEngine.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include <new>

void AudioEngine::VoiceCallback::OnBufferEnd(void* context){
    if (!owner || !context) return;
    auto* token = static_cast<VoiceToken*>(context);
    owner->Recycle(token->index, token->generation);
}

AudioEngine::~AudioEngine(){ Shutdown(); }

bool AudioEngine::Initialize(std::wstring& error){
    std::lock_guard<std::mutex> lock(stateMu_);
    if(ready_.load(std::memory_order_acquire)) return true;

    HRESULT hr = XAudio2Create(&xa_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if(FAILED(hr)){ error=L"Windows audio could not be initialized."; return false; }
    xa_->RegisterForCallbacks(this);
    hr = xa_->CreateMasteringVoice(&master_);
    if(FAILED(hr)){
        xa_->UnregisterForCallbacks(this); xa_->Release(); xa_=nullptr;
        error=L"The Windows audio output could not be opened."; return false;
    }

    WAVEFORMATEX fmt{};
    fmt.wFormatTag=WAVE_FORMAT_PCM;
    fmt.nChannels=2;
    fmt.nSamplesPerSec=48000;
    fmt.wBitsPerSample=16;
    fmt.nBlockAlign=fmt.nChannels*(fmt.wBitsPerSample/8);
    fmt.nAvgBytesPerSec=fmt.nSamplesPerSec*fmt.nBlockAlign;

    for(size_t i=0;i<slots_.size();++i){
        slots_[i].callback.owner=this; slots_[i].callback.index=i;
        hr=xa_->CreateSourceVoice(&slots_[i].voice,&fmt,0,2.0f,&slots_[i].callback);
        if(FAILED(hr)){
            for(auto& x:slots_) if(x.voice){ x.voice->DestroyVoice(); x.voice=nullptr; }
            master_->DestroyVoice(); master_=nullptr;
            xa_->UnregisterForCallbacks(this); xa_->Release(); xa_=nullptr;
            error=L"The audio voice pool could not be created."; return false;
        }
        slots_[i].generation.store(0,std::memory_order_relaxed);
        slots_[i].busy.store(false,std::memory_order_relaxed);
    }
    ready_.store(true,std::memory_order_release);
    LogInfo(L"Audio engine initialized with 32 reusable voices.");
    return true;
}

void AudioEngine::Shutdown(){
    std::lock_guard<std::mutex> lock(stateMu_);
    ready_.store(false,std::memory_order_release);
    for(auto& s:slots_){
        if(s.voice){ s.voice->Stop(0); s.voice->FlushSourceBuffers(); s.voice->DestroyVoice(); s.voice=nullptr; }
        s.busy.store(false,std::memory_order_release);
    }
    if(master_){ master_->DestroyVoice(); master_=nullptr; }
    if(xa_){ xa_->UnregisterForCallbacks(this); xa_->Release(); xa_=nullptr; }
}

bool AudioEngine::Play(const std::shared_ptr<const WavData>& sample,float volume,float pitchSemitones,float pan){
    if(!sample || !ready_.load(std::memory_order_acquire) || sample->format.nChannels!=2 || sample->format.nSamplesPerSec!=48000 || sample->format.wBitsPerSample!=16) return false;
    size_t chosen=kMaxVoices;
    for(size_t i=0;i<slots_.size();++i){
        bool expected=false;
        if(slots_[i].busy.compare_exchange_strong(expected,true,std::memory_order_acq_rel)){ chosen=i; break; }
    }
    if(chosen==kMaxVoices) return false; // bounded policy: drop the newest sound rather than block

    auto& s=slots_[chosen];
    const auto generation=s.generation.fetch_add(1,std::memory_order_acq_rel)+1;
    s.voice->SetVolume(std::clamp(volume,0.0f,1.0f));
    s.voice->SetFrequencyRatio(std::clamp(std::pow(2.0f,pitchSemitones/12.0f),0.5f,2.0f));

    // Canonical stereo input: constant-power pan by scaling left/right.
    const float p=std::clamp(pan,-1.0f,1.0f);
    const float left=std::sqrt((1.0f-p)*0.5f);
    const float right=std::sqrt((1.0f+p)*0.5f);
    const float matrix[2]={left,right};
    if(FAILED(s.voice->SetOutputMatrix(master_,2,2,matrix))){ Recycle(chosen,generation); return false; }

    s.token={chosen,generation};
    XAUDIO2_BUFFER b{};
    b.AudioBytes=static_cast<UINT32>(sample->pcm.size());
    b.pAudioData=sample->pcm.data();
    b.pContext=&s.token;
    HRESULT hr=s.voice->SubmitSourceBuffer(&b);
    if(FAILED(hr)){ Recycle(chosen,generation); return false; }
    hr=s.voice->Start(0);
    if(FAILED(hr)){
        s.voice->FlushSourceBuffers();
        return false;
    }
    return true;
}

void AudioEngine::Recycle(size_t index,uint64_t generation){
    if(index>=slots_.size()) return;
    auto& s=slots_[index];
    if(s.generation.load(std::memory_order_acquire)==generation)
        s.busy.store(false,std::memory_order_release);
}

void AudioEngine::StopAll(){
    for(auto& s:slots_){
        if(s.voice){ s.voice->Stop(0); s.voice->FlushSourceBuffers(); }
        // Keep the slot occupied until the flushed-buffer callback releases its token.
        // A new playback cannot reuse this voice before that callback, so the token remains immutable.
    }
}

void AudioEngine::OnCriticalError(HRESULT){
    LogError(L"XAudio2 reported a critical error; audio has been disabled.");
    ready_.store(false,std::memory_order_release);
}
