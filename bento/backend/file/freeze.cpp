#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

/*
simple freeze script

i don't actually know if 'freezing' is the right term to use here
basically just puts the folders (passed in arguments) into a file
format:
[uint32] [uint64] [name] [data]
*/
int main(int argc, const char *argv[]) {
    if(argc > 2){
        std::ofstream out(argv[1],std::ios::binary);
        for(int i = 2; i < argc; i++){
            std::filesystem::path folder = argv[i];
            for(auto& ifile : std::filesystem::recursive_directory_iterator(folder)){
                if(!std::filesystem::is_regular_file(ifile))continue;
                std::string path = std::string(argv[i])+"/"+std::filesystem::relative(ifile.path(),folder).string();
                std::ifstream file(ifile.path(), std::ios::binary);
                file.seekg(0,std::ios::end);
                size_t fileSize = file.tellg();
                file.seekg(0,std::ios::beg);
                uint32_t nameLength = path.size();
                uint64_t dataLength = fileSize;
                out.write(reinterpret_cast<char*>(&nameLength),sizeof(nameLength));
                out.write(reinterpret_cast<char*>(&dataLength),sizeof(dataLength));
                out.write(path.data(),path.size());

                std::vector<char> buf(fileSize);
                file.read(buf.data(),fileSize);
                out.write(buf.data(),fileSize);
            }
        }
    }
}