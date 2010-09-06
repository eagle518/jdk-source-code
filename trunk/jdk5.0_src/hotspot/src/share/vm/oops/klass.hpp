#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)klass.hpp	1.121 04/01/12 18:18:49 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A Klass is the part of the klassOop that provides:
//  1: language level class object (method dictionary etc.)
//  2: provide vm dispatch behavior for the object
// Both functions are combined into one C++ class. The toplevel class "Klass"
// implements purpose 1 whereas all subclasses provide extra virtual functions 
// for purpose 2.

// One reason for the oop/klass dichotomy in the implementation is
// that we don't want a C++ vtbl pointer in every object.  Thus,
// normal oops don't have any virtual functions.  Instead, they
// forward all "virtual" functions to their klass, which does have
// a vtbl and does the C++ dispatch depending on the object's
// actual type.  (See oop.inline.hpp for some of the forwarding code.)
// ALL FUNCTIONS IMPLEMENTING THIS DISPATCH ARE PREFIXED WITH "oop_"!

//  Klass layout:
//    [header        ] klassOop
//    [klass pointer ] klassOop
//    [C++ vtbl ptr  ] (contained in Klass_vtbl)
//    [super_check_offset   ] for fast subtype checks
//    [secondary_super_cache] for fast subtype checks
//    [secondary_supers     ] array of 2ndary supertypes
//    [primary_supers 0]
//    [primary_supers 1]
//    [primary_supers 2]
//    ...
//    [primary_supers 7]
//    [size helper   ] (see encoding in oop.inline.hpp)
//    [java mirror   ]
//    [modifier flags]
//    [super         ]
//    [name          ]
//    [access flags  ]
//    [first subklass]
//    [next sibling  ]
//    [verify count  ] - not in product
//    [alloc count   ]
//    [is_objArray   ]


// Forward declarations.
class klassVtable;
class KlassHandle;

// Holder (or cage) for the C++ vtable of each kind of Klass.
// We want to tightly constrain the location of the C++ vtable in the overall layout.
class Klass_vtbl {
 protected:
  // The following virtual exists only to force creation of a C++ vtable,
  // so that this class truly is the location of the vtable of all Klasses.
  virtual void unused_initial_virtual() { }

 public:
  // The following virtual makes Klass_vtbl play a second role as a
  // factory protocol for subclasses of Klass ("sub-Klasses").
  // Here's how it works....
  //
  // This VM uses metaobjects as factories for their instances.
  //
  // In order to initialize the C++ vtable of a new instance, its
  // metaobject is forced to use the C++ placed new operator to
  // allocate the instance.  In a typical C++-based system, each
  // sub-class would have its own factory routine which
  // directly uses the placed new operator on the desired class,
  // and then calls the appropriate chain of C++ constructors.
  //
  // However, this system uses shared code to performs the first
  // allocation and initialization steps for all sub-Klasses.
  // (See base_create_klass() and base_create_array_klass().)
  // This does not factor neatly into a hierarchy of C++ constructors.
  // Each caller of these shared "base_create" routines knows
  // exactly which sub-Klass it is creating, but the shared routine
  // does not, even though it must perform the actual allocation.
  //
  // Therefore, the caller of the shared "base_create" must wrap
  // the specific placed new call in a virtual function which
  // performs the actual allocation and vtable set-up.  That
  // virtual function is here, Klass_vtbl::allocate_permanent.
  //
  // The arguments to Universe::allocate_permanent() are passed
  // straight through the placed new operator, which in turn
  // obtains them directly from this virtual call.
  //
  // This virtual is called on a temporary "example instance" of the
  // sub-Klass being instantiated, a C++ auto variable.  The "real"
  // instance created by this virtual is on the VM heap, where it is
  // equipped with a klassOopDesc header.
  //
  // It is merely an accident of implementation that we use "example
  // instances", but that is why the virtual function which implements
  // each sub-Klass factory happens to be defined by the same sub-Klass
  // for which it creates instances.
  //
  // The vtbl_value() call (see below) is used to strip away the
  // accidental Klass-ness from an "example instance" and present it as
  // a factory.  Think of each factory object as a mere container of the
  // C++ vtable for the desired sub-Klass.  Since C++ does not allow
  // direct references to vtables, the factory must also be delegated
  // the task of allocating the instance, but the essential point is
  // that the factory knows how to initialize the C++ vtable with the
  // right pointer value.  All other common initializations are handled
  // by the shared "base_create" subroutines.
  //
  virtual void* allocate_permanent(KlassHandle& klass, int size, TRAPS) const = 0;

