/*
 * @(#)jpda.c	1.11 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/****************************************************************************
 *
 *		+========+======================+
 *		|  MODE  |  FLAG PASSED TO JRE  |
 *		+========+============+=========+
 *		| JPDA   | -Djnlpx.   | -Xdebug |
 *		| value  |   jpda.env |         |
 *		+--------+------------+---------+
 *		| ???    | ~          | ~       |
 *		| JWS    | <-         | <-      |
 *		| JWSJNL | <-         | <->     |
 *		| JNL    | <-         | >       |
 *		+--------+------------+---------+
 *
 *		???  : variable JPDA un#define'd
 *
 *		-Djnlpx.jpda.env=...	: JPDA debugging environment
 *					  for Java Web Start; contains
 *					  static items (e.g., list of
 *					  proposed ports) and dynamic
 *					  items (e.g., selected port);
 *					  only dynamic environment items
 *					  may change as the environment
 *					  is passed on to the next JRE
 *
 *		-Xdebug			: causes JRE to run in JPDA
 *					  debugging mode
 *
 *		~    : none of the JREs in a JRE invocation chain
 *		<->  : all JREs in a JRE invocation chain
 *		<-   : all JREs in a JRE invocation chain, except last JRE
 *		>    : only last JRE in a JRE invocation chain
 *
 *		In the current version of Java Web Start, the smallest JRE
 *		invocation chain comprises one JRE, and the largest JRE
 *		invocation chain comprises two JREs.
 *
 ****************************************************************************/


#include "jpda.h"
#include "launcher.h"
#include "propertyParser.h"


static PropertyFileEntry* CfgFileHead_jpda = NULL;




void Help_JpdaCommandLineOption(FILE* out) {
	fprintf(out, "\n\
Syntax and semantics for  -jpda  option to Java Web Start's command-line\n\
launcher:\n\n\
   -jpda:help                    --   Print this message\n\
   -jpda:<debugging-directive>   --   Attempt debugging (see below)\n\
   -jpda:    (or  -jpda)         --   Disable debugging\n\n\
Only these three forms are valid.  The 3rd form is a special case of the 2nd\n\
form; the debugging directive is void, hence debugging is disabled.\n\n\
----\n\n");
	Help_JpdaDebuggingDirective(out);
}



void Help_JpdaDebuggingDirective(FILE* out) {
	fprintf(out, "\
On start up, Java Web Start looks for a debugging directive specified\n\n\
    on the command-line    --   -jpda:<debugging-directive>\n\
    in \"deployment.properties.jpda\"   --   deployment.javaws.jre.debug.options=<debugging-directive>\n\n\
If both debugging directives are specified and both are valid, then the\n\
directive specified on the command-line overrides the directive specified\n\
in \"deployment.properties.jpda\".  Thus, a valid but void debugging directive on the\n\
command-line (through  -jpda:  [disable debugging]) forces any JREs (\"java\"\n\
executables) invoked in Java Web Start -- including the JRE running Java Web\n\
Start itself -- to run in regular mode rather than debugging mode, even if a\n\
valid debugging directive was specified in \"deployment.properties.jpda\" and it would\n\
in absence of a (valid) debugging directive on the command-line have led to\n\
debugging mode; this is a convenient way to disable any debugging directive\n\
that may have been specified in \"deployment.properties.jpda\", without having to edit\n\
or delete this file manually.  Another important note: when the Application\n\
Manager or a browser launches a JNLP application, they do so by invoking the\n\
command-line launcher; they don't provide the  -jpda  option to the launcher,\n\
hence to enable debugging under those circumstances one must make sure that\n\
\"deployment.properties.jpda\" exists and contains a valid debugging directive.\n\n\
<debugging-directive> causes JREs invoked in Java Web Start to execute in\n\
debugging mode, subject to NOTE below.\n\n");

	fprintf(out, "\
Syntax and semantics for  <debugging-directive> :\n\n\
    <ports>[:classic]      --  Example:   9110,9111,9112:classic\n\n\
  <ports>  --  Must comprise a list of tokens, delimited by spaces, tabs\n\
and/or commas.  Those and only those tokens that represent a non-negative\n\
integer are considered valid ports.  These valid ports are stored in a so-\n\
called \"ports pool\" (with any duplicates automatically eliminated), from\n\
which a port selection manager will attempt to select the first available\n\
port.  In case all ports contained in the ports pool are either currently\n\
occupied or reserved by the underlying OS, the port selection manager will\n\
attempt to autoselect the first available ephemeral port.  If autoselection\n\
fails, JREs will run in regular mode instead of debugging mode.  If the ports\n\
pool is empty (i.e., if no valid ports were specified), then the debugging\n\
directive is considered invalid and port autoselection isn't even tried; JREs\n\
will run in regular mode instead of debugging mode.  Port autoselection can\n\
be forced by specifying valid ports that will not be selected, e.g., any ports\n\
reserved by the underlying OS (typically ports 0 to 1023).\n\n\
  classic  --  Causes JREs when invoked in debugging mode to use the option\n\
-classic.  For 1.3.x JREs, this results in the Classic VM being used instead\n\
of the HotSpot VM; for other JRE releases, the  -classic  option is ignored.\n\n"
#ifdef JPDA
"----\n\nNOTE:  In this release of Java Web Start, "
#if (JPDA == JWS)
"all JREs in a JRE invocation\n\
chain can run in JPDA debugging mode, except the last JRE (running a JNLP\n\
application); the latter always runs in regular mode.\n\n"
#endif
#if (JPDA == JWSJNL)
"all JREs in a JRE invocation\n\
chain can run in JPDA debugging mode, including the last JRE (running a JNLP\n\
application).\n\n"
#endif
#if (JPDA == JNL)
"of all the JREs in a JRE invocation\n\
chain, only the last JRE (running a JNLP application) can run in JPDA\n\
debugging mode; all others always run in regular mode.\n\n"
#endif
"JRE invocation chains in this release of Java Web Start comprise either only\n\
one JRE or two JREs.\n\n"
#endif
	);
}



