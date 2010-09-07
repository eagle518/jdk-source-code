/*
 * @(#)AppletPieces.java	1.17 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

import java.util.*;

/**
 *
 * AppletPieces encapsulates all the information around the APPLET tag.
 *
 */
public class AppletPieces extends Object {
    /*
      Breakdown of all the fields possible in an Applet Tag:
      <APPLET
      CODEBASE = codebaseURL
      ARCHIVE = archiveList
      CODE = appletFile
      OBJECT = serializedApplet
      ALT = alternateText
      NAME = appletInstanceName
      WIDTH = pixels
      HEIGHT = pixels
      ALIGN = alignment
      VSPACE = pixels
      HSPACE = pixels
      MAYSCRIPT
      >
      <PARAM NAME = appletAttribute1 VALUE = value >
      <PARAM NAME = appletAttribute2 VALUE = value >
      ...
      alternateHTML
      </APPLET>
      */
					
    private String _codeBase;
    private String _archive;
    private String _code;
    private String _object;
    private String _alt;
    private String _name;
    private String _width;
    private String _height;
    private String _align;
    private String _vspace;
    private String _hspace;
    private String _alternateHTML = "";
    private String _mayScript;
    private Vector params = new Vector();
	
    private String eolStr;

    public AppletPieces(String _eolStr) {
	eolStr = _eolStr;
    }
    public void setCODEBASE(String value) {	_codeBase = value; }
    public String getCODEBASE() { return _codeBase; }
	
    public void setARCHIVE(String value) {	_archive = value; }
    public String getARCHIVE() { return _archive; }
	
    public void setCODE(String value) {	_code = value; }
    public String getCODE() { return _code; }
	
    public void setOBJECT(String value) {	_object = value; }
    public String getOBJECT() { return _object; }
	
    public void setALT(String value) {	_alt = value; }
    public String getALT() { return _alt; }
	
    public void setNAME(String value) {	_name = value; }
    public String getNAME() { return _name; }
	
    public void setWIDTH(String value) { _width = value; }
    public String getWIDTH() { return _width; }
	
    public void setHEIGHT(String value) { _height = value; }
    public String getHEIGHT() { return _height; }
	
    public void setALIGN(String value) { _align = value; }
    public String getALIGN() { return _align; }
	
    public void setVSPACE(String value) { _vspace = value; }
    public String getVSPACE() { return _vspace; }
	
    public void setHSPACE(String value) { _hspace = value; }
    public String getHSPACE() { return _hspace; }

    public void setAlternateHTML(String value) { _alternateHTML = value; }
    public String getAlternateHTML() { return _alternateHTML; }
	
    public void setMAYSCRIPT(String value) { _mayScript = value; }
    public String getMAYSCRIPT() { return _mayScript; }

    private String getSeparator(boolean eol)  {
	return (eol ? eolStr : " ");
    }

    public String getAttributes(boolean eol) {
	StringBuffer value = new StringBuffer();

	if(_code!=null) 
	    value.append(getSeparator(eol) + _code);
	if(_codeBase!=null) 
	    value.append(getSeparator(eol) + _codeBase);
	if(_archive!=null) 
	    value.append(getSeparator(eol) + _archive);
	if(_object!=null) 
	    value.append(getSeparator(eol) + _object);
	if(_width!=null) 
	    value.append(getSeparator(eol) + _width);
	if(_height!=null) 
	    value.append(getSeparator(eol) + _height);
	if(_name!=null) 
	    value.append(getSeparator(eol) + _name);
	if(_align!=null) 
	    value.append(getSeparator(eol) + _align);
	if(_vspace!=null) 
	    value.append(getSeparator(eol) + _vspace);
	if(_hspace!=null) 
	    value.append(getSeparator(eol) + _hspace);
	if(_alt!=null) 
	    value.append(getSeparator(eol) + _alt);
	if(_mayScript!=null) 
	    value.append(getSeparator(eol) + _mayScript);

	return value.toString();
    }
    public String getObjectTagAttributes(boolean eol) {
	StringBuffer value = new StringBuffer();

	if(_width!=null) 
	    value.append(convertEscapes(_width)+getSeparator(eol));
	if(_height!=null) 
	    value.append(convertEscapes(_height)+getSeparator(eol));
	if(_name!=null) 
	    value.append(convertEscapes(_name)+getSeparator(eol));
	if(_align!=null) 
	    value.append(convertEscapes(_align)+getSeparator(eol));
	if(_vspace!=null) 
	    value.append(convertEscapes(_vspace)+getSeparator(eol));
	if(_hspace!=null) 
	    value.append(convertEscapes(_hspace)+getSeparator(eol));
	if(_alt!=null) 
	    value.append(convertEscapes(_alt)+getSeparator(eol));

	return value.toString();
    }
		
    public static String convertEscapes(String str) {
	StringTokenizer st = new StringTokenizer(str,"\'",true);
	String token = "";
	StringBuffer newStr = new StringBuffer();
	while(st.hasMoreTokens()) {
	    token = st.nextToken();
	    if(token.equals("\'")) newStr.append("\'");
	    else newStr.append(token);
	}
	return newStr.toString();
    }
		
