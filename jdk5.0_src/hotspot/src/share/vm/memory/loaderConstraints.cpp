#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)loaderConstraints.cpp	1.5 03/12/23 16:41:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */

# include "incls/_precompiled.incl"
# include "incls/_loaderConstraints.cpp.incl"

LoaderConstraintTable::LoaderConstraintTable(int nof_buckets)
  : Hashtable(nof_buckets, sizeof(LoaderConstraintEntry)) {};


LoaderConstraintEntry* LoaderConstraintTable::new_entry(
                                 unsigned int hash, symbolOop name,
                                 klassOop klass, int num_loaders,
                                 int max_loaders) {
  LoaderConstraintEntry* entry;
  entry = (LoaderConstraintEntry*)Hashtable::new_entry(hash, klass);
  entry->set_name(name);
  entry->set_num_loaders(num_loaders);
  entry->set_max_loaders(max_loaders);
  return entry;
}


void LoaderConstraintTable::oops_do(OopClosure* f) {
  for (int index = 0; index < table_size(); index++) {
    for (LoaderConstraintEntry* probe = bucket(index);
                                probe != NULL;
                                probe = probe->next()) {
      f->do_oop((oop*)(probe->name_addr()));
      if (probe->klass() != NULL) {
        f->do_oop((oop*)probe->klass_addr());
      }
      for (int n = 0; n < probe->num_loaders(); n++) {
        if (probe->loader(n) != NULL) {
          f->do_oop(probe->loader_addr(n));
        }
      }
    }
  }
}

// We must keep the symbolOop used in the name alive.  We'll use the
// loaders to decide if a particular entry can be purged. 
void LoaderConstraintTable::always_strong_classes_do(OopClosure* blk) {
  // We must keep the symbolOop used in the name alive.
  for (int cindex = 0; cindex < table_size(); cindex++) {
    for (LoaderConstraintEntry* lc_probe = bucket(cindex);
                                lc_probe != NULL;
                                lc_probe = lc_probe->next()) {
      assert (lc_probe->name() != NULL,  "corrupted loader constraint table");
      blk->do_oop((oop*)lc_probe->name_addr());
    }
  }
}


// The only unlocked/non-safepoint reader of the loader constraints
// appears to be find_defining_loader, below, which guards against
// unordered writes. Therefore, no membar/volatile is required when
// we're updating the loader constraints.
// 
// This is pretty tricky. It'd feel a little more comfortable if we
// took the system dictionary lock in find_defining_loader. Should we
// create a separate loader constraint lock?

LoaderConstraintEntry** LoaderConstraintTable::find_loader_constraint(
                                    symbolHandle name, Handle loader) {

  unsigned int hash = compute_hash(name);
  int index = hash_to_index(hash);
  LoaderConstraintEntry** pp = bucket_addr(index);
  while (*pp) {
    LoaderConstraintEntry* p = *pp;
    if (p->hash() == hash) {
      if (p->name() == name()) {
        for (int i = p->num_loaders() - 1; i >= 0; i--) {
          if (p->loader(i) == loader()) {
            return pp;
          }
        }
      }
    }
    pp = p->next_addr();
  }
  return pp;
}


