#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmtiImpl.cpp	1.36 04/01/05 16:38:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmtiImpl.cpp.incl"

JvmtiEnv *jvmti_env_for_jvmdi;

GrowableArray<JvmtiRawMonitor*> *JvmtiPendingMonitors::_monitors = new (ResourceObj::C_HEAP) GrowableArray<JvmtiRawMonitor*>(1,true);

void JvmtiPendingMonitors::transition_raw_monitors() {
  assert((Threads::number_of_threads()==1),
         "Java thread has not created yet or more than one java thread \
is running. Raw monitor transition will not work");
  JavaThread *current_java_thread = JavaThread::current();
  assert(current_java_thread->thread_state() == _thread_in_vm, "Must be in vm");
  {
    ThreadBlockInVM __tbivm(current_java_thread);
    for(int i=0; i< count(); i++) {
      JvmtiRawMonitor *rmonitor = monitors()->at(i);
      int r = rmonitor->raw_enter(current_java_thread, false);
      assert(r == ObjectMonitor::OM_OK, "raw_enter should have worked");
    }
  }
  // pending monitors are converted to real monitor so delete them all.
  dispose();
}

//
// class JvmtiAgentThread
//
// JavaThread used to wrap a thread started by an agent
// using the JVMTI method RunAgentThread.
//

JvmtiAgentThread::JvmtiAgentThread(JvmtiEnv* env, jvmtiStartFunction start_fn, const void *start_arg)
    : JavaThread(start_function_wrapper) {
    _env = env;
    _start_fn = start_fn;
    _start_arg = start_arg;
}

JvmtiAgentThread::JvmtiAgentThread(JVMDI_StartFunction jvmdi_start_fn, const void *start_arg)
    : JavaThread(start_function_wrapper) {
    _env = NULL;
    _jvmdi_start_fn = jvmdi_start_fn;
    _start_arg = start_arg;
}

void
JvmtiAgentThread::start_function_wrapper(JavaThread *thread, TRAPS) {
    // It is expected that any Agent threads will be created as
    // Java Threads.  If this is the case, notification of the creation
    // of the thread is given in JavaThread::thread_main().
    assert(thread->is_Java_thread(), "debugger thread should be a Java Thread");
    assert(thread == JavaThread::current(), "sanity check");
  
    JvmtiAgentThread *dthread = (JvmtiAgentThread *)thread;
    dthread->call_start_function();
}

void
JvmtiAgentThread::call_start_function() {
    ThreadToNativeFromVM transition(this);
    if (_env == NULL) {
      // JVMDI
     _jvmdi_start_fn((void*)_start_arg);
    } else {
      // JVMTI
      _start_fn(_env->jvmti_external(), jni_environment(), (void*)_start_arg);
    }
}

//
// class JvmtiUtil
//

ResourceArea* JvmtiUtil::_single_threaded_resource_area = NULL;

ResourceArea* JvmtiUtil::single_threaded_resource_area() {
  if (_single_threaded_resource_area == NULL) {
    // lazily create the single threaded resource area
    // pick a size which is not a standard since the pools don't exist yet
    _single_threaded_resource_area = new ResourceArea(Chunk::non_pool_size);
  }
  return _single_threaded_resource_area;
}

//
// class JvmtiTrace
//
// Support for JVMTI/JVMDI tracing code
//
// ------------
// Usage:
//    -XX:TraceJVMTI=DESC,DESC,DESC
//
//    DESC is   DOMAIN ACTION KIND
//
//    DOMAIN is function name
//              event name
//              "all" (all functions and events)
//              "func" (all functions except boring)
//              "allfunc" (all functions)
//              "event" (all events)
//              "ec" (event controller)
//
//    ACTION is "+" (add)
//              "-" (remove)
//
//    KIND is
//     for func
//              "i" (input params)
//              "e" (error returns)
//              "o" (output)
//     for event
//              "t" (event triggered aka posted)
//              "s" (event sent)
//
// Example:
//            -XX:TraceJVMTI=ec+,GetCallerFrame+ie,Breakpoint+s

#ifdef JVMTI_TRACE

bool JvmtiTrace::_initialized = false;
bool JvmtiTrace::_on = false;
bool JvmtiTrace::_trace_event_controller = false;

