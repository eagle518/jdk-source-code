/*
 * @(#)IUniqueIdentifier.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//  IUniqueIndentifier.h  by X.Lu
//
///=--------------------------------------------------------------------------=
// Used by applet legacy life cycle.
//

#ifndef __IUNIQUEIDENTIFIER_H
#define __IUNIQUEIDENTIFIER_H

#define UNIQUE_IDENTIFIER_IID \
{ /* A8F70EB5-AAEF-11d6-95A4-0050BAAC8BD3 */ \
	0xa8f70eb5, \
	0xaaef, \
	0x11d6, \
	{0x95, 0xa4, 0x0, 0x50, 0xba, 0xac, 0x8b, 0xd3} \
}
class IUniqueIdentifier: public ISupports {
public:
   JD_DEFINE_STATIC_IID_ACCESSOR(UNIQUE_IDENTIFIER_IID);

   JD_IMETHOD SetUniqueId(long id) = 0;
   JD_IMETHOD GetUniqueId(long* pId) = 0;
};


#define UNIQUE_IDENTIFIER_ID	"A8F70EB5-AAEF-11d6-95A4-0050BAAC8BD3"

#endif //__IUNIQUEIDENTIFIER_H
