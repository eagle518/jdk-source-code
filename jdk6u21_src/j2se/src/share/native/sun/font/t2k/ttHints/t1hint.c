/*
 * @(#)t1hint.c	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include"t2k.h"
#include"t1hint.h"

/* #define DEBUG_T1HINTING */

/* scales logical coordinates */
#define SCALE_Y(v)  (((int) (v))*t->yPixelsPerEm*64/t->font->T1->upem)
#define SCALE_X(v)  (((int) (v))*t->xPixelsPerEm*64/t->font->T1->upem)

/* scales val proportionally change of min/max values */ 
#define RESCALE(val, oldMin, oldMax, newMin, newMax)  \
    ((oldMax) == (oldMin) ? (newMax) : \
    ((newMin)+((val) - (oldMin))*((newMax)-(newMin))/((oldMax)-(oldMin))))

/* shifts p accordingly movement of another point */
#define SHIFT(p, orig, new) ((p) + ((new) - (orig)))

#define FLOOR(v)        ((v) & ~63)
#define PROUND(v)       FLOOR((v)+32) /* rounds point to nearest pixel boundary */
#define MIDDLE(x, y)    (((x)+(y)+1) >> 1)

/* calculates middle point of stem */
#define STEM_MIDDLE(stemsArray, j)  MIDDLE((stemsArray)[(j)*2], (stemsArray)[2*(j)+1])

/* Contours in type1 fonts are in counterclockwise direction but 
internal t2k state assumes countours are clockwise.
To resolve this T1 contours are flipped while loading them into glyph structure.
However, stem frames are defined on ranges of original point indices and 
therefore we need to convert them back.
This macro does this and should be kept consistent with FlipContourDirection() logic.
*/
#define GET_FLIPPED_POINT(contourStart, contourEnd, pt) \
    ((pt) == (contourStart) ? (pt) : ((contourEnd) - ((pt) - (contourStart) - 1)))


/* Returns id of top alignment zone that includes point y.
Returns -1 if no zone were found. */
static int find_top_range(T2K *t, short y) {
    int i;
    /* NB: first zone in blueValues is bottom zone - we need to skip it */
    for(i=2; i<t->font->T1->blueValuesCnt; i+=2) {
        if ((t->font->T1->blueValues[i] - t->font->T1->blueFuzz) <= y 
            && (t->font->T1->blueValues[i+1]  + t->font->T1->blueFuzz) >= y) {
                return i;
            }
    }
    return -1;
}

/* Returns id of bottom alignment zone that includes point y.
Returns -1 if no zone were found. */
static int find_bottom_range(T2K *t, short y) {
    int i;
    /* NB: first zone from blueValues that is also bottom zone is replicated in the otherBlues */
    for(i=0; i<t->font->T1->otherBluesCnt; i+=2) {
        if ((t->font->T1->otherBlues[i] - t->font->T1->blueFuzz) <= y 
            && (t->font->T1->otherBlues[i+1]  + t->font->T1->blueFuzz) >= y) {
                return (i + t->font->T1->blueValuesCnt);
            }
    }
    return -1;
}

/* Returns id of alignment zone that includs point y.
First checks top zones. 
Returns -1 if no zone were found. */
static int find_range(T2K *t, short y) {
    int r = find_top_range(t, y);

    if (r == -1) {
        r = find_bottom_range(t, y);
    }
    return r;
}

/* Returns point "representing" given alignment zone.
We use these points to adjust stems and contour points. */
static F26Dot6 get_range_point(T2K *t, int range) {
    /* we should not ever return default value 
    - so initialize with something really weird to detect problem */
    F26Dot6 ret = -10000; 

    /* We use "flat" side of the alignment zone
       (bottom for top zones and top for bottom zones)
       as reference point to be bound to the pixel grid. */
    if (range >= 0 && range < t->font->T1->blueValuesCnt) { /* id of top zone */        
        ret = SCALE_Y(t->font->T1->blueValues[range]);
        ret = PROUND(ret); /* round to pixel grid */
    } else {
        range -= t->font->T1->blueValuesCnt;
        if (range >= 0 && range < t->font->T1->otherBluesCnt) { /* id of bottom zone */
            ret = SCALE_Y(t->font->T1->otherBlues[range+1]);
            ret = PROUND(ret); 
        }
    }
    return ret;
}

