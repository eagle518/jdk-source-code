#include <stdio.h>
#include <limits.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include "adaptercommon.h"
#include "IVersion.h"


void (*fptr)(const char *, void **) = NULL;

void ac_loadDLLandFindSym() {

    Dl_info dlinfo;
    char buf[PATH_MAX];
    char real[PATH_MAX];
    static int adapterdummy;
    dladdr((void *)&adapterdummy, &dlinfo);
    strcpy(buf, dlinfo.dli_fname);

    if (realpath(buf, real) == NULL) {
        fprintf(stderr, "Error: realpath(`%s') failed.\n", buf);
        return;
    }

    // this is a weasly trick to prevent the adapters from being
    // unloaded from the process when the browser calls dlclose
    void *thisdll = dlopen(real, RTLD_NOW | RTLD_GLOBAL);

    *(strrchr(real, '/')) = '\0';  /* library file         */
    *(strrchr(real, '/')) = '\0';  /* containing directory */
    *(strrchr(real, '/')) = '\0';  /* $(ARCH) subdirectory */
    *(strrchr(real, '/')) = '\0';  /* plugin subdirectory  */

    strcat(real,"/lib/"ARCH);
#if defined(DEBUG_VERSION)
    strcat(real,"/libjcomponent_g.so"); 
#else
    strcat(real,"/libjcomponent.so"); 
#endif
    
    void * dll = dlopen(real,
                        RTLD_NOW | RTLD_GLOBAL);
    if (dll == NULL) {
       fprintf(stderr,"dlopen failed: %s\n",dlerror());
    } else {
       fptr = (void (*)(const char *, void **)) dlsym( dll,
                                                       "createJavaService");
       if (fptr == NULL) {
          fprintf(stderr,"dlsym failed\n");
       }
    }
}

void ac_createJavaService(const char * userAgent, void ** ppv) {

    if (fptr == NULL) {
        ac_loadDLLandFindSym();
    }

    (*fptr)(userAgent,ppv);
}

void ac_createMimeTable(IEgo * js, 
                        char ** table) {
    int tableSize = 2048;
    char buff[2048];
    int tableUsed = 0;
    IVersion * vers;
    const char **supported,**containing;


    js->QI(IVersion_IID,(void **) &vers);
    vers->supportedVersions(&supported);
    vers->containingVersions(&containing);
    vers->release();

    *table = (char *) malloc(tableSize);

    sprintf(buff,"application/x-java-applet::Java(tm) Plug-in;application/x-java-bean::Java(tm) Plug-in");
    strcpy(*table,buff);
    tableUsed += strlen(buff);

    while(*supported != NULL) {
        sprintf(buff,";application/x-java-applet;version=%s::Java(tm) Plug-in;application/x-java-bean;version=%s::Java(tm) Plug-in",*supported,*supported);
        int used = tableUsed + strlen(buff);
        if (used >= tableSize) {
            tableSize += 2048;
            *table = (char *) realloc(*table,tableSize);
        }
        strcat(*table,buff);
        tableUsed += strlen(buff);
        supported++;
    }

    while(*containing != NULL) {
        sprintf(buff,";application/x-java-applet;jpi-version=%s::Java(tm) Plug-in;application/x-java-bean;jpi-version=%s::Java(tm) Plug-in",*containing,*containing);
        int used = tableUsed + strlen(buff);
        if (used >= tableSize) {
            tableSize += 2048;
            *table = (char *) realloc(*table,tableSize);
        }
        strcat(*table,buff);
        tableUsed += strlen(buff);
        containing++;
    }
}
