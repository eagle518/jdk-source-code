/*
 * @(#)versionId.h	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef VERSIONID_H
#define VERSIONID_H

/*
 *  Minimal VersionID parser. It can handle all strings
 *  of the form <n1>.<n2>. ... . <n10>
 *
 * where n1,n2, .., n10 are integers. That should be good
 * enough for most purposes. If we guess wrong, the Java code
 * will do the right thing.
 *
 */
#define MAX_VERSIONID_LENGTH 10

typedef struct _VersionID {
  int length;
  int value[MAX_VERSIONID_LENGTH];
} VersionID;


void CreateVersionID(char *str, VersionID* vid);
int  EqualsVersionID(VersionID* vid1, VersionID* vid2);
int  GreaterThanOrEqualVersionID(VersionID* vid1, VersionID* vid2);
int  PrefixMatchVersionID(VersionID* vid1, VersionID* vid2);
int  MatchVersionString(char* verStr, char* version);

/* Debugging */
void PrintVersionID(char* msg, VersionID* vid);

#endif


