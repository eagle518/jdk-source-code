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

// During the development of the JDK 1.4 reflection implementation
// based on dynamic bytecode generation, it was hoped that the bulk of
// the native code for reflection could be removed. Unfortunately
// there is currently a significant cost associated with loading the
// stub classes which impacts startup time. Until this cost can be
// reduced, the JVM entry points JVM_InvokeMethod and
// JVM_NewInstanceFromConstructor are still needed; these and their
// dependents currently constitute the bulk of the native code for
// reflection. If this cost is reduced in the future, the
// NativeMethodAccessorImpl and NativeConstructorAccessorImpl classes
// can be removed from sun.reflect and all of the code guarded by this
// flag removed from the product build. (Non-product builds,
// specifically the "optimized" target, would retain the code so they
// could be dropped into earlier JDKs for comparative benchmarking.)

//#ifndef PRODUCT
# define SUPPORT_OLD_REFLECTION
//#endif