void JvmtiTrace::initialize() {
  if (_initialized) {
    return;
  }
  SafeResourceMark rm;
  
  const char *very_end;
  const char *curr;
  if (strlen(TraceJVMTI)) {
    curr = TraceJVMTI;
  } else {
    curr = "";  // hack in fixed tracing here
  }
  very_end = curr + strlen(curr);
  while (curr < very_end) {
    const char *curr_end = strchr(curr, ',');
    if (curr_end == NULL) {
      curr_end = very_end;
    }
    const char *op_pos = strchr(curr, '+');
    const char *minus_pos = strchr(curr, '-');
    if (minus_pos != NULL && (minus_pos < op_pos || op_pos == NULL)) {
      op_pos = minus_pos;
    }
    char op;
    const char *flags = op_pos + 1;
    const char *flags_end = curr_end;
    if (op_pos == NULL || op_pos > curr_end) {
      flags = "ies";
      flags_end = flags + strlen(flags);
      op_pos = curr_end;
      op = '+';
    } else {
      op = *op_pos;
    }
    jbyte bits = 0;
    for (; flags < flags_end; ++flags) {
      switch (*flags) {
      case 'i': 
        bits |= SHOW_IN;
        break;
      case 'I': 
        bits |= SHOW_IN_DETAIL;
        break;
      case 'e': 
        bits |= SHOW_ERROR;
        break;
      case 'o': 
        bits |= SHOW_OUT;
        break;
      case 'O': 
        bits |= SHOW_OUT_DETAIL;
        break;
      case 't': 
        bits |= SHOW_EVENT_TRIGGER;
        break;
      case 's': 
        bits |= SHOW_EVENT_SENT;
        break;
      default:
        tty->print_cr("Invalid trace flag '%c'", *flags);
        break;
      }
    }
    const int FUNC = 1;
    const int EXCLUDE  = 2;
    const int ALL_FUNC = 4;
    const int EVENT = 8;
    const int ALL_EVENT = 16;
    int domain = 0;
    size_t len = op_pos - curr;
    if (op_pos == curr) {
      domain = ALL_FUNC | FUNC | ALL_EVENT | EVENT | EXCLUDE;
    } else if (len==3 && strncmp(curr, "all", 3)==0) {
      domain = ALL_FUNC | FUNC | ALL_EVENT | EVENT;
    } else if (len==7 && strncmp(curr, "allfunc", 7)==0) {
      domain = ALL_FUNC | FUNC;
    } else if (len==4 && strncmp(curr, "func", 4)==0) {
      domain = ALL_FUNC | FUNC | EXCLUDE;
    } else if (len==8 && strncmp(curr, "allevent", 8)==0) {
      domain = ALL_EVENT | EVENT;
    } else if (len==5 && strncmp(curr, "event", 5)==0) {
      domain = ALL_EVENT | EVENT;
    } else if (len==2 && strncmp(curr, "ec", 2)==0) {
      _trace_event_controller = true;
      tty->print_cr("JVMTI Tracing the event controller");
    } else {
      domain = FUNC | EVENT;  // go searching
    }

    int exclude_index = 0;
    if (domain & FUNC) {
      if (domain & ALL_FUNC) {
        if (domain & EXCLUDE) {
          tty->print("JVMTI Tracing all significant functions");
        } else {
          tty->print_cr("JVMTI Tracing all functions");
        }
      }
      for (int i = 0; i <= _max_function_index; ++i) {
        if (domain & EXCLUDE && i == _exclude_functions[exclude_index]) {
          ++exclude_index;
        } else {
          bool do_op = false;
          if (domain & ALL_FUNC) {
            do_op = true;
          } else {
            const char *fname = function_name(i);
            if (fname != NULL) {
              size_t fnlen = strlen(fname);
              if (len==fnlen && strncmp(curr, fname, fnlen)==0) {
                tty->print_cr("JVMTI Tracing the function: %s", fname);
                do_op = true;
              }
            }
          }
          if (do_op) {
            if (op == '+') {
              _trace_flags[i] |= bits;
            } else {
              _trace_flags[i] &= ~bits;
            }
            _on = true;
          }
        }
      }
    }
    if (domain & EVENT) {
      if (domain & ALL_EVENT) {
        tty->print_cr("JVMTI Tracing all events");
      }
      for (int i = 0; i <= _max_event_index; ++i) {
        bool do_op = false;
        if (domain & ALL_EVENT) {
          do_op = true;
        } else {
          const char *ename = event_name(i);
          if (ename != NULL) {
            size_t evtlen = strlen(ename);
            if (len==evtlen && strncmp(curr, ename, evtlen)==0) {
              tty->print_cr("JVMTI Tracing the event: %s", ename);
              do_op = true;
            }
          }
        }
        if (do_op) {
          if (op == '+') {
            _event_trace_flags[i] |= bits;
          } else {
            _event_trace_flags[i] &= ~bits;
          }
          _on = true;
        }
      }
    }
    if (!_on && (domain & (FUNC|EVENT))) {
      tty->print_cr("JVMTI Trace domain not found");
    }
    curr = curr_end + 1;
  }
  _initialized = true;
}