/* Returns stem legnth in the hinted glyph.
NB: We assume that length is integer at some places. */
F26Dot6 get_recommendedStemLength(F26Dot6 start, F26Dot6 end) {
    F26Dot6 res = end - start;

    /* these rules are empirical and might neen refinement */
    if (res == 0) {
        res = 0;
    } else if (res <= 64) {
        res = 64; /* one pixel */
    } else {
        res = PROUND(res);
    }
    return res;
}

/* Given position of stem lower (bottom/left) end and stem length 
   choose best adjustment to be made. */
static F26Dot6 placeStemLowerEnd(F26Dot6 middle, F26Dot6 stemLen) {
    F26Dot6 optimal;
    /* Assuming that stem lenght is integral we try to 
       place it to have both ends on the pixel grid. */
    if (stemLen & 64) { /* if stem length is odd number of pixels */
        /* pick closest pixel center */
        optimal = FLOOR(middle) + 32;
    } else {
        /* pick closest pixel border */
        optimal = PROUND(middle);
    }
    return optimal - (stemLen >> 1);
}

/* This procedure process all stems associated with glyph.
I.e. it determines best dimensions and placement for these stems
taking into account actual size and alignment zones.

As a result number of derived values in glyph stem frames are set.
*/
static void process_stems(GlyphClass *glyph, T2K *t) {
    int i, j;
    int firstPt, lastPt;
    int lowerFixedStem, upperFixedStem, rangeB, rangeT;
    F26Dot6 stemLen;
    T1StemsFrame *frame;

    for (i=0; i<glyph->stemFramesCount; i++) {
        frame = glyph->stemFrames + i;
        /* allocate memory for computed data */
        if (frame->hstemsCount > 0 && 
            frame->scaledHstems == NULL) {
                frame->scaledHstems = tsi_AllocMem(t->mem, 
                    sizeof(F26Dot6)*2*frame->hstemsCount);
                frame->fixedHstems  = tsi_AllocMem(t->mem, 
                    sizeof(char)*frame->hstemsCount);
        }
        if (frame->vstemsCount > 0 && 
            frame->scaledVstems == NULL) {
                frame->scaledVstems = tsi_AllocMem(t->mem, 
                    sizeof(F26Dot6)*2*frame->vstemsCount);
        }

        /* determine scope (set of contour points) of current stem frame */
        firstPt = frame->firstPointIndex;
        if (i < glyph->stemFramesCount-1) {
            lastPt = glyph->stemFrames[i+1].firstPointIndex;
        } else {
            lastPt = glyph->pointCount;
        }

        /* check for top and bottom alignment zones
           and determine max/min coordinates in the scaled space.
           If we find alignment zone then we take its coordinate 
           and mark frame top/bottom fixed.
           NB: we should NOT have both ends fixed. */
        rangeT = find_top_range(t, frame->maxOOY);
        if (rangeT != -1) { /* FOUND ZONE */
            frame->maxY = get_range_point(t, rangeT);
            frame->isFixedTop = 1;
        } else {
            frame->maxY = SCALE_Y(frame->maxOOY);
            frame->isFixedTop = 0;
        }

        rangeB = find_bottom_range(t, frame->minOOY);
        if (rangeB != -1) {
            frame->minY = get_range_point(t, rangeB);
            frame->isFixedBottom = 1;
        } else {
            frame->minY = SCALE_Y(frame->minOOY);
            frame->isFixedBottom  = 0;
        }

        /* Now process stems in this frame and 
           calculate their representation in the scaled space */

        if (frame->hstemsCount > 0) {
            /* on the first scan place fixed stems */
            lowerFixedStem = -1;
            upperFixedStem = -1;
            /* after this scan we will have "fixed" stems 
               (i.e. ones that has one of the ends in the alignment zone) 
               scaled and placed */
            for(j=0; j < frame->hstemsCount; j++) {
                frame->scaledHstems[j*2] = SCALE_Y(frame->hstems[j*2]);
                frame->scaledHstems[j*2+1] = SCALE_Y(frame->hstems[j*2+1]);
                frame->fixedHstems[j] = 0;

                stemLen = get_recommendedStemLength(
                          frame->scaledHstems[j*2], frame->scaledHstems[j*2+1]);

                /* if stem edge falls into alignment zone 
                   then it should be processed in special way */
                rangeB = find_range(t, frame->hstems[j*2]);
                rangeT = find_range(t, frame->hstems[j*2+1]);

                if (rangeB != -1 && rangeT != -1) {
                    /* This actually should not happen - both ends can not be fixed.
                       For now handle it in same way as "fixed top" */
                    frame->scaledHstems[j*2+1] = get_range_point(t, rangeT);
                    frame->scaledHstems[j*2] = frame->scaledHstems[j*2+1] - stemLen;
                    frame->fixedHstems[j] = 1;
                } else if (rangeB != -1) {
                    /* fixed bottom end */
                    frame->scaledHstems[j*2] = get_range_point(t, rangeB);
                    frame->scaledHstems[j*2+1] = frame->scaledHstems[j*2] + stemLen;
                    frame->fixedHstems[j] = 1;
                } else if (rangeT != -1) {
                    /* fixed top end */
                    frame->scaledHstems[j*2+1] = get_range_point(t, rangeT);
                    frame->scaledHstems[j*2] = frame->scaledHstems[j*2+1] - stemLen;
                    frame->fixedHstems[j] = 1;
                }

                /* NB: assume stems are sorted from lower to upper */
                if (frame->fixedHstems[j] == 1) {
                    /* we will need lowest and uppermost fixed stems later on */
                    if (lowerFixedStem == -1) {
                         lowerFixedStem = j;
                    }
                    upperFixedStem = j;                     
                }
            }               

            /* Lowest and uppermost stems should be fixed to process other stems. */ 
            /* if upperpmost stem is not fixed yet ... */
            if (upperFixedStem != frame->hstemsCount-1) {
                frame->fixedHstems[frame->hstemsCount-1] = 1;
                j = frame->hstemsCount-1;
                stemLen = get_recommendedStemLength(
                              frame->scaledHstems[2*j], frame->scaledHstems[2*j+1]);
                frame->scaledHstems[j*2]   = 
                    placeStemLowerEnd(STEM_MIDDLE(frame->scaledHstems, j), stemLen);
                frame->scaledHstems[j*2+1] = frame->scaledHstems[j*2] + stemLen;
            }
            /* if lowermost stem is not fixed yet ... */
            if (frame->fixedHstems[0] != 1) {
                frame->fixedHstems[0] = 1;
                stemLen = get_recommendedStemLength(
                              frame->scaledHstems[0], frame->scaledHstems[1]);
                frame->scaledHstems[0] =
                    placeStemLowerEnd(STEM_MIDDLE(frame->scaledHstems, 0), stemLen);
                frame->scaledHstems[1] = frame->scaledHstems[0] + stemLen;
            }


            /*  Now we can place other stems by interpolating their positions 
                relative to closest fixed stems above and below. */
            upperFixedStem = lowerFixedStem = 0;   /* points to closest fixed stems above/below */
            for(j=1; j < frame->hstemsCount-1; j++) {  
                if (frame->fixedHstems[j] == 0) { /* got next non-fixed stem */
                    F26Dot6 optimal;

                    /* get closest fixed stem above - 
                       There always should be one because uppermost - stem was fixed.*/
                    if (upperFixedStem < j) {
                        upperFixedStem = j+1;
                        while (frame->fixedHstems[upperFixedStem] != 1 && 
                            upperFixedStem < frame->hstemsCount) 
                            upperFixedStem++;
                        }

                        /* Interpolate position  of stem middle */
                        stemLen = get_recommendedStemLength(
                                  frame->scaledHstems[j*2], frame->scaledHstems[j*2+1]);

                        /* The idea here is that we try to preserve proportions 
                           of the glyph by keeping proportions between stems.
                           Stem middles are least affected by hinting adjustments
                           - use them as basis for rescaling. */
                        optimal = RESCALE(STEM_MIDDLE(frame->hstems, j),
                                STEM_MIDDLE(frame->hstems, lowerFixedStem), 
                                STEM_MIDDLE(frame->hstems, upperFixedStem), 
                                STEM_MIDDLE(frame->scaledHstems, lowerFixedStem), 
                                STEM_MIDDLE(frame->scaledHstems, upperFixedStem));

                        frame->scaledHstems[j*2]   = placeStemLowerEnd(optimal, stemLen);;
                        frame->scaledHstems[j*2+1] = frame->scaledHstems[j*2] + stemLen;

                    }
                    lowerFixedStem = j;
                }

#ifdef DEBUG_T1HINTING
                for(j=0; j < frame->hstemsCount; j++) {
                    printf("HStem %d-%d: (%d, %d, %d) -> (%d, %d, %d) -> (%d, %d, %d)\n", 
                            i, j, frame->hstems[j*2], 
                            frame->hstems[j*2+1], 
                            frame->hstems[j*2+1] - frame->hstems[j*2],
                            SCALE_Y(frame->hstems[j*2]), 
                            SCALE_Y(frame->hstems[j*2+1]), 
                            SCALE_Y(frame->hstems[j*2+1] - frame->hstems[j*2]), 
                            frame->scaledHstems[j*2], 
                            frame->scaledHstems[j*2+1], stemLen);
                }
#endif
        }        

        /* now process vstems:
           No alignment zones are involved but to preserve relationships 
           between stems we place leftmost and rightmost stems first.
           Other stems locations are rescaled to preserve glyph proportions. */
        if (frame->vstemsCount > 0) {
            F26Dot6 min, max, scaledMin, scaledMax, optimal;

            min = STEM_MIDDLE(frame->vstems, 0);
            max = STEM_MIDDLE(frame->vstems, frame->vstemsCount-1);

            /* place leftmost stem */
            stemLen = get_recommendedStemLength(
                           SCALE_X(frame->vstems[0]), SCALE_X(frame->vstems[1]));
            frame->scaledVstems[0] = placeStemLowerEnd(SCALE_X(min), stemLen);
            frame->scaledVstems[1] = frame->scaledVstems[0] + stemLen;

            /* place rightmost stem */
            if (frame->vstemsCount > 1) {
                  j = frame->vstemsCount-1;
                stemLen = get_recommendedStemLength(
                           SCALE_X(frame->vstems[j*2]), SCALE_X(frame->vstems[j*2+1]));
                frame->scaledVstems[j*2]   = placeStemLowerEnd(SCALE_X(max), stemLen);
                frame->scaledVstems[j*2+1] = frame->scaledVstems[j*2] + stemLen;
            }
            
            scaledMin = STEM_MIDDLE(frame->scaledVstems, 0);
            scaledMax = STEM_MIDDLE(frame->scaledVstems, frame->vstemsCount-1);

            /* Find best places for other stems. */
            for(j=1; j < frame->vstemsCount-1; j++) {
                frame->scaledVstems[j*2] = SCALE_X(frame->vstems[j*2]);
                frame->scaledVstems[j*2+1] = SCALE_X(frame->vstems[j*2+1]);
                stemLen = get_recommendedStemLength(
                              frame->scaledVstems[j*2], frame->scaledVstems[j*2+1]);

                optimal = RESCALE(STEM_MIDDLE(frame->vstems, j),
                                min, max, scaledMin, scaledMax);
                frame->scaledVstems[j*2]   = placeStemLowerEnd(STEM_MIDDLE(frame->scaledVstems, j), stemLen);
                frame->scaledVstems[j*2+1] = frame->scaledVstems[j*2] + stemLen;
            }
#ifdef DEBUG_T1HINTING
            for(j=0; j < frame->vstemsCount; j++) {
                printf("VStem %d-%d: (%d, %d, %d) -> (%d, %d, %d) -> (%d, %d, %d)\n",
                            i, j, frame->vstems[j*2], 
                            frame->vstems[j*2+1], 
                            frame->vstems[j*2+1] - frame->vstems[j*2], 
                            SCALE_X(frame->vstems[j*2]), 
                            SCALE_X(frame->vstems[j*2+1]), 
                            SCALE_X(frame->vstems[j*2+1] - frame->vstems[j*2]),
                            frame->scaledVstems[j*2], 
                            frame->scaledVstems[j*2+1], 
                            stemLen);
            }
#endif
        }
    }  
}