  // Every subclass on which vtbl_value is called must include this macro.
#define DEFINE_ALLOCATE_PERMANENT(thisKlass) \
  void* allocate_permanent(KlassHandle& klass, int size, TRAPS) const { \
    return new(klass, size, THREAD) thisKlass(); \
  }


 protected:
  void* operator new(size_t ignored, KlassHandle& klass, int size, TRAPS);
};


class Klass : public Klass_vtbl {
  friend class VMStructs;
 protected:
  // note: put frequently-used fields together at start of klass structure
  // for better cache behavior (may not make much of a difference but sure won't hurt)
  enum { _primary_super_limit = 8 };

  // These next 4 fields all help make fast subtype checks.
  // See big discussion in doc/server_compiler/checktype.txt
  //
  // Where to look to observe a supertype (It is &_secondary_super_cache
  // for secondary supers, else is &_primary_supers[depth()].
  juint       _super_check_offset;
  // Cache of last observed secondary supertype
  klassOop    _secondary_super_cache;
  // array of all secondary supertypes
  objArrayOop _secondary_supers;
  // Ordered list of all primary supertypes
  klassOop    _primary_supers[_primary_super_limit];

  int       _size_helper;       // for instances, positive equal to size.
                                // for arrays, -1-log2 of element size.
  oop       _java_mirror;       // instance of java/lang/Class mirroring this class
  jint      _modifier_flags;    // Processed access flags, for use by Class.getModifiers.
  klassOop  _super;             // superclass
  symbolOop _name;              // class name
                                //   instance classes: java/lang/String, etc.
                                //   array classes: [I, [Ljava/lang/String;, etc.
                                //   Set to zero for all other kinds of classes.
  AccessFlags _access_flags;    // Access flags. The class/interface distinction is stored here.
  klassOop _subklass;           // first subclass (NULL if none); _subklass->next_sibling() is next one
  klassOop _next_sibling;       // sibling link (or NULL); links all subklasses of a klass

#ifndef PRODUCT
  int           _verify_count;  // to avoid redundant verifies
#endif

  juint    _alloc_count;        // allocation profiling support - update klass_size_in_bytes() if moved/deleted

  bool     _is_objArray;        // True iff is an obj array.


 public:

  // returns the enclosing klassOop
  klassOop as_klassOop() const {
    // see klassOop.hpp for layout.
    return (klassOop) (((char*) this) - sizeof(klassOopDesc));
  }

 public:
  // Allocation
  const Klass_vtbl& vtbl_value() const { return *this; }  // used only on "example instances"
  static KlassHandle base_create_klass(KlassHandle& klass, int size, const Klass_vtbl& vtbl, TRAPS);
  static klassOop base_create_klass_oop(KlassHandle& klass, int size, const Klass_vtbl& vtbl, TRAPS);

  // super
  klassOop super() const               { return _super; }
  void set_super(klassOop k)           { oop_store_without_check((oop*) &_super, (oop) k); }

  // initializes _super link, _primary_supers & _secondary_supers arrays
  void initialize_supers(klassOop k, TRAPS);
  void initialize_supers_impl1(klassOop k);
  void initialize_supers_impl2(klassOop k);

  // klass-specific helper for initializing _secondary_supers
  virtual objArrayOop compute_secondary_supers(int num_extra_slots, TRAPS);

  // java_super is the Java-level super type as specified by Class.getSuperClass.
  virtual klassOop java_super() const  { return NULL; }

  juint    super_check_offset() const  { return _super_check_offset; }
  void set_super_check_offset(juint o) { _super_check_offset = o; }

  klassOop secondary_super_cache() const     { return _secondary_super_cache; }
  void set_secondary_super_cache(klassOop k) { oop_store_without_check((oop*) &_secondary_super_cache, (oop) k); }

