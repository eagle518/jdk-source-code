/*
 * @(#)HeadspaceInstrument.java	1.9 03/12/19
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
 * Instrument from a Headspace Soundbank.
 *
 * @version 1.9 03/12/19
 * @author Kara Kytle
 */
class HeadspaceInstrument extends Instrument {

    // each headspace "bank" contains 128 instruments
    private static final int BANK_SIZE = 128;
		
    private final int id;
    private final int size;


    // CONSTRUCTOR

    HeadspaceInstrument(HeadspaceSoundbank hsb, String name, int id, int size) {

	// we can calculate the bank and program from the id returned
	super(hsb, new Patch((id / BANK_SIZE), (id % BANK_SIZE)), name, null);

	this.id = id;
	this.size = size;
    }			 
																				   

    // ABSTRACT METHOD IMPLEMENTATIONS
																				   
    public Object getData() {
	return null;		 
    }


    // ADDITIONAL METHODS

    int getId() {
	return id;
    }  


    // OVERRIDES

    public String toString() {
	return new String("Instrument " + getName() + " (bank " + getPatch().getBank() + " program " + getPatch().getProgram() + ")");
    }

}
