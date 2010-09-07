/*
 * @(#)MemberDefinition.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

import java.util.Vector;

/** This is class for analysis of the member definitions. **/
class MemberDefinition implements SignatureConstants {
    static final int PRIVATE   = 0;
    static final int DEFAULT   = 1;
    static final int PROTECTED = 2;
    static final int PUBLIC    = 3;

    /** words from definition. **/
    Vector definitions;
    /** original member definition. **/
    String stringDefinition;
    /** name of the enclosing class. **/
    String name;

    /** creates MemberDefinition for given definition and enclosing class.
     *  @param name name of the enclosing class
     *  @param def member definition. **/
    MemberDefinition(String name, String def) {
        definitions = new Vector();
	int fromIndex;
	for (fromIndex = 0; def.indexOf(' ', fromIndex) >= 0;) {
	    int pos = def.indexOf(' ', fromIndex);
	    definitions.addElement(def.substring(fromIndex, pos));
	    fromIndex = pos + 1;
	}
	if (fromIndex < def.length())
	    definitions.addElement(def.substring(fromIndex));
        this.stringDefinition = def;
        this.name = name;
    }

    /** Returns member signature with local name **/
    String getShortSignature() {
        String sign = getSignature();
        int pos = sign.lastIndexOf('(');
        pos = (pos < 0) ? (sign.length() - 1) : (pos - 1);
        pos = Math.max(sign.lastIndexOf('.', pos), 
                       Math.max(sign.lastIndexOf('$', pos), 
                                sign.lastIndexOf(' ', pos)));
        return sign.substring(pos + 1);
    }

    /** Returns qualified name of the class which declares member. **/
    String getDeclaringClass() {
        String sign = getSignature();
        int pos = sign.lastIndexOf('(');
        pos = (pos < 0) ? (sign.length() - 1) : (pos - 1);
        pos = Math.max(sign.lastIndexOf('.', pos), sign.lastIndexOf('$', pos));
        return sign.substring(0, pos);
    }   
            

    /** Returns type of the fields or return type of the methods. **/
    String getType() {
        if (stringDefinition.startsWith(METHOD) ||
            stringDefinition.startsWith(FIELD)) {
            int pos = definitions.lastIndexOf("throws");
            if (pos >= 0) 
                return (String)definitions.elementAt(pos - 2);
            else
                return (String)definitions.elementAt(definitions.size() - 2);
        } else {
            return null;
        }
    }

    /** Returns int code of the access modifier. **/
    int getAccesModifier() {
        if (definitions.contains("private"))
            return PRIVATE;
        if (definitions.contains("protected"))
            return PROTECTED;
        if (definitions.contains("public"))
            return PUBLIC;
        return DEFAULT;
    }

    /** Returns member signature with qualified name **/
    private String getSignature() {
        int pos = definitions.indexOf("throws");
        pos = (pos < 0) ? definitions.size() : pos;
        return (String)definitions.elementAt(pos - 1);
    }
}