void JvmtiTrace::shutdown() {
  int i;
  _on = false;
  _trace_event_controller = false;
  for (i = 0; i <= _max_function_index; ++i) {
    _trace_flags[i] = 0;
  }
  for (i = 0; i <= _max_event_index; ++i) {
    _event_trace_flags[i] = 0;
  }
}


const char* JvmtiTrace::enum_name(const char** names, const jint* values, jint value) {
  for (int index = 0; names[index] != 0; ++index) {
    if (values[index] == value) {
      return names[index];
    }
  }
  return "*INVALID-ENUM-VALUE*";
}


// return a valid string no matter what state the thread is in
const char *JvmtiTrace::safe_get_thread_name(JavaThread *java_thread) {
  if (java_thread == NULL) {
    return "NULL";
  }
  oop threadObj = java_thread->threadObj();
  if (threadObj == NULL) {
    return "NULL";
  }
  typeArrayOop name = java_lang_Thread::name(threadObj);
  if (name == NULL) {
    return "<NOT FILLED IN>";
  }
  return UNICODE::as_utf8((jchar*) name->base(T_CHAR), name->length());
}
    

// return the name of the current thread
const char *JvmtiTrace::safe_get_current_thread_name() {
  if (JvmtiEnv::is_vm_live()) {
    return JvmtiTrace::safe_get_thread_name(JavaThread::current());
  } else {
    return "VM not live";
  }
}

// return a valid string no matter what the state of k_mirror
const char * JvmtiTrace::get_class_name(oop k_mirror) {
  if (java_lang_Class::is_primitive(k_mirror)) {
    return "primitive";
  }
  klassOop k_oop = java_lang_Class::as_klassOop(k_mirror);
  if (k_oop == NULL) {
    return "INVALID";
  }
  return Klass::cast(k_oop)->external_name();
}

#endif /*JVMTI_TRACE */

//
// class GrowableCache - private methods
//

void GrowableCache::recache() {
  int len = _elements->length();

  FREE_C_HEAP_ARRAY(address, _cache);
  _cache = NEW_C_HEAP_ARRAY(address,len+1);

  for (int i=0; i<len; i++) {
    _cache[i] = _elements->at(i)->getCacheValue();
    //
    // The cache entry has gone bad. Without a valid frame pointer
    // value, the entry is useless so we simply delete it in product
    // mode. The call to remove() will rebuild the cache again
    // without the bad entry.
    //
    if (_cache[i] == NULL) {
      assert(false, "cannot recache NULL elements");
      remove(i);
      return;
    }
  }
  _cache[len] = NULL;

  _listener_fun(_this_obj,_cache);
}

bool GrowableCache::equals(void* v, GrowableElement *e2) {
  GrowableElement *e1 = (GrowableElement *) v;
  assert(e1 != NULL, "e1 != NULL");
  assert(e2 != NULL, "e2 != NULL");

  return e1->equals(e2);
}

//
// class GrowableCache - public methods
//

GrowableCache::GrowableCache() {
  _this_obj       = NULL;
  _listener_fun   = NULL;    
  _elements       = NULL;
  _cache          = NULL;
}

