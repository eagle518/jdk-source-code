/*
 * @(#)OldStubs.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javah.oldjavah;

import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.Vector;
import java.util.Enumeration;
import sun.tools.java.MemberDefinition;
import sun.tools.java.ClassDefinition;
import sun.tools.java.ClassDeclaration;
import sun.tools.java.Type;
import sun.tools.java.ArrayType;
import sun.tools.java.ClassType;
import sun.tools.java.MethodType;

/**
 * Generate stubs for JDK 1.0 style native method interface.
 *
 * Note that we go great lengths to generate code that looks as much as
 * possible like what the C version of javah generated.  Though this goal
 * has made this code awkward at certain places, it has the advantage that
 * we can stress test this implementation against the reference C
 * implementation manually.
 *
 * @version 1.4, 12/03/01
 */
public class OldStubs extends Gen {

    protected String getIncludes() {
	return "#include <StubPreamble.h>\n";
    }

    protected void write(OutputStream o, String clazz)
	throws ClassNotFoundException {
	PrintWriter pw = wrapWriter(o);

	MemberDefinition[] methods = env.getNativeMethods(clazz);

	String mangledClazz = Mangle.mangle(clazz, Mangle.Type.CLASS);
	
	pw.println("/* Stubs for class " + mangledClazz + " */");

	for (int i = 0; i < methods.length; i++) {
	    StringBuffer     sb         = new StringBuffer(200);
	    MemberDefinition method     = methods[i];
	    String methodName = method.getName().toString();
	    String mangledMethod = Mangle.mangle(methodName, 
					       Mangle.Type.FIELDSTUB);
	    String methodSig = method.getType().getTypeSignature();
	    String implName = mangledClazz + '_' + mangledMethod;

	    String stubName = Mangle.mangleMethod(method, clazz,
						Mangle.Type.METHOD_JDK_1);
	    
	    /*
	     * Comment string.
	     */
	    sb.append("/* SYMBOL: \"");
	    sb.append(mangledClazz);
	    sb.append('/');
	    sb.append(mangledMethod);
	    sb.append(methodSig);
	    sb.append("\", ");
	    sb.append(stubName);
	    sb.append(" */\n");

	    /*
	     * Stub method call.
	     */
	    String exp = Util.getPlatformString("dll.export");
	    if (exp != null)
		sb.append(exp);
	    sb.append("stack_item *");
	    sb.append(stubName);
	    sb.append("(stack_item *_P_,struct execenv *_EE_) {\n");
	    
	    /* 
	     * Prototype and call to real method. 
	     */
	    StringBuffer proto   = new StringBuffer(100);
	    StringBuffer call    = new StringBuffer(100);
	    StringBuffer fwds    = new StringBuffer(100);
	    
	    Type    retType      = method.getType().getReturnType();
	    int     rtc          = retType.getTypeCode();
	    boolean returnIs64   = (rtc == TC_LONG || rtc == TC_DOUBLE);
	    boolean returnIsVoid = (rtc == TC_VOID);

	    /* return value */
	    proto.append("\textern ");
	    proto.append(getStubCType(retType, true));
	    proto.append(' ');

	    call.append('\t');
	    if (rtc == TC_VOID) {
		call.append("(void) ");
	    } else if (rtc == TC_DOUBLE) {
		call.append("SET_DOUBLE(_tval, _P_, ");
	    } else if (rtc == TC_LONG) {
		call.append("SET_INT64(_tval, _P_, ");
	    } else {
		call.append("_P_[0].");
		call.append(getStackItemName32(rtc));
		call.append(" = ");
	    }
	    if (rtc == TC_BOOLEAN)
		call.append('(');
	    if (returnIs64) 
		fwds.append("\tjvalue _tval;\n");

	    /* method name */
	    proto.append(implName);
	    proto.append('(');

	    call.append(implName);
	    call.append('(');
	    
	    proto.append("void *");
	    if (method.isStatic()) {
		call.append("NULL");
	    } else {
		call.append("_P_[0].p");
	    }
	    /* arguments */
	    Type[] argTypes = method.getType().getArgumentTypes();
	    int    nargs    = argTypes.length;
	    
	    for (int j = 0, nthWord = 0; j < nargs; j++) {
		if (j == 0) {
		    proto.append(',');
		    call.append(',');
		    if (!method.isStatic()) {
			nthWord++;
		    }
		}

		proto.append(getStubCType(argTypes[j], false));
		
		int atc = argTypes[j].getTypeCode();
		if (atc == TC_LONG) {
		    call.append("GET_INT64(_t");
		    call.append(nthWord);
		    call.append(", _P_+");
		    call.append(nthWord);
		    call.append(')');
		} else if (atc == TC_DOUBLE) {
		    call.append("GET_DOUBLE(_t");
		    call.append(nthWord);
		    call.append(", _P_+");
		    call.append(nthWord);
		    call.append(')');
		} else {
		    call.append("((_P_[");
		    call.append(nthWord);
		    call.append("].");
		    call.append(getStackItemName32(atc));
		    call.append("))");
		}
		if (atc == TC_LONG || atc == TC_DOUBLE) {
		    fwds.append("\tjvalue _t");
		    fwds.append(nthWord);
		    fwds.append(";\n");
		    nthWord += 2;
		} else {
		    nthWord += 1;
		}
		
		if (j < (nargs - 1)) {
		    proto.append(',');
		    call.append(',');
		}
	    }

	    if (rtc == TC_BOOLEAN)
		call.append(") ? TRUE : FALSE");
	    
	    /* finished arguments */
	    proto.append(");\n");
	    if (returnIs64)
		call.append(')');
	    call.append(");\n");
	    
	    /* now really print prototype and call */
	    sb.append(fwds);
	    sb.append(proto);
	    sb.append(call);

	    /*
	     * Return statement.
	     */
	    sb.append("\treturn _P_");
	    if (returnIs64)
		sb.append(" + 2");
	    else if (!returnIsVoid)
		sb.append(" + 1");
	    
	    sb.append(";\n}");
	    
	    pw.println(sb);
	}
    }

    private String getStackItemName32(int typecode) {
	if (typecode == TC_INT ||
	    typecode == TC_BOOLEAN || 
	    typecode == TC_CHAR ||
	    typecode == TC_SHORT ||
	    typecode == TC_BYTE)
	    return "i";
	
	if (typecode == TC_FLOAT)
	    return "f";
	
	return "p";
    }
    

    private String getStubCType(Type p, boolean isReturn) {
	int tc = p.getTypeCode();
	switch (tc) {
	case TC_VOID:
	    return "void";
	case TC_BOOLEAN:
	case TC_BYTE:
	case TC_SHORT:
	case TC_CHAR:
	case TC_INT:
	    return "int32_t";
	case TC_FLOAT:
	    return "float";
	case TC_DOUBLE:
	    return "double";
	case TC_LONG:
	    return "int64_t";
	case TC_ARRAY:
	case TC_CLASS:
	    if (isReturn) 
		return "void*";
	    else 
		return "void *";
	default:
	    Util.bug("unknown.type.in.method.signature");
	}
	return null; /* dead code */
    }

    protected String getFileSuffix() {
	return ".c";
    }

}
