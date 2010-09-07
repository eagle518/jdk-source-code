/*
 * @(#)FontGlue.h	1.30 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Charlton Innovations, Inc.
 */
#ifndef fontGlue_header
#define fontGlue_header

#include <jni.h>
#include <jni_util.h>
#include <jlong_md.h>

#include <memory.h>

class FontTransform {
public:
    jdouble fMatrix[4];
private:
    FontTransform& operator =(const FontTransform&);
public:
    FontTransform() {
        fMatrix[0] = 1;
        fMatrix[1] = 0;
        fMatrix[2] = 0;
        fMatrix[3] = 1;
    }
    FontTransform(jdouble m00, jdouble m10, jdouble m01, jdouble m11) {
	fMatrix[0] = m00;
	fMatrix[1] = m10;
	fMatrix[2] = m01;
	fMatrix[3] = m11;
    }
    FontTransform(float *matrix) {
        if (matrix) {
            fMatrix[0] = matrix[0];
            fMatrix[1] = matrix[1];
            fMatrix[2] = matrix[2];
            fMatrix[3] = matrix[3];
        }
    }
    FontTransform(JNIEnv *env, jdoubleArray matRef) {
        jint matSize = env->GetArrayLength(matRef);
        if (matSize >= 4) {
            env->GetDoubleArrayRegion(matRef, 0, 4, fMatrix);
        }
    }
    ~FontTransform() {
    }
    void getMatrixInto(float *dest, unsigned int destSize) {
        if (dest && (destSize >= 4)) {
            dest[0] = fMatrix[0];
            dest[1] = fMatrix[1];
            dest[2] = fMatrix[2];
            dest[3] = fMatrix[3];
        } else if (dest) {
            memset(dest, 0, destSize * sizeof(*dest));
        }
    }
    int equals(const FontTransform& cTX) const {
        return fMatrix
            && cTX.fMatrix
            && fMatrix[0] == cTX.fMatrix[0]
            && fMatrix[1] == cTX.fMatrix[1]
            && fMatrix[2] == cTX.fMatrix[2]
            && fMatrix[3] == cTX.fMatrix[3];
    }
    FontTransform(FontTransform& cTX) {
        fMatrix[0] = cTX.fMatrix[0];
        fMatrix[1] = cTX.fMatrix[1];
        fMatrix[2] = cTX.fMatrix[2];
        fMatrix[3] = cTX.fMatrix[3];
    }
};

#endif // fontGlue_header
