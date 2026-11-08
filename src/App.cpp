#include "App.h"
#include "Logger.h"
#include <commctrl.h>
#include <commdlg.h>
#include <filesystem>
#include <random>
#include <algorithm>
#include <cmath>
#include <cwctype>
#include <tuple>
#include <vector>
#pragma comment(lib,"comctl32.lib")

namespace {
constexpr int IDC_ENABLE=1001, IDC_VOLUME=1002, IDC_SELECTED=1003, IDC_CHOOSE=1004, IDC_PREVIEW=1005, IDC_STATUS=1006, IDC_ASSIGN=1007;
constexpr UINT WM_KEYFORGE_KEY=(WM_APP+1);
constexpr COLORREF kBackground=RGB(16,20,29), kPanel=RGB(25,31,43), kKey=RGB(41,51,68), kKeyEdge=RGB(59,72,93), kText=RGB(235,240,249), kMuted=RGB(151,164,184), kAccent=RGB(110,132,255), kActive=RGB(31,161,138);
std::wstring KeyLabel(int k){static const std::unordered_map<int,std::wstring> m={{0x01,L"ESC"},{0x02,L"1"},{0x03,L"2"},{0x04,L"3"},{0x05,L"4"},{0x06,L"5"},{0x07,L"6"},{0x08,L"7"},{0x09,L"8"},{0x0A,L"9"},{0x0B,L"0"},{0x0C,L"-"},{0x0D,L"="},{0x0E,L"⌫"},{0x0F,L"TAB"},{0x10,L"Q"},{0x11,L"W"},{0x12,L"E"},{0x13,L"R"},{0x14,L"T"},{0x15,L"Y"},{0x16,L"U"},{0x17,L"I"},{0x18,L"O"},{0x19,L"P"},{0x1A,L"["},{0x1B,L"]"},{0x1C,L"ENTER"},{0x1D,L"CTRL"},{0x1E,L"A"},{0x1F,L"S"},{0x20,L"D"},{0x21,L"F"},{0x22,L"G"},{0x23,L"H"},{0x24,L"J"},{0x25,L"K"},{0x26,L"L"},{0x27,L";"},{0x28,L"'"},{0x29,L"`"},{0x2A,L"SHIFT"},{0x2B,L"\\"},{0x2C,L"Z"},{0x2D,L"X"},{0x2E,L"C"},{0x2F,L"V"},{0x30,L"B"},{0x31,L"N"},{0x32,L"M"},{0x33,L","},{0x34,L"."},{0x35,L"/"},{0x36,L"SHIFT"},{0x38,L"ALT"},{0x39,L"SPACE"},{0x3A,L"CAPS"},{0x3B,L"F1"},{0x3C,L"F2"},{0x3D,L"F3"},{0x3E,L"F4"},{0x3F,L"F5"},{0x40,L"F6"},{0x41,L"F7"},{0x42,L"F8"},{0x43,L"F9"},{0x44,L"F10"},{0x57,L"F11"},{0x58,L"F12"},{0x11D,L"CTRL"},{0x136,L"SHIFT"},{0x138,L"ALT"},{0x147,L"HOME"},{0x148,L"↑"},{0x149,L"PG UP"},{0x14B,L"←"},{0x14D,L"→"},{0x14F,L"END"},{0x150,L"↓"},{0x151,L"PG DN"},{0x152,L"INS"},{0x153,L"DEL"},{0x15B,L"WIN"},{0x15C,L"WIN"},{0x15D,L"MENU"}};auto it=m.find(k);return it==m.end()?L"KEY":it->second;}
void SetFont(HWND h, HFONT f){SendMessageW(h,WM_SETFONT,(WPARAM)f,TRUE);}
void FillRoundRect(HDC dc,const RECT& r,COLORREF fill,COLORREF edge,int radius){HBRUSH b=CreateSolidBrush(fill);HPEN p=CreatePen(PS_SOLID,1,edge);HGDIOBJ ob=SelectObject(dc,b),op=SelectObject(dc,p);RoundRect(dc,r.left,r.top,r.right,r.bottom,radius,radius);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(p);}
}

