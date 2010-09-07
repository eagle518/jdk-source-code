/*
 * Copyright (c) 2005, 2008, Oracle and/or its affiliates. All rights reserved.
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

//
// Defines Solaris specific flags. They are not available on other platforms.
//
#define RUNTIME_OS_FLAGS(develop, develop_pd, product, product_pd, diagnostic, notproduct) \
                                                                               \
  product(bool, UseISM, false,                                                 \
          "Use Intimate Shared Memory (Solaris Only)")                         \
                                                                               \
  product(bool, UsePermISM, false,                                             \
          "Obsolete flag for compatibility (same as UseISM)")                  \
                                                                               \
  product(bool, UseMPSS, true,                                                 \
          "Use Multiple Page Size Support (Solaris 9 Only)")                   \
                                                                               \
  product(bool, UseExtendedFileIO, true,                                       \
          "Enable workaround for limitations of stdio FILE structure")

//
// Defines Solaris-specific default values. The flags are available on all
// platforms, but they may have different default values on other platforms.
//
define_pd_global(bool, UseLargePages, true);
define_pd_global(bool, UseLargePagesIndividualAllocation, false);
define_pd_global(bool, UseOSErrorReporting, false);
define_pd_global(bool, UseThreadPriorities, false);
