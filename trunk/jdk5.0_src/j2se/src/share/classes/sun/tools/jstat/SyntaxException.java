/*
 * @(#)SyntaxException.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import java.io.StreamTokenizer;
import java.util.Set;
import java.util.Iterator;

/**
 * An exception class for syntax exceptions detected by the options file
 * parser.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class SyntaxException extends ParserException {
    private String message;

    public SyntaxException(String message) {
        this.message = message;
    }

    public SyntaxException(int lineno, String expected, String found) {
        message = "Syntax error at line " + lineno
                  + ": Expected " + expected
                  + ", Found " + found;
    }

    public SyntaxException(int lineno, String expected, Token found) {
        message = "Syntax error at line " + lineno
                  + ": Expected " + expected
                  + ", Found " + found.toMessage();
    }

    public SyntaxException(int lineno, Token expected, Token found) {
        message = "Syntax error at line " + lineno
                  + ": Expected " + expected.toMessage()
                  + ", Found " + found.toMessage();
    }

    public SyntaxException(int lineno, Set expected, Token found) {
        StringBuilder msg = new StringBuilder();

        msg.append("Syntax error at line " + lineno + ": Expected one of \'");

        boolean first = true;
        for (Iterator i = expected.iterator(); i.hasNext(); /* empty */) {
            String keyWord = (String)i.next();
            if (first) {
                msg.append(keyWord);
                first = false;
            } else {
                msg.append("|" + keyWord);
            }
        }

        msg.append("\', Found " + found.toMessage());
        message = msg.toString();
    }

    public String getMessage() {
        return message;
    }
}
