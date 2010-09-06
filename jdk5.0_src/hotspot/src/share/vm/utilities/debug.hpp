#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)debug.hpp	1.37 04/02/25 09:58:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// assertions
#ifdef ASSERT
// Turn this off by default:
//#define USE_REPEATED_ASSERTS
#ifdef USE_REPEATED_ASSERTS
  #define assert(p,msg)                                              \
    { for (int __i = 0; __i < AssertRepeat; __i++) {                 \
        if (!(p)) {                                                  \
          report_assertion_failure(__FILE__, __LINE__,               \
                                  "assert(" XSTR(p) ",\"" msg "\")");\
          BREAKPOINT;                                                \
        }                                                            \
      }                                                              \
    }
#else
  #define assert(p,msg)                                          \
    if (!(p)) {                                                  \
      report_assertion_failure(__FILE__, __LINE__,               \
                              "assert(" XSTR(p) ",\"" msg "\")");\
      BREAKPOINT;                                                \
    }
#endif
// Do not assert this condition if there's already another error reported.
#define assert_if_no_error(cond,msg) assert((cond) || is_error_reported(), msg)
#else
  #define assert(p,msg)
  #define assert_if_no_error(cond,msg)
#endif


// fatals
#define fatal(m)                             { report_fatal(__FILE__, __LINE__, m                          ); BREAKPOINT; }
#define fatal1(m,x1)                         { report_fatal_vararg(__FILE__, __LINE__, m, x1               ); BREAKPOINT; }
#define fatal2(m,x1,x2)                      { report_fatal_vararg(__FILE__, __LINE__, m, x1, x2           ); BREAKPOINT; }
#define fatal3(m,x1,x2,x3)                   { report_fatal_vararg(__FILE__, __LINE__, m, x1, x2, x3       ); BREAKPOINT; }
#define fatal4(m,x1,x2,x3,x4)                { report_fatal_vararg(__FILE__, __LINE__, m, x1, x2, x3, x4   ); BREAKPOINT; }

// guarantee is like assert except it's always executed -- use it for cheap tests that catch errors 
// that would otherwise be hard to find
#define guarantee(b,msg)         { if (!(b)) fatal(msg); }

#define ShouldNotCallThis()      { report_should_not_call        (__FILE__, __LINE__); BREAKPOINT; }
#define ShouldNotReachHere()     { report_should_not_reach_here  (__FILE__, __LINE__); BREAKPOINT; }
#define Unimplemented()          { report_unimplemented          (__FILE__, __LINE__); BREAKPOINT; }
#define Untested(msg)            { report_untested               (__FILE__, __LINE__, msg); BREAKPOINT; }

// No plain-text messages should be printed out in product mode
void obfuscate_location(const char *file_name, int line_no, char* buf, int buflen);

// error reporting helper functions
void report_assertion_failure(const char* file_name, int line_no, const char* message);
void report_fatal_vararg(const char* file_name, int line_no, const char* format, ...);
void report_fatal(const char* file_name, int line_no, const char* message);
void report_should_not_call(const char* file_name, int line_no);
void report_should_not_reach_here(const char* file_name, int line_no);
void report_unimplemented(const char* file_name, int line_no);
void report_untested(const char* file_name, int line_no, const char* msg);
void warning(const char* format, ...);

// Support for self-destruct
bool is_error_reported();
void set_error_reported();

void pd_ps(frame f);
void pd_obfuscate_location(char *buf, int buflen);
