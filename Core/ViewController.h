//
//  ViewController.h
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 6/26/22.
//

#if defined(TARGET_IOS) || defined(TARGET_TVOS)
@import UIKit;
#define PlatformViewController UIViewController
#else
@import AppKit;
#define PlatformViewController NSViewController
#endif

@import MetalKit;

@interface ViewController : PlatformViewController

@end

