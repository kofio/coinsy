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

#include <coinsy/stack.hpp>

#import "NSString+Additions.hpp"

#import "COStack.h"

NSString * kCODidConnectNotification = @"coDidConnectNotification";
NSString * kCODidDisconnectNotification = @"coDidDisconnectNotification";
NSString * kCODidSignInNotification = @"coDidSignInNotification";
NSString * kCODidUpdateBalanceNotification = @"coidUpdateBalanceNotification";
NSString * kCOMarketDidUpdatePublicAsksNotification = @"kCOMarketDidUpdatePublicAsksNotification";
NSString * kCOMarketDidUpdatePublicBidsNotification = @"kCOMarketDidUpdatePublicBidsNotification";
NSString * kCOAskStateDidChangeNotification = @"kCOAskStateDidChangeNotification";
NSString * kCOBidStateDidChangeNotification = @"kCOBidStateDidChangeNotification";
NSString * kCOTradeDidUpdateNotification = @"kCOTradeDidUpdateNotification";
NSString * kCOTrollboxDidUpdateNotification = @"kCOTrollboxDidUpdateNotification";
NSString * kCODidReceiveChatMessageNotification = @"kCODidReceiveChatMessageNotification";

NSString * kCOOnVersionNotification = @"coOnVersionNotification";

class my_coinsy_stack : public coinsy::stack
{
    public:
    
        /**
         * Called when connected to the network.
         * @param addr The address.
         * @param port The port.
         */
        virtual void on_connected(
            const char * addr, const std::uint16_t & port
            )
        {
            @autoreleasepool
            {
                NSMutableDictionary * dict = [NSMutableDictionary dictionary];
                
                dispatch_async(dispatch_get_main_queue(),^
                {
                    [[NSNotificationCenter defaultCenter]
                        postNotificationName:kCODidConnectNotification
                        object:dict
                    ];
                });
            }
        }
    
        /**
         * Called when disconnected from the network.
         * @param addr The address.
         * @param port The port.
         */
        virtual void on_disconnected(
            const char * addr, const std::uint16_t & port
            )
        {
            @autoreleasepool
            {
                NSMutableDictionary * dict = [NSMutableDictionary dictionary];
                
                dispatch_async(dispatch_get_main_queue(),^
                {
                    [[NSNotificationCenter defaultCenter]
                        postNotificationName:kCODidDisconnectNotification
                        object:dict
                    ];
                });
            }
        }
    
        virtual void on_sign_in(const std::string & status)
        {
            @autoreleasepool
            {
                NSMutableDictionary * dict = [NSMutableDictionary dictionary];

                [dict setObject:
                    [NSString stringWithUTF8String:status.c_str()]
                    forKey:@"status"
                ];
                
                dispatch_async(dispatch_get_main_queue(),^
                {
                    [[NSNotificationCenter defaultCenter]
                        postNotificationName:kCODidSignInNotification
                        object:dict
                    ];
                });
            }
        }
    
        virtual void on_balance(const std::string & symbol, const float & balance)
        {
            @autoreleasepool
            {
                NSMutableDictionary * dict = [NSMutableDictionary dictionary];

                [dict setObject:
                    [NSString stringWithUTF8String:symbol.c_str()]
                    forKey:@"symbol"
                ];
                [dict setObject:[NSNumber numberWithFloat:balance]
                    forKey:@"balance"
                ];
                
                dispatch_async(dispatch_get_main_queue(),^
                {
                    [[NSNotificationCenter defaultCenter]
                        postNotificationName:kCODidUpdateBalanceNotification
                        object:dict
                    ];
                });
            }
        }
    
        virtual void on_market_public_asks(
            const std::string & key, const float & lowest_price,
            const std::vector<market_public_ask_t> & public_asks
            )
        {
            NSMutableDictionary * dict = [NSMutableDictionary dictionary];
            
            [dict setObject:
                [NSString stringWithUTF8String:key.c_str()]
                forKey:@"market"
            ];
        
            NSMutableArray * asks = [NSMutableArray new];
            
            for (auto & i : public_asks)
            {
                NSMutableDictionary * ask = [NSMutableDictionary dictionary];
                
                [ask setObject:
                    [NSNumber numberWithFloat:i.price] forKey:@"__p"
                ];
                [ask setObject:
                    [NSNumber numberWithFloat:i.quantity] forKey:@"__q"
                ];
                
                [asks addObject:ask];
            }
            
            [dict setObject:[NSNumber numberWithFloat:lowest_price]
                forKey:@"lowest_price"
            ];
            [dict setObject:asks forKey:@"asks"];
            
            dispatch_async(dispatch_get_main_queue(),^
            {
                [[NSNotificationCenter defaultCenter]
                    postNotificationName:
                    kCOMarketDidUpdatePublicAsksNotification object:dict
                ];
            });
        }
    
