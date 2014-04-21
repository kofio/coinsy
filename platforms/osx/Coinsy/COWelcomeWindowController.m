//
//  COWelcomeWindowController.m
//  Coinsy
//
//  Created by Adudalesdi Ganiladisdi on 3/14/14.
//  Copyright (c) 2014 Adudalesdi Ganiladisdi. All rights reserved.
//

#import "NSString+Random.h"

#import "COWelcomeWindowController.h"

@interface COWelcomeWindowController ()
@property (assign) IBOutlet NSTextField * textFieldWelcome;
@property (assign) IBOutlet NSTextField * textFieldCheck;
@property (assign) IBOutlet NSProgressIndicator * progressIndicator1;
@property (assign) IBOutlet NSProgressIndicator * progressIndicator2;
@property (strong) GRProAlert * alert;
@property BOOL signedIn;
@end

@implementation COWelcomeWindowController

- (id)init
{
    self = [super initWithWindowNibName:@"WelcomeWindow" owner:self];
    
    if (self)
    {
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(didConnectNotification:)
            name:kCODidConnectNotification object:nil
        ];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(didSignInNotification:)
            name:kCODidSignInNotification object:nil
        ];
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];

    [self.progressIndicator1 startAnimation:nil];
}

#pragma mark -

- (IBAction)actionSignIn:(id)sender
{
#if 0
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"username"];
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"password"];
    [[NSUserDefaults standardUserDefaults] synchronize];
#endif
    /**
     * :TODO: Store the password in the keychain.
     */
    NSString * username = [[NSUserDefaults standardUserDefaults]
        objectForKey:@"username"
    ];
    NSString * password = [[NSUserDefaults standardUserDefaults]
        objectForKey:@"password"
    ];
    
    if (username.length < 8 || password.length < 8)
    {
        username = [NSString randomizedStringWithAlphabet:
            [NSString defaultAlphabet] length:8
        ];
        
        password = [NSString randomizedStringWithAlphabet:
            [NSString defaultAlphabet] length:8
        ];
        
        [[NSUserDefaults standardUserDefaults] setObject:username
            forKey:@"username"
        ];
        [[NSUserDefaults standardUserDefaults] setObject:password
            forKey:@"password"
        ];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
    
    NSLog(@"%@:%@", username, password);
    
    [[COStack sharedInstance] signIn:username password:password];
}

- (IBAction)actionSignOut:(id)sender
{
    self.signedIn = NO;
    
    [[COStack sharedInstance] signOut];
}

#pragma mark -

- (void)didConnectNotification:(NSNotification *)aNotification
{
    if (self.signedIn == NO)
    {
        self.textFieldWelcome.stringValue = NSLocalizedString(
            @"Please wait while Coinsy generates credentials...",
            nil
        );
        [self.progressIndicator1 stopAnimation:nil];
        [self.progressIndicator2 startAnimation:nil];
        
        [self performSelector:@selector(actionSignIn:) withObject:nil
            afterDelay:0.0f
        ];
    }
}

- (void)didSignInNotification:(NSNotification *)aNotification
{
    NSDictionary * dict  = aNotification.object;
    
    if ([[dict objectForKey:@"status"] intValue] == 0)
    {
        self.signedIn = YES;
        
        [self.progressIndicator2 stopAnimation:nil];
        [self.textFieldCheck setHidden:NO];
        
        NSString * username = [[NSUserDefaults standardUserDefaults]
            objectForKey:@"username"
        ];
    
        self.textFieldWelcome.stringValue = [NSString stringWithFormat:
            NSLocalizedString(
            @"%@, you are all set!", nil), username
        ];
        [self performSelector:@selector(close) withObject:nil afterDelay:1.5f];
    }
    else
    {
        NSLog(@"%@", dict);
        
        self.alert = [[GRProAlert alloc] init];
        self.alert.title = NSLocalizedString(@"Opps", nil);
        self.alert.informativeText = NSLocalizedString(
            @"Something went wrong. Please restart the application.", nil
        );
        
        [self.alert beginSheetModalForWindow:
            self.window completionHandler:^(NSUInteger result)
        {
            exit(0);
        }];
    }
}

#pragma mark -

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