JpdaOptions* GetJpdaOptions() {
	static JpdaOptions jpdaOpts;
 	return &jpdaOpts;
}



JpdaOptions* InitJpdaOptions(JpdaOptions* jpdaOpts) {
	jpdaOpts->jpdaMode		= FALSE;
	jpdaOpts->in			= NULL;
	jpdaOpts->in_copy		= NULL;
	jpdaOpts->out			= NULL;
	jpdaOpts->classic		= FALSE;
	jpdaOpts->cmdLineArgIndex	= -1;
	jpdaOpts->jreIndex		= 0;
	jpdaOpts->portsPool		= InitPortsPool(GetPortsPool());
	return jpdaOpts;
}



JavaMain* GetJavaMain() {
	static JavaMain javaMain;
	return &javaMain;
}



JavaMain* InitJavaMain(JavaMain* javaMain, int argc) {
	javaMain->argc	= argc;   /* 1 + nr. arguments to Java main class */
	javaMain->args	= malloc(argc*MAXPATHLEN);
	*javaMain->args = '\0';
	return javaMain;
}



void HandleJpdaCommandLineArg(char* arg, int index, JpdaOptions* jpdaOpts) {
/* Argument arg (= argv[index], with argv as in "launcher.c") is assumed
   to point to a command-line option string starting with "-jpda".  If
   arg represents valid JPDA debugging options  --  i.e., if it is of the
   form "-jpda [: ...]", where a space denotes zero or more whitespaces,
   the ellipsis denotes any sequence of non-whitespace characters, and
   the [] denotes optional inclusion  --  then any value that jpdaOpts->in
   may have at this point   (namely the return value of the function
   GetCfgJpdaOptions() called in "launcher.c":main(), see first few
   statements in "launcher.c"main()'s definition)   will be overridden;
   thus, a valid debugging option on the command line will cause any
   debugging directive that may have been specified in "deployment.properties.jpda"
   to be suppressed: */

	/* Point jpdaOpts->in to arg+5, i.e., to character
	   immediately following "-jpda" in "-jpda [: ...]",
	   then skip any whitespace: */
	jpdaOpts->in = arg + 5;
	while (iswspace(*jpdaOpts->in)) ++jpdaOpts->in;

	/* If jpdaOpts->in points to ": ...", skip over
	   ':' and any whitespace following that, pointing
	   jpdaOpts->in to first non-whitespace character;
	   also, mark argv[i] as a valid "-jpda..." option: */
	if (*jpdaOpts->in == ':') {
		while (iswspace(*++jpdaOpts->in)) ;
		/* "-jpda : help"  prints explanation and exits: */
		if (!strcmp("help", jpdaOpts->in)) {
			Help_JpdaCommandLineOption(stderr);
			exit(0);
		}
		jpdaOpts->cmdLineArgIndex = index;
	}
	/* Else if jpdaOpts->in points to '\0', leave it as
	   is; also, mark argv[i] as a valid "-jpda..." option: */
	else if (*jpdaOpts->in == '\0') {
		jpdaOpts->cmdLineArgIndex = index;
	}
	/* Else, invalid "-jpda..." option (e.g., "-jpdaaa:9110:classic"),
	   hence restore jpdaOpts->in to point to original value; also,
	   don't update jpdaOpts->cmdLineArgIndex so it will remain -1
	   (initialization value), which will cause arg to be automatically
	   passed on -- see "launcher.c":LaunchJava()'s definition -- as an
	   argument to Java main class "com.sun.javaws.Main" when launched: */
	else {
		jpdaOpts->in = jpdaOpts->in_copy;
	}
}



