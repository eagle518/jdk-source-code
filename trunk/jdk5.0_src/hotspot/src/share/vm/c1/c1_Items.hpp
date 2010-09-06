#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Items.hpp	1.44 03/12/23 16:39:08 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// The classes responsible for code emission and register allocation


// class hierarchy:
//    Item
//      HintItem (instr = NULL, type explicit)
//      RootItem (asigned to root instructions, not stack allocated)

class Item;
class   HintItem;
class   RootItem;


class Item: public CompilationResourceObj {
 protected:
  enum ItemMode {
    regMode,
    stackMode,
    constMode,
    noResult,
    noMode
  };
  bool       _round32;         // true if the item must be in 32 bit precision
  ItemMode   _mode;            // see enumerator
  int        _loc;             // hold RInfo or stack slot index
  Value      _instr;           // instr specifies the type of item
  enum {
    not_spilled = -1
  };
  int       _spill_ix;           // < 0 if not spilled
  bool      _destroys_register;  // true if the register specified by this item will be destroyed
#ifdef ASSERT
  bool      _uninitialized;
#endif

 public:
  Item(Value instr = NULL):
     _instr(NULL), 
     _mode(noMode), 
     _spill_ix(not_spilled), 
     _destroys_register(false), 
     _loc(0),
#ifdef ASSERT
     _uninitialized(true),
#endif
     _round32(false) { if (instr != NULL) set_instruction(instr); }

  Item(const Item& item) :
    _instr(item._instr),
    _mode(item._mode),
    _spill_ix(item._spill_ix),
    _destroys_register(item._destroys_register),
    _loc(item._loc),
#ifdef ASSERT
     _uninitialized(item._uninitialized),
#endif
    _round32(item._round32)  {}

  ~Item() {
    if (_instr != NULL) { // hints have NULL instructions
      if (!_instr->is_root()) {
        assert(_instr->item() == this || _uninitialized, "wrong instr-item pairing");
        _instr->clear_item();
      }
    }
  }

  void set_instruction(Value instr) {
#ifdef ASSERT
    assert(_uninitialized, "must be the first time this is used");
    _uninitialized = false;
#endif
    assert(instr != NULL, "");
    _instr = instr;
    if (!instr->is_root()) {
      assert(instr->item() == NULL, "already set");
      instr->set_item(this);
    }
  }

  // testers
  bool is_root_item() { return as_root() != NULL; }
  bool is_hint_item() { return as_hint() != NULL; }

  virtual HintItem* as_hint() { return NULL;  }
  virtual RootItem* as_root() { return NULL;  }

  virtual bool is_same(Item* item); // slow comparision

  bool is_register () const { assert(!_uninitialized, "uninited"); return _mode == regMode && _spill_ix == not_spilled;   }
  bool is_stack    () const { assert(!_uninitialized, "uninited"); return _mode == stackMode || _spill_ix != not_spilled; }
  bool is_constant () const { assert(!_uninitialized, "uninited"); return _mode == constMode;       }
  bool is_spilled  () const { assert(!_uninitialized, "uninited"); return _spill_ix != not_spilled; }
  bool has_result  () const { assert(!_uninitialized, "uninited"); return _mode != noMode; }
  bool has_hint    () const { assert(!_uninitialized, "uninited"); return _mode != noMode; }
  bool is_valid    () const { assert(!_uninitialized, "uninited"); return _mode != noMode; }

  bool is_object() const;

  // getters
  virtual ValueType* type() const { return _instr->type(); }
  bool  is_round32  ()   const { return _round32;        }
  RInfo get_register()   const { assert(is_register(),""); return RInfo(_loc, 'a'); }
  int   get_stack   ()   const { assert(is_stack(),   ""); return _loc; }

  ciObject* get_jobject_constant() const;
  jint      get_jint_constant() const;
  jlong     get_jlong_constant() const;
  jfloat    get_jfloat_constant() const;
  jdouble   get_jdouble_constant() const;
  jint      get_address_constant() const;

  Value value()             const { return _instr;    }
  int   get_spilled_index() const { return _spill_ix; }

  // setters
  void set_round32 ()        { _round32 = true; }

  virtual void set_from_item(const Item* item) { 
    _mode = item->_mode; 
    _loc = item->_loc; 
    _spill_ix = item->_spill_ix;
    _round32 = item->_round32;
    _instr = item->value();
  }

  void set_register(RInfo reg) { 
    _mode = regMode;   _loc = reg.number(); _spill_ix = not_spilled; 
  }

  void set_stack   (int index) { _mode = stackMode; _loc = index; _spill_ix = not_spilled; }
  void set_constant()          { _mode = constMode; _spill_ix = not_spilled;}

  void set_no_result ()             { _mode = noMode; }
  void set_spill_ix  (int spill_ix) { _mode = regMode; _spill_ix = spill_ix; }

  void set_destroys_register()      { _destroys_register = true; }
  bool destroys_register() const    { return _destroys_register; }

  // The item stored in root is more up to date: copy state
  // of that item to yourself
  void update();

  // sets _destroys flag if item is float or double
  void handle_float_kind();

  void print() PRODUCT_RETURN;
};


define_array(ItemArray, Item*)
define_stack(ItemList, ItemArray)


class HintItem: public Item {
 private:
  static HintItem _no_hint;
  ValueType*      _type;
 public:
  HintItem(const HintItem& hint) : Item(hint), _type(hint._type) {
  }
 
  HintItem(ValueType* type, bool is_no_hint=false) : Item(), _type(type) { 
    DEBUG_ONLY(_uninitialized = false;)
    assert(is_no_hint || type != NULL, "type must be set");
  }

  HintItem(ValueType* type, const RInfo reg, bool cached = false) : Item(), _type(type) {
    DEBUG_ONLY(_uninitialized = false;)
    assert(reg.is_valid(), "invalid register");
    set_register(reg);
  }

  virtual HintItem*  as_hint()    { return this;  }
  virtual ValueType* type() const { return _type; }

  virtual bool is_same(Item* item);
  virtual void set_from_item(const Item* item);

  static HintItem* no_hint()   { return &_no_hint;   }
};


class RootItem: public Item {
 public:
  RootItem(Value instr): Item(instr) { }
  virtual RootItem* as_root() { return this; }
};
