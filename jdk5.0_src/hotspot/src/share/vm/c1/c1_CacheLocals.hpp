#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_CacheLocals.hpp	1.49 03/12/23 16:38:58 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class IR;
class ALocalList;
class RInfoCollection;
class RegisterState;
class LIR_ScanInfo;

class LocalMapping: public CompilationResourceObj {
 private:
  WordSizeList*          _local_name_to_offset_map;
  RInfoCollection*       _mapping;
  RInfoCollection*       _offset_to_register_mapping;
  RegisterManager*       _cached_regs;
  RegisterManager*       _free_regs;

  void init_cached_regs();

  // add and remove registers from the mapping.
  // add affects the free_regs but remove does not.
  void add(int name, RInfo reg);
  void remove(int name);

 public: 
  LocalMapping(WordSizeList* local_name_to_offset_map): 
      _mapping(new RInfoCollection())
    , _local_name_to_offset_map(local_name_to_offset_map)
    , _free_regs(NULL)
    , _offset_to_register_mapping(NULL)
    , _cached_regs(NULL)                            { init_cached_regs(); }

  LocalMapping(WordSizeList* local_name_to_offset_map, RInfoCollection* map, RegisterManager* free_regs = NULL): 
      _mapping(map)
    , _local_name_to_offset_map(local_name_to_offset_map)
    , _free_regs(free_regs)
    , _offset_to_register_mapping(NULL)
    , _cached_regs(NULL)                            { init_cached_regs(); }

  LIR_Opr get_cache_reg(LIR_Opr) const;
  RInfo get_cache_reg(int local_index, ValueTag tag) const;
  RInfo get_cache_reg(int local_index) const;

  // The following accessor is only used during debug information
  // generation and is needed because of the mismatch between local
  // names and how the caching of locals algorithm works (on frame
  // slots). Can be removed once the new register allocator is in
  // place.
  RInfo get_cache_reg_for_local_offset(int local_offset) const;
  
  bool is_cache_reg(LIR_Opr opr) const;
  bool is_cache_reg(RInfo reg) const;

  // Only for oop map generation from LIR; allows generator to find
  // all local names corresponding to a given cached register.
  int  local_names_begin();
  int  local_names_end();                           // 1 beyond last valid local name
  bool is_local_name_cached_in_reg(int local_name, LIR_Opr opr);

  int length () const                               { return _mapping->length(); }

  // intersect the two mappings so that indices that have the same
  // caching register appear in both
  void intersection(LocalMapping* other_mapping);

  // add entries from other_mapping that are compatible with the free
  // regs in this mapping.
  void join(LocalMapping* other_mapping);

  // add entries from other_mapping that refers to locals with aren't
  // cached in this mapping.
  void merge(LocalMapping* other_mapping); 

  // emit code to move values between register and stack to change from one mapping to another
  static void emit_transition(LIR_List* lir, LocalMapping* pred_mapping, LocalMapping* sux_mapping, IR* ir);

  void print() const PRODUCT_RETURN;
};




// utility class for assigning a LocalMapping to a collection of block
class LocalMappingSetter: public BlockClosure {
 private:
  LocalMapping* _mapping;
 public:
  LocalMappingSetter(LocalMapping* mapping): _mapping(mapping) {}

  virtual void block_do(BlockBegin* block);
};



class LIR_LocalCaching: public CompilationResourceObj {
 private:
  IR* _ir;
  LocalMapping* _preferred;

  IR* ir() const                                 { return _ir; }
  const LocalMapping* preferred()                { return _preferred; }

  void cache_locals();
  LocalMapping* cache_locals_for_blocks(BlockList* blocks, RegisterManager* blocks_scan_info, bool is_reference = false);

  // inserts code between blocks to move locals between registers and stack
  void insert_transition_blocks();

  // given a set up locals with a use count and a set of available
  // registers, return a mapping of locals to registers
  LocalMapping* compute_caching(ALocalList* locals, RegisterManager* free_regs);

  // machine dependent mapping of which locals which have a preferred
  // register that they should be assigned to if they are going to be in
  // a register.  mainly used on sparc to make register arguments stay
  // in the register they came in.
  LocalMapping* preferred_locals (const ciMethod* method);

 public:
  LIR_LocalCaching(IR* ir);
  void compute_cached_locals();

  // Helpers used only by this and LocalMapping
  static void add_at_all_names(RInfoCollection* mapping, int offset, RInfo reg, WordSizeList* local_name_to_offset_map);
  static void remove_at_all_names(RInfoCollection* mapping, int offset, WordSizeList* local_name_to_offset_map);
  static bool is_illegal_at_all_names(RInfoCollection* mapping, int offset, WordSizeList* local_name_to_offset_map);
};
