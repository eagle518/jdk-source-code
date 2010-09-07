/*
 * @(#)versionId.c	1.8 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include "versionId.h"
#include "system.h"

/* Parse a string to a version ID. It assumes that integers are
 * used in version IDs. This might go horrible wrong, but should
 * be right in most cases.
 */
void CreateVersionID(char *str, VersionID* vid) {
   int i, value;
   /* Reset version ID */
   vid->length = 0;
   for(i = 0; i < MAX_VERSIONID_LENGTH; i++) {
       vid->value[i] = 0;
   }

   /* Scan string */
   while(*str && vid->length < MAX_VERSIONID_LENGTH) {
       /* Parse value */
       value = 0;
       while(*str && isdigit(*str)) {
           value = (10 * value) + (*str - '0');
           str++;
       }
       /* Store value */
       vid->value[vid->length] = value;
       vid->length++;

       /* Skip until next digit sequence */
       while(*str && !isdigit(*str)) str++;
   }
}

/* Check if two version IDs are equal. Checks entire array. This
 * provides zero-padding for free
 */
int EqualsVersionID(VersionID* vid1, VersionID* vid2) {
    int i;
    for(i = 0; i < MAX_VERSIONID_LENGTH; i++) {
        if (vid1->value[i] != vid2->value[i]) return FALSE;
    }
    return TRUE;
}

/* Checks if vid1 is greater than or equal to vid2. Checks entire
 * array since that provides zero padding for free
 */
int GreaterThanOrEqualVersionID(VersionID* vid1, VersionID* vid2) {
    int i;
    
    for(i = 0; i < MAX_VERSIONID_LENGTH; i++) {
        if (vid1->value[i] > vid2->value[i]) return TRUE;
        if (vid1->value[i] < vid2->value[i]) return FALSE;
    }
    return TRUE; /* Equal */

}

/* Check if vid1 is a prefix of vid2 */
int PrefixMatchVersionID(VersionID* vid1, VersionID* vid2) {
    int i;
    for(i = 0; i < vid1->length; i++) {
        if (vid1->value[i] != vid2->value[i]) return FALSE;
    }
    return TRUE;
}

/* Debugging */
void PrintVersionID(char *msg, VersionID* vid) {
    int i;

    
    for(i = 0; i < MAX_VERSIONID_LENGTH; i++) {
        printf("%d", vid->value[i]);
        if (i < MAX_VERSIONID_LENGTH-1) printf(",");
    }
    printf("}\n");
}

/* Match the given version string against the version */
int MatchSimpleVersionString(char* verStr, char* version) {
    char* begin;
    char* end;
    char type;
    VersionID target;
    VersionID key;
    int result;
    /* Target version */
    CreateVersionID(version, &target);

    /* Create copy since we overwrite string */
    verStr = strdup(verStr); /* This is a tiny memory leak! */

    while(*verStr) {
        /* Skip spaces */
        while(*verStr && iswspace(*verStr)) verStr++;

        /* Find end of word */
        begin = verStr;
        while(*verStr && !iswspace(*verStr)) verStr++;
        end = verStr;

        if (*verStr) verStr++; /* advance past space */

        /* At least one character */
        if (begin < end) {
            /* Check if it is a prefix match or not */
            type = 0;
            if (*(end - 1) == '+') {
                type = '+';
                end--;
            } else if (*(end - 1) == '*') {
                type = '*';
                end--;
            }
            /* Null terminate string */
            *end = '\0';

            /* Setup version ID */
            CreateVersionID(begin, &key);
           
            /* Debugging
            PrintVersionID("key:", &key);
            PrintVersionID("target:", &target);
            */
            
            switch(type) {
                case '+': result = GreaterThanOrEqualVersionID(&target, &key); break;
                case '*': result = PrefixMatchVersionID(&key, &target); break;
                default:  result = EqualsVersionID(&key, &target); break;
            }

            /* Debugging
            printf("Result: %d\n\n", result);
            */

            if (result) return TRUE;
        }
    }
    return FALSE;
}

/* Match the given version string against the version */
int MatchVersionString(char* verStr, char* version) {
    int ret;
    char *amptr = strchr(verStr, '&');
    if (*verStr == '\0') { 	
	return FALSE; 		     /* gaurd against "1.3&" */
    }
    if (amptr == NULL) {
	return MatchSimpleVersionString(verStr, version);
    }
    *amptr = '\0';
    ret = MatchSimpleVersionString(verStr, version) &&
	  MatchVersionString((amptr + 1), version);
    *amptr = '&';
    return ret;
}