        virtual void on_market_public_bids(
            const std::string & key, const float & highest_price,
            const std::vector<market_public_bid_t> & public_bids
            )
        {
            NSMutableDictionary * dict = [NSMutableDictionary dictionary];
            
            [dict setObject:
                [NSString stringWithUTF8String:key.c_str()]
                forKey:@"market"
            ];
        
            NSMutableArray * bids = [NSMutableArray new];
            
            for (auto & i : public_bids)
            {
                NSMutableDictionary * bid = [NSMutableDictionary dictionary];
                
                [bid setObject:
                    [NSNumber numberWithFloat:i.price] forKey:@"__p"
                ];
                [bid setObject:
                    [NSNumber numberWithFloat:i.quantity] forKey:@"__q"
                ];
                
                [bids addObject:bid];
            }
            
            [dict setObject:[NSNumber numberWithFloat:highest_price]
                forKey:@"highest_price"
            ];
            [dict setObject:bids forKey:@"bids"];
            
            dispatch_async(dispatch_get_main_queue(),^
            {
                [[NSNotificationCenter defaultCenter]
                    postNotificationName:
                    kCOMarketDidUpdatePublicBidsNotification object:dict
                ];
            });
        }
    
        virtual void on_ask_state(
            const std::string & key, const std::uint32_t & identifier,
            const std::uint32_t & state
        )
        {
            NSMutableDictionary * dict = [NSMutableDictionary dictionary];
            
            [dict setObject:
                [NSString stringWithUTF8String:key.c_str()]
                forKey:@"market"
            ];
            
            [dict setObject:[NSNumber numberWithUnsignedLong:identifier]
                forKey:@"id"
            ];
            [dict setObject:[NSNumber numberWithUnsignedLong:state]
                forKey:@"state"
            ];
            
            dispatch_async(dispatch_get_main_queue(),^
            {
                [[NSNotificationQueue defaultQueue] enqueueNotification:
                    [NSNotification notificationWithName:
                    kCOAskStateDidChangeNotification object:dict]
                    postingStyle:NSPostASAP
                ];
            });
        }

        virtual void on_bid_state(
            const std::string & key, const std::uint32_t & identifier,
            const std::uint32_t & state
        )
        {
            NSMutableDictionary * dict = [NSMutableDictionary dictionary];
            
            [dict setObject:
                [NSString stringWithUTF8String:key.c_str()]
                forKey:@"market"
            ];
            [dict setObject:[NSNumber numberWithUnsignedLong:identifier]
                forKey:@"id"
            ];
            [dict setObject:[NSNumber numberWithUnsignedLong:state]
                forKey:@"state"
            ];
            
            dispatch_async(dispatch_get_main_queue(),^
            {
                [[NSNotificationQueue defaultQueue] enqueueNotification:
                    [NSNotification notificationWithName:
                    kCOBidStateDidChangeNotification object:dict]
                    postingStyle:NSPostASAP
                ];
            });
        }
    
        virtual void on_trade(
            const std::string & key, const std::uint32_t & tid,
            std::map<std::string, std::string> & info
            )
        {
            NSMutableDictionary * dict = [NSMutableDictionary dictionary];
            
            [dict setObject:
                [NSString stringWithUTF8String:key.c_str()]
                forKey:@"market"
            ];
            [dict setObject:[NSNumber numberWithUnsignedLong:tid]
                forKey:@"tid"
            ];

            NSMutableDictionary * dictInfo = [NSMutableDictionary dictionary];

            for (auto & i : info)
            {
                NSString * key = [NSString stringWithUTF8String:
                    i.first.c_str()
                ];
                NSString * val = [NSString stringWithUTF8String:
                    i.second.c_str()
                ];
                
                if (key && val)
                {
                    [dictInfo setObject:val forKey:key];
                }
            }
            
            [dict setObject:dictInfo forKey:@"info"];
            
            dispatch_async(dispatch_get_main_queue(),^
            {
                [[NSNotificationCenter defaultCenter]
                    postNotificationName:
                    kCOTradeDidUpdateNotification object:dict
                ];
            });
        }
    
        virtual void on_trollbox(
            const std::string & key,
            const std::vector< std::map<std::string, std::string> > & trolls
            )
        {
            NSMutableDictionary * dict = [NSMutableDictionary dictionary];
            
            [dict setObject:
                [NSString stringWithUTF8String:key.c_str()]
                forKey:@"market"
            ];

            NSMutableArray * trollEntries = [NSMutableArray array];
            
            for (auto & i : trolls)
            {
                NSMutableDictionary * troll = [NSMutableDictionary dictionary];
                
                for (auto & j : i)
                {
                    NSString * key = [NSString stringWithUTF8String:
                        j.first.c_str()
                    ];
                    NSString * val = [NSString stringWithUTF8String:
                        j.second.c_str()
                    ];
                    
                    if (key && val)
                    {
                        [troll setObject:val forKey:key];
                    }
                }
                
                [trollEntries addObject:troll];
            }
            
            [dict setObject:trollEntries forKey:@"trolls"];
            
            dispatch_async(dispatch_get_main_queue(),^
            {
                [[NSNotificationCenter defaultCenter]
                    postNotificationName:
                    kCOTrollboxDidUpdateNotification object:dict
                ];
            });
        }
    
