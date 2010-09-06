/*
 * @(#)HeadspaceSoundbank.java	1.22 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.net.URL;
import java.io.InputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import java.util.Vector;

import javax.sound.midi.*;


/**
 * HeadspaceSoundbank
 *
 * @version 1.22, 03/12/19
 * @author David Rivas
 * @author Kara Kytle
 */
class HeadspaceSoundbank implements Soundbank {


    // STATE VARIABLES

    String name;
    String version;
    String vendor;
    String description;
    Vector instruments = new Vector();
    Vector samples = new Vector();
    Vector sequences = new Vector();


    // IMPLEMENTATION VARIABLES

    private long id = 0;		// resource id for engine


    // CONSTRUCTOR

    HeadspaceSoundbank(URL url) throws IOException {

	if(Printer.trace) Printer.trace("HeadspaceSoundbank: constructor: url: " + url);

	String protocol = url.getProtocol();
	if ( ! (protocol.equals("file")) ) { 

	    InputStream stream = url.openStream();
	    try {
	    	initialize(stream, false);
	    } catch (IllegalArgumentException e) {
	    	stream.close();
	    	throw e;
	    }
	} else {
	    String path = url.getFile();
	    initialize(path);
	}

	if(Printer.trace) Printer.trace("HeadspaceSoundbank: constructor: url: " + url + " completed");
    }		


    HeadspaceSoundbank(String path) {

	initialize(path);
	if(Printer.trace) Printer.trace("HeadspaceSoundbank: constructor: path: " + path + " completed");
    }

    HeadspaceSoundbank(InputStream stream) throws IOException {
		
	if(Printer.trace) Printer.trace("HeadspaceSoundbank: constructor: stream: " + stream);

	initialize(stream, true);
    }

    HeadspaceSoundbank(File file) throws IOException {

	if(Printer.trace) Printer.trace("HeadspaceSoundbank: constructor: file: " + file);

	FileInputStream fis = new FileInputStream( file );
	try {
	    initialize(fis, false);
	} catch (IllegalArgumentException e) {
	    fis.close();
	    throw e;
	}
    }




    // SOUNDBANK METHODS

    public String getName() {
	return name;
    }


    public String getVersion() {
	return version;
    }


    public String getVendor() {
	return vendor;
    }


    public String getDescription() {
	return description;
    }


    public SoundbankResource[] getResources() {
		
	SoundbankResource[] sampleArray;
		
	synchronized(samples) {
	
	    sampleArray = new SoundbankResource[samples.size()];

	    for (int i = 0; i < sampleArray.length; i++) {
		sampleArray[i] = (SoundbankResource)samples.elementAt(i);
	    }
	}

	/* $$kk: 09.27.99: need to implement for embedded sequences, too! */

	return sampleArray;
    }


    public Instrument[] getInstruments() {
		
	Instrument[] instrumentArray;
		
	synchronized(instruments) {
	
	    instrumentArray = new Instrument[instruments.size()];

	    for (int i = 0; i < instrumentArray.length; i++) {
		instrumentArray[i] = (Instrument)instruments.elementAt(i);
	    }
	}

	return instrumentArray;
    }


    public Instrument getInstrument(Patch patch) {

	Instrument returnedInstrument;
		
	synchronized(instruments) {
	
	    for (int i = 0; i < instruments.size(); i++) {

		returnedInstrument = (Instrument)instruments.elementAt(i);
		if ( (returnedInstrument.getPatch().getBank() == patch.getBank()) &&
		     (returnedInstrument.getPatch().getProgram() == patch.getProgram()) )

		    return returnedInstrument;
	    }
	}

	return null;		
    }


    // HELPER METHODS

    private void setupInformation() {
	
	// set up all the information now

	name = nGetName(id);
	if (name == null) {
	    name = "Untitled Beatnik Soundbank";
	}

	int versionMajor = nGetVersionMajor(id);
	int versionMinor = nGetVersionMinor(id);
	int versionSubMinor = nGetVersionSubMinor(id);

	version = new String(versionMajor + "." + versionMinor + "." + versionSubMinor);

	vendor = "Sun Microsystems, Beatnik";
	description = "Soundbank for use with Java Sound Audio Engine";

	nGetInstruments(id, instruments);
	nGetSamples(id, samples);
	//nGetSequences(id, sequences);

    }
	
