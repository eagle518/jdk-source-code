/*
 * Copyright (c) 2003, 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include "incls/_precompiled.incl"
#include "incls/_gcStats.cpp.incl"

GCStats::GCStats() {
    _avg_promoted       = new AdaptivePaddedNoZeroDevAverage(
                                                  AdaptiveSizePolicyWeight,
                                                  PromotedPadding);
}

CMSGCStats::CMSGCStats() {
    _avg_promoted       = new AdaptivePaddedNoZeroDevAverage(
                                                  CMSExpAvgFactor,
                                                  PromotedPadding);
}