GrowableCache::~GrowableCache() {
  clear();
  _elements->clear_and_deallocate();
  FreeHeap(_elements);
  FREE_C_HEAP_ARRAY(address, _cache);
}

void GrowableCache::initialize(void *this_obj, void listener_fun(void *, address*) ) {
  _this_obj       = this_obj;
  _listener_fun   = listener_fun;    
  _elements       = new (ResourceObj::C_HEAP) GrowableArray<GrowableElement*>(5,true);
  recache();
}

// number of elements in the collection
int GrowableCache::length() { 
  return _elements->length(); 
}

// get the value of the index element in the collection
GrowableElement* GrowableCache::at(int index) {
  GrowableElement *e = (GrowableElement *) _elements->at(index);
  assert(e != NULL, "e != NULL");
  return e;
}
 
int GrowableCache::find(GrowableElement* e) {
  return _elements->find(e, GrowableCache::equals);
}

// append a copy of the element to the end of the collection
void GrowableCache::append(GrowableElement* e) {
  GrowableElement *new_e = e->clone();
  _elements->append(new_e);
  recache();
}

// insert a copy of the element using lessthan()
void GrowableCache::insert(GrowableElement* e) {
  GrowableElement *new_e = e->clone();
  _elements->append(new_e);

  int n = length()-2;
  for (int i=n; i>=0; i--) {
    GrowableElement *e1 = _elements->at(i);
    GrowableElement *e2 = _elements->at(i+1);
    if (e2->lessThan(e1)) {
      _elements->at_put(i+1, e1);
      _elements->at_put(i,   e2);
    }
  }

  recache();
}

// remove the element at index
void GrowableCache::remove (int index) {
  GrowableElement *e = _elements->at(index);
  assert(e != NULL, "e != NULL");
  _elements->remove(e);
  delete e;
  recache();
}

// clear out all elements, release all heap space and
// let our listener know that things have changed.
void GrowableCache::clear() {
  int len = _elements->length();
  for (int i=0; i<len; i++) {
    delete _elements->at(i);
  }
  _elements->clear();
  recache();
}

void GrowableCache::oops_do(OopClosure* f) {
  int len = _elements->length();
  for (int i=0; i<len; i++) {
    GrowableElement *e = _elements->at(i);
    e->oops_do(f);
  }
}

void GrowableCache::gc_epilogue() {
  int len = _elements->length();
  // recompute the new cache value after GC
  for (int i=0; i<len; i++) {
    _cache[i] = _elements->at(i)->getCacheValue();
  }
}


//
// class JvmtiRawMonitor
//

JvmtiRawMonitor::JvmtiRawMonitor(const char *name) {
#ifdef ASSERT
  _name = strcpy(NEW_C_HEAP_ARRAY(char, strlen(name) + 1), name);
#else
  _name = NULL;
#endif
  _magic = JVMTI_RM_MAGIC;
}

JvmtiRawMonitor::~JvmtiRawMonitor() {
#ifdef ASSERT
  FreeHeap(_name);
#endif
  _magic = 0;
}


//
// class JvmtiBreakpoint
//

JvmtiBreakpoint::JvmtiBreakpoint() {
  _method = NULL;
  _bci    = 0;
}

JvmtiBreakpoint::JvmtiBreakpoint(methodOop m_method, jlocation location) {
  _method        = m_method;
  assert(_method != NULL, "_method != NULL");
  _bci           = (int) location;  

  assert(_bci >= 0, "_bci >= 0"); 
}

void JvmtiBreakpoint::copy(JvmtiBreakpoint& bp) {
  _method   = bp._method;
  _bci      = bp._bci;
}

bool JvmtiBreakpoint::lessThan(JvmtiBreakpoint& bp) {
  Unimplemented();
  return false;
}

bool JvmtiBreakpoint::equals(JvmtiBreakpoint& bp) {
  return _method   == bp._method
    &&   _bci      == bp._bci;
}

bool JvmtiBreakpoint::is_valid() {
  return _method != NULL &&
         _bci >= 0;
}

address JvmtiBreakpoint::getBcp() {
  return _method->bcp_from(_bci);
}

