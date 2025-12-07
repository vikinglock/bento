#include "sound.h"

Sound::Sound(const char* path){
    std::string dir = getExecutablePath();

    ma_decoder decoder;
    ma_result result = ma_decoder_init_file((dir+"/"+std::string(path)).c_str(),NULL,&decoder);
    if(result != MA_SUCCESS)std::cerr << "couldn't load " << (dir+"/"+std::string(path)) << "!!!!!" << std::endl;

    format = decoder.outputFormat;
    channels = decoder.outputChannels;
    sampleRate = decoder.outputSampleRate;
    
    result = ma_decoder_get_length_in_pcm_frames(&decoder,&frameCount);
    if(result != MA_SUCCESS){
        std::cerr << "couldn't get " << path << "'s frame count" << std::endl;
        ma_decoder_uninit(&decoder);
    }
    pSampleData = new float[frameCount * channels];
    ma_uint64 framesRead;
    result = ma_decoder_read_pcm_frames(&decoder,pSampleData,frameCount,&framesRead);
    if(result != MA_SUCCESS){
        std::cerr << "couldn't get " << path << "'s pcm frames" << std::endl;
        delete[] pSampleData;
        ma_decoder_uninit(&decoder);
    }

    //static_cast<float>(framesRead) / sampleRate  is time in seconds apparantly
    
    ma_decoder_uninit(&decoder);


    nextGain = 1.0;
    nextPitch = 1.0;
    nextLoops = false;
    nextPosition = glm::vec3(0,0,0);
}

Sound::Sound(File file){
    ma_decoder decoder;
    ma_result result = ma_decoder_init_memory(file.getData()->data(),file.getData()->size(),NULL,&decoder);
    if(result != MA_SUCCESS)std::cerr << "couldn't load " << file.name << "!!!!!"<<std::endl;

    format = decoder.outputFormat;
    channels = decoder.outputChannels;
    sampleRate = decoder.outputSampleRate;
    
    result = ma_decoder_get_length_in_pcm_frames(&decoder,&frameCount);
    if(result != MA_SUCCESS){
        std::cerr << "couldn't get " << file.name << "'s frame count" << std::endl;
        ma_decoder_uninit(&decoder);
    }
    pSampleData = new float[frameCount * channels];
    ma_uint64 framesRead;
    result = ma_decoder_read_pcm_frames(&decoder,pSampleData,frameCount,&framesRead);
    if(result != MA_SUCCESS){
        std::cerr << "couldn't get " << file.name << "'s pcm frames" << std::endl;
        delete[] pSampleData;
        ma_decoder_uninit(&decoder);
    }
    ma_decoder_uninit(&decoder);

    nextGain = 1.0;
    nextPitch = 1.0;
    nextLoops = false;
    nextPosition = glm::vec3(0,0,0);
}

Sound::~Sound(){
    for(ma_sound* sound : sounds){
        ma_sound_uninit(sound);
        delete sound;
    }
    sounds.clear();
    for(ma_audio_buffer* buffer : buffers){
        ma_audio_buffer_uninit(buffer);
        delete buffer;
    }
    buffers.clear();
    delete[] pSampleData;
}

void Sound::setGain(float gain){
    for(ma_sound* sound : sounds)
    ma_sound_set_volume(sound,gain);
    nextGain = gain;
}

void Sound::setPitch(float pitch){
    for(ma_sound* sound : sounds)
    ma_sound_set_pitch(sound,pitch);
    nextPitch = pitch;
}

void Sound::setLoop(bool loops){
    for(ma_sound* sound : sounds)
    ma_sound_set_looping(sound,loops ? MA_TRUE : MA_FALSE);
    nextLoops = loops;
}

void Sound::setPosition(glm::vec3 position){
    for(ma_sound* sound : sounds)
    ma_sound_set_position(sound,position.x,position.y,position.z);
    nextPosition = position;
}

void Sound::setNextGain(float gain){nextGain = gain;}
void Sound::setNextPitch(float pitch){nextPitch = pitch;}
void Sound::setNextLoop(bool loops){nextLoops = loops;}
void Sound::setNextPosition(glm::vec3 position){nextPosition = position;}


void Sound::setIndGain(float gain,int index){ma_sound_set_volume(sounds[index],gain);}
void Sound::setIndPitch(float pitch,int index){ma_sound_set_pitch(sounds[index],pitch);}
void Sound::setIndLoop(bool loops,int index){ma_sound_set_looping(sounds[index],loops ? MA_TRUE : MA_FALSE);}
void Sound::setIndPosition(glm::vec3 position,int index){ma_sound_set_position(sounds[index],position.x,position.y,position.z);}

void Sound::play(int frame){
    ma_audio_buffer* buffer = new ma_audio_buffer;

    ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(format,channels,frameCount,pSampleData,NULL);
    ma_audio_buffer_init(&bufferConfig,buffer);

    ma_audio_buffer_seek_to_pcm_frame(buffer,frame);

    ma_sound* sound = new ma_sound;

    ma_sound_init_from_data_source(&engine,buffer,0,NULL,sound);

    ma_sound_set_volume(sound,nextGain);
    ma_sound_set_pitch(sound,nextPitch);
    ma_sound_set_looping(sound,nextLoops ? MA_TRUE : MA_FALSE);
    ma_sound_set_position(sound,nextPosition.x,nextPosition.y,nextPosition.z);

    ma_sound_start(sound);

    sounds.push_back(sound);
    buffers.push_back(buffer);
}