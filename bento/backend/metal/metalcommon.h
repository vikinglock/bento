#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#ifdef IOS
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif
extern id<MTLDevice> device;
extern id<MTLCommandQueue> commandQueue;