/*
 * @(#)PatchInPlace.hpp	1.9 10/06/07
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// PatchInPlace.hpp by Chris Gruszka    
//=--------------------------------------------------------------------------=
// Contains WinMain of the JInstall
//

#ifndef _PATCHINPLACE_H_
#define _PATCHINPLACE_H_


#define VER_VALUE_LEN  16
#define MODE_VALUE_LEN  16


class WhatsInstalled {
public:
    WhatsInstalled(void);                    // default constructor
    BOOL IsConsumerInstalled(void)          { 
                return (no_consumer_installed != _consumer_state); };
    BOOL IsSameVersionStaticInstalled(void) { 
                return (_same_static); };
    BOOL IsSameVersionConsumerInstalled(void) { 
                return (same_version_consumer_installed == _consumer_state); };
    BOOL IsSameVersionInstalled(void)       { 
                return (_same_static || (same_version_consumer_installed == _consumer_state)); };
    BOOL IsUpdateable(void)                 { 
                // Updateable does not imply Patchable, see IsPatchable()
                return (updateable_consumer_version_installed == _consumer_state); };
    BOOL IsPatchable()                      {
                // make sure updateable and _PatchFrom is set
                if (IsUpdateable() && (lstrcmp(_PatchFrom, "") != 0)) {
                    return TRUE;
                } else {
                    return FALSE;
                } };
    bool GetPatchFromString(TCHAR * pszPatchFromString, const size_t bufsize);
    BOOL IsNewerConsumerVersionInstalled(void);
    // CString is not available on our development environment on Win64
    TCHAR _installed_consumer_version[VER_VALUE_LEN];
private:
    BOOL    _same_static;
    enum    ConsumerStates {
        no_consumer_installed,
        same_version_consumer_installed,
        updateable_consumer_version_installed,
        other_consumer_version_installed
    } _consumer_state;
    TCHAR   _PatchFrom[BUFFER_SIZE];
    BOOL WhatsInstalled::IsVersionUpdateable(LPCTSTR ConsumerVersion, const CRegKey& jreRegKey);
};



LONG GetMode(const CRegKey& VersionRegKey, LPTSTR pszModeValue, ULONG* pnCharsModeValue)
{
    LONG lResult = ERROR_SUCCESS;

    CRegKey MSIRegKey;

    // open MSI reg key
    lResult = MSIRegKey.Open(VersionRegKey, TEXT("MSI"), KEY_READ );
    if (lResult != ERROR_SUCCESS) {
        // versions we are interested should have an MSI reg key
        return lResult;
    } else {

        // QueryValue Mode
        // warning C4996: 'ATL::CRegKey::QueryValue' was declared deprecated
        // but we have an old development environment on Win64 that does not have QueryStringValue
        lResult = MSIRegKey.QueryValue(pszModeValue, TEXT("MODE"), pnCharsModeValue);
        if (lResult != ERROR_SUCCESS) {
            // the same version should have an MSI\MODE value in the registry
            return lResult;
        } else {
            // have MODE in pszModeValue
        }
    }
    return lResult;
}


LONG GetJREMode(const CRegKey& jreRegKey, LPCTSTR szVersion, LPTSTR pszModeValue, ULONG* pnCharsModeValue)
{
    CRegKey VersionRegKey;
    LONG lResult = ERROR_SUCCESS;

    // check if the version is installed
    lResult = VersionRegKey.Open(jreRegKey, szVersion, KEY_READ );
    if (lResult != ERROR_SUCCESS) {
        // this version (e.g. "1.6.0_05") is not installed
        return lResult;
    } else {
        lResult = GetMode(VersionRegKey, pszModeValue, pnCharsModeValue);
    }
    return lResult;
}


WhatsInstalled::WhatsInstalled()
{
    // default constructor

    _same_static = FALSE;
    _consumer_state = no_consumer_installed;
    lstrcpy(_PatchFrom, "");
    lstrcpy(_installed_consumer_version, "");

    CRegKey jreRegKey;
    TCHAR szMessage[1024] = {NULL};
    LONG lResult = jreRegKey.Open(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\JavaSoft\\Java Runtime Environment"), KEY_READ );
    if (lResult != ERROR_SUCCESS) {
        return;                        // JRE not installed
    } else {
        // a JRE is installed

        TCHAR szFamilyVersionValueName[BUFFER_SIZE] = {NULL};
        ULONG nCharFamilyVer = VER_VALUE_LEN;

        wsprintf(szFamilyVersionValueName, "Java%sFamilyVersion", PLUGIN_MINOR_VERSION);

        // warning C4996: 'ATL::CRegKey::QueryValue' was declared deprecated
        // but we have an old development environment on Win64 that does not have QueryStringValue
        lResult = jreRegKey.QueryValue(_installed_consumer_version, szFamilyVersionValueName, &nCharFamilyVer);
        if (lResult != ERROR_SUCCESS) {
            // Consumer version not installed (properly)
        } else {
            // same version may be updateable to a later build
            if (WhatsInstalled::IsVersionUpdateable(_installed_consumer_version, jreRegKey)) {
                _consumer_state = updateable_consumer_version_installed;
            } else if (lstrcmp(VERSION, _installed_consumer_version) == 0) {     // if VERSION == version
                _consumer_state = same_version_consumer_installed;
            } else {
                _consumer_state = other_consumer_version_installed;
            }

            {
                // [installed_consumer_version]\MSI\MODE should not be static
                // _installed_consumer_version has to be installed
                // the same version should have an MSI reg key 
                //  and an MSI\MODE value in the registry

                ULONG nCharsModeValue = MODE_VALUE_LEN;
                TCHAR szModeValue[MODE_VALUE_LEN] = {NULL};

                lResult = GetJREMode( jreRegKey, _installed_consumer_version, szModeValue, &nCharsModeValue);
                if (lResult != ERROR_SUCCESS) {
                    wsprintf(szMessage, "Couldn't read MODE for %s=%s", szFamilyVersionValueName, _installed_consumer_version);
                    logit(szMessage);
                    lstrcpy(_installed_consumer_version, "");
                    _consumer_state = no_consumer_installed;
                } else {
                    if (lstrcmpi(szModeValue,"S") == 0) {
                        wsprintf( szMessage, "%s=%s has an inconsistent MODE=%s.",
                                    szFamilyVersionValueName, _installed_consumer_version, szModeValue);
                        logit(szMessage);
                        lstrcpy(_installed_consumer_version, "");
                        _consumer_state = no_consumer_installed;
                    } else { // not static, must be consumer
                        // _installed_consumer_version's MODE is not static
                        // it is OK
                    }
                }
            }
        }

        {
            // check if the same version is installed
            ULONG nCharsModeValue = MODE_VALUE_LEN;
            TCHAR szModeValue[MODE_VALUE_LEN] = {NULL};
            CRegKey VersionRegKey;

            // check if the version is installed
            lResult = VersionRegKey.Open(jreRegKey, TEXT(VERSION), KEY_READ );
            if (lResult != ERROR_SUCCESS) {
                // this version (e.g. "1.6.0_05") is not installed
            } else {
                // the same version should have an MSI reg key and an MSI\MODE value in the registry

                lResult = GetMode(VersionRegKey, szModeValue, &nCharsModeValue);
                if (lResult != ERROR_SUCCESS) {
                    wsprintf(szMessage, "Couldn't read MODE for version=%s", TEXT(VERSION));
                    logit(szMessage);
                    // ignore this version
                } else {
                    if (lstrcmpi(szModeValue,"S") == 0) {
                        _same_static = TRUE;
                    } else { // not static, must be consumer
                        if (lstrcmp(VERSION, _installed_consumer_version) == 0) {
                            // _installed_consumer_version from JavaFamilyVersion matches
                        } else if (lstrcmp(_installed_consumer_version, "") != 0) {
                            // let JavaFamilyVersion take precedence, but log the situation
                            wsprintf( szMessage, "There should only be one consumer version installed.  %s=%s, VERSION=%s",
                                        szFamilyVersionValueName, _installed_consumer_version, TEXT(VERSION));
                            logit(szMessage);
                        } else {
                            // JavaFamilyVersion wasn't set, but this VERSION is installed as consumer
                            wsprintf( szMessage, "%s wasn't set, but VERSION=%s is installed as consumer.",
                                    szFamilyVersionValueName, TEXT(VERSION));
                            logit(szMessage);

                            lstrcpy(_installed_consumer_version, VERSION);
                            _consumer_state = same_version_consumer_installed;
                        }
                    }
                }
            }
        }
    } 
}


// WhatsInstalled::GetPatchFromString(TCHAR * pszPatchFromString, const size_t bufsize)
//   pszPatchFromString - output, with substring to create name of patch MSI
//                      For example, if patching from 1.6.0_24-b77 to 1.6.0_25, 
//                      GetPatchFromString() will return the "24-b77" string to construct "jre1.6.0_25-pfrom24-b77.msi"
//   bufsize - input, size of buffer at pszPatchFromString
//   returns TRUE if successful

bool WhatsInstalled::GetPatchFromString(TCHAR * pszPatchFromString, const size_t bufsize)
{
    //Initialize all incoming strings
    pszPatchFromString[0] = NULL;

    char * pszUnderscore = strstr(_PatchFrom, "_");
    
    if (pszUnderscore != NULL) {
        //update release is found

        char * pszUpdate = pszUnderscore+1;

        size_t len = strlen(pszUpdate);
        if (len > bufsize) {
            return FALSE;
        }
        memcpy(pszPatchFromString, pszUpdate, len);
        pszPatchFromString[len] = 0;

        return TRUE;

    } 

    return FALSE;
    
};


BOOL WhatsInstalled::IsNewerConsumerVersionInstalled()
{
    BOOL bNewerConsumerInstalled = FALSE;

    if (lstrcmpi(_installed_consumer_version, VERSION) > 0) {
        bNewerConsumerInstalled = TRUE;
    }
    return bNewerConsumerInstalled;
}


BOOL WhatsInstalled::IsVersionUpdateable(LPCTSTR InstalledConsumerVersion, const CRegKey& jreRegKey)
{
    BOOL    bUpdateable = FALSE;

    lstrcpy(_PatchFrom, "");

#ifdef PATCH_LIST
// see obj\PatchStruct.h created by Makefile

    for (int i = 0; i < (sizeof(Patches)/sizeof(PATCH_INFO)); i++) {

        // if the version matches one in the patch list
        if (lstrcmpi(Patches[i].szVersion, InstalledConsumerVersion) == 0) {       
            // The release can be Updated (update 6uN Consumer to 6uM Consumer)
            // Updateable does not imply Patchable, see IsPatchable()
            bUpdateable = TRUE;

            CRegKey VersionRegKey;

            LONG lResult = VersionRegKey.Open(jreRegKey, InstalledConsumerVersion, KEY_READ );
            if (lResult == ERROR_SUCCESS) {
                CRegKey MSIRegKey;

                lResult = MSIRegKey.Open(VersionRegKey, TEXT("MSI"), KEY_READ );
                if (lResult == ERROR_SUCCESS) {

                    TCHAR InstalledConsumerVersionCkSum[BUFFER_SIZE] = {NULL};
                    ULONG nCharsCkSum = BUFFER_SIZE;

                    // QueryValue ImageCkSum
                    // warning C4996: 'ATL::CRegKey::QueryValue' was declared deprecated
                    // but we have an old development environment on Win64 that does not have QueryStringValue
                    lResult = MSIRegKey.QueryValue(InstalledConsumerVersionCkSum, TEXT("ImageCkSum"), &nCharsCkSum);
                    if (ERROR_SUCCESS == lResult) {
                        // The following block handles CR 6953173, which covers ImageCkSum issues with 6u17, 6u18, 6u19, and 6u20 Enhanced Plus Builds.
                        if (lstrcmp(InstalledConsumerVersion, "1.6.0_17") == 0) {
                            // If 1.6.0_17 ImageCkSum is blank, use 3407744024.
                            // Also handle 6u17-b79 which has ImageCkSum==793498781 and 6u17-b81 which has ImageCkSum==4130599865,
                            // but are patchable as ImageCkSum=3407744024.

                            lstrcpy(InstalledConsumerVersionCkSum, "3407744024");

                        } else if (lstrcmp(InstalledConsumerVersionCkSum, "") == 0) {
                            // If 1.6.0_18 ImageCkSum is blank, use 95497213
                            // If 1.6.0_19 ImageCkSum is blank, use 1439521033
                            // If 1.6.0_20 ImageCkSum is blank, use 3505863356

                            if (lstrcmp(InstalledConsumerVersion, "1.6.0_18") == 0) {
                                lstrcpy(InstalledConsumerVersionCkSum, "95497213");
                            } else if (lstrcmp(InstalledConsumerVersion, "1.6.0_19") == 0) {
                                lstrcpy(InstalledConsumerVersionCkSum, "1439521033");
                            } else if (lstrcmp(InstalledConsumerVersion, "1.6.0_20") == 0) {
                                lstrcpy(InstalledConsumerVersionCkSum, "3505863356");
                            }
                        }

                        if (lstrcmpi(Patches[i].szCkSum, InstalledConsumerVersionCkSum) == 0) {       
                            // The release can be Patched In Place
                            lstrcpy(_PatchFrom, Patches[i].szName);
                            break;
                        }
                        // else there may be an entry for another build that has a matching checksum,
                        // continue for loop . . .
                    }
                }
            }
        }
         
        // get the next version
    }
#endif // PATCH_LIST
    return bUpdateable;
}

#endif // _PATCHINPLACE_H_
