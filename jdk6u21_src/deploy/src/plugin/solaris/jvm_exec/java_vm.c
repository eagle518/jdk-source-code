/*
 * @(#)java_vm.c	1.81 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This process is forked from the navigator plugin to run the Java VM.
 * All the real work is done in Java in the sun.plugin.Plugin class.
 *
 *							    KGH Dec 97
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>    
#include <string.h>
#include <jni.h>
#include "plugin_defs.h"
#include "pluginversion.h"
#include "utils.h"

#include <assert.h>
#include <sys/param.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <pwd.h>

static char *find_options();
static void SetClassPath(char *s);
static void SetBootClassPath(char *s);
static jint ParseArguments( char *pargv);
static void AddOption(char *str, void *info);
static void *MemAlloc(size_t size);
static char *MemDup(char *src);
static void ResizeVmargsArray(char ***array, int *current_size);
jboolean trace = JNI_FALSE;

/*
 * Code for dealing with libjvm.so is swiped from src/share/bin,
 * src/solaris/bin.  A better code sharing solution is highly desirable.
 */

typedef jint (JNICALL *CreateJavaVM_t)(JavaVM **pvm, void **env, void *args);
typedef jint (JNICALL *GetDefaultJavaVMInitArgs_t)(void *args);

typedef struct {
    CreateJavaVM_t             CreateJavaVM;
    GetDefaultJavaVMInitArgs_t GetDefaultJavaVMInitArgs;
} InvocationFunctions;


/*
 * List of VM options to be specified when the VM is created.
 */
static JavaVMOption *options;
static int numOptions;

/* Tracing for the startup code. Write error messages to a file, rather
   than to the screen where they are lost. Use a file named
   plugin_child_startup.trace */
FILE* tracefile;

void startup_trace(char *format,...) {
    va_list arglist;
    va_start(arglist, format);
    if (trace) {
	if (tracefile != NULL) {
	    vfprintf(tracefile, format, arglist);
	    fprintf(stderr, "%s After exec, before startup:",PLUGIN_NODOTVERSION);
	    vfprintf(stderr, format, arglist);
	    fflush(tracefile);
	} else {
	    fprintf(stderr, "%s After exec, before startup:",PLUGIN_NODOTVERSION);
	    vfprintf(stderr, format, arglist);
	}
    }
    va_end(arglist);
}

void trace_env(char* envname) {
    char* env_value = getenv(envname);
    if (env_value) 
	startup_trace("Environment variable %s=%s\n",envname, env_value);
    else
	startup_trace("Environment variable %s not set\n", envname);
}

/*
 * Find the JVM entry points we need.
 */
static jboolean getJVMEntryPoints(char *java_home, InvocationFunctions *ifn) {
    void *libjvm;
    char jvmpath[MAXPATHLEN];
    const char *libname;

    libname = "libjvm.so";
    snprintf(jvmpath, sizeof jvmpath, "%s/lib/"LIBARCH"/client/%s", java_home, libname);
    libjvm = dlopen(jvmpath, RTLD_NOW + RTLD_GLOBAL);
  
    if (libjvm == NULL) {
        snprintf(jvmpath, sizeof jvmpath, "%s/lib/"LIBARCH"/%s", java_home, libname);
        libjvm = dlopen(jvmpath, RTLD_NOW + RTLD_GLOBAL);
    }


    if (libjvm != NULL) {

	ifn->CreateJavaVM = (CreateJavaVM_t) dlsym(libjvm, "JNI_CreateJavaVM");
	ifn->GetDefaultJavaVMInitArgs = (GetDefaultJavaVMInitArgs_t)
	    			dlsym(libjvm, "JNI_GetDefaultJavaVMInitArgs");

	if (ifn->GetDefaultJavaVMInitArgs != NULL &&
	    ifn->CreateJavaVM != NULL)
	    return JNI_TRUE;
    }
    return JNI_FALSE;
}

