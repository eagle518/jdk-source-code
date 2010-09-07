/*
 * @(#)system_md.c	1.43 05/06/09
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Implementation of solaris/linux specific OS layer */
#include <locale.h>
#include <wchar.h>
#include <malloc.h>
#include <langinfo.h>
#include <iconv.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/utsname.h>	/* For os_name */
#include "system.h"
#include "util.h"
#include "configurationFile.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>

#ifdef SOLARIS                                                          
#include <sys/systeminfo.h>       /* For os_arch */
#include <dlfcn.h>
#endif

#ifdef __linux__
#define CODESET _NL_CTYPE_CODESET_NAME
#endif

static char *splashExtension = "gif"; /* use gif file on unix */

struct timeval start_tp;
struct timeval end_tp;

int sysSetStartTime() {
  if (gettimeofday(&start_tp, NULL) != 0) {
    sysMessage("ERROR IN GETTIMEOFDAY");
    exit(0);
  }
  return 0; 
}

void sysSetEndTime() {
  if (gettimeofday(&end_tp, NULL) != 0) {
    sysMessage("ERROR IN GETTIMEOFDAY");
    exit(0);
  }
}

void sysPrintTimeUsed(char *filename) {
  long start_us;
  long end_us;
  long used_us;
  char buffer[1024];
  char* filepath = NULL;
  FILE* fp = NULL;

  if (filename == NULL) return;

  sprintf(buffer, "%s", filename);

  filepath = strstr(buffer, ":");

  if (filepath == NULL) return;

  filepath = strcat(++filepath, ".native");

  if (filepath == NULL) return;

  fp = fopen(filepath, "w");

  if (fp == NULL) return;

  fprintf(fp, "JavaWebStart native start: s = %ld, us = %ld\n", start_tp.tv_sec, start_tp.tv_usec);
 
  fprintf(fp, "JavaWebStart native end: s = %ld, us = %ld\n", end_tp.tv_sec, end_tp.tv_usec);
 

  if (end_tp.tv_sec > start_tp.tv_sec) {
    used_us = ((end_tp.tv_sec - start_tp.tv_sec) * 1000000) + end_tp.tv_usec - start_tp.tv_usec;
  } else {
    used_us = end_tp.tv_usec - start_tp.tv_usec;
  }
 
  fprintf(fp, "Startup time - Native Code (us): = %ld\n", used_us);
  fprintf(fp, "Startup time - Native Code (ms): = %ld\n", used_us/1000);
  fclose(fp);
}

/* 
 * Print an (obscure) error message to stderr and exit.  If errno has 
 * been set then print the corresponding error messages too.
 *
 */
void sysErrorExit(char *msg) 
{

    fprintf(stderr, getMsgString(MSG_SPLASH_EXIT));
    if (errno != 0) {
	perror(msg);
    }
    else {
	fprintf(stderr, "\t%s\n", msg);
    }
    exit(-1);
}

/*
 * Print a message to console. This is abstracted, since windows do not
 * have a console
 */
void sysMessage(char *msg) {
    fprintf(stdout, JAVAWS_NAME " %s", msg);
}

/* 
 * Initialize the socket library.   This function is called by all of the 
 * other functions in this file.
 * 
 * Note that initialization is only required on Win32.
 * 
 */
void sysInitSocketLibrary()
{
    /* Nothing to do */
}



/* 
 *  Safely close a socket. (This is more complicated on Win32)
 */
void sysCloseSocket(SOCKET s) 
{
    sysInitSocketLibrary();

    if (s <= 0) {
	return;
    }
    close(s);
}



/* 
 * Fork a subprocess and exec the specified application.  Remember
 * that argv[0] must be the 'name' of the application, conventionally
 * it's the same as the executable filename, and the last argv element
 * must be NULL.  Here's a Unix example:
 * 
 *   char *argv[] = {"ls", "/etc", "/tmp", NULL};
 *   int pid = sysExec("/usr/bin/ls", argv);
 */

int sysExec(int type, char *path, char *argv[]) 
{
    int pid;
    char **argvp;
    int ret = 0;

    if (type == SYS_EXEC_REPLACE) {
        return execv(path, argv);
    } else {
        
        if ((pid = fork()) == 0) {
            int err = execv(path, argv);
            /* It's neccessary to call "_exit" here, rather than exit, see
             * the fork(2) manual page.
             */
            perror(getMsgString(MSG_BADINST_EXECV));
            _exit(-1);
        } else {
	  if (type == SYS_EXEC_WAIT) {
	    waitpid(pid, &ret, 0);
	    if (ret != 0) {
	      return -1;
	    }
	  }
	}
        return pid;
    }
}


