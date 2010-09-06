#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)adapters.cpp	1.83 04/04/05 13:05:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ---------------------------------------------------------------------------
// Implementation of I2CAdapter

# include "incls/_precompiled.incl"
# include "incls/_adapters.cpp.incl"


#ifndef PRODUCT
static int I2C_generation_count = 0;
static int C2I_generation_count = 0;
#endif

// ComputeAdapterInfo computes a unique value for a given method. The
// fingerprint is a bitvector value characterizing the methods
// signature (including the receiver) modulo oop-type and long/double,
// along with information about (1) synchronized or not, and (2)
// native call or not
//
class ComputeAdapterInfo : public SignatureIterator {
 private:  
  methodHandle _method;     // current Method  

  intptr_t *_value;         // result pointer
  int _shift_count;         // current shift count into result
  

 public:
  enum {
    // To ease the logic, the general_feature_size and feature_size have been
    // made the same and made to divide into BitsPerWord (basically forcing
    // both to be the same power of 2).  feature_size could otherwise be 3
    // bits, which would allow a few more signatures to fit in the fast 1-word
    // fingerprint but not enough to make the engineering worthwhile.  These
    // things are over-engineered as it is.
    general_feature_size   = 4, // Enough bits to hold AbstractInterpreter::number_of_method_entries
    feature_size           = 4  // Enough bits to hold max_type below
  };
  
  enum CompressedType {
    int_type     = 0,  // includes boolean, char, byte, int, short
    long_type    = 1,
    float_type   = 2,
    double_type  = 3,
    oop_type     = 4,  // includes all object and array references
    void_type    = 5,  // for return values
    max_type     = 5   // for asserts
  };

  void set_type(int type) {
    assert(type >= 0 && type <= max_type, "wrong type passed in");
    assert(((*_value) >> _shift_count) == 0, "clear bits in type field");
    (*_value) |= ((intptr_t)type << _shift_count);
    assert(((*_value) >> _shift_count & right_n_bits(feature_size)) == type,
	   "successful insertion of type field");
    _shift_count += feature_size;
    if ( _shift_count > BitsPerWord-feature_size) {      
      _shift_count = 0;
      _value++;      
    }
  }  

  void do_bool  ()                     { set_type(int_type);    }
  void do_char  ()                     { set_type(int_type);    }
  void do_byte  ()                     { set_type(int_type);    }
  void do_short ()                     { set_type(int_type);    }
  void do_int   ()                     { set_type(int_type);    }
  void do_float ()                     { set_type(float_type);  }
  void do_double()                     { set_type(double_type); }
  void do_long  ()                     { set_type(long_type);   }
  void do_object(int begin, int end)   { set_type(oop_type);    }
  void do_array (int begin, int end)   { set_type(oop_type);    }
  void do_void  ()                     { ShouldNotReachHere();  }
  
  ComputeAdapterInfo (methodHandle method) : SignatureIterator(method->signature()) {
    _method = method;
  }

  void compute(intptr_t *value, int max_bits, bool supress_kind) {    
    assert(size_in_bits(_method) <= max_bits, "signature too big");    
    assert(max_type < (1 << feature_size), "bit overflow");
    _value = value;

    // Store kind of interpreter entry is needed. For C2I adapter, we need to distinguish
    // on the different entrypoints. For I2C adapters, we do not.
    if (supress_kind) {
      *_value = AbstractInterpreter::zerolocals; // Always use the same kind
    } else {
      *_value = AbstractInterpreter::method_kind(_method); // Kind depends on method
    } 
    _shift_count = general_feature_size;
    assert( (1<<general_feature_size) >= AbstractInterpreter::number_of_method_entries, 
            "not enough room to store general features");


    // Return type. Handled specially, so we do not mix up the last
    // argument with a return type
    set_type(return_type(_method->result_type()));
    // receiver    
    if (!_method->is_static()) set_type(oop_type);    
    // arguments
    iterate_parameters(Fingerprinter(_method).fingerprint());    
  }

  // Computes number of bits needed to store a condensed signature
  static int size_in_bits(methodHandle method) {
    ArgumentCount args(method->signature());
    int size = general_feature_size  + 
               (1 + args.size() + (method->is_static() ? 0 : 1)) * feature_size;
    return size;  
  }

