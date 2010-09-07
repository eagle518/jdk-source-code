/*
 * @(#)ConverterHelpTemplates.java	1.5.0 03/07/30
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

/**
 * Printable string versions of template files (*.tpl),
 * factored out for use by ConverterHelp[_xx[_yy]] classes.
 *
 * @author Carlos Lucasius
 */

public class ConverterHelpTemplates {

	private static String newline = System.getProperty("line.separator");

	static final String DEFAULT_TPL =
	"         <!-- HTML CONVERTER -->" + newline +
	"         <object" + newline +
	"             classid=\"$ClassId$\"" + newline +
	"             $ObjectAttributes$" + newline +
	"             codebase=\"$CabFileLocation$\">" + newline +
	"             $ObjectParams$" + newline +
	"             <param name=\"type\" value=\"$MimeType$\">" + newline +
	"             <param name=\"scriptable\" value=\"false\">" + newline +
	"         $AppletParams$" + newline +
	"             <comment>" + newline +
	"         	<embed" + newline +
	"                     type=\"$MimeType$\" $EmbedAttributes$ $EmbedParams$ " + newline +
	"         	    scriptable=false " + newline +
	"         	    pluginspage=\"$NSFileLocation$\">" + newline +
	"         	        <noembed>" + newline +
	"         		$AlternateHTML$" + newline +
	"         		</noembed>" + newline +
	"         	</embed>" + newline +
	"             </comment>" + newline +
	"         </object>" + newline + newline +
	"         <!--" + newline +
	"         $ORIGINALAPPLET$" + newline +
	"         -->";

	static final String IEONLY_TPL =
	"         <!-- HTML CONVERTER -->" + newline +
	"         <object" + newline +
	"             classid=\"$ClassId$\"" + newline +
	"             $ObjectAttributes$" + newline +
	"             codebase=\"$CabFileLocation$\">" + newline +
	"             $ObjectParams$" + newline +
	"             <param name=\"type\" value=\"$MimeType$\">" + newline +
	"             <param name=\"scriptable\" value=\"false\">" + newline +
	"         $AppletParams$" + newline +
	"             $AlternateHTML$" + newline +
	"         </object>" + newline + newline +
	"         <!--" + newline +
	"         $ORIGINALAPPLET$" + newline +
	"         -->";

	static final String NSONLY_TPL =
	"         <!-- HTML CONVERTER -->" + newline +
	"         <embed" + newline +
	"                     type=\"$MimeType$\" $EmbedAttributes$ $EmbedParams$" + newline +
	"                     scriptable=false" + newline +
	"                     pluginspage=\"$NSFileLocation$\">" + newline +
	"         	        <noembed>" + newline +
	"         		$AlternateHTML$" + newline +
	"         		</noembed>" + newline +
	"         </embed>" + newline + newline +
	"         <!--" + newline +
	"         $ORIGINALAPPLET$" + newline +
	"         -->";

	static final String EXTEND_TPL =
	"         <!-- HTML CONVERTER -->" + newline +
	"         <script language=\"JavaScript\" type=\"text/javascript\"><!--" + newline +
	"             var _info = navigator.userAgent;" + newline +
	"             var _ns = false;" + newline +
	"             var _ns6 = false;" + newline +
	"             var _ie = (_info.indexOf(\"MSIE\") > 0 && _info.indexOf(\"Win\") > 0 && _info.indexOf(\"Windows 3.1\") < 0);" + newline +
	"         //--></script>" + newline +
	"             <comment>" + newline +
	"                 <script language=\"JavaScript\" type=\"text/javascript\"><!--" + newline +
	"                 var _ns = (navigator.appName.indexOf(\"Netscape\") >= 0 && ((_info.indexOf(\"Win\") > 0 && _info.indexOf(\"Win16\") < 0 && java.lang.System.getProperty(\"os.version\").indexOf(\"3.5\") < 0) || (_info.indexOf(\"Sun\") > 0) || (_info.indexOf(\"Linux\") > 0) || (_info.indexOf(\"AIX\") > 0) || (_info.indexOf(\"OS/2\") > 0) || (_info.indexOf(\"IRIX\") > 0)));" + newline +
	"                 var _ns6 = ((_ns == true) && (_info.indexOf(\"Mozilla/5\") >= 0));" + newline +
	"         //--></script>" + newline +
	"             </comment>" + newline + newline +
	"         <script language=\"JavaScript\" type=\"text/javascript\"><!--" + newline +
	"             if (_ie == true) document.writeln('<object classid=\"$ClassId$\" $ObjectAttributes$ codebase=\"$CabFileLocation$\"><noembed><xmp>');" + newline +
	"             else if (_ns == true && _ns6 == false) document.writeln('<embed ' +" + newline +
	"         	    'type=\"$MimeType$\"$EmbedAttributes$$EmbedParams$ ' +" + newline +
	"         	    'scriptable=false ' +" + newline +
	"         	    'pluginspage=\"$NSFileLocation$\"><noembed><xmp>');" + newline +
	"         //--></script>" + newline +
	"         <applet $AppletAttributes$></xmp>" + newline +
	"             $ObjectParams$" + newline +
	"             <param name=\"type\" value=\"$MimeType$\">" + newline +
	"             <param name=\"scriptable\" value=\"false\">" + newline +
	"         $AppletParams$" + newline +
	"         $AlternateHTML$" + newline +
	"         </applet>" + newline +
	"         </noembed>" + newline +
	"         </embed>" + newline +
	"         </object>" + newline + newline +
	"         <!--" + newline +
	"         $ORIGINALAPPLET$" + newline +
	"         -->";
}
