/*
 * @(#)launcher.c	1.182 10/03/24
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * Java Web Start launcher
 * -----------------------
 *
 * Synopsis:
 *    
 * Usage: javaws [run-options] <jnlp-file> 
 *        javaws [control-options]
 * 
 * where run-options include:
 *   -verbose           display additional output
 *   -offline           run the application in offline mode
 *   -system            run the application from the system cache only
 *   -Xnosplash         run without showing a splash screen
 *   -J<options>        supply options to the vm
 *   -wait              start java process and wait for its exit
 *   -online            --- internal option ---
 *   -localfile         --- internal option ---
 *   -docbase           --- internal option ---
 * 
 * control-options include:
 *   -viewer            show the cache viewer in the java control panel
 *   -uninstall         remove all applications from the cache
 *   -uninstall <jnlp-file>                remove the application from the cache
 *   -import [import-options] <jnlp-file>  import the application to the cache
 *   -shortcut                           --- internal option ---
 *   -association                        --- internal option ---
 *   -Xclearcache                        --- internal option ---
 *   javaws -splash <port> <image file>  --- internal option ---
 *   -quick                              --- internal option ---
 * 
 * import-options include:
 *   -silent            import silently (with no user interface)
 *   -system            import application into the system cache
 *   -codebase <url>    retrieve resources from the given codebase
 *   -shortcut          install shortcuts as if user allowed prompt
 *   -association       install associations as if user allowed prompt
 *   -timestamp <MM/dd/yy hh:mm a>      --- internal option ---
 *   -expiration <MM/dd/yy hh:mm a>     --- internal option ---
 *   -quiet                             --- internal option ---
 */

#include "launcher.h"
#include "configurationFile.h"

/*  Reads the filename to load from from the passed in file. It is the callers
 *  responsibility to free the returned string. This will return NULL if
 *  there is a problem determining the filename.
 */
char *ReadFileNameFromFile(char *file) {
  char *contents ;
  int size = ReadFileToBuffer(file, &contents);

  if (contents != NULL) {
    char *ptr = contents;

    while (*ptr != '\r' && *ptr != '\n' && *ptr != '\0') {
        ptr++;
    }
    *ptr = '\0';
  }
  return contents;
}

/*
 * Splash Screen
 */

static int SplashPort = -1;
static int SplashPID = -1;

int GetSplashPort() {
    return SplashPort;
}

char * ShowSplashScreen(char* splash) {

    int port;      /* the port that splash process will callback on */
    SOCKET server; /* the socket that splash process will callback on */
    int ret = 0;

    /* Create a temporary socket on an ephemeral port.We'll pass this
     * port to the splash application (as a command line argument)
     * so that it can send us the ephemeral port it's going to use.
     */
    if ((server = sysCreateListenerSocket(&port)) == INVALID_SOCKET) {
        return getMsgString(MSG_LISTENER_FAILED);
    }

    /* Launch the splash screen application with the callback port and
     * splash screen jpeg file to use as it's arguments
     */
    {
        char str[MAXPATHLEN], *argv[8];
        char exe[MAXPATHLEN];

        sysStrNPrintF(exe, sizeof(exe), "%s%c%s", sysGetApplicationHome(),
                FILE_SEPARATOR, sysGetJavawsbin());

        sysStrNPrintF(str, sizeof(str), "%d", port);

        argv[0] = "JavaWSSplashScreen";
        argv[1] = "-splash";
        argv[2] = str;
        argv[3] = splash;
        argv[4] = NULL;
        ret = sysExec(SYS_EXEC_FORK, exe, argv);
    }

    /* Wait for the splash screen to connect and tell us what ephemeral
     * port it's listening on.
     */
    if (ret > 0 ) {
        SOCKADDR_IN iname = {0};
        int client, length = sizeof(iname);
        char data[6];

        if ((client = accept(server, (SOCKADDR *)&iname, &length)) ==
                INVALID_SOCKET) {
            return getMsgString(MSG_ACCEPT_FAILED);
        }
        if (recv(client, data, 6, 0) != 6) {
            return getMsgString(MSG_RECV_FAILED);
        }
        if ((SplashPort = atoi(data)) <= 0) {
            return getMsgString(MSG_INVALID_PORT);
        }

        sysCloseSocket(client);
        sysCloseSocket(server);
    } else {
        return getMsgString(MSG_BADMSG);
    }
    return NULL;
}


/*
 *
 * java_arg stuff
 *
 */

