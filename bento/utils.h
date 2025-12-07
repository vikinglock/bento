#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef MACOS
#include <mach-o/dyld.h>
#elif WINDOWS
#include <windows.h>
#endif

std::string getExecutablePath();

std::vector<uint8_t> loadFile(std::string path);
std::string loadFileString(std::string path);
void* loadFileVoid(std::string path);