void JvmtiBreakpoint::each_method_version_do(method_action meth_act) {
  (_method->*meth_act)(_bci);

  // add/remove breakpoint to/from all EMCP previous
  // versions of the method
  Thread *thread = Thread::current();
  ResourceMark rm(thread);
  instanceKlassHandle ikh = instanceKlassHandle(thread, _method->method_holder());
  symbolOop meth_name = _method->name();
  symbolOop meth_signature = _method->signature();
  while (ikh->has_previous_version()) {
    ikh = instanceKlassHandle(thread, ikh->previous_version());
    methodOop m = ikh->find_method(meth_name, meth_signature);
    if (m == NULL) {
      assert(0, "method addition isn't implemented yet");
      return;  // method must have been added
    } else {
      // matching method found
      if (m->is_non_emcp_with_new_version()) {
        return; // don't set/clear in directly or transitively non-EMCP
      } else {
        (m->*meth_act)(_bci);
      }
    }
  }
}

void JvmtiBreakpoint::set() {
  each_method_version_do(&methodOopDesc::set_breakpoint);
}

void JvmtiBreakpoint::clear() {
  each_method_version_do(&methodOopDesc::clear_breakpoint);
}

void JvmtiBreakpoint::print() {
#ifndef PRODUCT
  const char *class_name  = (_method == NULL) ? "NULL" : _method->klass_name()->as_C_string();
  const char *method_name = (_method == NULL) ? "NULL" : _method->name()->as_C_string();

  tty->print("Breakpoint(%s,%s,%d,%p)",class_name, method_name, _bci, getBcp());
#endif
}


//
// class VM_ChangeBreakpoints
//
// Modify the Breakpoints data structure at a safepoint
//

void VM_ChangeBreakpoints::doit() {
  switch (_operation) {
  case SET_BREAKPOINT:
    _breakpoints->set_at_safepoint(*_bp);
    break;
  case CLEAR_BREAKPOINT:
    _breakpoints->clear_at_safepoint(*_bp);
    break;
  case CLEAR_ALL_BREAKPOINT:
    _breakpoints->clearall_at_safepoint();
    break;
  default:
    assert(false, "Unknown operation");
  }
}

//
// class JvmtiBreakpoints 
//
// a JVMTI internal collection of JvmtiBreakpoint
//

JvmtiBreakpoints::JvmtiBreakpoints(void listener_fun(void *,address *)) {
  _bps.initialize(this,listener_fun);
}

JvmtiBreakpoints:: ~JvmtiBreakpoints() {}

void  JvmtiBreakpoints::oops_do(OopClosure* f) {  
  _bps.oops_do(f);
} 

void  JvmtiBreakpoints::gc_epilogue() {  
  _bps.gc_epilogue();
} 

void  JvmtiBreakpoints::print() {
#ifndef PRODUCT
  ResourceMark rm;

  int n = _bps.length();
  for (int i=0; i<n; i++) {
    JvmtiBreakpoint& bp = _bps.at(i);
    tty->print("%d: ", i);
    bp.print();
    tty->print_cr("");
  }
#endif
}


void JvmtiBreakpoints::set_at_safepoint(JvmtiBreakpoint& bp) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");

  int i = _bps.find(bp);
  if (i == -1) { 
    _bps.append(bp);
    bp.set();
  }
}

void JvmtiBreakpoints::clear_at_safepoint(JvmtiBreakpoint& bp) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");

  int i = _bps.find(bp);
  if (i != -1) {
    _bps.remove(i);
    bp.clear();
  }
}

void JvmtiBreakpoints::clearall_at_safepoint() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");

  int len = _bps.length();
  for (int i=0; i<len; i++) {
    _bps.at(i).clear();
  }
  _bps.clear();
}
 
int JvmtiBreakpoints::length() { return _bps.length(); }

int JvmtiBreakpoints::set(JvmtiBreakpoint& bp) {
  if ( _bps.find(bp) != -1) {
     return JVMTI_ERROR_DUPLICATE;
  }
  VM_ChangeBreakpoints set_breakpoint(this,VM_ChangeBreakpoints::SET_BREAKPOINT, &bp);
  VMThread::execute(&set_breakpoint);
  return JVMTI_ERROR_NONE;
}