  // compress type for retun value
  int return_type(BasicType type) {    
    switch (type) {
      case T_BOOLEAN: // fall through
      case T_CHAR   : // fall through
      case T_BYTE   : // fall through
      case T_SHORT  : // fall through
      case T_INT    : return int_type;
      case T_LONG   : return long_type;
      case T_VOID   : return void_type;
      case T_FLOAT  : return float_type;
      case T_DOUBLE : return double_type;
      case T_OBJECT : // fall through
      case T_ARRAY  : return oop_type;
      default       : ShouldNotReachHere() return 0;
    }    
  }  

  // Words needed to store the bitmap
  static int words_needed(int length) { 
    const int group = general_feature_size;
    assert(general_feature_size == feature_size, "must be the same for below calculation to work");
    assert(group <= BitsPerWord, "group cannot exceed word size");
    assert(length % group == 0, "must have complete groups");
    return ((length/group) - 1 + (BitsPerWord/group))/(BitsPerWord/group);    
  }

#ifndef PRODUCT
  static const char* type_to_string(int type) {
    switch(type) {
      case int_type:     return "I";
      case long_type:    return "J";
      case void_type:    return "V";
      case float_type:   return "F";
      case double_type:  return "D";
      case oop_type:     return "L";
      default: ShouldNotReachHere(); return NULL;
    }
  }  
#endif
};

//----------------------------------------------------------------------------
// Implementation of AdapterInfo

AdapterInfo::~AdapterInfo() {
  if (!is_clean() && !inlined()) {
    // Deallocate Adapter Info if needed
    FREE_C_HEAP_ARRAY(intptr_t, _bitmap);
  }
}

void AdapterInfo::compute(methodHandle method, bool supress_kind) {
  assert(_length == -1, "already computed?");
  _length = ComputeAdapterInfo::size_in_bits(method);
  ComputeAdapterInfo cai(method);  
  if (inlined()) {
    cai.compute(&_bitmap, _length, supress_kind);
  } else {
    assert(sizeof(intptr_t) == sizeof(int *), "must be able to store a pointer in an int field")
    int words = ComputeAdapterInfo::words_needed(_length);
    _bitmap = (intptr_t)NEW_C_HEAP_ARRAY(intptr_t, words);  
    memset((void*)_bitmap, 0, sizeof(intptr_t) * words); 
    cai.compute((intptr_t *)_bitmap, _length, supress_kind);
  }
}


bool AdapterInfo::equal(AdapterInfo *entry) const {
  if ( _length != entry->_length) return false;

  // Inline allocation
  if (inlined()) {
    return _bitmap == entry->_bitmap;
  }
  
  // Out-of-line allocation
  for(int i = 0; i < ComputeAdapterInfo::words_needed(_length); i++) {
    if (((intptr_t *)_bitmap)[i] != ((intptr_t *)entry->_bitmap)[i]) return false;
  }
  return true;
}


void AdapterInfo::copy_to(AdapterInfo *entry) {
  entry->_length = _length;
  if (inlined()) {
     entry->_bitmap = _bitmap;
  } else {
    int w_needed = ComputeAdapterInfo::words_needed(_length);
    intptr_t *bitvector = NEW_C_HEAP_ARRAY(intptr_t, w_needed);
    for(int i = 0; i < w_needed; i++) {
      bitvector[i] = ((intptr_t *)_bitmap)[i];
    } 
    entry->_bitmap = (intptr_t)bitvector;
  }
}

intptr_t AdapterInfo::hash_value() const {   
  // We just use the first word (32 or 64 bits), except the native/synchronzied info.
  assert(!is_clean(), "must be computed");
  intptr_t value = (inlined()) ? _bitmap : ((intptr_t *)_bitmap)[0];
  return value >> ComputeAdapterInfo::general_feature_size; 
} 


//-----------------------------------------------------------------------------
// Implentation of Basic Adapter

