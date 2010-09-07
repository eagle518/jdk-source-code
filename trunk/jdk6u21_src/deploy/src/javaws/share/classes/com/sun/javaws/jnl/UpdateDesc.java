/*
 * @(#)UpdateDesc.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.deploy.xml.*;
import com.sun.deploy.util.Trace;


/**
 *   Describes an Update element
 */
public class UpdateDesc implements XMLable {

    public final static int CHECK_ALWAYS = 0;
    public final static int CHECK_TIMEOUT = 1;
    public final static int CHECK_BACKGROUND = 2;

    public final static int POLICY_ALWAYS = 0;
    public final static int POLICY_PROMPT_UPDATE = 1;
    public final static int POLICY_PROMPT_RUN = 2;

    private int _check;
    private int _policy;

    
    public UpdateDesc(String check, String policy) {
        if (check.equalsIgnoreCase("always")) {
            _check = CHECK_ALWAYS;
        } else if (check.equalsIgnoreCase("background")) {
            _check = CHECK_BACKGROUND;
        } else {
            _check = CHECK_TIMEOUT;
        }
        if (policy.equalsIgnoreCase("prompt-run")) {
            _policy = POLICY_PROMPT_RUN;
        } else if (policy.equalsIgnoreCase("prompt-update")) {
            _policy = POLICY_PROMPT_UPDATE;
        } else {
            _policy = POLICY_ALWAYS;
        }
    }
             
    public int getCheck() { return _check; }

    /**
     * @return true if the update check is background
     */
    public boolean isBackgroundCheck() { return CHECK_BACKGROUND==_check; }

    /**
     * Check if the policy requires prompt users for update
     * @return true if the policy is prompt-update or prompt-run
     */
    public boolean isPromptPolicy() {
	return _policy != POLICY_ALWAYS;
    }

    public int getPolicy() { return _policy; }

    public String getCheckString() {
        if (_check == CHECK_ALWAYS) {
            return "always";
        } else if (_check == CHECK_TIMEOUT) {
            return "timeout";
        } else {
            return "background";
        }
    }

    public String getPolicyString() {
        if (_policy == POLICY_ALWAYS) {
            return "always";
        } else if (_policy == POLICY_PROMPT_UPDATE) {
            return "prompt-update";
        } else {
            return "prompt-run";
        }
    }

    
    /** Outputs as XML */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("check", getCheckString());
        ab.add("policy", getPolicyString());
        XMLNodeBuilder nb = new XMLNodeBuilder("update", ab.getAttributeList());
        return nb.getNode();
    }
}




