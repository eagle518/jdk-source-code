/*
 * @(#)CPX2.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.javap;

/**
 *  Stores constant pool entry information with two fields.
 *
 * @author  Sucheta Dambalkar (Adopted code from jdis)
 */
class CPX2 {
    int cpx1,cpx2;
    
    CPX2 (int cpx1, int cpx2) {
	this.cpx1=cpx1;
	this.cpx2=cpx2;
    }
}