        virtual void on_chat_message(
            const std::map<std::string, std::wstring> & params
            )
        {
            NSMutableDictionary * dict = [NSMutableDictionary dictionary];

            for (auto & i : params)
            {
                NSString * key = [NSString stringWithUTF8String:
                    i.first.c_str()
                ];
                NSString * val = [NSString stringWithStdWString:i.second];
                
                if (key && val)
                {
                    [dict setObject:val forKey:key];
                }
            }
            
            dispatch_async(dispatch_get_main_queue(),^
            {
                [[NSNotificationCenter defaultCenter]
                    postNotificationName:
                    kCODidReceiveChatMessageNotification object:dict
                ];
            });
        }
    
        virtual void on_find_message(
            const std::uint16_t & transaction_id,
            const std::map<std::string, std::string> & pairs,
            const std::vector<std::string> & tags
            )
        {
            // ...
        }
        
        virtual void on_find_profile(
            const std::uint16_t & transaction_id,
            const std::map<std::string, std::string> & pairs
            )
        {
            // ...
        }
    
        /**
         * Called when a version check completes.
         */
        virtual void on_version(
            const std::map<std::string, std::string> & pairs
            )
        {
            @autoreleasepool
            {
                NSMutableDictionary * dict = [NSMutableDictionary dictionary];

                for (auto & i : pairs)
                {
                    NSString * key = [NSString stringWithUTF8String:
                        i.first.c_str()
                    ];
                    NSString * val = [NSString stringWithUTF8String:
                        i.second.c_str()
                    ];
                    
                    if (key && val)
                    {
                        [dict setObject:val forKey:key];
                    }
                }
                
                dispatch_async(dispatch_get_main_queue(),^
                {
                    [[NSNotificationCenter defaultCenter]
                        postNotificationName:kCOOnVersionNotification
                        object:dict
                    ];
                });
            }
        }
    
    private:
    
    protected:
};

static my_coinsy_stack * g_my_coinsy_stack = 0;

@interface COStack ()
@property (nonatomic, strong) NSTimer * timer;
@end

@implementation COStack

+ (COStack *)sharedInstance
{
    static COStack * gCOStack = nil;
    
    if (!gCOStack)
    {
        gCOStack = [COStack new];

        [[NSNotificationCenter defaultCenter] addObserver:gCOStack
            selector:@selector(didConnectNotification:)
            name:kCODidConnectNotification object:nil
        ];
        
        [[NSNotificationCenter defaultCenter] addObserver:gCOStack
            selector:@selector(didDisconnectNotification:)
            name:kCODidDisconnectNotification object:nil
        ];
        
        [[NSNotificationCenter defaultCenter] addObserver:gCOStack
            selector:@selector(didSignInNotification:)
            name:kCODidSignInNotification object:nil
        ];
    
        [[NSUserDefaults standardUserDefaults] setObject:
            [NSNumber numberWithInt:0] forKey:@"coStatsConnects"
        ];
        
        [[NSUserDefaults standardUserDefaults] setObject:
            [NSNumber numberWithInt:0] forKey:@"coStatsDisconnects"
        ];
    }
    
    return gCOStack;
}

- (void)start
{
    if (g_my_coinsy_stack)
    {
        // ...
    }
    else
    {
        g_my_coinsy_stack = new my_coinsy_stack();
        
        g_my_coinsy_stack->start();
    }
}

- (void)start:(NSNumber *)aPort
{
    if (g_my_coinsy_stack)
    {
        // ...
    }
    else
    {
        g_my_coinsy_stack = new my_coinsy_stack();
        
        g_my_coinsy_stack->start(aPort.intValue);
    }
}

- (void)stop
{
    [self.timer invalidate];
    
    if (g_my_coinsy_stack)
    {
        g_my_coinsy_stack->stop();
        
        delete g_my_coinsy_stack, g_my_coinsy_stack = 0;
    }
    else
    {
        // ...
    }
}

- (void)signIn:(NSString *)aUsername password:(NSString *)aPassword
{
    if (g_my_coinsy_stack)
    {
        self.username = aUsername;
        
        g_my_coinsy_stack->sign_in(
            aUsername.UTF8String, aPassword.UTF8String
        );
    }
}

- (void)signOut
{
    if (g_my_coinsy_stack)
    {
        self.username = nil;
        
        g_my_coinsy_stack->sign_out();
    }
}

