/*
 * @(#)IPluginStreamListener.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __IBROWSERAUTHENTICATOR_H
#define __IBROWSERAUTHENTICATOR_H

#include "ISupports.h"

/* 82274a32-a196-42ee-8e3b-fcb73e339518 */   
#define IBROWSERAUTHENTICATOR_IID	\
	{0x82274a32, 0xa196, 0x42ee,	\
	{0x8e, 0x3b, 0xfc, 0xb7, 0x3e, 0x33, 0x95, 0x18}}

class IBrowserAuthenticator: public ISupports {

public:
	JD_DEFINE_STATIC_IID_ACCESSOR(IBROWSERAUTHENTICATOR_IID)

	JD_IMETHOD GetAuthInfo(const char* protocol, const char* host, int port, const char* scheme,
		const char* realm, char* lpszUsername, int nUserNameSize, char* lpszPassword, int nPasswordSize)=0;
};

#endif __IBROWSERAUTHENTICATOR_H
