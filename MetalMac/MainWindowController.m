//
//  MainWindowController.m
//  MetalMac
//
//  Created by Jordan Hall on 10/10/22.
//

#import "MainWindowController.h"
#import "ViewController.h"

@interface MainWindowController ()

@end

@implementation MainWindowController

- (void)windowWillLoad {
    [super windowWillLoad];
    
    NSLog(@"window will load");
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
//    self.window.contentViewController = [[ViewController alloc] init];
}

@end
