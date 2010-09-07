/*
 * @(#)MozExports.h	1.6 10/03/24 12:03:40
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __MOZEXPORTS_H_
#define __MOZEXPORTS_H_

#ifdef __cplusplus
extern "C" {
#endif

NPError MozNPN_GetURLNotify(NPP instance, const char *url, const char *target, 
                         void* notifyData);
NPError MozNPN_GetURL(NPP instance, const char *url, const char *target);
NPError MozNPN_PostURLNotify(NPP instance, const char* url, const char* window, 
                          uint32_t len, const char* buf, NPBool file, 
                          void* notifyData);
NPError MozNPN_PostURL(NPP instance, const char* url, const char* window, 
                    uint32_t len, const char* buf, NPBool file);
NPError MozNPN_RequestRead(NPStream* stream, NPByteRange* rangeList);
NPError MozNPN_NewStream(NPP instance, NPMIMEType type, const char* target, 
                      NPStream** stream);
int32_t MozNPN_Write(NPP instance, NPStream *stream, int32_t len, void *buffer);
NPError MozNPN_DestroyStream(NPP instance, NPStream* stream, NPError reason);
void MozNPN_Status(NPP instance, const char *message);
const char* MozNPN_UserAgent(NPP instance);
void* MozNPN_MemAlloc(uint32_t size);
uint32_t MozNPN_MemFlush(uint32_t size);
void MozNPN_MemFree(void* ptr);
NPError MozNPN_GetValue(NPP instance, NPNVariable variable, void *value);
NPError MozNPN_SetValue(NPP instance, NPPVariable variable, void *value);
void MozNPN_InvalidateRect(NPP instance, NPRect *invalidRect);
void MozNPN_ForceRedraw(NPP instance);
NPIdentifier MozNPN_GetStringIdentifier(const NPUTF8 *name);
void MozNPN_GetStringIdentifiers(const NPUTF8 **names, int32_t nameCount, 
                              NPIdentifier *identifiers);
NPIdentifier MozNPN_GetIntIdentifier(int32_t intid);
bool MozNPN_IdentifierIsString(NPIdentifier identifier);
NPUTF8* MozNPN_UTF8FromIdentifier(NPIdentifier identifier);
int32_t MozNPN_IntFromIdentifier(NPIdentifier identifier);
NPObject *MozNPN_CreateObject(NPP npp, NPClass *aClass);
NPObject *MozNPN_RetainObject(NPObject *npobj);
void MozNPN_ReleaseObject(NPObject *npobj);
bool MozNPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName,
                const NPVariant *args, uint32_t argCount, NPVariant *result);
bool MozNPN_InvokeDefault(NPP npp, NPObject *npobj, const NPVariant *args,
                       uint32_t argCount, NPVariant *result);
bool MozNPN_Evaluate(NPP npp, NPObject *npobj, NPString *script,
                  NPVariant *result);
bool MozNPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                     NPVariant *result);
bool MozNPN_SetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                     const NPVariant *value);
bool MozNPN_RemoveProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName);
bool MozNPN_HasProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName);
bool MozNPN_HasMethod(NPP npp, NPObject *npobj, NPIdentifier methodName);
void MozNPN_ReleaseVariantValue(NPVariant *variant);
void MozNPN_SetException(NPObject *npobj, const NPUTF8 *message);
void MozNPN_PluginThreadAsyncCall(NPP instance, void (*func)(void *), void *userData);
void MozNPN_PushPopupsEnabledState(NPP instance, bool enable);
void MozNPN_PopPopupsEnabledState(NPP instance);
NPError MozNPN_GetValueForURL(NPP npp, NPNURLVariable variable, const char *url, char **value, uint32_t *len);
NPError MozNPN_SetValueForURL(NPP npp, NPNURLVariable variable, const char *url, const char *value, uint32_t len);
NPError MozNPN_GetAuthenticationInfo(NPP npp, const char *protocol, const char *host, int32_t port, const char *scheme, const char *realm, char **username, uint32_t *ulen, char **password, uint32_t *plen);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif // __MOZEXPORTS_H