void BasicAdapter::follow_roots_or_mark_for_unloading(BoolObjectClosure* is_alive, 
                                      OopClosure* keep_alive,
                                      bool unloading_occurred,
                                      bool& marked_for_unloading) {
  // Just one oop to preserve
  // YSR: is this necessary? If this is a strong root it must already
  // have been traversed during the tracing phase; if it's a weak
  // root, it must have been traversed during the weak tracing
  // phase. In either case, there should not be any recursive
  // marking that this should entail. !!! FIX ME
  keep_alive->do_oop((oop*)&_prototypical_signature);
}

void BasicAdapter::oops_do(OopClosure* f) {
  // Just one oop to adjust
  f->do_oop((oop*)&_prototypical_signature);
}


//-----------------------------------------------------------------------------
// Implentation of AdapterCache


AdapterCache::AdapterCache() {
  assert(is_power_of_2(hashtable_size), "size must be a power of two");
  for(int i = 0; i < hashtable_size; i++) {
    _table[i] = NULL;
  }
}

BasicAdapter* AdapterCache::lookup(AdapterInfo *info) {
  MutexLocker mu(AdapterCache_lock); // We use same lock for both all instances of the cache.
  assert(info != NULL, "sanity check");
  int key = (info->hash_value()) & (hashtable_size - 1);
  assert(key >= 0 && key < hashtable_size, "sanity check");
  BasicAdapter *adapter= _table[key];
  while(adapter != NULL) {
    if (info->equal(adapter->info())) return adapter;
    adapter = adapter->link();
  }
  return NULL;
}


void AdapterCache::insert(AdapterInfo *info, BasicAdapter* adapter) {  
  MutexLocker mu(AdapterCache_lock); // We use same lock for both all instances of the cache.
  assert(info != NULL && adapter != NULL, "sanity check");
  int key = info->hash_value() & (hashtable_size - 1);
  assert(key >= 0 && key < hashtable_size, "sanity check");
  // Copy info into adapter
  info->copy_to(adapter->info());
  // Link into hashtable
  adapter->set_link(_table[key]);
  _table[key] = adapter; 
}


//-----------------------------------------------------------------------------

I2CAdapter::I2CAdapter(CodeBuffer* cb, OopMapSet *oop_maps, int size, int frame_size)
: BasicAdapter("I2CAdapter", cb, sizeof(I2CAdapter), size, frame_size, oop_maps) {}


I2CAdapter* I2CAdapter::new_i2c_adapter(CodeBuffer* cb, OopMapSet *oop_maps, int frame_size) {
  I2CAdapter* adapter = NULL;
  unsigned int size = allocation_size(cb, sizeof(I2CAdapter));
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    adapter = new (size) I2CAdapter(cb, oop_maps, size, frame_size );
  }

  // Do not hold the CodeCache lock during name formatting.
  if (adapter != NULL) {
    char adapter_id[256];
    jio_snprintf(adapter_id, sizeof(adapter_id), "I2CAdapter@" PTR_FORMAT, adapter->instructions_begin());
    VTune::register_stub(adapter_id, adapter->instructions_begin(), adapter->instructions_end());
    Forte::register_stub(adapter_id, adapter->instructions_begin(), adapter->instructions_end());
  }

  return adapter;
}


void* I2CAdapter::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);
  if (!p) fatal("out of space in code cache");
  return p;
}


// Preserves outgoing arguments. 
void I2CAdapter::preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map, OopClosure* f) {
  fr.oops_compiled_arguments_do(prototypical_signature(), is_static_call(), reg_map, f);
}


// ------------------------ C2IAdapter code blob ------------------------------

C2IAdapter::C2IAdapter(CodeBuffer* cb, int size, unsigned std_verified_offset, OopMapSet* oop_maps, int frame_size)
: BasicAdapter("C2IAdapter", cb, sizeof(C2IAdapter), size, frame_size, oop_maps) {
  _std_verified_offset      = std_verified_offset;
}