bool App::Initialize(HINSTANCE h,int show,std::wstring& error){
 instance_=h; InitCommonControls();
 HRESULT co=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
 if(FAILED(co) && co!=RPC_E_CHANGED_MODE){ error=L"Windows COM initialization failed."; return false; }
 comInitialized_=SUCCEEDED(co);
 InitLogger();
 config_.ResetDefaults(); if(!config_.Load(error)){ LogError(error); config_.ResetDefaults(); }
 for(auto& [key,assignment]:config_.assignments){(void)key;if(!assignment.press.empty()&&assignment.press.front()==L"click.wav")assignment.press.front()=L"mechanical_default.wav";}
 wchar_t exe[MAX_PATH]{};GetModuleFileNameW(h,exe,MAX_PATH);const auto exeRoot=std::filesystem::path(exe).parent_path();const auto packRoot=config_.SoundPackRoot();std::error_code fsError;std::filesystem::create_directories(packRoot,fsError);
 const auto bundledSounds=exeRoot/L"sounds";if(std::filesystem::exists(bundledSounds,fsError)){for(const auto& item:std::filesystem::directory_iterator(bundledSounds,fsError)){if(fsError)break;if(!item.is_regular_file())continue;auto ext=item.path().extension().wstring();std::transform(ext.begin(),ext.end(),ext.begin(),[](wchar_t c){return static_cast<wchar_t>(std::towlower(c));});if(ext==L".wav"){const auto target=packRoot/item.path().filename();if(!std::filesystem::exists(target,fsError))std::filesystem::copy_file(item.path(),target,std::filesystem::copy_options::skip_existing,fsError);fsError.clear();}}}
 if(!LoadSoundPack(packRoot,pack_,error)){ LogError(error); }
 for(const auto& [k,a] : config_.assignments){ if(!a.press.empty()) pack_.assignments[k]=a; }
 pressSamples_.clear(); releaseSamples_.clear();
 for(const auto& [k,a] : pack_.assignments){
   if(!a.press.empty()){ auto sample=PickSample(a,false); if(sample) pressSamples_[k]=sample; }
   if(!a.release.empty()){ auto sample=PickSample(a,true); if(sample) releaseSamples_[k]=sample; }
 }
 if(!audio_.Initialize(error)){ LogError(error); /* safe disabled mode */ config_.enabled=false; }
 backgroundBrush_=CreateSolidBrush(kBackground);titleFont_=CreateFontW(-30,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");bodyFont_=CreateFontW(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");keyFont_=CreateFontW(-15,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");
 WNDCLASSEXW wc{sizeof(wc)};wc.hInstance=h;wc.lpfnWndProc=WndProc;wc.lpszClassName=L"KEYFORGE_MAIN";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=backgroundBrush_;wc.hIcon=LoadIconW(h,MAKEINTRESOURCEW(101));RegisterClassExW(&wc);
 main_=CreateWindowExW(0,wc.lpszClassName,L"KEYFORGE | Mechanical Keyboard Sound",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,1320,700,nullptr,nullptr,h,this);
 if(!main_){error=L"KEYFORGE could not create its window.";return false;} BuildUI(main_); DragAcceptFiles(main_,TRUE);
 if(!keyboard_.Initialize(main_,[this](const KeyEvent&e){HandleKey(e);},error)){ LogError(error); }
 if(!RegisterHotKey(main_,1,MOD_CONTROL|MOD_ALT|MOD_SHIFT,'K')) LogError(L"Emergency hotkey could not be registered."); ShowWindow(main_,show);UpdateWindow(main_);return true;
}

void App::BuildUI(HWND hwnd){
  auto label=[&](const wchar_t* text,int x,int y,int w,int h){HWND c=CreateWindowW(L"STATIC",text,WS_CHILD|WS_VISIBLE,x,y,w,h,hwnd,nullptr,instance_,nullptr);SetFont(c,bodyFont_);return c;};
  auto action=[&](const wchar_t* text,int id,int x,int y,int w){HWND c=CreateWindowW(L"BUTTON",text,WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|WS_TABSTOP,x,y,w,38,hwnd,(HMENU)(INT_PTR)id,instance_,nullptr);SetFont(c,bodyFont_);return c;};
  label(L"KEYFORGE",34,20,390,44);HWND title=GetWindow(hwnd,GW_CHILD);SetFont(title,titleFont_);
  HWND tagline=label(L"Give every keystroke a mechanical sound.",36,62,580,25);SetWindowTextW(tagline,L"Turn your everyday keyboard into a mechanical soundboard.");
  status_=label(L"READY  ·  LOCAL AUDIO",790,34,225,28);SetWindowTextW(status_,L"READY  ·  SOUND ENGINE ON");
  enable_=CreateWindowW(L"BUTTON",L"Sound on",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_FLAT|WS_TABSTOP,1090,31,160,32,hwnd,(HMENU)(INT_PTR)IDC_ENABLE,instance_,nullptr);SendMessage(enable_,BM_SETCHECK,config_.enabled?BST_CHECKED:BST_UNCHECKED,0);SetFont(enable_,bodyFont_);
  label(L"MASTER VOLUME",36,111,122,25);volume_=CreateWindowW(TRACKBAR_CLASSW,L"",WS_CHILD|WS_VISIBLE|TBS_NOTICKS|WS_TABSTOP,160,101,220,35,hwnd,(HMENU)(INT_PTR)IDC_VOLUME,instance_,nullptr);SendMessage(volume_,TBM_SETRANGE,TRUE,MAKELONG(0,100));SendMessage(volume_,TBM_SETPOS,TRUE,(LPARAM)std::lround(config_.masterVolume*100));
  label(L"SELECTED KEY",405,111,100,25);selected_=CreateWindowW(L"STATIC",L"A",WS_CHILD|WS_VISIBLE|SS_CENTER,505,103,72,32,hwnd,(HMENU)(INT_PTR)IDC_SELECTED,instance_,nullptr);SetFont(selected_,keyFont_);
  label(L"SOUND LIBRARY",602,111,120,25);soundList_=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_TABSTOP,716,101,250,240,hwnd,nullptr,instance_,nullptr);SetFont(soundList_,bodyFont_);
  choose_=action(L"+  Add WAV",IDC_CHOOSE,982,99,105);assign_=action(L"Assign",IDC_ASSIGN,1095,99,96);preview_=action(L"Preview",IDC_PREVIEW,1199,99,87);
  label(L"YOUR KEYBOARD",36,174,230,25);label(L"Select a key, choose a sound, then assign it. Press a key to hear it.",240,174,820,25);
  struct Cap{int key;float units;float before;};constexpr int x0=36,keyW=51,gap=7;
  auto makeRow=[&](int y,const std::vector<Cap>& caps){float cursor=static_cast<float>(x0);for(const auto& cap:caps){cursor+=cap.before*(keyW+gap);int width=static_cast<int>(std::lround(cap.units*keyW+(cap.units-1)*gap));HWND b=CreateWindowW(L"BUTTON",KeyLabel(cap.key).c_str(),WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|WS_TABSTOP,static_cast<int>(std::lround(cursor)),y,width,46,hwnd,nullptr,instance_,nullptr);SetFont(b,keyFont_);keyButtons_[cap.key]=b;buttonKeys_[b]=cap.key;cursor+=width+gap;}};
  auto row=[&](std::initializer_list<Cap> caps){return std::vector<Cap>(caps);};
  makeRow(208,row({{0x01,1,0},{0x3B,1,.75f},{0x3C,1,0},{0x3D,1,0},{0x3E,1,0},{0x3F,1,.75f},{0x40,1,0},{0x41,1,0},{0x42,1,0},{0x43,1,.75f},{0x44,1,0},{0x57,1,0},{0x58,1,0}}));
  makeRow(264,row({{0x29,1,0},{0x02,1,0},{0x03,1,0},{0x04,1,0},{0x05,1,0},{0x06,1,0},{0x07,1,0},{0x08,1,0},{0x09,1,0},{0x0A,1,0},{0x0B,1,0},{0x0C,1,0},{0x0D,1,0},{0x0E,2,0}}));
  makeRow(320,row({{0x0F,1.5f,0},{0x10,1,0},{0x11,1,0},{0x12,1,0},{0x13,1,0},{0x14,1,0},{0x15,1,0},{0x16,1,0},{0x17,1,0},{0x18,1,0},{0x19,1,0},{0x1A,1,0},{0x1B,1,0},{0x2B,1.5f,0}}));
  makeRow(376,row({{0x3A,1.75f,0},{0x1E,1,0},{0x1F,1,0},{0x20,1,0},{0x21,1,0},{0x22,1,0},{0x23,1,0},{0x24,1,0},{0x25,1,0},{0x26,1,0},{0x27,1,0},{0x28,1,0},{0x1C,2.25f,0}}));
  makeRow(432,row({{0x2A,2.25f,0},{0x2C,1,0},{0x2D,1,0},{0x2E,1,0},{0x2F,1,0},{0x30,1,0},{0x31,1,0},{0x32,1,0},{0x33,1,0},{0x34,1,0},{0x35,1,0},{0x36,2.75f,0}}));
  makeRow(488,row({{0x1D,1.25f,0},{0x15B,1.25f,0},{0x38,1.25f,0},{0x39,6.5f,0},{0x138,1.25f,0},{0x15C,1.25f,0},{0x15D,1.25f,0},{0x11D,1.25f,0},{0x14B,1,1.0f},{0x148,1,0},{0x14D,1,0}}));
  HWND down=CreateWindowW(L"BUTTON",KeyLabel(0x150).c_str(),WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|WS_TABSTOP,1037,536,keyW,46,hwnd,nullptr,instance_,nullptr);SetFont(down,keyFont_);keyButtons_[0x150]=down;buttonKeys_[down]=0x150;
  label(L"TIP  ·  Add WAV files to your library. The sound files are copied into your user folder.",36,610,920,24);
  HWND emergency=label(L"Ctrl + Alt + Shift + K  ·  Emergency mute",1000,610,285,24);SetWindowTextW(emergency,L"CTRL + ALT + SHIFT + K  ·  MUTE");
  RefreshSoundLibrary();SelectKey(selectedKey_);InvalidateRect(hwnd,nullptr,TRUE);
}

