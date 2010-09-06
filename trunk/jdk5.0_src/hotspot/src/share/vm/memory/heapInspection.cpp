#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)heapInspection.cpp	1.5 03/12/23 16:41:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_heapInspection.cpp.incl"

// HeapInspection

int KlassInfoEntry::compare(KlassInfoEntry* e1, KlassInfoEntry* e2) {
  if(e1->_instance_words > e2->_instance_words) {
    return -1;
  } else if(e1->_instance_words < e2->_instance_words) {
    return 1;
  }
  return 0;
}

void KlassInfoEntry::print_on(outputStream* st) const {
  ResourceMark rm;
  const char* name;;
  if (_klass->klass_part()->name() != NULL) {
    name = _klass->klass_part()->external_name();
  } else {
    if (_klass == Universe::klassKlassObj())             name = "<klassKlass>";             else
    if (_klass == Universe::arrayKlassKlassObj())        name = "<arrayKlassKlass>";        else
    if (_klass == Universe::objArrayKlassKlassObj())     name = "<objArrayKlassKlass>";     else
    if (_klass == Universe::instanceKlassKlassObj())     name = "<instanceKlassKlass>";     else
    if (_klass == Universe::typeArrayKlassKlassObj())    name = "<typeArrayKlassKlass>";    else
    if (_klass == Universe::symbolKlassObj())            name = "<symbolKlass>";            else
    if (_klass == Universe::boolArrayKlassObj())         name = "<boolArrayKlass>";         else
    if (_klass == Universe::charArrayKlassObj())         name = "<charArrayKlass>";         else
    if (_klass == Universe::singleArrayKlassObj())       name = "<singleArrayKlass>";       else
    if (_klass == Universe::doubleArrayKlassObj())       name = "<doubleArrayKlass>";       else
    if (_klass == Universe::byteArrayKlassObj())         name = "<byteArrayKlass>";         else
    if (_klass == Universe::shortArrayKlassObj())        name = "<shortArrayKlass>";        else
    if (_klass == Universe::intArrayKlassObj())          name = "<intArrayKlass>";          else
    if (_klass == Universe::longArrayKlassObj())         name = "<longArrayKlass>";         else
    if (_klass == Universe::methodKlassObj())            name = "<methodKlass>";            else
#ifndef CORE
    if (_klass == Universe::methodDataKlassObj())        name = "<methodDataKlass>";        else
#endif // !CORE
    if (_klass == Universe::constantPoolKlassObj())      name = "<constantPoolKlass>";      else
    if (_klass == Universe::constantPoolCacheKlassObj()) name = "<constantPoolCacheKlass>"; else
#ifndef CORE
    if (_klass == Universe::compiledICHolderKlassObj())  name = "<compiledICHolderKlass>";  else
#endif // !CORE
      name = "<no name>";
  }
  st->print_cr("%9d   %9d  %s",
               _instance_count, 
               _instance_words * HeapWordSize,
               name);
}

KlassInfoEntry* KlassInfoBucket::lookup(const klassOop k) {
  KlassInfoEntry* elt = _list;
  while (elt != NULL) {
    if (elt->is_equal(k)) {
      return elt;
    }
    elt = elt->next();
  }
  elt = new KlassInfoEntry(k, list());
  set_list(elt);
  return elt;
}

void KlassInfoBucket::iterate(KlassInfoClosure* cic) {
  KlassInfoEntry* elt = _list;
  while (elt != NULL) {
    cic->do_cinfo(elt);
    elt = elt->next();
  }
}

void KlassInfoBucket::empty() {
  KlassInfoEntry* elt = _list;
  _list = NULL;
  while (elt != NULL) {
    KlassInfoEntry* next = elt->next();
    delete elt;
    elt = next;
  }
}

KlassInfoTable::KlassInfoTable(int size, HeapWord* permgen_bottom) {
  _size = size;
  _permgen_bottom = permgen_bottom;
  _buckets = NEW_C_HEAP_ARRAY(KlassInfoBucket, _size);

  for (int index = 0; index < _size; index++) {
    _buckets[index].initialize();
  }
}