int JvmtiBreakpoints::clear(JvmtiBreakpoint& bp) {
  if ( _bps.find(bp) == -1) {
     return JVMTI_ERROR_NOT_FOUND;
  }

  VM_ChangeBreakpoints clear_breakpoint(this,VM_ChangeBreakpoints::CLEAR_BREAKPOINT, &bp);
  VMThread::execute(&clear_breakpoint);
  return JVMTI_ERROR_NONE;
}

void JvmtiBreakpoints::clearall_in_class_at_safepoint(klassOop klass) {
  bool changed = true;
  // We are going to run thru the list of bkpts
  // and delete some.  This deletion probably alters
  // the list in some implementation defined way such
  // that when we delete entry i, the next entry might
  // no longer be at i+1.  To be safe, each time we delete
  // an entry, we'll just start again from the beginning.
  // We'll stop when we make a pass thru the whole list without
  // deleting anything.
  while (changed) {
    int len = _bps.length();
    changed = false;
    for (int i = 0; i < len; i++) {
      JvmtiBreakpoint& bp = _bps.at(i);
      if (bp.method()->method_holder() == klass) {
        bp.clear();
        _bps.remove(i);
        // This changed 'i' so we have to start over.
        changed = true;
        break;
      }
    }
  }
}

void JvmtiBreakpoints::clearall() {
  VM_ChangeBreakpoints clearall_breakpoint(this,VM_ChangeBreakpoints::CLEAR_ALL_BREAKPOINT);
  VMThread::execute(&clearall_breakpoint);
}

//
// class JvmtiCurrentBreakpoints
// 

JvmtiBreakpoints *JvmtiCurrentBreakpoints::_jvmti_breakpoints  = NULL;
address *         JvmtiCurrentBreakpoints::_breakpoint_list    = NULL;


JvmtiBreakpoints& JvmtiCurrentBreakpoints::get_jvmti_breakpoints() {
  if (_jvmti_breakpoints != NULL) return (*_jvmti_breakpoints);
  _jvmti_breakpoints = new JvmtiBreakpoints(listener_fun);
  assert(_jvmti_breakpoints != NULL, "_jvmti_breakpoints != NULL");
  return (*_jvmti_breakpoints);
}

void  JvmtiCurrentBreakpoints::listener_fun(void *this_obj, address *cache) { 
  JvmtiBreakpoints *this_jvmti = (JvmtiBreakpoints *) this_obj;
  assert(this_jvmti != NULL, "this_jvmti != NULL");

  debug_only(int n = this_jvmti->length(););
  assert(cache[n] == NULL, "cache must be NULL terminated");

  set_breakpoint_list(cache);
}


void JvmtiCurrentBreakpoints::oops_do(OopClosure* f) { 
  if (_jvmti_breakpoints != NULL) {
    _jvmti_breakpoints->oops_do(f);
  }
}

void JvmtiCurrentBreakpoints::gc_epilogue() { 
  if (_jvmti_breakpoints != NULL) {
    _jvmti_breakpoints->gc_epilogue();
  }
}


///////////////////////////////////////////////////////////////
//
// class VM_GetOrSetLocal
//

VM_GetOrSetLocal::VM_GetOrSetLocal(JavaThread* thread, jint depth, int index, BasicType type)
  : _thread(thread)
  , _depth(depth)
  , _index(index)
  , _type(type)
  , _set(false)
  , _result(JVMTI_ERROR_NONE)
{  
}

VM_GetOrSetLocal::VM_GetOrSetLocal(JavaThread* thread, jint depth, int index, BasicType type, jvalue value)
  : _thread(thread)
  , _depth(depth)
  , _index(index)
  , _type(type)
  , _value(value)
  , _set(true)
  , _result(JVMTI_ERROR_NONE)
{
}

VM_GetOrSetLocal::VM_GetOrSetLocal(JavaThread* thread, jint depth, int index, Handle* value)
  : _thread(thread)
  , _depth(depth)
  , _index(index)
  , _type(T_OBJECT)
  , _obj(value)
  , _set(true)
  , _result(JVMTI_ERROR_NONE)
{
}