#define MAX_COUNT  50  /* 50 seems to be more than enough */
static char *java_args[MAX_COUNT + 1];
static int java_arg_strlen = 0;   /* strlen of concat .. */
static int java_arg_count = 1;   /* skip over executable path */
static char *java_arg_auxargs = NULL;
static char *java_arg_Filename = NULL;
static char *java_arg_command = NULL;

char *GetVMArgsOption(char *vmoptions) {
    char * option=NULL;
    int n;
    if ((vmoptions != NULL) && (strlen(vmoptions) < MAXPATHLEN)) {
        static char tmp[MAXPATHLEN + 16];
        if(sysGetQuotesWholePropertySpec()) {
            n=sysStrNPrintF(tmp, sizeof(tmp), "-Djnlpx.vmargs=%s", vmoptions);
            if(0<=n && n<sizeof(tmp)) {
                option=sysQuoteString(tmp);
            }
        } else {
            char *vmOptionsQ = sysQuoteString(vmoptions);
            n=sysStrNPrintF(tmp, sizeof(tmp), "-Djnlpx.vmargs=%s", vmOptionsQ);
            if(0<=n && n<sizeof(tmp)) {
                option=strdup(tmp);
            }
            free(vmOptionsQ);
        }
    }
    return option;
}

char *GetProfileOption(char *path) {
    if ((path != NULL) && (strlen(path) > 0) && (strlen(path) < MAXPATHLEN)) {
        static char tmp[MAXPATHLEN + 32];
        sysStrNPrintF(tmp, sizeof(tmp), "-Djavaplugin.user.profile=%s", path);
        return tmp;
    }
    return NULL;
}

char *GetStartTimeOption(int time) {
    if (time > 0) {
        static char tmp[MAXPATHLEN + 16];
        sysStrNPrintF(tmp, sizeof(tmp)-1, "-Djnlp.start.time=%d", time);
        return strdup(tmp);
    }
    return NULL;
}
    
char *GetPerfLogOption(char *perflog) {
    if (perflog != NULL) {
        static char tmp[MAXPATHLEN + 16];
        char * ptr = strstr(perflog, "=");
        sysStrNPrintF(tmp, sizeof(tmp)-1, "-Dsun.perflog%s", (ptr == NULL ? "=NULL" : ptr));
        return strdup(tmp);
    }
    return NULL;
}

char *GetLaunchTimeOption(char *launchTime) {
    if (launchTime != NULL) {
        static char tmp[MAXPATHLEN];
        char * ptr = strstr(launchTime, "=");
        sysStrNPrintF(tmp, sizeof(tmp), "-Djnlp.launchTime%s", (ptr == NULL ? "=NULL" : ptr));
        return tmp;
    }
    return NULL;
}

char* GetPluginBootClassPath(void) {
    static char tmp[2 * MAXPATHLEN];

    sysStrNPrintF(tmp, sizeof(tmp), "%s%c%s%c%s%c%s%c%s%c%s",
            sysGetJarLib(), FILE_SEPARATOR, "javaws.jar",
            PATH_SEPARATOR, sysGetJarLib(), FILE_SEPARATOR, "deploy.jar",
	    PATH_SEPARATOR, sysGetJarLib(), FILE_SEPARATOR, "plugin.jar");
    return tmp;
}

char* GetPluginBootClassPathArg(void) {
    static char tmp[MAXPATHLEN];
    sysStrNPrintF(tmp, sizeof(tmp), "-Xbootclasspath/a:%s",GetPluginBootClassPath());
    return tmp;
}

char* GetBootClassPath(void) {
    static char tmp[2 * MAXPATHLEN];

    sysStrNPrintF(tmp, sizeof(tmp), "%s%c%s%c%s%c%s",
            sysGetJarLib(), FILE_SEPARATOR, "javaws.jar",
            PATH_SEPARATOR, sysGetJarLib(), FILE_SEPARATOR, "deploy.jar");
    return tmp;
}

/* Get the arg for -Xbootclasspath */
char* GetBootClassPathArg(void) {
    static char tmp[MAXPATHLEN];
    sysStrNPrintF(tmp, sizeof(tmp), "-Xbootclasspath/a:%s",GetPluginBootClassPath());
    return tmp;
}

char * GetNonBootClassPath(void) {
    static char tmp[MAXPATHLEN];

    sysStrNPrintF(tmp, sizeof(tmp), "%s%c%s",
            sysGetJarLib(), FILE_SEPARATOR, "deploy.jar");
    return tmp;
}

