#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)taskqueue.cpp	1.13 03/12/23 16:44:51 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_taskqueue.cpp.incl"

bool TaskQueueSuper::peek() {
  return _bottom != _age.top();
}

int TaskQueueSetSuper::randomParkAndMiller(int *seed0) {
  const int a =      16807;
  const int m = 2147483647;
  const int q =     127773;  /* m div a */
  const int r =       2836;  /* m mod a */
  assert(sizeof(int) == 4, "I think this relies on that");
  int seed = *seed0;
  int hi   = seed / q;
  int lo   = seed % q;
  int test = a * lo - r * hi;
  if (test > 0)
    seed = test;
  else
    seed = test + m;
  *seed0 = seed;
  return seed;
}


ParallelTaskTerminator::
ParallelTaskTerminator(int n_threads, TaskQueueSetSuper* queue_set) :
  _n_threads(n_threads),
  _queue_set(queue_set), 
  _offered_termination(0), _terminated(0),
  _term_monitor(Mutex::leaf+1, "ParTaskTerm", true)
{}

bool ParallelTaskTerminator::peek_in_queue_set() {
  return _queue_set->peek();
}

bool ParallelTaskTerminator::offer_termination() {
  Atomic::inc(&_offered_termination);
  while (true) {
    if (_offered_termination == _n_threads) {
      //inner_termination_loop();
      return true;
    } else {
      os::yield();
      if (peek_in_queue_set()) {
        Atomic::dec(&_offered_termination);
        return false;
      }
    }
  }
}

void ParallelTaskTerminator::reset_for_reuse() {
  if (_offered_termination != 0) {
    assert(_offered_termination == _n_threads,
           "Terminator may still be in use");
    _offered_termination = 0;
  }
}

OopTaskQueue::OopTaskQueue() : TaskQueueSuper() {
  assert(sizeof(Age) == sizeof(jint), "Depends on this.");
}

void OopTaskQueue::initialize() {
  _elems = NEW_C_HEAP_ARRAY(Task, n());
  guarantee(_elems != NULL, "Allocation failed.");
}

bool OopTaskQueue::push_slow(Task t, juint dirty_n_elems) {
  if (dirty_n_elems == n() - 1) {
    // Actually means 0, so do the push.
    juint localBot = _bottom;
    _elems[localBot] = t;
    _bottom = increment_index(localBot);
    return true;
  } else
    return false;
}

bool OopTaskQueue::
pop_local_slow(juint localBot, Age oldAge) {
  // This queue was observed to contain exactly one element; either this
  // thread will claim it, or a competing "pop_global".  In either case,
  // the queue will be logically empty afterwards.  Create a new Age value
  // that represents the empty queue for the given value of "_bottom".  (We 
  // must also increment "tag" because of the case where "bottom == 1",
  // "top == 0".  A pop_global could read the queue element in that case,
  // then have the owner thread do a pop followed by another push.  Without
  // the incrementing of "tag", the pop_global's CAS could succeed,
  // allowing it to believe it has claimed the stale element.)
  Age newAge;
  newAge._top = localBot;
  newAge._tag = oldAge.tag() + 1;
  // Perhaps a competing pop_global has already incremented "top", in which 
  // case it wins the element.
  if (localBot == oldAge.top()) {
    Age tempAge;
    // No competing pop_global has yet incremented "top"; we'll try to
    // install new_age, thus claiming the element.
    assert(sizeof(Age) == sizeof(jint) && sizeof(jint) == sizeof(juint),
	   "Assumption about CAS unit.");
    *(jint*)&tempAge = Atomic::cmpxchg(*(jint*)&newAge, (volatile jint*)&_age, *(jint*)&oldAge);
    if (tempAge == oldAge) {
      // We win.
      assert(dirty_size(localBot, get_top()) != n() - 1,
	     "Shouldn't be possible...");
      return true;
    }
  }
  // We fail; a completing pop_global gets the element.  But the queue is
  // empty (and top is greater than bottom.)  Fix this representation of
  // the empty queue to become the canonical one.
  set_age(newAge);
  assert(dirty_size(localBot, get_top()) != n() - 1,
	 "Shouldn't be possible...");
  return false;
}