void App::HandleKey(const KeyEvent& e){ if(shuttingDown_)return; int id=PhysicalKeyId(e); if(!e.down){RefreshKeyVisual(id,false); if(config_.enabled && audio_.IsReady() && config_.releaseSounds){auto it=pack_.assignments.find(id);if(it!=pack_.assignments.end()){auto s=releaseSamples_.find(id);if(s!=releaseSamples_.end())audio_.Play(s->second,it->second.volume*config_.masterVolume,it->second.pitch,it->second.pan);}}return;} RefreshKeyVisual(id,true); if(!config_.enabled||!audio_.IsReady()||!config_.pressSounds)return; auto it=pack_.assignments.find(id);if(it==pack_.assignments.end())return; auto sit=pressSamples_.find(id);if(sit==pressSamples_.end())return; auto s=sit->second;std::uniform_real_distribution<float> rp(-it->second.randomPitch,it->second.randomPitch),rv(-it->second.randomVolume,it->second.randomVolume);float pitch=it->second.pitch+(config_.randomization?rp(rng_):0);float vol=it->second.volume*(1.0f+(config_.randomization?rv(rng_):0));audio_.Play(s,vol*config_.masterVolume,pitch,it->second.pan);}

int App::PhysicalKeyId(const KeyEvent&e){int id=e.makeCode;if(e.flags&RI_KEY_E0)id|=0x100;return id;}
void App::RefreshKeyVisual(int key,bool down){auto it=keyButtons_.find(key);if(it==keyButtons_.end())return;if(down)activeKeys_.insert(key);else activeKeys_.erase(key);InvalidateRect(it->second,nullptr,FALSE);if(status_)SetWindowTextW(status_,down?(config_.enabled?L"KEY PRESSED  ·  SOUND PLAYING":L"KEY PRESSED  ·  MUTED"):(config_.enabled?L"SOUND ENGINE ON  ·  READY":L"MUTED  ·  SOUND ENGINE OFF"));}
void App::SelectKey(int key){auto old=keyButtons_.find(selectedKey_);if(old!=keyButtons_.end())InvalidateRect(old->second,nullptr,FALSE);selectedKey_=key;SetWindowTextW(selected_,KeyLabel(key).c_str());auto current=keyButtons_.find(selectedKey_);if(current!=keyButtons_.end())InvalidateRect(current->second,nullptr,FALSE);if(status_)SetWindowTextW(status_,(L"SELECTED  ·  "+KeyLabel(key)).c_str());}

