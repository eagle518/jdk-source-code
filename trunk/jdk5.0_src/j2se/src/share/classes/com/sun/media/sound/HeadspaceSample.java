/*
 * @(#)HeadspaceSample.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Vector;

import javax.sound.midi.*;

			 
/******************************************************************************************
IMPLEMENTATION TODO:


******************************************************************************************/

/**
 * Sample from a Headspace Soundbank.
 *
 * @version 1.9 03/12/19
 * @author Kara Kytle
 */
class HeadspaceSample extends SoundbankResource {

    private final int index;	
    private final int id;
    private final int size;


    // CONSTRUCTOR

    HeadspaceSample(HeadspaceSoundbank hsb, String name, int index, int id, int size) {

	// we can calculate the bank and program from the id returned
	//super(hsb, index);

	super(hsb, name, null);

	// $$kk: 09.27.99: should make this provide an AudioInputStream as the data class

	this.index = index;
	this.id = id;
	this.size = size;
    }		
	

    // ABSTRACT METHOD IMPLEMENTATIONS
																				   
    public Object getData() {

	// $$kk: 04.12.99: need to implement this to return an AudioInputStream!!
	return null;		 
    }


    // OVERRIDES

    public String toString() {
	return getName();
    }
}
