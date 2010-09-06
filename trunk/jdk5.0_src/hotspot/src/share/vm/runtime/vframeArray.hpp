#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vframeArray.hpp	1.69 03/12/23 16:44:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A vframeArray is an array used for momentarily storing off stack Java method activations
// during deoptimization. Essentially it is an array of vframes where each vframe
// data is stored off stack. This structure will never exist across a safepoint so
// there is no need to gc any oops that are stored in the structure.


class LocalsClosure;
class ExpressionStackClosure;
class MonitorStackClosure;
class MonitorArrayElement;
class StackValueCollection;

// A vframeArrayElement is an element of a vframeArray. Each element
// represent an interpreter frame which will eventually be created.

class vframeArrayElement : public _ValueObj {
  private:

    frame _frame;                                                // the interpreter frame we will unpack into
    int _bci;                                                    // raw bci for this vframe
    methodOop  _method;                                          // the method for this vframe
    MonitorChunk* _monitors;                                     // active monitors for this vframe
    StackValueCollection* _locals;
    StackValueCollection* _expressions;

  public:

  frame* iframe(void)                { return &_frame; }

  int bci(void) const;

  int raw_bci(void) const            { return _bci; }

  methodOop method(void) const       { return _method; }

  MonitorChunk* monitors(void) const { return _monitors; }

  StackValueCollection* locals(void) const             { return _locals; }

  StackValueCollection* expressions(void) const        { return _expressions; }

  void fill_in(compiledVFrame* vf);

  // Formerly part of deoptimizedVFrame


  // Returns the on stack word size for this frame
  // callee_parameters is the number of callee locals residing inside this frame
  int on_stack_size(int callee_parameters, 
		    int callee_locals, 
		    bool is_top_frame, 
		    int popframe_extra_stack_expression_els) const;

  // Unpacks the element to skeletal interpreter frame
  void unpack_on_stack(int callee_parameters, 
		       int callee_locals,
		       frame* caller,
		       bool is_top_frame,
		       int exec_mode);

#ifndef PRODUCT
  void print(outputStream* st);
#endif /* PRODUCT */
};

// this can be a ResourceObj if we don't save the last one...
// but it does make debugging easier even if we can't look
// at the data in each vframeElement

class vframeArray: public CHeapObj {
 private:


  // Here is what a vframeArray looks like in memory

  /*
      fixed part 
	description of the original frame
	_frames - number of vframes in this array
	adapter info
	callee register save area
      variable part
	vframeArrayElement   [ 0 ]
	...
	vframeArrayElement   [_frames - 1]
       
  */

  JavaThread*                  _owner_thread;
  vframeArray*                 _next;
  CodeBlob*                    _old_adapter;       // I2C/OSR adapter, if being removed
  frame                        _original;          // the original frame of the deoptee
  frame                        _caller;            // caller of root frame in vframeArray
  // This frame is the sender (ignoring adapters) of the frame being deoptimized
  // When an I2C or OSR adapter is being removed is identical to _adapter_caller
  // it'd be nice to remove _adapter_caller and use sender with an appropriate flag.
  frame                        _sender;
#ifdef COMPILER2
  // Either a C2I adapter will be added or an existing I2C adapter will be removed.
  C2IAdapter*                  _new_adapter;           // C2I adapter, if required
  bool                         _adjust_adapter_caller; // Does interp-frame need more space for non-parameter_locals
#endif
  frame                        _adapter_caller;    // caller of I2C or OSR adapter to be removed

  Deoptimization::UnrollBlock* _unroll_block;
  int                          _frame_size;

  int                          _frames; // number of javavframes in the array (does not count any adapter)

  intptr_t                     _callee_registers[RegisterMap::reg_count];

  vframeArrayElement           _elements[1];   // First variable section. 

  void fill_in_element(int index, compiledVFrame* vf);

 public:


  // Tells whether index is within bounds.
  bool is_within_bounds(int index) const        { return 0 <= index && index < frames(); }

  // Accessores for instance variable
  int frames() const                            { return _frames;   }

  static vframeArray* allocate(JavaThread* thread, int frame_size, GrowableArray<compiledVFrame*>* chunk,
                               RegisterMap* reg_map, frame sender, frame caller, frame self, frame adapter_caller);


  vframeArrayElement* element(int index)        { assert(is_within_bounds(index), "Bad index"); return &_elements[index]; }

  // Allocates a new vframe in the array and fills the array with vframe information in chunk
  void fill_in(JavaThread* thread, int frame_size, GrowableArray<compiledVFrame*>* chunk, const RegisterMap *reg_map, bool needs_adapter);

  // Returns the owner of this vframeArray
  JavaThread* owner_thread() const           { return _owner_thread; }

  // Accessors for next
  vframeArray* next() const                  { return _next; }
  void set_next(vframeArray* value)          { _next = value; }

  // Accessors for sp
  intptr_t* sp() const                       { return _original.sp(); }

  intptr_t* unextended_sp() const            { return _original.unextended_sp(); }

  address original_pc() const                { return _original.pc(); }

  frame original() const                     { return _original; }

  frame caller() const                       { return _caller; }

  frame sender() const                       { return _sender; }

  void set_old_adapter(CodeBlob* adapter)    { _old_adapter = adapter; }
  CodeBlob* old_adapter(void) const          { return _old_adapter; }

#ifdef COMPILER2
  C2IAdapter* new_adapter() const            { return _new_adapter; }
  bool        adjust_adapter_caller() const  { return _adjust_adapter_caller; }
  void set_adjust_adapter_caller(bool adjust){ _adjust_adapter_caller = adjust; }
#endif
  // Like to remove these guys
  frame adapter_caller() const               { return _adapter_caller; }

  // Accessors for unroll block
  Deoptimization::UnrollBlock* unroll_block() const         { return _unroll_block; }
  void set_unroll_block(Deoptimization::UnrollBlock* block) { _unroll_block = block; }

  // Returns the size of the frame that got deoptimized
  int frame_size() const { return _frame_size; }

  // Machine dependent code for extending the caller_frame for interpreted callee's locals
  COMPILER1_ONLY(int extend_caller_frame(int callee_parameters, int callee_locals);)

  // Machine dependent code for extending the adapter_caller for interpreted callee's locals
  COMPILER2_ONLY(int i2c_frame_adjust(int callee_parameters, int callee_locals);)

  // Unpack the array on the stack passed in stack interval
  void unpack_to_stack(frame &unpack_frame, int exec_mode);

  // Deallocates monitor chunks allocated during deoptimization.
  // This should be called when the array is not used anymore.
  void deallocate_monitor_chunks();



  // Accessor for register map
  address register_location(int i) const;

  void print_on_2(outputStream* st) PRODUCT_RETURN;
  void print_value_on(outputStream* st) const PRODUCT_RETURN;

#ifndef PRODUCT
  // Comparing
  bool structural_compare(JavaThread* thread, GrowableArray<compiledVFrame*>* chunk);
#endif
  
};

