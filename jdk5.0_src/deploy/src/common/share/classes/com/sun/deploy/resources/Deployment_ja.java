/*
 * @(#)Deployment_ja.java	1.31 04/07/19
 *
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;


/**
 * English verison of Deployment strings.
 *
 * @author Stanley Man-Kit Ho
 */

public final class Deployment_ja extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
        { "product.javapi.name", "Java Plug-in {0}" },
        { "product.javaws.name", "Java Web Start {0}" },
	{ "console.version", "\u30d0\u30fc\u30b8\u30e7\u30f3" },
	{ "console.default_vm_version", "\u30c7\u30d5\u30a9\u30eb\u30c8\u306e\u4eee\u60f3\u30de\u30b7\u30f3\u306e\u30d0\u30fc\u30b8\u30e7\u30f3 " },
	{ "console.using_jre_version", "\u4f7f\u7528\u4e2d\u306e JRE \u306e\u30d0\u30fc\u30b8\u30e7\u30f3" },
	{ "console.user_home", "\u30e6\u30fc\u30b6\u306e\u30db\u30fc\u30e0\u30c7\u30a3\u30ec\u30af\u30c8\u30ea" },
        { "console.caption", "Java \u30b3\u30f3\u30bd\u30fc\u30eb" },
        { "console.clear", "\u6d88\u53bb(C)" },
        { "console.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "console.close", "\u9589\u3058\u308b(E)" },
        { "console.close.acceleratorKey", new Integer(KeyEvent.VK_E) },
        { "console.copy", "\u30b3\u30d4\u30fc(Y)" },
        { "console.copy.acceleratorKey", new Integer(KeyEvent.VK_Y) },
	{ "console.menu.text.top", "----------------------------------------------------\n" },
	{ "console.menu.text.c", "c:   \u30b3\u30f3\u30bd\u30fc\u30eb\u30a6\u30a3\u30f3\u30c9\u30a6\u3092\u30af\u30ea\u30a2\n" },
	{ "console.menu.text.f", "f:   \u30d5\u30a1\u30a4\u30ca\u30e9\u30a4\u30ba\u30ad\u30e5\u30fc\u306e\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u3092\u30d5\u30a1\u30a4\u30ca\u30e9\u30a4\u30ba\n" },
	{ "console.menu.text.g", "g:   \u30ac\u30d9\u30fc\u30b8\u30b3\u30ec\u30af\u30c8\n" },
	{ "console.menu.text.h", "h:   \u3053\u306e\u30d8\u30eb\u30d7\u30e1\u30c3\u30bb\u30fc\u30b8\u3092\u8868\u793a\n" },
	{ "console.menu.text.j", "j:   jcov \u30c7\u30fc\u30bf\u3092\u30c0\u30f3\u30d7\n"},
	{ "console.menu.text.l", "l:   \u30af\u30e9\u30b9\u30ed\u30fc\u30c0\u30ea\u30b9\u30c8\u3092\u30c0\u30f3\u30d7\n" },
	{ "console.menu.text.m", "m:   \u30e1\u30e2\u30ea\u4f7f\u7528\u7387\u3092\u8868\u793a\n" },
	{ "console.menu.text.o", "o:   \u30c8\u30ea\u30ac\u30ed\u30b0\n" },
	{ "console.menu.text.p", "p:   \u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u518d\u30ed\u30fc\u30c9\n" },
	{ "console.menu.text.q", "q:   \u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u975e\u8868\u793a\n" },
	{ "console.menu.text.r", "r:   \u30dd\u30ea\u30b7\u30fc\u8a2d\u5b9a\u3092\u518d\u30ed\u30fc\u30c9\n" },
	{ "console.menu.text.s", "s:   \u30b7\u30b9\u30c6\u30e0\u30d7\u30ed\u30d1\u30c6\u30a3\u3068\u914d\u5099\u30d7\u30ed\u30d1\u30c6\u30a3\u3092\u30c0\u30f3\u30d7\n" },
	{ "console.menu.text.t", "t:   \u30b9\u30ec\u30c3\u30c9\u30ea\u30b9\u30c8\u3092\u30c0\u30f3\u30d7\n" },
	{ "console.menu.text.v", "v:   \u30b9\u30ec\u30c3\u30c9\u30b9\u30bf\u30c3\u30af\u3092\u30c0\u30f3\u30d7\n" },
	{ "console.menu.text.x", "x:   \u30af\u30e9\u30b9\u30ed\u30fc\u30c0\u30ad\u30e3\u30c3\u30b7\u30e5\u3092\u30af\u30ea\u30a2\n" },
	{ "console.menu.text.0", "0-5: \u30c8\u30ec\u30fc\u30b9\u30ec\u30d9\u30eb\u3092 <n> \u306b\u8a2d\u5b9a\n" },
	{ "console.menu.text.tail", "----------------------------------------------------\n" },
	{ "console.done", "\u5b8c\u4e86\u3057\u307e\u3057\u305f\u3002" },
	{ "console.trace.level.0", "\u30c8\u30ec\u30fc\u30b9\u30ec\u30d9\u30eb\u3092 0 \u306b\u8a2d\u5b9a: \u306a\u3057 ... \u5b8c\u4e86\u3002" },
	{ "console.trace.level.1", "\u30c8\u30ec\u30fc\u30b9\u30ec\u30d9\u30eb\u3092 1 \u306b\u8a2d\u5b9a: \u57fa\u672c ... \u5b8c\u4e86\u3002" },
	{ "console.trace.level.2", "\u30c8\u30ec\u30fc\u30b9\u30ec\u30d9\u30eb\u3092 2 \u306b\u8a2d\u5b9a: \u57fa\u672c\u3001\u30cd\u30c3\u30c8\u30ef\u30fc\u30af ... \u5b8c\u4e86\u3002" },
	{ "console.trace.level.3", "\u30c8\u30ec\u30fc\u30b9\u30ec\u30d9\u30eb\u3092 3 \u306b\u8a2d\u5b9a: \u57fa\u672c\u3001\u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u3001\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3 ... \u5b8c\u4e86\u3002" },
	{ "console.trace.level.4", "\u30c8\u30ec\u30fc\u30b9\u30ec\u30d9\u30eb\u3092 4 \u306b\u8a2d\u5b9a: \u57fa\u672c\u3001\u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u3001\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u3001\u62e1\u5f35\u6a5f\u80fd ... \u5b8c\u4e86\u3002" },
	{ "console.trace.level.5", "\u30c8\u30ec\u30fc\u30b9\u30ec\u30d9\u30eb\u3092 5 \u306b\u8a2d\u5b9a: \u3059\u3079\u3066 ... \u5b8c\u4e86\u3002" },
	{ "console.log", "\u30ed\u30b0\u3092\u8a2d\u5b9a\u3057\u307e\u3059 : " },
	{ "console.completed", " ... \u5b8c\u4e86\u3002" },
	{ "console.dump.thread", "\u30b9\u30ec\u30c3\u30c9\u30ea\u30b9\u30c8\u306e\u30c0\u30f3\u30d7 ...\n" },
	{ "console.dump.stack", "\u30b9\u30ec\u30c3\u30c9\u30b9\u30bf\u30c3\u30af\u306e\u30c0\u30f3\u30d7 ...\n" },
	{ "console.dump.system.properties", "\u30b7\u30b9\u30c6\u30e0\u30d7\u30ed\u30d1\u30c6\u30a3\u306e\u30c0\u30f3\u30d7 ...\n" },
	{ "console.dump.deployment.properties", "\u914d\u5099\u30d7\u30ed\u30d1\u30c6\u30a3\u306e\u30c0\u30f3\u30d7 ...\n" },
	{ "console.clear.classloader", "\u30af\u30e9\u30b9\u30ed\u30fc\u30c0\u30ad\u30e3\u30c3\u30b7\u30e5\u306e\u6d88\u53bb ... \u5b8c\u4e86\u3002" },
	{ "console.reload.policy", "\u30dd\u30ea\u30b7\u30fc\u8a2d\u5b9a\u306e\u518d\u30ed\u30fc\u30c9" },
	{ "console.reload.proxy", "\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u306e\u518d\u30ed\u30fc\u30c9 ..." },
	{ "console.gc", "\u30ac\u30d9\u30fc\u30b8\u30b3\u30ec\u30af\u30c8" },
	{ "console.finalize", "\u30d5\u30a1\u30a4\u30ca\u30e9\u30a4\u30ba\u30ad\u30e5\u30fc\u306e\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u3092\u30d5\u30a1\u30a4\u30ca\u30e9\u30a4\u30ba" },
	{ "console.memory", "\u30e1\u30e2\u30ea: {0}K  \u7a7a\u304d\u5bb9\u91cf: {1}K  ({2}%)" },
	{ "console.jcov.error", "Jcov \u306e\u5b9f\u884c\u6642\u30a8\u30e9\u30fc\u3002jcov \u30aa\u30d7\u30b7\u30e7\u30f3\u304c\u6b63\u3057\u304f\u6307\u5b9a\u3055\u308c\u3066\u3044\u308b\u304b\u3069\u3046\u304b\u3092\u30c1\u30a7\u30c3\u30af\u3057\u3066\u304f\u3060\u3055\u3044\n"},
	{ "console.jcov.info", "Jcov \u30c7\u30fc\u30bf\u304c\u6b63\u5e38\u306b\u30c0\u30f3\u30d7\u3055\u308c\u307e\u3057\u305f\n"},

	{ "https.dialog.caption", "\u8b66\u544a - HTTPS" },
	{ "https.dialog.text", "<html><b>\u30db\u30b9\u30c8\u540d\u306e\u4e0d\u4e00\u81f4</b></html>\u30b5\u30fc\u30d0\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a3c\u660e\u66f8\u306e\u30db\u30b9\u30c8\u540d\u304c\u30b5\u30fc\u30d0\u540d\u3068\u4e00\u81f4\u3057\u307e\u305b\u3093\u3002"
			     + "\n\nURL \u306e\u30db\u30b9\u30c8\u540d: {0}"
			     + "\n\u8a3c\u660e\u66f8\u306b\u793a\u3055\u308c\u3066\u3044\u308b\u30db\u30b9\u30c8\u540d: {1}"
			     + "\n\n\u7d9a\u884c\u3057\u307e\u3059\u304b?" },
	{ "https.dialog.unknown.host", "\u672a\u77e5\u306e\u30db\u30b9\u30c8" },

	{ "security.dialog.caption", "\u8b66\u544a - \u30bb\u30ad\u30e5\u30ea\u30c6\u30a3" },
	{ "security.dialog.text0", "\"{1}\" \u304c\u914d\u5e03\u3059\u308b\u7f72\u540d\u4ed8\u304d\u306e {0} \u3092\u4fe1\u983c\u3057\u307e\u3059\u304b?"
				 + "\n\n\u767a\u884c\u8005\u306e\u4fe1\u983c\u6027\u306f \"{2}\" \u306b\u3088\u3063\u3066\u691c\u8a3c\u3055\u308c\u307e\u3057\u305f\u3002" },
	{ "security.dialog.text0a", "\"{1}\" \u304b\u3089\u914d\u5e03\u3055\u308c\u305f\u7f72\u540d\u4ed8\u304d {0} \u3092\u4fe1\u983c\u3057\u307e\u3059\u304b?"
 				 + "\n\n\u767a\u884c\u8005\u306e\u4fe1\u983c\u6027\u3092\u691c\u8a3c\u3067\u304d\u307e\u305b\u3093\u3002" },
	{ "security.dialog.timestamp.text1", "{0} \u306f {1} \u306b\u7f72\u540d\u3055\u308c\u307e\u3057\u305f\u3002" },
	{ "security.dialog_https.text0", "\u6697\u53f7\u5316\u3055\u308c\u305f\u60c5\u5831\u3092\u4ea4\u63db\u3059\u308b\u76ee\u7684\u3067\u3001Web \u30b5\u30a4\u30c8 \"{0}\" \u304b\u3089\u306e\u8a3c\u660e\u66f8\u3092\u53d7\u3051\u5165\u308c\u307e\u3059\u304b?"
				 + "\n\n\u767a\u884c\u8005\u306e\u4fe1\u983c\u6027\u306f \"{1}\" \u306b\u3088\u3063\u3066\u691c\u8a3c\u3055\u308c\u307e\u3057\u305f\u3002" },
	{ "security.dialog_https.text0a", "\u6697\u53f7\u5316\u3055\u308c\u305f\u60c5\u5831\u3092\u4ea4\u63db\u3059\u308b\u76ee\u7684\u3067\u3001Web \u30b5\u30a4\u30c8 \"{0}\" \u304b\u3089\u306e\u8a3c\u660e\u66f8\u3092\u53d7\u3051\u5165\u308c\u307e\u3059\u304b?"
 				 + "\n\n\u767a\u884c\u8005\u306e\u4fe1\u983c\u6027\u3092\u691c\u8a3c\u3067\u304d\u307e\u305b\u3093\u3002" },
	{ "security.dialog.text1", "\n\u6ce8\u610f: \"{0}\" \u306f\u3053\u306e\u5185\u5bb9\u304c\u5b89\u5168\u3067\u3042\u308b\u3053\u3068\u3092\u8868\u660e\u3057\u307e\u3059\u3002\u3053\u306e\u8868\u660e\u3092\u884c\u3063\u3066\u3044\u308b \"{1}\" \u3092\u4fe1\u983c\u3059\u308b\u5834\u5408\u306b\u306e\u307f\u3053\u306e\u5185\u5bb9\u3092\u627f\u8a8d\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
	{ "security.dialog.unknown.issuer", "\u672a\u77e5\u306e\u767a\u884c\u8005" },
	{ "security.dialog.unknown.subject", "\u672a\u77e5\u306e\u88ab\u8a8d\u8a3c\u8005" },
	{ "security.dialog.certShowName", "{0} ({1})" },
	{ "security.dialog.rootCANotValid", "\u3053\u306e\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a3c\u660e\u66f8\u306f\u3001\u4fe1\u983c\u3067\u304d\u306a\u3044\u56e3\u4f53\u306b\u3088\u3063\u3066\u767a\u884c\u3055\u308c\u3066\u3044\u307e\u3059\u3002" },
	{ "security.dialog.rootCAValid", "\u3053\u306e\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a3c\u660e\u66f8\u306f\u3001\u4fe1\u983c\u3067\u304d\u308b\u56e3\u4f53\u306b\u3088\u3063\u3066\u767a\u884c\u3055\u308c\u3066\u3044\u307e\u3059\u3002" },
	{ "security.dialog.timeNotValid", "\u3053\u306e\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a3c\u660e\u66f8\u306f\u671f\u9650\u304c\u5207\u308c\u3066\u3044\u308b\u304b\u3001\u307e\u3060\u6709\u52b9\u306b\u306a\u3063\u3066\u3044\u307e\u305b\u3093\u3002" },
	{ "security.dialog.timeValid", "\u3053\u306e\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a3c\u660e\u66f8\u306f\u671f\u9650\u304c\u5207\u308c\u3066\u304a\u3089\u305a\u3001\u4f9d\u7136\u3068\u3057\u3066\u6709\u52b9\u3067\u3059\u3002" },
	{ "security.dialog.timeValidTS", "\u3053\u306e\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a3c\u660e\u66f8\u306f {0} \u304c\u7f72\u540d\u3055\u308c\u305f\u6642\u306b\u6709\u52b9\u306b\u306a\u308a\u307e\u3057\u305f\u3002" },
	{ "security.dialog.buttonAlways", "\u5e38\u306b(A)" },
        { "security.dialog.buttonAlways.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "security.dialog.buttonYes", "\u306f\u3044(Y)" },
	{ "security.dialog.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "security.dialog.buttonNo", "\u3044\u3044\u3048(N)" },
	{ "security.dialog.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "security.dialog.buttonViewCert", "\u8a73\u7d30(M)" },
        { "security.dialog.buttonViewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

        { "security.badcert.caption", "\u8b66\u544a - \u30bb\u30ad\u30e5\u30ea\u30c6\u30a3" },
        { "security.badcert.https.text", "SSL \u8a3c\u660e\u66f8\u3092\u78ba\u8a8d\u3067\u304d\u307e\u305b\u3093\u3002\n\u3053\u306e {0} \u306f\u5b9f\u884c\u3055\u308c\u307e\u305b\u3093\u3002" },
        { "security.badcert.config.text", "\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a2d\u5b9a\u3067\u30e6\u30fc\u30b6\u304c\u3053\u306e\u8a3c\u660e\u66f8\u3092\u78ba\u8a8d\u3067\u304d\u306a\u3044\u3088\u3046\u306b\u306a\u3063\u3066\u3044\u307e\u3059\u3002\u3053\u306e {0} \u306f\u5b9f\u884c\u3055\u308c\u307e\u305b\u3093\u3002" },
        { "security.badcert.text", "\u8a3c\u660e\u66f8\u306e\u78ba\u8a8d\u306b\u5931\u6557\u3057\u307e\u3057\u305f\u3002\u3053\u306e {0} \u306f\u5b9f\u884c\u3055\u308c\u307e\u305b\u3093\u3002" },
        { "security.badcert.viewException", "\u4f8b\u5916\u306e\u8868\u793a(S)" },
        { "security.badcert.viewException.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "security.badcert.viewCert", "\u8a73\u7d30(M)" },
        { "security.badcert.viewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

	{ "cert.dialog.caption", "\u8a73\u7d30 - \u8a3c\u660e\u66f8" },
	{ "cert.dialog.certpath", "\u8a3c\u660e\u66f8\u306e\u30d1\u30b9" },
	{ "cert.dialog.field.Version", "\u30d0\u30fc\u30b8\u30e7\u30f3" },
	{ "cert.dialog.field.SerialNumber", "\u30b7\u30ea\u30a2\u30eb\u756a\u53f7" },
	{ "cert.dialog.field.SignatureAlg", "\u7f72\u540d\u30a2\u30eb\u30b4\u30ea\u30ba\u30e0" },
	{ "cert.dialog.field.Issuer", "\u767a\u884c\u8005" },
	{ "cert.dialog.field.EffectiveDate", "\u5b9f\u52b9\u65e5" },
	{ "cert.dialog.field.ExpirationDate", "\u6709\u52b9\u671f\u9650" },
	{ "cert.dialog.field.Validity", "\u6709\u52b9\u6027" },
	{ "cert.dialog.field.Subject", "\u88ab\u8a8d\u8a3c\u8005" },
	{ "cert.dialog.field.Signature", "\u7f72\u540d" },
	{ "cert.dialog.field", "\u30d5\u30a3\u30fc\u30eb\u30c9" },
	{ "cert.dialog.value", "\u5024" },
        { "cert.dialog.close", "\u9589\u3058\u308b(C)" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_C) },

	{ "clientauth.password.dialog.buttonOK", "\u4e86\u89e3(O)" },
	{ "clientauth.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.password.dialog.buttonCancel", "\u53d6\u6d88\u3057(C)" },
	{ "clientauth.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.password.dialog.caption", "\u30d1\u30b9\u30ef\u30fc\u30c9\u306e\u8981\u6c42 - \u30af\u30e9\u30a4\u30a2\u30f3\u30c8\u8a8d\u8a3c\u30ad\u30fc\u30b9\u30c8\u30a2" },
	{ "clientauth.password.dialog.text", "\u30af\u30e9\u30a4\u30a2\u30f3\u30c8\u8a8d\u8a3c\u30ad\u30fc\u30b9\u30c8\u30a2\u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u305f\u3081\u306e\u30d1\u30b9\u30ef\u30fc\u30c9\u3092\u5165\u529b\u3057\u3066\u304f\u3060\u3055\u3044:\n" },
	{ "clientauth.password.dialog.error.caption", "\u30a8\u30e9\u30fc - \u30af\u30e9\u30a4\u30a2\u30f3\u30c8\u8a8d\u8a3c\u30ad\u30fc\u30b9\u30c8\u30a2" },
	{ "clientauth.password.dialog.error.text", "<html><b>\u30ad\u30fc\u30b9\u30c8\u30a2\u30a2\u30af\u30bb\u30b9\u30a8\u30e9\u30fc</b></html>\u30ad\u30fc\u30b9\u30c8\u30a2\u304c\u6539\u3056\u3093\u3055\u308c\u3066\u3044\u308b\u304b\u3001\u30d1\u30b9\u30ef\u30fc\u30c9\u304c\u6b63\u3057\u304f\u3042\u308a\u307e\u305b\u3093\u3002" },
		
	{ "clientauth.certlist.dialog.buttonOK", "\u4e86\u89e3(O)" },
	{ "clientauth.certlist.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.certlist.dialog.buttonCancel", "\u53d6\u6d88\u3057(C)" },
	{ "clientauth.certlist.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.certlist.dialog.buttonDetails", "\u8a73\u7d30(D)" },
	{ "clientauth.certlist.dialog.buttonDetails.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "clientauth.certlist.dialog.caption", "\u30af\u30e9\u30a4\u30a2\u30f3\u30c8\u8a8d\u8a3c" },
	{ "clientauth.certlist.dialog.text", "\u3053\u306e Web \u30b5\u30a4\u30c8\u306b\u63a5\u7d9a\u3059\u308b\u306b\u306f ID \u304c\u5fc5\u8981\u3067\u3059\u3002\n\u63a5\u7d9a\u6642\u306b\u4f7f\u7528\u3059\u308b\u8a3c\u660e\u66f8\u3092\u9078\u629e\u3057\u3066\u304f\u3060\u3055\u3044\u3002\n" },

	{ "dialogfactory.confirmDialogTitle", "\u78ba\u8a8d - Java" },
	{ "dialogfactory.inputDialogTitle", "\u5165\u529b - Java" },
	{ "dialogfactory.messageDialogTitle", "\u30e1\u30c3\u30bb\u30fc\u30b8 - Java" },
	{ "dialogfactory.exceptionDialogTitle", "\u30a8\u30e9\u30fc - Java" },
	{ "dialogfactory.optionDialogTitle", "\u30aa\u30d7\u30b7\u30e7\u30f3 - Java" },
	{ "dialogfactory.aboutDialogTitle", "\u60c5\u5831 - Java" },
	{ "dialogfactory.confirm.yes", "\u306f\u3044(Y)" },
        { "dialogfactory.confirm.yes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "dialogfactory.confirm.no", "\u3044\u3044\u3048(N)" },
        { "dialogfactory.confirm.no.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "dialogfactory.moreInfo", "\u8a73\u7d30\u60c5\u5831(M)" },
        { "dialogfactory.moreInfo.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "dialogfactory.lessInfo", "\u7c21\u6613\u60c5\u5831(L)" },
        { "dialogfactory.lessInfo.acceleratorKey", new Integer(KeyEvent.VK_L)},
	{ "dialogfactory.java.home.link", "http://www.java.com" },
	{ "dialogfactory.general_error", "<html><b>\u4e00\u822c\u4f8b\u5916</b></html>" },
	{ "dialogfactory.net_error", "<html><b>\u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u4f8b\u5916</b></html>" },
	{ "dialogfactory.security_error", "<html><b>\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u4f8b\u5916</b></html>" },
	{ "dialogfactory.ext_error", "<html><b>\u30aa\u30d7\u30b7\u30e7\u30f3\u30d1\u30c3\u30b1\u30fc\u30b8\u4f8b\u5916</b></html>" },
	{ "dialogfactory.user.selected", "\u9078\u629e\u3055\u308c\u305f\u30e6\u30fc\u30b6: {0}" },
	{ "dialogfactory.user.typed", "\u30e6\u30fc\u30b6\u30bf\u30a4\u30d7: {0}" },

	{ "deploycertstore.cert.loading", "\u914d\u5099\u8a3c\u660e\u66f8\u3092 {0} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
	{ "deploycertstore.cert.loaded", "\u914d\u5099\u8a3c\u660e\u66f8\u3092 {0} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u307e\u3057\u305f" },
	{ "deploycertstore.cert.saving", "\u914d\u5099\u8a3c\u660e\u66f8\u3092 {0} \u306b\u4fdd\u5b58\u3057\u3066\u3044\u307e\u3059" },
	{ "deploycertstore.cert.saved", "\u914d\u5099\u8a3c\u660e\u66f8\u3092 {0} \u306b\u4fdd\u5b58\u3057\u307e\u3057\u305f" },
	{ "deploycertstore.cert.adding", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u8ffd\u52a0\u3057\u3066\u3044\u307e\u3059", },
	{ "deploycertstore.cert.added", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u5225\u540d {0} \u3067\u8ffd\u52a0\u3057\u307e\u3057\u305f" },
	{ "deploycertstore.cert.removing", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u3066\u3044\u307e\u3059" },
	{ "deploycertstore.cert.removed", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u5225\u540d {0} \u306e\u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u307e\u3057\u305f" },
	{ "deploycertstore.cert.instore", "\u8a3c\u660e\u66f8\u304c\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u5b58\u5728\u3059\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u3066\u3044\u307e\u3059" },
	{ "deploycertstore.cert.canverify", "\u8a3c\u660e\u66f8\u304c\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u8a3c\u660e\u66f8\u3092\u4f7f\u3063\u3066\u691c\u8a3c\u3067\u304d\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "deploycertstore.cert.iterator", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u3042\u308b\u8a3c\u660e\u66f8\u53cd\u5fa9\u5b50\u3092\u53d6\u5f97\u3057\u307e\u3059" },
	{ "deploycertstore.cert.getkeystore", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u30ad\u30fc\u30b9\u30c8\u30a2\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u3092\u53d6\u5f97\u3057\u307e\u3059" },

	{ "httpscertstore.cert.loading", "\u914d\u5099 SSL \u8a3c\u660e\u66f8\u3092 {0} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
	{ "httpscertstore.cert.loaded", "\u914d\u5099 SSL \u8a3c\u660e\u66f8\u3092 {0} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u307e\u3057\u305f" },
	{ "httpscertstore.cert.saving", "\u914d\u5099 SSL \u8a3c\u660e\u66f8\u3092 {0} \u306b\u4fdd\u5b58\u3057\u3066\u3044\u307e\u3059" },
	{ "httpscertstore.cert.saved", "\u914d\u5099 SSL \u8a3c\u660e\u66f8\u3092 {0} \u306b\u4fdd\u5b58\u3057\u307e\u3057\u305f" },
	{ "httpscertstore.cert.adding", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b SSL \u8a3c\u660e\u66f8\u3092\u8ffd\u52a0\u3057\u3066\u3044\u307e\u3059", },
	{ "httpscertstore.cert.added", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b SSL \u8a3c\u660e\u66f8\u3092\u5225\u540d {0} \u3068\u3057\u3066\u8ffd\u52a0\u3057\u307e\u3057\u305f" },
	{ "httpscertstore.cert.removing", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e SSL \u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u3066\u3044\u307e\u3059" },
	{ "httpscertstore.cert.removed", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u5225\u540d {0} \u306e SSL \u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u307e\u3057\u305f" },
	{ "httpscertstore.cert.instore", "SSL \u8a3c\u660e\u66f8\u304c\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u5b58\u5728\u3059\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u3066\u3044\u307e\u3059" },
	{ "httpscertstore.cert.canverify", "SSL \u8a3c\u660e\u66f8\u304c\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u8a3c\u660e\u66f8\u3092\u4f7f\u3063\u3066\u691c\u8a3c\u3067\u304d\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "httpscertstore.cert.iterator", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u3042\u308b SSL \u8a3c\u660e\u66f8\u53cd\u5fa9\u5b50\u3092\u53d6\u5f97\u3057\u307e\u3059" },
	{ "httpscertstore.cert.getkeystore", "\u914d\u5099\u6c38\u4e45\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u30ad\u30fc\u30b9\u30c8\u30a2\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u3092\u53d6\u5f97\u3057\u307e\u3059" },

	{ "rootcertstore.cert.loading", "Root CA \u8a3c\u660e\u66f8\u3092 {0} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
	{ "rootcertstore.cert.loaded", "Root CA \u8a3c\u660e\u66f8\u3092 {0} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u307e\u3057\u305f" },
	{ "rootcertstore.cert.noload", "Root CA \u8a3c\u660e\u66f8\u30d5\u30a1\u30a4\u30eb\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093: {0}" },
	{ "rootcertstore.cert.saving", "Root CA \u8a3c\u660e\u66f8\u3092 {0} \u306b\u4fdd\u5b58\u3057\u3066\u3044\u307e\u3059" },
	{ "rootcertstore.cert.saved", "Root CA \u8a3c\u660e\u66f8\u3092 {0} \u306b\u4fdd\u5b58\u3057\u307e\u3057\u305f" },
	{ "rootcertstore.cert.adding", "Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u8ffd\u52a0\u3057\u3066\u3044\u307e\u3059", },
	{ "rootcertstore.cert.added", "Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u5225\u540d {0} \u3068\u3057\u3066\u8ffd\u52a0\u3057\u307e\u3057\u305f" },
	{ "rootcertstore.cert.removing", "Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u3066\u3044\u307e\u3059" },
	{ "rootcertstore.cert.removed", "Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u5225\u540d {0} \u306e\u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u307e\u3057\u305f" },
	{ "rootcertstore.cert.instore", "\u8a3c\u660e\u66f8\u304c Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u5b58\u5728\u3059\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u3066\u3044\u307e\u3059" },
	{ "rootcertstore.cert.canverify", "\u8a3c\u660e\u66f8\u304c Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u8a3c\u660e\u66f8\u3092\u4f7f\u3063\u3066\u691c\u8a3c\u3067\u304d\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "rootcertstore.cert.iterator", "Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u3042\u308b\u8a3c\u660e\u66f8\u53cd\u5fa9\u5b50\u3092\u53d6\u5f97\u3057\u307e\u3059" },
	{ "rootcertstore.cert.getkeystore", "Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u30ad\u30fc\u30b9\u30c8\u30a2\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u3092\u53d6\u5f97\u3057\u307e\u3059" },
	{ "rootcertstore.cert.verify.ok", "Root CA \u8a3c\u660e\u66f8\u306b\u3088\u308b\u8a3c\u660e\u66f8\u306e\u691c\u8a3c\u306b\u6210\u529f\u3057\u307e\u3057\u305f" },
	{ "rootcertstore.cert.verify.fail", "Root CA \u8a3c\u660e\u66f8\u306b\u3088\u308b\u8a3c\u660e\u66f8\u306e\u691c\u8a3c\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
	{ "rootcertstore.cert.tobeverified", "\u691c\u8a3c\u3055\u308c\u308b\u8a3c\u660e\u66f8:\n{0}" },
	{ "rootcertstore.cert.tobecompared", "\u6bd4\u8f03\u5bfe\u8c61\u306e Root CA \u8a3c\u660e\u66f8:\n{0}" },

	{ "roothttpscertstore.cert.loading", "SSL Root CA \u8a3c\u660e\u66f8\u3092 {0} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
	{ "roothttpscertstore.cert.loaded", "SSL Root CA \u8a3c\u660e\u66f8\u3092 {0} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u307e\u3057\u305f" },
	{ "roothttpscertstore.cert.noload", "SSL Root CA \u8a3c\u660e\u66f8\u30d5\u30a1\u30a4\u30eb\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093: {0}" },
	{ "roothttpscertstore.cert.saving", "SSL Root CA \u8a3c\u660e\u66f8\u3092 {0} \u306b\u4fdd\u5b58\u3057\u3066\u3044\u307e\u3059" },
	{ "roothttpscertstore.cert.saved", "SSL Root CA \u8a3c\u660e\u66f8\u3092 {0} \u306b\u4fdd\u5b58\u3057\u307e\u3057\u305f" },
	{ "roothttpscertstore.cert.adding", "SSL Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u8ffd\u52a0\u3057\u3066\u3044\u307e\u3059", },
	{ "roothttpscertstore.cert.added", "SSL Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u5225\u540d {0} \u3067\u8ffd\u52a0\u3057\u307e\u3057\u305f" },
	{ "roothttpscertstore.cert.removing", "SSL Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u3066\u3044\u307e\u3059" },
	{ "roothttpscertstore.cert.removed", "SSL Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u5225\u540d {0} \u3067\u524a\u9664\u3057\u307e\u3057\u305f" },
	{ "roothttpscertstore.cert.instore", "\u8a3c\u660e\u66f8\u304c SSL Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u5b58\u5728\u3059\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u3066\u3044\u307e\u3059" },
	{ "roothttpscertstore.cert.canverify", "\u8a3c\u660e\u66f8\u304c SSL Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u8a3c\u660e\u66f8\u3092\u4f7f\u3063\u3066\u691c\u8a3c\u3067\u304d\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "roothttpscertstore.cert.iterator", "SSL Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u3042\u308b\u8a3c\u660e\u66f8\u53cd\u5fa9\u5b50\u3092\u53d6\u5f97\u3057\u307e\u3059" },
	{ "roothttpscertstore.cert.getkeystore", "SSL Root CA \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u30ad\u30fc\u30b9\u30c8\u30a2\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u3092\u53d6\u5f97\u3057\u307e\u3059" },
	{ "roothttpscertstore.cert.verify.ok", "SSL Root CA \u8a3c\u660e\u66f8\u306b\u3088\u308b\u8a3c\u660e\u66f8\u306e\u691c\u8a3c\u306b\u6210\u529f\u3057\u307e\u3057\u305f" },
	{ "roothttpscertstore.cert.verify.fail", "SSL Root CA \u8a3c\u660e\u66f8\u306b\u3088\u308b\u8a3c\u660e\u66f8\u306e\u691c\u8a3c\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
	{ "roothttpscertstore.cert.tobeverified", "\u691c\u8a3c\u3055\u308c\u308b\u8a3c\u660e\u66f8:\n{0}" },
	{ "roothttpscertstore.cert.tobecompared", "\u6bd4\u8f03\u5bfe\u8c61\u306e SSL Root CA \u8a3c\u660e\u66f8:\n{0}" },

	{ "sessioncertstore.cert.loading", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
	{ "sessioncertstore.cert.loaded", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u30ed\u30fc\u30c9\u3057\u307e\u3057\u305f" },
	{ "sessioncertstore.cert.saving", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u4fdd\u5b58\u3057\u3066\u3044\u307e\u3059" },
	{ "sessioncertstore.cert.saved", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u4fdd\u5b58\u3057\u307e\u3057\u305f" },
	{ "sessioncertstore.cert.adding", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u8ffd\u52a0\u3057\u3066\u3044\u307e\u3059", },
	{ "sessioncertstore.cert.added", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306b\u8a3c\u660e\u66f8\u3092\u8ffd\u52a0\u3057\u307e\u3057\u305f" },
	{ "sessioncertstore.cert.removing", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u3066\u3044\u307e\u3059" },
	{ "sessioncertstore.cert.removed", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u307e\u3057\u305f" },
	{ "sessioncertstore.cert.instore", "\u8a3c\u660e\u66f8\u304c\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u5b58\u5728\u3059\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u3066\u3044\u307e\u3059" },
	{ "sessioncertstore.cert.canverify", "\u8a3c\u660e\u66f8\u304c\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u8a3c\u660e\u66f8\u3092\u4f7f\u3063\u3066\u691c\u8a3c\u3067\u304d\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "sessioncertstore.cert.iterator", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u3042\u308b\u8a3c\u660e\u66f8\u53cd\u5fa9\u5b50\u3092\u53d6\u5f97\u3057\u307e\u3059" },
	{ "sessioncertstore.cert.getkeystore", "\u914d\u5099\u30bb\u30c3\u30b7\u30e7\u30f3\u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u30ad\u30fc\u30b9\u30c8\u30a2\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u3092\u53d6\u5f97\u3057\u307e\u3059" },

	{ "iexplorer.cert.loading", "Internet Explorer {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
	{ "iexplorer.cert.loaded", "Internet Explorer {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u30ed\u30fc\u30c9\u3057\u307e\u3057\u305f" },
	{ "iexplorer.cert.instore", "\u8a3c\u660e\u66f8\u304c Internet Explorer {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u5b58\u5728\u3059\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u3066\u3044\u307e\u3059" },
	{ "iexplorer.cert.canverify", "\u8a3c\u660e\u66f8\u304c Internet Explorer {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u306e\u8a3c\u660e\u66f8\u3092\u4f7f\u3063\u3066\u691c\u8a3c\u3067\u304d\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "iexplorer.cert.iterator", "Internet Explorer {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u3042\u308b\u8a3c\u660e\u66f8\u53cd\u5fa9\u5b50\u3092\u53d6\u5f97\u3057\u307e\u3059" },
	{ "iexplorer.cert.verify.ok", "Internet Explorer {0} \u8a3c\u660e\u66f8\u306b\u3088\u308b\u8a3c\u660e\u66f8\u306e\u691c\u8a3c\u306b\u6210\u529f\u3057\u307e\u3057\u305f" },
	{ "iexplorer.cert.verify.fail", "Internet Explorer {0} \u8a3c\u660e\u66f8\u306b\u3088\u308b\u8a3c\u660e\u66f8\u306e\u691c\u8a3c\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
	{ "iexplorer.cert.tobeverified", "\u691c\u8a3c\u3055\u308c\u308b\u8a3c\u660e\u66f8:\n{0}" },
	{ "iexplorer.cert.tobecompared", "\u6bd4\u8f03\u5bfe\u8c61\u306e Internet Explorer {0} \u8a3c\u660e\u66f8:\n{1}" },

	{ "mozilla.cert.loading", "Mozilla {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
        { "mozilla.cert.loaded", "Mozilla {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u304b\u3089\u8a3c\u660e\u66f8\u3092\u30ed\u30fc\u30c9\u3057\u307e\u3057\u305f" },
        { "mozilla.cert.instore", "\u8a3c\u660e\u66f8\u304c Mozilla {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u5b58\u5728\u3059\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u3066\u3044\u307e\u3059" },
        { "mozilla.cert.canverify", "\u8a3c\u660e\u66f8\u304c Mozilla {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306e\u8a3c\u660e\u66f8\u3092\u4f7f\u3063\u3066\u691c\u8a3c\u3067\u304d\u308b\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
        { "mozilla.cert.iterator", "Mozilla {0} \u8a3c\u660e\u66f8\u30b9\u30c8\u30a2\u5185\u306b\u3042\u308b\u8a3c\u660e\u66f8\u53cd\u5fa9\u5b50\u3092\u53d6\u5f97\u3057\u307e\u3059" },
        { "mozilla.cert.verify.ok", "Mozilla {0} \u8a3c\u660e\u66f8\u306b\u3088\u308b\u8a3c\u660e\u66f8\u306e\u691c\u8a3c\u306b\u6210\u529f\u3057\u307e\u3057\u305f" },
        { "mozilla.cert.verify.fail", "Mozilla {0} \u8a3c\u660e\u66f8\u306b\u3088\u308b\u8a3c\u660e\u66f8\u306e\u691c\u8a3c\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
        { "mozilla.cert.tobeverified", "\u691c\u8a3c\u3055\u308c\u308b\u8a3c\u660e\u66f8:\n{0}" },
        { "mozilla.cert.tobecompared", "\u6bd4\u8f03\u5bfe\u8c61\u306e Mozilla {0} \u8a3c\u660e\u66f8:\n{1}" },

        { "browserkeystore.jss.no", "JSS \u30d1\u30c3\u30b1\u30fc\u30b8\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093" },
        { "browserkeystore.jss.yes", "JSS \u30d1\u30c3\u30b1\u30fc\u30b8\u304c\u30ed\u30fc\u30c9\u3055\u308c\u307e\u3057\u305f" },
        { "browserkeystore.jss.notconfig", "JSS \u304c\u69cb\u6210\u3055\u308c\u307e\u305b\u3093" },
        { "browserkeystore.jss.config", "JSS \u306f\u69cb\u6210\u3055\u308c\u307e\u3059" },
        { "browserkeystore.mozilla.dir", "Mozilla \u30e6\u30fc\u30b6\u30d7\u30ed\u30d5\u30a1\u30a4\u30eb\u5185\u306e\u30ad\u30fc\u3068\u8a3c\u660e\u66f8\u306b\u30a2\u30af\u30bb\u30b9\u3057\u3066\u3044\u307e\u3059: {0}" },
	{ "browserkeystore.password.dialog.buttonOK", "\u4e86\u89e3(O)" },
	{ "browserkeystore.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "browserkeystore.password.dialog.buttonCancel", "\u53d6\u6d88\u3057(C)" },
	{ "browserkeystore.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "browserkeystore.password.dialog.caption", "\u30d1\u30b9\u30ef\u30fc\u30c9\u304c\u5fc5\u8981\u3067\u3059" },
	{ "browserkeystore.password.dialog.text", "{0} \u306e\u30d1\u30b9\u30ef\u30fc\u30c9\u3092\u5165\u529b\u3057\u3066\u304f\u3060\u3055\u3044:\n" },
	{ "mozillamykeystore.priv.notfound", "\u6b21\u306e\u8a3c\u660e\u66f8\u306e\u975e\u516c\u958b\u9375\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093: {0}" },
	{ "hostnameverifier.automation.ignoremismatch", "\u81ea\u52d5\u5316: \u30db\u30b9\u30c8\u540d\u306e\u4e0d\u4e00\u81f4\u3092\u7121\u8996" },

	{ "trustdecider.check.basicconstraints", "\u8a3c\u660e\u66f8\u306e\u57fa\u672c\u5236\u7d04\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.leafkeyusage", "\u8a3c\u660e\u66f8\u306e\u30ea\u30fc\u30d5\u30ad\u30fc\u4f7f\u7528\u6cd5\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.signerkeyusage", "\u8a3c\u660e\u66f8\u306e\u7f72\u540d\u8005\u30ad\u30fc\u4f7f\u7528\u6cd5\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.extensions", "\u8a3c\u660e\u66f8\u306e\u30af\u30ea\u30c6\u30a3\u30ab\u30eb\u62e1\u5f35\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.signature", "\u8a3c\u660e\u66f8\u306e\u7f72\u540d\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.basicconstraints.certtypebit", "\u8a3c\u660e\u66f8\u306e Netscape \u578b\u306e\u30d3\u30c3\u30c8\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.basicconstraints.extensionvalue", "\u8a3c\u660e\u66f8\u306e Netscape \u62e1\u5f35\u5024\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.basicconstraints.bitvalue", "\u8a3c\u660e\u66f8\u306e Netscape \u578b\u306e\u30d3\u30c3\u30c8 5\u30016\u30017 \u306e\u5024\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.basicconstraints.enduser", "\u8a3c\u660e\u66f8\u306e CA \u3068\u3057\u3066\u6a5f\u80fd\u3059\u308b\u30a8\u30f3\u30c9\u30e6\u30fc\u30b6\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.basicconstraints.pathlength", "\u8a3c\u660e\u66f8\u306e\u30d1\u30b9\u306e\u9577\u3055\u5236\u7d04\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.leafkeyusage.length", "\u8a3c\u660e\u66f8\u306e\u30ad\u30fc\u4f7f\u7528\u6cd5\u306e\u9577\u3055\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.leafkeyusage.digitalsignature", "\u8a3c\u660e\u66f8\u306e\u30c7\u30b8\u30bf\u30eb\u7f72\u540d\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.leafkeyusage.extkeyusageinfo", "\u8a3c\u660e\u66f8\u306e\u62e1\u5f35\u30ad\u30fc\u4f7f\u7528\u6cd5\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.leafkeyusage.tsaextkeyusageinfo", "TSA \u62e1\u5f35\u30ad\u30fc\u306e\u4f7f\u7528\u6cd5\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.leafkeyusage.certtypebit", "\u8a3c\u660e\u66f8\u306e Netscape \u578b\u306e\u30d3\u30c3\u30c8\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.signerkeyusage.lengthandbit", "\u8a3c\u660e\u66f8\u306e\u9577\u3055\u3068\u30d3\u30c3\u30c8\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.signerkeyusage.keyusage", "\u8a3c\u660e\u66f8\u306e\u30ad\u30fc\u4f7f\u7528\u6cd5\u304c\u4e0d\u6b63\u3067\u306a\u3044\u304b\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059" },
	{ "trustdecider.check.canonicalize.updatecert", "cacerts \u30d5\u30a1\u30a4\u30eb\u5185\u306e\u8a3c\u660e\u66f8\u3092\u4f7f\u3063\u3066\u30eb\u30fc\u30c8\u8a3c\u660e\u66f8\u3092\u66f4\u65b0\u3057\u307e\u3059" },
	{ "trustdecider.check.canonicalize.missing", "\u4e0d\u8db3\u3057\u3066\u3044\u308b\u30eb\u30fc\u30c8\u8a3c\u660e\u66f8\u3092\u8ffd\u52a0\u3057\u307e\u3059" },
	{ "trustdecider.check.gettrustedcert.find", "\u6709\u52b9\u306a\u30eb\u30fc\u30c8 CA \u3092 cacerts \u30d5\u30a1\u30a4\u30eb\u5185\u3067\u691c\u7d22\u3057\u307e\u3059" },
	{ "trustdecider.check.gettrustedissuercert.find", "\u4e0d\u8db3\u3057\u3066\u3044\u308b\u6709\u52b9\u306a\u30eb\u30fc\u30c8 CA \u3092 cacerts \u30d5\u30a1\u30a4\u30eb\u5185\u3067\u691c\u7d22\u3057\u307e\u3059" },
	{ "trustdecider.check.timestamping.no", "\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7\u60c5\u5831\u306f\u3042\u308a\u307e\u305b\u3093" },
	{ "trustdecider.check.timestamping.yes", "\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7\u60c5\u5831\u304c\u3042\u308a\u307e\u3059" },
	{ "trustdecider.check.timestamping.tsapath", "TSA \u8a3c\u660e\u66f8\u30d1\u30b9\u306e\u30c1\u30a7\u30c3\u30af\u3092\u958b\u59cb\u3057\u307e\u3059" },
	{ "trustdecider.check.timestamping.inca", "\u8a3c\u660e\u66f8\u306f\u671f\u9650\u5207\u308c\u3067\u3059\u304c\u3001\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7\u306e\u6642\u671f\u3068 TSA \u304c\u6709\u52b9\u3067\u3059" },
	{ "trustdecider.check.timestamping.notinca", "\u8a3c\u660e\u66f8\u306f\u671f\u9650\u5207\u308c\u3067\u3001TSA \u304c\u7121\u52b9\u3067\u3059" },
	{ "trustdecider.check.timestamping.valid", "\u8a3c\u660e\u66f8\u306f\u671f\u9650\u5207\u308c\u3067\u3059\u304c\u3001\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7\u306e\u6642\u671f\u306f\u6709\u52b9\u3067\u3059" },
	{ "trustdecider.check.timestamping.invalid", "\u8a3c\u660e\u66f8\u306f\u671f\u9650\u5207\u308c\u3067\u3001\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7\u306e\u6642\u671f\u304c\u7121\u52b9\u3067\u3059" },
	{ "trustdecider.check.timestamping.need", "\u8a3c\u660e\u66f8\u306e\u671f\u9650\u304c\u5207\u308c\u3066\u3044\u307e\u3059\u3002\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7\u60c5\u5831\u3092\u30c1\u30a7\u30c3\u30af\u3057\u3066\u304f\u3060\u3055\u3044" },
	{ "trustdecider.check.timestamping.noneed", "\u8a3c\u660e\u66f8\u306e\u671f\u9650\u306f\u307e\u3060\u5207\u308c\u3066\u3044\u307e\u305b\u3093\u3002\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7\u60c5\u5831\u3092\u30c1\u30a7\u30c3\u30af\u3059\u308b\u5fc5\u8981\u306f\u3042\u308a\u307e\u305b\u3093" },
	{ "trustdecider.check.timestamping.notfound", "\u65b0\u3057\u3044\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7 API \u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093\u3067\u3057\u305f" },
	{ "trustdecider.user.grant.session", "\u30e6\u30fc\u30b6\u306f\u3053\u306e\u30bb\u30c3\u30b7\u30e7\u30f3\u3067\u306e\u307f\u3001\u3053\u306e\u30b3\u30fc\u30c9\u306b\u6a29\u9650\u3092\u4ed8\u4e0e\u3057\u307e\u3057\u305f" },
	{ "trustdecider.user.grant.forever", "\u30e6\u30fc\u30b6\u306f\u6c38\u7d9a\u7684\u306b\u3001\u3053\u306e\u30b3\u30fc\u30c9\u306b\u6a29\u9650\u3092\u4ed8\u4e0e\u3057\u307e\u3057\u305f" },
	{ "trustdecider.user.deny", "\u30e6\u30fc\u30b6\u306f\u3001\u3053\u306e\u30b3\u30fc\u30c9\u3078\u306e\u6a29\u9650\u4ed8\u4e0e\u3092\u62d2\u5426\u3057\u307e\u3057\u305f" },
	{ "trustdecider.automation.trustcert", "\u81ea\u52d5\u5316: \u7f72\u540d\u7528\u306e RSA \u8a3c\u660e\u66f8\u3092\u4fe1\u983c\u3059\u308b" },
	{ "trustdecider.code.type.applet", "\u30a2\u30d7\u30ec\u30c3\u30c8" },
	{ "trustdecider.code.type.application", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3" },
	{ "trustdecider.code.type.extension", "\u62e1\u5f35" },
	{ "trustdecider.code.type.installer", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30e9" },
	{ "trustdecider.user.cannot.grant.any", "\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a2d\u5b9a\u306b\u3088\u308a\u3001\u65b0\u3057\u3044\u8a3c\u660e\u66f8\u306b\u6a29\u9650\u3092\u4ed8\u4e0e\u3067\u304d\u306a\u3044\u3088\u3046\u306b\u306a\u3063\u3066\u3044\u307e\u3059\u3002" },
	{ "trustdecider.user.cannot.grant.notinca", "\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8a2d\u5b9a\u306b\u3088\u308a\u3001\u81ea\u8eab\u3067\u7f72\u540d\u3057\u305f\u8a3c\u660e\u66f8\u306b\u6a29\u9650\u3092\u4ed8\u4e0e\u3067\u304d\u306a\u3044\u3088\u3046\u306b\u306a\u3063\u3066\u3044\u307e\u3059\u3002" },
	{ "x509trustmgr.automation.ignoreclientcert", "\u81ea\u52d5\u5316: \u4fe1\u983c\u3067\u304d\u306a\u3044\u30af\u30e9\u30a4\u30a2\u30f3\u30c8\u8a3c\u660e\u66f8\u3092\u7121\u8996\u3059\u308b" },
	{ "x509trustmgr.automation.ignoreservercert", "\u81ea\u52d5\u5316: \u4fe1\u983c\u3067\u304d\u306a\u3044\u30b5\u30fc\u30d0\u8a3c\u660e\u66f8\u3092\u7121\u8996\u3059\u308b" },

	{ "net.proxy.text", "\u30d7\u30ed\u30ad\u30b7: " },
	{ "net.proxy.override.text", "\u30d7\u30ed\u30ad\u30b7\u306e\u4e0a\u66f8\u304d: " },
	{ "net.proxy.configuration.text", "\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a: " },
	{ "net.proxy.type.browser", "\u30d6\u30e9\u30a6\u30b6\u306e\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a" },
	{ "net.proxy.type.auto", "\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a" },
	{ "net.proxy.type.manual", "\u624b\u52d5\u8a2d\u5b9a" },
	{ "net.proxy.type.none", "\u30d7\u30ed\u30ad\u30b7\u306a\u3057" },
	{ "net.proxy.type.user", "\u30e6\u30fc\u30b6\u306b\u3088\u3063\u3066\u30d6\u30e9\u30a6\u30b6\u306e\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u304c\u4e0a\u66f8\u304d\u3055\u308c\u307e\u3057\u305f\u3002" },
	{ "net.proxy.loading.ie", "Internet Explorer \u304b\u3089\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 ..."},
	{ "net.proxy.loading.ns", "Netscape Navigator \u304b\u3089\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 ..."},
	{ "net.proxy.loading.userdef", "\u30e6\u30fc\u30b6\u5b9a\u7fa9\u306e\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 ..."},
	{ "net.proxy.loading.direct", "\u30c0\u30a4\u30ec\u30af\u30c8\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 ..."},
	{ "net.proxy.loading.manual", "\u624b\u52d5\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 ..."},
	{ "net.proxy.loading.auto",   "\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 ..."},
	{ "net.proxy.loading.browser",   "\u30d6\u30e9\u30a6\u30b6\u306e\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 ..."},
	{ "net.proxy.loading.manual.error", "\u624b\u52d5\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u4f7f\u7528\u3067\u304d\u307e\u305b\u3093 - DIRECT \u306b\u5207\u308a\u66ff\u3048\u307e\u3059"},
	{ "net.proxy.loading.auto.error", "\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u4f7f\u7528\u3067\u304d\u307e\u305b\u3093 - MANUAL \u306b\u5207\u308a\u66ff\u3048\u307e\u3059"},
	{ "net.proxy.loading.done", "\u5b8c\u4e86\u3002"},
	{ "net.proxy.browser.pref.read", "\u30e6\u30fc\u30b6\u8a2d\u5b9a\u30d5\u30a1\u30a4\u30eb\u3092 {0} \u304b\u3089\u8aad\u307f\u53d6\u3063\u3066\u3044\u307e\u3059"},
	{ "net.proxy.browser.proxyEnable", "    \u6709\u52b9\u306a\u30d7\u30ed\u30ad\u30b7: {0}"},
	{ "net.proxy.browser.proxyList",     "    \u30d7\u30ed\u30ad\u30b7\u30ea\u30b9\u30c8: {0}"},
	{ "net.proxy.browser.proxyOverride", "    \u30d7\u30ed\u30ad\u30b7\u306e\u4e0a\u66f8\u304d: {0}"},
	{ "net.proxy.browser.autoConfigURL", "    \u81ea\u52d5\u8a2d\u5b9a URL: {0}"},
	{ "net.proxy.browser.smartConfig", "\u30dd\u30fc\u30c8 {1} \u4e0a\u306e\u30d7\u30ed\u30ad\u30b7\u30b5\u30fc\u30d0 {0} \u306b\u5bfe\u3057\u3066 Ping \u3092\u5b9f\u884c"},
        { "net.proxy.browser.connectionException", "\u30dd\u30fc\u30c8 {1} \u4e0a\u306e\u30d7\u30ed\u30ad\u30b7\u30b5\u30fc\u30d0 {0} \u306b\u30a2\u30af\u30bb\u30b9\u3067\u304d\u307e\u305b\u3093"},
	{ "net.proxy.ns6.regs.exception", "\u30ec\u30b8\u30b9\u30c8\u30ea\u30d5\u30a1\u30a4\u30eb\u306e\u8aad\u307f\u53d6\u308a\u30a8\u30e9\u30fc: {0}"},
	{ "net.proxy.pattern.convert", "\u30d7\u30ed\u30ad\u30b7\u30d0\u30a4\u30d1\u30b9\u30ea\u30b9\u30c8\u3092\u6b63\u898f\u8868\u73fe\u306b\u5909\u63db: "},
	{ "net.proxy.pattern.convert.error", "\u30d7\u30ed\u30ad\u30b7\u30d0\u30a4\u30d1\u30b9\u30ea\u30b9\u30c8\u3092\u6b63\u898f\u8868\u73fe\u306b\u5909\u63db\u3067\u304d\u307e\u305b\u3093 - \u7121\u8996"},
	{ "net.proxy.auto.download.ins", "INS \u30d5\u30a1\u30a4\u30eb\u3092 {0} \u304b\u3089\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
	{ "net.proxy.auto.download.js", "\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u30d5\u30a1\u30a4\u30eb\u3092 {0} \u304b\u3089\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
	{ "net.proxy.auto.result.error", "\u8a55\u4fa1\u304b\u3089\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u5224\u5b9a\u3067\u304d\u307e\u305b\u3093 - DIRECT \u306b\u5207\u308a\u66ff\u3048\u307e\u3059"},
        { "net.proxy.service.not_available", "{0} \u306b\u5bfe\u3059\u308b\u30d7\u30ed\u30ad\u30b7\u30b5\u30fc\u30d3\u30b9\u304c\u5229\u7528\u3067\u304d\u307e\u305b\u3093 - \u30c7\u30d5\u30a9\u30eb\u30c8\u3092 DIRECT \u306b\u8a2d\u5b9a\u3057\u307e\u3059" },
	{ "net.proxy.error.caption", "\u30a8\u30e9\u30fc - \u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a" },
	{ "net.proxy.nsprefs.error", "<html><b>\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u306e\u53d6\u5f97\u4e0d\u53ef</b></html>\u307b\u304b\u306e\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u306b\u5207\u308a\u66ff\u3048\u307e\u3059\u3002\n" },
	{ "net.proxy.connect", "{0} \u306b\u63a5\u7d9a (\u30d7\u30ed\u30ad\u30b7={1})" },

	{ "net.authenticate.caption", "\u30d1\u30b9\u30ef\u30fc\u30c9\u306e\u8981\u6c42 - \u30cd\u30c3\u30c8\u30ef\u30fc\u30af"},
	{ "net.authenticate.label", "<html><b>\u30e6\u30fc\u30b6\u540d\u3068\u30d1\u30b9\u30ef\u30fc\u30c9\u3092\u5165\u529b\u3057\u3066\u304f\u3060\u3055\u3044:</b></html>"},
	{ "net.authenticate.resource", "\u30ea\u30bd\u30fc\u30b9:" },
	{ "net.authenticate.username", "\u30e6\u30fc\u30b6\u540d(U):" },
        { "net.authenticate.username.mnemonic", "VK_U" },
	{ "net.authenticate.password", "\u30d1\u30b9\u30ef\u30fc\u30c9(P):" },
        { "net.authenticate.password.mnemonic", "VK_P" },
	{ "net.authenticate.firewall", "\u30b5\u30fc\u30d0:" },
	{ "net.authenticate.domain", "\u30c9\u30e1\u30a4\u30f3(D):"},
        { "net.authenticate.domain.mnemonic", "VK_D" },
	{ "net.authenticate.realm", "\u30ec\u30eb\u30e0:" },
	{ "net.authenticate.scheme", "\u30b9\u30ad\u30fc\u30de:" },
	{ "net.authenticate.unknownSite", "\u672a\u77e5\u306e\u30b5\u30a4\u30c8" },

	{ "net.cookie.cache", "Cookie \u30ad\u30e3\u30c3\u30b7\u30e5: " },
	{ "net.cookie.server", "\u30b5\u30fc\u30d0 {0} \u304c  \"{1}\" \u3067 Cookie \u8a2d\u5b9a\u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059" },
	{ "net.cookie.connect", "Cookie \"{1}\" \u3092\u4f7f\u3063\u3066 {0} \u306b\u63a5\u7d9a\u3057\u3066\u3044\u307e\u3059" },
	{ "net.cookie.ignore.setcookie", "Cookie \u30b5\u30fc\u30d3\u30b9\u304c\u5229\u7528\u3067\u304d\u307e\u305b\u3093 - \"Set-Cookie\" \u3092\u7121\u8996\u3057\u307e\u3059" },
	{ "net.cookie.noservice", "Cookie \u30b5\u30fc\u30d3\u30b9\u304c\u5229\u7528\u3067\u304d\u307e\u305b\u3093 - \u30ad\u30e3\u30c3\u30b7\u30e5\u3092\u4f7f\u3063\u3066 \"Cookie\" \u3092\u6c7a\u5b9a\u3057\u307e\u3059" },

	{"about.java.version", "\u30d0\u30fc\u30b8\u30e7\u30f3 {0} (\u30d3\u30eb\u30c9 {1})" },
	{"about.prompt.info", "Java \u30c6\u30af\u30ce\u30ed\u30b8\u306b\u95a2\u3059\u308b\u8a73\u7d30\u3084\u512a\u308c\u305f Java \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3092\u63a2\u3059\u306b\u306f\u6b21\u306e\u30b5\u30a4\u30c8\u3092\u53c2\u7167: " },
	{"about.home.link", "http://www.java.com" },
	{"about.option.close", "\u9589\u3058\u308b(C)" },
	{"about.option.close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{"about.copyright", "Copyright 2004 Sun Microsystems, Inc."},
	{"about.legal.note", "All rights reserved. Use is subject to license terms."},


	{ "cert.remove_button", "\u524a\u9664(M)" },
        { "cert.remove_button.mnemonic", "VK_M" },
        { "cert.import_button", "\u30a4\u30f3\u30dd\u30fc\u30c8(I)" },
        { "cert.import_button.mnemonic", "VK_I" },
        { "cert.export_button", "\u30a8\u30af\u30b9\u30dd\u30fc\u30c8(E)" },
        { "cert.export_button.mnemonic", "VK_E" },
        { "cert.details_button", "\u8a73\u7d30(D)" },
        { "cert.details_button.mnemonic", "VK_D" },
        { "cert.viewcert_button", "\u8a3c\u660e\u66f8\u3092\u8868\u793a(V)" },
        { "cert.viewcert_button.mnemonic", "VK_V" },
        { "cert.close_button", "\u9589\u3058\u308b(C)" },
        { "cert.close_button.mnemonic", "VK_C" },
        { "cert.type.trusted_certs", "\u4fe1\u983c\u3067\u304d\u308b\u8a3c\u660e\u66f8" },
        { "cert.type.secure_site", "\u30bb\u30ad\u30e5\u30a2\u30b5\u30a4\u30c8" },
        { "cert.type.client_auth", "\u30af\u30e9\u30a4\u30a2\u30f3\u30c8\u8a8d\u8a3c" },
        { "cert.type.signer_ca", "\u7f72\u540d\u8005\u306e CA" },
        { "cert.type.secure_site_ca", "\u30bb\u30ad\u30e5\u30a2\u30b5\u30a4\u30c8\u306e CA" },
        { "cert.rbutton.user", "\u30e6\u30fc\u30b6" },
        { "cert.rbutton.system", "\u30b7\u30b9\u30c6\u30e0" },
        { "cert.settings", "\u8a3c\u660e\u66f8" },
        { "cert.dialog.import.error.caption", "\u30a8\u30e9\u30fc - \u8a3c\u660e\u66f8\u306e\u30a4\u30f3\u30dd\u30fc\u30c8" },
        { "cert.dialog.export.error.caption", "\u30a8\u30e9\u30fc - \u8a3c\u660e\u66f8\u306e\u30a8\u30af\u30b9\u30dd\u30fc\u30c8" },
	{ "cert.dialog.import.format.text", "<html><b>\u8a8d\u8b58\u3067\u304d\u306a\u3044\u30d5\u30a1\u30a4\u30eb\u5f62\u5f0f\u3067\u3059</b></html>\u8a3c\u660e\u66f8\u306f\u30a4\u30f3\u30dd\u30fc\u30c8\u3055\u308c\u307e\u305b\u3093\u3002" },
	{ "cert.dialog.export.password.text", "<html><b>\u7121\u52b9\u306a\u30d1\u30b9\u30ef\u30fc\u30c9</b></html>\u5165\u529b\u3057\u305f\u30d1\u30b9\u30ef\u30fc\u30c9\u304c\u6b63\u3057\u304f\u3042\u308a\u307e\u305b\u3093\u3002" },
	{ "cert.dialog.import.file.text", "<html><b>\u30d5\u30a1\u30a4\u30eb\u304c\u5b58\u5728\u3057\u307e\u305b\u3093</b></html>\u8a3c\u660e\u66f8\u306f\u30a4\u30f3\u30dd\u30fc\u30c8\u3055\u308c\u307e\u305b\u3093\u3002" },
	{ "cert.dialog.import.password.text", "<html><b>\u7121\u52b9\u306a\u30d1\u30b9\u30ef\u30fc\u30c9</b></html>\u5165\u529b\u3057\u305f\u30d1\u30b9\u30ef\u30fc\u30c9\u304c\u6b63\u3057\u304f\u3042\u308a\u307e\u305b\u3093\u3002" },
        { "cert.dialog.password.caption", "\u30d1\u30b9\u30ef\u30fc\u30c9" },
        { "cert.dialog.password.import.caption", "\u30d1\u30b9\u30ef\u30fc\u30c9\u306e\u8981\u6c42 - \u30a4\u30f3\u30dd\u30fc\u30c8" },
        { "cert.dialog.password.export.caption", "\u30d1\u30b9\u30ef\u30fc\u30c9\u306e\u8981\u6c42 - \u30a8\u30af\u30b9\u30dd\u30fc\u30c8" },
        { "cert.dialog.password.text", "\u3053\u306e\u30d5\u30a1\u30a4\u30eb\u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u305f\u3081\u306e\u30d1\u30b9\u30ef\u30fc\u30c9\u3092\u5165\u529b\u3057\u3066\u304f\u3060\u3055\u3044:\n" },
        { "cert.dialog.exportpassword.text", "\u30af\u30e9\u30a4\u30a2\u30f3\u30c8\u8a8d\u8a3c\u30ad\u30fc\u30b9\u30c8\u30a2\u5185\u306e\u79d8\u5bc6\u9375\u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u305f\u3081\u306e\u30d1\u30b9\u30ef\u30fc\u30c9\u3092\u5165\u529b\u3057\u3066\u304f\u3060\u3055\u3044:\n" },
        { "cert.dialog.savepassword.text", "\u3053\u306e\u30ad\u30fc\u30d5\u30a1\u30a4\u30eb\u3092\u4fdd\u5b58\u3059\u308b\u305f\u3081\u306e\u30d1\u30b9\u30ef\u30fc\u30c9\u3092\u5165\u529b\u3057\u3066\u304f\u3060\u3055\u3044:\n" },
        { "cert.dialog.password.okButton", "\u4e86\u89e3" },
        { "cert.dialog.password.cancelButton", "\u53d6\u6d88\u3057" },
        { "cert.dialog.export.error.caption", "\u30a8\u30e9\u30fc - \u8a3c\u660e\u66f8\u306e\u30a8\u30af\u30b9\u30dd\u30fc\u30c8" },
        { "cert.dialog.export.text", "<html><b>\u30a8\u30af\u30b9\u30dd\u30fc\u30c8\u3067\u304d\u307e\u305b\u3093</b></html>\u8a3c\u660e\u66f8\u306f\u30a8\u30af\u30b9\u30dd\u30fc\u30c8\u3055\u308c\u307e\u305b\u3093\u3002" },
        { "cert.dialog.remove.text", "\u8a3c\u660e\u66f8\u3092\u524a\u9664\u3057\u3066\u3082\u3088\u308d\u3057\u3044\u3067\u3059\u304b?" },
	{ "cert.dialog.remove.caption", "\u8a3c\u660e\u66f8\u306e\u524a\u9664" },
	{ "cert.dialog.issued.to", "\u767a\u884c\u5148" },
	{ "cert.dialog.issued.by", "\u767a\u884c\u5143" },
	{ "cert.dialog.user.level", "\u30e6\u30fc\u30b6" },
	{ "cert.dialog.system.level", "\u30b7\u30b9\u30c6\u30e0" },
	{ "cert.dialog.certtype", "\u8a3c\u660e\u66f8\u30bf\u30a4\u30d7: "},

	{ "controlpanel.jre.platformTableColumnTitle","\u30d7\u30e9\u30c3\u30c8\u30d5\u30a9\u30fc\u30e0"},
	{ "controlpanel.jre.productTableColumnTitle","\u88fd\u54c1" },
	{ "controlpanel.jre.locationTableColumnTitle","\u5834\u6240" },
	{ "controlpanel.jre.pathTableColumnTitle","\u30d1\u30b9" },
	{ "controlpanel.jre.enabledTableColumnTitle", "\u6709\u52b9" },

	{ "jnlp.jre.title", "JNLP \u30e9\u30f3\u30bf\u30a4\u30e0\u8a2d\u5b9a" },
	{ "jnlp.jre.versions", "Java \u30e9\u30f3\u30bf\u30a4\u30e0\u306e\u30d0\u30fc\u30b8\u30e7\u30f3" },
	{ "jnlp.jre.choose.button", "\u9078\u629e(H)" },
	{ "jnlp.jre.find.button", "\u691c\u7d22(F)" },
	{ "jnlp.jre.add.button", "\u8ffd\u52a0(A)" },
	{ "jnlp.jre.remove.button", "\u524a\u9664(R)" },
	{ "jnlp.jre.ok.button", "\u4e86\u89e3(O)" },
	{ "jnlp.jre.cancel.button", "\u53d6\u6d88\u3057(C)" },
	{ "jnlp.jre.choose.button.mnemonic", "VK_H" },
	{ "jnlp.jre.find.button.mnemonic", "VK_F" },
	{ "jnlp.jre.add.button.mnemonic", "VK_A" },
	{ "jnlp.jre.remove.button.mnemonic", "VK_R" },
	{ "jnlp.jre.ok.button.mnemonic", "VK_O" },
	{ "jnlp.jre.cancel.button.mnemonic", "VK_C" },

	{ "find.dialog.title", "JRE \u691c\u7d22"},
	{ "find.title", "Java Runtime Environment"},
	{ "find.cancelButton", "\u53d6\u6d88\u3057(C)"},
	{ "find.prevButton", "\u623b\u308b(P)"},
	{ "find.nextButton", "\u6b21\u3078(N)"},
	{ "find.cancelButtonMnemonic", "VK_C"},
	{ "find.prevButtonMnemonic", "VK_P"},
	{ "find.nextButtonMnemonic", "VK_N"},
	{ "find.intro", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3092\u8d77\u52d5\u3059\u308b\u306b\u306f Java Web Start \u306f\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u6e08\u307f Java Runtime Environment \u306e\u5834\u6240\u3092\u77e5\u308b\u5fc5\u8981\u304c\u3042\u308a\u307e\u3059\u3002\n\n\u65e2\u77e5\u306e JRE \u3092\u9078\u629e\u3059\u308b\u3053\u3068\u3082\u3001JRE \u3092\u691c\u7d22\u3057\u3066\u30d5\u30a1\u30a4\u30eb\u30b7\u30b9\u30c6\u30e0\u5185\u306e\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u9078\u629e\u3059\u308b\u3053\u3068\u3082\u3067\u304d\u307e\u3059\u3002" },

	{ "find.searching.title", "\u5229\u7528\u53ef\u80fd\u306a JRE \u3092\u691c\u7d22\u3057\u3066\u3044\u307e\u3059\u3002\u6570\u5206\u3092\u8981\u3059\u308b\u5834\u5408\u304c\u3042\u308a\u307e\u3059\u3002" },
	{ "find.searching.prefix", "\u78ba\u8a8d\u4e2d: " },
	{ "find.foundJREs.title", "\u6b21\u306e JRE \u304c\u898b\u3064\u304b\u308a\u307e\u3057\u305f\u3002\u3053\u308c\u3089\u3092\u8ffd\u52a0\u3059\u308b\u306b\u306f [\u6b21\u3078] \u3092\u30af\u30ea\u30c3\u30af\u3057\u3066\u304f\u3060\u3055\u3044" },
	{ "find.noJREs.title", "JRE \u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093\u3067\u3057\u305f\u3002\u5225\u306e\u691c\u7d22\u5834\u6240\u3092\u9078\u629e\u3059\u308b\u306b\u306f [\u623b\u308b] \u3092\u30af\u30ea\u30c3\u30af\u3057\u3066\u304f\u3060\u3055\u3044" },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Java(tm) \u914d\u5099\u7528\u30d7\u30ed\u30d1\u30c6\u30a3\n"
                        + "# \u3053\u306e\u30d5\u30a1\u30a4\u30eb\u3092\u7de8\u96c6\u3057\u306a\u3044\u3067\u304f\u3060\u3055\u3044\u3002\u3053\u308c\u306f\u81ea\u52d5\u751f\u6210\u3055\u308c\u305f\u30d5\u30a1\u30a4\u30eb\u3067\u3059\u3002\n"
                        + "# \u30d7\u30ed\u30d1\u30c6\u30a3\u3092\u7de8\u96c6\u3059\u308b\u306b\u306f Java \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb\u3092\u4f7f\u7528\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
        { "config.unknownSubject", "\u672a\u77e5\u306e\u88ab\u8a8d\u8a3c\u8005" },
        { "config.unknownIssuer", "\u672a\u77e5\u306e\u767a\u884c\u8005" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>\u7121\u52b9\u306a URL</b></html>\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u306e URL \u304c\u7121\u52b9\u3067\u3059\u3002" },
        { "config.proxy.autourl.invalid.caption", "\u30a8\u30e9\u30fc - \u30d7\u30ed\u30ad\u30b7" },
	// Java Web Start Properties
	 { "APIImpl.clipboard.message.read", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u30b7\u30b9\u30c6\u30e0\u306e\u30af\u30ea\u30c3\u30d7\u30dc\u30fc\u30c9\u3078\u306e\u8aad\u307f\u53d6\u308a\u5c02\u7528\u30a2\u30af\u30bb\u30b9\u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059\u3002\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u3001\u30af\u30ea\u30c3\u30d7\u30dc\u30fc\u30c9\u306b\u683c\u7d0d\u3055\u308c\u3066\u3044\u308b\u6a5f\u5bc6\u60c5\u5831\u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u53ef\u80fd\u6027\u304c\u3042\u308a\u307e\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u307e\u3059\u304b?" },
        { "APIImpl.clipboard.message.write", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u30b7\u30b9\u30c6\u30e0\u306e\u30af\u30ea\u30c3\u30d7\u30dc\u30fc\u30c9\u3078\u306e\u66f8\u304d\u8fbc\u307f\u30a2\u30af\u30bb\u30b9\u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059\u3002\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u3001\u30af\u30ea\u30c3\u30d7\u30dc\u30fc\u30c9\u306b\u683c\u7d0d\u3055\u308c\u3066\u3044\u308b\u60c5\u5831\u3092\u4e0a\u66f8\u304d\u3059\u308b\u53ef\u80fd\u6027\u304c\u3042\u308a\u307e\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u307e\u3059\u304b?" },
        { "APIImpl.file.open.message", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u30d5\u30a1\u30a4\u30eb\u30b7\u30b9\u30c6\u30e0\u3078\u306e\u8aad\u307f\u53d6\u308a\u30a2\u30af\u30bb\u30b9\u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059\u3002\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u3001\u30d5\u30a1\u30a4\u30eb\u30b7\u30b9\u30c6\u30e0\u306b\u683c\u7d0d\u3055\u308c\u3066\u3044\u308b\u6a5f\u5bc6\u60c5\u5831\u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u53ef\u80fd\u6027\u304c\u3042\u308a\u307e\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u307e\u3059\u304b?" },
        { "APIImpl.file.save.fileExist", "{0} \u306f\u3059\u3067\u306b\u5b58\u5728\u3057\u3066\u3044\u307e\u3059\u3002\n \u7f6e\u63db\u3057\u3066\u3082\u3088\u308d\u3057\u3044\u3067\u3059\u304b?" },
        { "APIImpl.file.save.fileExistTitle", "\u30d5\u30a1\u30a4\u30eb\u304c\u5b58\u5728\u3057\u3066\u3044\u307e\u3059" },
        { "APIImpl.file.save.message", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u30ed\u30fc\u30ab\u30eb\u30d5\u30a1\u30a4\u30eb\u30b7\u30b9\u30c6\u30e0\u4e0a\u306e\u30d5\u30a1\u30a4\u30eb\u3078\u306e\u8aad\u307f\u53d6\u308a/\u66f8\u304d\u8fbc\u307f\u30a2\u30af\u30bb\u30b9\u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u3066\u3082\u3001\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304c\u30a2\u30af\u30bb\u30b9\u3059\u308b\u306e\u306f\u3001\u6b21\u306e\u30d5\u30a1\u30a4\u30eb\u30c0\u30a4\u30a2\u30ed\u30b0\u30dc\u30c3\u30af\u30b9\u3067\u9078\u629e\u3057\u305f\u30d5\u30a1\u30a4\u30eb\u306e\u307f\u3067\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u307e\u3059\u304b?" },
        { "APIImpl.persistence.accessdenied", "URL {0} \u304b\u3089\u6c38\u7d9a\u8a18\u61b6\u9818\u57df\u3078\u306e\u30a2\u30af\u30bb\u30b9\u304c\u62d2\u5426\u3055\u308c\u307e\u3057\u305f" },
        { "APIImpl.persistence.filesizemessage", "\u6700\u5927\u30d5\u30a1\u30a4\u30eb\u9577\u3092\u8d85\u3048\u3066\u3044\u307e\u3059" },
        { "APIImpl.persistence.message", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u8ffd\u52a0\u306e\u30ed\u30fc\u30ab\u30eb\u30c7\u30a3\u30b9\u30af\u8a18\u61b6\u9818\u57df\u3092\u8981\u6c42\u3057\u307e\u3057\u305f\u3002\u73fe\u5728\u306e\u6700\u5927\u5272\u5f53\u9818\u57df\u306f {1} \u30d0\u30a4\u30c8\u3067\u3059\u3002\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u3001\u3053\u308c\u3092 {0} \u30d0\u30a4\u30c8\u307e\u3067\u62e1\u5f35\u3059\u308b\u3088\u3046\u306b\u8981\u6c42\u3057\u3066\u3044\u307e\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u307e\u3059\u304b?" },
        { "APIImpl.print.message", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u30c7\u30d5\u30a9\u30eb\u30c8\u306e\u30d7\u30ea\u30f3\u30bf\u3078\u306e\u30a2\u30af\u30bb\u30b9\u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3059\u308b\u3068\u3001\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u30d7\u30ea\u30f3\u30bf\u306b\u66f8\u304d\u8fbc\u307f\u30e2\u30fc\u30c9\u3067\u30a2\u30af\u30bb\u30b9\u3057\u307e\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u307e\u3059\u304b?" },
	{ "APIImpl.extended.fileOpen.message1", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u3001\u30ed\u30fc\u30ab\u30eb\u30d5\u30a1\u30a4\u30eb\u30b7\u30b9\u30c6\u30e0\u4e0a\u306e\u6b21\u306e\u30d5\u30a1\u30a4\u30eb\u306b\u5bfe\u3059\u308b\u8aad\u307f\u53d6\u308a/\u66f8\u304d\u8fbc\u307f\u30a2\u30af\u30bb\u30b9\u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059:"},
	{ "APIImpl.extended.fileOpen.message2", "\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u3066\u3082\u3001\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304c\u30a2\u30af\u30bb\u30b9\u3059\u308b\u306e\u306f\u3001\u4e0a\u8a18\u306e\u30d5\u30a1\u30a4\u30eb\u3060\u3051\u3067\u3059\u3002\u3053\u306e\u8981\u6c42\u3092\u8a31\u53ef\u3057\u307e\u3059\u304b?"},
        { "APIImpl.securityDialog.no", "\u3044\u3044\u3048" },
        { "APIImpl.securityDialog.remember", "\u4eca\u5f8c\u3053\u306e\u8b66\u544a\u3092\u8868\u793a\u3057\u306a\u3044" },
        { "APIImpl.securityDialog.title", "\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u306e\u8b66\u544a" },
        { "APIImpl.securityDialog.yes", "\u306f\u3044" },
        { "Launch.error.installfailed", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
        { "aboutBox.closeButton", "\u9589\u3058\u308b" },
        { "aboutBox.versionLabel", "\u30d0\u30fc\u30b8\u30e7\u30f3 {0} (\u30d3\u30eb\u30c9 {1})" },
        { "applet.failedToStart", "\u30a2\u30d7\u30ec\u30c3\u30c8 {0} \u306e\u8d77\u52d5\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
        { "applet.initializing", "\u30a2\u30d7\u30ec\u30c3\u30c8\u306e\u521d\u671f\u5316" },
        { "applet.initializingFailed", "\u30a2\u30d7\u30ec\u30c3\u30c8 {0} \u306e\u521d\u671f\u5316\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
        { "applet.running", "\u5b9f\u884c\u4e2d..." },
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "\u5909\u66f4\u3092\u6709\u52b9\u306b\u3059\u308b\u306b\u306f Windows \u3092\u518d\u8d77\u52d5\u3059\u308b\u5fc5\u8981\u304c\u3042\u308a\u307e\u3059\u3002\n\n\u4eca\u3059\u3050 Windows \u3092\u518d\u8d77\u52d5\u3057\u307e\u3059\u304b?" },
        { "extensionInstall.rebootTitle", "Windows \u306e\u518d\u8d77\u52d5" }, 
        { "install.configButton", "\u8a2d\u5b9a(C) ..." },
        { "install.configMnemonic", "VK_C" },
        { "install.errorInstalling", "Java Runtime Environment \u306e\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u4e2d\u306b\u4e88\u671f\u3057\u306a\u3044\u30a8\u30e9\u30fc\u304c\u767a\u751f\u3057\u307e\u3057\u305f\u3002\u3082\u3046\u4e00\u5ea6\u8a66\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
        { "install.errorRestarting", "\u8d77\u52d5\u4e2d\u306b\u4e88\u671f\u3057\u306a\u3044\u30a8\u30e9\u30fc\u304c\u767a\u751f\u3057\u307e\u3057\u305f\u3002\u3082\u3046\u4e00\u5ea6\u8a66\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
        { "install.title", "{0} - \u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u306e\u4f5c\u6210" },

        { "install.windows.both.message", "\u30c7\u30b9\u30af\u30c8\u30c3\u30d7\u3068\u30b9\u30bf\u30fc\u30c8\u30e1\u30cb\u30e5\u30fc\u306b\u4ee5\u4e0b\u306e\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3057\u307e\u3059\u304b?\n{0}" },
	{ "install.gnome.both.message", "\u30c7\u30b9\u30af\u30c8\u30c3\u30d7\u3068\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u30e1\u30cb\u30e5\u30fc\u306b\u4ee5\u4e0b\u306e\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3057\u307e\u3059\u304b?\n{0}" },
	{ "install.desktop.message", "\u30c7\u30b9\u30af\u30c8\u30c3\u30d7\u306b\u4ee5\u4e0b\u306e\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3057\u307e\u3059\u304b?\n{0}" },
	{ "install.windows.menu.message", "\u30b9\u30bf\u30fc\u30c8\u30e1\u30cb\u30e5\u30fc\u306b\u4ee5\u4e0b\u306e\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3057\u307e\u3059\u304b?\n{0}" },
	{ "install.gnome.menu.message", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u30e1\u30cb\u30e5\u30fc\u306b\u4ee5\u4e0b\u306e\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3057\u307e\u3059\u304b?\n{0}" },
        { "install.noButton", "\u3044\u3044\u3048(N)" },
        { "install.noMnemonic", "VK_N" },
        { "install.yesButton", "\u306f\u3044(Y)" },
        { "install.yesMnemonic", "VK_Y" },
        { "launch.cancel", "\u53d6\u6d88\u3057" },
        { "launch.downloadingJRE", "{1} \u304b\u3089 JRE {0} \u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059" },
        { "launch.error.badfield", "\u30d5\u30a3\u30fc\u30eb\u30c9 {0} \u306b\u7121\u52b9\u306a\u5024\u304c\u8a2d\u5b9a\u3055\u308c\u3066\u3044\u307e\u3059: {1}" },
        { "launch.error.badfield-signedjnlp", "\u7f72\u540d\u6e08\u307f\u306e\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u3067\u3001\u30d5\u30a3\u30fc\u30eb\u30c9 {0} \u306b\u7121\u52b9\u306a\u5024\u304c\u8a2d\u5b9a\u3055\u308c\u3066\u3044\u307e\u3059: {1}" },
        { "launch.error.badfield.download.https", "HTTPS \u3092\u4f7f\u7528\u3057\u3066\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3067\u304d\u307e\u305b\u3093" },
        { "launch.error.badfield.https", "HTTPS \u30b5\u30dd\u30fc\u30c8\u3092\u4f7f\u7528\u3059\u308b\u306b\u306f Java 1.4+ \u304c\u5fc5\u8981\u306b\u306a\u308a\u307e\u3059" },
        { "launch.error.badjarfile", "JAR \u30d5\u30a1\u30a4\u30eb\u304c {0} \u3067\u58ca\u308c\u3066\u3044\u307e\u3059" },
        { "launch.error.badjnlversion", "\u30b5\u30dd\u30fc\u30c8\u3055\u308c\u3066\u3044\u306a\u3044 JNLP \u30d0\u30fc\u30b8\u30e7\u30f3\u304c\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u5185\u306b\u5b58\u5728\u3057\u307e\u3059: {0}\u3002\u3053\u306e\u30d0\u30fc\u30b8\u30e7\u30f3\u3067\u30b5\u30dd\u30fc\u30c8\u3055\u308c\u3066\u3044\u308b\u306e\u306f\u3001\u30d0\u30fc\u30b8\u30e7\u30f3 1.0 \u3068 1.5 \u3060\u3051\u3067\u3059\u3002\u3053\u306e\u554f\u984c\u3092\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u30d9\u30f3\u30c0\u30fc\u306b\u5831\u544a\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
        { "launch.error.badmimetyperesponse", "\u30ea\u30bd\u30fc\u30b9 {0} - {1} \u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u969b\u306b\u30b5\u30fc\u30d0\u304b\u3089\u8aa4\u3063\u305f MIME \u30bf\u30a4\u30d7\u304c\u8fd4\u3055\u308c\u307e\u3057\u305f" },
        { "launch.error.badsignedjnlp", "\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u306e\u7f72\u540d\u306e\u691c\u8a3c\u306b\u5931\u6557\u3057\u307e\u3057\u305f\u3002\u7f72\u540d\u6e08\u307f\u306e\u30d0\u30fc\u30b8\u30e7\u30f3\u3068\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3055\u308c\u305f\u30d0\u30fc\u30b8\u30e7\u30f3\u3068\u304c\u4e00\u81f4\u3057\u307e\u305b\u3093\u3002" },
        { "launch.error.badversionresponse", "\u30ea\u30bd\u30fc\u30b9 {0} - {1} \u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u969b\u306b\u30b5\u30fc\u30d0\u304b\u3089\u8fd4\u3055\u308c\u305f\u5fdc\u7b54\u306b\u3001\u8aa4\u3063\u305f\u30d0\u30fc\u30b8\u30e7\u30f3\u304c\u542b\u307e\u308c\u3066\u3044\u307e\u3057\u305f" },
        { "launch.error.canceledloadingresource", "\u30e6\u30fc\u30b6\u306b\u3088\u3063\u3066\u30ea\u30bd\u30fc\u30b9 {0} \u306e\u30ed\u30fc\u30c9\u304c\u53d6\u308a\u6d88\u3055\u308c\u307e\u3057\u305f" },
        { "launch.error.category.arguments", "\u7121\u52b9\u306a\u5f15\u6570\u30a8\u30e9\u30fc" },
        { "launch.error.category.download", "\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u30a8\u30e9\u30fc" },
        { "launch.error.category.launchdesc", "\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u30a8\u30e9\u30fc" },
        { "launch.error.category.memory", "\u30e1\u30e2\u30ea\u4e0d\u8db3\u30a8\u30e9\u30fc" },
        { "launch.error.category.security", "\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u30a8\u30e9\u30fc" },
        { "launch.error.category.config", "\u30b7\u30b9\u30c6\u30e0\u8a2d\u5b9a" },
        { "launch.error.category.unexpected", "\u4e88\u671f\u3057\u306a\u3044\u30a8\u30e9\u30fc" },
        { "launch.error.couldnotloadarg", "\u6307\u5b9a\u3055\u308c\u305f\u30d5\u30a1\u30a4\u30eb\u307e\u305f\u306f URL \u3092\u30ed\u30fc\u30c9\u3067\u304d\u307e\u305b\u3093\u3067\u3057\u305f: {0}" },
        { "launch.error.errorcoderesponse-known", "\u30ea\u30bd\u30fc\u30b9 {0} \u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u969b\u306b\u30b5\u30fc\u30d0\u304b\u3089\u30a8\u30e9\u30fc\u30b3\u30fc\u30c9 {1} ({2}) \u304c\u8fd4\u3055\u308c\u307e\u3057\u305f" },
        { "launch.error.errorcoderesponse-unknown", "\u30ea\u30bd\u30fc\u30b9 {0} \u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u969b\u306b\u30b5\u30fc\u30d0\u304b\u3089\u30a8\u30e9\u30fc\u30b3\u30fc\u30c9 99 (\u672a\u77e5\u306e\u30a8\u30e9\u30fc) \u304c\u8fd4\u3055\u308c\u307e\u3057\u305f" },
        { "launch.error.failedexec", "Java Runtime Environment \u30d0\u30fc\u30b8\u30e7\u30f3 {0} \u3092\u8d77\u52d5\u3067\u304d\u307e\u305b\u3093\u3067\u3057\u305f" },
        { "launch.error.failedloadingresource", "\u30ea\u30bd\u30fc\u30b9 {0} \u3092\u30ed\u30fc\u30c9\u3067\u304d\u307e\u305b\u3093" },
        { "launch.error.invalidjardiff", "\u30ea\u30bd\u30fc\u30b9 {0} \u306e\u8ffd\u52a0\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u3092\u9069\u7528\u3067\u304d\u307e\u305b\u3093" },
        { "launch.error.jarsigning-badsigning", "\u30ea\u30bd\u30fc\u30b9 {0} \u306e\u7f72\u540d\u3092\u691c\u8a3c\u3067\u304d\u307e\u305b\u3093\u3067\u3057\u305f" },
        { "launch.error.jarsigning-missingentry", "\u30ea\u30bd\u30fc\u30b9 {0} \u306b\u7f72\u540d\u6e08\u307f\u30a8\u30f3\u30c8\u30ea\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093" },
        { "launch.error.jarsigning-missingentryname", "\u898b\u3064\u304b\u3089\u306a\u3044\u7f72\u540d\u6e08\u307f\u30a8\u30f3\u30c8\u30ea: {0}" },
        { "launch.error.jarsigning-multicerts", "\u30ea\u30bd\u30fc\u30b9 {0} \u306e\u7f72\u540d\u306b\u8907\u6570\u306e\u8a3c\u660e\u66f8\u304c\u4f7f\u7528\u3055\u308c\u3066\u3044\u307e\u3059" },
        { "launch.error.jarsigning-multisigners", "\u30ea\u30bd\u30fc\u30b9 {0} \u306e\u30a8\u30f3\u30c8\u30ea\u4e0a\u306b\u8907\u6570\u306e\u7f72\u540d\u304c\u5b58\u5728\u3057\u307e\u3059" },
        { "launch.error.jarsigning-unsignedfile", "\u30ea\u30bd\u30fc\u30b9 {0} \u306b\u7f72\u540d\u3055\u308c\u3066\u3044\u306a\u3044\u30a8\u30f3\u30c8\u30ea\u304c\u898b\u3064\u304b\u308a\u307e\u3057\u305f" },
        { "launch.error.missingfield", "\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u306b\u6b21\u306e\u5fc5\u8981\u306a\u30d5\u30a3\u30fc\u30eb\u30c9\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093: {0}" },
        { "launch.error.missingfield-signedjnlp", "\u7f72\u540d\u6e08\u307f\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u306b\u6b21\u306e\u5fc5\u8981\u306a\u30d5\u30a3\u30fc\u30eb\u30c9\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093: {0}" },
        { "launch.error.missingjreversion", "\u3053\u306e\u30b7\u30b9\u30c6\u30e0\u306e\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u306b JRE \u306e\u30d0\u30fc\u30b8\u30e7\u30f3\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093" },
        { "launch.error.missingversionresponse", "\u30ea\u30bd\u30fc\u30b9 {0} \u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u969b\u306b\u30b5\u30fc\u30d0\u304b\u3089\u8fd4\u3055\u308c\u305f\u5fdc\u7b54\u306b\u3001\u30d0\u30fc\u30b8\u30e7\u30f3\u306e\u30d5\u30a3\u30fc\u30eb\u30c9\u304c\u898b\u3064\u304b\u308a\u307e\u305b\u3093" },
        { "launch.error.multiplehostsreferences", "\u30ea\u30bd\u30fc\u30b9\u5185\u3067\u8907\u6570\u306e\u30db\u30b9\u30c8\u304c\u53c2\u7167\u3055\u308c\u3066\u3044\u307e\u3059" },
        { "launch.error.nativelibviolation", "\u30cd\u30a4\u30c6\u30a3\u30d6\u306e\u30e9\u30a4\u30d6\u30e9\u30ea\u3092\u4f7f\u7528\u3059\u308b\u306b\u306f\u30b7\u30b9\u30c6\u30e0\u306b\u7121\u5236\u9650\u306b\u30a2\u30af\u30bb\u30b9\u3067\u304d\u308b\u5fc5\u8981\u304c\u3042\u308a\u307e\u3059" },
        { "launch.error.noJre", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304c\u3001\u73fe\u5728\u30ed\u30fc\u30ab\u30eb\u306b\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u3066\u3044\u306a\u3044\u30d0\u30fc\u30b8\u30e7\u30f3\u306e JRE \u3092\u8981\u6c42\u3057\u307e\u3057\u305f\u3002Java Web Start \u306f\u3001\u8981\u6c42\u3055\u308c\u305f\u30d0\u30fc\u30b8\u30e7\u30f3\u306e\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u304a\u3088\u3073\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3092\u81ea\u52d5\u3067\u884c\u3046\u3053\u3068\u304c\u3067\u304d\u307e\u305b\u3093\u3067\u3057\u305f\u3002\u305d\u306e JRE \u30d0\u30fc\u30b8\u30e7\u30f3\u3092\u624b\u52d5\u3067\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3059\u308b\u5fc5\u8981\u304c\u3042\u308a\u307e\u3059\u3002\n\n" },
        { "launch.error.wont.download.jre", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304c\u3001\u73fe\u5728\u30ed\u30fc\u30ab\u30eb\u306b\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u3066\u3044\u306a\u3044\u30d0\u30fc\u30b8\u30e7\u30f3\u306e JRE (\u30d0\u30fc\u30b8\u30e7\u30f3 {0}) \u3092\u8981\u6c42\u3057\u307e\u3057\u305f\u3002Java Web Start \u306f\u3001\u8981\u6c42\u3055\u308c\u305f\u30d0\u30fc\u30b8\u30e7\u30f3\u306e\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u304a\u3088\u3073\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3092\u81ea\u52d5\u3067\u884c\u3046\u3053\u3068\u3092\u8a31\u53ef\u3055\u308c\u307e\u305b\u3093\u3067\u3057\u305f\u3002\u305d\u306e JRE \u30d0\u30fc\u30b8\u30e7\u30f3\u3092\u624b\u52d5\u3067\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3059\u308b\u5fc5\u8981\u304c\u3042\u308a\u307e\u3059\u3002" },
        { "launch.error.cant.download.jre", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304c\u3001\u73fe\u5728\u30ed\u30fc\u30ab\u30eb\u306b\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u3066\u3044\u306a\u3044\u30d0\u30fc\u30b8\u30e7\u30f3\u306e JRE (\u30d0\u30fc\u30b8\u30e7\u30f3 {0}) \u3092\u8981\u6c42\u3057\u307e\u3057\u305f\u3002Java Web Start \u306f\u3001\u8981\u6c42\u3055\u308c\u305f\u30d0\u30fc\u30b8\u30e7\u30f3\u306e\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u304a\u3088\u3073\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3092\u81ea\u52d5\u3067\u884c\u3046\u3053\u3068\u304c\u3067\u304d\u307e\u305b\u3093\u3002\u305d\u306e JRE \u30d0\u30fc\u30b8\u30e7\u30f3\u3092\u624b\u52d5\u3067\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3059\u308b\u5fc5\u8981\u304c\u3042\u308a\u307e\u3059\u3002" },
        { "launch.error.cant.access.system.cache", "\u3053\u306e\u30e6\u30fc\u30b6\u306b\u306f\u3001\u30b7\u30b9\u30c6\u30e0\u30ad\u30e3\u30c3\u30b7\u30e5\u3078\u306e\u66f8\u304d\u8fbc\u307f\u6a29\u304c\u3042\u308a\u307e\u305b\u3093\u3002" },
        { "launch.error.cant.access.user.cache", "\u3053\u306e\u30e6\u30fc\u30b6\u306b\u306f\u3001\u30ad\u30e3\u30c3\u30b7\u30e5\u3078\u306e\u66f8\u304d\u8fbc\u307f\u6a29\u304c\u3042\u308a\u307e\u305b\u3093\u3002" },
        { "launch.error.noappresources", "\u3053\u306e\u30d7\u30e9\u30c3\u30c8\u30d5\u30a9\u30fc\u30e0\u7528\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u30ea\u30bd\u30fc\u30b9\u304c\u6307\u5b9a\u3055\u308c\u3066\u3044\u307e\u305b\u3093\u3002\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u30d9\u30f3\u30c0\u30fc\u306b\u554f\u3044\u5408\u308f\u305b\u3001\u3053\u306e\u30d7\u30e9\u30c3\u30c8\u30d5\u30a9\u30fc\u30e0\u304c\u30b5\u30dd\u30fc\u30c8\u3055\u308c\u3066\u3044\u308b\u304b\u3069\u3046\u304b\u3092\u78ba\u8a8d\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
        { "launch.error.nomainclass", "\u30e1\u30a4\u30f3\u30af\u30e9\u30b9 {0} \u304c {1} \u306b\u898b\u3064\u304b\u308a\u307e\u305b\u3093\u3067\u3057\u305f" },
        { "launch.error.nomainclassspec", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u30e1\u30a4\u30f3\u30af\u30e9\u30b9\u304c\u6307\u5b9a\u3055\u308c\u3066\u3044\u307e\u305b\u3093" },
        { "launch.error.nomainjar", "\u30e1\u30a4\u30f3 JAR \u30d5\u30a1\u30a4\u30eb\u304c\u6307\u5b9a\u3055\u308c\u3066\u3044\u307e\u305b\u3093\u3002" },
        { "launch.error.nonstaticmainmethod", "main() \u30e1\u30bd\u30c3\u30c9\u306f static \u3067\u3042\u308b\u5fc5\u8981\u304c\u3042\u308a\u307e\u3059" },
        { "launch.error.offlinemissingresource", "\u5fc5\u8981\u306a\u30ea\u30bd\u30fc\u30b9\u306e\u3059\u3079\u3066\u304c\u30ed\u30fc\u30ab\u30eb\u306b\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3055\u308c\u3066\u306f\u3044\u306a\u3044\u305f\u3081\u3001\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3092\u30aa\u30d5\u30e9\u30a4\u30f3\u3067\u5b9f\u884c\u3059\u308b\u3053\u3068\u306f\u3067\u304d\u307e\u305b\u3093" },
        { "launch.error.parse", "\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u3092\u89e3\u6790\u3067\u304d\u307e\u305b\u3093\u3067\u3057\u305f\u3002{0, number} \u884c\u76ee\u306b\u30a8\u30e9\u30fc\u304c\u3042\u308a\u307e\u3059\u3002" },
        { "launch.error.parse-signedjnlp", "\u7f72\u540d\u6e08\u307f\u306e\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u3092\u89e3\u6790\u3067\u304d\u307e\u305b\u3093\u3067\u3057\u305f\u3002{0, number} \u884c\u76ee\u306b\u30a8\u30e9\u30fc\u304c\u3042\u308a\u307e\u3059\u3002" },
        { "launch.error.resourceID", "{0}" },
        { "launch.error.resourceID-version", "({0}, {1})" },
        { "launch.error.singlecertviolation", "JNLP \u30d5\u30a1\u30a4\u30eb\u5185\u306e JAR \u30ea\u30bd\u30fc\u30b9\u304c\u5358\u4e00\u306e\u8a3c\u660e\u66f8\u306b\u3088\u3063\u3066\u7f72\u540d\u3055\u308c\u3066\u3044\u307e\u305b\u3093" },
        { "launch.error.toomanyargs", "\u6307\u5b9a\u3055\u308c\u305f\u5f15\u6570\u304c\u591a\u3059\u304e\u307e\u3059: {0}" },
        { "launch.error.unsignedAccessViolation", "\u7f72\u540d\u3055\u308c\u3066\u3044\u306a\u3044\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304c\u30b7\u30b9\u30c6\u30e0\u3078\u306e\u7121\u5236\u9650\u306e\u30a2\u30af\u30bb\u30b9\u3092\u8981\u6c42\u3057\u3066\u3044\u307e\u3059" },
        { "launch.error.unsignedResource", "\u7f72\u540d\u3055\u308c\u3066\u3044\u306a\u3044\u30ea\u30bd\u30fc\u30b9: {0}" },
        { "launch.estimatedTimeLeft", "\u6b8b\u308a\u6642\u9593 (\u4e88\u60f3): {0,number,00}:{1,number,00}:{2,number,00}" },
        { "launch.extensiondownload", "\u62e1\u5f35\u8a18\u8ff0\u5b50\u3092\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 (\u6b8b\u308a {0})" },
        { "launch.extensiondownload-name", "{0} \u8a18\u8ff0\u5b50\u3092\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059 (\u6b8b\u308a {1})" },
        { "launch.initializing", "\u521d\u671f\u5316\u4e2d..." },
        { "launch.launchApplication", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u8d77\u52d5\u4e2d..." },
        { "launch.launchInstaller", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30e9\u306e\u8d77\u52d5\u4e2d..." },
        { "launch.launchingExtensionInstaller", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30e9\u306e\u5b9f\u884c\u4e2d\u3002\u3057\u3070\u3089\u304f\u304a\u5f85\u3061\u304f\u3060\u3055\u3044..." },
        { "launch.loadingNetProgress", "{0} \u3092\u8aad\u307f\u8fbc\u307f\u307e\u3057\u305f" },
        { "launch.loadingNetProgressPercent", "{1} \u306e\u3046\u3061\u306e {0} \u3092\u8aad\u307f\u8fbc\u307f\u307e\u3057\u305f ({2}%)" },
        { "launch.loadingNetStatus", "{0} \u3092 {1} \u304b\u3089\u30ed\u30fc\u30c9\u3057\u3066\u3044\u307e\u3059" },
        { "launch.loadingResourceFailed", "\u30ea\u30bd\u30fc\u30b9\u306e\u30ed\u30fc\u30c9\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
        { "launch.loadingResourceFailedSts", "{0} \u3092\u8981\u6c42\u3057\u307e\u3057\u305f" },
        { "launch.patchingStatus", "{1} \u304b\u3089\u30d1\u30c3\u30c1\u3092 {0} \u306b\u5f53\u3066\u3066\u3044\u307e\u3059" },
        { "launch.progressScreen", "\u6700\u65b0\u30d0\u30fc\u30b8\u30e7\u30f3\u3092\u78ba\u8a8d\u3057\u3066\u3044\u307e\u3059..." },
        { "launch.stalledDownload", "\u30c7\u30fc\u30bf\u3092\u5f85\u3063\u3066\u3044\u307e\u3059..." },
        { "launch.validatingProgress", "\u30a8\u30f3\u30c8\u30ea\u306e\u30b9\u30ad\u30e3\u30f3 ({0}% \u5b8c\u4e86)" },
        { "launch.validatingStatus", "{0} \u3092 {1} \u304b\u3089\u691c\u8a3c\u3057\u3066\u3044\u307e\u3059" },
        { "launcherrordialog.abort", "\u4e2d\u6b62(A)" },
        { "launcherrordialog.abortMnemonic", "VK_A" },
        { "launcherrordialog.brief.continue", "\u51e6\u7406\u3092\u7d9a\u884c\u3067\u304d\u307e\u305b\u3093" },
        { "launcherrordialog.brief.details", "\u8a73\u7d30" },
        { "launcherrordialog.brief.message", "\u6307\u5b9a\u3055\u308c\u305f\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3092\u8d77\u52d5\u3067\u304d\u307e\u305b\u3093\u3002" },
	{ "launcherrordialog.import.brief.message", "\u6307\u5b9a\u3055\u308c\u305f\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3092\u30a4\u30f3\u30dd\u30fc\u30c8\u3067\u304d\u307e\u305b\u3093\u3002" },
        { "launcherrordialog.brief.messageKnown", "{0} \u3092\u8d77\u52d5\u3067\u304d\u307e\u305b\u3093\u3002" },
	{ "launcherrordialog.import.brief.messageKnown", "{0} \u3092\u30a4\u30f3\u30dd\u30fc\u30c8\u3067\u304d\u307e\u305b\u3093\u3002" },
        { "launcherrordialog.brief.ok", "\u4e86\u89e3" },
        { "launcherrordialog.brief.title", "Java Web Start - {0}" },
        { "launcherrordialog.consoleTab", "\u30b3\u30f3\u30bd\u30fc\u30eb" },
        { "launcherrordialog.errorcategory", "\u30ab\u30c6\u30b4\u30ea: {0}\n\n" },
        { "launcherrordialog.errorintro", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u8d77\u52d5\u307e\u305f\u306f\u5b9f\u884c\u4e2d\u306b\u30a8\u30e9\u30fc\u304c\u767a\u751f\u3057\u307e\u3057\u305f\u3002\n\n" },
	{ "launcherrordialog.import.errorintro", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u30a4\u30f3\u30dd\u30fc\u30c8\u4e2d\u306b\u30a8\u30e9\u30fc\u304c\u767a\u751f\u3057\u307e\u3057\u305f\u3002\n\n" },
        { "launcherrordialog.errormsg", "{0}" },
        { "launcherrordialog.errortitle", "\u30bf\u30a4\u30c8\u30eb: {0}\n" },
        { "launcherrordialog.errorvendor", "\u30d9\u30f3\u30c0\u30fc: {0}\n" },
        { "launcherrordialog.exceptionTab", "\u4f8b\u5916" },
        { "launcherrordialog.generalTab", "\u6982\u8981" },
        { "launcherrordialog.genericerror", "\u4e88\u671f\u3057\u306a\u3044\u4f8b\u5916: {0}" },
        { "launcherrordialog.jnlpMainTab", "\u30e1\u30a4\u30f3\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb" },
        { "launcherrordialog.jnlpTab", "\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb" },
        { "launcherrordialog.title", "Java Web Start - {0}" },
        { "launcherrordialog.wrappedExceptionTab", "\u30e9\u30c3\u30d7\u3055\u308c\u305f\u4f8b\u5916" },

        { "uninstall.failedMessage", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3092\u5b8c\u5168\u306b\u30a2\u30f3\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3059\u308b\u3053\u3068\u304c\u3067\u304d\u307e\u305b\u3093\u3002" },
        { "uninstall.failedMessageTitle", "\u30a2\u30f3\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb" },
        { "install.alreadyInstalled", "{0} \u306e\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u304c\u3059\u3067\u306b\u5b58\u5728\u3057\u307e\u3059\u3002\u305d\u308c\u3067\u3082\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3057\u307e\u3059\u304b?" },
        { "install.alreadyInstalledTitle", "\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u306e\u4f5c\u6210..." },
        { "install.desktopShortcutName", "{0}" },
        { "install.installFailed", "{0} \u306e\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3067\u304d\u307e\u305b\u3093\u3002" },
        { "install.installFailedTitle", "\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u306e\u4f5c\u6210" },
        { "install.startMenuShortcutName", "{0}" },
        { "install.startMenuUninstallShortcutName", "{0} \u306e\u30a2\u30f3\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb" },
        { "install.uninstallFailed", "{0} \u306e\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u524a\u9664\u3067\u304d\u307e\u305b\u3093\u3002\u3082\u3046\u4e00\u5ea6\u8a66\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
        { "install.uninstallFailedTitle", "\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u306e\u524a\u9664" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "\u3053\u306e\u30d7\u30ed\u30b0\u30e9\u30e0\u306f\u5b9f\u884c\u3067\u304d\u307e\u305b\u3093\u3002\u30b7\u30b9\u30c6\u30e0\u306e deployment.config \u30d5\u30a1\u30a4\u30eb\u306b\u3088\u308c\u3070\u3001\u30a8\u30f3\u30bf\u30fc\u30d7\u30e9\u30a4\u30ba\u8a2d\u5b9a\u30d5\u30a1\u30a4\u30eb\u306f\u5fc5\u9808\u3067\u3059\u304c\u3001\u8981\u6c42\u3055\u308c\u305f {0} \u304c\u5229\u7528\u3067\u304d\u307e\u305b\u3093\u3002" },

	// Jnlp Cache Viewer:
	{ "jnlp.viewer.title", "Java \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u30ad\u30e3\u30c3\u30b7\u30e5\u30d3\u30e5\u30fc\u30a2" },
	{ "jnlp.viewer.all", "\u3059\u3079\u3066" },
	{ "jnlp.viewer.type", "{0}" },
	{ "jnlp.viewer.totalSize",  "\u7dcf\u30ea\u30bd\u30fc\u30b9\u30b5\u30a4\u30ba:  {0}" },
 	{ "jnlp.viewer.emptyCache", "{0}\u30ad\u30e3\u30c3\u30b7\u30e5\u306f\u7a7a\u3067\u3059"},
 	{ "jnlp.viewer.noCache", "\u30b7\u30b9\u30c6\u30e0\u30ad\u30e3\u30c3\u30b7\u30e5\u306f\u69cb\u6210\u3055\u308c\u3066\u3044\u307e\u305b\u3093"},

	{ "jnlp.viewer.remove.btn.mnemonic", "VK_R" },
	{ "jnlp.viewer.launch.offline.btn.mnemonic", "VK_L" },
	{ "jnlp.viewer.launch.online.btn.mnemonic", "VK_N" },

	{ "jnlp.viewer.file.menu.mnemonic", "VK_F" },
	{ "jnlp.viewer.edit.menu.mnemonic", "VK_E" },
	{ "jnlp.viewer.app.menu.mnemonic", "VK_A" },
	{ "jnlp.viewer.view.menu.mnemonic", "VK_V" },
	{ "jnlp.viewer.help.menu.mnemonic", "VK_H" },

	{ "jnlp.viewer.cpl.mi.mnemonic", "VK_C" },
	{ "jnlp.viewer.exit.mi.mnemonic", "VK_X" },

	{ "jnlp.viewer.reinstall.mi.mnemonic", "VK_R" },
	{ "jnlp.viewer.preferences.mi.mnemonic", "VK_P" },

	{ "jnlp.viewer.launch.offline.mi.mnemonic", "VK_L" },
	{ "jnlp.viewer.launch.online.mi.mnemonic", "VK_N" },
	{ "jnlp.viewer.install.mi.mnemonic", "VK_I" },
	{ "jnlp.viewer.uninstall.mi.mnemonic", "VK_U" },
	{ "jnlp.viewer.remove.mi.mnemonic", "VK_R" },
	{ "jnlp.viewer.show.mi.mnemonic", "VK_S" },
	{ "jnlp.viewer.browse.mi.mnemonic", "VK_B" },

	{ "jnlp.viewer.view.0.mi.mnemonic", "VK_T" },
	{ "jnlp.viewer.view.1.mi.mnemonic", "VK_A" },
	{ "jnlp.viewer.view.2.mi.mnemonic", "VK_P" },
	{ "jnlp.viewer.view.3.mi.mnemonic", "VK_L" },
	{ "jnlp.viewer.view.4.mi.mnemonic", "VK_I" },

	{ "jnlp.viewer.about.mi.mnemonic", "VK_A" },
	{ "jnlp.viewer.help.java.mi.mnemonic", "VK_J" },
	{ "jnlp.viewer.help.jnlp.mi.mnemonic", "VK_H" },

	{ "jnlp.viewer.remove.btn", "\u524a\u9664(R)" },
	{ "jnlp.viewer.remove.1.btn", "\u9078\u629e\u3055\u308c\u305f{0}\u306e\u524a\u9664(R)" },
	{ "jnlp.viewer.remove.2.btn", "\u9078\u629e\u3055\u308c\u305f\u30a8\u30f3\u30c8\u30ea\u306e\u524a\u9664(R)" },
	{ "jnlp.viewer.uninstall.btn", "\u30a2\u30f3\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb" },
	{ "jnlp.viewer.launch.offline.btn", "\u30aa\u30d5\u30e9\u30a4\u30f3\u3067\u8d77\u52d5(L)" },
	{ "jnlp.viewer.launch.online.btn", "\u30aa\u30f3\u30e9\u30a4\u30f3\u3067\u8d77\u52d5(N)" },

        { "jnlp.viewer.file.menu", "\u30d5\u30a1\u30a4\u30eb(F)" },
        { "jnlp.viewer.edit.menu", "\u7de8\u96c6(E)" },
        { "jnlp.viewer.app.menu", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3(A)" },
        { "jnlp.viewer.view.menu", "\u8868\u793a(V)" },
        { "jnlp.viewer.help.menu", "\u30d8\u30eb\u30d7(H)" },

	{ "jnlp.viewer.cpl.mi", "Java \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb\u306e\u8d77\u52d5(C)" },
	{ "jnlp.viewer.exit.mi", "\u7d42\u4e86(X)" },

	{ "jnlp.viewer.reinstall.mi", "\u518d\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb(R)..." },
	{ "jnlp.viewer.preferences.mi", "\u8a2d\u5b9a(P)..." },

	{ "jnlp.viewer.launch.offline.mi", "\u30aa\u30d5\u30e9\u30a4\u30f3\u3067\u8d77\u52d5(L)" },
	{ "jnlp.viewer.launch.online.mi", "\u30aa\u30f3\u30e9\u30a4\u30f3\u3067\u8d77\u52d5(N)" },
	{ "jnlp.viewer.install.mi", "\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb(I)" },
	{ "jnlp.viewer.uninstall.mi", "\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u30a2\u30f3\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb(U)" },
	{ "jnlp.viewer.remove.0.mi", "\u524a\u9664(R)" },
	{ "jnlp.viewer.remove.mi", "{0}\u306e\u524a\u9664(R)" },
	{ "jnlp.viewer.show.mi", "JNLP \u8a18\u8ff0\u5b50\u3092\u8868\u793a(S)" },
	{ "jnlp.viewer.browse.mi", "\u30db\u30fc\u30e0\u30da\u30fc\u30b8\u3092\u30d6\u30e9\u30a6\u30ba(B)" },

	{ "jnlp.viewer.view.0.mi", "\u3059\u3079\u3066\u306e\u30bf\u30a4\u30d7(T)" },
	{ "jnlp.viewer.view.1.mi", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3(A)" },
	{ "jnlp.viewer.view.2.mi", "\u30a2\u30d7\u30ec\u30c3\u30c8(P)" },
	{ "jnlp.viewer.view.3.mi", "\u30e9\u30a4\u30d6\u30e9\u30ea(L)" },
	{ "jnlp.viewer.view.4.mi", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30e9(I)" },

	{ "jnlp.viewer.view.0", "\u3059\u3079\u3066\u306e\u30bf\u30a4\u30d7" },
	{ "jnlp.viewer.view.1", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3" },
	{ "jnlp.viewer.view.2", "\u30a2\u30d7\u30ec\u30c3\u30c8" },
	{ "jnlp.viewer.view.3", "\u30e9\u30a4\u30d6\u30e9\u30ea" },
	{ "jnlp.viewer.view.4", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30e9" },

	{ "jnlp.viewer.about.mi", "Java \u306b\u3064\u3044\u3066" },
	{ "jnlp.viewer.help.java.mi", "J2SE \u30db\u30fc\u30e0\u30da\u30fc\u30b8" },
	{ "jnlp.viewer.help.jnlp.mi", "JNLP \u30db\u30fc\u30e0\u30da\u30fc\u30b8(H)" },

        { "jnlp.viewer.app.column", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3" },
        { "jnlp.viewer.vendor.column", "\u30d9\u30f3\u30c0\u30fc" },
        { "jnlp.viewer.type.column", "\u30bf\u30a4\u30d7" },
        { "jnlp.viewer.size.column", "\u30b5\u30a4\u30ba" },
        { "jnlp.viewer.date.column", "\u65e5\u4ed8" },
        { "jnlp.viewer.status.column", "\u72b6\u614b" },

        { "jnlp.viewer.app.column.tooltip", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3001\u30a2\u30d7\u30ec\u30c3\u30c8\u3001\u307e\u305f\u306f\u30e9\u30a4\u30d6\u30e9\u30ea\u306e\u30a2\u30a4\u30b3\u30f3\u3068\u30bf\u30a4\u30c8\u30eb" },
        { "jnlp.viewer.vendor.column.tooltip", "\u3053\u306e\u9805\u76ee\u3092\u914d\u5099\u3057\u3066\u3044\u308b\u4f01\u696d" },
        { "jnlp.viewer.type.column.tooltip", "\u3053\u306e\u9805\u76ee\u306e\u7a2e\u985e" },
        { "jnlp.viewer.size.column.tooltip", "\u3053\u306e\u9805\u76ee\u3068\u305d\u306e\u3059\u3079\u3066\u306e\u30ea\u30bd\u30fc\u30b9\u306e\u30b5\u30a4\u30ba" },
        { "jnlp.viewer.date.column.tooltip", "\u3053\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3001\u30a2\u30d7\u30ec\u30c3\u30c8\u3001\u307e\u305f\u306f\u30a4\u30f3\u30b9\u30c8\u30fc\u30e9\u304c\u6700\u5f8c\u306b\u5b9f\u884c\u3055\u308c\u305f\u65e5\u4ed8" },
        { "jnlp.viewer.status.column.tooltip", "\u3053\u306e\u9805\u76ee\u306e\u8d77\u52d5\u53ef\u80fd\u72b6\u614b\u3068\u65b9\u6cd5\u3092\u8868\u3059\u30a2\u30a4\u30b3\u30f3" },


        { "jnlp.viewer.application", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3" },
        { "jnlp.viewer.applet", "\u30a2\u30d7\u30ec\u30c3\u30c8" },
        { "jnlp.viewer.extension", "\u30e9\u30a4\u30d6\u30e9\u30ea" },
        { "jnlp.viewer.installer", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30e9" },

	{ "jnlp.viewer.offline.tooltip",
		 "\u3053\u306e{0}\u306f\u30aa\u30f3\u30e9\u30a4\u30f3\u3068\u30aa\u30d5\u30e9\u30a4\u30f3\u306e\u3069\u3061\u3089\u3067\u3082\u8d77\u52d5\u3067\u304d\u307e\u3059" },
	{ "jnlp.viewer.online.tooltip", "\u3053\u306e{0}\u306f\u30aa\u30f3\u30e9\u30a4\u30f3\u3067\u8d77\u52d5\u3067\u304d\u307e\u3059" },
	{ "jnlp.viewer.norun1.tooltip", 
        	"\u3053\u306e{0}\u306f Web \u30d6\u30e9\u30a6\u30b6\u304b\u3089\u306e\u307f\u8d77\u52d5\u3067\u304d\u307e\u3059" },
        { "jnlp.viewer.norun2.tooltip", "\u30e9\u30a4\u30d6\u30e9\u30ea\u306f\u8d77\u52d5\u3067\u304d\u307e\u305b\u3093" },

	{ "jnlp.viewer.show.title", "JNLP \u8a18\u8ff0\u5b50: {0}" },

	{ "jnlp.viewer.removing", "\u524a\u9664\u4e2d..." },
	{ "jnlp.viewer.launching", "\u8d77\u52d5\u4e2d..." },
	{ "jnlp.viewer.browsing", "\u30d6\u30e9\u30a6\u30b6\u3092\u8d77\u52d5\u4e2d..." },
	{ "jnlp.viewer.sorting", "\u30a8\u30f3\u30c8\u30ea\u3092\u30bd\u30fc\u30c8\u4e2d..." },
	{ "jnlp.viewer.searching", "\u30a8\u30f3\u30c8\u30ea\u3092\u691c\u7d22\u4e2d..." },
	{ "jnlp.viewer.installing", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3057\u3066\u3044\u307e\u3059..." },

        { "jnlp.viewer.reinstall.title", "\u524a\u9664\u3055\u308c\u305f JNLP \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u518d\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb" },
	{ "jnlp.viewer.reinstallBtn", "\u9078\u629e\u3055\u308c\u305f\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3092\u518d\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb(R)" },
	{ "jnlp.viewer.reinstallBtn.mnemonic", "VK_R" },
        { "jnlp.viewer.closeBtn", "\u9589\u3058\u308b(C)" },
        { "jnlp.viewer.closeBtn.mnemonic", "VK_C" },

	{ "jnlp.viewer.reinstall.column.title", "\u30bf\u30a4\u30c8\u30eb:" },
	{ "jnlp.viewer.reinstall.column.location", "\u5834\u6240:" },

	// cache size warning
	{ "jnlp.cache.warning.title", "JNLP \u30ad\u30e3\u30c3\u30b7\u30e5\u30b5\u30a4\u30ba\u306e\u8b66\u544a" },
	{ "jnlp.cache.warning.message", "\u8b66\u544a: \n\n"+
		"\u30ad\u30e3\u30c3\u30b7\u30e5\u306e\u4f7f\u7528\u91cf\u304c JNLP \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3/\u30ea\u30bd\u30fc\u30b9\u306e\n"+
		"\u63a8\u5968\u30c7\u30a3\u30b9\u30af\u5bb9\u91cf\u3092\u8d85\u3048\u307e\u3057\u305f\u3002\n\n"+
		"\u73fe\u5728\u306e\u4f7f\u7528\u91cf: {0}\n"+
		"\u63a8\u5968\u306e\u5236\u9650\u5024: {1}\n\n"+
		"Java \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb\u3092\u4f7f\u3063\u3066\u3001\u7279\u5b9a\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\n"+
		"\u307e\u305f\u306f\u30ea\u30bd\u30fc\u30b9\u3092\u524a\u9664\u3059\u308b\u304b\u3001\u3088\u308a\u9ad8\u3044\u5236\u9650\u5024\u3092\u8a2d\u5b9a\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },

        // Control Panel
        { "control.panel.title", "Java \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb" },
        { "control.panel.general", "\u57fa\u672c" },
        { "control.panel.security", "\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3" },
        { "control.panel.java", "Java" },
        { "control.panel.update", "\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8" },
        { "control.panel.advanced", "\u8a73\u7d30" },

        // Common Strings used in different panels.
        { "common.settings", "\u8a2d\u5b9a" },
        { "common.ok_btn", "\u4e86\u89e3(O)" },
        { "common.ok_btn.mnemonic", "VK_O" },
        { "common.cancel_btn", "\u53d6\u6d88\u3057(C)" },
        { "common.cancel_btn.mnemonic", "VK_C" },
        { "common.apply_btn", "\u9069\u7528(A)" },
        { "common.apply_btn.mnemonic", "VK_A" },
        { "common.add_btn", "\u8ffd\u52a0(A)" },
        { "common.add_btn.mnemonic", "VK_A" },
        { "common.remove_btn", "\u524a\u9664(R)" },
        { "common.remove_btn.mnemonic", "VK_R" },

        // Network Settings Dialog
        { "network.settings.dlg.title", "\u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u8a2d\u5b9a" },
        { "network.settings.dlg.border_title", " \u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a " },
        { "network.settings.dlg.browser_rbtn", "\u30d6\u30e9\u30a6\u30b6\u306e\u8a2d\u5b9a\u3092\u4f7f\u7528(B)" },
        { "browser_rbtn.mnemonic", "VK_B" },
        { "network.settings.dlg.manual_rbtn", "\u30d7\u30ed\u30ad\u30b7\u30b5\u30fc\u30d0\u3092\u4f7f\u7528(P)" },
        { "manual_rbtn.mnemonic", "VK_P" },
        { "network.settings.dlg.address_lbl", "\u30a2\u30c9\u30ec\u30b9:" },
	{ "network.settings.dlg.port_lbl", "\u30dd\u30fc\u30c8:" },
        { "network.settings.dlg.advanced_btn", "\u8a73\u7d30(A)..." },
        { "network.settings.dlg.advanced_btn.mnemonic", "VK_A" },
        { "network.settings.dlg.bypass_text", "\u30ed\u30fc\u30ab\u30eb\u30a2\u30c9\u30ec\u30b9\u306b\u5bfe\u3057\u3066\u30d7\u30ed\u30ad\u30b7\u30b5\u30fc\u30d0\u3092\u901a\u3055\u306a\u3044(Y)" },
        { "network.settings.dlg.bypass.mnemonic", "VK_Y" },
        { "network.settings.dlg.autoconfig_rbtn", "\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u30b9\u30af\u30ea\u30d7\u30c8\u3092\u4f7f\u7528(T)" },
        { "autoconfig_rbtn.mnemonic", "VK_T" },
        { "network.settings.dlg.location_lbl", "\u30b9\u30af\u30ea\u30d7\u30c8\u306e\u5834\u6240: " },
        { "network.settings.dlg.direct_rbtn", "\u76f4\u63a5\u63a5\u7d9a(D)" },
        { "direct_rbtn.mnemonic", "VK_D" },
        { "network.settings.dlg.browser_text", "\u81ea\u52d5\u8a2d\u5b9a\u306b\u3088\u3063\u3066\u624b\u52d5\u8a2d\u5b9a\u304c\u4e0a\u66f8\u304d\u3055\u308c\u308b\u53ef\u80fd\u6027\u304c\u3042\u308a\u307e\u3059\u3002\u624b\u52d5\u8a2d\u5b9a\u304c\u78ba\u5b9f\u306b\u4f7f\u7528\u3055\u308c\u308b\u3088\u3046\u306b\u3059\u308b\u306b\u306f\u81ea\u52d5\u8a2d\u5b9a\u3092\u7121\u52b9\u306b\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
        { "network.settings.dlg.proxy_text", "\u30d6\u30e9\u30a6\u30b6\u306e\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u4e0a\u66f8\u304d\u3057\u307e\u3059\u3002" },
        { "network.settings.dlg.auto_text", "\u6307\u5b9a\u3055\u308c\u305f\u5834\u6240\u306b\u3042\u308b\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u30b9\u30af\u30ea\u30d7\u30c8\u3092\u4f7f\u7528\u3057\u307e\u3059\u3002" },
        { "network.settings.dlg.none_text", "\u76f4\u63a5\u63a5\u7d9a\u3092\u4f7f\u7528\u3057\u307e\u3059\u3002" },

        // Advanced Network Settings Dialog
        { "advanced.network.dlg.title", "\u8a73\u7d30\u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u8a2d\u5b9a" },
        { "advanced.network.dlg.servers", " \u30b5\u30fc\u30d0 " },
        { "advanced.network.dlg.type", "\u30bf\u30a4\u30d7" },
        { "advanced.network.dlg.http", "HTTP:" },
        { "advanced.network.dlg.secure", "Secure:" },
        { "advanced.network.dlg.ftp", "FTP:" },
        { "advanced.network.dlg.socks", "Socks:" },
        { "advanced.network.dlg.proxy_address", "\u30d7\u30ed\u30ad\u30b7\u30a2\u30c9\u30ec\u30b9" },
	{ "advanced.network.dlg.port", "\u30dd\u30fc\u30c8" },
        { "advanced.network.dlg.same_proxy", " \u3059\u3079\u3066\u306e\u30d7\u30ed\u30c8\u30b3\u30eb\u3067\u540c\u3058\u30d7\u30ed\u30ad\u30b7\u30b5\u30fc\u30d0\u3092\u4f7f\u3046(U)" },
        { "advanced.network.dlg.same_proxy.mnemonic", "VK_U" },
        { "advanced.network.dlg.exceptions", " \u4f8b\u5916 " },
        { "advanced.network.dlg.no_proxy", " \u6b21\u306e\u6587\u5b57\u3067\u59cb\u307e\u308b\u30a2\u30c9\u30ec\u30b9\u306b\u5bfe\u3057\u3066\u30d7\u30ed\u30ad\u30b7\u30b5\u30fc\u30d0\u3092\u4f7f\u7528\u3057\u306a\u3044" },
        { "advanced.network.dlg.no_proxy_note", " \u30a8\u30f3\u30c8\u30ea\u3092\u533a\u5207\u308b\u306b\u306f\u30bb\u30df\u30b3\u30ed\u30f3 (;) \u3092\u4f7f\u7528\u3057\u307e\u3059\u3002" },

        // DeleteFilesDialog
        { "delete.files.dlg.title", "\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u306e\u524a\u9664" },
        { "delete.files.dlg.temp_files", "\u6b21\u306e\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u3092\u524a\u9664\u3057\u307e\u3059\u304b?" },
        { "delete.files.dlg.applets", "\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3055\u308c\u305f\u30a2\u30d7\u30ec\u30c3\u30c8" },
        { "delete.files.dlg.applications", "\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u3055\u308c\u305f\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3" },
        { "delete.files.dlg.other", "\u305d\u306e\u4ed6\u306e\u30d5\u30a1\u30a4\u30eb" },

	// General
	{ "general.cache.border.text", " \u30a4\u30f3\u30bf\u30fc\u30cd\u30c3\u30c8\u4e00\u6642\u30d5\u30a1\u30a4\u30eb " },
	{ "general.cache.delete.text", "\u30d5\u30a1\u30a4\u30eb\u306e\u524a\u9664(D)..." },
        { "general.cache.delete.text.mnemonic", "VK_D" },
	{ "general.cache.settings.text", "\u8a2d\u5b9a(S)..." },
        { "general.cache.settings.text.mnemonic", "VK_S" },
	{ "general.cache.desc.text", "Java \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3067\u4f7f\u7528\u3055\u308c\u305f\u30d5\u30a1\u30a4\u30eb\u306f\u3001\u6b21\u56de\u3059\u3070\u3084\u304f\u5b9f\u884c\u3067\u304d\u308b\u3088\u3046\u306b\u7279\u5225\u306a\u30d5\u30a9\u30eb\u30c0\u5185\u306b\u683c\u7d0d\u3055\u308c\u307e\u3059\u3002\u30d5\u30a1\u30a4\u30eb\u306e\u524a\u9664\u3084\u8a2d\u5b9a\u306e\u5909\u66f4\u3092\u884c\u3048\u308b\u306e\u306f\u4e0a\u7d1a\u30e6\u30fc\u30b6\u3060\u3051\u3067\u3059\u3002" },
	{ "general.network.border.text", " \u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u8a2d\u5b9a " },
	{ "general.network.settings.text", "\u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u8a2d\u5b9a(N)..." },
        { "general.network.settings.text.mnemonic", "VK_N" },
	{ "general.network.desc.text", "\u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u8a2d\u5b9a\u306f\u63a5\u7d9a\u6642\u306b\u4f7f\u7528\u3055\u308c\u307e\u3059\u3002\u30c7\u30d5\u30a9\u30eb\u30c8\u3067\u306f\u3001Java \u306f Web \u30d6\u30e9\u30a6\u30b6\u306e\u30cd\u30c3\u30c8\u30ef\u30fc\u30af\u8a2d\u5b9a\u3092\u4f7f\u7528\u3057\u307e\u3059\u3002\u3053\u308c\u3089\u306e\u8a2d\u5b9a\u3092\u5909\u66f4\u3067\u304d\u308b\u306e\u306f\u4e0a\u7d1a\u30e6\u30fc\u30b6\u3060\u3051\u3067\u3059\u3002" },
        { "general.about.border", "\u88fd\u54c1\u60c5\u5831" },
        { "general.about.text", "Java \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb\u306b\u3064\u3044\u3066\u306e\u30d0\u30fc\u30b8\u30e7\u30f3\u60c5\u5831\u3092\u8868\u793a\u3057\u307e\u3059\u3002" },
        { "general.about.btn", "\u30d0\u30fc\u30b8\u30e7\u30f3\u60c5\u5831(B)" },
        { "general.about.btn.mnemonic", "VK_B" },


	// Security
	{ "security.certificates.border.text", " \u8a3c\u660e\u66f8 " },
	{ "security.certificates.button.text", "\u8a3c\u660e\u66f8(E)..." },
        { "security.certificates.button.mnemonic", "VK_E" },
	{ "security.certificates.desc.text", "\u8a3c\u660e\u66f8\u306f\u3001\u81ea\u5206\u81ea\u8eab\u3001\u8a3c\u660e\u66f8\u3001\u8a3c\u660e\u66f8\u767a\u884c\u5c40\u3001\u304a\u3088\u3073\u767a\u884c\u8005\u3092\u7a4d\u6975\u7684\u306b\u8b58\u5225\u3059\u308b\u5834\u5408\u306b\u4f7f\u7528\u3057\u307e\u3059\u3002" },
	{ "security.policies.border.text", " \u30dd\u30ea\u30b7\u30fc " },
	{ "security.policies.advanced.text", "\u8a73\u7d30(D)..." },
        { "security.policies.advanced.mnemonic", "VK_D" },
	{ "security.policies.desc.text", "\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u30dd\u30ea\u30b7\u30fc\u306f\u3001\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304a\u3088\u3073\u30a2\u30d7\u30ec\u30c3\u30c8\u306e\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u969c\u58c1\u3092\u5236\u5fa1\u3059\u308b\u5834\u5408\u306b\u4f7f\u7528\u3057\u307e\u3059\u3002" },

	// Update
	{ "update.notify.border.text", " \u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u306e\u901a\u77e5 " }, // this one is not currently used.  See update panel!!!
	{ "update.updatenow.button.text", "\u4eca\u3059\u3050\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8(U)" },
	{ "update.updatenow.button.mnemonic", "VK_U" },
	{ "update.advanced.button.text", "\u8a73\u7d30(D)..." },
	{ "update.advanced.button.mnemonic", "VK_D" },
	{ "update.desc.text", "Java Update \u30e1\u30ab\u30cb\u30ba\u30e0\u3092\u4f7f\u3048\u3070\u3001\u6700\u65b0\u7248\u306e Java \u30d7\u30e9\u30c3\u30c8\u30d5\u30a9\u30fc\u30e0\u3092\u78ba\u5b9f\u306b\u5165\u624b\u3059\u308b\u3053\u3068\u304c\u3067\u304d\u307e\u3059\u3002\u4ee5\u4e0b\u306e\u30aa\u30d7\u30b7\u30e7\u30f3\u3092\u8a2d\u5b9a\u3059\u308b\u3053\u3068\u3067\u3001\u6700\u65b0\u7248\u306e\u53d6\u5f97\u65b9\u6cd5\u3084\u9069\u7528\u65b9\u6cd5\u306b\u95a2\u3059\u308b\u51e6\u7406\u3092\u5236\u5fa1\u3059\u308b\u3053\u3068\u304c\u3067\u304d\u307e\u3059\u3002" },
        { "update.notify.text", "\u901a\u77e5:" },
        { "update.notify_install.text", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u524d" },
        { "update.notify_download.text", "\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u524d\u3068\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u524d" },
        { "update.autoupdate.text", "\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u3092\u81ea\u52d5\u7684\u306b\u30c1\u30a7\u30c3\u30af" },
        { "update.advanced_title.text", "\u81ea\u52d5\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u306e\u8a73\u7d30\u8a2d\u5b9a" },
        { "update.advanced_title1.text", "\u30b9\u30ad\u30e3\u30f3\u3092\u5b9f\u884c\u3059\u308b\u983b\u5ea6\u304a\u3088\u3073\u65e5\u6642\u3092\u8a2d\u5b9a\u3057\u307e\u3059\u3002" },
        { "update.advanced_title2.text", "\u983b\u5ea6" },
        { "update.advanced_title3.text", "\u65e5\u6642" },
        { "update.advanced_desc1.text", "\u6bce\u65e5 {0} \u306b\u30b9\u30ad\u30e3\u30f3\u3092\u5b9f\u884c" },
        { "update.advanced_desc2.text", "\u6bce\u9031{0}\u306e {1} \u306b\u30b9\u30ad\u30e3\u30f3\u3092\u5b9f\u884c" },
        { "update.advanced_desc3.text", "\u6bce\u6708 {0} \u65e5\u306e {1} \u306b\u30b9\u30ad\u30e3\u30f3\u3092\u5b9f\u884c" },
        { "update.check_daily.text", "\u6bce\u65e5" },
        { "update.check_weekly.text", "\u6bce\u9031" },
        { "update.check_monthly.text", "\u6bce\u6708" },
        { "update.check_date.text", "\u65e5:" },
        { "update.check_day.text", "\u66dc\u65e5:" },
        { "update.check_time.text", "\u6642\u523b:" },
        { "update.lastrun.text", "Java Update \u304c\u6700\u5f8c\u306b\u5b9f\u884c\u3055\u308c\u305f\u306e\u306f {1} \u306e {0} \u3067\u3059\u3002" },
        { "update.desc_autooff.text", "\u6700\u65b0\u7248\u3092\u78ba\u8a8d\u3059\u308b\u305f\u3081\u306b\u306f\u300c\u4eca\u3059\u3050\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u300d\u30dc\u30bf\u30f3\u3092\u30af\u30ea\u30c3\u30af\u3057\u307e\u3059\u3002\u5229\u7528\u53ef\u80fd\u306a\u5834\u5408\u306f\u30b7\u30b9\u30c6\u30e0\u30c8\u30ec\u30a4\u306b\u30a2\u30a4\u30b3\u30f3\u304c\u8868\u793a\u3055\u308c\u308b\u306e\u3067\u3001\u30ab\u30fc\u30bd\u30eb\u3092\u30a2\u30a4\u30b3\u30f3\u4e0a\u306b\u79fb\u52d5\u3057\u3066\u72b6\u6cc1\u3092\u78ba\u8a8d\u3057\u307e\u3059\u3002" },
        { "update.desc_check_daily.text", "Java Update \u306f\u6bce\u65e5 {0} \u306b\u6700\u65b0\u7248\u3092\u78ba\u8a8d\u3057\u307e\u3059\u3002 " },
        { "update.desc_check_weekly.text", "Java Update \u306f\u6bce\u9031{0}\u306e {1} \u306b\u6700\u65b0\u7248\u3092\u78ba\u8a8d\u3057\u307e\u3059\u3002 " },
        { "update.desc_check_monthly.text", "Java Update \u306f\u6bce\u6708 {0} \u65e5\u306e {1} \u306b\u6700\u65b0\u7248\u3092\u78ba\u8a8d\u3057\u307e\u3059\u3002 " },
        { "update.desc_systrayicon.text", "\u5229\u7528\u53ef\u80fd\u306a\u5834\u5408\u306f\u30b7\u30b9\u30c6\u30e0\u30c8\u30ec\u30a4\u306b\u30a2\u30a4\u30b3\u30f3\u304c\u8868\u793a\u3055\u308c\u308b\u306e\u3067\u3001\u30ab\u30fc\u30bd\u30eb\u3092\u30a2\u30a4\u30b3\u30f3\u4e0a\u306b\u79fb\u52d5\u3057\u3066\u72b6\u6cc1\u3092\u78ba\u8a8d\u3057\u307e\u3059\u3002" },
        { "update.desc_notify_install.text", "\u6700\u65b0\u7248\u304c\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u308b\u524d\u306b\u901a\u77e5\u3055\u308c\u307e\u3059\u3002" },
        { "update.desc_notify_download.text", "\u6700\u65b0\u7248\u304c\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u304a\u3088\u3073\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u308b\u524d\u306b\u901a\u77e5\u3055\u308c\u307e\u3059\u3002" },
	{ "update.launchbrowser.error.text", "Java Update \u30c1\u30a7\u30c3\u30ab\u30fc\u3092\u8d77\u52d5\u3067\u304d\u307e\u305b\u3093\u3002\u6700\u65b0\u306e Java Update \u3092\u5165\u624b\u3059\u308b\u306b\u306f http://java.sun.com/getjava/javaupdate \u3092\u53c2\u7167\u3057\u3066\u304f\u3060\u3055\u3044" },
	{ "update.launchbrowser.error.caption", "\u30a8\u30e9\u30fc - \u30a2\u30c3\u30d7\u30c7\u30fc\u30c8" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "\u30d5\u30a1\u30a4\u30eb\u306e\u524a\u9664(D)..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_D" },
        { "cache.settings.dialog.view_jws_btn", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u8868\u793a(V)..." },
        { "cache.settings.dialog.view_jws_btn.mnemonic", "VK_V" },
        { "cache.settings.dialog.view_jpi_btn", "\u30a2\u30d7\u30ec\u30c3\u30c8\u306e\u8868\u793a(A)..." },
        { "cache.settings.dialog.view_jpi_btn.mnemonic", "VK_A" },
        { "cache.settings.dialog.chooser_title", "\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u306e\u5834\u6240" },
        { "cache.settings.dialog.select", "\u9078\u629e" },
        { "cache.settings.dialog.select_tooltip", "\u9078\u629e\u3057\u305f\u5834\u6240\u3092\u4f7f\u7528" },
        { "cache.settings.dialog.select_mnemonic", "VK_S" },
        { "cache.settings.dialog.title", "\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u306e\u8a2d\u5b9a" },
        { "cache.settings.dialog.cache_location", "\u30ad\u30e3\u30c3\u30b7\u30e5\u306e\u5834\u6240:" },
        { "cache.settings.dialog.change_btn", "\u5909\u66f4(H)..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_H" },
        { "cache.settings.dialog.disk_space", "\u4f7f\u7528\u3059\u308b\u30c7\u30a3\u30b9\u30af\u5bb9\u91cf:" },
        { "cache.settings.dialog.unlimited_btn", "\u7121\u5236\u9650" },
        { "cache.settings.dialog.max_btn", "\u4e0a\u9650\u5024" },
        { "cache.settings.dialog.compression", "JAR \u5727\u7e2e:" },
        { "cache.settings.dialog.none", "\u306a\u3057" },
        { "cache.settings.dialog.high", "\u9ad8" },

	// JNLP File/MIME association dialog strings:
	{ "javaws.association.dialog.title", "JNLP \u30d5\u30a1\u30a4\u30eb/MIME \u306e\u95a2\u9023\u4ed8\u3051" },
	{ "javaws.association.dialog.exist.command", "{0} \u3068\u3057\u3066\u3059\u3067\u306b\u5b58\u5728\u3057\u3066\u3044\u307e\u3059\u3002\n"},
	{ "javaws.association.dialog.exist", "\u3059\u3067\u306b\u5b58\u5728\u3057\u3066\u3044\u307e\u3059\u3002" },
	{ "javaws.association.dialog.askReplace", "\n\u4ee3\u308f\u308a\u306b {0} \u3092\u4f7f\u3063\u3066\u51e6\u7406\u3059\u308b\u3088\u3046\u306b\u3057\u307e\u3059\u304b?"},
	{ "javaws.association.dialog.ext", "\u30d5\u30a1\u30a4\u30eb\u306e\u62e1\u5f35\u5b50: {0}" },
        { "javaws.association.dialog.mime", "MIME \u30bf\u30a4\u30d7: {0}" },
        { "javaws.association.dialog.ask", "{0} \u3092\u4f7f\u3063\u3066\u6b21\u3092\u51e6\u7406\u3057\u307e\u3059\u304b?:" },
        { "javaws.association.dialog.existAsk", "\u8b66\u544a! \u95a2\u9023\u4ed8\u3051:"},

        // Advanced panel strings:
        { "deployment.console.startup.mode", "Java \u30b3\u30f3\u30bd\u30fc\u30eb" },
        { "deployment.console.startup.mode.SHOW", "\u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u8868\u793a\u3059\u308b" },
        { "deployment.console.startup.mode.SHOW.tooltip", "<html>" +
                                                          "<font size=-1>" +
                                                          "Java \u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u6700\u5927\u5316\u3057\u3066\u8d77\u52d5" +
                                                          "</font>" +
                                                          "</html>" },
        { "deployment.console.startup.mode.HIDE", "\u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u8868\u793a\u3057\u306a\u3044" },
        { "deployment.console.startup.mode.HIDE.tooltip", "<html>" +
                                                          "<font size=-1>" +
                                                          "Java \u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u6700\u5c0f\u5316\u3057\u3066\u8d77\u52d5" +
                                                          "</font>" +
                                                          "</html>" },
        { "deployment.console.startup.mode.DISABLE", "\u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u958b\u59cb\u3057\u306a\u3044" },
        { "deployment.console.startup.mode.DISABLE.tooltip", "<html>" +
                                                             "<font size=-1>" +
                                                             "Java \u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u8d77\u52d5\u3057\u306a\u3044" +
                                                             "</font>" +
                                                             "</html>" },
        { "deployment.trace", "\u30c8\u30ec\u30fc\u30b9\u3092\u6709\u52b9\u306b\u3059\u308b" },
        { "deployment.trace.tooltip", "<html>" +
                                      "<font size=-1>" +
                                      "\u30c7\u30d0\u30c3\u30b0\u7528\u306b\u30c8\u30ec\u30fc\u30b9\u30d5\u30a1\u30a4\u30eb\u3092\u4f5c\u6210" +
                                      "</font>" +
                                      "</html>" },
        { "deployment.log", "\u30ed\u30ae\u30f3\u30b0\u3092\u6709\u52b9\u306b\u3059\u308b" },
        { "deployment.log.tooltip", "<html>" +
                                    "<font size=-1>" +
                                    "\u30a8\u30e9\u30fc\u3092\u6355\u6349\u3059\u308b\u305f\u3081\u306e\u30ed\u30b0\u30d5\u30a1\u30a4\u30eb\u3092\u4f5c\u6210" +
                                    "</font>" +
                                    "</html>" },
        { "deployment.control.panel.log", "\u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb\u3067\u306e\u30ed\u30ae\u30f3\u30b0" },
        { "deployment.javapi.lifecycle.exception", "\u30a2\u30d7\u30ec\u30c3\u30c8\u306e\u30e9\u30a4\u30d5\u30b5\u30a4\u30af\u30eb\u4f8b\u5916\u3092\u8868\u793a" },
        { "deployment.javapi.lifecycle.exception.tooltip", "<html>" +
                                      	  "<font size=-1>" +
                                      	  "\u30a2\u30d7\u30ec\u30c3\u30c8\u306e\u30ed\u30fc\u30c9\u4e2d\u306b\u30a8\u30e9\u30fc\u304c\u767a\u751f\u3057\u305f\u969b\u306b"+
                                      	  "<br>\u4f8b\u5916\u30c0\u30a4\u30a2\u30ed\u30b0\u3092\u8868\u793a"+
                                      	  "</font>" +
                                          "<html>" },
        { "deployment.browser.vm.iexplorer", "Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
                                                     "<font size=-1>" +
                                                     "Internet Explorer \u3067 APPLET \u30bf\u30b0\u306b Sun Java \u3092\u4f7f\u7528" +
                                                     "</font>" +
                                                     "</html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla \u304a\u3088\u3073 Netscape" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
                                                   "<font size=-1>" +
                                                   "Mozilla \u307e\u305f\u306f Netscape \u30d6\u30e9\u30a6\u30b6\u3067 APPLET \u30bf\u30b0\u306b Sun Java \u3092\u4f7f\u7528" +
                                                   "</font>" +
                                                   "</html>" },
        { "deployment.console.debugging", "\u30c7\u30d0\u30c3\u30b0" },
	{ "deployment.browsers.applet.tag", "<APPLET> \u30bf\u30b0\u306e\u30b5\u30dd\u30fc\u30c8" },
        { "deployment.javaws.shortcut", "\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u306e\u4f5c\u6210" },
        { "deployment.javaws.shortcut.ALWAYS", "\u5e38\u306b\u4f5c\u6210\u3059\u308b" },
        { "deployment.javaws.shortcut.ALWAYS.tooltip", "<html>" +
                                                                 "<font size=-1>" +
                                                                 "\u5e38\u306b\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210" +
                                                                 "</font>" +
                                                       "</html>" },
        { "deployment.javaws.shortcut.NEVER" , "\u4f5c\u6210\u3057\u306a\u3044" },
        { "deployment.javaws.shortcut.NEVER.tooltip", "<html>" +
                                                                "<font size=-1>" +
                                                                "\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3057\u306a\u3044" +
                                                                "</font>" +
                                                      "</html>" },
        { "deployment.javaws.shortcut.ASK_USER", "\u30e6\u30fc\u30b6\u306b\u5c0b\u306d\u308b" },
        { "deployment.javaws.shortcut.ASK_USER.tooltip", "<html>" +
                                                                   "<font size=-1>" +
                                                                   "\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3059\u308b\u304b\u3069\u3046\u304b\u3092" +
                                                                   "<br>\u30e6\u30fc\u30b6\u306b\u78ba\u8a8d" +
                                                                   "</font>" +
                                                         "</html>" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED", "\u4fc3\u3055\u308c\u305f\u5834\u5408\u306f\u5e38\u306b\u4f5c\u6210\u3059\u308b" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED.tooltip", "<html>" +
                                                                           "<font size=-1>" +
                                                                           "JNLP \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304b\u3089\u8981\u6c42\u304c\u3042\u3063\u305f\u5834\u5408\u306f" +
                                                                           "<br>\u5e38\u306b\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210" +
                                                                           "</font>" +
                                                                           "</html>" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED", "\u4fc3\u3055\u308c\u305f\u5834\u5408\u306f\u30e6\u30fc\u30b6\u306b\u5c0b\u306d\u308b" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED.tooltip", "<html>" +
                                                                        "<font size=-1>" +
                                                                        "JNLP \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u304b\u3089\u8981\u6c42\u304c\u3042\u3063\u305f\u5834\u5408\u306f" +
                                                                        "<br>\u30b7\u30e7\u30fc\u30c8\u30ab\u30c3\u30c8\u3092\u4f5c\u6210\u3059\u308b\u304b\u3069\u3046\u304b\u3092\u30e6\u30fc\u30b6\u306b\u78ba\u8a8d" +
                                                                        "</font>" +
                                                                        "</html>" },
	{ "deployment.javaws.associations.NEVER", "\u95a2\u9023\u4ed8\u3051\u3057\u306a\u3044" },
        { "deployment.javaws.associations.NEVER.tooltip", "<html>" +
                                                          "<font size=-1>" +
                                                          "\u30d5\u30a1\u30a4\u30eb\u62e1\u5f35\u5b50/MIME \u306e\u95a2\u9023\u4ed8\u3051\u3092\u4f5c\u6210\u3057\u306a\u3044" +
                                                          "</font>" +
                                                          "</html>" },
        { "deployment.javaws.associations.ASK_USER", "\u30e6\u30fc\u30b6\u306b\u5c0b\u306d\u308b" },
        { "deployment.javaws.associations.ASK_USER.tooltip", "<html>" +
                                                             "<font size=-1>" +
                                                             "\u30d5\u30a1\u30a4\u30eb\u62e1\u5f35\u5b50/MIME \u306e\u95a2\u9023\u4ed8\u3051\u3092\u4f5c\u6210\u3059\u308b\u524d\u306b" +
                                                             "<br>\u30e6\u30fc\u30b6\u306b\u78ba\u8a8d" +
                                                             "</font>" +
                                                             "</html>" },
        { "deployment.javaws.associations.REPLACE_ASK", "\u7f6e\u304d\u63db\u3048\u308b\u969b\u306b\u30e6\u30fc\u30b6\u306b\u5c0b\u306d\u308b" },
        { "deployment.javaws.associations.REPLACE_ASK.tooltip", "<html>" +
                                                                "<font size=-1>" +
                                                                "\u65e2\u5b58\u306e\u30d5\u30a1\u30a4\u30eb\u62e1\u5f35\u5b50/MIME \u306e\u95a2\u9023\u4ed8\u3051\u3092" +
                                                                "<br>\u7f6e\u63db\u3059\u308b\u5834\u5408\u306b\u306e\u307f\u30e6\u30fc\u30b6\u306b\u78ba\u8a8d" +
                                                                "</font>" +
                                                                "</html>" },
        { "deployment.javaws.associations.NEW_ONLY", "\u95a2\u9023\u4ed8\u3051\u304c\u65b0\u898f\u306e\u5834\u5408\u306e\u307f" },
        { "deployment.javaws.associations.NEW_ONLY.tooltip", "<html>" +
                                                             "<font size=-1>" +
                                                             "\u5e38\u306b\u65b0\u3057\u3044\u30d5\u30a1\u30a4\u30eb\u62e1\u5f35\u5b50/MIME \u306e\u95a2\u9023\u4ed8\u3051\u306e\u307f\u4f5c\u6210" +
                                                             "</font>" +
                                                             "</html>" },
        { "deployment.javaws.associations", "JNLP \u30d5\u30a1\u30a4\u30eb/MIME \u306e\u95a2\u9023\u4ed8\u3051" },
        { "deployment.security.settings", "\u30bb\u30ad\u30e5\u30ea\u30c6\u30a3" },
        { "deployment.security.askgrantdialog.show", "\u30e6\u30fc\u30b6\u304c\u7f72\u540d\u6e08\u307f\u30b3\u30f3\u30c6\u30f3\u30c4\u3078\u30a2\u30af\u30bb\u30b9\u6a29\u3092\u4e0e\u3048\u308b\u3053\u3068\u3092\u8a31\u53ef\u3059\u308b" },
        { "deployment.security.askgrantdialog.notinca", "\u30e6\u30fc\u30b6\u304c\u4fe1\u983c\u3067\u304d\u306a\u3044\u8a8d\u8a3c\u5c40\u304b\u3089\u306e\u30b3\u30f3\u30c6\u30f3\u30c4\u3078\u30a2\u30af\u30bb\u30b9\u6a29\u3092\u4e0e\u3048\u308b\u3053\u3068\u3092\u8a31\u53ef\u3059\u308b" },

	{ "deployment.security.browser.keystore.use", "\u30d6\u30e9\u30a6\u30b6\u306e\u30ad\u30fc\u30b9\u30c8\u30a2\u5185\u306e\u8a3c\u660e\u66f8\u304a\u3088\u3073\u30ad\u30fc\u3092\u4f7f\u7528\u3059\u308b" },
	{ "deployment.security.notinca.warning", "\u8a8d\u8a3c\u5c40\u3092\u691c\u8a3c\u3067\u304d\u306a\u3044\u5834\u5408\u306b\u8b66\u544a\u3059\u308b" },
        { "deployment.security.expired.warning", "\u8a3c\u660e\u66f8\u306e\u6709\u52b9\u671f\u9650\u304c\u5207\u308c\u3066\u3044\u308b\u304b\u8a3c\u660e\u66f8\u304c\u7121\u52b9\u306e\u5834\u5408\u306b\u8b66\u544a\u3059\u308b" },
        { "deployment.security.jsse.hostmismatch.warning", "\u30b5\u30a4\u30c8\u8a3c\u660e\u66f8\u3067\u30db\u30b9\u30c8\u540d\u304c\u4e00\u81f4\u3057\u306a\u3044\u5834\u5408\u306b\u8b66\u544a\u3059\u308b" },
        { "deployment.security.sandbox.awtwarningwindow", "\u30b5\u30f3\u30c9\u30dc\u30c3\u30af\u30b9\u8b66\u544a\u30d0\u30ca\u30fc\u3092\u8868\u793a\u3059\u308b" },
        { "deployment.security.sandbox.jnlp.enhanced", "\u30e6\u30fc\u30b6\u304c JNLP \u30bb\u30ad\u30e5\u30ea\u30c6\u30a3\u8981\u6c42\u3092\u53d7\u3051\u5165\u308c\u308b\u3053\u3068\u3092\u8a31\u53ef\u3059\u308b" },
        { "deploy.advanced.browse.title", "\u30c7\u30d5\u30a9\u30eb\u30c8\u30d6\u30e9\u30a6\u30b6\u306e\u8d77\u52d5\u30d5\u30a1\u30a4\u30eb\u3092\u9078\u629e" },
        { "deploy.advanced.browse.select", "\u9078\u629e" },
        { "deploy.advanced.browse.select_tooltip", "\u30d6\u30e9\u30a6\u30b6\u3092\u8d77\u52d5\u3059\u308b\u305f\u3081\u306b\u9078\u629e\u3057\u305f\u30d5\u30a1\u30a4\u30eb\u3092\u4f7f\u3046" },
        { "deploy.advanced.browse.select_mnemonic", "VK_S" },
        { "deploy.advanced.browse.browse_btn", "\u53c2\u7167..." },
        { "deploy.advanced.browse.browse_btn.mnemonic", "VK_B" },
        { "deployment.browser.default", "\u30c7\u30d5\u30a9\u30eb\u30c8\u30d6\u30e9\u30a6\u30b6\u3092\u8d77\u52d5\u3059\u308b\u30b3\u30de\u30f3\u30c9" },
        { "deployment.misc.label", "\u305d\u306e\u4ed6" },
        { "deployment.system.tray.icon", "\u30b7\u30b9\u30c6\u30e0\u30c8\u30ec\u30a4\u306b Java \u30a2\u30a4\u30b3\u30f3\u3092\u914d\u7f6e" },
        { "deployment.system.tray.icon.tooltip", "<html>" +
                                                 "<font size=-1>" +
                                                 "\u30d6\u30e9\u30a6\u30b6\u3067 Java \u3092\u5b9f\u884c\u3057\u3066\u3044\u308b\u3068\u304d\u3001\u30b7\u30b9\u30c6\u30e0\u30c8\u30ec\u30a4\u306b" +
                                                 "<br>Java \u30ab\u30c3\u30d7\u30a2\u30a4\u30b3\u30f3\u3092\u8868\u793a\u3059\u308b\u5834\u5408\u306f\u3053\u306e\u30aa\u30d7\u30b7\u30e7\u30f3\u3092\u9078\u629e" +                                                 "</font>" +
                                                 "</html>" },

        //PluginJresDialog strings:
        { "jpi.jres.dialog.title", "Java \u30e9\u30f3\u30bf\u30a4\u30e0\u8a2d\u5b9a" },
        { "jpi.jres.dialog.border", " Java \u30e9\u30f3\u30bf\u30a4\u30e0\u30d0\u30fc\u30b8\u30e7\u30f3 " },
        { "jpi.jres.dialog.column1", "\u88fd\u54c1\u540d" },
        { "jpi.jres.dialog.column2", "\u30d0\u30fc\u30b8\u30e7\u30f3" },
        { "jpi.jres.dialog.column3", "\u5834\u6240" },
        { "jpi.jres.dialog.column4", "Java \u30e9\u30f3\u30bf\u30a4\u30e0\u30d1\u30e9\u30e1\u30fc\u30bf" },
        { "jpi.jdk.string", "JDK" },
        { "jpi.jre.string", "JRE" },
        { "jpi.jres.dialog.product.tooltip", "\u88fd\u54c1\u540d\u3068\u3057\u3066 JRE \u307e\u305f\u306f JDK \u3092\u9078\u629e" },

        // AboutDialog strings:
        { "about.dialog.title", "Java \u306b\u3064\u3044\u3066" },

        // JavaPanel strings:
        { "java.panel.plugin.border", " Java \u30a2\u30d7\u30ec\u30c3\u30c8\u306e\u30e9\u30f3\u30bf\u30a4\u30e0\u8a2d\u5b9a " }, 
        { "java.panel.plugin.text", "\u3053\u306e\u30e9\u30f3\u30bf\u30a4\u30e0\u8a2d\u5b9a\u306f\u30d6\u30e9\u30a6\u30b6\u5185\u3067\u30a2\u30d7\u30ec\u30c3\u30c8\u304c\u5b9f\u884c\u3055\u308c\u308b\u6642\u306b\u4f7f\u7528\u3055\u308c\u307e\u3059\u3002" },
        { "java.panel.jpi_view_btn", "\u8868\u793a(V)..." },
        { "java.panel.javaws_view_btn", "\u8868\u793a(I)..." },
        { "java.panel.jpi_view_btn.mnemonic", "VK_V" },
        { "java.panel.javaws_view_btn.mnemonic", "VK_I" },
        { "java.panel.javaws.border", " Java \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306e\u30e9\u30f3\u30bf\u30a4\u30e0\u8a2d\u5b9a "},
        { "java.panel.javaws.text", "\u3053\u306e\u30e9\u30f3\u30bf\u30a4\u30e0\u8a2d\u5b9a\u306f Java Network Launching Protocol (JNLP) \u3092\u4f7f\u7528\u3057\u3066 Java \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3084\u30a2\u30d7\u30ec\u30c3\u30c8\u3092\u8d77\u52d5\u3057\u305f\u3068\u304d\u306b\u4f7f\u7528\u3055\u308c\u307e\u3059\u3002" },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "<html><b>\u3088\u308a\u65b0\u3057\u3044 Java \u30e9\u30f3\u30bf\u30a4\u30e0\u304c\u5b58\u5728\u3057\u3066\u3044\u307e\u3059</b></html>Internet Explorer \u306b\u306f\u3059\u3067\u306b\u3001\u3088\u308a\u65b0\u3057\u3044\u30d0\u30fc\u30b8\u30e7\u30f3\u306e Java \u30e9\u30f3\u30bf\u30a4\u30e0\u304c\u542b\u307e\u308c\u3066\u3044\u307e\u3059\u3002\u7f6e\u63db\u3057\u307e\u3059\u304b?\n" },
        { "browser.settings.success.caption", "\u6210\u529f - \u30d6\u30e9\u30a6\u30b6" },
        { "browser.settings.success.text", "<html><b>\u30d6\u30e9\u30a6\u30b6\u306e\u8a2d\u5b9a\u304c\u5909\u66f4\u3055\u308c\u307e\u3057\u305f</b></html>\u5909\u66f4\u306f\u30d6\u30e9\u30a6\u30b6\u306e\u518d\u8d77\u52d5\u5f8c\u306b\u6709\u52b9\u306b\u306a\u308a\u307e\u3059\u3002\n" },
        { "browser.settings.fail.caption", "\u8b66\u544a - \u30d6\u30e9\u30a6\u30b6" },
        { "browser.settings.fail.moz.text", "<html><b>Unable to change Browser Settings</b></html>"
                                        + "Mozilla \u307e\u305f\u306f Netscape \u304c\u30b7\u30b9\u30c6\u30e0\u4e0a\u306b\u6b63\u3057\u304f\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u3066\u3044\u308b\u3053\u3068\u3068\u3001"
       					+ "\u30b7\u30b9\u30c6\u30e0\u8a2d\u5b9a\u306e\u5909\u66f4\u306b\u5fc5\u8981\u306a\u3059\u3079\u3066\u306e\u6a29\u9650\u304c\u4e0e\u3048\u3089\u308c\u3066\u3044\u308b\u3053\u3068\u3092"
                                        + "\u78ba\u8a8d\u3057\u3066\u304f\u3060\u3055\u3044\u3002\n" },
        { "browser.settings.fail.ie.text", "<html><b>\u30d6\u30e9\u30a6\u30b6\u306e\u8a2d\u5b9a\u3092\u5909\u66f4\u3067\u304d\u307e\u305b\u3093</b></html>\u30b7\u30b9\u30c6\u30e0\u8a2d\u5b9a\u306e\u5909\u66f4\u306b\u5fc5\u8981\u306a\u3059\u3079\u3066\u306e\u6a29\u9650\u304c\u4e0e\u3048\u3089\u308c\u3066\u3044\u308b\u3053\u3068\u3092"
					+ "\u78ba\u8a8d\u3057\u3066\u304f\u3060\u3055\u3044\u3002\n" },


        // Tool tip strings.
        { "cpl.ok_btn.tooltip", "<html>" +
				"<font size=-1>" +
				"\u3059\u3079\u3066\u306e\u5909\u66f4\u3092\u4fdd\u5b58\u3057\u3066" +
				"<br>Java \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb\u3092\u9589\u3058\u308b" +
				"</font>" +
                                "</html>" },
        { "cpl.apply_btn.tooltip",  "<html>" +
                                    "<font size=-1>" +
                                    "Java \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb\u306f\u9589\u3058\u305a\u306b" +
                                    "<br>\u3059\u3079\u3066\u306e\u5909\u66f4\u3092\u4fdd\u5b58" +
                                    "</font>" +
                                    "</html>" },
        { "cpl.cancel_btn.tooltip", "<html>" +
                                    "<font size=-1>" +
                                    "\u5909\u66f4\u3092\u4fdd\u5b58\u305b\u305a\u306b" +
                                    "<br>Java \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb\u3092\u9589\u3058\u308b" +
                                    "</font>" +
                                    "</html>" },

        {"network.settings.btn.tooltip", "<html>"+
                                         "<font size=-1>" +
                                         "\u30a4\u30f3\u30bf\u30fc\u30cd\u30c3\u30c8\u63a5\u7d9a\u306e\u8a2d\u5b9a\u3092\u5909\u66f4" +
                                         "</font>" +
                                         "</html>"},

        {"temp.files.settings.btn.tooltip", "<html>"+
                                            "<font size=-1>" +
                                            "\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u306e\u8a2d\u5b9a\u3092\u5909\u66f4" +
                                            "</font>" +
                                            "</html>"},

        {"temp.files.delete.btn.tooltip", "<html>" +  // body bgcolor=\"#FFFFCC\">"+
                                          "<font size=-1>" +
                                          "\u4e00\u6642 Java \u30d5\u30a1\u30a4\u30eb\u3092\u524a\u9664" +
                                          "</font>" +
                                          "</html>"},

        {"delete.files.dlg.applets.tooltip", "<html>" +
                                             "<font size=-1>" +
                                             "Java \u30a2\u30d7\u30ec\u30c3\u30c8\u306b\u3088\u3063\u3066\u4f5c\u6210\u3055\u308c\u305f\u3059\u3079\u3066\u306e\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u3092" +
                                             "<br>\u524a\u9664\u3059\u308b\u5834\u5408\u306f\u3001\u3053\u306e\u30c1\u30a7\u30c3\u30af\u30dc\u30c3\u30af\u30b9\u3092\u9078\u629e\u3057\u307e\u3059\u3002" +
                                             "</font>" +
                                             "</html>" },

        {"delete.files.dlg.applications.tooltip", "<html>" +
                                                  "<font size=-1>" +
                                                  "Java Web Start \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306b\u3088\u3063\u3066\u4f5c\u6210\u3055\u308c\u305f\u3059\u3079\u3066\u306e" +
                                                  "<br>\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u3092\u524a\u9664\u3059\u308b\u5834\u5408\u306f\u3001\u3053\u306e\u30c1\u30a7\u30c3\u30af\u30dc\u30c3\u30af\u30b9\u3092" +
                                                  "<br>\u9078\u629e\u3057\u307e\u3059\u3002" +
                                                  "</font>" +
                                                  "</html>" },

        {"delete.files.dlg.other.tooltip", "<html>" +
                                           "<font size=-1>" +
                                           "Java \u306b\u3088\u3063\u3066\u4f5c\u6210\u3055\u308c\u305f\u305d\u306e\u4ed6\u306e\u3059\u3079\u3066\u306e\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u3092" +
                                           "<br>\u524a\u9664\u3059\u308b\u5834\u5408\u306f\u3001\u3053\u306e\u30c1\u30a7\u30c3\u30af\u30dc\u30c3\u30af\u30b9\u3092\u9078\u629e\u3057\u307e\u3059\u3002" +
                                           "</font>" +
                                           "</html>" },

        {"delete.files.dlg.temp_files.tooltip", "<html>" +
                                                "<font size=-1>" +
                                                "Java \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306f\u3001\u30b3\u30f3\u30d4\u30e5\u30fc\u30bf\u4e0a\u306b\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u3092" +
                                                "<br>\u683c\u7d0d\u3059\u308b\u5834\u5408\u304c\u3042\u308a\u307e\u3059\u304c\u3001\u3053\u308c\u3089\u306e\u30d5\u30a1\u30a4\u30eb\u306f\u5b89\u5168\u306b" +
                                                "<br>\u524a\u9664\u3059\u308b\u3053\u3068\u304c\u53ef\u80fd\u3067\u3059\u3002" +
                                                "<br>" +
                                                "<p>\u305f\u3060\u3057\u3001Java \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306b\u3088\u3063\u3066\u306f\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u3092" +
                                                "<br>\u524a\u9664\u3057\u305f\u76f4\u5f8c\u306e\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u8d77\u52d5\u6642\u306b\u3001\u901a\u5e38\u3088\u308a\u3082" +
                                                "<br>\u6642\u9593\u304c\u304b\u304b\u308b\u53ef\u80fd\u6027\u304c\u3042\u308a\u307e\u3059\u3002" +
                                                "</font>" +
                                                "</html>" },

        {"cache.settings.dialog.view_jws_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java Web Start \u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u306b\u3088\u3063\u3066" +
                                          "<br>\u4f5c\u6210\u3055\u308c\u305f\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u3092\u8868\u793a" +
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.view_jpi_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java \u30a2\u30d7\u30ec\u30c3\u30c8\u306b\u3088\u3063\u3066" +
                                          "<br>\u4f5c\u6210\u3055\u308c\u305f\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u3092\u8868\u793a" +
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u306e\u683c\u7d0d\u5148\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u6307\u5b9a"+
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.unlimited_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u683c\u7d0d\u7528\u306e\u30c7\u30a3\u30b9\u30af\u5bb9\u91cf\u3092" +
                                          "<br>\u5236\u9650\u3057\u306a\u3044" +
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.max_btn.tooltip", "<html>" +
                                                  "<font size=-1>" +
                                                  "\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u683c\u7d0d\u7528\u306e\u30c7\u30a3\u30b9\u30af\u5bb9\u91cf\u306e\u4e0a\u9650\u5024\u3092\u6307\u5b9a" +
                                                  "</font>" +
                                                  "</html>" },

        {"cache.settings.dialog.compression.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java \u30d7\u30ed\u30b0\u30e9\u30e0\u306b\u3088\u3063\u3066\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u5185\u306b" +
                                          "<br>\u683c\u7d0d\u3055\u308c\u308b JAR \u30d5\u30a1\u30a4\u30eb\u306b\u4f7f\u7528\u3059\u308b\u5727\u7e2e\u7387\u3092\u6307\u5b9a\u3057\u307e\u3059\u3002" +
                                          "<br>" +
                                          "<p>[\u306a\u3057] \u3092\u9078\u629e\u3059\u308b\u3068\u3001Java \u30d7\u30ed\u30b0\u30e9\u30e0\u306e\u8d77\u52d5\u6642\u9593\u304c" +
                                          "<br>\u77ed\u304f\u306a\u308a\u307e\u3059\u304c\u3001\u4e00\u6642\u30d5\u30a1\u30a4\u30eb\u306e\u683c\u7d0d\u306b\u5fc5\u8981\u306a\u30c7\u30a3\u30b9\u30af" +
                                          "<br>\u5bb9\u91cf\u306f\u5897\u52a0\u3057\u307e\u3059\u3002\u5727\u7e2e\u7387\u3092\u9ad8\u304f\u3059\u308b\u3068\u3001\u5fc5\u8981\u306a\u30c7\u30a3\u30b9\u30af" +
                                          "<br>\u5bb9\u91cf\u306f\u5c11\u306a\u304f\u306a\u308a\u307e\u3059\u304c\u3001\u8d77\u52d5\u6642\u9593\u304c\u82e5\u5e72\u9577\u304f\u306a\u308a\u307e\u3059\u3002" +
                                          "</font>" +
                                          "</html>" },

        { "common.ok_btn.tooltip",  "<html>" +
                                    "<font size=-1>" +
                                    "\u5909\u66f4\u3092\u4fdd\u5b58\u3057\u3066\u30c0\u30a4\u30a2\u30ed\u30b0\u3092\u9589\u3058\u308b" +
                                    "</font>" +
                                    "</html>" },

        { "common.cancel_btn.tooltip",  "<html>" +
                                        "<font size=-1>" +
                                        "\u5909\u66f4\u3092\u53d6\u308a\u6d88\u3057\u3066\u30c0\u30a4\u30a2\u30ed\u30b0\u3092\u9589\u3058\u308b" +
                                        "</font>" +
                                        "</html>"},

	{ "network.settings.advanced_btn.tooltip",  "<html>" +
                                                    "<font size=-1>" +
                                                    "\u8a73\u7d30\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a\u3092\u8868\u793a\u304a\u3088\u3073\u5909\u66f4"+
                                                    "</font>" +
                                                    "</html>"},

        {"security.certs_btn.tooltip", "<html>" +
                                       "<font size=-1>" +
                                       "\u8a3c\u660e\u66f8\u306e\u30a4\u30f3\u30dd\u30fc\u30c8\u3001\u30a8\u30af\u30b9\u30dd\u30fc\u30c8\u3001\u307e\u305f\u306f\u524a\u9664" +
                                       "</font>" +
                                       "</html>" },

        { "cert.import_btn.tooltip", "<html>" +
                                     "<font size=-1>" +
                                     "\u30ea\u30b9\u30c8\u306b\u542b\u307e\u308c\u3066\u3044\u306a\u3044\u8a3c\u660e\u66f8\u3092\u30a4\u30f3\u30dd\u30fc\u30c8" +
                                     "</font>" +
				     "</html>"},

        { "cert.export_btn.tooltip",    "<html>" +
                                        "<font size=-1>" +
                                        "\u9078\u629e\u3057\u305f\u8a3c\u660e\u66f8\u3092\u30a8\u30af\u30b9\u30dd\u30fc\u30c8" +
                                        "</font>" +
                                        "</html>"},

        { "cert.remove_btn.tooltip",  "<html>" +
                                      "<font size=-1>" +
                                      "\u9078\u629e\u3057\u305f\u8a3c\u660e\u66f8\u3092\u30ea\u30b9\u30c8\u304b\u3089\u524a\u9664"+
                                      "</font>" +
                		      "</html>"},

        { "cert.details_btn.tooltip", "<html>" +
                                      "<font size=-1>" +
                                      "\u9078\u629e\u3057\u305f\u8a3c\u660e\u66f8\u306e\u8a73\u7d30\u60c5\u5831\u3092\u8868\u793a" +
                                      "</font>" +
                                      "</html>"},

        { "java.panel.jpi_view_btn.tooltip",  "<html>" +
                                              "<font size=-1>" +
                                              "\u30a2\u30d7\u30ec\u30c3\u30c8\u7528\u306e Java \u30e9\u30f3\u30bf\u30a4\u30e0\u8a2d\u5b9a\u3092\u5909\u66f4"+
                                              "</font>" +
                                              "</html>" },

        { "java.panel.javaws_view_btn.tooltip",   "<html>" +
                                                  "<font size=-1>" +
                                                  "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u7528\u306e Java \u30e9\u30f3\u30bf\u30a4\u30e0\u8a2d\u5b9a\u3092\u5909\u66f4" +
                                                  "</font>" +
                                                  "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "<font size=-1>" +
                                            "J2SE Runtime Environment \u306e\u3053\u306e\u30d0\u30fc\u30b8\u30e7\u30f3\u306b\u95a2\u3059\u308b\u60c5\u5831\u3092\u8868\u793a" +
                                            "</font>" +
                                            "</html>" },

        { "update.notify_combo.tooltip",  "<html>" +
                                          "<font size=-1>" +
                                          "\u65b0\u3057\u3044 Java \u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u306e\u901a\u77e5\u30bf\u30a4\u30df\u30f3\u30b0\u3092\u9078\u629e" +
                                          "</font>" +
                                          "</html>" },

        { "update.advanced_btn.tooltip",  "<html>" +
                                          "<font size=-1>" +
                                          "\u81ea\u52d5\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u306e\u30b9\u30b1\u30b8\u30e5\u30fc\u30eb\u30dd\u30ea\u30b7\u30fc\u3092\u5909\u66f4" +
                                          "</font>" +
                                          "</html>" },

        { "update.now_btn.tooltip",    "<html>" +
                                      "<font size=-1>" +
                                      "Java Update \u3092\u8d77\u52d5\u3057\u3066\u5229\u7528\u53ef\u80fd\u306a\u6700\u65b0\u306e Java" +
                                      "<br>\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u3092\u78ba\u8a8d" +
                                      "</font>" +
                                      "</html>" },

        { "vm.options.add_btn.tooltip",   "<html>" +
                                          "<font size=-1>" +
                                          "\u65b0\u3057\u3044 JRE \u3092\u30ea\u30b9\u30c8\u306b\u8ffd\u52a0" +
                                          "</font>" +
                                          "</html>" },

        { "vm.options.remove_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                           "\u9078\u629e\u3055\u308c\u305f\u30a8\u30f3\u30c8\u30ea\u3092\u30ea\u30b9\u30c8\u304b\u3089\u524a\u9664" +
                                          "</font>" +
                                           "</html>" },

        { "vm.optios.ok_btn.tooltip",    "<html>" +
                         "<font size=-1>" +
                         "\u88fd\u54c1\u540d\u3001\u30d0\u30fc\u30b8\u30e7\u30f3\u3001\u5834\u6240\u306e\u5404\u60c5\u5831\u3092\u542b\u3080" +
                         "<br>\u3059\u3079\u3066\u306e\u30a8\u30f3\u30c8\u30ea\u3092\u4fdd\u5b58" +
                         "</font>" +
                         "</html>" },

        { "jnlp.jre.find_btn.tooltip",  "<html>" +
                        "<font size=-1>" +
                        "\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u6e08\u307f\u306e Java Runtime Environment \u3092\u691c\u7d22" +
                        "</font>" +
                        "</html>" },

        { "jnlp.jre.add_btn.tooltip",   "<html>" +
                                        "<font size=-1>" +
                                        "\u30ea\u30b9\u30c8\u306b\u65b0\u3057\u3044\u30a8\u30f3\u30c8\u30ea\u3092\u8ffd\u52a0" +
                                        "</font>" +
		                        "</html>" },

        { "jnlp.jre.remove_btn.tooltip",  "<html>" +
                                          "<font size=-1>" +
                                          "\u9078\u629e\u3057\u305f\u30a8\u30f3\u30c8\u30ea\u3092\u30e6\u30fc\u30b6\u306e\u30ea\u30b9\u30c8\u304b\u3089\u524a\u9664" +
                                          "</font>" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "JRE \u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u306e\u8a31\u53ef" },
        { "download.jre.prompt.text1", "\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3 \"{0}\" \u306e\u5b9f\u884c\u306b\u5fc5\u8981\u306a\u30d0\u30fc\u30b8\u30e7\u30f3\u306e "
                                     + "JRE (\u30d0\u30fc\u30b8\u30e7\u30f3 {1}) \u304c\u3001\u30b7\u30b9\u30c6\u30e0\u4e0a\u306b\u73fe\u5728\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb"
                                     + "\u3055\u308c\u3066\u3044\u307e\u305b\u3093\u3002" },
        { "download.jre.prompt.text2", "Java Web Start \u304c\u30e6\u30fc\u30b6\u306b\u4ee3\u308f\u3063\u3066\u3053\u306e JRE \u3092\u81ea\u52d5\u7684\u306b"
                                     + "\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u304a\u3088\u3073\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3059\u308b\u3053\u3068\u3092\u8a31\u53ef\u3057\u307e\u3059\u304b?" },
        { "download.jre.prompt.okButton", "\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9(D)" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "download.jre.prompt.cancelButton", "\u53d6\u6d88\u3057(C)" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "autoupdatecheck.buttonYes", "\u306f\u3044(Y)" },
	{ "autoupdatecheck.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "autoupdatecheck.buttonNo", "\u3044\u3044\u3048(N)" },
	{ "autoupdatecheck.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "autoupdatecheck.buttonAskLater", "\u5f8c\u3067\u78ba\u8a8d\u3059\u308b(A)" },
	{ "autoupdatecheck.buttonAskLater.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "autoupdatecheck.caption", "\u81ea\u52d5\u7684\u306b\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u3092\u78ba\u8a8d" },
	{ "autoupdatecheck.message", "Java Update \u306f\u3001\u6700\u65b0\u7248\u304c\u5229\u7528\u53ef\u80fd\u306b\u306a\u308b\u3068\u304a\u4f7f\u3044\u306e Java \u30bd\u30d5\u30c8\u30a6\u30a7\u30a2\u3092\u81ea\u52d5\u7684\u306b\u66f4\u65b0\u3057\u307e\u3059\u3002\u3053\u306e\u30b5\u30fc\u30d3\u30b9\u3092\u6709\u52b9\u306b\u3057\u307e\u3059\u304b?" },
    };
}

