/*
 * @(#)GeneralPath.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __GeneralPath_header
#define __GeneralPath_header

#include <jni.h>
#include <jni_util.h>
#include <sunt2kscaler.h>
#include <sunfontids.h>

struct GeneralPath {
  jbyte* pointTypes;
  jfloat* pointCoords;
  jint numTypes;
  jint numCoords;
  jint lenTypes;
  jint lenCoords;
  jint wr;

    enum {
       WIND_EVEN_ODD = 0,
       WIND_NON_ZERO = 1
    };

  GeneralPath(jint windingRule = (jint)WIND_NON_ZERO); 

  ~GeneralPath();

  jboolean needRoom(jint newTypes, jint newCoords);

  void moveTo(jfloat x, jfloat y) {
    if (needRoom(1, 2)) {
      pointTypes[numTypes++] = 0; // SEG_MOVETO
      pointCoords[numCoords++] = x;
      pointCoords[numCoords++] = y;
    }
  }

  void lineTo(jfloat x, jfloat y) {
    if (needRoom(1, 2)) {
      pointTypes[numTypes++] = 1; // SEG_LINETO
      pointCoords[numCoords++] = x;
      pointCoords[numCoords++] = y;
    }
  }

  void quadTo(jfloat x0, jfloat y0, jfloat x1, jfloat y1) {
    if (needRoom(1, 4)) {
      pointTypes[numTypes++] = 2; // SEG_QUADTO
      pointCoords[numCoords++] = x0;
      pointCoords[numCoords++] = y0;
      pointCoords[numCoords++] = x1;
      pointCoords[numCoords++] = y1;
    }
  }

  void curveTo(jfloat x0, jfloat y0, jfloat x1, jfloat y1, jfloat x2, jfloat y2) {
    if (needRoom(1, 6)) {
      pointTypes[numTypes++] = 3; // SEG_CUBICTO
      pointCoords[numCoords++] = x0;
      pointCoords[numCoords++] = y0;
      pointCoords[numCoords++] = x1;
      pointCoords[numCoords++] = y1;
      pointCoords[numCoords++] = x2;
      pointCoords[numCoords++] = y2;
    }
  }
      
  void closePath() {
    if (needRoom(1, 0)) {
      pointTypes[numTypes++] = 4; // SEG_CLOSE
    }
  }

  jobject getBounds(JNIEnv* env);

  jobject getShape(JNIEnv* env);
};

void addGlyphToGeneralPath(const GlyphClass& glyph, GeneralPath& path,
			   jfloat x, jfloat y, bool quadratic);

// __GeneralPath_header
#endif
