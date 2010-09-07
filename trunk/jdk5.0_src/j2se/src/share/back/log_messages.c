/*
 * @(#)log_messages.c	1.1 03/04/07
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"

#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>

#include "proc_md.h"

#include "log_messages.h"

#ifdef JDWP_LOGGING

#define MAXLEN_INTEGER 		20
#define MAXLEN_FILENAME 	256
#define MAXLEN_TIMESTAMP 	80
#define MAXLEN_LOCATION 	(MAXLEN_FILENAME+MAXLEN_INTEGER+16)
#define MAXLEN_MESSAGE  	256
#define MAXLEN_EXEC  		(MAXLEN_FILENAME*2+MAXLEN_INTEGER+16)

static MUTEX_T my_mutex = MUTEX_INIT;

/* Static variables (should be protected with mutex) */
static int logging;
static FILE * log_file;
static char logging_filename[MAXLEN_FILENAME+1+6];
static char location_stamp[MAXLEN_LOCATION+1];
static char precrash_exe[MAXLEN_FILENAME+1];
static PID_T processPid;
static int open_count;

/* Ascii id of current native thread. */
static void
get_time_stamp(char *tbuf, size_t ltbuf)
{
    char format[MAXLEN_TIMESTAMP+1];
    unsigned millisecs = 0;
    time_t t = 0;
    
    GETMILLSECS(millisecs);
    if ( time(&t) == (time_t)(-1) )
	t = 0;
    (void)strftime(format, sizeof(format),  
		/* Break this string up for SCCS's sake */
		"%" "d.%" "m.%" "Y %" "T.%%.3d %" "Z", localtime(&t));
    (void)snprintf(tbuf, ltbuf, format, (int)(millisecs));
}

/* Get basename of filename */
static const char *
file_basename(const char *file)
{
    char *p1;
    char *p2;

    if ( file==NULL )
	return "unknown";
    p1 = strrchr(file, '\\');
    p2 = strrchr(file, '/');
    p1 = ((p1 > p2) ? p1 : p2);
    if (p1 != NULL) {
	file = p1 + 1;
    }
    return file;
}

/* Fill in the exact source location of the LOG entry. */
static void
fill_location_stamp(const char *flavor, const char *file, int line)
{
    (void)snprintf(location_stamp, sizeof(location_stamp), 
		    "%s:\"%s\":%d;", 
		    flavor, file_basename(file), line);
    location_stamp[sizeof(location_stamp)-1] = 0;
}

/* Begin a log entry. */
void
log_message_begin(const char *flavor, const char *file, int line)
{
    MUTEX_LOCK(my_mutex); /* Unlocked in log_message_end() */
    if ( logging ) {
	location_stamp[0] = 0;
	fill_location_stamp(flavor, file, line);
    }
}

/* Standard Logging Format Entry */
static void
standard_logging_format(FILE *fp,
	const char *datetime,
	const char *level,
	const char *product,
	const char *module,
	const char *optional,
	const char *messageID,
	const char *message)
{
    const char *format;	
	
    /* "[#|Date&Time&Zone|LogLevel|ProductName|ModuleID|
     *     OptionalKey1=Value1;OptionalKeyN=ValueN|MessageID:MessageText|#]\n"
     */
    
    format="[#|%s|%s|%s|%s|%s|%s:%s|#]\n";
    
    (void)fprintf(fp, format,
	    datetime,
	    level,
	    product,
	    module,
	    optional,
	    messageID,
	    message);
}

