#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)synchronizer.cpp	1.59 03/12/23 16:44:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_synchronizer.cpp.incl"

// COMMENT:
//   This file includes implementations of lightweight monitor,
// monitor cache, object identity hash. The entire file should
// NOT contain any safepoint. Any safepoint may cause fatal error
// with monitor deflation. All codes in this file are extremely
// senstive to race condition, and most of the codes has been
// optimized to speed up fast path. Be careful when make any
// changes to the monitor system. Contact the original author
// when possible.
//
// NOTE:
//   The assembly versions of monitor enter and exit are used
// in the interpreter, compiler and runtime. Make sure you 
// review all of them when making changes.
//
// TODO:
//   Advanced debugging support. This monitor system provide 
// good debugging support, such as dead lock detection, 
// profiling, priority inheritence, monitor dump. However
// the current code base does not implement all these features.
// It is desired to add those features in the future release.

// -----------------------------------------------------------------------------

void BasicLock::print_on(outputStream* st) const {
  st->print("monitor");
}

void BasicLock::move_to(oop obj, BasicLock* dest) {
  // Check to see if we need to inflate the lock. This is only needed
  // if an object is locked using "this" lightweight monitor. In that
  // case, the displaced_header() is unlocked, because the 
  // displaced_header() contains the header for the originally unlocked
  // object. However the object could has already been inflated. But it 
  // does not matter, the inflation will just a no-op. For other cases,
  // the displaced header will be either 0x0 or 0x3, which are location
  // independent, therefore the BasicLock is free to move.
  

  if (displaced_header()->is_unlocked()) {
    ObjectSynchronizer::inflate_helper(obj);
    // WARNING: We can not put check here, because the inflation
    // will not update the displaced header. Once BasicLock is inflated, 
    // no one should ever look at its content.
  } else {
    assert(displaced_header() == NULL || displaced_header() == markOop(markOopDesc::marked_value), "must be unlocked or a placeholder");
  }
// [RGV] The next line appears to do nothing!
  intptr_t dh = (intptr_t) displaced_header();
  dest->set_displaced_header(displaced_header());
}

// -----------------------------------------------------------------------------

// standard constructor, allows locking failures
ObjectLocker::ObjectLocker(Handle obj, Thread* thread) {
  _thread = thread;
  debug_only(if (StrictSafepointChecks) _thread->check_for_valid_safepoint_state(false);)

  _obj = obj;
  ObjectSynchronizer::fast_enter(_obj, &_lock, _thread);
}

ObjectLocker::~ObjectLocker() {
  ObjectSynchronizer::fast_exit(_obj(), &_lock, _thread);
}

// -----------------------------------------------------------------------------

// Global monitor list and free list
ObjectMonitor* ObjectSynchronizer::gBlockList = NULL;
ObjectMonitor* ObjectSynchronizer::gFreeList = NULL;

// NOTE: More debugging info will be needed if we want to do full 
// monitor cache dump. Many new global variables will be needed
// to implmement advanced debugging support. But be careful when
// accessing these global variables. Also, you have to turn off
// aggressive monitor deflation during safepoint. Otherwise, the
// safepoint will deflate all idle monitors, and we can not get
// the correct debugging and profile information about monitors.
static int gMonitorCount = 0;
static int gConversionCount = 0;

// Get the next block in the block list.
static inline ObjectMonitor* next(ObjectMonitor* block) {
  assert(block->object() == (oop)-1, "must be a block header");
  block = (ObjectMonitor*) block->queue();
  assert(block == NULL || block->object() == (oop)-1, "must be a block header");
  return block;
}

// Fast path code shared by multiple functions
inline ObjectMonitor* ObjectSynchronizer::inflate_helper(oop obj) {
  markOop mark = obj->mark();
  if (mark->has_monitor()) {
    assert(ObjectSynchronizer::verify_objmon_isinpool(mark->monitor()), "monitor is invalid");
    assert(mark->monitor()->header()->is_unlocked(), "monitor must record a good object header");
    return mark->monitor();
  }
  return ObjectSynchronizer::inflate(obj);
}

