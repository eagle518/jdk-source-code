#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interfaceSupport.cpp	1.77 03/12/23 16:43:49 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interfaceSupport.cpp.incl" 


// Implementation of InterfaceSupport

#ifdef ASSERT

long InterfaceSupport::_number_of_calls       = 0;
long InterfaceSupport::_scavenge_alot_counter = 1;
long InterfaceSupport::_fullgc_alot_counter   = 1;
long InterfaceSupport::_fullgc_alot_invocation = 0;

void InterfaceSupport::trace(const char* result_type, const char* header) {
  tty->print_cr("%6d  %s", _number_of_calls, header);
}

void InterfaceSupport::gc_alot() {
  Thread *thread = Thread::current();
  if (thread->is_VM_thread()) return; // Avoid concurrent calls
  // Check for new, not quite initialized thread. A thread in new mode cannot initiate a GC.
  JavaThread *current_thread = (JavaThread *)thread;
  if (current_thread->active_handles() == NULL) return; 

  if (is_init_completed()) {

    if (++_fullgc_alot_invocation < FullGCALotStart) {
      return;
    }

    // Use this line if you want to block at a specific point,
    // e.g. one number_of_calls/scavenge/gc before you got into problems
    if (FullGCALot) _fullgc_alot_counter--;

    // Check if we should force a full gc
    if (_fullgc_alot_counter == 0) {
      // Release dummy so objects are forced to move
      if (!Universe::release_fullgc_alot_dummy()) {
        warning("FullGCALot: Unable to release more dummies at bottom of heap");
      }
      HandleMark hm(thread);
      GenCollectedHeap::heap()->collect(GCCause::_full_gc_alot);
      int invocations = GenCollectedHeap::heap()->perm_gen()->stat_record()->invocations;
      // Compute new interval
      if (FullGCALotInterval > 1) {
        _fullgc_alot_counter = 1+(long)((double)FullGCALotInterval*os::random()/(max_jint+1.0));
        tty->print_cr("Full gc no: %d\tInterval: %d", invocations, _fullgc_alot_counter);
      } else {
        _fullgc_alot_counter = 1;
      }
      // Print progress message
      if (invocations % 100 == 0) {
        tty->print_cr("Full gc no: %d", invocations);
      }
    } else {
      if (ScavengeALot) _scavenge_alot_counter--;
      // Check if we should force a scavenge
      if (_scavenge_alot_counter == 0) {
        HandleMark hm(thread);
        GenCollectedHeap::heap()->collect(GCCause::_full_gc_alot, 0);
        int invocations = GenCollectedHeap::heap()->get_gen(0)->stat_record()->invocations;
        // Compute new interval
        if (ScavengeALotInterval > 1) {
          _scavenge_alot_counter = 1+(long)((double)ScavengeALotInterval*os::random()/(max_jint+1.0));
          tty->print_cr("Scavenge no: %d\tInterval: %d", invocations, _scavenge_alot_counter);
        } else {
          _scavenge_alot_counter = 1;
        }
        // Print progress message
        if (invocations % 1000 == 0) {
          tty->print_cr("Scavenge no: %d", invocations);
        }
      }
    }
  }
}


vframe* vframe_array[50];
int walk_stack_counter = 0;

void InterfaceSupport::walk_stack_from(vframe* start_vf) {
  // walk 
  int i = 0;
  for (vframe* f = start_vf; f; f = f->sender() ) {
    if (i < 50) vframe_array[i++] = f;
  }
}


void InterfaceSupport::walk_stack() {
  JavaThread* thread = JavaThread::current();
  walk_stack_counter++;
  if (!thread->has_last_Java_frame()) return;
  ResourceMark rm(thread);
  RegisterMap reg_map(thread);
  walk_stack_from(thread->last_java_vframe(&reg_map));
}


# ifdef ENABLE_ZAP_DEAD_LOCALS

static int zap_traversals = 0;  

void InterfaceSupport::zap_dead_locals_old() {
  JavaThread* thread = JavaThread::current();
  if (zap_traversals == -1) // edit constant for debugging
    warning("I am here");
  int zap_frame_count = 0; // count frames to help debugging
  for (StackFrameStream sfs(thread); !sfs.is_done(); sfs.next()) {
    sfs.current()->zap_dead_locals(thread, sfs.register_map());
    ++zap_frame_count;
  }
  ++zap_traversals;
}

# endif


#ifndef CORE
int deoptimizeAllCounter = 0;
int zombieAllCounter = 0;
#endif


void InterfaceSupport::zombieAll() {
#ifndef CORE
  if (is_init_completed() && zombieAllCounter > ZombieALotInterval) {
    zombieAllCounter = 0;
    VM_ZombieAll op;
    VMThread::execute(&op);
  } else {
    zombieAllCounter++;
  }
#endif
}

void InterfaceSupport::deoptimizeAll() {
#ifndef CORE
  if (is_init_completed() ) {
    if (DeoptimizeALot && deoptimizeAllCounter > DeoptimizeALotInterval) {
      deoptimizeAllCounter = 0;
      VM_DeoptimizeAll op;
      VMThread::execute(&op);
    } else if (DeoptimizeRandom && (deoptimizeAllCounter & 0x1f) == (os::random() & 0x1f)) {
      VM_DeoptimizeAll op;
      VMThread::execute(&op);
    }
  }
  deoptimizeAllCounter++;

#endif
}


void InterfaceSupport::stress_derived_pointers() {
#ifndef CORE
#ifdef COMPILER2  
  JavaThread *thread = JavaThread::current();
  if (!is_init_completed()) return;
  ResourceMark rm(thread);
  bool found = false;  
  for (StackFrameStream sfs(thread); !sfs.is_done() && !found; sfs.next()) {    
    address pc = (address)sfs.current()->pc();
    CodeBlob* cb = CodeCache::find_blob(pc);
    if (cb != NULL && cb->oop_maps() ) {
      // Find oopmap for current method
      OopMap* map = cb->oop_map_for_return_address(pc, sfs.is_pc_at_call());
      assert(map != NULL, "no oopmap found for pc");
      found = map->has_derived_pointer();
    }
  }
  if (found) {
    // $$$ Not sure what to do here.
    /*
    Scavenge::invoke(0);
    */
  }
#endif
#endif
}


void InterfaceSupport::verify_stack() {
#ifndef CORE
  JavaThread* thread = JavaThread::current();
  ResourceMark rm(thread);
  // disabled because it throws warnings that oop maps should only be accessed
  // in VM thread or during debugging
  
  if (!thread->has_pending_exception()) {
    // verification does not work if there are pending exceptions
    StackFrameStream sfs(thread);  
    CodeBlob* cb = CodeCache::find_blob(sfs.current()->pc());
      // In case of exceptions we might not have a runtime_stub on
      // top of stack, hence, all callee-saved registers are not going
      // to be setup correctly, hence, we cannot do stack verify
    if (cb != NULL && !cb->is_runtime_stub()) return;

    for (; !sfs.is_done(); sfs.next()) {
      sfs.current()->verify(sfs.register_map());
    }
  }
#endif
}


void InterfaceSupport::verify_last_frame() {
#ifndef CORE
  JavaThread* thread = JavaThread::current();
  ResourceMark rm(thread);
  RegisterMap reg_map(thread);
  frame fr = thread->last_frame();
  fr.verify(&reg_map);
#endif
}


#endif // ASSERT


void InterfaceSupport_init() {
#ifdef ASSERT
  if (ScavengeALot || FullGCALot) {
    srand(ScavengeALotInterval * FullGCALotInterval);
  }
#endif
}

