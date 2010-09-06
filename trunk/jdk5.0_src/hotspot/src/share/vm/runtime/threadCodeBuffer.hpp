#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadCodeBuffer.hpp	1.26 03/12/23 16:44:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
  
//
// On-demand allocated code-buffer for roll-forward at safepoint
// 
// Contains all information need for roll-forward when patching code.
//

class ThreadCodeBuffer : public CHeapObj {
 private:  
  address   _code;
  int       _size;
  nmethod*  _method;
  address   _real_pc;         // value of pc that correspond to code_begin()

 public:
  ThreadCodeBuffer(int size_in_bytes, nmethod *nm, address real_pc);
  ~ThreadCodeBuffer();
  
  // Accessors   
  address   code_begin() const          { return _code; }
  address   code_end() const            { return _code + _size; }
  bool	    contains(address pc) const  { return (code_begin() <= pc && pc < code_end()); }
  bool      captures(address pc) const  { return (real_pc() <= pc && pc < real_pc() + size()); }
  int       size() const                { return _size; }
  address   real_pc() const             { return _real_pc; }
  nmethod * method() const              { return _method; }
  
  // calculates real_pc (pointing into nmethod) from pc pointing into code_buffer
  address compute_adjusted_pc(address pc);

  // the inverse:  pc was a "real_pc" and must be redirected into the code_buffer
  address capture_pc(address pc);

  // Execute code with thread
  void	  copy     (int offset, address start, int length);  
  void	  copy_code(int offset, address start, int length);  // do relocation
  bool    reposition_and_resume_thread(JavaThread *thread, ExtendedPC addr);
};

inline address ThreadCodeBuffer::compute_adjusted_pc(address pc) {
  assert(contains(pc), "pc must point into codebuffer") 
  pc = real_pc() + (pc - code_begin());
  assert(method()->contains(pc), "result must be in nmethod");      
  return pc;
}
 
inline address ThreadCodeBuffer::capture_pc(address pc) {
  assert(captures(pc), "pc must point into original code for codebuffer") 
  pc = code_begin() + (pc - real_pc());
  assert(contains(pc), "result must be in codebuffer");
  return pc;
}
 
