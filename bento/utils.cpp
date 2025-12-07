#include "utils.h"

std::string getExecutablePath(){
    #ifdef WINDOWS
        char pth[MAX_PATH];
        GetModuleFileNameA(NULL,pth,MAX_PATH);
        std::string fullPath(pth);
        size_t lSlash = fullPath.find_last_of("\\/");
        return fullPath.substr(0,lSlash);
    #elif LINUX
        char pth[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe",pth,PATH_MAX);
        if (count != -1) {
            std::string fullPath(pth,count);
            size_t lSlash = fullPath.find_last_of('/');
            return fullPath.substr(0,lSlash+1);
        }
    #elif MACOS
        char pth[PATH_MAX];
        uint32_t size = sizeof(pth);
        if(_NSGetExecutablePath(pth,&size)==0){
            std::string fullPath(pth);
            #ifdef FORAPP
            fullPath = fullPath.substr(0,fullPath.rfind("MacOS/"));
            #endif
            size_t lSlash = fullPath.find_last_of('/');
            return fullPath.substr(0,lSlash+1);
        }
    #endif
    return "";
}

std::vector<uint8_t> loadFile(std::string path){
    if(path[0]=='.')path=getExecutablePath()+path.substr(1);
    std::ifstream fileStream(path,std::ios::binary|std::ios::ate);
    if(!fileStream.is_open())std::cerr<<"couldn't open "<<path<<"!!!!!!"<<std::endl;
    std::vector<uint8_t> data(fileStream.tellg());
    fileStream.seekg(0);
    fileStream.read((char*)data.data(),data.size());
    return data;
}
std::string loadFileString(std::string path){
    if(path[0]=='.')path=getExecutablePath()+path.substr(1);
    std::ifstream fileStream(path);
    if(!fileStream.is_open())std::cerr<<"couldn't open "<<path<<"!!!!!!"<<std::endl;
    std::stringstream data;
    data << fileStream.rdbuf();
    return data.str();
}
void* loadFileVoid(std::string path){
    if(path[0]=='.')path=getExecutablePath()+path.substr(1);
    std::ifstream fileStream(path,std::ios::binary|std::ios::ate);
    if(!fileStream.is_open())std::cerr<<"couldn't open "<<path<<"!!!!!!"<<std::endl;
    size_t size = fileStream.tellg();
    fileStream.seekg(0);
    void* data = malloc(size);
    fileStream.read((char*)data,size);
    return data;
}

__attribute__((weak))
void loop(){}
__attribute__((weak))
void exit(){}