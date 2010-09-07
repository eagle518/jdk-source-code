/*
 * @(#)ConverterProgressListener.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

public interface ConverterProgressListener extends java.util.EventListener {

    public abstract void converterProgressUpdate(ConverterProgressEvent e);

}