/* code copied from JRE 1.4 java_props_md.c */
char* sysGetOsName(void) {

   struct utsname name;
   uname(&name);
   return strdup(name.sysname);
}

/* code copied from JRE 1.4 java_props_md.c */
char* sysGetOsArch(void) {

  char arch[12];

#ifdef WIN32
  return "x86";
#endif

#ifdef LINUX
#ifdef _LP64
  return "amd64";
#else
  return "i386";
#endif
#else
  sysinfo(SI_ARCHITECTURE, arch, sizeof(arch));
  if (strcmp(arch,"sparc") == 0 ) {
#ifdef _LP64
    return "sparcv9";
#else
    return "sparc";
#endif
  } else if (strcmp(arch,"i386") == 0 ) {
    /* use x86 to match the value received on win32 */
    return "x86";
  } else if (strcmp(arch,"ppc") == 0 ) {
    return "ppc";
#ifdef __linux__
  } else if (strcmp(arch,"m68k") == 0 ) {
    return "m68k";
#endif
  } else {
    return "Unknown";
  }
#endif
}

char *getLibArchDir(void) {
    char arch[12];

#ifdef LINUX
#ifdef _LP64
    return "amd64";
#else
    return "i386";
#endif
#else
    sysinfo(SI_ARCHITECTURE, arch, sizeof(arch));
    if (strcmp(arch,"sparc") == 0 ) {
#ifdef _LP64
        return "sparc"; // do not return "sparcv9";
#else
        return "sparc";
#endif
    } else {
#ifdef _LP64
        return "amd64"; 
#else
        return "i386";
#endif
    }
#endif
}

/*
 * Return true if the named program exists
 */
static int
ProgramExists(char *name)
{
    struct stat sb;
    if (stat(name, &sb) != 0) return 0;
    if (S_ISDIR(sb.st_mode)) return 0;
    return (sb.st_mode & S_IEXEC) != 0;
}
 
 
/*
 * Find a command in a directory, returning the path.
 */
static char *
Resolve(char *indir, char *cmd)
{
    char name[MAXPATHLEN + 2], *real;

    if ((strlen(indir) + strlen(cmd) + 1)  > MAXPATHLEN) return 0;
    sprintf(name, "%s%c%s", indir, FILE_SEPARATOR, cmd);
    if (!ProgramExists(name)) return 0;
    real = malloc(MAXPATHLEN + 2);
    if (!realpath(name, real))
        strcpy(real, name);
    return real;
}


/*
 * Find a path for the executable
 */
static char *
FindExecName(char *program)
{
    char cwdbuf[MAXPATHLEN+2];
    char *path;
    char *tmp_path;
    char *f;
    char *result = NULL;

    /* absolute path? */
    if (*program == FILE_SEPARATOR ||
        (FILE_SEPARATOR=='\\' && strrchr(program, ':')))
        return Resolve("", program+1);

    /* relative path? */
    if (strrchr(program, FILE_SEPARATOR) != 0) {
        char buf[MAXPATHLEN+2];
        return Resolve(getcwd(cwdbuf, sizeof(cwdbuf)), program);
    }   

    /* from search path? */
    path = getenv("PATH");
    if (!path || !*path) path = ".";
    tmp_path = malloc(strlen(path) + 2);
    strcpy(tmp_path, path);

    for (f=tmp_path; *f && result==0; ) {
        char *s = f;
        while (*f && (*f != PATH_SEPARATOR)) ++f;
        if (*f) *f++ = 0;
        if (*s == FILE_SEPARATOR)
            result = Resolve(s, program);
        else {
            /* relative path element */
            char dir[2*MAXPATHLEN];
            sprintf(dir, "%s%c%s", getcwd(cwdbuf, sizeof(cwdbuf)),
                    FILE_SEPARATOR, s);
            result = Resolve(dir, program);
        }
        if (result != 0) break;
    }
 
    free(tmp_path);
    return result;
}


/* Store the name of the executable once computed */
static char *execname = NULL;

