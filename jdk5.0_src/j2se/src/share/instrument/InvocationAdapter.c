/*
 * @(#)InvocationAdapter.c	1.8 04/06/14
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#include    <string.h>
#include    <stdlib.h>

#include    "jni.h"

#include    "Utilities.h"   
#include    "JPLISAssert.h"
#include    "JPLISAgent.h"
#include    "JavaExceptions.h"

#include    "EncodingSupport.h"
#include    "FileSystemSupport.h"
#include    "JarFacade.h"
#include    "PathCharsValidator.h"

#define JAVA_CLASS_PATH_PROP			"java.class.path"

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/**
 * This module contains the direct interface points with the JVMTI.
 * The OnLoad handler is here, along with the various event handlers.
 */


/* Forward references */

static int 
parseManifest(const char* name, 	       
	      char** premainClass,
	      char** bootClassPath,
	      jboolean* canRedefineClasses);

static void
appendClassPath(JPLISAgent* agent,
	        const char* jarfile);

static void
appendBootClassPath(JPLISAgent* agent,
		    const char* jarfile,
		    const char* pathList);


/*
 * Parse -javaagent tail, of the form name[=options], into name 
 * and options. Returned values are heap allocated and options maybe
 * NULL. Returns 0 if parse succeeds, -1 if allocation fails.
 */
static int 
parseArgumentTail(char* tail, char** name, char** options) {
    int len;
    char* pos;

    pos = strchr(tail, '=');
    len = (pos == NULL) ? strlen(tail) : (pos - tail);

    *name = (char*)malloc(len+1);
    if (*name == NULL) {
	return -1;
    }
    memcpy(*name, tail, len);
    (*name)[len] = '\0';

    if (pos == NULL) {
	*options = NULL;
    } else {
	char * str = (char*)malloc( strlen(pos + 1) + 1 );
	if (str == NULL) {
	    free(*name);
	    return -1;
	}
	strcpy(str, pos +1);
	*options = str;
    }
    return 0;
}


/*
 *  This will be called once for every -javaagent on the command line.
 *  The structure of the data sharing in this code depends on the fact
 *  that multiple calls to Agent_OnLoad during a single instantiation of the JVM
 *  will share the same static data world.
 *
 *  The JVMTI spec guarantees that Agent_OnLoad(s) will be called serially, so
 *  synchronization against multiple copies of ourself is not necessary.
 *  The first time we are called, we allocate our native agent data structure
 *  and store it in a static pointer.
 *  Subsequent calls look up the already allocated native data structure.
 *  In every case we capture the command line for later processing (when Java is available)
 *
 *  The argument tail string provided to Agent_OnLoad will be of form
 *  <jarfile>[=<options>]. The tail string is split into the jarfile and
 *  options components. The jarfile manifest is parsed and the value of the
 *  Premain-Class attribute will become the agent's premain class. The jar  
 *  file is then added to the system class path, and if the Boot-Class-Path
 *  attribute is present then all relative URLs in the value are processed
 *  to create boot class path segments to append to the boot class path.
 */
