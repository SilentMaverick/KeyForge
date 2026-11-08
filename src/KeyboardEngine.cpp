#include "KeyboardEngine.h"
#include "Logger.h"
#include <array>

bool KeyboardEngine::Initialize(HWND sink, Callback cb, std::wstring& error){
    sink_=sink; callback_=std::move(cb);
    RAWINPUTDEVICE rid{};
    rid.usUsagePage=0x01; rid.usUsage=0x06; rid.dwFlags=RIDEV_INPUTSINK; rid.hwndTarget=sink_;
    if(!RegisterRawInputDevices(&rid,1,sizeof(rid))){ error=L"Windows keyboard input monitoring could not be initialized."; return false; }
    registered_=true; down_.fill(false); return true;
}

void KeyboardEngine::Shutdown(){
    if(registered_){ RAWINPUTDEVICE rid{}; rid.usUsagePage=0x01; rid.usUsage=0x06; rid.dwFlags=RIDEV_REMOVE; RegisterRawInputDevices(&rid,1,sizeof(rid)); }
    registered_=false; callback_={}; sink_=nullptr; down_.fill(false);
}

UINT16 KeyboardEngine::NormalizeMakeCode(const RAWKEYBOARD& k){
    // Preserve E0/E1 in the high bits so right-side modifiers/navigation keys remain distinct.
    const USHORT code=k.MakeCode;
    if(code==0xE0 || code==0xE1) return 0;
    return code;
}

void KeyboardEngine::ProcessRawInput(HRAWINPUT input){
    UINT size=0;
    if(GetRawInputData(input,RID_INPUT,nullptr,&size,sizeof(RAWINPUTHEADER))==(UINT)-1 || size==0 || size>kRawBufferSize) return;
    std::array<BYTE,kRawBufferSize> buf{};
    if(GetRawInputData(input,RID_INPUT,buf.data(),&size,sizeof(RAWINPUTHEADER))!=(UINT)size) return;
    auto* raw=reinterpret_cast<RAWINPUT*>(buf.data());
    if(raw->header.dwType!=RIM_TYPEKEYBOARD) return;
    const auto& k=raw->data.keyboard;
    const bool isBreak=(k.Flags & RI_KEY_BREAK)!=0;
    const UINT16 code=NormalizeMakeCode(k);
    if(code==0) return;
    const size_t idx=(static_cast<size_t>(code) + ((k.Flags & RI_KEY_E1)?256u:0u) + ((k.Flags & RI_KEY_E0)?128u:0u)) % down_.size();
    if(!isBreak){ if(down_[idx]) return; down_[idx]=true; }
    else down_[idx]=false;
    if(callback_) callback_({code,k.Flags,!isBreak,static_cast<ULONGLONG>(static_cast<DWORD>(GetMessageTime()))});
}
