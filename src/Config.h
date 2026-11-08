#pragma once
#include "Wav.h"
#include <windows.h>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

struct KeyAssignment {
    std::vector<std::wstring> press;
    std::vector<std::wstring> release;
    float volume=1.0f;
    float pitch=0.0f;
    float randomPitch=0.0f;
    float randomVolume=0.0f;
    float pan=0.0f;
};

class Config final {
public:
    bool Load(std::wstring& error);
    bool Save(std::wstring& error) const;
    void ResetDefaults();
    std::filesystem::path Root() const { return root_; }
    std::filesystem::path SoundPackRoot() const { return root_/L"sounds"; }
    std::wstring packName=L"Classic Mechanical";
    float masterVolume=0.65f;
    bool enabled=true;
    bool pressSounds=true;
    bool releaseSounds=false;
    bool randomization=true;
    bool startup=false;
    std::wstring emergencyHotkey=L"Ctrl+Alt+Shift+K";
    std::unordered_map<int,KeyAssignment> assignments;
private:
    std::filesystem::path root_;
};

struct SoundPack {
    std::wstring name;
    std::filesystem::path root;
    std::unordered_map<int,KeyAssignment> assignments;
};

bool LoadSoundPack(const std::filesystem::path& root, SoundPack& out, std::wstring& error);
