#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Compilation.hpp	1.72 03/12/23 16:39:02 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class BlockBegin;
class CompilationResourceObj;
class ExceptionScope;
class ExceptionInfo;
class ExceptionRangeTable;
class DebugInformationRecorder;
class FrameMap;
class IR;
class IRScope;
class Instruction;
class LocalMapping;
class RInfoCollection;
class OopMap;
class LIR_Emitter;
class LIR_Assembler;
class CodeOffsets;
class CodeEmitInfo;
class ciEnv;
class ciLocalMap;
class ciMethod;
class ValueStack;
class Item;
class LIR_OprDesc;
class C1_MacroAssembler;
typedef LIR_OprDesc* LIR_Opr;

define_array(ExceptionInfoArray, ExceptionInfo*)
define_stack(ExceptionInfoList,  ExceptionInfoArray)

class Compilation: public StackObj {
  friend class CompilationResourceObj;
 private:

  static Arena* _arena; 
  static Arena* arena() { return _arena; }

  static Compilation* _compilation;

 private:
  // compilation specifics
  AbstractCompiler*  _compiler;
  ciEnv*             _env;
  ciMethod*          _method;
  int                _osr_bci;
  IR*                _hir;
  int                _max_spills;
  ciLocalMap*        _oop_map;
  FrameMap*          _frame_map;
  C1_MacroAssembler* _masm;
  bool               _needs_debug_information;
  bool               _has_exception_handlers;
  bool               _has_unsafe_access;
  const char*        _bailout_msg;
  ExceptionInfoList* _exception_info_list;
  ExceptionRangeTable _exception_range_table;
  ImplicitExceptionTable _null_check_table;
  DEBUG_ONLY(BlockBegin* _cur_assembled_block;)  // Current block being assembled via
                                                 // LIR_Assembler; debugging purposes only

  // JVMPI specifics
  bool _jvmpi_event_compiled_method_enabled;
  bool _jvmpi_event_method_entry_enabled;
  bool _jvmpi_event_method_entry2_enabled;
  bool _jvmpi_event_method_exit_enabled;

  // compilation helpers
  void initialize();
  void initialize_oop_maps();
  void build_hir();
  void emit_lir();
  void emit_code_prolog_native(FrameMap* map);
  void emit_code_prolog_non_native(FrameMap* map);
  void emit_code_epilog(LIR_Assembler* assembler);
  int  emit_code_body(CodeOffsets* offsets);

  int  compile_java_method(CodeOffsets* offsets);
  void emit_code_for_native(address native_entry, CodeOffsets* offsets);
  int  compile_native_method(CodeOffsets* offsets);
  int  compile_library_method(CodeOffsets* offsets);
  void install_code(CodeOffsets* offsets, int frame_size);
  void compile_method();

  void init_framemap(FrameMap* map);

  ExceptionInfoList* exception_info_list() const { return _exception_info_list; }
  void generate_exception_range_table();
  void add_exception_range_entries(int pco, bool at_call, ExceptionScope* scope, bool extend, ExceptionScope** last_recorded_scope, int* last_pco);
  ExceptionRangeTable* exception_range_table() { return &_exception_range_table; }

#ifndef PRODUCT
  Instruction*       _current_instruction;       // the instruction currently being processed
  Instruction*       _last_instruction_printed;  // the last instruction printed during traversal
#endif // PRODUCT

 public:
  // creation
  Compilation(AbstractCompiler* compiler, ciEnv* env, ciMethod* method, int osr_bci, C1_MacroAssembler* masm);
  ~Compilation();

  static Compilation* current_compilation()      { return _compilation; }

  intStack* get_init_vars();

  // accessors
  AbstractCompiler* compiler() const             { return _compiler; }
  bool needs_debug_information() const           { return _needs_debug_information; }
  bool has_exception_handlers() const            { return _has_exception_handlers; }
  bool has_unsafe_access() const                 { return _has_unsafe_access; }
  ciMethod* method() const                       { return _method; }
  int osr_bci() const                            { return _osr_bci; }
  bool is_osr_compile() const                    { return osr_bci() >= 0; }
  IR* hir() const                                { return _hir; }
  int max_spills() const                         { return _max_spills; }
  ciLocalMap* oop_map() const                    { return _oop_map; }
  FrameMap* frame_map() const                    { return _frame_map; }
  void set_frame_map(FrameMap* map)              { _frame_map = map; }
  CodeBuffer* code() const;
  C1_MacroAssembler* masm() const                { return _masm; }
  bool is_optimized_library_method() const;
#ifdef ASSERT
  BlockBegin* cur_assembled_block() const        { return _cur_assembled_block; }
#endif

