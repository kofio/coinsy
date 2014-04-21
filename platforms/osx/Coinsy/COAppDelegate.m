/**
 -----BEGIN PGP MESSAGE-----
hQIMA8Hhi7HOnyjIAQ//dmGnnsJBxqGuX8U1iJ/JxEj2IMKUHs48ceYMOsb4bEpO
aq7cCcGHgEgBUJnlIhBDYE58CRfQIRs4yJLfuRGLPgYj0cu2i9hP3ZWnCyM3pb8I
V54CS4FlPw8nksz04DGPI3L1y73sFdNkNY3VjrFATGuXNL5915IRcv0AiGHxrJaD
CJn2Brxhnh5hTA+/hZ9g65O2YsWhxj1WIBEwibiOWYilYGuonL6ZWLwPI/oKxG94
bx98gKfoQPMUPglGHh5KGqIBMeNgEQvjHfZ5kL9SW31Iq0uZ+3D8MSZ3w0EvsfCq
qSSp5bLSEmnGX/a56DyrDPxvqqGozbCLK90MOQVTcJgKJQnFlcQDUbFPZ4xU4zcl
GeSNML0mxfmHOloetCbVIIhavkQ5jV7Fd04emo2duATW/4d4tsw1lk1IFYcy9LGk
ricRF/RTh60y5R5RNtB4VkcnnyM3zki7I/FKNADTaSKAmlVsfhiEkJ1W/2hj0wSb
qRiPPLzEV0bK9DN8Cpvfcna3Hv1h0rkAgUD+hBN+VicAdn6UBSdLdDVIcBiF2xe0
r2hJxFza5/XrFdR99Hv7eEgpMYt0ZRI3XiWh+1DKlvFE16LikXP8I4cGu0FDmNi/
pQyNiD5ipzq2K2zQ/XWjuYszpgXtaaOeUCfM41zey0FldQurMKMLovqCncunRa7S
qwGrjluCuJ8d8UfQAFPGBmVJjEFICkGUl5T2mO5+PjVpr4U8/CUsv0G0UJmbslMD
dQYCYEJ5h27LRDGjO4KapkrepOVQuugECEnJXpVrfN/8VziLeeHQm4joQ/vIa1OJ
FBPs5EQ6200jJ66oUdwC4KKsKZqzmaXW0yllw9dQHCK4WhiRinLVXx+Jol1RpjFm
doCvtEuIpZIUraAc2YsitMBTzZnA1CS+IBtGBA==
=a9Uy
-----END PGP MESSAGE-----
 */

#import <QuartzCore/QuartzCore.h>

#import "COAppDelegate.h"
#import "COStack.h"
#import "COWelcomeWindowController.h"
#import "COMainWindowController.h"

@interface COAppDelegate ()
@property (strong) COWelcomeWindowController * welcomeWindowController;
@property (strong) COMainWindowController * mainWindowController;
@end

@implementation COAppDelegate

int rand_int(int upper_bound)
{
    srand((unsigned int)clock());

    return rand () % upper_bound;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    /**
     * Show the welcome window.
     */
    self.welcomeWindowController = [COWelcomeWindowController new];
    self.welcomeWindowController.window.level = NSModalPanelWindowLevel;
    [self.welcomeWindowController showWindow:nil];
    [self.welcomeWindowController.window makeKeyAndOrderFront:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(didConnectNotification:)
        name:kCODidConnectNotification object:nil
    ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(didDisconnectNotification:)
        name:kCODidDisconnectNotification object:nil
    ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(didSignInNotification:)
        name:kCODidSignInNotification object:nil
    ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(onVersionNotification:)
        name:kCOOnVersionNotification object:nil
    ];
    
    NSString * listenPort = [[NSUserDefaults standardUserDefaults]
        objectForKey:kCOPrefsListenPort
    ];
    
    if (!listenPort || listenPort.intValue < 49152)
    {
        uint16_t randomPort = rand_int(65535  - 49152 + 1) + 49152;
        
        listenPort = [NSString stringWithFormat:@"%d", randomPort];
    }
    
    [[NSUserDefaults standardUserDefaults] setObject:listenPort
        forKey:kCOPrefsListenPort
    ];
    
    NSLog(@"Listen port = %@\n", listenPort);
    
    /**
     * Start the stack.
     */
    [[COStack sharedInstance] start:
        [NSNumber numberWithInt:listenPort.intValue]
    ];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
//    [self.window orderOut:nil];
//    
    /**
     * Sign out of the stack.
     */
    [[COStack sharedInstance] signOut];
#if 0
    /**
     * Stop the stack.
     */
    [[COStack sharedInstance] stop];
#endif
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
#if 0
    if (1)
    {
        NSRunAlertPanel(
            NSLocalizedString(@"Are you sure you want to quit?", nil),
            NSLocalizedString(@"Quit", nil), NSLocalizedString(@"Cancel", nil),
            nil, [NSApp mainWindow], self,
            @selector(sheetDidEnd:returnCode:contextInfo:), nil, @"quit",
            NSLocalizedString(@"Staying online greatly benefits the network.", nil)
        );
       
        return NSTerminateLater;
    }
#endif
    return NSTerminateNow;
}

- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode
       contextInfo:(void *)contextInfo
{
    if (contextInfo == @"quit")
    {
        if (returnCode == NSAlertDefaultReturn)
        {
            // Do stuff before quitting.
            
            [NSApp replyToApplicationShouldTerminate:YES];
        }
        else
        {
           [NSApp replyToApplicationShouldTerminate:NO];
        }
    }
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender
    hasVisibleWindows:(BOOL)flag
{
//    if (self.setupWindow != nil)
//    {
//        return NO;
//    }
//    else if ([COStack sharedInstance].username == nil)
//    {
//        return NO;
//    }
//    else
//    {
//        [self.window makeKeyAndOrderFront:self];
//    }
    
    return YES;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return YES;
}

#pragma mark - NSNotification's

- (void)didConnectNotification:(NSNotification *)aNotification
{
//    if ([COStack sharedInstance].username == nil)
//    {
//        NSString * username = [[NSUserDefaults standardUserDefaults]
//            objectForKey:@"username"
//        ];
//        
//        EMGenericKeychainItem * keychainItem = [EMGenericKeychainItem
//            genericKeychainItemForService:@"authService"
//            withUsername:username
//        ];
//        
//        if (username && keychainItem)
//        {
//            NSLog(
//                @"Got password = %@ for username = %@.", [keychainItem password],
//                username
//            );
//            
//            [[FNSignInManager sharedInstance] signInWithUsername:username
//                password:[keychainItem password]
//            ];
//        }
//        else
//        {    
//            // Sign in window.
//            
//            [self performSelector:@selector(showSignInWindow:) withObject:nil];
//        }
//    }
}

- (void)didDisconnectNotification:(NSNotification *)aNotification
{
    // ...
}

- (void)didSignInNotification:(NSNotification *)aNotification
{
    if (self.mainWindowController == nil)
    {
        /**
         * Allocate the main window controller.
         */
        self.mainWindowController = [COMainWindowController new];
    }
    
    [self.mainWindowController.window performSelector:
        @selector(makeKeyAndOrderFront:) withObject:nil afterDelay:1.0f
    ];
}

- (void)onVersionNotification:(NSNotification *)aNotification
{
    NSDictionary * dict = aNotification.object;

    BOOL upgrade = [[dict objectForKey:@"upgrade"] boolValue];
    
    if (upgrade)
    {
        BOOL required = [[dict objectForKey:@"required"] boolValue];
        
        if (required)
        {
            NSAlert * alert = [[NSAlert alloc] init];

            alert.informativeText = NSLocalizedString(
                @"A required update is available.", nil
            );
            alert.messageText = NSLocalizedString(@"Update Required", nil);
            
            [alert runModal];
            
            [[NSWorkspace sharedWorkspace] openURL:
                [NSURL URLWithString:@"http://coinsy.net/?platform=osx"]
            ];
            
            // Exit as we may want this node off the network.
            exit(0);
        }
        else
        {
            NSAlert * alert = [[NSAlert alloc] init];

            alert.informativeText = NSLocalizedString(
                @"An update is available.", nil
            );
            alert.messageText = NSLocalizedString(@"Update", nil);
            
            [alert runModal];
            
            [[NSWorkspace sharedWorkspace] openURL:
                [NSURL URLWithString:@"http://coinsy.net/?platform=osx"]
            ];
        }
    }
}

@end
