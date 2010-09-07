/*
 * @(#)BasicPrinter.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import java.awt.*;
import java.io.*;
import java.util.*;


public class BasicPrinter extends Object {

    private static Properties printprefs = new Properties();
    private static Font printerFont = new Font("Courier",Font.PLAIN,10);
    private static final boolean debug = false;
	
    public static void printDocument(Frame someFrame, String doc, String setupTitle) {
	
	Toolkit toolkit = Toolkit.getDefaultToolkit();
	if(debug)
	    System.out.println("starting print");

	PrintJob job = toolkit.getPrintJob(someFrame,setupTitle,printprefs);
		
	if(job == null) return;
				
	Graphics page = job.getGraphics();
	page.setFont(printerFont);
	FontMetrics fm = page.getFontMetrics();
	Dimension pageSize = job.getPageDimension();
		
	StringTokenizer st = new StringTokenizer(doc,"\n\r",false);
	int vMargin = 10;
	int hMargin = 5;
	int h = vMargin;
	int pageNumber = 1;
	String token = "";
		
	//  if next token
	while(st.hasMoreTokens()) {
	    token = st.nextToken();

	    if( h < pageSize.height ) {
		if(debug) System.out.println(token);
		else page.drawString(token,hMargin,h);
				
		h += fm.getMaxDescent()+fm.getMaxAscent();
	    }
	    else {
		pageNumber++;
		h = vMargin;
		if(!debug) {
		    page.dispose();
		    page = job.getGraphics();
		    page.setFont(printerFont);
		    fm = page.getFontMetrics();
		}
	    }
	}
		
	if(!debug) page.dispose();
	job.end();	
					
    }

    /*	public static void printDocument(Frame someFrame, String doc, String setupTitle) {
	
	Toolkit toolkit = Toolkit.getDefaultToolkit();
	if(debug)
	System.out.println("starting print");

	PrintJob job = toolkit.getPrintJob(someFrame,setupTitle,printprefs);
		
	if(job == null) return;
				
	Graphics page = job.getGraphics();
	page.setFont(printerFont);
	FontMetrics fm = page.getFontMetrics();
	Dimension pageSize = job.getPageDimension();
		
	StringTokenizer st = new StringTokenizer(doc," \t\n\r",true);
	int vMargin = 10;
	int hMargin = 5;
	int w = hMargin;
	int h = vMargin;
	int lineSpacing = 1;
	int pageNumber = 1;
	String token = "";
	boolean goodPage = false;
		
	//  if next token
	while(st.hasMoreTokens()) {
	token = st.nextToken();
			
	NEWPAGE: 
	if( token.charAt(0) == 10 || token.charAt(0) == 13 ) {
	if(debug)  System.out.println("Length:  "+token.length()+":  "+(token.charAt(0) == 10? "10":"13"));
	h += fm.getMaxDescent()+fm.getMaxAscent();
	w = hMargin;
	if(h > pageSize.height) {
	pageNumber++;
	h = vMargin;
	if(debug) {
	System.out.println("New Page Number:  "+pageNumber);
	}
	else {
	page.dispose();
	page = job.getGraphics();
	page.setFont(printerFont);
	fm = page.getFontMetrics();
	goodPage = false;
	}
	}
	if(st.hasMoreTokens()) {
	token = st.nextToken();
	if(token.charAt(0) != 13) {
	break NEWPAGE;
	}
	}
	}
	else {
	if( h < pageSize.height ) {
	if(debug) {
	System.out.println("draw:  ->"+token+"<-");
	}
	else {
	page.drawString(token,w,h);
	}
	goodPage = true;
	w += fm.stringWidth(token);
	}
	else {
	pageNumber++;
	w = hMargin;
	h = vMargin;
	if(debug) {
	System.out.println("New Page Number:  "+pageNumber);
	}
	else {
	page.dispose();
	page = job.getGraphics();
	page.setFont(printerFont);
	fm = page.getFontMetrics();
	goodPage = false;
	}
	break NEWPAGE;
	}
	}	
	}
		
	if(debug)
	System.out.println("Done");
			
	if(goodPage) page.dispose();
	job.end();	
					
	}*/
}
