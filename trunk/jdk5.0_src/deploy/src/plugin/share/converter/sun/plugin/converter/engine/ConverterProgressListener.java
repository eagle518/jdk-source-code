/*
 * @(#)ConverterProgressListener.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

public interface ConverterProgressListener extends java.util.EventListener {

    public abstract void converterProgressUpdate(ConverterProgressEvent e);

}