/* get the entire classpath (boot, + regular) */
char * GetDeployJarPath(void) {
    static char tmp[MAXPATHLEN];

    sysStrNPrintF(tmp, sizeof(tmp), "%s%c%s",
            sysGetJarLib(), FILE_SEPARATOR, "deploy.jar");
    return tmp;
}

char* GetMozExtDir(char* mozExtDir) {
    static char tmp[MAXPATHLEN];
    if ((mozExtDir != NULL) && (strlen(mozExtDir) > 0)) {
        sysStrNPrintF(tmp, sizeof(tmp), "-Djava.ext.dirs=%s%c%s%c%s%c%s",
                        sysGetJarLib(),FILE_SEPARATOR, "ext", PATH_SEPARATOR,
                        mozExtDir, FILE_SEPARATOR, "jss");
        return tmp;
    }
    return NULL;
}
char* GetJnlpxHomeOption(void) {
    static char tmp[MAXPATHLEN];
    sysStrNPrintF(tmp, sizeof(tmp), "-Djnlpx.home=%s", sysGetApplicationHome());
    return tmp;
}

char* GetJnlpxSplashPortOption() {
    static char tmp[MAXPATHLEN];
    sysStrNPrintF(tmp, sizeof(tmp),  "-Djnlpx.splashport=%d", GetSplashPort());
    return tmp;
}

char* GetJnlpxJVMOption(char* path) {
    static char tmp[MAXPATHLEN];
    char * option=NULL;
    int n;
    if(sysGetQuotesWholePropertySpec()) {
        n = sysStrNPrintF(tmp, sizeof(tmp), "-Djnlpx.jvm=%s", path);
        if(0<=n && n<sizeof(tmp)) {
            option=sysQuoteString(tmp);
        }
    } else {
        char *pathQ = sysQuoteString(path);
        n = sysStrNPrintF(tmp, sizeof(tmp), "-Djnlpx.jvm=%s", pathQ);
        if(0<=n && n<sizeof(tmp)) {
            option=strdup(tmp);
        }
        free(pathQ);
    }
    return option;
}

char* GetJnlpxRemoveOption(int copiledFile) {
    static char option[MAXPATHLEN];
    sysStrNPrintF(option, sizeof(option), "-Djnlpx.remove=%s", (copiledFile) ? "true" : "false");
    return option;
}

char* GetSecurityPolicyOption(void) {
    static char option[MAXPATHLEN];
    sysStrNPrintF(option, sizeof(option), "-Djava.security.policy=file:%s%cjavaws.policy",
        sysGetSecurityLib(), FILE_SEPARATOR);
    return option;
}

char* GetTrustProxyOption(void) {
    static char option[MAXPATHLEN];
    sysStrNPrintF(option, sizeof(option), "-DtrustProxy=true");
    return option;
}

char* GetVerifierOption(void) {
    static char option[MAXPATHLEN];
    sysStrNPrintF(option, sizeof(option), "-Xverify:remote");
    return option;
}

char* GetHeadlessOption(void) {
    static char option[MAXPATHLEN];
    sysStrNPrintF(option, sizeof(option), "-Djava.awt.headless=true");
    return option;
}

void java_arg_addArg(char *arg, int max_totallen) {
    if (arg != NULL && java_arg_count < MAX_COUNT) {
        int argLen = strlen(arg);
        if ( 0<argLen && (java_arg_strlen+argLen)<max_totallen) {
            java_args[java_arg_count] = strdup(arg);
            java_arg_count++;
            java_arg_strlen+=argLen;
        }
    }
}

/**
 * @param arg - quoted, if necessary
 * @param max_totallen maximum length this arg can become, 
 *        note: it will be added 2 times: java_arg_auxargs and java_args!
 * @return string.length() <= MAXPATHLEN
 */
void java_arg_addAux(char *arg, int max_totallen) {
    if (strlen(arg) < max_totallen) {
        if (java_arg_auxargs == NULL) {
            java_arg_auxargs = strdup(arg);
        } else {
            int newLen = strlen(arg) + strlen(java_arg_auxargs) + 2;
            if(newLen<max_totallen) {
                char *new = malloc (newLen);
                sysStrNPrintF(new, newLen, "%s %s", java_arg_auxargs, arg);
                free(java_arg_auxargs);
                java_arg_auxargs = new;
            }
        }
        java_arg_addArg(arg, max_totallen);
    }
}

