#include "Config.h"
#include "Logger.h"
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <limits>

namespace {
std::filesystem::path AppRoot(){ PWSTR p=nullptr; std::filesystem::path r; if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&p))){r=std::filesystem::path(p)/L"KEYFORGE";CoTaskMemFree(p);} return r; }
std::string Esc(const std::wstring& w){
 if(w.empty())return {};const int n=WideCharToMultiByte(CP_UTF8,0,w.data(),static_cast<int>(w.size()),nullptr,0,nullptr,nullptr);if(n<=0)return {};std::string bytes(static_cast<size_t>(n),'\0');WideCharToMultiByte(CP_UTF8,0,w.data(),static_cast<int>(w.size()),bytes.data(),n,nullptr,nullptr);std::string out;out.reserve(bytes.size());for(unsigned char c:bytes){if(c=='\\'||c=='"')out.push_back('\\');out.push_back(static_cast<char>(c));}return out;
}
std::wstring Unesc(const std::string& s){
 std::string bytes;bytes.reserve(s.size());bool escaped=false;for(char c:s){if(escaped){bytes.push_back(c);escaped=false;}else if(c=='\\')escaped=true;else bytes.push_back(c);}if(bytes.empty())return {};const int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(bytes.size()),nullptr,0);if(n<=0){std::wstring fallback;for(unsigned char c:bytes)fallback.push_back(static_cast<wchar_t>(c));return fallback;}std::wstring out(static_cast<size_t>(n),L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(bytes.size()),out.data(),n);return out;
}
std::string ReadAll(const std::filesystem::path& p){std::ifstream f(p);return std::string((std::istreambuf_iterator<char>(f)),{});}
int Key(const char* name){
 static const std::unordered_map<std::string,int> m=[](){
  std::unordered_map<std::string,int> v;
  const std::pair<const char*,int> letters[]={{"A",0x1E},{"B",0x30},{"C",0x2E},{"D",0x20},{"E",0x12},{"F",0x21},{"G",0x22},{"H",0x23},{"I",0x17},{"J",0x24},{"K",0x25},{"L",0x26},{"M",0x32},{"N",0x31},{"O",0x18},{"P",0x19},{"Q",0x10},{"R",0x13},{"S",0x1F},{"T",0x14},{"U",0x16},{"V",0x2F},{"W",0x11},{"X",0x2D},{"Y",0x15},{"Z",0x2C}};
  for(const auto& entry:letters)v.emplace(entry.first,entry.second);
  const char* digits[]={"1","2","3","4","5","6","7","8","9","0"}; for(int i=0;i<10;++i)v.emplace(digits[i],0x02+i);
  const std::pair<const char*,int> keys[]={{"Space",0x39},{"Enter",0x1C},{"Backspace",0x0E},{"Tab",0x0F},{"Esc",0x01},{"Shift",0x2A},{"Ctrl",0x1D},{"Alt",0x38},{"Left",0x14B},{"Right",0x14D},{"Up",0x148},{"Down",0x150},{"Grave",0x29},{"Minus",0x0C},{"Equals",0x0D},{"LeftBracket",0x1A},{"RightBracket",0x1B},{"Backslash",0x2B},{"CapsLock",0x3A},{"Semicolon",0x27},{"Apostrophe",0x28},{"Comma",0x33},{"Period",0x34},{"Slash",0x35},{"RightShift",0x36},{"RightCtrl",0x11D},{"RightAlt",0x138},{"LeftWin",0x15B},{"RightWin",0x15C},{"Menu",0x15D},{"Insert",0x152},{"Home",0x147},{"PageUp",0x149},{"Delete",0x153},{"End",0x14F},{"PageDown",0x151}};
  for(const auto& entry:keys)v.emplace(entry.first,entry.second);
  for(int i=0;i<10;++i)v.emplace("F"+std::to_string(i+1),0x3B+i); v.emplace("F11",0x57);v.emplace("F12",0x58);
  return v;
 }(); auto it=m.find(name);return it==m.end()?-1:it->second;
}
std::string KeyName(int k){
 static const std::unordered_map<int,std::string> m=[](){std::unordered_map<int,std::string> v;
  const std::pair<const char*,int> letters[]={{"A",0x1E},{"B",0x30},{"C",0x2E},{"D",0x20},{"E",0x12},{"F",0x21},{"G",0x22},{"H",0x23},{"I",0x17},{"J",0x24},{"K",0x25},{"L",0x26},{"M",0x32},{"N",0x31},{"O",0x18},{"P",0x19},{"Q",0x10},{"R",0x13},{"S",0x1F},{"T",0x14},{"U",0x16},{"V",0x2F},{"W",0x11},{"X",0x2D},{"Y",0x15},{"Z",0x2C}};for(const auto& e:letters)v.emplace(e.second,e.first);
  const char* digits[]={"1","2","3","4","5","6","7","8","9","0"};for(int i=0;i<10;++i)v.emplace(0x02+i,digits[i]);
  const std::pair<const char*,int> keys[]={{"Space",0x39},{"Enter",0x1C},{"Backspace",0x0E},{"Tab",0x0F},{"Esc",0x01},{"Shift",0x2A},{"Ctrl",0x1D},{"Alt",0x38},{"Left",0x14B},{"Right",0x14D},{"Up",0x148},{"Down",0x150},{"Grave",0x29},{"Minus",0x0C},{"Equals",0x0D},{"LeftBracket",0x1A},{"RightBracket",0x1B},{"Backslash",0x2B},{"CapsLock",0x3A},{"Semicolon",0x27},{"Apostrophe",0x28},{"Comma",0x33},{"Period",0x34},{"Slash",0x35},{"RightShift",0x36},{"RightCtrl",0x11D},{"RightAlt",0x138},{"LeftWin",0x15B},{"RightWin",0x15C},{"Menu",0x15D},{"Insert",0x152},{"Home",0x147},{"PageUp",0x149},{"Delete",0x153},{"End",0x14F},{"PageDown",0x151}};for(const auto& e:keys)v.emplace(e.second,e.first);
  for(int i=0;i<10;++i)v.emplace(0x3B+i,"F"+std::to_string(i+1));v.emplace(0x57,"F11");v.emplace(0x58,"F12");return v;}();auto it=m.find(k);return it==m.end()?"Unknown":it->second;
}
const std::vector<int>& SupportedKeys(){static const std::vector<int> keys=[](){std::vector<int> v;for(int k=0x01;k<=0x36;++k)v.push_back(k);for(int k=0x38;k<=0x44;++k)v.push_back(k);for(int k:{0x57,0x58,0x11D,0x138,0x147,0x148,0x149,0x14B,0x14D,0x14F,0x150,0x151,0x152,0x153,0x15B,0x15C,0x15D})v.push_back(k);return v;}();return keys;}
}

