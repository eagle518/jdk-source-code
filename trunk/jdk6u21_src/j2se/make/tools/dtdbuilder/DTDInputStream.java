/*
 * @(#)DTDInputStream.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package dtdbuilder;

import javax.swing.text.html.parser.*;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.Reader;
import java.io.InputStreamReader;
import java.io.CharArrayReader;
import java.io.FilterReader;
import java.util.Stack;
import java.net.URL;

/**
 * A stream for reading HTML files. This stream takes care
 * of \r\n conversions and parameter entity expansion.
 *
 * @see DTD
 * @see DTDParser
 * @version    1.11, 03/23/10
 * @author Arthur van Hoff
 * @author Steven B. Byrne
 */
public final
class DTDInputStream extends FilterReader implements DTDConstants {
    public DTD dtd;
    public Stack stack = new Stack();
    public char str[] = new char[64];
    public int replace = 0;
    public int ln = 1;
    public int ch;

    /**
     * Create the stream.
     */
    public DTDInputStream(InputStream in, DTD dtd) throws IOException {
	super(new InputStreamReader(in));
	this.dtd = dtd;
	this.ch = in.read();
    }

    /**
     * Error
     */
    public void error(String msg) {
	System.out.println("line " + ln + ": dtd input error: " + msg);
    }

    /**
     * Push a single character
     */
    public void push(int ch) throws IOException {
	char data[] = {(char)ch};
	push(new CharArrayReader(data));
    }


    /**
     * Push an array of bytes.
     */
    public void push(char data[]) throws IOException {
	if (data.length > 0) {
	    push(new CharArrayReader(data));
	}
    }

    /**
     * Push an entire input stream
     */
    void push(Reader in) throws IOException {
	stack.push(new Integer(ln));
	stack.push(new Integer(ch));
	stack.push(this.in);
	this.in = in;
	ch = in.read();
    }

    /**
     * Read a character from the input. Automatically pop
     * a stream of the stack if the EOF is reached. Also replaces
     * parameter entities.
     * [60] 350:22
     */
    public int read() throws IOException {
	switch (ch) {
	  case '%': {
	    ch = in.read();
	    if (replace > 0) {
		return '%';
	    }

	    int pos = 0;
	    while (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) ||
		   ((ch >= '0') && (ch <= '9')) || (ch == '.') || (ch == '-')) {
		str[pos++] = (char)ch;
		ch = in.read();
	    }
	    if (pos == 0) {
		return '%';
	    }

	    String nm = new String(str, 0, pos);
	    Entity ent = dtd.getEntity(nm);
	    if (ent == null) {
		error("undefined entity reference: " + nm);
		return read();
	    }

	    // Skip ; or RE
	    switch (ch) {
	      case '\r':
		ln++;
	      case ';':
		ch = in.read();
		break;
	      case '\n':
		ln++;
		if ((ch = in.read()) == '\r') {
		    ch = in.read();
		}
		break;
	    }

	    // Push the entity.
	    try {
		push(getEntityInputReader(ent));
	    } catch (Exception e) {
		error("entity data not found: " + ent + ", " + ent.getString());
	    }
	    return read();
	  }

	  case '\n':
	    ln++;
	    if ((ch = in.read()) == '\r') {
		ch = in.read();
	    }
	    return '\n';

	  case '\r':
	    ln++;
	    ch = in.read();
	    return '\n';

	  case -1:
	    if (stack.size() > 0) {
		in = (Reader)stack.pop();
		ch = ((Integer)stack.pop()).intValue();
		ln = ((Integer)stack.pop()).intValue();
		return read();
	    }
	    return -1;

	  default:
	    int c = ch;
	    ch = in.read();
	    return c;
	}
    }

    /**
    * Return the data as a stream.
    */
    private Reader getEntityInputReader(Entity ent) throws IOException {
        if ((ent.type & Entity.PUBLIC) != 0) {
            // InputStream is = DTDBuilder.mapping.get(ent.getString()).openStream();
            // return new InputStreamReader(is);
	    String path = DTDBuilder.mapping.get(ent.getString());

            return new InputStreamReader(new FileInputStream(path));
        }
        if ((ent.type & Entity.SYSTEM) != 0) {
            //InputStream is =  new URL(DTDBuilder.mapping.base, ent.getString()).openStream();
	    String path = DTDBuilder.mapping.baseStr +  ent.getString();
            return new InputStreamReader(new FileInputStream(path));
        }
        return new CharArrayReader(ent.data);
    }

}