/* Similar to LoadConfigurationFile() defined in "configurationFile.c" */
void LoadJpdaConfigurationFile() {
	LoadCfgFile(CFG_JPDA_FILENAME, &CfgFileHead_jpda);
	/* don't bail out if file not found */
}



char* GetJpdaCfgOptions() {
	return GetPropertyValue(CfgFileHead_jpda, CFG_JPDA_OPTIONS);
}



/* Decodes jpdaOpts->in, assumed to be either NULL or to represent a string
 * of the form "<string1> [: <string2>]", where a space denotes zero or
 * more whitespaces.  The result is: jpdaOpts->out and jpdaOpts->classic.
 * A more detailed spec of intended behavior follows:
 *
 *    If jpdaOpts->in == NULL, jpdaOpts->out and jpdaOpts->classic will
 * be set to NULL and FALSE, respectively, hence they basically retain
 * their original init values.  Also, the function returns at this point,
 * so jpdaOpts->jpdaMode remains FALSE (init value), i.e., debugging
 * remains disabled.
 *    In the special case that jpdaOpts->in == "" or == ": <string2>"
 * (in both cases implying that <string1> == ""), jpdaOpts->out will
 * be (re)set to "" and jpdaOpts->classic to FALSE (unchanged).  Also,
 * the function returns at this point, so jpdaOpts->jpdaMode remains
 * FALSE (init value), i.e., debugging remains disabled.
 *    jpdaOpts->jpdaMode will be set to TRUE only upon successful port
 * selection (by GetAvailableServerPort()); Java Webstart will then run
 * in JPDA debugging mode.  Port selection heuristics are as follows:
 *    <string1> is assumed to comprise a number of tokens, delimited
 * by spaces, tabs and/or commas.  Those (and only those) tokens that
 * represent a non-negative int are considered valid, user-proposed
 * ports (manually proposed through either the command line option
 * "-debug[ : ...]" or through the "deployment.javaws.jre.debug.options"
 * property in config file "deployment.properties.jpda").  The first proposed port
 * that is available (i.e., not busy) and successfully selected will be
 * used.  If none of the proposed ports are available (i.e., if all are
 * busy), the system will automatically try to select an ephemeral port
 * that is available.  If this is the case and that port is successfully
 * selected, that port will be used.  Otherwise (i.e., if both manual
 * and automatic port selection failed at this point), jpdaOpts->jpdaMode
 * will retain its original value (FALSE); Java Webstart will not run in
 * in JPDA debugging mode.
 *    If <string2> == "classic", jpdaOpts->classic will be set to TRUE
 * *iff* a port can be selected successfully.
 *    (Note: To enforce automatic port selection, one could just manually
 * propose a non-available port, e.g., port 0.)
*/
void DecodeJpdaOptions(JpdaOptions* jpdaOpts) {
	int i, j, len;
	static char c, buf[MAXPATHLEN];
	PortsPool* pp = jpdaOpts->portsPool;

	if (jpdaOpts->in == NULL) {
		jpdaOpts->out = NULL;
		jpdaOpts->classic = FALSE;
		return;
	}
	/* ( Note: jpdaOpts->in will be pointing to
	     non-whitespace character at this point;
	     see ScanFileArgumentForOptions() ) */
	if (*jpdaOpts->in == '\0' || *jpdaOpts->in == ':') {
		c = '\0';  jpdaOpts->out = &c;
		jpdaOpts->classic = FALSE;
		return;
	}

	if ((len = strlen(jpdaOpts->in)) > MAXPATHLEN) {
		len = MAXPATHLEN;
	}

	/* Copy jpdaOpts->in to buf[] -- but if ':' is found
	   before string end then (1) replace it by '\0' in buf[]
	   and (2) if substring beyond ':' (and any following white-
	   space) equals "classic" then set boolean jpdaOpts->classic
	   to TRUE: */
	for (i = 0; i < len; i++) {
		if (jpdaOpts->in[i] == ':') {
			j = i;  /* next, skip over ':' and any whitespace: */
			while (iswspace(jpdaOpts->in[++j])) ;
			jpdaOpts->classic = !strcmp("classic", jpdaOpts->in + j);
			break;
		}
		buf[i] = jpdaOpts->in[i];
	}
	buf[i] = '\0';

	String2PortsPool(pp, buf, ", \t");
	if (pp->fill == 0 || (i = GetAvailableServerPort(pp, TRUE), 
	    pp->selectedPort = i) < 0) {
		/* disable debugging: */
		c = '\0';  jpdaOpts->out = &c;
		jpdaOpts->classic = FALSE;
	}
	else {
		/* use selected port: */
		sprintf(buf,	"-Xrunjdwp:transport=dt_socket,server=y"
				",address=%d,suspend=y",
			pp->selectedPort);
		jpdaOpts->out = buf;

		jpdaOpts->jpdaMode = TRUE;
	}
}



