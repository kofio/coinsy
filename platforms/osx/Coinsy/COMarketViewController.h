//
//  COMarketViewController.h
//  Coinsy
//
//  Created by Adudalesdi Ganiladisdi on 3/15/14.
//  Copyright (c) 2014 Adudalesdi Ganiladisdi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface COMarketViewController : NSViewController
@property (copy) NSString * market;
- (id)initWithMarket:(NSString *)aMarket;
@end
