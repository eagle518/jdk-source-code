#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)memnode.hpp	1.101 03/12/23 16:42:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

class MultiNode;
class PhaseCCP;
class PhaseTransform;

//------------------------------MemNode----------------------------------------
// Load or Store, possibly throwing a NULL pointer exception
class MemNode : public Node {
protected:
#ifdef ASSERT
  const TypePtr* _adr_type;     // What kind of memory is being addressed?
#endif
  virtual uint size_of() const; // Size is bigger (ASSERT only)
public:
  enum { Control,               // When is it safe to do this load?
         Memory,                // Chunk of memory is being loaded from
         Address,               // Actually address, derived from base
         ValueIn,               // Value to store
         OopStore               // Preceeding oop store, only in StoreCM
  };
protected:
  MemNode( Node *c0, Node *c1, Node *c2, const TypePtr* at )
    : Node(c0,c1,c2   ) { debug_only(_adr_type=at; adr_type();) }
  MemNode( Node *c0, Node *c1, Node *c2, const TypePtr* at, Node *c3 )
    : Node(c0,c1,c2,c3) { debug_only(_adr_type=at; adr_type();) }
  MemNode( Node *c0, Node *c1, Node *c2, const TypePtr* at, Node *c3, Node *c4)
    : Node(c0,c1,c2,c3,c4) { debug_only(_adr_type=at; adr_type();) }

public:
  // Is this Node a MemNode or some descendent?  Default is YES.
  virtual MemNode *is_Mem() { return this; }
  virtual Node *Ideal_DU_postCCP( PhaseCCP *ccp );

  virtual const class TypePtr *adr_type() const;  // returns bottom_type of address
  
  // Shared code for Ideal methods:
  Node *Ideal_common(PhaseGVN *phase, bool can_reshape);  // Return -1 for short-circuit NULL.

  // Helper function for adr_type() implementations.
  static const TypePtr* calculate_adr_type(const Type* t, const TypePtr* cross_check = NULL);

  // Raw access function, to allow copying of adr_type efficiently in
  // product builds and retain the debug info for debug builds.
  const TypePtr *raw_adr_type() const { 
#ifdef ASSERT
    return _adr_type;
#else
    return 0;
#endif
  }

#ifndef PRODUCT
  static void dump_adr_type(const Node* mem, const TypePtr* adr_type);
  virtual void dump_spec() const;
#endif
};

//------------------------------LoadNode---------------------------------------
// Load value; requires Memory and Address
class LoadNode : public MemNode {
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const; // Size is bigger
protected:
  const Type* const _type;      // What kind of value is loaded?
public:
         
  LoadNode( Node *c, Node *mem, Node *adr, const TypePtr* at, const Type *rt )
    : MemNode(c,mem,adr,at), _type(rt) {}

  // Polymorphic factory method:
  static LoadNode* make( Node *c, Node *mem, Node *adr, const TypePtr* at, const Type *rt, BasicType bt );

  virtual uint hash()   const;  // Check the type

  // Handle algebraic identities here.  If we have an identity, return the Node
  // we are equivalent to.  We look for Load of a Store.
  virtual Node *Identity( PhaseTransform *phase );

  // If the load is from Field memory and the pointer is non-null, we can
  // zero out the control input.
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);

  // Compute a new Type for this node.  Basically we just do the pre-check,
  // then call the virtual add() to set the type.
  virtual const Type *Value( PhaseTransform *phase ) const;

  virtual uint ideal_reg() const;
  virtual const Type *bottom_type() const;
  virtual void raise_bottom_type(const Type* new_type);
  // Following method is copied from TypeNode:
  void set_type(const Type* t) {
    assert(t != NULL, "sanity");
    debug_only(uint check_hash = (VerifyHashTableKeys && _hash_lock) ? hash() : NO_HASH);
    *(const Type**)&_type = t;   // cast away const-ness
    // If this node is in the hash table, make sure it doesn't need a rehash.
    assert(check_hash == NO_HASH || check_hash == hash(), "type change must preserve hash code");
  }

  // Is this Node an LoadNode or some descendent?  Default is YES.
  virtual LoadNode *is_Load() { return this; }
  // Do not match memory edge
  virtual uint match_edge(uint idx) const;

  // Map a load opcode to it's corresponding store opcode.
  virtual int store_Opcode() const = 0;