JNIEXPORT jint JNICALL 
Agent_OnLoad(JavaVM *vm, char *tail, void * reserved) {
    JPLISInitializationError initerror  = JPLIS_INIT_ERROR_NONE;
    jint                     result     = JNI_OK;
    JPLISAgent *             agent      = NULL;

    initerror = insureSingletonJPLISAgent(vm, &agent);
    if ( initerror == JPLIS_INIT_ERROR_NONE ) {
	int		oldLen, newLen;
	char * 		jarfile;
	char *		options;
	char * 	 	ext;
	char *		premainClass;
	char *		bootClassPath;
	jboolean	canRedefineClasses;

	/*
	 * Parse <jarfile>[=options] into jarfile and options
	 */
	if (parseArgumentTail(tail, &jarfile, &options) != 0) {
	    fprintf(stderr, "-javaagent: memory allocation failure.\n");
	    return JNI_ERR;
	}

	/*
	 * Agent_OnLoad is specified to provide the agent options
	 * argument tail in modified UTF8. However for 1.5.0 this is
	 * actually in the platform encoding - see 5049313. 
	 *
	 * Open zip/jar file and parse archive. If can't be opened or
	 * not a zip file return error. Also if Premain-Class attribute
	 * isn't present we return an error.
	 */
	if (parseManifest( jarfile, 
			   &premainClass, 
			   &bootClassPath,
			   &canRedefineClasses)) {
	    fprintf(stderr, "Error opening zip file: %s\n", jarfile);
	    return JNI_ERR;
        }
	if (premainClass == NULL) {
	    fprintf(stderr, "Failed to load Premain-Class manifest attribute from %s\n",
		jarfile);
	    return JNI_ERR;
	}

	/*
	 * Add to the jarfile 
	 */
	appendClassPath(agent, jarfile);

	/*
	 * The value of the Premain-Class attribute becomes the agent
	 * class name. The manifest is in UTF8 so need to convert to
	 * modified UTF8 (see JNI spec).
	 */
	oldLen = strlen(premainClass);
	newLen = modifiedUtf8LengthOfUtf8(premainClass, oldLen);
	if (newLen != oldLen) {
	    char* str = (char*)malloc( newLen+1 );
	    if (str == NULL) {
	        fprintf(stderr, "-javaagent: memory allocation failed\n");
	   	return JNI_ERR;
	    }
	    convertUtf8ToModifiedUtf8(premainClass, oldLen, str, newLen);
	    free(premainClass);
	    premainClass = str;
	}

	/*
	 * If the Boot-Class-Path attribute is specified then we process
	 * each relative URL and add it to the bootclasspath.
	 */
	if (bootClassPath != NULL) {
	    appendBootClassPath(agent, jarfile, bootClassPath);
        }

	/*
	 * Track (record) the agent details 
	 */
        initerror = trackJavaAgentCommandLine( agent, premainClass, options, canRedefineClasses );

	/*
	 * Clean-up
	 */
	free(jarfile);
	if (options != NULL) free(options);
	free(premainClass);
	if (bootClassPath != NULL) free(bootClassPath);
    }
    
    switch (initerror) {
    case JPLIS_INIT_ERROR_NONE:
      result = JNI_OK;
      break;
    case JPLIS_INIT_ERROR_CANNOT_CREATE_NATIVE_AGENT:
      result = JNI_ERR;
      fprintf(stderr, "java.lang.instrument/-javaagent: cannot create native agent.\n");
      break;
    case JPLIS_INIT_ERROR_FAILURE:
      result = JNI_ERR;
      fprintf(stderr, "java.lang.instrument/-javaagent: initialization of native agent failed.\n");
      break;
    case JPLIS_INIT_ERROR_ALLOCATION_FAILURE:
      result = JNI_ERR;
      fprintf(stderr, "java.lang.instrument/-javaagent: allocation failure.\n");
      break;
    case JPLIS_INIT_ERROR_AGENT_CLASS_NOT_SPECIFIED:
      result = JNI_ERR;
      fprintf(stderr, "-javaagent: agent class not specified.\n");
      break;
    default: 
      result = JNI_ERR;
      fprintf(stderr, "java.lang.instrument/-javaagent: unknown error\n");
      break;
    }
    return result;
}

JNIEXPORT void JNICALL 
Agent_OnUnload(JavaVM *vm) {
}


/*
 *  JVMTI callback support
 *
 *  We have two "stages" of callback support.
 *  At OnLoad time, we install a VMInit handler.
 *  When the VMInit handler runs, we remove the VMInit handler and install a
 *  ClassFileLoadHook handler.
 */

