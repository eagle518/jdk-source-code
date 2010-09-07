/*
 * Copyright (c) 1999, 2006, Oracle and/or its affiliates. All rights reserved.
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

// ciCallProfile
//
// This class is used to determine the frequently called method
// at some call site
class ciCallProfile : StackObj {
private:
  // Fields are initialized directly by ciMethod::call_profile_at_bci.
  friend class ciMethod;

  enum { MorphismLimit = 2 }; // Max call site's morphism we care about
  int  _limit;                // number of receivers have been determined
  int  _morphism;             // determined call site's morphism
  int  _count;                // # times has this call been executed
  int  _receiver_count[MorphismLimit + 1]; // # times receivers have been seen
  ciMethod* _method[MorphismLimit + 1];    // receivers methods
  ciKlass*  _receiver[MorphismLimit + 1];  // receivers (exact)

  ciCallProfile() {
    _limit = 0;
    _morphism    = 0;
    _count = -1;
    _receiver_count[0] = -1;
    _method[0]   = NULL;
    _receiver[0] = NULL;
  }

  void add_receiver(ciKlass* receiver, int receiver_count);

public:
  // Note:  The following predicates return false for invalid profiles:
  bool      has_receiver(int i) { return _limit > i; }
  int       morphism()          { return _morphism; }

  int       count()             { return _count; }
  int       receiver_count(int i)  {
    assert(i < _limit, "out of Call Profile MorphismLimit");
    return _receiver_count[i];
  }
  float     receiver_prob(int i)  {
    assert(i < _limit, "out of Call Profile MorphismLimit");
    return (float)_receiver_count[i]/(float)_count;
  }
  ciMethod* method(int i)          {
    assert(i < _limit, "out of Call Profile MorphismLimit");
    return _method[i];
  }
  ciKlass*  receiver(int i)        {
    assert(i < _limit, "out of Call Profile MorphismLimit");
    return _receiver[i];
  }
};