#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};

//------------------------------LoadBNode--------------------------------------
// Load a byte (8bits signed) from memory
class LoadBNode : public LoadNode {
public:
  LoadBNode( Node *c, Node *mem, Node *adr, const TypePtr* at, const TypeInt *ti = TypeInt::BYTE )
    : LoadNode(c,mem,adr,at,ti) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual int store_Opcode() const { return Op_StoreB; }
};

//------------------------------LoadCNode--------------------------------------
// Load a char (16bits unsigned) from memory
class LoadCNode : public LoadNode {
public:
  LoadCNode( Node *c, Node *mem, Node *adr, const TypePtr* at, const TypeInt *ti = TypeInt::CHAR ) 
    : LoadNode(c,mem,adr,at,ti) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual int store_Opcode() const { return Op_StoreC; }
};

//------------------------------LoadINode--------------------------------------
// Load an integer from memory
class LoadINode : public LoadNode {
public:
  LoadINode( Node *c, Node *mem, Node *adr, const TypePtr* at, const TypeInt *ti = TypeInt::INT ) 
    : LoadNode(c,mem,adr,at,ti) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual int store_Opcode() const { return Op_StoreI; }
};

//------------------------------LoadRangeNode----------------------------------
// Load an array length from the array
class LoadRangeNode : public LoadINode {
public:
  LoadRangeNode( Node *c, Node *mem, Node *adr, const TypeInt *ti = TypeInt::POS )
    : LoadINode(c,mem,adr,TypeAryPtr::RANGE,ti) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
};

//------------------------------LoadLNode--------------------------------------
// Load a long from memory
class LoadLNode : public LoadNode {
public:
  LoadLNode( Node *c, Node *mem, Node *adr, const TypePtr* at, const TypeLong *tl = TypeLong::LONG )
    : LoadNode(c,mem,adr,at,tl) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegL; }
  virtual int store_Opcode() const { return Op_StoreL; }
};

//------------------------------LoadL_unalignedNode----------------------------
// Load a long from unaligned memory
class LoadL_unalignedNode : public LoadLNode {
public:
  LoadL_unalignedNode( Node *c, Node *mem, Node *adr, const TypePtr* at ) 
    : LoadLNode(c,mem,adr,at) {}
  virtual int Opcode() const;
};

//------------------------------LoadFNode--------------------------------------
// Load a float (64 bits) from memory
class LoadFNode : public LoadNode {
public:
  LoadFNode( Node *c, Node *mem, Node *adr, const TypePtr* at, const Type *t = Type::FLOAT ) 
    : LoadNode(c,mem,adr,at,t) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegF; }
  virtual int store_Opcode() const { return Op_StoreF; }
};

//------------------------------LoadDNode--------------------------------------
// Load a double (64 bits) from memory
class LoadDNode : public LoadNode {
public:
  LoadDNode( Node *c, Node *mem, Node *adr, const TypePtr* at, const Type *t = Type::DOUBLE ) 
    : LoadNode(c,mem,adr,at,t) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegD; }
  virtual int store_Opcode() const { return Op_StoreD; }
};

//------------------------------LoadD_unalignedNode----------------------------
// Load a double from unaligned memory
class LoadD_unalignedNode : public LoadDNode {
public:
  LoadD_unalignedNode( Node *c, Node *mem, Node *adr, const TypePtr* at ) 
    : LoadDNode(c,mem,adr,at) {}
  virtual int Opcode() const;
};

