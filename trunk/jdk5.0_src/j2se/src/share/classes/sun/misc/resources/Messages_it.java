/*
 * @(#)Messages_it.java	1.4 04/04/26
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc.resources;

/**
 * <p> This class represents the <code>ResourceBundle</code>
 * for sun.misc.
 * 
 * @author Michael Colburn
 * @version 1.00, 02/08/01
 */

public class Messages_it extends java.util.ListResourceBundle {

    /**
     * Returns the contents of this <code>ResourceBundle</code>.     
     * <p>   
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
	return contents;
    }
        
    private static final Object[][] contents = {
	{ "optpkg.versionerror", "ERRORE: Formato versione non valido nel file JAR {0}. Verificare nella documentazione il formato della versione supportato." },
	{ "optpkg.attributeerror", "ERRORE: L''attributo manifesto JAR {0} richiesto non \u00e8 impostato nel file JAR {1}." },
	{ "optpkg.attributeserror", "ERRORE: Alcuni attributi manifesti JAR {0} richiesti non sono impostati nel file JAR {1}." }
    };
    
}