bool OopTaskQueue::pop_global(Task& t) {
  Age newAge;
  Age oldAge = get_age();
  juint localBot = _bottom;
  juint n_elems = size(localBot, oldAge.top());
  if (n_elems == 0) {
    return false;
  }
  t = _elems[oldAge.top()];
  newAge = oldAge;
  newAge._top = increment_index(newAge.top());
  if ( newAge._top == 0 ) newAge._tag++;
  Age resAge;
  *(jint*)&resAge = Atomic::cmpxchg(*(jint*)&newAge, (volatile jint*)&_age, *(jint*)&oldAge);
  // Note that using "_bottom" here might fail, since a pop_local might
  // have decremented it.
  assert(dirty_size(localBot, newAge._top) != n() - 1,
	 "Shouldn't be possible...");
  return (resAge == oldAge);
}

OopTaskQueue::~OopTaskQueue() {
  FREE_C_HEAP_ARRAY(Task, _elems);
}

void OopTaskQueueSet::register_queue(int i, OopTaskQueue* q) {
  assert(0 <= i && i < _n, "index out of range.");
  _queues[i] = q;
}

OopTaskQueue* OopTaskQueueSet::queue(int i) {
  return _queues[i];
}

bool OopTaskQueueSet::steal(int queue_num, int* seed, Task& t) {
  for (int i = 0; i < 2 * _n; i++)
    if (steal_best_of_2(queue_num, seed, t))
      return true;
  return false;
}

bool OopTaskQueueSet::steal_best_of_all(int queue_num, int* seed, Task& t) {
  if (_n > 2) {
    int best_k;
    jint best_sz = 0;
    for (int k = 0; k < _n; k++) {
      if (k == queue_num) continue;
      jint sz = _queues[k]->size();
      if (sz > best_sz) {
	best_sz = sz;
	best_k = k;
      }
    }
    return best_sz > 0 && _queues[best_k]->pop_global(t);
  } else if (_n == 2) {
    // Just try the other one.
    int k = (queue_num + 1) % 2;
    return _queues[k]->pop_global(t);
  } else {
    assert(_n == 1, "can't be zero.");
    return false;
  }
}

bool OopTaskQueueSet::steal_1_random(int queue_num, int* seed, Task& t) {
  if (_n > 2) {
    int k = queue_num;
    while (k == queue_num) k = randomParkAndMiller(seed) % _n;
    return _queues[2]->pop_global(t);
  } else if (_n == 2) {
    // Just try the other one.
    int k = (queue_num + 1) % 2;
    return _queues[k]->pop_global(t);
  } else {
    assert(_n == 1, "can't be zero.");
    return false;
  }
}

bool OopTaskQueueSet::steal_best_of_2(int queue_num, int* seed, Task& t) {
  if (_n > 2) {
    int k1 = queue_num;
    while (k1 == queue_num) k1 = randomParkAndMiller(seed) % _n;
    int k2 = queue_num;
    while (k2 == queue_num || k2 == k1) k2 = randomParkAndMiller(seed) % _n;
    // Sample both and try the larger.
    juint sz1 = _queues[k1]->size();
    juint sz2 = _queues[k2]->size();
    if (sz2 > sz1) return _queues[k2]->pop_global(t);
    else return _queues[k1]->pop_global(t);
  } else if (_n == 2) {
    // Just try the other one.
    int k = (queue_num + 1) % 2;
    return _queues[k]->pop_global(t);
  } else {
    assert(_n == 1, "can't be zero.");
    return false;
  }
}

bool OopTaskQueueSet::peek() {
  // Try all the queues.
  for (int j = 0; j < _n; j++) {
    if (_queues[j]->peek())
      return true;
  }
  return false;
}
