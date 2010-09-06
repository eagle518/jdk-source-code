/*
 * @(#)CJavaConsole.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CJavaConsole.h  by Stanley Man-Kit Ho
//
///=--------------------------------------------------------------------------=

#ifndef CJavaConsole_h__
#define CJavaConsole_h__

//=--------------------------------------------------------------------------=
//
// CJavaConsole object encapsulates the Java console
//
class IJVMConsole;
class JavaPluginFactory5;

class CJavaConsole : public IJVMConsole
{
public:

    ////////////////////////////////////////////////////////////////////////////
    // from ISupports and AggregatedQueryInterface:

    JD_DECL_AGGREGATED

    static JD_METHOD Create(ISupports* outer, JavaPluginFactory5* pJavaPluginFactory, 
			    const JDIID& aIID, void* *aInstancePtr);

    //=--------------------------------------------------------------=
    // IJVMConsole
    //=--------------------------------------------------------------=
    JD_IMETHOD Show(void);

    JD_IMETHOD Hide(void);

    JD_IMETHOD IsVisible(JDBool *result);

    // Prints a message to the Java console. The encodingName specifies the
    // encoding of the message, and if NULL, specifies the default platform
    // encoding.
    JD_IMETHOD Print(const char* msg, const char* encodingName = NULL);

    CJavaConsole(ISupports *aOuter, JavaPluginFactory5* pJavaPluginFactory);
    virtual ~CJavaConsole();

protected:
    JavaPluginFactory5*           m_pJavaPluginFactory5;
};
#endif // CJavaConsole_h___
