/*
 * @(#)PerfHelper.java	1.3 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.perf;

import java.lang.String;
import com.sun.deploy.perf.PerfLabel;

public interface PerfHelper {
    /***************************************************************************
     * Set the world start time of all time
     *
     * If used, all followup time logging shall be relative to t0
     *
     * Optional extended time reference functionality
     *
     * @param  t0 world start time
     */
    public void setInitTime(long t0);

    /***************************************************************************
     * Puts a time stamped label entry into storage.
     *
     * @param  label  the label to store.
     */
    public void put(String label);

    /***************************************************************************
     * Puts a time stamped label entry into storage,
     * where the relative time stamp uses deltaStart as it's datum reference
     *
     * Optional extended time reference functionality
     *
     * @param  deltaStart  the start of the delta, which is to be printed
     * @param  label  the label to store.
     * @return the current time, relative to init time
     * @see setInitTime
     */
    public long put(long deltaStart, String label);

    /***************************************************************************
     * Gets an array containing all the labels that have been stored.
     *
     * @returns an array of stored labels.
     */
    public PerfLabel [] toArray();
}
