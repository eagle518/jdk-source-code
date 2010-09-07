/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

/* decode_instructions -- dump a range of addresses as native instructions
   This implements the protocol required by the HotSpot PrintAssembly option.

   The starting and ending addresses are within the current process's address space.

   The option string, if not empty, is interpreted by the disassembler implementation.

   The printf callback is 'fprintf' or any other workalike.
   It is called as (*printf_callback)(printf_stream, "some format...", some, format, args).

   The event callback receives an event tag (a string) and an argument (a void*).
   It is called as (*event_callback)(event_stream, "tag", arg).

   Events:
     <insn pc='%p'>             begin an instruction, at a given location
     </insn pc='%d'>            end an instruction, at a given location
     <addr value='%p'/>         emit the symbolic value of an address

   A tag format is one of three basic forms: "tag", "/tag", "tag/",
   where tag is a simple identifier, signifying (as in XML) a element start,
   element end, and standalone element.  (To render as XML, add angle brackets.)
*/
extern
#ifdef DLL_EXPORT
  DLL_EXPORT
#endif
void* decode_instructions(void* start, void* end,
                          void* (*event_callback)(void*, const char*, void*),
                          void* event_stream,
                          int (*printf_callback)(void*, const char*, ...),
                          void* printf_stream,
                          const char* options);

/* convenience typedefs */

typedef void* (*decode_instructions_event_callback_ftype)  (void*, const char*, void*);
typedef int   (*decode_instructions_printf_callback_ftype) (void*, const char*, ...);
typedef void* (*decode_instructions_ftype) (void* start, void* end,
                                            decode_instructions_event_callback_ftype event_callback,
                                            void* event_stream,
                                            decode_instructions_printf_callback_ftype printf_callback,
                                            void* printf_stream,
                                            const char* options);
