/*
 * @(#)GeneralPath.cpp	1.8 04/05/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GeneralPath.h"
#include "fontscalerdefs.h"
#include "sunfontids.h"

#include <stdlib.h>

static const jint DEFAULT_LEN_TYPES = 10;
static const jint DEFAULT_LEN_COORDS = 50;

static const jint TYPES_GROW_SIZE = 15;
static const jint TYPES_GROW_MASK = 0xfffffff0;

static const jint COORDS_GROW_SIZE = 31;
static const jint COORDS_GROW_MASK = 0xffffffe0;

GeneralPath::GeneralPath(jint windingRule)
  : pointTypes((jbyte*)malloc(sizeof(jbyte) * DEFAULT_LEN_TYPES))
  , pointCoords((jfloat*)malloc(sizeof(jfloat) * DEFAULT_LEN_COORDS))
  , numTypes(0)
  , numCoords(0)
  , lenTypes(DEFAULT_LEN_TYPES)
  , lenCoords(DEFAULT_LEN_COORDS)
  , wr(windingRule)
{
}

GeneralPath::~GeneralPath() 
{
  if (pointTypes) {
    free(pointTypes); 
    pointTypes = 0;
  }
  if (pointCoords) {
    free(pointCoords); 
    pointCoords = 0;
  }
}

jboolean
GeneralPath::needRoom(jint newTypes, jint newCoords) {
  jint reqTypes = numTypes + newTypes;
  if (reqTypes > lenTypes) {
    lenTypes = (reqTypes + TYPES_GROW_SIZE) & TYPES_GROW_MASK;
    pointTypes = (jbyte*)realloc(pointTypes, lenTypes * sizeof(jbyte));
  }

  jint reqCoords = numCoords + newCoords;
  if (reqCoords > lenCoords) {
    lenCoords = (reqCoords + COORDS_GROW_SIZE) & COORDS_GROW_MASK;
    pointCoords = (jfloat*)realloc(pointCoords, lenCoords * sizeof(jfloat));
  }

  return pointTypes && pointCoords;
}

jobject 
GeneralPath::getShape(JNIEnv *env) {
  jbyteArray types = env->NewByteArray(numTypes);
  jfloatArray coords = env->NewFloatArray(numCoords);

  if (types && coords) {
      env->SetByteArrayRegion(types, 0, numTypes, pointTypes);
      env->SetFloatArrayRegion(coords, 0, numCoords, pointCoords);
  
      return env->NewObject(sunFontIDs.gpClass, sunFontIDs.gpCtr, wr, types, numTypes, coords, numCoords);
  }

  return NULL;
}

#define MAX_FLOAT 3.4028235e+38f
#define MIN_FLOAT 1.4e-45f

jobject
GeneralPath::getBounds( JNIEnv *env) {
  jfloat left = MAX_FLOAT;
  jfloat top = MAX_FLOAT;
  jfloat right = MIN_FLOAT;
  jfloat bottom = MIN_FLOAT;
  jboolean moved = false;
  jboolean first = true;
  jint p = 0;

  //  fprintf(stderr, "gp::getBounds\n");
  //  fflush(stderr);

  // moveto's not followed by rendering (lineto, quadto, cubicto)
  // are not counted towards the bounds

  for (jint i = 0; i < numTypes; ++i) {
    jint ptype = pointTypes[i];
    switch (ptype) {
    case 0: // moveto
      moved = true;
      p += 2;
      break;
    case 1: // lineto
    case 2: // quadto
    case 3: // cubicto
      {
        if (moved) {
  	    moved = false;
	    if (first) {
	      first = false;
	      left = right = pointCoords[p-2];
	      top = bottom = pointCoords[p-1];
	    }
	}
	// ptype conveniently matches the number of points...
	for (int j = 0; j < ptype; ++j) { 
	  jfloat pt = pointCoords[p++];
	  if (pt < left) left = pt;
	  else if (pt > right) right = pt;

	  pt = pointCoords[p++];
	  if (pt < top)  top = pt;
	  else if (pt > bottom)  bottom = pt;
	}
      }
      break;
    case 4:
    default:
      break;
    }
  }

  //    fprintf(stderr, "gp ltrb %g %g %g %g\n", left, top, right-left, bottom-top);
  //    fflush(stderr);
  if (left >= right || top >= bottom) {
    return env->NewObject(sunFontIDs.rect2DFloatClass, sunFontIDs.rect2DFloatCtr);
  }
  return env->NewObject(sunFontIDs.rect2DFloatClass, sunFontIDs.rect2DFloatCtr4,
			left, top, right-left, bottom-top);
}

/**
static inline t2kScalar F26Dot6_To_Scalar(F26Dot6 value) {
    return (t2kScalar)value / (t2kScalar)64;
}

inline jfloat F26Dot6ToJFloat(F26Dot6 value) {
  return (jfloat)F26Dot6_To_Scalar(value);
}
*/

struct JPoint {
  jfloat x;
  jfloat y;
};

class Walker {
public:
    const GlyphClass& glyph;
    int contourIndex;
    int start;
    int limit;
    int index0;
    int index1;
    jfloat x;
    jfloat y;
    bool closed;
    bool done;

    Walker(const GlyphClass& _glyph, jfloat _x, jfloat _y, bool _closed)
        : glyph(_glyph)
        , contourIndex(0)
        , start(0)
        , limit(0)
        , index0(0)
        , index1(0)
        , x(_x)
        , y(_y)
        , closed(_closed)
        , done(true)
    {
    }

