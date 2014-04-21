//
//  COMarketViewController.m
//  Coinsy
//
//  Created by Adudalesdi Ganiladisdi on 3/15/14.
//  Copyright (c) 2014 Adudalesdi Ganiladisdi. All rights reserved.
//

#import "COMarketViewController.h"

@interface COMarketViewController ()
@property (assign) IBOutlet NSTextField * textFieldLeftBalance;
@property (assign) IBOutlet NSTextField * textFieldRightBalance;
@property (assign) IBOutlet NSTextField * textFieldLowestAsk;
@property (assign) IBOutlet NSTextField * textFieldHighestBid;
@property (assign) IBOutlet NSTextField * textFieldAskPrice;
@property (assign) IBOutlet NSTextField * textFieldAskQuantity;
@property (assign) IBOutlet NSTextField * textFieldBidPrice;
@property (assign) IBOutlet NSTextField * textFieldBidQuantity;
@property (assign) IBOutlet NSTableView * tableViewPublicAsks;
@property (assign) IBOutlet NSTableView * tableViewPublicBids;
@property (assign) IBOutlet NSTableView * tableViewOrders;
@property (assign) IBOutlet NSTableView * tableViewTrolls;
@property (assign) IBOutlet NSTextField * textFieldTroll;
@property (strong) NSMutableArray * arrayPublicAsks;
@property (strong) NSMutableArray * arrayPublicBids;
@property (strong) NSMutableArray * arrayOpenOrders;
@property (strong) NSMutableArray * arrayTrolls;
@end

@implementation COMarketViewController

- (id)initWithMarket:(NSString *)aMarket;
{
    self = [super initWithNibName:@"MarketViewController" bundle:nil];
    
    if (self)
    {
        self.market = aMarket;
        self.arrayPublicAsks = [NSMutableArray new];
        self.arrayPublicBids = [NSMutableArray new];
        self.arrayOpenOrders = [NSMutableArray new];
        self.arrayTrolls = [NSMutableArray new];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(marketDidUpdatePublicAsksNotification:)
            name:kCOMarketDidUpdatePublicAsksNotification object:nil
        ];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(marketDidUpdatePublicBidsNotification:)
            name:kCOMarketDidUpdatePublicBidsNotification object:nil
        ];

        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(askStateDidChangeNotification:)
            name:kCOAskStateDidChangeNotification object:nil
        ];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(bidStateDidChangeNotification:)
            name:kCOBidStateDidChangeNotification object:nil
        ];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(didUpdateBalanceNotification:)
            name:kCODidUpdateBalanceNotification object:nil
        ];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(didUpdateTrollboxNotification:)
            name:kCOTrollboxDidUpdateNotification object:nil
        ];
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    COMarketViewController * marketViewController = [[[self class]
        allocWithZone:zone] init
    ];
    
    marketViewController.market = self.market;
    
    return marketViewController;
}

- (IBAction)actionAsk:(id)sender
{
    NSMutableDictionary * ask = [NSMutableDictionary new];
    
    NSNumber * price = [NSNumber numberWithFloat:
        self.textFieldAskPrice.floatValue
    ];
    NSNumber * quantity = [NSNumber numberWithFloat:
        self.textFieldAskQuantity.floatValue
    ];
    
    if (price && quantity)
    {
        [ask setObject:@"ask" forKey:@"type"];
        [ask setObject:price forKey:@"__p"];
        [ask setObject:quantity forKey:@"__q"];
        
        NSNumber * orderId = [[COStack sharedInstance] market:
            self.market ask:ask
        ];
        
        if (orderId.unsignedIntValue > 0)
        {
            [ask setObject:orderId forKey:@"id"];
            
            [self.arrayOpenOrders addObject:ask];
            
            [self.tableViewOrders reloadData];
        }
    }
}

