/*
 * @(#)jpda.h	1.7 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */



#ifndef JPDA_H
#define JPDA_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "system.h"
#include "configurationFile.h"



/*
 * If JPDA is defined (i.e., through -DJPDA=... on make command line),
 * then its value determines which Java process(es) will be a "debuggee"
 * (i.e., will be started in JPDA debug mode) -- Java Web Start only,
 * the launched Java application only, or both.
 * The only possible values for JPDA are:  JWS , JNL , JWSJNL (see
 * <JAVAWS_HOME>/build/share/JPDA.gmk):
 */
#define JWS	1		/* JPDA==JWS: only debug JWS (JavaWebStart) */
#define	JWSJNL	2		/* JPDA==JWSJNL: debug both JWS & JNL */
#define JNL	3		/* JPDA==JNL: only debug JNL (launched app) */

/* Config related string constants: */
#define CFG_JPDA_OPTIONS	"javaws-jpda.cfg.options"
#define CFG_JPDA_FILENAME	"javaws-jpda.cfg"

/* Ports pool related constants: */
#define MAXPORTS	256	/* max. nr. of ports in simultaneous use */
#define MAXPORTNR	0xFFFF	/* 65535 (accepted standard, see R.Stevens) */
#define POOLFULL	-1	/* error: pool is full */
#define POOLDUPL	0	/* inaction: port already in pool */
#define NAPN		-1	/* not a port number or port not available*/

/* Convenience macros for variable jpdaOptions and its type JpdaOptions*
   (both mostly used in "launcher.c"): */
#ifdef JPDA
#define  JPDA_OPTIONS		  jpdaOptions
#define _JPDA_OPTIONS		, jpdaOptions
#define  JPDA_OPTIONS_T		  JpdaOptions*
#define _JPDA_OPTIONS_T		, JpdaOptions*
#else
#define  JPDA_OPTIONS
#define _JPDA_OPTIONS
#define  JPDA_OPTIONS_T
#define _JPDA_OPTIONS_T
#endif

#define MIN(x,y) ((x) < (y) ? (x) : (y))




typedef struct {
	int	ports[MAXPORTS];/* pool of manually proposed ports */
	char	portsList[6*MAXPORTS]; /* comma-separated list of ports[] */
	int	fill;		/* number of manually proposed ports */
	int	selectedPort;	/* selected (manually or automatically) port */
	int	autoSelected;	/* TRUE if port was automatically selected */
} PortsPool;



typedef struct {
	int	jpdaMode;	/* TRUE if JavaWS to run in JPDA debugging mode
				   (set in DecodeJpdaOptions()), else FALSE */
	char*	in;		/* "<ports>[:classic]" (debugging directive) */
	char*	in_copy;	/* "<ports>[:classic]" (copy of pntr 'in') */
	char*	out;		/* "-Xrunjdwp:transport=dt_socket,server=y,address=<selectedport>,suspend=y" */
	int	classic;	/* TRUE if ":classic" was specified in 'in' */
	int	cmdLineArgIndex;/* index of command line arg  "-jpda[:...]";
				   -1 if not specified */
	int	jreIndex;	/* index of JRE to which JpdaOptions apply */

	PortsPool*	portsPool;
} JpdaOptions;



typedef struct {
	int	argc;	/* 1 + nr. of arguments to Java main class */
	char*	args;	/* comma-separated list of arguments to be passed
			   space-separated to Java main class */
} JavaMain;


void Help_JpdaCommandLineOption		(FILE*);
void Help_JpdaDebuggingDirective	(FILE*);
JpdaOptions* GetJpdaOptions		(void);
JpdaOptions* InitJpdaOptions		(JpdaOptions*);
JavaMain* GetJavaMain			(void);
JavaMain* InitJavaMain			(JavaMain*, int);
void HandleJpdaCommandLineArg		(char*, int, JpdaOptions*);
void LoadJpdaConfigurationFile		(void);
char* GetJpdaCfgOptions			(void);
void  DecodeJpdaOptions			(JpdaOptions*);
char* GetJpdaEnvOption			(JpdaOptions*, JavaMain*);
void  ShowJpdaNotificationWindow	(char*, int);

PortsPool* GetPortsPool			(void);
PortsPool* InitPortsPool		(PortsPool*);
int JdpaAddPort				(PortsPool*, int);
int String2Port				(char*);
PortsPool* String2PortsPool		(PortsPool*, char*, const char*);
int GetAvailableServerPort		(PortsPool*, int);

#endif JPDA_H
