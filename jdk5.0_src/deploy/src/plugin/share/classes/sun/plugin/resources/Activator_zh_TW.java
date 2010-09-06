/*
 * @(#)Activator_zh_TW.java	1.61 04/06/11 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Traditional Chinese verison of Activator strings.
 *
 * @author Graham Hamilton
 */

public class Activator_zh_TW extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "loading", "\u8f09\u5165 {0} ..." },
	{ "java_applet", "Java Applet" },
	{ "failed", "\u8f09\u5165 Java Applet \u5931\u6557..." },
        { "image_failed", "\u5275\u5efa\u7528\u6236\u5b9a\u7fa9\u7684\u5716\u50cf\u5931\u6557\u3002\u6aa2\u67e5\u5716\u50cf\u6a94\u6848\u540d\u3002" },

	{ "java_not_enabled", "Java \u672a\u88ab\u555f\u7528" },
	{ "exception", "\u4f8b\u5916\uff1a{0}" },

	{ "bean_code_and_ser", "Bean \u7121\u6cd5\u540c\u6642\u5b9a\u7fa9 CODE \u548c JAVA_OBJECT" },
	{ "status_applet", "Applet {0} {1}" },

	// Resources associated with SecurityManager print Dialog:
	{ "print.caption", "\u9700\u8981\u78ba\u8a8d  -  \u5217\u5370" },
	{ "print.message", new String[]{
		"<html><b>\u300c\u5217\u5370\u8981\u6c42\u300d</b></html>Applet \u60f3\u8981\u5217\u5370\u3002\u60a8\u60f3\u8981\u7e7c\u7e8c\u55ce\uff1f"}},
	{ "print.checkBox", "\u4e0d\u518d\u986f\u793a\u6b64\u5c0d\u8a71\u65b9\u584a" }, 
	{ "print.buttonYes", "\u662f(Y)" }, 
	{ "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)}, 
	{ "print.buttonNo", "\u5426(N)" }, 
	{ "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)}, 
		
	{ "optpkg.cert_expired", "<html><b>\u8b49\u66f8\u5df2\u5230\u671f</b></html>\u9078\u7528\u7684\u5957\u4ef6\u5b89\u88dd\u5df2\u4e2d\u6b62\u3002\n" },
	{ "optpkg.cert_notyieldvalid", "<html><b>\u8b49\u66f8\u7121\u6548</b></html>\u9078\u7528\u7684\u5957\u4ef6\u5b89\u88dd\u5df2\u4e2d\u6b62\u3002\n" },
	{ "optpkg.cert_notverify", "<html><b>\u8b49\u66f8\u672a\u7d93\u9a57\u8b49</b></html>\u9078\u7528\u7684\u5957\u4ef6\u5b89\u88dd\u5df2\u4e2d\u6b62\u3002\n" },
	{ "optpkg.general_error", "<html><b>\u4e00\u822c\u7570\u5e38\u72c0\u6cc1</b></html>\u9078\u7528\u7684\u5957\u4ef6\u5b89\u88dd\u5df2\u4e2d\u6b62\u3002\n" },
	{ "optpkg.caption", "\u8b66\u544a - \u9078\u7528\u7684\u5957\u4ef6" },
	{ "optpkg.installer.launch.wait", "<html><b>\u5b89\u88dd\u9078\u7528\u7684\u5957\u4ef6</b></html>\u82e5\u9078\u7528\u7684\u5957\u4ef6\u5b89\u88dd\u7a0b\u5f0f\u5df2\u5b58\u5728\uff0c\u6309\u4e00\u4e0b\u78ba\u5b9a\uff0c\u7e7c\u7e8c\u8f09\u5165 Applet\u3002\n" },
	{ "optpkg.installer.launch.caption", "\u5b89\u88dd\u9032\u884c\u4e2d - \u9078\u7528\u7684\u5957\u4ef6"},
	{ "optpkg.prompt_user.new_spec.text", "<html><b>\u4e0b\u8f09\u8981\u6c42</b></html>Applet \u9700\u8981\u66f4\u65b0\u7684\u7248\u672c\uff08\u4f86\u81ea {2} \u7684\u9078\u7528\u5957\u4ef6 \"{1}\" \u7684\u898f\u683c {0} \n\n \u60a8\u8981\u7e7c\u7e8c\u55ce\uff1f" },
	{ "optpkg.prompt_user.new_impl.text", "<html><b>\u4e0b\u8f09\u8981\u6c42</b></html>Applet \u9700\u8981 {2} \u7684 \u9078\u7528\u5957\u4ef6 \"{1}\" \u7684\u66f4\u65b0\u7684\u7248\u672c\uff08\u5be6\u4f5c {0}\uff09\n\n\u60a8\u8981\u7e7c\u7e8c\u55ce\uff1f" },
	{ "optpkg.prompt_user.new_vendor.text", "<html><b>\u4e0b\u8f09\u8981\u6c42</b></html>Applet \u9700\u8981 {3} \u7684\u9078\u7528\u7684\u5957\u4ef6 \"{1}\"  {2} \u7684 ({0})  \n\n\u60a8\u8981\u7e7c\u7e8c\u55ce\uff1f" },
	{ "optpkg.prompt_user.default.text", "<html><b>\u4e0b\u8f09\u8981\u6c42</b></html>Applet \u9700\u8981 {1} \u7684\u9078\u7528\u5957\u4ef6 \"{0}\" \u7684\u5b89\u88dd \n\n\u60a8\u8981\u7e7c\u7e8c\u55ce\uff1f" },

	{ "cache.error.text", "<html><b>\u5feb\u53d6\u932f\u8aa4</b></html>\u7121\u6cd5\u5728\u5feb\u53d6\u4e2d\u5132\u5b58\u6216\u66f4\u65b0\u6a94\u6848\u3002" },
	{ "cache.error.caption", "\u932f\u8aa4 - \u5feb\u53d6" },
	{ "cache.version_format_error", "{0} \u4e0d\u662f xxxx.xxxx.xxxx.xxxx \u683c\u5f0f\uff0c\u5176\u4e2d\u7684 x \u70ba\u5341\u516d\u9032\u4f4d\u6578\u5b57\u3002" },
 	{ "cache.version_attrib_error", "\u5728 \'cache_archive\' \u4e2d\u6307\u5b9a\u7684\u5c6c\u6027\u6578\u76ee\u8207 \'cache_version\' \u4e2d\u7684\u4e0d\u76f8\u7b26\u3002" },
	{ "cache.header_fields_missing", "\u4e0d\u80fd\u5f97\u5230\u6700\u8fd1\u4fee\u6539\u7684\u6642\u9593\u548c/\u6216\u6709\u6548\u671f\u3002Jar\u6587\u4ef6\u5c07\u4e0d\u88ab\u5feb\u53d6\u3002" },

 	{ "applet.progress.load", "\u8f09\u5165 Applet \u4e2d..." },
 	{ "applet.progress.init", "\u6b63\u5728\u8d77\u59cb\u8a2d\u5b9a Applet..." },
 	{ "applet.progress.start", "\u6b63\u5728\u555f\u52d5 Applet..." },
 	{ "applet.progress.stop", "\u6b63\u5728\u505c\u6b62 Applet..." },
 	{ "applet.progress.destroy", "\u6b63\u5728\u92b7\u6bc0 Applet..." },
 	{ "applet.progress.dispose", "\u6b63\u5728\u68c4\u7f6e Applet..." },
 	{ "applet.progress.quit", "\u6b63\u5728\u9000\u51fa Applet..." },
 	{ "applet.progress.stoploading", "\u5df2\u505c\u6b62\u8f09\u5165..." },
 	{ "applet.progress.interrupted", "\u4e2d\u65b7\u7684\u57f7\u884c\u7dd2..." },
        { "applet.progress.joining", "\u6b63\u5728\u5408\u4f75 Applet \u57f7\u884c\u7dd2..." },
        { "applet.progress.joined", "\u5df2\u5408\u4f75 Applet \u57f7\u884c\u7dd2..." },
 	{ "applet.progress.loadImage", "\u6b63\u5728\u8f09\u5165\u5f71\u50cf " },
 	{ "applet.progress.loadAudio", "\u6b63\u5728\u8f09\u5165\u97f3\u8a0a " },
 	{ "applet.progress.findinfo.0", "\u6b63\u5728\u5c0b\u627e\u8cc7\u8a0a..." },
 	{ "applet.progress.findinfo.1", "\u5df2\u5b8c\u6210..." },
 	{ "applet.progress.timeout.wait", "\u7b49\u5f85\u903e\u6642\u4e2d..." },
	{ "applet.progress.findinfo.1", "\u5df2\u5b8c\u6210..." },
	{ "applet.progress.timeout.wait", "\u7b49\u5f85\u903e\u6642\u4e2d..." },
	{ "applet.progress.timeout.jointing", "\u6b63\u5728\u57f7\u884c\u4e00\u9805\u5408\u4f75..." },
	{ "applet.progress.timeout.jointed", "\u5df2\u5b8c\u6210\u5408\u4f75 ..." },

	{ "modality.register", "\u5df2\u767b\u9304\u5f62\u5f0f\u63a5\u6536\u7a0b\u5f0f" },
	{ "modality.unregister", "\u672a\u767b\u9304\u7684\u5f62\u5f0f\u5075\u807d\u7a0b\u5f0f" },
	{ "modality.pushed", "\u5f62\u5f0f\u5df2\u63a8\u5c55" },
	{ "modality.popped", "\u5f62\u5f0f\u5df2\u5448\u73fe" },

 	{ "progress.listener.added", "\u65b0\u589e\u9032\u5ea6\u5075\u807d\u7a0b\u5f0f\uff1a {0}" },
 	{ "progress.listener.removed", "\u79fb\u9664\u9032\u5ea6\u5075\u807d\u7a0b\u5f0f\uff1a {0}" },

 	{ "liveconnect.UniversalBrowserRead.enabled", "JavaScript\uff1aUniversalBrowserRead \u5df2\u555f\u7528" },
 	{ "liveconnect.java.system", "JavaScript\uff1a\u547c\u53eb Java \u7cfb\u7d71\u7a0b\u5f0f\u78bc" },
 	{ "liveconnect.same.origin", "JavaScript\uff1a\u547c\u53eb\u7a0b\u5f0f\u8207\u88ab\u547c\u53eb\u7684\u7a0b\u5f0f\u5177\u6709\u76f8\u540c\u7684\u767c\u7aef" },
 	{ "liveconnect.default.policy", "JavaScript\uff1a\u9810\u8a2d\u7684\u5b89\u5168\u539f\u5247 = {0}" },
 	{ "liveconnect.UniversalJavaPermission.enabled", "JavaScript\uff1aUniversalJavaPermission \u5df2\u555f\u7528" },
 	{ "liveconnect.wrong.securitymodel", "Netscape \u5b89\u5168\u6a21\u578b\u5df2\u4e0d\u518d\u53d7\u652f\u63f4\u3002\n"
 					     + "\u8acb\u79fb\u8f49\u81f3 Java 2 \u5b89\u5168\u6a21\u578b\u3002\n" },

	{ "pluginclassloader.created_files", "\u5728\u5feb\u53d6\u4e2d\u5275\u5efa {0}\u3002" },
        { "pluginclassloader.deleting_files", "\u522a\u9664\u5feb\u53d6\u4e2d\u7684 JAR \u6a94\u3002" },
        { "pluginclassloader.file", "   \u5f9e\u5feb\u53d6 {0} \u4e2d\u522a\u9664 " },
        { "pluginclassloader.empty_file", "{0} \u662f\u7a7a\u7684\uff0c\u5f9e\u5feb\u53d6\u4e2d\u522a\u9664\u3002" },
        
 	{ "appletcontext.audio.loaded", "\u5df2\u8f09\u5165\u97f3\u6548\u7247\u6bb5\uff1a {0}" },
 	{ "appletcontext.image.loaded", "\u5df2\u8f09\u5165\u5f71\u50cf\uff1a {0}"},

 	{ "securitymgr.automation.printing", "\u81ea\u52d5\u5316\uff1a\u63a5\u53d7\u5217\u5370" },

	{ "classloaderinfo.referencing", "\u53c3\u8003\u985e\u5225\u8f09\u5165\u5668: {0}\uff0crefcount={1}" },
	{ "classloaderinfo.releasing", "\u91cb\u653e\u985e\u5225\u8f09\u5165\u5668: {0}\uff0crefcount={1}" },
 	{ "classloaderinfo.caching", "\u5feb\u53d6\u985e\u5225\u8f09\u5165\u5668\uff1a {0}" },
 	{ "classloaderinfo.cachesize", "\u73fe\u884c\u7684\u985e\u5225\u8f09\u5165\u5668\u5feb\u53d6\u5927\u5c0f\uff1a {0}" },
	{ "classloaderinfo.num", "\u5feb\u53d6\u985e\u5225\u8f09\u5165\u5668\u7684\u6578\u76ee\u5df2\u8d85\u904e {0}\uff0c\u672a\u53c3\u8003 {1}" },
        { "jsobject.call", "JSObject::call: name={0}" },
        { "jsobject.eval", "JSObject::eval({0})" },
        { "jsobject.getMember", "JSObject::getMember: name={0}" },
        { "jsobject.setMember", "JSObject::setMember: name={0}" },
        { "jsobject.removeMember", "JSObject::removeMember: name={0}" },
        { "jsobject.getSlot", "JSObject::getSlot: {0}" },
        { "jsobject.setSlot", "JSObject::setSlot: slot={0}" },
	{ "jsobject.invoke.url.permission", "Applet \u7684 URL \u70ba {0} \u4e26\u4e14\u6b0a\u9650 = {1}"},

 	{ "optpkg.install.info", "\u6b63\u5728\u5b89\u88dd\u9078\u7528\u7684\u5957\u88dd\u8edf\u9ad4 {0}" },
 	{ "optpkg.install.fail", "\u9078\u7528\u7684\u5957\u88dd\u8edf\u9ad4\u5b89\u88dd\u5df2\u5931\u6557\u3002" },
 	{ "optpkg.install.ok", "\u9078\u7528\u7684\u5957\u88dd\u8edf\u9ad4\u5b89\u88dd\u5df2\u6210\u529f\u3002" },
 	{ "optpkg.install.automation", "\u81ea\u52d5\u5316\uff1a\u63a5\u53d7\u9078\u7528\u7684\u5957\u88dd\u8edf\u9ad4\u5b89\u88dd" },
 	{ "optpkg.install.granted", "\u4f7f\u7528\u8005\u5df2\u6388\u6b0a\u4e0b\u8f09\u9078\u7528\u7684\u5957\u88dd\u8edf\u9ad4\uff0c\u4e0b\u8f09\u4f86\u6e90\uff1a {0}" },
 	{ "optpkg.install.deny", "\u4f7f\u7528\u8005\u672a\u6388\u6b0a\u4e0b\u8f09\u9078\u7528\u7684\u5957\u88dd\u8edf\u9ad4" },
 	{ "optpkg.install.begin", "\u6b63\u5728\u5b89\u88dd {0}" },
 	{ "optpkg.install.java.launch", "\u555f\u52d5 Java Installer" },
 	{ "optpkg.install.java.launch.command", "\u900f\u904e ''{0}'' \u555f\u52d5 Java Installer" },
 	{ "optpkg.install.native.launch", "\u555f\u52d5\u539f\u751f\u7684 Installer" },
	{ "optpkg.install.native.launch.fail.0", "\u7121\u6cd5\u57f7\u884c {0}" },
	{ "optpkg.install.native.launch.fail.1", "\u5b58\u53d6 {0} \u5931\u6557" },
	{ "optpkg.install.raw.launch", "\u6b63\u5728\u5b89\u88dd\u539f\u59cb\u7684\u9078\u7528\u5957\u88dd\u8edf\u9ad4" },
	{ "optpkg.install.raw.copy", "\u5c07\u300c\u539f\u59cb\u9078\u7528\u7684\u5957\u88dd\u8edf\u9ad4\u300d\u5f9e {0} \u8907\u88fd\u5230 {1}" },
	{ "optpkg.install.error.nomethod", "\u672a\u5b89\u88dd Dependent Extension Provider\uff1a\u7121\u6cd5\u53d6\u5f97 "
				         + " addExtensionInstallationProvider \u65b9\u6cd5" },
	{ "optpkg.install.error.noclass", "\u672a\u5b89\u88dd Dependent Extension Provider\uff1a\u7121\u6cd5\u53d6\u5f97 "
					 + "sun.misc.ExtensionDependency \u985e\u5225" },


	{"progress_dialog.downloading", "Plug-in\uff1a\u4e0b\u8f09\u4e2d..."},
        {"progress_dialog.dismiss_button", "\u95dc\u9589(D)"},
        {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
         {"progress_dialog.from", "\u5f9e"},

         {"applet_viewer.color_tag", "{0} \u7684\u5143\u4ef6\u6578\u4e0d\u6b63\u78ba"},
                
        {"progress_info.downloading", "\u6b63\u5728\u4e0b\u8f09 JAR \u6a94"},
        {"progress_bar.preload", "\u6b63\u5728\u9810\u5148\u8f09\u5165 JAR \u6a94\uff1a {0}"},
        
        {"cache.size", "\u5feb\u53d6\u5927\u5c0f: {0}"},
        {"cache.cleanup", "\u5feb\u53d6\u7684\u5927\u5c0f\u5df2\u9054: {0} \u500b\u4f4d\u5143\u7d44\uff0c\u5fc5\u9808\u57f7\u884c\u6e05\u9664\u4f5c\u696d"},
        {"cache.full", "\u5feb\u53d6\u5df2\u6eff: \u6b63\u5728\u522a\u9664\u6a94\u6848 {0}"},
        {"cache.inuse", "\u7531\u65bc\u9019\u500b\u61c9\u7528\u7a0b\u5f0f\u4ecd\u5728\u4f7f\u7528\u6a94\u6848 {0}\uff0c\u6240\u4ee5\u7121\u6cd5\u522a\u9664\u9019\u500b\u6a94\u6848"},
        {"cache.notdeleted", "\u7121\u6cd5\u522a\u9664\u6a94\u6848 {0}\uff0c\u53ef\u80fd\u662f\u6b64\u61c9\u7528\u7a0b\u5f0f\u548c\uff08\u6216\uff09\u5176\u4ed6\u7684\u61c9\u7528\u7a0b\u5f0f\u4ecd\u5728\u4f7f\u7528\u9019\u500b\u6a94\u6848"},
        {"cache.out_of_date", "{0} \u7684\u5feb\u53d6\u8907\u672c\u5df2\u904e\u6642\n \u5feb\u53d6\u7684\u526f\u672c\u70ba: {1}\n \u4f3a\u670d\u5668\u7684\u526f\u672c\u70ba: {2}"},
        {"cache.loading", "\u6b63\u5728\u5f9e\u5feb\u53d6\u4e2d\u8f09\u5165 {0}"},
        {"cache.cache_warning", "\u8b66\u544a: \u7121\u6cd5\u5feb\u53d6 {0}"},
        {"cache.downloading", "\u6b63\u5728\u5c07 {0} \u4e0b\u8f09\u81f3\u5feb\u53d6\u4e2d"},
        {"cache.cached_name", "\u5df2\u5feb\u53d6\u7684\u6a94\u6848\u540d\u7a31: {0}"},
        {"cache.load_warning", "\u8b66\u544a\uff1a\u5f9e\u5feb\u53d6\u4e2d\u8b80\u53d6 {0} \u6642\u767c\u751f\u932f\u8aa4\u3002"},
        {"cache.disabled", "\u4f7f\u7528\u8005\u5df2\u505c\u7528\u5feb\u53d6"},
        {"cache.minSize", "\u5feb\u53d6\u5df2\u505c\u7528\uff0c\u5feb\u53d6\u4e0a\u9650\u8a2d\u5b9a\u70ba {0}\uff0c\u81f3\u5c11\u9700\u6307\u5b9a 5 MB"},
        {"cache.directory_warning", "\u8b66\u544a: {0} \u4e0d\u662f\u4e00\u500b\u76ee\u9304; \u5c07\u505c\u7528\u5feb\u53d6\u529f\u80fd\u3002"},
        {"cache.response_warning", "\u8b66\u544a: {1} \u767c\u751f\u4e86\u975e\u9810\u671f\u7684\u56de\u61c9 {0}; \u5c07\u6703\u91cd\u65b0\u4e0b\u8f09\u6a94\u6848\u3002"},
        {"cache.enabled", "\u5df2\u555f\u7528\u5feb\u53d6"},
        {"cache.location", "\u4f4d\u7f6e: {0}"},
        {"cache.maxSize", "\u5927\u5c0f\u4e0a\u9650: {0}"},
        {"cache.create_warning", "\u8b66\u544a: \u7121\u6cd5\u5efa\u7acb\u5feb\u53d6\u76ee\u9304 {0}; \u5c07\u505c\u7528\u5feb\u53d6\u529f\u80fd\u3002"},
        {"cache.read_warning", "\u8b66\u544a: \u7121\u6cd5\u8b80\u53d6\u5feb\u53d6\u76ee\u9304 {0}; \u5c07\u505c\u7528\u5feb\u53d6\u529f\u80fd\u3002"},
        {"cache.write_warning", "\u8b66\u544a: \u7121\u6cd5\u5beb\u5165\u81f3\u5feb\u53d6\u76ee\u9304 {0}; \u5c07\u505c\u7528\u5feb\u53d6\u529f\u80fd\u3002"},
        {"cache.compression", "\u58d3\u7e2e\u5c64\u7d1a: {0}"},
        {"cache.cert_load", "\u5df2\u5f9e JAR \u5feb\u53d6\u4e2d\u8b80\u53d6 {0} \u7684\u8b49\u66f8"},
	{"cache.jarjar.invalid_file", ".jarjar \u6a94\u6848\u5167\u542b\u975e .jar \u7684\u6a94\u6848"},
	{"cache.jarjar.multiple_jar", ".jarjar \u6a94\u6848\u5167\u542b\u4e00\u500b\u4ee5\u4e0a\u7684 .jar \u6a94\u6848"},
        {"cache.version_checking", "\u6aa2\u67e5 {0} \u7684\u7248\u672c\uff0c\u6240\u6307\u5b9a\u7684\u7248\u672c\u70ba {1}"},
        { "cache.preloading", "\u6b63\u5728\u9810\u5148\u8f09\u5165\u6a94\u6848 {0}"},

	{ "cache_viewer.caption", "Java Applet \u5feb\u53d6\u6aa2\u8996\u5668" },
	{ "cache_viewer.refresh", "\u66f4\u65b0(R)" },
	{ "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_R) },
 	{ "cache_viewer.remove", "\u522a\u9664(D)" },
	{ "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_D) },
	{ "cache_viewer.OK", "\u78ba\u5b9a(O)" },
	{ "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_O) },
 	{ "cache_viewer.name", "\u540d\u7a31" },
	{ "cache_viewer.type", "\u985e\u578b" },
	{ "cache_viewer.size", "\u5927\u5c0f" },
	{ "cache_viewer.modify_date", "\u6700\u5f8c\u66f4\u65b0" },
	{ "cache_viewer.expiry_date", "\u5230\u671f\u65e5\u671f" },
	{ "cache_viewer.url", "URL" },
 	{ "cache_viewer.version", "\u7248\u672c" },
	{ "cache_viewer.help.name", "\u5feb\u53d6\u7684\u6a94\u6848\u540d\u7a31" },
	{ "cache_viewer.help.type", "\u5feb\u53d6\u7684\u6a94\u6848\u985e\u578b" },
	{ "cache_viewer.help.size", "\u5feb\u53d6\u7684\u6a94\u6848\u5927\u5c0f" },
	{ "cache_viewer.help.modify_date", "\u5feb\u53d6\u6a94\u6848\u6700\u5f8c\u7684\u4fee\u6539\u65e5\u671f" },
	{ "cache_viewer.help.expiry_date", "\u5feb\u53d6\u6a94\u6848\u5230\u671f\u65e5\u671f" },
	{ "cache_viewer.help.url", "\u5feb\u53d6\u6a94\u6848\u4e0b\u8f09 URL" },
	{ "cache_viewer.help.version", "\u5feb\u53d6\u7684\u6a94\u6848\u7248\u672c" },
	{ "cache_viewer.delete.text", "<html><b>\u6a94\u6848\u5c1a\u672a\u522a\u9664</b></html>{0} \u53ef\u80fd\u6b63\u5728\u4f7f\u7528\u4e2d\u3002\n" },
	{ "cache_viewer.delete.caption", "\u932f\u8aa4 - \u5feb\u53d6" },
	{ "cache_viewer.type.zip", "Jar" },
	{ "cache_viewer.type.class", "\u985e\u5225" },
	{ "cache_viewer.type.wav", "Wav \u8072\u97f3" },
	{ "cache_viewer.type.au", "Au \u8072\u97f3" },
	{ "cache_viewer.type.gif", "Gif \u5f71\u50cf" },
	{ "cache_viewer.type.jpg", "Jpeg \u5f71\u50cf" },
	{ "cache_viewer.menu.file", "\u6a94\u6848(F)" },
	{ "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_F) },
	{ "cache_viewer.menu.options", "\u9078\u9805(P)" },
	{ "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_P) },
	{ "cache_viewer.menu.help", "\u8f14\u52a9\u8aaa\u660e(H)" },
	{ "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_H) },
	{ "cache_viewer.menu.item.exit", "\u95dc\u9589(X)" },
	{ "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_X) },
	{ "cache_viewer.disable", "\u555f\u7528\u5feb\u53d6(N)" },
	{ "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_N) },
	{ "cache_viewer.menu.item.about", "\u95dc\u65bc(A)" },
	{ "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_A) },

	{ "net.proxy.auto.result.error", "\u7121\u6cd5\u5f9e\u8a55\u4f30\u4f86\u6c7a\u5b9a Proxy \u8a2d\u5b9a - \u5931\u6548\u6298\u8fd4\u6210 DIRECT"},

	{ "lifecycle.applet.found", "\u5f9e\u751f\u547d\u9031\u671f\u5feb\u53d6\u627e\u5230\u4e4b\u524d\u505c\u6b62\u7684 Applet" },
	{ "lifecycle.applet.support", "Applet \u652f\u63f4\u7e7c\u627f\u7684\u751f\u547d\u9031\u671f\u6a21\u5f0f - \u65b0\u589e Applet \u81f3\u751f\u547d\u9031\u671f\u5feb\u53d6" },
	{ "lifecycle.applet.cachefull", "\u751f\u547d\u9031\u671f\u5feb\u53d6\u5df2\u6eff - \u8acb\u522a\u6539\u6700\u8fd1\u6700\u4e0d\u5e38\u7528\u7684 Applet" },
	{ "com.method.ambiguous", "\u4e0d\u80fd\u9078\u64c7\u65b9\u6cd5\uff0c\u4e0d\u660e\u53c3\u6578" },
        { "com.method.notexists", "{0} :\u6b64\u65b9\u6cd5\u4e0d\u5b58\u5728" },
        { "com.notexists", "{0} :\u6b64\u65b9\u6cd5/\u5c6c\u6027\u4e0d\u5b58\u5728" },
        { "com.method.invoke", "\u547c\u53eb\u65b9\u6cd5: {0}" },
        { "com.method.jsinvoke", "\u547c\u53eb JS \u65b9\u6cd5: {0}" },
        { "com.method.argsTypeInvalid", "\u4e0d\u80fd\u8f49\u63db\u53c3\u6578\u5230\u6240\u9700\u985e\u578b" },
        { "com.method.argCountInvalid", "\u53c3\u6578\u6578\u76ee\u4e0d\u5c0d" },
        { "com.field.needsConversion", "\u9700\u8981\u8f49\u63db: {0} --> {1}" },
        { "com.field.typeInvalid", " \u4e0d\u80fd\u88ab\u8f49\u63db\u5230\u4ee5\u4e0b\u985e\u578b: {0}" },
        { "com.field.get", "\u53d6\u5f97\u5c6c\u6027: {0}" },
        { "com.field.set", "\u8a2d\u5b9a\u5c6c\u6027: {0}" },

	{ "rsa.cert_expired", "<html><b>\u8b49\u66f8\u5df2\u5230\u671f</b></html>\u6b64\u78bc\u5c07\u88ab\u8996\u70ba\u672a\u7c3d\u7f72\u3002\n" },
	{ "rsa.cert_notyieldvalid", "<html><b>\u8b49\u66f8\u7121\u6548</b></html>\u6b64\u78bc\u5c07\u88ab\u8996\u70ba\u672a\u7c3d\u7f72\u3002\n" },
	{ "rsa.general_error", "<html><b>\u8b49\u66f8\u672a\u7d93\u9a57\u8b49</b></html>\u6b64\u78bc\u5c07\u88ab\u8996\u70ba\u672a\u7c3d\u7f72\u3002\n" },

	{ "dialogfactory.menu.show_console", "\u986f\u793a Java \u4e3b\u63a7\u53f0" },
	{ "dialogfactory.menu.hide_console", "\u96b1\u85cf Java \u4e3b\u63a7\u53f0" },
	{ "dialogfactory.menu.about", "\u95dc\u65bc Java Plug-in" },
	{ "dialogfactory.menu.copy", "\u8907\u88fd" },
	{ "dialogfactory.menu.open_console", "\u958b\u555f Java \u4e3b\u63a7\u53f0" },
	{ "dialogfactory.menu.about_java", "\u95dc\u65bc Java(TM)" },
    };
}

  
