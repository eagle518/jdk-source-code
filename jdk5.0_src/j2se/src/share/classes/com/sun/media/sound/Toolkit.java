/*
 * @(#)Toolkit.java	1.15 04/07/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

/**
 * Common conversions etc.
 *
 * @version 1.15 04/07/09
 * @author Kara Kytle
 * @author Florian Bomers
 */
public class Toolkit {


    /**
     * Converts bytes from signed to unsigned.
     */
    static void getUnsigned8(byte[] b, int off, int len) {
	for (int i = off; i < (off+len); i++) {
	    // $$fb 2002-10-17: we don't need a lot of calculation...
	    //b[i] = (b[i] >= 0) ? (byte)(0x80 | b[i]) : (byte)(0x7F & b[i]);
	    b[i] += 128;
	}
    }


    /**
     * Swaps bytes.
     * @throws ArrayOutOfBoundsException if len is not a multiple of 2.
     */
    static void getByteSwapped(byte[] b, int off, int len) {

	byte tempByte;
	for (int i = off; i < (off+len); i+=2) {

	    tempByte = b[i];
	    b[i] = b[i+1];
	    b[i+1] = tempByte;
	}
    }


    /**
     * Linear to DB scale conversion.
     */
    static float linearToDB(float linear) {

	float dB = (float) (Math.log((double)((linear==0.0)?0.0001:linear))/Math.log(10.0) * 20.0);
	return dB;
    }


    /**
     * DB to linear scale conversion.
     */
    static float dBToLinear(float dB) {

	float linear = (float) Math.pow(10.0, dB/20.0);
	return linear;
    }

    /*
     * returns bytes aligned to a multiple of blocksize
     * the return value will be in the range of (bytes-blocksize+1) ... bytes
     */
    static long align(long bytes, int blockSize) {
	// prevent null pointers
	if (blockSize <= 1) {
	    return bytes;
	}
	return bytes - (bytes % blockSize);
    }

    static int align(int bytes, int blockSize) {
	// prevent null pointers
	if (blockSize <= 1) {
	    return bytes;
	}
	return bytes - (bytes % blockSize);
    }


    /*
     * gets the number of bytes needed to play the specified number of milliseconds
     */
    static long millis2bytes(AudioFormat format, long millis) {
	long result = (long) (millis * format.getFrameRate() / 1000.0f * format.getFrameSize());
	return align(result, format.getFrameSize());
    }

    /*
     * gets the time in milliseconds for the given number of bytes
     */
    static long bytes2millis(AudioFormat format, long bytes) {
	return (long) (bytes / format.getFrameRate() * 1000.0f / format.getFrameSize());
    }

    /*
     * gets the number of bytes needed to play the specified number of microseconds
     */
    static long micros2bytes(AudioFormat format, long micros) {
	long result = (long) (micros * format.getFrameRate() / 1000000.0f * format.getFrameSize());
	return align(result, format.getFrameSize());
    }

    /*
     * gets the time in microseconds for the given number of bytes
     */
    static long bytes2micros(AudioFormat format, long bytes) {
	return (long) (bytes / format.getFrameRate() * 1000000.0f / format.getFrameSize());
    }

    /*
     * gets the number of frames needed to play the specified number of microseconds
     */
    static long micros2frames(AudioFormat format, long micros) {
	return (long) (micros * format.getFrameRate() / 1000000.0f);
    }

    /*
     * gets the time in microseconds for the given number of frames
     */
    static long frames2micros(AudioFormat format, long frames) {
	return (long) (((double) frames) / format.getFrameRate() * 1000000.0d);
    }

    static void isFullySpecifiedAudioFormat(AudioFormat format) {
	if (!format.getEncoding().equals(AudioFormat.Encoding.PCM_SIGNED)
	    && !format.getEncoding().equals(AudioFormat.Encoding.PCM_UNSIGNED)
	    && !format.getEncoding().equals(AudioFormat.Encoding.ULAW)
	    && !format.getEncoding().equals(AudioFormat.Encoding.ALAW)) {
	    // we don't know how to verify possibly non-linear encodings
	    return;
	}
	if (format.getFrameRate() <= 0) {
	    throw new IllegalArgumentException("invalid frame rate: "
	                                       +((format.getFrameRate()==-1)?
	                                         "NOT_SPECIFIED":String.valueOf(format.getFrameRate())));
	}
	if (format.getSampleRate() <= 0) {
	    throw new IllegalArgumentException("invalid sample rate: "
	                                       +((format.getSampleRate()==-1)?
	                                         "NOT_SPECIFIED":String.valueOf(format.getSampleRate())));
	}
	if (format.getSampleSizeInBits() <= 0) {
	    throw new IllegalArgumentException("invalid sample size in bits: "
	                                       +((format.getSampleSizeInBits()==-1)?
	                                         "NOT_SPECIFIED":String.valueOf(format.getSampleSizeInBits())));
	}
	if (format.getFrameSize() <= 0) {
	    throw new IllegalArgumentException("invalid frame size: "
	                                       +((format.getFrameSize()==-1)?
	                                         "NOT_SPECIFIED":String.valueOf(format.getFrameSize())));
	}
	if (format.getChannels() <= 0) {
	    throw new IllegalArgumentException("invalid number of channels: "
	                                       +((format.getChannels()==-1)?
	                                         "NOT_SPECIFIED":String.valueOf(format.getChannels())));
	}
    }


    static boolean isFullySpecifiedPCMFormat(AudioFormat format) {
	if (!format.getEncoding().equals(AudioFormat.Encoding.PCM_SIGNED)
	    && !format.getEncoding().equals(AudioFormat.Encoding.PCM_UNSIGNED)) {
	    return false;
	}
	if ((format.getFrameRate() <= 0)
	    || (format.getSampleRate() <= 0)
	    || (format.getSampleSizeInBits() <= 0)
	    || (format.getFrameSize() <= 0)
	    || (format.getChannels() <= 0)) {
	    return false;
	}
	return true;
    }


    public static AudioInputStream getPCMConvertedAudioInputStream(AudioInputStream ais) {
	// we can't open the device for non-PCM playback, so we have
	// convert any other encodings to PCM here (at least we try!)
	AudioFormat af = ais.getFormat();

	if( (!af.getEncoding().equals(AudioFormat.Encoding.PCM_SIGNED)) &&
	    (!af.getEncoding().equals(AudioFormat.Encoding.PCM_UNSIGNED))) {

	    try {
	    	AudioFormat newFormat =
		    new AudioFormat( AudioFormat.Encoding.PCM_SIGNED,
				     af.getSampleRate(),
				     16,
				     af.getChannels(),
				     af.getChannels() * 2,
				     af.getSampleRate(),
				     Platform.isBigEndian());
		ais = AudioSystem.getAudioInputStream(newFormat, ais);
	    } catch (Exception e) {
		if (Printer.err) e.printStackTrace();
		ais = null;
	    }
	}

	return ais;
    }

}
