#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <time.h>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <chrono>
#include <queue>

#include <bento/bento.h>
Bento* bento;

//note this is a test
//do not model your code after mine

std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
float deltaTime = 0.0f, elapsedTime = 0.0f, timeScale = 1.0f;

void genCube(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& uvs, float hx, float hy, float hz,glm::vec3 uv) {glm::vec3 positions[8] = {{-hx, -hy, -hz}, {hx, -hy, -hz}, {hx, hy, -hz}, {-hx, hy, -hz},{-hx, -hy, hz},  {hx, -hy, hz},  {hx, hy, hz},  {-hx, hy, hz}};int faces[6][4] = {{1,0,3,2},{4,5,6,7},{0,4,7,3},{5,1,2,6},{2,3,7,6},{5,4,0,1}};glm::vec3 faceNormals[6] = {{0,0,-1},{0,0,1},{-1,0,0},{1,0,0},{0,1,0},{0,-1,0}};glm::vec2 uvTemplate[4] = {{0,0},{1,0},{1,1},{0,1}};glm::vec2 faceScales[6] = {{2*hx/uv.y, 2*hy/uv.y},{2*hx/uv.y, 2*hy/uv.y},{2*hz/uv.z, 2*hy/uv.y},{2*hz/uv.z, 2*hy/uv.y},{2*hx/uv.y, 2*hz/uv.z},{2*hx/uv.y, 2*hz/uv.z}};for (int i = 0; i < 6; ++i) {int* f = faces[i];glm::vec3 n = faceNormals[i];vertices.push_back(positions[f[0]]);vertices.push_back(positions[f[1]]);vertices.push_back(positions[f[2]]);vertices.push_back(positions[f[2]]);vertices.push_back(positions[f[3]]);vertices.push_back(positions[f[0]]);for (int j = 0; j < 6; ++j) normals.push_back(n);glm::vec2 scaledUVs[4] = {uvTemplate[0] * faceScales[i],uvTemplate[1] * faceScales[i],uvTemplate[2] * faceScales[i],uvTemplate[3] * faceScales[i]};uvs.insert(uvs.end(), {scaledUVs[0], scaledUVs[1], scaledUVs[2], scaledUVs[2], scaledUVs[3], scaledUVs[0]});}}

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec2> uvs;
    
glm::mat4 model(1);

float ticks = 0;

VAO vao({{3,AttribFormat::Float},{3,AttribFormat::Float},{2,AttribFormat::Float},{3,AttribFormat::Float}});
Shader* shd = new Shader("./shaders/shader.vert","./shaders/shader.frag",vao);

Texture* texture = new Texture("./resources/glass.png");
Texture* renderTexture = new Texture(nullptr,glm::ivec2(1000,1000));

void resizeCallback(glm::ivec2 size);

FrameBuffer* framebuffer = new FrameBuffer();

int main(){
    bento = new Bento("vulkan",1000,1000);
    bento->initImgui();
    bento->focus();

    framebuffer->setRenderTargets({renderTexture,Bento::AppTexture},{0,1});

    bento->setResizeCallback(resizeCallback);
    bento->setShader(shd);


    bento->startLoop();
    return 0;
}

void resizeCallback(glm::ivec2 size){framebuffer->resize({size.x,size.y});}

glm::vec2 windowSize;
glm::vec2 windowPos;
glm::vec2 mousePos;
bool locked = false;

glm::vec3 position(0,0,-4);
glm::quat rotation(1,0,0,0);

glm::vec2 lookAngle(0);

