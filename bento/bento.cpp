#include <iostream>
#include <stdio.h>
#include <string.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>

enum class Backend{
    Metal,OpenGL,Vulkan,WebGL,OpenGL3,DirectX11,DirectX12,OpenGLES,
};
enum class OS{
    MacOS,Windows,Linux,Android,IOS,Browser,
    //consoles coming at some point between now and never
};

#define DI std::filesystem::directory_iterator
#define DE std::filesystem::directory_entry

void cacheFolder(std::string path,std::string flags="",std::string inc="bento"){
    std::string folderName = path.substr(path.find_last_of("/\\")+1);
    #ifdef _WIN32
    std::string name = "bento/cache/"+inc+folderName+".lib";
    if(!std::filesystem::exists(name)){
    #else
    std::string name = "bento/cache/lib"+inc+folderName+".a";
    if(!std::filesystem::exists(name)){
    #endif
        #ifdef _WIN32
        std::string complib = "C:/msys64/mingw64/bin/ar.exe rcs "+name;
        #else
        std::string complib = "ar rcs "+name;
        #endif

        if(!std::filesystem::is_directory("bento/cache"))std::filesystem::create_directory("bento/cache");

        for(DE dir:std::filesystem::recursive_directory_iterator(path)){
            if(dir.is_regular_file()){
                std::string defines = "-DIMGUI";
                #ifdef _WIN32
                defines += " -DWINDOWS";
                #else
                defines += " -DMACOS";
                #endif
                if(dir.path().extension() == ".cpp")system(("clang++ -c "+dir.path().string()+" -o bento/cache/"+dir.path().stem().string()+".o -std=c++23 "+defines+flags).c_str());
                else if(dir.path().extension() == ".mm")system(("clang++ -x objective-c++ -c "+dir.path().string()+" -o bento/cache/"+dir.path().stem().string()+".o -std=c++23 "+defines+flags).c_str());
                if(dir.path().extension() == ".c")system(("clang -c "+dir.path().string()+" -o bento/cache/"+dir.path().stem().string()+".o -std=c23 "+defines+flags).c_str());
                else if(dir.path().extension() == ".m")system(("clang -c "+dir.path().string()+" -o bento/cache/"+dir.path().stem().string()+".o -ObjC "+defines+flags).c_str());
            }
        }
        
        for(DE dir:DI("bento/cache"))
            if(dir.path().extension() == ".o")
                complib += " "+dir.path().string();
        
        system(complib.c_str());

        for(DE dir:DI("bento/cache"))
            if(dir.path().extension() == ".o")
                std::filesystem::remove(dir);
        
        //system("string -x bento/lib/shaders/libshaderconvert.a");*/
        std::cout<<"cached "<<folderName<<" in "<<name<<std::endl;
    }
}

int main(int argc,char** argv){
    if(argc >= 2){
        int shift = 0;
        if(shift+1<argc&&strcmp(argv[shift+1],"help")==0){
            std::cout<<"usage: bentoc [options...] <output> [inputs...]\n\n\
(-windows|-macos|-linux|-ios|-browser) (required) select operating system\n\
(-opengl|-metal|-vulkan|-webgl)        (required) select backend\n\
-convert     convert shaders\n\
-freeze      freeze resource folder\n\
-debug       enable debug symbols\n\
-imgui       enable dearImgui\n\
-bullet      enable bullet\n\
-sound       enable miniaudio\n\
-nocompile   skip compilation\n\
-cached      use cached libraries (requires 'bentoc cache')\n\
-timed       output compile time\n"<<std::endl;
        }else if(shift+1<argc&&strcmp(argv[shift+1],"ui")==0){
            std::cout<<""<<std::endl;
        }else if(shift+1<argc&&strcmp(argv[shift+1],"config")==0){
            std::cout<<""<<std::endl;
        }else if(shift+1<argc&&strcmp(argv[shift+1],"run")==0){
            std::cout<<""<<std::endl;
        }else if(shift+1<argc&&strcmp(argv[shift+1],"cache")==0){
            for(DE dir:std::filesystem::directory_iterator("bento/backend"))
                if(std::filesystem::is_directory(dir))cacheFolder(dir.path().string());
                
            #ifdef _WIN32
            cacheFolder("bento/lib/GLFW/glfw3"," -D_GLFW_WIN32","");
            #elif _APPLE_ 
            cacheFolder("bento/lib/GLFW/glfw3"," -D_GLFW_COCOA","");
            #endif
        }else{
            std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

            std::string flags = "-std=c++23";//"-Wall -Wextra -Wno-unused-parameter";
            std::string sources = "bento/utils.cpp";
            std::string libs = "";
            std::string includes = "-I./bento/lib -L./bento/cache";

            Backend backend;
            OS os;

            bool cached = false;
            bool timed = false;

            for(int i = 0; i < argc; i++)
                if(strcmp(argv[i],"-cached") == 0)
                    cached = true;

            if(shift+1<argc&&strcmp(argv[shift+1],"-macos")==0){
                os = OS::MacOS;
                shift++;
                if(shift+1<argc&&strcmp(argv[shift+1],"-metal")==0){
                    flags += " -DUSE_METAL -DMACOS";
                    libs += " -framework Metal -framework QuartzCore -framework Cocoa -framework IOKit -framework MetalKit -framework CoreVideo -framework CoreFoundation -framework Carbon";
                    if(!cached)sources += " bento/backend/metal/*.mm";
                    else libs += " -lbentometal";
                    shift++;
                    backend = Backend::Metal;
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-opengl")==0){
                    flags += " -DUSE_OPENGL -DMACOS";
                    libs += " -lglfw3 -framework OpenGL -framework QuartzCore -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreFoundation -framework Carbon";
                    if(!cached)sources += " bento/backend/opengl/*.cpp bento/lib/glad/glad3.cpp";
                    else libs += " -lbentoopengl bento/lib/glad/glad3.cpp";
                    shift++;
                    backend = Backend::OpenGL3;
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-vulkan")==0){
                    std::cerr << "vulkan not supported on macos" << std::endl;
                    flags += " -DUSE_VULKAN -DMACOS -static -lglfw3 -framework Cocoa -framework IOKit";
                    if(!cached)sources += " bento/backend/vulkan/*.cpp";
                    else libs += " -lbentovulkan";
                    shift++;
                    backend = Backend::Vulkan;
                    std::exit(1);
                }else{
                    if(argc >= 3){
                        std::cerr << "bentoc: unknown backend \""<< argv[shift+1] <<"\""<< std::endl;
                    }else{
                        std::cerr << "bentoc: no backend selected" << std::endl;
                    }
                    std::exit(1);
                }
            }else if(shift+1<argc&&strcmp(argv[shift+1],"-windows")==0){
                os= OS::Windows;
                shift++;
                if(shift+1<argc&&strcmp(argv[shift+1],"-opengl")==0){
                    flags += " -DUSE_OPENGL -DWINDOWS -static";
                    libs += " -lws2_32 -lglfw3 -lgdi32 -lopengl32 -luser32 -lole32 -loleaut32 -luuid -lshell32 -lkernel32 -lmsvcrt";
                    if(!cached)sources += " bento/backend/opengl/*.cpp bento/lib/glad/glad4.cpp";
                    else libs += " -lbentoopengl bento/lib/glad/glad4.cpp";
                    shift++;
                    backend = Backend::OpenGL;
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-vulkan")==0){
                    flags += " -DUSE_VULKAN -DWINDOWS -static -fuse-ld=lld";
                    includes += " -IC:/VulkanSDK/1.4.328.1/Include -LC:/VulkanSDK/1.4.328.1/Lib";
                    libs += " -lvulkan-1 -lws2_32 -lglfw3 -lgdi32 -lopengl32 -luser32 -lole32 -loleaut32 -luuid -lshell32 -lkernel32 -lmsvcrt";
                    if(!cached)sources += " bento/backend/vulkan/*.cpp";
                    else libs += " -lbentovulkan";
                    shift++;
                    backend = Backend::Vulkan;
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-metal")==0){
                    std::cerr << "metal not supported on windows" << std::endl;
                    std::exit(1);
                }else{
                    if(argc >= 3){
                        std::cerr << "bentoc: unknown backend \""<< argv[shift+1] <<"\""<< std::endl;
                    }else{
                        std::cerr << "bentoc: no backend selected" << std::endl;
                    }
                    std::exit(1);
                }
            }else if(shift+1<argc&&strcmp(argv[shift+1],"-ios")==0){
                os = OS::IOS;
                shift++;
                if(shift+1<argc&&strcmp(argv[shift+1],"-opengl")==0){
                    std::cerr << "opengl not supported on ios" << std::endl;
                    std::exit(1);
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-vulkan")==0){
                    std::cerr << "vulkan not supported on ios" << std::endl;
                    std::exit(1);
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-metal")==0){
                    flags += " -DUSE_METAL -DIOS";
                    libs += " -framework UIKit -framework Metal -framework MetalKit -framework Foundation -framework AudioToolbox -framework CoreGraphics -framework AVFoundation -framework ImageIO -framework QuartzCore";
                    
                    //todo: change
                    sources += " bento/backend/file/file.mm ";
                    for(DE dir : std::filesystem::recursive_directory_iterator("bento/backend/metal")){
                        if(dir.is_regular_file() && dir.path().extension() == ".mm")sources += dir.path().string()+" ";
                    }

                    shift++;
                    backend = Backend::Metal;
                }else{
                    if(argc >= 3){
                        std::cerr << "bentoc: unknown backend \""<< argv[shift+1] <<"\""<< std::endl;
                    }else{
                        std::cerr << "bentoc: no backend selected" << std::endl;
                    }
                    std::exit(1);
                }
            }else{
                std::cerr << "bentoc: unknown argument \""<< argv[shift+1] <<"\""<< std::endl;
                std::exit(1);
            }
            bool compile = true;

            for(int i = 0; i < 9; i++){
                if(shift+1<argc&&strcmp(argv[shift+1],"-convert") == 0){
                    flags += " -DCONVERT";
                    includes += " -L./bento/lib/shaders";
                    libs += " -lshaderconvert";
                    if     (backend==Backend::Metal)sources += " bento/backend/metal/convert.cc";
                    else if(backend==Backend::OpenGL||
                            backend==Backend::OpenGL3)sources += " bento/backend/opengl/convert.cc";
                    else if(backend==Backend::Vulkan)sources += " bento/backend/vulkan/convert.cc";
                    #ifdef _WIN32
                    if(!std::filesystem::exists("bento/lib/shaders/shaderconvert.lib")){
                    #else
                    if(!std::filesystem::exists("bento/lib/shaders/libshaderconvert.a")){
                    #endif
                        for(DE dir:DI("bento/lib/shaders/glslang/glslang/GenericCodeGen"))if(dir.is_regular_file() && dir.path().extension() == ".cpp")system(("clang++ -c "+dir.path().string()+" -o bento/lib/shaders/"+dir.path().stem().string()+".o -std=c++23").c_str());
                        for(DE dir:DI("bento/lib/shaders/glslang/glslang/MachineIndependent"))if(dir.is_regular_file() && dir.path().extension() == ".cpp")system(("clang++ -c "+dir.path().string()+" -o bento/lib/shaders/"+dir.path().stem().string()+".o -std=c++23").c_str());
                        for(DE dir:DI("bento/lib/shaders/glslang/glslang/MachineIndependent/preprocessor"))if(dir.is_regular_file() && dir.path().extension() == ".cpp")system(("clang++ -c "+dir.path().string()+" -o bento/lib/shaders/"+dir.path().stem().string()+".o -std=c++23").c_str());
                        for(DE dir:DI("bento/lib/shaders/glslang/SPIRV"))if(dir.is_regular_file() && dir.path().extension() == ".cpp")system(("clang++ -c "+dir.path().string()+" -o bento/lib/shaders/"+dir.path().stem().string()+".o -std=c++23").c_str());
                        for(DE dir:DI("bento/lib/shaders/SPIRV-Cross"))if(dir.is_regular_file() && dir.path().extension() == ".cpp")system(("clang++ -c "+dir.path().string()+" -o bento/lib/shaders/"+dir.path().stem().string()+".o -std=c++23").c_str());
                        system("clang++ -c bento/lib/shaders/glslang/glslang/ResourceLimits/ResourceLimits.cpp -o bento/lib/shaders/ResourceLimits.o -std=c++23");
                        
                        #ifdef _WIN32
                        system("clang++ -c bento/lib/shaders/glslang/glslang/OSDependent/Windows/ossource.cpp -o bento/lib/shaders/ossource.o -std=c++23");
                        #else
                        // system("clang++ -c bento/lib/shaders/glslang/glslang/OSDependent/Unix/ossource.cpp -o bento/lib/shaders/ossource.o -std=c++23");
                        #endif

                        #ifdef _WIN32
                        std::string complib = "C:/msys64/mingw64/bin/ar.exe rcs bento/lib/shaders/shaderconvert.lib";
                        for(DE dir:std::filesystem::directory_iterator("bento/lib/shaders"))
                            if(dir.path().extension() == ".o")
                                complib += " "+dir.path().string();//i hate windows
                        system(complib.c_str());
                        for(DE dir:std::filesystem::directory_iterator("bento/lib/shaders"))
                            if(dir.path().extension() == ".o")
                                std::filesystem::remove(dir);
                        #else
                        system("ar rcs bento/lib/shaders/libshaderconvert.a bento/lib/shaders/*.o");
                        system("string -x bento/lib/shaders/libshaderconvert.a");
                        system("rm bento/lib/shaders/*.o");
                        #endif
                    }
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-debug")     == 0)flags += " -g -O0 -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE";
                else if(shift+1<argc&&strcmp(argv[shift+1],"-imgui")     == 0){
                    flags += " -DIMGUI";
                    includes += " -L./bento/lib/imgui";
                    if(backend==Backend::Metal){
                        sources += " bento/backend/metal/imgui.cc";
                        libs += " -limguimetal";
                        if(!std::filesystem::exists("bento/lib/imgui/libimguimetal.a")){
                            system("clang++ -c bento/lib/imgui/imgui.cpp -o bento/lib/imgui/imgui.o -std=c++23 -fobjc-arc");
                            system("clang++ -c bento/lib/imgui/imgui_demo.cpp -o bento/lib/imgui/imgui_demo.o -std=c++23 -fobjc-arc");
                            system("clang++ -c bento/lib/imgui/imgui_draw.cpp -o bento/lib/imgui/imgui_draw.o -std=c++23 -fobjc-arc");
                            system("clang++ -c bento/lib/imgui/imgui_tables.cpp -o bento/lib/imgui/imgui_tables.o -std=c++23 -fobjc-arc");
                            system("clang++ -c bento/lib/imgui/imgui_widgets.cpp -o bento/lib/imgui/imgui_widgets.o -std=c++23 -fobjc-arc");
                            system("clang++ -c bento/lib/imgui/backends/imgui_impl_metal.mm -o bento/lib/imgui/backends/imgui_impl_metal.o -std=c++23 -fobjc-arc");

                            system("ar rcs bento/lib/imgui/libimguimetal.a bento/lib/imgui/*.o bento/lib/imgui/backends/imgui_impl_metal.o");
                            system("string -x bento/lib/imgui/libimguimetal.a");

                            system("rm bento/lib/imgui/backends/*.o");
                            system("rm bento/lib/imgui/*.o");
                        }
                    }else if(backend==Backend::OpenGL||backend==Backend::OpenGL3){
                        libs += " -limguiglfw";
                        sources += " bento/backend/opengl/imgui.cc";
                        #ifdef _WIN32
                        if(!std::filesystem::exists("bento/lib/imgui/imguiglfw.lib")){
                        #else
                        if(!std::filesystem::exists("bento/lib/imgui/libimguiglfw.a")){
                        #endif
                            system("clang++ -c bento/lib/imgui/imgui.cpp -o bento/lib/imgui/imgui.o -std=c++23");
                            system("clang++ -c bento/lib/imgui/imgui_demo.cpp -o bento/lib/imgui/imgui_demo.o -std=c++23");
                            system("clang++ -c bento/lib/imgui/imgui_draw.cpp -o bento/lib/imgui/imgui_draw.o -std=c++23");
                            system("clang++ -c bento/lib/imgui/imgui_tables.cpp -o bento/lib/imgui/imgui_tables.o -std=c++23");
                            system("clang++ -c bento/lib/imgui/imgui_widgets.cpp -o bento/lib/imgui/imgui_widgets.o -std=c++23");
                            system("clang++ -c bento/lib/imgui/backends/imgui_impl_opengl3.cpp -o bento/lib/imgui/backends/imgui_impl_opengl3.o -std=c++23");
                            system("clang++ -c -I./bento/lib bento/lib/imgui/backends/imgui_impl_glfw.cpp -o bento/lib/imgui/backends/imgui_impl_glfw.o -std=c++23");

                            #ifdef _WIN32
                            std::string complib = "C:/msys64/mingw64/bin/ar.exe rcs bento/lib/imgui/imguiglfw.lib bento/lib/imgui/backends/imgui_impl_opengl3.o bento/lib/imgui/backends/imgui_impl_glfw.o";
                            for(DE dir:std::filesystem::directory_iterator("bento/lib/imgui"))
                                if(dir.path().extension() == ".o")
                                    complib += " "+dir.path().string();//i hate windows
                            system(complib.c_str());
                            for(DE dir:std::filesystem::directory_iterator("bento/lib/imgui"))
                                if(dir.path().extension() == ".o")
                                    std::filesystem::remove(dir);
                            for(DE dir:std::filesystem::directory_iterator("bento/lib/imgui/backends"))
                                if(dir.path().extension() == ".o")
                                    std::filesystem::remove(dir);
                            #else
                            system("ar rcs bento/lib/imgui/libimguiglfw.a bento/lib/imgui/*.o bento/lib/imgui/backends/imgui_impl_opengl3.o bento/lib/imgui/backends/imgui_impl_glfw.o");
                            system("string -x bento/lib/imgui/libimguiglfw.a");
                            system("rm bento/lib/imgui/backends/*.o");
                            system("rm bento/lib/imgui/*.o");
                            #endif

                        }
                    }else if(backend==Backend::Vulkan){
                        sources += " bento/backend/vulkan/imgui.cc";
                    }
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-freeze")   == 0){
                    flags += " -DFREEZE_FILES";
                    system("clang++ bento/backend/file/freeze.cpp -o bento/backend/file/freeze -std=c++23");
                    system("./bento/backend/file/freeze resources.fz resources");
                    system("cp resources.fz bento/ios/Resources/resources.fz");
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-bullet")   == 0){
                    includes += " -I./bento/lib/bullet/bullet-install/include -L./bento/lib/buller/bullet-install/lib";
                    libs += " -lBulletDynamics -lBulletCollision";
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-sound")    == 0)flags += " -DSOUND";
                else if(shift+1<argc&&strcmp(argv[shift+1],"-nocompile") == 0)compile = false;
                else if(shift+1<argc&&strcmp(argv[shift+1],"-cached") == 0){
                }else if(shift+1<argc&&strcmp(argv[shift+1],"-timed") == 0)timed = true;
                else{
                    break;
                }
                shift++;
            }
            std::string output;
            if(shift+2 <= argc){
                output = argv[shift+1];
                shift++;
            }else{
                std::cerr << "bentoc: no output"<< std::endl;
            }

            if(shift+2 <= argc){
                std::string files;
                for(int i = shift+1; i < argc; i++){
                    if(os==OS::IOS)files += "\n        - ../../";
                    files += argv[i];
                    if(os!=OS::IOS && i < argc-1)files += " ";
                }

                if(compile){
                    if(os == OS::IOS){
                        std::filesystem::remove_all("bento/ios/"+output+".xcodeproj");

                                    
                        std::istringstream stream(sources);
                        std::string source;
                        std::string isources;
                        while(stream >> source)isources += "\n        - ../../"+source;

                        std::ofstream file("bento/ios/project.yml");
                                    
                        file << "name: "+output+"\n\
options:\n\
    bundleIdPrefix: com.vikinglock\n\
targets:\n\
    "+output+":\n\
        type: application\n\
        platform: iOS\n\
        deploymentTarget: \"12.0\"\n\
        sources:\n\
        - src"+isources+files+"\n\
        - Resources\n\
        resources:\n\
        - Resources\n\
        settings:\n\
            OTHER_CPLUSPLUSFLAGS: \""+flags+" -Wno-c++17-extensions\"\n\
            OTHER_LDFLAGS: \""+libs+"\"\n\
            PRODUCT_BUNDLE_IDENTIFIER: com.vikinglock."+output+"\n\
            CODE_SIGN_STYLE: Automatic\n\
            DEVELOPMENT_TEAM: \"T6JW8VHUK2\"\n\
            CODE_SIGN_IDENTITY: \"Apple Development\"\n\
            GENERATE_INFOPLIST_FILE: YES\n\
            frameworks:\n\
            - UIKit\n\
            - Metal\n\
            - MetalKit\n\
            - Foundation\n\
            - AudioToolbox\n\
            - CoreGraphics\n\
            - AVFoundation\n\
            - ImageIO\n\
            - QuartzCore\n\
        base:\n\
            ASSETCATALOG_COMPILER_APPICON_NAME: AppIcon\n\
        info:\n\
            path: Info.plist\n\
            properties:\n\
                CFBundleIdentifier: com.vikinglock."+output+"\n\
                CFBundleShortVersionString: \"1.0\"\n\
                CFBundleVersion: \"1\"\n\
                UILaunchStoryboardName: LaunchScreen\n\
                UISupportedInterfaceOrientations:\n\
                - UIInterfaceOrientationPortrait\n\
                - UIInterfaceOrientationLandscapeLeft  \n\
                - UIInterfaceOrientationLandscapeRight\n\
            resources:\n\
            - LaunchScreen.storyboard";

                        file.close();

                        system("cd bento/ios && xcodegen generate");

                        if(compile){
                            std::string command = "xcodebuild -project bento/ios/"+output+".xcodeproj -scheme "+output+" -sdk iphoneos -configuration Debug -allowProvisioningUpdates \
                                OTHER_CPLUSPLUSFLAGS=\""+flags+" -Wno-c++17-extensions\"\
                                OTHER_LDFLAGS=\""+libs+"\"";
                            system(command.c_str());
                        }
                        system(("ideviceinstaller -i \"$(xcodebuild -project bento/ios/"+output+".xcodeproj -scheme "+output+" -configuration Debug -sdk iphoneos -showBuildSettings | grep \"BUILT_PRODUCTS_DIR\" | head -1 | cut -d= -f2 | xargs)/"+output+".app").c_str());

                    }else{
                        
                        std::ifstream fileStream("bento/flags");
                        if(!fileStream.is_open())std::cerr<<"bentoc: couldn't flags!!!!!!"<<std::endl;
                        std::stringstream data;
                        data << fileStream.rdbuf();

                        #ifdef _WIN32
                        
                        /*system("clang-cl /MD !FILES!\
                            /Ibento/lib \
                            /Ibento/lib/bullet-install/include \
                            /link /LIBPATH:\"bento/lib/winbins\" /LIBPATH:\"bento/lib/bullet-install/lib\" glfw3.lib gdi32.lib opengl32.lib BulletDynamics.lib BulletCollision.lib LinearMath.lib");
                        */
                        std::string command = "clang++ -MD -mwindows -o "+output+" "+sources+" "+flags+" "+includes+" "+libs+" "+data.str()+" "+files;
                        

                        if(system(command.c_str())!=0){std::exit(1);}
                        
                        if(timed)std::cout<<"compilation took "<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-begin).count()<<"ms."<<std::endl;
                        return system(output.c_str());
                        
                        #elif __APPLE__
                        
                        std::string command = "clang++ "+(backend==Backend::Metal?std::string(" -x objective-c++ "):std::string(""))+" -o "+output+" "+sources+" "+flags+" "+includes+" "+libs+" "+data.str()+" "+files;
                        
                        
                        if(system(command.c_str())!=0){std::exit(1);}
                        system(("codesign -s - -v -f --entitlements bento/debug.plist "+output+" 2> /dev/null").c_str());

                        if(timed)std::cout<<"compilation took "<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-begin).count()<<"ms."<<std::endl;

                        return system(("./"+output).c_str());
                        
                        
                        #endif
                    }
                }
            }else{
                std::cerr << "bentoc: no input(s)"<< std::endl;
            }
        }
    }else{
        std::cerr<<"bentoc: run 'bentoc help'"<<std::endl;
    }
    return 0;
}