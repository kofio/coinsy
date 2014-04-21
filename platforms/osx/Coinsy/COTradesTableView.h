//
//  COTradesTableView.h
//  Coinsy
//
//  Created by Adudalesdi Ganiladisdi on 4/2/14.
//  Copyright (c) 2014 Adudalesdi Ganiladisdi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class COTradesTableView;

@protocol COTradesTableViewDelegate <NSObject>

- (void)tableView:(COTradesTableView *)tableView didClickedRow:(NSInteger)row;

@end

@interface COTradesTableView : GRProTableView
@property (nonatomic, weak) id<COTradesTableViewDelegate> delegateTrades;
@end
