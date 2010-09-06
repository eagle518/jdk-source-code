#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)init.cpp	1.104 04/06/15 12:17:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_init.cpp.incl"

// Initialization done by VM thread in vm_init_globals()
void check_ThreadShadow();
void check_basic_types();
void eventlog_init();
void mutex_init();
void chunkpool_init();
void perfMemory_init();

// Initialization done by Java thread in init_globals()
void management_init();
void vtune_init();
void bytecodes_init();
void classLoader_init();
void codeCache_init();
void icache_init();
void VM_Version_init();
void stubRoutines_init1();
void carSpace_init();
jint universe_init();  // dependent on codeCache_init and stubRoutines_init 
void interpreter_init();  // before any methods loaded
void invocationCounter_init();  // before any methods loaded
void marksweep_init();
void accessFlags_init();
void templateTable_init();
void InterfaceSupport_init();
void universe2_init();  // dependent on codeCache_init and stubRoutines_init
void referenceProcessor_init();
void jni_handles_init();
void vmStructs_init();
void verificationType_init();

#ifdef COMPILER1
void compiler1_init();
void vtableStubs_init();
void InlineCacheBuffer_init();
void compilerOracle_init();
void onStackReplacement_init();
void compilationPolicy_init();
#endif

#ifdef COMPILER2
void adapter_init();
void vtableStubs_init();
void InlineCacheBuffer_init();
void compilerOracle_init();
void compiler2_init();  // needs ObjectArrayKlass vtables
void onStackReplacement_init();
void compilationPolicy_init();
#endif


// Initialization after compiler initialization
void universe_post_init();  // must happen after compiler_init
void javaClasses_init();  // must happen after vtable initialization
void stubRoutines_init2(); // note: StubRoutines need 2-phase init 

// Do not disable thread-local-storage, as it is important for some
// JNI/JVM/JVMPI/JVMDI functions and signal handlers to work properly
// during VM shutdown
void verificationType_exit();
void perfMemory_exit();
void ostream_exit();

#ifdef CORE
void adapter_init()           {}
void invocationCounter_init() {}
void sweeper_init()           {}
#endif // CORE


void vm_init_globals() {
  check_ThreadShadow();
  check_basic_types();
  eventlog_init();
  mutex_init();
  chunkpool_init();
  perfMemory_init();
}


jint init_globals() {  
  management_init();
  vtune_init();
  bytecodes_init();
  classLoader_init();
  codeCache_init();
  icache_init();
  VM_Version_init();
  stubRoutines_init1();
  carSpace_init();
  jint status = universe_init();  // dependent on codeCache_init and stubRoutines_init 
  if (status != JNI_OK)
    return status;

  interpreter_init();  // before any methods loaded
  invocationCounter_init();  // before any methods loaded
  marksweep_init();
  accessFlags_init();
  templateTable_init();
  InterfaceSupport_init();
  universe2_init();  // dependent on codeCache_init and stubRoutines_init
  referenceProcessor_init();
  jni_handles_init();
  vmStructs_init();
  verificationType_init();

#ifdef COMPILER1
  compiler1_init();
  vtableStubs_init();
  InlineCacheBuffer_init();
  compilerOracle_init();
  onStackReplacement_init();
  compilationPolicy_init();
#endif

#ifdef COMPILER2
  adapter_init();
  vtableStubs_init();
  InlineCacheBuffer_init();
  compilerOracle_init();
  compiler2_init();  // needs ObjectArrayKlass vtables
  onStackReplacement_init();
  compilationPolicy_init();
#endif

  universe_post_init();  // must happen after compiler_init
  javaClasses_init();  // must happen after vtable initialization
  stubRoutines_init2(); // note: StubRoutines need 2-phase init 

  // Although we'd like to, we can't easily do a heap verify
  // here because the main thread isn't yet a JavaThread, so
  // its TLAB may not be made parseable from the usual interfaces.
  if (VerifyBeforeGC && !UseTLAB &&
      Universe::heap()->total_collections() >= VerifyGCStartAt) { 
    Universe::heap()->prepare_for_verify();
    Universe::verify();   // make sure we're starting with a clean slate
  }

  return JNI_OK;
}


void exit_globals() {
  static bool destructorsCalled = false;
  if (!destructorsCalled) {
    destructorsCalled = true;
    verificationType_exit();
    perfMemory_exit();
    ostream_exit();
  }
}


static bool _init_completed = false;

bool is_init_completed() {
  return _init_completed;
}


void set_init_completed() {
  _init_completed = true;
}
