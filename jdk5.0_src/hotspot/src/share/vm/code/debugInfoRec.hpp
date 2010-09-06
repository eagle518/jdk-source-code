#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)debugInfoRec.hpp	1.27 03/12/23 16:39:51 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//** The DebugInformationRecorder collects debugging information
//   for a compiled method.
//   Debugging information is used for:
//   - garbage collecting compiled frames
//   - stack tracing across compiled frames
//   - deoptimizating compiled frames
//
//   The implementation requires the compiler to use the recorder
//   in the following order:
//   1) Describe all dependents (use add_dependent)
//   2) Describe debug information for safepoints at increasing adresses.
//      a) Add safepoint entry (use add_safepoint)
//      b) Describe scopes for that safepoint
//         - create locals if needed (use create_scope_values)
//         - create expressions if needed (use create_scope_values)
//         - create monitor stack if needed (use create_monitor_values)
//         - describe scope (use describe_scope)
//         "repeat last four steps for all scopes"
//         "outer most scope first and inner most scope last" 
//         NB: nodes from create_scope_values and create_locations 
//             can be reused for simple sharing.
//   3) Use oop_size, data_size, pcs_size to create the nmethod and
//      finally migrate the debugging information into the nmethod
//      by calling copy_to.

class DebugToken; // Opaque datatype for stored:
                  //  - GrowableArray<ScopeValue*>
                  //  - GrowableArray<MonitorValue*>

class PcDescNode;

const int SynchronizationEntryBCI = -1;

class DebugInformationRecorder: public ResourceObj {
 public:
  // constructor
  DebugInformationRecorder(OopRecorder* oop_recorder);

  // adds an oopmap at a specific offset
  void add_oopmap(int pc_offset, bool at_call, OopMap* map);

  // adds a jvm mapping at pc-offset, at_call tells whether the safepoint
  // is associated with a call instruction.
  void add_safepoint(int pc_offset, bool at_call, OopMap* map);

  // describes debugging information for a scope at current pc_offset
  // (defined as pc_offset at last call to add_safepoint)
  void describe_scope(ciMethod*   method,
		      int bci,
                      DebugToken* locals      = (DebugToken*)serialized_null,
                      DebugToken* expressions = (DebugToken*)serialized_null,
                      DebugToken* monitors    = (DebugToken*)serialized_null);
  
  // helper fuctions for describe_scope to enable sharing 
  DebugToken* create_scope_values(GrowableArray<ScopeValue*>* values);
  DebugToken* create_monitor_values(GrowableArray<MonitorValue*>* monitors);

  // adds dependent (class, method) pair.
  // The resulting nmethod is invalidated if a new class is added below class
  // and the added class overrides method.
  void add_dependent(ciInstanceKlass* klass, ciMethod* method);

  // returns the size of the generated scopeDescs.
  int data_size();
  int pcs_size();
  int oop_size() { return oop_recorder()->oop_size(); }

  // copy the generated debugging information to nmethod
  void copy_to(nmethod* nm);

  // verifies the debug information
  void verify(const nmethod* code);

  // Low level dumping of handle used by ScopeValue
  int append_handle(jobject h);  

  // Method for setting oopmaps to temporarily preserve old handling of oopmaps
  OopMapSet *_oopmaps;
  void set_oopmaps(OopMapSet *oopmaps) { _oopmaps = oopmaps; }

  // Returns the number of collected dependents (see add_dependent)
  int number_of_dependents() const { return _number_of_dependents; }

  // Returns the index of the first collected dependent.
  int first_dependent()      const { return _first_dependent; }

  OopRecorder* oop_recorder() { return _oop_recorder; }

 private:
  friend class ScopeDesc;
  friend class vframeStreamCommon;

  enum Phase { describe_dependents, describe_safepoints, completed } phase;
 
  DebugInfoWriteStream* _stream;
  
  DebugInfoWriteStream* stream() const { return _stream; }

  OopRecorder* _oop_recorder;

  int     _first_dependent;
  int     _number_of_dependents;

  PcDescNode* _pcs;
  int         _pcs_size;
  int         _pcs_length;

  int  serialize_monitor_values(GrowableArray<MonitorValue*>* monitors);
  int  serialize_scope_values(GrowableArray<ScopeValue*>* values);

  void check_phase(DebugInformationRecorder::Phase new_phase);

  enum { serialized_null = -1 };
};

