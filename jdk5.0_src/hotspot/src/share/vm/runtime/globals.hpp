#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals.hpp	1.826 04/05/13 09:57:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

typedef const char* ccstr;  // string type alias used only in this file

struct Flag {
  const char *type;
  const char *name;
  void*       addr;
  const char *kind;
  bool        was_set;

  // points to all Flags static array
  static Flag *flags;

  // number of flags
  static size_t numFlags;

  static Flag* find_flag(char* name, size_t length);

  bool is_bool() const        { return strcmp(type, "bool") == 0; }
  int get_bool() const        { return *((bool*) addr); }
  void set_bool(bool value)   { *((bool*) addr) = value; }

  bool is_intx()  const       { return strcmp(type, "intx")  == 0; }
  intx get_intx() const       { return *((intx*) addr); }
  void set_intx(intx value)   { *((intx*) addr) = value; }

  bool is_uintx() const       { return strcmp(type, "uintx") == 0; }
  uintx get_uintx() const     { return *((uintx*) addr); }
  void set_uintx(uintx value) { *((uintx*) addr) = value; }

  bool is_ccstr() const       { return strcmp(type, "ccstr") == 0; }
  ccstr get_ccstr() const     { return *((ccstr*) addr); }
  void set_ccstr(ccstr value) { *((ccstr*) addr) = value; }

  bool is_unlocker() const;
  bool is_unlocked() const;

  void print_on(outputStream* st);
  void print_as_flag(outputStream* st);
};

// debug flags control various aspects of the VM and are global accessible

// use FlagSetting to temporarily change some debug flag
// e.g. FlagSetting fs(DebugThisAndThat, true);   
// restored to previous value upon leaving scope
class FlagSetting {
  bool val;
  bool* flag;
 public:
  FlagSetting(bool& fl, bool newValue) { flag = &fl; val = fl; fl = newValue; }
  ~FlagSetting()                       { *flag = val; }
};


class CounterSetting {
  intx* counter;
 public:
  CounterSetting(intx* cnt) { counter = cnt; (*counter)++; }
  ~CounterSetting()         { (*counter)--; }
};


class IntFlagSetting {
  intx val;
  intx* flag;
 public:
  IntFlagSetting(intx& fl, intx newValue) { flag = &fl; val = fl; fl = newValue; }
  ~IntFlagSetting()                       { *flag = val; }
};


class CommandLineFlags {
 public:
  static bool boolAt(char* name, size_t len, bool* value);
  static bool boolAt(char* name, bool* value)      { return boolAt(name, strlen(name), value); }
  static bool boolAtPut(char* name, size_t len, bool* value);
  static bool boolAtPut(char* name, bool* value)   { return boolAtPut(name, strlen(name), value); }

  static bool intxAt(char* name, size_t len, intx* value);
  static bool intxAt(char* name, intx* value)      { return intxAt(name, strlen(name), value); }
  static bool intxAtPut(char* name, size_t len, intx* value);
  static bool intxAtPut(char* name, intx* value)   { return intxAtPut(name, strlen(name), value); }

  static bool uintxAt(char* name, size_t len, uintx* value);
  static bool uintxAt(char* name, uintx* value)    { return uintxAt(name, strlen(name), value); }
  static bool uintxAtPut(char* name, size_t len, uintx* value);
  static bool uintxAtPut(char* name, uintx* value) { return uintxAtPut(name, strlen(name), value); }

  static bool ccstrAt(char* name, size_t len, ccstr* value);
  static bool ccstrAt(char* name, ccstr* value)    { return ccstrAt(name, strlen(name), value); }
  static bool ccstrAtPut(char* name, size_t len, ccstr* value);
  static bool ccstrAtPut(char* name, ccstr* value) { return ccstrAtPut(name, strlen(name), value); }

  // Returns false if name is not a command line flag.
  static bool wasSetOnCmdline(const char* name, bool* value);
  static void printSetFlags();

  static void printFlags() PRODUCT_RETURN;
  
  static void verify() PRODUCT_RETURN;
};

// use this for flags that are true per default in the debug version but false in the 
// optimized version, and vice versa
#ifdef ASSERT
#define trueInDebug  true
#define falseInDebug false
#else
#define trueInDebug  false
#define falseInDebug true
#endif

// use this for flags that are true per default in the product build
// but false in development builds, and vice versa
#ifdef PRODUCT
#define trueInProduct  true
#define falseInProduct false
#else
#define trueInProduct  false
#define falseInProduct true
#endif

// develop flags are settable / visible only during development and are constant in the PRODUCT version
// product flags are always settable / visible

// A flag must be declared with one of the following types:
// bool, intx, uintx, ccstr.
// The type "ccstr" is an alias for "const char*" and is used
// only in this file, because the macrology requires single-token type names.

// Note: Diagnostic options not meant for VM tuning or for product modes.
// They are to be used for VM quality assurance or field diagnosis
// of VM bugs.  They are hidden so that users will not be encouraged to
// try them as if they were VM ordinary execution options.  However, they
// are available in the product version of the VM.  Under instruction
// from support engineers, VM customers can turn them on to collect
// diagnostic information about VM problems.  To use a VM diagnostic
// option, you must first specify +UnlockDiagnosticVMOptions.
// (This master switch also affects the behavior of -Xprintflags.)

