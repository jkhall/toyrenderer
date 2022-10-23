//
//  AppDelegate.m
//  MetalMac
//
//  Created by Jordan Hall on 10/10/22.
//

#import "AppDelegate.h"
#import "MainWindowController.h"
#import "ViewController.h"
@interface AppDelegate ()

// should this be weak...?
@property MainWindowController* mwc;

@end

@implementation AppDelegate

// this is just not working_ copy MetalCPP
- (void) applicationWillFinishLaunching:(NSNotification *)notification {
   
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    _mwc = [[MainWindowController alloc] initWithWindowNibName:@"MainWindowController"];
    ViewController* vc = [[ViewController alloc] init];
    
    NSWindow* newWindow = [NSWindow windowWithContentViewController:vc];
//    newWindow.title = @"Jordan's Renderer";
    
    _mwc.window = newWindow;
//    ViewController* vc = (ViewController *)_mwc.window.contentViewController;
//    vc.device = MTLCreateSystemDefaultDevice();
    _mwc.window.title = @"Jordan's Renderer";
    [_mwc showWindow:self];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


@end
