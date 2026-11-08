#pragma once
#include <windows.h>
#include <functional>
#include <array>
#include <string>
#include <utility>

struct KeyEvent { USHORT makeCode{}; USHORT flags{}; bool down{}; ULONGLONG timestamp{}; };

class KeyboardEngine final {
public:
    static constexpr size_t kRawBufferSize=1024;
    using Callback=std::function<void(const KeyEvent&)>;
    bool Initialize(HWND sink, Callback cb, std::wstring& error);
    void Shutdown();
    void ProcessRawInput(HRAWINPUT input);
    static UINT16 NormalizeMakeCode(const RAWKEYBOARD& k);
private:
    HWND sink_{}; Callback callback_; bool registered_{}; std::array<bool,512> down_{};
};
