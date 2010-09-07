#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "adaptercommon.h"
#include "IEgo.h"

extern IEgo * javaService;


#define envName "ns4plugin"PLUGIN_NODOTVERSION

void on_load() {

    fprintf(stderr,"dll being loaded\n");
    ac_createJavaService("Mozilla/4", (void **) &javaService);
}

#pragma init (on_load)
