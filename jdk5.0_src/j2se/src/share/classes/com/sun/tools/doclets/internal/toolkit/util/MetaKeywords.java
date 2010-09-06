/*
 * @(#)MetaKeywords.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.util;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.javadoc.*;
import java.util.*;

/**
 * Provides methods for creating an array of class, method and
 * field names to be included as meta keywords in the HTML header
 * of class pages.  These keywords improve search results
 * on browsers that look for keywords.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Doug Kramer
 */
public class MetaKeywords {
    
    private static MetaKeywords instance = null;

    /**
     * The global configuration information for this run.
     */
    private final Configuration configuration;

    /**
     * Constructor
     */
    private MetaKeywords(Configuration configuration) {
        this.configuration = configuration;
    }
    
    /**
     * Return an instance of MetaKeywords.  This class is a singleton.
     *
     * @param configuration the current configuration of the doclet.
     */
    public static MetaKeywords getInstance(Configuration configuration) {
        if (instance == null) {
            instance = new MetaKeywords(configuration);
        }
        return instance;
    }

    /**
     * Returns an array of strings where each element
     * is a class, method or field name.  This array is
     * used to create one meta keyword tag for each element.
     * Method parameter lists are converted to "()" and
     * overloads are combined.
     *
     * Constructors are not included because they have the same
     * name as the class, which is already included.
     * Nested class members are not included because their
     * definitions are on separate pages.
     */
    public String[] getMetaKeywords(ClassDoc classdoc) {
        ArrayList results = getClassKeyword(classdoc);

        // Add field and method keywords only if -keywords option is used
        if( configuration.keywords ) {
            results.addAll(getMemberKeywords(classdoc.fields()));
            results.addAll(getMemberKeywords(classdoc.methods()));
        }
        return (String[]) results.toArray(new String[]{});
    }

    /**
     * Get the current class for a meta tag keyword, as the first
     * and only element of an array list.
     */
    protected ArrayList getClassKeyword(ClassDoc classdoc) {
        String cltypelower = classdoc.isInterface() ? "interface" : "class";
        ArrayList metakeywords = new ArrayList(1);
        metakeywords.add(classdoc.qualifiedName() + " " + cltypelower);
        return metakeywords;
    }

    /**
     * Get members for meta tag keywords as an array,
     * where each member name is a string element of the array.
     * The parameter lists are not included in the keywords;
     * therefore all overloaded methods are combined.<br>
     * Example: getValue(Object) is returned in array as getValue()
     *
     * @param memberdocs  array of MemberDoc objects to be added to keywords
     */
    protected ArrayList getMemberKeywords(MemberDoc[] memberdocs) {
        ArrayList results = new ArrayList();
        String membername;
        for (int i=0; i < memberdocs.length; i++) {
            membername = memberdocs[i].name()
                             + (memberdocs[i].isMethod() ? "()" : "");
            if ( ! results.contains(membername) ) {
                results.add(membername);
            }
        }
        return results;
    }
}