//------------------------------LoadPNode--------------------------------------
// Load a pointer from memory (either object or array)
class LoadPNode : public LoadNode {
public:
  LoadPNode( Node *c, Node *mem, Node *adr, const TypePtr *at, const TypePtr* t ) 
    : LoadNode(c,mem,adr,at,t) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegP; }
  virtual int store_Opcode() const { return Op_StoreP; }
  // depends_only_on_test is almost always true, and needs to be almost always
  // true to enable key hoisting & commoning optimizations.  However, for the
  // special case of RawPtr loads from TLS top & end, the control edge carries
  // the dependence preventing hoisting past a Safepoint instead of the memory
  // edge.  (An unfortunate consequence of having Safepoints not set Raw
  // Memory; itself an unfortunate consequence of having Nodes which produce
  // results (new raw memory state) inside of loops preventing all manner of
  // other optimizations).  Basically, it's ugly but so is the alternative.
  // See comment in graphkit.cpp, around line 1923 GraphKit::allocate_heap.
  virtual bool depends_only_on_test() const { return adr_type() != TypeRawPtr::BOTTOM; } 
};

//------------------------------LoadKlassNode----------------------------------
// Load a Klass from an object
class LoadKlassNode : public LoadPNode {
public:
  LoadKlassNode( Node *c, Node *mem, Node *adr, const TypePtr *at, const TypeKlassPtr *tk = TypeKlassPtr::OBJECT )
    : LoadPNode(c,mem,adr,at,tk) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual bool depends_only_on_test() const { return true; } 
};

//------------------------------LoadSNode--------------------------------------
// Load a short (16bits signed) from memory
class LoadSNode : public LoadNode {
public:
  LoadSNode( Node *c, Node *mem, Node *adr, const TypePtr* at, const TypeInt *ti = TypeInt::SHORT ) 
    : LoadNode(c,mem,adr,at,ti) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual int store_Opcode() const { return Op_StoreC; }
};

//------------------------------StoreNode--------------------------------------
// Store value; requires Store, Address and Value
class StoreNode : public MemNode {
  virtual uint cmp( const Node &n ) const;
  virtual bool depends_only_on_test() const { return false; }

protected:
  Node *Ideal_masked_input       (PhaseGVN *phase, uint mask);
  Node *Ideal_sign_extended_input(PhaseGVN *phase, int  num_bits);

public:
  StoreNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val )
    : MemNode(c,mem,adr,at,val) {}
  StoreNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val, Node *oop_store )
    : MemNode(c,mem,adr,at,val,oop_store) {}

  // Polymorphic factory method:
  static StoreNode* make( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val, BasicType bt );

  virtual uint hash() const;    // Check the type

  // If the store is to Field memory and the pointer is non-null, we can
  // zero out the control input.
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);

  // Compute a new Type for this node.  Basically we just do the pre-check,
  // then call the virtual add() to set the type.
  virtual const Type *Value( PhaseTransform *phase ) const;

  // Check for identity function on memory (Load then Store at same address)
  virtual Node *Identity( PhaseTransform *phase );

  // Is this Node an StoreNode or some descendent?  Default is YES.
  virtual const StoreNode *is_Store() const { return this; }
  // Do not match memory edge
  virtual uint match_edge(uint idx) const;

  virtual const Type *bottom_type() const;  // returns Type::MEMORY
};

//------------------------------StoreBNode-------------------------------------
// Store byte to memory
class StoreBNode : public StoreNode {
public:
  StoreBNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val ) : StoreNode(c,mem,adr,at,val) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
};

//------------------------------StoreCNode-------------------------------------
// Store char/short to memory
class StoreCNode : public StoreNode {
public:
  StoreCNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val ) : StoreNode(c,mem,adr,at,val) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
};