int main(int argc, char **argv) {
    JavaVM *jvm;
    JNIEnv *env;
    jclass clz;
    jmethodID meth;
    char classpath[6*MAXPATHLEN], *s;
    char nodotversion[255];
    char version[255];
    char *java_home;
    char *plugin_home;
    char *optionstr;
    JavaVMInitArgs args;
    InvocationFunctions ifn;
    int i = 0;
    char allOptions[8096];
    char* java_options;       

    if (argc >1 && strcmp(argv[1], "-t") == 0) {   
	trace = JNI_TRUE;
	fprintf(stderr, "Opening the trace file %s\n", EXEC_TRACE_FILE(PLUGIN_NODOTVERSION));
	tracefile = fopentrace(EXEC_TRACE_FILE(PLUGIN_NODOTVERSION));	
	
        startup_trace("Java process: entered main\n");

	/* Print out possible environment flags for tracing */
	trace_env("CLASSPATH");
	trace_env("LD_LIBRARY_PATH");
	trace_env("JAVA_HOME");
	trace_env("PLUGIN_HOME");
	trace_env("JAVA_PLUGIN_AGENT");
	trace_env("JAVA_VM_WAIT");
    }

    startup_trace("Going to sleep for debugging...\n");
    if (getenv("JAVA_VM_WAIT")) {
	    sleep(30);
    }
    startup_trace("Woke up...\n");
    
    java_home = getenv("JAVA_HOME");
    plugin_home = getenv("PLUGIN_HOME");
    
    if (java_home == NULL || plugin_home == NULL) {
      fprintf(stderr, "%s\n",
	      DGETTEXT(JAVA_PLUGIN_DOMAIN, 
		       "java_vm process: You need to set both JAVA_HOME and PLUGIN_HOME"));
      exit(-1);
    }
   
    /* JAVA_PLUGIN_VERSION must be set to actual plug-in version
       and it should not be derived from the build environment
       variable VERSION because this will be used to construct
       the directory path name for lot of things
       (Deva, 10/06/00)
    */
    putenv("JAVA_PLUGIN_VERSION=" PLUGIN_VERSION);


    /* Disable DNS spoofing check */
    AddOption("-DtrustProxy=true", NULL);

    /* Verify remote code only */
    AddOption("-Xverify:remote", NULL);

    /* Set user-defined options for the Java VM */
    optionstr = find_options();
    ParseArguments(optionstr);
    
    snprintf(classpath, sizeof classpath,
	    "%s/lib/plugin.jar:"		/* from plugin_home */
	    "%s/lib/deploy.jar:"	/* from plugin_home */ 
	    "%s/lib/javaplugin_l10n.jar"	/* from plugin_home */
	    , plugin_home, plugin_home, plugin_home);

    startup_trace("Classpath = %s \n",classpath);
    s = classpath;

    /* Get JVM function pointers. */
    if (!getJVMEntryPoints(java_home, &ifn)) {
	fprintf(stderr, "%s\n", 
		DGETTEXT(JAVA_PLUGIN_DOMAIN, 
			 "java_vm process: could not find Java VM symbols"));
	exit(3);
    }

    SetBootClassPath(s);

    /* Set javaplugin.home. */
    {
	int buflen = strlen(plugin_home) + 100;
	char *buf = MemAlloc(buflen);
	snprintf(buf, buflen, "-Djavaplugin.lib=%s/lib/" LIBARCH "/libjavaplugin_jni.so",
		plugin_home);
	AddOption(buf, NULL);

    free(buf);
    }

    /* Set java.ext.dirs for JSS package*/
    {
    	char extbuf[1024];
	char *mozDir = getenv("MOZILLA_HOME");
    	char jssDir[1024];

	// Add java.ext.dirs when it was not set up in Java control panel
	if ((optionstr != NULL && strstr(optionstr, "-Djava.ext.dirs") == NULL)
	    || optionstr == NULL)
	{
	   if ((mozDir != NULL) && (strlen(mozDir) > 0))
 	   {
		// Make sure the directory exist
		sprintf(jssDir, "%s/jss", mozDir);
		int ret = access(jssDir, F_OK);

		if (ret == 0)
		{
    	  	   sprintf(extbuf, "-Djava.ext.dirs=%s/lib/ext:%s", plugin_home, jssDir);
    	  	   //sprintf(extbuf, "-Djava.ext.dirs=%s/lib/ext:%s/jss", plugin_home, mozDir);
    	  	   AddOption(extbuf, NULL);
		}
	   }
	}
    }

    /* Workaround Mozilla bug */
    if (getenv("MOZILLA_WORKAROUND"))
      AddOption("-Dmozilla.workaround=true", NULL);

    /* Set the user plugin home directory */
	{
    	char *userPluginHome = getenv("USER_JPI_PROFILE");
		if(userPluginHome != NULL)
		{
			char profileProp[1024];
			sprintf(profileProp, "-Djavaplugin.user.profile=%s", userPluginHome);
			AddOption(profileProp, NULL);
		}
	}

    sprintf(nodotversion, "%s%s", "-Djavaplugin.nodotversion=", PLUGIN_NODOTVERSION);
    AddOption(nodotversion, NULL);

    sprintf(version, "%s%s", "-Djavaplugin.version=", PLUGIN_VERSION);     
    AddOption(version, NULL);

    /* Dump all the options passed to JVM before start JVM */ 
    allOptions[0] = '\0';
    strcat(allOptions, "-Djavaplugin.vm.options=");
    for (i = 0; i < numOptions; i ++)
    {
        if (options[i].optionString != NULL)
	{
            strcat(allOptions, options[i].optionString);
            strcat(allOptions, " ");
	}
    }
    
    AddOption(allOptions, NULL);

    
    /* Set user-defined options for the Java VM */
    memset(&args, 0, sizeof(args));
    args.version  = JNI_VERSION_1_2;
    args.nOptions = numOptions;
    args.options  = options;
    args.ignoreUnrecognized = JNI_TRUE;

    startup_trace("Version is: %d \n", args.version);
    /* Now we can start up the java virtual machine */
    if (ifn.CreateJavaVM(&jvm, (void *)&env, &args) != JNI_OK) {
	fprintf(stderr, "Could not startup JVM properly!\n");
	fprintf(stderr, "%s\n", 
		DGETTEXT(JAVA_PLUGIN_DOMAIN, 
			 "java_vm process: could not start Java VM"));
	exit(3);
    }

    free(options);
      
    startup_trace("java_vm process: started Java VM OK\n");

    /* Clear any exception status */
    (*env)->ExceptionClear(env);

    /* Clear any exception status */
    (*env)->ExceptionClear(env);


    /* Find the main javaplugin class */
    clz = (*env)->FindClass(env, "sun/plugin/navig/motif/Plugin");
    if (clz == 0) {
	fprintf(stderr, "%s sun/plugin/navig/motif/Plugin\n",
		DGETTEXT(JAVA_PLUGIN_DOMAIN, 
			 "java_vm process: Couldn't find class"));
	(*env)->ExceptionDescribe(env);
	exit(5);
    }

    /* Find its "start" method */
    meth = (*env)->GetStaticMethodID(env, clz, "start", "(Z)V");
    if (meth == 0) {
	fprintf(stderr, "%s\n", 
		DGETTEXT(JAVA_PLUGIN_DOMAIN, 
			 "Java process: Couldn't find Plugin.start"));
	(*env)->ExceptionDescribe(env);
	exit(6);
    }
    startup_trace("Ready to call start method\n");
    /* Call into the Java code */
    (*env)->CallStaticVoidMethod(env, clz, meth, trace);
    if ((*env)->ExceptionOccurred(env)) {
	fprintf(stderr, "%s\n",
		DGETTEXT(JAVA_PLUGIN_DOMAIN, "Java process: caught exception from sun.plugin.navig.motif.Plugin.start"));
        (*env)->ExceptionDescribe(env);
    }

    return(0);
}