C2IAdapter* C2IAdapter::new_c2i_adapter(CodeBuffer* cb, 
                                        unsigned std_vep_offset,
                                        OopMapSet* oop_maps,
                                        int frame_size) {
  C2IAdapter* adapter = NULL;
  unsigned int size = allocation_size(cb, sizeof(C2IAdapter));
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    adapter =  new (size) C2IAdapter(cb, size, std_vep_offset, oop_maps, frame_size);
  }

  // Do not hold the CodeCache lock during name formatting.
  if (adapter != NULL) {
    char adapter_id[256];
    jio_snprintf(adapter_id, sizeof(adapter_id), "C2IAdapter@" PTR_FORMAT, adapter->instructions_begin());
    VTune::register_stub(adapter_id, adapter->instructions_begin(), adapter->instructions_end());
    Forte::register_stub(adapter_id, adapter->instructions_begin(), adapter->instructions_end());
  }

  return adapter;
}


void* C2IAdapter::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);
  if (!p) fatal("out of space in code cache");
  return p;
}

// Preserves outgoing arguments. 
void C2IAdapter::preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map, OopClosure* f) {
  fr.c2i_arguments_do(prototypical_signature(), is_static_call(), f);
}


address C2IAdapter::return_from_interpreter() {
  // find offset from OopMapSet
  return  entry_point() + oop_maps()->singular_oop_map()->offset();
}


void C2IAdapter::setup_stack_frame(frame fr, vframeArray* vframe_array) {  
  OopMap* map = oop_maps()->singular_oop_map();

  // Setup all callee-saved registers for the C2I adapter frame with the callee-saved values
  // from the vframe.
  OopMapValue omv;
  for(OopMapStream oms(map,OopMapValue::callee_saved_value); !oms.is_done(); oms.next()) {
    omv = oms.current();
    assert(omv.is_stack_loc(), "sanity check");
    intptr_t* location = (intptr_t*)fr.oopmapreg_to_location(omv.reg(),NULL);
    VMReg::Name callee_reg = omv.content_reg();
    *location = * (intptr_t*) vframe_array->register_location(callee_reg);      
  }
}

// ===========================================================================
// ---------------------------------------------------------------------------
// Implementation of C2IAdapter


// -------------------------------- C2IAdapterGenerator -----------------------

AdapterCache* C2IAdapterGenerator::_cache;


void C2IAdapterGenerator::initialize() {
  _cache = new AdapterCache();
  guarantee(_cache != NULL, "initialization failed for I2C adapter cache");
}


C2IAdapter *C2IAdapterGenerator::generate_c2i_adapter(methodHandle m) {
  assert(_cache != NULL, "missing initialization of C2IAdapterGenerator");

  // Compute condensed signature for method
  ResourceMark rm;
  AdapterInfo info(m, false);
  
  // Check cache for I2C_adapter
  C2IAdapter* adapter = (C2IAdapter *)_cache->lookup(&info);

  // if it exists then return adapter for this method
  if( adapter )  {
    assert( adapter->is_c2i_adapter(), "sanity check" );
    return adapter;
  }
    
  // Create a compiled to interpreter adapter
  adapter = CompileBroker::compile_c2i_adapter_for(m, true);
  assert(adapter, "Must be able to generate C2I adapter for target");
 
#ifndef PRODUCT
  if (TraceAdapterGeneration) {     
    tty->print("%d Generated ", C2I_generation_count++);
    adapter->print_value_on(tty);
    tty->cr();
  }
#endif

  // return adapter for this method
  return adapter;
}


address C2IAdapterGenerator::mkh_unverified_entry(methodHandle m) {
  C2IAdapter *adapter = generate_c2i_adapter(m);
  return  adapter->mkh_unverified_entry();
}


address C2IAdapterGenerator::std_verified_entry(methodHandle m) {
  C2IAdapter *adapter = generate_c2i_adapter(m);
  return  adapter->std_verified_entry();
}


address C2IAdapterGenerator::lazy_std_verified_entry(methodHandle m) {
  assert(_cache != NULL, "missing initialization of C2IAdapterGenerator");

  // Compute condensed signature for method
  ResourceMark rm;
  AdapterInfo info(m, false);  

  // Check cache for C2I_adapter
  C2IAdapter* adapter = (C2IAdapter*)_cache->lookup(&info);
  // if it exists then return adapter for this method
  if(adapter != NULL )  {
    assert(adapter->is_c2i_adapter(), "wrong codeblob returned");
    return adapter->std_verified_entry();
  }
  return OptoRuntime::lazy_c2i_adapter_stub();
}


