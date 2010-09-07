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
// Defines Windows specific flags. They are not available on other platforms.
//
#define RUNTIME_OS_FLAGS(develop, develop_pd, product, product_pd,       \
                         diagnostic, notproduct)                         \
                                                                         \
  product(bool, UseUTCFileTimestamp, true,                               \
          "Adjust the timestamp returned from stat() to be UTC")


//
// Defines Windows-specific default values. The flags are available on all
// platforms, but they may have different default values on other platforms.
//
define_pd_global(bool, UseLargePages, false);
define_pd_global(bool, UseLargePagesIndividualAllocation, true);
define_pd_global(bool, UseOSErrorReporting, false);  // for now.
define_pd_global(bool, UseThreadPriorities, true) ;