void App::TrimCacheLocked(){
    size_t attempts=lru_.size();
    while(cacheBytes_>kMaxCacheBytes && !lru_.empty() && attempts--){
        const auto key=lru_.back();
        auto it=cache_.find(key);
        if(it==cache_.end()){ lru_.pop_back(); continue; }
        if(it->second.sample.use_count()>1){
            lru_.splice(lru_.begin(),lru_,it->second.lru);
            continue;
        }
        cacheBytes_-=it->second.bytes;
        lru_.erase(it->second.lru);
        cache_.erase(it);
    }
}

bool App::LoadSample(const std::filesystem::path&p,std::shared_ptr<const WavData>&out){
    std::error_code ec;
    const auto absolute=std::filesystem::weakly_canonical(p,ec);
    if(ec || !std::filesystem::is_regular_file(absolute,ec) || ec){ LogError(L"Sound asset path is invalid."); return false; }
    const std::wstring k=absolute.wstring();
    {
        std::lock_guard<std::mutex>l(cacheMu_);
        if(rejectedSamples_.contains(k)) return false;
        auto it=cache_.find(k);
        if(it!=cache_.end()){
            lru_.splice(lru_.begin(),lru_,it->second.lru);
            out=it->second.sample; return true;
        }
    }
    auto w=std::make_shared<WavData>();
    std::wstring err;
    if(!LoadPcmWav(k,*w,err)){
        LogError(err);
        std::lock_guard<std::mutex>l(cacheMu_);
        rejectedSamples_.insert(k);
        return false;
    }
    if(w->pcm.size()>kMaxCacheBytes){ LogError(L"Decoded sound exceeds the cache safety limit."); return false; }
    {
        std::lock_guard<std::mutex>l(cacheMu_);
        auto [it,inserted]=cache_.try_emplace(k);
        if(inserted){
            lru_.push_front(k);
            it->second={w,w->pcm.size(),lru_.begin()};
            cacheBytes_+=w->pcm.size();
        } else {
            lru_.splice(lru_.begin(),lru_,it->second.lru);
            w=it->second.sample;
        }
        rejectedSamples_.erase(k);
        TrimCacheLocked();
        out=w;
    }
    return true;
}
void App::InvalidateSample(const std::filesystem::path& p){
    std::error_code ec;
    const auto k=std::filesystem::weakly_canonical(p,ec).wstring();
    if(ec || k.empty()) return;
    std::lock_guard<std::mutex> l(cacheMu_);
    rejectedSamples_.erase(k);
    auto it=cache_.find(k);
    if(it!=cache_.end()){
        cacheBytes_-=it->second.bytes;
        lru_.erase(it->second.lru);
        cache_.erase(it);
    }
}

