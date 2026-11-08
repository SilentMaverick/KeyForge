#pragma once
#include "Wav.h"
#include <xaudio2.h>
#include <array>
#include <atomic>
#include <mutex>
#include <memory>
#include <cstdint>

class AudioEngine final : public IXAudio2EngineCallback {
public:
    static constexpr size_t kMaxVoices = 32;
    AudioEngine() = default;
    ~AudioEngine();
    bool Initialize(std::wstring& error);
    void Shutdown();
    bool IsReady() const noexcept { return ready_.load(std::memory_order_acquire); }
    bool Play(const std::shared_ptr<const WavData>& sample, float volume, float pitchSemitones, float pan);
    void StopAll();
    void OnProcessingPassStart() override {}
    void OnProcessingPassEnd() override {}
    void OnCriticalError(HRESULT Error) override;
private:
    struct VoiceToken { size_t index{}; uint64_t generation{}; };
    struct VoiceCallback final : IXAudio2VoiceCallback {
        AudioEngine* owner{}; size_t index{};
        void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
        void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
        void STDMETHODCALLTYPE OnStreamEnd() override {}
        void STDMETHODCALLTYPE OnBufferStart(void*) override {}
        void STDMETHODCALLTYPE OnBufferEnd(void* context) override;
        void STDMETHODCALLTYPE OnLoopEnd(void*) override {}
        void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) override {}
    };
    struct Slot {
        IXAudio2SourceVoice* voice{};
        VoiceCallback callback{};
        std::atomic<bool> busy{false};
        std::atomic<uint64_t> generation{0};
        VoiceToken token{};
    };
    void Recycle(size_t index, uint64_t generation);
    IXAudio2* xa_{nullptr}; IXAudio2MasteringVoice* master_{nullptr};
    std::array<Slot,kMaxVoices> slots_{};
    std::mutex stateMu_;
    std::atomic<bool> ready_{false};
};