    public String getObjectTagParams(boolean eol) {
	StringBuffer value = new StringBuffer();
	StringTokenizer st;
	String token;

	if(_code!=null) {
	    String theValue = "";
	    st = new StringTokenizer(_code);
	    token = st.nextToken(); token = st.nextToken();
	    while(st.hasMoreTokens()) {
		if((token = st.nextToken()).endsWith(">"))
		    token = token.substring(0,token.length()-2);
		theValue += token+" ";
	    }
				
	    value.append("<PARAM NAME = CODE VALUE = "+theValue+">"+eolStr);
	}
	if(_codeBase!=null) {
	    String theValue = "";
	    st = new StringTokenizer(_codeBase);
	    token = st.nextToken(); token = st.nextToken();
	    while(st.hasMoreTokens()) {
		if((token = st.nextToken()).endsWith(">"))
		    token = token.substring(0,token.length()-2);
		theValue += token+" ";
	    }

	    value.append("    <PARAM NAME = CODEBASE VALUE = "+theValue+">"+eolStr);
	}
	if(_archive!=null) {
	    String theValue = "";
	    st = new StringTokenizer(_archive);
	    token = st.nextToken(); token = st.nextToken();  //  Clear out the name and = 
	    while(st.hasMoreTokens()) {
		if((token = st.nextToken()).endsWith(">"))
		    token = token.substring(0,token.length()-2);
		theValue += token+" ";
	    }
				
	    value.append("    <PARAM NAME = ARCHIVE VALUE = "+theValue+">"+eolStr);
	}
	if(_object!=null) {
	    String theValue = "";
	    st = new StringTokenizer(_object);
	    token = st.nextToken(); token = st.nextToken();  //  Clear out the name and = 
	    while(st.hasMoreTokens()) {
		if((token = st.nextToken()).endsWith(">"))
		    token = token.substring(0,token.length()-2);
		theValue += token+" ";
	    }
				
	    value.append("    <PARAM NAME = OBJECT VALUE = "+theValue+">"+eolStr);
	}
	if(_name!=null) {
	    String theValue = "";
	    st = new StringTokenizer(_name);
	    token = st.nextToken(); token = st.nextToken();  //  Clear out the name and = 
	    while(st.hasMoreTokens()) {
		if((token = st.nextToken()).endsWith(">"))
		    token = token.substring(0,token.length()-2);
		theValue += token+" ";
	    }
				
	    value.append("    <PARAM NAME = NAME VALUE = "+theValue+">"+eolStr);
	}
	if(_mayScript!=null) {
	    String theValue = "";
	    st = new StringTokenizer(_mayScript);
	    token = st.nextToken(); token = st.nextToken();  //  Clear out the name and = 
	    while(st.hasMoreTokens()) {
		if((token = st.nextToken()).endsWith(">"))
		    token = token.substring(0,token.length()-2);
		theValue += token+" ";
	    }
				
	    value.append("    <PARAM NAME = MAYSCRIPT VALUE = "+theValue+">"+eolStr);
	}
	return value.toString().trim();
    }
	
    public String getAppletText(boolean eol) {
	StringBuffer value = new StringBuffer();

	value.append("<APPLET");
	value.append(getAttributes(eol));
	value.append(">"+eolStr);
				
	for(int i = 0;i<params.size();i++) 
	    value.append((String)params.elementAt(i)+eolStr);
			
	value.append(_alternateHTML+eolStr);
		
	value.append(eolStr+"</APPLET>");
		
	return value.toString();
    }
    public void addParam(String p) { params.addElement(p); }
    public Enumeration getParamEnumeration() { return params.elements(); };

    public String getEmbedTagAttributes()	{
	StringBuffer value = new StringBuffer();
	StringTokenizer st;
	String token;
	String tabX3 = "            ";
	String newLine = "\n";
        String escape = " \\";

	if(_code!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_code));
	}
	if(_codeBase!=null) {
            /*
             * Workaround for Netscape bug:  Netscape looks at codebase parameter
             * to find plugins' page instead of looking at pluginspage parameter.
             * Workaround:  convert codebase to JAVA_CODEBASE for EMBED tag only.
             */
            _codeBase = "JAVA_"+_codeBase;
	    value.append(escape + newLine + tabX3 + convertEscapes(_codeBase));
	}
	if(_archive!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_archive));
	}
	if(_object!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_object));
	}
	if(_alt!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_alt));
	}
	if(_name!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_name));
	}
	if(_width!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_width));
	}
	if(_height!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_height));
	}
	if(_align!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_align));
	}
	if(_vspace!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_vspace));
	}
	if(_hspace!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_hspace));
	}
	if(_mayScript!=null) {
	    value.append(escape + newLine + tabX3 + convertEscapes(_mayScript));
	}

	return value.toString();
    }

    public String toString() {
	String retVal =  
	    "\nCODEBASE: "+_codeBase+
	    "\nARCHIVE: "+_archive+
	    "\nCODE: "+_code+
	    "\nOBJECT: "+_object+
	    "\nALT: "+_alt+
	    "\nNAME: "+_name+
	    "\nWIDTH: "+_width+
	    "\nHEIGHT: "+_height+
	    "\nALIGN: "+_align+
	    "\nVSPACE: "+_vspace+
	    "\nHSPACE: "+ _hspace+
	    "\nMAYSCRIPT: "+ _mayScript;

	retVal += "\nParams:";
	Enumeration e = getParamEnumeration();
	while(e.hasMoreElements()) {
	    retVal+="\n"+(String)e.nextElement();
	}

	retVal += "\nAlternate HTML:";
	retVal += "\n"+_alternateHTML;
		
	return retVal;
    }
}
