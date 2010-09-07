/*
 * @(#)DocBeanInfo.java	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import java.util.HashMap;

/** 
 * Class that holds information for populating a FeatureDescriptor. For the class,
 * This information represents the BeanDescriptor, for a property, it represents
 * a PropertyDescriptor.
 */
public class DocBeanInfo {

    // Values of the BeanFlags
    public static final int BOUND = 1;
    public static final int EXPERT = 2;
    public static final int CONSTRAINED = 4;
    public static final int HIDDEN = 8;
    public static final int PREFERRED = 16 ;
    
    public String name;         
    public int beanflags;
    public String desc;
    public String displayname;
    public String propertyeditorclass;
    public String customizerclass;

    public HashMap attribs;
    public HashMap enums;

    public DocBeanInfo(){}

    public DocBeanInfo(String p, int flags, String d, 
                         String displayname, String pec, String cc,
                         HashMap attribs, HashMap enums) {
        this.name = p;
        this.beanflags = flags;
        this.desc = d;
        this.displayname = displayname;
        this.propertyeditorclass = pec;
        this.customizerclass = cc;

        this.attribs = attribs;
        this.enums = enums;
    }
    
    public String toString() {
        StringBuffer buffer = new StringBuffer("*****");
        buffer.append("\nProperty: " + name);
        buffer.append("\tDescription: " + desc);
        buffer.append("\nDisplayname: " + displayname);
        buffer.append("\nPropertyEditorClass: " + propertyeditorclass);
        buffer.append("\nCustomizerClass: " + customizerclass);
        
        if ((beanflags & BOUND) != 0)
            buffer.append("\nBound: true");
        
        if ((beanflags & EXPERT) != 0)
            buffer.append("\nExpert: true");
        
        if ((beanflags & CONSTRAINED) != 0)
            buffer.append("\nConstrained: true");

        if ((beanflags & HIDDEN) !=0)
            buffer.append("\nHidden:  true");

        if ((beanflags & PREFERRED) !=0)
        
        if (attribs != null)
            buffer.append(attribs.toString());
            
        if (enums != null)
            buffer.append(enums.toString());
        
        return buffer.toString();
    }
      
}
