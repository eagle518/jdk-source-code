/*
 * @(#)DOMObjectHelper.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom;

import org.w3c.dom.*;
import sun.plugin.dom.exception.*;
import netscape.javascript.JSException;
import netscape.javascript.JSObject;


/** 
 * DOMObjectHelper is a class to help make DOM object access easier.
 */
public class DOMObjectHelper
{
    /** 
     * Retrieve a named member of a DOM object as bool.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static boolean getBooleanMember(DOMObject obj, String name) throws DOMException
    {
	Object result = obj.getMember(name);

	if (result != null)
	    return new Boolean(result.toString()).booleanValue();
	else
	    return false;
    }

    /** 
     * Retrieve a named member of a DOM object as bool, without exception
     *
     * @param name Member name
     * @return Value of the member
     */
    public static boolean getBooleanMemberNoEx(DOMObject obj, String name)
    {
	try
	{
	    return getBooleanMember(obj, name);
	}
	catch (DOMException e)
	{
	    return false;
	}
    }

    /** 
     * Set a named member of a DOM object with bool value.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static void setBooleanMember(DOMObject obj, String name, boolean value) throws DOMException
    {
	obj.setMember(name, value + "");
    }

    /** 
     * Set a named member of a DOM object with bool value, without exception.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static void setBooleanMemberNoEx(DOMObject obj, String name, boolean value)
    {
	try
	{
	    setBooleanMember(obj, name, value);
	}
	catch (DOMException e)
	{
	}	
    }

    /** 
     * Retrieve a named member of a DOM object as int.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static int getIntMember(DOMObject obj, String name) throws DOMException
    {
	Object result = obj.getMember(name);

	if (result != null)
	    return new Float(result.toString()).intValue();
	else
	    return 0;
    }

    /** 
     * Retrieve a named member of a DOM object as int.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static int getIntMemberNoEx(DOMObject obj, String name)
    {
	try
	{
	    return getIntMember(obj, name);
	}
	catch (DOMException e)
	{
	}

	return 0;
    }

    /** 
     * Set a named member of a DOM object with int value.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static void setIntMember(DOMObject obj, String name, int value) throws DOMException
    {
	obj.setMember(name, value + "");
    }

    /** 
     * Set a named member of a DOM object with int value, without exception
     *
     * @param name Member name
     * @return Value of the member
     */
    public static void setIntMemberNoEx(DOMObject obj, String name, int value)
    {
	try
	{
	    setIntMember(obj, name, value);
	}
	catch (DOMException e)
	{
	}
    }
    
    /** 
     * Retrieve a named member of a DOM object as string.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static String getStringMember(DOMObject obj, String name) throws DOMException
    {
	Object result = obj.getMember(name);

	if (result != null)
	    return result.toString();
	else
	    return null;
    }
    
    /** 
     * Retrieve a named member of a DOM object as string.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static String getStringMemberNoEx(DOMObject obj, String name)
    {
	try
	{
	    return getStringMember(obj, name);
	}
	catch (DOMException e)
	{
	}

	return null;
    }

    /** 
     * Set a named member of a DOM object with string value.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static void setStringMember(DOMObject obj, String name, String value) throws DOMException
    {
	obj.setMember(name, value);
    }


    /** 
     * Set a named member of a DOM object with string value.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static void setStringMemberNoEx(DOMObject obj, String name, String value)
    {
	try
	{
	    setStringMember(obj, name, value);
	}
	catch (DOMException e)
	{
	}
    }


    /** 
     * Call a method of a DOM object and return the result as string.
     *
     * @param name Member name
     * @return Value of the member
     */
    public static String callStringMethod(DOMObject obj, String name, Object[] args)
    {
	Object result = obj.call(name, args);

	if (result != null)
	    return result.toString();
	else
	    return null;
    }
}