    bool nextContour() {
        while (contourIndex < glyph.contourCount) {
            start = glyph.sp[contourIndex];
            limit = glyph.ep[contourIndex] + 1;
            ++contourIndex;
            if (limit - start > 2) {
                index0 = start;
                index1 = start + 1;
                done = false;
                return true;
            }
        }
        return false;
    }

    inline bool doneWithContour() const {
        return done;
    }

    void next() {
        index0 = index1;
        if (++index1 == limit) {
            index1 = start;
            if (!closed) {
                done = true;
            }
        }
        if (index0 == start) {
            done = true;
        }
    }

    inline bool currentOnCurve() const {
        return glyph.onCurve[index0];
    }

    inline bool nextOnCurve() const {
        return glyph.onCurve[index1];
    }

    inline void setCurrent(JPoint& pt) const {
      //      jint p = glyph.x[index0];
      //      fprintf(stderr, "sizeof t2kScalar: %d\n", sizeof(t2kScalar));
      //      fprintf(stderr, "index[%d] ", index0);
      //      fprintf(stderr, "orig: 0x%lx ", (jint)p);
      //      fprintf(stderr, "t2ks: 0x%lx ",(jint)t2kScalar(p));
      //      fprintf(stderr, "64x: 0x%lx ", (jint)64);
      //      fprintf(stderr, "fts: 0x%lx ", (jint)F26Dot6_To_Scalar(p));
      //      fprintf(stderr, "ftf: 0x%lx ", (jint)F26Dot6ToJFloat(p));
      //      fprintf(stderr, "val: %g\n", F26Dot6ToJFloat(p));

        pt.x = x + F26Dot6ToScalar(glyph.x[index0]);
        pt.y = y - F26Dot6ToScalar(glyph.y[index0]);
    }

    inline void setNext(JPoint& pt) const {
        pt.x = x + F26Dot6ToScalar(glyph.x[index1]);
        pt.y = y - F26Dot6ToScalar(glyph.y[index1]);
    }

    inline void setAverage(JPoint& pt) const {
        t2kScalar p0 = F26Dot6ToScalar(glyph.x[index0]);
        t2kScalar p1 = F26Dot6ToScalar(glyph.x[index1]);
        pt.x = x + t2kScalarAverage(p0, p1);

        p0 = F26Dot6ToScalar(glyph.y[index0]);
        p1 = F26Dot6ToScalar(glyph.y[index1]);
        pt.y = y - t2kScalarAverage(p0, p1);
    }
};

void addGlyphToGeneralPath(const GlyphClass& glyph, GeneralPath& path,
				  jfloat x, jfloat y, bool quadratic) {
  bool debug = false;

    Walker walker(glyph, x, y, quadratic); // quadratics are always 'closed'
    if (debug) fprintf(stderr, "\nwalker quad: %c\n", quadratic ? 't' : 'f');
    while (walker.nextContour()) {
        JPoint a, b, c, d;
        walker.setCurrent(a);
	/* "firstTime" is necessary because we can't just moveto the first
	 * point as its not necessarily on the curve. This is known to be
	 * true of Solaris JA fonts, and probably many others
	 */
	bool firstTime = true;
        do {
            if (quadratic) {
                if (walker.currentOnCurve()) {
                  if (debug) fprintf(stderr, "currentOnCurve\n");
                    walker.setCurrent(a);
                } else if (walker.nextOnCurve()) {
                  if (debug) fprintf(stderr, "nextOnCurve\n");
                    walker.setNext(a);
                    walker.next();
                } else {
                  if (debug) fprintf(stderr, "average\n");
                    walker.setAverage(a);
                }
		if (firstTime) {
		    if (debug) {
		        fprintf(stderr, "moveto %g %g\n", a.x, a.y);
		    }
		    path.moveTo(a.x, a.y);
		    firstTime = false;
		}
                walker.next();
                if (debug) fprintf(stderr, "next\n");
                walker.setCurrent(b);
                if (debug) fprintf(stderr, "setCurrent\n");
                if (walker.currentOnCurve()) {
                  if (debug) fprintf(stderr, "currentOnCurve, lineto %g %g\n", b.x, b.y);
                    path.lineTo(b.x, b.y);
                    continue;
                }
                if (walker.nextOnCurve()) {
                    if (debug) fprintf(stderr, "nextOnCurve\n");
                    walker.setNext(c);
                    if (debug) fprintf(stderr, "setNext\n");
                    walker.next();
                    if (debug) fprintf(stderr, "next\n");
                } else {
                    walker.setAverage(c);
                    if (debug) fprintf(stderr, "setAverage\n");
                }
                if (debug) fprintf(stderr, "quadto %g %g %g %g\n", b.x, b.y, c.x, c.y);
                path.quadTo(b.x, b.y, c.x, c.y);
            } else { // cubic
		/* REMIND: this looks like an infinite loop if there's
		 * never a point on the curve.
		 */
                while (!walker.currentOnCurve()) {
                    walker.next();
                }
                walker.setCurrent(a);
		if (firstTime) {
		    if (debug) {
		        fprintf(stderr, "moveto %g %g\n", a.x, a.y);
		    }
		    path.moveTo(a.x, a.y);
		    firstTime = false;
		}		
                walker.next();
                walker.setCurrent(b);
                if (walker.currentOnCurve()) {
                    path.lineTo(b.x, b.y);
                    continue;
                }
                walker.next();
                walker.setCurrent(c);
                walker.next();
                walker.setCurrent(d);
                path.curveTo(b.x, b.y, c.x, c.y, d.x, d.y);
            }
        } while (!walker.doneWithContour());
        path.closePath();
        //        fflush(stderr);
    }
}
