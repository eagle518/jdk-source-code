/*
 * Copyright (c) 2001, 2007, Oracle and/or its affiliates. All rights reserved.
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

  // Total virtual time so far.
inline double ConcurrentMarkThread::vtime_accum() {
  return _vtime_accum + _cm->all_task_accum_vtime();
}

// Marking virtual time so far
inline double ConcurrentMarkThread::vtime_mark_accum() {
  return _vtime_mark_accum + _cm->all_task_accum_vtime();
}
