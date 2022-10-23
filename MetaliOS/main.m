//
//  main.m
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 6/26/22.
//
#import <TargetConditionals.h>

#if defined(TARGET_IOS) || defined(TARGET_TVOS)
#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#else
#import <Cocoa/Cocoa.h>
#endif

//int main(int argc, char * argv[]) {
//    NSString * appDelegateClassName;
//    @autoreleasepool {
//        // Setup code that might create autoreleased objects goes here.
//        appDelegateClassName = NSStringFromClass([AppDelegate class]);
//    }
//    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
//}

#if defined(TARGET_IOS) || defined(TARGET_TVOS)

int main(int argc, char * argv[]) {

#if TARGET_OS_SIMULATOR && (!defined(__IPHONE_13_0) ||  !defined(__TVOS_13_0))
#error No simulator support for Metal API for this SDK version.  Must build for a device
#endif

    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

#elif defined(TARGET_MACOS)

int main(int argc, const char * argv[]) {
    return NSApplicationMain(argc, argv);
}

#endif
