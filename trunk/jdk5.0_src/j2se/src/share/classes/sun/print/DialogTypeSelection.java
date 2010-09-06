/*
 * 1.2 @(#)DialogTypeSelection.java	1.2
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import javax.print.attribute.EnumSyntax;
import javax.print.attribute.PrintRequestAttribute;

/**
 * Class DialogTypeSelection is a printing attribute class, an enumeration, 
 * that indicates the dialog to be used for PrintService selection.
 * If NATIVE is specified, the native print dialog is displayed.
 * If COMMON is specified, a cross-platform print dialog is displayed.
 *
 * <P>
 * <B>IPP Compatibility:</B> This is not an IPP attribute.
 * <P>
 *
 */
public final class DialogTypeSelection extends EnumSyntax 
	implements PrintRequestAttribute {

    /**
     * 
     */
    public static final DialogTypeSelection 
	NATIVE = new DialogTypeSelection(0);	
	    
    /**
     * 
     */
    public static final DialogTypeSelection 
	COMMON = new DialogTypeSelection(1);
    
    /**
     * Construct a new dialog type selection enumeration value with the 
     * given integer value. 
     *
     * @param  value  Integer value.
     */
    protected DialogTypeSelection(int value) {
		super(value);
    }

    private static final String[] myStringTable = {
	"native", "common"};


    private static final DialogTypeSelection[] myEnumValueTable = {
	NATIVE,
	COMMON
    };

    /**
     * Returns the string table for class DialogTypeSelection.
     */
    protected String[] getStringTable() {
	return myStringTable;
    }

    /**
     * Returns the enumeration value table for class DialogTypeSelection.
     */
    protected EnumSyntax[] getEnumValueTable() {
	return myEnumValueTable;
    }


   /**
     * Get the printing attribute class which is to be used as the "category" 
     * for this printing attribute value.
     * <P>
     * For class DialogTypeSelection the category is class 
     * DialogTypeSelection itself. 
     *
     * @return  Printing attribute class (category), an instance of class
     *          {@link java.lang.Class java.lang.Class}.
     */
    public final Class getCategory() {
        return DialogTypeSelection.class;
    }
   

    /**
     * Get the name of the category of which this attribute value is an 
     * instance. 
     * <P>
     * For class DialogTypeSelection the category name is
     * <CODE>"dialog-type-selection"</CODE>. 
     *
     * @return  Attribute category name.
     */
    public final String getName() {
        return "dialog-type-selection";
    }

}
