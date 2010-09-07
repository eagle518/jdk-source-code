/*
 * @(#)awt_xembed.c	1.1 03/08/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_p.h"

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Xm/MwmUtil.h>

/* JNI headers */
#include "java_awt_Frame.h"     /* for frame state constants */

#include "awt_wm.h"
#include "awt_util.h"           /* for X11 error handling macros */


static Window getParent(Window window);
static Window getEmbedder(Window client);

#define XEMBED_VERSION  0
#define XEMBED_MAPPED  (1 << 0)
/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY              0
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_WINDOW_DEACTIVATE    2
#define XEMBED_REQUEST_FOCUS         3
#define XEMBED_FOCUS_IN             4
#define XEMBED_FOCUS_OUT            5
#define XEMBED_FOCUS_NEXT           6
#define XEMBED_FOCUS_PREV           7
/* 8-9 were used for XEMBED_GRAB_KEY/XEMBED_UNGRAB_KEY */
#define XEMBED_MODALITY_ON          10
#define XEMBED_MODALITY_OFF         11
#define XEMBED_REGISTER_ACCELERATOR     12
#define XEMBED_UNREGISTER_ACCELERATOR   13
#define XEMBED_ACTIVATE_ACCELERATOR     14
    
#define XEMBED_LAST_MSG XEMBED_ACTIVATE_ACCELERATOR

//     A detail code is required for XEMBED_FOCUS_IN. The following values are valid:    
/* Details for  XEMBED_FOCUS_IN: */
#define XEMBED_FOCUS_CURRENT        0
#define XEMBED_FOCUS_FIRST          1
#define XEMBED_FOCUS_LAST           2    

const char * error_msg = "UNKNOWN XEMBED MESSAGE";

const char * xembed_strs[] = {
    "EMBEDDED_NOTIFY",
    "WINDOW_ACTIVATE",
    "WINDOW_DEACTIVATE",
    "REQUEST_FOCUS", 
    "FOCUS_IN", 
    "FOCUS_OUT", 
    "FOCUS_NEXT", 
    "FOCUS_PREV" ,
    "GRAB_KEY",
    "UNGRAB_KEY",
    "MODALITY_ON" ,
    "MODALITY_OFF",
    "REGISTER_ACCELERATOR",
    "UNREGISTER_ACCELERATOR",
    "ACTIVATE_ACCELERATOR"
};

const char * 
msg_to_str(int msg) {
    if (msg >= 0 && msg <= XEMBED_LAST_MSG) {
        return xembed_strs[msg];
    } else {
        return error_msg;
    }
}

typedef struct _xembed_info {
    CARD32 version;
    CARD32 flags;
} xembed_info;

typedef struct _xembed_data {
    struct FrameData * wdata; // pointer to EmbeddedFrame wdata
    Window client; // pointer to plugin intermediate widget, XEmbed client
    Boolean active; // whether xembed is active for this client
    Boolean applicationActive; // whether the embedding application is active
    Window embedder; // Window ID of the embedder
    struct _xembed_data * next;
} xembed_data, * pxembed_data;

static pxembed_data xembed_list = NULL;

