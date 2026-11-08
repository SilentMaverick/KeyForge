#include "App.h"
#include <windows.h>
int APIENTRY wWinMain(HINSTANCE h,HINSTANCE,LPWSTR,int show){App app;std::wstring error;if(!app.Initialize(h,show,error)){MessageBoxW(nullptr,error.c_str(),L"KEYFORGE",MB_ICONERROR);return 1;}return app.Run();}
