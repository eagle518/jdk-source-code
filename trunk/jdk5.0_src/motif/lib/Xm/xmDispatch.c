#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#pragma init (Setup_LD_LIBRARY_PATH_to_find_libXm)

#define MOTIF_INSTALLATION_BASEDIR "/usr/dt/lib/Xm/"
#define MOTIF_DEFAULT_DIR MOTIF_INSTALLATION_BASEDIR "default/"
#define MOTIF_VERSION_3_DIR MOTIF_INSTALLATION_BASEDIR "v3/"
#define MOTIF_VERSION_4_DIR MOTIF_INSTALLATION_BASEDIR "v4/"
#define MOTIF_SHARED_LIB "libXm.so"

#define GET_XM_VERSION(Xmversion,Xmrevision) (Xmversion * 1000 + Xmrevision)

#define LD_LIBRARY_PATH "LD_LIBRARY_PATH"

#ifdef LD_DEBUG
#define DBG_OUTPUT(x) fprintf(stderr, "xmDispatch: "),x
#else
#define DBG_OUTPUT(x)
#endif


void Setup_LD_LIBRARY_PATH_to_find_libXm()
{
	void *  handle;
	char *   dptr;
	char *version=(char*)0;
	char *newEnv;
	char *oldEnv;
	int len;
	static char *envEqual=LD_LIBRARY_PATH "=";
	int versionKnown=1;

	/* NB: VERSION DEPENDENT CODE!!! */
	/* The symbol 'xmUseVersion' is a global data symbol that is common to
	 * both libXm.so.3 and libXm.so.4 (and presumably also future versions!)
	 */
	if ((dptr = dlsym(RTLD_DEFAULT, "_XmVersionString")) == NULL) {
		/* No version of libXm loaded yet - choose default */
		DBG_OUTPUT(fprintf(stderr, "No version of libXm loaded yet - choose default\n"));
		version = MOTIF_DEFAULT_DIR;
	} else {
		/* Determine version that is already loaded */
		if (strstr(dptr, " 2.1") != 0) {
			version = MOTIF_VERSION_4_DIR;
		} else if (strstr(dptr, " 1.2") != 0) {
			version = MOTIF_VERSION_3_DIR;
		} else {
			/* version string doesn't match any known - choose default */
			version = MOTIF_DEFAULT_DIR;
			versionKnown=0;
		}
		if (versionKnown) {
			DBG_OUTPUT(fprintf(stderr, "Loading libXm \"%s\", version is %s\n", dptr, version));
		} else {
			DBG_OUTPUT(fprintf(stderr, "Warning - unrecognised libXm version string \"%s\", version is %s\n", dptr, version));
		}
	}
	if (oldEnv = getenv(LD_LIBRARY_PATH)) {
		len = strlen(oldEnv) + 2;
	} else {
		len = 1;
		oldEnv = envEqual;
	}

	len += strlen(LD_LIBRARY_PATH) + 3;

	newEnv = malloc(len + sizeof(MOTIF_DEFAULT_DIR));
	sprintf(newEnv, "%s%s%s%s", envEqual, version,
		(oldEnv == envEqual) ? "" : ":",  
		(oldEnv == envEqual) ? "" : oldEnv);
	putenv(newEnv);
	if ((handle = dlopen(MOTIF_SHARED_LIB, RTLD_LAZY)) == NULL) {
		(void) fprintf(stderr, "dlopen: %s\n", dlerror());
		exit (1);
	}
	putenv(oldEnv);
	free(newEnv);
}
