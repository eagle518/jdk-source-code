HTML Converter


Contents:

    1. Introduction
    2. Running the GUI Version of the HTML Converter

       2.1 Choosing files within folders to convert
       2.2 Choosing a backup folder
       2.3 Generating a log file
       2.4 Choosing a conversion template
       2.5 Choosing the versioning type
       2.6 Converting
       2.7 Quit or convert more files
       2.8 Details about templates

    3. Running the converter from the command line 


Notes:

     o It is recommended that you use the same version HTML
       Converter and Java Plug-in. 
     o Backup all files before converting them with this tool. 
     o Canceling a conversion will not rollback the changes. 
     o Comments within the applet tag are ignored.
     o Additional Java Plug-in documentation provided on the website

         http://java.sun.com/products/plugin
     

1. Introduction

JavaTM Plug-in HTML Converter is a utility allowing you to convert any
HTML page (file) containing applets to a format that will use the
JavaTM Plug-in. The conversion process is as follows:
  
First, HTML that is not part of an applet is transferred from the
source file to a temporary file.  Then, when an <APPLET tag is
reached, the converter will parse the applet to the first </APPLET tag
(not contained in quotes) and merge the applet data with the
template. (See details about templates below.) If this completes
without error, the original html file is moved to the backup folder
and the temporary file is then renamed as the original file's name.
  
The converter effectively converts the files in place.  Thus, once you
have run the converter, your files are setup to use the Java Plug-in.


2. Running the GUI Version of the HTML Converter

2.1 Choosing files within folders to convert

To convert all files within a folder, you may type in the path to the
folder, or choose the browse button to select a folder from a dialog.
Once you have chosen a path, you may supply any number of file
specifiers in the "Matching File Names".  Each specifier must be
separated by a comma.  You may use * as a wildcard.  If you put a
filename with wildcard, only that single file will be
converted. Finally, select the checkbox "Include Subfolders", if you
would like all files in nested folders which match the file name to be
converted.
  

2.2 Choosing a backup folder: 

Microsoft Windows:

The default backup folder path is the source path with an "_BAK"
appended to the name; e.g., if the source path is c:/html/applet.html
(one file to be converted), then the backup path would be c:/html_BAK.
If the source path is c:/html (converting all files in the path), then
the backup path would be c:/html_BAK. The backup path may be changed
by typing a path in the field next to "Backup files to folder:", or by
browsing for a folder.

Unix:

The default backup folder path is the source path with an "_BAK"
appended to the name; e.g., if the source path is
/home/user1/html/applet.html (one file to be converted), then the
backup path would be /home/user1/html_BAK. If the source path is
/home/user1/html (converting all files in path) then the backup path
would be /home/user1/html_BAK. The backup path may be changed by
typing a path in the field next to "Backup files to folder:", or by
browsing for a folder.
  

2.3 Generating a log file

If you would like a log file to be generated, choose the checkbox
"Generate Log File". Type in a path or browse to choose a folder.  The
log file contains basic information related to the converting process.
  

2.4 Choosing a conversion template

A default template will be used if none is chosen.  This template will
produce converted html files that will work with IE and Netscape.  If
you would like to use a different template, you may choose it from the
menu on the main screen.  If you choose other, you will be allowed to
choose a file that will be used as the template.  If you choose a
file, be sure that it is a template.
  

2.5 Choosing the versioning type

Select the versioning type desired.  If you select the default option,
applets will only use this particular version of Java.  If not
installed, this version will be auto-downloaded if possible.
Otherwise, the user will be redirected to a manual download page.
Please refer to http://java.sun.com/products/plugin for details on the
auto-download process and End of Life (EOL) policies for all Java
releases.

If you select the dynamic versioning option, and if no such version is
installed, the current default download for the indicated Java release
family will be auto-downloaded, if possible.  Otherwise, the user will
be redirected to a manual download page.


2.6 Converting

Click the "Convert..." button to begin the conversion process.  A
dialog will show the files being processed, the number of files
processed, the number of applets found, and number of errors.
  

2.7 Quit or Convert More Files
  
When the conversion is complete, the button in the process dialog will
change from "Cancel" to "Done".  You may choose "Done" to close the
dialog.  At this point, choose "Quit" to close the JavaTM Plug-in HTML
Converter, or select another set of files to convert and choose
"Convert..."  again.


2.8 Details about templates

The template file is the basis behind converting applets.  It is
simply a text file containing tags that represent parts of the
original applet.  By adding/removing/moving the tags in a template
file, you can alter the output of the converted file.

Supported Tags: 

   $OriginalApplet$     This tag is substituted with the complete text
                        of the original applet. 

   $AppletAttributes$   This tag is substituted with all of the applets
                        attributes (code, codebase, width, height, etc.).

   $ObjectAttributes$   This tag is substituted with all the attributes
                        required by the object tag.

   $EmbedAttributes$    This tag is substituted with all the attributes
                        required by the embed tag.

   $AppletParams$       This tag is substituted with all the applet's
                        <param ...> tags

   $ObjectParams$       This tag is substituted with all the <param...>
                        tags required by the object tag.

   $EmbedParams$        This tag is substituted with all the <param...>
                        tags required by the embed tag in the form name=value 

   $AlternateHTML$      This tag is substituted with the text in the No
                        support for applets area of the original applet

   $CabFileLocation$    This is the URL of the cab file that should be
                        used in each template that targets IE.

   $NSFileLocation$     This is the URL of the Netscape plugin to be used
                        in each template that targets Netscape.

   $SmartUpdate$        This is the URL of the Netscape SmartUpdate to be
                        used in each template that targets Netscape
 		        Navigator 4.0 or later.

   $MimeType$           This is the MIME type of the Java object. 


