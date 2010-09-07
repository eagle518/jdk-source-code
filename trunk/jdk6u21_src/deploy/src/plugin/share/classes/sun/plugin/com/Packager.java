/*
 * @(#)Packager.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.beans.*;
import java.io.*;
import java.util.*;
import java.lang.reflect.*;
import java.awt.Component;

public class Packager
{
    private PrintStream ps = null;
    private File idlFile = null;
    private boolean isGUI = false;
    public static final int NONGUI_EXITCODE = 101;

    private String javaProps[] = { 
	"background",
	"font",
	"foreground",
    };

    private String notAllowedMethods[] = {
	"enable", "disable", "class", "minimumSize", "preferredSize", "warning"
    };

    private String notAllowedProps[] = {
	"layout", "action"
    };

    private String comProps[] = { 
	"[id(0xfffffe0b)]\nOLE_COLOR BackColor;",
	"[id(0xfffffe00)]\nFont* Font;",
	"[id(0xfffffdff)]\nOLE_COLOR ForeColor;",
    };

    private HashMap methodMap = new HashMap();
    private HashMap eventMap = new HashMap();
    private HashMap overLoadedMap = new HashMap();

    Packager(String name, String tempPath){
	try {
	    String idlName = name.substring(name.lastIndexOf('.')+1); 
	    idlFile = new File(tempPath + File.separator + idlName + ".idl");
	    ps = new PrintStream(new FileOutputStream(idlFile));
	}catch(Throwable exc) {
	    exc.printStackTrace();
	    if(idlFile != null)
		idlFile.delete();
	}
    }

    public static void main(String[] args){
	Packager pkgr = new Packager(args[0], args[1]);
	pkgr.generate(args[0], args[2], args[3], args[4], args[5]);
	int exitCode = 0;
	if(!pkgr.isGUI)
	    exitCode = NONGUI_EXITCODE;
	Runtime.getRuntime().exit(exitCode);
    }

    public void generate(String name, String clsid, String libIID, String srcIID, String dispIID) {
	try {
	    ClassLoader cl = this.getClass().getClassLoader();
	    Object bean = Beans.instantiate(cl, name);
	    if(bean instanceof Component) {
		isGUI = true;
	    }
	    BeanInfo bInfo = Introspector.getBeanInfo(bean.getClass());
	    int lIndex = name.lastIndexOf('.');
	    String shortName = name.substring(lIndex+1);

	    ps.println("[");
	    ps.println("uuid("+ libIID.substring(1, libIID.length()-1) + "),");
	    ps.println("version(1.0)");
	    ps.println("]");

	    ps.println("library " + shortName);
	    ps.println("{");

	    ps.println("importlib(\"Stdole2.tlb\");");
	    ps.println("dispinterface " + shortName + "Source;");
	    ps.println("dispinterface " + shortName + "Dispatch;");

	    ps.println("[");
	    ps.println("uuid(" + srcIID.substring(1, srcIID.length()-1) + "),");
	    ps.println("version(1.0)");
	    ps.println("]");

	    ps.println("dispinterface " + shortName + "Source {");
	    ps.println("properties:");
	    ps.println("methods:");
	    printEvents(bInfo.getEventSetDescriptors());
	    ps.println("};");

	    ps.println("[");
	    ps.println("uuid(" + dispIID.substring(1, dispIID.length()-1) + "),");
	    ps.println("version(1.0)");
	    ps.println("]");

	    ps.println("dispinterface " + shortName + "Dispatch {");
	    ps.println("properties:");
	    printProperties(bInfo, true);
	    ps.println("methods:");
	    printProperties(bInfo, false);
	    printMethods(bInfo.getMethodDescriptors(), Dispatch.methodBase, methodMap);
	    ps.println("};");

	    ps.println("[");
	    ps.println("uuid(" + clsid.substring(1, clsid.length()-1) + "),");
	    ps.println("version(1.0)");
	    ps.println("]");
	    
	    ps.println("coclass " + shortName + " {");
	    ps.println("[default, source] dispinterface " + shortName + "Source;");
	    ps.println("[default] dispinterface " + shortName + "Dispatch;");
	    ps.println("};");

	    ps.println("};");
	}catch(Exception exc){
	    exc.printStackTrace();
	    if(idlFile != null)
		idlFile.delete();
	}
    }

    private void printMethods(MethodDescriptor[] mds, int base, HashMap methMap){
	
	//Keep the one which takes the highest number of arguments among
	//the overloaded methods
	HashMap localMap = new HashMap();
	for(int i=0;i<mds.length;i++){
	    Method m = mds[i].getMethod();
	    Method prevM = (Method)localMap.get(m.getName());
	    if(prevM != null){
		if(prevM.getParameterTypes().length < m.getParameterTypes().length)
		    overLoadedMap.put(m.getName(), m);
		else
		    overLoadedMap.put(m.getName(), prevM);
	    }
	    localMap.put(m.getName(), m);
	}

	//print the methods to .idl file
	sort(mds);
	for(int i=0;i<mds.length;i++){
	    Method m = mds[i].getMethod();

	    boolean notAllowed = false;
	    for(int q=0;q<notAllowedMethods.length;q++){
		if(m.getName().equals(notAllowedMethods[q])){
		    notAllowed = true;
		    break;
		}
	    }

	    if(notAllowed == true)
		continue;

	    //Check if it is an overloaded method
	    Method oM = (Method)overLoadedMap.get(m.getName());
	    if(oM != null) {
		if(!m.equals(oM))
		    continue;
	    }

	    methMap.put(m.getName(), m);

	    if(oM != null)
		printOverLoadedMethod(mds[i], base+i);
	    else
		printMethod(mds[i], base+i);
	}
    }


    //Overloaded method returns VARIANT and takes arguments as VARIANTs
    private void printOverLoadedMethod(MethodDescriptor md, int dispid) {
	Method m = md.getMethod();
	ParameterDescriptor[] pds = md.getParameterDescriptors();
	Class[] pTypes = m.getParameterTypes();

	StringBuffer lineStrBuff = new StringBuffer("[id(" + dispid + ")]\n");
	lineStrBuff.append("VARIANT " + m.getName() + "(");
	String endStr = ", ";
	for(int k =0;k<pTypes.length;k++) {
	    lineStrBuff.append("[optional] VARIANT ");
	    if(k == pTypes.length-1)
		endStr = ");";
	    if(pds != null && pds[k] != null) {
		lineStrBuff.append(pds[k].getName() + endStr);
	    } else {
		lineStrBuff.append("var" + k + endStr);
	    }
	}

	if(pTypes.length == 0) {
	    lineStrBuff.append(");");
	}

	ps.println(lineStrBuff);
    }

    private void printMethod(MethodDescriptor md, int dispid) {
	Method m = md.getMethod();
	StringBuffer lineStrBuff = new StringBuffer("[id(" + dispid + ")]\n");
	String retType = TypeMap.getCOMType(m.getReturnType());
	lineStrBuff.append(retType + " " + m.getName() + "(");
	Class[] pTypes = m.getParameterTypes();
	ParameterDescriptor[] pds = md.getParameterDescriptors();

	String endStr = ", ";
	for(int k =0;k<pTypes.length;k++) {
	    lineStrBuff.append(TypeMap.getCOMType(pTypes[k]) + " ");
	    if(k == pTypes.length-1)
		endStr = ");";
	    if(pds != null && pds[k] != null) {
		lineStrBuff.append(pds[k].getName() + endStr);
	    } else {
		lineStrBuff.append("arg" + k + endStr);
	    }
	}

	if(pTypes.length == 0) {
	    lineStrBuff.append(");");
	}

	ps.println(lineStrBuff);
    }

    private void printEventMethods(MethodDescriptor[] mds, int base, HashMap evtMap){
	for(int i=0;i<mds.length;i++){
	    Method m = mds[i].getMethod();

	    //Check if the method was already printed
	    Method prevM = (Method)evtMap.get(m.getName());
	    if(prevM != null)
		continue;

	    evtMap.put(m.getName(), m);
	    printMethod(mds[i], base+i);
	}
    }

    private void printEvents(EventSetDescriptor[] eds){
	if(eds == null)return;
	MethodDescriptor m[] = null;
	Vector eventMds = new Vector();
	for(int i=0;i<eds.length;i++){
	     m = eds[i].getListenerMethodDescriptors();
	    for(int j=0;j<m.length;j++){
		eventMds.addElement(m[j]);
	    }
	}

	m = (MethodDescriptor[])eventMds.toArray(new MethodDescriptor[0]);
	sort(m);
	printEventMethods(m, Dispatch.eventBase, eventMap);
    }

    private void printProperties(BeanInfo bInfo, boolean propOnly) {
	PropertyDescriptor[] pds = bInfo.getPropertyDescriptors();
	sort(pds);
	int defaultIndex = bInfo.getDefaultPropertyIndex();
	for(int i=0;i<pds.length;i++){
	    Method rm = pds[i].getReadMethod();
	    Method wm = pds[i].getWriteMethod();
	    if(rm != null && wm != null && propOnly) {
		printProperty(pds[i], i, i == defaultIndex);
	    }
	    
	    if(!propOnly) {
		if(methodMap.get(pds[i].getName()) != null)
		    continue;

		boolean noProcessing = false;
		for(int q=0;q<notAllowedMethods.length;q++){
		    if(pds[i].getName().equals(notAllowedMethods[q])){
			noProcessing = true;
		    }
		}

		if(noProcessing = true)
		    continue;

		if(rm != null)
		    printGetProperty(pds[i], i, i == defaultIndex);
		else if(wm != null)
		    printPutProperty(pds[i], i, i == defaultIndex);
	    }
	}
    }


    private void printPropertyAttrib(PropertyDescriptor pd, int i, String type) {
	int dispid = Dispatch.propertyBase + i;
	StringBuffer lineStrBuff = new StringBuffer("[id(" + dispid + ")");
	if(type != null)
	    lineStrBuff.append(", " + type);
	if(pd.isBound() == true) {
	    lineStrBuff.append(", bindable");
	}
	if(pd.isConstrained() == true){
	    lineStrBuff.append(", requestedit");
	}
	lineStrBuff.append("]");
	ps.println(lineStrBuff);
    }	


    private void printProperty(PropertyDescriptor pd, int i, boolean def) {
	for(int q=0;q<javaProps.length;q++) {
	    if(pd.getName().equals(javaProps[q])) {
		ps.println(comProps[q]);
		return;
	    }
	}

	for(int q=0;q<notAllowedProps.length;q++){
	    if(pd.getName().equals(notAllowedProps[q])){
		return;
	    }
	}

	printPropertyAttrib(pd, i, null);

	StringBuffer lineStrBuff = new StringBuffer();
	String propType = TypeMap.getCOMType(pd.getPropertyType());
	lineStrBuff.append(propType + " " + pd.getName() + ";");
	ps.println(lineStrBuff);	
    }

    private void printGetProperty(PropertyDescriptor pd, int i, boolean def){
	Method m = pd.getReadMethod();
	methodMap.put(pd.getName(), m);				
	printPropertyAttrib(pd, i, "propget");

	StringBuffer lineStrBuff = new StringBuffer();
	String propType = TypeMap.getCOMType(pd.getPropertyType());
	lineStrBuff.append(propType + " " + pd.getName() + "();");
	ps.println(lineStrBuff);
    } 

    private void printPutProperty(PropertyDescriptor pd, int i, boolean def){
	Method m = pd.getReadMethod();
	methodMap.put(pd.getName(), m);				
	printPropertyAttrib(pd, i, "propput");

	StringBuffer lineStrBuff = new StringBuffer();
	String propType = TypeMap.getCOMType(pd.getPropertyType());
	lineStrBuff.append("void " + pd.getName() + "(" + propType + ");");
	ps.println(lineStrBuff);
    }
    
    static void sort(Object[] elements) {
	Arrays.sort(elements, new Comparator() {
	    public int compare(Object o1, Object o2) {
		String x = ((FeatureDescriptor)o1).getName();
		String y = ((FeatureDescriptor)o2).getName();
		return x.compareTo(y);
	    }
	});
    }
}

class TypeMap {
    static Object[][] map = {
	{Long.TYPE, "long"},
	{Integer.TYPE, "int"},
	{Double.TYPE, "double"},
	{Float.TYPE, "float"},
	{Short.TYPE, "short"},
	{String.class, "BSTR"},
	{Boolean.TYPE, "VARIANT_BOOL"},
	{Character.TYPE, "short"},
	{Byte.TYPE, "BYTE"}
    };

    static String getCOMType(Class type) {
    	if( type.toString().equals("void") )
	    return "void";
	else if( type.isArray() )
	    return "VARIANT";

	for(int i=0;i<map.length;i++){
	    if(map[i][0] == type)
	    return (String)map[i][1];
	}

	//none of the primitive types
	return "IDispatch*";
    }
}


