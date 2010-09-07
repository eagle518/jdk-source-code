/*
 * @(#)PerfLabel.cpp	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>
#include "PerfLabel.h"


jobject PerfLabel::getJavaObj(JNIEnv * pEnv) const {
    jobject result = NULL;

    // convert the PerfLabel text to a Java string
    jstring label = labelToString(pEnv);
    if (label != NULL) {
        // instantiate the Java object
        jclass clazz = pEnv->FindClass("com/sun/deploy/perf/PerfLabel");
        if (clazz != NULL) {
            jmethodID mid = pEnv->GetMethodID(clazz,
                                              "<init>",
                                              "(JLjava/lang/String;)V");
            if (mid != NULL) {
                result = pEnv->NewObject(clazz, mid, m_time, label);
            }
        }
    }

    return (result);
}

PerfLabel::PerfLabel(void)
    : m_time(0)
{
    // zero out the message buffer
    ::memset(m_label, '\0', LABEL_BUFFER_SIZE);
}

PerfLabel::PerfLabel(jlong time, const char * label)
    : m_time(time)
{
    // zero out the message buffer, then copy the input
    ::memset(m_label, '\0', LABEL_BUFFER_SIZE);
    ::strncpy(m_label, label, LABEL_BUFFER_SIZE);
}

PerfLabel::PerfLabel(const PerfLabel & src)
    : m_time(src.m_time)
{
    // copy the entire input buffer
    ::memcpy(m_label, src.m_label, LABEL_BUFFER_SIZE);
}

PerfLabel & PerfLabel::operator=(const PerfLabel & src)
{
    m_time = src.m_time;
    ::memcpy(m_label, src.m_label, LABEL_BUFFER_SIZE);

    return (*this);
}

jstring PerfLabel::labelToString(JNIEnv * pEnv) const {
    char         tmp[LABEL_BUFFER_SIZE + 1];
    const char * src    = m_label;

    if (m_label[LABEL_BUFFER_SIZE - 1] != '\0') {
        // this label uses the full member buffer, so need to copy it into a
        // temp buffer with a null-terminator
        ::memcpy(tmp, m_label, LABEL_BUFFER_SIZE);
        tmp[LABEL_BUFFER_SIZE] = '\0';
        src = tmp;
    }

    return (pEnv->NewStringUTF(src));
}