static char *
find_options()
{
    startup_trace("FIND OPTIONS %s\n", PLUGIN_VERSION);
    FILE *fin;
    char *home = NULL;
    char path[MAXPATHLEN];
    char line[MAXPATHLEN];
    char key[MAXPATHLEN];
    char *result = 0;
    int index;
    const char* javaws_jre  = "deployment.javaws.jre.";
    char *jreIndex, *version, *jreProp;

    /* For RFE 4523267, if the user specify _JPI_JVM_OPTIONS, ignore jre params */
    char* vm_options = getenv("_JPI_VM_OPTIONS");
    if (vm_options)
        return vm_options;

    home  = getenv("USER_JPI_PROFILE");
    if(home == NULL)
    {
	home  = getenv("HOME");
    }

    snprintf(path, sizeof path, "%s/.java/deployment/deployment.properties", home);	
    fin = fopen(path, "r");
    if (fin == NULL) {
	return 0;
    }

    // look the jre info index matching PLUGIN_VERSION

    while (fgets(line, sizeof(line), fin)) {
        startup_trace(line);
	if (strstr(line, javaws_jre) == line) {
	    index = -1;
	    if (strstr(line, "=") == NULL) {
	        // no "=" in the line, not a correct property
	        continue;
	    }
	    jreIndex = jreProp = version = 0;
	    jreIndex = strtok(line + strlen(javaws_jre), ".");
	    if (jreIndex != NULL) {
	      index = atoi(jreIndex);
	    }

	    if (index == -1) {
	        continue;
	    }

	    jreProp = strtok(NULL, "=");
	    if (jreProp != NULL) {
	        // compare the product version with PLUGIN_VERSION
	        if (strncmp(jreProp, "product", 7) == 0) {
		    // skip '='
		    version = jreProp + 8;
		    if (strncmp(version, PLUGIN_VERSION, strlen(PLUGIN_VERSION)) == 0) {
			break;
		    }
		}
	    }
	}
    }
    
    if (index == -1) {
        // no jre found
        return result;
    }

    //rewind the file
    fseek(fin, 0L, SEEK_SET);
    sprintf(key, "deployment.javaws.jre.%d.args=", index);
    while (fgets(line, sizeof(line), fin)) {
	if (strstr(line, key) == line) {
	    /* Copy the value, ommiting the key and the ending newline */
	    int len = strlen(line) - strlen(key);
	    result = MemAlloc(len);
	    memcpy(result, line+strlen(key), len-1);
	    result[len-1] = 0;
	}
    }
    fclose(fin);

    return result;
}

