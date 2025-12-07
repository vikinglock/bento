#include "metal.h"

static bool imguiInit = false;

void Bento::initImgui(){
    IMGUI_CHECKVERSION();
    ImGuiContext* imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    ImGui_ImplMetal_Init(device);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imguiInit = true;
}
void Bento::imguiNewFrame(){
    if(imguiInit){
        ImGuiIO& io = ImGui::GetIO();
        glm::vec2 size = getWindowSize();
        io.DisplaySize = ImVec2(size.x,size.y);
        ImGui_ImplMetal_NewFrame(passDescriptor);
        ImGui::NewFrame();
    }
}
void Bento::imguiRender(){
    if(imguiInit){
        ImGui::Render();
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(),commandBuffer,commandEncoder);
    }
}

void Bento::exit(){
    if(imguiInit)ImGui_ImplMetal_Shutdown();

    ma_engine_uninit(&engine);

    #ifndef IOS
    [[NSApplication sharedApplication] terminate:nil];
    #endif
}


__attribute__((weak))
void Bento::poll(){
    prevKeyStates = keyStates;
    prevMouseStates = mouseStates;

    prevTouches = touches;

    wheelY = 0;
    wheelX = 0;
    mouseDelta = glm::vec2(0.0f);
    
    #ifndef IOS
    @autoreleasepool{
        NSEvent *event = nil;
        while ((event = [app nextEventMatchingMask:NSEventMaskAny
                                                untilDate:nil
                                                    inMode:NSDefaultRunLoopMode
                                                    dequeue:YES])){
            [app sendEvent:event];


            if([event type]==NSEventTypeMouseMoved||
            [event type]==NSEventTypeLeftMouseDragged||
            [event type]==NSEventTypeRightMouseDragged||
            [event type]==NSEventTypeOtherMouseDragged)
                mouseDelta = glm::vec2([event deltaX],[event deltaY]);
            
            if(event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp){
                prevKeyStates[event.keyCode] = keyStates[event.keyCode];//what the fuck is this bullshit
                keyStates[event.keyCode] = event.type == NSEventTypeKeyDown;
            }

            //command  0x10
            //alt/option  0x08
            //control  0x04
            //shift  0x02

            if(event.type == NSEventTypeFlagsChanged){
                keyStates[0x38] = (event.modifierFlags & 0x2) != 0;
                keyStates[0x39] = (event.modifierFlags & 0x4) != 0;
                keyStates[0x3B] = (event.modifierFlags & 0x1) != 0;
                keyStates[0x3C] = (event.modifierFlags & 0x2000) != 0;
                keyStates[0x3A] = (event.modifierFlags & 0x20) != 0;
                keyStates[0x3D] = (event.modifierFlags & 0x40) != 0;
                keyStates[0x37] = (event.modifierFlags & 0x8) != 0;
                keyStates[0x36] = (event.modifierFlags & 0x10) != 0;
            }

            if(NSPointInRect([event locationInWindow],
                            [window contentView].frame)||
                            (event.type == NSEventTypeLeftMouseUp||
                            event.type == NSEventTypeRightMouseUp||event.type == NSEventTypeOtherMouseUp)){
                if(event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeLeftMouseUp){
                    prevMouseStates[0] = mouseStates[0];
                    mouseStates[0] = event.type == NSEventTypeLeftMouseDown;
                }else if(event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeRightMouseUp){
                    prevMouseStates[1] = mouseStates[1];
                    mouseStates[1] = event.type == NSEventTypeRightMouseDown;
                }else if(event.type == NSEventTypeOtherMouseDown || event.type == NSEventTypeOtherMouseUp){
                    prevMouseStates[event.buttonNumber] = mouseStates[event.buttonNumber];
                    mouseStates[event.buttonNumber] = event.type == NSEventTypeOtherMouseDown;
                }
            }

            if(event.type == NSEventTypeScrollWheel){
                wheelY = event.scrollingDeltaY;
                wheelX = event.scrollingDeltaX;
            }

            if(imguiInit){
                ImGuiIO &io = ImGui::GetIO();
                switch(event.type){
                    case NSEventTypeKeyUp:
                    case NSEventTypeKeyDown: {
                        bool isKeyDown = (event.type == NSEventTypeKeyDown);
                        
                        auto it = KeytoDearImguiKey.find(event.keyCode);
                        if(it!=KeytoDearImguiKey.end())io.AddKeyEvent(it->second,isKeyDown);

                        //printf("%02x\n",event.keyCode);
                        if(isKeyDown){
                            NSString *characters = event.characters;
                            if(characters.length > 0 && event.type != NSEventTypeFlagsChanged){
                                unichar c = [characters characterAtIndex:0];
                                if(c>=33&&c!=127){
                                    NSEventModifierFlags flags = event.modifierFlags & NSEventModifierFlagDeviceIndependentFlagsMask;
                                    if(!(flags & NSEventModifierFlagFunction)){
                                        io.AddInputCharactersUTF8(characters.UTF8String);
                                    }
                                }
                            }
                        }
                        
                        io.AddKeyEvent(ImGuiMod_Shift,(event.modifierFlags & NSEventModifierFlagShift) != 0);
                        io.AddKeyEvent(ImGuiMod_Ctrl, (event.modifierFlags & NSEventModifierFlagControl) != 0);
                        io.AddKeyEvent(ImGuiMod_Alt,  (event.modifierFlags & NSEventModifierFlagOption) != 0);
                        io.AddKeyEvent(ImGuiMod_Super,(event.modifierFlags & NSEventModifierFlagCommand) != 0);
                    }
                    break;

                    case NSEventTypeFlagsChanged:{
                        io.AddKeyEvent(ImGuiMod_Shift,(event.modifierFlags & NSEventModifierFlagShift) != 0);
                        io.AddKeyEvent(ImGuiMod_Ctrl, (event.modifierFlags & NSEventModifierFlagControl) != 0);
                        io.AddKeyEvent(ImGuiMod_Alt,  (event.modifierFlags & NSEventModifierFlagOption) != 0);
                        io.AddKeyEvent(ImGuiMod_Super,(event.modifierFlags & NSEventModifierFlagCommand) != 0);
                    }
                    break;

                    case NSEventTypeLeftMouseDown:
                    case NSEventTypeLeftMouseUp:io.MouseDown[0] = (event.type == NSEventTypeLeftMouseDown);break;
                    case NSEventTypeRightMouseDown:
                    case NSEventTypeRightMouseUp:io.MouseDown[1] = (event.type == NSEventTypeRightMouseDown);break;
                    case NSEventTypeOtherMouseDown:
                    case NSEventTypeOtherMouseUp:io.MouseDown[event.buttonNumber] = (event.type == NSEventTypeOtherMouseDown);break;
                    case NSEventTypeScrollWheel:io.MouseWheel += event.scrollingDeltaY;io.MouseWheelH += event.scrollingDeltaX;break;
                    case NSEventTypeMouseMoved:
                    case NSEventTypeLeftMouseDragged:
                    case NSEventTypeRightMouseDragged:
                    case NSEventTypeOtherMouseDragged:{
                        CGPoint locationInWindow = [event locationInWindow];
                        io.MousePos = ImVec2(locationInWindow.x,window.contentView.frame.size.height - locationInWindow.y);
                    }break;
                    default:break;
                }
            }

            if(event.type == NSEventTypeKeyDown){
                NSString *chars = [event characters];
                char c = 0;
                if([chars length] > 0){
                    const char *utf8 = [chars UTF8String];
                    if(utf8 && utf8[0]){
                        c = utf8[0];
                    }
                }
                if(keyCallback)keyCallback((char)c,(KeyCode)event.keyCode);
            }
        }
    }
    [app updateWindows];
    #endif
    if(getKey(KEY_LEFT_CONTROL)&&getKey(KEY_LEFT_COMMAND)&&getKey(KEY_F)&&fullscreenable){
        toggleFullscreen();
        fullscreenable = false;
    }
    if(!getKey(KEY_F))fullscreenable = true;
    if(getKey(KEY_LEFT_COMMAND)&&getKey(KEY_Q))exit();
    
    focused = [window isKeyWindow];
    windowFrame = [window frame];
}