vframe *VM_GetOrSetLocal::get_vframe() {
  RegisterMap reg_map(_thread);
  vframe *vf = _thread->last_java_vframe(&reg_map);
  int d = 0;
  while ((vf != NULL) && (d < _depth)) {
    vf = vf->java_sender();
    d++;
  }
  return vf;
}

javaVFrame *VM_GetOrSetLocal::get_java_vframe() {
  vframe* vf = get_vframe();

  if (vf == NULL) {
    _result = JVMTI_ERROR_NO_MORE_FRAMES;
    return NULL;
  }
  if (!vf->is_java_frame()) {
    _result = JVMTI_ERROR_OPAQUE_FRAME;
    return NULL;
  }
  javaVFrame *jvf = (javaVFrame*)vf;

  if (jvf->method()->is_native()) {
    _result = JVMTI_ERROR_OPAQUE_FRAME;
    return NULL;
  }
  return jvf;
}

// Checks error conditions:
//   JVMTI_ERROR_INVALID_SLOT
//   JVMTI_ERROR_TYPE_MISMATCH
// Returns: 'true' - everything is Ok, 'false' - error code
  
bool VM_GetOrSetLocal::check_slot_type(javaVFrame* jvf) {
  methodOop method_oop = jvf->method();

  if (!method_oop->has_localvariable_table()) {
    // Just to check index boundaries
    jint extra_slot = (_type == T_LONG || _type == T_DOUBLE) ? 1 : 0;
    if (_index < 0 || _index + extra_slot >= method_oop->max_locals()) {
      _result = JVMTI_ERROR_INVALID_SLOT;
      return false;
    }
    return true;
  }

  jint num_entries = method_oop->localvariable_table_length();
  if (num_entries == 0) {
    _result = JVMTI_ERROR_INVALID_SLOT;
    return false;	// There are no slots
  }
  int signature_idx = -1;
  int vf_bci = jvf->bci();
  LocalVariableTableElement* table = method_oop->localvariable_table_start();
  for (int i = 0; i < num_entries; i++) {
    int start_bci = table[i].start_bci;
    int end_bci = start_bci + table[i].length;

    // Here we assume that locations of LVT entries
    // with the same slot number cannot be overlapped

    if (_index == (jint) table[i].slot && start_bci <= vf_bci && vf_bci <= end_bci) {
      signature_idx = (int) table[i].descriptor_cp_index;
      break;
    }
  }
  if (signature_idx == -1) {
    _result = JVMTI_ERROR_INVALID_SLOT;
    return false;	// Incorrect slot index
  }
  constantPoolOop constants = method_oop->constants();
  const char *signature = (const char *)constants->symbol_at(signature_idx)->as_utf8();
  BasicType slot_type = char2type(signature[0]);

  switch (slot_type) {
  case T_BYTE:
  case T_SHORT:
  case T_CHAR:
  case T_BOOLEAN:
    slot_type = T_INT;
    break;
  case T_ARRAY:
    slot_type = T_OBJECT;
    break;
  };    
  if (_type != slot_type) {
    _result = JVMTI_ERROR_TYPE_MISMATCH;
    return false;
  }
  return true;
}

#ifndef CORE
static bool can_be_deoptimized(vframe* vf) {
  return (vf->is_compiled_frame() && vf->fr().can_be_deoptimized());
}
#endif /* CORE */


bool VM_GetOrSetLocal::doit_prologue() { 
  if (!_thread->has_last_Java_frame()) {
    _result = JVMTI_ERROR_NO_MORE_FRAMES;
    return false;
  }
  return true;
}

