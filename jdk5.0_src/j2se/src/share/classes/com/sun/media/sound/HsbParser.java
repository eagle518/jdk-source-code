/*
 * @(#)HsbParser.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.io.InputStream;
import java.io.IOException;
import java.io.File;

import java.net.URL;

import javax.sound.midi.Soundbank;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.spi.SoundbankReader;

			 


/******************************************************************************************
IMPLEMENTATION TODO:


******************************************************************************************/



/**
 * Headspace Soundbank parser.
 *
 * @version 1.13 03/12/19
 * @author Kara Kytle
 */
public class HsbParser extends SoundbankReader {


    // CONSTRUCTOR

    public HsbParser() {
    }			 
																				   

    // SOUNDBANK PARSER METHODS


    public Soundbank getSoundbank(URL url) throws InvalidMidiDataException, IOException {

	try {

	    HeadspaceSoundbank sb = new HeadspaceSoundbank(url);
	    return sb;

	} catch (IllegalArgumentException e) {

	    return null;
	}
    }

    public Soundbank getSoundbank(InputStream stream) throws InvalidMidiDataException, IOException {

	try {

	    HeadspaceSoundbank sb = new HeadspaceSoundbank(stream);
	    return sb;

	} catch (IllegalArgumentException e) {

	    return null;
	}
    }

    public Soundbank getSoundbank(File file) throws InvalidMidiDataException, IOException {

	try {

	    HeadspaceSoundbank sb = new HeadspaceSoundbank(file);
	    return sb;

	} catch (IllegalArgumentException e) {

	    return null;
	}
    }
}

										