// Real monitor inflation code. It uses fancy monitor inflation technique.
// The global lock MonitorCache_lock is used to coordinate the inflations.
// We don't have to use the global lock. However the global lock gives us
// significant advantages in debugging support.
ObjectMonitor* ObjectSynchronizer::inflate(oop object) {
  // NOTE: It is much simpler not to have safepint here.
  MonitorCache_lock->lock_without_safepoint_check();

  // increment inflation count, just for tracing purpose
  gConversionCount++;

  if (gFreeList == 0) {
    // TODO: The monitor initialization is done in constructor.
    ObjectMonitor* temp = new ObjectMonitor[BLOCK_SIZE];
    // NOTE: (almost) no way to recover if synchronization failed
    guarantee(temp, "Out of memory in ObjectSynchronizer::inflate()!");

    // initialize the linked list, each monitor points to its next
    // forming the single linked free list, the very first monitor
    // will points to next block, which forms the block list. All
    // monitors on the block list will not be used for synchronization.
    for (int i = 1; i < BLOCK_SIZE - 1; i++) {
      temp[i].set_queue(&temp[i+1]);
    }
    // terminate the last monitor as the end of list
    temp[BLOCK_SIZE - 1].set_queue(NULL);
    // set up the block list
    temp[0].set_queue(gBlockList);
    temp[0].set_object((oop)-1);
    // the first list must skip the first one
    gFreeList = temp + 1;
    // the very first ones form the block list
    gBlockList = temp;
    // statistics
    gMonitorCount += BLOCK_SIZE;
  }

  // Set the object mark to be null.  According to markOopDesc usage, null
  // is interpreted as a lightweight monitor.  Because no thread can have
  // null as its stack pointer, every thread considers this object as
  // locked by some other thread, and calls inflate() to convert the
  // monitor.  Since the current thread owns the MonitorCache_lock, all other
  // threads will be blocked until the current thread finishes the inflation.
  // This is the ONLY case where an object could have a null mark word.
  //
  // Note that xchg_ptr doesn't necessarily do an acquire, so memory accesses
  // between it and the release_set_mark can float above it.  Not a problem
  // here because we don't mind if stores to the monitor move, but we could
  // have one if the code changes in such a way as to require ordering.
  markOop test = (markOop) Atomic::xchg_ptr(0, object->mark_addr());
  assert(test != NULL, "Unexpected object header as 0!");

  ObjectMonitor* monitor = NULL;
  markOop        mark    = NULL; // Original header

  if (test->has_locker()) {
    // Lightweight monitor case.  We need to create a heavyweight monitor,
    // set its owner to be the same as the lightweight monitor, and store
    // its encoded pointer into the object header.  The current thread
    // may already own this monitor, but it does not matter.
    monitor = gFreeList;
    gFreeList = (ObjectMonitor*) monitor->queue();
    // Save original header in the monitor.
    mark = test->displaced_mark_helper();
    assert(mark->is_unlocked(), "lock must record a good object header");
    // Setup monitor fields to proper values.
    monitor->set_queue(NULL);
    monitor->set_header(mark);
    monitor->set_owner(test->locker());
    monitor->set_object(object);
    monitor->Recycle(); 
  } else if (test->is_unlocked()) {
    // The object is in normal unlocked state because the owner thread just
    // exited.  But we still need to inflate it to avoid further thrashing.
    monitor = gFreeList;
    gFreeList = (ObjectMonitor*) monitor->queue();
    // test contains the original header, which we save in the monitor.
    mark = test;
    assert(mark->is_unlocked(), "lock must record a good object header");
    // Set monitor to initial state
    monitor->set_queue(NULL);
    monitor->set_header(mark);
    monitor->set_owner(NULL);
    monitor->set_object(object);
    monitor->Recycle();
  } else {
    // The object already has a heavyweight monitor, maybe some other thread
    // just inflated it.  OK, we can just use it.
    monitor = test->monitor();
    mark = monitor->header();
  }

  // During the above code, the MonitorCache_lock will prevent any
  // other thread from modifying this object's header.

  assert(mark->is_unlocked(), "monitor must record a good object header");
  assert(monitor->header() == mark, "header in monitor must be stable");
  assert(object->mark() == NULL, "must keep the null that was swapped");

  // WARNING:
  // Must preserve store ordering. The monitor state must
  // be stable at the time of publishing the monitor address.
  object->release_set_mark(markOopDesc::encode(monitor));

  // The unlock does a release, thus publishes the monitor address.
  MonitorCache_lock->unlock();

  assert(monitor->object() == object, "inflated monitor must point to its object");
#ifdef ASSERT
  {
    markOop temp = object->mark();
    assert(temp == NULL || temp->monitor() == monitor, "Unexpected monitor gc!");
  }
#endif

  return monitor;
}

