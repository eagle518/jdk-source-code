/*
 * @(#)SignatureConstants.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;
	
interface SignatureConstants {
    static final String 
        CLASS        = "CLSS ",
        SUPER        = "supr ",
        INTERFACE    = "intf ",
        CONSTRUCTOR  = "cons ",
        METHOD       = "meth ",
        FIELD        = "fld  ",
        // If the field is a primitive constants, then this modifier is added
        // to the field definition.
        PRIMITIVE_CONSTANT        = "constant",
	INNER        = "innr ",
	NATIVE       = "native",
	SYNCHRONIZED = "synchronized",
        TRANSIENT    = "transient",
        FLAG_SUPER   = "flag_super",
        // if the member is synthetic, then this modifier is added
        // to the field definition.
        SYNTHETIC    = "<synthetic>";
    

    static final String[][] prefixes = {
	{CLASS, "class "},
	{SUPER, "superclass "},
	{INTERFACE, "interface "},
	{CONSTRUCTOR, "constructor "},
	{METHOD, "method "},
	{FIELD, "field "},
	{PRIMITIVE_CONSTANT, "field "},
	{INNER, "innerclass "},
    };
}
