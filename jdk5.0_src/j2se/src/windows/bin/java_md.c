/*
 * @(#)java_md.c	1.41 04/04/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <jni.h>
#include "java.h"
#include "version_comp.h"

#ifdef DEBUG
#define JVM_DLL "jvm_g.dll"
#define JAVA_DLL "java_g.dll"
#else
#define JVM_DLL "jvm.dll"
#define JAVA_DLL "java.dll"
#endif

/*
 * Prototypes.
 */
static jboolean GetPublicJREHome(char *path, jint pathsize);
static jboolean GetJVMPath(const char *jrepath, const char *jvmtype,
			   char *jvmpath, jint jvmpathsize);
static jboolean GetJREPath(char *path, jint pathsize);

const char *
GetArch()
{

#ifdef _M_AMD64
    return "amd64";
#elif defined(_M_IA64)
    return "ia64";
#else
    return "i386";
#endif
}

/*
 *
 */
void
CreateExecutionEnvironment(int *_argc,
			   char ***_argv,
			   char jrepath[],
			   jint so_jrepath,
			   char jvmpath[],
			   jint so_jvmpath,
			   char **original_argv) {
   char * jvmtype;

    /* Find out where the JRE is that we will be using. */
    if (!GetJREPath(jrepath, so_jrepath)) {
	ReportErrorMessage("Error: could not find Java 2 Runtime Environment.",
			   JNI_TRUE);
	exit(2);
    }

    /* Find the specified JVM type */
    if (ReadKnownVMs(jrepath, (char*)GetArch(), JNI_FALSE) < 1) {
	ReportErrorMessage("Error: no known VMs. (check for corrupt jvm.cfg file)", 
			   JNI_TRUE);
	exit(1);
    }
    jvmtype = CheckJvmType(_argc, _argv, JNI_FALSE);

    jvmpath[0] = '\0';
    if (!GetJVMPath(jrepath, jvmtype, jvmpath, so_jvmpath)) {
        char * message=NULL;
	const char * format = "Error: no `%s' JVM at `%s'.";
	message = (char *)MemAlloc((strlen(format)+strlen(jvmtype)+
				    strlen(jvmpath)) * sizeof(char));
	sprintf(message,format, jvmtype, jvmpath); 
	ReportErrorMessage(message, JNI_TRUE);
	exit(4);
    }
    /* If we got here, jvmpath has been correctly initialized. */

}

/*
 * Find path to JRE based on .exe's location or registry settings.
 */
jboolean
GetJREPath(char *path, jint pathsize)
{
    char javadll[MAXPATHLEN];
    struct stat s;

    if (GetApplicationHome(path, pathsize)) {
	/* Is JRE co-located with the application? */
	sprintf(javadll, "%s\\bin\\" JAVA_DLL, path);
	if (stat(javadll, &s) == 0) {
	    goto found;
	}

	/* Does this app ship a private JRE in <apphome>\jre directory? */
	sprintf(javadll, "%s\\jre\\bin\\" JAVA_DLL, path);
	if (stat(javadll, &s) == 0) {
	    strcat(path, "\\jre");
	    goto found;
	}
    }

    /* Look for a public JRE on this machine. */
    if (GetPublicJREHome(path, pathsize)) {
	goto found;
    }

    fprintf(stderr, "Error: could not find " JAVA_DLL "\n");
    return JNI_FALSE;

 found:
    if (_launcher_debug)
      printf("JRE path is %s\n", path);
    return JNI_TRUE;
}

/*
 * Given a JRE location and a JVM type, construct what the name the
 * JVM shared library will be.  Return true, if such a library
 * exists, false otherwise.
 */
static jboolean
GetJVMPath(const char *jrepath, const char *jvmtype,
	   char *jvmpath, jint jvmpathsize)
{
    struct stat s;
    if (strchr(jvmtype, '/') || strchr(jvmtype, '\\')) {
	sprintf(jvmpath, "%s\\" JVM_DLL, jvmtype);
    } else {
	sprintf(jvmpath, "%s\\bin\\%s\\" JVM_DLL, jrepath, jvmtype);
    }
    if (stat(jvmpath, &s) == 0) {
	return JNI_TRUE;
    } else {
	return JNI_FALSE;
    }
}

/*
 * Load a jvm from "jvmpath" and initialize the invocation functions.
 */
