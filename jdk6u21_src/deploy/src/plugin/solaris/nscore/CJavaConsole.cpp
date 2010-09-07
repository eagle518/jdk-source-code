/*
 *  @(#)CJavaConsole.cpp	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CJavaConsole.cpp  by Stanley Man-Kit Ho
//
///=--------------------------------------------------------------------------=
//
// CJavaConsole encapsulates most functionalities in Java Console.
//
#include "commonhdr.h"
#include "IJVMConsole.h"
#include "JavaPluginFactory5.h"
#include "CJavaConsole.h"
#include "Debug.h"

static JD_DEFINE_IID(jIJVMConsole, IJVMCONSOLE_IID);
static JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);

////////////////////////////////////////////////////////////////////////////
// from ISupports and AggregatedQueryInterface:

// Thes macro expands to the aggregated query interface scheme.

JD_IMPL_AGGREGATED(CJavaConsole);

JD_METHOD
CJavaConsole::AggregatedQueryInterface(const JDIID& aIID, void** aInstancePtr)
{
    trace("CJavaConsole::AggregatedQueryInterface\n");

    if (aIID.Equals(jISupportsIID)) {
      *aInstancePtr = GetInner();
      AddRef();
      return JD_OK;
    }
    if (aIID.Equals(jIJVMConsole)) {
        *aInstancePtr = (IJVMConsole *)this;
        AddRef();
        return JD_OK;
    }
    return JD_NOINTERFACE;
}



///=--------------------------------------------------------------------------=
// CJavaConsole::CJavaConsole
///=--------------------------------------------------------------------------=
// Implements the CJavaConsole object for showing, hiding, printing to the Java
// Console.
//
// parameters :
//
// return :
// 
// notes :
//
CJavaConsole::CJavaConsole(ISupports *aOuter, JavaPluginFactory5* pJavaPluginFactory5)
{
    trace("CJavaConsole::CJavaConsole\n");

    JD_INIT_AGGREGATED(aOuter);

    m_pJavaPluginFactory5 = pJavaPluginFactory5;
    
    if (m_pJavaPluginFactory5)
        m_pJavaPluginFactory5->AddRef();
    
}



///=--------------------------------------------------------------------------=
// CJavaConsole::~CJavaConsole
///=--------------------------------------------------------------------------=
// Implements the CJavaConsole object for showing, hiding, printing to the Java
// Console.
//
// parameters :
//
// return :
// 
// notes :
//
CJavaConsole::~CJavaConsole()  
{
    trace("CJavaConsole::~CJavaConsole\n");
    
    if (m_pJavaPluginFactory5)
        m_pJavaPluginFactory5->Release();
    
}

///=--------------------------------------------------------------------------=
// CJavaConsole::Create
///=--------------------------------------------------------------------------=
// Create the CJavaConsole object for showing, hiding, printing to the Java
// Console.
//
// parameters :
//
// return :
// 
// notes :
//
JD_METHOD
CJavaConsole::Create(ISupports* outer, JavaPluginFactory5* pJavaPluginFactory5, 
		     const JDIID& aIID, void* *aInstancePtr)
{
    trace("CJavaConsole::Create\n");

    if (outer && !aIID.Equals(jISupportsIID))
        return JD_NOINTERFACE;   // XXX right error?
    CJavaConsole* console = new CJavaConsole(outer, pJavaPluginFactory5);
    if (console == NULL)
        return JD_ERROR_OUT_OF_MEMORY;
    //console->AddRef();
    *aInstancePtr = console->GetInner();
    *aInstancePtr = (outer != NULL)? (void *)console->GetInner(): (void *)console;
    return JD_OK;
}


////////////////////////////////////////////////////////////////////////////
// from IJVMConsole:
//

///=--------------------------------------------------------------------------=
// CJavaConsole::Show
//=---------------------------------------------------------------------------=
//
JD_IMETHODIMP CJavaConsole::Show(void) 
{
    trace("CJavaConsole::Show\n");

    return m_pJavaPluginFactory5->ShowJavaConsole();
}


///=--------------------------------------------------------------------------=
// CJavaConsole::Hide
//=---------------------------------------------------------------------------=
//
JD_IMETHODIMP CJavaConsole::Hide(void)
{
    trace("CJavaConsole::Hide\n");

    return JD_OK;
}


///=--------------------------------------------------------------------------=
// CJavaConsole::IsConsoleVisible
//=---------------------------------------------------------------------------=
//
JD_IMETHODIMP CJavaConsole::IsVisible(JDBool *result) 
{
    trace("CJavaConsole::IsConsoleVisible\n");

    if (result == NULL)
	return JD_ERROR_NULL_POINTER;

    return JD_OK;
}


///=--------------------------------------------------------------------------=
// CJavaConsole::Print
//=---------------------------------------------------------------------------=
//
// Prints a message to the Java console. The encodingName specifies the
// encoding of the message, and if NULL, specifies the default platform
// encoding.
//
JD_IMETHODIMP CJavaConsole::Print(const char* msg, const char* encodingName) 
{
    trace("CJavaConsole::Print\n");

    if (msg == NULL || encodingName == NULL)
	return JD_ERROR_NULL_POINTER;

    return JD_OK;
}