- (IBAction)actionBid:(id)sender
{
    NSMutableDictionary * bid = [NSMutableDictionary new];
    
    NSNumber * price = [NSNumber numberWithFloat:
        self.textFieldBidPrice.floatValue
    ];
    NSNumber * quantity = [NSNumber numberWithFloat:
        self.textFieldBidQuantity.floatValue
    ];
    
    if (price && quantity)
    {
        [bid setObject:@"bid" forKey:@"type"];
        [bid setObject:price forKey:@"__p"];
        [bid setObject:quantity forKey:@"__q"];
        
        NSNumber * orderId = [[COStack sharedInstance] market:
            self.market bid:bid
        ];
        
        if (orderId.unsignedIntValue > 0)
        {
            [bid setObject:orderId forKey:@"id"];
            
            [self.arrayOpenOrders addObject:bid];
            
            [self.tableViewOrders reloadData];
        }
    }
}

- (IBAction)actionCancel:(id)sender
{
    NSInteger rowIndex = [self.tableViewOrders clickedRow];
    
    if (rowIndex >= 0)
    {
        NSDictionary * order = [self.arrayOpenOrders objectAtIndex:rowIndex];
        
        NSNumber * orderId = [order objectForKey:@"id"];
        
        [[COStack sharedInstance] market:self.market cancel:orderId];
        
        [self.arrayOpenOrders removeObject:order];
        [self.tableViewOrders reloadData];
    }
}

#pragma mark -

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (tableView == self.tableViewPublicAsks)
    {
        return self.arrayPublicAsks.count;
    }
    else if (tableView == self.tableViewPublicBids)
    {
        return self.arrayPublicBids.count;
    }
    else if (tableView == self.tableViewOrders)
    {
        return self.arrayOpenOrders.count;
    }
    else if (tableView == self.tableViewTrolls)
    {
        return self.arrayTrolls.count;
    }
    
    return 0;
}

- (id)tableView:(NSTableView *)tableView
    objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if (tableView == self.tableViewPublicAsks)
    {
        NSDictionary * ask = [self.arrayPublicAsks objectAtIndex:row];

        switch (tableColumn.identifier.intValue)
        {
            case 0:
                return [ask objectForKey:@"__p"];
            break;
            case 1:
                return [ask objectForKey:@"__q"];
            break;
            default:
            break;
        }
    }
    else if (tableView == self.tableViewPublicBids)
    {
        NSDictionary * ask = [self.arrayPublicBids objectAtIndex:row];

        switch (tableColumn.identifier.intValue)
        {
            case 0:
                return [ask objectForKey:@"__p"];
            break;
            case 1:
                return [ask objectForKey:@"__q"];
            break;
            default:
            break;
        }
    }
    else if (tableView == self.tableViewOrders)
    {
        NSDictionary * order = [self.arrayOpenOrders objectAtIndex:row];

        switch (tableColumn.identifier.intValue)
        {
            case 0:
            {
                NSString * state = nil;
                
                if ([order objectForKey:@"state"])
                {
                    switch ([[order objectForKey:@"state"] unsignedLongValue])
                    {
                        case 0:
                        {
                            state = NSLocalizedString(@"Pending", nil);
                        }
                        break;
                        case 1:
                        {
                            state = NSLocalizedString(@"Pending", nil);
                        }
                        break;
                        default:
                        {
                            state = NSLocalizedString(@"Closed", nil);
                        }
                        break;
                    }
                }
                else
                {
                    state = NSLocalizedString(@"Open", nil);
                }
                
                return [NSString
                    stringWithFormat:@"%@ (%@)", [order objectForKey:@"type"],
                    state
                ].capitalizedString;
            }
            break;
            case 1:
                return [order objectForKey:@"__p"];
            break;
            case 2:
                return [order objectForKey:@"__q"];
            break;
            default:
            break;
        }
    }
    else if (tableView == self.tableViewTrolls)
    {
        NSDictionary * troll = [self.arrayTrolls objectAtIndex:row];
        
        return [troll objectForKey:@"m"];
    }
    
    return nil;
}

#pragma mark -

- (BOOL)control:(NSControl *)aControl textView:(NSTextView *)aTextView 
    doCommandBySelector:(SEL)commandSelector
{
    if (aControl == self.textFieldTroll)
    {
        if (commandSelector == @selector(insertNewline:))
        {
            NSString * message = self.textFieldTroll.stringValue;
            
            if (message.length > 0)
            {
                self.textFieldTroll.stringValue = @"";
                
                [[COStack sharedInstance] market:self.market troll:message];
                
                return YES;
            }
        }
    }
    
    return NO;
}