//------------------------------StoreINode-------------------------------------
// Store int to memory
class StoreINode : public StoreNode {
public:
  StoreINode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val ) : StoreNode(c,mem,adr,at,val) {}
  virtual int Opcode() const;
};

//------------------------------StoreLNode-------------------------------------
// Store long to memory
class StoreLNode : public StoreNode {
public:
  StoreLNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val ) : StoreNode(c,mem,adr,at,val) {}
  virtual int Opcode() const;
};

//------------------------------StoreFNode-------------------------------------
// Store float to memory
class StoreFNode : public StoreNode {
public:
  StoreFNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val ) : StoreNode(c,mem,adr,at,val) {}
  virtual int Opcode() const;
};

//------------------------------StoreDNode-------------------------------------
// Store double to memory
class StoreDNode : public StoreNode {
public:
  StoreDNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val ) : StoreNode(c,mem,adr,at,val) {}
  virtual int Opcode() const;
};

//------------------------------StorePNode-------------------------------------
// Store pointer to memory
class StorePNode : public StoreNode {
public:
  StorePNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val ) : StoreNode(c,mem,adr,at,val) {}
  virtual int Opcode() const;
};

//------------------------------StoreCMNode-----------------------------------
// Store card-mark byte to memory for CM
// The last StoreCM before a SafePoint must be preserved and occur after its "oop" store
// Preceeding equivalent StoreCMs may be eliminated.
class StoreCMNode : public StoreNode {
public:
  StoreCMNode( Node *c, Node *mem, Node *adr, const TypePtr* at, Node *val, Node *oop_store ) : StoreNode(c,mem,adr,at,val,oop_store) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual const Type *Value( PhaseTransform *phase ) const;
};

//------------------------------LoadPLockedNode---------------------------------
// Load-locked a pointer from memory (either object or array).
// On Sparc & Intel this is implemented as a normal pointer load.
// On PowerPC and friends it's a real load-locked.
class LoadPLockedNode : public LoadPNode {
public:
  LoadPLockedNode( Node *c, Node *mem, Node *adr )
    : LoadPNode(c,mem,adr,TypeRawPtr::BOTTOM, TypeRawPtr::BOTTOM) {}
  virtual int Opcode() const;
  virtual int store_Opcode() const { return Op_StorePConditional; }
  virtual bool depends_only_on_test() const { return true; } 
};

//------------------------------LoadLLockedNode---------------------------------
// Load-locked a pointer from memory (either object or array).
// On Sparc & Intel this is implemented as a normal long load.
class LoadLLockedNode : public LoadLNode {
public:
  LoadLLockedNode( Node *c, Node *mem, Node *adr )
    : LoadLNode(c,mem,adr,TypeRawPtr::BOTTOM, TypeLong::LONG) {}
  virtual int Opcode() const;
  virtual int store_Opcode() const { return Op_StoreLConditional; }
};

//------------------------------SCMemProjNode---------------------------------------
// This class defines a projection of the memory  state of a store conditional node.
// These nodes return a value, but also update memory.
class SCMemProjNode : public ProjNode {
public:
  enum {SCMEMPROJCON = -2};
  SCMemProjNode( Node *src) : ProjNode( src, SCMEMPROJCON) { }
  virtual int Opcode() const;
  virtual bool      is_CFG() const  { return false; }
  virtual bool depends_only_on_test() const { return false; }
  virtual const Type *bottom_type() const {return Type::MEMORY;}
  virtual const TypePtr *adr_type() const { return in(0)->in(MemNode::Memory)->adr_type();}
  virtual uint ideal_reg() const { return 0;} // memory projections don't have a register
  virtual const Type *Value( PhaseTransform *phase ) const;
#ifndef PRODUCT
  virtual void dump_spec() const {};
#endif
};

