/*
 * @(#)ParameterListCorrelator.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import sun.plugin.util.Trace;
import sun.plugin.resources.ResourceHandler;

public class ParameterListCorrelator
{
    private static final int BOOL_CODE          = 0;
    private static final int BYTE_CODE          = 1;
    private static final int SHORT_CODE         = 2;
    private static final int INT_CODE           = 3;
    private static final int LONG_CODE          = 4;
    private static final int FLOAT_CODE         = 5;
    private static final int DOUBLE_CODE        = 6;
    private static final int CHAR_CODE          = 7;
    private static final int NOT_PRIMITIVE_CODE = 8;

    private Class[]     expectedClasses;
    private Object[]    givenParameters;
    private boolean     analysisIsDone;
    private boolean     parametersCorrelateToClasses;
    private int         numberOfConversionsNeeded;

    ParameterListCorrelator(Class []expectedClasses, Object []givenParameters)
    {
        this.expectedClasses = expectedClasses;
        if (this.expectedClasses == null)
            this.expectedClasses = new Class[0];

        this.givenParameters = givenParameters;
        if (this.givenParameters == null)
            this.givenParameters = new Object[0];

        analysisIsDone = false;
        numberOfConversionsNeeded = 0;
        parametersCorrelateToClasses = false;
    }


    boolean parametersCorrelateToClasses()
        throws Exception
    {
        analyze();
        return parametersCorrelateToClasses;
    }

    int numberOfConversionsNeeded()
        throws Exception
    {
        analyze();

        if (parametersCorrelateToClasses == false)
	    throw new Exception(ResourceHandler.getMessage("com.method.argsTypeInvalid"));

        return numberOfConversionsNeeded;
    }

    private void analyze()
        throws Exception
    {
        if (analysisIsDone == false)
        {
            parametersCorrelateToClasses = true;

            if (expectedClasses.length != givenParameters.length)
                reportParametersDoNotCorrelate();

            int i = 0;
            while (analysisIsDone == false && i < givenParameters.length)
            {
                analyzeParameter(expectedClasses[i], givenParameters[i]);
                ++i;
            }

            analysisIsDone = true;
        }
    }

    private void analyzeParameter(Class expectedClass, Object param)
        throws Exception
    {
        if (param != null)
	    analyzeParameter(expectedClass, param.getClass());
    }

    private void analyzeParameter(Class expectedClass, Class classOfParameter)
        throws Exception
    {
        if (classOfParameter == null)
            return;

        // If the type matches, no need to convert
        if (expectedClass.equals(classOfParameter))
        {
            return; // Perfect match
        }
        else
        if (expectedClass.isAssignableFrom(classOfParameter))
        {
            ++numberOfConversionsNeeded; // Coversion is possible.
        }
        //No implicit conversion to string
        else
        if (expectedClass == java.lang.String.class)
        {
            ++numberOfConversionsNeeded; // Any object can be stringified.
        }
	else
	if( (expectedClass.isPrimitive() || Number.class.isAssignableFrom(expectedClass) ||
	    expectedClass == Character.class || expectedClass == Boolean.class) &&  
	    classOfParameter == String.class)
	{
	    //primitive type arguments could be passed as strings
            ++numberOfConversionsNeeded; 
	}
        else
        if (expectedClass.isArray())
        {
            if (classOfParameter.isArray() == false)
            {
                reportParametersDoNotCorrelate();
            }
            else
            {
                Class compType = expectedClass.getComponentType();
                Class paramCompType = classOfParameter.getComponentType();
		analyzeParameter(compType, paramCompType);
            }
        } else 
	if (expectedClass.equals(netscape.javascript.JSObject.class)) {
	    // we can wrap any object into JSObject
	    ++numberOfConversionsNeeded;
	}
        else 
	if (!typesAreEquivalentPrimitives(expectedClass, classOfParameter))
        {
            // We cannot handle conversion for non-primitive type
            // -- good luck.
            reportParametersDoNotCorrelate();
        }
    }

    private boolean typesAreEquivalentPrimitives(Class leftType, Class rightType)
    {
        int leftCode = typeCodeFromTypeName(leftType.getName());

        if (leftCode != NOT_PRIMITIVE_CODE)
        {
            int rightCode = typeCodeFromTypeName(rightType.getName());

            if (rightCode == NOT_PRIMITIVE_CODE)
                return false;

            if (leftCode != rightCode) // Conversion needed, otherwise a "perfect" match
                ++numberOfConversionsNeeded;

            return true;
        }

        return false;
    }

    private static int typeCodeFromTypeName(String typeName)
    {
        // Notice that the primitive type and object wrapper are treated as
        // the same type in overloading -- because reflection always requires
        // us to convert all the primitive type into object wrapper, we
        // won't be able to figure out what the original type is in VB.
        if (typeName.equals("boolean") || typeName.equals("java.lang.Boolean"))
            return BOOL_CODE;
        else if (typeName.equals("byte") || typeName.equals("java.lang.Byte"))
            return BYTE_CODE;
        else if (typeName.equals("short") || typeName.equals("java.lang.Short"))
            return SHORT_CODE;
        else if (typeName.equals("int") || typeName.equals("java.lang.Integer"))
            return INT_CODE;
        else if (typeName.equals("long") || typeName.equals("java.lang.Long"))
            return LONG_CODE;
        else if (typeName.equals("float") || typeName.equals("java.lang.Float"))
            return FLOAT_CODE;
        else if (typeName.equals("double") || typeName.equals("java.lang.Double"))
            return DOUBLE_CODE;
        else if (typeName.equals("char") || typeName.equals("java.lang.Character"))
            return CHAR_CODE;
        else
            return NOT_PRIMITIVE_CODE;
    }

    private void reportParametersDoNotCorrelate()
    {
        parametersCorrelateToClasses = false;
        analysisIsDone = true;
    }

}