jboolean
LoadJavaVM(const char *jvmpath, InvocationFunctions *ifn)
{
    HINSTANCE handle;

    if (_launcher_debug) {
	printf("JVM path is %s\n", jvmpath);
    }

    /* Load the Java VM DLL */
    if ((handle = LoadLibrary(jvmpath)) == 0) {
	ReportErrorMessage2("Error loading: %s", (char *)jvmpath, JNI_TRUE);
	return JNI_FALSE;
    }

    /* Now get the function addresses */
    ifn->CreateJavaVM =
	(void *)GetProcAddress(handle, "JNI_CreateJavaVM");
    ifn->GetDefaultJavaVMInitArgs =
	(void *)GetProcAddress(handle, "JNI_GetDefaultJavaVMInitArgs");
    if (ifn->CreateJavaVM == 0 || ifn->GetDefaultJavaVMInitArgs == 0) {
	ReportErrorMessage2("Error: can't find JNI interfaces in: %s", 
			    (char *)jvmpath, JNI_TRUE);
	return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Get the path to the file that has the usage message for -X options.
 */
void
GetXUsagePath(char *buf, jint bufsize)
{
    GetModuleFileName(GetModuleHandle(JVM_DLL), buf, bufsize);
    *(strrchr(buf, '\\')) = '\0';
    strcat(buf, "\\Xusage.txt");
}

/*
 * If app is "c:\foo\bin\javac", then put "c:\foo" into buf.
 */
jboolean
GetApplicationHome(char *buf, jint bufsize)
{
    char *cp;
    GetModuleFileName(0, buf, bufsize);
    *strrchr(buf, '\\') = '\0'; /* remove .exe file name */
    if ((cp = strrchr(buf, '\\')) == 0) {
	/* This happens if the application is in a drive root, and
	 * there is no bin directory. */
	buf[0] = '\0';
	return JNI_FALSE;
    }
    *cp = '\0';  /* remove the bin\ part */
    return JNI_TRUE;
}

#ifdef JAVAW
__declspec(dllimport) char **__initenv;

int WINAPI
WinMain(HINSTANCE inst, HINSTANCE previnst, LPSTR cmdline, int cmdshow)
{
    int   ret;

    __initenv = _environ;
    ret = main(__argc, __argv);

    return ret; 
}
#endif

/*
 * Helpers to look in the registry for a public JRE.
 */
                    /* Same for 1.5.0, 1.5.1, 1.5.2 etc. */
#define DOTRELEASE  JDK_MAJOR_VERSION "." JDK_MINOR_VERSION
#define JRE_KEY	    "Software\\JavaSoft\\Java Runtime Environment"

static jboolean
GetStringFromRegistry(HKEY key, const char *name, char *buf, jint bufsize)
{
    DWORD type, size;

    if (RegQueryValueEx(key, name, 0, &type, 0, &size) == 0
	&& type == REG_SZ
	&& (size < (unsigned int)bufsize)) {
	if (RegQueryValueEx(key, name, 0, 0, buf, &size) == 0) {
	    return JNI_TRUE;
	}
    }
    return JNI_FALSE;
}

static jboolean
GetPublicJREHome(char *buf, jint bufsize)
{
    HKEY key, subkey;
    char version[MAXPATHLEN];

    /* Find the current version of the JRE */
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, JRE_KEY, 0, KEY_READ, &key) != 0) {
	fprintf(stderr, "Error opening registry key '" JRE_KEY "'\n");
	return JNI_FALSE;
    }

    if (!GetStringFromRegistry(key, "CurrentVersion",
			       version, sizeof(version))) {
	fprintf(stderr, "Failed reading value of registry key:\n\t"
		JRE_KEY "\\CurrentVersion\n");
	RegCloseKey(key);
	return JNI_FALSE;
    }

    if (strcmp(version, DOTRELEASE) != 0) {
	fprintf(stderr, "Registry key '" JRE_KEY "\\CurrentVersion'\nhas "
		"value '%s', but '" DOTRELEASE "' is required.\n", version);
	RegCloseKey(key);
	return JNI_FALSE;
    }

    /* Find directory where the current version is installed. */
    if (RegOpenKeyEx(key, version, 0, KEY_READ, &subkey) != 0) {
	fprintf(stderr, "Error opening registry key '"
		JRE_KEY "\\%s'\n", version);
	RegCloseKey(key);
	return JNI_FALSE;
    }

    if (!GetStringFromRegistry(subkey, "JavaHome", buf, bufsize)) {
	fprintf(stderr, "Failed reading value of registry key:\n\t"
		JRE_KEY "\\%s\\JavaHome\n", version);
	RegCloseKey(key);
	RegCloseKey(subkey);
	return JNI_FALSE;
    }

    if (_launcher_debug) {
	char micro[MAXPATHLEN];
	if (!GetStringFromRegistry(subkey, "MicroVersion", micro,
				   sizeof(micro))) {
	    printf("Warning: Can't read MicroVersion\n");
	    micro[0] = '\0';
	}
	printf("Version major.minor.micro = %s.%s\n", version, micro);
    }

    RegCloseKey(key);
    RegCloseKey(subkey);
    return JNI_TRUE;
}

