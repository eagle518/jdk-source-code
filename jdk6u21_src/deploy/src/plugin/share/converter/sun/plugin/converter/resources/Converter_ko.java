/*
 * @(#)Converter_ko.java	1.50 10/04/22
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

public class Converter_ko extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "\uc624\ub958" },
	{ "caption.warning", "\uacbd\uace0" },
	{ "caption.absdirnotfound", "\uc808\ub300 \ub514\ub809\ud1a0\ub9ac\ub97c \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "caption.reldirnotfound", "\uc0c1\ub300 \ub514\ub809\ud1a0\ub9ac\ub97c \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "about_dialog.info", "Java(TM) Plug-in HTML Converter v{0}" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "Java(TM) Plug-in HTML \ubcc0\ud658\uae30 \uc815\ubcf4" },
	{ "nottemplatefile_dialog.caption", "\ud15c\ud50c\ub9bf \ud30c\uc77c\uc774 \uc544\ub2d9\ub2c8\ub2e4."},
	{ "nottemplatefile_dialog.info0", "\uc9c0\uc815\ud55c \ud15c\ud50c\ub9bf \ud30c\uc77c" + newline + 
					" ({0})\uc740 " + newline +
					"\uc720\ud6a8\ud55c \ud15c\ud50c\ub9bf \ud30c\uc77c\uc774 \uc544\ub2d9\ub2c8\ub2e4.\ud30c\uc77c\uc740 " + newline +
					".tpl \ud655\uc7a5\uc790\ub85c \ub05d\ub098\uc57c \ud569\ub2c8\ub2e4." + newline + newline +
					"\ud15c\ud50c\ub9bf \ud30c\uc77c\uc744 \uae30\ubcf8\uc73c\ub85c \uc7ac\uc124\uc815"},
	{ "warning_dialog.info", "\ub300\uc0c1 \ud3f4\ub354\uc640 \ubc31\uc5c5 \ud3f4\ub354\uac00 \ub3d9\uc77c\ud55c \uacbd\ub85c\ub97c \uac00\uc9c8 \uc218" + newline +
				"\uc5c6\uc2b5\ub2c8\ub2e4. \ubc31\uc5c5 \ud3f4\ub354 \uacbd\ub85c\ub97c \ub2e4\uc74c\uacfc \uac19\uc774 " + newline +
				"\ubcc0\uacbd\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" + newline + 
				"{0}_BAK"},
	{ "notemplate_dialog.caption", "\ud15c\ud50c\ub9bf \ud30c\uc77c\uc744 \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4."},
        { "notemplate_dialog.info", "\uae30\ubcf8 \ud15c\ud50c\ub9bf \ud30c\uc77c({0})\uc744 \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." + newline + 
				     "\ud074\ub798\uc2a4 \uacbd\ub85c \ub610\ub294 \uc791\uc5c5 \ub514\ub809\ud1a0\ub9ac\uc5d0" + newline +
				     "\uc5c6\uc2b5\ub2c8\ub2e4."},
        { "file_unwritable.info", "\ud30c\uc77c\uc5d0 \uae30\ub85d\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. "},
	{ "file_notexists.info", "\ud30c\uc77c\uc774 \uc5c6\uc2b5\ub2c8\ub2e4."},
	{ "illegal_source_and_backup.info", "\ub300\uc0c1 \ub514\ub809\ud1a0\ub9ac\uc640 \ubc31\uc5c5 \ub514\ub809\ud1a0\ub9ac\ub294 \uac19\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4!"},
	{ "button.reset", "\uae30\ubcf8\uac12\uc73c\ub85c \uc7ac\uc124\uc815(R)"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "\ud655\uc778(O)"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "\ucde8\uc18c(C)"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.done", "\uc644\ub8cc(D)"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "button.browse.dir", "\ucc3e\uc544\ubcf4\uae30(B)..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_B)},
        { "button.browse.backup", "\ucc3e\uc544\ubcf4\uae30(R)..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.convert", "\ubcc0\ud658(C)..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_C)},

	{ "advanced_dialog.caption", "\uace0\uae09 \uc635\uc158"},
	{ "advanced_dialog.cab", "ActiveX CAB \ud30c\uc77c\uc758 \uc18c\uc2a4 \uc704\uce58\ub97c \uc9c0\uc815\ud569\ub2c8\ub2e4."},
	{ "advanced_dialog.plugin", "Netscape Plug-In\uc758 \uc18c\uc2a4 \uc704\uce58\ub97c \uc9c0\uc815\ud569\ub2c8\ub2e4."},
	{ "advanced_dialog.smartupdate", "Netscape SmartUpdate\uc6a9 \ud2b9\uc815 \uc18c\uc2a4 \uc704\uce58:"},
	{ "advanced_dialog.mimetype", "Java Plug-In HTML \ubcc0\ud658\uc758 MIME \uc720\ud615\uc744 \uc9c0\uc815\ud569\ub2c8\ub2e4."},
	{ "advanced_dialog.log", "\ub85c\uadf8 \ud30c\uc77c \uc704\uce58\ub97c \uc9c0\uc815\ud569\ub2c8\ub2e4:"},
	{ "advanced_dialog.generate", "\ub85c\uadf8 \ud30c\uc77c\uc744 \uc0dd\uc131\ud569\ub2c8\ub2e4(G)."},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "\uc9c4\ud589..."},
	{ "progress_dialog.processing", "\ucc98\ub9ac \uc911..."},
	{ "progress_dialog.folder", "\ud3f4\ub354:"},
	{ "progress_dialog.file", "\ud30c\uc77c:"},
	{ "progress_dialog.totalfile", "\ucc98\ub9ac\ub41c \uc804\uccb4 \ud30c\uc77c"},
	{ "progress_dialog.totalapplet", "\uac80\uc0c9\ub41c \uc804\uccb4 \uc560\ud50c\ub9bf:"},
	{ "progress_dialog.totalerror", "\uc804\uccb4 \uc624\ub958:"},

	{ "notdirectory_dialog.caption0", "\uc720\ud6a8\ud55c \ud30c\uc77c\uc774 \uc544\ub2d9\ub2c8\ub2e4."},
	{ "notdirectory_dialog.caption1", "\uc720\ud6a8\ud55c \ud3f4\ub354\uac00 \uc544\ub2d9\ub2c8\ub2e4."},
        { "notdirectory_dialog.info0", "\ub2e4\uc74c \ud3f4\ub354\uac00 \uc874\uc7ac\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." + newline + "{0}"},
        { "notdirectory_dialog.info1", "\ub2e4\uc74c \ud30c\uc77c\uc774 \uc874\uc7ac\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." + newline + "{0}"},
	{ "notdirectory_dialog.info5", "\ub2e4\uc74c \ud3f4\ub354\uac00 \uc874\uc7ac\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." + newline + "<\ube44\uc5b4 \uc788\uc74c>"},
        
	{ "converter_gui.lablel0", "\ud30c\uc77c \ub610\ub294 \ub514\ub809\ud1a0\ub9ac \uacbd\ub85c \uc9c0\uc815:"},
	{ "converter_gui.lablel1", "\uc77c\uce58\ud558\ub294 \ud30c\uc77c \uc774\ub984:"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "\ud558\uc704 \ud3f4\ub354 \ud3ec\ud568(I)"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "\ub2e8\ub3c5 \ud30c\uc77c:"},
	{ "converter_gui.lablel5", "\ud30c\uc77c\uc744 \ud3f4\ub354\ub85c \ubc31\uc5c5:"},
	{ "converter_gui.lablel7", "\ud15c\ud50c\ub9bf \ud30c\uc77c:"},


	{ "template.default", "Windows \ubc0f Solaris \uc804\uc6a9 \ud45c\uc900(IE & Navigator)"},
	{ "template.extend",  "\ud655\uc7a5(\ud45c\uc900 + \ubaa8\ub4e0 \ube0c\ub77c\uc6b0\uc800/\ud50c\ub7ab\ud3fc)"},
	{ "template.ieonly",  "Windows \ubc0f Solaris \uc804\uc6a9 Internet Explorer"},
	{ "template.nsonly",  "Windows \uc804\uc6a9 Navigator"},
	{ "template.other",   "\ub2e4\ub978 \ud15c\ud50c\ub9bf..."},

        { "template_dialog.title", "\ud30c\uc77c \uc120\ud0dd"},
	
        { "help_dialog.caption", "\ub3c4\uc6c0\ub9d0"},
        { "help_dialog.error", "\ub3c4\uc6c0\ub9d0 \ud30c\uc77c\uc5d0 \uc561\uc138\uc2a4\ud558\uc9c0 \ubabb\ud568"},

	{ "menu.file", "\ud30c\uc77c(F)"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "menu.exit", "\uc885\ub8cc(X)"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_X)},
	{ "menu.edit", "\ud3b8\uc9d1(E)"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.option", "\uc635\uc158(O)"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "\ub3c4\uc6c0\ub9d0(H)"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "menu.about", "\uc815\ubcf4(A)"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_A)},

        { "static.versioning.label", "\uc560\ud50c\ub9bf\uc5d0 Java \ubc84\uc804 \uc9c0\uc815:"},
        { "static.versioning.radio.button", " JRE {0}\ub9cc \uc0ac\uc6a9(U)"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "\uc560\ud50c\ub9bf\uc740 JRE\uc758 \uc774 \ud2b9\uc815 \ubc84\uc804\ub9cc\uc744 \uc0ac\uc6a9\ud558\uac8c \ub429\ub2c8\ub2e4. \uc774 \ubc84\uc804\uc774 \uc124\uce58\ub418\uc5b4 \uc788\uc9c0 \uc54a\uc73c\uba74 \uc790\ub3d9 \ub2e4\uc6b4\ub85c\ub4dc\ub420 \uac83\uc785\ub2c8\ub2e4. \uadf8\ub807\uc9c0 \uc54a\uc73c\uba74, \uc0ac\uc6a9\uc790\ub294 \uc218\ub3d9 \ub2e4\uc6b4\ub85c\ub4dc \ud398\uc774\uc9c0\ub85c \uc7ac\uc9c0\uc815\ub429\ub2c8\ub2e4. \ubaa8\ub4e0 Java \ub9b4\ub9ac\uc2a4\uc5d0 \ub300\ud55c EOL(End of Life) \uc815\ucc45\uacfc \uc790\ub3d9 \ub2e4\uc6b4\ub85c\ub4dc \ud504\ub85c\uc138\uc2a4\uc5d0 \ub300\ud55c \uc790\uc138\ud55c \uc0ac\ud56d\uc740 http://java.sun.com/products/plugin\uc744 \ucc38\uc870\ud558\uc2ed\uc2dc\uc624."},
        { "dynamic.versioning.radio.button", "\ubaa8\ub4e0 JRE {0}(\ub610\ub294 \uc774\uc0c1) \uc0ac\uc6a9(S)"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "\ud574\ub2f9 \ubc84\uc804\uc774 \uc124\uce58\ub418\uc9c0 \uc54a\uc73c\uba74 JRE {0} \uacc4\uc5f4\uad70\uc758 \ud604\uc7ac \uae30\ubcf8 \ub2e4\uc6b4\ub85c\ub4dc\uac00 \uc790\ub3d9\uc73c\ub85c \ub2e4\uc6b4\ub85c\ub4dc\ub420 \uac83\uc785\ub2c8\ub2e4. \uadf8\ub807\uc9c0 \uc54a\uc73c\uba74, \uc0ac\uc6a9\uc790\ub294 \uc218\ub3d9 \ub2e4\uc6b4\ub85c\ub4dc \ud398\uc774\uc9c0\ub85c \uc7ac\uc9c0\uc815\ub429\ub2c8\ub2e4."},
        
	{ "progress_event.preparing", "\uc900\ube44 \uc911"},
	{ "progress_event.converting", "\ubcc0\ud658 \uc911"},
	{ "progress_event.copying", "\ubcf5\uc0ac \uc911"},
	{ "progress_event.done", "\uc644\ub8cc"},
	{ "progress_event.destdirnotcreated", "\ub300\uc0c1 \ub514\ub809\ud1a0\ub9ac\ub97c \uc791\uc131\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4."},
	{ "progress_event.error", "\uc624\ub958"},
	
	{ "plugin_converter.logerror", "\ub85c\uadf8 \ud30c\uc77c \ucd9c\ub825\uc744 \ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4."},
	{ "plugin_converter.saveerror", "\ub4f1\ub85d \uc815\ubcf4 \ud30c\uc77c\uc744 \uc800\uc7a5\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4:  "},
	{ "plugin_converter.appletconv", "\uc560\ud50c\ub9bf \ubcc0\ud658 "},
	{ "plugin_converter.failure", "\ud30c\uc77c \ubcc0\ud658\uc774 \ubd88\uac00\ub2a5 "},
	{ "plugin_converter.overwrite1", "\ubc31\uc5c5 \ubcf5\uc0ac\ubcf8\uc774 \uc774\ubbf8 \uc788\uc2b5\ub2c8\ub2e4..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "\uc774 \ubc31\uc5c5 \ubcf5\uc0ac\ubcf8\uc744 \uacb9\uccd0\uc4f0\uc2dc\uaca0\uc2b5\ub2c8\uae4c?"},
	{ "plugin_converter.done", "\uc644\ub8cc\ub418\uc5c8\uc2b5\ub2c8\ub2e4. \ucc98\ub9ac\ub41c \ud30c\uc77c:  "},
	{ "plugin_converter.appletfound", "  \uac80\uc0c9\ub41c \uc560\ud50c\ub9bf:  "},
	{ "plugin_converter.processing", "  \ucc98\ub9ac \uc911..."},
	{ "plugin_converter.cancel", "\ubcc0\ud658\uc774 \ucde8\uc18c\ub418\uc5c8\uc2b5\ub2c8\ub2e4."},
	{ "plugin_converter.files", "\ubcc0\ud658\ub420 \ud30c\uc77c: "},
	{ "plugin_converter.converted", "\ud30c\uc77c\uc774 \uc774\uc804\uc5d0 \ubcc0\ud658\ub418\uc5c8\uc73c\ubbc0\ub85c \ubcc0\ud658\uc774 \ud544\uc694 \uc5c6\uc2b5\ub2c8\ub2e4. "},
	{ "plugin_converter.donefound", "\uc644\ub8cc  \uc560\ud50c\ub9bf\uc744 \ucc3e\uc558\uc2b5\ub2c8\ub2e4:  "},
	{ "plugin_converter.seetrace", "\ud30c\uc77c\uc5d0 \uc624\ub958\uac00 \uc788\uc2b5\ub2c8\ub2e4. \uc544\ub798\uc758 \ucd94\uc801\uc744 \ucc38\uc870\ud558\uc2ed\uc2dc\uc624."},
	{ "plugin_converter.noapplet", "\ud30c\uc77c\uc5d0 \uc560\ud50c\ub9bf\uc774 \uc5c6\uc2b5\ub2c8\ub2e4. "},
	{ "plugin_converter.nofiles", "\ucc98\ub9ac\ub420 \ud30c\uc77c\uc774 \uc5c6\uc2b5\ub2c8\ub2e4. "},
	{ "plugin_converter.nobackuppath", "\ubc31\uc5c5 \uacbd\ub85c\ub97c \uc791\uc131\ud558\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4."},
	{ "plugin_converter.writelog", "\ub3d9\uc77c\ud55c \uc774\ub984\uc73c\ub85c \ub85c\uadf8 \ud30c\uc77c\uc5d0 \uacb9\uccd0\uc4f0\ub294 \uc911"},
	{ "plugin_converter.backup_path", "\ubc31\uc5c5 \uacbd\ub85c"},
	{ "plugin_converter.log_path", "\ub85c\uadf8 \uacbd\ub85c"},
	{ "plugin_converter.template_file", "\ud15c\ud50c\ub9bf \ud30c\uc77c"},
	{ "plugin_converter.process_subdirs", "\ubd80\uc18d \ub514\ub809\ud1a0\ub9ac \ucc98\ub9ac"},
	{ "plugin_converter.show_progress", "\uc9c4\ud589 \ud45c\uc2dc"},
	{ "plugin_converter.write_permission", "\ud604\uc7ac \uc791\uc5c5 \ub514\ub809\ud1a0\ub9ac\uc5d0 \uc4f0\uae30 \uad8c\ud55c\uc774 \ud544\uc694\ud569\ub2c8\ub2e4."},
	{ "plugin_converter.overwrite", "\uc784\uc2dc \ud30c\uc77c .tmpSource_stdin\uc774 \uc774\ubbf8 \uc874\uc7ac\ud569\ub2c8\ub2e4. \uc0ad\uc81c \ub610\ub294 \uc774\ub984\uc744 \ubc14\uafb8\uc2ed\uc2dc\uc624."},
	{ "plugin_converter.help_message", newline + 
	                                  "\uc0ac\uc6a9\ubc95: Htmlconverter [-option1 value1 [-option2 value2 [...]]] [-simulate]  [filespecs]" + newline + newline +
	                                  "\uc5ec\uae30\uc11c \uc635\uc158\uc740 \ub2e4\uc74c\uacfc \uac19\uc2b5\ub2c8\ub2e4." + newline + newline +
	                                  "    -source: \uc6d0\ubcf8 \ud30c\uc77c\uc744 \uac00\uc838\uc624\ub294 \uacbd\ub85c. \uae30\ubcf8\uac12:<userdir>" + newline +
                                          "    -source -:  \ud45c\uc900 \uc785\ub825\uc5d0\uc11c \ubcc0\ud658\ub41c \ud30c\uc77c \uc77d\uae30" + newline +
	                                  "    -dest: \ubcc0\ud658\ub41c \ud30c\uc77c \uc4f0\uae30 \uacbd\ub85c. \uae30\ubcf8\uac12:<userdir>" + newline +
                                          "    -dest -:    \ud45c\uc900 \ucd9c\ub825\uc73c\ub85c \ubcc0\ud658\ub41c \ud30c\uc77c\uc744 \uc791\uc131" + newline +
	                                  "    -backup: \ubc31\uc5c5 \ud30c\uc77c \uc4f0\uae30 \uacbd\ub85c. \uae30\ubcf8\uac12:<dirname>_BAK" + newline +
	                                  "    -f: \ubc31\uc5c5 \ud30c\uc77c\uc744 \uacb9\uccd0 \uc4f0\uae30." + newline +
                                          "    -subdirs: \ubd80\uc18d \ub514\ub809\ud1a0\ub9ac\uc758 \ud30c\uc77c\uae4c\uc9c0 \ucc98\ub9ac\ud574\uc57c \ud568." + newline +
	                                  "    -template: \ud15c\ud50c\ub9bf \ud30c\uc77c \uacbd\ub85c. \ud655\uc2e4\ud558\uc9c0 \uc54a\uc73c\uba74 \uae30\ubcf8\uac12\uc744 \uc0ac\uc6a9\ud558\uc2ed\uc2dc\uc624." + newline +
	                                  "    -log: \ub85c\uadf8 \uc791\uc131 \uacbd\ub85c. \uacbd\ub85c\ub97c \uc9c0\uc815\ud558\uc9c0 \uc54a\uc73c\uba74, \ub85c\uadf8\uac00 \uae30\ub85d\ub418\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." + newline +
	                                  "    -progress: \ubcc0\ud658\ud558\ub294 \ub3d9\uc548 \uc9c4\ud589 \uc0c1\ud669 \ud45c\uc2dc. \uae30\ubcf8\uac12:false" + newline +
	                                  "    -simulate: \uc2e4\uc81c \ubcc0\ud658 \uc791\uc5c5 \uc5c6\uc774 \ubcc0\ud658\uc2dc \uc0c1\uc138 \ud56d\ubaa9 \ud45c\uc2dc." + newline + 
                                          "    -latest: \uc774 \ub9b4\ub9ac\uc2a4 mimetype\uc744 \uc9c0\uc6d0\ud558\ub294 \ucd5c\uc2e0 \ubc84\uc804\uc758 JRE\uc774 \uc0ac\uc6a9." + newline + 
                                          "    -gui: converter\uc6a9 \uadf8\ub798\ud53d \uc0ac\uc6a9\uc790 \uc778\ud130\ud398\uc774\uc2a4 \ud45c\uc2dc." + newline + newline +
	                                  "    filespecs: \ud30c\uc77c \uc2a4\ud399\uc758 \ubd84\ub9ac \ubaa9\ub85d \uacf5\uac04. \uae30\ubcf8\uac12:\"*.html *.htm\"(\uc778\uc6a9 \ubd80\ud638 \ud544\uc694)" + newline},
	
	{ "product_name", "Java(TM) Plug-in HTML \ubcc0\ud658\uae30" },
    };
}