/*
 * Compute the name of the executable
 *
 * In order to re-exec securely we need the absolute path of the
 * executable. On Solaris getexecname(3c) may not return an absolute
 * path so we use dladdr to get the filename of the executable and
 * then use realpath to derive an absolute path. From Solaris 9
 * onwards the filename returned in DL_info structure from dladdr is
 * an absolute pathname so technically realpath isn't required.
 * On Linux we read the executable name from /proc/self/exe.
 * As a fallback, and for platforms other than Solaris and Linux,
 * we use FindExecName to compute the executable name.
 */
char * SetExecname(char **argv) {
    char* exec_path = NULL;

    if (execname != NULL)       /* Already determined */
        return (execname);
  
#if defined(__sun)
    {
        Dl_info dlinfo;
        if (dladdr((void*)&SetExecname, &dlinfo)) {
            char *resolved = (char*)malloc(MAXPATHLEN+1);
            if (resolved != NULL) {
                exec_path = realpath(dlinfo.dli_fname, resolved);
                if (exec_path == NULL) {
                    free(resolved);
                }
            }    
        }
    }
#elif defined(__linux__)
    {
        const char* self = "/proc/self/exe";
        char buf[MAXPATHLEN+1];
        int len = readlink(self, buf, MAXPATHLEN);
        if (len >= 0) {
            buf[len] = '\0';            /* readlink doesn't nul terminate */
            exec_path = strdup(buf);
        }    
    }
#else /* !__sun && !__linux */
    {
        /* Not implemented */
    }
#endif
    if (exec_path == NULL) {
        exec_path = FindExecName(argv[0]);
    }
    execname = exec_path;
    return exec_path;
}
 
/*
 * Return the name of the executable.  Used in java_md.c to find the JRE area.
 */
static char * GetExecname() {
  return execname;
}


/*
 * if program foo is run from xxx/bin/foo, return xxx
 */
char home[MAXPATHLEN];
char *getHome(void) {
#ifdef __linux__
    char *execname = GetExecname();

    if (execname) {
        strncpy(home, execname, MAXPATHLEN-1);
        home[MAXPATHLEN-1] = '\0';
    } else {
        Abort(getMsgString(MSG_BADINST_NOHOME)); 
    }   
#else
    Dl_info dlinfo;

    dladdr((void *)getHome, &dlinfo);
    if (realpath(dlinfo.dli_fname, home) == NULL) {
        Abort(getMsgString(MSG_BADINST_NOHOME)); 
    }   
#endif

    if (strrchr(home, '/') == 0) {
        Abort(getMsgString(MSG_BADINST_NOHOME)); 
    }   
    *(strrchr(home, '/')) = '\0';        /* executable file      */
    if (strlen(home) < 4 || strrchr(home, '/') == 0) {
        Abort(getMsgString(MSG_BADINST_NOHOME)); 
    }   
    if (strcmp("/bin", home + strlen(home) - 4) != 0) {
        *(strrchr(home, '/')) = '\0';    /* sparcv9              */
    }
    if (strlen(home) < 4 || strcmp("/bin", home + strlen(home) - 4) != 0) {
        Abort(getMsgString(MSG_BADINST_NOHOME)); 
    }   
    *(strrchr(home, '/')) = '\0';        /* bin                  */
    return home;
}

/*
 * return the path to the given .so library in the JRE of this distribution
 */
char libPath[MAXPATHLEN];
char *sysGetLibPath(char *name) {
    sprintf(libPath, "%s/lib/%s/lib%s.so", getHome(), getLibArchDir(), name);
    return libPath;
}

/*
 * return the bin directory of the jre
 */
char binjava[MAXPATHLEN];
char *bindir = NULL;
char* sysGetApplicationHome(void) {
    char libjava[MAXPATHLEN];
    char *path;
    struct stat statBuf;
 
    if (bindir == NULL) {
        path = getHome();

        /* Is JRE co-located with the application? */
        sprintf(libjava, "%s/lib/javaws.jar", path);
        if (stat(libjava, &statBuf) == 0) {
            sprintf(binjava, "%s/bin", path);
	    bindir = binjava;
        } else {
            /* Does the app ship a private JRE in <apphome>/jre directory? */
            sprintf(libjava, "%s/jre/lib/javaws.jar", path);
            if (stat(libjava, &statBuf) == 0) {
                sprintf(binjava, "%s/jre/bin", path);
                bindir = binjava;
            }
        }
    }

    if (bindir == NULL) {
        Abort(getMsgString(MSG_BADINST_NOHOME));
    }
    return bindir;
}


