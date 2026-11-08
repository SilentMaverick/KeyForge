#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <xaudio2.h>

struct WavData {
    WAVEFORMATEX format{};
    std::vector<std::uint8_t> pcm;
    std::wstring path;
};

bool LoadPcmWav(const std::wstring& path, WavData& out, std::wstring& error);