void LoaderConstraintTable::purge_loader_constraints(BoolObjectClosure* is_alive) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint")
  // Remove unloaded entries from constraint table
  for (int index = 0; index < table_size(); index++) {
    LoaderConstraintEntry** p = bucket_addr(index);
    while(*p) {
      LoaderConstraintEntry* probe = *p;
      klassOop klass = probe->klass();
      // Remove klass that is no longer alive
      if (klass != NULL && !is_alive->do_object_b(klass)) {
        probe->set_klass(NULL);
      }
      // Remove entries no longer alive from loader array
      int n = 0; 
      while (n < probe->num_loaders()) {
        if (probe->loader(n) != NULL) {
          if (!is_alive->do_object_b(probe->loader(n))) {
            // Compact array
            int num = probe->num_loaders() - 1;
            probe->set_num_loaders(num);
            probe->set_loader(n, probe->loader(num));
            probe->set_loader(num, NULL);
            continue;  // current element replaced, so restart without
                       // incrementing n
          }
        }
        n++;
      }
      // Check whether entry should be purged
      if (probe->num_loaders() < 2) {
        // Purge entry
        *p = probe->next();
        FREE_C_HEAP_ARRAY(oop, probe->loaders());
        free_entry(probe);
      } else {
#ifdef ASSERT
        assert(is_alive->do_object_b(probe->name()), "name should be live");
        if (probe->klass() != NULL) {
          assert(is_alive->do_object_b(probe->klass()), "klass should be live");
        }
        for (n = 0; n < probe->num_loaders(); n++) {
          if (probe->loader(n) != NULL) {
            assert(is_alive->do_object_b(probe->loader(n)), "loader should be live");
          }
        }
#endif
        // Go to next entry
        p = probe->next_addr();
      }
    }
  }
}


bool LoaderConstraintTable::add_entry(symbolHandle class_name,
                                      klassOop klass1, Handle class_loader1,
                                      klassOop klass2, Handle class_loader2,
                                      TRAPS) {
  bool linkage_error = false;
  {
    if (klass1 != NULL && klass2 != NULL && klass1 != klass2) {
      linkage_error = true;
    } else {
      klassOop klass = klass1 != NULL ? klass1 : klass2;
      
      LoaderConstraintEntry** pp1 = find_loader_constraint(class_name,
                                                           class_loader1);
      if (*pp1 != NULL && (*pp1)->klass() != NULL) {
        if (klass != NULL) {
          if (klass != (*pp1)->klass()) {
            linkage_error = true;
          }
        } else {
          klass = (*pp1)->klass();
        }
      }

      LoaderConstraintEntry** pp2 = find_loader_constraint(class_name,
                                                           class_loader2);
      if (*pp2 != NULL && (*pp2)->klass() != NULL) {
        if (klass != NULL) {
          if (klass != (*pp2)->klass()) {
            linkage_error = true;
          }
        } else {
          klass = (*pp2)->klass();
        }
      }

      if (!linkage_error) {
        if (*pp1 == NULL && *pp2 == NULL) {
          unsigned int hash = compute_hash(class_name());
          int index = hash_to_index(hash);
          LoaderConstraintEntry* p;
          p = new_entry(hash, class_name(), klass, 2, 2);
          p->set_loaders(NEW_C_HEAP_ARRAY(oop, 2));
          p->set_loader(0, class_loader1());
          p->set_loader(1, class_loader2());
          p->set_num_loaders(2);
          p->set_max_loaders(2);
          p->set_klass(klass);
          p->set_next(bucket(index));
          set_entry(index, p);
        } else if (*pp1 == *pp2) {
          /* constraint already imposed */
          if ((*pp1)->klass() == NULL) {
            (*pp1)->set_klass(klass);
          } else {
            assert((*pp1)->klass() == klass, "loader constraints corrupted");
          }
        } else if (*pp1 == NULL) {
          extend_loader_constraint(*pp2, class_loader1, klass);
        } else if (*pp2 == NULL) {
          extend_loader_constraint(*pp1, class_loader2, klass);
        } else {
	  merge_loader_constraints(pp1, pp2, klass);
        }
      }
    }
  }
  return !linkage_error;
}


const char* LoaderConstraintTable::check_or_update(instanceKlassHandle k,
                                                   Handle loader,
                                                   symbolHandle name) {
  LoaderConstraintEntry* p = *(find_loader_constraint(name, loader));
      
  if (p && p->klass() != NULL && p->klass() != k()) {
    return  "Class %s violates loader constraints";
  } else {
    if (p && p->klass() == NULL) {
      p->set_klass(k());
    }
    return NULL;
  }
}