#pragma mark - NSSplitView Delegate

- (CGFloat)splitView:(NSSplitView *)splitView
    constrainSplitPosition:(CGFloat)proposedPosition
    ofSubviewAt:(NSInteger)dividerIndex
{
    return self.view.frame.size.width - 180.0f;
}

//- (CGFloat)splitView:(NSSplitView *)splitView
//    constrainMinCoordinate:(CGFloat)proposedMinimumPosition
//    ofSubviewAt:(NSInteger)dividerIndex
//{
//    if (dividerIndex == 0)
//    {
//        return 120.0f;
//    }
//    else if (dividerIndex == 1)
//    {
//        return 120.0f;
//    }
//    
//    return proposedMinimumPosition;
//}
//
//- (CGFloat)splitView:(NSSplitView *)splitView
//    constrainMaxCoordinate:(CGFloat)proposedMaximumPosition
//    ofSubviewAt:(NSInteger)dividerIndex
//{
//    if (dividerIndex == 0)
//    {
//        return proposedMaximumPosition;
//    }
//    else if (dividerIndex == 1)
//    {
//        return proposedMaximumPosition - 120.0f;
//    }
//    
//    return proposedMaximumPosition;
//}
//
//- (void)splitView:(NSSplitView *)sender
//    resizeSubviewsWithOldSize:(NSSize)oldSize
//{
//    CGFloat dividerThickness = sender.dividerThickness;
//    NSRect leftRect  = [[sender.subviews objectAtIndex:0] frame];
//    NSRect rightRect = [[sender.subviews objectAtIndex:1] frame];
//    NSRect newFrame  = sender.frame;
//
//    leftRect.size.height = newFrame.size.height;
//    leftRect.origin = NSMakePoint(0, 0);
//    
//    rightRect.size.width =
//        newFrame.size.width - leftRect.size.width - dividerThickness
//    ;
//    rightRect.size.height = newFrame.size.height;
//    rightRect.origin.x = leftRect.size.width + dividerThickness;
//
//    [[[sender subviews] objectAtIndex:0] setFrame:leftRect];
//    [[[sender subviews] objectAtIndex:1] setFrame:rightRect];
//}

#pragma mark -

- (void)marketDidUpdatePublicAsksNotification:(NSNotification *)aNotification
{
    NSDictionary * dict = aNotification.object;
    
    NSString * market = [dict objectForKey:@"market"];
    
    if ([market isEqualToString:self.market])
    {
        NSNumber * lowestPrice = [dict objectForKey:@"lowest_price"];
        
        self.textFieldLowestAsk.stringValue = [NSString stringWithFormat:
            NSLocalizedString(@"Ask: %@", nil), lowestPrice
        ];
        
        self.arrayPublicAsks = [[dict objectForKey:@"asks"] mutableCopy];
        
        [self.tableViewPublicAsks reloadData];
    }
}

- (void)marketDidUpdatePublicBidsNotification:(NSNotification *)aNotification
{
    NSDictionary * dict = aNotification.object;
    
    NSString * market = [dict objectForKey:@"market"];
    
    if ([market isEqualToString:self.market])
    {
        NSNumber * highestPrice = [dict objectForKey:@"highest_price"];
        
        self.textFieldHighestBid.stringValue = [NSString stringWithFormat:
            NSLocalizedString(@"Bid: %@", nil), highestPrice
        ];
        
        self.arrayPublicBids = [[dict objectForKey:@"bids"] mutableCopy];
        
        [self.tableViewPublicBids reloadData];
    }
}

