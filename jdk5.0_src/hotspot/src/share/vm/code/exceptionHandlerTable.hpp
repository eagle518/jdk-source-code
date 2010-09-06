#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)exceptionHandlerTable.hpp	1.21 04/04/14 17:27:20 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifdef COMPILER2
// A HandlerTableEntry describes an individual entry of a subtable
// of ExceptionHandlerTable. An entry consists of a pair(bci, pco),
// where bci is the exception handler bci, and pco is the pc offset
// relative to the nmethod code start for the compiled exception
// handler corresponding to the (interpreted) exception handler
// starting at bci.
//
// The first HandlerTableEntry of each subtable holds the length
// and catch_pco for the subtable (the length is the number of
// subtable entries w/o header).

class HandlerTableEntry {
 private:
  int _bci;
  int _pco;

 public:
  HandlerTableEntry(int bci, int pco) {
    assert( 0 <= pco, "pco must be positive");
    _bci = bci;
    _pco = pco;
  }

  int len() const { return _bci; } // for entry at subtable begin
  int bci() const { return _bci; }
  int pco() const { return _pco; }
};


// An ExceptionHandlerTable is an abstraction over a list of subtables
// of exception handlers for CatchNodes. Each subtable has a one-entry
// header holding length and catch_pco of the subtable, followed
// by 'length' entries for each exception handler that can be reached
// from the corresponding CatchNode. The catch_pco is the pc offset of
// the CatchNode in the corresponding nmethod. Empty subtables are dis-
// carded.
//
// Structure of the table:
//
// table    = { subtable }.
// subtable = header entry { entry }.
// header   = a pair (number of subtable entries, catch pc offset)
// entry    = a pair (handler bci, handler pc offset)
//
// An ExceptionHandlerTable can be created from scratch, in which case
// it is possible to add subtables. It can also be created from an
// nmethod (for lookup purposes) in which case the table cannot be
// modified.

class nmethod;
class ExceptionHandlerTable VALUE_OBJ_CLASS_SPEC {
 private:
  HandlerTableEntry* _table;    // the table
  int                _length;   // the current length of the table
  int                _size;     // the number of allocated entries
  ReallocMark        _nesting;  // assertion check for reallocations

  // add the entry & grow the table if needed
  void add_entry(HandlerTableEntry entry); 
  HandlerTableEntry* subtable_for(int catch_pco) const;

 public:
  // (compile-time) construction within compiler
  ExceptionHandlerTable(int initial_size = 8);

  // (run-time) construction from nmethod
  ExceptionHandlerTable(const nmethod* nm);

  // (compile-time) add entries
  void add_subtable(
    int                 catch_pco, // the pc offset for the CatchNode
    GrowableArray<intptr_t>* handler_bcis, // the exception handler entry point bcis
    GrowableArray<intptr_t>* handler_pcos // pc offsets for the compiled handlers
  );

  // nmethod support
  int  size_in_bytes() const { return _length * sizeof(HandlerTableEntry); }
  void copy_to(nmethod* nm);

  // lookup
  HandlerTableEntry* entry_for(int catch_pco, int handler_bci) const;

  // debugging
  void print_subtable(HandlerTableEntry* t) const;
  void print() const;
  void print_subtable_for(int catch_pco) const;
};
#endif /* COMPILER2 */


// ----------------------------------------------------------------------------
// Implicit null exception tables.  Maps an exception PC offset to a
// continuation PC offset.  Table has a first length word, then pairs of
// <excp-offset, const-offset>.
// // Use 32-bit representation for offsets
typedef  uint              implicit_null_entry;

class ImplicitExceptionTable VALUE_OBJ_CLASS_SPEC {
  uint _size;
  implicit_null_entry *_data;
  implicit_null_entry *adr( uint idx ) const { return &_data[2*idx+1]; }
  ReallocMark          _nesting;  // assertion check for reallocations
public:
  ImplicitExceptionTable( ) :  _data(0) { }
  // (run-time) construction from nmethod
  ImplicitExceptionTable( const nmethod *nm );