/*
 * If app is "c:\j2se1.4.2\javaws\javaws.exe", 
 *    then put "c:\j2se1.4.2\bin\java" into buf. (if it exists)
 */
char* sysGetInstallJRE(void) {
    struct stat statBuf;
    static char jre[MAXPATHLEN];
    char nativeStr[100];
    static int  init1 = FALSE;
    char *ptr;
     
    if (!init1) {  
	strcpy(jre, sysGetApplicationHome());
	ptr = strrchr(jre, FILE_SEPARATOR);
	sprintf(nativeStr, "%s%c%s", "bin", FILE_SEPARATOR, "java"); 
        strcpy(ptr+1, nativeStr);
    }
    if (stat(jre, &statBuf) == 0) {
	return jre;
    }
    return NULL;
}

static char* getUserHome(void) {
    static char userhome[MAXPATHLEN];
    static int  initialized = FALSE;
    
    if (!initialized) {  
	char *envhome = getenv("USER_JPI_PROFILE");
	if ((envhome != NULL) && (strlen(envhome) > 0)) {
	    strcpy(userhome, envhome);
	} else {
            struct passwd *pwent = getpwuid(getuid());
            strcpy(userhome, pwent ? pwent->pw_dir : "");
	}
        if (userhome[strlen(userhome)-1] == FILE_SEPARATOR) {
            userhome[strlen(userhome)-1] = '\0';
        }
        initialized = TRUE;   
    }
    return userhome;
}

/*  Returns a tempoary filename. The string returned must be
 *  freed by the caller
 */
char* sysTempnam(void) {
  char* argFileName = tempnam(NULL, "javaws");
  return argFileName;
}

/*
 *  get extension for splash files
 */
char* sysGetSplashExtension() {
  return splashExtension;
}


int sysStrNPrintF (char* str, size_t size, const char *format, ...)
{
    int num;
    va_list ap;

    va_start(ap, format);
    num = vsnprintf(str, size, format, ap);
    va_end(ap);

    if(num<0 || num>=size) {
        // ensure null termination
        str[size-1] = '\0';
    }

    return num;
}

/*
 *  case insensitive compare of two string
 */
int sysStrCaseCmp(char* s1, char* s2) {
    return strcasecmp(s1, s2);
}
int sysStrNCaseCmp(char* s1, char* s2, size_t n) {
    return strncasecmp(s1, s2, n);
}

char *sysGetJavawsbin() {
    return "javaws";
}

char *sysGetJavabin() {
    return "java";
}

static char _localeStr[64];
static int locale_initialized = FALSE;
char *sysGetLocaleStr() {

    /*
    ** The following code to get the locale string is taken mostly from java's:
    ** src/solaris/native/java/lang/java_props_md.c.
    */
    if (!locale_initialized) {
        char *p; 
        char *lc = setlocale(LC_CTYPE, "");
	char *javawsloc = getenv("_JAVAWS_LOCALE");
	if (javawsloc != NULL) {
	    strcpy(_localeStr, javawsloc);
	} else if (lc == NULL) {
            /*   
             * 'lc == null' means system doesn't support user's environment
             * variable's locale.
             */  
            strcpy(_localeStr, "C");
        } else { 
            /*
             * <language name>_<region name>.<encoding>@<varient>
             * we only want the first two if they are there:
             */  
            strcpy(_localeStr, lc);
            p = strchr(_localeStr, '.');
            if (p == NULL) {
                p = strchr(_localeStr, '@');
            }
            if (p != NULL) {
                *p = '\0';    /* chop off .<encoding>@<varient> or @<varient> */            }
        }
        if (sysStrCaseCmp(_localeStr, "C") == 0) {
            strcpy(_localeStr, "en_US");
        }
	locale_initialized = TRUE;
    }
    return _localeStr;
}