- (void)askStateDidChangeNotification:(NSNotification *)aNotification
{
    NSDictionary * dict = aNotification.object;
    
    NSString * market = [dict objectForKey:@"market"];
    
    if ([market isEqualToString:self.market])
    {
        NSNumber * identifier = [dict objectForKey:@"id"];
        NSNumber * state = [dict objectForKey:@"state"];

        if (state.intValue == 0 || state.intValue == 1)
        {
            NSUInteger index = 0;
            
            for (NSDictionary * order in self.arrayOpenOrders)
            {
                NSNumber * orderId = [order objectForKey:@"id"];
                
                if ([identifier isEqualToNumber:orderId])
                {
                    NSMutableDictionary * orderCopy = order.mutableCopy;
                    
                    [orderCopy setObject:state forKey:@"state"];
                    
                    [self.arrayOpenOrders replaceObjectAtIndex:index
                        withObject:orderCopy
                    ];
                    
                    [self.tableViewOrders reloadData];
            
                    break;
                }
                
                index++;
            }
        }
        else if (state.intValue == 2)
        {
            if (self.arrayOpenOrders.count > 0)
            {
                NSUInteger index = 0;

                for (NSDictionary * order in self.arrayOpenOrders)
                {
                    NSNumber * orderId = [order objectForKey:@"id"];
                    
                    if ([identifier isEqualToNumber:orderId])
                    {
                        break;
                    }
                    else
                    {
                        index++;
                    }
                }
                
                [self.arrayOpenOrders removeObjectAtIndex:index];
                [self.tableViewOrders reloadData];
            }
        }
    }
}

- (void)bidStateDidChangeNotification:(NSNotification *)aNotification
{
    NSDictionary * dict = aNotification.object;
    
    NSString * market = [dict objectForKey:@"market"];
    
    if ([market isEqualToString:self.market])
    {
        NSNumber * identifier = [dict objectForKey:@"id"];
        NSNumber * state = [dict objectForKey:@"state"];
        
        if (state.intValue == 0 || state.intValue == 1)
        {
            NSUInteger index = 0;
            
            for (NSDictionary * order in self.arrayOpenOrders)
            {
                NSNumber * orderId = [order objectForKey:@"id"];
                
                if (
                    [identifier isEqualToNumber:orderId]
                    )
                {
                    NSMutableDictionary * orderCopy = order.mutableCopy;
                    
                    [orderCopy setObject:state forKey:@"state"];
                    
                    [self.arrayOpenOrders replaceObjectAtIndex:index
                        withObject:orderCopy
                    ];
                    
                    [self.tableViewOrders reloadData];
            
                    break;
                }
                
                index++;
            }
        }
        else if (state.intValue == 2)
        {
            if (self.arrayOpenOrders.count > 0)
            {
                NSUInteger index = 0;

                for (NSDictionary * order in self.arrayOpenOrders)
                {
                    NSNumber * orderId = [order objectForKey:@"id"];
                    
                    if ([identifier isEqualToNumber:orderId])
                    {
                        break;
                    }
                    else
                    {
                        index++;
                    }
                }
                
                [self.arrayOpenOrders removeObjectAtIndex:index];
                [self.tableViewOrders reloadData];
            }
        }
    }
}

- (void)didUpdateBalanceNotification:(NSNotification *)aNotification
{
    NSDictionary * dict = aNotification.object;

    NSString * symbol = [dict objectForKey:@"symbol"];
    NSNumber * balance = [dict objectForKey:@"balance"];

    NSLog(@"symbol = %@, balance = %@", symbol, balance);
    
    NSArray * symbols = [self.market componentsSeparatedByString:@"/"];
    
    if ([symbol isEqualToString:symbols[1]])
    {
        self.textFieldLeftBalance.stringValue = [NSString
            stringWithFormat:NSLocalizedString(@"%@: %@", nil),
            symbol.uppercaseString, balance
        ];
    }
    else if ([symbol isEqualToString:symbols[0]])
    {
        self.textFieldRightBalance.stringValue = [NSString
            stringWithFormat:NSLocalizedString(@"%@: %@", nil),
            symbol.uppercaseString, balance
        ];
    }
}

- (void)didUpdateTrollboxNotification:(NSNotification *)aNotification
{
    NSDictionary * dict = aNotification.object;

    NSString * market = [dict objectForKey:@"market"];
    
    if ([market isEqualToString:self.market])
    {
        NSArray * trolls = [dict objectForKey:@"trolls"];
        
        [self.arrayTrolls removeAllObjects];
        [self.arrayTrolls addObjectsFromArray:trolls];
        [self.tableViewTrolls reloadData];

        NSInteger numberOfRows = [self.tableViewTrolls numberOfRows];

        if (numberOfRows > 0)
        {
            [self.tableViewTrolls scrollRowToVisible:numberOfRows - 1];
        }
    }
}

#pragma mark -

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
