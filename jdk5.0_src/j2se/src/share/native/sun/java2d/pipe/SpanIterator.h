/*
 * @(#)SpanIterator.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _Included_SpanIterator
#define _Included_SpanIterator

/*
 * This structure defines the methods used to communicate with a
 * Java SpanIterator at the native level.
 */
typedef struct {
    /**
     * Init and return native data
     */
    void     *(*open)(JNIEnv *env, jobject iterator);

    /**
     * End iteration, dispose data
     */
    void      (*close)(JNIEnv *env, void *clientData);

    /**
     * See SpanIterator.getPathBox()
     */
    void      (*getPathBox)(JNIEnv *env, void *clientData, jint pathbox[]);

    /**
     * See ShapeSpanIterator.ShapeSIIntersectClipBox
     */
    void      (*intersectClipBox)(JNIEnv *env, void *clientData,
				    jint lox, jint loy, jint hix, jint hiy);

    /**
     * See SpanIterator.nextSpan()
     */
    jboolean  (*nextSpan)(void *clientData, jint spanbox[]);

    /**
     * See SpanIterator.skipDownTo()
     */
    void      (*skipDownTo)(void *clientData, jint y);
} SpanIteratorFuncs;

#endif
