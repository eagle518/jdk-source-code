#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)sweeper.cpp	1.32 03/12/23 16:44:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_sweeper.cpp.incl"

long      NMethodSweeper::_traversals = 0;   // No. of stack traversals performed
CodeBlob* NMethodSweeper::_current = NULL;   // Current nmethod
int       NMethodSweeper::_seen = 0 ;        // No. of blobs we have currently processed in current pass of CodeCache
int       NMethodSweeper::_invocations = 0;  // No. of invocations left until we are completed with this pass
jint      NMethodSweeper::_nof_zombies = 0;  // No. of zombies (i.e., any work to do?)
jint      NMethodSweeper::_nof_not_entrants = 0; // No. of zombies (i.e., any work to do?)
jint      NMethodSweeper::_nof_unloaded = 0; // No. of zombies (i.e., any work to do?)

void NMethodSweeper::sweep() {   
  assert(SafepointSynchronize::is_at_safepoint(), "must be executed at a safepoint");
  if (!MethodFlushing) return;  

  // No need to synchronize access, since this is always executed at a safepoint
  if (_nof_zombies == 0 && _nof_not_entrants == 0 && _nof_unloaded == 0) return;

  // Make sure CompiledIC_lock in unlocked, since we might update some
  // inline caches. If it is, we just bail-out and try later.
  if (CompiledIC_lock->is_locked() || Patching_lock->is_locked()) return;
  
  // Check for restart
  assert(CodeCache::find_blob_unsafe(_current) == _current, "Sweeper nmethod cached state invalid");
  if (_current == NULL) {
    _seen        = 0;
    _invocations = NmethodSweepFraction;
    _current     = CodeCache::first();
    _traversals  += 1;
    if (PrintMethodFlushing) {
      tty->print_cr("### Sweep: stack traversal %d", _traversals);
    }
    Threads::nmethods_do();
  }

  if (PrintMethodFlushing && Verbose) {
    tty->print_cr("### Sweep at %d out of %d. Invocations left: %d", _seen, CodeCache::nof_blobs(), _invocations);
  }

  // We want to visit all nmethods after NmethodSweepFraction invocations. 
  // If invocation is 1 we do the rest
  int todo = CodeCache::nof_blobs();
  if (_invocations != 1) {
    todo = (CodeCache::nof_blobs() - _seen) / _invocations;    
    _invocations--;
  }
      
  for(int i = 0; i < todo && _current != NULL; i++) {      
    CodeBlob* next = CodeCache::next(_current); // Read next before we potentially delete current
    if (_current->is_nmethod()) {
      process_nmethod((nmethod *)_current);      
    }    
    _seen++;
    _current = next;
  }  
  // Because we could stop on a codeBlob other than an nmethod we skip forward
  // to the next nmethod (if any). codeBlobs other than nmethods can be freed
  // async to us and make _current invalid while we sleep.
  while (_current != NULL && !_current->is_nmethod()) {
    _current = CodeCache::next(_current);
  }
}

   
void NMethodSweeper::process_nmethod(nmethod *nm) {  
  // Skip methods that are currently referenced by the VM
  if (nm->is_locked_by_vm()) {
    // But still remember to clean-up inline caches for alive nmethods
    if (nm->is_alive()) {
      // Clean-up all inline caches that points to zombie/non-reentrant methods
      nm->cleanup_inline_caches();     
    }
    return;
  }

  if (nm->is_zombie()) {
    // If it is first time, we see nmethod then we mark it. Otherwise,
    // we reclame it. When we have seen a zombie method twice, we know that
    // there are no inline caches that referes to it.
    if (nm->is_marked_for_reclamation()) {
      assert(!nm->is_locked_by_vm(), "must not flush locked nmethods");
      nm->flush();      
      _nof_zombies--;
      assert(_nof_zombies >= 0, "cannot go negative");
    } else {
      nm->mark_for_reclamation();
    }  
  } else if (nm->is_not_entrant()) {    
    // If there is no current activations of this method on the
    // stack we can safely convert it to a zombie method
    if (nm->can_not_entrant_be_converted()) {
      nm->make_zombie();
      _nof_not_entrants--;
      assert(_nof_not_entrants >= 0, "cannot go negative");
    } else {
      // Still alive, clean up its inline caches 
      nm->cleanup_inline_caches();    
    }
  } else if (nm->is_unloaded()) {
    // Unloaded code, just make it a zombie 
    if (nm->is_osr_only_method()) {
      // No inline caches will ever point to osr methods, so we can just remove it
      nm->flush();
    } else {
      nm->make_zombie();
    }
    _nof_unloaded--;
  } else {
    assert(nm->is_alive(), "should be alive");
    // Clean-up all inline caches that points to zombie/non-reentrant methods
    nm->cleanup_inline_caches();    
  }
}