default.tpl (the default template for the converter) -- the converted
page can be used in IE and Navigator on Microsoft Windows to invoke
JavaTM Plug-in. This template can also be used with Netscape on
Unix.

<!-- HTML CONVERTER -->
<object
    classid="$ClassId$"
    $ObjectAttributes$
    codebase="$CabFileLocation$">
    $ObjectParams$
    <param name="type" value="$MimeType$">
    <param name="scriptable" value="false">
$AppletParams$
    <comment>
	<embed
            type="$MimeType$" $EmbedAttributes$ $EmbedParams$ 
	    scriptable=false 
	    pluginspage="$NSFileLocation$">
	        <noembed>
		$AlternateHTML$
		</noembed>
	</embed>
    </comment>
</object>

<!--
$ORIGINALAPPLET$
-->


ieonly.tpl -- the converted page can be used to invoke JavaTM Plug-in
in IE on Microsoft Windows only.

<!-- HTML CONVERTER -->
<object
    classid="$ClassId$"
    $ObjectAttributes$
    codebase="$CabFileLocation$">
    $ObjectParams$
    <param name="type" value="$MimeType$">
    <param name="scriptable" value="false">
$AppletParams$
    $AlternateHTML$
</object>

<!--
$ORIGINALAPPLET$
-->


nsonly.tpl -- the converted page can be used to invoke JavaTM Plug-in
in Navigator on Microsoft Windows and the Unix operating
environment.

<!-- HTML CONVERTER -->
<embed
            type="$MimeType$" $EmbedAttributes$ $EmbedParams$
            scriptable=false
            pluginspage="$NSFileLocation$">
	        <noembed>
		$AlternateHTML$
		</noembed>
</embed>

<!--
$ORIGINALAPPLET$
-->


extend.tpl -- the converted page can be used in any browser and any
platform. If the browser is IE or Navigator on Microsoft Windows
(Navigator on the Unix operating environment), JavaTM Plug-in will
be invoked. Otherwise, the browser's default JVM is used.
  
<!-- HTML CONVERTER -->
<script language="JavaScript" type="text/javascript"><!--
    var _info = navigator.userAgent;
    var _ns = false;
    var _ns6 = false;
    var _ie = (_info.indexOf("MSIE") > 0 && _info.indexOf("Win") > 0 && _info.indexOf("Windows 3.1") < 0);
//--></script>
    <comment>
        <script language="JavaScript" type="text/javascript"><!--
        var _ns = (navigator.appName.indexOf("Netscape") >= 0 && ((_info.indexOf("Win") > 0 && _info.indexOf("Win16") < 0 && java.lang.System.getProperty("os.version").indexOf("3.5") < 0) || (_info.indexOf("Sun") > 0) || (_info.indexOf("Linux") > 0) || (_info.indexOf("AIX") > 0) || (_info.indexOf("OS/2") > 0) || (_info.indexOf("IRIX") > 0)));
        var _ns6 = ((_ns == true) && (_info.indexOf("Mozilla/5") >= 0));
//--></script>
    </comment>

<script language="JavaScript" type="text/javascript"><!--
    if (_ie == true) document.writeln('<object classid="$ClassId$" $ObjectAttributes$ codebase="$CabFileLocation$"><noembed><xmp>');
    else if (_ns == true && _ns6 == false) document.writeln('<embed ' +
	    'type="$MimeType$"$EmbedAttributes$$EmbedParams$ ' +
	    'scriptable=false ' +
	    'pluginspage="$NSFileLocation$"><noembed><xmp>');
//--></script>
<applet $AppletAttributes$></xmp>
    $ObjectParams$
    <param name="type" value="$MimeType$">
    <param name="scriptable" value="false">
$AppletParams$
$AlternateHTML$
</applet>
</noembed>
</embed>
</object>

<!--
$ORIGINALAPPLET$
-->


3. Running the converter from the command line

Usage: HtmlConverter [-option1 value1 [-option2 value2 [...]]] [-simulate]  [filespecs]

where options include:

    -source:    Path to get original files.  Default: <userdir>
    -dest:      Path to write converted files.  Default: <userdir>
    -backup:    Path to write backup files.  Default: <dirname>_BAK
    -f:         Force overwrite backup files.
    -subdirs:   Should files in subdirectories be processed.
    -template:  Path to template file.  Use default if unsure.
    -log:       Path to write log.  If not provided, no log is written.
    -progress:  Display progress while converting.  Default: false
    -simulate:  Display the specifics to the conversion without converting.
    -latest:    Use the latest JRE supporting the release mimetype.
    -gui:       Display the graphical user interface for the converter.

    filespecs:  Space delimited list of files specs.  Default: "*.html *.htm" (quotes required)
