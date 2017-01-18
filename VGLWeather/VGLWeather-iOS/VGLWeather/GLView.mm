//
//  GLView.m
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#import "vgl.h"
#import "GLView.h"
#import "Renderer.h"
#import "System.h"
#import "Window.h"
#import "EventDispatcher.h"
#import "AnimationManager.h"

@implementation GLView
{
  EAGLContext *context;
  CADisplayLink *displayLink;
  GLuint colorRenderbuffer, depthRenderbuffer;
  GLint backingWidth, backingHeight;
  BOOL touchDown;
  
  Renderer *renderer;
}

@synthesize defaultFramebuffer;

+(Class) layerClass
{
  return [CAEAGLLayer class];
}

-(id) initWithCoder:(NSCoder *)coder
{
  if((self = [super initWithCoder:coder]))
  {
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

    eaglLayer.opaque = TRUE;
    eaglLayer.drawableProperties = @{
      kEAGLDrawablePropertyRetainedBacking : @NO,
      kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8
    };

    if([[UIScreen mainScreen] respondsToSelector:@selector(scale)] && [[UIScreen mainScreen] scale] > 1)
    {
      [self setContentScaleFactor:[UIScreen mainScreen].scale];
    }

    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    if(!context || ![EAGLContext setCurrentContext:context])
    {
      return nil;
    }

    // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
    glGenFramebuffers(1, &defaultFramebuffer);
    glGenRenderbuffers(1, &colorRenderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
    
    //create a depth buffer
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    
    //make things easier on resource loading code
    NSBundle *mainBundle;
    mainBundle = [NSBundle mainBundle];
    
    NSString *str = [ mainBundle resourcePath ];
    chdir([str UTF8String ]);
    
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(update:)];
    [displayLink setFrameInterval:1];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  }
  
  return self;
}

-(void) layoutSubviews
{
  CAEAGLLayer *layer = (CAEAGLLayer *)self.layer;
  
  [EAGLContext setCurrentContext:context];
  
  // Allocate color buffer backing based on the current layer size
  glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
  [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
  
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
  
#ifdef DEBUG
  NSLog(@"backing %d x %d", backingWidth, backingHeight);
#endif
  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
  
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    return;
  }
  
  if(!renderer)
    renderer = new Renderer(false, self.contentScaleFactor, defaultFramebuffer);
  
  //notify the VGL system
  renderer->updateWindowSize(backingWidth, backingHeight);
  vgl::System::system().backingResized();
  
  [self drawView:nil];
}

-(void) update:(id)sender
{
  if(renderer->needsRender())
  {
    [self drawView:nil];
  }
  else
  {
    vui::Window::getCurrentWindow()->getAnimationManager()->update();
    renderer->runScheduledTasks();
    
    if(renderer->needsRender())
    {
      [self drawView:nil];
    }
  }
}

float2 cgPointToFloat2(UIView *view, const CGPoint &p)
{
  return { (float)p.x, (float)(view.bounds.size.height-p.y) };
}

-(void) touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
  vui::EventDispatcher::dispatcher().mouseDown(cgPointToFloat2(self, [touches.anyObject locationInView:self]));
}

-(void) touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
  vui::EventDispatcher::dispatcher().mouseMoved(cgPointToFloat2(self, [touches.anyObject locationInView:self]));
}

-(void) touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
  vui::EventDispatcher::dispatcher().mouseUp(cgPointToFloat2(self, [touches.anyObject locationInView:self]));
}

-(void) touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
  vui::EventDispatcher::dispatcher().mouseUp(cgPointToFloat2(self, [touches.anyObject locationInView:self]));
}

-(void) drawView:(id)sender
{
  [EAGLContext setCurrentContext:context];

  renderer->draw();
  
  glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
  if(sender != self)
  {
    GLenum discards[2] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, discards);
    
    [context presentRenderbuffer:GL_RENDERBUFFER];
  }
}

-(BOOL) canBecomeFirstResponder
{
  return YES;
}

-(BOOL) resignFirstResponder
{
  BOOL ret = [super resignFirstResponder];
  
  if(auto oldKeyboardFocusedView = vui::Window::getCurrentWindow()->getKeyboardFocusedView())
  {
    oldKeyboardFocusedView->keyboardUnFocused();
    oldKeyboardFocusedView->setNeedsRender();
  }

  return ret;
}

-(void) insertText:(NSString *)text
{
  //TODO: add scancodes to some of these via reverse char mapping
  const char *str = [text UTF8String];
  if(str && strlen(str))
  {
    char c = str[0];
    if(isprint(c))
    {
      vui::Key k = { vui::SC_UNKNOWN, c };
        
      vui::EventDispatcher::dispatcher().keyDown(k);
      vui::EventDispatcher::dispatcher().keyUp(k);
    }
  }
}

-(void) deleteBackward
{
  vui::Key k = { vui::SC_BACKSPACE, NULL };
  vui::EventDispatcher::dispatcher().keyDown(k);
  vui::EventDispatcher::dispatcher().keyUp(k);
}

-(BOOL) hasText
{
  return NO;
}

-(void) dealloc
{
  [displayLink invalidate];
  
  [EAGLContext setCurrentContext:context];
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  
  // Tear down GL
  if(defaultFramebuffer)
  {
    glDeleteFramebuffers(1, &defaultFramebuffer);
    defaultFramebuffer = 0;
  }
  
  if(colorRenderbuffer)
  {
    glDeleteRenderbuffers(1, &colorRenderbuffer);
    colorRenderbuffer = 0;
  }
  
  if(depthRenderbuffer)
  {
    glDeleteRenderbuffers(1, &depthRenderbuffer);
    depthRenderbuffer = 0;
  }
  
  if(renderer)
    delete renderer;
}

@end
