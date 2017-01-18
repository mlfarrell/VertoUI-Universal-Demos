//
//  GLView.h
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#import "vgl.h"
#import <UIKit/UIKit.h>

@interface GLView : UIView<UIKeyInput>

@property (nonatomic, assign) GLuint defaultFramebuffer;

@end
