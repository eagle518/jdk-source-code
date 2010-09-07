/*
 * @(#)JPLISAssert.h	1.2 03/09/08
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/*
 *  Super-cheesy assertions that aren't efficient when they are turned on, but
 *  are free when turned off (all pre-processor stuff)
 */


#ifndef _JPLISASSERT_H_
#define _JPLISASSERT_H_

#include    <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JPLISASSERT_ENABLEASSERTIONS    (1)


#ifndef JPLISASSERT_ENABLEASSERTIONS
#define JPLISASSERT_ENABLEASSERTIONS    (0)
#endif


#if JPLISASSERT_ENABLEASSERTIONS
#define jplis_assert(x)             JPLISAssertCondition(x, #x, __FILE__, __LINE__)
#define jplis_assert_msg(x, msg)    JPLISAssertConditionWithMessage(x, #x, msg, __FILE__, __LINE__)
#else
#define jplis_assert(x)
#define jplis_assert_msg(x, msg)
#endif

/*
 * Test the supplied condition.
 * If false, print a constructed message including source site info to stderr.
 * If true, do nothing.
 */
extern void
JPLISAssertCondition(   jboolean        condition,
                        const char *    assertionText,
                        const char *    file,
                        int             line);

/*
 * Test the supplied condition.
 * If false, print a constructed message including source site info
 * and the supplied message to stderr.
 * If true, do nothing.
 */
extern void
JPLISAssertConditionWithMessage(    jboolean        condition,
                                    const char *    assertionText,
                                    const char *    message,
                                    const char *    file,
                                    int             line);




#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif


