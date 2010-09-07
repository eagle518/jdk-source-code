/*
 * @(#)NativePlatform.java	1.3 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.config;

import java.lang.*;
import com.sun.deploy.util.Trace;

public class NativePlatform {
    String _osname;
    String _osarch;
    String _unifiedOsArch;
    boolean _is32Bit;
    boolean _is64Bit;

    public String getOSName() { return _osname; }
    public String getOSArch() { return _osarch; }
    public String getOsArchUnified() { return _unifiedOsArch; }

    public boolean is32Bit() { return _is32Bit; }
    public boolean is64Bit() { return _is64Bit; }

    private static NativePlatform _currentNativePlatform = null;

    public static NativePlatform getCurrentNativePlatform() {
        if(null==_currentNativePlatform) {
            _currentNativePlatform = new NativePlatform(null, null);
        }
        return _currentNativePlatform;
    }

    public NativePlatform (String osname, String osarch) {
        if (osname == null ) {
            osname = Config.getOSName();
        }
        if (osarch == null) {
            osarch = Config.getOSArch();
        }
        _osname = osname;
        _osarch = osarch;

        // unify osArch ..
        if( osarch.equals("x86") ||
            osarch.equals("i386") ||
            osarch.equals("i486") ||
            osarch.equals("i586") ||
            osarch.equals("i686")) {
            _is32Bit = true;
            _is64Bit = false;
            _unifiedOsArch = "x86";
         } else
         if( osarch.equals("ppc") ||
             osarch.equals("sparc") ||
             osarch.equals("pa_risc2.0")) {
           _is32Bit = true;
           _is64Bit = false;
           _unifiedOsArch = osarch;
         } else
         if( osarch.equals("x86_64") ||
             osarch.equals("amd64")) {
           _is32Bit = false;
           _is64Bit = true;
           _unifiedOsArch = "x86_64";
         } else
         if( osarch.equals("ia64") ||
             osarch.equals("sparcv9")) {
           _is32Bit = false;
           _is64Bit = true;
           _unifiedOsArch = osarch;
         } else {
           String msg = "JREInfo: unknown osArch: <"+osarch+">, considering 32bit";
           Trace.println(msg);
           System.out.println(msg);
           _is32Bit = true;
           _is64Bit = false;
           _unifiedOsArch = osarch;
         }
    }

    public boolean compatible(NativePlatform np) {
        return getOSName().equals(np.getOSName()) &&
               getOsArchUnified().equals(np.getOsArchUnified()) ;
    }

    public String toString() {
        String bitDepth;
        if(is32Bit()) bitDepth="32bit";
        else if(is64Bit()) bitDepth="64bit";
        else bitDepth  = "??bit";
        return _osname + ", " + _osarch + " [ "+_unifiedOsArch+", "+bitDepth+" ]";
    }
}

