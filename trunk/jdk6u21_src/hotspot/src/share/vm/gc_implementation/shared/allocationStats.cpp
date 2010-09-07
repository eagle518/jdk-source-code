/*
 * Copyright (c) 2005, 2008, Oracle and/or its affiliates. All rights reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_allocationStats.cpp.incl"

// Technically this should be derived from machine speed, and
// ideally it would be dynamically adjusted.
float AllocationStats::_threshold = ((float)CMS_SweepTimerThresholdMillis)/1000;