/* End a log entry */
void
log_message_end(const char *format, ...)
{
    if ( logging ) {
	va_list ap;
        THREAD_T tid;
	char datetime[MAXLEN_TIMESTAMP+1];
	const char *level;
	const char *product;
	const char *module;
	char optional[MAXLEN_INTEGER+6+MAXLEN_INTEGER+6+MAXLEN_LOCATION+1];
	const char *messageID;
	char message[MAXLEN_MESSAGE+1];

	/* Grab the location, start file if needed, and clear the lock */
	if ( log_file == NULL && open_count == 0 && logging_filename[0] != 0 ) {
	    open_count++;
	    log_file = fopen(logging_filename, "w");
	    if ( log_file!=NULL ) {
		(void)setvbuf(log_file, NULL, _IOLBF, BUFSIZ);
	    } else {
		logging = 0;
	    }
	}
	
	if ( log_file != NULL ) {
	    
	    /* Get the rest of the needed information */
	    tid = GET_THREAD_ID();
	    level = "FINEST"; /* FIXUP? */
	    product = "J2SE1.5"; /* FIXUP? */
	    module = "jdwp"; /* FIXUP? */
	    messageID = ""; /* FIXUP: Unique message string ID? */
	    (void)snprintf(optional, sizeof(optional), 
			"LOC=%s;PID=%d;THR=t@%d", 
			location_stamp,
			(int)processPid,
			(int)tid);
	    
	    /* Construct message string. */
	    va_start(ap, format);
	    (void)vsnprintf(message, sizeof(message), format, ap);
	    va_end(ap);

	    get_time_stamp(datetime, sizeof(datetime));
	    
	    /* Send out standard logging format message */
	    standard_logging_format(log_file,
		datetime,
		level,
		product,
		module,
		optional,
		messageID,
		message);
	}
	location_stamp[0] = 0;
    }
    MUTEX_UNLOCK(my_mutex); /* Locked in log_message_begin() */
}

/* Signal action handler function. */
static void
signal_handler(int sig)
{
    finish_logging(1);
}

/* Setup a signal action handler for this signal. */
static void
setup_signal_handler(int sig)
{
    (void)signal(sig, (void(*)(int))(void*)&signal_handler);
}

#endif

/* Set up the logging with the name of a logging file. */
void
setup_logging(const char *filename, unsigned flags, const char *precrash, ...)
{
#ifdef JDWP_LOGGING
    FILE *fp = NULL;
    
    /* Turn off logging */
    logging = 0;
    gdata->log_flags = 0;
   
    /* Just return if not doing logging */
    if ( filename==NULL || flags==0 )
	return;
    
    /* Create potential filename for logging */
    processPid = GETPID();
    (void)snprintf(logging_filename, sizeof(logging_filename), 
		    "%s.%d", filename, (int)processPid);
    
    /* Save precrash name and setup signal handlers */
    if ( precrash!=NULL ) {
	va_list ap;
	int sig;
	size_t maxlen = sizeof(precrash_exe)-1;
	(void)strncpy(precrash_exe, precrash, maxlen);
	precrash_exe[maxlen] = 0;
	va_start(ap, precrash);
	sig = va_arg(ap, int);
	while ( sig > 0 ) {
	    setup_signal_handler(sig);
	    sig = va_arg(ap, int);
	}
	va_end(ap);
    }
    
    /* Turn on logging (do this last) */
    logging = 1;
    gdata->log_flags = flags;

#endif
}

/* Finish up logging, sending precrash output to the logfile. */
void
finish_logging(int exit_code)
{
#ifdef JDWP_LOGGING
    MUTEX_LOCK(my_mutex);
    if ( logging ) {
        logging = 0;
	if ( log_file != NULL ) {
	    (void)fflush(log_file);
	    (void)fclose(log_file);
	    log_file = NULL;
	}
	if ( exit_code!=0 && precrash_exe[0]!=0 && logging_filename[0]!=0 ) {
	    FILE *fp; 
	    fp = fopen(precrash_exe, "r");
	    if ( fp != NULL ) {
		char buf[MAXLEN_EXEC+1];
		(void)fclose(fp);
		(void)snprintf(buf, sizeof(buf), "%s -p %d -o %s", 
			precrash_exe, (int)processPid, logging_filename);
		fp = popen(buf, "r");
		if (fp != NULL) {
		    (void)pclose(fp);
		}
	    }
	} 
    }
    MUTEX_UNLOCK(my_mutex);
#endif
}