static void place_points(GlyphClass *glyph, T2K *t) {
    int  stemId;
    F26Dot6 min, max;
    int i, j, idx, c;
    F26Dot6 accumulatedDeltaX=0, accumulatedDeltaY=0; 
    T1StemsFrame *frame;

    for (c=0; c<glyph->contourCount; c++) {
        for(i=glyph->sp[c]; i<=glyph->ep[c] /* && i < glyph->pointCount */; i++) {

            idx = GET_FLIPPED_POINT(glyph->sp[c], glyph->ep[c], i);

            /* find appropriate stem frame */
            for(stemId=0; stemId<glyph->stemFramesCount && 
                          glyph->stemFrames[stemId].firstPointIndex <= idx; stemId++) {}
            /* note: stemId != 0 here because this means we had no stem frames.
                     But this can not be - we are inserting dummy frame to prevent it */
            frame = glyph->stemFrames+stemId-1;

#ifdef DEBUG_T1HINTING
            printf("======== Process point %d (oncurve=%d, idx=%d, frame=%d) [%d, %d -> %d, %d]\n", i, 
                glyph->onCurve[i], idx, stemId-1, glyph->oox[i], glyph->ooy[i], glyph->x[i], glyph->y[i]);
#endif

            /***************** Update Y coordinates *******************/

            /*
            There are number of different cases here. 
            Most important factor is location of point relative to available stems:
            1. inside the stem range
            2. fall between two stems
            3. below all stems
            4. above all stems
            5. there are no stems
            Depending on the other conditions (whether top or bottom is fixed, 
            if point hits stem end proint) we either rescale or shift point vetical position.
            */

            /* first try to find nearby stem end points above/below */
            min = -1, max = -1;
            if (frame->scaledHstems != NULL) {
                for(j=0; j<frame->hstemsCount*2; j++) {
                    if (glyph->ooy[i] <= frame->hstems[j] && 
                        (max == -1 || frame->hstems[j] < frame->hstems[max])) {
                        max = j;
                    }
                    if (glyph->ooy[i] >= frame->hstems[j] && 
                        (min == -1 || frame->hstems[j] > frame->hstems[min])) {
                        min = j;
                    }
                }

#ifdef DEBUG_T1HINTING
                printf("point %d (x=%d, y=%d) min=%d (%d -> %d) max=%d (%d -> %d)\n", 
                    i, glyph->oox[i], glyph->ooy[i], 
                    min, (min == -1) ? frame->minOOY : frame->hstems[min],
                    (min == -1) ? frame->minY : frame->scaledHstems[min],
                    max, (max == -1) ? frame->maxOOY : frame->hstems[max], 
                    (max == -1) ? frame->maxY : frame->scaledHstems[max]);
#endif
            }

            if (min != -1 && max != -1) { /* have both ends - cases 1 and 2 */
                if (min == max) { /* special case: exactly match stem end point */        
                    glyph->y[i] = frame->scaledHstems[min];
                } else { /* scale proportionally */
#if 0               /* why we need this adjustment? - disable for now */
                    min &= ~1;
                    max |= 1;
#endif
                    glyph->y[i] = RESCALE(glyph->ooy[i],
                                          frame->hstems[min], frame->hstems[max], 
                                          frame->scaledHstems[min], frame->scaledHstems[max]);
                }
            } else if (min != -1) { /* have only stem below */
                if (frame->isFixedTop == 1) { /* if top is fixed - rescale */
                    glyph->y[i] = RESCALE(glyph->ooy[i], 
                                          frame->hstems[min], frame->maxOOY,
                                          frame->scaledHstems[min], frame->maxY);
                } else { /* shift scaled point */
                    glyph->y[i] = SHIFT(glyph->y[i],
                                        SCALE_Y(frame->hstems[min]),
                                        frame->scaledHstems[min]);
                }
            } else if (max != -1) { /* have only step above */
                if (frame->isFixedBottom == 1) { /* if bottom is fixed - rescale */
                    glyph->y[i] = RESCALE(glyph->ooy[i], 
                                          frame->minOOY, frame->hstems[max],
                                          frame->minY, frame->scaledHstems[max]);
                } else { /* shift scaled point */
                    glyph->y[i] = SHIFT(glyph->y[i], 
                                        SCALE_Y(frame->hstems[max]),
                                        frame->scaledHstems[max]);
                }
            } else {
                /* if we got here then there are no hstems defined for this point 
                but we might have had top/botom alignment zones.
                If so then max/min coordinates were adjusted.
                So, rescale proportionally changes of max/min coordinates. */
                glyph->y[i] = RESCALE(glyph->ooy[i], 
                                      frame->minOOY, frame->maxOOY, 
                                      frame->minY, frame->maxY);
            }
#ifdef DEBUG_T1HINTING
            printf(" -> %d [%f]\n", glyph->y[i], ((float) glyph->y[i])/64.0f);
#endif


            /******************** Update X coordinates ************************/

            /* Similiar to case of Y coordinates but simplier - 
               we have no alignment zones involved */
            min = -1, max = -1;
            if (frame->scaledVstems != NULL) {
                /* try to find nearby stem end points on the left/right */ 
                for(j=0; j<frame->vstemsCount*2; j++) {
                    if (glyph->oox[i] <= frame->vstems[j] && 
                        (max == -1 || frame->vstems[j] < frame->vstems[max])) {
                            max = j;
                        }
                        if (glyph->oox[i] >= frame->vstems[j] && 
                            (min == -1 || frame->vstems[j] > frame->vstems[min])) {
                                min = j;
                            }
                }


                if (min != -1 && max != -1) { /* have stem ends on both sides => rescale*/
                    if (min == max) { /* special case: exact match */
                        glyph->x[i] = frame->scaledVstems[min];
                    } else {
#if 0                   /* why we need this adjustment? - disable for now */
                        min &= ~1;
                        max |= 1;
#endif
                        glyph->x[i] = RESCALE(glyph->oox[i], 
                                              frame->vstems[min], frame->vstems[max],
                                              frame->scaledVstems[min], frame->scaledVstems[max]);
                    }
                } else if (min != -1) { /* one end fixed => shift */
                    glyph->x[i] = SHIFT(glyph->x[i], 
                                        SCALE_X(frame->vstems[min]), 
                                        frame->scaledVstems[min]);
                } else if (max != -1) {
                    glyph->x[i] = SHIFT(glyph->x[i], 
                                        SCALE_X(frame->vstems[max]), 
                                        frame->scaledVstems[max]);        
                } 
            }
        }
    }

    /* METRICS? */
    if (frame->scaledVstems != NULL) {
        /* if we adjusted X coordinates then we might need to adjust metrics.
           
           Probably we can be smarter but for now lets just shift right sidebearing point
           preserving distance to point with maximum X coordinate.
         */

        max = 0;
        for(j=1; j<glyph->pointCount; j++) {
            if (glyph->oox[j] > glyph->oox[max]) {
                max = j;
            }
        }
        glyph->x[glyph->pointCount+1] = SHIFT(glyph->x[glyph->pointCount+1], 
                                              SCALE_X(glyph->oox[max]), 
                                              glyph->x[max]);
    }
}

int Type1HintGlyph(GlyphClass *glyph, T2K *t) {
    if (glyph->pointCount == 0) {
        return 0;
    }
    /* Note: If number of points > 0 then we can assume 
         we have at least one stem frame */

    /* scale and place stems */
    process_stems(glyph, t);

    /* adjust contour points */
    place_points(glyph, t);

    return 0;
}

