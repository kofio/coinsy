/**
 -----BEGIN PGP MESSAGE-----
Comment: GPGTools - https://gpgtools.org

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

#import <objc/objc-runtime.h>

#import "COMainWindowController.h"
#import "COMarketViewController.h"
#import "COMessageWindowController.h"
#import "COTradesTableView.h"

@interface COMainWindowController ()
@property (assign) IBOutlet NSWindow * windowMarketAdd;
@property (assign) IBOutlet NSTextField * textFieldMarketAddLeftSymbol;
@property (assign) IBOutlet NSTextField * textFieldMarketAddRightSymbol;
@property (assign) IBOutlet NSTableView * tableViewMarkets;
@property (assign) IBOutlet COTradesTableView * tableViewTrades;
@property (assign) IBOutlet NSView * viewMarket;
@property (assign) IBOutlet NSWindow * windowTrade;
@property (assign) IBOutlet NSPopover * popoverTrade;
@property (assign) IBOutlet NSTextField * textFieldTrade0;
@property (assign) IBOutlet NSTextField * textFieldTrade1;
@property (assign) IBOutlet NSTextField * textFieldTrade2;
@property (assign) IBOutlet NSTextField * textFieldTrade3;
@property (assign) IBOutlet NSTextField * textFieldTrade4;
@property (assign) IBOutlet NSTextField * textFieldTrade5;
@property (strong) NSMutableArray * markets;
@property (strong) NSMutableArray * trades;
@end

/**
 * This needs to be a class but I am under extreme pressure that is out of
 * my control.
 */
static NSMutableDictionary * gMessageWindowControllers = nil;

@implementation COMainWindowController

- (id)init
{
    self = [super initWithWindowNibName:@"MainWindow" owner:self];
    
    if (self)
    {
        self.markets = [NSMutableArray new];
        self.trades = [NSMutableArray new];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(tradeDidUpdateNotification:)
            name:kCOTradeDidUpdateNotification object:nil
        ];
        
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
    
    self.tableViewTrades.delegateTrades = self;
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

#pragma mark -

- (IBAction)actionShowMarketAdd:(id)sender
{
    [self.window beginSheet:self.windowMarketAdd completionHandler:nil];
}

- (IBAction)actionMarketAdd:(id)sender
{
    NSString * leftSymbol = self.textFieldMarketAddLeftSymbol.stringValue;
    NSString * rightSymbol = self.textFieldMarketAddRightSymbol.stringValue;
    
    if (leftSymbol.length > 0 && leftSymbol.length > 0)
    {
        NSString * market = [NSString stringWithFormat:
            @"%@/%@", leftSymbol.uppercaseString, rightSymbol.uppercaseString
        ];
        
        if ([[COStack sharedInstance] marketAdd:market])
        {
            COMarketViewController * marketViewController = [
                [COMarketViewController alloc] initWithMarket:market
            ];
            
            [self.viewMarket addSubview:marketViewController.view];
            
            marketViewController.view.frame = self.viewMarket.bounds;
            
            [self.markets addObject:marketViewController];
            
            [self.tableViewMarkets reloadData];
        }
        
        [self.window endSheet:self.windowMarketAdd];
    }
    else
    {
        NSBeep();
    }
}

- (IBAction)actionMarketCancel:(id)sender
{
    [self.window endSheet:self.windowMarketAdd];
}

- (IBAction)actionSendMessage:(id)sender
{
    NSDictionary * info = objc_getAssociatedObject(
        self.popoverTrade, "info"
    );
    
    NSString * username =
        [info objectForKey:@"seller"] ? [info objectForKey:@"seller"] :
        [info objectForKey:@"buyer"]
    ;
    
    NSLog(@"username = %@", username);
    
    if (gMessageWindowControllers == nil)
    {
        gMessageWindowControllers = [NSMutableDictionary new];
    }
    
    COMessageWindowController * messageWindowController = [
        gMessageWindowControllers objectForKey:username
    ];
    
    if (messageWindowController == nil)
    {
        messageWindowController = [[
            COMessageWindowController alloc] initWithUsername:username
        ];
        
        [gMessageWindowControllers setObject:messageWindowController
            forKey:username
        ];
    }
    
    [messageWindowController.window makeKeyAndOrderFront:nil];
}

#pragma mark -

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (self.tableViewMarkets == tableView)
    {
        return self.markets.count;
    }
    else if (self.tableViewTrades == tableView)
    {
        return self.trades.count;
    }
    
    return 0;
}

