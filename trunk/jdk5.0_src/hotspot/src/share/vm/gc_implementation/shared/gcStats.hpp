#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcStats.hpp	1.2 03/12/23 16:40:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class GCStats : public CHeapObj {
 protected:
  // Avg amount promoted; used for avoiding promotion undo
  // This class does not update deviations if the sample is zero.
  AdaptivePaddedAverage*   _avg_promoted;

 public:
  GCStats();

  AdaptivePaddedAverage*  avg_promoted() const { return _avg_promoted; }

  // Average in bytes
  size_t average_promoted_in_bytes() const {
    return (size_t)_avg_promoted->average();
  }

  // Padded average in bytes
  size_t padded_average_promoted_in_bytes() const {
    return (size_t)_avg_promoted->padded_average();
  }
};

class CMSGCStats : public GCStats {
 public:
  CMSGCStats();
};
