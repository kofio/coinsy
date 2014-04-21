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

#import "COMessageWindowController.h"

@interface COMessageWindowController ()
@property (assign) IBOutlet NSTableView * tableView;
@property (assign) IBOutlet NSTextField * textFieldInput;
@property (strong) NSMutableArray * messages;
@end

@implementation COMessageWindowController

- (id)initWithUsername:(NSString *)aUsername
{
    self = [super initWithWindowNibName:@"MessageWindow" owner:self];
    
    if (self)
    {
        self.username = aUsername;
        self.messages = [NSMutableArray new];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(didReceiveChatMessageNotification:)
            name:kCODidReceiveChatMessageNotification object:nil
        ];
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

#pragma mark -

- (IBAction)actionSend:(id)sender
{
    NSString * message = self.textFieldInput.stringValue;
    
    if (message.length > 0)
    {
        NSString * messageId = [[COStack sharedInstance] message:message
            username:self.username
        ];
        
        NSLog(@"Sent message %@.", messageId);
        
        self.textFieldInput.stringValue = @"";
        
        NSString * username = [[NSUserDefaults standardUserDefaults]
            objectForKey:@"username"
        ];
    
        [self.messages addObject:
            @{@"id" : messageId, @"message_attribute" : message,
            @"to" : self.username, @"from" : username}
        ];
        
        [self.tableView reloadData];
    }
}

#pragma mark -

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return self.messages.count;
}

- (id)tableView:(NSTableView *)tableView
    objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSDictionary * message = [self.messages objectAtIndex:row];
    
    return [NSString stringWithFormat:@"%@: %@",
        [message objectForKey:@"from"],
        [message objectForKey:@"message_attribute"]
    ];

}

#pragma mark -

- (BOOL)control:(NSControl *)aControl textView:(NSTextView *)aTextView 
    doCommandBySelector:(SEL)commandSelector
{
    if (commandSelector == @selector(insertNewline:))
    {
        [self actionSend:nil];
        
        return YES;
    }
    
    return NO;
}

#pragma mark -

- (void)didReceiveChatMessageNotification:(NSNotification *)aNotification
{
    NSDictionary * message = aNotification.object;
    
    NSString * from = [message objectForKey:@"from"];
    
    if ([from isEqualToString:self.username])
    {
        [self.messages addObject:message];
        
        [self.tableView reloadData];
    }
}

#pragma mark -

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