//------------------------------StorePConditionalNode---------------------------
// Conditionally store pointer to memory, if no change since prior
// load-locked.  Sets flags for success or failure of the store.
class StorePConditionalNode : public Node {
public:
  enum {
    LL_input = MemNode::ValueIn+1 // One more input to a StorePConditional
  };
  StorePConditionalNode( Node *c, Node *mem, Node *adr, Node *val, Node *ll );
  virtual int Opcode() const;
  // Produces flags
  virtual const Type *bottom_type() const { return TypeInt::BOOL; }
  virtual uint ideal_reg() const { return Op_RegFlags; }
  virtual bool is_CFG() const { return false; }
  virtual bool depends_only_on_test() const { return false; }
  // Do not match memory edge
  virtual uint match_edge(uint idx) const { return idx == MemNode::Address || idx == MemNode::ValueIn; }
};

//------------------------------StoreLConditionalNode---------------------------
// Conditionally store long to memory, if no change since prior
// load-locked.  Sets flags for success or failure of the store.
class StoreLConditionalNode : public Node {
public:
  enum {
    LL_input = MemNode::ValueIn+1 // One more input to a StoreLConditional
  };
  StoreLConditionalNode( Node *c, Node *mem, Node *adr, Node *val, Node *ll );
  virtual int Opcode() const;
  // Produces flags
  virtual const Type *bottom_type() const { return TypeInt::BOOL; }
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual bool is_CFG() const { return false; }
  virtual bool depends_only_on_test() const { return false; }
  // Do not match memory edge
  virtual uint match_edge(uint idx) const { return idx == MemNode::Address || idx == MemNode::ValueIn; }
};

//------------------------------CompareAndSwapNode---------------------------
class CompareAndSwapNode : public Node {
public:
  enum {
    ExpectedIn = MemNode::ValueIn+1 // One more input than MemNode
  };
  CompareAndSwapNode( Node *c, Node *mem, Node *adr, Node *val, Node *ex);
  virtual bool is_CFG() const { return false; }
  virtual bool depends_only_on_test() const { return false; }
  virtual const Type *bottom_type() const { return TypeInt::BOOL; }
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual uint match_edge(uint idx) const { return idx == MemNode::Address || idx == MemNode::ValueIn; }
};


//------------------------------CompareAndSwapLNode---------------------------
class CompareAndSwapLNode : public CompareAndSwapNode {
public:
  CompareAndSwapLNode( Node *c, Node *mem, Node *adr, Node *val, Node *ex);
  virtual int Opcode() const;
};


//------------------------------CompareAndSwapINode---------------------------
class CompareAndSwapINode : public CompareAndSwapNode {
public:
  CompareAndSwapINode( Node *c, Node *mem, Node *adr, Node *val, Node *ex);
  virtual int Opcode() const;
};


//------------------------------CompareAndSwapPNode---------------------------
class CompareAndSwapPNode : public CompareAndSwapNode {
public:
  CompareAndSwapPNode( Node *c, Node *mem, Node *adr, Node *val, Node *ex);
  virtual int Opcode() const;
};

//------------------------------ClearArray-------------------------------------
class ClearArrayNode: public Node {
public:
  ClearArrayNode( Node *ctrl, Node *arymem, Node *word_cnt, Node *base ) : Node(ctrl,arymem,word_cnt,base) {}
  virtual int         Opcode() const;
  virtual const Type *bottom_type() const { return Type::MEMORY; }
  // ClearArray modifies array elements, and so affects only the
  // array memory addressed by the bottom_type of its base address.
  virtual const class TypePtr *adr_type() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint match_edge(uint idx) const;
};

//------------------------------StrComp-------------------------------------
class StrCompNode: public Node {
public:
  StrCompNode(Node *control,
              Node* char_array_mem,
              Node* value_mem,
              Node* count_mem,
              Node* offset_mem,
              Node* s1, Node* s2): Node(control,
                                        char_array_mem,
                                        value_mem,
                                        count_mem,
                                        offset_mem,
                                        s1, s2) {};
  virtual int Opcode() const;
  virtual bool depends_only_on_test() const { return false; }
  virtual const Type* bottom_type() const { return TypeInt::INT; }
  // a StrCompNode (conservatively) aliases with everything:
  virtual const TypePtr* adr_type() const { return TypePtr::BOTTOM; }
  virtual uint match_edge(uint idx) const;
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
};

