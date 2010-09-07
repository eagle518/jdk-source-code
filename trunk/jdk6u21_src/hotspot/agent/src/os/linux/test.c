/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "libproc.h"

int main(int argc, char** argv) {
   struct ps_prochandle* ph;

   init_libproc(true);
   switch (argc) {
      case 2: {
         // process
         ph = Pgrab(atoi(argv[1]));
         break;
      }

      case 3: {
        // core
        ph = Pgrab_core(argv[1], argv[2]);
        break;
      }

      default: {
        printf("usage %s <pid> or %s <exec file> <core file>\n");
        return 1;
      }
   }

   if (ph) {
      Prelease(ph);
      return 0;
   } else {
      printf("can't connect to debuggee\n");
      return 1;
   }
}