void java_arg_initArgs() {
    char *str;

    java_arg_addArg(GetBootClassPathArg(), CMDLNMAXLEN);
    java_arg_addArg("-classpath", CMDLNMAXLEN);
    java_arg_addArg(GetNonBootClassPath(), CMDLNMAXLEN);
    java_arg_addArg(GetSecurityPolicyOption(), CMDLNMAXLEN);
    java_arg_addArg(GetTrustProxyOption(), CMDLNMAXLEN);
    java_arg_addArg(GetVerifierOption(), CMDLNMAXLEN);
    java_arg_addArg(GetJnlpxHomeOption(), CMDLNMAXLEN);

    java_arg_addArg(GetProfileOption(getenv("USER_JPI_PROFILE")), CMDLNMAXLEN);
    java_arg_addArg(GetMozExtDir(getenv("MOZILLA_HOME")), CMDLNMAXLEN);

    // always add "AWT warmup" property
    // laucher may ignore this parameter depending on platform and other settings
    java_arg_addArg("-Dsun.awt.warmup=true", CMDLNMAXLEN);

    str = getenv("JAVAWS_VM_ARGS");
    if (str != NULL) {
        char *arg;
        while(*str) {
            /* Skip leading withspace */
            while(iswspace(*str)) str++;
            if (*str) {
                arg = str;
                /* skip argument (change first whitespace to 0) */
                while(*str && !iswspace(*str)) {
                    str++;
                }
                if (*str) {
                    *str++ = '\0';
                }
                java_arg_addAux(arg, CMDLNMAXLEN);
            }
        }
    }
}

void java_arg_setFile(char *filename, int remove) {
    java_arg_Filename = strdup(filename);
    java_arg_addArg(GetJnlpxRemoveOption(remove), CMDLNMAXLEN);
}

void java_arg_addHeap(char *initialHeap, char *maxHeap) {
    char str[MAXPATHLEN];
    if ((initialHeap != NULL) || (maxHeap != NULL)) {
        if (initialHeap != NULL && (strlen(initialHeap) < (MAXPATHLEN/4))) {
            sysStrNPrintF(str, sizeof(str), "-Xms%s",initialHeap);
            java_arg_addArg(str, CMDLNMAXLEN);
        } else {
            initialHeap = NULL;
        }
        if (maxHeap != NULL && (strlen(maxHeap) < (MAXPATHLEN/4))) {
            sysStrNPrintF(str, sizeof(str), "-Xmx%s",maxHeap);
            java_arg_addArg(str, CMDLNMAXLEN);
        } else {
            maxHeap = NULL;
        }
        sysStrNPrintF(str, sizeof(str), "-Djnlpx.heapsize=%s,%s",
                (initialHeap == NULL ? "NULL" : initialHeap),
                (maxHeap == NULL ? "NULL" : maxHeap));
        java_arg_addArg(str, CMDLNMAXLEN);
    }
}

/*
 * SingleInstance implementation
 */

int SetupSingleInstance(char *jnlpfile, char* canonicalHome,
                        int argc, char** argv) {
    FILE* sisIN = NULL;
    char* magicword = "javaws.singleinstance.init";
    char* magicword_openprint = "javaws.singleinstance.init.openprint";
    char* eof = "EOF";
    char* ack = "javaws.singleinstance.ack";
    char sispath[MAXPATHLEN];
    int port;
    int ret;
    char* ret_s;
    SOCKET s;
    int i;
    char* sisProperties;
    int size;
    char* arg1 = NULL;
    char* arg2 = NULL;
    char* number;
 
    int a = 0;

    while (a < argc - 1) {
        if (strcmp(argv[a], "-open") == 0 || strcmp(argv[a], "-print") == 0) {
            arg1 = argv[a];
            arg2 = argv[++a];
            break;
        }
        a++;
    }

    port = sysFindSiPort(canonicalHome);

    if (port != -1) {
        /* talk to single instance server */
        s = sysCreateClientSocket(port);
        if (s == INVALID_SOCKET) {
            return 0;
        }
        number = sysFindSiNumber(canonicalHome);
 
        if (number == NULL) {
            return 0;
        }   
         
        /* send si random number */
        ret = sysWriteSocket(s, number);
        if (ret == 0) {
            return 0;
        }
        /* send MAGICWORD */
        if (arg1 != NULL && arg2 != NULL) {
            ret = sysWriteSocket(s, magicword_openprint);
            if (ret == 0) {
                return 0;
            }
            /* send over the open print arguments */
            ret = sysWriteSocket(s, arg1);
            if (ret == 0) {
                return 0;
            }
            ret = sysWriteSocket(s, arg2);
            if (ret == 0) {
                return 0;
            }
        } else {
            ret = sysWriteSocket(s, magicword);
            if (ret == 0) {
                return 0;
            }
            /* send over the jnlp file */
            ret = sysWriteSocket(s, jnlpfile);
            if (ret == 0) {
                return 0;
            }
        }

        /* incicate EOF */
        ret = sysWriteSocket(s, eof);
        if (ret == 0) {
            return 0;
        }

        /* wait for ack from server */
        /* try read socket for 5 times - do we need to retry at all? */
        for (i = 0; i< 5; i++) {
            ret_s = sysReadSocket(s);

            if (ret_s != NULL && strcmp(ret_s, ack) == 0) {
                /* if acked, exit program */
                sysCloseSocket(s);
                return 1;
            }
        }
        sysCloseSocket(s);
    }
    return 0;
}

