/*
 * %W% %E%
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package com.sun.deploy.security;

import java.net.PasswordAuthentication;

public abstract class AbstractBrowserAuthenticator implements BrowserAuthenticator {

    protected PasswordAuthentication getPAFromCharArray(char[] credential) {
        if(credential == null)
            return null;

        int index = 0;
        while(index < credential.length && ':' != credential[index])
            index ++;
                
        PasswordAuthentication pa = null;
        if(index < credential.length) {
            String userName = new String(credential, 0, index);
            char[] password = extractArray(credential, index + 1);
            pa = new PasswordAuthentication(userName, password);
            resetArray(password);
        }
        resetArray(credential);

        return pa;
    }

    private void resetArray(char[] arr) {
        java.util.Arrays.fill(arr, ' ');
    }


    private char[] extractArray(char[] src, int start) {
        char[] dest = new char[src.length - start];
        for(int index = 0; index < dest.length; index ++)
            dest[index] = src[index + start];

        return dest;
    }
}