  objArrayOop secondary_supers() const { return _secondary_supers; }
  void set_secondary_supers(objArrayOop k) { oop_store_without_check((oop*) &_secondary_supers, (oop) k); }

  // Return the element of the _super chain of the given depth.
  // If there is no such element, return either NULL or this.
  klassOop primary_super_of_depth(juint i) const {
    assert(i < primary_super_limit(), "oob");
    klassOop super = _primary_supers[i];
    assert(super == NULL || super->klass_part()->super_depth() == i, "correct display");
    return super;
  }

  // Can this klass be a primary super?  False for interfaces and arrays of
  // interfaces.  False also for arrays or classes with long super chains.
  bool can_be_primary_super() const {
    const juint secondary_offset = secondary_super_cache_offset_in_bytes() + sizeof(oopDesc);
    return super_check_offset() != secondary_offset;
  }
  virtual bool can_be_primary_super_slow() const;

  // Returns number of primary supers; may be a number in the inclusive range [0, primary_super_limit].
  juint super_depth() const {
    if (!can_be_primary_super()) {
      return primary_super_limit();
    } else {
      juint d = (super_check_offset() - (primary_supers_offset_in_bytes() + sizeof(oopDesc))) / sizeof(klassOop);
      assert(d < primary_super_limit(), "oob");
      assert(_primary_supers[d] == as_klassOop(), "proper init");
      return d;
    }
  }

  // java mirror
  oop java_mirror() const              { return _java_mirror; }
  void set_java_mirror(oop m)          { oop_store((oop*) &_java_mirror, m); }

  // modifier flags
  jint modifier_flags() const          { return _modifier_flags; }
  void set_modifier_flags(jint flags)  { _modifier_flags = flags; }

  // size helper
  int size_helper() const              { return _size_helper; }
  void set_size_helper(int size)       { _size_helper = size; }

  // Note: for instances size_helper() may include padding.
  // Use instanceKlass::contains_field_offset to classify field offsets.

  // sub/superklass links
  instanceKlass* superklass() const;
  Klass* subklass() const;
  Klass* next_sibling() const;
  void append_to_sibling_list();           // add newly created receiver to superklass' subklass list
  void remove_from_sibling_list();         // remove receiver from sibling list
 protected:                                // internal accessors
  klassOop subklass_oop() const            { return _subklass; }
  klassOop next_sibling_oop() const        { return _next_sibling; }
  void     set_subklass(klassOop s); 
  void     set_next_sibling(klassOop s);

  oop* adr_super()           const { return (oop*)&_super;             }
  oop* adr_primary_supers()  const { return (oop*)&_primary_supers[0]; }
  oop* adr_secondary_super_cache() const { return (oop*)&_secondary_super_cache; }
  oop* adr_secondary_supers()const { return (oop*)&_secondary_supers;  }
  oop* adr_java_mirror()     const { return (oop*)&_java_mirror;       }
  oop* adr_name()            const { return (oop*)&_name;              }
  oop* adr_subklass()        const { return (oop*)&_subklass;          }
  oop* adr_next_sibling()    const { return (oop*)&_next_sibling;      }

 public:
  // Allocation profiling support
  juint alloc_count() const          { return _alloc_count; }
  void set_alloc_count(juint n)      { _alloc_count = n; }
  virtual juint alloc_size() const = 0;
  virtual void set_alloc_size(juint n) = 0;

  // Compiler support
  static int super_offset_in_bytes()         { return (intptr_t)&((Klass*)NULL)->_super;           }
  static int super_check_offset_offset_in_bytes() { return (intptr_t)&((Klass*)NULL)->_super_check_offset; }
  static int primary_supers_offset_in_bytes(){ return (intptr_t)&((Klass*)NULL)->_primary_supers[0]; }
  static int secondary_super_cache_offset_in_bytes() { return (intptr_t)&((Klass*)NULL)->_secondary_super_cache; }
  static int secondary_supers_offset_in_bytes() { return (intptr_t)&((Klass*)NULL)->_secondary_supers; }
  static int java_mirror_offset_in_bytes()   { return (intptr_t)&((Klass*)NULL)->_java_mirror;     }
  static int modifier_flags_offset_in_bytes(){ return (intptr_t)&((Klass*)NULL)->_modifier_flags;      }
  static int size_helper_offset_in_bytes()   { return (intptr_t)&((Klass*)NULL)->_size_helper;    }
  static int access_flags_offset_in_bytes()  { return (intptr_t)&((Klass*)NULL)->_access_flags;    }