    private void initialize(InputStream stream, boolean doReset) 
              throws IOException {

	if(Printer.trace) Printer.trace("HeadspaceSoundbank: initialize: stream: " + stream);

	// $$kk: 04.12.99: this is sort of a hack.  we need to make sure the library
	// gets loaded and (potentially) that the mixer gets initialized.
	HeadspaceMixer mixer = HeadspaceMixer.getMixerInstance();

	if (doReset) {
	    //$$fb 2001-07-17: bug 4444193: mark the stream here
	    // the 1024 is completely arbitrary as the entire file will be read
	    stream.mark(1024);
	}

	// load soundbank into a local byte array
	ByteArrayOutputStream baos = new ByteArrayOutputStream();
	byte buffer[] = new byte[16384];
	byte loadedbank[] = null;
	int bytesRead = 0;
	boolean parsedHeader = false;
	int readLength = 4;
		
	// this loop can throw an IOException
	while(true) {
	    bytesRead = stream.read(buffer, 0, readLength);
	    readLength = buffer.length;
	    if(bytesRead <= 0) {
		//$$fb 2001-07-17: Bug 4444193
		//  do not close the stream, it may be needed for following
		//  SoundbankReaders in MidiSystem's iteration
		//stream.close();
		break;
	    }
	    if (!parsedHeader) {
	    	// these soundbanks start with IREZ = 0x49 0x52 0x45 0x5A
	    	if ((bytesRead == 4)
	    	    && ((buffer[0] & 0xFF) == 0x49)
	    	    && ((buffer[1] & 0xFF) == 0x52)
	    	    && ((buffer[2] & 0xFF) == 0x45)
	    	    && ((buffer[3] & 0xFF) == 0x5A)) {
	    	    parsedHeader = true;
	    	} else {
	    	    break;
	    	}
	    }
	    baos.write(buffer, 0, bytesRead);
	}
	if (parsedHeader) {
	    loadedbank = baos.toByteArray();

	    // bring the byte array down to native code and open the resource

	    if(Printer.debug) Printer.debug("> calling nOpenResourceFromByteArray with length: " + loadedbank.length );
	    id = nOpenResourceFromByteArray(loadedbank, loadedbank.length);
	    if(Printer.debug) Printer.debug("> returned from nOpenResourceFromByteArray; id = " + id);
	}

	if (id == 0) {
	    //$$fb 2001-07-17: Bug 4444193
	    //  reset the stream to give other SoundbankReaders a chance !
	    if (doReset) {
		stream.reset(); // may throw an IOException
	    }
	    throw new IllegalArgumentException("unable to initialize soundbank");
	}
	
	setupInformation();
		
	if(Printer.trace) Printer.trace("HeadspaceSoundbank: initialize: stream: " + stream + " completed");


    }

    private void initialize(String path) {
	
	if(Printer.trace) Printer.trace("HeadspaceSoundbank: initialize: path: " + path);

	// $$kk: 04.12.99: this is sort of a hack.  we need to make sure the library
	// gets loaded and (potentially) that the mixer gets initialized.
	HeadspaceMixer mixer = HeadspaceMixer.getMixerInstance();


	if(Printer.debug) Printer.debug("> calling nOpenResource with path: " + path);
	id = nOpenResource(path);
	if(Printer.debug) Printer.debug("> returned from nOpenResource; id = " + id);

	if (id == 0) {
	    throw new IllegalArgumentException("unable to load soundbank");
	}

	setupInformation();

	if(Printer.trace) Printer.trace("HeadspaceSoundbank: initialize: path: " + path + " completed");
    }		


    // NATIVE METHODS

    private native long nOpenResource(String path);
    private native long nOpenResourceFromByteArray(byte[] soundbankData, int length);
	
    // $$kk: 04.11.99: we are never calling this, but we should!!
    private native boolean nCloseResource(long id);
	
    // $$kk: 04.12.99: this seems not to be working
    private native String nGetName(long id);

    private native int nGetVersionMajor(long id);
    private native int nGetVersionMinor(long id);
    private native int nGetVersionSubMinor(long id);

    private native void nGetInstruments(long id, Vector instruments);
    private native void nGetSamples(long id, Vector samples);

    //protected native void nGetSequences();

    // $$kk: 04.11.99: this is *not* the right way to accomplish
    // what we're doing here....  i am using this to set this
    // soundbank as the first resource file so that the following
    // calls operate on it....
    //private native boolean nSetCurrentResource(long id);
}