C2IAdapter* C2IAdapterGenerator::adapter_for(methodHandle m) {
  return generate_c2i_adapter(m);  
}


// -------------------------------- I2CAdapterGenerator -----------------------

AdapterCache* I2CAdapterGenerator::_cache;


void I2CAdapterGenerator::initialize() {
  _cache = new AdapterCache();
  guarantee(_cache != NULL, "initialization failed for I2C adapter cache");
}


I2CAdapter* I2CAdapterGenerator::generate_i2c_adapter(methodHandle m) {
  assert(_cache != NULL, "missing initialization of I2CAdapterGenerator");

  // Compute condensed signature for method
  ResourceMark rm;
  AdapterInfo info(m, true);
  
  // Check cache for I2C_adapter
  I2CAdapter* adapter = (I2CAdapter *)_cache->lookup(&info);

  // if it exists then return adapter for this method
  if( adapter )  {
    assert( adapter->is_i2c_adapter(), "sanity check" );
    return adapter;
  }
    
  // Create an interpreter to compiler adapter
  adapter = CompileBroker::compile_i2c_adapter_for(m, true);
  assert( adapter, "Must be able to generate I2C adapter for target");  

#ifndef PRODUCT
  if (TraceAdapterGeneration) {     
    tty->print("%d Generated ", I2C_generation_count++);
    adapter->print_value_on(tty);
    tty->cr();
  }
#endif

  // return adapter for this method
  return adapter;
}


address I2CAdapterGenerator::std_verified_entry(methodHandle m) {
  I2CAdapter *adapter = generate_i2c_adapter(m);
  return  adapter->std_verified_entry();
}


// Initialization of module
void adapter_init() {
  I2CAdapterGenerator::initialize();
  C2IAdapterGenerator::initialize();
}


//------------------------------------------------------------------------
// non-product code

#ifndef PRODUCT

void AdapterInfo::print() const {
  if (is_clean()) {
    tty->print("<no signature>");
    return;
  }
  
  intptr_t *value = (inlined()) ? (intptr_t*)&_bitmap : (intptr_t*)_bitmap;  

  // Entry kind (used only for C2I adapters)
  int kind = *value & right_n_bits(ComputeAdapterInfo::general_feature_size);
  tty->print("["); 
  AbstractInterpreter::print_method_kind((AbstractInterpreter::MethodKind)kind); 
  
  // Return signature
  int ret_type = ((*value) >> ComputeAdapterInfo::general_feature_size) & right_n_bits(ComputeAdapterInfo::feature_size);
  tty->print("](");

  // Arguments  
  int offset = ComputeAdapterInfo::general_feature_size + ComputeAdapterInfo::feature_size;
  int length = offset;
  assert(offset <= _length, "must contain general info and return info");
  while(length < _length) {
    int type = ((*value) >> offset) & right_n_bits(ComputeAdapterInfo::feature_size);
    tty->print("%s", ComputeAdapterInfo::type_to_string(type));

    // Compute next offset
    offset += ComputeAdapterInfo::feature_size;
    if (offset >= BitsPerWord) {
       value++;
       offset = 0;
    }
    length += ComputeAdapterInfo::feature_size;
  }
  assert(length == _length, "should match");
  tty->print(")%s", ComputeAdapterInfo::type_to_string(ret_type));  
}  


void BasicAdapter::print() const {
  _info.print();
}


void BasicAdapter::verify() {
  assert(prototypical_signature()->is_oop(), "must be an oop");
}


void I2CAdapter::print() const {
  tty->print("I2C Adapter");  
  BasicAdapter::print();
  Disassembler::decode((CodeBlob*)this);
}


void I2CAdapter::print_value_on(outputStream* st) const {
  st->print("I2CAdapter: ");
  BasicAdapter::print();
}


void C2IAdapter::print() const {
  tty->print("C2I Adapter: ");  
  BasicAdapter::print();
  Disassembler::decode((CodeBlob*)this);
}


void C2IAdapter::print_value_on(outputStream* st) const {
  st->print("C2IAdapter: "); 
  BasicAdapter::print();
}


#endif