//------------------------------MemBar-----------------------------------------
// There are different flavors of Memory Barriers to match the Java Memory
// Model.  Monitor-enter and volatile-load act as Aquires: no following ref
// can be moved to before them.  We insert a MemBar-Acquire after a FastLock or
// volatile-load.  Monitor-exit and volatile-store act as Release: no
// preceeding ref can be moved to after them.  We insert a MemBar-Release
// before a FastUnlock or volatile-store.  All volatiles need to be
// serialized, so we follow all volatile-stores with a MemBar-Volatile to
// seperate it from any following volatile-load.
class MemBarNode: public MultiNode {
  virtual uint hash() const ;                  // { return NO_HASH; }
  virtual uint cmp( const Node &n ) const ;    // Always fail, except on self
public:
  MemBarNode();
  virtual int Opcode() const = 0;
  virtual const MemBarNode *is_MemBar() const { return this; }
  virtual const class TypePtr *adr_type() const { return TypePtr::BOTTOM; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint ideal_reg() const { return NotAMachineReg; }
  virtual uint match_edge(uint idx) const { return 0; }
  virtual const Type *bottom_type() const { return TypeTuple::MEMBAR; }
  virtual Node *match( const ProjNode *proj, const Matcher *m );
};

// "Acquire" - no following ref can move before (but earlier refs can
// follow, like an early Load stalled in cache).  Requires multi-cpu
// visibility.  Inserted after a volatile load or FastLock.
class MemBarAcquireNode: public MemBarNode {
public:
  MemBarAcquireNode() : MemBarNode() {}
  virtual int Opcode() const;
};

// "Release" - no earlier ref can move after (but later refs can move
// up, like a speculative pipelined cache-hitting Load).  Requires
// multi-cpu visibility.  Inserted before a volatile store or FastUnLock.
class MemBarReleaseNode: public MemBarNode {
public:
  MemBarReleaseNode() : MemBarNode() {}
  virtual int Opcode() const;
};

// Ordering between a volatile store and a following volatile load.
// Requires multi-CPU visibility?
class MemBarVolatileNode: public MemBarNode {
public:
  MemBarVolatileNode() : MemBarNode() {}
  virtual int Opcode() const;
};

// Ordering within the same CPU.  Used to order unsafe memory references
// inside the compiler when we lack alias info.  Not needed "outside" the
// compiler because the CPU does all the ordering for us.
class MemBarCPUOrderNode: public MemBarNode {
public:
  MemBarCPUOrderNode() : MemBarNode() {}
  virtual int Opcode() const;
};

//------------------------------MergeMem---------------------------------------
// (See comment in memnode.cpp near MergeMemNode::MergeMemNode for semantics.)
class MergeMemNode: public Node {
  virtual uint hash() const ;                  // { return NO_HASH; }
  virtual uint cmp( const Node &n ) const ;    // Always fail, except on self
  friend class MergeMemStream;
public:
  MergeMemNode(Node *def);
  virtual int Opcode() const;
  virtual MergeMemNode *is_MergeMem() { return this; }
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint ideal_reg() const { return NotAMachineReg; }
  virtual uint match_edge(uint idx) const { return 0; }
  virtual const RegMask &out_RegMask() const;
  virtual const Type *bottom_type() const { return Type::MEMORY; }
  virtual const TypePtr *adr_type() const { return TypePtr::BOTTOM; }
  // sparse accessors
  // Fetch the previously stored "set_memory_at", or else the base memory.
  // (Caller should clone it if it is a phi-nest.)
  Node* memory_at(uint alias_idx) const;
  // set the memory, regardless of its previous value
  void set_memory_at(uint alias_idx, Node* n);
  // the "base" is the memory that provides the non-finite support
  Node* base_memory() const       { return in(Compile::AliasIdxBot); }
  // warning: setting the base can implicitly set any of the other slices too
  void set_base_memory(Node* def);
  // sentinel value which denotes a copy of the base memory:
  Node*   empty_memory() const    { return in(Compile::AliasIdxTop); }
  static Node* make_empty_memory(); // where the sentinel comes from
  bool is_empty_memory(Node* n) const { assert((n == empty_memory()) == n->is_top(), "sanity"); return n->is_top(); }
  // Clone a whole memory state, with all its slices intact.
  static MergeMemNode* clone_all_memory(Node* mem);
  // hook for the iterator, to perform any necessary setup
  void iteration_setup(const MergeMemNode* other = NULL);
  // push sentinels until I am at least as long as the other (semantic no-op)
  void grow_to_match(const MergeMemNode* other);
  bool verify_sparse() const PRODUCT_RETURN0;
#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};

class MergeMemStream : public StackObj {
 private:
  MergeMemNode*       _mm;
  const MergeMemNode* _mm2;  // optional second guy, contributes non-empty iterations
  Node*               _mm_base;  // loop-invariant base memory of _mm
  int                 _idx;
  int                 _cnt;
  Node*               _mem;
  Node*               _mem2;
  int                 _cnt2;

