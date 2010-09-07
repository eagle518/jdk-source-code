/*
 * @(#)PerfLabel.h	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _PERF_LABEL_H_
#define _PERF_LABEL_H_

#include "jni.h"
#include "PerfLib.h"


class PERFLIB_API PerfLabel {

public:
    jobject getJavaObj(JNIEnv * pEnv) const;

    PerfLabel(void);
    PerfLabel(jlong time, const char * label);
    PerfLabel(const PerfLabel & src);

    PerfLabel & operator=(const PerfLabel & src);

//private:
    static const size_t LABEL_BUFFER_SIZE = 120;

    jstring labelToString(JNIEnv * pEnv) const;

    jlong m_time;
    char  m_label[LABEL_BUFFER_SIZE];
};

#endif    // _PERF_LABEL_H_
