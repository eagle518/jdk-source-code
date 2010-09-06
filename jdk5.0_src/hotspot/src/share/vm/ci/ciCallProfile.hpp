#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciCallProfile.hpp	1.8 04/03/02 02:08:39 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciCallProfile
//
// This class is used to determine the frequently called method
// at some call site
class ciCallProfile : StackObj {
private:
  // Fields are initialized directly by ciMethod::call_profile_at_bci.
  friend class ciMethod;

  int       _count;             // # times has this call been executed
  int       _receiver_count;    // # times has the majority type been seen
  ciMethod* _method;            // the majority method
  ciKlass*  _receiver;          // the majority receiver type (exact)

  ciCallProfile() {
    _count = -1;
    _receiver_count = -1;
    _method = NULL;
    _receiver = NULL;
  }

public:
  bool      is_valid()          { return _count >= 0; }
  // Note:  The following predicates return false for invalid profiles:
  bool      has_receiver()      { return _receiver != NULL; }
  bool      is_monomorphic()    { return _receiver && _receiver_count >= _count; }
  
  int       count()             { return _count; }
  int       receiver_count()    { return _receiver_count; }
  ciMethod* method()            { return _method; }
  ciKlass*  receiver()          { return _receiver; }

  void apply_prof_factor(float prof_factor);
};
