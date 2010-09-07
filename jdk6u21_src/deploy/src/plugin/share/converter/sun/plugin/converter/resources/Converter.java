/*
 * @(#)Converter.java	1.52 10/04/22
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Java Plug-in HTML Converter strings.
 *
 * @author Stanley Man-Kit Ho
 */

public class Converter extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "Error" },
	{ "caption.warning", "Warning" },
	{ "caption.absdirnotfound", "Absolute Directory not Found" },
	{ "caption.reldirnotfound", "Relative Directory not Found" },
        { "about_dialog.info", "Java(TM) Plug-in HTML Converter v{0}" + newline + 
                               "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "About Java(TM) Plug-in HTML Converter" },
	{ "nottemplatefile_dialog.caption", "Not a Template File"},
	{ "nottemplatefile_dialog.info0", "The specified template file " + newline +
                                          " {0} " + newline + 
					  "is not a valid template file.  The file must end" + newline +
					  "with the extention .tpl" + newline + newline +
                                          "Resetting template file to the default."},
	{ "warning_dialog.info", "The backup folder and the destination folder cannot " + newline +
	                         "have the same path.  Would you like the backup" + newline +
	                         "folder path changed to the following: " + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "Template File Not Found"},
        { "notemplate_dialog.info", "The default template file ({0})" + newline +
                                    "could not be found.  It is either not in the classpath" + newline +
                                    "or it is not in the working directory."},
        { "file_unwritable.info", "File is not writable: "},
	{ "file_notexists.info", "File does not exist: "},
	{ "illegal_source_and_backup.info", "Destination and backup directories cannot be the same!"},
	{ "button.reset", "Reset to Defaults"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "OK"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "Cancel"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.done", "Done"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "button.browse.dir", "Browse..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_B)},
        { "button.browse.backup", "Browse..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.convert", "Convert..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_C)},

	{ "advanced_dialog.caption", "Advanced Options"},
	{ "advanced_dialog.cab", "Specify Source Location for ActiveX CAB File:"},
	{ "advanced_dialog.plugin", "Specify Source Location for Netscape Plug-In:"},
	{ "advanced_dialog.smartupdate", "Specify Source Location for Netscape SmartUpdate:"},
	{ "advanced_dialog.mimetype", "Specify MIME type for Java Plug-In HTML Conversion:"},
	{ "advanced_dialog.log", "Specify Location for Log File:"},
	{ "advanced_dialog.generate", "Generate Log File"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "Progress..."},
	{ "progress_dialog.processing", "Processing..."},
	{ "progress_dialog.folder", "Folder:"},
	{ "progress_dialog.file", "File:"},
	{ "progress_dialog.totalfile", "Total Files Processed:"},
	{ "progress_dialog.totalapplet", "Total Applets Found:"},
	{ "progress_dialog.totalerror", "Total Errors:"},

	{ "notdirectory_dialog.caption0", "Not a Valid File"},
	{ "notdirectory_dialog.caption1", "Not a Valid Folder"},
        { "notdirectory_dialog.info0", "The following folder does not exist" + newline + "{0}"},
        { "notdirectory_dialog.info1", "The following file does not exist" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "The following folder does not exist " + newline + "<empty>"},
        
	{ "converter_gui.lablel0", "Specify a file or a directory path:"},
	{ "converter_gui.lablel1", "Matching File Names:"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "Include Subfolders"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "One File:"},
	{ "converter_gui.lablel5", "Backup Files to Folder:"},
	{ "converter_gui.lablel7", "Template File:"},


	{ "template.default", "Standard (IE & Navigator) for Windows & Solaris Only"},
	{ "template.extend",  "Extended (Standard + All Browsers/Platforms)"},
	{ "template.ieonly",  "Internet Explorer for Windows & Solaris Only"},
	{ "template.nsonly",  "Navigator for Windows Only"},
	{ "template.other",   "Other Template..."},

        { "template_dialog.title", "Select File"},
	
        { "help_dialog.caption", "Help"},
        { "help_dialog.error", "Could not access help file"},

	{ "menu.file", "File"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "menu.exit", "Exit"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_X)},
	{ "menu.edit", "Edit"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.option", "Options"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "Help"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "menu.about", "About"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_A)},

        { "static.versioning.label", "Java Versioning for Applets:"},
        { "static.versioning.radio.button", "Use only JRE version {0}"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "Applets will only use this particular version of the JRE.  If not installed, this version will be auto-downloaded if possible.  Otherwise, the user will be redirected to a manual download page.  Please refer to http://java.sun.com/products/plugin for details on the auto-download process and End of Life (EOL) policies for all Java releases."},
        { "dynamic.versioning.radio.button", "Use any JRE version {0}, or higher"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "If no such version is installed, the current default download for the JRE version {0} family will be auto-downloaded, if possible.  Otherwise, the user will be redirected to a manual download page."},
        
	{ "progress_event.preparing", "Preparing"},
	{ "progress_event.converting", "Converting"},
	{ "progress_event.copying", "Copying"},
	{ "progress_event.done", "Done"},
	{ "progress_event.destdirnotcreated", "Could not create destination directory."},
	{ "progress_event.error", "Error"},
	
	{ "plugin_converter.logerror", "Log file output could not be established"},
	{ "plugin_converter.saveerror", "Could not save properties file:  "},
	{ "plugin_converter.appletconv", "Applet Conversion "},
	{ "plugin_converter.failure", "Unable to convert the file "},
	{ "plugin_converter.overwrite1", "A backup copy already exists for..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "Do you want to overwrite this backup copy?"},
	{ "plugin_converter.done", "All Done  Files Processed:  "},
	{ "plugin_converter.appletfound", "  Applets Found:  "},
	{ "plugin_converter.processing", "  Processing..."},
	{ "plugin_converter.cancel", "Conversion Cancelled"},
	{ "plugin_converter.files", "Files to be converted: "},
	{ "plugin_converter.converted", "File previously converted, no conversion is necessary. "},
	{ "plugin_converter.donefound", "Done  Applets Found:  "},
	{ "plugin_converter.seetrace", "Error on file - see trace below"},
	{ "plugin_converter.noapplet", "No applets in file "},
	{ "plugin_converter.nofiles", "No files to be processed "},
	{ "plugin_converter.nobackuppath", "Didn't create the backup path"},
	{ "plugin_converter.writelog", "Writing over log file with same name"},
	{ "plugin_converter.backup_path", "Backup Path"},
	{ "plugin_converter.log_path", "Log Path"},
	{ "plugin_converter.template_file", "Template File"},
	{ "plugin_converter.process_subdirs", "Process Subdirectories"},
	{ "plugin_converter.show_progress", "Show progress"},
	{ "plugin_converter.write_permission", "Need to have the write permission in the current work directory"},
	{ "plugin_converter.overwrite", "The temporary file .tmpSource_stdin already exists. Please delete or rename it."},
	{ "plugin_converter.help_message", newline + 
	                                  "Usage: HtmlConverter [-option1 value1 [-option2 value2 [...]]] [-simulate]  [filespecs]" + newline + newline +
	                                  "where options include:" + newline + newline +
	                                  "    -source:    Path to get original files.  Default: <userdir>" + newline +
                                          "    -source -:  read converting file from the standard input" + newline + 
	                                  "    -dest:      Path to write converted files.  Default: <userdir>" + newline + 
                                          "    -dest -:    write converted file to the standard output" + newline +
	                                  "    -backup:    Path to write backup files.  Default: <dirname>_BAK" + newline +
                                          "    -f:         Force overwrite backup files." + newline +
	                                  "    -subdirs:   Should files in subdirectories be processed." + newline +
	                                  "    -template:  Path to template file.  Use default if unsure." + newline +
	                                  "    -log:       Path to write log.  If not provided, no log is written." + newline +
	                                  "    -progress:  Display progress while converting.  Default: false" + newline +
	                                  "    -simulate:  Display the specifics to the conversion without converting." + newline + 
	                                  "    -latest:    Use the latest JRE supporting the release mimetype." + newline + 
                                          "    -gui:       Display the graphical user interface for the converter." + newline + newline +
	                                  "    filespecs:  Space delimited list of files specs.  Default: \"*.html *.htm\" (quotes required)" + newline},
	
	{ "product_name", "Java(TM) Plug-in HTML Converter" },
    };
}