/*
 * Returns a pointer to a block of at least 'size' bytes of memory.
 * Prints error message and exits if the memory could not be allocated.
 */
static void *
MemAlloc(size_t size)
{
    void *p = malloc(size);
    if (p == 0) {
        perror("malloc");
        exit(1);
    }
    return p;
}

static char *
MemDup(char *src)
{
    int len = strlen(src);
    char *dst = (char *) MemAlloc(len + 1);
    strcpy(dst, src);
    return dst;
}

static void
SetClassPath(char *s)
{
    int deflen = 0;
    char *def = NULL;
    char *java_home = getenv("JAVA_HOME");

    if (s == NULL)
    {    
      deflen = 200;
      def = MemAlloc(deflen);
      snprintf(def, deflen, "-Djava.class.path=%s/classes", java_home);
    }
    else
    { 
      deflen = strlen(s) + 200;
      def = MemAlloc(deflen);
      snprintf(def, deflen, "-Djava.class.path=%s/classes:%s", java_home, s); 
    }      
       
    AddOption(def, NULL);

    free(def);
}

static void
SetBootClassPath(char *s)
{
    int deflen = strlen(s) + 40;
    char *def = MemAlloc(deflen);

    snprintf(def, deflen, "-Xbootclasspath/a:%s", s);

    AddOption(def, NULL);

    free(def);

}

/*
 * Parses command line arguments.
 */