void Config::ResetDefaults(){assignments.clear();for(int k:SupportedKeys()){KeyAssignment a;a.press={L"mechanical_default.wav"};a.volume=(k==0x39?0.8f:1.0f);assignments[k]=a;}}

bool Config::Load(std::wstring& error){
    root_=AppRoot();
    if(root_.empty()){ error=L"Windows user data folder could not be located."; return false; }
    std::error_code ec; std::filesystem::create_directories(root_,ec);
    auto p=root_/L"config.json";
    if(!std::filesystem::exists(p)){ ResetDefaults(); return Save(error); }
    std::error_code sec;
    const auto fileSize=std::filesystem::file_size(p,sec);
    if(sec || fileSize>1024u*1024u){
        std::filesystem::path backup=p; backup+=L".invalid";
        std::filesystem::copy_file(p,backup,std::filesystem::copy_options::overwrite_existing,ec);
        ResetDefaults(); Save(error); return true;
    }
    const std::string s=ReadAll(p);
    if(s.empty() || s.front()!='{' || s.back()!='}'){
        std::filesystem::path backup=p; backup+=L".invalid";
        std::filesystem::copy_file(p,backup,std::filesystem::copy_options::overwrite_existing,ec);
        ResetDefaults(); Save(error); return true;
    }
    ResetDefaults();
    try {
        std::smatch m;
        std::regex rv(R"("masterVolume"\s*:\s*([0-9.]+))"); if(std::regex_search(s,m,rv)) masterVolume=std::clamp(std::stof(m[1]),0.0f,1.0f);
        std::regex re(R"("enabled"\s*:\s*(true|false))"); if(std::regex_search(s,m,re)) enabled=m[1]=="true";
        std::regex rr(R"("randomization"\s*:\s*(true|false))"); if(std::regex_search(s,m,rr)) randomization=m[1]=="true";
        std::regex rp(R"json("packName"\s*:\s*"([^"]*)")json"); if(std::regex_search(s,m,rp)) packName=Unesc(m[1]);
        std::regex ra(R"json("([A-Za-z0-9]+)"\s*:\s*"([^"]*)")json");
        for(auto it=std::sregex_iterator(s.begin(),s.end(),ra);it!=std::sregex_iterator();++it){
            const int k=Key((*it)[1].str().c_str()); if(k<0) continue;
            assignments[k].press={Unesc((*it)[2].str())};
        }
        return true;
    } catch(...) {
        std::filesystem::path backup=p; backup+=L".invalid"; std::filesystem::copy_file(p,backup,std::filesystem::copy_options::overwrite_existing,ec);
        ResetDefaults(); Save(error); return true;
    }
}

bool Config::Save(std::wstring& error) const {
    try {
        std::filesystem::create_directories(root_);
        const auto p=root_/L"config.json"; const auto tmp=root_/L"config.json.tmp";
        if(packName.size()>128){ error=L"Configuration contains an invalid sound-pack name."; return false; }
        std::ofstream f(tmp,std::ios::trunc); if(!f){ error=L"Configuration could not be saved."; return false; }
        f<<"{\n  \"version\": 1,\n  \"packName\": \""<<Esc(packName)<<"\",\n  \"masterVolume\": "<<masterVolume<<",\n  \"enabled\": "<<(enabled?"true":"false")<<",\n  \"randomization\": "<<(randomization?"true":"false")<<",\n  \"assignments\": {\n";
        bool first=true;
        for(const auto& [k,a]:assignments){ if(a.press.empty()) continue; if(!first) f<<",\n"; first=false; f<<"    \""<<KeyName(k)<<"\": \""<<Esc(a.press.front())<<"\""; }
        f<<"\n  }\n}\n"; f.close();
        if(!MoveFileExW(tmp.c_str(),p.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){
            DeleteFileW(tmp.c_str());
            error=L"Configuration could not be committed safely."; return false;
        }
        return true;
    } catch(...) { error=L"Configuration could not be saved."; return false; }
}

bool LoadSoundPack(const std::filesystem::path& root, SoundPack& out, std::wstring& error){if(!std::filesystem::exists(root)){error=L"Sound pack does not exist.";return false;}out.root=root;out.name=root.filename().wstring();out.assignments.clear();KeyAssignment a;a.press={L"mechanical_default.wav"};for(int k:SupportedKeys())out.assignments[k]=a;return true;}