void JNICALL
eventHandlerVMInit( jvmtiEnv *      jvmtienv,
                    JNIEnv *        jnienv,
                    jthread         thread) {
    JPLISAgent *    agent   = NULL;
    jboolean        success = JNI_FALSE;

    agent = getSingletonJPLISAgent(jvmtienv);
    
    /* process the premain calls on the all the JPL agents */
    if ( agent != NULL ) {
        jthrowable outstandingException = preserveThrowable(jnienv);
        success = processJavaStart( agent,
                                    jnienv);
        restoreThrowable(jnienv, outstandingException);
    }

    /* if we fail to start cleanly, bring down the JVM */
    if ( !success ) {
        abortJVM(jnienv, JPLIS_ERRORMESSAGE_CANNOTSTART);
    }
} 
 
void JNICALL
eventHandlerClassFileLoadHook(  jvmtiEnv *              jvmtienv,
                                JNIEnv *                jnienv,
                                jclass                  class_being_redefined,
                                jobject                 loader, 
                                const char*             name, 
                                jobject                 protectionDomain,
                                jint                    class_data_len, 
                                const unsigned char*    class_data, 
                                jint*                   new_class_data_len, 
                                unsigned char**         new_class_data) {
    JPLISAgent * agent;

    agent = getSingletonJPLISAgent(jvmtienv);
    
    /* if something is internally inconsistent (no agent), just silently return without touching the buffer */
    if ( agent != NULL ) {
        jthrowable outstandingException = preserveThrowable(jnienv);
        transformClassFile( agent,
                            jnienv,
                            loader,
                            name,
                            class_being_redefined, 
                            protectionDomain,
                            class_data_len,
                            class_data,
                            new_class_data_len,
                            new_class_data);
        restoreThrowable(jnienv, outstandingException);
    }
}


/*
 * Parse the manifest of the specified jar/zip file and returns the 
 * values of the Premain-Class, Boot-Class-Path, and Can-Redefine-Classes
 * attributes. The value of Premain-Class is returned as a heap allocated
 * string, or NULL if the attribute is not present. The value of
 * Boot-Class-Path is also heap allocated, or NULL if the attribute is
 * not present. The value of the Can-Redefine-Classes attribute is returned
 * as a boolean. If present, with a value of "true" (case insenstitive)
 * then JNI_TRUE is returned, false otherwise.
 * The function returns 0 if the manifest was successfully parsed, or 
 * non-0 otherwise.
 */
static int 
parseManifest( const char* name, 	       
	       char** premainClass,
	       char** bootClassPath,
	       jboolean* canRedefineClasses )
{	      
    char* attributes[] = { "Premain-Class", "Boot-Class-Path", "Can-Redefine-Classes" };
    char* values[3];
    int rc;

    rc = parseJarFile(name, 3, attributes, values);
    if (rc == 0) {
	*premainClass = values[0];
	*bootClassPath = values[1];
	if (values[2] != NULL && strcasecmp(values[2], "true") == 0) {
	    *canRedefineClasses = JNI_TRUE;
	} else {
	    *canRedefineClasses = JNI_FALSE;
	}
	if (values[2] != NULL) {
	    free(values[2]);
	}
    }

    return rc;
}


/*
 * URLs in Boot-Class-Path attributes are separated by one or more spaces.
 * This function splits the attribute value into a list of path segments.
 * The attribute value is in UTF8 but cannot contain NUL. Also non US-ASCII
 * characters must be escaped (URI syntax) so safe to iterate through the
 * value as a C string.
 */
static void 
splitPathList(const char* str, int* pathCount, char*** paths) {
    int count = 0;
    char** segments = NULL;
    char* c = (char*) str;
    while (*c != '\0') {
        while (*c == ' ') c++;		/* skip leading spaces */
	if (*c == '\0') {
	    break;
	}
	if (segments == NULL) {
	    segments = (char**)malloc( sizeof(char**) );
	} else {
	    segments = (char**)realloc( segments, (count+1)*sizeof(char**) );
	}
	jplis_assert(segments != NULL);
	segments[count++] = c;
	c = strchr(c, ' ');
	if (c == NULL) {
	    break;
	}
	*c = '\0';
	c++;
    }
    *pathCount = count;
    *paths = segments;
}