static jint
ParseArguments( char *pargv)
{
    char seps[] = " ";
    char *arg = NULL;
    char **vmargs;
    char targ[MAXPATHLEN];
    char *classpath = NULL;
    char *token = NULL;
    char *temp = NULL;
    int argnum = 0;
    int vmargs_size = 1;
    int i = 0;

    if (pargv != NULL){
        arg = strtok( pargv, seps );
        vmargs = malloc(vmargs_size * sizeof(char *));
    }

    while (arg != NULL) {
        /* Assemble the token */
        if (token == NULL){
          token = MemAlloc(strlen(arg)+1);
          token[0] = NULL;
          strcat(token, arg);
        }
        else{
            /*
             * re-allocate token, copy arg to it
             */
            temp = MemAlloc(strlen(token)+strlen(arg)+strlen(" ")+2);
            temp[0] = NULL;
            strcat(temp, token);
            strcat(temp, " ");
            strcat(temp, arg);
            free(token);
            token = temp;          
        }
        /* check if the end of token has been found:*/
        arg = strtok(NULL, seps);
        if (arg != NULL && strncmp(arg, "-", 1) == 0){
            /*
             * We found the beginning of next token
             */
            if(argnum >= vmargs_size){
                /*
	 * Resize the vmargs array.
	 */
                ResizeVmargsArray(&vmargs, &vmargs_size);
            }
            vmargs[argnum] = token;
            argnum++;
            token = NULL;
        }
        else if(arg == NULL){
            if (argnum >= vmargs_size){
                /*
	 * Resize the vmargs array.
	 */
                ResizeVmargsArray(&vmargs, &vmargs_size);
            }

            /* 
             * Put last argument into the vmargs array.
             */
            vmargs[argnum] = token;
            argnum++;
        }
    }/* end token assembly while loop */
    

    
    for (i=0; i<argnum; i++){
        /*
         * Check if classpath is set.
         */
        if (strncmp(vmargs[i], "-classpath", 10) == 0 || 
            strncmp(vmargs[i], "-cp", 3) == 0) {
            classpath = strtok( vmargs[i], "=");
            if (classpath != NULL){
                /*
	 * Get the value of the classpath:
	 */
	classpath = strtok(NULL, "=");
            }
        } else if (strcmp(vmargs[i], "-help") == 0 ||
	   strcmp(vmargs[i], "-h") == 0 ||
	   strcmp(vmargs[i], "-?") == 0) {
        } else if (strcmp(vmargs[i], "-verbosegc") == 0) {
            AddOption("verbose:gc", NULL);
        } else if (strcmp(vmargs[i], "-trace") == 0) {
            AddOption("-Xt", NULL);
        } else if (strcmp(vmargs[i], "-noclassgc") == 0) {
            AddOption("-Xnoclassgc", NULL);
        } else if (strcmp(vmargs[i], "-verify") == 0) {
            AddOption("-Xverify:all", NULL);
        } else if (strcmp(vmargs[i], "-verifyremote") == 0) {
            AddOption("-Xverify:remote", NULL);
        } else if (strcmp(vmargs[i], "-noverify") == 0) {
            AddOption("-Xverify:none", NULL);
        } else if (strncmp(vmargs[i], "-ss", 3) == 0 ||
	   strncmp(vmargs[i], "-oss", 4) == 0 ||
	   strncmp(vmargs[i], "-ms", 3) == 0 ||
	   strncmp(vmargs[i], "-mx", 3) == 0) {
            int tmplen = strlen(vmargs[i]) + 6;
            char *tmp = MemAlloc(tmplen);
            snprintf(tmp, tmplen, "-X%s", vmargs[i] + 1);  /*skip '-' */
            AddOption(tmp, NULL);
        } else if (strcmp(vmargs[i], "-checksource") == 0 ||
	   strcmp(vmargs[i], "-cs") == 0 ||
	   strcmp(vmargs[i], "-noasyncgc") == 0) {
            /* No longer supported */
            fprintf(stderr, "Warning: %s option is no longer supported.\n",
	    vmargs[i]);
        } else if (strcmp(vmargs[i], "-classic") == 0) {
        } else if (strcmp(vmargs[i], "-client") == 0) {
        } else if (strcmp(vmargs[i], "-server") == 0) {
        } else if (strcmp(vmargs[i], "-native") == 0) {
        } else if (strcmp(vmargs[i], "-hotspot") == 0) {
        } else if (strcmp(vmargs[i], "-green") == 0) {
        } else if (strcmp(vmargs[i], "-Xoldjava") == 0) {
        } else {
            strcpy(targ,"");  /* Clean up the targ */ 
            strcat(targ, vmargs[i]);
            AddOption(targ, NULL);
        }
    }/* end for loop */

    /*
     * Set class path here if it is null or not.
     */
    SetClassPath(classpath);

    return 0;    

}


/*
 * Resizes the array which holds tags with vm arguments.
 */
static void
ResizeVmargsArray(char ***array, int *current_size)
{
    char **temp = NULL;
    int i = 0;

    *current_size = *current_size * 2;
   
    temp = realloc(*array, *current_size * sizeof(char*));
    if (temp == NULL) {
        perror("realloc");
        exit(1);
    }

    *array = temp;
}

/*
 * Adds a new VM option with the given name and value.
 */
static void
AddOption(char *str, void *info)
{
    int i=0, j=0;
    int len = 0;
    char* optionStr = NULL;

    /*
     * Expand options array if needed to accomodate at least one more
     * VM option.
     */

    JavaVMOption *tmp = MemAlloc((numOptions+1) * sizeof(JavaVMOption));
    if ( numOptions > 0 ) {
        /* Copy the old options array to the new one */
        memcpy(tmp, options, numOptions * sizeof(JavaVMOption));
    }

    /* Release old options array */
    free(options);
    options = tmp;

    /* Add the new vm option */
    /*
     * 4355034: looks like a few callers are passing an automatic
     * variable as str; by the time we use options we no longer
     * own much of the memory in the argument list :(
     * Instead of enforcing a malloc-only string rule on all our
     * callers, let's just call strdup here.
     */
    /*
     * Removed unnecessary '\' character from the option string.
     */
    len = strlen(str);
    optionStr = (char*)malloc((len + 1) * sizeof(char));

    for (i=0, j=0; i < len; i++, j++)
      {
        if (str[i] == '\\')
          {
            i++;
          }

        if (i < len)
           optionStr[j] = str[i];
    }

    optionStr[j] = '\0';

    options[numOptions].optionString = optionStr;
    options[numOptions].extraInfo = info;

    startup_trace("New Option[%d] = %s\n",numOptions,optionStr);
    startup_trace("New Info[%d] = %s\n",numOptions,info == NULL ? "null" : info);

    numOptions++;
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