void VM_GetOrSetLocal::doit() {
  javaVFrame* jvf = get_java_vframe();

  if (jvf == NULL) {
    return;
  }
  if (!check_slot_type(jvf)) {
    return;
  }

  if (_set) {
#ifndef CORE
    // Force deoptimization of frame if compiled because it's
    // possible the compiler emitted some locals as constant values,
    // meaning they are not mutable.
    if (can_be_deoptimized(jvf)) {

      // Schedule deoptimization so that eventually the local
      // update will be written to an interpreter frame.
      VM_DeoptimizeFrame deopt(jvf->thread(), jvf->fr().id());
      VMThread::execute(&deopt);

      // Now store a new value for the local which will be applied
      // once deoptimization occurs. Note however that while this
      // write is deferred until deoptimization actually happens
      // can vframe created after this point will have its locals
      // reflecting this update so as far as anyone can see the
      // write has already taken place.

      // If we are updating an oop then get the oop from the handle
      // since the handle will be long gone by the time the deopt
      // happens. The oop stored in the deferred local will be
      // gc'd on its own.
      if (_type == T_OBJECT) {
	_value.l = (jobject) (*_obj)();
      }
      ((compiledVFrame*)jvf)->update_local(_type, _index, _value);

      return;
    }
#endif /* CORE */
    StackValueCollection *locals = jvf->locals();

    switch (_type) {
    case T_INT:    locals->set_int_at   (_index, _value.i); break;
    case T_LONG:   locals->set_long_at  (_index, _value.j); break;
    case T_FLOAT:  locals->set_float_at (_index, _value.f); break;
    case T_DOUBLE: locals->set_double_at(_index, _value.d); break;
    case T_OBJECT: locals->set_obj_at   (_index, *_obj);    break;
    default: ShouldNotReachHere();
    }
    jvf->set_locals(locals);
  } else {
    StackValueCollection *locals = jvf->locals();

    switch (_type) {
    case T_INT:    _value.i = locals->int_at   (_index);   break;
    case T_LONG:   _value.j = locals->long_at  (_index);   break;
    case T_FLOAT:  _value.f = locals->float_at (_index);   break;
    case T_DOUBLE: _value.d = locals->double_at(_index);   break;
    case T_OBJECT: _oop_result = locals->obj_at(_index)(); break;
    default: ShouldNotReachHere();
    }
  }
}


bool VM_GetOrSetLocal::allow_nested_vm_operations() const {
  return true; // May need to deoptimize
}


/////////////////////////////////////////////////////////////////////////////////////////

//
// class JvmtiSuspendControl - see comments in jvmtiImpl.hpp
//

bool JvmtiSuspendControl::suspend(JavaThread *java_thread) {  
  // external suspend should have caught suspending a thread twice

  // Immediate suspension required for JPDA back-end so JVMTI agent threads do
  // not deadlock due to later suspension on transitions while holding
  // raw monitors.  Passing true causes the immediate suspension.
  // java_suspend() will catch threads in the process of exiting
  // and will ignore them.
  java_thread->java_suspend();

  // It would be nice to have the following assertion in all the time,
  // but it is possible for a racing resume request to have resumed
  // this thread right after we suspended it. Temporarily enable this
  // assertion if you are chasing a different kind of bug.
  //
  // assert(java_lang_Thread::thread(java_thread->threadObj()) == NULL ||
  //   java_thread->is_being_ext_suspended(), "thread is not suspended");

  if (java_lang_Thread::thread(java_thread->threadObj()) == NULL) {
    // check again because we can get delayed in java_suspend():
    // the thread is in process of exiting.
    return false;
  }

  return true;
}

bool JvmtiSuspendControl::resume(JavaThread *java_thread) {  
  // external suspend should have caught resuming a thread twice
  assert(java_thread->is_being_ext_suspended(), "thread should be suspended");

  JvmtiThreadState *state = java_thread->jvmti_thread_state();
  if (state != NULL && JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) {
    state->jvmdi_cached_frames()->clear_cached_frames();
  }

  // resume thread
  {
    // must always grab Threads_lock, see JVM_SuspendThread
    MutexLocker ml(Threads_lock);  
    java_thread->java_resume(); 
  }
 
  return true;
}


void JvmtiSuspendControl::print() {
#ifndef PRODUCT
  MutexLocker mu(Threads_lock);
  ResourceMark rm;

  tty->print("Suspended Threads: [");
  for (JavaThread *thread = Threads::first(); thread != NULL; thread = thread->next()) {
#if JVMTI_TRACE
    const char *name   = JvmtiTrace::safe_get_thread_name(thread);
#else
    const char *name   = "";
#endif /*JVMTI_TRACE */
    tty->print("%s(%c ", name, thread->is_being_ext_suspended() ? 'S' : '_');
    if (!thread->has_last_Java_frame()) {
      tty->print("no stack");
    }
    tty->print(") ");
  }
  tty->print_cr("]");
#endif  
}
