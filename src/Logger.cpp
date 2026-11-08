#include "Logger.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <mutex>
namespace { std::wstring g_path; std::mutex g_mu;
void Write(const wchar_t* level,const std::wstring& m){ std::lock_guard<std::mutex> l(g_mu); if(g_path.empty()) return; std::wofstream f(g_path,std::ios::app); if(f) f<<level<<L" "<<m<<L"\n"; }
}
void InitLogger(){ PWSTR p=nullptr; if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&p))){ std::wstring d=p; CoTaskMemFree(p); d+=L"\\KEYFORGE"; CreateDirectoryW(d.c_str(),nullptr); g_path=d+L"\\keyforge.log"; } }
void LogInfo(const std::wstring& m){Write(L"[INFO]",m);} void LogError(const std::wstring& m){Write(L"[ERROR]",m);}
