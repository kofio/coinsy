//
//  COBackgroundView.m
//  Coinsy
//
//  Created by Adudalesdi Ganiladisdi on 3/14/14.
//  Copyright (c) 2014 Adudalesdi Ganiladisdi. All rights reserved.
//

#import "COBackgroundView.h"

@implementation COBackgroundView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSGradient * gradient = [[NSGradient alloc] initWithStartingColor:
        [NSColor colorWithCalibratedWhite:0.2f alpha:1.0f] endingColor:
        [NSColor colorWithCalibratedRed:32.0f / 255.0f green:36.0f / 255.0f
        blue:41.0f / 255.0f alpha:1.0f]
    ];
    [gradient drawInRect:self.bounds relativeCenterPosition:NSZeroPoint];
}

@end
