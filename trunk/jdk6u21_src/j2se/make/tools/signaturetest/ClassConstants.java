/*
 * @(#)ClassConstants.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

public interface ClassConstants {
    /* Class File Constants */
    int JAVA_MAGIC                   = 0xcafebabe;
    int JAVA_VERSION                 = 45;
    int JAVA_MINOR_VERSION           = 3;

    /* Constant table */
    int CONSTANT_UTF8                = 1;
    int CONSTANT_UNICODE             = 2;
    int CONSTANT_INTEGER             = 3;
    int CONSTANT_FLOAT               = 4;
    int CONSTANT_LONG                = 5;
    int CONSTANT_DOUBLE              = 6;
    int CONSTANT_CLASS               = 7;
    int CONSTANT_STRING              = 8;
    int CONSTANT_FIELD               = 9;
    int CONSTANT_METHOD              = 10;
    int CONSTANT_INTERFACEMETHOD     = 11;
    int CONSTANT_NAMEANDTYPE         = 12;

    /* Access and modifier flags */
    int ACC_PUBLIC                   = 0x00000001;
    int ACC_PRIVATE                  = 0x00000002;
    int ACC_PROTECTED                = 0x00000004;
    int ACC_STATIC                   = 0x00000008;
    int ACC_FINAL                    = 0x00000010;
    int ACC_SYNCHRONIZED             = 0x00000020;
    int ACC_VOLATILE                 = 0x00000040;
    int ACC_TRANSIENT                = 0x00000080;
    int ACC_NATIVE                   = 0x00000100;
    int ACC_INTERFACE                = 0x00000200;
    int ACC_ABSTRACT                 = 0x00000400;
    int ACC_SUPER                    = 0x00000020;
    int ACC_STRICT		     = 0x00000800;
    int ACC_EXPLICIT		     = 0x00001000;
}