  // What is the maximum number of primary superclasses any klass can have?
#ifdef PRODUCT
  static juint primary_super_limit()         { return _primary_super_limit; }
#else
  static juint primary_super_limit() {
    assert(FastSuperclassLimit <= _primary_super_limit, "parameter oob");
    return FastSuperclassLimit;
  }
#endif

  // vtables
  virtual klassVtable* vtable() const        { return NULL; }

  static int klass_size_in_bytes()           { return ((intptr_t)&((Klass*)NULL)->_alloc_count) + sizeof(juint); }  // all "visible" fields

  // subclass check
  bool is_subclass_of(klassOop k) const;
  // subtype check: true if is_subclass_of, or if k is interface and receiver implements it
  bool is_subtype_of(klassOop k) const { 
    juint    off = k->klass_part()->super_check_offset();
    klassOop sup = *(klassOop*)( (address)as_klassOop() + off );
    const juint secondary_offset = secondary_super_cache_offset_in_bytes() + sizeof(oopDesc);
    if (sup == k) {
      return true;
    } else if (off != secondary_offset) {
      return false;
    } else {
      return search_secondary_supers(k);
    }
  }
  bool search_secondary_supers(klassOop k) const;

  // Find LCA in class heirarchy
  Klass *LCA( Klass *k );

  // dispatch optimization: can m be called w/o dynamic dispatch?
  static bool can_be_statically_bound(methodOop m);

  // Check whether reflection/jni/jvm code is allowed to instantiate this class;
  // if not, throw either an Error or an Exception.
  virtual void check_valid_for_instantiation(bool throwError, TRAPS);

  // Casting
  static Klass* cast(klassOop k) {
    assert(k->is_klass(), "cast to Klass");
    return k->klass_part(); 
  }

  // array copying
  virtual void  copy_array(arrayOop s, int src_pos, arrayOop d, int dst_pos, int length, TRAPS);

  // tells if the class should be initialized
  virtual bool should_be_initialized() const    { return false; }
  // initializes the klass
  virtual void initialize(TRAPS);
  // lookup operation for MethodLookupCache
  friend class MethodLookupCache;
  virtual methodOop uncached_lookup_method(symbolOop name, symbolOop signature) const;
 public:  
  methodOop lookup_method(symbolOop name, symbolOop signature) const; // uses MethodLookupCache

  // array class with specific rank
  klassOop array_klass(int rank, TRAPS)         {  return array_klass_impl(false, rank, THREAD); }

  // array class with this klass as element type
  klassOop array_klass(TRAPS)                   {  return array_klass_impl(false, THREAD); }

  // These will return NULL instead of allocating on the heap:
  // NB: these can block for a mutex, like other functions with TRAPS arg.
  klassOop array_klass_or_null(int rank);
  klassOop array_klass_or_null();

  virtual oop protection_domain() { return NULL; }

 protected:
  virtual klassOop array_klass_impl(bool or_null, int rank, TRAPS);
  virtual klassOop array_klass_impl(bool or_null, TRAPS);

 public:
  virtual void remove_unshareable_info();

 protected:
  // computes the subtype relationship
  virtual bool compute_is_subtype_of(klassOop k);
 public:
  // subclass accessor (here for convenience; undefined for non-klass objects)
  virtual bool is_leaf_class() const { fatal("not a class"); return false; }
 public:
  // ALL FUNCTIONS BELOW THIS POINT ARE DISPATCHED FROM AN OOP
  // These functions describe behavior for the oop not the KLASS.

  // actual oop size of obj in memory
  virtual int oop_size(oop obj) const = 0;

  // actual oop size of this klass in memory
  virtual int klass_oop_size() const = 0;