// This the fast monitor enter. The interpreter and compiler use
// some assembly copies of this code. Make sure update those code
// if the following function is changed. The implementation is
// extremely sensitive to race condition. Be careful.
void ObjectSynchronizer::fast_enter(Handle obj, BasicLock* lock, TRAPS) {
  THREAD->update_highest_lock((address)lock);

  // I assume the object is not locked. If that is not true, set_unlocked()
  // will change the test value and cause the following atomic operation
  // to fail. Then the slow case handle will kick in. 
  markOop test = obj->mark();
  test = test->set_unlocked();
  // WARNING:
  // This code must publish the header before the atomic operation.
  // This is an absolute requirement. Make sure compilers generate correct
  // code. Also make sure of the assembly code in the runtime systems.
  lock->set_displaced_header(test);
  // WARNING:
  // We must do a fence here to satisfy the JMM and to ensure that the
  // header is published before we attempt to lock the object.
  // Atomic::cmpxchg_ptr is *defined* to include a fence, so we don't
  // need to do it here.  See atomic.hpp.
  //OrderAccess::fence();
  // Use atomic operation to setup the ownship.  It tests if the object is
  // unlocked.  If so, it sets the header to point to the location of
  // BasicLock, where the original header was saved.  If the object was not
  // unlocked, it will fail.  Use the xxx_ptr version of the atomic operation
  // to make sure the code is 64-bit transparent. 
  markOop mark = (markOop) Atomic::cmpxchg_ptr(lock, obj->mark_addr(), test);
  if (mark == test) {
    return;
  }
  ObjectSynchronizer::slow_enter(obj, lock, THREAD);
}

void ObjectSynchronizer::fast_exit(oop object, BasicLock* lock, TRAPS) {
  ObjectMonitor* monitor;
  // if displaced header is null, the previous enter is recursive enter, no-op
  markOop test = lock->displaced_header();
  if (test == NULL) {
    return;
  }
  // release the monitor by restoring the original header using atomic operation
  // and it will fail if monitor is not locked by using the same BasicLock
  markOop mark = (markOop) Atomic::cmpxchg_ptr(test, object->mark_addr(), lock);
  if (mark == (markOop)lock) {
    return;
  }
  // get heavy weight monitor or inflate it
  if (mark->has_monitor()) {
    monitor = mark->monitor();
  } else {
    monitor = ObjectSynchronizer::inflate(object);
  }
  monitor->exit(THREAD);
}

// This routine is used to handle interpreter/compiler slow case
// We don't need to use fast path here, because it must have been
// failed in the interpreter/compiler code.
void ObjectSynchronizer::slow_enter(Handle obj, BasicLock* lock, TRAPS) {
  ObjectMonitor* monitor = NULL;
  markOop mark = obj->mark();
 
  if (mark->has_locker() && THREAD->is_lock_owned((address)mark->locker())) {
    assert(lock != mark->locker(), "must not re-lock the same lock");
    assert(lock != (BasicLock*)obj->mark(), "don't relock with same BasicLock");
    lock->set_displaced_header(NULL);
    return;
  } else if (mark->has_monitor()) {
    monitor = mark->monitor();
    if (monitor->is_entered(THREAD)) {
      lock->set_displaced_header(NULL);
      return;
    }
  } else {
    // Try getting the lock by spinning before inflating
    if (UseSpinning) {
      for (int i = PreInflateSpin; i > 0; i--) {
        if (PreSpinYield) {
          os::yield(); 
        }
        for (int j = ReadSpinIterations; j > 0; j--) {
          markOop test = obj()->mark();
          if (test->is_unlocked()) {
            // WARNING:
            // This code must publish the header before the atomic operation.
            // This is an absolute requirement.
            lock->set_displaced_header(test);
            // WARNING:
            // We must do a fence here to satisfy the JMM and to ensure that the
            // header is published before we attempt to lock the object.
            // Atomic::cmpxchg_ptr is *defined* to include a fence, so we don't
            // need to do it here.  See atomic.hpp.
            //OrderAccess::fence();
            mark = (markOop) Atomic::cmpxchg_ptr(lock, obj()->mark_addr(), test);
            if (mark == test) {
              return;
            }
          }
        }
        if (PostSpinYield) {
          os::yield();
        }
      }
    }
    monitor = ObjectSynchronizer::inflate(obj());
  }
  // The object header will never be displaced to this lock,
  // so it does not matter what the value is, except that it
  // must be non-zero to avoid looking like a re-entrant lock,
  // and must not look locked either.
  lock->set_displaced_header(markOopDesc::unused_mark());
  monitor->enter(THREAD);
}