unsigned short *sysMBCSToWideChar(char *mbcs) {
  size_t len = strlen(mbcs);
  char *p = NULL;
  char *utf8 = NULL, *ucs2 = NULL;
  iconv_t cd;
  size_t oleft = MAXPATHLEN*2;

  /* get codeset (encoding) */
  setlocale(LC_ALL, "");
  p = nl_langinfo(CODESET);
  
  /* use Latin-1 encoding if we couldn't find a default one */
  if (p==NULL || p[0]=='\0') {
    p = "ISO8859-1";
  }
  
  if (strcmp(p, "UTF-8") != 0) {
    /* Convert from MB(locale specific) to UTF-8 , we use UTF-8 as a universal
       format from which other conversions can be done easily */
    
    cd = iconv_open("UTF-8", p);
    if (cd == (iconv_t)-1) {
      return NULL;
    }

    utf8 = (char *)malloc(MAXPATHLEN*2);
    if (utf8==NULL) 
      return NULL;

    memset(utf8, 0 , MAXPATHLEN*2);

    p = utf8;
    
    if (iconv(cd, (const char**)&mbcs, &len, &p, &oleft) == (size_t)-1) {
      free(utf8);
      return NULL;
    } else {
      *p=0;
    }
    iconv_close(cd);
    
    len = (MAXPATHLEN*2) - oleft; 
  } else {
    /* we are in UTF-8 already! */
    utf8 = mbcs;
  }
      
  /* Convert from UTF-8 to the UCS-2 encoding */
  cd = iconv_open("UCS-2", "UTF-8");
  if (cd == (iconv_t)-1) {
    free(utf8);
    return NULL;
  }
  
  ucs2 = (char *)malloc(MAXPATHLEN*2);
  if (ucs2==NULL) {
    free(utf8);
    return NULL;
  }

  memset(ucs2, 0, MAXPATHLEN*2);
  
  p = ucs2;
  oleft = MAXPATHLEN*2;
  len = strlen(utf8);
  
  if (iconv(cd, (const char **)&utf8, &len, (char**)&p, &oleft) == (size_t)-1) {
    free(utf8);
    free(ucs2);
    return NULL;
  } else {
    *p = 0;
  }
  iconv_close(cd);

  return (unsigned short*)ucs2;
  
}


char *sysMBCSToSeqUnicode(char *mbcs) {
  unsigned short * stringU;
  char * output;
  
  stringU = sysMBCSToWideChar(mbcs);

  output = sysSaveConvert(stringU);

  free(stringU);
  return output;
}

char *sysWideCharToMBCS(twchar_t *message, size_t len) {
  char *p = NULL, *p2 = NULL;
  char *out = NULL, *out2 = NULL;
  iconv_t cd;
  size_t oleft = MAXSTRINGLEN;
  int i;
  size_t count = 0;
  size_t len2 = len*2;

  /* get codeset (encoding) */
  p = nl_langinfo(CODESET);

  /* use Latin-1 encoding if we couldn't find a default one */
  if (p==NULL || p[0]=='\0') {
    p = "ISO8859-1";
  }

  /* Convert from UCS-2 to UTF-8 , we use UTF-8 as a universal */
  /* format from which other conversions can be done easily */
  cd = iconv_open("UTF-8", "UCS-2");
  if (cd == (iconv_t)-1) {
    return NULL;
  }
  
  out = (char *)malloc(MAXSTRINGLEN);
  if (out==NULL) return NULL;
  p2 = out;
  if (iconv(cd, (const char **)&message, &len2, &p2, &oleft) == (size_t)-1) {
    free(out);
    out = NULL;
  } else {
    *p2=0;
  }
  iconv_close(cd);
  p2 = out;

  /* Convert from UTF-8 to the locale-specific encoding */
  if ((strcmp(p, "UTF-8") != 0) && (p2!=NULL)) {
    cd = iconv_open(p, "UTF-8");
    if (cd == (iconv_t)-1) {
      free(p2);
      return NULL;
    }
    
    out2 = (char *)malloc(MAXSTRINGLEN);
    if (out2==NULL) {
      free(p2);
      return NULL;
    }
    p2 = out2;
    len2 = MAXSTRINGLEN - oleft ;
    oleft = MAXSTRINGLEN;
    if (iconv(cd, (const char **)&out, &len2, &p2, &oleft) == (size_t)-1) {
      free(out);
      free(out2);
      out2 = NULL;
    } else {
      *p2 = 0;
    }
    iconv_close(cd);
    p2 = out2;
  }
  
  return p2;
}