/* Starts a JRE in regular (non-debugging) mode in order to notify the
 * user that another JRE has been started in debugging mode and that it
 * is waiting for a connection from a JPDA-compliant debugger to receive
 * instructions */
void ShowJpdaNotificationWindow(char* jpdaEnv, int jreIndex) {
	int n = 0, totalargs = 7;
	char** javaargv = (char**)malloc(sizeof(char*) * totalargs);

	javaargv[n++] = GetJREJavaCmd(jreIndex);
	javaargv[n++] = "-classpath";
	javaargv[n++] = GetClassPath();
	/* This serves as a "do nothing more than just notify" signal for
	   "com.sun.javaws.Main" to be run by the JRE that is going to be
	   invoked by sysExec() below: */
	javaargv[n++] = "-Djpda.notification";
	javaargv[n++] = jpdaEnv;
	javaargv[n++] = "com.sun.javaws.Main";
	javaargv[n++] = NULL;
	assert(n == totalargs, "wrong memory allocation");

	if (sysExec(SYS_EXEC_FORK, javaargv[0], javaargv) == -1) {
		char *msg = malloc(2*MAXPATHLEN); 
		sprintf(msg, "%s \n%s",getMsgString(MSG_BADINST_SYSEXE),
			javaargv[0]); 
		Abort(msg);
	}
}



/* Called by LaunchJava() -- in "launcher.c" from within  #ifdef JPDA ...
 * ... #endif  block.
 * If we are in debugging mode, return the JPDA debugging environment as
 * a system property definition for the "java" executable to be invoked;
 * it includes: (1) the information required by the invoked "java" executable
 * to, if necessary, recursively launch a new "java" process in debugging
 * mode (using the next available port); (2) some diagnostic information
 * for the debugging mode notification window shown to the user.  If we
 * are not in debugging mode, "-Djnlpx.jpda.env" (empty environment) is
 * returned.
 */
char* GetJpdaEnvOption(JpdaOptions* jpdaOpts, JavaMain* javaMain) {
	char		*plist, *alist, *env;
	PortsPool	*pp = jpdaOpts->portsPool;

	if (!jpdaOpts->jpdaMode) {
		return "-Djnlpx.jpda.env";	/* empty environment */
	}

	env = malloc((javaMain->argc + 2) * MAXPATHLEN);
	sprintf(env,
		"-Djnlpx.jpda.env="
		"debuggeeType=%d"
		"&jpdaConfigIsFromCmdLine=%s"
		"&portsList=%s"
		"&selectedPort=%d"
		"&portIsAutoSelected=%s"
		"&excludedportsList=NONE"
		"&jreProductVersion=%s"
		"&jreNestingLevel=0"
		"&jreUsesDashClassic=%s"
		"&javaMainArgsList=%s",
#ifdef JPDA
		JPDA,	/*  1 = JWS, 2 = JWSJNL, 3 = JNL  */
#else
		0,	/*  dummy */
#endif
		(jpdaOpts->cmdLineArgIndex > 0 ? "1" : "0"),
		(*(plist = pp->portsList) == '\0' ? "NONE" : plist),
		pp->selectedPort,
		(pp->autoSelected ? "1" : "0"),
		GetJREProductVersion(jpdaOpts->jreIndex),
		(jpdaOpts->classic ? "1" : "0"),
		(*(alist = javaMain->args) == '\0' ? "NONE" : alist));

	return env;
}