std::shared_ptr<const WavData> App::PickSample(const KeyAssignment&a,bool release){
    const auto&v=release?a.release:a.press;
    if(v.empty() || v.front().empty()) return {};
    std::filesystem::path rel=v.front();
    if(rel.is_absolute()) return {};
    std::error_code ec;
    const auto root=std::filesystem::weakly_canonical(pack_.root,ec);
    if(ec) return {};
    const auto path=std::filesystem::weakly_canonical(pack_.root/rel,ec);
    if(ec) return {};
    const auto relative=path.lexically_relative(root);
    if(relative.empty() || relative==std::filesystem::path(L"..") || relative.native().rfind(std::wstring(L"..")+std::filesystem::path::preferred_separator,0)==0) return {};
    std::shared_ptr<const WavData>s;
    return LoadSample(path,s)?s:std::shared_ptr<const WavData>{};
}
void App::PreviewSelected(){const auto name=SelectedSoundName();std::shared_ptr<const WavData> sample;if(!name.empty()){if(!LoadSample(pack_.root/name,sample)){MessageBoxW(main_,L"This file is not a supported PCM WAV sound.",L"KEYFORGE",MB_ICONWARNING);return;}}else{auto s=pressSamples_.find(selectedKey_);if(s!=pressSamples_.end())sample=s->second;}if(sample)audio_.Play(sample,config_.masterVolume,0.0f,0.0f);}
void App::ToggleEnabled(){config_.enabled=SendMessage(enable_,BM_GETCHECK,0,0)==BST_CHECKED;if(!config_.enabled)audio_.StopAll();SetWindowTextW(status_,config_.enabled?L"SOUND ENGINE ON  ·  READY":L"MUTED  ·  SOUND ENGINE OFF");SaveConfig();}
void App::SaveConfig(){std::wstring e;if(!config_.Save(e))LogError(e);}
bool App::AddSoundToLibrary(const std::filesystem::path& source,std::wstring& storedName){
    auto ext=source.extension().wstring();std::transform(ext.begin(),ext.end(),ext.begin(),[](wchar_t c){return static_cast<wchar_t>(std::towlower(c));});if(ext!=L".wav")return false;
    std::error_code ec;std::filesystem::create_directories(pack_.root,ec);if(ec)return false;
    auto name=source.filename();auto destination=pack_.root/name;int suffix=2;while(std::filesystem::exists(destination,ec)){destination=pack_.root/(source.stem().wstring()+L" ("+std::to_wstring(suffix++)+L")"+source.extension().wstring());}
    std::filesystem::copy_file(source,destination,std::filesystem::copy_options::none,ec);if(ec)return false;
    std::shared_ptr<const WavData> sample;if(!LoadSample(destination,sample)){std::filesystem::remove(destination,ec);return false;}storedName=destination.filename().wstring();return true;
}
void App::RefreshSoundLibrary(const std::wstring& selectName){
    if(!soundList_)return;const std::wstring previous=selectName.empty()?SelectedSoundName():selectName;SendMessageW(soundList_,CB_RESETCONTENT,0,0);std::vector<std::wstring> names;std::error_code ec;
    for(std::filesystem::directory_iterator it(pack_.root,ec),end;!ec&&it!=end;it.increment(ec)){if(!it->is_regular_file(ec))continue;auto ext=it->path().extension().wstring();std::transform(ext.begin(),ext.end(),ext.begin(),[](wchar_t c){return static_cast<wchar_t>(std::towlower(c));});if(ext==L".wav")names.push_back(it->path().filename().wstring());}
    std::sort(names.begin(),names.end(),[](const auto&a,const auto&b){return _wcsicmp(a.c_str(),b.c_str())<0;});int selected=-1;for(const auto& name:names){const int i=static_cast<int>(SendMessageW(soundList_,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str())));if(name==previous)selected=i;}if(selected<0&&!names.empty())selected=0;if(selected>=0)SendMessageW(soundList_,CB_SETCURSEL,selected,0);
}
std::wstring App::SelectedSoundName() const{if(!soundList_)return {};const LRESULT index=SendMessageW(soundList_,CB_GETCURSEL,0,0);if(index==CB_ERR)return {};const LRESULT length=SendMessageW(soundList_,CB_GETLBTEXTLEN,index,0);if(length==CB_ERR||length<0)return {};std::wstring name(static_cast<size_t>(length)+1,L'\0');SendMessageW(soundList_,CB_GETLBTEXT,index,reinterpret_cast<LPARAM>(name.data()));name.resize(static_cast<size_t>(length));return name;}
void App::AssignSelectedSound(){const auto name=SelectedSoundName();if(name.empty()){MessageBoxW(main_,L"Add a WAV sound to your library first.",L"KEYFORGE",MB_ICONINFORMATION);return;}std::shared_ptr<const WavData> sample;if(!LoadSample(pack_.root/name,sample)){MessageBoxW(main_,L"This file is not a supported PCM WAV sound.",L"KEYFORGE",MB_ICONWARNING);return;}auto& assignment=pack_.assignments[selectedKey_];assignment.press={name};config_.assignments[selectedKey_]=assignment;pressSamples_[selectedKey_]=sample;SaveConfig();SetWindowTextW(status_,(L"ASSIGNED  ·  "+KeyLabel(selectedKey_)).c_str());InvalidateRect(keyButtons_[selectedKey_],nullptr,FALSE);}
void App::ImportForSelected(){OPENFILENAMEW ofn{sizeof(ofn)};wchar_t file[MAX_PATH]{};ofn.hwndOwner=main_;ofn.lpstrFilter=L"PCM WAV sounds (*.wav)\0*.wav\0";ofn.lpstrFile=file;ofn.nMaxFile=MAX_PATH;ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;if(!GetOpenFileNameW(&ofn))return;std::wstring name;if(!AddSoundToLibrary(file,name)){MessageBoxW(main_,L"That file could not be added. Use a valid PCM WAV sound.",L"KEYFORGE",MB_ICONWARNING);return;}RefreshSoundLibrary(name);SetWindowTextW(status_,L"WAV ADDED TO SOUND LIBRARY");}
void App::ImportDropped(HDROP drop){
    UINT n=DragQueryFileW(drop,0xFFFFFFFF,nullptr,0);
    if(n){
        wchar_t f[MAX_PATH]{};
        DragQueryFileW(drop,0,f,MAX_PATH);
        std::filesystem::path p=f;
        std::wstring name;if(AddSoundToLibrary(p,name)){RefreshSoundLibrary(name);AssignSelectedSound();PreviewSelected();}
    }
    DragFinish(drop);
}

