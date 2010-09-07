/*
 * @(#)Converter_ja.java	1.47 10/04/22
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

public class Converter_ja extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "\u30a8\u30e9\u30fc" },
	{ "caption.warning", "\u8b66\u544a" },
	{ "caption.absdirnotfound", "\u7d76\u5bfe\u30d1\u30b9\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u304c\u3042\u308a\u307e\u305b\u3093" },
	{ "caption.reldirnotfound", "\u76f8\u5bfe\u30d1\u30b9\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u304c\u3042\u308a\u307e\u305b\u3093" },
        { "about_dialog.info", "Java(TM) Plug-in HTML Converter v{0}" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "Java(TM) Plug-in HTML Converter \u306b\u3064\u3044\u3066" },
	{ "nottemplatefile_dialog.caption", "\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb\u3067\u306f\u3042\u308a\u307e\u305b\u3093"},
	{ "nottemplatefile_dialog.info0", "\u6307\u5b9a\u3055\u308c\u305f\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb " + newline +
                                          " {0} " + newline + 
					  "\u6b63\u898f\u306e\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb\u3067\u306f\u3042\u308a\u307e\u305b\u3093\u3002\u30d5\u30a1\u30a4\u30eb\u540d\u306e" + newline +
					  "\u62e1\u5f35\u5b50\u306f .tpl \u3067\u306a\u3051\u308c\u3070\u306a\u308a\u307e\u305b\u3093\u3002" + newline + newline +
                                          "\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb\u3092\u30c7\u30d5\u30a9\u30eb\u30c8\u306b\u518d\u8a2d\u5b9a\u3057\u3066\u3044\u307e\u3059\u3002"},
	{ "warning_dialog.info", "\u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u30d5\u30a9\u30eb\u30c0\u306e\u30d1\u30b9\u3068\u5b9b\u5148\u30d5\u30a9\u30eb\u30c0\u306e\u30d1\u30b9\u3092" + newline +
	                         "\u540c\u3058\u306b\u3059\u308b\u3053\u3068\u306f\u3067\u304d\u307e\u305b\u3093\u3002\u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u30d5\u30a9\u30eb\u30c0\u306e" + newline +
	                         "\u30d1\u30b9\u3092\u6b21\u306e\u3088\u3046\u306b\u5909\u66f4\u3057\u307e\u3059\u304b: " + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb\u304c\u3042\u308a\u307e\u305b\u3093"},
        { "notemplate_dialog.info", "\u30c7\u30d5\u30a9\u30eb\u30c8\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb ({0}) \u304c" + newline +
                                    "\u898b\u3064\u304b\u308a\u307e\u305b\u3093\u3002\u30d5\u30a1\u30a4\u30eb\u306f\u30af\u30e9\u30b9\u30d1\u30b9\u5185\u306b\u3082\u3001" + newline +
                                    "\u4f5c\u696d\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u5185\u306b\u3082\u5b58\u5728\u3057\u307e\u305b\u3093\u3002"},
        { "file_unwritable.info", "\u30d5\u30a1\u30a4\u30eb\u306b\u66f8\u304d\u8fbc\u307f\u3067\u304d\u307e\u305b\u3093: "},
	{ "file_notexists.info", "\u30d5\u30a1\u30a4\u30eb\u304c\u3042\u308a\u307e\u305b\u3093: "},
	{ "illegal_source_and_backup.info", "\u51fa\u529b\u5148\u3068\u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u306e\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u540c\u3058\u306b\u3059\u308b\u3053\u3068\u306f\u3067\u304d\u307e\u305b\u3093!"},
	{ "button.reset", "\u30c7\u30d5\u30a9\u30eb\u30c8\u306b\u623b\u3059(R)"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "OK"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "\u30ad\u30e3\u30f3\u30bb\u30eb(C)"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.done", "\u5b8c\u4e86(D)"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "button.browse.dir", "\u53c2\u7167(B)..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_B)},
        { "button.browse.backup", "\u53c2\u7167(R)..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.convert", "\u5909\u63db(C)..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_C)},

	{ "advanced_dialog.caption", "\u8a73\u7d30\u8a2d\u5b9a"},
	{ "advanced_dialog.cab", "ActiveX CAB \u30d5\u30a1\u30a4\u30eb\u306e\u3042\u308b\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u6307\u5b9a:"},
	{ "advanced_dialog.plugin", "Netscape Plug-in \u306e\u3042\u308b\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u6307\u5b9a:"},
	{ "advanced_dialog.smartupdate", "Netscape SmartUpdate \u306e\u3042\u308b\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u6307\u5b9a:"},
	{ "advanced_dialog.mimetype", "Java Plug-in \u306e HTML \u5909\u63db\u7528 MIME \u30bf\u30a4\u30d7\u3092\u6307\u5b9a:"},
	{ "advanced_dialog.log", "\u30ed\u30b0\u30d5\u30a1\u30a4\u30eb\u306e\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u6307\u5b9a:"},
	{ "advanced_dialog.generate", "\u30ed\u30b0\u30d5\u30a1\u30a4\u30eb\u3092\u751f\u6210(G)"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "\u9032\u884c\u4e2d..."},
	{ "progress_dialog.processing", "\u51e6\u7406\u4e2d..."},
	{ "progress_dialog.folder", "\u30d5\u30a9\u30eb\u30c0:"},
	{ "progress_dialog.file", "\u30d5\u30a1\u30a4\u30eb:"},
	{ "progress_dialog.totalfile", "\u51e6\u7406\u3055\u308c\u305f\u30d5\u30a1\u30a4\u30eb\u306e\u7dcf\u6570:"},
	{ "progress_dialog.totalapplet", "\u691c\u51fa\u3055\u308c\u305f\u30a2\u30d7\u30ec\u30c3\u30c8\u306e\u7dcf\u6570:"},
	{ "progress_dialog.totalerror", "\u7dcf\u30a8\u30e9\u30fc\u6570:"},

	{ "notdirectory_dialog.caption0", "\u30d5\u30a1\u30a4\u30eb\u304c\u4e0d\u6b63"},
	{ "notdirectory_dialog.caption1", "\u30d5\u30a9\u30eb\u30c0\u304c\u4e0d\u6b63"},
        { "notdirectory_dialog.info0", "\u6b21\u306e\u30d5\u30a9\u30eb\u30c0\u306f\u5b58\u5728\u3057\u307e\u305b\u3093" + newline + "{0}"},
        { "notdirectory_dialog.info1", "\u6b21\u306e\u30d5\u30a1\u30a4\u30eb\u306f\u5b58\u5728\u3057\u307e\u305b\u3093" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "\u6b21\u306e\u30d5\u30a9\u30eb\u30c0\u306f\u5b58\u5728\u3057\u307e\u305b\u3093" + newline + "<empty>"},
        
	{ "converter_gui.lablel0", "\u30d5\u30a1\u30a4\u30eb\u307e\u305f\u306f\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u30d1\u30b9\u3092\u6307\u5b9a:"},
	{ "converter_gui.lablel1", "\u30d5\u30a1\u30a4\u30eb\u540d:"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "\u30b5\u30d6\u30d5\u30a9\u30eb\u30c0\u3092\u542b\u3081\u308b(I)"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "1 \u3064\u306e\u30d5\u30a1\u30a4\u30eb:"},
	{ "converter_gui.lablel5", "\u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u30d5\u30a1\u30a4\u30eb\u7528\u306e\u30d5\u30a9\u30eb\u30c0:"},
	{ "converter_gui.lablel7", "\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb:"},


	{ "template.default", "\u6a19\u6e96 (IE \u3068 Navigator) - Windows \u304a\u3088\u3073 Solaris \u306e\u307f"},
	{ "template.extend",  "\u62e1\u5f35 (\u6a19\u6e96 + \u3059\u3079\u3066\u306e\u30d6\u30e9\u30a6\u30b6\u3068\u30d7\u30e9\u30c3\u30c8\u30d5\u30a9\u30fc\u30e0)"},
	{ "template.ieonly",  "Internet Explorer - Windows \u304a\u3088\u3073 Solaris \u306e\u307f"},
	{ "template.nsonly",  "Netscape Navigator - Windows \u306e\u307f"},
	{ "template.other",   "\u4ed6\u306e\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8..."},

        { "template_dialog.title", "\u30d5\u30a1\u30a4\u30eb\u3092\u9078\u629e"},
	
        { "help_dialog.caption", "\u30d8\u30eb\u30d7"},
        { "help_dialog.error", "\u30d8\u30eb\u30d7\u30d5\u30a1\u30a4\u30eb\u306b\u30a2\u30af\u30bb\u30b9\u3067\u304d\u307e\u305b\u3093\u3067\u3057\u305f"},

	{ "menu.file", "\u30d5\u30a1\u30a4\u30eb(F)"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "menu.exit", "\u7d42\u4e86(X)"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_X)},
	{ "menu.edit", "\u7de8\u96c6(E)"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.option", "\u30aa\u30d7\u30b7\u30e7\u30f3(O)"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "\u30d8\u30eb\u30d7(H)"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "menu.about", "\u88fd\u54c1\u60c5\u5831(A)"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_A)},

        { "static.versioning.label", "\u30a2\u30d7\u30ec\u30c3\u30c8\u306e Java \u30d0\u30fc\u30b8\u30e7\u30f3\u7ba1\u7406\u6a5f\u80fd:"},
        { "static.versioning.radio.button", "JRE {0} \u306e\u307f\u3092\u4f7f\u7528(U)"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "\u30a2\u30d7\u30ec\u30c3\u30c8\u306f JRE \u306e\u3053\u306e\u7279\u5b9a\u30d0\u30fc\u30b8\u30e7\u30f3\u3060\u3051\u3092\u4f7f\u7528\u3057\u307e\u3059\u3002\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u3066\u3044\u306a\u3044\u5834\u5408\u3001\u53ef\u80fd\u3067\u3042\u308c\u3070\u81ea\u52d5\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3092\u884c\u306a\u3044\u307e\u3059\u3002\u4e0d\u53ef\u80fd\u306a\u5834\u5408\u306f\u30e6\u30fc\u30b6\u3092\u624b\u52d5\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u30da\u30fc\u30b8\u306b\u5c0e\u304d\u307e\u3059\u3002\u81ea\u52d5\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u624b\u9806\u3001\u304a\u3088\u3073\u3059\u3079\u3066\u306e Java \u30ea\u30ea\u30fc\u30b9\u306b\u3064\u3044\u3066\u306e End of Life (EOL) \u30dd\u30ea\u30b7\u30fc\u306e\u8a73\u7d30\u306f\u3001http://java.sun.com/products/plugin \u3092\u53c2\u7167\u3057\u3066\u304f\u3060\u3055\u3044\u3002"},
        { "dynamic.versioning.radio.button", "JRE {0} \u4ee5\u964d\u3092\u4f7f\u7528(S)"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "\u6307\u5b9a\u3055\u308c\u305f\u30d0\u30fc\u30b8\u30e7\u30f3\u304c\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u3066\u3044\u306a\u3044\u5834\u5408\u3001\u53ef\u80fd\u3067\u3042\u308c\u3070 JRE {0} \u30d5\u30a1\u30df\u30ea\u306e\u73fe\u5728\u306e\u30c7\u30d5\u30a9\u30eb\u30c8\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u304c\u81ea\u52d5\u7684\u306b\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3055\u308c\u307e\u3059\u3002\u305d\u308c\u4ee5\u5916\u306e\u5834\u5408\u3001\u30e6\u30fc\u30b6\u306f\u624b\u52d5\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u30da\u30fc\u30b8\u306b\u30ea\u30c0\u30a4\u30ec\u30af\u30c8\u3055\u308c\u307e\u3059\u3002"},
        
	{ "progress_event.preparing", "\u4f5c\u6210\u4e2d"},
	{ "progress_event.converting", "\u5909\u63db\u4e2d"},
	{ "progress_event.copying", "\u30b3\u30d4\u30fc\u4e2d"},
	{ "progress_event.done", "\u7d42\u4e86"},
	{ "progress_event.destdirnotcreated", "\u51fa\u529b\u5148\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u4f5c\u6210\u3067\u304d\u307e\u305b\u3093\u3002"},
	{ "progress_event.error", "\u30a8\u30e9\u30fc"},
	
	{ "plugin_converter.logerror", "\u30ed\u30b0\u30d5\u30a1\u30a4\u30eb\u306e\u51fa\u529b\u5148\u304c\u78ba\u5b9a\u3057\u3066\u3044\u307e\u305b\u3093"},
	{ "plugin_converter.saveerror", "\u30d7\u30ed\u30d1\u30c6\u30a3\u30d5\u30a1\u30a4\u30eb\u3092\u4fdd\u5b58\u3067\u304d\u307e\u305b\u3093:  "},
	{ "plugin_converter.appletconv", "\u30a2\u30d7\u30ec\u30c3\u30c8\u306e\u5909\u63db "},
	{ "plugin_converter.failure", "\u30d5\u30a1\u30a4\u30eb\u3092\u5909\u63db\u3059\u308b\u3053\u3068\u304c\u3067\u304d\u307e\u305b\u3093 "},
	{ "plugin_converter.overwrite1", "\u3059\u3067\u306b\u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u30b3\u30d4\u30fc\u304c\u3042\u308a\u307e\u3059..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "\u3053\u306e\u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u30b3\u30d4\u30fc\u3092\u4e0a\u66f8\u304d\u3057\u3066\u3082\u3088\u3044\u3067\u3059\u304b?"},
	{ "plugin_converter.done", "\u3059\u3079\u3066\u5b8c\u4e86 - \u51e6\u7406\u3057\u305f\u30d5\u30a1\u30a4\u30eb:  "},
	{ "plugin_converter.appletfound", "  \u691c\u51fa\u3057\u305f\u30a2\u30d7\u30ec\u30c3\u30c8:  "},
	{ "plugin_converter.processing", "  \u51e6\u7406\u4e2d..."},
	{ "plugin_converter.cancel", "\u5909\u63db\u3092\u4e2d\u65ad\u3057\u307e\u3057\u305f"},
	{ "plugin_converter.files", "\u5909\u63db\u3055\u308c\u305f\u30d5\u30a1\u30a4\u30eb: "},
	{ "plugin_converter.converted", "\u30d5\u30a1\u30a4\u30eb\u306f\u3059\u3067\u306b\u5909\u63db\u3055\u308c\u3066\u3044\u308b\u306e\u3067\u51e6\u7406\u306e\u5fc5\u8981\u306f\u3042\u308a\u307e\u305b\u3093\u3002 "},
	{ "plugin_converter.donefound", "\u7d42\u4e86 - \u767a\u898b\u3057\u305f\u30a2\u30d7\u30ec\u30c3\u30c8:  "},
	{ "plugin_converter.seetrace", "\u30a8\u30e9\u30fc\u304c\u767a\u751f\u3057\u305f\u30d5\u30a1\u30a4\u30eb - \u4ee5\u4e0b\u306e\u30c8\u30ec\u30fc\u30b9\u60c5\u5831\u3092\u53c2\u7167\u3057\u3066\u304f\u3060\u3055\u3044"},
	{ "plugin_converter.noapplet", "\u30d5\u30a1\u30a4\u30eb\u306b\u30a2\u30d7\u30ec\u30c3\u30c8\u304c\u3042\u308a\u307e\u305b\u3093 - "},
	{ "plugin_converter.nofiles", "\u51e6\u7406\u3059\u3079\u304d\u30d5\u30a1\u30a4\u30eb\u306f\u3042\u308a\u307e\u305b\u3093 "},
	{ "plugin_converter.nobackuppath", "\u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u306e\u30d1\u30b9\u3092\u4f5c\u6210\u3057\u307e\u305b\u3093\u3067\u3057\u305f"},
	{ "plugin_converter.writelog", "\u540c\u3058\u540d\u524d\u306e\u30ed\u30b0\u30d5\u30a1\u30a4\u30eb\u3092\u4e0a\u66f8\u304d\u3057\u307e\u3059"},
	{ "plugin_converter.backup_path", "\u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u306e\u30d1\u30b9"},
	{ "plugin_converter.log_path", "\u30ed\u30b0\u30d5\u30a1\u30a4\u30eb\u306e\u30d1\u30b9"},
	{ "plugin_converter.template_file", "\u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb"},
	{ "plugin_converter.process_subdirs", "\u30b5\u30d6\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u51e6\u7406"},
	{ "plugin_converter.show_progress", "\u72b6\u6cc1\u3092\u8868\u793a"},
	{ "plugin_converter.write_permission", "\u73fe\u5728\u306e\u4f5c\u696d\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u306b\u306f\u66f8\u304d\u8fbc\u307f\u6a29\u304c\u5fc5\u8981\u3067\u3059"},
	{ "plugin_converter.overwrite", "\u4e00\u6642\u30d5\u30a1\u30a4\u30eb .tmpSource_stdin \u306f\u3059\u3067\u306b\u5b58\u5728\u3057\u307e\u3059\u3002\u524a\u9664\u3059\u308b\u304b\u540d\u524d\u3092\u5909\u66f4\u3057\u3066\u304f\u3060\u3055\u3044\u3002"},
	{ "plugin_converter.help_message", newline + 
	                                  "\u4f7f\u3044\u65b9: HtmlConverter [-option1 value1 [-option2 value2 [...]]] [-simulate]  [filespecs]" + newline + newline +
	                                  "\u30aa\u30d7\u30b7\u30e7\u30f3\u306b\u306f\u6b21\u306e\u3082\u306e\u304c\u3042\u308a\u307e\u3059:" + newline + newline +
	                                  "    -source:    \u30aa\u30ea\u30b8\u30ca\u30eb\u30d5\u30a1\u30a4\u30eb\u3092\u53d6\u5f97\u3059\u308b\u30d1\u30b9\u3002\u30c7\u30d5\u30a9\u30eb\u30c8: <userdir>" + newline +
                                          "    -source -:  \u5909\u63db\u3059\u308b\u30d5\u30a1\u30a4\u30eb\u3092\u6a19\u6e96\u5165\u529b\u304b\u3089\u8aad\u307f\u8fbc\u3080" + newline + 
	                                  "    -dest:      \u5909\u63db\u3057\u305f\u30d5\u30a1\u30a4\u30eb\u3092\u66f8\u304d\u51fa\u3059\u30d1\u30b9\u3002\u30c7\u30d5\u30a9\u30eb\u30c8: <userdir>" + newline + 
                                          "    -dest -:    \u5909\u63db\u3057\u305f\u30d5\u30a1\u30a4\u30eb\u3092\u6a19\u6e96\u51fa\u529b\u306b\u66f8\u304d\u51fa\u3059" + newline +
	                                  "    -backup:    \u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u30d5\u30a1\u30a4\u30eb\u3092\u66f8\u304d\u51fa\u3059\u30d1\u30b9\u3002\u30c7\u30d5\u30a9\u30eb\u30c8: <dirname>_BAK" + newline +
                                          "    -f:         \u30d0\u30c3\u30af\u30a2\u30c3\u30d7\u30d5\u30a1\u30a4\u30eb\u3092\u5f37\u5236\u7684\u306b\u4e0a\u66f8\u304d\u3059\u308b" + newline +
	                                  "    -subdirs:   \u30b5\u30d6\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u5185\u306e\u30d5\u30a1\u30a4\u30eb\u3092\u51e6\u7406\u3059\u308b" + newline +
	                                  "    -template:  \u30c6\u30f3\u30d7\u30ec\u30fc\u30c8\u30d5\u30a1\u30a4\u30eb\u306e\u30d1\u30b9\u3002\u6307\u5b9a\u306a\u3057\u306e\u5834\u5408\u306f\u30c7\u30d5\u30a9\u30eb\u30c8" + newline +
	                                  "    -log:       \u30ed\u30b0\u30d5\u30a1\u30a4\u30eb\u306e\u30d1\u30b9\u3002\u6307\u5b9a\u306a\u3057\u306e\u5834\u5408\u306f\u30ed\u30b0\u306f\u751f\u6210\u3055\u308c\u306a\u3044" + newline +
	                                  "    -progress:  \u5909\u63db\u4e2d\u306b\u72b6\u6cc1\u3092\u8868\u793a\u3002\u30c7\u30d5\u30a9\u30eb\u30c8: false" + newline +
	                                  "    -simulate:  \u5b9f\u884c\u305b\u305a\u306b\u5909\u63db\u306e\u8a73\u7d30\u3092\u8868\u793a" + newline + 
	                                  "    -latest:    \u516c\u958b MIME \u30bf\u30a4\u30d7\u3092\u30b5\u30dd\u30fc\u30c8\u3059\u308b\u6700\u65b0\u306e JRE \u3092\u4f7f\u7528\u3059\u308b" + newline + 
                                          "    -gui:       GUI \u30e2\u30fc\u30c9" + newline + newline +
	                                  "    filespecs:  \u30b9\u30da\u30fc\u30b9\u3067\u533a\u5207\u3089\u308c\u305f\u30d5\u30a1\u30a4\u30eb\u30ea\u30b9\u30c8\u3002\u30c7\u30d5\u30a9\u30eb\u30c8: \"*.html *.htm\" (\u8981\u5f15\u7528\u7b26)" + newline},
	
	{ "product_name", "Java(TM) Plug-in HTML Converter" },
    };
}