/*
 * Support for doing cheap, accurate interval timing.
 */
static jboolean counterAvailable = JNI_FALSE;
static jboolean counterInitialized = JNI_FALSE;
static LARGE_INTEGER counterFrequency;

jlong CounterGet()
{
    LARGE_INTEGER count;

    if (!counterInitialized) {
	counterAvailable = QueryPerformanceFrequency(&counterFrequency);
	counterInitialized = JNI_TRUE;
    }
    if (!counterAvailable) {
	return 0;
    }
    QueryPerformanceCounter(&count);
    return (jlong)(count.QuadPart);
}

jlong Counter2Micros(jlong counts)
{
    if (!counterAvailable || !counterInitialized) {
	return 0;
    }
    return (counts * 1000 * 1000)/counterFrequency.QuadPart;
}

void ReportErrorMessage(char * message, jboolean always) {
#ifdef JAVAW
  if (message != NULL) {
    MessageBox(NULL, message, "Java Virtual Machine Launcher",
	       (MB_OK|MB_ICONSTOP|MB_APPLMODAL)); 
  }
#else
  if (always) {
    fprintf(stderr, "%s\n", message);
  }
#endif
}

void ReportErrorMessage2(char * format, char * string, jboolean always) { 
  /*
   * The format argument must be a printf format string with one %s
   * argument, which is passed the string argument.
   */
#ifdef JAVAW
  size_t size;
  char * message;
  size = strlen(format) + strlen(string);
  message = (char*)MemAlloc(size*sizeof(char));
  sprintf(message, (const char *)format, string);
  
  if (message != NULL) {
    MessageBox(NULL, message, "Java Virtual Machine Launcher",
	       (MB_OK|MB_ICONSTOP|MB_APPLMODAL)); 
  }
#else
  if (always) {
    fprintf(stderr, (const char *)format, string);
    fprintf(stderr, "\n");
  }
#endif
}

void  ReportExceptionDescription(JNIEnv * env) {
#ifdef JAVAW
  /*
   * This code should be replaced by code which opens a window with
   * the exception detail message.
   */
  (*env)->ExceptionDescribe(env);
#else
  (*env)->ExceptionDescribe(env);
#endif
}


/*
 * Return JNI_TRUE for an option string that has no effect but should
 * _not_ be passed on to the vm; return JNI_FALSE otherwise. On
 * windows, there are no options that should be screened in this
 * manner.
 */
jboolean RemovableMachineDependentOption(char * option) {
  return JNI_FALSE;
}

void PrintMachineDependentOptions() {
  return;
}

jboolean
ServerClassMachine() {
  jboolean result = JNI_FALSE;
  return result;
}

/*
 * Determine if there is an acceptable JRE in the registry directory top_key.
 * Upon locating the "best" one, return a fully qualified path to it.
 * "Best" is defined as the most advanced JRE meeting the constraints
 * contained in the manifest_info. If no JRE in this directory meets the
 * constraints, return NULL.
 *
 * It doesn't matter if we get an error reading the registry, or we just
 * don't find anything interesting in the directory.  We just return NULL
 * in either case.
 */
static char *
ProcessDir(manifest_info* info, HKEY top_key) {
    DWORD   index = 0;
    HKEY    ver_key;
    char    name[MAXNAMELEN];
    int	    len;
    char    *best = NULL;

    /*
     * Enumerate "<top_key>/SOFTWARE/JavaSoft/Java Runtime Environment"
     * searching for the best available version.
     */
    while (RegEnumKey(top_key, index, name, MAXNAMELEN) == ERROR_SUCCESS) {   
	index++;
	if (acceptable_release(name, info->jre_version))
	    if ((best == NULL) || (exact_version_id(name, best) > 0)) {
		if (best != NULL)
		    free(best);
		best = strdup(name);
	    }
    }

    /*
     * Extract "JavaHome" from the "best" registry directory and return
     * that path.  If no appropriate version was located, or there is an
     * error in extracting the "JavaHome" string, return null.
     */
    if (best == NULL)
	return (NULL);
    else { 
	if (RegOpenKeyEx(top_key, best, 0, KEY_READ, &ver_key)
	  != ERROR_SUCCESS) {
	    free(best);
	    if (ver_key != NULL)
		RegCloseKey(ver_key);
	    return (NULL);
	}
	free(best);
	len = MAXNAMELEN;
	if (RegQueryValueEx(ver_key, "JavaHome", NULL, NULL, (LPBYTE)name, &len)
	  != ERROR_SUCCESS) {
	    if (ver_key != NULL)
		RegCloseKey(ver_key);
	    return (NULL);
	}
	if (ver_key != NULL)
	    RegCloseKey(ver_key);
	return (strdup(name));
    }
}

