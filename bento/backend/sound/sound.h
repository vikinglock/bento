#pragma once
#include "soundcommon.h"

#include <iostream>
#include <cstring>

#include "../../lib/miniaudio/miniaudio.h"
#include "../../lib/glm/glm.hpp"

#include "../../utils.h"
#include "../file/file.h"

//#define MINIAUDIO_IMPLEMENTATION

#include <limits.h>
#include <string>

#ifdef MACOS
#include <unistd.h>
#include <mach-o/dyld.h>
#elif WINDOWS
#include <windows.h>
#endif

class Sound {
public:
    Sound(const char* path);
    Sound(File file);

    ~Sound();

    void setGain(float gain);
    void setPitch(float pitch);
    void setLoop(bool loops);
    void setPosition(glm::vec3 position);
    
    void setNextGain(float gain);
    void setNextPitch(float pitch);
    void setNextLoop(bool loops);
    void setNextPosition(glm::vec3 position);


    void setIndGain(float gain, int index);
    void setIndPitch(float pitch, int index);
    void setIndLoop(bool loops, int index);
    void setIndPosition(glm::vec3 position,int index);

    void play(int frame = 0);

    ma_format format;
    ma_uint64 frameCount;
    ma_uint32 channels;
    ma_uint32 sampleRate;
    float* pSampleData;
    std::vector<ma_sound*> sounds;
    std::vector<ma_audio_buffer*> buffers;


    float nextGain;
    float nextPitch;
    bool nextLoops;
    glm::vec3 nextPosition;
};
