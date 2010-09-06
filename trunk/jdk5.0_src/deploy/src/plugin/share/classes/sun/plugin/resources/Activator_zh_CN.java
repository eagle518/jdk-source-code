/*
 * @(#)Activator_zh_CN.java	1.59 04/06/11 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved. 
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Simplified Chinese version of Activator strings.
 *
 * @author Graham Hamilton
 */

public class Activator_zh_CN extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "loading", "\u6b63\u5728\u8f7d\u5165 {0}... " },
	{ "java_applet", "Java \u5c0f\u5e94\u7528\u7a0b\u5e8f" },
	{ "failed", "\u8f7d\u5165 Java \u5c0f\u5e94\u7528\u7a0b\u5e8f\u5931\u8d25..." },
	{ "image_failed", "\u521b\u5efa\u7528\u6237\u5b9a\u4e49\u7684\u56fe\u8c61\u5931\u8d25\u3002\u8bf7\u68c0\u67e5\u56fe\u8c61\u7684\u6587\u4ef6\u540d\u3002" },
	{ "java_not_enabled", "Java \u6ca1\u6709\u542f\u7528" },
	{ "exception", "\u5f02\u5e38\uff1a{0}" },
        


	{ "bean_code_and_ser", "Bean \u4e0d\u80fd\u540c\u65f6\u5b9a\u4e49 CODE \u548c JAVA_OBJECT " },	
        { "status_applet", "\u5c0f\u5e94\u7528\u7a0b\u5e8f {0} {1}" },	

	// Resources associated with SecurityManager print Dialog:
	{ "print.caption", "\u9700\u8981\u786e\u8ba4 - \u6253\u5370" },
	{ "print.message", new String[]{
	        "<html><b>\u6253\u5370\u8bf7\u6c42</b></html>\u5c0f\u5e94\u7528\u7a0b\u5e8f\u9700\u8981\u6253\u5370\u3002\u8981\u7ee7\u7eed\u5417\uff1f"}},
	{ "print.checkBox", "\u4e0d\u518d\u663e\u793a\u6b64\u5bf9\u8bdd\u6846" },
	{ "print.buttonYes", "\u662f(Y)" },
	{ "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "print.buttonNo", "\u5426(N)" },
	{ "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},

        { "optpkg.cert_expired", "<html><b>\u8bc1\u4e66\u8fc7\u671f</b></html>\u53ef\u9009\u7684\u8f6f\u4ef6\u5305\u5b89\u88c5\u4e2d\u6b62\u3002\n" }, 
        { "optpkg.cert_notyieldvalid", "<html><b>\u8bc1\u4e66\u65e0\u6548</b></html>\u53ef\u9009\u8f6f\u4ef6\u5305\u5b89\u88c5\u4e2d\u6b62\u3002\n" }, 
        { "optpkg.cert_notverify", "<html><b>\u8bc1\u4e66\u672a\u7ecf\u6821\u9a8c</b></html>\u53ef\u9009\u8f6f\u4ef6\u5305\u5b89\u88c5\u4e2d\u6b62\u3002\n" }, 
        { "optpkg.general_error", "<html><b>\u4e00\u822c\u5f02\u5e38</b></html>\u53ef\u9009\u8f6f\u4ef6\u5305\u5b89\u88c5\u4e2d\u6b62\u3002\n" }, 
        { "optpkg.caption", "\u8b66\u544a - \u53ef\u9009\u8f6f\u4ef6\u5305" }, 
        { "optpkg.installer.launch.wait", "<html><b>\u5b89\u88c5\u53ef\u9009\u8f6f\u4ef6\u5305</b></html>\u9000\u51fa\u53ef\u9009\u8f6f\u4ef6\u5305\u5b89\u88c5\u7a0b\u5e8f\u540e\uff0c\u5355\u51fb\u201c\u786e\u5b9a\u201d\u4ee5\u7ee7\u7eed\u5c0f\u5e94\u7528\u7a0b\u5e8f\u8f7d\u5165\u3002\n" }, 
        { "optpkg.installer.launch.caption", "\u5b89\u88c5\u8fdb\u884c\u4e2d - \u53ef\u9009\u8f6f\u4ef6\u5305"}, 
        { "optpkg.prompt_user.new_spec.text", "<html><b>\u4e0b\u8f7d\u8bf7\u6c42</b></html>\u5c0f\u5e94\u7528\u7a0b\u5e8f\u8981\u6c42 {2} \u7684\u8f83\u65b0\u7248\u672c (\u89c4\u683c\u4e3a {0}) \u7684\u53ef\u9009\u8f6f\u4ef6\u5305 \"{1}\" \n\n\u8981\u7ee7\u7eed\u5417\uff1f" }, 
        { "optpkg.prompt_user.new_impl.text", "<html><b>\u4e0b\u8f7d\u8bf7\u6c42</b></html>\u5c0f\u5e94\u7528\u7a0b\u5e8f\u8981\u6c42 {2} \u7684\u8f83\u65b0\u7248\u672c(\u5b9e\u73b0 {0}) \u7684\u53ef\u9009\u8f6f\u4ef6\u5305 \"{1}\" \n\n\u8981\u7ee7\u7eed\u5417\uff1f" }, 
        { "optpkg.prompt_user.new_vendor.text", "<html><b>\u4e0b\u8f7d\u8bf7\u6c42</b></html>\u5c0f\u5e94\u7528\u7a0b\u5e8f\u8981\u6c42 {3} \u7684\u53ef\u9009\u8f6f\u4ef6\u5305 \"{1}\" {2} \u7684 ({0})\n\n\u8981\u7ee7\u7eed\u5417\uff1f" }, 
        { "optpkg.prompt_user.default.text", "<html><b>\u4e0b\u8f7d\u8bf7\u6c42</b></html>\u5c0f\u5e94\u7528\u7a0b\u5e8f\u8981\u6c42\u5b89\u88c5 {1} \u7684\u53ef\u9009\u8f6f\u4ef6\u5305 \"{0}\"\n\n\u8981\u7ee7\u7eed\u5417\uff1f" }, 

        { "cache.error.text", "<html><b>\u9ad8\u901f\u7f13\u5b58\u9519\u8bef</b></html>\u4e0d\u80fd\u5b58\u50a8\u6216\u66f4\u65b0\u9ad8\u901f\u7f13\u5b58\u4e2d\u7684\u6587\u4ef6\u3002" }, 
        { "cache.error.caption", "\u9519\u8bef - \u9ad8\u901f\u7f13\u5b58" }, 
        { "cache.version_format_error", "{0} \u7684\u683c\u5f0f\u4e0d\u662f xxxx.xxxx.xxxx.xxxx, \u6b64\u5904 x \u662f\u5341\u516d\u8fdb\u5236\u6570\u5b57" }, 


	{ "cache.version_attrib_error", "\u5728 \'cache_archive\' \u4e2d\u6307\u5b9a\u7684\u5c5e\u6027\u4e2a\u6570\u4e0e\u5728 \'cache_version\' \u4e2d\u7684\u5c5e\u6027\u4e2a\u6570\u4e0d\u5339\u914d" },
	{ "cache.header_fields_missing", "\u4e0d\u80fd\u5f97\u5230\u4e0a\u6b21\u4fee\u6539\u65f6\u95f4\u548c/\u6216\u6709\u6548\u671f\u3002Jar \u6587\u4ef6\u4e0d\u4f1a\u653e\u5165\u9ad8\u901f\u7f13\u5b58\u3002" },


	{ "applet.progress.load", "\u6b63\u5728\u8f7d\u5165\u5c0f\u5e94\u7528\u7a0b\u5e8f..." },
	{ "applet.progress.init", "\u6b63\u5728\u521d\u59cb\u5316\u5c0f\u5e94\u7528\u7a0b\u5e8f..." },
	{ "applet.progress.start", "\u6b63\u5728\u542f\u52a8\u5c0f\u5e94\u7528\u7a0b\u5e8f..." },
	{ "applet.progress.stop", "\u505c\u6b62\u5c0f\u5e94\u7528\u7a0b\u5e8f..." },
	{ "applet.progress.destroy", "\u6b63\u5728\u9500\u6bc1\u5c0f\u5e94\u7528\u7a0b\u5e8f..." },
	{ "applet.progress.dispose", "\u6b63\u5728\u5904\u7f6e\u5c0f\u5e94\u7528\u7a0b\u5e8f..." },
	{ "applet.progress.quit", "\u6b63\u5728\u9000\u51fa\u5c0f\u5e94\u7528\u7a0b\u5e8f..." },
	{ "applet.progress.stoploading", "\u5df2\u505c\u6b62\u8f7d\u5165..." },
	{ "applet.progress.interrupted", "\u5df2\u4e2d\u65ad\u7684\u7ebf\u7a0b..." },
	{ "applet.progress.joining", "\u6b63\u5728\u8fde\u63a5\u5c0f\u5e94\u7528\u7a0b\u5e8f\u7ebf\u7a0b ..." },
	{ "applet.progress.joined", "\u5df2\u8fde\u63a5\u5c0f\u5e94\u7528\u7a0b\u5e8f\u7ebf\u7a0b ..." },
	{ "applet.progress.loadImage", "\u6b63\u5728\u8f7d\u5165\u56fe\u8c61 " },
	{ "applet.progress.loadAudio", "\u6b63\u5728\u8f7d\u5165\u97f3\u9891 " },
	{ "applet.progress.findinfo.0", "\u6b63\u5728\u67e5\u627e\u4fe1\u606f..." },
	{ "applet.progress.findinfo.1", "\u5b8c\u6210..." },
	{ "applet.progress.timeout.wait", "\u7b49\u5f85\u8d85\u65f6..." },
	{ "applet.progress.timeout.jointing", "\u6b63\u5728\u8fdb\u884c\u8054\u5408..." },
	{ "applet.progress.timeout.jointed", "\u5b8c\u6210\u8054\u5408 ..." },

	{ "modality.register", "\u5df2\u6ce8\u518c\u539f\u578b\u76d1\u542c\u7a0b\u5e8f" },
	{ "modality.unregister", "\u672a\u6ce8\u518c\u539f\u578b\u76d1\u542c\u7a0b\u5e8f" },
	{ "modality.pushed", "\u6a21\u6001\u5df2\u63a8\u8fdb" },
	{ "modality.popped", "\u6a21\u6001\u5df2\u5f39\u51fa" },

	{ "progress.listener.added", "\u5df2\u6dfb\u52a0\u8fdb\u5ea6\u76d1\u542c\u7a0b\u5e8f\uff1a{0} " },
	{ "progress.listener.removed", "\u5df2\u5220\u9664\u8fdb\u5ea6\u76d1\u542c\u7a0b\u5e8f\uff1a{0} " },

	{ "liveconnect.UniversalBrowserRead.enabled", "JavaScript: UniversalBrowserRead \u5df2\u542f\u7528" },
	{ "liveconnect.java.system", "JavaScript: \u8c03\u7528 Java \u7cfb\u7edf\u4ee3\u7801" },
	{ "liveconnect.same.origin", "JavaScript: \u8c03\u7528\u7a0b\u5e8f\u548c\u88ab\u8c03\u7528\u7a0b\u5e8f\u6709\u76f8\u540c\u7684\u6e90" },
	{ "liveconnect.default.policy", "JavaScript: \u7f3a\u7701\u5b89\u5168\u7b56\u7565 = {0} " }, 
	{ "liveconnect.UniversalJavaPermission.enabled", "JavaScript: UniversalJavaPermission \u5df2\u542f\u7528" },
	{ "liveconnect.wrong.securitymodel", "\u4e0d\u518d\u652f\u6301 Netscape \u5b89\u5168\u6a21\u5f0f\u3002\n"
					     + "\u8bf7\u79fb\u690d\u5230 Java 2 \u5b89\u5168\u6a21\u5f0f\u3002\n" },

	{ "pluginclassloader.created_files", "\u5df2\u5728\u9ad8\u901f\u7f13\u5b58\u4e2d\u521b\u5efa {0}\u3002" },
	{ "pluginclassloader.deleting_files", "\u6b63\u5728\u4ece\u9ad8\u901f\u7f13\u5b58\u4e2d\u5220\u9664 JAR \u6587\u4ef6\u3002" },
	{ "pluginclassloader.file", "   \u4ece\u9ad8\u901f\u7f13\u5b58 {0} \u4e2d\u5220\u9664" },
	{ "pluginclassloader.empty_file", " {0} \u4e3a\u7a7a\uff0c\u4ece\u9ad8\u901f\u7f13\u5b58\u4e2d\u5220\u9664\u3002" },
	{ "appletcontext.audio.loaded", "\u5df2\u8f7d\u5165\u97f3\u9891\u526a\u8f91\uff1a{0} " },
	{ "appletcontext.image.loaded", "\u5df2\u8f7d\u5165\u56fe\u8c61\uff1a{0} " },

	{ "securitymgr.automation.printing", "\u81ea\u52a8\uff1a\u63a5\u53d7\u6253\u5370" },

	{ "classloaderinfo.referencing", "\u6b63\u5728\u5f15\u7528\u7c7b\u8f7d\u5165\u7a0b\u5e8f\uff1a{0}, refcount={1}" },
	{ "classloaderinfo.releasing", "\u6b63\u5728\u91ca\u653e\u7c7b\u8f7d\u5165\u7a0b\u5e8f\uff1a{0}, refcount={1}" },
	{ "classloaderinfo.caching", "\u6b63\u5728\u5c06\u7c7b\u8f7d\u5165\u7a0b\u5e8f\u653e\u5165\u9ad8\u901f\u7f13\u5b58\uff1a{0} " },
	{ "classloaderinfo.cachesize", "\u5f53\u524d\u7c7b\u8f7d\u5165\u7a0b\u5e8f\u9ad8\u901f\u7f13\u5b58\u7684\u5927\u5c0f\uff1a{0} " },
	{ "classloaderinfo.num", "\u5df2\u9ad8\u901f\u7f13\u5b58\u7684\u7c7b\u8f7d\u5165\u7a0b\u5e8f\u7684\u6570\u91cf\u8d85\u8fc7 {0}\uff0c\u672a\u5f15\u7528 {1}" },
         { "jsobject.call", "JSObject::call: name={0}" },
         { "jsobject.eval", "JSObject::eval({0})" },
         { "jsobject.getMember", "JSObject::getMember: name={0}" },
         { "jsobject.setMember", "JSObject::setMember: name={0}" },
         { "jsobject.removeMember", "JSObject::removeMember: name={0}" },
         { "jsobject.getSlot", "JSObject::getSlot: {0}" },
         { "jsobject.setSlot", "JSObject::setSlot: slot={0}" },
         { "jsobject.invoke.url.permission", "\u5c0f\u7a0b\u5e8f\u7684 URL \u4e3a {0} \u4e14\u6743\u9650 = {1}"},

	{ "optpkg.install.info", "\u6b63\u5728\u5b89\u88c5\u53ef\u9009\u8f6f\u4ef6\u5305 {0}" },
	{ "optpkg.install.fail", "\u53ef\u9009\u8f6f\u4ef6\u5305\u5b89\u88c5\u5931\u8d25\u3002" },
	{ "optpkg.install.ok", "\u53ef\u9009\u8f6f\u4ef6\u5305\u5b89\u88c5\u6210\u529f\u3002" },
	{ "optpkg.install.automation", "\u81ea\u52a8\uff1a\u63a5\u53d7\u53ef\u9009\u8f6f\u4ef6\u5305\u5b89\u88c5" },
	{ "optpkg.install.granted", "\u53ef\u9009\u8f6f\u4ef6\u5305\u4e0b\u8f7d\u5df2\u83b7\u5f97\u7528\u6237\u7684\u6388\u6743\uff0c\u4ece {0} \u4e0b\u8f7d " },
	{ "optpkg.install.deny", "\u53ef\u9009\u8f6f\u4ef6\u5305\u4e0b\u8f7d\u672a\u83b7\u5f97\u7528\u6237\u7684\u6388\u6743" },
	{ "optpkg.install.begin", "\u6b63\u5728\u5b89\u88c5 {0}" },
	{ "optpkg.install.java.launch", "\u6b63\u5728\u542f\u52a8 Java \u5b89\u88c5\u7a0b\u5e8f" },
	{ "optpkg.install.java.launch.command", "\u6b63\u5728\u901a\u8fc7 ''{0}'' \u542f\u52a8 Java \u5b89\u88c5\u7a0b\u5e8f" },
	{ "optpkg.install.native.launch", "\u6b63\u5728\u542f\u52a8\u672c\u5730\u5b89\u88c5\u7a0b\u5e8f" },
	{ "optpkg.install.native.launch.fail.0", "\u65e0\u6cd5\u6267\u884c {0}" },
	{ "optpkg.install.native.launch.fail.1", "\u5bf9 {0} \u7684\u8bbf\u95ee\u5931\u8d25" },
	{ "optpkg.install.raw.launch", "\u6b63\u5728\u5b89\u88c5\u539f\u53ef\u9009\u8f6f\u4ef6\u5305" },
	{ "optpkg.install.raw.copy", "\u6b63\u5728\u5c06\u539f\u53ef\u9009\u8f6f\u4ef6\u5305\u4ece {0} \u590d\u5236\u5230 {1}" },
	{ "optpkg.install.error.nomethod", "\u672a\u5b89\u88c5\u4ece\u5c5e\u7684\u6269\u5c55\u63d0\u4f9b\u7a0b\u5e8f\uff1a\u65e0\u6cd5\u83b7\u53d6 "
				         + " addExtensionInstallationProvider \u65b9\u6cd5" },
	{ "optpkg.install.error.noclass", "\u672a\u5b89\u88c5\u4ece\u5c5e\u7684\u6269\u5c55\u63d0\u4f9b\u7a0b\u5e8f\uff1a\u65e0\u6cd5\u83b7\u5f97 "
					 + "sun.misc.ExtensionDependency \u7c7b" },

	{"progress_dialog.downloading", "Plug-in:\u6b63\u5728\u4e0b\u8f7d..."},
        {"progress_dialog.dismiss_button", "\u5173\u95ed(D)"},
        {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
        {"progress_dialog.from", "\u4ece"},                

        {"applet_viewer.color_tag", "{0} \u4e2d\u7684\u7ec4\u4ef6\u6570\u6709\u8bef"},
                
        {"progress_info.downloading", "\u6b63\u5728\u4e0b\u8f7d JAR \u6587\u4ef6"},
        
        {"progress_bar.preload", "\u6b63\u5728\u9884\u8f7d JAR \u6587\u4ef6\uff1a{0}"},
        
        {"cache.size", "\u9ad8\u901f\u7f13\u5b58\u5927\u5c0f\uff1a{0}"},
        {"cache.cleanup", "\u9ad8\u901f\u7f13\u5b58\u5927\u5c0f\u4e3a\uff1a{0} \u5b57\u8282\uff0c\u6709\u5fc5\u8981\u6e05\u7406"},
        {"cache.full", "\u9ad8\u901f\u7f13\u5b58\u5df2\u6ee1\uff1a\u6b63\u5728\u5220\u9664\u6587\u4ef6 {0}"},
        {"cache.inuse", "\u4e0d\u80fd\u5220\u9664\u6587\u4ef6 {0}\uff0c\u56e0\u4e3a\u6b64\u5e94\u7528\u7a0b\u5e8f\u6b63\u5728\u4f7f\u7528\u8be5\u6587\u4ef6"},
        {"cache.notdeleted", "\u4e0d\u80fd\u5220\u9664\u6587\u4ef6 {0}\uff0c\u56e0\u4e3a\u6b64\u5e94\u7528\u7a0b\u5e8f\u548c/\u6216\u5176\u4ed6\u5e94\u7528\u7a0b\u5e8f\u53ef\u80fd\u6b63\u5728\u4f7f\u7528"},
        {"cache.out_of_date", "{0} \u7684\u9ad8\u901f\u7f13\u5b58\u526f\u672c\u5df2\u8fc7\u671f\n  \u9ad8\u901f\u7f13\u5b58\u526f\u672c\uff1a{1}\n  \u670d\u52a1\u5668\u526f\u672c\uff1a{2}"},
        {"cache.loading", "\u6b63\u5728\u4ece\u9ad8\u901f\u7f13\u5b58\u52a0\u8f7d {0}"},
        {"cache.cache_warning", "\u8b66\u544a\uff1a\u4e0d\u80fd\u9ad8\u901f\u7f13\u5b58{0}"},
        {"cache.downloading", "\u6b63\u5728\u5c06{0}\u4e0b\u8f7d\u5230\u9ad8\u901f\u7f13\u5b58"},
        {"cache.cached_name", "\u5df2\u9ad8\u901f\u7f13\u5b58\u7684\u6587\u4ef6\u540d\u79f0\uff1a{0}"},
        {"cache.load_warning", "\u8b66\u544a\uff1a\u6b63\u4ece\u9ad8\u901f\u7f13\u5b58\u8bfb\u53d6 {0} \u65f6\u51fa\u9519\u3002"},
        {"cache.disabled", "\u9ad8\u901f\u7f13\u5b58\u5df2\u88ab\u7528\u6237\u7981\u7528"},
        {"cache.minSize", "\u9ad8\u901f\u7f13\u5b58\u88ab\u7981\u7528\u3002\u9ad8\u901f\u7f13\u5b58\u9650\u989d\u8bbe\u7f6e\u4e3a {0}\uff0c\u5df2\u6307\u5b9a\u81f3\u5c11\u5e94\u4e3a 5 MB"},
        {"cache.directory_warning", "\u8b66\u544a\uff1a{0} \u4e0d\u662f\u4e00\u4e2a\u76ee\u5f55\u3002\u9ad8\u901f\u7f13\u5b58\u5c06\u88ab\u7981\u7528\u3002"},
        {"cache.response_warning", "\u8b66\u544a\uff1a{1} \u7684\u5f02\u5e38\u54cd\u5e94 {0}\u3002\u5c06\u91cd\u65b0\u4e0b\u8f7d\u6587\u4ef6\u3002"},
        {"cache.enabled", "\u9ad8\u901f\u7f13\u5b58\u88ab\u542f\u7528"},
        {"cache.location", "\u4f4d\u7f6e\uff1a{0}"},
        {"cache.maxSize", "\u6700\u5927\uff1a{0}"},
        {"cache.create_warning", "\u8b66\u544a\uff1a\u4e0d\u80fd\u521b\u5efa\u9ad8\u901f\u7f13\u5b58\u76ee\u5f55 {0}\u3002\u9ad8\u901f\u7f13\u5b58\u5c06\u88ab\u7981\u7528\u3002"},
        {"cache.read_warning", "\u8b66\u544a\uff1a\u4e0d\u80fd\u8bfb\u53d6\u9ad8\u901f\u7f13\u5b58\u76ee\u5f55 {0}\u3002\u9ad8\u901f\u7f13\u5b58\u5c06\u88ab\u7981\u7528\u3002"},
        {"cache.write_warning", "\u8b66\u544a\uff1a\u4e0d\u80fd\u5199\u5165\u9ad8\u901f\u7f13\u5b58\u76ee\u5f55 {0}\u3002\u9ad8\u901f\u7f13\u5b58\u5c06\u88ab\u7981\u7528\u3002"},
        {"cache.compression", "\u538b\u7f29\u7ea7\u522b\uff1a{0}"},
        {"cache.cert_load", "{0} \u7684\u8bc1\u4e66\u4ece JAR \u9ad8\u901f\u7f13\u5b58\u4e2d\u8bfb\u53d6"},
	{"cache.jarjar.invalid_file", ".jarjar \u6587\u4ef6\u5305\u542b\u4e00\u4e2a\u975e .jar \u6587\u4ef6"},
	{"cache.jarjar.multiple_jar", ".jarjar \u6587\u4ef6\u5305\u542b\u591a\u4e2a .jar \u6587\u4ef6"},
        {"cache.version_checking", "\u6b63\u5728\u68c0\u67e5 {0} \u7684\u7248\u672c\uff0c\u6307\u5b9a\u7248\u672c\u4e3a {1}"},
        { "cache.preloading", "\u6b63\u5728\u9884\u8f7d\u6587\u4ef6 {0}"},

	{ "cache_viewer.caption", "Java \u5c0f\u5e94\u7528\u7a0b\u5e8f\u9ad8\u901f\u7f13\u5b58\u67e5\u770b\u5668" },
	{ "cache_viewer.refresh", "\u5237\u65b0(R)" },
        { "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_R) },
	{ "cache_viewer.remove", "\u5220\u9664(D)" },
        { "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_D) },
	{ "cache_viewer.OK", "\u786e\u5b9a(O)" },
        { "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_O) },
	{ "cache_viewer.name", "\u540d\u79f0" },
	{ "cache_viewer.type", "\u7c7b\u578b" },
	{ "cache_viewer.size", "\u5927\u5c0f" },
	{ "cache_viewer.modify_date", "\u6700\u65b0\u4fee\u6b63\u7248" },
	{ "cache_viewer.expiry_date", "\u5230\u671f\u65e5\u671f" },
	{ "cache_viewer.url", "URL" },
	{ "cache_viewer.version", "\u7248\u672c" },
	{ "cache_viewer.help.name", "\u9ad8\u901f\u7f13\u5b58\u7684\u6587\u4ef6\u540d" },
	{ "cache_viewer.help.type", "\u9ad8\u901f\u7f13\u5b58\u7684\u6587\u4ef6\u7c7b\u578b" },
	{ "cache_viewer.help.size", "\u9ad8\u901f\u7f13\u5b58\u7684\u6587\u4ef6\u5927\u5c0f" },
	{ "cache_viewer.help.modify_date", "\u9ad8\u901f\u7f13\u5b58\u6587\u4ef6\u7684\u6700\u540e\u4fee\u6539\u65e5\u671f" },
	{ "cache_viewer.help.expiry_date", "\u9ad8\u901f\u7f13\u5b58\u6587\u4ef6\u7684\u5230\u671f\u65e5\u671f" },
	{ "cache_viewer.help.url", "\u9ad8\u901f\u7f13\u5b58\u6587\u4ef6\u4e0b\u8f7d URL" },
	{ "cache_viewer.help.version", "\u9ad8\u901f\u7f13\u5b58\u6587\u4ef6\u7248\u672c" },
	{ "cache_viewer.delete.text", "<html><b>\u6587\u4ef6\u672a\u5220\u9664</b></html>{0} \u53ef\u80fd\u5728\u4f7f\u7528\u4e2d\u3002\n" },
        { "cache_viewer.delete.caption", "\u9519\u8bef - \u9ad8\u901f\u7f13\u5b58" }, 
	{ "cache_viewer.type.zip", "Jar" },
	{ "cache_viewer.type.class", "\u7c7b" },
	{ "cache_viewer.type.wav", "Wav \u97f3\u9891" },
	{ "cache_viewer.type.au", "Au \u97f3\u9891" },
	{ "cache_viewer.type.gif", "Gif \u56fe\u50cf" },
	{ "cache_viewer.type.jpg", "Jpeg \u56fe\u50cf" },

	{ "cache_viewer.menu.file", "\u6587\u4ef6(F)" },
	{ "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_F) },
	{ "cache_viewer.menu.options", "\u9009\u9879(P)" },
	{ "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_P) },
	{ "cache_viewer.menu.help", "\u5e2e\u52a9(H)" },
	{ "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_H) },
	{ "cache_viewer.menu.item.exit", "\u9000\u51fa(X)" },
	{ "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_X) },
	{ "cache_viewer.disable", "\u542f\u7528\u9ad8\u901f\u7f13\u5b58(N)" },
	{ "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_N) },
	{ "cache_viewer.menu.item.about", "\u5173\u4e8e(A)" },
	{ "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_A) },

	{ "net.proxy.auto.result.error", "\u65e0\u6cd5\u4ece\u8bc4\u4f30\u4e2d\u786e\u5b9a\u4ee3\u7406\u8bbe\u7f6e - \u540e\u9000\u5230 DIRECT"},

	{ "lifecycle.applet.found", "\u4ece\u8fd0\u884c\u5468\u671f\u9ad8\u901f\u7f13\u5b58\u4e2d\u627e\u5230\u4ee5\u524d\u505c\u6b62\u8fd0\u884c\u7684\u5c0f\u7a0b\u5e8f" },
	{ "lifecycle.applet.support", "\u5c0f\u7a0b\u5e8f\u652f\u6301\u7ee7\u627f\u8fd0\u884c\u5468\u671f\u6a21\u5f0f - \u5c06\u5c0f\u7a0b\u5e8f\u6dfb\u52a0\u5230\u8fd0\u884c\u5468\u671f\u9ad8\u901f\u7f13\u5b58" },
	{ "lifecycle.applet.cachefull", "\u8fd0\u884c\u5468\u671f\u9ad8\u901f\u7f13\u5b58\u5df2\u6ee1 - \u8bf7\u5378\u6389\u6700\u8fd1\u7528\u7684\u6700\u5c11\u7684\u5c0f\u7a0b\u5e8f" },
	{ "com.method.ambiguous", "\u4e0d\u80fd\u9009\u62e9\u65b9\u6cd5\uff0c\u53c2\u6570\u5b9a\u4e49\u4e0d\u660e\u786e" },
	{ "com.method.notexists", "{0}: \u6b64\u65b9\u6cd5\u4e0d\u5b58\u5728" },
	{ "com.notexists", "{0}: \u6b64\u65b9\u6cd5/\u5c5e\u6027\u4e0d\u5b58\u5728" },
	{ "com.method.invoke", "\u8c03\u7528\u65b9\u6cd5\uff1a{0}" }, 
	{ "com.method.jsinvoke", "\u8c03\u7528 JS \u65b9\u6cd5\uff1a{0}" },
	{ "com.method.argsTypeInvalid", "\u53c2\u6570\u4e0d\u80fd\u8f6c\u6362\u6210\u6240\u9700\u7c7b\u578b" },
	{ "com.method.argCountInvalid", "\u53c2\u6570\u6570\u76ee\u4e0d\u6b63\u786e" },
	{ "com.field.needsConversion", "\u9700\u8981\u8f6c\u6362\uff1a{0} --> {1} " },
	{ "com.field.typeInvalid", "\u4e0d\u80fd\u8f6c\u6362\u4e3a\u7c7b\u578b\uff1a{0}" },
	{ "com.field.get", "\u83b7\u53d6\u7279\u6027\uff1a{0}" },
	{ "com.field.set", "\u8bbe\u7f6e\u7279\u6027\uff1a{0}" },

	{ "dialogfactory.menu.show_console", "\u663e\u793a Java \u63a7\u5236\u53f0" },
	{ "dialogfactory.menu.hide_console", "\u9690\u85cf Java \u63a7\u5236\u53f0" },
	{ "dialogfactory.menu.about", "\u5173\u4e8e Java Plug-in" },
	{ "dialogfactory.menu.copy", "\u590d\u5236" },
	{ "dialogfactory.menu.open_console", "\u6253\u5f00 Java \u63a7\u5236\u53f0" },
	{ "dialogfactory.menu.about_java", "\u5173\u4e8e Java(TM)" },

        { "rsa.cert_expired", "<html><b>\u8bc1\u4e66\u8fc7\u671f</b></html>\u4ee3\u7801\u5c06\u6309\u672a\u7b7e\u540d\u5904\u7406 \u3002\n" }, 
        { "rsa.cert_notyieldvalid", "<html><b>\u8bc1\u4e66\u65e0\u6548</b></html>\u4ee3\u7801\u5c06\u6309\u672a\u7b7e\u540d\u5904\u7406 \u3002\n" }, 
        { "rsa.general_error", "<html><b>\u8bc1\u4e66\u672a\u7ecf\u6821\u9a8c</b></html>\u4ee3\u7801\u5c06\u6309\u672a\u7b7e\u540d\u5904\u7406\u3002\n" }, 

    };
}