/*
 * This is the global entry point. It examines the host for the optimal
 * JRE to be used by scanning a set of registry entries.  This set of entries
 * is hardwired on Windows as "Software\JavaSoft\Java Runtime Environment"
 * under the set of roots "{ HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE }".
 *  
 * This routine simply opens each of these registry directories before passing
 * control onto ProcessDir().
 */
char *
LocateJRE(manifest_info* info) {
    HKEY    key = NULL;
    char    *path;
    int	    key_index;
    HKEY    root_keys[2] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };

    for (key_index = 0; key_index <= 1; key_index++) {
	if (RegOpenKeyEx(root_keys[key_index], JRE_KEY, 0, KEY_READ, &key)
	  == ERROR_SUCCESS)
	    if ((path = ProcessDir(info, key)) != NULL) {
		if (key != NULL)
		    RegCloseKey(key);
		return (path);
	    }
	if (key != NULL)
	    RegCloseKey(key);
    }
    return NULL;
}

/*
 * Given a path to a jre to execute, this routine checks if this process
 * is indeed that jre.  If not, it exec's that jre.
 *
 * We want to actually check the paths rather than just the version string
 * built into the executable, so that given version specification will yield
 * the exact same Java environment, regardless of the version of the arbitrary
 * launcher we start with.
 */
void
ExecJRE(char *jre, char **argv) {
    char    *progname;
    char    path[MAXPATHLEN];

    /*
     * Resolve the real path to the currently running launcher.
     */
    if (GetModuleFileName(NULL, path, MAXPATHLEN) == 0) {
	ReportErrorMessage("Unable to resolve path to current executable",
	  JNI_TRUE);
	exit(1);
    }

    /*
     * If the path to the selected JRE directory is a match to the initial
     * portion of the path to the currently executing JRE, we have a winner!
     * If so, just return. (strnicmp() is the Windows equiv. of strncasecmp().)
     */
    if (strnicmp(jre, path, strlen(jre)) == 0)
	return;			/* I am the droid you were looking for */

    /*
     * Determine the executable we are building (or in the rare case, running).
     */
#ifdef JAVA_ARGS  /* javac, jar and friends. */
    progname = "java";
#else             /* java, oldjava, javaw and friends */
#ifdef PROGNAME
    progname = PROGNAME;
#else
    progname = *argv;
    if ((s = strrchr(progname, FILE_SEPARATOR)) != 0) {
	progname = s + 1;
    }
#endif /* PROGNAME */
#endif /* JAVA_ARGS */
    
    /*
     * If this isn't the selected version, exec the selected version.
     */
    (void)strcat(strcat(strcpy(path, jre), "\\bin\\"), progname);
    argv[0] = progname;
    if (_launcher_debug) {
	int i;
	printf("execv(\"%s\"", path);
	for (i = 0; argv[i] != NULL; i++)
	    printf(", \"%s\"", argv[i]);
	printf(")\n");
    }

    /*
     * Although Windows has an execv() entrypoint, it really doesn't
     * know how to overlay a process: it can only create a new
     * process.  This sounds well and good except that any processes
     * waiting on the process wake up and they shouldn't.  Hence we need
     * to keep a chain of pseudo-zombie processes around just to
     * maintain the proper wait semantics. Fortunately the image size
     * of the launcher isn't too large at this time.
     *
     * In a more reasonable world, the code below would be ...
     *
     *     execv(path, argv);
     *     ReportErrorMessage2("Exec of %s failed\n", path, JNI_TRUE);
     *     exit(1);
     *
     * Also note that typecast from the output of spawn to the parameter
     * of exit shouldn't be required.  Whatever type these are, they should
     * match.  However, in Visual C++ .NET, spawn is declared to be of
     * type intptr_t while the parameter to exit remains type int.  Hence,
     * on the IA64 environment where an int is 32 bits and an intptr_t is
     * 64 bits, this is a narrowing cast which is reported as an error if
     * not explicit.
     */
    exit((int)spawnv(_P_WAIT, path, argv));
}

/*
 * Wrapper for platform dependent unsetenv function.
 */
int
UnsetEnv(char *name)
{
    int ret;
    char *buf = MemAlloc(strlen(name) + 2);
    buf = strcat(strcpy(buf, name), "=");
    ret = _putenv(buf);
    free(buf);
    return (ret);
}

/*
 * The following just map the common UNIX name to the Windows API name,
 * so that the common UNIX name can be used in shared code.
 */
int
strcasecmp(const char *s1, const char *s2)
{
    return (stricmp(s1, s2));
}

int
strncasecmp(const char *s1, const char *s2, size_t n)
{
    return (strnicmp(s1, s2, n));
}
