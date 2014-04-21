//
//  COTradesTableView.m
//  Coinsy
//
//  Created by Adudalesdi Ganiladisdi on 4/2/14.
//  Copyright (c) 2014 Adudalesdi Ganiladisdi. All rights reserved.
//

#import "COTradesTableView.h"

@implementation COTradesTableView

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
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint point = [self convertPoint:[theEvent locationInWindow]
        fromView:nil
    ];
    NSInteger row = [self rowAtPoint:point];

    [super mouseDown:theEvent];

    if (row != -1)
    {
        [self.delegateTrades tableView:self didClickedRow:row];
    }
}

@end
