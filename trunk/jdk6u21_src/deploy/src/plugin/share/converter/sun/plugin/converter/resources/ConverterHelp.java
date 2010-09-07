/*
 * @(#)ConverterHelp.java	1.5.0 03/07/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * US English version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Java(TM) Plug-in HTML Converter Readme" + newline + newline +
      "Version:  " + j2seVersion + newline + newline + newline +
      "*****   BACKUP ALL FILES BEFORE CONVERTING THEM WITH THIS TOOL." + newline +
      "*****   CANCELLING A CONVERSION WILL NOT ROLLBACK THE CHANGES." + newline +
      "*****   COMMENTS WITHIN THE APPLET TAG ARE IGNORED." + newline + newline + newline +
      "Contents:" + newline +
      "   1.  New Features" + newline +
      "   2.  Bug Fixes" + newline +
      "   3.  About Java(TM) Plug-in HTML Converter" + newline +
      "   4.  The conversion process" + newline +
      "   5.  Choosing files within folders to convert" + newline +
      "   6.  Choosing backup folder" + newline +
      "   7.  Generating a log file" + newline +
      "   8.  Choosing a conversion template" + newline +
      "   9.  Converting" + newline +
      "  10.  More Conversions or Quit" + newline +
      "  11.  Details about templates" + newline +
      "  12.  Running HTML Converter (Windows & Solaris)" + newline + newline +
      "1)  New Features:" + newline + newline +
      "    o Updated extended templates to support Netscape 6." + newline +
      "    o Updated all templates to support new Multi-versioning features in Java Plug-in." + newline +
      "    o Enhanced user interface with Swing 1.1 for i18n support." + newline +
      "    o Enhanced Advanced Option dialog to support new SmartUpdate and" + newline +
      "      MimeType template tags." + newline +
      "    o Enhanced HTML Converter to be used with both Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x, Java Plug-in 1.4.x" + newline +
      "      and Java Plugin-in 1.5.x." + newline +
      "    o Enhanced SmartUpdate and MimeType support in all conversion" + newline +
      "      templates." + newline +
      "    o Added \"scriptable=false\" to the OBJECT/EMBED tag in all templates." + newline + newline +
      "     This is used for disable typelib generation when Java" + newline +
      "    Plug-in is not used for scripting." + newline + newline + newline +
      "2)  Bugs Fixed:" + newline + newline +
      "    o Enhanced error handling when properties files are not found." + newline +
      "    o Enhanced HTML conversion so the result EMBED/OBJECT tag can be" + newline +
      "      used in AppletViewer in JDK 1.2.x." + newline +
      "    o Eliminated unnecessary files which were left over from HTML Converter 1.1.x." + newline +
      "    o Generated EMBED/OBJECT with CODE, CODEBASE, etc attribute names" + newline +
      "      instead of JAVA_CODE, JAVA_CODEBASE, etc. This enabled" + newline +
      "      generated page to be used in JDK 1.2.x AppletViewer." + newline +
      "    o Support MAYSCRIPT conversion if it is presented in the" + newline +
      "      APPLET tag." + newline + newline +
      "3)  About Java(TM) Plug-in HTML Converter:" + newline + newline +
      "        Java(TM) Plug-in HTML Converter is a utility that allows you to convert any" + newline +
      "        HTML page which contains applets to a format that will use the Java(TM)" + newline +
      "        Plug-in." + newline + newline +
      "4)  The conversion process:" + newline + newline +
      "        The Java(TM) Plug-in HTML Converter will convert any file(s) containing" + newline +
      "        applets to a form that can be used with the Java(TM) Plug-in." + newline + newline +
      "        The process of converting each file is as follows:" + newline +
      "        First, HTML that is not part of an applet is transferred from the source" + newline +
      "        file to a temporary file.  When an <APPLET tag is reached, the converter" + newline +
      "        will parse the applet to the first </APPLET tag (not contained in qoutes)," + newline +
      "        and merge the applet data with the template. (See Details about templates," + newline +
      "        below) If this completes without error, the original html file will be moved" + newline +
      "        to the backup folder and the temporary file will then be renamed to the" + newline +
      "        original file's name.  Thus, your original files will never be removed from disk." + newline + newline +
      "        Note that the converter will effectively convert the files in place.  So," + newline +
      "        once you have run the converter, your files will be setup to use the Java(TM) Plug-in." + newline +


      "5)  Choosing files within folders to convert:" + newline + newline +
      "       To convert all files within a folder, you may type in the path to the folder," + newline +
      "       or choose the browse button to select a folder from a dialog." + newline  +
      "       Once you have chosen a path, you may supply any number to file specifiers in the" + newline +
      "       \"Matching File Names\".  Each specifier must be separated by a comma.  You may use * as" + newline +
      "       a wildcard.  If you put a filename with wildcard, only that single file will" + newline +
      "       be converted. Lastly, select the checkbox \"Include Subfolders\", if you would" + newline +
      "       like all files in nested folders which match the file name to be converted." + newline + newline +
      "6)  Choosing backup folder:" + newline +

      "       The default backup folder path is the source path with an \"_BAK\" appended" + newline +
      "       to the name. i.e.  If the source path is c:/html/applet.html (converting one file)" + newline +
      "       then the backup path would be c:/html_BAK.  If the source path" + newline +
      "       is c:/html (converting all files in path) then the backup path would be" + newline +
      "       c:/html_BAK. The backup path may be changed by typing a path in the field" + newline +
      "       next to \"Backup files to folder:\", or by browsing for a folder." + newline + newline +

      "       Unix(Solaris):" + newline +
      "       The default backup folder path is the source path with an \"_BAK\" appended to" + newline +
      "       the name. i.e.  If the source path is /home/user1/html/applet.html (converting one file)" + newline +
      "       then the backup path would be /home/user1/html_BAK. If the source" + newline +
      "       path is /home/user1/html (converting all files in path) then the backup path" + newline +
      "       would be /home/user1/html_BAK. The backup path may be changed by typing" + newline +
      "       a path in the field next to \"Backup files to folder:\", or by browsing for a folder." + newline + newline +
      "7)  Generating a log file:" + newline + newline +
      "       If you would like a log file to be generated, select the checkbox" + newline +
      "       \"Generate Log File\". You can enter the path and filename or browse" + newline +
      "       to choose a folder, then type the filename and select open." + newline +
      "       The log file contains basic information related to the converting" + newline +
      "       process." + newline + newline +
      "8)  Choosing a conversion template:" + newline + newline +
      "       If a default template will be used if none is chosen.  This template will" + newline +
      "       produce converted html files that will work with IE and Netscape." + newline  +
      "       If you would like to use a different template, you may choose it from the menu on" + newline +
      "       the main screen.  If you choose other, you will be allowed to choose a file" + newline +
      "       that will be used as the template." + newline +
      "       If you choose a file, BE SURE THAT IT IS A TEMPLATE." + newline + newline +
      "9)  Converting:" + newline + newline +
      "       Click the \"Convert...\" button to begin the conversion process.  A process" + newline +
      "       dialog will show the files being processed, the number for files process," + newline +
      "       the number of applets found, and number of errors found." + newline + newline +
      "10) More Conversions or Quit:" + newline + newline +
      "       When the conversion is complete, the button in the process dialog will change" + newline +
      "       from \"Cancel\" to \"Done\".  You may choose \"Done\" to close the dialog." + newline  +
      "       At this point, choose \"Quit\" to close the Java(TM) Plug-in HTML Converter," + newline +
      "       or selected another set of files to convert and choose \"Convert...\" again." + newline + newline +
      "11)  Details about templates:" + newline + newline +
      "       The template file is the basis behind converting applets.  It is simply a text" + newline +
      "       file containing tag the represent parts of the original applet." + newline +
      "       By add/removing/moving the tags in a template file, you can alter the output" + newline +
      "       of the converted file." + newline + newline +
      "       Supported Tags:" + newline + newline +
      "        $OriginalApplet$    This tag is substituted with the complete text" + newline +
      "        of the original applet." + newline + newline +
      "        $AppletAttributes$   This tag is substituted with all of the" + newline +
      "        applets attributes. (code, codebase, width, height, etc.)" + newline + newline +
      "        $ObjectAttributes$   This tag is substituted with all the" + newline +
      "        attributes required by the object tag." + newline + newline +
      "        $EmbedAttributes$   This tag is substituted with all the attributes" + newline +
      "        required by the embed tag." + newline + newline +
      "        $AppletParams$    This tag is substituted with all the applet's" + newline +
      "        <param ...> tags" + newline + newline +
      "        $ObjectParams$    This tag is substituted with all the <param...>" + newline +
      "        tags required by the object tag." + newline + newline +
      "        $EmbedParams$     This tag is substituted with all the <param...>" + newline +
      "        tags required by the embed tag in the form  NAME=VALUE" + newline + newline +
      "        $AlternateHTML$  This tag is substituted with the text in the No" + newline +
      "        support for applets area of the original applet" + newline + newline +
      "        $CabFileLocation$   This is the URL of the cab file that should be" + newline +
      "        used in each template that targets IE." + newline + newline +
      "        $NSFileLocation$    This is the URL of the Netscape plugin that be" + newline +
      "        used in each template that targets Netscape" + newline + newline +
      "        $SmartUpdate$   This is the URL of the Netscape SmartUpdate" + newline +
      "        that be used in each template that targets Netscape Navigator 4.0 or later." + newline + newline +
      "        $MimeType$    This is the MIME type of the Java object" + newline + newline +
      "      default.tpl (the default template for the converter) -- the converted page can" + newline +
      "      be used in IE and Navigator on Windows to invoke Java(TM) Plug-in." + newline +
      "      This template can also be used with Netscape on Unix (Solaris)" + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- the converted page can be used to invoke Java(TM)" + newline +
      "      Plug-in in IE on Windows only." + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- the converted page can be used to invoke Java(TM)" + newline +
      "      Plug-in in Navigator on Windows and Solaris." + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- the converted page can be used in any browser and any platform." + newline +
      "      If the browser is IE or Navigator on Windows (Navigator on Solaris), Java(TM)" + newline +
      "      Plug-in will be invoked. Otherwise, the browser's default JVM is used." + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  Running HTML Converter:" + newline + newline +
      "      Running the GUI version of the HTML Converter" + newline + newline +
      "      The HTML Converter is contained in the JDK, not the JRE. To run the converter, go to the" + newline +
      "      lib sub directory of your JDK installation directory. For example," + newline +
      "      if you installed the JDK on Windows into C:\\jdk" + j2seVersion + ", then cd to" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\" + newline + newline +
      "      The converter (htmlconverter.jar) is  contained in that directory." + newline + newline +
      "      To launch the converter type:" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      Launching the converter on UNIX/Linux is analogous using the above commands." + newline +
      "      Below are some alternate ways of starting the converter" + newline + newline +
      "      On Windows" + newline +
      "      To launch Converter using explorer." + newline +
      "      Use explorer to navigate to the following directory." + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      Double click on the HtmlConverter application." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Run the following commands" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      Running the converter from the command line:" + newline + newline +
      "      Format:" + newline + newline +
      "      java -jar htmlconverter.jar [-options1 value1 [-option2 value2" + newline +
      "      [...]]] [-simulate] [filespecs]" + newline + newline +
      "      filespecs:  space delimited list of file specs, * wildcard. " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      Options:" + newline + newline +
      "       source:    Path to files.  (c:\\htmldocs in Windows," + newline +
      "                  /home/user1/htmldocs in Unix) Default: <userdir>" + newline +
      "                  If the path is relative, it is assumed to be relative to the" + newline +
      "                  directory that HTMLConverter was launched." + newline + newline +
      "       backup:    Path to write backup files.  Default:" + newline +
      "                  <userdir>/<source>_bak" + newline +
      "                  If the path is relative, it is assumed to be relative to the" + newline +
      "                  directory that HTMLConverter was launched." + newline + newline +
      "       subdirs:   Should files in subdirectories be processed. " + newline +
      "                  Default:  FALSE" + newline + newline +
      "       template:  Name of template file.  Default:  default.tpl-Standard " + newline +
      "                  (IE & Navigator) for Windows & Solaris Only. USE DEFAULT IF UNSURE." + newline + newline +
      "       log:       Path and filename to write log.  (Default <userdir>/convert.log)" + newline + newline +
      "       progress:  Display standard out progress while converting. " + newline +
      "                  Default: false" + newline + newline +
      "       simulate:  Display the specifics to the conversion without converting." + newline +
      "                  USE THIS OPTION IF UNSURE ABOUT CONVERTING." + newline +
      "                  YOU WILL BE GIVEN A LIST OF DETAILS SPECIFIC TO" + newline +
      "                  THE CONVERSION." + newline + newline +
      "      If only \"java -jar htmlconverter.jar -gui\" is specified (only -gui" + newline +
      "      option with no filespecs), the GUI version of the converter will be launched." + newline  +
      "      Otherwise, the GUI will be suppressed." + newline + newline +
      "      For more information see the following url:" + newline + newline +
      "      http://java.sun.com/j2se/" + 
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}