/* argv MUST include cmd. */
void sysExec2Buf(char *cmd, int argc, char *argv[], char *buf, int *buflen) {
  int fdin[2], fdout[2], fderr[2], k, resultPid, n, ofs = 0, len, status;
  siginfo_t info;
  len = *buflen;
  *buflen = 0;
  if ((k=0, pipe(fdin)<0) || (k=1, pipe(fdout)<0) || (k=2, pipe(fderr)<0)) {
    switch (k) {
    case 2:	
      close(fdout[0]);
      close(fdout[1]);
    case 1:
      close(fdin[0]);
      close(fdin[1]);
    case 0: ;
    }
    return;
  }

#ifdef SOLARIS
  resultPid = fork1();
#else
  resultPid = fork();
#endif
  
  if (resultPid < 0) {
    /* fork error - make sure we clean up the pipes */
    close(fdin[1]);
    close(fdout[0]);
    close(fderr[0]);
    close(fdin[0]);
    close(fdout[1]);
    close(fderr[1]);
    return;
  }

  if (resultPid == 0) {
    /* 0 open for reading, 1 open for writing */
    /* (Note: it is possible for fdin[0] == 0 - 4180429) */

    dup2(fdin[0], 0);
    dup2(fdout[1], 1);
    dup2(fdout[1], 2);
    
    execv(cmd, argv);
  }
  
  /* parent process */

  /* Wait for the child process to exit.  This returns immediately if
     the child has already exited. */
  /* fix for 6188963: java web start cannot start on linux with
     new glibc */
  /* from Runtime.exec() UNIXProcess_md.c */
  while (waitpid(resultPid, &status, 0) < 0) {
    switch (errno) {
    case EINTR: break;
    default: 
      close(fdin[1]);
      close(fdout[0]);
      close(fderr[0]);
      close(fdin[0]);
      close(fdout[1]);
      close(fderr[1]);
      return;
    }
  }

  /* clean up the child's side of the pipes */
  close(fdin[0]);
  close(fdout[1]);
  close(fderr[1]);

  while ((n = read(fdout[0], &(buf[ofs]), len - ofs)) > 0) {
    ofs += n;
  }
  buf[ofs] = 0;
  *buflen = ofs-1;
  
  close(fdin[1]);
  close(fdout[0]);
  close(fderr[0]);
}

void sysGetRegistryJREs (JREDescription jre_list[], int *nJREs) {
  char *path = sysGetInstallJRE();
  if (path != NULL) {
      jre_list[0].product_version = NULL;
      jre_list[0].path = strdup(path);
      jre_list[0].osarch = ARCH;
      jre_list[0].osname = PLATFORM;
      *nJREs = 1;
  } else {
      *nJREs = 0;
  }
}

void sysCreateDirectory(char *dir) {
  mkdir(dir, S_IRWXG | S_IRWXU | S_IRWXO );
}

char *sysGetDeploymentUserHome() {
    static char deploymentUserHome[MAXPATHLEN];
    static int  initialized = FALSE;
    if (!initialized) {
	sprintf(deploymentUserHome, "%s/.java/deployment",getUserHome());
	initialized = TRUE;
    }
    return deploymentUserHome;
}

char *sysGetDeploymentSystemHome() {
    return "/etc/.java/.deployment";
}

/* return 1 if file is found, otherwise 0 */
int sysFindSiFile(char *canonicalHome, char *siFilename) {
  char searchPath[MAXPATHLEN];
  DIR *siDir;
  struct dirent* siDirEnt;
  int found = 0;
  char *sessionID;
  char *display;

  display = getenv("DISPLAY");

  if (display == NULL) {
    display = "";
  }

  sessionID = (char*)malloc(sizeof(char) * (strlen(canonicalHome) + 
					    strlen(display) + 1));

  sprintf(sessionID, "%s%s", canonicalHome, display);

  sysReplaceChar(sessionID, '/', '_');

  sysReplaceChar(sessionID, ':', '_');

  sprintf(searchPath, "%s%c%s%c%s", sysGetDeploymentUserHome(), FILE_SEPARATOR, "tmp", FILE_SEPARATOR, "si");

  recursive_create_directory(searchPath);
 
  siDir = opendir(searchPath);

  while ((siDirEnt = readdir(siDir)) != NULL) {

    if (strstr(siDirEnt->d_name, sessionID) != NULL) {   
      found = 1;
      strcpy(siFilename, siDirEnt->d_name);
      break;
    }
  }

  closedir(siDir);

  free(sessionID);

  return found; 
}

char* sysGetDebugJavaCmd(char* javaCmd) {
  return strdup(javaCmd);
}

