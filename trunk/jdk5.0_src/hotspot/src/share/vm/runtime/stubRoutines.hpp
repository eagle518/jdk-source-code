#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)stubRoutines.hpp	1.103 04/02/25 22:15:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// StubRoutines provides entry points to assembly routines used by
// compiled code and the run-time system. Platform-specific entry
// points are defined in the platform-specific inner class.
//
// Class scheme:
//
//    platform-independent               platform-dependent
//
//    stubRoutines.hpp  <-- included --  stubRoutines_<arch>.hpp
//           ^                                  ^
//           |                                  |
//       implements                         implements
//           |                                  |
//           |                                  |
//    stubRoutines.cpp                   stubRoutines_<arch>.cpp
//    stubRoutines_<os_family>.cpp       stubGenerator_<arch>.cpp
//    stubRoutines_<os_arch>.cpp
//
// Note 1: The important thing is a clean decoupling between stub
//         entry points (interfacing to the whole vm; i.e., 1-to-n
//         relationship) and stub generators (interfacing only to
//         the entry points implementation; i.e., 1-to-1 relationship).
//         This significantly simplifies changes in the generator
//         structure since the rest of the vm is not affected.
//
// Note 2: stubGenerator_<arch>.cpp contains a minimal portion of
//         machine-independent code; namely the generator calls of
//         the generator functions that are used platform-independently.
//         However, it comes with the advantage of having a 1-file
//         implementation of the generator. It should be fairly easy
//         to change, should it become a problem later.
//
// Scheme for adding a new entry point:
//
// 1. determine if it's a platform-dependent or independent entry point
//    a) if platform independent: make subsequent changes in the independent files
//    b) if platform   dependent: make subsequent changes in the   dependent files
// 2. add a private instance variable holding the entry point address
// 3. add a public accessor function to the instance variable
// 4. implement the corresponding generator function in the platform-dependent
//    stubGenerator_<arch>.cpp file and call the function in generate_all() of that file


class StubRoutines: AllStatic {

 public:
  enum platform_independent_constants {
    max_size_of_parameters = 256                           // max. parameter size supported by megamorphic lookups
  };

  // Dependencies
  friend class StubGenerator;
  #include "incls/_stubRoutines_pd.hpp.incl"               // machine-specific parts

  static jint    _verify_oop_count;
  static address _verify_oop_subroutine_entry;

  static address _call_stub_return_address;                // the return PC, when returning to a call stub
  static address _call_stub_entry;  
  static address _forward_exception_entry;
  static address _catch_exception_entry;
  static address _throw_AbstractMethodError_entry;
  static address _throw_ArithmeticException_entry;
  static address _throw_NullPointerException_entry;
  static address _throw_NullPointerException_at_call_entry;
  static address _throw_StackOverflowError_entry;
  static address _atomic_xchg_entry;
  static address _atomic_xchg_ptr_entry;
  static address _atomic_store_entry;
  static address _atomic_store_ptr_entry;
  static address _atomic_cmpxchg_entry;
  static address _atomic_cmpxchg_ptr_entry;
  static address _atomic_cmpxchg_long_entry;
  static address _atomic_add_entry;
  static address _atomic_add_ptr_entry;
  static address _fence_entry;
  static address _d2i_wrapper;
  static address _d2l_wrapper;

  static jint    _fpu_cntrl_wrd_std;
  static jint    _fpu_cntrl_wrd_24;
  static jint    _fpu_cntrl_wrd_64;
  static jint    _fpu_cntrl_wrd_trunc;
  static jint    _mxcsr_std;
  static jint    _fpu_subnormal_bias1[3];
  static jint    _fpu_subnormal_bias2[3];

  static BufferBlob* _code1;                               // code buffer for initial routines
  static BufferBlob* _code2;                               // code buffer for all other routines

 public:
  // Initialization/Testing
  static void    initialize1();                            // must happen before universe::genesis
  static void    initialize2();                            // must happen after  universe::genesis

  static bool contains(address addr) {
    return
      (_code1 != NULL && _code1->blob_contains(addr)) ||
      (_code2 != NULL && _code2->blob_contains(addr)) ;
  }

  // Debugging
  static jint    verify_oop_count()                        { return _verify_oop_count; }  
  static jint*   verify_oop_count_addr()                   { return &_verify_oop_count; }  
  // a subroutine for debugging the GC
  static address verify_oop_subroutine_entry_address()    { return (address)&_verify_oop_subroutine_entry; }

  static address catch_exception_entry()                   { return _catch_exception_entry; }

  // Calls to Java
  typedef void (*CallStub)(
    address   link,
    intptr_t* result,
    BasicType result_type,
    methodOop method,
    address   entry_point,
    intptr_t* parameters,
    int       size_of_parameters,
    TRAPS
  );

  static CallStub call_stub()                              { return CAST_TO_FN_PTR(CallStub, _call_stub_entry); }
  static bool    returns_to_call_stub(address return_pc)   { return return_pc == _call_stub_return_address; }

  // Exceptions
  static address forward_exception_entry()                 { return _forward_exception_entry; }
  // Implicit exceptions
  static address throw_AbstractMethodError_entry()         { return _throw_AbstractMethodError_entry; }
  static address throw_ArithmeticException_entry()         { return _throw_ArithmeticException_entry; }
  static address throw_NullPointerException_entry()        { return _throw_NullPointerException_entry; }
  static address throw_NullPointerException_at_call_entry(){ return _throw_NullPointerException_at_call_entry; }
  static address throw_StackOverflowError_entry()          { return _throw_StackOverflowError_entry; }

  static address atomic_xchg_entry()                       { return _atomic_xchg_entry; }
  static address atomic_xchg_ptr_entry()                   { return _atomic_xchg_ptr_entry; }
  static address atomic_store_entry()                      { return _atomic_store_entry; }
  static address atomic_store_ptr_entry()                  { return _atomic_store_ptr_entry; }
  static address atomic_cmpxchg_entry()                    { return _atomic_cmpxchg_entry; }
  static address atomic_cmpxchg_ptr_entry()                { return _atomic_cmpxchg_ptr_entry; }
  static address atomic_cmpxchg_long_entry()               { return _atomic_cmpxchg_long_entry; }
  static address atomic_add_entry()                        { return _atomic_add_entry; }
  static address atomic_add_ptr_entry()                    { return _atomic_add_ptr_entry; }
  static address fence_entry()                             { return _fence_entry; }

  static address d2i_wrapper()                             { return _d2i_wrapper; }
  static address d2l_wrapper()                             { return _d2l_wrapper; }
  static address addr_fpu_cntrl_wrd_std()                  { return (address)&_fpu_cntrl_wrd_std;   }
  static address addr_fpu_cntrl_wrd_24()                   { return (address)&_fpu_cntrl_wrd_24;   }
  static address addr_fpu_cntrl_wrd_64()                   { return (address)&_fpu_cntrl_wrd_64;   }
  static address addr_fpu_cntrl_wrd_trunc()                { return (address)&_fpu_cntrl_wrd_trunc; }
  static address addr_mxcsr_std()                          { return (address)&_mxcsr_std; }
  static address addr_fpu_subnormal_bias1()                { return (address)&_fpu_subnormal_bias1; }
  static address addr_fpu_subnormal_bias2()                { return (address)&_fpu_subnormal_bias2; }
};