#define RUNTIME_FLAGS(develop, develop_pd, product, product_pd, diagnostic) \
                                                                            \
  product(bool, PrintCommandLineFlags, false,                               \
          "Prints flags that appeared on the command line")                 \
                                                                            \
  diagnostic(bool, UnlockDiagnosticVMOptions, trueInDebug,                  \
          "Enable processing of flags relating to field diagnostics")       \
                                                                            \
  product(bool, JavaMonitorsInStackTrace, true,                             \
          "Print info. about Java monitor locks when the stacks are dumped")\
                                                                            \
  product(bool, UseISM, false,                                              \
          "Use Intimate Shared Memory (Solaris Only)")                      \
                                                                            \
  product(bool, UsePermISM, false,                                          \
          "Use ISM that stays in the system (Solaris Only)")                \
									    \
  product(bool, UseMPSS, true,                                              \
          "Use Multiple Page Size Support (Solaris 9 Only)")                \
                                                                            \
  product(uintx, LargePageSizeInBytes, 4*M,                                 \
          "Large page size")						    \
                                               				    \
  product(uintx, LargePageHeapSizeThreshold, 128*M,                         \
          "Use large pages if max heap is at least this big")		    \
                                               				    \
  develop(bool, UseNewOSExceptions, true,                                   \
          "Temp. flag for new OS exception tracking on Win32")              \
                                                                            \
  product(bool, ForceTimeHighResolution, false,                             \
          "Using high time resolution(For Win32 only)")                     \
                                                                            \
  develop(bool, TraceItables, false,                                        \
          "Trace initialization and use of itables")                        \
                                                                            \
  develop(bool, TracePcPatching, false,                                     \
          "Trace usage of frame::patch_pc")                                 \
                                                                            \
  develop(bool, TraceJumps, false,                                          \
          "Trace assembly jumps in thread ring buffer")                     \
                                                                            \
  develop(bool, TraceRelocator, false,                                      \
          "Trace the bytecode relocator")                                   \
                                                                            \
  develop(bool, TraceLongCompiles, false,                                   \
          "Print out every time compilation is longer than "                \
          "a given threashold")                                             \
                                                                            \
  product_pd(bool, SafepointPolling,                                        \
          "Uses polling rather than code patching for Safepoints")          \
                                                                            \
  develop(bool, SafepointALot, false,                                       \
          "Generates a lot of safepoints. Works with "                      \
          "GuaranteedSafepointInterval")                                    \
                                                                            \
  product_pd(bool, BackgroundCompilation,                                   \
          "A thread requesting compilation is not blocked during "          \
          "compilation")                                                    \
                                                                            \
  product_pd(bool, UseDefaultStackSize,                                     \
          "Use default size 0 for thread size on"                           \
          "Windows NT")                                                     \
                                                                            \
  develop(bool, TraceCompilationStalls, false,                              \
          "Prints out the length of a stall in ms. a thread is stalled "    \
          "due to compilation")                                             \
                                                                            \
  develop(bool, BailoutToInterpreterForThrows, false,                       \
          "Compiled methods which throws/catches exceptions will be "       \
          "deopt and intp.")                                                \
                                                                            \
  develop(bool, NoYieldsInMicrolock, false,                                 \
          "Disable yields in microlock")                                    \
                                                                            \
  develop(bool, TraceOopMapGeneration, false,                               \
          "Shows oopmap generation")                                        \
                                                                            \
  develop(bool, MethodFlushing, true,                                       \
          "Reclamation of zombie and not-entrant methods")                  \
                                                                            \
  develop(bool, VerifyStack, false,                                         \
          "Verify stack of each thread when it is entering a runtime call") \
                                                                            \
  develop(bool, StressDerivedPointers, false,                               \
          "Force scavenge when a derived pointers is detected on stack "    \
          "after rtm call")                                                 \
                                                                            \
  develop(bool, TraceDerivedPointers, false,                                \
          "Trace traversal of derived pointers on stack")                   \
                                                                            \
  develop(bool, TraceCodeBlobStacks, false,                                 \
          "Trace stack-walk of codeblobs")                                  \
                                                                            \
  develop(bool, TraceAdapterGeneration, false,                              \
          "Prints out whih adapters gets generated")                        \
                                                                            \
  product(bool, PrintJNIResolving, false,                                   \
          "Used to implement -v:jni")                                       \
                                                                            \
  develop(bool, PrintRewrites, false,                                       \
          "Print methods that are being rewritten")                         \
                                                                            \
  product(bool, UseInlineCaches, true,                                      \
          "Use Inline Caches for virtual calls ")                           \
                                                                            \
  develop(bool, InlineArrayCopy, true,                                      \
          "inline arraycopy native that is known to be part of "            \
          "base library DLL")                                               \
                                                                            \
  develop(bool, InlineObjectHash, true,                                     \
          "inline Object::hashCode() native that is known to be part "      \
          "of base library DLL")                                            \
                                                                            \
  develop(bool, InlineNatives, true,                                        \
          "inline natives that are known to be part of base library DLL")   \
                                                                            \
  develop(bool, InlineMathNatives, true,                                    \
          "inline SinD, CosD, etc.")                                        \
                                                                            \
  develop(bool, InlineClassNatives, true,                                   \
          "inline Class.isInstance, etc")                                   \
                                                                            \
  develop(bool, InlineAtomicLong, true,                                     \
          "inline sun.misc.AtomicLong")                                     \
                                                                            \
  develop(bool, InlineThreadNatives, true,                                  \
          "inline Thread.currentThread, etc")                               \
                                                                            \
  develop(bool, InlineReflectionGetCallerClass, true,                       \
          "inline sun.reflect.Reflection.getCallerClass(), known to be part "\
          "of base library DLL")                                            \
                                                                            \
  develop(bool, InlineUnsafeOps, true,                                      \
          "inline memory ops (native methods) from sun.misc.Unsafe")        \
                                                                            \
  product(bool, CheckUnsafeOps, false,                                      \
          "perform extra consistency checks on calls to sun.misc.Unsafe")   \
                                                                            \
  develop(bool, ConvertCmpD2CmpF, true,                                     \
          "Convert cmpD to cmpF when one input is constant in float range") \
                                                                            \
  develop(bool, ConvertFloat2IntClipping, true,                             \
          "Convert float2int clipping idiom to integer clipping")           \
                                                                            \
  develop(bool, SpecialStringCompareTo, true,                               \
          "special version of string compareTo")                            \
                                                                            \
  develop(bool, TraceCallFixup, false,                                      \
          "traces all call fixups")                                         \
                                                                            \
  develop(bool, DeoptimizeALot, false,                                      \
          "deoptimize at every exit from the runtime system")               \
                                                                            \
  develop(bool, DeoptimizeRandom, false,                                    \
          "deoptimize random frames on random exit from the runtime system")\
                                                                            \
  develop(bool, ZombieALot, false,                                          \
          "creates zombies (non-entrant) at exit from the runt. system")    \
                                                                            \
  develop(bool, WalkStackALot, false,                                       \
          "trace stack (no print) at every exit from the runtime system")   \
                                                                            \
  develop(bool, Debugging, false,                                           \
          "set when executing debug methods in debug.ccp "                  \
          "(to prevent triggering assertions)")                             \
                                                                            \
  develop(bool, StrictSafepointChecks, trueInDebug,                         \
          "Enable strict checks that safepoints cannot happen for threads " \
          "that used No_Safepoint_Verifier")                                \
                                                                            \
  develop(bool, VerifyLastFrame, false,                                     \
          "Verify oops on last frame on entry to VM")                       \
                                                                            \
  develop(bool, TraceHandleAllocation, false,                               \
          "Prints out warnings when suspicious many handles are allocated") \
                                                                            \
  product(bool, UseCompilerSafepoints, true,                                \
          "use safepoints in compiled code")                                \
                                                                            \
  product(bool, UseSplitVerifier, false,                                    \
          "use split verifier using StackMap attributes (experimental)")    \
                                                                            \
  develop(bool, UseStrictCompilerSafepoints, true,                          \
          "force safepoints in all casses in compiled code (SPARC only)")   \
                                                                            \
  develop_pd(bool, UseCallBackInInterpreter,                                \
          "Do call back to reach safepoint when a thread is in "            \
          "the interpreter")                                                \
                                                                            \
  develop(bool, ShowSafepointMsgs, false,                                   \
          "Show msg. about safepoint synch.")                               \
                                                                            \
  develop(bool, SafepointTimeout, false,                                    \
          "Time out and warn or fail after SafepointTimeoutDelay "          \
          "milliseconds if failed to reach safepoint")                      \
                                                                            \
  develop(bool, DieOnSafepointTimeout, false,                               \
          "Die upon failure to reach safepoint (see SafepointTimeout)")     \
                                                                            \
  product(bool, UseSuspendResumeThreadLists, true,                          \
          "Enable SuspendThreadList and ResumeThreadList")                  \
                                                                            \
  product(bool, MaxFDLimit, true,                                           \
          "Bump the number of file descriptors to max in solaris.")         \
                                                                            \
  develop(bool, LogEvents, trueInDebug,                                     \
          "Enable Event log")                                               \
                                                                            \
  product(bool, BytecodeVerificationRemote, true,                           \
          "Enables the Java bytecode verifier for remote classes")          \
                                                                            \
  product(bool, BytecodeVerificationLocal, false,                           \
          "Enables the Java bytecode verifier for local classes")           \
                                                                            \
  develop(bool, ForceFloatExceptions, trueInDebug,                          \
          "Force exceptions on FP stack under/overflow")                    \
                                                                            \
  develop(bool, SoftMatchFailure, trueInProduct,                            \
          "If the DFA fails to match a node, print a message and bail out") \
                                                                            \
  develop(bool, VerifyStackAtCalls, false,                                  \
          "Verify that the stack pointer is unchanged after calls")         \
                                                                            \
  develop(bool, TraceJavaAssertions, false,                                 \
          "Trace java language assertions")                                 \
                                                                            \
  develop(bool, CheckAssertionStatusDirectives, false,                      \
          "temporary - see javaClasses.cpp")                                \
                                                                            \
  develop(bool, PrintMallocFree, false,                                     \
          "Trace calls to C heap malloc/free allocation")                   \
                                                                            \
  develop(bool, PrintOopAddress, false,                                     \
          "Always print the location of the oop")                           \
                                                                            \
  develop(bool, VerifyScopes, false,                                        \
          "Verify scope descs when nmethod is created")                     \
                                                                            \
  develop(bool, VerifyCodeCacheOften, false,                                \
          "Verify compiled-code cache often")                               \
                                                                            \
  develop(bool, ZapDeadCompiledLocals, false,                               \
          "Zap dead locals in compiler frames")                             \
                                                                            \
  develop(bool, ZapDeadLocalsOld, false,                                    \
          "Zap dead locals (old version, zaps all frames when "             \
          "entering the VM")                                                \
                                                                            \
  develop(bool, CheckOopishValues, false,                                   \
          "Warn if value contains oop ( requires ZapDeadLocals)")           \
                                                                            \
  develop(bool, UseMallocOnly, false,                                       \
          "use only malloc/free for allocation (no resource area/arena)")   \
                                                                            \
  develop(bool, PrintMalloc, false,                                         \
          "print all malloc/free calls")                                    \
                                                                            \
  develop(bool, ZapResourceArea, trueInDebug,                               \
          "Zap freed resource/arena space with 0xABABABAB")                 \
                                                                            \
  develop(bool, ZapVMHandleArea, trueInDebug,                               \
          "Zap freed VM handle space with 0xBCBCBCBC")                      \
                                                                            \
  develop(bool, ZapJNIHandleArea, trueInDebug,                              \
          "Zap freed JNI handle space with 0xFEFEFEFE")                     \
                                                                            \
  develop(bool, ZapUnusedHeapArea, trueInDebug,                             \
          "Zap unused heap space with 0xBAADBABE")                          \
                                                                            \
  develop(bool, PrintVMMessages, true,                                      \
          "Print vm messages on console")                                   \
									    \
  product(bool, PrintGCApplicationConcurrentTime, false,		    \
	  "Print the time the application has been running") 		    \
									    \
  product(bool, PrintGCApplicationStoppedTime, false,			    \
	  "Print the time the application has been stopped") 		    \
                                                                            \
  develop(bool, Verbose, false,                                             \
          "Prints additional debugging information from other modes")       \
                                                                            \
  develop(bool, PrintMiscellaneous, false,                                  \
          "Prints uncategorized debugging information (requires +Verbose)") \
                                                                            \
  develop(bool, WizardMode, false,                                          \
          "Prints much more debugging information")                         \
                                                                            \
  product(bool, ShowMessageBoxOnError, false,                               \
          "Keep process alive on VM fatal error")                           \
                                                                            \
  product(ccstr, OnError, "",                                               \
          "Run user-defined commands on fatal error; see VMError.cpp "      \
          "for examples")                                                   \
                                                                            \
  develop(bool, BreakAtWarning, false,                                      \
          "Execute breakpoint upon encountering VM warning")                \
                                                                            \
  develop(bool, UseTopLevelExceptionFilter, true,                           \
          "Filter top level exceptions in OS - turn off for debugging")     \
                                                                            \
  develop(bool, TraceVMOperation, false,                                    \
          "Trace vm operations")                                            \
                                                                            \
  develop(bool, UseFakeTimers, false,                                       \
          "Tells whether the VM should use system time or a fake timer")    \
                                                                            \
  diagnostic(bool, LogCompilation, false,                                   \
          "Log compilation activity in detail to hotspot.log or LogFile")   \
                                                                            \
  product(bool, PrintCompilation, false,                                    \
          "Print compilations")                                             \
                                                                            \
  develop(bool, PrintAdapterCompilation, false,                             \
          "Print adapter compilations")                                     \
                                                                            \
  develop(bool, PrintCompilation2, false,                                   \
          "Print additional statistics per compilation")                    \
                                                                            \
  develop(bool, PrintAssembly, false,                                       \
          "Print assembly code")                                            \
                                                                            \
  develop(bool, PrintNMethods, false,                                       \
          "Print assembly code for nmethods when generated")                \
                                                                            \
  develop(bool, PrintNativeNMethods, false,                                 \
          "Print assembly code for native nmethods when generated")         \
                                                                            \
  develop(bool, PrintDebugInfo, false,                                      \
          "Print debug information for all nmethods when generated")        \
                                                                            \
  develop(bool, PrintRelocations, false,                                    \
          "Print relocation information for all nmethods when generated")   \
                                                                            \
  develop(bool, PrintDependencies, false,                                   \
          "Print dependency information for all nmethods when generated")   \
                                                                            \
  develop(bool, PrintExceptionHandlers, false,                              \
          "Print exception handler tables for all nmethods when generated") \
                                                                            \
  develop(bool, InterceptOSException, false,                                \
          "Starts debugger when an implicit OS (e.g., NULL) "               \
          "exception happens")                                              \
                                                                            \
  develop(bool, PrintCodeCache, false,                                      \
          "Print the compiled_code cache when exiting")                     \
                                                                            \
  develop(bool, PrintCodeCache2, false,                                     \
          "Print detailed info on the compiled_code cache when exiting")    \
                                                                            \
  develop(bool, DumpEventsAtError, false,                                   \
          "Prnt part of event log when crashing")                           \
                                                                            \
  develop(bool, PrintStubCode, false,                                       \
          "Print generated stub code")                                      \
                                                                            \
  product(bool, StackTraceInThrowable, true,                                \
          "Collect backtrace in throwable when exception happens")          \
                                                                            \
  product(bool, OmitStackTraceInFastThrow, true,                            \
          "Omit backtraces for some 'hot' exceptions in optimized code")    \
                                                                            \
  product(bool, ProfilerPrintByteCodeStatistics, false,                     \
          "Prints byte code statictics when dumping profiler output")       \
                                                                            \
  product(bool, ProfilerRecordPC, false,                                    \
          "Collects tick for each 16 byte interval of compiled code")       \
                                                                            \
  product(bool, ProfileVM,  false,                                          \
          "Profiles ticks that fall within VM (either in the VM Thread "    \
          "or VM code called through stubs)")                               \
                                                                            \
  product(bool, ProfileIntervals, false,                                    \
          "Prints profiles for each interval (see ProfileIntervalsTicks)")  \
                                                                            \
  product(bool, ProfilerLight, false,                                       \
          "Tries to avoid Solaris libthread Bug Id: 4207397 at a possible " \
          "precision cost. Needed to profile JDK under Solaris, etc.")      \
                                                                            \
  develop(bool, ProfilerCheckIntervals, false,                              \
          "Collect and print info on spacing of profiler ticks")            \
                                                                            \
  develop(bool, PrintJVMWarnings, false,                                    \
          "Prints warnings for unimplemented JVM functions")                \
                                                                            \
  develop(uintx, WarnOnStalledSpinLock, 0,                                  \
          "Prints warnings for stalled SpinLocks")                          \
                                                                            \
  develop(bool, InitializeJavaLangSystem, true,                             \
          "Initialize java.lang.System - turn off for individual "          \
          "method debugging")                                               \
                                                                            \
  develop(bool, InitializeJavaLangString, true,                             \
          "Initialize java.lang.String - turn off for individual "          \
          "method debugging")                                               \
                                                                            \
  develop(bool, InitializeJavaLangExceptionsErrors, true,                   \
          "Initialize various error and exception classes - turn off for "  \
          "individual method debugging")                                    \
                                                                            \
  develop(bool, RegisterFinalizers, true,                                   \
          "Tells whether the VM should register finalizable objects")       \
                                                                            \
  develop(bool, RegisterReferences, true,                                   \
          "Tells whether the VM should register soft/weak/final/phantom "   \
          "references")                                                     \
                                                                            \
  develop(bool, IgnoreRewrites, false,                                      \
          "Supress rewrites of bytecodes in the oopmap generator. "         \
          "This is unsafe!")                                                \
                                                                            \
  develop(bool, PrintCodeCacheExtension, false,                             \
          "Print extension of code cache")                                  \
                                                                            \
  develop(bool, UsePrivilegedStack, true,                                   \
          "Enable the security JVM functions")                              \
                                                                            \
  develop(bool, IEEEPrecision, true,                                        \
          "Enables IEEE precision (for INTEL only)")                        \
                                                                            \
  develop(bool, ProtectionDomainVerification, true,                         \
          "Verifies protection domain before resolution in system "         \
          "dictionary")                                                     \
                                                                            \
  product(bool, ClassUnloading, true,                                       \
          "Do unloading of classes")                                        \
                                                                            \
  develop(bool, AlwaysUseXchg, false,                                       \
          "Use xchg-based locking even on uniprocessors")                   \
                                                                            \
  develop(bool, DisableStartThread, false,                                  \
          "Disable starting of additional Java threads "                    \
          "(for debugging only)")                                           \
                                                                            \
  develop(bool, MemProfiling, false,                                        \
          "Write memory usage profiling to log file")                       \
                                                                            \
  develop(bool, PrintSystemDictionaryAtExit, false,                         \
          "Prints the system dictionary at exit")                           \
                                                                            \
  develop(bool, UseInterruptibleIO, true,                                   \
          "Interrupt threads using signals (for SOLARIS only)")             \
                                                                            \
  develop(bool, UseThrGetStateWorkaround, true,                             \
          "Use sanity checks to work around thr_getstate() bug 4335248."    \
          "(SOLARIS only)")                                                 \
                                                                            \
  product_pd(bool, DontYieldALot,                                           \
          "Throw away obvious excess yield calls (for SOLARIS only)")       \
                                                                            \
  product_pd(bool, ConvertSleepToYield,                                     \
          "Converts sleep(0) to thread yield "                              \
          "(may be off for SOLARIS to improve GUI)")                        \
                                                                            \
  product(bool, ConvertYieldToSleep, false,                                 \
          "Converts yield to a sleep of MinSleepInterval to simulate Win32 "\
          "behavior (SOLARIS only)")                                        \
                                                                            \
  product(bool, UseBoundThreads, true,                                      \
          "Bind user level threads to kernel threads (for SOLARIS only)")   \
                                                                            \
  develop(bool, UseDetachedThreads, true,                                   \
          "Use detached threads that are recycled upon termination "        \
          "(for SOLARIS only)")                                             \
                                                                            \
  product(bool, UseLWPSynchronization, true,                                \
          "Use LWP-based instead of libthread-based synchronization "       \
          "(SPARC only)")                                                   \
                                                                            \
  product(ccstr, NativeSynchronization, "",                                 \
          "Selects native synchronization mechanism")                       \
                                                                            \
  product(intx, AppendRatio, 11,                                            \
          "Monitor entry queue append-prepend ratio")                       \
                                                                            \
  product(bool, FilterSpuriousWakeups , true,                               \
	  "Prevent spurious or premature wakeups from object.wait"          \
	  "(Solaris only)")                                                 \
                                                                            \
  develop(bool, UsePthreads, false,                                         \
          "Use pthread-based instead of libthread-based synchronization "   \
          "(SPARC only)")                                                   \
                                                                            \
  product(bool, AdjustConcurrency, false,                                   \
          "call thr_setconcurrency at thread create time to avoid "         \
          "LWP starvation on MP systems (For Solaris Only)")                \
                                                                            \
  product(bool, OverrideDefaultLibthread, false,                            \
          "Default libthread for Solaris 9 is T2, default for previous "    \
          "is T1. This flag overrides the default. (For Solaris Only)")     \
                                                                            \
  develop(bool, UpdateHotSpotCompilerFileOnError, true,                     \
          "Should the system attempt to update the compiler file when "     \
          "an error occurs?")                                               \
                                                                            \
  product(bool, ReduceSignalUsage, false,                                   \
          "Reduce the use of OS signals in Java and/or the VM")             \
                                                                            \
  develop(bool, ValidateMarkSweep, false,                                   \
          "Do extra validation during MarkSweep collection")                \
                                                                            \
  develop(bool, RecordMarkSweepCompaction, false,                           \
          "Enable GC-to-GC recording and querying of compaction during "    \
          "MarkSweep")							    \
                                                                            \
  develop_pd(bool, ShareVtableStubs,                                        \
          "Share vtable stubs (smaller code but worse branch prediction")   \
                                                                            \
  develop(bool, SwitchDecisionTrees, true,                                  \
          "Use decision trees instead of linear compare/branch when "       \
          "compiling switch")                                               \
                                                                            \
  develop(bool, LoadLineNumberTables, true,                                 \
          "Tells whether the class file parser loads line number tables")   \
                                                                            \
  develop(bool, LoadLocalVariableTables, true,                              \
          "Tells whether the class file parser loads local variable tables")\
                                                                            \
  develop(bool, LoadLocalVariableTypeTables, true,                          \
          "Tells whether the class file parser loads local variable type tables")\
                                                                            \
  product(bool, ExitOnBadLibThread, true,                                   \
          "Do not continue VM startup if libthread is unpatched "           \
          "(Solaris only)")                                                 \
                                                                            \
  product(bool, AllowUserSignalHandlers, false,                             \
          "Do not complain if the application installs signal handlers "    \
          "(Solaris & Linux only)")                                         \
                                                                            \
  product(bool, UseSignalChaining, true,                                    \
          "Use signal-chaining to invoke signal handlers installed "        \
          "by the application (Solaris & Linux only)")                      \
                                                                            \
  product(bool, UseAltSigs, false,                                          \
          "Use alternate signals instead of SIGUSR1 & SIGUSR2 for VM "      \
          "internal signals. (Solaris & Linux only)")                       \
                                                                            \
  product(bool, UseSpinning, false,                                         \
          "Use spinning in monitor inflation and before entry")             \
                                                                            \
  product(bool, PreSpinYield, false,                                        \
          "Yield before inner spinning loop")                               \
                                                                            \
  product(bool, PostSpinYield, true,                                        \
          "Yield after inner spinning loop")                                \
                                                                            \
  product(bool, CheckJNICalls, false,                                       \
          "Verify all arguments to JNI calls")                              \
                                                                            \
  product(bool, UseFastJNIAccessors, true,                                  \
          "Use optimized versions of Get<Primitive>Field")                  \
                                                                            \
  product(bool, FastNMethodDependencies, true,                              \
          "Record nmethod dependencies in instanceKlass for faster checks") \
                                                                            \
  product(bool, EagerXrunInit, false,                                       \
          "Eagerly initialize -Xrun libraries; allows startup profiling, "  \
          " but not all -Xrun libraries may support the state of the VM at this time") \
                                                                            \
  product(bool, PreserveAllAnnotations, false,                              \
          "Preserve RuntimeInvisibleAnnotations as well as RuntimeVisibleAnnotations") \
                                                                            \
  /* tracing */                                                             \
                                                                            \
  develop(bool, TraceRuntimeCalls, false,                                   \
          "Trace run-time calls")                                           \
                                                                            \
  develop(bool, TraceJVMPI, false,                                          \
          "Trace JVMPI")                                                    \
                                                                            \
  develop(bool, TraceJNICalls, false,                                       \
          "Trace JNI calls")                                                \
                                                                            \
  develop(bool, TraceJVMCalls, false,                                       \
          "Trace JVM calls")                                                \
                                                                            \
  product(ccstr, TraceJVMTI, "",                                            \
          "Trace flags for JVMTI/JVMDI functions and events")               \
                                                                            \
  develop(bool, StopOnJVMDIErrorReturn, false,                              \
          "Stop if JVMDI call returns with error")                          \
                                                                            \
  develop(bool, TraceJNIHandleAllocation, false,                            \
          "Trace allocation/deallocation of JNI handle blocks")             \
                                                                            \
  develop(bool, TraceThreadEvents, false,                                   \
          "Trace all thread events")                                        \
                                                                            \
  develop(bool, TraceBytecodes, false,                                      \
          "Trace bytecode execution")                                       \
                                                                            \
  develop(bool, TraceClassInitialization, false,                            \
          "Trace class initialization")                                     \
                                                                            \
  develop(bool, TraceExceptions, false,                                     \
          "Trace exceptions")                                               \
                                                                            \
  develop(bool, VMStackTraceAtException, false,                             \
          "Print VM stack trace at exceptions")                             \
                                                                            \
  develop(bool, VMStackTraceAtNullException, false,                         \
          "Print VM stack trace at NullPointerExceptions")                  \
                                                                            \
  develop(bool, TraceICs, false,                                            \
          "Trace inline cache changes")                                     \
                                                                            \
  develop(bool, TraceInvocationCounterOverflow, false,                      \
          "Trace method invocation counter overflow")                       \
                                                                            \
  develop(bool, TraceInlineCacheClearing, false,                            \
          "Trace clearing of inline caches in nmethods")                    \
                                                                            \
  develop(bool, TraceDependencies, false,                                   \
          "Trace dependencies")                                             \
                                                                            \
  develop(bool, TraceNewOopMapGeneration, false,                            \
          "Trace OopMapGeneration")                                         \
                                                                            \
  develop(bool, TraceNewOopMapGenerationDetailed, false,                    \
          "Trace OopMapGeneration: print detailed cell states")             \
                                                                            \
  develop(bool, TimeOopMap, false,                                          \
          "Time calls to GenerateOopMap::compute_map() in sum")             \
                                                                            \
  develop(bool, TimeOopMap2, false,                                         \
          "Time calls to GenerateOopMap::compute_map() individually")       \
                                                                            \
  develop(bool, TraceMonitorMismatch, false,                                \
          "Trace monitor matching failures during OopMapGeneration")        \
                                                                            \
  develop(bool, TraceOopMapRewrites, false,                                 \
          "Trace rewritting of method oops during oop map generation")      \
                                                                            \
  develop(bool, TraceSafepoint, false,                                      \
          "Trace safepoint operations")                                     \
                                                                            \
  develop(bool, TraceMethodIterator, false,                                 \
          "Trace execution of MethodIterator")                              \
                                                                            \
  develop(bool, TraceICBuffer, false,                                       \
          "Trace usage of IC buffer")                                       \
                                                                            \
  develop(bool, TraceCompiledIC, false,                                     \
          "Trace changes of compiled IC")                                   \
                                                                            \
  develop(bool, TraceZapDeadLocals, false,                                  \
          "Trace zapping dead locals")                                      \
                                                                            \
  develop(bool, TraceStartupTime, false,                                    \
          "Trace setup time")                                               \
                                                                            \
  develop(bool, TraceHPI, false,                                            \
          "Trace Host Porting Interface (HPI)")                             \
                                                                            \
  develop(bool, TraceProtectionDomainVerification, false,                   \
          "Trace protection domain verifcation")                            \
                                                                            \
  develop(bool, TraceClearedExceptions, false,                              \
          "Prints when an exception is forcibly cleared")                   \
                                                                            \
  product(bool, TraceClassResolution, false,                                \
          "Trace all constant pool resolutions (for debugging)")            \
                                                                            \
  /* assembler */                                                           \
  product(bool, Use486InstrsOnly, false,                                    \
          "Use 80486 Compliant instruction subset")                         \
                                                                            \
  /* gc */                                                                  \
                                                                            \
  product(bool, UseSerialGC, false,                                         \
          "Tells whether the VM should use serial garbage collector")       \
                                                                            \
  product(bool, UseTrainGC, false,                                          \
          "Tells whether the VM should use train algorithm GC for "         \
          "tenured objects")                                                \
                                                                            \
  product(bool, UseParallelGC, false,                                       \
          "Use the Parallel Scavenge garbage collector")                    \
                                                                            \
  product(uintx, ParallelGCThreads, 0,                                      \
          "Number of parallel threads parallel gc will use")                \
                                                                            \
  product(uintx, YoungPLABSize, 4096,                     		    \
          "Size of young gen promotion labs (in HeapWords)")                \
                                                                            \
  product(uintx, OldPLABSize, 1024,                                         \
          "Size of old gen promotion labs (in HeapWords)")                  \
                                                                            \
  product(uintx, GCTaskTimeStampEntries, 200,                               \
          "Number of time stamp entries per gc worker thread")              \
                                                                            \
  product(bool, AlwaysTenure, false,                                        \
          "Always tenure objects in eden. (ParallelGC only)")               \
                                                                            \
  product(bool, NeverTenure, false,                                         \
          "Never tenure objects in eden, May tenure on overflow"            \
          " (ParallelGC only)")                                             \
                                                                            \
  product(bool, ScavengeBeforeFullGC, true,                                 \
          "Scavenge youngest generation before each full GC,"               \
          " used with UseParallelGC")                                       \
                                                                            \
  product(bool, UseConcMarkSweepGC, false,                                  \
          "Use Concurrent Mark-Sweep GC in the old generation")             \
                                                                            \
  develop(bool, UseCMSAdaptiveFreeLists, true,                              \
          "Use Adaptive Free Lists in the CMS generation")      	    \
                                                                            \
  develop(bool, UseAsyncConcMarkSweepGC, true,                              \
          "Use Asynchronous Concurrent Mark-Sweep GC in the old generation")\
									    \
  product(bool, UseCMSBestFit, true,                                  	    \
          "Use CMS best fit allocation strategy")			    \
                                                                            \
  product(bool, UseCMSCollectionPassing, true,                              \
          "Use passing of collection from background to foreground")	    \
                                                                            \
  product(bool, UseParNewGC, false,                                         \
          "Use parallel threads in the new generation.")                    \
                                                                            \
  product(bool, ParallelGCVerbose, false,                                   \
          "Verbose output for parallel GC.")                                \
                                                                            \
  product(intx, ParallelGCOldGenAllocBufferSize, 1*K,                       \
          "Size of parallel GC allocation buffers in the old gen")          \
                                                                            \
  product(intx, ParallelGCToSpaceAllocBufferSize, 1*K,                      \
          "Size of parallel GC allocation buffers in to-space.")            \
                                                                            \
  product(intx, ParallelGCG1AllocBufferSize, 1*K,  	                    \
          "Size of parallel G1 allocation buffers in to-space.")            \
                                                                            \
  product(intx, ParallelGCTermSpinLoopMS, 5,                                \
          "msec of spinning iterations of 'peek' loop in termination.")     \
                                                                            \
  product(intx, ParallelGCBufferWastePct, 10,                               \
          "wasted fraction of parallel allocation buffer.")                 \
                                                                            \
  product(bool, ParallelGCRetainPLAB, true,                                \
          "Retain parallel allocation buffers across scavenges.")           \
                                                                            \
  product(intx, ParGCArrayScanChunk, 50,                                    \
          "Scan a subset and push remainder, if array is bigger than this") \
                                                                            \
  product(intx, ParGCDesiredObjsFromOverflowList, 20,                       \
          "The desired number of objects to claim from the overflow list")  \
                                                                            \
  product(intx, ParCMSPromoteBlocksToClaim, 50,                             \
          "Number of blocks to attempt to claim when refilling CMS LAB for "\
          "parallel GC.")				                    \
                                                                            \
  product(bool, CMSIncrementalMode, false,                    	            \
	  "Whether CMS GC should operate in \"incremental\" mode")          \
									    \
  product(uintx, CMSIncrementalDutyCycle, 50,				    \
	  "CMS incremental mode duty cycle (a percentage, 0-100).  If"	    \
	  "CMSIncrementalPacing is enabled, then this is just the initial"  \
 	  "value")							    \
									    \
  product(bool, CMSIncrementalPacing, false,				    \
	  "Whether the CMS incremental mode duty cycle should be "	    \
	  "automatically adjusted")					    \
									    \
  product(uintx, CMSIncrementalDutyCycleMin, 10,			    \
	  "Lower bound on the duty cycle when CMSIncrementalPacing is"	    \
 	  "enabled (a percentage, 0-100).")				    \
									    \
  product(uintx, CMSIncrementalSafetyFactor, 10,	    		    \
	  "Percentage (0-100) used to add conservatism when computing the"  \
	  "duty cycle.")						    \
									    \
  product(uintx, CMSIncrementalOffset, 0,				    \
	  "Percentage (0-100) by which the CMS incremental mode duty cycle" \
	  "is shifted to the right within the period between young GCs")    \
									    \
  product(uintx, CMSExpAvgFactor, 25,				    	    \
	  "Percentage (0-100) used to weight the current sample when"	    \
	  "computing exponential averages for CMS statistics.")		    \
									    \
  develop(bool, CMSTraceIncrementalMode, false,				    \
	  "Trace CMS incremental mode")					    \
									    \
  develop(bool, CMSTraceIncrementalPacing, false,			    \
	  "Trace CMS incremental mode pacing computation")		    \
									    \
  develop(bool, CMSTraceThreadState, false,				    \
	  "Trace the CMS thread state (enable the trace_state() method)")   \
									    \
  product(bool, CMSClassUnloadingEnabled, false,                      	    \
          "Whether class unloading enabled when using CMS GC")              \
                                                                            \
  product(bool, CMSCompactWhenClearAllSoftRefs, true,                 	    \
          "Compact when asked to collect CMS gen with clear_all_soft_refs") \
                                                                            \
  product(bool, UseCMSCompactAtFullCollection, true,                        \
          "Use mark sweep compact at full collections")	    		    \
                                                                            \
  product(uintx, CMSFullGCsBeforeCompaction, 0,                             \
          "Number of CMS full collection done before compaction if > 0")    \
                                                                            \
  develop(intx, CMSDictionaryChoice, 0,                                     \
          "Use BinaryTreeDictionary as default in the CMS generation")      \
                                                                            \
  product(uintx, CMSIndexedFreeListReplenish, 4,			    \
	  "Replenish and indexed free list with this number of chunks")	    \
                                                                            \
  product(bool, CMSLoopWarn, false,                                 	    \
          "Warn in case of excessive CMS looping")		            \
                                                                            \
  develop(bool, CMSOverflowEarlyRestoration, false,                    	    \
          "Whether preserved marks should be restored early")	            \
                                                                            \
  product(uintx, CMSMarkStackSize, 32*K,                                    \
          "Size of CMS marking stack")                                      \
									    \
  product(uintx, CMSMarkStackSizeMax, 4*M,                                  \
          "Max size of CMS marking stack")                                  \
									    \
  develop(bool, CMSMarkStackOverflowALot, false,                            \
          "Whether we should simulate frequent marking stack / work queue"  \
          " overflow")                                                      \
									    \
  develop(intx, CMSMarkStackOverflowInterval, 1000,                         \
          "A per-thread `interval' counter that determines how frequently"  \
          " we simulate overflow; a smaller number increases frequency")    \
									    \
  product(uintx, CMSMaxAbortablePrecleanLoops, 0,                           \
          "(Temporary, subject to experimentation)"                         \
          "Maximum number of abortable preclean iterations, if > 0")        \
									    \
  product(intx, CMSMaxAbortablePrecleanTime, 1000,                          \
          "(Temporary, subject to experimentation)"                         \
          "Maximum time in abortable preclean in ms")                       \
									    \
  product(uintx, CMSAbortablePrecleanMinWorkPerIteration, 100,              \
          "(Temporary, subject to experimentation)"                         \
          "Nominal minimum work per abortable preclean iteration")          \
									    \
  product(intx, CMSAbortablePrecleanWaitMillis, 50,                         \
          "(Temporary, subject to experimentation)"                         \
          " Time that we sleep between iterations when not given"           \
          " enough work per iteration")                                     \
									    \
  product(uintx, CMSRescanMultiple, 32,                                     \
          "Size of CMS parallel rescan tasks claimed sequentially")         \
									    \
  product(uintx, CMSRevisitStackSize, 1*M,                                  \
          "Size of CMS KlassKlass revisit stack")                           \
									    \
  product(bool, CMSParallelRemarkEnabled, true,                             \
          "Whether parallel remark enabled (only if ParNewGC)")             \
                                                                            \
  product(bool, CMSPermGenPrecleaningEnabled, false,                        \
          "Whether concurrent precleaning enabled in perm gen"              \
          " (effective only when CMSPrecleaningEnabled is true)")           \
                                                                            \
  product(bool, CMSPermGenSweepingEnabled, false,                           \
          "Whether sweeping of perm gen is enabled")                        \
                                                                            \
  product(bool, CMSPrecleaningEnabled, true,                                \
          "Whether concurrent precleaning enabled")                         \
                                                                            \
  product(uintx, CMSPrecleanIter, 3,                                        \
          "Maximum number of precleaning iteration passes")                 \
                                                                            \
  product(uintx, CMSPrecleanNumerator, 2,                                   \
          "CMSPrecleanNumerator:CMSPrecleanDenominator yields convergence"  \
          " ratio")                                                         \
                                                                            \
  product(uintx, CMSPrecleanDenominator, 3,                                 \
          "CMSPrecleanNumerator:CMSPrecleanDenominator yields convergence"  \
          " ratio")                                                         \
                                                                            \
  product(bool, CMSPrecleanRefLists1, true,                                 \
          "Preclean ref lists during (initial) preclean phase")             \
                                                                            \
  product(bool, CMSPrecleanRefLists2, false,                                \
          "Preclean ref lists during abortable preclean phase")             \
                                                                            \
  product(uintx, CMSPrecleanThreshold, 1000,                                \
          "Don't re-iterate if #dirty cards less than this")                \
									    \
  product(uintx, CMSScheduleRemarkEdenSizeThreshold, 2*M,                   \
          "If Eden used is below this value, don't try to schedule remark") \
                                                                            \
  product(uintx, CMSScheduleRemarkEdenPenetration, 50,             	    \
          "The Eden occupancy % at which to try and schedule remark pause") \
                                                                            \
  product(uintx, CMSScheduleRemarkSamplingRatio, 5,             	    \
          "Start sampling Eden top at least before yg occupancy reaches"    \
          " 1/<ratio> of the size at which we plan to schedule remark")     \
                                                                            \
  product(uintx, CMSSamplingGrain, 16*K,             	                    \
          "The minimum distance between eden samples for CMS (see above)")  \
                                                                            \
  develop(bool, CMSScavengeBeforeRemark, false,                        	    \
          "Attempt scavenge before the CMS remark step")		    \
                                                                            \
  develop(bool, CMSTraceSweeper, false,                               	    \
          "Trace some actions of the CMS sweeper")			    \
                                                                            \
  product(uintx, CMSWorkQueueDrainThreshold, 10,                            \
          "Don't drain below this size per parallel worker/thief")          \
									    \
  product(intx, CMSWaitDuration, 2000,                                      \
          "Time in milliseconds that CMS thread waits for young GC")        \
                                                                            \
  product(bool, CMSYield, true,                                             \
          "Yield between steps of concurrent mark & sweep")                 \
                                                                            \
  product(uintx, CMSBitMapYieldQuantum, 10*M,                               \
          "Bitmap operations should process at most this many bits"	    \
	  "between yields")						    \
                                                                            \
  develop(bool, FLSVerifyAllHeapReferences, false,                          \
          "Verify that all refs across the FLS boundary "                   \
          " are to valid objects")                                          \
                                                                            \
  develop(bool, FLSVerifyLists, false,                                      \
          "Do lots of (expensive) FreeListSpace verification")              \
                                                                            \
  develop(bool, FLSVerifyIndexTable, false,                                 \
          "Do lots of (expensive) FLS index table verification")            \
                                                                            \
  develop(bool, FLSVerifyDictionary, false,                                 \
          "Do lots of (expensive) FLS dictionary verification")             \
                                                                            \
  develop(bool, VerifyBlockOffsetArray, false,				    \
          "Do (expensive!) block offset array verification")		    \
                                                                            \
  product(bool, BlockOffsetArrayUseUnallocatedBlock, trueInDebug,           \
          "Maintain _unallocated_block in BlockOffsetArray"                 \
          " (currently applicable only to CMS collector)")       	    \
                                                                            \
  develop(bool, TraceCMSState, false,                                 	    \
          "Trace the state of the CMS collection")			    \
                                                                            \
  product(intx, RefDiscoveryPolicy, 0,                             	    \
          "Whether reference-based(0) or referent-based(1)")	            \
                                                                            \
  product(bool, ParallelRefProcEnabled, false,                        	    \
          "Enable parallel reference processing whenever possible")         \
                                                                            \
  product(intx, CMSTriggerRatio, 80,                                        \
          "Percentage of MinHeapFreeRatio in CMS generation that is "       \
          "  allocated before a CMS collection cycle commences")            \
                                                                            \
  product(intx, CMSBootstrapOccupancy, 50,                                  \
          "Percentage CMS generation occupancy at which to "                \
          " initiate CMS collection for bootstrapping collection stats")    \
                                                                            \
  product(intx, CMSInitiatingOccupancyFraction, -1,                         \
          "Percentage CMS generation occupancy to start a CMS collection "  \
          " cycle (A negative value means that CMSTirggerRatio is used)")   \
									    \
  product(bool, UseCMSInitiatingOccupancyOnly, false,			    \
	  "Only use occupancy as a crierion for starting a CMS collection") \
                                                                            \
  develop(bool, CMSTestInFreeList, false,                                   \
          "Check if the coalesced range is already in the "                 \
          "free lists as claimed.")                                         \
                                                                            \
  develop(bool, CMSVerifyReturnedBytes, false,                              \
          "Check that all the garbage collected was returned to the "	    \
          "free lists.")                                         	    \
                                                                            \
  develop(bool, ScavengeALot, false,                                        \
          "Force scavenge at every Nth exit from the runtime system "       \
          "(N=ScavengeALotInterval)")                                       \
                                                                            \
  develop(bool, FullGCALot, false,                                          \
          "Force full gc at every Nth exit from the runtime system "        \
          "(N=FullGCALotInterval)")                                         \
                                                                            \
  develop(bool, GCALotAtAllSafepoints, false,                               \
          "Enforce ScavengeALot/GCALot at all potential safepoints")        \
									    \
  product(bool, HandlePromotionFailure, false,                              \
          "The youngest generation collection does not require"             \
          " a guarantee of full promotion of all live objects.")            \
                                                                            \
  develop(bool, PromotionFailureALot, false,                                \
          "Use promotion failure handling on every youngest generation "    \
          "collection")                                                     \
                                                                            \
  develop(intx, PromotionFailureALotCount, 1000,                            \
          "Number of promotion failures occurring at ParGCAllocBuffer"      \
          "refill attempts (ParNew) or promotion attempts "		    \
	  "(other young collectors) ")                                      \
                                                                            \
  product(uintx, PreserveMarkStackSize, 40,				    \
	   "Size for stack used in promotion failure handling")		    \
                                                                            \
  product_pd(bool, UseTLAB, "Use thread-local object allocation")           \
                                                                            \
  product_pd(bool, ResizeTLAB,                                              \
          "Dynamically resize tlab size for threads")                       \
                                                                            \
  product(bool, ZeroTLAB, false,                                            \
          "Zero out the newly created TLAB")                                \
                                                                            \
  product(bool, PrintTLAB, false,                                           \
          "Print various TLAB related information")                         \
                                                                            \
  product(bool, TLABStats, true,                                            \
          "Print various TLAB related information")                         \
                                                                            \
  product_pd(bool, NeverActAsServerClassMachine,                            \
          "Never act like a server-class machine")                          \
                                                                            \
  product(bool, AlwaysActAsServerClassMachine, false,                       \
          "Always act like a server-class machine")                         \
                                                                            \
  product(uintx, DefaultMaxRAM, G,					    \
	  "Maximum real memory size for setting server class heap size")    \
									    \
  product(uintx, DefaultMaxRAMFraction, 4,				    \
	  "Fraction (1/n) of real memory used for server class max heap")   \
									    \
  product(uintx, DefaultInitialRAMFraction, 64,				    \
	  "Fraction (1/n) of real memory used for server class initial heap")  \
									    \
  product(bool, UseAdaptiveSizePolicy, true,                                \
          "Use adaptive generation sizing policies")                        \
                                                                            \
  product(bool, UsePSAdaptiveSurvivorSizePolicy, true,     		    \
          "Use adaptive survivor sizing policies")                          \
                                                                            \
  product(bool, UseAdaptiveGenerationSizePolicyAtMinorCollection, true,     \
          "Use adaptive young-old sizing policies at minor collections")    \
                                                                            \
  product(bool, UseAdaptiveGenerationSizePolicyAtMajorCollection, true,     \
          "Use adaptive young-old sizing policies at major collections")    \
                                                                            \
  product(bool, UseAdaptiveSizePolicyWithSystemGC, false,   		    \
          "Use statistics from System.GC for adaptive size policy")	    \
                                                                            \
  product(bool, UseAdaptiveGCBoundary, false,				    \
          "Allow young-old boundary to move")    			    \
                                                                            \
  develop(bool, TraceAdaptiveGCBoundary, false,				    \
          "Trace young-old boundary moves")    			            \
                                                                            \
  develop(intx, PSAdaptiveSizePolicyResizeVirtualSpaceAlot, -1,   	    \
          "Resize the virtual spaces of the young or old generations")      \
                                                                            \
  product(uintx, AdaptiveSizeThroughPutPolicy, 0,                           \
          "Policy for changeing generation size for throughput goals")	    \
                                                                            \
  develop(bool, PSAdjustTenuredGenForMinorPause, false,			    \
	  "Adjust tenured generation to achive a minor pause goal")	    \
                                                                            \
  develop(bool, PSAdjustYoungGenForMajorPause, false,			    \
	  "Adjust young generation to achive a major pause goal")	    \
									    \
  product(uintx, AdaptiveSizePolicyInitializingSteps, 20,                   \
          "Number of steps where heuristics is used before data is used")   \
									    \
  develop(uintx, AdaptiveSizePolicyReadyThreshold, 5,                       \
          "Number of collections before the adaptive sizing is started")    \
                                                                            \
  product(bool, UseAdaptiveSizePolicyFootprintGoal, true,                   \
          "Use adaptive minimum footprint as a goal")			    \
                                                                            \
  product(bool, UseAdaptiveSizePolicyAggressiveFootprintGoal, false,        \
          "Use adaptive minimum footprint as a goal per generation")        \
                                                                            \
  product(bool, UseAdaptiveSizeScaleFootprintChangeByRatio, true,           \
          "Scale the reduction of the generations by relative size")	    \
                                                                            \
  product(uintx, AdaptiveSizePolicyWeight, 10,                              \
          "Weight given to exponential resizing, between 0 and 100")        \
                                                                            \
  product(uintx, AdaptiveTimeWeight,       25,                              \
          "Weight given to time in adaptive policy, between 0 and 100")     \
                                                                            \
  product(uintx, PausePadding, 1,                                           \
          "How much buffer to keep for pause time")                  	    \
                                                                            \
  product(uintx, PromotedPadding, 3,                                        \
          "How much buffer to keep for promotion failure")                  \
                                                                            \
  product(uintx, SurvivorPadding, 3,                                        \
          "How much buffer to keep for survivor overflow")                  \
                                                                            \
  product(uintx, AdaptivePermSizeWeight, 20,                                \
          "Weight for perm gen exponential resizing, between 0 and 100")    \
                                                                            \
  product(uintx, PermGenPadding, 3,                                         \
          "How much buffer to keep for perm gen sizing")                    \
                                                                            \
  product(uintx, ThresholdTolerance, 10,                                    \
          "Allowed collection cost difference between generations")         \
                                                                            \
  product(uintx, AdaptiveSizePolicyCollectionCostMargin, 50,                \
          "If collection costs are within margin, reduce both by full delta") \
                                                                            \
  product(uintx, YoungGenerationSizeIncrement, 20,                          \
          "Adaptive size percentage change in young generation")            \
                                                                            \
  product(uintx, YoungGenerationSizeSupplement, 80,                         \
          "Supplement to YoungedGenerationSizeIncrement used at startup")   \
                                                                            \
  product(uintx, YoungGenerationSizeSupplementDecay, 8,                     \
          "Decay factor to YoungedGenerationSizeSupplement")    	    \
                                                                            \
  product(uintx, TenuredGenerationSizeIncrement, 20,                        \
          "Adaptive size percentage change in tenured generation")          \
                                                                            \
  product(uintx, TenuredGenerationSizeSupplement, 80,                       \
          "Supplement to TenuredGenerationSizeIncrement used at startup")   \
                                                                            \
  product(uintx, TenuredGenerationSizeSupplementDecay, 2,                   \
          "Decay factor to TenuredGenerationSizeIncrement")  		    \
                                                                            \
  product(uintx, MaxGCPauseMillis, max_uintx,                        	    \
          "Adaptive size policy maximum GC pause time goal in msec")        \
                                                                            \
  product(uintx, MaxGCMinorPauseMillis, max_uintx,                     	    \
          "Adaptive size policy maximum GC minor pause time goal in msec")  \
                                                                            \
  product(uintx, GCTimeRatio, 99,                     	                    \
          "Adaptive size policy application time to GC time ratio")         \
                                                                            \
  product(uintx, AdaptiveSizeDecrementScaleFactor, 4,                       \
          "Adaptive size scale down factor for shrinking")		    \
                                                                            \
  product(bool, UseAdaptiveSizeScaleThroughputChangeByRatio, true,          \
          "Adaptive size use ratio for major and minor throughput changes") \
                                                                            \
  product(bool, UseAdaptiveSizeDecayMajorGCCost, true,            	    \
          "Adaptive size decays the major cost for long major intervals")   \
                                                                            \
  product(uintx, AdaptiveSizeMajorGCDecayTimeScale, 10,           	    \
          "Time scale over which major costs decay")                        \
                                                                            \
  product(uintx, MinSurvivorRatio, 3,                                       \
          "Minimum ratio of young generation/survivor space size")          \
                                                                            \
  develop(intx, ConsecutiveSkippedScavengesBeforePromotion, 3,		    \
          "Number of consecutive skipped scavenges before eden promotion")  \
                                                                            \
  product(uintx, InitialSurvivorRatio, 8,                                   \
          "Initial ratio of eden/survivor space size")                      \
                                                                            \
  product(uintx, BaseFootPrintEstimate, 256*M,             	 	    \
          "Estimate of footprint other than Java Heap")                     \
                                                                            \
  product(bool, UseGCTimeLimit, true,                                       \
          "Use policy to limit of proportion of time spent in GC "	    \
	  "before an OutOfMemory error is thrown")                    	    \
                                                                            \
  product(uintx, GCTimeLimit, 98,                                           \
          "Limit of proportion of time spent in GC before an OutOfMemory"   \
          "error is thrown (used with GCHeapFreeLimit)")                    \
                                                                            \
  product(uintx, GCHeapFreeLimit, 2,                                        \
          "Minimum percentage of free space after a full GC before an "     \
          "OutOfMemoryError is thrown (used with GCTimeLimit)")             \
                                                                            \
  product(bool, PrintAdaptiveSizePolicy, false,                             \
          "Print information about AdaptiveSizePolicy")                     \
                                                                            \
  product_pd(intx, PrefetchCopyIntervalInBytes,                             \
          "How far ahead to prefetch destination area (<= 0 means off)")    \
                                                                            \
  product_pd(intx, PrefetchScanIntervalInBytes,                             \
          "How far ahead to prefetch scan area (<= 0 means off)")           \
                                                                            \
  product_pd(intx, PrefetchFieldsAhead,                                     \
          "How many fields ahead to prefetch in oop scan (<= 0 means off)") \
                                                                            \
  develop(bool, VerifyTLAB, false,                                          \
          "Enables verification of TLAB-related data structures")           \
                                                                            \
  develop(bool, VerifyBeforeExit, trueInDebug,                              \
          "Verify system before exiting")                                   \
                                                                            \
  develop(bool, VerifyBeforeGC, false,                                      \
          "Verify memory system before GC")                                 \
                                                                            \
  develop(bool, VerifyAfterGC, false,                                       \
          "Verify memory system after GC")                                  \
                                                                            \
  develop(bool, VerifyDuringGC, false,                                      \
          "Verify memory system during GC (between phases)")                \
                                                                            \
  develop(bool, VerifyRememberedSets, false,                                \
          "Verify GC remembered sets")                                      \
                                                                            \
  develop(bool, VerifyObjectStartArray, false,                              \
          "Verify GC object start array")                                   \
                                                                            \
  develop(bool, UseSpecialLargeObjectHandling, true,                        \
          "Tells whether the VM should handle large nonoop arrays "         \
          "specially for train GC")                                         \
                                                                            \
  product(bool, UseOversizedCarHandling, true,                              \
          "Use OversizedCarThreshold to handle large objects "              \
          "specially for train GC")                                         \
                                                                            \
  product(bool, DisableExplicitGC, false,                                   \
          "Tells whether calling System.gc() does a full GC")               \
                                                                            \
  develop(bool, CheckMemoryInitialization, false,                           \
          "Checks memory initialization")                                   \
                                                                            \
  product(bool, CollectGen0First, false,                                    \
          "Collect youngest generation before each full GC")                \
                                                                            \
  product(bool, BindGCTaskThreadsToCPUs, false,                             \
          "Bind GCTaskThreads to CPUs if possible")                         \
                                                                            \
  product(bool, UseGCTaskAffinity, false,                                   \
          "Use worker affinity when asking for GCTasks")                    \
                                                                            \
  product(uintx, ProcessDistributionStride, 4,                              \
          "Stride through processors when distributing processes")          \
                                                                            \
  /* gc tracing */                                                          \
  product(bool, PrintGC, false,                                             \
          "Print message at garbage collect")                               \
                                                                            \
  product(bool, PrintGCDetails, false,                                      \
          "Print more details at garbage collect")                          \
                                                                            \
  product(bool, PrintGCTimeStamps, false,                                   \
          "Print timestamps at garbage collect")                            \
                                                                            \
  product(bool, PrintGCTaskTimeStamps, false,                               \
          "Print timestamps for individual gc worker thread tasks")         \
                                                                            \
  develop(bool, TraceConcurrentMarkSweep, false,                            \
          "Trace concurrent mark sweep")                                    \
                                                                            \
  develop(bool, TraceMarkSweep, false,                                      \
          "Trace mark sweep")                                               \
                                                                            \
  develop(bool, PrintReferenceGC, false,                                    \
          "Print times spent handling reference objects during GC "         \
          " (enabled only when PrintGCDetails)")                            \
                                                                            \
  develop(bool, TraceReferenceGC, false,                                    \
          "Trace handling of soft/weak/final/phantom references")           \
                                                                            \
  develop(bool, TraceFinalizerRegistration, false,                          \
         "Trace registration of final references")                          \
                                                                            \
  develop(bool, TraceScavenge, false,                                       \
          "Trace scavenge")                                                 \
                                                                            \
  develop(bool, TraceSpecialLargeObjectHandling, false,                     \
          "Trace special handling of large objects")                        \
                                                                            \
  product(bool, TraceClassLoading, false,                                   \
          "Trace all classes loaded")                                       \
                                                                            \
  product(bool, TraceClassLoadingPreorder, false,                           \
          "Trace all classes loaded in order referenced (not loaded)")      \
                                                                            \
  product(bool, TraceClassUnloading, false,                                 \
          "Trace unloading of classes")                                     \
                                                                            \
  product(bool, TraceGen0Time, false,                                       \
          "Trace accumulated time for Gen 0 collection")                    \
                                                                            \
  product(bool, TraceGen1Time, false,                                       \
          "Trace accumulated time for Gen 1 collection")                    \
                                                                            \
  develop(bool, TraceCarAllocation, false,                                  \
          "Trace allocation of cars in train generation")                   \
                                                                            \
  product(bool, PrintTenuringDistribution, false,                           \
          "Print tenuring age information")                                 \
                                                                            \
  develop(bool, PrintHeapUsageOverTime, false,                              \
          "Print heap usage and capacity with timestamps")                  \
                                                                            \
  product(bool, PrintHeapAtGC, false,                                       \
          "Print heap layout before and after each GC")                     \
									    \
  product(bool, PrintClassHistogram, false,			            \
	  "Print a histogram of class instances") 		            \
                                                                            \
  develop(bool, TraceWorkGang, false,                                       \
          "Trace activities of work gangs")                                 \
                                                                            \
  develop(bool, TraceParallelMarkSweep, false,                              \
          "Trace multithreaded GC activity")                                \
                                                                            \
  develop(bool, TraceBlockOffsetTable, false,                               \
          "Print BlockOffsetTable maps")                                    \
                                                                            \
  develop(bool, TraceCardTableModRefBS, false,                              \
          "Print CardTableModRefBS maps")                                   \
                                                                            \
  develop(bool, TraceGCTaskManager, false,                                  \
          "Trace actions of the GC task manager")                           \
                                                                            \
  develop(bool, TraceGCTaskQueue, false,                                    \
          "Trace actions of the GC task queues")                            \
                                                                            \
  develop(bool, TraceGCTaskThread, false,                                   \
          "Trace actions of the GC task threads")                           \
                                                                            \
  develop(bool, IgnoreLibthreadGPFault, false,                              \
          "Suppress workaround for libthread GP fault")                     \
                                                                            \
  /* JVMTI heap profiling */						    \
									    \
  product(bool, TraceJVMTIObjectTagging, false,				    \
	  "Trace JVMTI object tagging calls")				    \
									    \
  product(bool, VerifyBeforeIteration, false,                               \
          "Verify memory system before JVMTI iteration")		    \
                                                                            \
  /* compiler interface */                                                  \
                                                                            \
  develop(bool, CIPrintCompilerName, false,                                 \
          "when CIPrint is active, print the name of the active compiler")  \
                                                                            \
  develop(bool, CIPrintCompileQueue, false,                                 \
          "display the contents of the compile queue whenever a "           \
          "compilation is enqueued")                                        \
                                                                            \
  develop(bool, CIPrintRequests, false,                                     \
          "display every request for compilation")                          \
                                                                            \
  product(bool, CITime, false,                                              \
          "collect timing information for compilation")                     \
                                                                            \
  develop(bool, CITimeEach, false,                                          \
          "display timing information after each successful compilation")   \
                                                                            \
  develop(bool, CICountOSR, true,                                           \
          "use a separate counter when assigning ids to osr compilations")  \
                                                                            \
  develop(bool, CICountNative, false,                                       \
          "use a separate counter when assigning ids to native "            \
          "compilations")                                                   \
                                                                            \
  develop(bool, CICompileNatives, true,                                     \
          "compile native methods if supported by the compiler")            \
                                                                            \
  develop_pd(bool, CICompileOSR,                                            \
          "compile on stack replacement methods if supported by the "       \
          "compiler")                                                       \
                                                                            \
  develop(bool, CIEagerAdapters, true,                                      \
          "compile adapters eagerly if the compiler needs adapters")        \
                                                                            \
  develop(bool, CICompileAdaptersInThread, false,                           \
          "compile adapters in the requesting thread")                      \
                                                                            \
  develop(bool, CIPrintMethodCodes, false,                                  \
          "print method bytecodes of the compiled code")                    \
                                                                            \
  develop(bool, CIPrintTypeFlow, false,                                     \
          "print the results of ciTypeFlow analysis")                       \
                                                                            \
  develop(bool, CITraceTypeFlow, false,                                     \
          "detailed per-bytecode tracing of ciTypeFlow analysis")           \
                                                                            \
  develop(intx, CICloneLoopTestLimit, 100,                                  \
          "size limit for blocks heuristically cloned in ciTypeFlow")       \
                                                                            \
  /* compiler */                                                            \
                                                                            \
  develop(bool, UseStackBanging, true,                                      \
          "use stack banging for stack overflow checks (required for "      \
          "proper StackOverflow handling; disable only to measure cost "    \
          "of stackbanging)")                                               \
                                                                            \
  develop(bool, Use24BitFPMode, true,                                       \
          "Set 24-bit FPU mode on a per-compile basis ")                    \
                                                                            \
  develop(bool, Use24BitFP, true,                                           \
          "use FP instructions that produce 24-bit precise results")        \
                                                                            \
  develop(bool, UseStrictFP, true,                                          \
          "use strict fp if modifier strictfp is set")                      \
                                                                            \
  develop(bool, GenerateSynchronizationCode, true,                          \
          "generate locking/unlocking code for synchronized methods and "   \
          "monitors")                                                       \
                                                                            \
  develop(bool, GenerateCompilerNullChecks, true,                           \
          "Generate explicit null checks for loads/stores/calls")           \
                                                                            \
  develop(bool, GenerateRangeChecks, true,                                  \
          "Generate range checks for array accesses")                       \
                                                                            \
  develop_pd(bool, ImplicitNullChecks,                                      \
          "generate code for implicit null checks")                         \
                                                                            \
  develop(bool, PrintSafepointStatistics, false,                            \
          "print statistics about safepoint synchronization")               \
                                                                            \
  develop(bool, PrintCHA, false,                                            \
          "print CHA information (clases and its subclasses")               \
                                                                            \
  develop(bool, InlineAccessors, true,                                      \
          "inline accessor methods (get/set)")                              \
                                                                            \
  product(bool, Inline, true,                                               \
          "enable inlining")                                                \
                                                                            \
  product(bool, ClipInlining, true,                                         \
          "clip inlining if aggregate method exceeds DesiredMethodLimit")   \
                                                                            \
  develop(bool, UseCHA, true,                                               \
          "enable CHA")                                                     \
                                                                            \
  product(bool, UseTypeProfile, true,                                       \
          "Check interpreter profile for historically monomorphic calls")   \
                                                                            \
  product(intx, TypeProfileMinimumRatio, 9,                                 \
          "Minimum ratio of profiled majority type to all minority types")  \
                                                                            \
  develop(bool, DeutschShiffmanALot, false,                                 \
          "do Deutsch-Shiffman type checks wherever UseTypeProfile might "  \
          "insert them")                                                    \
                                                                            \
  product(bool, InlineUnreachedCalls, true,                                 \
          "inline even those call sites which the interpreter has not yet " \
          "executed")                                                       \
                                                                            \
  develop(bool, TimeCompiler, false,                                        \
          "time the compiler")                                              \
                                                                            \
  develop(bool, TimeCompiler2, false,                                       \
          "detailed time the compiler (requires +TimeCompiler)")            \
                                                                            \
  develop(bool, PrintInlining, false,                                       \
          "prints inlining optimizations")                                  \
                                                                            \
  develop(bool, PrintMethodFlushing, false,                                 \
          "print the nmethods beeing flushed")                              \
                                                                            \
  develop(bool, LogMultipleMutexLocking, false,                             \
          "log locking and unlocking of mutexes (only if multiple locks "   \
          "are held)")                                                      \
                                                                            \
  develop(bool, UseRelocIndex, false,                                       \
         "use an index to speed random access to relocations")              \
                                                                            \
  product(bool, PrintVMOptions, trueInDebug,                                \
         "print VM flag settings")                                          \
                                                                            \
  diagnostic(bool, SerializeVMOutput, true,                                 \
         "Use a mutex to serialize output to tty and hotspot.log")          \
                                                                            \
  diagnostic(bool, DisplayVMOutput, true,                                   \
         "Display all VM output on the tty, independently of LogVMOutput")  \
                                                                            \
  diagnostic(bool, LogVMOutput, trueInDebug,                                \
         "Save VM output to hotspot.log, or to LogFile")                    \
                                                                            \
  diagnostic(ccstr, LogFile, "",                                            \
         "If LogVMOutput is on, save VM output to this file [hotspot.log]") \
                                                                            \
  product(bool, DisplayVMOutputToStderr, false,                             \
         "If DisplayVMOutput is true, display all VM output to stderr")     \
                                                                            \
  product(bool, DisplayVMOutputToStdout, false,                             \
         "If DisplayVMOutput is true, display all VM output to stdout")     \
                                                                            \
  product(bool, UseHeavyMonitors, false,                                    \
          "use heavyweight instead of lightweight Java monitors")           \
                                                                            \
  develop(bool, PrintSymbolTableSizeHistogram, false,                       \
          "print histogram of the symbol table")                            \
                                                                            \
  develop(bool, PrintScopeTree, false,                                      \
          "print scope tree")                                               \
                                                                            \
  develop(bool, ExitVMOnVerifyError, false,                                 \
          "standard exit from VM if bytecode verify error "                 \
          "(only in debug mode)")                                           \
                                                                            \
  develop(ccstr, AbortVMOnException, "",                                    \
          "Call fatal if this exception is thrown.  Example: "              \
          "java -XX:AbortVMOnException=java.lang.NullPointerException Foo") \
                                                                            \
  develop(bool, DebugVtables, false,                                        \
          "add debugging code to vtable dispatch")                          \
                                                                            \
  develop(bool, PrintVtables, false,                                        \
          "print vtables when printing klass")                              \
                                                                            \
  develop(bool, PrintVtableStats, false,                                    \
          "print vtables stats at end of run")                              \
                                                                            \
  develop(bool, TraceCreateZombies, false,                                  \
          "trace creation of zombie nmethods")                              \
                                                                            \
  develop(bool, IgnoreLockingAssertions, false,                             \
          "disable locking assertions (for speed)")                         \
                                                                            \
  develop(bool, VerifyLoopOptimizations, false,                             \
          "verify major loop optimizations")                                \
                                                                            \
  develop(bool, RangeCheckElimination, true,                                \
          "Split loop iterations to eliminate range checks")                \
                                                                            \
  develop_pd(bool, UncommonNullCast,                                        \
          "Uncommon-trap NULLs passed to check cast")                       \
                                                                            \
  develop(bool, MonomorphicArrayCheck, true,                                \
          "Uncommon-trap array store checks that require full type check")  \
                                                                            \
  develop(bool, DelayCompilationDuringStartup, true,                        \
          "Delay invoking the compiler until main application class is "    \
          "loaded")                                                         \
                                                                            \
  develop(bool, CompileTheWorld, false,                                     \
          "Compile all methods in all classes in bootstrap class path "     \
          "(stress test)")                                                  \
                                                                            \
  develop(bool, CompileTheWorldPreloadClasses, true,                        \
          "Preload all classes used by a class before start loading")       \
                                                                            \
  develop(bool, CompileTheWorldIgnoreInitErrors, false,                     \
          "Compile all methods although class initializer failed")          \
                                                                            \
  develop(bool, TraceIterativeGVN, false,                                   \
          "Print progress during Iterative Global Value Numbering")         \
                                                                            \
  develop(bool, FillDelaySlots, true,                                       \
          "Fill delay slots (on SPARC only)")                               \
                                                                            \
  develop(bool, VerifyIterativeGVN, false,                                  \
          "Verify Def-Use modifications during sparse Iterative Global "    \
          "Value Numbering")                                                \
                                                                            \
  develop(bool, TracePhaseCCP, false,                                       \
          "Print progress during Conditional Constant Propagation")         \
                                                                            \
  develop(bool, TimeLivenessAnalysis, false,                                \
          "Time computation of bytecode liveness analysis")                 \
                                                                            \
  develop(bool, TraceLivenessGen, false,                                    \
          "Trace the generation of liveness analysis information")          \
                                                                            \
  develop(bool, TraceLivenessQuery, false,                                  \
          "Trace queries of liveness analysis information")                 \
                                                                            \
  develop(bool, CollectIndexSetStatistics, false,                           \
          "Collect information about IndexSets")                            \
                                                                            \
  develop(bool, PrintDominators, false,                                     \
          "Print out dominator trees for GVN")                              \
                                                                            \
  develop(bool, UseLoopSafepoints, true,                                    \
          "Generate Safepoint nodes in every loop")                         \
                                                                            \
  develop(bool, TraceCISCSpill, false,                                      \
          "Trace allocators use of cisc spillable instructions")            \
                                                                            \
  develop(bool, TraceSpilling, false,                                       \
          "Trace spilling")                                                 \
                                                                            \
  develop(bool, DeutschShiffmanExceptions, true,                            \
          "Fast check to find exception handler for precisely typed "       \
          "exceptions")                                                     \
                                                                            \
  develop(bool, SplitIfBlocks, true,                                        \
          "Clone compares and control flow through merge points to fold "   \
          "some branches")                                                  \
                                                                            \
  develop(intx, FastAllocateSizeLimit, 100000,                              \
          "Inline allocations larger than this in doublewords must go slow")\
                                                                            \
  /* statistics */                                                          \
  develop(bool, UseVTune, false,                                            \
          "enable support for Intel's VTune profiler")                      \
                                                                            \
  develop(bool, CountCompiledCalls, false,                                  \
          "counts method invocations")                                      \
                                                                            \
  develop(bool, CountJNICalls, false,                                       \
          "counts jni method invocations")                                  \
                                                                            \
  develop(bool, CountJVMCalls, false,                                       \
          "counts jvm method invocations")                                  \
                                                                            \
  develop(bool, CountRemovableExceptions, false,                            \
          "count exceptions that could be replaced by branches due to "     \
          "inlining")                                                       \
                                                                            \
  develop(bool, ICMissHistogram, false,                                     \
          "produce histogram of IC misses")                                 \
                                                                            \
  develop(bool, PrintClassStatistics, false,                                \
          "prints class statistics at end of run")                          \
                                                                            \
  develop(bool, PrintMethodStatistics, false,                               \
          "prints method statistics at end of run")                         \
                                                                            \
  develop(bool, PrintTrainGCProcessingStats, false,                         \
          "print stats on train gc processing")                             \
                                                                            \
  /* interpreter */                                                         \
  develop(bool, ClearInterpreterLocals, false,                              \
          "Always clear local variables of interpreter activations upon "   \
          "entry")                                                          \
                                                                            \
  product_pd(bool, RewriteBytecodes,                                        \
          "Allow rewriting of bytecodes (bytecodes are not immutable)")     \
                                                                            \
  product_pd(bool, RewriteFrequentPairs,                                    \
          "Rewrite frequently used bytecode pairs into a single bytecode")  \
                                                                            \
  develop(bool, PrintInterpreter, false,                                    \
          "Prints the generated interpreter code")                          \
                                                                            \
  product(bool, UseInterpreter, true,                                       \
          "Use interpreter for non-compiled methods")                       \
                                                                            \
  develop(bool, UseFastSignatureHandlers, true,                             \
          "Use fast signature handlers for native calls")                   \
                                                                            \
  develop(bool, UseV8InstrsOnly, false,                                     \
          "Use SPARC-V8 Compliant instruction subset")                      \
                                                                            \
  develop(bool, UseCASForSwap, false,                                       \
          "Do not use swap instructions, but only CAS (in a loop) on SPARC")\
                                                                            \
  product(bool, UseLoopCounter, true,                                       \
          "Increment invocation counter on backward branch")                \
                                                                            \
  product(bool, UseFastEmptyMethods, true,                                  \
          "Use fast method entry code for empty methods")                   \
                                                                            \
  product(bool, UseFastAccessorMethods, true,                               \
          "Use fast method entry code for accessor methods")                \
                                                                            \
  product_pd(bool, UseOnStackReplacement,                                   \
           "Use on stack replacement, calls runtime if invoc. counter "     \
           "overflows in loop")                                             \
                                                                            \
  develop(bool, TraceOnStackReplacement, false,                             \
          "Trace on stack replacement")                                     \
                                                                            \
  develop(bool, PoisonOSREntry, true,                                       \
           "Detect abnormal calls to OSR code")                             \
                                                                            \
  product(bool, PreferInterpreterNativeStubs, true,                         \
          "Use always interpreter stubs for native methods invoked via "    \
          "interpreter")                                                    \
                                                                            \
  develop(bool, CountBytecodes, false,                                      \
          "Count number of bytecodes executed")                             \
                                                                            \
  develop(bool, PrintBytecodeHistogram, false,                              \
          "Print histogram of the executed bytecodes")                      \
                                                                            \
  develop(bool, PrintBytecodePairHistogram, false,                          \
          "Print histogram of the executed bytecode pairs")                 \
                                                                            \
  develop(bool, PrintSignatureHandlers, false,                              \
          "Print code generated for native method signature handlers")      \
                                                                            \
  develop(bool, VerifyOops, false,                                          \
          "Do plausibility checks for oops")                                \
                                                                            \
  develop(bool, VerifyJNIFields, trueInDebug,                               \
          "Verify jfieldIDs for instance fields")                           \
                                                                            \
  develop(bool, VerifyJNIEnvThread, false,                                  \
          "Verify JNIEnv.thread == Thread::current() when entering VM "     \
          "from JNI")                                                       \
                                                                            \
  develop(bool, VerifyFPU, false,                                           \
          "Verify FPU state (check for NaN's, etc.)")                       \
                                                                            \
  develop(bool, VerifyThread, false,                                        \
          "Watch the thread register for corruption (SPARC only)")          \
                                                                            \
  develop(bool, VerifyActivationFrameSize, false,                           \
          "Verify that activation frame didn't become smaller than its "    \
          "minimal size")                                                   \
                                                                            \
  develop(bool, VerifyReceiver, false,                                      \
          "Verify that receiver is a subclass of method holder")            \
                                                                            \
  product(bool, EnableJVMPIInstructionStartEvent, false,                    \
          "Enable JVMPI_EVENT_INSTRUCTION_START events - slows down "       \
          "interpretation")                                                 \
                                                                            \
  product(bool, JVMPICheckGCCompatibility, true,                            \
          "If JVMPI is used, make sure that we are using a "                \
          " JVMPI-compatible garbage collector ")                           \
                                                                            \
  develop(bool, TraceFrequencyInlining, false,                              \
          "Trace frequency based inlining")                                 \
                                                                            \
  develop(bool, TraceTypeProfile, false,                                    \
          "Trace type profile")                                             \
                                                                            \
  develop_pd(bool, InlineIntrinsics,                                        \
           "Inline intrinsics that can be statically resolved")             \
                                                                            \
  develop_pd(bool, ProfileInterpreter,                                      \
           "Profile at the bytecode level during interpretation")           \
                                                                            \
  develop_pd(bool, ProfileTraps,                                            \
          "Profile deoptimization traps at the bytecode level")             \
                                                                            \
  product(intx, ProfileMaturityPercentage, 20,                              \
          "number of method invocations/branches (expressed as % of "       \
          "CompileThreshold) before using the method's profile")            \
                                                                            \
  develop(bool, PrintMethodData, false,                                     \
           "Print the results of +ProfileInterpreter at end of run")        \
                                                                            \
  develop(bool, VerifyDataPointer, trueInDebug,                             \
          "Verify the method data pointer during interpreter profiling")    \
                                                                            \
                                                                            \
  /* compilation */                                                         \
  product(bool, UseCompiler, true,                                          \
          "use compilation")                                                \
                                                                            \
  develop(bool, TraceCompilationPolicy, false,                              \
          "Trace compilation policy")                                       \
                                                                            \
  develop(bool, TimeCompilationPolicy, false,                               \
          "Time the compilation policy")                                    \
                                                                            \
  develop(bool, UseCounterDecay, true,                                      \
           "adjust recompilation counters")                                 \
                                                                            \
  develop(bool, UseAdaptiveThresholds, false,                               \
          "use adaptive recompilation thresholds")                          \
                                                                            \
  product(bool, AlwaysCompileLoopMethods, false,                            \
          "when using recompilation, never interpret methods "              \
          "containing loops")                                               \
                                                                            \
  develop(bool, PrintRecompilation, false,                                  \
          "print one-line info for each recompilation")                     \
                                                                            \
  develop(bool, PrintRecompilation2, false,                                 \
          "print extensive info for each recompilation")                    \
                                                                            \
  develop(bool, PrintRecompilationMonitoring, false,                        \
          "print info about recompilation threshold adjusting")             \
                                                                            \
  product(bool, DontCompileHugeMethods, true,                               \
          "don't compile methods > HugeMethodLimit")                        \
                                                                            \
  /* deoptimization */                                                      \
  develop(bool, TraceDeoptimization, false,                                 \
          "Trace deoptimization")                                           \
                                                                            \
  develop(bool, DebugDeoptimization, false,                                 \
          "Tracing various information while debugging deoptimization")     \
                                                                            \
  product(intx, SelfDestructTimer, 0,                                       \
          "Will cause VM to terminate after a given time (in minutes) "     \
          "(0 means off)")                                                  \
                                                                            \
  product(intx, MaxJavaStackTraceDepth, 1024,                               \
          "Max. no. of lines in the stack trace for Java exceptions "       \
          "(0 means all)")                                                  \
                                                                            \
  develop(intx, GuaranteedSafepointInterval, 1000,                          \
          "Guarantee a safepoint (at least) every so many milliseconds "    \
          "(0 means none)")                                                 \
                                                                            \
  develop(intx, SafepointTimeoutDelay, 10000,                               \
          "Delay in milliseconds for option SafepointTimeout")              \
                                                                            \
  develop(intx, MaxICLEStubsBeforeSafepoint, 1000,                          \
          "Force a safepoint after allocating this many "                   \
          "InterfaceLookupCache entries ")                                  \
                                                                            \
  product(intx, NmethodSweepFraction, 4,                                    \
          "Number of invocations of sweeper to cover all nmethods")         \
                                                                            \
  develop(intx, MemProfilingInterval, 500,                                  \
          "Time between each invocation of the MemProfiler")                \
                                                                            \
  develop(intx, MallocCatchPtr, -1,                                         \
          "Hit breakpoint when mallocing/freeing this pointer")             \
                                                                            \
  develop(intx, PostMortemDump,falseInDebug,                                \
          "Dump info upon unhandled trap; "                                 \
          "1 = dump some, 2 = dump more (unsafer)")                         \
                                                                            \
  develop(intx, AssertRepeat, 1,                                            \
          "number of times to evaluate expression in assert "               \
          "(to estimate overhead); only works with -DUSE_REPEATED_ASSERTS") \
                                                                            \
  develop(ccstr, SuppressErrorAt, "",                                       \
          "List of assertions (file:line) to muzzle")                       \
                                                                            \
  develop(uintx, HandleAllocationLimit, 1024,                                \
          "Threshold for HandleMark allocation when +TraceHandleAllocation "\
          "is used")                                                        \
                                                                            \
  develop(uintx, TotalHandleAllocationLimit, 1024,                           \
          "Threshold for total handle allocation when "                     \
          "+TraceHandleAllocation is used")                                 \
                                                                            \
  develop(intx, StackPrintLimit, 100,                                       \
          "number of stack frames to print in VM-level stack dump")         \
                                                                            \
  develop(intx, MaxElementPrintSize, 256,                                   \
          "maximum number of elements to print")                            \
                                                                            \
  develop(intx, MaxSubklassPrintSize, 4,                                    \
          "maximum number of subklasses to print when printing klass")      \
                                                                            \
  develop(intx, MaxInlineLevel, 9,                                          \
          "maximum number of nested calls that are inlined")                \
                                                                            \
  develop(intx, MaxRecursiveInlineLevel, 1,                                 \
          "maximum number of nested recursive calls that are inlined")      \
                                                                            \
  develop(intx, InlineSmallCode, 1000,                                      \
          "Only inline already compiled methods if their code size is "     \
          "less than this")                                                 \
                                                                            \
  product(intx, MaxInlineSize, 35,                                          \
          "maximum bytecode size of a method to be inlined")                \
                                                                            \
  product_pd(intx, FreqInlineSize,                                          \
          "maximum bytecode size of a frequent method to be inlined")       \
                                                                            \
  develop(intx, MaxTrivialSize, 6,                                          \
          "maximum bytecode size of a trivial method to be inlined")        \
                                                                            \
  develop(intx, MinInliningThreshold, 250,                                  \
          "min. invocation count a method needs to have to be inlined")     \
                                                                            \
  develop(intx, AlignEntryCode, 4,                                          \
          "aligns entry code to specified value (in bytes)")                \
                                                                            \
  develop(intx, MaxNMethodSize, 8*K,                                        \
          "desired max. nmethod size")                                      \
                                                                            \
  develop(intx, MethodHistogramCutoff, 100,                                 \
          "cutoff value for method invoc. histogram (+CountCalls)")         \
                                                                            \
  develop(intx, ProfilerNumberOfInterpretedMethods, 25,                     \
          "# of interpreted methods to show in profile")                    \
                                                                            \
  develop(intx, ProfilerNumberOfCompiledMethods, 25,                        \
          "# of compiled methods to show in profile")                       \
                                                                            \
  develop(intx, ProfilerNumberOfStubMethods, 25,                            \
          "# of stub methods to show in profile")                           \
                                                                            \
  develop(intx, ProfilerNumberOfRuntimeStubNodes, 25,                       \
          "# of runtime stub nodes to show in profile")                     \
                                                                            \
  product(intx, ProfileIntervalsTicks, 100,                                 \
          "# of ticks between printing of interval profile "                \
          "(+ProfileIntervals)")                                            \
                                                                            \
  develop(intx, ScavengeALotInterval,     1,                                \
          "Interval between which scavenge will occur with +ScavengeALot")  \
                                                                            \
  develop(intx, FullGCALotInterval,     1,                                  \
          "Interval between which full gc will occur with +FullGCALot")     \
                                                                            \
  develop(intx, FullGCALotStart,     0,                                     \
          "For which invocation to start FullGCAlot")                       \
                                                                            \
  develop(intx, FullGCALotDummies,  32*K,                                   \
          "Dummy object allocated with +FullGCALot, forcing all objects "   \
          "to move")                                                        \
                                                                            \
  develop(intx, DontYieldALotInterval,    10,                               \
          "Interval between which yields will be dropped (milliseconds)")   \
                                                                            \
  develop(intx, MinSleepInterval,     1,                                    \
          "Minimum sleep() interval (milliseconds) when "                   \
          "ConvertSleepToYield is off (used for SOLARIS)")                  \
                                                                            \
  product(intx, EventLogLength,  2000,                                      \
          "maximum nof events in event log")                                \
                                                                            \
  develop(intx, ProfilerPCTickThreshold,    15,                             \
          "Number of ticks in a PC buckets to be a hotspot")                \
                                                                            \
  develop(intx, DeoptimizeALotInterval,     5,                              \
          "Number of exits until DeoptimizeALot kicks in")                  \
                                                                            \
  develop(intx, ZombieALotInterval,     5,                                  \
          "Number of exits until ZombieALot kicks in")                      \
                                                                            \
  develop(intx, MallocVerifyInterval,     0,                                \
          "if non-zero, verify C heap after every N calls to "              \
          "malloc/realloc/free")                                            \
                                                                            \
  develop(intx, MallocVerifyStart,     0,                                   \
          "if non-zero, start verifying C heap after Nth call to "          \
          "malloc/realloc/free")                                            \
                                                                            \
  develop(intx, AllocateLargeLiveRangePolicy,    10,                        \
          "Do not color (share stack slots) live ranges that are bigger "   \
          "than this")                                                      \
                                                                            \
  develop(intx, BranchFreqTableSize,    10,                                 \
          "Size of branch frequency counter table is 2 to the n power")     \
                                                                            \
  develop(intx, CallFreqTableSize,    10,                                   \
          "Size of call frequency counter table is 2 to the n power")       \
                                                                            \
  develop(intx, TypeProfileWidth,      2,                                   \
          "number of receiver types to record in call profile")             \
                                                                            \
  develop(intx, BciProfileWidth,      2,                                    \
          "number of return bci's to record in ret profile")                \
                                                                            \
  develop(intx, CallProfileSize,  1024,                                     \
          "Size of call profile table (must be a power of 2)")              \
                                                                            \
  product(intx, PerMethodRecompilationCutoff, 400,                          \
          "After recompiling N times, stay in the interpreter (-1=>'Inf')") \
                                                                            \
  product(intx, PerBytecodeRecompilationCutoff, 100,                        \
          "Per-BCI limit on repeated recompilation (-1=>'Inf')")            \
                                                                            \
  product(intx, PerMethodTrapLimit,  100,                                   \
          "Limit on traps (of one kind) in a method (includes inlines)")    \
                                                                            \
  product(intx, PerBytecodeTrapLimit,  4,                                   \
          "Limit on traps (of one kind) at a particular BCI")               \
                                                                            \
  develop(intx, FreqCountInvocations,  1500,                                \
          "Number of method invocations that will collect branch frequency "\
          "counts")                                                         \
                                                                            \
  develop(intx, InlineFrequencyRatio,    20,                                \
          "Ratio of call site execution to caller method invocation")       \
                                                                            \
  develop_pd(intx, InlineFrequencyCount,                                    \
          "Count of call site execution necessary to trigger frequent "     \
          "inlining")                                                       \
                                                                            \
  develop(intx, InlineThrowCount,    50,                                    \
          "Force inlining of interpreted methods that throw this often")    \
                                                                            \
  develop(intx, InlineThrowMaxSize,   200,                                  \
          "Force inlining of throwing methods smaller than this")           \
                                                                            \
  product(intx, AliasLevel,     2,                                          \
          "0 for no aliasing, 1 for oop/field/static/array split, "         \
          "2 for best")                                                     \
                                                                            \
  develop(bool, VerifyAliases, false,                                       \
          "perform extra checks on the results of alias analysis")          \
                                                                            \
  develop(intx, ProfilerNodeSize,  1024,                                    \
          "Size in K to allocate for the Profile Nodes of each thread")     \
                                                                            \
  develop(intx, V8AtomicOperationUnderLockSpinCount,    50,                 \
          "Number of times to spin wait on a v8 atomic operation lock")     \
                                                                            \
  product(intx, ReadSpinIterations,   100,                                  \
          "Number of read attempts before a yield (spin inner loop)")       \
                                                                            \
  product_pd(intx, PreInflateSpin,                                          \
          "Number of times to spin wait before inflation")                  \
                                                                            \
  product(intx, PreBlockSpin,    10,                                        \
          "Number of times to spin in an inflated lock before going to "    \
          "an OS lock")                                                     \
                                                                            \
  /* gc parameters */                                                       \
  product(uintx, MaxHeapSize, ScaleForWordSize(64*M),                       \
          "Default maximum size for object heap (in bytes)")                \
                                                                            \
  product_pd(uintx, NewSize, 						    \
          "Default size of new generation (in bytes)")                      \
                                                                            \
  product(uintx, MaxNewSize, max_uintx,                             	    \
          "Maximum size of new generation (in bytes)")                      \
                                                                            \
  product(uintx, PretenureSizeThreshold, 0,                                 \
          "Max size in bytes of objects allocated in DefNew generation")    \
                                                                            \
  product_pd(uintx, TLABSize,                                               \
          "Default (or starting) size of TLAB (in bytes)")                  \
                                                                            \
  product(uintx, MinTLABSize, 2*K,                        		    \
          "Minimum allowed TLAB size (in bytes)")                           \
                                                                            \
  product(uintx, TLABAllocationWeight, 35,                                  \
          "Allocation averaging weight")                                    \
                                                                            \
  product(uintx, TLABWasteTargetPercent, 1,                                 \
          "Percentage of Eden that can be wasted")                          \
                                                                            \
  product(uintx, TLABRefillWasteFraction,    64,                            \
          "Max TLAB waste at a refill (internal fragmentation)")            \
                                                                            \
  product(uintx, TLABWasteIncrement,    4,                                  \
          "Increment allowed waste at slow allocation")                     \
                                                                            \
  product_pd(intx, SurvivorRatio,                                           \
          "Ratio of eden/survivor space size")                              \
                                                                            \
  product_pd(intx, NewRatio,                                                \
          "Ratio of new/old generation sizes")                              \
                                                                            \
  product(uintx, MaxLiveObjectEvacuationRatio, 100,                         \
          "Max percent of eden objects that will be live at scavenge")      \
                                                                            \
  product_pd(uintx, NewSizeThreadIncrease,                                  \
          "Additional size added to desired new generation size per "       \
          "non-daemon thread (in bytes)")                                   \
                                                                            \
  product(uintx, OldSize, ScaleForWordSize(1408*K),                   	    \
          "Default size of tenured generation (in bytes)")                  \
                                                                            \
  product_pd(uintx, PermSize,                                               \
          "Default size of permanent generation (in bytes)")                \
                                                                            \
  product_pd(uintx, MaxPermSize,					    \
          "Maximum size of permanent generation (in bytes)")                \
                                                                            \
  product(uintx, MinHeapFreeRatio,    40,                                   \
          "Min percentage of heap free after GC to avoid expansion")        \
                                                                            \
  product(uintx, MaxHeapFreeRatio,    70,                                   \
          "Max percentage of heap free after GC to avoid shrinking")        \
                                                                            \
  product(intx, SoftRefLRUPolicyMSPerMB, 1000,                              \
          "Number of milliseconds per MB of free space in the heap")        \
                                                                            \
  product(uintx, MinHeapDeltaBytes, ScaleForWordSize(128*K),                \
          "Min change in heap space due to GC (in bytes)")                  \
                                                                            \
  product(uintx, MinPermHeapExpansion, ScaleForWordSize(256*K),             \
          "Min expansion of permanent heap (in bytes)")                     \
                                                                            \
  product(uintx, MaxPermHeapExpansion, ScaleForWordSize(4*M),               \
          "Max expansion of permanent heap without full GC (in bytes)")     \
                                                                            \
  develop(intx, LogOfCarSpaceSize,    16,                                   \
          "Size of car spaces in trains (2^n bytes)")                       \
                                                                            \
  product(uintx, OversizedCarThreshold, 4,                                  \
          "Size of car to be considered oversized, in CarSize units")       \
                                                                            \
  product(intx, MinTickInterval,    -2,                                     \
          "Minimum train tick interval")                                    \
                                                                            \
  product(intx, DefaultTickInterval,     1,                                 \
          "Default train tick interval")                                    \
                                                                            \
  product(intx, MaxTickInterval,     3,                                     \
          "Maximum train tick interval")                                    \
                                                                            \
  product(intx, DelayTickAdjustment,    10,                                 \
          "Delay before adaptively adjusting tick rate after startup/full " \
          "gc")                                                             \
                                                                            \
  product(intx, ProcessingToTenuringRatio,     6,                           \
          "Adaptive ratio of processed objects to tenured objects")         \
                                                                            \
  product(intx, QueuedAllocationWarningCount, 0,                            \
          "Number of times an allocation that queues behind a GC "          \
          "will retry before printing a warning")                           \
                                                                            \
  develop(intx, MinTrainLength,     4,                                      \
          "Minimum length of (last) train before creating new train")       \
                                                                            \
  develop(uintx, VerifyGCStartAt,     0,                                    \
          "GC invoke count where +VerifyBefore/AfterGC kicks in")           \
                                                                            \
  develop(intx, VerifyGCLevel,     0,                                       \
          "Generation level at which to start +VerifyBefore/AfterGC")       \
                                                                            \
  develop(uintx, ExitAfterGCNum,   0,                                       \
          "If non-zero, exit after this GC.")	                            \
                                                                            \
  product(intx, MaxTenuringThreshold,    31,                                \
          "Maximum value for tenuring threshold")                           \
                                                                            \
  product(intx, InitialTenuringThreshold,    15,                            \
          "Initial value for tenuring threshold")                           \
                                                                            \
  product(intx, TargetSurvivorRatio,    50,                                 \
          "Desired percentage of survivor space used after scavenge")       \
                                                                            \
  product(intx, MarkSweepDeadRatio,     5,                                  \
          "Percentage dead space in the tenured space before a compaction " \
          "kicks in")                                                       \
                                                                            \
  product(intx, PermMarkSweepDeadRatio,    20,                              \
          "Percentage dead space in the perm space before a compaction "    \
          "kicks in")                                                       \
                                                                            \
  product(intx, MarkSweepAlwaysCompactCount,     4,                         \
          "How often should we fully compact the heap (ignoring the dead "  \
          "space parameters)")                                              \
                                                                            \
  product(intx, PrintCMSStatistics, 0,                                      \
          "Statistics for CMS")                                             \
                                                                            \
  develop(bool, PrintCMSInitiationStatistics, false,                        \
          "Statistics for initiating a CMS collection")                     \
                                                                            \
  product(intx, PrintFLSStatistics, 0,                                      \
          "Statistics for CMS' FreeListSpace")                              \
                                                                            \
  product(intx, PrintFLSCensus, 0,                                          \
          "Census for CMS' FreeListSpace")                                  \
                                                                            \
  product(intx, DeferThrSuspendLoopCount,     0,                            \
          "Number of times to loop safepoint loop before suspending "       \
          "threads")                                                        \
                                                                            \
  /* stack parameters */                                                    \
  product_pd(intx, StackYellowPages,                                        \
          "Number of yellow zone (recoverable overflows) pages")            \
                                                                            \
  product_pd(intx, StackRedPages,                                           \
          "Number of red zone (unrecoverable overflows) pages")             \
                                                                            \
  product_pd(intx, StackShadowPages,                                        \
          "Number of shadow zone (for overflow checking) pages"             \
          " this should exceed the depth of the VM and native call stack")  \
                                                                            \
  product_pd(intx, ThreadStackSize,                                         \
          "Thread Stack Size (in Kbytes)")                                  \
                                                                            \
  product_pd(intx, VMThreadStackSize,                                       \
          "Non-Java Thread Stack Size (in Kbytes)")                         \
                                                                            \
  product_pd(intx, CompilerThreadStackSize,                                 \
          "Compiler Thread Stack Size (in Kbytes)")                         \
                                                                            \
  develop_pd(uintx, JVMInvokeMethodSlack,                                   \
          "Stack space (bytes) required for JVM_InvokeMethod to complete")  \
                                                                            \
  product(intx, ThreadSafetyMargin, 50*M,                                   \
          "Thread safety margin is used on fixed-stack LinuxThreads (on "   \
          "Linux/x86 only) to prevent heap-stack collision. Set to 0 to "   \
          "disable this feature")                                           \
                                                                            \
  /* code cache parameters */                                               \
  develop(intx, CodeCacheSegmentSize, 64,                                   \
          "Code cache segment size (in bytes) - smallest unit of "          \
          "allocation")                                                     \
                                                                            \
  develop_pd(intx, CodeEntryAlignment,                                      \
          "Code entry alignment for generated code (in bytes)")             \
                                                                            \
  product_pd(intx, InitialCodeCacheSize, 			            \
          "Initial code cache size (in bytes)")                             \
                                                                            \
  product_pd(intx, ReservedCodeCacheSize,                                   \
          "Reserved code cache size (in bytes) - maximum code cache size")  \
                                                                            \
  product(uintx, CodeCacheMinimumFreeSpace, 500*K,                          \
          "When less than X space left, we stop compiling.")                \
                                                                            \
  product_pd(intx, CodeCacheExpansionSize,                                  \
          "Code cache expansion size (in bytes)")                           \
                                                                            \
  develop_pd(uintx, CodeCacheMinBlockLength,                                \
          "Minimum number of segments in a code cache block.")              \
                                                                            \
  develop(bool, ExitOnFullCodeCache, false,                                 \
          "Exit the VM if we fill the code cache.")                         \
                                                                            \
  /* interpreter debugging */                                               \
  develop(intx, BinarySwitchThreshold, 5,                                   \
          "Minimal number of lookupswitch entries for rewriting to binary " \
          "switch")                                                         \
                                                                            \
  develop(intx, StopInterpreterAt, 0,                                       \
          "Stops interpreter execution at specified bytecode number")       \
                                                                            \
  develop(intx, TraceBytecodesAt, 0,                                        \
          "Traces bytecodes starting with specified bytecode number")       \
                                                                            \
  /* compiler interface */                                                  \
  develop(intx, CIStart, 0,                                                 \
          "the id of the first compilation to permit")                      \
                                                                            \
  develop(intx, CIStop,    -1,                                              \
          "the id of the last compilation to permit")                       \
                                                                            \
  develop(intx, CIStartOSR,     0,                                          \
          "the id of the first osr compilation to permit "                  \
          "(CICountOSR must be on)")                                        \
                                                                            \
  develop(intx, CIStopOSR,    -1,                                           \
          "the id of the last osr compilation to permit "                   \
          "(CICountOSR must be on)")                                        \
                                                                            \
  develop(intx, CIStartNative,     0,                                       \
          "the id of the first native compilation to permit "               \
          "(CICountNative must be on")                                      \
                                                                            \
  develop(intx, CIStopNative,    -1,                                        \
          "the id of the last native compilation to permit "                \
          "(CICountNative must be on")                                      \
                                                                            \
  develop(intx, CIBreakAtOSR,    -1,                                        \
          "id of osr compilation to break at")                              \
                                                                            \
  develop(intx, CIBreakAtNative,    -1,                                     \
          "id of native compilation to break at")                           \
                                                                            \
  develop(intx, CIBreakAt,    -1,                                           \
          "id of compilation to break at")                                  \
                                                                            \
  product(ccstr, CompileOnly, "",                                           \
          "List of methods (pkg/class.name) to restrict compilation to")    \
                                                                            \
  product(ccstr, CompileCommandFile, "",                                    \
          "Read compiler commands from this file [.hotspot_compiler]")      \
                                                                            \
  product(ccstr, CompileCommand, "",                                        \
          "Prepend to .hotspot_compiler; e.g. log,java/lang/String.<init>") \
                                                                            \
  product(bool, CICompilerCountPerCPU, false,                               \
          "1 compiler thread for log(N CPUs)")                              \
                                                                            \
  develop(intx, CIFireOOMAt,    -1,                                         \
          "Fire OutOfMemoryErrors throughout CI for testing the compiler "  \
          "(non-negative value throws OOM after this many CI accesses "     \
          "in each compile)")                                               \
                                                                            \
  develop(intx, CIFireOOMAtDelay, -1,                                       \
          "Wait for this many CI accesses to occur in all compiles before " \
          "beginning to throw OutOfMemoryErrors in each compile")           \
                                                                            \
  /* Priorities */                                                          \
  product(bool, UseThreadPriorities, true,                                  \
          "Use native thread priorities")	                            \
                                                                            \
  product(intx, ThreadPriorityPolicy, 0,                                    \
          "(Solaris Only)"                                                  \
          "=0 : NormalPriority and above map to native LWP default priority"  \
          "     which is high for timeshare and interactive scheduling classes." \
          "     Java priorities below NormPriority map to lower native LWP" \
          "     priority values."                                           \
          "=1 : Java thread priorities map over the range of native LWP"    \
          "     priorities.  Higher java thread priorities always map to"   \
          "     higher native>proirities. ")                                \
                                                                            \
  product(bool, ThreadPriorityVerbose, false,                               \
          "print priority changes")                                         \
                                                                            \
  product(intx, DefaultThreadPriority, -1,                                  \
          "what native priority threads run at if not specified elsewhere (-1 means no change)") \
                                                                            \
  product(intx, CompilerThreadPriority, -1,                                 \
          "what priority should compiler threads run at (-1 means no change)") \
                                                                            \
  product(intx, VMThreadPriority, -1,                                       \
          "what priority should VM threads run at (-1 means no change)")    \
                                                                            \
  product(bool, CompilerThreadHintNoPreempt, true,                          \
          "(Solaris only) Give compiler threads an extra quanta")           \
                                                                            \
  product(bool, VMThreadHintNoPreempt, false,                               \
          "(Solaris only) Give VM thread an extra quanta")                  \
                                                                            \
  product(intx, JavaPriority1_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority2_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority3_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority4_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority5_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority6_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority7_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority8_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority9_To_OSPriority, -1, "Map Java priorities to OS priorities") \
  product(intx, JavaPriority10_To_OSPriority,-1, "Map Java priorities to OS priorities") \
                                                                            \
  /* compiler debugging */                                                  \
  develop(intx, CompileTheWorldStartAt,     1,                              \
          "First class to consider when using +CompileTheWorld")            \
                                                                            \
  develop(intx, CompileTheWorldStopAt, max_jint,                            \
          "Last class to consider when using +CompileTheWorld")             \
                                                                            \
  develop(intx, NewCodeParameter,      0,                                   \
          "Testing Only: Create a dedicated integer parameter before "      \
          "putback")                                                        \
                                                                            \
  /* new oopmap storage allocation */                                       \
  develop(intx, MinOopMapAllocation,     8,                                 \
          "Minimum number of OopMap entries in an OopMapSet")               \
                                                                            \
  /* Background Compilation */                                              \
  develop(intx, LongCompileThreshold,     50,                               \
          "Used with +TraceLongCompiles")                                   \
                                                                            \
  product(intx, StarvationMonitorInterval,    200,                          \
          "Pause between each check in ms")                                 \
                                                                            \
  /* recompilation */                                                       \
  product_pd(intx, CompileThreshold,                                        \
          "number of method invocations/branches before (re-)compiling")    \
                                                                            \
  product_pd(intx, Tier2CompileThreshold,                                   \
          "threshold at which a tier 2 compilation is invoked")             \
                                                                            \
  product_pd(bool, TieredCompilation,                                       \
          "Enable two-tier compilation")                                    \
                                                                            \
  product_pd(intx, OnStackReplacePercentage,                                \
          "number of method invocations/branches (expressed as % of "       \
          "CompileThreshold) before (re-)compiling OSR code")               \
                                                                            \
  product(intx, InterpreterProfilePercentage, 33,                           \
          "number of method invocations/branches (expressed as % of "       \
          "CompileThreshold) before profiling in the interpreter")          \
                                                                            \
  develop(intx, MaxRecompilationSearchLength,    10,                        \
          "max. # frames to inspect searching for recompilee")              \
                                                                            \
  develop(intx, MaxInterpretedSearchLength,     3,                          \
          "max. # interp. frames to skip when searching for recompilee")    \
                                                                            \
  develop(intx, DesiredMethodLimit,  8000,                                  \
          "desired max. method size (in bytecodes) after inlining")         \
                                                                            \
  develop(intx, HugeMethodLimit,  8000,                                     \
          "don't compile methods larger than this if "                      \
          "+DontCompileHugeMethods")                                        \
                                                                            \
  /* recompilation monitoring / throttling */                               \
  /* (most values read only at startup)    */                               \
  develop(intx, RecompilationMonitorIntervalLength,   100,                  \
          "length (in ms) of recompilation monitor interval")               \
                                                                            \
  develop(intx, RecompilationMonitorIntervals,    10,                       \
          "number of recomp. monitor intervals for sliding avg.")           \
                                                                            \
  develop(intx, CompilationPercentageThresholdHigh,    20,                  \
          "throttle compilation if %time above this threshold")             \
                                                                            \
  develop(intx, CompilationPercentageThresholdLow,     5,                   \
          "ok to do more compilation if %time below this threshold")        \
                                                                            \
  develop(intx, InterpreterOverheadThresholdHigh,15,                        \
          "max. desired interpreter overhead")                              \
                                                                            \
  develop(intx, InterpreterOverheadThresholdLow,     5,                     \
          "min. desired interpreter overhead")                              \
                                                                            \
  develop_pd(intx, CompileThresholdMin,                                     \
          "minimal compile threshold")                                      \
                                                                            \
  develop_pd(intx, CompileThresholdMax,                                     \
          "maximal compile threshold")                                      \
                                                                            \
  develop(intx, CompileThresholdAdjustFactor,   150,                        \
          "adjustment factor (scaled by 100)")                              \
                                                                            \
  develop(intx, CounterHalfLifeTime,    30,                                 \
          "half-life time of invocation counters (in secs)")                \
                                                                            \
  develop(intx, CounterDecayMinIntervalLength,   500,                       \
          "Min. ms. between invocation of CounterDecay")                    \
                                                                            \
  /* dynamic class reloading monitoring */                                  \
                                                                            \
  develop(bool, HotSwap, true,                                              \
          "Allow dynamic class redefinition")                               \
                                                                            \
  develop(bool, PrintHotSwap, false,                                        \
          "Dynamic class redefinition monitoring")                          \
                                                                            \
  /* New JDK 1.4 reflection implementation */                               \
                                                                            \
  develop(bool, UseNewReflection, true,                                     \
          "Temporary flag for transition to reflection based on dynamic "   \
          "bytecode generation in 1.4; can no longer be turned off in 1.4 " \
          "JDK, and is unneeded in 1.3 JDK, but marks most places VM "      \
          "changes were needed")                                            \
                                                                            \
  develop(bool, VerifyReflectionBytecodes, false,                           \
          "Force verification of 1.4 reflection bytecodes. Does not work "  \
          "in situations like that described in 4486457 or for "            \
          "constructors generated for serialization, so can not be enabled "\
          "in product.")                                                    \
                                                                            \
  develop(intx, FastSuperclassLimit, 8,                                     \
          "Depth of hardwired instanceof accelerator array")                \
                                                                            \
  /* Properties for Java libraries  */                                      \
                                                                            \
  product(intx, MaxDirectMemorySize, -1,                                    \
          "Maximum total size of NIO direct-buffer allocations")            \
                                                                            \
  /* temporary developer defined flags  */                                  \
                                                                            \
  develop(bool, UseNewCode, true,                                           \
          "Testing Only: Use the new version while testing")                \
                                                                            \
  develop(bool, UseNewCode2, true,                                          \
          "Testing Only: Use the new version while testing")                \
                                                                            \
  develop(bool, UseNewCode3, true,                                          \
          "Testing Only: Use the new version while testing")                \
                                                                            \
  /* flags for performance data collection */                               \
                                                                            \
  product(bool, UsePerfData, true,                                          \
          "Flag to disable jvmstat instrumentation for performance testing" \
          "and problem isolation purposes.")                                \
                                                                            \
  product(bool, PerfDataSaveToFile, false,                                  \
          "Save PerfData memory to hsperfdata_<pid> file on exit")          \
                                                                            \
  product(intx, PerfDataSamplingInterval, 50 /*ms*/,                        \
          "Data sampling interval in milliseconds")                         \
                                                                            \
  develop(bool, PerfTraceDataCreation, false,                               \
          "Trace creation of Performance Data Entries")                     \
                                                                            \
  develop(bool, PerfTraceMemOps, false,                                     \
          "Trace PerfMemory create/attach/detach calls")                    \
                                                                            \
  product(bool, PerfDisableSharedMem, false,                                \
          "Store performance data in standard memory")                      \
                                                                            \
  product(intx, PerfDataMemorySize, 32*K,                                   \
          "Size of performance data memory region. Will be rounded "        \
          "up to a multiple of the native os page size.")                   \
                                                                            \
  product(intx, PerfMaxStringConstLength, 1024,                             \
          "Maximum PerfStringConstant string length before truncation")     \
                                                                            \
  product(bool, PerfAllowAtExitRegistration, false,                         \
          "Allow registration of atexit() methods")                         \
                                                                            \
  product(bool, PerfBypassFileSystemCheck, false,                           \
          "Bypass Win32 file system criteria checks (Windows Only)")        \
                                                                            \
  product(intx, UnguardOnExecutionViolation, 0,                             \
          "Unguard page and retry on no-execute fault (Win32 only)"         \
          "0=off, 1=conservative, 2=aggressive")                            \
                                                                            \
  product(bool, ManagementServer, false,                                    \
          "Create JMX Management Server")                                   \
                                                                    	    \
  /* Shared spaces */                                                       \
                                                                            \
  product(bool, UseSharedSpaces, true,                                      \
          "Use shared spaces in the permanent generation")                  \
                                                                            \
  product(bool, RequireSharedSpaces, false,                                 \
          "Require shared spaces in the permanent generation")              \
                                                                            \
  product(bool, ForceSharedSpaces, false,                                   \
          "Require shared spaces in the permanent generation")              \
                                                                            \
  product(bool, DumpSharedSpaces, false,                                    \
           "Special mode: JVM reads a class list, loads classes, builds "   \
            "shared spaces, and dumps the shared spaces to a file to be "   \
            "used in future JVM runs.")                                     \
                                                                            \
  product(bool, PrintSharedSpaces, false,                                   \
          "Print usage of shared spaces")                                   \
                                                                            \
  product(uintx, SharedDummyBlockSize, 512*M,                               \
          "Size of dummy block used to shift heap addresses (in bytes)")    \
                                                                            \
  product(uintx, SharedReadWriteSize,  12*M,                                \
          "Size of read-write space in permanent generation (in bytes)")    \
                                                                            \
  product(uintx, SharedReadOnlySize,    8*M,                                \
          "Size of read-only space in permanent generation (in bytes)")     \
                                                                            \
  product(uintx, SharedMiscDataSize,    4*M,                                \
          "Size of the shared data area adjacent to the heap (in bytes)")   \
                                                                            \
  product(uintx, SharedMiscCodeSize,    4*M,                                \
          "Size of the shared code area adjacent to the heap (in bytes)")   \
                                                                            \
  develop(bool, SharedSkipVerify,       false,                              \
          "Skip assert() and verify() which page-in unwanted shared "       \
          "objects. ")                                                      \
                                                                            \


