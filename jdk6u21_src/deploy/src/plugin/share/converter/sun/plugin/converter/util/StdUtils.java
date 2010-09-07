/*
 * @(#)StdUtils.java	1.17 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import java.awt.Component;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.MediaTracker;
import java.io.*;
import java.awt.Window;
import java.awt.Dimension;
import java.util.StringTokenizer;

public class StdUtils {
    public static boolean debug = false;

    private static StreamTokenizer buildWordChars(StreamTokenizer st, String word, boolean reset, boolean bothCases) {
	
	if(reset) st.resetSyntax();
		
	for(int i = 0;i<word.length();i++) {
	    char c = word.charAt(i);			
	    st.wordChars(c,c);
	    if(bothCases) {
		if(Character.isLowerCase(c)) st.wordChars(Character.toUpperCase(c),Character.toUpperCase(c));
		else st.wordChars(Character.toLowerCase(c),Character.toLowerCase(c));
	    }
	}
	return st;
    }
	
    public static boolean createDirs(File dir){
	if(dir.exists()) return true;
	return dir.mkdirs();
    }


    //added the encoding parameter so we know the encoding when we
    //use the InputStreamReader
    public static String getEOLs(InputStream is, String encoding) 
		  throws IOException
    {
	InputStreamReader isReader = null;
	if ( encoding == null ) {
	    isReader = new InputStreamReader(is);
	}
	else {
	    isReader = new InputStreamReader(is, encoding);
	}
	BufferedReader in = new BufferedReader(isReader);

	String eolStr = "";
	try {
	    int lineEnd1 = -1;
	    int lineEnd2 = -1;
	    while( (lineEnd1 = in.read())!=-1 ) {
		if(lineEnd1 == 13 || lineEnd1 == 10) {
		    eolStr = String.valueOf((char)lineEnd1);
		    lineEnd2 = in.read();
		    if((lineEnd2 == 13 || lineEnd2 == 10) && lineEnd2 != lineEnd1) {
			eolStr = eolStr+String.valueOf((char)lineEnd2);
			if(debug) System.out.println("EOLs are 2 chars (ASCII):  "+(int)eolStr.charAt(0)+" "+(int)eolStr.charAt(1));
		    }
		    else {
			if(debug) System.out.println("EOLs are 1 char (ASCII):  "+(int)eolStr.charAt(0));
		    }
		    return eolStr;
		}
	    }
	    return eolStr;
	}
	catch(IOException e) 
	{
	    e.printStackTrace();
	}

	return eolStr;
    }

    public static int countWords(File file, String target, boolean anyCase, String encoder) {
	String[] targets = { target };
	int targetCount = 0;
	StreamTokenizer st;
	try {
	   
	    //if there is an encoding us it, else use the default
	    if(encoder == null)
		st = new StreamTokenizer(new InputStreamReader(new BufferedInputStream(new FileInputStream(file))));
	    else
		st = new StreamTokenizer(new InputStreamReader(new BufferedInputStream(new FileInputStream(file)), encoder));

	    st.resetSyntax();  	//  Initialize to no words/tokens
	    st.eolIsSignificant(true); 	//  We do want to see EOL's
			
	    //  Set up the chars that should be part of words
	    for(int i = 0;i<targets.length;i++)
		StdUtils.buildWordChars(st,targets[i],false, anyCase);
			
	    int token;
	    boolean targetReached;
	    //  Get each token, compare with targets, copy if not target, stop if target found
	found:
	    while((token = st.nextToken())!=StreamTokenizer.TT_EOF) {
		if(token == StreamTokenizer.TT_WORD) {
		    for(int i = 0;i<targets.length;i++) {
			if(anyCase)
			    targetReached = st.sval.toUpperCase().equals(targets[i]);
			else 
			    targetReached = st.sval.equals(targets[i]);
			if(targetReached) {
			    targetCount++;
			}
		    }
		}
	    }
	}
	catch(IOException e) {
	    e.printStackTrace();
	}
	return targetCount;
    }
}
