/*
 * Copyright (c) 2001, Oracle and/or its affiliates. All rights reserved.
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

#ifndef _LIBINFO_
#define _LIBINFO_

#include <vector>
#include <string>
#include <windows.h>

struct LibInfo {
  std::string name;
  void*  base;

  LibInfo(const std::string& name, void* base) {
    this->name = name;
    this->base = base;
  }
};

void libInfo(DWORD pid, std::vector<LibInfo>& info);

#endif  // #defined _LIBINFO_
