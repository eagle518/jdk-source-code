/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include "isNT4.hpp"
#include <windows.h>

bool
isNT4() {
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);

  if (!GetVersionEx(&info)) {
    return false;
  }

  return ((info.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
          (info.dwMajorVersion == 4));
}
