
/*
 * @(#)WindowsFlags.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef WINDOWSFLAGS_H
#define WINDOWSFLAGS_H

extern BOOL      accelReset;         // reset registry 2d acceleration settings
extern BOOL      useD3D;             // d3d enabled flag
extern BOOL      forceD3DUsage;      // force d3d on or off
extern jboolean  g_offscreenSharing; // JAWT accelerated surface sharing
extern BOOL      checkRegistry;      // Diag tool: outputs 2d registry settings
extern BOOL      disableRegistry;    // Diag tool: disables registry interaction
extern BOOL      setHighDPIAware;    // whether to set High DPI Aware flag on Vista

void SetD3DEnabledFlag(JNIEnv *env, BOOL d3dEnabled, BOOL d3dSet);

BOOL IsD3DEnabled();
BOOL IsD3DForced();

#endif WINDOWSFLAGS_H

