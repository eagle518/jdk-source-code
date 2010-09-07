#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "Debug.h"
#include "utils.h"
#include <unistd.h>
#include <stropts.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <locale.h>
#include <errno.h>
#include <strings.h>
#include <dlfcn.h>
#include "plugin_defs.h"
#include "pluginversion.h"
#include <pwd.h>

/*
 * Internal utility routine to get size of a "C" string.
 * If the string is a null pointer we return zero.
 */
int tracing = 0;

static int init_utils_called = 0;

static void check_initialized(){
    if (!init_utils_called) init_utils();
}

static FILE *tracefile = NULL;

/* Initialization of the utility package. Should be called only once. */
void
init_utils() {
    char *name;    
    if (init_utils_called) 
	return;
    init_utils_called = 1;
    name =  getenv("JAVA_PLUGIN_TRACE");
    if (name != NULL) {
	fprintf(stderr, "Turning tracing on....\n");
	tracing = 1;
	/* Parent file's name defined in plugin_defs.h */
    	tracefile = fopentrace(PARENT_TRACE_FILE(PLUGIN_NODOTVERSION));	
    } else {
      tracing = 0;
    }

}

int
slen(const char *s) {
    if (s == NULL) {
	return 0;
    }
    return strlen(s);
}

int 
slenUTF(const char *s) {
    if (s == NULL)
	return 0;
    return strlen(s);
}


/*
 * Put an integer into the buffer in big-endian byte order.
 */
void
put_int(char *buff, int offset, int x) {
    buff[offset] = (x >> 24) & 0xFF;
    buff[offset+1] = (x >> 16) & 0xFF;
    buff[offset+2] = (x >> 8) & 0xFF;
    buff[offset+3] = x & 0xFF;
}

/*
 * Put as short into the buffer in big-endian byte order.
 */
void
put_short(char *buff, int offset, short x) {
    buff[offset] = (x >> 8) & 0xFF;
    buff[offset+1] = x & 0xFF;
}

/*
 * Get an integer from a buffer in big-endian byte order.
 */
int
get_int(char *buff, int offset) {
    int result = (buff[offset] << 24) |
		 ((buff[offset+1] << 16) & 0xFF0000) |
		 ((buff[offset+2] << 8) & 0xFF00) |
		 (buff[offset+3] & 0xFF);
    return result;
}

/*
 * Get a short from a buffer in big-endian byte order.
 */
short
get_short(char *buff, int offset) {
    short result = (buff[offset] << 8) |
		 (buff[offset+1] & 0xFF);
    return result;
}


void
trace_verbose(const char *format, ...) 
{
#ifdef DO_TRACE
    va_list ap;
    va_start(ap, format);
    check_initialized();
    if (verbose_tracing) {
	if (tracefile != NULL) {
	   fprintf(tracefile, "Plugin: ");
	    vfprintf(tracefile, format, ap);
	    fflush(tracefile);
	} else {
	    fprintf(stderr, "Plugin: ");
	    vfprintf(stderr, format, ap);
	}
    }
    va_end(ap);
#else
    UNUSED(format);
#endif
}


void
trace(const char *format, ...) 
{
    va_list ap;
    va_start(ap, format);
    check_initialized();
    if (tracing) {
	if (tracefile != NULL) {
	  fprintf(tracefile, "Plugin: ");
	  vfprintf(tracefile, format, ap);
	  fflush(tracefile);
	} else {
	    fprintf(stdout, "Plugin: ");
	    vfprintf(stdout, format, ap);
	}
    }
    va_end(ap);
}

/* Perform a dlsym(libhandle,fn_name), with error checking for the result */
void*
load_function(void* libhandle, const char* fn_name) {
    void* result_fn;
    if (libhandle == NULL) 
	plugin_error("Cannot load %s from null library\n", fn_name);
    result_fn = (void *)dlsym(libhandle,  fn_name);
    if (result_fn == NULL) 
	plugin_error("Could not load the function %s\n", fn_name);
    return result_fn;
}

/* A formal error is one that we attempt to localize (maybe?) */
void
plugin_formal_error(const char *s) {
    fprintf(stderr, "%s\n", DGETTEXT(JAVA_PLUGIN_DOMAIN, s));
    trace("%s\n", s);
}

/* A formal error that is not localized (such as file name */
void
plugin_raw_formal_error(const char *s) {
    fprintf(stderr, "%s\n", s);
    trace("%s\n", s);
}

/* Any other plugin error */
void
plugin_error(const char *format, ...) 
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "INTERNAL ERROR on Browser End: ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    perror("System error?:");
    va_end(ap);
    exit(-1);
}

/* Open trace file */
FILE *fopentrace(char *fname)
{
    FILE *fp;
    char trace_buffer[50];
    uid_t userid;
    struct passwd *user_passwd;
    
    userid = getuid();
    user_passwd = getpwuid (userid);
    
    sprintf (trace_buffer, "%s%s%s", fname, user_passwd->pw_name, ".trace");
    fp = fopen(trace_buffer, "w");
    	
    return fp;
}