LRESULT CALLBACK App::WndProc(HWND h,UINT m,WPARAM w,LPARAM l){App*p=reinterpret_cast<App*>(GetWindowLongPtrW(h,GWLP_USERDATA));if(m==WM_NCCREATE){auto*cs=reinterpret_cast<CREATESTRUCTW*>(l);p=static_cast<App*>(cs->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,(LONG_PTR)p);}return p?p->Handle(h,m,w,l):DefWindowProcW(h,m,w,l);}
LRESULT App::Handle(HWND h,UINT m,WPARAM w,LPARAM l){
 switch(m){
 case WM_INPUT:keyboard_.ProcessRawInput((HRAWINPUT)l);return 0;
 case WM_HOTKEY:config_.enabled=false;SendMessage(enable_,BM_SETCHECK,BST_UNCHECKED,0);audio_.StopAll();SaveConfig();if(status_)SetWindowTextW(status_,L"MUTED  ·  PRESS SOUND ON TO RESUME");return 0;
 case WM_ERASEBKGND:{HDC dc=(HDC)w;RECT client{};GetClientRect(h,&client);FillRect(dc,&client,backgroundBrush_);RECT header{22,16,1290,88},controls{22,94,1290,150},keyboard{22,164,1290,594};FillRoundRect(dc,header,kPanel,kPanel,16);FillRoundRect(dc,controls,kPanel,kPanel,14);FillRoundRect(dc,keyboard,kPanel,kPanel,16);return 1;}
 case WM_CTLCOLORSTATIC:{HDC dc=(HDC)w;SetBkMode(dc,TRANSPARENT);SetTextColor(dc,(HWND)l==status_?RGB(122,223,190):kText);return (LRESULT)backgroundBrush_;}
 case WM_CTLCOLORBTN:{HDC dc=(HDC)w;SetBkMode(dc,TRANSPARENT);SetTextColor(dc,kText);return (LRESULT)backgroundBrush_;}
 case WM_DRAWITEM:{auto*item=reinterpret_cast<DRAWITEMSTRUCT*>(l);const HWND control=item->hwndItem;const auto keyIt=buttonKeys_.find(control);const bool isKey=keyIt!=buttonKeys_.end();const int id=static_cast<int>(item->CtlID);COLORREF fill=isKey?kKey:RGB(49,61,82),edge=isKey?kKeyEdge:RGB(68,82,108);if(id==IDC_ASSIGN){fill=RGB(70,89,191);edge=kAccent;}if(isKey){const int key=keyIt->second;if(key==selectedKey_){fill=kAccent;edge=RGB(154,169,255);}else if(activeKeys_.contains(key)){fill=kActive;edge=RGB(100,215,190);}else if(!pressSamples_.contains(key)){fill=RGB(31,38,50);edge=RGB(48,57,72);}}if(item->itemState&ODS_DISABLED)fill=RGB(35,40,50);if(item->itemState&ODS_SELECTED)fill=RGB(85,104,157);FillRoundRect(item->hDC,item->rcItem,fill,edge,isKey?10:9);wchar_t text[128]{};GetWindowTextW(control,text,128);SetBkMode(item->hDC,TRANSPARENT);SetTextColor(item->hDC,(item->itemState&ODS_DISABLED)?kMuted:kText);HFONT font=isKey?keyFont_:bodyFont_;HGDIOBJ old=SelectObject(item->hDC,font);RECT r=item->rcItem;InflateRect(&r,-4,-2);DrawTextW(item->hDC,text,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);SelectObject(item->hDC,old);return TRUE;}
 case WM_COMMAND:{int id=LOWORD(w);if(id==IDC_ENABLE)ToggleEnabled();else if(id==IDC_CHOOSE)ImportForSelected();else if(id==IDC_ASSIGN)AssignSelectedSound();else if(id==IDC_PREVIEW)PreviewSelected();else {auto it=buttonKeys_.find((HWND)l);if(it!=buttonKeys_.end())SelectKey(it->second);}return 0;}
 case WM_HSCROLL:if((HWND)l==volume_){config_.masterVolume=SendMessage(volume_,TBM_GETPOS,0,0)/100.0f;SaveConfig();}return 0;
 case WM_DROPFILES:ImportDropped((HDROP)w);return 0;
 case WM_DESTROY:shuttingDown_=true;UnregisterHotKey(h,1);keyboard_.Shutdown();audio_.Shutdown();SaveConfig();PostQuitMessage(0);return 0;
 }
 return DefWindowProcW(h,m,w,l);
}

int App::Run(){MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}if(comInitialized_)CoUninitialize();if(titleFont_)DeleteObject(titleFont_);if(bodyFont_)DeleteObject(bodyFont_);if(keyFont_)DeleteObject(keyFont_);if(backgroundBrush_)DeleteObject(backgroundBrush_);return (int)msg.wParam;}