- (id)tableView:(NSTableView *)tableView
    objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if (self.tableViewMarkets == tableView)
    {
        COMarketViewController * marketViewController =
            [self.markets objectAtIndex:row]
        ;

        return marketViewController.market;
    }
    else if (self.tableViewTrades == tableView)
    {
        NSDictionary * trade = [self.trades objectAtIndex:row];
        
        NSDictionary * info = [trade objectForKey:@"info"];
        
        NSString * timestamp = [info objectForKey:@"timestamp"];
        
        NSDate * epoch = [[NSDate alloc]
            initWithTimeIntervalSince1970:timestamp.doubleValue
        ];
        
        NSDateFormatter * dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateStyle:NSDateFormatterMediumStyle];
        [dateFormatter setTimeStyle:NSDateFormatterMediumStyle];
        
        return [dateFormatter stringFromDate:epoch];

    }

    return nil;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 22.0f;
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    NSInteger rowIndex = -1;
    
    if (self.tableViewMarkets == aNotification.object)
    {
        rowIndex = [self.tableViewMarkets selectedRow];
        
        COMarketViewController * marketViewController = [self.markets
            objectAtIndex:rowIndex
        ];
        
        [self.viewMarket.subviews[0] removeFromSuperview];
        
        marketViewController.view.frame = self.viewMarket.bounds;
        
        [self.viewMarket addSubview:marketViewController.view];
    }
    else if (self.tableViewTrades == aNotification.object)
    {
        rowIndex = [self.tableViewTrades selectedRow];
    }
    
}

#pragma mark -

- (void)tableView:(COTradesTableView *)tableView didClickedRow:(NSInteger)row
{
    [self.popoverTrade close];

    CGRect rect = [tableView rectOfRow:row];
    
    NSDictionary * trade = [self.trades objectAtIndex:row];
    
    NSDictionary * info = [trade objectForKey:@"info"];

    NSArray * marketSymbols = [[info objectForKey:@"market"]
        componentsSeparatedByString:@"/"
    ];
    
    NSParameterAssert(marketSymbols.count == 2);
    
    self.textFieldTrade0.stringValue = [NSString
        stringWithFormat:NSLocalizedString(@"Market: %@", nil),
        [info objectForKey:@"market"]
    ];
    
    if ([info objectForKey:@"buyer"])
    {
        self.textFieldTrade1.stringValue = [NSString
            stringWithFormat:NSLocalizedString(@"Buyer: %@", nil),
            [info objectForKey:@"buyer"]
        ];
        
        NSNumber * total = [NSNumber numberWithFloat:
            [[info objectForKey:@"price"] floatValue] *
            [[info objectForKey:@"quantity"] floatValue]
        ];
        
        self.textFieldTrade5.stringValue = [NSString
            stringWithFormat:NSLocalizedString(
            @"Notes: You owe %@ %@ %@. %@ owes you %@ %@.", nil),
            [info objectForKey:@"buyer"], [info objectForKey:@"quantity"],
            marketSymbols[0], [info objectForKey:@"buyer"],
            total, marketSymbols[1]
        ];
    }
    else if ([info objectForKey:@"seller"])
    {
        self.textFieldTrade1.stringValue = [NSString
            stringWithFormat:NSLocalizedString(@"Seller: %@", nil),
            [info objectForKey:@"seller"]
        ];
        
        NSNumber * total = [NSNumber numberWithFloat:
            [[info objectForKey:@"price"] floatValue] *
            [[info objectForKey:@"quantity"] floatValue]
        ];
        
        self.textFieldTrade5.stringValue = [NSString
            stringWithFormat:NSLocalizedString(
            @"Notes: You owe %@ %@ %@. %@ owes you %@ %@.", nil),
            [info objectForKey:@"seller"], total,
            marketSymbols[1], [info objectForKey:@"seller"],
            [info objectForKey:@"quantity"], marketSymbols[0]
        ];
    }
    else
    {
        assert(0);
    }
    
    self.textFieldTrade2.stringValue = [NSString
        stringWithFormat:NSLocalizedString(@"Price: %@", nil),
        [info objectForKey:@"price"]
    ];
    self.textFieldTrade3.stringValue = [NSString
        stringWithFormat:NSLocalizedString(@"Quantity: %@", nil),
        [info objectForKey:@"quantity"]
    ];
    
    self.textFieldTrade4.stringValue = [NSString
        stringWithFormat:NSLocalizedString(@"State: %@", nil),
        [[info objectForKey:@"state"] capitalizedString]
    ];
    
    objc_setAssociatedObject(
        self.popoverTrade, "info", info, OBJC_ASSOCIATION_RETAIN
    );
    
    [self.popoverTrade showRelativeToRect:rect
        ofView:tableView preferredEdge:NSMaxXEdge
    ];
}

