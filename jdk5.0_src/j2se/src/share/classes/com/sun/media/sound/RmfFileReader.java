/*
 * @(#)RmfFileReader.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;	

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.DataInputStream;
import java.io.BufferedInputStream;
import java.net.URL;
import java.net.MalformedURLException;

import javax.sound.midi.MidiFileFormat;
import javax.sound.midi.Sequence;
import javax.sound.midi.InvalidMidiDataException;	
import javax.sound.midi.spi.MidiFileReader;



/**
 * Rmf file reader.
 *
 * @version 1.19, 03/12/19
 * @author Kara Kytle
 * @author Jan Borgersen
 */
public class RmfFileReader extends MidiFileReader {


    static  final int RMF_MAGIC          = 1230128474;

    private static final int MIDI_TYPE_1 = 1;
    private static final int bisBufferSize = 1024;

    /**
     * RMF types
     */
    public static final int types[] = {
	// $$jb: 06.04.99:  This is not completely accurate, but until we define
	// a nicer interface into RMF's, we essentially treat them as MIDI files
	// from which you can't obtain a sequence.

	MIDI_TYPE_1	
    };


    /**
     * Obtains the MIDI file format of the input stream provided.  The stream must
     * point to valid MIDI file data.  In general, MIDI file providers may 
     * need to read some data from the stream before determining whether they
     * support it.  These parsers must
     * be able to mark the stream, read enough data to determine whether they 
     * support the stream, and, if not, reset the stream's read pointer to its original 
     * position.  If the input stream does not support this, this method may fail
     * with an IOException. 
     * @param stream the input stream from which file format information should be
     * extracted
     * @return an <code>MidiFileFormat</code> object describing the MIDI file format
     * @throws InvalidMidiDataException if the stream does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     * @see InputStream#markSupported
     * @see InputStream#mark
     */
    public MidiFileFormat getMidiFileFormat(InputStream stream) throws InvalidMidiDataException, IOException {


	int maxReadLength = 16;
	int magic;
	int length = MidiFileFormat.UNKNOWN_LENGTH;
	float divisionType = (float) -1.0;
	int resolution = MidiFileFormat.UNKNOWN_LENGTH;
	int duration = MidiFileFormat.UNKNOWN_LENGTH;
	MidiFileFormat format = null;
	int type = types[0];

	DataInputStream dis = new DataInputStream( stream );

	dis.mark(maxReadLength);

	magic = dis.readInt();

	if( !(magic == RMF_MAGIC) ) {

	    // not RMF, throw exception
	    dis.reset();
	    throw new InvalidMidiDataException("not a valid RMF file");
			
	} 

	dis.reset();

	format = new MidiFileFormat( type, divisionType, resolution, length, duration );

	return format;
    }


    /**
     * Obtains the MIDI file format of the URL provided.  The URL must
     * point to valid MIDI file data.
     * @param url the URL from which file format information should be
     * extracted
     * @return an <code>MidiFileFormat</code> object describing the MIDI file format
     * @throws InvalidMidiDataException if the URL does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     */
    public MidiFileFormat getMidiFileFormat(URL url) throws InvalidMidiDataException, IOException {

	InputStream			urlStream = null;
	BufferedInputStream		bis = null;
	MidiFileFormat			fileFormat = null;

	urlStream = url.openStream();	// throws IOException

	bis = new BufferedInputStream( urlStream, bisBufferSize );
	try {
	    fileFormat = getMidiFileFormat( bis );		// throws InvalidMidiDataException
	} finally {
	    bis.close();
	}

	return fileFormat;
    }

    /**
     * Obtains the MIDI file format of the File provided.  The File must
     * point to valid MIDI file data.
     * @param file the File from which file format information should be
     * extracted
     * @return an <code>MidiFileFormat</code> object describing the MIDI file format
     * @throws InvalidMidiDataException if the File does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     */
    public MidiFileFormat getMidiFileFormat(File file) throws InvalidMidiDataException, IOException {

	FileInputStream			fis = null;
	BufferedInputStream		bis = null;
	MidiFileFormat			fileFormat = null;

	fis = new FileInputStream( file );	// throws IOException

	bis = new BufferedInputStream( fis, bisBufferSize );
	try {
	    fileFormat = getMidiFileFormat( bis );		// throws UnsupportedAudioFileException
	} finally {
	    bis.close();
	}

	return fileFormat;
    }

    /**
     * Obtains a MIDI sequence from the input stream provided.  The stream must
     * point to valid MIDI file data.  In general, MIDI file providers may 
     * need to read some data from the stream before determining whether they
     * support it.  These parsers must
     * be able to mark the stream, read enough data to determine whether they 
     * support the stream, and, if not, reset the stream's read pointer to its original 
     * position.  If the input stream does not support this, this method may fail
     * with an IOException. 
     * @param stream the input stream from which the <code>Sequence</code> should be
     * constructed
     * @return an <code>Sequence</code> object based on the MIDI file data contained
     * in the input stream.
     * @throws InvalidMidiDataException if the stream does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     * @see InputStream#markSupported
     * @see InputStream#mark
     */
    public Sequence getSequence(InputStream stream) throws InvalidMidiDataException, IOException {

	throw new InvalidMidiDataException("cannot get sequence from RMF file");

    }

    /**
     * Obtains a MIDI sequence from the URL provided.  The URL must
     * point to valid MIDI file data.
     * @param url the URL for which the <code>Sequence</code> should be
     * constructed
     * @return an <code>Sequence</code> object based on the MIDI file data pointed
     * to by the URL
     * @throws InvalidMidiDataException if the URL does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     */
    public  Sequence getSequence(URL url) throws InvalidMidiDataException, IOException {

	throw new InvalidMidiDataException("cannot get sequence from RMF file");

    }

    /**
     * Obtains a MIDI sequence from the File provided.  The File must
     * point to valid MIDI file data.
     * @param file the File for which the <code>Sequence</code> should be
     * constructed
     * @return an <code>Sequence</code> object based on the MIDI file data pointed
     * to by the File
     * @throws InvalidMidiDataException if the File does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     */
    public  Sequence getSequence(File file) throws InvalidMidiDataException, IOException {

	throw new InvalidMidiDataException("cannot get sequence from RMF file");

    }

}