/* URI path decoding - ported from src/share/classes/java/net/URI.java */

static int 
decodeNibble(char c) {
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;
    return -1;
}

static int 
decodeByte(char c1, char c2) {
    return (((decodeNibble(c1) & 0xf) << 4) | ((decodeNibble(c2) & 0xf) << 0));
}

/* 
 * Evaluates all escapes in s.  Assumes that escapes are well-formed 
 * syntactically, i.e., of the form %XX.
 * If the path does not require decoding the the original path is
 * returned. Otherwise the decoded path (heap allocated) is returned,
 * along with the length of the decoded path. Note that the return
 * string will not be null terminated after decoding.
 */
static
char *decodePath(const char *s, int* decodedLen) {
    int n;
    char *result;
    char *resultp;
    int c;
    int i;

    n = strlen(s);
    if (n == 0) {
        *decodedLen = 0;
        return (char*)s;
    }
    if (strchr(s, '%') == NULL) {
        *decodedLen = n;
        return (char*)s; /* no escapes, we are done */
    }

    resultp = result = calloc(n+1, 1);
    c = s[0];
    for (i = 0; i < n;) {
        if (c != '%') {
            *resultp++ = c;
            if (++i >= n)
	        break;
	    c = s[i];
	    continue;
	}
        for (;;) {
            int b1 = s[++i];
            int b2 = s[++i];
            int decoded = decodeByte(b1, b2);
            *resultp++ = decoded;
            if (++i >= n)
	        break;
            c = s[i];
            if (c != '%')
	        break;
	}
    }
    *decodedLen = (resultp - result);
    return result; // not null terminated.
}

/*
 * Append the given jar file to the system class path.
 * Uses JVMTI get value of java.class.path property, appends jarfile
 * and sets the new value.
 *
 * Note: JVMTI GetSystemProperty/SetSystemProperty are specified to
 * use modified UTF8. However for 1.5.0 this is not implemented so
 * the platform encoding is used.
 */
static void
appendClassPath( JPLISAgent* agent,
                 const char* jarfile ) {
    char* old_value;
    char* new_value;
    int len;
    jvmtiEnv* jvmtienv = agent->mJVMTIEnv;
    jvmtiError jvmtierr;

    jvmtierr = (*jvmtienv)->GetSystemProperty(jvmtienv, JAVA_CLASS_PATH_PROP, &old_value);
    jplis_assert(jvmtierr == JVMTI_ERROR_NONE);

    /* Append :<jarfile> or ;<jarfile> depending on platform */
    len = strlen(old_value);
    new_value = (char*)malloc(len + strlen(jarfile) + 2);
    jplis_assert(new_value != NULL);
    memcpy(new_value, old_value, len);
    new_value[len++] = pathSeparator();
    strcpy(new_value+len, jarfile);

    jvmtierr = (*jvmtienv)->SetSystemProperty(jvmtienv, JAVA_CLASS_PATH_PROP, new_value);
    jplis_assert(jvmtierr == JVMTI_ERROR_NONE);
 
    (*jvmtienv)->Deallocate(jvmtienv, (unsigned char*)old_value); 
    free(new_value);
}


/* 
 * res = func, free'ing the previous value of 'res' if function
 * returns a new result.
 */
#define TRANSFORM(res,func) { 	 \
    char* tmp = func;		 \
    if (tmp != res) {        	 \
	free(res);               \
	res = tmp;	  	 \
    }				 \
    jplis_assert(res != NULL);	 \
}


/*
 * This function takes the value of the Boot-Class-Path attribute,
 * splits it into the individual path segments, and then combines it
 * with the path to the jar file to create the path to be added
 * to the bootclasspath.
 *
 * Each individual path segment starts out as a UTF8 string. Additionally
 * as the path is specified to use URI path syntax all non US-ASCII 
 * characters are escaped. Once the URI path is decoded we get a UTF8
 * string which must then be converted to the platform encoding (as it
 * will be combined with the platform path of the jar file). Once 
 * converted it is then normalized (remove duplicate slashes, etc.).
 * If the resulting path is an absolute path (starts with a slash for
 * example) then the path will be added to the bootclasspath. Otherwise
 * if it's not absolute then we get the canoncial path of the agent jar
 * file and then resolve the path in the context of the base path of
 * the agent jar.
 */