// This routine is used to handle interpreter/compiler slow case
// We don't need to use fast path here, because it must have 
// failed in the interpreter/compiler code. Simply use the heavy
// weight monitor should be ok, unless someone find otherwise.
void ObjectSynchronizer::slow_exit(oop object, BasicLock* lock, TRAPS) {
  if (lock->displaced_header() == NULL) {
    // the enter was recursive via UseHeavyMonitors so we are done already
    if (UseHeavyMonitors) return;
    assert(false, "fast_exit() should have handled this already");
  }
  ObjectMonitor* monitor = ObjectSynchronizer::inflate_helper(object);
  monitor->exit(THREAD);
}

// NOTE: must use heavy weight monitor to handle jni monitor enter
void ObjectSynchronizer::jni_enter(Handle obj, TRAPS) { // possible entry from jni enter
  ObjectMonitor* monitor = ObjectSynchronizer::inflate_helper(obj());
  // the current locking is from JNI instead of Java code
  THREAD->set_current_pending_monitor_is_from_java(false);
  monitor->enter(THREAD);
  THREAD->set_current_pending_monitor_is_from_java(true);
}

// NOTE: must use heavy weight monitor to handle jni monitor exit
void ObjectSynchronizer::jni_exit(oop obj, Thread* THREAD) {
  ObjectMonitor* monitor = ObjectSynchronizer::inflate_helper(obj);
  // If this thread has locked the object, exit the monitor.  Note:  can't use
  // monitor->check(CHECK); must exit even if an exception is pending.
  if (monitor->check(THREAD)) {
    monitor->exit(THREAD);
  }
}

// NOTE: must use heavy weight monitor to handle wait()
void ObjectSynchronizer::wait(Handle obj, jlong millis, TRAPS) {  
  if (millis < 0) {
    THROW_MSG(vmSymbols::java_lang_IllegalArgumentException(), "timeout value is negative");
  }
  ObjectMonitor* monitor = ObjectSynchronizer::inflate_helper(obj());
  monitor->wait(millis, true, THREAD);
}

void ObjectSynchronizer::notify(Handle obj, TRAPS) {
  ObjectMonitor* monitor;
  markOop mark = obj->mark();
  if (mark->has_monitor()) {
    // object already has a monitor, we just use it
    monitor = mark->monitor();
  } else if (mark->has_locker() && THREAD->is_lock_owned((address)mark->locker())) {
    // object has a lightweight monitor, and current thread owns it,
    // therefore no other thread could be waiting, no-op
    return;
  } else {
    // WARNING: must inflate the monitor, and let heavy 
    // weight monitor to check the illegal monitor state.
    // It does not worth to use extra code for error check.
    monitor = ObjectSynchronizer::inflate(obj());
  }
  monitor->notify(THREAD);
}

// NOTE: see comment of notify()
void ObjectSynchronizer::notifyall(Handle obj, TRAPS) {  
  markOop mark = obj->mark();
  ObjectMonitor* monitor = NULL;
  
  if (mark->has_monitor()) {
    monitor = mark->monitor();
  } else if (mark->has_locker() && THREAD->is_lock_owned((address)mark->locker())) {
    return;
  } else {
    monitor = ObjectSynchronizer::inflate(obj());
  }
  monitor->notifyAll(THREAD);
}