void loop(){
    bento->poll();//should only run on the main thread
    
    windowSize = bento->getWindowSize();
    windowPos = bento->getWindowPos();
    mousePos = bento->getMousePosition();

    bento->setClearColor(glm::vec4(0,0,0,1));

    bento->bindFrameBuffer(framebuffer);
    bento->predraw();

    vertices.clear();
    normals.clear();
    uvs.clear();
    genCube(vertices,normals,uvs,sin(ticks*1.124)*0.25+0.5,sin(ticks*1.2562+10.2441)*0.25+0.5,sin(ticks*0.9532+3.24)*0.25+0.5,glm::vec3(1));

    model = glm::rotate(model,4*deltaTime,glm::normalize(glm::vec3(sin(ticks)+0.14,cos(ticks)*0.2+0.4,cos(ticks)*0.567+0.1)));
    ticks += deltaTime*2;
    
    if(bento->getKey(KEY_A))position += 5.0f*deltaTime*glm::vec3(cos(lookAngle.x),0,sin(lookAngle.x));
    if(bento->getKey(KEY_S))position += 5.0f*deltaTime*glm::vec3(sin(lookAngle.x),0,-cos(lookAngle.x));
    if(bento->getKey(KEY_D))position -= 5.0f*deltaTime*glm::vec3(cos(lookAngle.x),0,sin(lookAngle.x));
    if(bento->getKey(KEY_W))position -= 5.0f*deltaTime*glm::vec3(sin(lookAngle.x),0,-cos(lookAngle.x));
    if(bento->getKey(KEY_E))position += glm::vec3(0,5*deltaTime,0);
    if(bento->getKey(KEY_Q))position -= glm::vec3(0,5*deltaTime,0);

    rotation = glm::angleAxis(lookAngle.y,glm::vec3(1,0,0))*glm::angleAxis(lookAngle.x,glm::vec3(0,1,0));
    
    static bool uiHover = false;
    if(!uiHover&&bento->getMouseDown(MOUSE_LEFT)&&!locked){
        locked = true;
        bento->lockMouse(true);
        bento->setMouseCursor(true,0);
    }
    if(bento->getKeyDown(KEY_ESCAPE)&&locked){
        locked = false;
        bento->lockMouse(false);
        bento->setMouseCursor(false,0);
    }
    if(locked){        
        glm::vec2 delta = bento->getMouseDelta();
        delta.y = -delta.y;
        lookAngle += delta*0.01f;
    }

    bento->setUniform("model",model,true);
    bento->setUniform("projection",glm::perspective(glm::radians(110.0f),windowSize.x/windowSize.y,0.1f,100000.0f),true);
    bento->setUniform("view",glm::translate(glm::mat4_cast(rotation),position),true);
    bento->setVertexBuffer(0,vertices);
    bento->setVertexBuffer(1,normals);
    bento->setVertexBuffer(2,uvs);
    bento->setVertexBuffer(3,vertices);

    bento->setUniform("color",glm::vec3(0.5,0.3,0.8));
    bento->bindTexture(texture,0);
    bento->draw(Primitive::Triangles);

    bento->setUniform("model",glm::translate(glm::mat4(1),glm::vec3(0,0,2))*model,true);
    bento->setVertexBuffer(0,vertices);
    bento->setVertexBuffer(1,normals);
    bento->setVertexBuffer(2,uvs);
    bento->setVertexBuffer(3,normals);

    bento->setUniform("color",glm::vec3(0.5,0.3,0.8));
    bento->bindTexture(texture,0);
    bento->draw(Primitive::Triangles);

    bento->drawScreen();
    
    bento->imguiNewFrame();
    ImGui::Begin("balls");
    uiHover = ImGui::IsWindowHovered();
    ImGui::Image((ImTextureID)Bento::AppTexture->getTexture(),ImVec2(500,500),ImVec2(0,1),ImVec2(1,0));
    ImGui::Image((ImTextureID)renderTexture->getTexture(),ImVec2(500,500),ImVec2(0,1),ImVec2(1,0));
    ImGui::End();
    
    bento->imguiRender();

    bento->render();

    auto currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration<float>(currentTime-lastTime).count()*timeScale;//we love our timescales
    elapsedTime += deltaTime;
    lastTime = currentTime;
}

void exit(){
    delete texture;
    delete renderTexture;//i'd probably rather just delete it all in a vector or something
    delete shd;
    bento->exit();
}