#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)heapInspection.hpp	1.4 03/12/23 16:41:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// HeapInspection

// KlassInfoTable is a bucket hash table that
// maps klassOops to extra information:
//    instance count and instance word size.
// 
// A KlassInfoBucket is the head of a link list
// of KlassInfoEntry's
//
// KlassInfoHisto is a growable array of pointers
// to KlassInfoEntry's and is used to sort
// the entries.

class KlassInfoEntry: public CHeapObj {
 private:
  KlassInfoEntry* _next;
  klassOop        _klass;
  long            _instance_count;
  size_t          _instance_words;

 public:
  KlassInfoEntry(klassOop k, KlassInfoEntry* next) :
    _klass(k), _instance_count(0), _instance_words(0), _next(next)
  {}
  KlassInfoEntry* next()     { return _next; }
  bool is_equal(klassOop k)  { return k == _klass; }
  klassOop klass()           { return _klass; }
  long count()               { return _instance_count; }
  void set_count(long ct)    { _instance_count = ct; }
  size_t words()             { return _instance_words; }
  void set_words(size_t wds) { _instance_words = wds; }
  int compare(KlassInfoEntry* e1, KlassInfoEntry* e2);
  void print_on(outputStream* st) const;
};

class KlassInfoClosure: public StackObj {
 public:
  // Called for each KlassInfoEntry.
  virtual void do_cinfo(KlassInfoEntry* cie) = 0;
};

class KlassInfoBucket: public CHeapObj {
 private:
  KlassInfoEntry* _list;
  KlassInfoEntry* list()           { return _list; }
  void set_list(KlassInfoEntry* l) { _list = l; }
 public:
  KlassInfoEntry* lookup(const klassOop k);
  void initialize() { _list = NULL; }
  void empty();
  void iterate(KlassInfoClosure* cic);
};

class KlassInfoTable: public StackObj {
 private:
  int _size;
  HeapWord* _permgen_bottom;
  KlassInfoBucket* _buckets;
  uint hash(klassOop p);
  KlassInfoEntry* lookup(const klassOop k);

 public:
  // Table size
  enum {
    cit_size = 20011
  };
  KlassInfoTable(int size, HeapWord* permgen_bottom);
  ~KlassInfoTable();
  void record_instance(const oop obj);
  void iterate(KlassInfoClosure* cic);
};

class KlassInfoHisto : public StackObj {
 private:
  GrowableArray<KlassInfoEntry*>* _elements;
  GrowableArray<KlassInfoEntry*>* elements() const { return _elements; }
  const char* _title;
  const char* title() const { return _title; }
  static int sort_helper(KlassInfoEntry** e1, KlassInfoEntry** e2);
  void print_elements(outputStream* st) const;
 public:
  enum {
    histo_initial_size = 1000
  };
  KlassInfoHisto(const char* title,
             int estimatedCount);
  ~KlassInfoHisto();
  void add(KlassInfoEntry* cie);
  void print_on(outputStream* st) const;
  void sort();
};

class HeapInspection : public AllStatic {
 public:
  static void heap_inspection();
};