klassOop LoaderConstraintTable::find_constrained_klass(symbolHandle name,
                                                       Handle loader) {
  LoaderConstraintEntry *p = *(find_loader_constraint(name, loader));
  if (p != NULL && p->klass() != NULL)
    return p->klass();

  // No constraints, or else no klass loaded yet.
  return NULL;
}


void LoaderConstraintTable::ensure_loader_constraint_capacity(
                                                     LoaderConstraintEntry *p,
                                                    int nfree) {
    if (p->max_loaders() - p->num_loaders() < nfree) {
        int n = nfree + p->num_loaders();
        oop* new_loaders = NEW_C_HEAP_ARRAY(oop, n);
        memcpy(new_loaders, p->loaders(), sizeof(oop) * p->num_loaders());
        p->set_max_loaders(n);
        FREE_C_HEAP_ARRAY(oop, p->loaders());
        p->set_loaders(new_loaders);
    }
}
 

void LoaderConstraintTable::extend_loader_constraint(LoaderConstraintEntry* p,
                                                     Handle loader,
                                                     klassOop klass) {
  ensure_loader_constraint_capacity(p, 1);
  int num = p->num_loaders();
  p->set_loader(num, loader());
  p->set_num_loaders(num + 1);
  if (p->klass() == NULL) {
    p->set_klass(klass);
  } else {
    assert(klass == NULL || p->klass() == klass, "constraints corrupted");
  }
}


void LoaderConstraintTable::merge_loader_constraints(
                                                   LoaderConstraintEntry** pp1,
                                                   LoaderConstraintEntry** pp2,
                                                   klassOop klass) {
  // make sure *pp1 has higher capacity 
  if ((*pp1)->max_loaders() < (*pp2)->max_loaders()) {
    LoaderConstraintEntry** tmp = pp2;
    pp2 = pp1;
    pp1 = tmp;
  }
  
  LoaderConstraintEntry* p1 = *pp1;
  LoaderConstraintEntry* p2 = *pp2;
  
  ensure_loader_constraint_capacity(p1, p2->num_loaders());
  
  for (int i = 0; i < p2->num_loaders(); i++) {
    int num = p1->num_loaders();
    p1->set_loader(num, p2->loader(i));
    p1->set_num_loaders(num + 1);
  }
  
  // p1->klass() will hold NULL if klass, p2->klass(), and old
  // p1->klass() are all NULL.  In addition, all three must have
  // matching non-NULL values, otherwise either the constraints would
  // have been violated, or the constraints had been corrupted (and an
  // assertion would fail).
  if (p2->klass()) {
    assert(p2->klass() == klass, "constraints corrupted");
  }
  if (p1->klass() == NULL) {
    p1->set_klass(klass);
  } else {
    assert(p1->klass() == klass, "constraints corrupted");
  }

  *pp2 = p2->next();
  FREE_C_HEAP_ARRAY(oop, p2->loaders());
  free_entry(p2);
  return;
}


#ifndef PRODUCT

void LoaderConstraintTable::verify(Dictionary* dictionary) {
  Thread *thread = Thread::current();
  for (int cindex = 0; cindex < _loader_constraint_size; cindex++) {
    for (LoaderConstraintEntry* probe = bucket(cindex);
                                probe != NULL;
                                probe = probe->next()) {
      guarantee(probe->name()->is_symbol(), "should be symbol");
      if (probe->klass() != NULL) {
        instanceKlass* ik = instanceKlass::cast(probe->klass()); 
        guarantee(ik->name() == probe->name(), "name should match");
        symbolHandle name (thread, ik->name());
        Handle loader(thread, ik->class_loader());
        unsigned int d_hash = dictionary->compute_hash(name, loader);
        int d_index = dictionary->hash_to_index(d_hash);
        klassOop k = dictionary->find_class(d_index, d_hash, name, loader);
        guarantee(k == probe->klass(), "klass should be in dictionary");
      }
      for (int n = 0; n< probe->num_loaders(); n++) {
        guarantee(probe->loader(n)->is_oop_or_null(), "should be oop");
      }
    }
  }
}
#endif
