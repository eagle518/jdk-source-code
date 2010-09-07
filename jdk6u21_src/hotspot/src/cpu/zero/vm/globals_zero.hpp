/*
 * Copyright (c) 2000, 2006, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008, 2009, 2010 Red Hat, Inc.
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

// Set the default values for platform dependent flags used by the
// runtime system.  See globals.hpp for details of what they do.

define_pd_global(bool,  ConvertSleepToYield,  true);
define_pd_global(bool,  ShareVtableStubs,     true);
define_pd_global(bool,  CountInterpCalls,     true);
define_pd_global(bool,  NeedsDeoptSuspend,    false);

define_pd_global(bool,  ImplicitNullChecks,   true);
define_pd_global(bool,  UncommonNullCast,     true);

define_pd_global(intx,  CodeEntryAlignment,   32);
define_pd_global(intx,  InlineFrequencyCount, 100);
define_pd_global(intx,  PreInflateSpin,       10);

define_pd_global(intx,  StackYellowPages,     2);
define_pd_global(intx,  StackRedPages,        1);
define_pd_global(intx,  StackShadowPages,     5 LP64_ONLY(+1) DEBUG_ONLY(+3));

define_pd_global(bool,  RewriteBytecodes,     true);
define_pd_global(bool,  RewriteFrequentPairs, true);
