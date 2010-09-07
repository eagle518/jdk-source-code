/*
 * @(#)JavaPlugIn2Safari.m	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#import <WebKit/WebKit.h>
#import <WebKit/WebPlugin.h>
#import <JavaScriptCore/JSValueRef.h>
#import <assert.h>

#import "JavaPlugIn2Safari.h"
#import "StringUtil.h"
#import "AbstractPlugin.h"
#import "LocalFramePusher.h"
#import "JNIExceptions.h"

#include <stdio.h>

static BOOL firstInitialization = TRUE;
static BOOL initFailed = TRUE;
static jclass safariPluginClass = NULL;
static jmethodID safariPluginCtorID = NULL;
static jmethodID safariPluginAddParametersID = NULL;
static jmethodID safariPluginSetBaseURLID = NULL;
static jmethodID safariPluginSetBoundsID = NULL;
static jmethodID safariPluginSetFrameID = NULL;
static jmethodID safariPluginSetWindowHandleID = NULL;
static jmethodID safariPluginSetLocationAndClipID = NULL;
static jmethodID safariPluginWebPlugInInitializeID = NULL;
static jmethodID safariPluginWebPlugInStartID = NULL;
static jmethodID safariPluginWebPlugInStopID = NULL;
static jmethodID safariPluginWebPlugInDestroyID = NULL;
static jmethodID safariPluginWebPlugInSetIsSelectedID = NULL;
static jmethodID safariPluginDrainRunnableQueueID = NULL;

@implementation JavaPlugIn2Safari

+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments
{
    JavaPlugIn2Safari *javaPlugIn = [[[self alloc] init] autorelease];
    [javaPlugIn setArguments:arguments];
    javaPlugIn->appletObject = NULL;
    javaPlugIn->appletContext = NULL;
    return javaPlugIn;
}

- (void)dealloc
{   
    [_arguments release];
    if (appletObject != NULL) {
        JSValueUnprotect(appletContext, appletObject);
        appletContext = NULL;
    }

    [super dealloc];
}

- (void)setArguments:(NSDictionary *)arguments
{
    if (arguments != _arguments) {
        [_arguments release];
        _arguments = [arguments copy];
    }
}

- (void) updateLocationAndClip
{
    if (pluginObject != NULL) {
        LocalFramePusher pusher;
        JNIEnv* env = pusher.getEnv();

        // Find the top-level scrollView in the hierarchy
        NSView* scrollView = self;
        NSView* cur = self;
        while (cur != nil) {
            cur = [cur superview];
            if (cur != nil) {
                if ([cur isKindOfClass: [NSScrollView class]]) {
                    scrollView = cur;
                }
            }
        }

        NSRect frameRect = [self frame];
        // Find our origin in window coordinates
        // NOTE that we use the upper left coordinate here -- not 100%
        // clear what is going on in the coordinate conversion to
        // require this, but this is correct
        NSPoint basePoint = [self convertPoint: NSMakePoint(0, frameRect.size.height) toView: nil];
        NSWindow* win = [self window];
        NSPoint screenOrigin = [win convertBaseToScreen: basePoint];
        // We need to consider the screen height in order to get the right coordinate system
        NSScreen* screen = [win screen];
        NSRect screenFrame = [screen frame];

        // Clip the window to its visible rect
        NSRect visibleRect = [self visibleRect];
        CGFloat oldBottom = visibleRect.origin.y;
        CGFloat oldTop = oldBottom + visibleRect.size.height;
        CGFloat newBottom = frameRect.size.height - oldTop;
        // Height stays the same

        env->CallVoidMethod(pluginObject, safariPluginSetLocationAndClipID,
                            // Location
                            (jdouble) screenOrigin.x,
                            (jdouble) (screenFrame.size.height - screenOrigin.y),
                            // Clip
                            (jdouble) visibleRect.origin.x,
                            (jdouble) newBottom,
                            (jdouble) visibleRect.size.width,
                            (jdouble) visibleRect.size.height);
        CLEAR_EXCEPTION(env);
    }    
}

- (void) drawRect: (NSRect) aRect
{
    [self updateLocationAndClip];
}

- (void)webPlugInInitialize
{
    if (firstInitialization) {
        firstInitialization = FALSE;

        // Initialize JNI method IDs we need
        LocalFramePusher pusher;
        JNIEnv* env = pusher.getEnv();
        // Guard against crashes in product builds
        if (env == NULL) {
            return;
        }
        jclass clazz = env->FindClass("sun/plugin2/main/server/SafariPlugin");
        if (clazz == NULL) {
            env->ExceptionDescribe();
        }
        assert(clazz != NULL);  // Stop in debug builds
        if (clazz == NULL) {
            return;
        }
        safariPluginClass = (jclass) env->NewGlobalRef(clazz);
        safariPluginCtorID = env->GetMethodID(safariPluginClass, "<init>", "(JJJ)V");
        safariPluginAddParametersID = env->GetMethodID(safariPluginClass, "addParameters",
                                                       "([Ljava/lang/String;[Ljava/lang/String;)V");
        safariPluginSetBaseURLID = env->GetMethodID(safariPluginClass, "setBaseURL", "(Ljava/lang/String;)V");
        safariPluginSetBoundsID = env->GetMethodID(safariPluginClass, "setBounds", "(FFFF)V");
        safariPluginSetFrameID = env->GetMethodID(safariPluginClass, "setFrame", "(FFFF)V");
        safariPluginSetWindowHandleID = env->GetMethodID(safariPluginClass, "setWindowHandle", "(J)V");
        safariPluginSetLocationAndClipID = env->GetMethodID(safariPluginClass, "setLocationAndClip", "(DDDDDD)V");
        safariPluginWebPlugInInitializeID = env->GetMethodID(safariPluginClass, "webPlugInInitialize", "()V");
        safariPluginWebPlugInStartID      = env->GetMethodID(safariPluginClass, "webPlugInStart",      "()V");
        safariPluginWebPlugInStopID       = env->GetMethodID(safariPluginClass, "webPlugInStop",       "()V");
        safariPluginWebPlugInDestroyID    = env->GetMethodID(safariPluginClass, "webPlugInDestroy",    "()V");
        safariPluginWebPlugInSetIsSelectedID = env->GetMethodID(safariPluginClass, "webPlugInSetIsSelected", "(Z)V");
        safariPluginDrainRunnableQueueID  = env->GetStaticMethodID(safariPluginClass, "drainRunnableQueue", "()V");
        assert(safariPluginCtorID != NULL &&
               safariPluginAddParametersID != NULL &&
               safariPluginSetBaseURLID != NULL &&
               safariPluginSetBoundsID != NULL &&
               safariPluginSetFrameID != NULL &&
               safariPluginSetWindowHandleID != NULL &&
               safariPluginSetLocationAndClipID != NULL &&
               safariPluginWebPlugInInitializeID != NULL &&
               safariPluginWebPlugInStartID != NULL &&
               safariPluginWebPlugInStopID != NULL &&
               safariPluginWebPlugInDestroyID != NULL &&
               safariPluginWebPlugInSetIsSelectedID != NULL &&
               safariPluginDrainRunnableQueueID != NULL);
        if (safariPluginCtorID == NULL ||
            safariPluginAddParametersID == NULL ||
            safariPluginSetBaseURLID == NULL ||
            safariPluginSetBoundsID == NULL ||
            safariPluginSetFrameID == NULL ||
            safariPluginSetWindowHandleID == NULL ||
            safariPluginSetLocationAndClipID == NULL ||
            safariPluginWebPlugInInitializeID == NULL ||
            safariPluginWebPlugInStartID == NULL ||
            safariPluginWebPlugInStopID == NULL ||
            safariPluginWebPlugInDestroyID == NULL ||
            safariPluginWebPlugInSetIsSelectedID == NULL ||
            safariPluginDrainRunnableQueueID == NULL) {
            env->ExceptionDescribe();
            return;
        }

        AbstractPlugin::initialize();

        initFailed = FALSE;
    }

    if (!initFailed) {
        LocalFramePusher pusher;
        JNIEnv* env = pusher.getEnv();

        // Construct the Java-side SafariPlugin object
        NSObject* container = (NSObject*) [_arguments objectForKey: WebPlugInContainerKey];
        WebFrame* frame = (WebFrame*) [container webFrame];
        WebScriptObject* scriptObject = [frame windowObject];
        JSObjectRef jsScriptObject = [scriptObject JSObject];
        jobject obj = env->NewObject(safariPluginClass,
                                     safariPluginCtorID,
                                     (jlong) self,
                                     (jlong) container,
                                     (jlong) jsScriptObject);
        CHECK_EXCEPTION(env);
        pluginObject = env->NewGlobalRef(obj);
    
        // Set up the applet parameters
        NSDictionary* params = [_arguments objectForKey: WebPlugInAttributesKey];
        NSArray* keys = [params allKeys];
        int numKeys = [keys count];
        jclass strClass = env->FindClass("java/lang/String");
        CHECK_EXCEPTION(env);

        jobjectArray keyArray = env->NewObjectArray(numKeys, strClass, NULL);
        CHECK_EXCEPTION(env);
        jobjectArray valArray = env->NewObjectArray(numKeys, strClass, NULL);
        CHECK_EXCEPTION(env);
        {
            int i;
            for (i = 0; i < numKeys; i++) {
                NSString* objcKey = [keys objectAtIndex: i];
                jstring key = NSStringToJString(objcKey, env);
                jstring val = NSStringToJString([params objectForKey: objcKey], env);
                env->SetObjectArrayElement(keyArray, i, key);
                CHECK_EXCEPTION(env);
                env->SetObjectArrayElement(valArray, i, val);
                CHECK_EXCEPTION(env);
            }
        }
        env->CallVoidMethod(pluginObject, safariPluginAddParametersID,
                            keyArray, valArray);
        CHECK_EXCEPTION(env);

        // Pass up the base URL as well
        // It looks like this is already in the parameter map under
        // the key "baseURL", but since that doesn't appear to be
        // specified, better safe than sorry
        NSURL* baseURL = [_arguments objectForKey: WebPlugInBaseURLKey];
        if (baseURL != nil) {
            env->CallVoidMethod(pluginObject, safariPluginSetBaseURLID,
                                NSStringToJString([baseURL absoluteString], env));
            CHECK_EXCEPTION(env);
        }

        // Pass up the window number to the Java level so that we can
        // link the applet's content in to the browser window
        NSWindow* window = [self window];
        NSInteger windowNum = [window windowNumber];
        env->CallVoidMethod(pluginObject, safariPluginSetWindowHandleID, (jlong) windowNum);
        CHECK_EXCEPTION(env);

        // Call webPlugInInitialize at the Java level
        env->CallVoidMethod(pluginObject, safariPluginWebPlugInInitializeID);
        CHECK_EXCEPTION(env);
    }
}

- (void)webPlugInStart
{
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    env->CallVoidMethod(pluginObject, safariPluginWebPlugInStartID);
    CLEAR_EXCEPTION(env);
}

- (void)webPlugInStop
{
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    env->CallVoidMethod(pluginObject, safariPluginWebPlugInStopID);
    CLEAR_EXCEPTION(env);
}

- (void)webPlugInDestroy
{
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    env->CallVoidMethod(pluginObject, safariPluginWebPlugInDestroyID);
    CLEAR_EXCEPTION(env);
}

- (void)webPlugInSetIsSelected:(BOOL)isSelected
{
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    env->CallVoidMethod(pluginObject, safariPluginWebPlugInSetIsSelectedID,
                        (jboolean) isSelected);
    CLEAR_EXCEPTION(env);
}

// Notifications we need to pass up to Java
- (void)setBounds:(NSRect)boundsRect
{
    [super setBounds:boundsRect];
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    if (env == NULL || initFailed) {
        return;
    }
    env->CallVoidMethod(pluginObject, safariPluginSetBoundsID,
                        (jfloat) boundsRect.origin.x,
                        (jfloat) boundsRect.origin.y,
                        (jfloat) boundsRect.size.width,
                        (jfloat) boundsRect.size.height);
    CLEAR_EXCEPTION(env);
}

// Notifications we need to pass up to Java
- (void)setFrame:(NSRect)boundsRect
{
    [super setFrame:boundsRect];
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    if (env == NULL || initFailed) {
        return;
    }
    env->CallVoidMethod(pluginObject, safariPluginSetFrameID,
                        (jfloat) boundsRect.origin.x,
                        (jfloat) boundsRect.origin.y,
                        (jfloat) boundsRect.size.width,
                        (jfloat) boundsRect.size.height);
    CLEAR_EXCEPTION(env);
}

- (void)drainRunnableQueue
{
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    env->CallStaticVoidMethod(safariPluginClass, safariPluginDrainRunnableQueueID);
    CLEAR_EXCEPTION(env);
}

- (JSObjectRef)jsObjectWithContext: (JSContextRef)context
{
    assert(pluginObject != NULL);
    if (pluginObject == NULL) {
        // Something went wrong during initialization
        return NULL;
    }

    if (appletObject == NULL) {
        LocalFramePusher pusher;
        // Guard against crashes in product builds
        if (pusher.getEnv() == NULL || initFailed) { 
            return NULL;
        }
        appletObject =
            (JSObjectRef) AbstractPlugin::getScriptingObjectForApplet(pluginObject, 0);
        // This is the protect call for our persistent reference
        if (appletObject != NULL) {
            JSValueProtect(context, appletObject);
            appletContext = context;
        }
    }

    return appletObject;
}

- (void) _clipViewAncestorDidScroll:(NSClipView*) clipView
{
    [self updateLocationAndClip];
}

@end