/****************************************************************************/

/*			PORTS  POOL  MANAGER
 *
 * This section provides functionality to manage a pool of ports,
 * allowing the selection of a port not already in use.  This is
 * useful in Java Web Start when various Java applications (Java
 * Web Start itself or Java apps launched by it) are to be de-
 * bugged using JPDA.  The maximum value of a port stored in the
 * ports pool is MAXPORTNR=65535 (5 digits max); input values
 * larger than this are automatically stored as MAXPORTNR.
 *
 * If GetAvailableServerPort() > 0, then from Java Web Start's
 * perspective this means that it is running in JPDA debugging
 * mode; see "src/share/native/launcher.c" for further details.
 */


PortsPool* GetPortsPool() {
	static PortsPool portsPool;
 	return &portsPool;
}



PortsPool* InitPortsPool(PortsPool* pp) {
	pp->fill = 0;
	pp->portsList[0] = '\0';
	pp->selectedPort = NAPN;
	pp->autoSelected = FALSE;
	return pp;
}


/*
 * Private function -- to be used by String2PortsPool().
 */
int JpdaAddPort(PortsPool* pp, int port) {
	int i;
	static char buf[7];  /* 7 = 1 ','  +  5-digits-max port#  +  1 '\0' */

	if (pp->fill == 0) {
		pp->ports[pp->fill++] = port;
		sprintf(buf, "%d", port);
		strcat(pp->portsList, buf);
		return port;
	}

	if (pp->fill >= MAXPORTS) {
		return POOLFULL;
	}

	for (i = 0; i < MAXPORTS; i++) {
		if (port == pp->ports[i]) {
			return POOLDUPL;
		}
		if (i == pp->fill) {
			pp->ports[pp->fill++] = port;
			sprintf(buf, ",%d", port);
			strcat(pp->portsList, buf);
			return port;
		}
	}
	return POOLFULL;
}



/*
 * Private function -- to be used by String2PortsPool().
 * Converts str to an integer port number if str represents a non-
 * negative integer.  Returns NAPN if str doesn't represent an
 * integer.  Returns MAXPORTNR if str represents an integer larger
 * than MAXPORTNR
 */
int String2Port(char *str) {
	char* s = str;
	long  n;

	for (; *s != '\0'; s++)
		if (!isdigit(*s)) {
			return NAPN;
		}
	n = strtol(str, (char**)NULL, 10);
	if (errno == ERANGE) {
		errno = 0;
		return MAXPORTNR;
	}

	return (int)MIN(n, (long)MAXPORTNR);
}



PortsPool* String2PortsPool(PortsPool* pp, char* string, const char* delimiters) {
	char *str = NULL, *s;
	int p;

	if (string == NULL || *string =='\0') {
		return pp;
	}

	str = strdup(string);

	for (s = strtok(str, delimiters); s != NULL;
			s = strtok(NULL, delimiters))
		if ((p = String2Port(s)) != NAPN) {
			JpdaAddPort(pp, p);
		}
	if (str != NULL) {
		free(str);
	}

	return pp;
}



/*
 * First looks in the ports pool (= return value of GetPortsPool())
 * for an available (unused) server port.  If found, it is returned.
 * If not found in the pool and dynamic==TRUE, dynamic (automatic)
 * allocation of an ephemeral port is tried.  Upon success, that
 * allocated port is returned.  If up to this point everything has
 * failed, NAPN is returned.
 */
int GetAvailableServerPort(PortsPool* pp, int dynamic) {
	int i, p;

	pp->autoSelected = FALSE;

	for (i = 0; i < pp->fill; i++) {
		if ((p = pp->ports[i]) == 0) {
			continue;
		}
		if (sysTestServerSocketCreatable(&p) != INVALID_SOCKET) {
			return p;
		}
	}
	if (dynamic) {
		p = 0;
		if (sysTestServerSocketCreatable(&p) != INVALID_SOCKET) {
			pp->autoSelected = TRUE;
			return p;
		}
	}

	return NAPN;
}