#pragma mark - NSPopoverDelegate

- (NSWindow *)detachableWindowForPopover:(NSPopover *)popover
{
    #warning :FIXME
    return self.windowTrade;
}

#pragma mark - NSSplitView Delegate

- (CGFloat)splitView:(NSSplitView *)splitView
    constrainMinCoordinate:(CGFloat)proposedMinimumPosition
    ofSubviewAt:(NSInteger)dividerIndex
{
    return 120;
}

- (CGFloat)splitView:(NSSplitView *)splitView
    constrainMaxCoordinate:(CGFloat)proposedMaximumPosition
    ofSubviewAt:(NSInteger)dividerIndex
{
    return 180;
}

- (void)splitView:(NSSplitView *)sender
    resizeSubviewsWithOldSize:(NSSize)oldSize
{
    CGFloat dividerThickness = sender.dividerThickness;
    NSRect leftRect  = [[sender.subviews objectAtIndex:0] frame];
    NSRect rightRect = [[sender.subviews objectAtIndex:1] frame];
    NSRect newFrame  = sender.frame;

    leftRect.size.height = newFrame.size.height;
    leftRect.origin = NSMakePoint(0, 0);
    
    rightRect.size.width =
        newFrame.size.width - leftRect.size.width - dividerThickness
    ;
    rightRect.size.height = newFrame.size.height;
    rightRect.origin.x = leftRect.size.width + dividerThickness;

    [[[sender subviews] objectAtIndex:0] setFrame:leftRect];
    [[[sender subviews] objectAtIndex:1] setFrame:rightRect];
}

#pragma mark -

- (void)tradeDidUpdateNotification:(NSNotification *)aNotification
{
    NSDictionary * dict = aNotification.object;
    
    NSNumber * tid = [dict objectForKey:@"tid"];
    
    NSDictionary * trade = dict.copy;
    
    BOOL found = NO;
    
    NSDictionary * trade2 = nil;
    
    for (trade2 in self.trades)
    {
        NSString * buyer = [[trade objectForKey:@"info"] objectForKey:@"buyer"];
        NSString * seller = [[trade objectForKey:@"info"] objectForKey:@"seller"];
        
        if ([tid isEqualToNumber:[trade2 objectForKey:@"tid"]])
        {
            if (
                buyer && [buyer isEqualToString:
                [[trade2 objectForKey:@"info"] objectForKey:@"buyer"]]
                )
            {
                found = YES;
                
                break;
            }
            else if (
                seller && [seller isEqualToString:
                [[trade2 objectForKey:@"info"] objectForKey:@"seller"]]
                )
            {
            
                found = YES;
                
                break;
            }
        }
    }
    
    if  (found == NO)
    {
        [self.trades addObject:trade];
    }
    else
    {
        [self.trades replaceObjectAtIndex:
            [self.trades indexOfObject:trade2] withObject:trade
        ];
    }
    
    [self.tableViewTrades reloadData];
}

#pragma mark -

- (void)didReceiveChatMessageNotification:(NSNotification *)aNotification
{
    NSDictionary * message = aNotification.object;
    
    NSString * to = [message objectForKey:@"to"];
    
    if (
        [to isEqualToString:[[NSUserDefaults standardUserDefaults]
        objectForKey:@"username"]]
    )
    {
        NSString * from = [message objectForKey:@"from"];
        
        NSLog(@"from = %@", from);
        
        COMessageWindowController * messageWindowController = [
            gMessageWindowControllers objectForKey:from
        ];
        
        if (messageWindowController == nil)
        {
            messageWindowController = [[
                COMessageWindowController alloc] initWithUsername:from
            ];
            
            [gMessageWindowControllers setObject:messageWindowController
                forKey:from
            ];
        }
        
        [messageWindowController.window makeKeyAndOrderFront:nil];
    }
}

#pragma mark -

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