  void init(MergeMemNode* mm, const MergeMemNode* mm2 = NULL) {
    // subsume_node will break sparseness at times, whenever a memory slice
    // folds down to a copy of the base ("fat") memory.  In such a case,
    // the raw edge will update to base, although it should be top.
    // This iterator will recognize either top or base_memory as an
    // "empty" slice.  See is_empty, is_empty2, and next below.
    //
    // The sparseness property is repaired in MergeMemNode::Ideal.
    // As long as access to a MergeMem goes through this iterator
    // or the memory_at accessor, flaws in the sparseness will
    // never be observed.
    //
    // Also, iteration_setup repairs sparseness.
    assert(mm->verify_sparse(), "please, no dups of base");
    assert(mm2==NULL || mm2->verify_sparse(), "please, no dups of base");

    _mm  = mm;
    _mm_base = mm->base_memory();
    _mm2 = mm2;
    _cnt = mm->req();
    _idx = Compile::AliasIdxBot-1; // start at the base memory
    _mem = NULL;
    _mem2 = NULL;
  }

#ifdef ASSERT
  Node* check_memory() const {
    if (at_base_memory())
      return _mm->base_memory();
    else if ((uint)_idx < _mm->req() && !_mm->in(_idx)->is_top())
      return _mm->memory_at(_idx);
    else
      return _mm_base;
  }
  Node* check_memory2() const {
    return at_base_memory()? _mm2->base_memory(): _mm2->memory_at(_idx);
  }
#endif

  static bool match_memory(Node* mem, const MergeMemNode* mm, int idx) PRODUCT_RETURN0;
  void assert_synch() const {
    assert(!_mem || _idx >= _cnt || match_memory(_mem, _mm, _idx),
           "no side-effects except through the stream");
  }

 public:

  // expected usages:
  // for (MergeMemStream mms(mem->is_MergeMem()); next_non_empty(); ) { ... }
  // for (MergeMemStream mms(mem1, mem2); next_non_empty2(); ) { ... }

  // iterate over one merge
  MergeMemStream(MergeMemNode* mm) {
    mm->iteration_setup();
    init(mm);
    debug_only(_cnt2 = 999);
  }
  // iterate in parallel over two merges
  // only iterates through non-empty elements of mm2
  MergeMemStream(MergeMemNode* mm, const MergeMemNode* mm2) {
    assert(mm2, "second argument must be a MergeMem also");
    ((MergeMemNode*)mm2)->iteration_setup();  // update hidden state
    mm->iteration_setup(mm2);
    init(mm, mm2);
    _cnt2 = mm2->req();
  }
#ifdef ASSERT
  ~MergeMemStream() {
    assert_synch();
  }
#endif

