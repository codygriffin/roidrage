#import "AssetManager.h"
#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#include "Log.h"

#include <memory>
#include <string>

using namespace pronghorn;

std::unique_ptr<char[]>
AssetManager::loadBitmap(const std::string& str,
                         unsigned& width,
                         unsigned& height) {
  // Create and compile fragment shader.
  //auto path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:str.c_str()] ofType:nil];
  
  auto path =[NSString stringWithUTF8String:str.c_str()];
  NSImage* image = [[NSImage alloc] initWithContentsOfFile:path];
  auto     imageData = std::unique_ptr<char[]>(new char[(size_t)(image.size.width * image.size.height * 4)]);

  CGContextRef imageContext = CGBitmapContextCreate(imageData.get(),
                                                    image.size.width,
                                                    image.size.height,
                                                    8,
                                                    image.size.width * 4,
                                                    CGColorSpaceCreateDeviceRGB(),
                                                    kCGImageAlphaPremultipliedLast);
  
  auto rect = NSMakeRect(0.0, 0.0, image.size.width, image.size.height);
  Log::info("load % x %", image.size.width, image.size.height);
  CGContextSetBlendMode(imageContext, kCGBlendModeCopy);

  CGContextDrawImage(
    imageContext, 
    CGRectMake(0.0, 0.0, image.size.width, image.size.height), 
    [image CGImageForProposedRect: &rect context: NULL hints:NULL]
  );
  CGContextRelease(imageContext);
  
  return imageData;
}

std::unique_ptr<char[]>
AssetManager::loadText(const std::string& str) {
  // Create and compile fragment shader.
  //auto path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:str.c_str()] ofType:nil];
  auto path =[NSString stringWithUTF8String:str.c_str()];
  
  NSString* pText = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
  auto length = [pText length];
  auto data = std::unique_ptr<char[]>(new char[length+1]);
  memset(data.get(), 0, length+1);
  memcpy(data.get(), (char*)[pText UTF8String], length);
  
  return data;
}