static pxembed_data
getData(Window client) {
    pxembed_data temp = xembed_list;
    while (temp != NULL) {
        if (temp->client == client) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

static pxembed_data
getDataByFrame(struct FrameData* wdata) {
    pxembed_data temp = xembed_list;
    while (temp != NULL) {
        if (temp->wdata == wdata) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

static pxembed_data 
addData(Window client) {
    xembed_data * data = malloc(sizeof(xembed_data));
    memset(data, 0, sizeof(xembed_data));
    data->client = client;
    data->next = xembed_list;
    xembed_list = data;
    return data;
}

static void 
removeData(Window client) {
    pxembed_data * temp = &xembed_list;
    while (*temp != NULL) {
        if ((*temp)->client == client) {
            xembed_data * data = *temp;            
            *temp = (*temp)->next;
            free(data);
            return;
        }
        temp = &(*temp)->next;
    }    
}

static Atom XA_XEmbedInfo;
static Atom XA_XEmbed;

void 
init_xembed() {
    XA_XEmbedInfo = XInternAtom(awt_display, "_XEMBED_INFO", False);
    XA_XEmbed = XInternAtom(awt_display, "_XEMBED", False);
}

static Time
getCurrentServerTime() {
    return awt_util_getCurrentServerTime();
}


static void 
sendMessageHelper(Window window, int message, long detail, 
                              long data1, long data2) 
{
    JNIEnv      *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    XEvent ev;
    XClientMessageEvent * req = (XClientMessageEvent*)&ev;
    memset(&ev, 0, sizeof(ev));
    
    req->type = ClientMessage;
    req->window = window;
    req->message_type = XA_XEmbed;
    req->format = 32;
    req->data.l[0] = getCurrentServerTime();
    req->data.l[1] = message;
    req->data.l[2] = detail;
    req->data.l[3] = data1;
    req->data.l[4] = data2;
    AWT_LOCK();
    XSendEvent(awt_display, window, False, NoEventMask, &ev);
    AWT_UNLOCK();
}

static void 
sendMessage(Window window, int message) {
    sendMessageHelper(window, message, 0, 0, 0);
}


static Window 
getParent(Window window) {
    Window root, parent = None, *children = NULL;
    unsigned int count;
    XQueryTree(awt_display, window, &root, &parent, &children, &count);
    if (children != NULL) {
        XFree(children);
    }
    return parent;
}

static Window 
getEmbedder(Window client) {
    return getParent(client);
}

static void
genWindowFocus(struct FrameData *wdata, Boolean gain) {
    XEvent ev;
    JNIEnv      *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    memset(&ev, 0, sizeof(ev));

    ev.type = (gain?FocusIn:FocusOut);
    ev.xany.send_event = True;
    ev.xany.display = awt_display;
    ev.xfocus.mode = NotifyNormal;
    ev.xfocus.detail = NotifyNonlinear;
    ev.xfocus.window = XtWindow(wdata->winData.shell);
    awt_put_back_event(env, &ev);
}

extern Boolean skipNextFocusIn;

void 
xembed_eventHandler(XEvent *event) 
{
    JNIEnv      *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    struct FrameData *wdata;
    xembed_data * data;
    
    data = getData(event->xany.window);
    if (data == NULL) {
        return;
    }

    wdata = data->wdata;

    if (event->xany.type == ClientMessage) {
        // Probably a message from embedder
        if (event->xclient.message_type == XA_XEmbed) {
            // XEmbed message, data[1] contains message
            switch ((int)event->xclient.data.l[1]) {
              case XEMBED_EMBEDDED_NOTIFY:
                  data->active = True;
                  data->embedder = getEmbedder(data->client);
                  // If Frame has not been reparented already we should "reparent"
                  // it manually
                  if (!(wdata->reparented)) {
                      wdata->reparented = True;                      
                      // in XAWT we also update WM_NORMAL_HINTS here.
                  }
                  break;
              case XEMBED_WINDOW_DEACTIVATE:
                  data->applicationActive = False;
                  break;
              case XEMBED_WINDOW_ACTIVATE:
                  data->applicationActive = True;
                  break;
              case XEMBED_FOCUS_IN:
                  skipNextFocusIn = False;
                  genWindowFocus(wdata, True);
                  break;
              case XEMBED_FOCUS_OUT:                  
                  genWindowFocus(wdata, False);
                  break;
            }
        }
    } else if (event->xany.type == ReparentNotify) {
        data->embedder = event->xreparent.parent;
    }
}

void 
install_xembed(Widget client_widget, struct FrameData* wdata) {
    JNIEnv      *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    xembed_info info = {XEMBED_VERSION, XEMBED_MAPPED};
    Window client = XtWindow(client_widget);
    xembed_data * data;
    
    AWT_LOCK();
    data = addData(client);
    data->wdata = wdata;

    // Install event handler for messages from embedder
    XSelectInput(awt_display, client, StructureNotifyMask);

    // Install XEMBED_INFO information
    XChangeProperty(awt_display, client, XA_XEmbedInfo, 
                    XA_XEmbedInfo, 32, PropModeReplace, 
                    (unsigned char*)&info, 2);        
    AWT_UNLOCK();
}

void 
deinstall_xembed(struct FrameData* wdata) {
    xembed_data * data = getDataByFrame(wdata);

    if (data != NULL) {        
        removeData(data->client);
    }
}

void 
requestXEmbedFocus(struct FrameData * wdata) {
    xembed_data * data = getDataByFrame(wdata);

    if (data != NULL) {
        if (data->active && data->applicationActive) {
            sendMessage(data->embedder, XEMBED_REQUEST_FOCUS);
        }
    }
}

Boolean 
isXEmbedActive(struct FrameData * wdata) {
    xembed_data * data = getDataByFrame(wdata);
    return (data != NULL && data->active);
}

Boolean 
isXEmbedActiveByWindow(Window client) {
    xembed_data * data = getData(client);
    return (data != NULL && data->active);
}


Boolean 
isXEmbedApplicationActive(struct FrameData * wdata) {
    xembed_data * data = getDataByFrame(wdata);
    return (data != NULL && data->applicationActive);
}