/*
 *  Macros for factoring of globals
 */

// Interface macros
#define DECLARE_PRODUCT_FLAG(type, name, value, doc)    extern "C" type name;
#define DECLARE_PD_PRODUCT_FLAG(type, name, doc)        extern "C" type name;
#define DECLARE_DIAGNOSTIC_FLAG(type, name, value, doc) extern "C" type name;
#ifdef PRODUCT
#define DECLARE_DEVELOPER_FLAG(type, name, value, doc)  const type name = value; 
#define DECLARE_PD_DEVELOPER_FLAG(type, name, doc)      const type name = pd_##name; 
#else
#define DECLARE_DEVELOPER_FLAG(type, name, value, doc)  extern "C" type name;
#define DECLARE_PD_DEVELOPER_FLAG(type, name, doc)      extern "C" type name;
#endif

// Implementation macros
#define MATERIALIZE_PRODUCT_FLAG(type, name, value, doc)   type name = value;
#define MATERIALIZE_PD_PRODUCT_FLAG(type, name, doc)       type name = pd_##name; 
#define MATERIALIZE_DIAGNOSTIC_FLAG(type, name, value, doc) type name = value;
#ifdef PRODUCT
#define MATERIALIZE_DEVELOPER_FLAG(type, name, value, doc) /* flag name is constant */ 
#define MATERIALIZE_PD_DEVELOPER_FLAG(type, name, doc)     /* flag name is constant */ 
#else
#define MATERIALIZE_DEVELOPER_FLAG(type, name, value, doc) type name = value; 
#define MATERIALIZE_PD_DEVELOPER_FLAG(type, name, doc)     type name = pd_##name; 
#endif

RUNTIME_FLAGS(DECLARE_DEVELOPER_FLAG, DECLARE_PD_DEVELOPER_FLAG, DECLARE_PRODUCT_FLAG, DECLARE_PD_PRODUCT_FLAG, DECLARE_DIAGNOSTIC_FLAG)