static void
appendBootClassPath( JPLISAgent* agent,
		     const char* jarfile, 
		     const char* pathList ) {
    char canonicalPath[MAXPATHLEN];
    char *parent = NULL;
    int haveBasePath = 0;

    int count, i;
    char **paths;
    jvmtiEnv* jvmtienv = agent->mJVMTIEnv;

    /*
     * Split the attribute value into the individual path segments
     * and process each in sequence
     */
    splitPathList(pathList, &count, &paths);

    for (i=0; i<count; i++) {
	int len;
	char* path;
	char* pos;

	/*
	 * The path segment at this point is a pointer into the attribute
	 * value. As it will go through a number of transformation (tossing away 
         * the previous results as we go along) it make it easier if the path
	 * starts out as a heap allocated string.
	 */
	path = strdup(paths[i]);
	jplis_assert(path != NULL);

	/*
	 * The attribute is specified to be a list of relative URIs so in theory
	 * there could be a query component - if so, get rid of it.
	 */
	pos = strchr(path, '?');
	if (pos != NULL) {
	    *pos = '\0';
	}

        /*
	 * Check for characters that are not allowed in the path component of
 	 * a URI.
  	 */
        if (validatePathChars(path)) {
            fprintf(stderr, "WARNING: illegal character in Boot-Class-Path value: %s\n",
               path);
            free(path);
            continue;
        }


	/*
	 * Next decode any escaped characters. The result is a UTF8 string.
	 */
	TRANSFORM(path, decodePath(path,&len));

	/* 
	 * Convert to the platform encoding
	 */
	{
	    char platform[MAXPATHLEN];
	    int new_len = convertUft8ToPlatformString(path, len, platform, MAXPATHLEN);
	    free(path);
	    if (new_len  < 0) {
		/* bogus value - exceeds maximum path size or unable to convert */
	 	continue;
	    }
	    path = strdup(platform);
	    jplis_assert(path != NULL);
	}

	/*
	 * Post-process the URI path - needed on Windows to transform
	 * /c:/foo to c:/foo. 
	 */
	TRANSFORM(path, fromURIPath(path));

	/*
	 * Normalize the path - no duplicate slashes (except UNCs on Windows), trailing
 	 * slash removed.
	 */
	TRANSFORM(path, normalize(path));

	/*
	 * If the path is an absolute path then add to the bootclassloader
	 * search path. Otherwise we get the canonical path of the agent jar
         * and then use its base path (directory) to resolve the given path
         * segment.
 	 *
 	 * NOTE: JVMTI is specified to use modified UTF8 strings (like JNI). 
 	 * In 1.5.0 the AddToBootstrapClassLoaderSearch takes a platform string
  	 * - see 5049313.
	 */
	if (isAbsolute(path)) {
	    (*jvmtienv)->AddToBootstrapClassLoaderSearch(jvmtienv, path);
	} else {
	    char* resolved;

	    if (!haveBasePath) {
	        if (canonicalize((char*)jarfile, canonicalPath, sizeof(canonicalPath)) != 0) {
		    fprintf(stderr, "WARNING: unable to canonicalize %s\n", jarfile);
		    free(path);
		    continue;
		} 
		parent = basePath(canonicalPath);
		jplis_assert(parent != NULL);
		haveBasePath = 1;
	    }

	    resolved = resolve(parent, path);
	    (*jvmtienv)->AddToBootstrapClassLoaderSearch(jvmtienv, resolved);
	}

	/* finished with the path */
	free(path);	
    }


    /* clean-up */
    if (haveBasePath && parent != canonicalPath) {
	free(parent);
    }
}



