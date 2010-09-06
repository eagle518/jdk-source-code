#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)frame_amd64.hpp	1.6 04/03/04 16:56:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A frame represents a physical stack frame (an activation).  Frames
// can be C or Java frames, and the Java frames can be interpreted or
// compiled.  In contrast, vframes represent source-level activations,
// so that one physical frame can correspond to multiple source level
// frames because of inlining.  A frame is comprised of {pc, fp, sp}

// Layout of interpreter frame:
//    [expression stack      ] * <- sp
//    [monitors              ]   \
//     ...                        | monitor block size
//    [monitors              ]   /
//    [monitor block size    ]
//    [byte code index/pointr]          = bcx()         bcx_offset
//    [pointer to locals     ]          = locals()      locals_offset
//    [constant pool cache   ]          = cache()       cache_offset
//    [methodData	     ]          = mdp()         mdx_offset
//    [methodOop             ]          = method()      method_offset
//    [old stack pointer     ]          (sender_sp)     sender_sp_offset
//    [old frame pointer     ]   <- fp  = link()
//    [return pc             ]
//    [method holder mirror  ]                     (only for native calls)
//    [locals and parameters ]   
//                               <- sender sp

 public:
  enum {
    pc_return_offset                                 =  0,
    // All frames
    link_offset                                      =  0,
    return_addr_offset                               =  1,
    sender_sp_offset                                 =  2,

    // Interpreter frames
    interpreter_frame_mirror_offset                  =  2, // for native calls only

    interpreter_frame_sender_sp_offset               = -1,
    interpreter_frame_method_offset                  = interpreter_frame_sender_sp_offset - 1,
#ifndef CORE
    interpreter_frame_mdx_offset                     = interpreter_frame_method_offset - 1,
    interpreter_frame_cache_offset                   = interpreter_frame_mdx_offset - 1,
#else // !CORE
    interpreter_frame_cache_offset                   = interpreter_frame_method_offset - 1,
#endif // CORE
    interpreter_frame_locals_offset                  = interpreter_frame_cache_offset - 1,
    interpreter_frame_bcx_offset                     = interpreter_frame_locals_offset - 1,    
    interpreter_frame_initial_sp_offset              = interpreter_frame_bcx_offset - 1,

    interpreter_frame_monitor_block_top_offset       = interpreter_frame_initial_sp_offset,
    interpreter_frame_monitor_block_bottom_offset    = interpreter_frame_initial_sp_offset,

    // Entry frames.  See call stub in stubGenerator_amd64.cpp.
#ifdef _WIN64
    entry_frame_after_call_words                     =  8,
    entry_frame_call_wrapper_offset                  =  2,
#else
    entry_frame_after_call_words                     = 13,
    entry_frame_call_wrapper_offset                  = -6,
#endif

    // Native frames XXX What's that for???
    native_frame_initial_param_offset                =  2,

    // Native caller frames
#ifdef _WIN64
    arg_reg_save_area_bytes                          = 32 // Register argument save area
#else
    arg_reg_save_area_bytes                          =  0
#endif
  };

  jlong long_at(int offset) const
  {
    return *long_at_addr(offset);
  }

  void long_at_put(int offset, jlong value)
  {
    *long_at_addr(offset) = value; 
  }

 private:
  // an additional field beyond _sp and _pc:
  intptr_t* _fp; // frame pointer

  jlong* long_at_addr(int offset) const
  {
    return (jlong*) addr_at(offset); 
  }

 public:
  // Constructors
  frame(intptr_t* sp, intptr_t* fp, address pc) 
  { 
    _sp = sp;
    _fp = fp;
    _pc = pc;
  }

  frame(intptr_t* sp, intptr_t* fp) 
  {
    _sp = sp;
    _fp = fp;
    _pc = (address) (sp[-1]);
    // In case of native stubs, the pc retreived here might be 
    // wrong. (the _last_native_pc will have the right value)
    // So do not put add any asserts on the _pc here.
  }

  // accessors for the instance variables
  intptr_t* fp() const 
  {
    return _fp; 
  }

  void patch_pc(Thread* thread, address pc);

  inline address* sender_pc_addr() const;

  // return address of param, zero origin index.
  inline address* frame::native_param_addr(int idx) const;
  
