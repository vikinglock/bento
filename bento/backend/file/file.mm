#include "file.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#import <Metal/Metal.h>

#ifdef IOS


void File::loadFrozenFilesystem(std::string path) {
    @autoreleasepool{
        size_t dot = path.find_last_of('.');
        NSString *npath = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:path.substr(0,dot).c_str()]
                                                         ofType:[NSString stringWithUTF8String:path.substr(dot+1).c_str()]];
        std::ifstream file([npath UTF8String], std::ios::binary);
        if(!file.is_open())return;

        files.clear();

        //uint64_t fileAmount;
        //file.read(reinterpret_cast<char*>(&fileAmount), sizeof(uint32_t));
        //later


        while(!file.eof()){
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
}
#endif
