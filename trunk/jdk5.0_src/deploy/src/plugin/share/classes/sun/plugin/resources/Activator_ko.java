/*
 * @(#)Activator_ko.java	1.61 04/06/11 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
/**
 * Korean verison of Activator strings.
 *
 * @author Graham Hamilton
 */

public class Activator_ko extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "loading", "\ub85c\ub529 {0} ..." },
	{ "java_applet", "Java \uc560\ud50c\ub9bf" },
        { "failed", "Java \uc560\ud50c\ub9bf \ub85c\ub529\uc5d0 \uc2e4\ud328\ud588\uc2b5\ub2c8\ub2e4..." },
        { "image_failed", "\uc0ac\uc6a9\uc790 \uc815\uc758 \uc774\ubbf8\uc9c0\ub97c \uc791\uc131\ud558\ub294\ub370 \uc2e4\ud328\ud588\uc2b5\ub2c8\ub2e4. \uc774\ubbf8\uc9c0 \ud30c\uc77c \uc774\ub984\uc744 \ud655\uc778\ud558\uc2ed\uc2dc\uc624." },
	{ "java_not_enabled", "Java\ub97c \uc0ac\uc6a9\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "exception", "\uc608\uc678: {0}" },

	{ "bean_code_and_ser", "Beans\uc740 \uc815\uc758\ub41c CODE\uc640 JAVA_OBJECT\ub97c \uac00\uc9c8 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. " },
	{ "status_applet", "\uc560\ud50c\ub9bf {0} {1}" },

	// Resources associated with SecurityManager print Dialog:
	{ "print.caption", "\ud655\uc778 \ud544\uc694 - \uc778\uc1c4" },
	{ "print.message", new String[]{
		"<html><b>\uc778\uc1c4 \uc694\uccad</b></html>\uc560\ud50c\ub9bf\uc774 \uc778\uc1c4\ub429\ub2c8\ub2e4. \uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?"}},
	{ "print.checkBox", "\uc774 \ub300\ud654 \uc0c1\uc790\ub97c \ub2e4\uc2dc \ud45c\uc2dc \uc548 \ud568" },
	{ "print.buttonYes", "\uc608(Y)" },
	{ "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "print.buttonNo", "\uc544\ub2c8\uc624(N)" },
	{ "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},

	{ "optpkg.cert_expired", "<html><b>\uc778\uc99d\uc11c \ub9cc\uae30\ub428</b></html>\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58\uac00 \uc911\ub2e8\ub418\uc5c8\uc2b5\ub2c8\ub2e4.\n" },
	{ "optpkg.cert_notyieldvalid", "<html><b>\uc778\uc99d\uc11c \uc62c\ubc14\ub974\uc9c0 \uc54a\uc74c</b></html>\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58\uac00 \uc911\ub2e8\ub418\uc5c8\uc2b5\ub2c8\ub2e4.\n" },
	{ "optpkg.cert_notverify", "<html><b>\uc778\uc99d\uc11c \uac80\uc99d\ub418\uc9c0 \uc54a\uc74c</b></html>\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58\uac00 \uc911\ub2e8\ub418\uc5c8\uc2b5\ub2c8\ub2e4.\n" },
	{ "optpkg.general_error", "<html><b>\uc77c\ubc18\uc801\uc778 \uc608\uc678</b></html>\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58\uac00 \uc911\ub2e8\ub418\uc5c8\uc2b5\ub2c8\ub2e4.\n" },
	{ "optpkg.caption", "\uacbd\uace0 - \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0" },
	{ "optpkg.installer.launch.wait", "<html><b>\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58</b></html>\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc885\ub8cc \ud6c4 \ud655\uc778\uc744 \ub20c\ub7ec \uc560\ud50c\ub9bf\uc744 \ub85c\ub529\ud558\uc2ed\uc2dc\uc624.\n" },
	{ "optpkg.installer.launch.caption", "\uc124\uce58 \uc9c4\ud589 \uc911 - \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0"},
	{ "optpkg.prompt_user.new_spec.text", "<html><b>\ub2e4\uc6b4\ub85c\ub4dc \uc694\uccad</b></html>\uc560\ud50c\ub9bf\uc740 {2}\uc5d0\uc11c \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \"{1}\"\uc758 \ub354 \uc0c8\ub85c\uc6b4 \ubc84\uc804(\uc2a4\ud399 {0})\uc774 \ud544\uc694\ud569\ub2c8\ub2e4\n\n\uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "optpkg.prompt_user.new_impl.text", "<html><b>\ub2e4\uc6b4\ub85c\ub4dc \uc694\uccad</b></html>\uc560\ud50c\ub9bf\uc740 {2}\uc5d0\uc11c \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \"{1}\"\uc758 \ub354 \uc0c8\ub85c\uc6b4 \ubc84\uc804(\uc2a4\ud399 {0})\uc774 \ud544\uc694\ud569\ub2c8\ub2e4\n\n\uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "optpkg.prompt_user.new_vendor.text", "<html><b>\ub2e4\uc6b4\ub85c\ub4dc \uc694\uccad</b></html>\uc560\ud50c\ub9bf\uc740 {3}\uc5d0\uc11c \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0  \"{1}\" {2}\uc758 ({0})\uac00 \ud544\uc694\ud569\ub2c8\ub2e4\n\n\uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "optpkg.prompt_user.default.text", "<html><b>\ub2e4\uc6b4\ub85c\ub4dc \uc694\uccad</b></html>\uc560\ud50c\ub9bf\uc740 {1}\uc5d0\uc11c \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \"{0}\" \uc124\uce58\uac00 \ud544\uc694\ud569\ub2c8\ub2e4\n\n\uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },

	{ "cache.error.text", "<html><b>\uce90\uc2f1 \uc624\ub958</b></html>\uce90\uc2dc\uc5d0 \uc788\ub294 \ud30c\uc77c\uc744 \uc800\uc7a5\ud558\uac70\ub098 \uac31\uc2e0\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "cache.error.caption", "\uc624\ub958 - \uce90\uc2dc" },
	{ "cache.version_format_error", "{0}\uc774(\uac00) xxxx.xxxx.xxxx.xxxx \ud615\uc2dd\uc774 \uc544\ub2d9\ub2c8\ub2e4. x\ub294 16\uc9c4\uc218\uc785\ub2c8\ub2e4." },
	{ "cache.version_attrib_error", "\'cache_archive\'\uc5d0\uc11c \uc9c0\uc815\ub41c \uc18d\uc131 \uc218\uac00 \'cache_version\'\uc5d0 \uc788\ub294 \uc218\uc640 \uc77c\uce58\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "cache.header_fields_missing", "\ub9c8\uc9c0\ub9c9 \uc218\uc815 \uc2dc\uac04\uacfc \ub9cc\uae30\uac12\uc744 \uc0ac\uc6a9\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4.  Jar \ud30c\uc77c\uc740 \uce90\uc2dc\ub418\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4."},

	{ "applet.progress.load", "\uc560\ud50c\ub9bf \ub85c\ub529 \uc911 ..." },
	{ "applet.progress.init", "\uc560\ud50c\ub9bf \ucd08\uae30\ud654 \uc911 ..." },
	{ "applet.progress.start", "\uc560\ud50c\ub9bf \uc2dc\uc791 \uc911 ..." },
	{ "applet.progress.stop", "\uc560\ud50c\ub9bf \uc815\uc9c0 \uc911 ..." },
	{ "applet.progress.destroy", "\uc560\ud50c\ub9bf \uc644\uc804 \uc0ad\uc81c \uc911 ..." },
	{ "applet.progress.dispose", "\uc560\ud50c\ub9bf \uc9c0\uc6b0\ub294 \uc911 ..." },
	{ "applet.progress.quit", "\uc560\ud50c\ub9bf \uc885\ub8cc \uc911 ..." },
	{ "applet.progress.stoploading", "\ub85c\ub529\uc774 \uc815\uc9c0\ub418\uc5c8\uc74c ..." },
	{ "applet.progress.interrupted", "\uc778\ud130\ub7fd\ud2b8\ub41c \uc2a4\ub808\ub4dc ..." },
	{ "applet.progress.joining", "\uc560\ud50c\ub9bf \uc2a4\ub808\ub4dc \uacb0\ud569 \uc911..." },
	{ "applet.progress.joined", "\uc560\ud50c\ub9bf \uc2a4\ub808\ub4dc \uacb0\ud569..." },
	{ "applet.progress.loadImage", "\uc774\ubbf8\uc9c0 \ub85c\ub529 \uc911 " },
	{ "applet.progress.loadAudio", "\uc624\ub514\uc624 \ub85c\ub529 \uc911 " },
	{ "applet.progress.findinfo.0", "\uc815\ubcf4\ub97c \ucc3e\ub294 \uc911 ..." },
	{ "applet.progress.findinfo.1", "\uc644\ub8cc ..." },
	{ "applet.progress.timeout.wait", "\uc2dc\uac04 \ucd08\uacfc \ub300\uae30 \uc911 ..." },
	{ "applet.progress.timeout.jointing", "\uacb0\ud569 \uc218\ud589 \uc911 ..." },
	{ "applet.progress.timeout.jointed", "\uacb0\ud569 \uc644\ub8cc ..." },

	{ "modality.register", "\ub4f1\ub85d\ub41c \ubaa8\ub2ec \uc218\uc2e0\uae30" },
	{ "modality.unregister", "\ub4f1\ub85d\ub418\uc9c0 \uc54a\uc740 \ubaa8\ub2ec \uc218\uc2e0\uae30" },
	{ "modality.pushed", "\ubaa8\ub2ec \ud478\uc2dc\ub428" },
	{ "modality.popped", "\ubaa8\ub2ec \ud31d\uc5c5\ub428" },

	{ "progress.listener.added", "\ucd94\uac00\ub41c \uc9c4\ud589 \uc218\uc2e0\uae30: {0}" },
	{ "progress.listener.removed", "\uc81c\uac70\ub41c \uc9c4\ud589 \uc218\uc2e0\uae30: {0}" },

	{ "liveconnect.UniversalBrowserRead.enabled", "JavaScript: UniversalBrowserRead\ub97c \uc0ac\uc6a9\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
	{ "liveconnect.java.system", "JavaScript: Java \uc2dc\uc2a4\ud15c \ucf54\ub4dc \ud638\ucd9c" },
	{ "liveconnect.same.origin", "JavaScript: \ud638\ucd9c\uc790\uc640 \ud53c\ud638\ucd9c\uc790\uac00 \ub3d9\uc77c\ud569\ub2c8\ub2e4." },
	{ "liveconnect.default.policy", "JavaScript: \uae30\ubcf8 \ubcf4\uc548 \uc815\ucc45 = {0}" }, 
	{ "liveconnect.UniversalJavaPermission.enabled", "JavaScript: UniversalJavaPermission\uc744 \uc0ac\uc6a9\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
	{ "liveconnect.wrong.securitymodel", "Netscape \ubcf4\uc548 \ubaa8\ub378\uc740 \ub354 \uc774\uc0c1 \uc9c0\uc6d0\ub418\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4.\n"
					     + "\ub300\uc2e0 Java 2 \ubcf4\uc548 \ubaa8\ub378\ub85c \uc774\uc804\ud558\uc2ed\uc2dc\uc624.\n" },

        { "pluginclassloader.created_files", "\uce90\uc2dc\uc5d0\uc11c {0} \uc791\uc131." },
        { "pluginclassloader.deleting_files", "\uce90\uc2dc\uc5d0\uc11c JAR \ud30c\uc77c \uc0ad\uc81c \uc911." },
        { "pluginclassloader.file", "   \uce90\uc2dc {0}\uc5d0\uc11c \uc0ad\uc81c \uc911" },
        { "pluginclassloader.empty_file", "{0}\uc774(\uac00) \ube44\uc5b4 \uc788\uc2b5\ub2c8\ub2e4. \uce90\uc2dc\uc5d0\uc11c \uc0ad\uc81c \uc911." },

	{ "appletcontext.audio.loaded", "\ub85c\ub4dc\ub41c \uc624\ub514\uc624 \ud074\ub9bd: {0}" },
	{ "appletcontext.image.loaded", "\ub85c\ub4dc\ub41c \uc774\ubbf8\uc9c0: {0}" },

	{ "securitymgr.automation.printing", "\uc790\ub3d9\ud654: \uc778\uc1c4 \uc2b9\uc778" },

	{ "classloaderinfo.referencing", "classloader \ucc38\uc870: {0}, refcount={1}" },
	{ "classloaderinfo.releasing", "classloader \ub9b4\ub9ac\uc988: {0}, refcount={1}" },
	{ "classloaderinfo.caching", "\ud074\ub798\uc2a4\ub85c\ub354 \uce90\uc2dc \uc911: {0}" },
	{ "classloaderinfo.cachesize", "\ud604\uc7ac \ud074\ub798\uc2a4\ub85c\ub354 \uce90\uc2dc \ud06c\uae30: {0}" },
	{ "classloaderinfo.num", "{0}\uc5d0\uc11c \uce90\uc2dc\ub41c classloader \uc218, {1} \ucc38\uc870 \ucde8\uc18c" },

        { "jsobject.call", "JSObject::call: \uc774\ub984={0}" },
        { "jsobject.eval", "JSObject::eval({0})" },
        { "jsobject.getMember", "JSObject::getMember: \uc774\ub984={0}" },
        { "jsobject.setMember", "JSObject::setMember: \uc774\ub984={0}" },
        { "jsobject.removeMember", "JSObject::removeMember: \uc774\ub984={0}" },
        { "jsobject.getSlot", "JSObject::getSlot: {0}" },
        { "jsobject.setSlot", "JSObject::setSlot: \uc2ac\ub86f={0}" },
	{ "jsobject.invoke.url.permission", "\uc560\ud50c\ub9bf url={0}, \uad8c\ud55c={1}"},

	{ "optpkg.install.info", "\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58 \uc911 {0}" },
	{ "optpkg.install.fail", "\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58\uc5d0 \uc2e4\ud328\ud588\uc2b5\ub2c8\ub2e4." },
	{ "optpkg.install.ok", "\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0\ub97c \uc124\uce58\ud588\uc2b5\ub2c8\ub2e4." },
	{ "optpkg.install.automation", "\uc790\ub3d9\ud654: \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58\ub97c \uc2b9\uc778\ud569\ub2c8\ub2e4." },
	{ "optpkg.install.granted", "\uc0ac\uc6a9\uc790\uac00 \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \ub2e4\uc6b4\ub85c\ub4dc \uad8c\ud55c\uc744 \ubd80\uc5ec\ud588\uc2b5\ub2c8\ub2e4. {0}\uc5d0\uc11c \ub2e4\uc6b4\ub85c\ub4dc\ud558\uc2ed\uc2dc\uc624" },
	{ "optpkg.install.deny", "\uc0ac\uc6a9\uc790\uac00 \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \ub2e4\uc6b4\ub85c\ub4dc \uad8c\ud55c\uc744 \ubd80\uc5ec\ud558\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4." },
	{ "optpkg.install.begin", "{0} \uc124\uce58\ud558\ub294 \uc911" },
	{ "optpkg.install.java.launch", "Java \uc124\uce58 \uad00\ub9ac\uc790\ub97c \uc2dc\uc791\ud558\ub294 \uc911" },
	{ "optpkg.install.java.launch.command", "''{0}''\uc744(\ub97c) \ud1b5\ud574 Java \uc124\uce58 \uad00\ub9ac\uc790\ub97c \uc2dc\uc791\ud558\ub294 \uc911" },
	{ "optpkg.install.native.launch", "\uc6d0\uc2dc \uc124\uce58 \uad00\ub9ac\uc790\ub97c \uc2dc\uc791\ud558\ub294 \uc911" },
	{ "optpkg.install.native.launch.fail.0", "{0} \uc2e4\ud589 \ubd88\uac00\ub2a5" },
	{ "optpkg.install.native.launch.fail.1", "{0}\uc5d0 \uc561\uc138\uc2a4 \uc2e4\ud328" },
	{ "optpkg.install.raw.launch", "\uc6d0\uc2dc \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc124\uce58 \uc911" },
	{ "optpkg.install.raw.copy", "{0}\uc5d0\uc11c {1}(\uc73c)\ub85c \uc6d0\uc2dc \uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \ubcf5\uc0ac" },
	{ "optpkg.install.error.nomethod", "Dependent Extension Provider\uac00 \uc124\uce58\ub418\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4. "
				         + " addExtensionInstallationProvider \uba54\uc18c\ub4dc\ub97c \uac00\uc838\uc62c \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "optpkg.install.error.noclass", "Dependent Extension Provider\uac00 \uc124\uce58\ub418\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4. "
					 + "sun.misc.ExtensionDependency \ud074\ub798\uc2a4\ub97c \uac00\uc838\uc62c \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },

	{"progress_dialog.downloading", "Plug-in: \ub2e4\uc6b4\ub85c\ub4dc \uc911 ..."},
        {"progress_dialog.dismiss_button", "\uc5c6\uc560\uae30(D)"},
        {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
        {"progress_dialog.from", "\ub2e4\uc74c \uc704\uce58\uc5d0\uc11c"},                

        {"applet_viewer.color_tag", "{0}\uc5d0 \uc788\ub294 \uad6c\uc131 \uc694\uc18c\uc758 \uc218\uac00 \uc62c\ubc14\ub974\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4."},
                
        {"progress_info.downloading", "JAR \ud30c\uc77c \ub2e4\uc6b4\ub85c\ub4dc"},
        {"progress_bar.preload", "JAR \ud30c\uc77c \ubbf8\ub9ac \ub85c\ub4dc: {0}"},
        
        {"cache.size", "\uce90\uc2dc \ud06c\uae30: {0}"},
        {"cache.cleanup", "\uce90\uc2dc \ud06c\uae30: {0} \ubc14\uc774\ud2b8, \uc815\ub9ac\uac00 \ud544\uc694\ud569\ub2c8\ub2e4."},
        {"cache.full", "\uce90\uc2dc\uac00 \uaf49 \ucc3c\uc2b5\ub2c8\ub2e4. {0} \ud30c\uc77c\uc744 \uc0ad\uc81c\ud558\ub294 \uc911\uc785\ub2c8\ub2e4."},
        {"cache.inuse", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c {0} \ud30c\uc77c\uc744 \uc0ac\uc6a9 \uc911\uc774\ubbc0\ub85c \uc774\ub97c \uc0ad\uc81c\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4."},
        {"cache.notdeleted", "{0} \ud30c\uc77c\uc744 \uc0ad\uc81c\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. \uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \ub610\ub294 \ub2e4\ub978 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c \uc0ac\uc6a9 \uc911\uc77c \uc218 \uc788\uc2b5\ub2c8\ub2e4."},
        {"cache.out_of_date", "{0}\uc758 \uce90\uc2dc\ub41c \uc0ac\ubcf8\uc774 \uc624\ub798\ub418\uc5c8\uc2b5\ub2c8\ub2e4\n  \uce90\uc2dc\ub41c \uc0ac\ubcf8: {1}\n  \uc11c\ubc84 \uc0ac\ubcf8:{2}"},
        {"cache.loading", "\uce90\uc2dc\uc5d0\uc11c {0} \ub85c\ub4dc"},
        {"cache.cache_warning", "\uacbd\uace0: {0}\uc744(\ub97c) \uce90\uc2dc\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4."},
        {"cache.downloading", "{0}\uc744(\ub97c) \uce90\uc2dc\ub85c \ub2e4\uc6b4\ub85c\ub4dc"},
        {"cache.cached_name", "\uce90\uc2dc\ub41c \ud30c\uc77c \uc774\ub984: {0}"},
        {"cache.load_warning", "\uacbd\uace0: \uce90\uc2dc\uc5d0\uc11c {0}\uc744(\ub97c) \uc77d\ub294 \uc911 \uc624\ub958\uac00 \ubc1c\uc0dd\ud588\uc2b5\ub2c8\ub2e4."},
        {"cache.disabled", "\uc0ac\uc6a9\uc790\uac00 \uce90\uc2dc\ub97c \uc0ac\uc6a9 \ubd88\uac00\ub2a5\ud558\uac8c \ud558\uc600\uc2b5\ub2c8\ub2e4."},
        {"cache.minSize", "\uce90\uc2dc\uac00 \uc0ac\uc6a9 \ubd88\uac00\ub2a5\ud558\uace0 \uce90\uc2dc \ud55c\uacc4\uac00 {0}(\uc73c)\ub85c \uc124\uc815\ub418\uc5b4 \uc788\uc73c\uba70, \ucd5c\uc18c\ud55c 5MB\uac00 \uc9c0\uc815\ub418\uc5b4\uc57c \ud569\ub2c8\ub2e4."},
        {"cache.directory_warning", "\uacbd\uace0: {0}\uc740(\ub294) \ub514\ub809\ud1a0\ub9ac\uac00 \uc544\ub2d9\ub2c8\ub2e4. \uce90\uc2dc\uac00 \uc0ac\uc6a9 \ubd88\uac00\ub2a5\ud569\ub2c8\ub2e4."},
        {"cache.response_warning", "\uacbd\uace0: {1}\uc5d0 \ub300\ud574 \uc608\uae30\uce58 \ubabb\ud55c \uc751\ub2f5 {0}. \ud30c\uc77c\uc774 \ub2e4\uc2dc \ub2e4\uc6b4\ub85c\ub4dc\ub429\ub2c8\ub2e4."},
        {"cache.enabled", "\uce90\uc2dc\uac00 \uc0ac\uc6a9 \uac00\ub2a5\ud569\ub2c8\ub2e4."},
        {"cache.location", "\uc704\uce58: {0}"},
        {"cache.maxSize", "\ucd5c\ub300 \ud06c\uae30: {0}"},
        {"cache.create_warning", "\uacbd\uace0: \uce90\uc2dc \ub514\ub809\ud1a0\ub9ac {0}\uc744(\ub97c) \uc791\uc131\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. \uce90\uc2f1\uc774 \uc0ac\uc6a9 \ubd88\uac00\ub2a5\ud569\ub2c8\ub2e4."},
        {"cache.read_warning", "\uacbd\uace0: \uce90\uc2dc \ub514\ub809\ud1a0\ub9ac {0}\uc744(\ub97c) \uc77d\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. \uce90\uc2f1\uc774 \uc0ac\uc6a9 \ubd88\uac00\ub2a5\ud569\ub2c8\ub2e4."},
        {"cache.write_warning", "\uacbd\uace0: \uce90\uc2dc \ub514\ub809\ud1a0\ub9ac {0}\uc5d0 \uc4f8 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. \uce90\uc2f1\uc774 \uc0ac\uc6a9 \ubd88\uac00\ub2a5\ud569\ub2c8\ub2e4."},
        {"cache.compression", "\uc555\ucd95 \ub808\ubca8: {0}"},
        {"cache.cert_load", "JAR \uce90\uc2dc\uc5d0\uc11c {0}\uc5d0 \ub300\ud55c \uc778\uc99d\uc11c\ub97c \uc77d\uc5c8\uc2b5\ub2c8\ub2e4."},
	{"cache.jarjar.invalid_file", ".jarjar \ud30c\uc77c\uc5d0 .jar \ud30c\uc77c\uc774 \ub4e4\uc5b4 \uc788\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4."},
	{"cache.jarjar.multiple_jar", ".jarjar \ud30c\uc77c\uc774 \ud558\ub098 \uc774\uc0c1\uc758 .jar \ud30c\uc77c\uc744 \ud3ec\ud568\ud558\uace0 \uc788\uc2b5\ub2c8\ub2e4."},
        {"cache.version_checking", "{0}\uc5d0 \ub300\ud55c \ubc84\uc804\uc744 \uac80\uc0ac \uc911\uc774\uba70, \uc9c0\uc815\ub41c \ubc84\uc804\uc740 {1}\uc785\ub2c8\ub2e4."},
        {"cache.preloading", "{0} \ud30c\uc77c \ubbf8\ub9ac \ub85c\ub4dc \uc911"},

	{ "cache_viewer.caption", "Java \uc560\ud50c\ub9bf \uce90\uc2dc \ubdf0\uc5b4" },
	{ "cache_viewer.refresh", "\uc7ac\uc0dd(R)" },
	{ "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_R) },
        { "cache_viewer.remove", "\uc0ad\uc81c(D)" },
	{ "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_D) },
        { "cache_viewer.OK", "\ud655\uc778" },
	{ "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_O) },
        { "cache_viewer.name", "\uc774\ub984" },
	{ "cache_viewer.type", "\uc720\ud615" },
	{ "cache_viewer.size", "\ud06c\uae30" },
	{ "cache_viewer.modify_date", "\ub9c8\uc9c0\ub9c9 \uc218\uc815" },
	{ "cache_viewer.expiry_date", "\ub9cc\ub8cc \ub0a0\uc9dc" },
	{ "cache_viewer.url", "URL" },
	{ "cache_viewer.version", "\ubc84\uc804" },
	{ "cache_viewer.help.name", "\uce90\uc2dc\ub41c \ud30c\uc77c \uc774\ub984" },
	{ "cache_viewer.help.type", "\uce90\uc2dc\ub41c \ud30c\uc77c \uc720\ud615" },
	{ "cache_viewer.help.size", "\uce90\uc2dc\ub41c \ud30c\uc77c \ud06c\uae30" },
	{ "cache_viewer.help.modify_date", "\uce90\uc2dc\ub41c \ud30c\uc77c\uc758 \ub9c8\uc9c0\ub9c9 \uc218\uc815\uc77c" },
	{ "cache_viewer.help.expiry_date", "\uce90\uc2dc\ub41c \ud30c\uc77c \ub9cc\uae30\uc77c" },
	{ "cache_viewer.help.url", "\uce90\uc2dc\ub41c \ud30c\uc77c \ub2e4\uc6b4\ub85c\ub4dc URL" },
	{ "cache_viewer.help.version", "\uce90\uc2dc\ub41c \ud30c\uc77c \ubc84\uc804" },
	{ "cache_viewer.delete.text", "<html><b>\ud30c\uc77c \uc0ad\uc81c\ub418\uc9c0 \uc54a\uc74c</b></html>{0}\uc774(\uac00) \uc0ac\uc6a9 \uc911\uc785\ub2c8\ub2e4.\n" },
	{ "cache_viewer.delete.caption", "\uc624\ub958 - \uce90\uc2dc" },
	{ "cache_viewer.type.zip", "Jar" },
	{ "cache_viewer.type.class", "\ud074\ub798\uc2a4" },
	{ "cache_viewer.type.wav", "Wav \uc0ac\uc6b4\ub4dc" },
	{ "cache_viewer.type.au", "Au \uc0ac\uc6b4\ub4dc" },
	{ "cache_viewer.type.gif", "Gif \uc774\ubbf8\uc9c0" },
	{ "cache_viewer.type.jpg", "Jpeg \uc774\ubbf8\uc9c0" },
        { "cache_viewer.menu.file", "\ud30c\uc77c(F)" },
        { "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_F) },
        { "cache_viewer.menu.options", "\uc635\uc158(P)" },
        { "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_P) },
        { "cache_viewer.menu.help", "\ub3c4\uc6c0\ub9d0(H)" },
        { "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_H) },
        { "cache_viewer.menu.item.exit", "\uc885\ub8cc(X)" },
        { "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_X) },
        { "cache_viewer.disable", "\uce90\uc2f1 \uc0ac\uc6a9 \uac00\ub2a5(N)" },
        { "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_N) },
        { "cache_viewer.menu.item.about", "\uc815\ubcf4(A)" },
        { "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_A) },

	{ "net.proxy.auto.result.error", "\ud3c9\uac00\ub85c\ubd80\ud130 \ud504\ub85d\uc2dc \uc124\uc815\uc744 \uacb0\uc815\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4 - DIRECT\ub85c \ud3f4\ubc31"},

	{ "lifecycle.applet.found", "\uc774\uc804 \uc21c\ud658 \uce90\uc2dc\ub85c\ubd80\ud130 \uc815\uc9c0\ub41c \uc774\uc804 \uc560\ud50c\ub9bf\uc744 \ubc1c\uacac\ud588\uc2b5\ub2c8\ub2e4." },
	{ "lifecycle.applet.support", "\uc560\ud50c\ub9bf\uc740 \ub808\uac70\uc2dc \uc21c\ud658 \ubaa8\ub378\uc744 \uc9c0\uc6d0\ud569\ub2c8\ub2e4. \uc21c\ud658 \uce90\uc2dc\uc5d0 \uc560\ud50c\ub9bf\uc744 \ucd94\uac00\ud558\uc2ed\uc2dc\uc624" },
	{ "lifecycle.applet.cachefull", "\uc21c\ud658 \uce90\uc2dc\uac00 \uaf49 \ucc3c\uc2b5\ub2c8\ub2e4. prune\uc774 \uac00\uc7a5 \ucd5c\uadfc\uc5d0 \uc560\ud50c\ub9bf\uc744 \uc0ac\uc6a9\ud588\uc2b5\ub2c8\ub2e4." },

	{ "com.method.ambiguous", "\uba54\uc18c\ub4dc\uc640 \uc560\ub9e4\ud55c \ub9e4\uac1c\ubcc0\uc218\ub97c \uc120\ud0dd\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "com.method.notexists", "{0} :\uba54\uc18c\ub4dc\ub294 \uc874\uc7ac\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "com.notexists", "{0} :\uba54\uc18c\ub4dc/\ud2b9\uc131\uc740 \uc874\uc7ac\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "com.method.invoke", "\uba54\uc18c\ub4dc \ud638\ucd9c: {0}" },
	{ "com.method.jsinvoke", "JS \uba54\uc18c\ub4dc \ud638\ucd9c: {0}" },
	{ "com.method.argsTypeInvalid", "\ub9e4\uac1c\ubcc0\uc218\uac00 \ud544\uc694\ud55c \uc720\ud615\uc73c\ub85c \ubcc0\ud658\ub420 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "com.method.argCountInvalid", "\uc778\uc218\uc758 \uc218\uac00 \uc815\ud655\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "com.field.needsConversion", "\ubcc0\ud658 \ud544\uc694: {0} --> {1}" },
	{ "com.field.typeInvalid", "{0} \uc720\ud615\uc73c\ub85c \ubcc0\ud658\ub420 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "com.field.get", "\ub4f1\ub85d \uc815\ubcf4 \uc5bb\uae30: {0}" },
	{ "com.field.set", "\ub4f1\ub85d \uc815\ubcf4 \uc124\uc815: {0}" },

	{ "rsa.cert_expired", "<html><b>\uc778\uc99d\uc11c \ub9cc\uae30\ub428</b></html>\ucf54\ub4dc\uac00 '\uc11c\uba85\ub418\uc9c0 \uc54a\uc74c'\uc73c\ub85c \ucc98\ub9ac\ub429\ub2c8\ub2e4.\n" },
	{ "rsa.cert_notyieldvalid", "<html><b>\uc778\uc99d\uc11c \uc720\ud6a8\ud558\uc9c0 \uc54a\uc74c</b></html>\ucf54\ub4dc\uac00 '\uc11c\uba85\ub418\uc9c0 \uc54a\uc74c'\uc73c\ub85c \ucc98\ub9ac\ub429\ub2c8\ub2e4.\n" },
	{ "rsa.general_error", "<html><b>\uc778\uc99d\uc11c \uac80\uc99d\ub418\uc9c0 \uc54a\uc74c</b></html>\ucf54\ub4dc\uac00 '\uc11c\uba85\ub418\uc9c0 \uc54a\uc74c'\uc73c\ub85c \ucc98\ub9ac\ub429\ub2c8\ub2e4.\n" },

	{ "dialogfactory.menu.show_console", "Java \ucf58\uc194 \ud45c\uc2dc)" },
	{ "dialogfactory.menu.hide_console", "Java \ucf58\uc194 \uc228\uae40" },
	{ "dialogfactory.menu.about", "Java Plug-in \uc815\ubcf4" },
	{ "dialogfactory.menu.copy", "\ubcf5\uc0ac" },
	{ "dialogfactory.menu.open_console", "Java \ucf58\uc194 \uc5f4\uae30" },
	{ "dialogfactory.menu.about_java", "Java(TM) \uc815\ubcf4" },
    };
}

  