- (NSUInteger)find:(NSString *)aQuery
{
    if (g_my_coinsy_stack)
    {
        return g_my_coinsy_stack->find(aQuery.UTF8String, 200);
    }
    
    return 0;
}

- (NSString *)message:(NSString *)aMessage username:(NSString *)aUsername
{
    if (g_my_coinsy_stack)
    {
        return [NSString stringWithUTF8String:g_my_coinsy_stack->message(
            aUsername.UTF8String, aMessage.stdWString
        ).c_str()];
    }
    
    return nil;
}

- (void)updateProfile:(NSDictionary *)aProfile
{
    if (g_my_coinsy_stack)
    {
        std::map<std::string, std::string> profile;
        
        for (id key in aProfile)
        {
            if ([[aProfile objectForKey:key] isKindOfClass:NSString.class])
            {
                profile[[key UTF8String]] =
                    [[aProfile objectForKey:key] UTF8String
                ];
            }
        }
    
        g_my_coinsy_stack->update_profile(profile);
    }
}

- (void)updateProfile
{
    NSUserDefaults * userDefaults = [NSUserDefaults standardUserDefaults];
    
    NSString * username = [userDefaults objectForKey:@"username"];
    
    NSDictionary * preferences = [userDefaults objectForKey:username];
    
    NSDictionary * profile = [preferences objectForKey:@"profile"];
    
    if (profile || profile.count > 0)
    {
        [self updateProfile:profile];
    }
}

#pragma mark - NSNotification's

- (void)didConnectNotification:(NSNotification *)aNotification
{
    self.isConnected = YES;
    
    NSNumber * connects = [[NSUserDefaults standardUserDefaults]
        objectForKey:@"coStatsConnects"
    ];
    
    if (connects)
    {
        connects = [NSNumber numberWithInt:connects.intValue + 1];
    }
    else
    {
        connects = [NSNumber numberWithInt:1];
    }
    
    NSLog(@"Stats connects = %@", connects);
    
    [[NSUserDefaults standardUserDefaults] setObject:connects
        forKey:@"coStatsConnects"
    ];
    
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)didDisconnectNotification:(NSNotification *)aNotification
{
    self.isConnected = NO;
    
    NSNumber * disconnects = [[NSUserDefaults standardUserDefaults]
        objectForKey:@"coStatsDisconnects"
    ];
    
    if (disconnects)
    {
        disconnects = [NSNumber numberWithInt:disconnects.intValue + 1];
    }
    else
    {
        disconnects = [NSNumber numberWithInt:1];
    }
    
    NSLog(@"Stats disconnects = %@", disconnects);
    
    [[NSUserDefaults standardUserDefaults] setObject:disconnects
        forKey:@"coStatsDisconnects"
    ];
    
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)didSignInNotification:(NSNotification *)aNotification
{
    NSDictionary * dict  = aNotification.object;
    
    if ([[dict objectForKey:@"status"] intValue] == 0)
    {
        NSString * query = [NSString stringWithFormat:@"u=%@", self.username];
        
        [self performSelector:@selector(find:) withObject:query afterDelay:1.5f];
    }
    else
    {
        self.username = nil;
    }
}

#pragma mark -

- (BOOL)marketAdd:(NSString *)aMarket
{
    return g_my_coinsy_stack->market_add(aMarket.UTF8String);
}

- (BOOL)marketRemove:(NSString *)aMarket
{
    return g_my_coinsy_stack->market_remove(aMarket.UTF8String);
}

- (NSNumber *)market:(NSString *)aMarket ask:(NSDictionary *)anAsk
{
    NSNumber * price = [anAsk objectForKey:@"__p"];
    NSNumber * quantity = [anAsk objectForKey:@"__q"];
    
    return [NSNumber numberWithUnsignedInt:
        g_my_coinsy_stack->market_ask(
        aMarket.UTF8String, price.floatValue, quantity.floatValue)
    ];
}

- (NSNumber *)market:(NSString *)aMarket bid:(NSDictionary *)aBid
{
    NSNumber * price = [aBid objectForKey:@"__p"];
    NSNumber * quantity = [aBid objectForKey:@"__q"];
    
    return [NSNumber numberWithUnsignedInt:
        g_my_coinsy_stack->market_bid(
        aMarket.UTF8String, price.floatValue, quantity.floatValue)
    ];
}

- (BOOL)market:(NSString *)aMarket cancel:(NSNumber *)orderId
{
    return g_my_coinsy_stack->market_cancel(
        aMarket.UTF8String, orderId.unsignedIntValue
    );
}

- (void)market:(NSString *)aMarket troll:(NSString *)aMessage
{
    g_my_coinsy_stack->market_troll(aMarket.UTF8String, aMessage.UTF8String);
}

@end
