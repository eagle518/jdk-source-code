/*
 * @(#)Converter_zh_TW.java	1.47 10/04/22
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

public class Converter_zh_TW extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "\u932f\u8aa4" },
	{ "caption.warning", "\u8b66\u544a" },
	{ "caption.absdirnotfound", "\u627e\u4e0d\u5230\u7d55\u5c0d\u76ee\u9304" },
	{ "caption.reldirnotfound", "\u627e\u4e0d\u5230\u76f8\u5c0d\u76ee\u9304" },
        { "about_dialog.info", "Java(TM) Plug-in HTML Converter v{0}" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "\u95dc\u65bc Java(TM) Plug-in HTML Converter" },
	{ "nottemplatefile_dialog.caption", "\u4e0d\u662f\u4e00\u500b\u7bc4\u672c\u6a94"},
	{ "nottemplatefile_dialog.info0", "\u6307\u5b9a\u7684\u7bc4\u672c\u6a94 " + newline +
                                          " {0} " + newline + 
					  "\u4e0d\u662f\u4e00\u500b\u6709\u6548\u7684\u7bc4\u672c\u6a94\u3002\u9019\u500b\u6a94\u6848\u7684\u7d50\u5c3e" + newline +
					  "\u5fc5\u9808\u6709\u526f\u6a94\u540d .tpl" + newline + newline +
                                          "\u5c07\u7bc4\u672c\u6a94\u91cd\u8a2d\u70ba\u9810\u8a2d\u503c\u3002"},
	{ "warning_dialog.info", "\u5099\u4efd\u6a94\u6848\u593e\u548c\u76ee\u6a19\u6a94\u6848\u593e\u4e0d\u80fd\u6709" + newline
				+"\u76f8\u540c\u7684\u8def\u5f91\u3002\u60a8\u662f\u5426\u8981\u628a\u5099\u4efd" + newline
				+"\u6a94\u6848\u593e\u7684\u8def\u5f91\u6539\u70ba\uff1a" + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "\u627e\u4e0d\u5230\u7bc4\u672c\u6a94"},
        { "notemplate_dialog.info", "\u9810\u8a2d\u7684\u7bc4\u672c\u6a94 ({0})" + newline +
                                    "\u627e\u4e0d\u5230\u3002\u5b83\u4e0d\u5728\u985e\u5225\u8def\u5f91" + newline +
				   "\u6216\u662f\u4e0d\u5728\u5de5\u4f5c\u76ee\u9304\u4e2d\u3002"},
        { "file_unwritable.info", "\u7121\u6cd5\u5beb\u5165\u6a94\u6848\uff1a"},
	{ "file_notexists.info", "\u6a94\u6848\u4e0d\u5b58\u5728\uff1a "},
	{ "illegal_source_and_backup.info", "\u76ee\u6a19\u8207\u5099\u4efd\u76ee\u9304\u4e0d\u80fd\u662f\u76f8\u540c\u7684!"},
	{ "button.reset", "\u91cd\u8a2d\u70ba\u9810\u8a2d\u503c(R)"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "\u78ba\u5b9a(O)"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "\u53d6\u6d88(C)"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.done", "\u5b8c\u6210(D)"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "button.browse.dir", "\u700f\u89bd(B)..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_B)},
        { "button.browse.backup", "\u700f\u89bd(R)..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.convert", "\u8f49\u63db(C)..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_C)},

	{ "advanced_dialog.caption", "\u9032\u968e\u9078\u9805"},
	{ "advanced_dialog.cab", "\u6307\u5b9a ActiveX CAB \u6a94\u6848\u7684\u4f86\u6e90\u4f4d\u7f6e\uff1a"},
	{ "advanced_dialog.plugin", "\u6307\u5b9a Netscape Plug-in \u7684\u4f86\u6e90\u4f4d\u7f6e\uff1a"},
	{ "advanced_dialog.smartupdate", "\u6307\u5b9a Netscape SmartUpdate \u7684\u4f86\u6e90\u4f4d\u7f6e\uff1a"},
	{ "advanced_dialog.mimetype", "\u6307\u5b9a Java Plug-in HTML \u8f49\u63db\u7684 MIME \u985e\u578b\uff1a"},
	{ "advanced_dialog.log", "\u6307\u5b9a\u65e5\u8a8c\u6a94\u7684\u4f4d\u7f6e\uff1a"},
	{ "advanced_dialog.generate", "\u7522\u751f\u65e5\u8a8c\u6a94(G)"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "\u9032\u884c\u4e2d..."},
	{ "progress_dialog.processing", "\u8655\u7406\u4e2d..."},
	{ "progress_dialog.folder", "\u6a94\u6848\u593e\uff1a"},
	{ "progress_dialog.file", "\u6a94\u6848\uff1a"},
	{ "progress_dialog.totalfile", "\u8655\u7406\u7684\u6a94\u6848\u7e3d\u8a08\uff1a"},
	{ "progress_dialog.totalapplet", "\u627e\u5230\u7684 Applet \u7e3d\u8a08\uff1a"},
	{ "progress_dialog.totalerror", "\u932f\u8aa4\u7e3d\u8a08\uff1a"},

	{ "notdirectory_dialog.caption0", "\u4e0d\u662f\u6709\u6548\u7684\u6a94\u6848"},
	{ "notdirectory_dialog.caption1", "\u4e0d\u662f\u6709\u6548\u7684\u6a94\u6848\u593e"},
        { "notdirectory_dialog.info0", "\u4e0b\u5217\u6a94\u6848\u593e\u4e0d\u5b58\u5728" + newline + "{0}"},
        { "notdirectory_dialog.info1", "\u4e0b\u5217\u6a94\u6848\u4e0d\u5b58\u5728"  + newline + "{0}"},
	{ "notdirectory_dialog.info5", "\u4ee5\u4e0b\u7684\u6a94\u6848\u593e\u4e0d\u5b58\u5728 " + newline + "<empty>"},
        
	{ "converter_gui.lablel0", "\u6307\u5b9a\u6a94\u6848\u6216\u662f\u76ee\u9304\u8def\u5f91\uff1a"},
	{ "converter_gui.lablel1", "\u9078\u53d6\u76f8\u7b26\u7684\u6a94\u6848\u540d\u7a31\uff1a"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "\u52a0\u5165\u5b50\u6a94\u6848\u593e(I)"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "\u4e00\u500b\u6a94\u6848\uff1a"},
	{ "converter_gui.lablel5", "\u5c07\u6a94\u6848\u5099\u4efd\u81f3\u4ee5\u4e0b\u6a94\u6848\u593e\uff1a"},
	{ "converter_gui.lablel7", "\u7bc4\u672c\u6a94\uff1a"},


	{ "template.default", "\u6a19\u6e96\u578b\uff08IE & Navigator\uff09\u9069\u7528\u65bc Windows \u548c Solaris \u5e73\u53f0"},
	{ "template.extend",  "\u5ef6\u4f38\u578b\uff08\u6a19\u6e96\u578b + \u6240\u6709\u700f\u89bd\u5668/\u5e73\u53f0\u652f\u63f4\uff09"},
	{ "template.ieonly",  "\u9069\u7528\u65bc Windows \u548c Solaris \u7248\u7684 Internet Explorer"},
	{ "template.nsonly",  "\u9069\u7528\u65bc Windows \u7248\u7684 Navigator"},
	{ "template.other",   "\u5176\u4ed6\u7bc4\u672c..."},

        { "template_dialog.title", "\u9078\u53d6\u6a94\u6848"},
	
        { "help_dialog.caption", "\u8aaa\u660e"},
        { "help_dialog.error", "\u7121\u6cd5\u5b58\u53d6\u8aaa\u660e\u6a94"},

	{ "menu.file", "\u6a94\u6848(F)"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "menu.exit", "\u95dc\u9589(X)"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_X)},
	{ "menu.edit", "\u7de8\u8f2f(E)"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.option", "\u9078\u9805(O)"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "\u8aaa\u660e(H)"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "menu.about", "\u95dc\u65bc(A)"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_A)},

        { "static.versioning.label", "Applets \u7684 Java \u7248\u672c\uff1a"},
        { "static.versioning.radio.button", "\u53ea\u7528 JRE {0}(U)"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "Applet \u5c07\u53ea\u4f7f\u7528\u6b64 JRE \u7684\u7279\u5225\u7248\u672c\u3002\u5982\u679c\u5c1a\u672a\u5b89\u88dd\uff0c\u53ef\u80fd\u7684\u8a71\uff0c\u6b64\u7248\u672c\u5c07\u6703\u81ea\u52d5\u4e0b\u8f09\u3002\u5426\u5247\uff0c\u4f7f\u7528\u8005\u5c07\u6703\u88ab\u5c0e\u5f15\u81f3\u624b\u52d5\u4e0b\u8f09\u7db2\u9801\u3002\u8acb\u5728\u81ea\u52d5\u4e0b\u8f09\u904e\u7a0b\u4e2d\u53c3\u95b1 http://java.sun.com/products/plugin \u7db2\u7ad9\u4ee5\u53d6\u5f97\u8a73\u7d30\u8cc7\u8a0a\uff0c\u4e26\u81f3\u6a94\u6848\u7d50\u5c3e (EOL) \u53d6\u5f97\u6240\u6709Java \u767c\u884c\u7248\u672c\u7684\u8cc7\u8a0a\u3002"},
        { "dynamic.versioning.radio.button", "\u4f7f\u7528\u4efb\u4f55 JRE {0} \u6216\u66f4\u65b0\u7684\u7248\u672c(S)"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "\u5982\u679c\u6c92\u6709\u5b89\u88dd\u6b64\u7248\u672c\uff0c\u53ef\u80fd\u7684\u8a71\uff0cJRE {0} \u7cfb\u5217\u76ee\u524d\u7684\u9810\u8a2d\u4e0b\u8f09\u6703\u81ea\u52d5\u4e0b\u8f09\u3002\u5426\u5247\uff0c\u4f7f\u7528\u8005\u5c07\u6703\u88ab\u5c0e\u5f15\u81f3\u624b\u52d5\u4e0b\u8f09\u7db2\u9801\u3002 "},
        
	{ "progress_event.preparing", "\u6e96\u5099\u4e2d"},
	{ "progress_event.converting", "\u8f49\u63db\u4e2d"},
	{ "progress_event.copying", "\u8907\u88fd\u4e2d"},
	{ "progress_event.done", "\u5b8c\u6210"},
	{ "progress_event.destdirnotcreated", "\u7121\u6cd5\u5efa\u7acb\u76ee\u6a19\u76ee\u9304\u3002"},
	{ "progress_event.error", "\u932f\u8aa4"},
	
	{ "plugin_converter.logerror", "\u7121\u6cd5\u8f38\u51fa\u65e5\u8a8c\u6a94"},
	{ "plugin_converter.saveerror", "\u7121\u6cd5\u5132\u5b58\u5c6c\u6027\u6a94\uff1a"},
	{ "plugin_converter.appletconv", "Applet \u8f49\u63db"},
	{ "plugin_converter.failure", "\u7121\u6cd5\u8f49\u63db\u6a94\u6848 "},
	{ "plugin_converter.overwrite1", "\u5176\u5099\u4efd\u7248\u672c\u5df2\u5b58\u5728..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "\u60a8\u662f\u5426\u8981\u6539\u5beb\u6b64\u5099\u4efd\u7248\u672c\uff1f"},
	{ "plugin_converter.done", "\u5168\u90e8\u5b8c\u6210\u8655\u7406\u7684\u6a94\u6848\uff1a  "},
	{ "plugin_converter.appletfound", "\u627e\u5230\u7684 Applet\uff1a"},
	{ "plugin_converter.processing", "\u8655\u7406\u4e2d"},
	{ "plugin_converter.cancel", "\u53d6\u6d88\u8f49\u63db"},
	{ "plugin_converter.files", "\u6b32\u8f49\u63db\u7684\u6a94\u6848\uff1a "},
	{ "plugin_converter.converted", "\u6a94\u6848\u5148\u524d\u5df2\u8f49\u63db\u904e\uff0c\u4e0d\u9700\u518d\u8f49\u63db\u3002"},
	{ "plugin_converter.donefound", "\u5b8c\u6210   \u627e\u5230\u7684 Applet\uff1a"},
	{ "plugin_converter.seetrace", "\u6a94\u6848\u932f\u8aa4\u2500\u8acb\u770b\u4ee5\u4e0b\u7684\u8ffd\u8e64"},
	{ "plugin_converter.noapplet", "\u4e0b\u5217\u6a94\u6848\u4e2d\u6c92\u6709 applet\uff1a"},
	{ "plugin_converter.nofiles", "\u6c92\u6709\u8981\u8655\u7406\u7684\u6a94\u6848 "},
	{ "plugin_converter.nobackuppath", "\u4e26\u672a\u5efa\u7acb\u5099\u7528\u8def\u5f91"},
	{ "plugin_converter.writelog", "\u4ee5\u76f8\u540c\u540d\u7a31\u6539\u5beb\u65e5\u8a8c\u6a94"},
	{ "plugin_converter.backup_path", "\u5099\u7528\u8def\u5f91"},
	{ "plugin_converter.log_path", "\u65e5\u8a8c\u8def\u5f91"},
	{ "plugin_converter.template_file", "\u7bc4\u672c\u6a94"},
	{ "plugin_converter.process_subdirs", "\u8655\u7406\u5b50\u76ee\u9304"},
	{ "plugin_converter.show_progress", "\u986f\u793a\u9032\u5ea6"},
	{ "plugin_converter.write_permission", "\u5fc5\u9808\u5177\u6709\u76ee\u524d\u5de5\u4f5c\u76ee\u9304\u7684\u5beb\u5165\u6b0a\u9650"},
	{ "plugin_converter.overwrite", "\u81e8\u6642\u6587\u4ef6 .tmpSource_stdin \u5df2\u7d93\u5b58\u5728\u3002\u8acb\u5c07\u5b83\u522a\u9664\u6216\u91cd\u65b0\u547d\u540d\u3002"},
	{ "plugin_converter.help_message", newline + 
	                                  "\u7528\u6cd5\uff1aHtmlConverter [-option1 value1 [-option2 value2 [...]]] [-simulate]  [filespecs]" + newline + newline +
	                                  "\u5176\u4e2d\u9078\u9805\u5305\u62ec\uff1a" + newline + newline +
	                                  "    -source:    \u53d6\u5f97\u539f\u59cb\u6a94\u7684\u8def\u5f91\u3002\u9810\u8a2d\u503c\uff1a<userdir>" + newline +
                                          "    -source -:  \u5f9e\u6a19\u6e96\u8f38\u5165\u8b80\u53d6\u6b32\u8f49\u63db\u7684\u6a94\u6848" + newline +
	                                  "    -dest:      \u5beb\u5165\u8f49\u63db\u6a94\u7684\u8def\u5f91\u3002\u9810\u8a2d\u503c\uff1a<userdir>" + newline +
                                          "    -dest -:    \u5c07\u5b8c\u6210\u8f49\u63db\u7684\u6a94\u6848\u5beb\u81f3\u6a19\u6e96\u8f38\u51fa" + newline +
                                          "    -backup:    \u5beb\u5165\u5099\u4efd\u6a94\u7684\u8def\u5f91\u3002\u9810\u8a2d\u503c\uff1a<dirname>_BAK" + newline +
                                          "    -f:         \u5f37\u5236\u6539\u5beb\u5099\u4efd\u6a94\u3002" + newline +
	                                  "    -subdirs:   \u82e5\u5b50\u76ee\u9304\u4e2d\u7684\u6a94\u6848\u8981\u8655\u7406\u3002" + newline +
	                                  "    -template:  \u7bc4\u672c\u6a94\u7684\u8def\u5f91\u3002\u82e5\u4e0d\u78ba\u5b9a\u8acb\u4f7f\u7528\u9810\u8a2d\u503c\u3002" + newline +
	                                  "    -log:       \u5beb\u5165\u65e5\u8a8c\u7684\u8def\u5f91\u3002\u82e5\u4e0d\u63d0\u4f9b\u5247\u4e0d\u5beb\u5165\u65e5\u8a8c\u3002" + newline +
	                                  "    -progress:  \u8f49\u63db\u6642\u986f\u793a\u9032\u5ea6\u3002\u9810\u8a2d\u503c\uff1afalse" + newline +
	                                  "    -simulate:  \u986f\u793a\u8f49\u63db\u7d30\u7bc0\u800c\u4e0d\u9032\u884c\u8f49\u63db\u3002" + newline + 
                                          "    -latest:    \u4f7f\u7528\u6700\u65b0\u7684 JRE \u652f\u63f4\u7248\u6b21 Mimetype\u3002" + newline + 
	                                  "    -gui:       \u986f\u793a\u8f49\u63db\u5668\u7684\u5716\u5f62\u4f7f\u7528\u8005\u4ecb\u9762\u3002" + newline + newline +
	                                  "     filespecs:  \u6a94\u6848\u898f\u683c\u7684\u7a7a\u683c\u5206\u9694\u6e05\u55ae\u3002\u9810\u8a2d\u503c\uff1a\"*.html *.htm\" (\u9700\u8981\u96d9\u5f15\u865f)" + newline},
	
	{ "product_name", "Java(TM) Plug-in HTML Converter" },
    };
}

