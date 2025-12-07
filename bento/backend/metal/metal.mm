#include "../../lib/glm/glm.hpp"
#include "../../lib/glm/gtc/matrix_transform.hpp"
#include "../../lib/glm/gtc/type_ptr.hpp"
//#include "../../lib/hidapi/hidapi.h"
#include "metal.h"
#include "framebuffer.h"
#include "metalcommon.h"
#include "../../utils.h"

#include <unistd.h>
#include <limits.h>
#include <string>
#include <map>
#include <mach-o/dyld.h>

#include <iostream>
#include <signal.h>
#include <Metal/Metal.h>
#include "metalcommon.h"

__attribute__((constructor))//i think because of this it has to be clang or gcc but i may be wrong
static void pre() {
    device = MTLCreateSystemDefaultDevice();
    //signal(SIGSEGV,[](int s){std::cout<<"segmentation fault: "<<s<<std::endl;_exit(128+s);});
    //signal(SIGABRT,[](int s){std::cout<<"sigabrt: "<<s<<std::endl;_exit(128+s);});
    //signal(SIGBUS,[](int s){std::cout<<"bus error: "<<s<<std::endl;_exit(128+s);});
    //signal(SIGILL,[](int s){std::cout<<"illegal instruction: "<<s<<std::endl;_exit(128+s);});
    //signal(SIGFPE,[](int s){std::cout<<"floating point exception: "<<s<<std::endl;_exit(128+s);});
    //signal(SIGTRAP,[](int s){std::cout<<"breakpoint trap: "<<s<<std::endl;_exit(128+s);});
}


#ifdef IOS
#include <UIKit/UIKit.h>
#else
#include <Cocoa/Cocoa.h>
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "../../lib/miniaudio/miniaudio.h"

//ay yo i'm gonna leave blending as it is
//because for my own use it's gonna be about as primitive as it is here
//improve upon it if you wish but i'm gonna fix it later


uint32_t currentFrame = 0;

#ifdef IMGUI
bool imguiInit = false;
#endif

ma_engine engine;

id<MTLDevice> device = nil;
id<MTLCommandQueue> commandQueue = nil;

int startAtt;
int endAtt;
int startTex;
int endTex;
int depthInd;

std::vector<id<MTLTexture>> outTexture;


std::vector<std::array<id<MTLBuffer>,MAX_FRAMES_IN_FLIGHT>> buffers;
std::vector<size_t> bufferCounts;

std::vector<id<MTLTexture>> textures;
std::vector<id<MTLSamplerState>> sampler;

std::vector<id<MTLTexture>> depthTextures;

int maxVertInd = 3;

Texture* Bento::AppTexture;
FrameBuffer* Bento::DefaultFrameBuffer;
Shader* Bento::DefaultShader;


#ifdef IOS
BentoRenderer* delegate = nil;
int reallylonganduniquenameforbentosoyoullneveraccidentallymakeafunctionforthis(int argc,const char *argv[]);
@interface BTView : MTKView
@end

@implementation BTView

- (void)layoutSubviews {
    [super layoutSubviews];
    if(self.superview){
        self.frame = self.superview.bounds;
    }
}

@end

@implementation BentoRenderer

    - (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
        delegate = self;
        reallylonganduniquenameforbentosoyoullneveraccidentallymakeafunctionforthis(0,nullptr);
        
        return YES;
    }

    - (void)drawInMTKView:(BTView *)view {
        loop();
        self.view = view;
    }

    - (void)mtkView:(BTView *)view drawableSizeWillChange:(CGSize)size {}

    std::map<NSNumber*,size_t> touchMap;
    
    - (void)updateTouch:(UITouch*)touch phase:(UITouchPhase)phase {
        NSNumber *touchKey = [NSNumber numberWithUnsignedLong:(uintptr_t)touch];
        
        switch(phase){
            case UITouchPhaseBegan: {
                for(size_t i = 0; i < touches.size(); i++){
                    if(!touches[i].down){
                        touches[i].down = true;
                        touches[i].id = i;
                        CGPoint pos = [touch locationInView:self.view];
                        touches[i].position = glm::vec2((pos.x/self.view.bounds.size.width)*2-1,1-(pos.y/self.view.bounds.size.height)*2);
                        touchMap[touchKey] = i;
                        break;
                    }
                }
            }
            break;
            case UITouchPhaseMoved:{
                auto it = touchMap.find(touchKey);
                if(it!=touchMap.end()){
                    CGPoint pos = [touch locationInView:self.view];
                    touches[it->second].position = glm::vec2((pos.x/self.view.bounds.size.width)*2-1,1-(pos.y/self.view.bounds.size.height)*2);
                }
            }
            break;
            case UITouchPhaseEnded:case UITouchPhaseCancelled:{
                auto it = touchMap.find(touchKey);
                if(it!=touchMap.end()){
                    touches[it->second].down = false;
                    touchMap.erase(it);
                }
            }
            break;
        }
    }
    - (void)touchesBegan:(NSSet<UITouch*>*)tchs withEvent:(UIEvent*)event{for(UITouch *t in tchs)[self updateTouch:t phase:UITouchPhaseBegan];}
    - (void)touchesMoved:(NSSet<UITouch*>*)tchs withEvent:(UIEvent*)event{for(UITouch *t in tchs)[self updateTouch:t phase:UITouchPhaseMoved];}
    - (void)touchesEnded:(NSSet<UITouch*>*)tchs withEvent:(UIEvent*)event{for(UITouch *t in tchs)[self updateTouch:t phase:UITouchPhaseEnded];}
    - (void)touchesCancelled:(NSSet<UITouch*>*)tchs withEvent:(UIEvent*)event{for(UITouch *t in tchs)[self updateTouch:t phase:UITouchPhaseCancelled];}
