
/*
 * @(#)WindowsFlags.h	1.2 03/07/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef WINDOWSFLAGS_H
#define WINDOWSFLAGS_H

extern BOOL      ddVramForced;       // disable punting of ddraw buffers
extern BOOL      accelReset;         // reset registry 2d acceleration settings
extern BOOL      useDD;	             // ddraw enabled flag
extern BOOL      useD3D;             // d3d enabled flag
extern BOOL      forceD3DUsage;      // force d3d on or off
extern jboolean  g_offscreenSharing; // JAWT accelerated surface sharing
extern BOOL      useDDLock;          // Disabled for win2k/XP
extern BOOL      checkRegistry;      // Diag tool: outputs 2d registry settings
extern BOOL      disableRegistry;    // Diag tool: disables registry interaction

void SetD3DEnabledFlag(JNIEnv *env, BOOL d3dEnabled, BOOL d3dSet);

void SetDDEnabledFlag(JNIEnv *env, BOOL ddEnabled);

#endif WINDOWSFLAGS_H

