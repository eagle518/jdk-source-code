/*
 * @(#)JNLP2Tag.java	1.5 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.util.*;

import com.sun.javaws.jnl.AppletDesc;

/**
 * Helper to manage JNLP tags of the applet tag within an HTML file
 *
 */
public final class JNLP2Tag {

    public static String JNLP_HREF = "jnlp_href" ;

    public static Map/*<String, String>*/ toStringMap(Map/*<String, String>*/ params, Properties props, boolean overwrite) {

        for (Enumeration e = props.propertyNames(); e.hasMoreElements() ;) {
              String name  = (String) e.nextElement();
              String value = (String) props.getProperty(name);
              if (name != null && value != null) {
                    name = name.toLowerCase();
                    if(!overwrite && params.get(name)!=null) 
                        continue;
                    params.put(name, value);
              }
        }
        return params;
    }

    
    /**
     * add the JNLP Descriptor Parameter to the parameter map,
     * following the specification.
     *
     * The parameter map (params) contains the applet tags already.
     * JNLP parameter shall not overwrite applet tag parameter
     */
    public static Map/*<String, String>*/ addJNLParams2Map(Map/*<String, String>*/ params, AppletDesc ad) {
        /**
         * fill the remaining parameters from the jnlp description
         */
        if(params==null) params = new HashMap();
        
        params = JNLP2Tag.toStringMap(params, ad.getParameters(), false);

        if(params.get("code")==null)
            params.put("code", ad.getAppletClass()); // make it compatible with the Applet code
        if(params.get("width")==null)
            params.put("width", String.valueOf(ad.getWidth()));
        if(params.get("height")==null)
            params.put("height", String.valueOf(ad.getHeight()));

        return params;
    }
}

