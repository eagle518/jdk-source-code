/*
 * @(#)CustomMediaTray.java	1.3     03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import javax.print.attribute.EnumSyntax;
import javax.print.attribute.standard.MediaTray;
import javax.print.attribute.standard.Media;
import java.util.ArrayList;

class CustomMediaTray extends MediaTray {
    private static ArrayList customStringTable = new ArrayList();
    private static ArrayList customEnumTable = new ArrayList();
    private String choiceName;

    private CustomMediaTray(int x) {
	super(x);
	
    }

    private synchronized static int nextValue(String name) {
      customStringTable.add(name);     
      return (customStringTable.size()-1);
    }
    

    public CustomMediaTray(String name, String choice) {
	super(nextValue(name));
	choiceName = choice;
	customEnumTable.add(this);
    }

    /**
     * Version ID for serialized form.
     */
    private static final long serialVersionUID = 1019451298193987013L;


    /**
     * Returns the command string for this media tray.
     */
    public String getChoiceName() {
	return choiceName;
    }


    /**
     * Returns the string table for super class MediaTray.
     */
    public Media[] getSuperEnumTable() {
      return (Media[])super.getEnumValueTable();
    }


    /**
     * Returns the string table for class CustomMediaTray.
     */
    protected String[] getStringTable() {     
      String[] nameTable = new String[customStringTable.size()];
      return (String[])customStringTable.toArray(nameTable);
    }
  
    /**
     * Returns the enumeration value table for class CustomMediaTray.
     */
    protected EnumSyntax[] getEnumValueTable() {     
      MediaTray[] enumTable = new MediaTray[customEnumTable.size()];
      return (MediaTray[])customEnumTable.toArray(enumTable);
    }
    
}



