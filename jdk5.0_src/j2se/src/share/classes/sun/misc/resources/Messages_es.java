/*
 * @(#)Messages_es.java	1.4 03/12/19
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

public class Messages_es extends java.util.ListResourceBundle {

    /**
     * Returns the contents of this <code>ResourceBundle</code>.     
     * <p>   
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
	return contents;
    }
        
    private static final Object[][] contents = {
	{ "optpkg.versionerror", "ERROR: El formato del archivo JAR {0} pertenece a una versi\u00f3n no v\u00e1lida. Busque en la documentaci\u00f3n un formato de una versi\u00f3n compatible." },
	{ "optpkg.attributeerror", "ERROR: El atributo obligatorio JAR manifest {0} no est\u00e1 definido en el archivo JAR {1}." },
	{ "optpkg.attributeserror", "ERROR: Algunos atributos obligatorios JAR manifest no est\u00e1n definidos en el archivo JAR {0}." }
    };
    
}

