#include "opengl.h"

static bool imguiInit = false;

void Bento::initImgui(){
    IMGUI_CHECKVERSION();
    ImGuiContext* imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSetKeyCallback(window,keyCallbackLoc);

    glfwSetCharCallback(window,[](GLFWwindow *window,unsigned int c){
        ImGui_ImplGlfw_CharCallback(window,c);
    });

    glfwSetMouseButtonCallback(window,[](GLFWwindow *window,int button,int action,int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window,button,action,mods);
    });
    imguiInit = true;
}

void Bento::imguiNewFrame() {
    if(imguiInit){
        int width,height;
        glfwGetWindowSize(window,&width,&height);
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.DisplaySize = ImVec2((float)width,(float)height);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
    }
}

void Bento::imguiRender() {
    if(imguiInit){
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
void Bento::exit() {
    if(imguiInit)ImGui_ImplOpenGL3_Shutdown();

    ma_engine_uninit(&engine);


    glfwTerminate();
    for(GLuint vao : vaos)glDeleteVertexArrays(1,&vao);
    for(GLuint buffer : buffs)glDeleteBuffers(1,&buffer);
    std::exit(0);
}