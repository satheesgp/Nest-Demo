//
//  ControllerInfoCell.h
//  iOS-NestDK
//
//  Created by Satheesh Prabhu Gurusamy on 11/17/15.
//  Copyright © 2015 Nest Labs. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ControllerInfoCell : UITableViewCell
@property (weak, nonatomic) IBOutlet UILabel *controllerName;
@property (weak, nonatomic) IBOutlet UILabel *controllerID;

@end
