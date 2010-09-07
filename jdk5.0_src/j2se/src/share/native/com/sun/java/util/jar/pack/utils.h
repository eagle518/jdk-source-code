/*
 * @(#)utils.h	1.16 04/01/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
//Definitions of our util functions

void* must_malloc(int size);
#ifndef USE_MTRACE
#define mtrace(c, ptr, size) (0)
#else
void mtrace(char c, void* ptr, size_t size);
#endif

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