  void set_size( uint size );
  void append( uint exec_off, uint cont_off );
  void replicate_faulting_entry( uint old_exec_off, uint new_exec_off );
  uint at( uint exec_off ) const;

  uint len() const { return _data[0]; }
  int size_in_bytes() const { return (2*len() +1) * sizeof(implicit_null_entry); }
  
  void copy_to(nmethod* nm);
  void print(address base) const;
  void verify(nmethod *nm) const;
};

#ifdef COMPILER1
//----------------------------------------------------------------------
// The ExceptionRangeTable (name chosen to avoid collision with
// ExceptionHandlerTable above) mimics the structure of the per-method
// exception handler table in the class file.

class ExceptionRangeEntry VALUE_OBJ_CLASS_SPEC {
 private:
  int _start_pco;      // starting PC offset of range (inclusive)
  int _end_pco;        // ending PC offset of range (exclusive)
  int _scope_count;    // scope number for identifying the enclosing method
                       // and thereby a constant pool. 0 == innermost. (Note
                       // discrepancy with how debug info is recorded, but
                       // this is done to simplify lookup.)

  int _exception_type; // constant pool index of exception type which this
                       // handler catches.

  int _handler_pco;    // PC offset of start of compiled exception handler
                       // in this method for this exception
  int _handler_bci;    // JVMDI support

 public:

  ExceptionRangeEntry(int start_pco, int end_pco, int scope_count,
                      int exception_tpe, int handler_pco, int handler_bci);

  int start_pco()      { return _start_pco;      }
  int end_pco()        { return _end_pco;        }
  int scope_count()    { return _scope_count;    }
  int exception_type() { return _exception_type; }
  int handler_pco()    { return _handler_pco;    }
  int handler_bci()    { return _handler_bci;    }

  bool covers(int pco) { return ((_start_pco <= pco) && (pco < _end_pco)); }

  // Construction time only. Allows construction of PC ranges leading to exception handlers.
  // Be very careful using this. It is very easy to create PC ranges that are not what are
  // specified by the bytecodes. It is essential that the caller keep track of which bytecodes
  // are covered (and which are _not_ covered) by any given exception handler.
  void set_end_pco(int new_end_pco) { _end_pco = new_end_pco; }
};


class ExceptionRangeTable VALUE_OBJ_CLASS_SPEC {
 private:
  ExceptionRangeEntry* _table;    // the table
  int                  _length;   // the current length of the table
  int                  _size;     // the number of allocated entries

  void add_entry(ExceptionRangeEntry entry);

 public:
  // construction (within compiler)
  ExceptionRangeTable(int initial_size = 8);
  ~ExceptionRangeTable();

  // We can not handle two separate sets of exception ranges at the
  // same PC, which can happen when we have a potentially
  // exception-throwing instruction following a call. This routine
  // should be called for all PC offsets before adding entries or
  // searching the table.
  static int compute_modified_at_call_pco(int pco, bool at_call);

  // ExceptionRangeEntries must be added in ascending order of start
  // PC offset. This is the only invariant that is (or can be)
  // verified by this data structure.

  // (compile-time) add entries
  void add_entry(int start_pco, int end_pco,
                 int scope_count, int exception_type,
                 int handler_pco, int handler_bci);

  // Number of entries in the table
  int length() const { return _length; }

  // Fetch entry at index i (0..length() - 1)
  ExceptionRangeEntry* entry_at(int i) const;

  // Look up index of "closest" entry covering passed PC offset.
  // The definition is tricky: this is the entry whose PC offset is
  // less than or equal but closest to the passed one, but if there
  // are several such entries, it is the first of those (i.e., the one
  // with the smallest index).
  // Returns -1 if not found.
  int entry_index_for_pco(int pco) const;

  // nmethod support
  int  size_in_bytes() const { return sizeof(ExceptionRangeTable) +
                                 _length * sizeof(ExceptionRangeEntry); }
  void copy_to(nmethod* nm);

  void print(address base) const PRODUCT_RETURN;
};

#endif /* COMPILER1 */