void showUsage() {
    sysMessage(getMsgString(MSG_JAVAWS_USAGE));
}

/*
 *
 * Main Entry Point.
 *
 */

int main(int argc, char** argv) {
    char* jreversion = NULL;
    char* jrelocation = NULL;
    int   jreIndex   = 0;
    int   copiedfile = FALSE;
    int   wait       = FALSE;
    int   silent     = FALSE;
    int   import     = FALSE;
    int   uninstall  = FALSE;
    int   isViewer   = FALSE;
    int   isSystem   = FALSE;
    int   doSplash   = TRUE;
    int   verbose    = FALSE;
    int   nverbose   = FALSE;
    int   copyTmp    = TRUE;
    char* splash     = NULL;
    char* jnlbuffer  = NULL;
    char *filename   = NULL;
    char *copyfilename = NULL;
    int   i, ret, len, size;
    int   bExists, bUpdated, bIsCurrent;
    char* perflog = NULL;
    char* launchTime = NULL;
    JNLFile *jnlfile = NULL;
    int custCmdMaxLen;
    char javaExe[MAXPATHLEN] = {0};

    int startTime = sysSetStartTime();
  
    /* 
     * Platform-specific setup. 
     */
    LauncherSetup_md(argv);
    
    /*
     * Handle "javaws -splash <port> <image file>" here, and then exit
     */
    if ((argc >= 3) && (strcmp("-splash", argv[1]) == 0)) {
      int port = atoi(argv[2]);
      ret = sysSplash(port, argv[3]);
      exit(0);
    }
  
    if (getenv("JAVAWS_TRACE_NATIVE") != NULL) {
        verbose  = TRUE;
        nverbose = TRUE;
    }
  
    /* Load configuration file into memory */
    bExists = LoadConfigurationFile(nverbose);
  
    /* rescan system registry for new or removed JREs */
    bUpdated = RescanJREs(nverbose);
  
    /*
     * Handle "javaws -userConfig <property name> [<property-value>]" here,
     * and then exit
     */
    if (argc > 2) {
        if (strcmp("-userConfig", argv[1]) == 0) {
            if (argc == 4) {
                SetProperty(argv[2], argv[3]);
            } else if (argc == 3) {
                SetProperty(argv[2], NULL);
            }
            StoreConfigurationFile(nverbose);
            exit(0);
        }
    }
  
    /* Find first non-option argument */
    for(i = 1; i < argc && filename == NULL; i++) {
        len = strlen(argv[i]);
        if (len > 0) {
            if (*argv[i] != '-') {
                filename = argv[i];
                argv[i] = "";
            } else if (!strcmp("-uninstall", argv[i])) {
                uninstall = TRUE;
                doSplash = FALSE;
                wait = TRUE;
            } else if (!strcmp("-Xclearcache", argv[i])) {
                uninstall = TRUE;
                doSplash = FALSE;
                wait = TRUE;
                argv[i] = "-uninstall";
            } else if (!strcmp("-Xnosplash", argv[i])) {
                doSplash = FALSE;
                argv[i] = "";
            } else if (!strcmp("-verbose", argv[i])) {
                verbose = TRUE;
                argv[i] = "";
            } else if (!strcmp("-import", argv[i])) {
                wait = TRUE;
                import = TRUE;
            } else if (!strcmp("-wait", argv[i])) {
                wait = TRUE;
            } else if (!strcmp("-silent", argv[i])) {
                silent = TRUE;
            } else if (!strcmp("-codebase", argv[i])) {
                /* skip the next arg */
                i++;
            } else if (!strcmp("-docbase", argv[i])) {
                /* skip the next arg */
                i++;
            } else if (!strcmp("-open", argv[i])) {
		/* skip the next arg */
		i++;
            } else if (!strcmp("-print", argv[i])) {
		/* skip the next arg */
		i++;
            } else if (strstr(argv[i], "-perflog=file:") && 
               strlen(argv[i]) > strlen("-perflog=file:")) {
                perflog = strdup(argv[i]);
            } else if (strstr(argv[i], "-launchTime=") && 
               strlen(argv[i]) > strlen("-launchTime=")) {
                launchTime = strdup(argv[i]);
            } else if (!strncmp("-J", argv[i], strlen("-J"))) {
            } else if (!strcmp("-timestamp", argv[i])) {
                /* skip the next arg */
                i++;
            } else if (!strcmp("-expiration", argv[i])) {
                /* skip the next arg */
                i++;
            } else if (!strcmp("-viewer", argv[i])) {
                isViewer = TRUE;
                doSplash = FALSE;
            } else if (!strcmp("-system", argv[i])) {
                isSystem = TRUE;
            } else if (!strcmp("-quiet", argv[i])) {
                doSplash = FALSE;
	    } else if (!strcmp("-localfile", argv[i]) || 
                !strcmp("-draggedApplet", argv[i])) {
                /* -draggedApplet is legacy from 6u10 and 6u11 
                 treat as -localfile */
	        copyTmp = FALSE;
            } else if (!strcmp("-online", argv[i])) {
            } else if (!strcmp("-installer", argv[i])) {
            } else if (!strcmp("-association", argv[i])) {
            } else if (!strcmp("-shortcut", argv[i])) {
            } else if (!strcmp("-secure", argv[i])) {
            } else if (!strcmp("-reverse", argv[i])) {
            } else if (!strcmp("-javafx", argv[i])) {
            } else if (!strcmp("-javafxau", argv[i])) {
            } else if (!strcmp("-prompt", argv[i])) {
            } else if (!strcmp("-offline", argv[i])) {
            } else if (!strcmp("-quick", argv[i])) {
            } else if (!strncmp("-X", argv[i], strlen("-X"))) {
            } else {
                showUsage();
                exit (-1);
            }
        }
    }

    /*
     * if uninstalling, and sysGetDeploymentUserHome() dosn't exist, 
     * just exit (before StoreConfigurationFile() will create home dir)
     */
    if (uninstall & !isSystem) {
        struct stat statBuf;
        if (stat(sysGetDeploymentUserHome(), &statBuf) < 0) {
            exit(0);
        }
    }

    /* store the user-specific configuration file changes resulting from
     * the rescan of registry 
     */
    if (bUpdated || !bExists) {
        StoreConfigurationFile(nverbose);
    }

    /*
     * Handle "javaws -quick" - exit after rescan of jres
     */
    if ((argc == 2) && (strcmp("-quick", argv[1]) == 0)) {
        /* just wanted to update config file with new jre(s) */
        exit(0);
    }
  
    /*
     * several error conditions 
     * 1 - can not be both system and viewer
     * 2 - if arg required following arg that wasn't there
     * 3 - no jnlp file or url, and not viewer or uninstall
     */
    if ((isViewer && isSystem) || 
        (i > argc) ||
        (!isViewer && !isSystem && !uninstall && (filename == NULL))) {
        showUsage();
        exit (-1);
    }
  
    /* only enable silent mode if import or uninstall */
    if((import == TRUE || uninstall == TRUE) && silent == TRUE) {
        doSplash = FALSE;
    } else {
        silent = FALSE;
    }
  
    java_arg_initArgs();
  
    sysSetupQuotesWholePropertySpec(nverbose);

    /*
     * OK, do we have a filename or a url argument ?
     */
    if (filename != NULL) {

        /* pass in original filename argument to Java side */
        /* this is needed because if we launch with a local jnlp file, we
           need the original filename to figure out the relative codebase.
           by default, the launcher will always make a copy of the jnlp file,
           and pass that to main (Java) of Java Web Start */
        char *prefix = "-Djnlpx.origFilenameArg=";
        int len = strlen(prefix) + strlen(filename) + 1;
        char *origFilenameArg = (char*)malloc(sizeof(char) * len);
        if (origFilenameArg != NULL) {
            int ret = sysStrNPrintF(origFilenameArg, len, "%s%s", prefix,
                filename);
            if (ret != -1) {
                java_arg_addArg(origFilenameArg, CMDLNMAXLEN);
            }
        }
        /* If uninstalling or importing, no need to copy and no need to parse
         * the file for the JRE to use, we can use default.
         */
        if (uninstall == TRUE || import == TRUE) {
            java_arg_setFile(filename, FALSE);
            jnlfile = ParseJNLFile(NULL, nverbose);
        } else {
            /*
             * We need to read, copy, parse the given jnlp file (if possible)
             */
            size = ReadFileToBuffer(filename, &jnlbuffer);
            if (jnlbuffer != NULL) {
                if (copyTmp) {
                    /*
                     * Create a copy of the file
                     */
                    copyfilename = sysTempnam();
                    if (SaveBufferToFile(copyfilename, jnlbuffer, size) == FALSE) {
                        /* Ignore error - just did not make a copy */
                        if (TRUE == verbose) {
                            Message("Failed to copy argument");
                            Message(copyfilename);
                        }
                        java_arg_setFile(filename, FALSE);
                    } else {
                        /*
                         * Update arguments to have a reference to the copy
                         */
                        java_arg_setFile(copyfilename, TRUE);
                    }
                } else {
                    // -localfile specified (copyTmp is FALSE)
                    // use cached JNLP directly, no need to remove
                    java_arg_setFile(filename, FALSE);
                }

                /* Extract jre version out of option file */
                if (isUTF8(jnlbuffer, size)) {
                    jnlfile = ParseJNLFile(jnlbuffer, nverbose);
                } else {
                    /* 
                     * arg is non utf-8 - cannot parse 
                     */
                    jnlfile = ParseJNLFile(NULL, nverbose);
                    jnlfile->isPlayer = FALSE;
                }
            } else {
                /* 
                 * arg is url, not a file - cannot read or parse 
                 */
                jnlfile = ParseJNLFile(NULL, nverbose);
                jnlfile->jnlp_url = filename;
                jnlfile->isPlayer = FALSE;
                java_arg_setFile(filename, FALSE);
            }
        }
    } else {
        /*
         * filename (or url) arg is NULL
         */
         jnlfile = ParseJNLFile(NULL, nverbose);
         jnlfile->isPlayer = TRUE;
    }
  
    custCmdMaxLen  = CMDLNMAXLEN - java_arg_strlen - 8 /* safety */;

    custCmdMaxLen -= strlen(JAVAWS_MAIN_CLASS);

    for (i=1; i<argc; i++) {
        if (argv[i] != NULL) {
            custCmdMaxLen -=strlen(argv[i]);
        }
    }
    if(java_arg_Filename!=NULL) {
        custCmdMaxLen -= strlen(java_arg_Filename);
    }

    /*
     * now that we have jnlfile structure, use various info from it.
     */
  
    for (i=0; i<jnlfile->auxArgCount; i++) {
        if (jnlfile->auxArg[i] != NULL) {
            java_arg_addAux(jnlfile->auxArg[i], custCmdMaxLen/2);
        }
    }
  
    for (i = 0; i < jnlfile->auxPropCount; i++) {
        java_arg_addAux(jnlfile->auxProp[i], custCmdMaxLen/2);
    }
  
    java_arg_addHeap(jnlfile->initialHeap, jnlfile->maxHeap);
  
    if (silent) {
        java_arg_addArg("-Djava.awt.headless=true", custCmdMaxLen);
    }

    // splash preference: 0 = normal, 1 = never, 2 = no default (custom only)
    if (jnlfile->splashPref == SPLASH_NEVER) {
        doSplash = FALSE;
    }
  
    if (doSplash == TRUE) {
        if ((jnlfile->canonicalHome != NULL) || (jnlfile->jnlp_url != NULL)) {
            getAppSplashFiles(jnlfile, &splash);
        } else {
            getDefaultSplashFiles(jnlfile->isPlayer, &splash);
        }
    }
  
    if (perflog != NULL) {
        java_arg_addArg(GetPerfLogOption(perflog), custCmdMaxLen);
        java_arg_addArg(GetStartTimeOption(startTime), custCmdMaxLen);
    }
  
    if (launchTime != NULL) {
        java_arg_addArg(GetLaunchTimeOption(launchTime), custCmdMaxLen);
    }
  
    /*
     * OK - now determine version of java to run
     */
    if (jnlfile->jreVersion != NULL) {
        jreversion  = strdup(jnlfile->jreVersion);
        if (jnlfile->jreLocation != NULL) {
            jrelocation = strdup(jnlfile->jreLocation);
        }
    }
  
    if (jnlfile->canonicalHome != NULL) {
        /* 
         * try to setup single instance service 
         */   
        ret = SetupSingleInstance(jnlbuffer, jnlfile->canonicalHome, 
                                  argc, argv);   
        if (ret == 1) {
            /* Free allocated memory */
            FreeJNLFile(jnlfile);
            if (jnlbuffer != NULL) {
                free(jnlbuffer);
            }
            sysExit(copyfilename);
        }
    }

  
    /* 
     * A NULL jreversions implies we should use the default, otherwise map the
     * the version string to a valid version and look up the location of the
     * VM. 
     */
    if (jreversion == NULL) {
        jreversion = getDefaultJREs(); /* pref is to use latest */
        /* use default JRE location if nothing is specified */
        jrelocation = strdup(DEFHREF);
    }
  
    jreIndex = DetermineVersion(jreversion, jrelocation, nverbose);
    
     /* check EULA for win32 platforms */
    if (EULA_md(argc, argv, jnlfile->isPlayer) != 1) {
        sysExit(copyfilename);
    }
  
    /* Free allocated memory */
    FreeJNLFile(jnlfile);
    if (jnlbuffer != NULL) {
        free(jnlbuffer);
    }
  
    if (doSplash) {
        char *error = ShowSplashScreen(splash);
        if (error != NULL) {
            if (silent) {
                exit(-1);
            } else {
                Abort(error);
            }
        }
        java_arg_addArg(GetJnlpxSplashPortOption(), custCmdMaxLen);
    }

    java_arg_command = GetJREJavaCmd(jreIndex);
  
    if (java_arg_command == NULL) {
        /* This should never happen */ 
        if (silent) {
            exit(-1);
        } else {
            Abort(getMsgString(MSG_BADINST_NOJRE));
        }
    }
    java_args[0] = java_arg_command;  /* we saved index 0 for exec path */

    for (i=1; i<argc; i++) {
        // remaining -J args go before the main class
        if ((argv[i] != NULL) && (!strncmp("-J", argv[i], 2))) {
            if (strlen(argv[i]) > 2) {
                java_arg_addAux(argv[i] + 2, custCmdMaxLen/2);
            }
            argv[i] = "";
        }
    }
 
    java_arg_addArg(GetJnlpxJVMOption(java_arg_command), custCmdMaxLen);

    java_arg_addArg(GetVMArgsOption(java_arg_auxargs), custCmdMaxLen);

    java_arg_addArg(JAVAWS_MAIN_CLASS, CMDLNMAXLEN);

    for (i=1; i<argc; i++) {
	// remaining args go after the main class. remove "-localfile" 
        // since it is not used in Main
        if (argv[i] != NULL && strcmp("-localfile", argv[i])) {
	    // no need to quote the string again, the whole string is already
	    // treated as a separate argument
	    java_arg_addArg(argv[i], CMDLNMAXLEN);
	}
    }

    // final arg is the jnlp file
    java_arg_addArg(java_arg_Filename, CMDLNMAXLEN);

    if(nverbose) {
        printf("total cmdline length: %d\n", java_arg_strlen);
        printf("total arguments: : %d\n", java_arg_count);
    }

    assert((java_arg_count < MAX_COUNT - 1), "too many java args to run");

    if (verbose) {
        int i;
        char message[CMDLNMAXLEN+1];
        strcpy(message, "Launching: ");
        strcat(message, java_arg_command);
        strcat(message, "\n");
        for(i=0; i < java_arg_count; i++) {
            if (java_args[i] != NULL) {
                strcat(message, java_args[i]);
                strcat(message, "\n");
            }
            strcat(message, " ");
        }
        strcat(message, "\n");
        sysMessage(message);
    }

    /* 
     * Spawn the java launcher
     */
    if (wait) {
        if (sysExec(SYS_EXEC_WAIT, java_arg_command, java_args) == -1) {
            if (silent) {
                exit(-1);
            } else {
                char *msg = malloc(2*MAXPATHLEN);
                sysStrNPrintF(msg, 2*MAXPATHLEN,  "%s \n%s",
                        getMsgString(MSG_BADINST_EXECV), java_arg_command);
                Abort(msg);
            }
        }
    } else {
        if (sysExec(SYS_EXEC_FORK, java_arg_command, java_args) == -1) {
            if (silent) {
                exit(-1);
            } else {
                char *msg = malloc(2*MAXPATHLEN);
                sysStrNPrintF(msg, 2*MAXPATHLEN, "%s \n%s",
                    getMsgString(MSG_BADINST_SYSEXE), java_arg_command);
                Abort(msg);
            }
        }
    }
  
    sysSetEndTime();
    if (verbose) {
        sysPrintTimeUsed(perflog);
    }
    return 0;
}


