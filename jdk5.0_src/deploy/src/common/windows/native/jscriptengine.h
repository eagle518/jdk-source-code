/*
 * @(#)jscriptengine.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// jscriptengine.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#ifndef __JSCRIPTENGINE_H__
#define __JSCRIPTENGINE_H__

//=--------------------------------------------------------------------------=
// Interface Smart Pointer
//=--------------------------------------------------------------------------=
_COM_SMARTPTR_TYPEDEF(IActiveScript, __uuidof(IActiveScript));
_COM_SMARTPTR_TYPEDEF(IActiveScriptParse, __uuidof(IActiveScriptParse));
_COM_SMARTPTR_TYPEDEF(IActiveScriptSite, __uuidof(IActiveScriptSite));


///=--------------------------------------------------------------------------=
// JScriptEngine class
//=---------------------------------------------------------------------------=
class JScriptEngine
{
public:
    JScriptEngine()
    {
	::CoInitialize(NULL);

	Initialize();
    }

    ~JScriptEngine()
    {
	::CoUninitialize();
    }

    HRESULT Eval(LPCWSTR lpwszScript, LPWSTR lpwszResult);

private:
    // Instantiate the JScript engine. 
    //
    HRESULT Initialize(void);

    IActiveScriptPtr m_spScript;
};

#endif // __JSCRIPTENGINE_H__