  // setters
  void set_needs_debug_information(bool f)       { _needs_debug_information = f; }
  void set_has_exception_handlers(bool f)        { _has_exception_handlers = f; }
  void set_has_unsafe_access(bool f)             { _has_unsafe_access = f; }
  // Add a set of exception handlers covering the given PC offset
  void add_exception_handlers_for_pco(int pco, bool at_call, ExceptionScope* exception_scope);
  // Statistics gathering
  void notice_inlined_method(ciMethod* method);
#ifdef ASSERT
  void set_cur_assembled_block(BlockBegin* block){ _cur_assembled_block = block; }
#endif

  DebugInformationRecorder* debug_info_recorder() const; // = _env->recorder();
  ImplicitExceptionTable* null_check_table()     { return &_null_check_table; }

  // jvmpi flags
  bool jvmpi_event_compiled_method_enabled()const{ return _jvmpi_event_compiled_method_enabled; };
  bool jvmpi_event_method_entry_enabled() const  { return _jvmpi_event_method_entry_enabled;}
  bool jvmpi_event_method_entry2_enabled() const { return _jvmpi_event_method_entry2_enabled;}
  bool jvmpi_event_method_exit_enabled() const   { return _jvmpi_event_method_exit_enabled;}
  bool jvmpi_event_method_enabled() const {
    return
      jvmpi_event_method_entry_enabled()  ||
      jvmpi_event_method_entry2_enabled() ||
      jvmpi_event_method_exit_enabled();
  }

#ifndef PRODUCT
  Instruction* set_current_instruction(Instruction* instr) {
    Instruction* previous = _current_instruction;
    _current_instruction = instr;
    return previous;
  }

  void maybe_print_current_instruction();
#endif // PRODUCT

  // error handling
  void bailout(const char* msg);
  bool bailed_out() const                        { return _bailout_msg != NULL; }
  const char* bailout_msg() const                { return _bailout_msg; }

  // timers
  static void print_timers();

#ifndef PRODUCT
  // debugging support.
  // produces a file named c1compileonly in the current directory with
  // directives to compile only the current method and it's inlines.
  // The file can be passed to the command line option -XX:Flags=<filename>
  void compile_only_this_method();
  void compile_only_this_scope(outputStream* st, IRScope* scope);
#endif // PRODUCT

  LIR_Opr lir_opr_for_instruction(Instruction* v);
  GrowableArray<LIR_Opr>* value_stack2lir_stack(ValueStack* value_stack);
  int item2stack(const Item* item);
  LIR_Opr item2lir(const Item* item);
};


#ifdef ASSERT
class InstructionMark: public StackObj {
 private:
  Compilation* _compilation;
  Instruction*  _previous;
  
 public:
  InstructionMark(Compilation* compilation, Instruction* instr) {
    _compilation = compilation;
    _previous = _compilation->set_current_instruction(instr);
  }
  ~InstructionMark() {
    _compilation->set_current_instruction(_previous);
  }
};
#endif // ASSERT


//----------------------------------------------------------------------
// Base class for objects allocated by the compiler in the compilation arena
class CompilationResourceObj ALLOCATION_SUPER_CLASS_SPEC {
 public:
  void* operator new(size_t size) { return Compilation::arena()->Amalloc(size); }
  void  operator delete(void* p) {} // nothing to do
};


//----------------------------------------------------------------------
// Class for aggregating exception handler information.

// Effectively extends ExceptionScope class with PC offset of
// potentially exception-throwing instruction. This class is used so
// that one ExceptionScope can be used for a HIR instruction which may
// correspond to multiple LIR, and thereby machine, instructions.
// (Note that if it is necessary to supply a non-shared copy of the
// ExceptionScope to each of these machine instructions, for example
// to handle "adapter blocks" (for canonicalizing register locations)
// as XHandlers, we do not currently perform this cloning in the
// backend, so the same ExceptionScope is used for multiple machine
// instructions.)
// This class is used at the end of the compilation to build the
// ExceptionRangeTable.
class ExceptionScope;
class ExceptionInfo: public CompilationResourceObj {
 private:
  int             _pco;                // PC of potentially exception-throwing instruction
  bool            _at_call;            // PC is at a call instruction
  ExceptionScope* _exception_scope;    // nested list of exception handlers covering this PC

 public:
  ExceptionInfo(int pco, bool at_call, ExceptionScope* exception_scope);

  int pco()                                      { return _pco; }
  bool at_call()                                 { return _at_call; }
  ExceptionScope* exception_scope()              { return _exception_scope; }
};