static inline intptr_t get_next_hash() {
  intptr_t value;
  do {
    // use a pseudo-random 32-bit number as the hash value
    value = os::random();
    value &= markOopDesc::hash_mask;
  } while (value == markOopDesc::no_hash);
  return value;
}

// NOTE:
//   Retrieve/Allocate object identity hash. Since the operation
// will run concurrently with all other monitor operations, the
// code is extremely sensitive to race condition. The code has
// been optimized for hash code retrieval instead of allocation.
// The aggressive monitor inflation is compensated by aggressive
// monitor deflation   
intptr_t ObjectSynchronizer::identity_hash_value_for(Handle obj) {
  ObjectMonitor* monitor = NULL;
  markOop temp, test;
  intptr_t hash;
  markOop mark = obj->mark();
 
  if (mark->is_unlocked()) {
    hash = mark->hash();              // this is a normal header
    if (hash) {                       // if it has hash, just return it
      return hash;
    }
    hash = get_next_hash();           // allocate a new hash code
    temp = mark->copy_set_hash(hash); // merge the hash code into header
    // use (machine word version) atomic operation to install the hash
    test = (markOop) Atomic::cmpxchg_ptr(temp, obj->mark_addr(), mark);
    if (test == mark) {
      return hash;
    }
    // If atomic operation failed, we must inflate the header
    // into heavy weight monitor. We could add more code here
    // for fast path, but it does not worth the complexity.
  } else if (mark->has_monitor()) {
    monitor = mark->monitor();
    temp = monitor->header();
    hash = temp->hash();
    if (hash) {
      return hash;
    }
    // Skip to the following code to reduce code size
  } else if (Thread::current()->is_lock_owned((address)mark->locker())) {
    temp = mark->displaced_mark_helper(); // this is a lightweight monitor owned
    hash = temp->hash();              // by current thread, check if the displaced
    if (hash) {                       // header contains hash code
      return hash;
    }
    // WARNING:
    //   The displaced header is strictly immutable.
    // It can NOT be changed in ANY cases. So we have 
    // to inflate the header into heavyweight monitor
    // even the current thread owns the lock. The reason
    // is the BasicLock (stack slot) will be asynchronously 
    // read by other threads during the inflate() function.
    // Any change to stack may not propagate to other threads
    // correctly.
  }
  // Inflate the monitor to set hash code
  monitor = ObjectSynchronizer::inflate_helper(obj());
  // Load displaced header and check it has hash code
  mark = monitor->header();
  hash = mark->hash();
  if (hash == 0) {
    hash = get_next_hash();
    temp = mark->copy_set_hash(hash); // merge hash code into header
    test = (markOop) Atomic::cmpxchg_ptr(temp, monitor, mark);
    if (test != mark) {
      // The only update to the header in the monitor (outside GC)
      // is install the hash code. If someone add new usage of
      // displaced header, please update this code
      hash = test->hash();
      assert(hash != 0, "Trivial unexpected object/monitor header usage.");
    }
  }
  // We finally get the hash
  return hash;
}


bool ObjectSynchronizer::current_thread_holds_lock(JavaThread* thread,
                                                   Handle h_obj) {
  assert(thread == JavaThread::current(), "Can only be called on current thread");
  oop obj = h_obj();
  assert(obj != NULL && obj->is_oop(), "sanity check");
  markOop mark = obj->mark();

  // Get mark under lock if in process of inflation, when we get the lock
  // the value is stable.
  if (mark->is_being_inflated()) {
    MonitorCache_lock->lock_without_safepoint_check();
    mark = h_obj->mark();
    MonitorCache_lock->unlock();
  }

  // Uncontended case, header points to stack
  if (mark->has_locker()) {
    return thread->is_lock_owned((address)mark->locker());
  }
  // Contended case, header points to ObjectMonitor (tagged pointer)
  if (mark->has_monitor()) {
    ObjectMonitor* monitor = mark->monitor();
    return monitor->is_entered(thread);
  }
  // Unlocked case, header in place
  assert(mark->is_unlocked(), "sanity check");
  return false;
}