  MergeMemNode* all_memory() const {
    return _mm;
  }
  Node* base_memory() const {
    assert(_mm_base == _mm->base_memory(), "no update to base memory, please");
    return _mm_base;
  }
  const MergeMemNode* all_memory2() const {
    assert(_mm2 != NULL, "");
    return _mm2;
  }
  bool at_base_memory() const {
    return _idx == Compile::AliasIdxBot;
  }
  int alias_idx() const {
    assert(_mem, "must call next 1st");
    return _idx;
  }
  const TypePtr* adr_type() const {
    return Compile::current()->get_adr_type(alias_idx());
  }
  bool is_empty() const {
    assert(_mem, "must call next 1st");
    assert(_mem->is_top() == (_mem==_mm->empty_memory()), "correct sentinel");
    return _mem->is_top();
  }
  bool is_empty2() const {
    assert(_mem2, "must call next 1st");
    assert(_mem2->is_top() == (_mem2==_mm2->empty_memory()), "correct sentinel");
    return _mem2->is_top();
  }
  Node* memory() const {
    assert(!is_empty(), "must not be empty");
    assert_synch();
    return _mem;
  }
  // get the current memory, regardless of empty or non-empty status
  Node* force_memory() const {
    assert(!is_empty() || !at_base_memory(), "");
    // Use _mm_base to defend against updates to _mem->base_memory().
    Node *mem = _mem->is_top() ? _mm_base : _mem;
    assert(mem == check_memory(), "");
    return mem;
  }
  Node* memory2() const {
    assert(_mem2 == check_memory2(), "");
    return _mem2;
  }
  void set_memory(Node* mem) {
    if (at_base_memory()) {
      // Note that this does not change the invariant _mm_base.
      _mm->set_base_memory(mem);
    } else {
      _mm->set_memory_at(_idx, mem);
    }
    _mem = mem;
    assert_synch();
  }

  // Recover from a side effect to the MergeMemNode.
  void set_memory() {
    _mem = _mm->in(_idx);
  }

  bool next()  { return next(false); }
  bool next2() { return next(true); }

  bool next_non_empty()  { return next_non_empty(false); }
  bool next_non_empty2() { return next_non_empty(true); }
  // next_non_empty2 can yield states where is_empty() is true

 private:
  // find the next item, which might be empty
  bool next(bool have_mm2) {
    assert((_mm2 != NULL) == have_mm2, "use other next");
    assert_synch();
    if (++_idx < _cnt) {
      // Note:  This iterator allows _mm to be non-sparse.
      // It behaves the same whether _mem is top or base_memory.
      _mem = _mm->in(_idx);
      if (have_mm2)
        _mem2 = _mm2->in((_idx < _cnt2) ? _idx : Compile::AliasIdxTop);
      return true;
    }
    return false;
  }

  // find the next non-empty item
  bool next_non_empty(bool have_mm2) {
    while (next(have_mm2)) {
      if (!is_empty()) {
        // make sure _mem2 is filled in sensibly
        if (have_mm2 && _mem2->is_top())  _mem2 = _mm2->base_memory();
        return true;
      } else if (have_mm2 && !is_empty2()) {
        return true;   // is_empty() == true
      }
    }
    return false;
  }
};

//------------------------------Prefetch---------------------------------------
// Non-faulting prefetch load.  Prefetch for many reads & many writes.
class PrefetchNode : public Node {
public:
  PrefetchNode(Node *abio, Node *adr) : Node(0,abio,adr) {}
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return NotAMachineReg; }
  virtual uint match_edge(uint idx) const { return idx==2; }
  virtual const Type *bottom_type() const { return Type::ABIO; }
};