KlassInfoTable::~KlassInfoTable() {
  for (int index = 0; index < _size; index++) {
    _buckets[index].empty();
  }
  FREE_C_HEAP_ARRAY(KlassInfoBucket, _buckets);
  _size = 0;
}

uint KlassInfoTable::hash(klassOop p) {
  assert(_permgen_bottom <= (HeapWord*)p, "all klasses in permgen");
  return (uint)(((uintptr_t)p - (uintptr_t)_permgen_bottom) >> 2);
}

KlassInfoEntry* KlassInfoTable::lookup(const klassOop k) {
  uint         idx = hash(k) % _size;
  KlassInfoEntry*  e   = _buckets[idx].lookup(k);
  assert(k == e->klass(), "must be equal");
  return e;
}

void KlassInfoTable::record_instance(const oop obj) {
  klassOop      k = obj->klass();
  KlassInfoEntry* elt = lookup(k);
  elt->set_count(elt->count() + 1);
  elt->set_words(elt->words() + obj->size());
}

void KlassInfoTable::iterate(KlassInfoClosure* cic) {
  for (int index = 0; index < _size; index++) {
    _buckets[index].iterate(cic);
  }
}

int KlassInfoHisto::sort_helper(KlassInfoEntry** e1, KlassInfoEntry** e2) {
  return (*e1)->compare(*e1,*e2);
}
  
KlassInfoHisto::KlassInfoHisto(const char* title, int estimatedCount) :
  _title(title) {
  _elements = new GrowableArray<KlassInfoEntry*>(estimatedCount,true);
}

KlassInfoHisto::~KlassInfoHisto() {
  elements()->clear_and_deallocate();
}

void KlassInfoHisto::add(KlassInfoEntry* cie) {
  elements()->append(cie);
}

void KlassInfoHisto::sort() {
  elements()->sort(KlassInfoHisto::sort_helper);
}

void KlassInfoHisto::print_elements(outputStream* st) const {
  jint total = 0;
  size_t totalw = 0;
  for(int i=0; i < elements()->length(); i++) {
    st->print("%3d: ", i+1);
    elements()->at(i)->print_on(st);
    total += elements()->at(i)->count();
    totalw += elements()->at(i)->words();
  }
  st->print_cr("Total %8d   %9d", total, totalw * HeapWordSize);
}

void KlassInfoHisto::print_on(outputStream* st) const {
  st->print_cr("%s",title());
  print_elements(st);
}

class HistoClosure : public KlassInfoClosure {
 private:
  KlassInfoHisto* _cih;
 public:
  HistoClosure(KlassInfoHisto* cih) : _cih(cih) {}

  void do_cinfo(KlassInfoEntry* cie) {
    _cih->add(cie);
  }
};

class RecordInstanceClosure : public ObjectClosure {
 private:
  KlassInfoTable* _cit;
 public:
  RecordInstanceClosure(KlassInfoTable* cit) : _cit(cit) {}

  void do_object(oop obj) {
    _cit->record_instance(obj);
  }
};

void HeapInspection::heap_inspection() {
  ResourceMark rm;
  HeapWord* permgen_bottom = NULL;

  if (Universe::heap()->kind() == CollectedHeap::GenCollectedHeap) {
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    gch->gc_prologue(false /* !full */); // get any necessary locks
    permgen_bottom = gch->perm_gen()->used_region().start();
  } else {
    return;
  }

  // Collect klass instance info

  // Iterate over objects in the heap
  KlassInfoTable cit(KlassInfoTable::cit_size, permgen_bottom);
  RecordInstanceClosure ric(&cit);
  Universe::heap()->object_iterate(&ric);

  // Sort and print klass instance info
  KlassInfoHisto histo("\n"
                   "num   #instances    #bytes  class name\n"
                   "--------------------------------------",
                   KlassInfoHisto::histo_initial_size);
  HistoClosure hc(&histo);
  cit.iterate(&hc);
  histo.sort();
  histo.print_on(gclog_or_tty);
  gclog_or_tty->flush();

  if (Universe::heap()->kind() == CollectedHeap::GenCollectedHeap) {
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    gch->gc_epilogue(false /* !full */); // release all acquired locks
  }
}
