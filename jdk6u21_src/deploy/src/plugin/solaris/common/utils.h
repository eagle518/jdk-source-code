/*
 * @(#)utils.h	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


#ifdef __cplusplus
extern "C" {
#endif
    int slen(const char *s);
    int slenUTF(const char *s);
    void put_int(char *buff, int offset, int x);
    void put_short(char *buff, int offset, short x);
    int get_int(char *buff, int offset);
    short get_short(char *buff, int offset);
    int s_pipe(int fds[2]);
    void init_utils();		/* Initialize the utility package */
    void plugin_error(const char *format, ...);     
    FILE *fopentrace(char *name);
        
    /* An error that is reported using dgettext */
    void plugin_formal_error(const char *msg);
    /* An formal error that is not localized - usually things like
       filenames that are a part of error messages */
    void plugin_raw_formal_error(const char *msg);
    /* Wrapper that does a dlsym with error checking */
    void *load_function(void* library_handle, const char* function_name);
#ifdef __cplusplus
}
#endif
