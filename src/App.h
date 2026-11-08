#pragma once
#include "AudioEngine.h"
#include "KeyboardEngine.h"
#include "Config.h"
#include <windows.h>
#include <shellapi.h>
#include <unordered_map>
#include <random>
#include <mutex>
#include <list>
#include <unordered_set>

class App final {
public:
    bool Initialize(HINSTANCE instance, int show, std::wstring& error);
    int Run();
private:
    static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
    LRESULT Handle(HWND,UINT,WPARAM,LPARAM);
    void BuildUI(HWND hwnd);
    void HandleKey(const KeyEvent& e);
    void RefreshKeyVisual(int key, bool down);
    void SelectKey(int key);
    void ImportForSelected();
    void ImportDropped(HDROP drop);
    bool AddSoundToLibrary(const std::filesystem::path& source, std::wstring& storedName);
    void RefreshSoundLibrary(const std::wstring& selectName = L"");
    void AssignSelectedSound();
    std::wstring SelectedSoundName() const;
    bool LoadSample(const std::filesystem::path& p, std::shared_ptr<const WavData>& out);
    void TrimCacheLocked();
    void InvalidateSample(const std::filesystem::path& p);
    std::shared_ptr<const WavData> PickSample(const KeyAssignment& a, bool release);
    void PreviewSelected();
    void ToggleEnabled();
    void SaveConfig();
    static int PhysicalKeyId(const KeyEvent& e);

    HINSTANCE instance_{}; HWND main_{}; HWND status_{}; HWND volume_{}; HWND enable_{}; HWND selected_{}; HWND soundList_{}; HWND choose_{}; HWND assign_{}; HWND preview_{};
    HFONT titleFont_{}; HFONT bodyFont_{}; HFONT keyFont_{}; HBRUSH backgroundBrush_{};
    std::unordered_map<int,HWND> keyButtons_; std::unordered_map<HWND,int> buttonKeys_;
    std::unordered_set<int> activeKeys_;
    Config config_; SoundPack pack_{}; AudioEngine audio_; KeyboardEngine keyboard_;
    bool comInitialized_{};
    struct CacheEntry { std::shared_ptr<WavData> sample; size_t bytes{}; std::list<std::wstring>::iterator lru; };
    std::unordered_map<std::wstring,CacheEntry> cache_;
    std::list<std::wstring> lru_;
    std::mutex cacheMu_;
    size_t cacheBytes_{};
    std::unordered_set<std::wstring> rejectedSamples_;
    std::unordered_map<int,std::shared_ptr<const WavData>> pressSamples_;
    std::unordered_map<int,std::shared_ptr<const WavData>> releaseSamples_;
    static constexpr size_t kMaxCacheBytes=128u*1024u*1024u;
    std::mt19937 rng_{std::random_device{}()}; int selectedKey_=0x1E; bool shuttingDown_=false;
};
