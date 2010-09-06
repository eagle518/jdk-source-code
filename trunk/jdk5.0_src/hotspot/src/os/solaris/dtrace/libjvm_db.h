#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)libjvm_db.h	1.7 04/07/29 16:36:13 JVM_DB"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <proc_service.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jvm_agent jvm_agent_t;

#define	JVM_DB_VERSION	1

jvm_agent_t *Jagent_create(struct ps_prochandle *P, int vers);

/* 
 * Called from Jframe_iter() for each java frame.  If it returns 0, then
 * Jframe_iter() proceeds to the next frame.  Otherwise, the return value is
 * immediately returned to the caller of Jframe_iter().
 *
 * Parameters:
 *    'cld' is client supplied data (to maintain iterator state, if any).
 *    'name' is java method name.
 *    'bci' is byte code index. it will be -1 if not available.
 *    'line' is java source line number. it will be 0 if not available.
 *    'handle' is an abstract client handle, reserved for future expansions
 */

typedef int java_stack_f(void *cld, const prgregset_t regs, const char* name, int bci, int line, void *handle);

/*
 * Iterates over the java frames at the current location.  Returns -1 if no java
 * frames were found, or if there was some unrecoverable error.  Otherwise,
 * returns the last value returned from 'func'.
 */
int Jframe_iter(jvm_agent_t *agent, prgregset_t gregs, java_stack_f *func, void* cld);

void Jagent_destroy(jvm_agent_t *J);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
