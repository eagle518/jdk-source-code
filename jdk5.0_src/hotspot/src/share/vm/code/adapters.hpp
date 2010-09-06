#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)adapters.hpp	1.44 04/03/17 12:08:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Helper class for calculating hash-value/condensed signature for an adapter
class AdapterInfo : public StackObj {
  int _length;
  intptr_t _bitmap;    // if !inlined() reference to a bitmap  

  // Helpers  
  bool inlined() const      { return _length <= BitsPerWord; }
  void clean()              { _length = -1; }
  bool is_clean() const     { return _length == -1; }
  void compute(methodHandle m, bool supress_kind);
public:
  AdapterInfo()                { clean(); }
  AdapterInfo(methodHandle m, bool supress_kind)  { clean(); compute(m, supress_kind); }
  ~AdapterInfo();

  // Hash value for method
  intptr_t hash_value() const;
  
  bool equal(AdapterInfo *entry) const;
  void copy_to(AdapterInfo *entry);
  
  // debugging
  void print() const PRODUCT_RETURN;
};

// Common superclass for I2CAdapter and C2IAdapter
// Stores signature information, as well as a link element so they can
// be easily linked in a hashtable (see AdapterCache)
class BasicAdapter : public CodeBlob {
  AdapterInfo   _info;  // Signature info. for an adapter  
  bool _is_static_call;       
  symbolOop _prototypical_signature; // Example signature. Used for oop-iteration
  BasicAdapter* _link; 
 public:
  BasicAdapter(const char* name, CodeBuffer *cb, int header_size, int size, int frame_size, OopMapSet* oop_maps) :
       CodeBlob(name, cb, header_size, size, frame_size, oop_maps) {};

  // Signature info accessor
  AdapterInfo* info()                          { return &_info; }  
  BasicAdapter* link() const                   { return _link;  }
  symbolOop     prototypical_signature() const { return _prototypical_signature; }
  bool          is_static_call() const         { return _is_static_call; }
  void          set_link(BasicAdapter *l)      { _link = l; }

  void set_prototypical_signature(symbolOop signature, bool is_static_call) { 
    _is_static_call = is_static_call;
    _prototypical_signature = signature;
  }

  // GC support for oops
  void follow_roots_or_mark_for_unloading( BoolObjectClosure* is_alive,
                       OopClosure* keep_alive,
                       bool unloading_occurred,
                       bool& marked_for_unloading);
  void oops_do(OopClosure* f);

  // Debugging
  void print() const PRODUCT_RETURN;
  void verify()      PRODUCT_RETURN;
};

//
// Class I2CAdapter describes an adapter used by the interpreter to call a
// compiled method
//
class I2CAdapter : public BasicAdapter {
 private:
  // Creation support
  I2CAdapter(CodeBuffer *cb, OopMapSet *oop_maps, int size, int frame_size);
  void* operator new(size_t s, unsigned size);
 public:
  // Creation
  static I2CAdapter *new_i2c_adapter(CodeBuffer *cb, OopMapSet *oop_maps, int frame_size);

  // Typing
  bool is_i2c_adapter() const      { return true; }
  
  address std_verified_entry()     { return instructions_begin(); }

  // GC/Verification support
  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map,
				     OopClosure* f);
  bool is_alive() const            { return true; }  
  void print() const                          PRODUCT_RETURN;
  void print_value_on(outputStream* st) const PRODUCT_RETURN;   
};


//
// Class C2IAdapter describes an adapter used by the interpreter to call a
// compiled method
//
class C2IAdapter : public BasicAdapter {
 private:  
  int   _std_verified_offset;      
  // Creation support
  C2IAdapter(CodeBuffer *cb, int size, unsigned std_verified_offset, OopMapSet *oop_maps, int frame_size);
  void* operator new(size_t s, unsigned size);

  address entry_point()            { return instructions_begin(); }
 public:
  // Creation
  static C2IAdapter *new_c2i_adapter(CodeBuffer *cb, 
                                     unsigned    std_verified_offset, 
                                     OopMapSet  *oop_maps,                                      
                                     int         frame_size);

  // Typing
  bool is_c2i_adapter() const      { return true; }
  
  // Entry Points   
  address mkh_unverified_entry()   { return entry_point(); }
  address std_verified_entry()     { return entry_point() + _std_verified_offset; }      

  // Deoptimiztion support
  // Returns the point where the interpreter returns back to the adapter
  address return_from_interpreter();

  // Check if pc points into inline cache check of this adapter
  bool inlinecache_check_contains(address pc) { return mkh_unverified_entry() <= pc && pc < std_verified_entry(); }

  // GC/Verification support
  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map,
				     OopClosure* f);
  bool is_alive() const            { return true; }  
  void print() const                           PRODUCT_RETURN;
  void print_value_on(outputStream* st) const  PRODUCT_RETURN;
  
  // Deoptimization support
  void unpack_c2i_adapter(frame stub, frame adapter_frame, vframeArray* vframe_array);

  private:
  void setup_stack_frame(frame fr, vframeArray* vframe_array);
};

//==========================================================================================
// Cache's for storing I2C and C2I adapters

// An adapter cache
class AdapterCache : public CHeapObj {
 private:
  enum LocalConstants {
    hashtable_size = 256    // Must be a power of 2
  };
  BasicAdapter* _table[hashtable_size];
 public:
  AdapterCache();
  ~AdapterCache();

  // access
  BasicAdapter* lookup(AdapterInfo* info);
  void insert(AdapterInfo* info, BasicAdapter* adapter);
};


// C2IAdapters manages the creation and caching of adapters from
// compiled java into the interpreter.  There is one C2I Adapter per
// signature (method independent).
class C2IAdapterGenerator : AllStatic {
private:

  static C2IAdapter  *generate_c2i_adapter(methodHandle m);
public:
  static AdapterCache *_cache;

  // Require compiledICHolder (mkh) in EAX for i486
  static address      mkh_unverified_entry(methodHandle m);

  // Requires methodOop in EAX for i486
  static address      std_verified_entry(methodHandle m);
  static address      lazy_std_verified_entry(methodHandle m);

  static C2IAdapter  *adapter_for(methodHandle m);

  // initialization
  static void initialize();
  
};

// I2CAdapterGenerator manages the creation and caching of adapters from 
// the interpreter into compiled-java. There is one I2CAdapter per signature
class I2CAdapterGenerator : AllStatic {  
private:

  static I2CAdapter  *generate_i2c_adapter(methodHandle m);
 public:                
  static AdapterCache *_cache;

   // Requires methodOop in EAX for i486
  static address std_verified_entry(methodHandle m);
  
  // initialization
  static void initialize();
};


// Initialization on start-up
void adapter_init();


