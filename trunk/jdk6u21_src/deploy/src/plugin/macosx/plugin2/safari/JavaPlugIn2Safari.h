/*
 * @(#)JavaPlugIn2Safari.h	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#import <AppKit/AppKit.h>
#import "jni.h"
#import <JavaScriptCore/JSBase.h>

@interface JavaPlugIn2Safari : NSView
{
    NSDictionary *_arguments;
    jobject pluginObject;           // SafariPluginObject Java instance
    JSObjectRef appletObject;       // The JavaScript mirror for the applet
    JSContextRef appletContext;     // The context in which the applet object was instantiated
}

- (void) setArguments:(NSDictionary *)arguments;
- (void) updateLocationAndClip;

@end