  // Returns the Java name for a class (Resource allocated)
  // For arrays, this returns the name of the element with a leading '['.
  // For classes, this returns the name with the package separators 
  //     turned into '.'s.
  const char* external_name() const;
  // Returns the name for a class (Resource allocated) as the class 
  // would appear in a signature.
  // For arrays, this returns the name of the element with a leading '['.
  // For classes, this returns the name with a leading 'L' and a trailing ';'
  //     and the package separators as '/'.
  virtual char* signature_name() const;

  // garbage collection support
  virtual void oop_follow_contents(oop obj) = 0;
  virtual int  oop_adjust_pointers(oop obj) = 0;
  virtual bool oop_being_unloaded(BoolObjectClosure* is_alive,
                                          oop obj);

  virtual void oop_copy_contents(PSPromotionManager* pm, oop obj) = 0;

 public:
  // type testing operations
  virtual bool oop_is_instance()            const { return false; }
  virtual bool oop_is_instanceRef()         const { return false; }
  virtual bool oop_is_array()               const { return false; }
  virtual bool oop_is_objArray()            const { return false; }
          bool oop_is_objArray_fast()       const { return _is_objArray; }
  virtual bool oop_is_symbol()              const { return false; }
  virtual bool oop_is_klass()               const { return false; }
  virtual bool oop_is_thread()              const { return false; }
  virtual bool oop_is_method()              const { return false; }
  virtual bool oop_is_constMethod()	    const { return false; }
#ifndef CORE
  virtual bool oop_is_methodData()          const { return false; }
#endif // !CORE
  virtual bool oop_is_constantPool()        const { return false; }
  virtual bool oop_is_constantPoolCache()   const { return false; }
  virtual bool oop_is_typeArray()           const { return false; }
  virtual bool oop_is_arrayKlass()          const { return false; }
  virtual bool oop_is_objArrayKlass()       const { return false; }
  virtual bool oop_is_typeArrayKlass()      const { return false; }
  virtual bool oop_is_compiledICHolder()    const { return false; }
  virtual bool oop_is_instanceKlass()       const { return false; }
          
  void set_is_objArray(bool b)   { _is_objArray = b; }

  // Unless overridden, oop is parsable if it has a klass pointer.
  virtual bool oop_is_parsable(oop obj) const { return true; }

  // Access flags
  AccessFlags access_flags() const         { return _access_flags;  }
  void set_access_flags(AccessFlags flags) { _access_flags = flags; }

  bool is_public() const                { return _access_flags.is_public(); }
  bool is_final() const                 { return _access_flags.is_final(); }
  bool is_interface() const             { return _access_flags.is_interface(); }
  bool is_abstract() const              { return _access_flags.is_abstract(); }
  bool is_super() const                 { return _access_flags.is_super(); }
  bool is_synthetic() const             { return _access_flags.is_synthetic(); }
  void set_is_synthetic()               { _access_flags.set_is_synthetic(); }
  bool has_finalizer() const            { return _access_flags.has_finalizer(); }
  void set_has_finalizer()              { _access_flags.set_has_finalizer(); }
  bool is_cloneable() const             { return _access_flags.is_cloneable(); }
  void set_is_cloneable()               { _access_flags.set_is_cloneable(); }
  bool has_vanilla_constructor() const  { return _access_flags.has_vanilla_constructor(); }
  void set_has_vanilla_constructor()    { _access_flags.set_has_vanilla_constructor(); }
  bool has_miranda_methods () const     { return access_flags().has_miranda_methods(); }
  void set_has_miranda_methods()        { _access_flags.set_has_miranda_methods(); }
  bool can_be_fastpath_allocated()const { return access_flags().can_be_fastpath_allocated(); }
  void set_can_be_fastpath_allocated()  { _access_flags.set_can_be_fastpath_allocated(); }


  // garbage collection support
  virtual void follow_weak_klass_links(
    BoolObjectClosure* is_alive, OopClosure* keep_alive);

