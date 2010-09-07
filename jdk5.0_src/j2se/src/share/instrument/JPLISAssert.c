/*
 * @(#)JPLISAssert.c	1.2 03/09/08
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#include    <jni.h>

#include    "JPLISAssert.h"

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/*
 *  Super-cheesy assertions that aren't efficient when they are turned on, but
 *  are free when turned off (all pre-processor stuff)
 */

void
JPLISAssertCondition(   jboolean        condition,
                        const char *    assertionText,
                        const char *    file,
                        int             line) {
    if ( !condition ) {
        fprintf(stderr, "*** java.lang.instrument ASSERTION FAILED ***: \"%s\" at %s line: %d\n",
                                            assertionText,
                                            file,
                                            line);
    }
}


void
JPLISAssertConditionWithMessage(    jboolean        condition,
                                    const char *    assertionText,
                                    const char *    message,
                                    const char *    file,
                                    int             line) {
    if ( !condition ) {
        fprintf(stderr, "*** java.lang.instrument ASSERTION FAILED ***: \"%s\" with message %s at %s line: %d\n",
                                            assertionText,
                                            message,
                                            file,
                                            line);
    }
}

