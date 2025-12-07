#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>


class File {
private:
    static std::unordered_map<std::string, std::vector<uint8_t> > files;
    inline static bool loaded = false;

    std::vector<uint8_t>* data;
    
    void load(std::string name){
        #ifdef FREEZE_FILES
        if(File::loaded){
            auto it=files.find(name);
            if(it==files.end()){
                std::cerr<<"couldn't find \""<<name<<"\" in the loaded frozen filesystem";
                return;
            }
            data=&it->second;
            //std::cout<<"loaded "<<name<<"!"<<std::endl;
        }
        #else
        std::ifstream file(name, std::ios::binary);
        if(!file.is_open())return;

        file.seekg(0,std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0,std::ios::beg);

        data = new std::vector<uint8_t>(fileSize);
        file.read((char*)data->data(), fileSize);        
        #endif
    }
public:
    std::string name;
    File(std::vector<uint8_t>* d){
        data = d;
    }
    File(std::string n){
        name = n;
        load(name);
    }
    std::vector<uint8_t>* getData(){
        if(!data)load(name);
        return data;
    }
    #ifdef IOS
    static void loadFrozenFilesystem(std::string path);
    #else
    static void loadFrozenFilesystem(std::string path) {
        std::ifstream file(path, std::ios::binary);
        if(!file.is_open())return;

        files.clear();

        while (!file.eof()) {
            uint32_t nameLength;
            uint64_t dataLength;
            file.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
            file.read(reinterpret_cast<char*>(&dataLength), sizeof(uint64_t));
            if(!file)break;

            std::string name(nameLength, '\0');
            file.read(&name[0], nameLength);

            std::vector<uint8_t> data(dataLength);
            file.read((char*)data.data(), dataLength);

            files[name] = std::move(data);
        }
        File::loaded = true;
    }
    #endif
};