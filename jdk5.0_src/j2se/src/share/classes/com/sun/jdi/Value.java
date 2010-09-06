/*
 * @(#)Value.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * The mirror for a value in the target VM.
 * This interface is the root of a 
 * value hierarchy encompassing primitive values and object values. 
 * <P>
 * Some examples of where values may be accessed:
 * <BLOCKQUOTE><TABLE SUMMARY="layout">
 * <TR>
 *   <TD>{@link ObjectReference#getValue(com.sun.jdi.Field)
 *                 ObjectReference.getValue(Field)}
 *   <TD>- value of a field
 * <TR>
 *   <TD>{@link StackFrame#getValue(com.sun.jdi.LocalVariable)
 *                 StackFrame.getValue(LocalVariable)}
 *   <TD>- value of a variable
 * <TR>
 *   <TD>{@link VirtualMachine#mirrorOf(double)
 *                 VirtualMachine.mirrorOf(double)}
 *   <TD>- created in the target VM by the JDI client
 * <TR>
 *   <TD>{@link com.sun.jdi.event.ModificationWatchpointEvent#valueToBe()
 *                 ModificationWatchpointEvent.valueToBe()}
 *   <TD>- returned with an event
 * </TABLE></BLOCKQUOTE>
 * <P>
 * The following table illustrates which subinterfaces of Value 
 * are used to mirror values in the target VM --
 * <TABLE BORDER=1 SUMMARY="Maps each kind of value to a mirrored
 *  instance of a subinterface of Value">
 * <TR BGCOLOR="#EEEEFF">
 *   <TH id="primval" colspan=4>Subinterfaces of {@link PrimitiveValue}</TH>
 * <TR BGCOLOR="#EEEEFF">
 *   <TH id="kind"     align="left">Kind of value</TD>
 *   <TH id="example"  align="left">For example -<br>expression in target</TD>
 *   <TH id="mirrored" align="left">Is mirrored as an<br>instance of</TD>
 *   <TH id="type"     align="left">{@link Type} of value<br>{@link #type() Value.type()}</TD>
 * <TR>
 *   <TD headers="primval kind">     a boolean</CODE></TD>
 *   <TD headers="primval example">  <CODE>true</CODE></TD>
 *   <TD headers="primval mirrored"> {@link BooleanValue}</TD>
 *   <TD headers="primval type">     {@link BooleanType}</TD>
 * <TR>
 *   <TD headers="primval kind">     a byte</TD>
 *   <TD headers="primval example">  <CODE>(byte)4</CODE></TD>
 *   <TD headers="primval mirrored"> {@link ByteValue}</TD>
 *   <TD headers="primval type">     {@link ByteType}</TD>
 * <TR>
 *   <TD headers="primval kind">     a char</TD>
 *   <TD headers="primval example">  <CODE>'a'</CODE></TD>
 *   <TD headers="primval mirrored"> {@link CharValue}</TD>
 *   <TD headers="primval type">     {@link CharType}</TD>
 * <TR>
 *   <TD headers="primval kind">     a double</TD>
 *   <TD headers="primval example">  <CODE>3.1415926</CODE></TD>
 *   <TD headers="primval mirrored"> {@link DoubleValue}</TD>
 *   <TD headers="primval type">     {@link DoubleType}</TD>
 * <TR>
 *   <TD headers="primval kind">     a float</TD>
 *   <TD headers="primval example">  <CODE>2.5f</CODE></TD>
 *   <TD headers="primval mirrored"> {@link FloatValue}</TD>
 *   <TD headers="primval type">     {@link FloatType}</TD>
 * <TR>
 *   <TD headers="primval kind">     an int</TD>
 *   <TD headers="primval example">  <CODE>22</CODE></TD>
 *   <TD headers="primval mirrored"> {@link IntegerValue}</TD>
 *   <TD headers="primval type">     {@link IntegerType}</TD>
 * <TR>
 *   <TD headers="primval kind">     a long</TD>
 *   <TD headers="primval example">  <CODE>1024L</CODE></TD>
 *   <TD headers="primval mirrored"> {@link LongValue}</TD>
 *   <TD headers="primval type">     {@link LongType}</TD>
 * <TR>
 *   <TD headers="primval kind">     a short</TD>
 *   <TD headers="primval example">  <CODE>(short)12</CODE></TD>
 *   <TD headers="primval mirrored"> {@link ShortValue}</TD>
 *   <TD headers="primval type">     {@link ShortType}</TD>
 * <TR>
 *   <TD headers="primval kind">     a void</TD>
 *   <TD headers="primval example">  <CODE>&nbsp;</CODE></TD>
 *   <TD headers="primval mirrored"> {@link VoidValue}</TD>
 *   <TD headers="primval type">     {@link VoidType}</TD>
 * <TR BGCOLOR="#EEEEFF">
 *   <TH id="objref" colspan=4>Subinterfaces of {@link ObjectReference}</TH>
 * <TR BGCOLOR="#EEEEFF">
 *   <TH id="kind2"     align="left">Kind of value</TD>
 *   <TH id="example2"  align="left">For example -<br>expression in target</TD>
 *   <TH id="mirrored2" align="left">Is mirrored as an<br>instance of</TD>
 *   <TH id="type2"     align="left">{@link Type} of value<br>{@link #type() Value.type()}</TD>
 * <TR>
 *   <TD headers="objref kind2">     a class instance</TD>
 *   <TD headers="objref example2">  <CODE>this</CODE></TD>
 *   <TD headers="objref mirrored2"> {@link ObjectReference}</TD>
 *   <TD headers="objref type2">     {@link ClassType}</TD>
 * <TR>
 *   <TD headers="objref kind2">     an array</TD>
 *   <TD headers="objref example2">  <CODE>new int[5]</CODE></TD>
 *   <TD headers="objref mirrored2"> {@link ArrayReference}</TD>
 *   <TD headers="objref type2">     {@link ArrayType}</TD>
 * <TR>
 *   <TD headers="objref kind2">     a string</TD>
 *   <TD headers="objref example2">  <CODE>"hello"</CODE></TD>
 *   <TD headers="objref mirrored2"> {@link StringReference}</TD>
 *   <TD headers="objref type2">     {@link ClassType}</TD>
 * <TR>
 *   <TD headers="objref kind2">     a thread</TD>
 *   <TD headers="objref example2">  <CODE>Thread.currentThread()</CODE></TD>
 *   <TD headers="objref mirrored2"> {@link ThreadReference}</TD>
 *   <TD headers="objref type2">     {@link ClassType}</TD>
 * <TR>
 *   <TD headers="objref kind2">     a thread group</TD>
 *   <TD headers="objref example2">  <CODE>Thread.currentThread()<br>&nbsp;&nbsp;.getThreadGroup()</CODE></TD>
 *   <TD headers="objref mirrored2"> {@link ThreadGroupReference}</TD>
 *   <TD headers="objref type2">     {@link ClassType}</TD>
 * <TR>
 *   <TD headers="objref kind2">     a <CODE>java.lang.Class</CODE><br>instance</TD>
 *   <TD headers="objref example2">  <CODE>this.getClass()</CODE></TD>
 *   <TD headers="objref mirrored2"> {@link ClassObjectReference}</TD>
 *   <TD headers="objref type2">     {@link ClassType}</TD>
 * <TR>
 *   <TD headers="objref kind2">     a class loader</TD>
 *   <TD headers="objref example2">  <CODE>this.getClass()<br>&nbsp;&nbsp;.getClassLoader() </CODE></TD>
 *   <TD headers="objref mirrored2"> {@link ClassLoaderReference}</TD>
 *   <TD headers="objref type2">     {@link ClassType}</TD>
 * <TR BGCOLOR="#EEEEFF">
 *   <TH id="other" colspan=4>Other</TH>
 * <TR BGCOLOR="#EEEEFF">
 *   <TH id="kind3"     align="left">Kind of value</TD>
 *   <TH id="example3"  align="left">For example -<br>expression in target</TD>
 *   <TH id="mirrored3" align="left">Is mirrored as</TD>
 *   <TH id="type3"     align="left">{@link Type} of value</TD>
 * <TR>
 *   <TD headers="other kind3">     null</TD>
 *   <TD headers="other example3">  <CODE>null</CODE></TD>
 *   <TD headers="other mirrored3"> <CODE>null</CODE></TD>
 *   <TD headers="other type3">     n/a</TD>
 * </TABLE>
 *
 * @author Robert Field
 * @author Gordon Hirsch
 * @author James McIlree
 * @since  1.3
 */

public interface Value extends Mirror
{
    /**
     * Returns the run-time type of this value. 
     *
     * @see Type
     * @return a {@link Type} which mirrors the value's type in the 
     * target VM.
     */
    Type type();
}

