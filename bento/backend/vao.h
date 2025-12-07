#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include <regex>
#include <vector>

//i am now become c++, c++ of c++

enum class AttribFormat{Float,Int,UInt,Short};

struct Attribute{
    int size;
    AttribFormat format;
    int offset;
};

class VAO{
    public:
    VAO(std::vector<std::pair<int,AttribFormat>> in){attributes.resize(in.size());for(int i=0;i<in.size();i++){attributes[i].size=in[i].first;attributes[i].format=in[i].second;attributes[i].offset=0;}}
    VAO(){}
    void setAttrib(int index,int size,AttribFormat attribFormat,int offset = 0){
        if(attributes.size()<=index)
            attributes.resize(index+1);
        attributes[index].size = size;
        attributes[index].format = attribFormat;
        attributes[index].offset = offset;
    }
    std::vector<Attribute> attributes;
};