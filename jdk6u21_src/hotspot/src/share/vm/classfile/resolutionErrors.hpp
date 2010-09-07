/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
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

class ResolutionErrorEntry;

// ResolutionError objects are used to record errors encountered during
// constant pool resolution (JVMS 5.4.3).

class ResolutionErrorTable : public Hashtable {

public:
  ResolutionErrorTable(int table_size);

  ResolutionErrorEntry* new_entry(int hash, constantPoolOop pool, int cp_index, symbolOop error);

  ResolutionErrorEntry* bucket(int i) {
    return (ResolutionErrorEntry*)Hashtable::bucket(i);
  }

  ResolutionErrorEntry** bucket_addr(int i) {
    return (ResolutionErrorEntry**)Hashtable::bucket_addr(i);
  }

  void add_entry(int index, ResolutionErrorEntry* new_entry) {
    Hashtable::add_entry(index, (HashtableEntry*)new_entry);
  }

  void add_entry(int index, unsigned int hash,
                 constantPoolHandle pool, int which, symbolHandle error);


  // find error given the constant pool and constant pool index
  ResolutionErrorEntry* find_entry(int index, unsigned int hash,
                                   constantPoolHandle pool, int cp_index);


  unsigned int compute_hash(constantPoolHandle pool, int cp_index) {
    return (unsigned int) pool->identity_hash() + cp_index;
  }

  // purges unloaded entries from the table
  void purge_resolution_errors(BoolObjectClosure* is_alive);

  // this table keeps symbolOops alive
  void always_strong_classes_do(OopClosure* blk);

  // GC support.
  void oops_do(OopClosure* f);
};


class ResolutionErrorEntry : public HashtableEntry {
 private:
  int               _cp_index;
  symbolOop         _error;

 public:
  constantPoolOop    pool() const               { return (constantPoolOop)literal(); }
  constantPoolOop*   pool_addr()                { return (constantPoolOop*)literal_addr(); }

  int                cp_index() const           { return _cp_index; }
  void               set_cp_index(int cp_index) { _cp_index = cp_index; }

  symbolOop          error() const              { return _error; }
  void               set_error(symbolOop e)     { _error = e; }
  symbolOop*         error_addr()               { return &_error; }

  ResolutionErrorEntry* next() const {
    return (ResolutionErrorEntry*)HashtableEntry::next();
  }

  ResolutionErrorEntry** next_addr() {
    return (ResolutionErrorEntry**)HashtableEntry::next_addr();
  }

  // GC support
  void oops_do(OopClosure* blk);
};
