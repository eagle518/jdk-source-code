/*
 * @(#)eventFilter.h	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_EVENTFILTER_H
#define JDWP_EVENTFILTER_H

#include "eventHandler.h"

/***** filter set-up *****/

jvmtiError eventFilter_setConditionalFilter(HandlerNode *node,
                                      jint index, jint exprID);
jvmtiError eventFilter_setCountFilter(HandlerNode *node,
                                jint index, jint count);
jvmtiError eventFilter_setThreadOnlyFilter(HandlerNode *node,
                                     jint index, jthread thread);
jvmtiError eventFilter_setLocationOnlyFilter(HandlerNode *node,
                                       jint index, 
                                       jclass clazz, 
                                       jmethodID method, 
                                       jlocation location);
jvmtiError eventFilter_setFieldOnlyFilter(HandlerNode *node,
                                    jint index, 
                                    jclass clazz, 
                                    jfieldID field);
jvmtiError eventFilter_setClassOnlyFilter(HandlerNode *node,
                                    jint index, 
                                    jclass clazz);
jvmtiError eventFilter_setExceptionOnlyFilter(HandlerNode *node,
                                        jint index, 
                                        jclass exceptionClass, 
                                        jboolean caught, 
                                        jboolean uncaught);
jvmtiError eventFilter_setInstanceOnlyFilter(HandlerNode *node,
                                       jint index, 
                                       jobject object);
jvmtiError eventFilter_setClassMatchFilter(HandlerNode *node,
                                     jint index, 
                                     char *classPattern);
jvmtiError eventFilter_setClassExcludeFilter(HandlerNode *node,
                                       jint index, 
                                       char *classPattern);
jvmtiError eventFilter_setStepFilter(HandlerNode *node,
                               jint index, 
                               jthread thread, 
                               jint size, jint depth);
jvmtiError eventFilter_setSourceNameMatchFilter(HandlerNode *node, 
                                                jint index, 
                                                char *sourceNamePattern);

/***** misc *****/

jboolean eventFilter_predictFiltering(HandlerNode *node, jclass clazz, char *classname);
jboolean isBreakpointSet(jclass clazz, jmethodID method, jlocation location);

#endif /* _EVENT_FILTER_H */
