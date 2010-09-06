#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)onStackReplacement.hpp	1.25 03/12/23 16:39:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Class OSRAdapter describes an interpreter frame that has been
// on-stack replaced. All oops will be dead in this frame.  This
// adapter frame keeps track of the size of the frame, and where the
// link (ebp on Win32) is stored, so stack-traversal works.  They have
// two different entry points, one for methods returning an FP value
// and another for returning other values.  On most platforms these
// are the same but on x86 we need to ffree FP registers and need to
// make sure we don't ffree the result.
//
class OSRAdapter: public CodeBlob {
  friend class VMStructs;
  
 private:
  int _returning_fp_entry_offset;

  OSRAdapter::OSRAdapter(CodeBuffer *cb, OopMapSet *oop_maps, int size,
                         int frame_size, int returning_fp_entry_offset);
  void* operator new(size_t s, unsigned size);

 public:
  // Creation
  static OSRAdapter *new_osr_adapter(CodeBuffer *cb, OopMapSet *oop_maps,
                                     int frame_size, int returning_fp_entry_offset);

  address entry_point(bool returning_fp) {
    if (returning_fp) {
      return instructions_begin() + _returning_fp_entry_offset;
    } else {
      return instructions_begin();
    }
  }

  // Define Codeblob behaviour
  bool is_osr_adapter()                  const  { return true; }
  bool is_alive()                        const  { return true; }
  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map,
				     OopClosure* f)
  { /* nothing to do */ }  
  void follow_roots_or_mark_for_unloading(BoolObjectClosure* is_alive,
                                OopClosure* keep_alive,
                                bool unloading_occurred,
                                bool& marked_for_unloading)
  { /* do-nothing*/ }

  // Iteration.
  void oops_do(OopClosure* f) {}        

  // Debugging code
  void verify()                                PRODUCT_RETURN; 
  void print()                          const  PRODUCT_RETURN;
  void print_value_on(outputStream* st) const  PRODUCT_RETURN; 
};


// Namespace for OnStackReplacement functionality

class OnStackReplacement: public AllStatic {
 private:
  // List of adapters for different frame sizes.
  static GrowableArray<OSRAdapter*>* _osr_adapters;

  enum Constants {    
    InitialAdapterVectorSize   = 64,     // Initial size of adapter array
    MinTypicalAdapterSize      = 6,      // The typical range is precomputed 
    MaxTypicalAdapterSize      = 16      //   (framesize is in words)
  };

 public:
  // Initialization
  static void initialize();  

  // Returns an OSRAdapter for the given framesize
  static OSRAdapter* get_osr_adapter(frame fr, methodHandle method);
};


