#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_win32.inline.hpp	1.37 03/12/23 16:37:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline const char* os::file_separator()                { return "\\"; }
inline const char* os::line_separator()                { return "\r\n"; }
inline const char* os::path_separator()                { return ";"; }

inline const char* os::jlong_format_specifier()        { return "%I64d"; }
inline const char* os::julong_format_specifier()       { return "%I64u"; }

// File names are case-insensitive on windows only
inline int os::file_name_strcmp(const char* s, const char* t) {
  return stricmp(s, t);
}

// Used to improve time-sharing on some systems
inline void os::loop_breaker(int attempts) {}

inline bool os::obsolete_option(const JavaVMOption *option) {
  return false;
}

inline bool os::uses_stack_guard_pages() {
  return os::win32::is_nt();
}

inline bool os::allocate_stack_guard_pages() {
  assert(uses_stack_guard_pages(), "sanity check");
  return true;
}

inline int os::readdir_buf_size(const char *path)
{
  /* As Windows doesn't use the directory entry buffer passed to
     os::readdir() this can be as short as possible */

  return 1;
}

// Bang the shadow pages if they need to be touched to be mapped.
inline void os::bang_stack_shadow_pages() {
  // Write to each page of our new frame to force OS mapping.
  // If we decrement stack pointer more than one page
  // the OS may not map an intervening page into our space
  // and may fault on a memory access to interior of our frame.
  address sp = current_stack_pointer();
  for (int pages = 1; pages <= StackShadowPages; pages++) {
    *((int *)(sp - (pages * vm_page_size()))) = 0;
  }
}