@end
#endif
void loop();
void exit();

std::vector<TouchPoint> Bento::touches(11);
std::vector<TouchPoint> Bento::prevTouches(11);

// #### MAIN ####

#include <Cocoa/Cocoa.h>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <crt_externs.h>

Bento::Bento(const char *title,int width,int height,int x,int y){
    @autoreleasepool{
        app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        shouldClose = false;
        NSRect frame = NSMakeRect(0,0,width,height);
        
        window = [NSWindow alloc];
        window = [window initWithContentRect:frame styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable) backing:NSBackingStoreBuffered defer:NO];

        [window setFrameOrigin:NSMakePoint(x,([NSScreen mainScreen].frame.size.height-window.frame.size.height)-y)];
        [window setTitle:@(title)];
        [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

        commandQueue = [device newCommandQueue];


        metalLayer = [CAMetalLayer layer];

        metalLayer.device = device;
        metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        metalLayer.colorspace = nil;
        metalLayer.wantsExtendedDynamicRangeContent = YES;
        metalLayer.framebufferOnly = YES;
        

        metalLayer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
        [metalLayer setFrame:frame];


        NSView* view = [window contentView];
        [view setLayer:metalLayer];
        [view setWantsLayer:YES];

        NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
        [notificationCenter addObserverForName:NSWindowWillCloseNotification
                                        object:window
                                         queue:nil
                                    usingBlock:^(NSNotification *notification){
                                        shouldClose = true;
                                        [app terminate:nil];
                                    }];

        [notificationCenter addObserverForName:NSWindowDidResizeNotification
                                        object:window
                                         queue:nil
                                    usingBlock:^(NSNotification *notification){
            NSView* view = [notification.object contentView];

            if(view.layer){
                CAMetalLayer *metalLayer = (CAMetalLayer *)view.layer;

                CGSize drawableSize = [view convertSizeToBacking:view.frame.size];
                metalLayer.drawableSize = drawableSize;

                MTLTextureDescriptor *depthTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float width:drawableSize.width height:drawableSize.height mipmapped:NO];
                depthTextureDesc.storageMode = MTLStorageModePrivate;
                depthTextureDesc.usage = MTLTextureUsageRenderTarget;

                MTLTextureDescriptor *appTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:drawableSize.width height:drawableSize.height mipmapped:NO];
                appTextureDesc.storageMode = MTLStorageModePrivate;
                appTextureDesc.usage = MTLTextureUsageRenderTarget;
                
                
                depthTexture = [device newTextureWithDescriptor:depthTextureDesc];
                appTexture = [device newTextureWithDescriptor:appTextureDesc];

                if(resizeCallback)resizeCallback(glm::ivec2(drawableSize.width,drawableSize.height));
            }
        }];//why did i get rid of this???
        

        Bento::AppTexture = new Texture(nullptr,glm::ivec2(width,height));

        Bento::DefaultFrameBuffer = new FrameBuffer(Bento::AppTexture);
        Bento::DefaultFrameBuffer->size = glm::ivec2(width,height);
        bindFrameBuffer(Bento::DefaultFrameBuffer);

        MTLTextureDescriptor *appTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:width height:height mipmapped:NO];
        appTextureDesc.storageMode = MTLStorageModePrivate;
        appTextureDesc.usage = MTLTextureUsageRenderTarget;
        appTexture = [device newTextureWithDescriptor:appTextureDesc];
        
        MTLSamplerDescriptor *samplerDescriptor = [MTLSamplerDescriptor alloc];
        samplerDescriptor = [samplerDescriptor init];
        samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
        samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
        samplerDescriptor.rAddressMode = MTLSamplerAddressModeRepeat;
        samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
        samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
        rTSampler = [device newSamplerStateWithDescriptor:samplerDescriptor];
        /*#ifdef IOS
            this->renderer = (__bridge void*)delegate;

            delegate.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
            delegate.commandQueue = [device newCommandQueue];
            UIViewController *vc = [[UIViewController alloc] init];
            BTView *view = [[BTView alloc] initWithFrame:delegate.window.bounds device:device];
            view.delegate = delegate;
            view.clearColor = MTLClearColorMake(0,0,0,1);
            view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
            [vc.view addSubview:view];
            delegate.window.rootViewController = vc;
            [delegate.window makeKeyAndVisible];

            window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
            BTView *view = [[BTView alloc] initWithFrame:window.bounds device:device];
            view.delegate = renderer;
            view.clearColor = MTLClearColorMake(0,0,0,1);
            view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
            UIViewController *vc = [UIViewController new];
            [vc.view addSubview:view];
            window.rootViewController = vc;
            [window makeKeyAndVisible];
        #else//be sane

        #endif*/
        std::string dir = getExecutablePath();

        NSError *error = nil;
        NSString *shaderSource = @"#include <metal_stdlib>\n\
        using namespace metal;\
        struct main0_in{\
            float2 position [[attribute(0)]];\
            float2 uv [[attribute(1)]];\
        };\
        struct main0_out{\
            float4 position [[position]];float2 fuv [[user(locn0)]];\
        };\
        vertex main0_out vertexMain(main0_in in [[stage_in]]){\
            main0_out out;\
            out.position = float4(in.position,0.0,1.0);\
            out.fuv=in.uv;\
            return out;\
        }\
        \
        fragment float4 fragmentMain(main0_out in [[stage_in]],texture2d<float> tex[[texture(0)]],sampler texSmplr[[sampler(0)]]){\
            return tex.sample(texSmplr,in.fuv);\
        }";

        id<MTLLibrary> library = [device newLibraryWithSource:shaderSource options:nil error:&error];
        if(!library){
            NSString* localizedDescription = [error localizedDescription];
            std::cerr << "couldn't create the default shaders (somehow): " << [localizedDescription UTF8String] << std::endl << "report this or something this shouldn't happen unless you changed the source code" << std::endl;
        }
        
        std::string vertShaderSourceC = std::string([shaderSource UTF8String]);
        std::string fragShaderSourceC = std::string([shaderSource UTF8String]);

        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexMain"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentMain"];

        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor alloc];
        vertexDescriptor = [vertexDescriptor init];


        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
        vertexDescriptor.attributes[1].offset = 0;
        vertexDescriptor.attributes[1].bufferIndex = 1;
        vertexDescriptor.layouts[0].stride = sizeof(float) * 2;
        vertexDescriptor.layouts[0].stepRate = 1;
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[1].stride = sizeof(float) * 2;
        vertexDescriptor.layouts[1].stepRate = 1;
        vertexDescriptor.layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;


        MTLRenderPipelineDescriptor* pipelineDescriptor = [MTLRenderPipelineDescriptor alloc];
        pipelineDescriptor = [pipelineDescriptor init];

        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;

        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        pipelineDescriptor.colorAttachments[0].blendingEnabled = YES;
        pipelineDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pipelineDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        pipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;

        pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if(!pipelineState){
            NSString* localizedDescription = [error localizedDescription];
            std::cerr << "couldn't create default pipeline state (somehow): " << [localizedDescription UTF8String] << std::endl << "again, (please don't) report this because this shouldn't happen unless you changed the source code" << std::endl;
        }
        VAO vao;
        vao.setAttrib(0,2,AttribFormat::Float);
        vao.setAttrib(1,2,AttribFormat::Float);
        
        Bento::DefaultShader = new Shader(pipelineState,vertShaderSourceC,fragShaderSourceC,vao);
        shader = Bento::DefaultShader;

        
        MTLTextureDescriptor *depthTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float width:width height:height mipmapped:NO];
        depthTextureDesc.usage = MTLTextureUsageRenderTarget;
        depthTexture = [device newTextureWithDescriptor:depthTextureDesc];
        
        MTLDepthStencilDescriptor *depthStencilDesc = [MTLDepthStencilDescriptor alloc];
        depthStencilDesc = [depthStencilDesc init];
        depthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDesc.depthWriteEnabled = YES;
        yesDepthStencilState = [device newDepthStencilStateWithDescriptor:depthStencilDesc];
        depthStencilDesc = [MTLDepthStencilDescriptor alloc];
        depthStencilDesc = [depthStencilDesc init];
        depthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDesc.depthWriteEnabled = NO;
        noDepthStencilState = [device newDepthStencilStateWithDescriptor:depthStencilDesc];


        depthStencilState = noDepthStencilState;

        //gone for now

        [app finishLaunching];


        windowFrame = [window frame];
        
        
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown handler:^NSEvent * _Nullable(NSEvent *event){
            return nil;
        }];//idk what this is


        NSMenu* bar = [NSMenu alloc];
        bar = [bar init];
        [NSApp setMainMenu:bar];

        NSMenuItem* appMenuItem = [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
        NSMenu* appMenu = [NSMenu alloc];
        appMenu = [appMenu init];
        [appMenuItem setSubmenu:appMenu];

        [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@",@(title)] action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
        [appMenu addItem:[NSMenuItem separatorItem]];
        NSMenu* servicesMenu = [NSMenu alloc];
        servicesMenu = [servicesMenu init];
        [NSApp setServicesMenu:servicesMenu];
        [[appMenu addItemWithTitle:@"Services" action:NULL keyEquivalent:@""] setSubmenu:servicesMenu];
        [servicesMenu release];
        [appMenu addItem:[NSMenuItem separatorItem]];
        [appMenu addItemWithTitle:[NSString stringWithFormat:@"Hide %@",@(title)] action:@selector(hide:) keyEquivalent:@"h"];
        [[appMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"] setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
        [appMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
        [appMenu addItem:[NSMenuItem separatorItem]];
        [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@",@(title)] action:@selector(terminate:) keyEquivalent:@"q"];

        NSMenuItem* windowMenuItem = [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
        [bar release];
        NSMenu* windowMenu = [NSMenu alloc];
        windowMenu = [windowMenu initWithTitle:@"Window"];
        [NSApp setWindowsMenu:windowMenu];
        [windowMenuItem setSubmenu:windowMenu];

        [windowMenu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
        [windowMenu addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];
        [windowMenu addItem:[NSMenuItem separatorItem]];
        [windowMenu addItemWithTitle:@"Bring All to Front" action:@selector(arrangeInFront:) keyEquivalent:@""];
        [windowMenu addItem:[NSMenuItem separatorItem]];
        NSMenuItem* fullscreenMenuItem = [windowMenu addItemWithTitle:@"Enter Full Screen" action:@selector(toggleFullScreen:) keyEquivalent:@"f"];
        [fullscreenMenuItem setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];

        // Prior to Snow Leopard, we need to use this oddly-named semi-private API
        // to get the application menu working properly.
        SEL setAppleMenuSelector = NSSelectorFromString(@"setAppleMenu:");
        [NSApp performSelector:setAppleMenuSelector withObject:appMenu];


        #ifdef FREEZE_FILES
        File::loadFrozenFilesystem("resources.fz");
        #endif
    }
}
void Bento::initSound(){ma_engine_init(NULL,&engine);}
void Bento::setClearColor(glm::vec4 col){clearColor = col;}

void Bento::predraw(){
    if(encoding){
        [commandEncoder endEncoding];
        [commandEncoder release];
        [commandBuffer commit];
        [commandBuffer release];
    }
    commandBuffer = [commandQueue commandBuffer];

    #ifdef IOS
    view.clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,1.0);
    passDescriptor = view.currentRenderPassDescriptor;
    if(!passDescriptor) return;
    #else//be normal
    if(!encoding)drawable = [metalLayer nextDrawable];
    if(!drawable||!appTexture)return;
    /*passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = (__bridge id<MTLTexture>)Bento::AppTexture->getTexture();
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    passDescriptor.depthAttachment.texture = depthTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;*/
    #endif

    for(int i = 0; i < framebuffer->attachments.size(); i++){
        framebuffer->passDescriptor.colorAttachments[framebuffer->attachments[i]].texture = (__bridge id<MTLTexture>)framebuffer->targets[i]->getTexture();
        framebuffer->passDescriptor.colorAttachments[framebuffer->attachments[i]].clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
        if(clearColor.w==0)framebuffer->passDescriptor.colorAttachments[framebuffer->attachments[i]].loadAction = MTLLoadActionLoad;
        else framebuffer->passDescriptor.colorAttachments[framebuffer->attachments[i]].loadAction = MTLLoadActionClear;
        if(framebuffer->depthEnabled){
            framebuffer->passDescriptor.depthAttachment.texture = (__bridge id<MTLTexture>)framebuffer->depthTexture->getTexture();
            depthStencilState = yesDepthStencilState;
        }else depthStencilState = noDepthStencilState;
    }
    
    
    commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:framebuffer->passDescriptor];
    [commandEncoder setRenderPipelineState:pipelineState];
    encoding = true;
}

void Bento::draw(Primitive primitive){
    if(!drawable)return;
    @autoreleasepool{
        MTLPrimitiveType prim;
        switch(primitive){
            case Primitive::Points:prim=MTLPrimitiveTypePoint;break;
            case Primitive::Lines:prim=MTLPrimitiveTypeLine;break;
            case Primitive::LineStrip:prim=MTLPrimitiveTypeLineStrip;break;
            case Primitive::Triangles:prim=MTLPrimitiveTypeTriangle;break;
            case Primitive::TriangleStrip:prim=MTLPrimitiveTypeTriangleStrip;break;
        }
        if(!passDescriptor)return;
        [commandEncoder setCullMode:MTLCullModeFront];
        [commandEncoder setDepthStencilState:depthStencilState];

        for(int i=0;i<buffers.size();i++){
            [commandEncoder setVertexBuffer:buffers[i][currentFrame] offset:0 atIndex:i];
        }
        for(int i=0;i<textures.size();i++){
            [commandEncoder setFragmentTexture:textures[i] atIndex:i];
            [commandEncoder setFragmentSamplerState:sampler[i] atIndex:i];
        }

        id<MTLBuffer> fragBuffer = shader->fragBuffer;
        id<MTLBuffer> vertBuffer = shader->vertBuffer;


        //[commandEncoder setFragmentBuffer:shader->fragBuffer offset:0 atIndex:0];
        if(fragBuffer)[commandEncoder setFragmentBytes:[fragBuffer contents] length:[fragBuffer length] atIndex:0];
        //use this if you wanna switch inbetween idk
        if(vertBuffer)[commandEncoder setVertexBytes:[vertBuffer contents] length:[vertBuffer length] atIndex:maxVertInd];

        [commandEncoder drawPrimitives:prim vertexStart:0 vertexCount:bufferCounts[0]];
        currentFrame = (currentFrame+1)%MAX_FRAMES_IN_FLIGHT;
    }
}


void Bento::bindFrameBuffer(FrameBuffer* framebuffer){this->framebuffer = framebuffer;}//relationship ended with Bento::drawTex()  now Bento::bindFrameBuffer(FrameBuffer*) is my best friend
void Bento::render(){
    if(!passDescriptor)return;
    if(!drawable)return;
    if(encoding){
        [commandEncoder endEncoding];
        [commandEncoder release];
    }


    /*id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
    [blitEncoder copyFromTexture:appTexture
            sourceSlice:0
            sourceLevel:0
            sourceOrigin:MTLOriginMake(0,0,0)
            sourceSize:MTLSizeMake(appTexture.width,appTexture.height,1)//window.frame.size.width*window.backingScaleFactor
            toTexture:drawable.texture
        destinationSlice:0
        destinationLevel:0
    destinationOrigin:MTLOriginMake(0,0,0)];
    [blitEncoder endEncoding];*/

    #ifdef IOS
    [commandBuffer presentDrawable:view.currentDrawable];
    #else
    [commandBuffer presentDrawable:drawable];
    #endif
    [commandBuffer commit];
    [commandBuffer release];
    [drawable release];
}

void Bento::drawScreen(Shader* shd){
    if(!passDescriptor)passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = drawable.texture;
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    passDescriptor.depthAttachment.texture = depthTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    
    if(encoding){
        [commandEncoder endEncoding];
        [commandEncoder release];
    }
    commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [commandEncoder setRenderPipelineState:shd->pipelineState];//Bento::DefaultShader
    encoding = true;

    
    bindTexture(Bento::AppTexture,0);
    static std::vector<glm::vec2> buf = {
        glm::vec2(-1,-1),glm::vec2(1,-1),glm::vec2(-1,1),
        glm::vec2(-1,1),glm::vec2(1,-1),glm::vec2(1,1)
    };
    static std::vector<glm::vec2> uvbuf = {
        glm::vec2(0,0),glm::vec2(1,0),glm::vec2(0,1),
        glm::vec2(0,1),glm::vec2(1,0),glm::vec2(1,1)
    };
    static id<MTLBuffer> screenBuffer = [device newBufferWithBytes:buf.data() length:buf.size() * sizeof(glm::vec2) options:MTLResourceStorageModePrivate];
    static id<MTLBuffer> screenBufferUV = [device newBufferWithBytes:uvbuf.data() length:uvbuf.size() * sizeof(glm::vec2) options:MTLResourceStorageModePrivate];

    [commandEncoder setCullMode:MTLCullModeFront];
    [commandEncoder setDepthStencilState:depthStencilState];

    [commandEncoder setVertexBuffer:screenBuffer offset:0 atIndex:0];
    [commandEncoder setVertexBuffer:screenBufferUV offset:0 atIndex:1];

    for(int i=0;i<textures.size();i++){
        [commandEncoder setFragmentTexture:textures[i] atIndex:i];
        [commandEncoder setFragmentSamplerState:sampler[i] atIndex:i];
    }
    
    [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
    currentFrame = (currentFrame+1)%MAX_FRAMES_IN_FLIGHT;
    
    [commandEncoder endEncoding];
    [commandEncoder release];
    encoding = false;
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

    /*#ifndef IOS
    NSScreen* mainScreen = [NSScreen mainScreen];
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:mainScreen.maximumRefreshInterval]];
    #endif*/ //nvm clangd and we're good
}


void Bento::predrawTex(int width,int height){
    commandBuffer = [commandQueue commandBuffer];

    #ifdef IOS
    view.clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,1.0);
    passDescriptor = view.currentRenderPassDescriptor;
    if(!passDescriptor) return;
    
    if(!outTexture[startTex] || outTexture[startTex].width != width || outTexture[startTex].height != height){
        @autoreleasepool{
            MTLTextureDescriptor *textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                                width:width
                                                                                                height:height
                                                                                            mipmapped:NO];
            textureDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            outTexture[startTex] = [device newTextureWithDescriptor:textureDesc];
        }
    }
    
    passDescriptor.colorAttachments[0].texture = outTexture[startTex];
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    #else//be normal
    passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];

    if(!outTexture[startTex] || outTexture[startTex].width != width || outTexture[startTex].height != height){
        @autoreleasepool{
            MTLTextureDescriptor *textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                                width:width
                                                                                                height:height
                                                                                            mipmapped:NO];
            textureDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            outTexture[startTex] = [device newTextureWithDescriptor:textureDesc];
        }
    }
    
    passDescriptor.colorAttachments[0].texture = outTexture[startTex];
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    passDescriptor.depthAttachment.texture = depthTTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    #endif

    commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [commandEncoder setRenderPipelineState:pipelineState];
}

void Bento::drawTex(Primitive primitive){
    MTLPrimitiveType prim;
    switch(primitive){
        case Primitive::Points:prim=MTLPrimitiveTypePoint;break;
        case Primitive::Lines:prim=MTLPrimitiveTypeLine;break;
        case Primitive::LineStrip:prim=MTLPrimitiveTypeLineStrip;break;
        case Primitive::Triangles:prim=MTLPrimitiveTypeTriangle;break;
        case Primitive::TriangleStrip:prim=MTLPrimitiveTypeTriangleStrip;break;
    }
    
    if(!passDescriptor)return;
    [commandEncoder setCullMode:MTLCullModeFront];
    [commandEncoder setDepthStencilState:depthStencilState];

    for(int i=0;i<buffers.size();i++){
        [commandEncoder setVertexBuffer:buffers[i][currentFrame] offset:0 atIndex:i];
    }
    for(int i=0;i<textures.size();i++){
        [commandEncoder setFragmentTexture:textures[i] atIndex:i];
        [commandEncoder setFragmentSamplerState:sampler[i] atIndex:i];
    }

    id<MTLBuffer> fragBuffer = shader->fragBuffer;
    id<MTLBuffer> vertBuffer = shader->vertBuffer;


    //[commandEncoder setFragmentBuffer:shader->fragBuffer offset:0 atIndex:0];
    if(fragBuffer)[commandEncoder setFragmentBytes:[fragBuffer contents] length:[fragBuffer length] atIndex:0];
    
    if(vertBuffer)[commandEncoder setVertexBytes:[vertBuffer contents] length:[vertBuffer length] atIndex:maxVertInd];
    
    [commandEncoder drawPrimitives:prim vertexStart:0 vertexCount:bufferCounts[0]];
    currentFrame = (currentFrame+1)%MAX_FRAMES_IN_FLIGHT;//even though it's literally deprecated
}

void Bento::renderTex(){
    [commandEncoder endEncoding];
    [commandBuffer commit];
}

void Bento::setActiveTextures(int start,int end){
    startTex = start;
    endTex = end+1;
    if(outTexture.size()<end+1)outTexture.resize(end+1);
}
void Bento::setActiveTextures(int ind){
    startTex = ind;
    endTex = ind+1;
    if(outTexture.size()<ind+1)outTexture.resize(ind+1);
}

void Bento::setActiveDepthTexture(int ind){
    depthInd = ind;
    if(depthTextures.size()<ind+1)depthTextures.resize(ind+1);
}

void Bento::setActiveAttachments(int start,int end){
    startAtt = start;
    endAtt = end+1;
}
void Bento::setActiveAttachments(int ind){
    startAtt = ind;
    endAtt = ind+1;
}

void Bento::renderToTex(Texture*& tex,int ind){
    @autoreleasepool{
        if(!tex)tex = new Texture();
        tex->texture = outTexture[ind];
    }
}
void Bento::renderDepthToTex(Texture*& tex,int ind){
    @autoreleasepool{
        if(!tex)tex = new Texture();
        tex->texture = depthTextures[ind];
    }
}

void Bento::normalizeTexture(int index,bool normalized){
    //metal textures aren't normalized
    //i made it like that and if isn't like that then metal blows up
    //i could fix it but i just wanna make games man i don't like this no more ):
}
//return new Texture([renderer renderTex],self.rTSampler);

void Bento::toggleFullscreen(){
    #ifndef IOS
    dispatch_async(dispatch_get_main_queue(),^{[window toggleFullScreen:nil];});
    #endif
}


bool Bento::isWindowFocused(){return focused;}

// #### UNIFORMS ####
void Bento::setVertices(const std::vector<glm::vec3>& vertices){
    if(buffers.size() == 0)buffers.resize(1);
    buffers[0][currentFrame] = [device newBufferWithBytes:vertices.data()
                                            length:vertices.size() * sizeof(glm::vec3)
                                        options:MTLResourceStorageModeShared];
}

void Bento::setNormals(const std::vector<glm::vec3>& normals){
    if(buffers.size() <= 1)buffers.resize(2);
    buffers[1][currentFrame] = [device newBufferWithBytes:normals.data()
                                            length:normals.size() * sizeof(glm::vec3)
                                        options:MTLResourceStorageModeShared];
}

void Bento::setUvs(const std::vector<glm::vec2>& uvs){
    if(buffers.size() <= 2)buffers.resize(3);
    buffers[2][currentFrame] = [device newBufferWithBytes:uvs.data()
                                        length:uvs.size() * sizeof(glm::vec2)
                                    options:MTLResourceStorageModeShared];
}

void Bento::internalSetVertexBuffer(int index,void* data,size_t size,size_t typeSize){
    if(buffers.size() <= index){
        buffers.resize(index+1);
        bufferCounts.resize(index+1);
        maxVertInd = index+1;
    }
    if(!buffers[index][currentFrame]||size > bufferCounts[index]){
        if(buffers[index][currentFrame])[buffers[index][currentFrame] release];
        buffers[index][currentFrame] = [device newBufferWithLength:size*typeSize
                                            options:MTLResourceStorageModeShared];
    }
    
    memcpy([buffers[index][currentFrame] contents],data,size*typeSize);

    bufferCounts[index] = size;
}

void Bento::bindTexture(Texture *tex,int index){
    @autoreleasepool{
        if(textures.size() <= index){
            textures.resize(index + 1);
            sampler.resize(index + 1);
        }
        if(tex){
            textures[index] = (__bridge id<MTLTexture>)tex->getTexture();
            sampler[index] = tex->sampler;
        }else{
            textures[index] = nil;
            sampler[index] = nil;
        }
    }
}

void Bento::unbindTexture(){}

// #### INPUT ####

bool Bento::getKey(int key){return keyStates[key];}
bool Bento::getMouse(int mouse){return mouseStates[mouse];}
bool Bento::getKeyDown(int key){return(keyStates[key]&&!prevKeyStates[key]);}
bool Bento::getKeyUp(int key){return(!keyStates[key]&&prevKeyStates[key]);}
bool Bento::getMouseDown(int button){return(mouseStates[button]&&!prevMouseStates[button]);}
bool Bento::getMouseUp(int button){return(!mouseStates[button]&&prevMouseStates[button]);}

double Bento::getScroll(int wheel){
    if(wheel==0)return wheelY;
    if(wheel==1)return wheelX;
    return 0;
}

// #### MOUSE AND WINDOWS ####

glm::vec2 Bento::getWindowSize(){
    #ifndef IOS
    return glm::vec2(windowFrame.size.width,windowFrame.size.height);
    #endif
    return getDisplaySize();
}

glm::vec2 Bento::getWindowPos(){
    @autoreleasepool{
        #ifndef IOS
        glm::vec2 dSize = getDisplaySize();
        return glm::vec2(windowFrame.origin.x,dSize.y-(windowFrame.origin.y+windowFrame.size.height));
        #endif
    }
    return glm::vec2(0);
}

void Bento::setMouseCursor(bool hide,int cursor){
    @autoreleasepool{
        #ifndef IOS
        if(hide)[NSCursor hide];
        else{
            [NSCursor unhide];
            NSCursor* nscursor=[NSCursor arrowCursor];
            switch(cursor){
                case  0:nscursor=[NSCursor arrowCursor];break;
                case  1:nscursor=[NSCursor IBeamCursor];break;
                case  2:nscursor=[NSCursor crosshairCursor];break;
                case  3:nscursor=[NSCursor closedHandCursor];break;
                case  4:nscursor=[NSCursor openHandCursor];break;
                case  5:nscursor=[NSCursor pointingHandCursor];break;
                case  6:nscursor=[NSCursor resizeLeftCursor];break;
                case  7:nscursor=[NSCursor resizeRightCursor];break;
                case  8:nscursor=[NSCursor resizeLeftRightCursor];break;
                case  9:nscursor=[NSCursor resizeUpCursor];break;
                case 10:nscursor=[NSCursor resizeDownCursor];break;
                case 11:nscursor=[NSCursor resizeUpDownCursor];break;
                case 12:nscursor=[NSCursor disappearingItemCursor];break;
                case 13:nscursor=[NSCursor IBeamCursorForVerticalLayout];break;
                case 14:nscursor=[NSCursor operationNotAllowedCursor];break;
                case 15:nscursor=[NSCursor dragLinkCursor];break;
                case 16:nscursor=[NSCursor dragCopyCursor];break;
                case 17:nscursor=[NSCursor contextualMenuCursor];break;
                case 18:nscursor=[NSCursor zoomInCursor];break;
                case 19:nscursor=[NSCursor zoomOutCursor];break;
                case 20:nscursor=[NSCursor columnResizeCursor];break;
                case 21:nscursor=[NSCursor rowResizeCursor];break;
            }
            [nscursor set];
        }
        #endif
    }
}

void Bento::lockMouse(bool locked){
    #ifndef IOS
    CGAssociateMouseAndMouseCursorPosition(!locked);
    #endif
}

glm::vec2 Bento::getMousePosition(){
    #ifndef IOS
    @autoreleasepool{
        glm::vec2 dsize = getDisplaySize();
        CGPoint mouseLocation = [NSEvent mouseLocation];
        return glm::vec2(mouseLocation.x,dsize.y-mouseLocation.y);
    }
    #endif
    return glm::vec2(0);
}

glm::vec2 Bento::getMouseDelta(){
    #ifndef IOS
    return mouseDelta;
    #endif
    return glm::vec2(0);
}

void Bento::setMousePosition(glm::vec2 pos,bool needsFocus){
    #ifndef IOS
    @autoreleasepool{
        if(isWindowFocused()||!needsFocus){
            CGEventRef moveEvent = CGEventCreateMouseEvent(NULL,kCGEventMouseMoved,NSMakePoint(pos.x,pos.y),kCGMouseButtonLeft);
            CGEventPost(kCGHIDEventTap,moveEvent);
            CFRelease(moveEvent);
        }
    }
    #endif
}


void Bento::setWindowPos(glm::vec2 pos){
    #ifndef IOS
    @autoreleasepool{
        glm::vec2 wSize = getWindowSize();
        glm::vec2 dSize = getDisplaySize();
        dispatch_async(dispatch_get_main_queue(),^{[window setFrameOrigin:NSMakePoint(pos.x,dSize.y-(pos.y+wSize.y))];});
    }
    #endif
}

glm::vec2 Bento::getControllerAxis(int controller,JoystickType joystick){
    return glm::vec2(0);
}

bool Bento::getControllerButton(int controller,ButtonType button){
    return false;
}

glm::vec2 Bento::getDisplaySize(){
    @autoreleasepool{
        #ifndef IOS
        NSScreen* mainScreen = [NSScreen mainScreen];
        CGSize size = [mainScreen frame].size;
        return glm::vec2(size.width,size.height);
        #else
        CGSize size = [UIScreen mainScreen].bounds.size;
        CGFloat scale = [UIScreen mainScreen].scale;
        return glm::vec2(size.width*scale,size.height*scale);
        #endif
    }
}

glm::vec2 Bento::getFramebufferSize(){
    @autoreleasepool{
        CGSize size = metalLayer.drawableSize;
        return glm::vec2(size.width,size.height);
    }
}

void (*Bento::keyCallback)(char,KeyCode) = nullptr;
void (*Bento::resizeCallback)(glm::ivec2) = nullptr;

void Bento::setKeyCallback(void (*callback)(char,KeyCode)){keyCallback = callback;}
void Bento::setResizeCallback(void (*callback)(glm::ivec2)){resizeCallback = callback;}


void Bento::setShader(Shader* shd){
    shader = shd;
    pipelineState = shader->pipelineState;
}

void Bento::enable(Feature f,bool enabled){
    @autoreleasepool{
        switch(f){
            case 0:
            #ifndef IOS
            metalLayer.displaySyncEnabled = enabled?YES:NO;
            #endif
            break;
            case 1:depthStencilState = enabled?yesDepthStencilState:noDepthStencilState;break;
            case 2:
            #ifndef IOS
            if(enabled){
                dispatch_async(dispatch_get_main_queue(),^{
                    NSWindowStyleMask mask = [window styleMask];
                    mask |= NSWindowStyleMaskFullSizeContentView;
                    [window setStyleMask:mask];
                    [window setTitleVisibility:NSWindowTitleHidden];
                    [window setTitlebarAppearsTransparent:YES];
                });
            }else{
                dispatch_async(dispatch_get_main_queue(),^{
                    NSWindowStyleMask mask = [window styleMask];
                    mask |= NSWindowStyleMaskTitled;
                    [window setStyleMask:mask];
                    [window setTitleVisibility:NSWindowTitleVisible];
                    [window setTitlebarAppearsTransparent:NO];
                });
            }
            break;
            #endif
            case 3:
            #ifndef IOS
            if(enabled){
                dispatch_async(dispatch_get_main_queue(),^{
                    NSWindowStyleMask mask = [window styleMask];
                    mask &= ~NSWindowStyleMaskTitled;
                    mask |= NSWindowStyleMaskFullSizeContentView;
                    [window setStyleMask:mask];
                    [window setTitleVisibility:NSWindowTitleHidden];
                    [window setTitlebarAppearsTransparent:YES];
                });
            }else{
                dispatch_async(dispatch_get_main_queue(),^{
                    NSWindowStyleMask mask = [window styleMask];
                    mask |= NSWindowStyleMaskTitled;
                    [window setStyleMask:mask];
                    [window setTitleVisibility:NSWindowTitleVisible];
                    [window setTitlebarAppearsTransparent:NO];
                });
            }
            #endif
            break;
        }
    }
}

void Bento::focus(){
    dispatch_async(dispatch_get_main_queue(),^{
        [window setReleasedWhenClosed:NO];
        [window makeKeyAndOrderFront:nil];
        [app activateIgnoringOtherApps:YES];
    });
}

void Bento::startLoop(){
    #ifndef IOS
    while(this->isRunning())loop();
    exit();
    #endif
}

__attribute__((weak))
void Bento::exit(){
    ma_engine_uninit(&engine);

    #ifndef IOS
    [[NSApplication sharedApplication] terminate:nil];
    #endif
}

bool Bento::isRunning(){return !shouldClose;}
/*

#ifdef IOS
glm::vec2 Bento::getTouchPos(int index){return touches[index].position;}
bool Bento::getTouch(int index){return touches[index].down;}
bool Bento::getTouchDown(int index){return (touches[index].down && !prevTouches[index].down);}
bool Bento::getTouchUp(int index){return (!touches[index].down && prevTouches[index].down);}
#endif
*/

