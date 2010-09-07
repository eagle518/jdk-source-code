/*
 * @(#)RegistryKey.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifndef REGISTRYKEY_H
#define REGISTRYKEY_H


/**
 * Meaning of the following variables:
 * - UNVERIFIED: this value has not yet been tested (and needs to be)
 * - TESTING: this value is currently being tested.  If we get this value
 * from the registry, this indicates that we probably crashed while we were
 * testing it last time, so we should disable the appropriate component.
 * - FAILURE: this component failed testing, so it should be disabled
 * - SUCCESS: this component succeeded, so we can enable it
 */
#define J2D_ACCEL_UNVERIFIED   -1
#define J2D_ACCEL_TESTING      0
#define J2D_ACCEL_FAILURE      1
#define J2D_ACCEL_SUCCESS      2

class RegistryKey {
public:
    RegistryKey(WCHAR *keyName, REGSAM permissions);
    ~RegistryKey();
        
    DWORD EnumerateSubKeys(DWORD index, WCHAR *subKeyName, 
			   DWORD *buffSize);
			   
    int  GetIntValue(WCHAR *valueName);

    static int  GetIntValue(WCHAR *keyName, WCHAR *valueName);

    BOOL SetIntValue(WCHAR *valueName, int regValue, BOOL flush);

    static BOOL SetIntValue(WCHAR *keyName, WCHAR *valueName, int regValue, 
			    BOOL flush);
    
    static void DeleteKey(WCHAR *keyName);

    static void PrintValue(WCHAR *keyName, WCHAR *valueName, 
			   WCHAR *msg);
    
private:
    HKEY hKey;
    static void PrintRegistryError(LONG errNum, char *message);
};

#endif REGISTRYKEY_H

