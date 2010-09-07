/*
 * @(#)MozExports.cpp	1.6 10/03/24 12:03:41
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "StdAfx.h"
#include "npapi.h"
#include "npfunctions.h"
#include "npruntime.h"
#include "MozExports.h"

extern NPNetscapeFuncs*   g_pMozillaFuncs;  
extern bool               g_haveCookieAndProxyNPAPIs;  

extern "C" {

NPError MozNPN_GetURLNotify(NPP instance, const char *url, const char *target, 
                         void* notifyData) {

  return g_pMozillaFuncs->geturlnotify(instance, url, target, notifyData);
}

NPError MozNPN_GetURL(NPP instance, const char *url, const char *target) {
  return g_pMozillaFuncs->geturl(instance, url, target);
}

NPError MozNPN_PostURLNotify(NPP instance, const char* url, const char* window, 
                          uint32_t len, const char* buf, NPBool file, 
                          void* notifyData) {
  return g_pMozillaFuncs->posturlnotify(instance, url, window, len, buf, file, 
                                            notifyData);
}

NPError MozNPN_PostURL(NPP instance, const char* url, const char* window, 
                    uint32_t len, const char* buf, NPBool file) {
  return g_pMozillaFuncs->posturl(instance, url, window, len, buf, file);
}

NPError MozNPN_RequestRead(NPStream* stream, NPByteRange* rangeList) {
  return g_pMozillaFuncs->requestread(stream, rangeList);
}

NPError MozNPN_NewStream(NPP instance, NPMIMEType type, const char* target, 
                      NPStream** stream) {
  return g_pMozillaFuncs->newstream(instance, type, target, stream);
}
  
int32_t MozNPN_Write(NPP instance, NPStream *stream, int32_t len, void *buffer) {
  return g_pMozillaFuncs->write(instance, stream, len, buffer);
}

NPError MozNPN_DestroyStream(NPP instance, NPStream* stream, NPError reason) {
  return  g_pMozillaFuncs->destroystream(instance, stream, reason);
}

void MozNPN_Status(NPP instance, const char *message) {
  g_pMozillaFuncs->status(instance, message);
}

const char* MozNPN_UserAgent(NPP instance) {
  return  g_pMozillaFuncs->uagent(instance);
}

void* MozNPN_MemAlloc(uint32_t size) {
  return  g_pMozillaFuncs->memalloc(size);
}

uint32_t MozNPN_MemFlush(uint32_t size) {
  return  g_pMozillaFuncs->memflush(size);
}

void MozNPN_MemFree(void* ptr) {
  g_pMozillaFuncs->memfree(ptr);
}

NPError MozNPN_GetValue(NPP instance, NPNVariable variable, void *value) {
  return  g_pMozillaFuncs->getvalue(instance, variable, value);
}

NPError MozNPN_SetValue(NPP instance, NPPVariable variable, void *value) {
  return  g_pMozillaFuncs->setvalue(instance, variable, value);
}

void MozNPN_InvalidateRect(NPP instance, NPRect *invalidRect) {
  return  g_pMozillaFuncs->invalidaterect(instance, invalidRect);
}

void MozNPN_ForceRedraw(NPP instance) {
  g_pMozillaFuncs->forceredraw(instance);
}

void MozNPN_PluginThreadAsyncCall(NPP instance, void (*func) (void *), void *userData) {
  g_pMozillaFuncs->pluginthreadasynccall(instance, func, userData);
}

NPIdentifier MozNPN_GetStringIdentifier(const NPUTF8 *name) {
  return g_pMozillaFuncs->getstringidentifier(name);
}

void MozNPN_GetStringIdentifiers(const NPUTF8 **names, int32_t nameCount, 
                              NPIdentifier *identifiers) {
  g_pMozillaFuncs->getstringidentifiers(names, nameCount, identifiers);
}

NPIdentifier MozNPN_GetIntIdentifier(int32_t intid) {
  return g_pMozillaFuncs->getintidentifier(intid);
}

bool MozNPN_IdentifierIsString(NPIdentifier identifier) {
  return g_pMozillaFuncs->identifierisstring(identifier);
}

NPUTF8* MozNPN_UTF8FromIdentifier(NPIdentifier identifier) {
  return  g_pMozillaFuncs->utf8fromidentifier(identifier);
}

int32_t MozNPN_IntFromIdentifier(NPIdentifier identifier) {
  return  g_pMozillaFuncs->intfromidentifier(identifier);
}

NPObject *MozNPN_CreateObject(NPP npp, NPClass *aClass) {
  return  g_pMozillaFuncs->createobject(npp, aClass);
}

NPObject *MozNPN_RetainObject(NPObject *npobj) {
  return  g_pMozillaFuncs->retainobject(npobj);
}

void MozNPN_ReleaseObject(NPObject *npobj) {
  g_pMozillaFuncs->releaseobject(npobj);
}

bool MozNPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName,
                const NPVariant *args, uint32_t argCount, NPVariant *result) {
  return  g_pMozillaFuncs->invoke(npp, npobj, methodName, args, argCount, result);
}

bool MozNPN_InvokeDefault(NPP npp, NPObject *npobj, const NPVariant *args,
                       uint32_t argCount, NPVariant *result) {
  return  g_pMozillaFuncs->invokeDefault(npp, npobj, args, argCount, result);
}

bool MozNPN_Evaluate(NPP npp, NPObject *npobj, NPString *script,
                  NPVariant *result) {
  return  g_pMozillaFuncs->evaluate(npp, npobj, script, result);
}

bool MozNPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                     NPVariant *result) {
  return  g_pMozillaFuncs->getproperty(npp, npobj, propertyName, result);
}

bool MozNPN_SetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                     const NPVariant *value) {
  return  g_pMozillaFuncs->setproperty(npp, npobj, propertyName, value);
}

bool MozNPN_RemoveProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName) {
  return  g_pMozillaFuncs->removeproperty(npp, npobj, propertyName);
}

bool MozNPN_HasProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName) {
  return  g_pMozillaFuncs->hasproperty(npp, npobj, propertyName);
}

bool MozNPN_HasMethod(NPP npp, NPObject *npobj, NPIdentifier methodName) {
  return  g_pMozillaFuncs->hasmethod(npp, npobj, methodName);
}

void MozNPN_ReleaseVariantValue(NPVariant *variant) {
  g_pMozillaFuncs->releasevariantvalue(variant);
}

void MozNPN_SetException(NPObject *npobj, const NPUTF8 *message) {
  g_pMozillaFuncs->setexception(npobj, message);
}

void MozNPN_PushPopupsEnabledState(NPP instance, bool enable) {
  g_pMozillaFuncs->pushpopupsenabledstate(instance, enable);
}

void MozNPN_PopPopupsEnabledState(NPP instance) {
  g_pMozillaFuncs->poppopupsenabledstate(instance);
}

NPError MozNPN_GetValueForURL(NPP npp, NPNURLVariable variable, const char *url, char **value, uint32_t *len) {
    // Guard against incorrect usage
    if (!g_haveCookieAndProxyNPAPIs) {
        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    }

    return g_pMozillaFuncs->getvalueforurl(npp, variable, url, value, len);
}

NPError MozNPN_SetValueForURL(NPP npp, NPNURLVariable variable, const char *url, const char *value, uint32_t len) {
    // Guard against incorrect usage
    if (!g_haveCookieAndProxyNPAPIs) {
        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    }

    return g_pMozillaFuncs->setvalueforurl(npp, variable, url, value, len);
}

NPError MozNPN_GetAuthenticationInfo(NPP npp, const char *protocol, const char *host, int32_t port, const char *scheme, const char *realm, char **username, uint32_t *ulen, char **password, uint32_t *plen) {
    // Guard against incorrect usage
    if (!g_haveCookieAndProxyNPAPIs) {
        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    }

    return g_pMozillaFuncs->getauthenticationinfo(npp, protocol, host, port, scheme, realm, username, ulen, password, plen);
}

} // extern "C"
