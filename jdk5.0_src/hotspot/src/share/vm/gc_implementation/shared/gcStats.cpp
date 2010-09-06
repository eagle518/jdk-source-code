#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gcStats.cpp	1.2 03/12/23 16:40:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_gcStats.cpp.incl"

GCStats::GCStats() {
    _avg_promoted       = new AdaptivePaddedAverage(
                                                  AdaptiveSizePolicyWeight,
                                                  PromotedPadding);
}

CMSGCStats::CMSGCStats() {
    _avg_promoted       = new AdaptivePaddedAverage(
                                                  CMSExpAvgFactor,
                                                  PromotedPadding);
}
