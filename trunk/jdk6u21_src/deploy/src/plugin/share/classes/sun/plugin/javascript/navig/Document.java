/*
 * @(#)Document.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Document object in the JavaScript Document Object Model
 * in Navigator 3.x.
 * </p>
 */
public class Document extends sun.plugin.javascript.navig.JSObject {


   /**
    * <p> Method table contains all method info in the Document object. </p>
    */
   private static HashMap methodTable = new HashMap();

   /**
    * <p> Field table contains all properties info in the Document object. </p>
    */
   private static HashMap fieldTable = new HashMap();

   static {

	// Initialize all method and field info in the Document object.
	//
	methodTable.put("clear",   Boolean.FALSE);
	methodTable.put("close",   Boolean.FALSE);
	methodTable.put("open",	   Boolean.FALSE);
	methodTable.put("write",   Boolean.FALSE);
	methodTable.put("writeln", Boolean.FALSE);

	fieldTable.put("alinkColor",	Boolean.TRUE);
	fieldTable.put("anchors",	Boolean.FALSE);
	fieldTable.put("applets",	Boolean.FALSE);
	fieldTable.put("bgColor",	Boolean.TRUE);
	fieldTable.put("cookie",	Boolean.TRUE);
	fieldTable.put("domain",	Boolean.TRUE);
	fieldTable.put("embeds",	Boolean.FALSE);
	fieldTable.put("fgColor",	Boolean.TRUE);
	fieldTable.put("forms",		Boolean.FALSE);
	fieldTable.put("images",	Boolean.FALSE);
	fieldTable.put("lastModified",	Boolean.FALSE);
	fieldTable.put("linkColor",	Boolean.TRUE);
	fieldTable.put("links",		Boolean.FALSE);
	fieldTable.put("location",	Boolean.TRUE);
	fieldTable.put("plugins",	Boolean.FALSE);
	fieldTable.put("referrer",	Boolean.FALSE);
	fieldTable.put("title",		Boolean.FALSE);
	fieldTable.put("URL",		Boolean.FALSE);
	fieldTable.put("vlinkColor",	Boolean.TRUE);
    }


    /**
     * <p> Construct a new Document object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    protected Document(int instance, String context)  {
	super(instance, context);

	// Setup object property and method table.
	//
	addObjectTable(fieldTable, methodTable);
    }

 
    /**
     * <p> Retrieves a named member of the Document object. Equivalent to 
     * "self.document.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException {

	if (name.equals("links"))  {
	    return resolveObject(JSType.LinkArray, context + ".links");
	}
	else if (name.equals("anchors")) {
	    return resolveObject(JSType.AnchorArray, context + ".anchors");
	}
	else if (name.equals("forms")) {
	    return resolveObject(JSType.FormArray, context + ".forms");
	}
	else if (name.equals("images")) {
	    return resolveObject(JSType.ImageArray, context + ".images");
	}
	else if (name.equals("applets")) {
	    return resolveObject(JSType.AppletArray, context + ".applets");
	}
	else if (name.equals("embeds")) {
	    return resolveObject(JSType.EmbedArray, context + ".embeds");
	}
	else if (name.equals("location")) {
	    return resolveObject(JSType.Location, context + ".location");
	}

        // Try to match the name of the property with the field table.
        //
        try {
                return super.getMember(name);
        }
        catch(JSException exp)
        {
                //Check whether it is a valid object- if so check is it of type "Form"
                //before resolveObject otherwise throw exception.

                String type = evalScript(instance, "javascript: typeof(" + context + "." + name + ")" );
                if(type != null && type.equalsIgnoreCase("object"))
                {
                        String objClass = evalScript(instance, "javascript:" + context + "." + name + ".constructor.name");
                        if(objClass.equalsIgnoreCase("Form") || objClass.equalsIgnoreCase("HTMLFormElement"))
                                return resolveObject(JSType.Form, context + "." + name);
                        else
                                throw exp;
                }
                else
                {
                        throw exp;
                }
        }

    }
}
