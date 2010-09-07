/*
 * @(#)Converter_zh_CN.java	1.45 10/04/22
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

public class Converter_zh_CN extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "\u9519\u8bef" },
	{ "caption.warning", "\u8b66\u544a" },
	{ "caption.absdirnotfound", "\u6ca1\u6709\u627e\u5230\u7edd\u5bf9\u8def\u5f84" },
	{ "caption.reldirnotfound", "\u6ca1\u6709\u627e\u5230\u76f8\u5bf9\u8def\u5f84" },
        { "about_dialog.info", "Java(TM) Plug-in HTML \u8f6c\u6362\u5668 v{0}" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "\u5173\u4e8e Java(TM) Plug-in HTML \u8f6c\u6362\u7a0b\u5e8f" },
	{ "nottemplatefile_dialog.caption", "\u4e0d\u662f\u6a21\u677f\u6587\u4ef6"},
	{ "nottemplatefile_dialog.info0", "\u6307\u5b9a\u7684\u6a21\u677f\u6587\u4ef6" + newline + 
                                           " {0} " + newline   
					+ "\u4e0d\u662f\u6709\u6548\u7684\u6a21\u677f\u6587\u4ef6\u3002\u8be5\u6587\u4ef6\u7684\u6269\u5c55\u540d\u5fc5\u987b" + newline
					+ "\u4e3a .tpl " + newline + newline
                                        + "\u5c06\u6a21\u677f\u6587\u4ef6\u91cd\u8bbe\u4e3a\u7f3a\u7701\u503c\u3002"},
	{ "warning_dialog.info", "\u76ee\u6807\u6587\u4ef6\u5939\u548c\u5907\u4efd\u6587\u4ef6\u5939\u4e0d\u53ef" + newline
				+"\u62e5\u6709\u540c\u4e00\u8def\u5f84\u3002\u60a8\u662f\u5426\u613f\u610f\u5c06\u5907\u4efd" + newline
				+"\u6587\u4ef6\u5939\u8def\u5f84\u66f4\u6539\u4e3a\u4e0b\u5217\u8def\u5f84\uff1a" + newline + 
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "\u6ca1\u6709\u627e\u5230\u6a21\u677f\u6587\u4ef6"},
        { "notemplate_dialog.info", "\u7f3a\u7701\u6a21\u677f\u6587\u4ef6 ({0})" + newline + 
                                      "\u6ca1\u6709\u627e\u5230\u3002 \u5b83\u6216\u8005\u4e0d\u5728\u5206\u7c7b\u8def\u5f84\u4e2d"                        + newline +  
				     "\u6216\u8005\u4e0d\u5728\u5de5\u4f5c\u76ee\u5f55\u4e2d\u3002"},
        { "file_unwritable.info", "\u6587\u4ef6\u4e0d\u53ef\u5199\u5165: "},
	{ "file_notexists.info", "\u6587\u4ef6\u4e0d\u5b58\u5728: "},
	{ "illegal_source_and_backup.info", "\u76ee\u6807\u76ee\u5f55\u548c\u5907\u4efd\u76ee\u5f55\u4e0d\u80fd\u76f8\u540c!"},
	{ "button.reset", "\u91cd\u8bbe\u4e3a\u7f3a\u7701\u503c(R)"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "\u786e\u5b9a(O)"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "\u53d6\u6d88"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.done", "\u5b8c\u6210(D)"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "button.browse.dir", "\u6d4f\u89c8(B)..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_B)},
        { "button.browse.backup", "\u6d4f\u89c8(R)..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.convert", "\u8f6c\u6362(C)..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_C)},

	{ "advanced_dialog.caption", "\u9ad8\u7ea7\u9009\u9879"},
	{ "advanced_dialog.cab", "\u4e3a ActiveX CAB \u6587\u4ef6\u6307\u5b9a\u6e90\u4f4d\u7f6e\uff1a"},
	{ "advanced_dialog.plugin", "\u4e3a Netscape Plug-in \u6307\u5b9a\u6e90\u4f4d\u7f6e\uff1a"},
	{ "advanced_dialog.smartupdate", "\u6307\u5b9a Netscape SmartUpdate \u7684\u6e90\u4f4d\u7f6e:"},
	{ "advanced_dialog.mimetype", "\u6307\u5b9a Java Plug-in HTML \u8f6c\u6362\u7684 MIME \u7c7b\u578b:"},
	{ "advanced_dialog.log", "\u6307\u5b9a\u65e5\u5fd7\u6587\u4ef6\u7684\u4f4d\u7f6e\uff1a"},
	{ "advanced_dialog.generate", "\u751f\u6210\u65e5\u5fd7\u6587\u4ef6(G)"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "\u8fdb\u5ea6..."},
	{ "progress_dialog.processing", "\u6b63\u5728\u5904\u7406..."},
	{ "progress_dialog.folder", "\u6587\u4ef6\u5939\uff1a"},
	{ "progress_dialog.file", "\u6587\u4ef6\uff1a"},
	{ "progress_dialog.totalfile", "\u5904\u7406\u8fc7\u7684\u6587\u4ef6\u603b\u8ba1\uff1a"},
	{ "progress_dialog.totalapplet", "\u5df2\u627e\u5230\u7684\u5c0f\u5e94\u7528\u7a0b\u5e8f\u603b\u8ba1\uff1a"},
	{ "progress_dialog.totalerror", "\u9519\u8bef\u603b\u8ba1\uff1a"},

	{ "notdirectory_dialog.caption0", "\u4e0d\u662f\u4e00\u4e2a\u6709\u6548\u7684\u6587\u4ef6"},
	{ "notdirectory_dialog.caption1", "\u4e0d\u662f\u4e00\u4e2a\u6709\u6548\u7684\u6587\u4ef6\u5939"},
        { "notdirectory_dialog.info0", "\u4e0b\u5217\u6587\u4ef6\u5939\u4e0d\u5b58\u5728" + newline + "{0}"},
        { "notdirectory_dialog.info1", "\u4e0b\u5217\u6587\u4ef6\u4e0d\u5b58\u5728" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "\u4e0b\u5217\u6587\u4ef6\u5939\u4e0d\u5b58\u5728 " + newline + "<empty>"},
        
	{ "converter_gui.lablel0", "\u6307\u5b9a\u6587\u4ef6\u6216\u76ee\u5f55\u8def\u5f84\uff1a"},
	{ "converter_gui.lablel1", "\u6b63\u5728\u5339\u914d\u6587\u4ef6\u540d\uff1a"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "\u5305\u542b\u5b50\u6587\u4ef6\u5939(I)"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "\u4e00\u4e2a\u6587\u4ef6\uff1a"},
	{ "converter_gui.lablel5", "\u5c06\u6587\u4ef6\u5907\u4efd\u5230\u6587\u4ef6\u5939\uff1a"},
	{ "converter_gui.lablel7", "\u6a21\u677f\u6587\u4ef6\uff1a"},


	{ "template.default", "\u53ea\u9002\u7528\u4e8e Windows \u548c Solaris \u7684\u6807\u51c6\u7ec4\u4ef6\uff08IE \u548c Navigator\uff09"},
	{ "template.extend",  "\u6269\u5c55\u7ec4\u4ef6\uff08\u6807\u51c6\u7ec4\u4ef6 + \u6240\u6709\u7684\u6d4f\u89c8\u5668/\u5e73\u53f0\uff09"},
	{ "template.ieonly",  "\u53ea\u9002\u7528\u4e8e Windows \u548c  Solaris \u7684 Internet Explorer"},
	{ "template.nsonly",  "\u53ea\u9002\u7528\u4e8e Windows \u7684 Navigator"},
	{ "template.other",   "\u5176\u5b83\u6a21\u677f..."},

        { "template_dialog.title", "\u9009\u62e9\u6587\u4ef6"},
	
        { "help_dialog.caption", "\u5e2e\u52a9"},
        { "help_dialog.error", "\u65e0\u6cd5\u8bbf\u95ee\u5e2e\u52a9\u6587\u4ef6"},

	{ "menu.file", "\u6587\u4ef6(F)"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "menu.exit", "\u9000\u51fa(X)"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_X)},
	{ "menu.edit", "\u7f16\u8f91(E)"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.option", "\u9009\u9879(O)"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "\u5e2e\u52a9(H)"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "menu.about", "\u5173\u4e8e(A)"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_A)},

        { "static.versioning.label", "\u9002\u7528\u4e8e\u5c0f\u7a0b\u5e8f\u7684 Java \u7248\u672c\uff1a"},
        { "static.versioning.radio.button", "\u53ea\u4f7f\u7528 JRE {0}(U)"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "\u5c0f\u7a0b\u5e8f\u53ea\u4f7f\u7528\u8be5\u7279\u5b9a\u7684 JRE \u7248\u672c\u3002\u82e5\u6ca1\u6709\u5b89\u88c5\u6b64\u7248\u672c\uff0c\u8be5\u7248\u672c\u5c06\u81ea\u52a8\u4e0b\u8f7d\u3002\u5426\u5219\uff0c\u5c06\u91cd\u65b0\u6307\u5b9a\u7528\u6237\u8fdb\u5165\u624b\u52a8\u4e0b\u8f7d\u9875\u8fdb\u884c\u4e0b\u8f7d\u3002 \u5173\u4e8e\u81ea\u52a8\u4e0b\u8f7d\u8fc7\u7a0b\u548c\u6240\u6709 Java \u53d1\u884c\u7248\u7684 End of Life (EOL) \u7b56\u7565\u7684\u8be6\u7ec6\u4fe1\u606f\uff0c\u8bf7\u53c2\u89c1 http://java.sun.com/products/plugin "},
        { "dynamic.versioning.radio.button", "\u4f7f\u7528\u4efb\u4f55 JRE {0}\uff0c\u6216\u66f4\u9ad8\u7248\u672c(S)"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "\u5982\u679c\u6ca1\u6709\u5b89\u88c5\u8fd9\u6837\u7684\u7248\u672c\uff0c \u5982\u679c\u53ef\u80fd\uff0c\u5c06\u81ea\u52a8\u4e0b\u8f7d\u5f53\u524d JRE {0} \u7cfb\u5217\u7684\u7f3a\u7701\u4e0b\u8f7d\u7248\u672c\u3002\u5426\u5219\uff0c\u7528\u6237\u5c06\u88ab\u91cd\u65b0\u5f15\u5bfc\u5230\u624b\u5de5\u4e0b\u8f7d\u9875\u9762\u3002"},
        
	{ "progress_event.preparing", "\u6b63\u5728\u51c6\u5907"},
	{ "progress_event.converting", "\u6b63\u5728\u8f6c\u6362"},
	{ "progress_event.copying", "\u6b63\u5728\u590d\u5236"},
	{ "progress_event.done", "\u5b8c\u6210"},
	{ "progress_event.destdirnotcreated", "\u65e0\u6cd5\u521b\u5efa\u76ee\u6807\u76ee\u5f55"},
	{ "progress_event.error", "\u9519\u8bef"},
	
	{ "plugin_converter.logerror", "\u65e0\u6cd5\u5efa\u7acb\u65e5\u5fd7\u6587\u4ef6\u8f93\u51fa"},
	{ "plugin_converter.saveerror", "\u65e0\u6cd5\u4fdd\u5b58\u5c5e\u6027\u6587\u4ef6\uff1a"},
	{ "plugin_converter.appletconv", "\u5c0f\u5e94\u7528\u7a0b\u5e8f\u8f6c\u6362"},
	{ "plugin_converter.failure", "\u65e0\u6cd5\u8f6c\u6362\u6587\u4ef6 "},
	{ "plugin_converter.overwrite1", "\u5907\u4efd\u526f\u672c\u5df2\u7ecf\u5b58\u5728..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "\u662f\u5426\u8981\u8986\u76d6\u6b64\u5907\u4efd\u526f\u672c\uff1f"},
	{ "plugin_converter.done", "\u5168\u90e8\u5b8c\u6210  \u6587\u4ef6\u5df2\u88ab\u5904\u7406\uff1a  "},
	{ "plugin_converter.appletfound", "  \u5df2\u627e\u5230\u5c0f\u5e94\u7528\u7a0b\u5e8f\uff1a"},
	{ "plugin_converter.processing", "  \u6b63\u5728\u5904\u7406..."},
	{ "plugin_converter.cancel", "\u5df2\u53d6\u6d88\u8f6c\u6362"},
	{ "plugin_converter.files", "\u8981\u8f6c\u6362\u7684\u6587\u4ef6: "},
	{ "plugin_converter.converted", "\u4ee5\u524d\u8f6c\u6362\u8fc7\u7684\u6587\u4ef6\uff0c\u65e0\u987b\u518d\u8fdb\u884c\u8f6c\u6362\u3002"},
	{ "plugin_converter.donefound", "\u5b8c\u6210   \u5df2\u627e\u5230\u5c0f\u5e94\u7528\u7a0b\u5e8f\uff1a"},
	{ "plugin_converter.seetrace", "\u6587\u4ef6\u4e2d\u6709\u9519\u8bef - \u8bf7\u53c2\u89c1\u4e0b\u9762\u7684\u8ddf\u8e2a"},
	{ "plugin_converter.noapplet", "\u6587\u4ef6\u4e2d\u6ca1\u6709\u5c0f\u5e94\u7528\u7a0b\u5e8f"},
	{ "plugin_converter.nofiles", "\u6ca1\u6709\u8981\u5904\u7406\u7684\u6587\u4ef6 "},
	{ "plugin_converter.nobackuppath", "\u4e0d\u8981\u521b\u5efa\u5907\u4efd\u8def\u5f84"},
	{ "plugin_converter.writelog", "\u8986\u5199\u5177\u6709\u540c\u4e00\u540d\u79f0\u7684\u65e5\u5fd7\u6587\u4ef6"},
	{ "plugin_converter.backup_path", "\u5907\u4efd\u8def\u5f84"},
	{ "plugin_converter.log_path", "\u8bb0\u5f55\u8def\u5f84"},
	{ "plugin_converter.template_file", "\u6a21\u677f\u6587\u4ef6"},
	{ "plugin_converter.process_subdirs", "\u7a0b\u5e8f\u5b50\u76ee\u5f55"},
	{ "plugin_converter.show_progress", "\u663e\u793a\u8fdb\u5ea6"},
	{ "plugin_converter.write_permission", "\u5fc5\u987b\u5177\u6709\u5f53\u524d\u5de5\u4f5c\u76ee\u5f55\u7684\u5199\u6743\u9650"},
	{ "plugin_converter.overwrite", "\u4e34\u65f6\u6587\u4ef6 .tmpSource_stdin \u5df2\u7ecf\u5b58\u5728\u3002\u8bf7\u5c06\u5b83\u5220\u9664\u6216\u91cd\u65b0\u547d\u540d\u3002"},
	{ "plugin_converter.help_message", newline + 
	                                  "\u7528\u6cd5\uff1a HtmlConverter [-option1 value1 [-option2 value2 [...]]] [-simulate]  [filespecs]" + newline + newline +
	                                  "\u5176\u4e2d\uff0c\u9009\u9879\u5305\u62ec\uff1a" + newline + newline +
	                                  "    -source:    \u83b7\u53d6\u6e90\u6587\u4ef6\u7684\u8def\u5f84\u3002  \u7f3a\u7701\u503c\uff1a <userdir>" + newline +
                                          "    -source -:  \u4ece\u6807\u51c6\u8f93\u5165\u8bfb\u53d6\u8f6c\u6362\u6587\u4ef6" + newline +
	                                  "    -dest:      \u5199\u5165\u5df2\u8f6c\u6362\u6587\u4ef6\u7684\u8def\u5f84\u3002  \u7f3a\u7701\u503c\uff1a <userdir>" + newline +
                                          "    -dest -:    \u5c06\u8f6c\u6362\u597d\u7684\u6587\u4ef6\u5199\u5165\u6807\u51c6\u8f93\u51fa" + newline +
	                                  "    -backup:    \u5199\u5907\u4efd\u6587\u4ef6\u7684\u8def\u5f84\u3002  \u7f3a\u7701\u503c\uff1a <dirname>_BAK" + newline +
                                          "    -f:         \u5f3a\u5236\u8986\u5199\u5907\u4efd\u6587\u4ef6\u3002" + newline +
	                                  "    -subdirs:   \u5e94\u5904\u7406\u5b50\u76ee\u5f55\u4e2d\u7684\u6587\u4ef6\u3002" + newline +
	                                  "    -template:  \u6a21\u677f\u6587\u4ef6\u7684\u8def\u5f84\u3002  \u5982\u679c\u4e0d\u786e\u5b9a\uff0c\u8bf7\u4f7f\u7528\u7f3a\u7701\u503c\u3002" + newline +
	                                  "    -log:       \u5199\u65e5\u5fd7\u7684\u8def\u5f84\u3002  \u5982\u679c\u6ca1\u6709\u63d0\u4f9b\uff0c\u5219\u4e0d\u4f1a\u5199\u5165\u4efb\u4f55\u65e5\u5fd7\u3002" + newline +
	                                  "    -progress:  \u8f6c\u6362\u65f6\u663e\u793a\u8fdb\u5ea6\u3002  \u7f3a\u7701\u503c\uff1a false" + newline +
	                                  "    -simulate:  \u5728\u6ca1\u6709\u8fdb\u884c\u8f6c\u6362\u65f6\u663e\u793a\u7279\u5b9a\u4e8e\u8f6c\u6362\u7684\u4fe1\u606f\u3002" + newline + 
	                                  "    -latest:    \u4f7f\u7528\u6700\u65b0\u7684 JRE \u652f\u6301\u53d1\u884c\u7248 mimetype\u3002" + newline + 
                                          "    -gui:       \u663e\u793a\u8f6c\u6362\u7a0b\u5e8f\u7684\u56fe\u5f62\u7528\u6237\u754c\u9762\u3002" + newline + newline +
	                                  "    filespecs:  \u7528\u7a7a\u683c\u5206\u5f00\u7684\u6587\u4ef6\u8bf4\u660e\u5217\u8868\u3002  \u7f3a\u7701\u503c\uff1a \"*.html *.htm\" \uff08\u9700\u8981\u5f15\u53f7\uff09" + newline},
	
	{ "product_name", "Java(TM) Plug-in HTML \u8f6c\u6362\u7a0b\u5e8f" },
    };
}