// FIXME: jvmti should call this
JavaThread* ObjectSynchronizer::get_lock_owner(Handle h_obj, bool doLock) {
  oop obj = h_obj();
  address owner = NULL;

  assert(obj != NULL && obj->is_oop(), "sanity check");
  markOop mark = obj->mark();

  // Get mark under lock if in process of inflation, when we get the lock
  // the value is stable.
  if (mark->is_being_inflated()) {
    MonitorCache_lock->lock_without_safepoint_check();
    mark = h_obj->mark();
    MonitorCache_lock->unlock();
  }

  // Uncontended case, header points to stack
  if (mark->has_locker()) {
    owner = (address) mark->locker();
  }

  // Contended case, header points to ObjectMonitor (tagged pointer)
  if (mark->has_monitor()) {
    ObjectMonitor* monitor = mark->monitor();
    assert(monitor != NULL, "monitor should be non-null");
    owner = (address) monitor->owner();
  }

  if (owner != NULL) {
    return Threads::owning_thread_from_monitor_owner(owner, doLock);
  }

  // Unlocked case, header in place
  // Cannot have assertion since this object may have been
  // locked by another thread when reaching here.
  // assert(mark->is_unlocked(), "sanity check");

  return NULL;
}

void ObjectSynchronizer::monitors_iterate(MonitorClosure* closure) {
  ObjectMonitor* block = gBlockList;
  ObjectMonitor* mid;
  while (block) {
    for (int i = BLOCK_SIZE - 1; i > 0; i--) {
      mid = block + i;
      oop object = (oop) mid->object();
      if (object) {
        closure->do_monitor(mid);
      }
    }
    block = (ObjectMonitor*) block->_queue;
  }
}


// This implements aggressive monitor deflation. All monitors
// that are not being used during safepoint will be deflated.
void ObjectSynchronizer::deflate_idle_monitors() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");
  assert(!MonitorCache_lock->is_locked(), "may not create monitors in GC");
  for (ObjectMonitor* block = gBlockList; block != NULL; block = next(block)) {
    for (int i = BLOCK_SIZE - 1; i > 0; i--) {
      ObjectMonitor* mid = &block[i];
      oop object = (oop) mid->object();
      if (object != NULL && !mid->is_busy()) {
	// Deflate the monitor if it is no longer being used
	assert(mid == object->mark()->monitor(), "It must be unmatched monitor enter/exit!");
	assert(mid->header()->is_unlocked(), "monitor must record a good object header");
        // Restore the header back to object
	object->release_set_mark(mid->header());
	mid->clear();
        // Put the monitor back to free list
	mid->set_queue(gFreeList);
	gFreeList = mid;
      }
    }
  }
}

void ObjectSynchronizer::oops_do(OopClosure* f) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");
  assert(!MonitorCache_lock->is_locked(), "may not create monitors in GC");
  for (ObjectMonitor* block = gBlockList; block != NULL; block = next(block)) {
    for (int i = 1; i < BLOCK_SIZE; i++) {
      ObjectMonitor* mid = &block[i];
      if (mid->object() != NULL) {
	f->do_oop((oop*)mid->object_addr());
      }
    }
  }
}

//------------------------------------------------------------------------------
// Non-product code

#ifndef PRODUCT

void ObjectSynchronizer::trace_locking(Handle locking_obj, bool is_compiled,
                                       bool is_method, bool is_locking) {
  // Don't know what to do here
}

// Verify all monitors in the monitor cache, the verification is weak.
void ObjectSynchronizer::verify() {
  ObjectMonitor* block = gBlockList;
  ObjectMonitor* mid;
  while (block) {
    for (int i = 1; i < BLOCK_SIZE; i++) {
      mid = block + i;
      oop object = (oop) mid->object();
      if (object != NULL) {
        mid->verify();
      }
    }
    block = (ObjectMonitor*) block->queue();
  }
}

// Check if monitor belongs to the monitor cache
int ObjectSynchronizer::verify_objmon_isinpool(ObjectMonitor *monitor) {
  ObjectMonitor* block = gBlockList;
  while (block) {
    if (monitor > &block[0] && monitor < &block[BLOCK_SIZE]) {
      address mon = (address) monitor;
      address blk = (address) block;
      size_t diff = mon - blk;
      assert((diff % sizeof(ObjectMonitor)) == 0, "check");
      return 1;
    }
    block = (ObjectMonitor*) block->queue();
  }
  return 0;
}

#endif