  // Prefetch within oop iterators.  This is a macro because we
  // can't guarantee that the compiler will inline it.  In 64-bit
  // it generally doesn't.  Signature is
  //
  // static void prefetch_beyond(oop* const start,
  //                             oop* const end,
  //                             const intx foffset,
  //                             const Prefetch::style pstyle);
#define prefetch_beyond(start, end, foffset, pstyle) {   \
    const intx foffset_ = (foffset);                     \
    const Prefetch::style pstyle_ = (pstyle);            \
    assert(foffset_ > 0, "prefetch beyond, not behind"); \
    if (pstyle_ != Prefetch::do_none) {                  \
      oop* ref = (start);                                \
      if (ref < (end)) {                                 \
        switch (pstyle_) {                               \
        case Prefetch::do_read:                          \
          Prefetch::read(*ref, foffset_);                \
          break;                                         \
        case Prefetch::do_write:                         \
          Prefetch::write(*ref, foffset_);               \
          break;                                         \
        default:                                         \
          ShouldNotReachHere();                          \
          break;                                         \
        }                                                \
      }                                                  \
    }                                                    \
  }

  // iterators
  virtual int oop_oop_iterate(oop obj, OopClosure* blk) = 0;
  virtual int oop_oop_iterate_v(oop obj, OopClosure* blk) {
    return oop_oop_iterate(obj, blk);
  }

  // Iterates "blk" over all the oops in "obj" (of type "this") within "mr".
  // (I don't see why the _m should be required, but without it the Solaris
  // C++ gives warning messages about overridings of the "oop_oop_iterate"
  // defined above "hiding" this virtual function.  (DLD, 6/20/00)) */
  virtual int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) = 0;
  virtual int oop_oop_iterate_v_m(oop obj, OopClosure* blk, MemRegion mr) {
    return oop_oop_iterate_m(obj, blk, mr);
  }

  // Versions of the above iterators specialized to particular subtypes
  // of OopClosure, to avoid closure virtual calls.
#define Klass_OOP_OOP_ITERATE_DECL(OopClosureType, nv_suffix)                \
  virtual int oop_oop_iterate##nv_suffix(oop obj, OopClosureType* blk) {     \
    /* Default implementation reverts to general version. */                 \
    return oop_oop_iterate(obj, blk);                                        \
  }                                                                          \
                                                                             \
  /* Iterates "blk" over all the oops in "obj" (of type "this") within "mr". \
     (I don't see why the _m should be required, but without it the Solaris  \
     C++ gives warning messages about overridings of the "oop_oop_iterate"   \
     defined above "hiding" this virtual function.  (DLD, 6/20/00)) */       \
  virtual int oop_oop_iterate##nv_suffix##_m(oop obj,                        \
                                             OopClosureType* blk,            \
                                             MemRegion mr) {                 \
    return oop_oop_iterate_m(obj, blk, mr);                                  \
  }

  SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_1(Klass_OOP_OOP_ITERATE_DECL)
  SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_2(Klass_OOP_OOP_ITERATE_DECL)
  SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_3(Klass_OOP_OOP_ITERATE_DECL)

  virtual void array_klasses_do(void f(klassOop k)) {}
  virtual void with_array_klasses_do(void f(klassOop k));

  // Return self, except for abstract classes with exactly 1
  // implementor.  Then return the 1 concrete implementation.
  Klass *up_cast_abstract();

  // klass name
  symbolOop name() const                   { return _name; }
  void set_name(symbolOop n)               { oop_store_without_check((oop*) &_name, (oop) n); }

  friend class klassKlass;

 public:
  // jvm support
  virtual jint compute_modifier_flags(TRAPS) const;

 public:
  // JVMTI support
  virtual jint jvmti_class_status() const;

#ifndef PRODUCT
 public:
  // Printing
  virtual void oop_print_on      (oop obj, outputStream* st);
  virtual void oop_print_value_on(oop obj, outputStream* st);
  virtual const char* internal_name() const = 0;

  // Verification
  virtual void oop_verify_on(oop obj, outputStream* st);
  virtual void oop_verify_old_oop(oop obj, oop* p, bool allow_dirty);
  // tells whether obj is partially constructed (gc during class loading)
  virtual bool oop_partially_loaded(oop obj) const { return false; }
  virtual void oop_set_partially_loaded(oop obj) {};
  void verify_vtable_index(int index);
#endif
};

