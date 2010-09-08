/*
 * @(#)utils.h	1.19 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
//Definitions of our util functions

void* must_malloc(size_t size);
#ifndef USE_MTRACE
#define mtrace(c, ptr, size) (0)
#else
void mtrace(char c, void* ptr, size_t size);
#endif

// overflow management
#define OVERFLOW ((size_t)-1)
#define PSIZE_MAX (OVERFLOW/2)  /* normal size limit */

inline size_t scale_size(size_t size, size_t scale) {
  return (size > PSIZE_MAX / scale) ? OVERFLOW : size * scale;
}

inline size_t add_size(size_t size1, size_t size2) {
  return ((size1 | size2 | (size1 + size2)) > PSIZE_MAX)
    ? OVERFLOW
    : size1 + size2;
}

inline size_t add_size(size_t size1, size_t size2, int size3) {
  return add_size(add_size(size1, size2), size3);
}

// These may be expensive, because they have to go via Java TSD,
// if the optional u argument is missing.
struct unpacker;
extern void unpack_abort(const char* msg, unpacker* u = null);
extern bool unpack_aborting(unpacker* u = null);

#ifndef PRODUCT
inline bool endsWith(const char* str, const char* suf) {
  size_t len1 = strlen(str);
  size_t len2 = strlen(suf);
  return (len1 > len2 && 0 == strcmp(str + (len1-len2), suf));
}
#endif

void mkdirs(int oklen, char* path);
