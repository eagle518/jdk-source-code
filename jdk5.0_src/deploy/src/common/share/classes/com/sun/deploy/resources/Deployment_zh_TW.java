/*
 * @(#)Deployment_zh_TW.java	1.28 04/07/16 
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

public final class Deployment_zh_TW extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
        { "product.javapi.name", "Java Plug-in {0}" },
        { "product.javaws.name", "Java Web Start {0}" },
	{ "console.version", "\u7248\u672c" },
	{ "console.default_vm_version", "\u9810\u8a2d Virtual Machine \u7248\u672c " },
	{ "console.using_jre_version", "\u4f7f\u7528 JRE \u7248\u672c" },
	{ "console.user_home", "\u4f7f\u7528\u8005\u4e3b\u76ee\u9304" },
        { "console.caption", "Java \u4e3b\u63a7\u53f0" },
        { "console.clear", "\u6e05\u9664(C)" },
        { "console.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "console.close", "\u95dc\u9589(E)" },
        { "console.close.acceleratorKey", new Integer(KeyEvent.VK_E) },
        { "console.copy", "\u8907\u88fd(Y)" },
        { "console.copy.acceleratorKey", new Integer(KeyEvent.VK_Y) },
	{ "console.menu.text.top", "----------------------------------------------------\n" },
	{ "console.menu.text.c", "c:   \u6e05\u9664\u4e3b\u63a7\u53f0\u8996\u7a97\n" },
	{ "console.menu.text.f", "f:   \u7d42\u7d50\u5728\u7d50\u675f\u4f47\u5217\u4e0a\u7684\u7269\u4ef6\n" },
	{ "console.menu.text.g", "g:   \u8cc7\u6e90\u56de\u6536\n" },
	{ "console.menu.text.h", "h:   \u986f\u793a\u6b64\u8aaa\u660e\u8a0a\u606f\n" },
	{ "console.menu.text.j", "j:   \u50be\u5370 jcov \u8cc7\u6599\n"},
	{ "console.menu.text.l", "l:   \u50be\u5370\u985e\u5225\u8f09\u5165\u5668\u6e05\u55ae\n" },
	{ "console.menu.text.m", "m:   \u5217\u5370\u8a18\u61b6\u9ad4\u7528\u91cf\n" },
	{ "console.menu.text.o", "o:   \u89f8\u767c\u8a18\u9304\n" },
	{ "console.menu.text.p", "p:   \u91cd\u65b0\u8f09\u5165 Proxy \u914d\u7f6e\n" },
	{ "console.menu.text.q", "q:   \u96b1\u85cf\u4e3b\u63a7\u53f0\n" },
	{ "console.menu.text.r", "r:   \u91cd\u65b0\u8f09\u5165\u7b56\u7565\u914d\u7f6e\n" },
	{ "console.menu.text.s", "s:   \u50be\u5370\u7cfb\u7d71\u548c\u90e8\u7f72\u5c6c\u6027\n" },
	{ "console.menu.text.t", "t:   \u50be\u5370\u57f7\u884c\u7dd2\u6e05\u55ae\n" },
	{ "console.menu.text.v", "v:   \u50be\u5370\u57f7\u884c\u7dd2\u5806\u758a\n" },
	{ "console.menu.text.x", "x:   \u6e05\u9664\u985e\u5225\u8f09\u5165\u5668\u5feb\u53d6\u8a18\u61b6\u9ad4\n" },
	{ "console.menu.text.0", "0-5: \u5c07\u8ffd\u8e64\u5c64\u6b21\u8a2d\u5b9a\u6210 <n>\n" },
	{ "console.menu.text.tail", "----------------------------------------------------\n" },
	{ "console.done", "\u5b8c\u6210\u3002" },
	{ "console.trace.level.0", "\u8ffd\u8e64\u5c64\u6b21\u8a2d\u5b9a\u6210 0\uff1a none ... \u5df2\u5b8c\u6210\u3002" },
	{ "console.trace.level.1", "\u8ffd\u8e64\u5c64\u6b21\u8a2d\u5b9a\u6210 1\uff1a basic ... \u5df2\u5b8c\u6210\u3002" },
	{ "console.trace.level.2", "\u8ffd\u8e64\u5c64\u6b21\u8a2d\u5b9a\u6210 2\uff1abasic\u3001net ... \u5df2\u5b8c\u6210" },
	{ "console.trace.level.3", "\u8ffd\u8e64\u5c64\u6b21\u8a2d\u5b9a\u6210 3\uff1a basic\u3001net\u3001security ... \u5df2\u5b8c\u6210\u3002" },
	{ "console.trace.level.4", "\u8ffd\u8e64\u5c64\u6b21\u8a2d\u5b9a\u6210 4\uff1a basic\u3001net\u3001security\u3001ext ... \u5df2\u5b8c\u6210\u3002" },
	{ "console.trace.level.5", "\u8ffd\u8e64\u5c64\u6b21\u8a2d\u5b9a\u6210 5\uff1a all ... \u5df2\u5b8c\u6210\u3002" },
	{ "console.log", "\u8a18\u9304\u8a2d\u5b9a\u6210\uff1a " },
	{ "console.completed", " ... \u5df2\u5b8c\u6210\u3002" },
	{ "console.dump.thread", "\u50be\u5370\u57f7\u884c\u7dd2\u6e05\u55ae ...\n" },
	{ "console.dump.stack", "\u50be\u5370\u57f7\u884c\u7dd2\u5806\u758a ...\n" },
	{ "console.dump.system.properties", "\u50be\u5370\u7cfb\u7d71\u5c6c\u6027 ...\n" },
	{ "console.dump.deployment.properties", "\u50be\u5370\u90e8\u7f72\u5c6c\u6027 ...\n" },
	{ "console.clear.classloader", "\u6e05\u9664\u985e\u5225\u8f09\u5165\u5668\u5feb\u53d6\u8a18\u61b6\u9ad4 ... \u5df2\u5b8c\u6210\u3002" },
	{ "console.reload.policy", "\u91cd\u65b0\u8f09\u5165\u7b56\u7565\u914d\u7f6e" },
	{ "console.reload.proxy", "\u91cd\u65b0\u8f09\u5165 Proxy \u914d\u7f6e ..." },
	{ "console.gc", "\u8cc7\u6e90\u56de\u6536" },
	{ "console.finalize", "\u7d42\u7d50\u5728\u7d50\u675f\u4f47\u5217\u4e0a\u7684\u7269\u4ef6" },
	{ "console.memory", "\u8a18\u61b6\u9ad4\uff1a {0}K  \u53ef\u7528\uff1a {1}K  ({2}%)" },
	{ "console.jcov.error", "Jcov \u57f7\u884c\u671f\u9593\u932f\u8aa4\uff1a \u8acb\u6aa2\u67e5\u60a8\u6307\u5b9a\u7684 jcov \u9078\u9805\u662f\u5426\u6b63\u78ba\n"},
	{ "console.jcov.info", "Jcov \u8cc7\u6599\u50be\u5370\u6210\u529f\n"},

	{ "https.dialog.caption", "\u8b66\u544a - HTTPS" },
	{ "https.dialog.text", "<html><b>\u4e3b\u96fb\u8166\u540d\u7a31\u4e0d\u76f8\u7b26</b></html>\u5728\u4f3a\u670d\u5668\u5b89\u5168\u8b49\u66f8\u4e2d\u7684\u4e3b\u96fb\u8166\u540d\u7a31\u8207\u4f3a\u670d\u5668\u4e2d\u7684\u540d\u7a31\u4e0d\u7b26\u3002"
			     + "\n\n\u8a72 URL \u7684\u4e3b\u6a5f\u540d\u7a31\u70ba\uff1a {0}"
		  	     + "\u4f86\u81ea\u8a8d\u8b49\u7684\u4e3b\u6a5f\u540d\u7a31\uff1a{1}" 
			     + "\n\n\u60a8\u8981\u7e7c\u7e8c\u55ce?" },
	{ "https.dialog.unknown.host", "\u4e0d\u660e\u7684\u4e3b\u6a5f" },

	{ "security.dialog.caption", "\u8b66\u544a - \u5b89\u5168" },
	{ "security.dialog.text0", "\u60a8\u8981\u4fe1\u4efb\u7531 \"{1}\" \u5206\u9001\u7684\u5df2\u7c3d\u7f72\u7684 {0} \u55ce\uff1f"
				 + "\n\n\u767c\u884c\u4eba\u7684\u78ba\u5be6\u6027\u7531\uff1a\"{2}\" \u6240\u9a57\u8b49" },
	{ "security.dialog.text0a", "\u60a8\u8981\u4fe1\u4efb\u7531 \"{1}\" \u5206\u9001\u7684\u5df2\u7c3d\u7f72\u7684 {0} \u55ce\uff1f"
 				 + "\n\n\u7121\u6cd5\u9a57\u8b49\u767c\u884c\u4eba\u7684\u78ba\u5be6\u6027\u3002" },
	{ "security.dialog.timestamp.text1", "\u5df2\u65bc {1} \u7c3d\u7f72{0}\u3002" },
	{ "security.dialog_https.text0", "\u60a8\u8981\u63a5\u53d7\u4f86\u81ea \"{0}\" \u7ad9\u53f0\u7684\u8b49\u66f8\u4ee5\u4ea4\u63db\u52a0\u5bc6\u7684\u8cc7\u8a0a\u55ce\uff1f"
				 + "\n\n\u767c\u884c\u4eba\u7684\u78ba\u5be6\u6027\u7531\uff1a\"{1}\" \u6240\u9a57\u8b49" },
	{ "security.dialog_https.text0a", "\u60a8\u8981\u63a5\u53d7\u4f86\u81ea \"{0}\" \u7ad9\u53f0\u7684\u8b49\u66f8\u4ee5\u4ea4\u63db\u52a0\u5bc6\u7684\u8cc7\u8a0a\u55ce\uff1f"
 				 + "\n\n\u7121\u6cd5\u9a57\u8b49\u767c\u884c\u4eba\u7684\u78ba\u5be6\u6027\u3002" },
	{ "security.dialog.text1", "\n\u5c0f\u5fc3\uff1a \"{0}\" \u8072\u7a31\u9019\u500b\u5167\u5bb9\u662f\u5b89\u5168\u7684\u3002 \u5982\u679c\u60a8\u4fe1\u4efb \"{1}\" \u7684\u8072\u660e\uff0c\u60a8\u61c9\u8a72\u53ea\u63a5\u53d7\u9019\u500b\u5167\u5bb9\u3002" },
	{ "security.dialog.unknown.issuer", "\u4e0d\u660e\u7684\u767c\u884c\u4eba" },
	{ "security.dialog.unknown.subject", "\u4e0d\u660e\u7684\u4e3b\u984c" },
	{ "security.dialog.certShowName", "{0} ({1})" },
	{ "security.dialog.rootCANotValid", "\u5b89\u5168\u8b49\u66f8\u7531\u672a\u6388\u4fe1\u7684\u516c\u53f8\u6240\u767c\u51fa\u3002" },
	{ "security.dialog.rootCAValid", "\u5b89\u5168\u8b49\u66f8\u7531\u6388\u4fe1\u7684\u516c\u53f8\u6240\u767c\u51fa\u3002" },
	{ "security.dialog.timeNotValid", "\u5b89\u5168\u8b49\u66f8\u5df2\u904e\u671f\u6216\u5c1a\u672a\u751f\u6548\u3002" },
	{ "security.dialog.timeValid", "\u5b89\u5168\u8b49\u66f8\u5c1a\u672a\u904e\u671f\u4e14\u4ecd\u7136\u6709\u6548\u3002" },
	{ "security.dialog.timeValidTS", "\u7c3d\u7f72{0}\u6642\u5b89\u5168\u8b49\u66f8\u4ecd\u7136\u6709\u6548\u3002" },
	{ "security.dialog.buttonAlways", "\u7e3d\u662f(A)" },
        { "security.dialog.buttonAlways.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "security.dialog.buttonYes", "\u662f(Y)" },
	{ "security.dialog.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "security.dialog.buttonNo", "\u5426(N)" },
	{ "security.dialog.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "security.dialog.buttonViewCert", "\u66f4\u591a\u8a73\u7d30\u8cc7\u8a0a(M)" },
        { "security.dialog.buttonViewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

        { "security.badcert.caption", "\u8b66\u544a - \u5b89\u5168" },
        { "security.badcert.https.text", "\u7121\u6cd5\u9a57\u8b49 SSL \u8b49\u66f8\u3002\n\u8a72 {0} \u5c07\u7121\u6cd5\u57f7\u884c\u3002" },
        { "security.badcert.config.text", "\u60a8\u7684\u5b89\u5168\u914d\u7f6e\u4e0d\u5141\u8a31\u60a8\u9a57\u8b49\u8a72\u8b49\u66f8\u3002  \u8a72 {0} \u5c07\u7121\u6cd5\u57f7\u884c\u3002" },
        { "security.badcert.text", "\u7121\u6cd5\u9a57\u8b49\u8b49\u66f8\u3002  \u8a72 {0} \u5c07\u7121\u6cd5\u57f7\u884c\u3002" },
        { "security.badcert.viewException", "\u986f\u793a\u7570\u5e38\u72c0\u6cc1(S)" },
        { "security.badcert.viewException.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "security.badcert.viewCert", "\u66f4\u591a\u8a73\u7d30\u8cc7\u8a0a(M)" },
        { "security.badcert.viewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

	{ "cert.dialog.caption", "\u8a73\u7d30\u8cc7\u8a0a - \u8b49\u66f8" },
	{ "cert.dialog.certpath", "\u8b49\u66f8\u8def\u5f91" },
	{ "cert.dialog.field.Version", "\u7248\u672c" },
	{ "cert.dialog.field.SerialNumber", "\u5e8f\u865f" },
	{ "cert.dialog.field.SignatureAlg", "\u7c3d\u540d\u6f14\u7b97\u6cd5" },
	{ "cert.dialog.field.Issuer", "\u767c\u884c\u4eba" },
	{ "cert.dialog.field.EffectiveDate", "\u6709\u6548\u65e5\u671f" },
	{ "cert.dialog.field.ExpirationDate", "\u622a\u6b62\u65e5\u671f" },
	{ "cert.dialog.field.Validity", "\u6709\u6548\u6027" },
	{ "cert.dialog.field.Subject", "\u4e3b\u984c" },
	{ "cert.dialog.field.Signature", "\u7c3d\u540d" },
	{ "cert.dialog.field", "\u6b04\u4f4d" },
	{ "cert.dialog.value", "\u6578\u503c" },
        { "cert.dialog.close", "\u95dc\u9589(C)" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_C) },

	{ "clientauth.password.dialog.buttonOK", "\u78ba\u5b9a(O)" },
	{ "clientauth.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.password.dialog.buttonCancel", "\u53d6\u6d88(C)" },
	{ "clientauth.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.password.dialog.caption", "\u9700\u8981\u5bc6\u78bc - \u7528\u6236\u7aef\u8a8d\u8b49\u5bc6\u9470\u5132\u5b58\u5eab" },
	{ "clientauth.password.dialog.text", "\u8acb\u8f38\u5165\u5bc6\u78bc\u4ee5\u5b58\u53d6\u7528\u6236\u7aef\u8a8d\u8b49\u5bc6\u9470\u5132\u5b58\u5eab\uff1a\n" },
	{ "clientauth.password.dialog.error.caption", "\u932f\u8aa4 - \u7528\u6236\u7aef\u8a8d\u8b49\u5bc6\u9470\u5132\u5b58\u5eab" },
	{ "clientauth.password.dialog.error.text", "<html><b>\u5bc6\u9470\u5132\u5b58\u5eab\u5b58\u53d6\u932f\u8aa4</b></html>\u5bc6\u9470\u5132\u5b58\u5eab\u88ab\u7be1\u6539\u6216\u5bc6\u78bc\u4e0d\u6b63\u78ba\u3002" },
		
	{ "clientauth.certlist.dialog.buttonOK", "\u78ba\u5b9a(O)" },
	{ "clientauth.certlist.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.certlist.dialog.buttonCancel", "\u53d6\u6d88(C)" },
	{ "clientauth.certlist.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.certlist.dialog.buttonDetails", "\u8a73\u7d30\u8cc7\u8a0a(D)" },
	{ "clientauth.certlist.dialog.buttonDetails.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "clientauth.certlist.dialog.caption", "\u7528\u6236\u7aef\u8a8d\u8b49" },
	{ "clientauth.certlist.dialog.text", "\u60a8\u8981\u9023\u7dda\u7684\u7ad9\u53f0\u8981\u6c42\u8eab\u4efd\u8b58\u5225\u3002\n\u8acb\u9078\u53d6\u9023\u7dda\u6642\u8981\u4f7f\u7528\u7684\u8b49\u66f8\u3002\n" },

	{ "dialogfactory.confirmDialogTitle", "\u9700\u8981\u78ba\u8a8d - Java" },
	{ "dialogfactory.inputDialogTitle", "\u9700\u8981\u8cc7\u8a0a - Java" },
	{ "dialogfactory.messageDialogTitle", "\u8a0a\u606f - Java" },
	{ "dialogfactory.exceptionDialogTitle", "\u932f\u8aa4 - Java" },
	{ "dialogfactory.optionDialogTitle", "\u9078\u9805 - Java" },
	{ "dialogfactory.aboutDialogTitle", "\u95dc\u65bc - Java" },
	{ "dialogfactory.confirm.yes", "\u662f(Y)" },
        { "dialogfactory.confirm.yes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "dialogfactory.confirm.no", "\u5426(N)" },
        { "dialogfactory.confirm.no.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "dialogfactory.moreInfo", "\u66f4\u591a\u8a73\u7d30\u8cc7\u8a0a(M)" },
        { "dialogfactory.moreInfo.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "dialogfactory.lessInfo", "\u8f03\u5c11\u8a73\u7d30\u8cc7\u8a0a(L)" },
        { "dialogfactory.lessInfo.acceleratorKey", new Integer(KeyEvent.VK_L)},
	{ "dialogfactory.java.home.link", "http://www.java.com" },
	{ "dialogfactory.general_error", "<html><b>\u4e00\u822c\u7570\u5e38\u72c0\u6cc1</b></html>" },
	{ "dialogfactory.net_error", "<html><b>\u7db2\u8def\u9023\u7dda\u7570\u5e38\u72c0\u6cc1</b></html>" },
	{ "dialogfactory.security_error", "<html><b>\u5b89\u5168\u7570\u5e38\u72c0\u6cc1</b></html>" },
	{ "dialogfactory.ext_error", "<html><b>\u9078\u7528\u5957\u4ef6\u7570\u5e38\u72c0\u6cc1</b></html>" },
	{ "dialogfactory.user.selected", "\u4f7f\u7528\u8005\u9078\u53d6\uff1a {0}" },
	{ "dialogfactory.user.typed", "\u4f7f\u7528\u8005\u9375\u5165\uff1a {0}" },

	{ "deploycertstore.cert.loading", "\u6b63\u5728\u5f9e {0} \u8f09\u5165\u90e8\u7f72\u8b49\u66f8" },
	{ "deploycertstore.cert.loaded", "\u5df2\u5f9e {0} \u8f09\u5165\u90e8\u7f72\u8b49\u66f8" },
	{ "deploycertstore.cert.saving", "\u6b63\u5728\u5c07\u90e8\u7f72\u8b49\u66f8\u5132\u5b58\u65bc {0}" },
	{ "deploycertstore.cert.saved", "\u5df2\u5132\u5b58\u90e8\u7f72\u8b49\u66f8\u65bc {0}" },
	{ "deploycertstore.cert.adding", "\u6b63\u5728\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e\u8b49\u66f8", },
	{ "deploycertstore.cert.added", "\u5728\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u5df2\u65b0\u589e\u5225\u540d\u70ba {0} \u7684\u8b49\u66f8" },
	{ "deploycertstore.cert.removing", "\u6b63\u5728\u79fb\u9664\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8" },
	{ "deploycertstore.cert.removed", "\u5df2\u79fb\u9664\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u5225\u540d\u70ba {0} \u7684\u8b49\u66f8" },
	{ "deploycertstore.cert.instore", "\u6aa2\u67e5\u8b49\u66f8\u662f\u5426\u5728\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d" },
	{ "deploycertstore.cert.canverify", "\u6aa2\u67e5\u8b49\u66f8\u662f\u5426\u53ef\u4f7f\u7528\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u4f86\u9a57\u8b49" },
	{ "deploycertstore.cert.iterator", "\u53d6\u5f97\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u8fed\u4ee3\u5668" },
	{ "deploycertstore.cert.getkeystore", "\u53d6\u5f97\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u7684\u5bc6\u9470\u5132\u5b58\u5eab\u7269\u4ef6" },

	{ "httpscertstore.cert.loading", "\u6b63\u81ea {0} \u8f09\u5165\u90e8\u7f72 SSL \u8b49\u66f8" },
	{ "httpscertstore.cert.loaded", "\u5df2\u81ea {0} \u8f09\u5165\u90e8\u7f72 SSL \u8b49\u66f8" },
	{ "httpscertstore.cert.saving", "\u6b63\u5728 {0} \u5132\u5b58\u90e8\u7f72 SSL \u8b49\u66f8" },
	{ "httpscertstore.cert.saved", "\u5df2\u5728 {0} \u5132\u5b58\u90e8\u7f72 SSL \u8b49\u66f8" },
	{ "httpscertstore.cert.adding", "\u6b63\u5728\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e SSL \u8b49\u66f8", },
	{ "httpscertstore.cert.added", "\u5df2\u5728\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e\u5225\u540d\u70ba {0} \u7684 SSL \u8b49\u66f8" },
	{ "httpscertstore.cert.removing", "\u6b63\u5728\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u79fb\u9664 SSL \u8b49\u66f8" },
	{ "httpscertstore.cert.removed", "\u5df2\u5728\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u79fb\u9664\u5225\u540d\u70ba {0} \u7684 SSL \u8b49\u66f8" },
	{ "httpscertstore.cert.instore", "\u6b63\u5728\u6aa2\u67e5 SSL \u8b49\u66f8\u662f\u5426\u5728\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d" },
	{ "httpscertstore.cert.canverify", "\u6aa2\u67e5 SSL \u8b49\u66f8\u662f\u5426\u53ef\u4ee5\u4f7f\u7528\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u9a57\u8b49" },
	{ "httpscertstore.cert.iterator", "\u53d6\u5f97\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684 SSL \u8b49\u66f8\u8fed\u4ee3\u5668" },
	{ "httpscertstore.cert.getkeystore", "\u53d6\u5f97\u90e8\u7f72\u6c38\u4e45\u8b49\u66f8\u5132\u5b58\u5eab\u7684\u5bc6\u9470\u5132\u5b58\u5eab\u7269\u4ef6" },

	{ "rootcertstore.cert.loading", "\u6b63\u5f9e {0} \u8f09\u5165 Root CA \u8b49\u66f8" },
	{ "rootcertstore.cert.loaded", "\u5df2\u5f9e {0} \u8f09\u5165 Root CA \u8b49\u66f8" },
	{ "rootcertstore.cert.noload", "\u7121\u6cd5\u627e\u5230 Root CA \u8b49\u66f8\u6a94\u6848\uff1a {0}" },
	{ "rootcertstore.cert.saving", "\u6b63\u5728\u5c07 Root CA \u8b49\u66f8\u5132\u5b58\u5230 {0}" },
	{ "rootcertstore.cert.saved", "\u5df2\u5132\u5b58 Root CA \u8b49\u66f8\u65bc {0}" },
	{ "rootcertstore.cert.adding", "\u6b63\u5728 Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e\u8b49\u66f8", },
	{ "rootcertstore.cert.added", "\u5df2\u5728 Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e\u5225\u540d\u70ba {0} \u7684\u8b49\u66f8" },
	{ "rootcertstore.cert.removing", "\u6b63\u5728\u79fb\u9664 Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8" },
	{ "rootcertstore.cert.removed", "\u5df2\u79fb\u9664 Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u5225\u540d\u70ba {0} \u7684\u8b49\u66f8" },
	{ "rootcertstore.cert.instore", "\u6b63\u5728\u6aa2\u67e5\u8b49\u66f8\u662f\u5426\u5728 Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d" },
	{ "rootcertstore.cert.canverify", "\u6aa2\u67e5\u8b49\u66f8\u662f\u5426\u53ef\u4f7f\u7528 Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u4f86\u9a57\u8b49" },
	{ "rootcertstore.cert.iterator", "\u53d6\u5f97 Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u8fed\u4ee3\u5668" },
	{ "rootcertstore.cert.getkeystore", "\u53d6\u5f97 Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u7684\u5bc6\u9470\u5132\u5b58\u5eab\u7269\u4ef6" },
	{ "rootcertstore.cert.verify.ok", "\u8b49\u66f8\u5df2\u9806\u5229\u901a\u904e Root CA \u8b49\u66f8\u7684\u9a57\u8b49" },
	{ "rootcertstore.cert.verify.fail", "\u8b49\u66f8\u672a\u901a\u904e Root CA \u8b49\u66f8\u7684\u9a57\u8b49" },
	{ "rootcertstore.cert.tobeverified", "\u5c07\u88ab\u9a57\u8b49\u7684\u8b49\u66f8\uff1a\n{0}" },
	{ "rootcertstore.cert.tobecompared", "\u6b63\u5728\u5c07\u8b49\u66f8\u8207\u4ee5\u4e0b Root CA \u8b49\u66f8\u6bd4\u8f03\uff1a\n{0}" },

	{ "roothttpscertstore.cert.loading", "\u6b63\u81ea {0} \u8f09\u5165 SSL Root CA \u8b49\u66f8" },
	{ "roothttpscertstore.cert.loaded", "\u5df2\u81ea {0} \u8f09\u5165 SSL Root CA \u8b49\u66f8" },
	{ "roothttpscertstore.cert.noload", "\u7121\u6cd5\u627e\u5230 SSL Root CA \u8b49\u66f8\u6a94\u6848\uff1a{0}" },
	{ "roothttpscertstore.cert.saving", "\u6b63\u5728\u5132\u5b58 SSL Root CA \u8b49\u66f8\u65bc {0}" },
	{ "roothttpscertstore.cert.saved", "\u5df2\u5132\u5b58 SSL Root CA \u8b49\u66f8\u65bc {0}" },
	{ "roothttpscertstore.cert.adding", "\u6b63\u5728 SSL Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e\u8b49\u66f8", },
	{ "roothttpscertstore.cert.added", "\u5df2\u5728 SSL Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e\u5225\u540d\u70ba {0} \u7684\u8b49\u66f8 " },
	{ "roothttpscertstore.cert.removing", "\u6b63\u5728\u5f9e SSL Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u79fb\u9664\u8b49\u66f8" },
	{ "roothttpscertstore.cert.removed", "\u5df2\u5f9e SSL Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u79fb\u9664\u5225\u540d\u70ba {0} \u7684\u8b49\u66f8 " },
	{ "roothttpscertstore.cert.instore", "\u6b63\u5728\u6aa2\u67e5\u8b49\u66f8\u662f\u5426\u5728 SSL Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d" },
	{ "roothttpscertstore.cert.canverify", "\u6aa2\u67e5\u662f\u5426\u53ef\u4f7f\u7528 SSL Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u7684\u8b49\u66f8\u4f86\u9a57\u8b49\u8b49\u66f8" },
	{ "roothttpscertstore.cert.iterator", "\u53d6\u5f97 SSL Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u8fed\u4ee3\u5668" },
	{ "roothttpscertstore.cert.getkeystore", "\u53d6\u5f97 SSL Root CA \u8b49\u66f8\u5132\u5b58\u5eab\u7684\u5bc6\u9470\u5132\u5b58\u5eab\u7269\u4ef6" },
	{ "roothttpscertstore.cert.verify.ok", "\u8b49\u66f8\u5df2\u7d93\u904e SSL Root CA \u8b49\u66f8\u9a57\u8b49\u6210\u529f" },
	{ "roothttpscertstore.cert.verify.fail", "\u8b49\u66f8\u7d93\u904e SSL Root CA \u8b49\u66f8\u9a57\u8b49\u5931\u6557" },
	{ "roothttpscertstore.cert.tobeverified", "\u5c07\u88ab\u9a57\u8b49\u7684\u8b49\u66f8\uff1a\n{0}" },
	{ "roothttpscertstore.cert.tobecompared", "\u6b63\u5728\u5c07\u8b49\u66f8\u8207\u4ee5\u4e0b SSL Root CA \u8b49\u66f8\u6bd4\u8f03\uff1a\n{0}" },

	{ "sessioncertstore.cert.loading", "\u6b63\u5728\u5f9e\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u8f09\u5165\u8b49\u66f8" },
	{ "sessioncertstore.cert.loaded", "\u5df2\u5f9e\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u8f09\u5165\u8b49\u66f8" },
	{ "sessioncertstore.cert.saving", "\u6b63\u5728\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u5132\u5b58\u8b49\u66f8" },
	{ "sessioncertstore.cert.saved", "\u5df2\u5728\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u5132\u5b58\u8b49\u66f8" },
	{ "sessioncertstore.cert.adding", "\u6b63\u5728\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e\u8b49\u66f8", },
	{ "sessioncertstore.cert.added", "\u5df2\u5728\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u65b0\u589e\u8b49\u66f8" },
	{ "sessioncertstore.cert.removing", "\u6b63\u5728\u79fb\u9664\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8" },
	{ "sessioncertstore.cert.removed", "\u5df2\u5f9e\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u79fb\u9664\u8b49\u66f8" },
	{ "sessioncertstore.cert.instore", "\u6b63\u5728\u6aa2\u67e5\u8b49\u66f8\u662f\u5426\u5728\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d" },
	{ "sessioncertstore.cert.canverify", "\u6aa2\u67e5\u662f\u5426\u53ef\u4f7f\u7528\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u4f86\u9a57\u8b49\u8b49\u66f8" },
	{ "sessioncertstore.cert.iterator", "\u53d6\u5f97\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u8fed\u4ee3\u5668" },
	{ "sessioncertstore.cert.getkeystore", "\u53d6\u5f97\u90e8\u7f72\u968e\u6bb5\u4f5c\u696d\u8b49\u66f8\u5132\u5b58\u5eab\u7684\u5bc6\u9470\u5132\u5b58\u5eab\u7269\u4ef6" },

	{ "iexplorer.cert.loading", "\u6b63\u5728\u5f9e Internet Explorer {0} \u8b49\u66f8\u5132\u5b58\u5eab\u8f09\u5165\u8b49\u66f8" },
	{ "iexplorer.cert.loaded", "\u5df2\u5f9e Internet Explorer {0} \u8b49\u66f8\u5132\u5b58\u5eab\u8f09\u5165\u8b49\u66f8" },
	{ "iexplorer.cert.instore", "\u6b63\u5728\u6aa2\u67e5\u8b49\u66f8\u662f\u5426\u5728 Internet Explorer {0} \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d" },
	{ "iexplorer.cert.canverify", "\u6aa2\u67e5\u662f\u5426\u53ef\u4f7f\u7528 Internet Explorer {0} \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u4f86\u9a57\u8b49\u8b49\u66f8" },
	{ "iexplorer.cert.iterator", "\u53d6\u5f97 Internet Explorer {0} \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u8fed\u4ee3\u5668" },
	{ "iexplorer.cert.verify.ok", "\u8b49\u66f8\u5df2\u7d93\u904e Internet Explorer {0} \u8b49\u66f8\u9a57\u8b49\u6210\u529f" },
	{ "iexplorer.cert.verify.fail", "\u8b49\u66f8\u7d93\u904e Internet Explorer {0} \u8b49\u66f8\u9a57\u8b49\u5931\u6557" },
	{ "iexplorer.cert.tobeverified", "\u5c07\u88ab\u9a57\u8b49\u7684\u8b49\u66f8\uff1a\n{0}" },
	{ "iexplorer.cert.tobecompared", "\u6b63\u5728\u5c07\u8b49\u66f8\u8207\u4ee5\u4e0b Internet Explorer {0} \u8b49\u66f8\u6bd4\u8f03\uff1a\n{1}" },

	{ "mozilla.cert.loading", "\u6b63\u5728\u5f9e Mozilla {0} \u8b49\u66f8\u5132\u5b58\u5eab\u8f09\u5165\u8b49\u66f8" },
        { "mozilla.cert.loaded", "\u5df2\u5f9e Mozilla {0} \u8b49\u66f8\u5132\u5b58\u5eab\u8f09\u5165\u8b49\u66f8" },
        { "mozilla.cert.instore", "\u6b63\u5728\u6aa2\u67e5\u8b49\u66f8\u662f\u5426\u5728 Mozilla {0} \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d" },
        { "mozilla.cert.canverify", "\u6aa2\u67e5\u662f\u5426\u53ef\u4f7f\u7528 Mozilla {0} \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u4f86\u9a57\u8b49\u8b49\u66f8" },
        { "mozilla.cert.iterator", "\u53d6\u5f97 Mozilla {0} \u8b49\u66f8\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u8fed\u4ee3\u5668" },
        { "mozilla.cert.verify.ok", "\u5df2\u6210\u529f\u4f7f\u7528 Mozilla {0} \u8b49\u66f8\u9a57\u8b49\u8b49\u66f8" },
        { "mozilla.cert.verify.fail", "\u7121\u6cd5\u4f7f\u7528 Mozilla {0} \u8b49\u66f8\u9a57\u8b49\u8b49\u66f8" },
        { "mozilla.cert.tobeverified", "\u5c07\u88ab\u9a57\u8b49\u7684\u8b49\u66f8\uff1a\n{0}" },
        { "mozilla.cert.tobecompared", "\u6b63\u5728\u5c07\u8b49\u66f8\u8207\u4ee5\u4e0b Mozilla {0} \u8b49\u66f8\u6bd4\u8f03\uff1a\n{1}" },

        { "browserkeystore.jss.no", "\u672a\u627e\u5230 JSS \u5957\u88dd\u8edf\u9ad4" },
        { "browserkeystore.jss.yes", "\u5df2\u8f09\u5165 JSS \u5957\u88dd\u8edf\u9ad4" },
        { "browserkeystore.jss.notconfig", "\u672a\u914d\u7f6e JSS" },
        { "browserkeystore.jss.config", "\u5df2\u914d\u7f6e JSS" },
        { "browserkeystore.mozilla.dir", "\u6b63\u5728\u5b58\u53d6 Mozilla \u4f7f\u7528\u8005\u8a2d\u5b9a\u6a94\u4e2d\u7684\u91d1\u9470\u548c\u8b49\u66f8\uff1a {0}" },
	{ "browserkeystore.password.dialog.buttonOK", "\u78ba\u5b9a(O)" },
	{ "browserkeystore.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "browserkeystore.password.dialog.buttonCancel", "\u53d6\u6d88(C)" },
	{ "browserkeystore.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "browserkeystore.password.dialog.caption", "\u9700\u8981\u5bc6\u78bc" },
	{ "browserkeystore.password.dialog.text", "\u8acb\u8f38\u5165 {0} \u7684\u5bc6\u78bc\uff1a\n" },
	{ "mozillamykeystore.priv.notfound", "\u672a\u627e\u5230\u8b49\u66f8\u7684\u79c1\u5bc6\u91d1\u9470\uff1a {0}" },
	{ "hostnameverifier.automation.ignoremismatch", "\u81ea\u52d5\u5316\uff1a \u5ffd\u7565\u4e3b\u6a5f\u540d\u7a31\u4e0d\u7b26" },

	{ "trustdecider.check.basicconstraints", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u57fa\u672c\u9650\u5236\u6642\u5931\u6557" },
	{ "trustdecider.check.leafkeyusage", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u8449\u5bc6\u9470\u4f7f\u7528\u6642\u5931\u6557" },
	{ "trustdecider.check.signerkeyusage", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u7c3d\u540d\u8005\u5bc6\u9470\u4f7f\u7528\u6642\u5931\u6557" },
	{ "trustdecider.check.extensions", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u81e8\u754c\u5ef6\u4f38\u6642\u5931\u6557" },
	{ "trustdecider.check.signature", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u7c3d\u540d\u6642\u5931\u6557" },
	{ "trustdecider.check.basicconstraints.certtypebit", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5 netscape \u985e\u578b\u4f4d\u5143\u6642\u5931\u6557" },
	{ "trustdecider.check.basicconstraints.extensionvalue", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5 netscape \u5ef6\u4f38\u503c\u6642\u5931\u6557" },
	{ "trustdecider.check.basicconstraints.bitvalue", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5 netscape \u4f4d\u5143 5\u30016\u30017 \u7684\u503c\u6642\u5931\u6557" },
	{ "trustdecider.check.basicconstraints.enduser", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u4f5c\u70ba CA \u7684\u4e00\u822c\u4f7f\u7528\u8005\u6642\u5931\u6557" },
	{ "trustdecider.check.basicconstraints.pathlength", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u8def\u5f91\u9577\u5ea6\u9650\u5236\u6642\u5931\u6557" },
	{ "trustdecider.check.leafkeyusage.length", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u5bc6\u9470\u4f7f\u7528\u9577\u5ea6\u6642\u5931\u6557" },
	{ "trustdecider.check.leafkeyusage.digitalsignature", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u6578\u4f4d\u7c3d\u540d\u6642\u5931\u6557" },
	{ "trustdecider.check.leafkeyusage.extkeyusageinfo", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u5ef6\u4f38\u5bc6\u9470\u4f7f\u7528\u8cc7\u8a0a\u6642\u5931\u6557" },
	{ "trustdecider.check.leafkeyusage.tsaextkeyusageinfo", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5 TSA \u5ef6\u4f38\u5bc6\u9470\u4f7f\u7528\u8cc7\u8a0a\u5931\u6557" },
	{ "trustdecider.check.leafkeyusage.certtypebit", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5 netscape \u985e\u578b\u4f4d\u5143\u6642\u5931\u6557" },
	{ "trustdecider.check.signerkeyusage.lengthandbit", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u9577\u5ea6\u548c\u4f4d\u5143\u6642\u5931\u6557" },
	{ "trustdecider.check.signerkeyusage.keyusage", "\u5728\u8b49\u66f8\u4e2d\u6aa2\u67e5\u5bc6\u9470\u4f7f\u7528\u6642\u5931\u6557" },
	{ "trustdecider.check.canonicalize.updatecert", "\u4f7f\u7528 cacerts \u6a94\u6848\u4e2d\u7684\u8b49\u66f8\u66f4\u65b0\u6839\u8b49\u66f8" },
	{ "trustdecider.check.canonicalize.missing", "\u65b0\u589e\u907a\u5931\u7684\u6839\u8b49\u66f8" },
	{ "trustdecider.check.gettrustedcert.find", "\u5728 cacerts \u6a94\u6848\u4e2d\u627e\u5230\u6709\u6548\u7684 Root CA" },
	{ "trustdecider.check.gettrustedissuercert.find", "\u5728 cacerts \u6a94\u6848\u4e2d\u627e\u5230\u907a\u5931\u7684\u6709\u6548 Root CA" },
	{ "trustdecider.check.timestamping.no", "\u7121\u53ef\u7528\u7684\u6642\u9593\u6233\u8a18\u8cc7\u8a0a" },
	{ "trustdecider.check.timestamping.yes", "\u6709\u53ef\u7528\u7684\u6642\u9593\u6233\u8a18\u8cc7\u8a0a" },
	{ "trustdecider.check.timestamping.tsapath", "\u958b\u59cb\u6aa2\u67e5 TSA \u8b49\u66f8\u8def\u5f91" },
	{ "trustdecider.check.timestamping.inca", "\u96d6\u7136\u8b49\u66f8\u5df2\u904e\u671f\uff0c\u4f46\u6642\u9593\u6233\u8a18\u6a19\u8a18\u5176\u4ecd\u5728\u6709\u6548\u671f\u4e2d\uff0c\u4e26\u5177\u6709\u6709\u6548 TSA" },
	{ "trustdecider.check.timestamping.notinca", "\u8b49\u66f8\u5df2\u904e\u671f\uff0c\u4f46 TSA \u7121\u6548" },
	{ "trustdecider.check.timestamping.valid", "\u8b49\u66f8\u5df2\u904e\u671f\uff0c\u4f46\u88ab\u6642\u9593\u6233\u8a18\u6a19\u8a18\u5176\u4ecd\u5728\u6709\u6548\u671f\u4e2d" },
	{ "trustdecider.check.timestamping.invalid", "\u8b49\u66f8\u5df2\u904e\u671f\uff0c\u4e26\u6642\u9593\u6233\u8a18\u6a19\u8a18\u5176\u7121\u6548" },
	{ "trustdecider.check.timestamping.need", "\u8b49\u66f8\u5df2\u904e\u671f\uff0c\u9700\u8981\u6aa2\u67e5\u6642\u9593\u6233\u8a18\u8cc7\u8a0a" },
	{ "trustdecider.check.timestamping.noneed", "\u8b49\u66f8\u5c1a\u672a\u904e\u671f\uff0c\u4e0d\u9700\u8981\u6aa2\u67e5\u6642\u9593\u6233\u8a18\u8cc7\u8a0a" },
	{ "trustdecider.check.timestamping.notfound", "\u7121\u6cd5\u627e\u5230\u6a19\u8a18\u6642\u9593\u6233\u8a18\u7684\u65b0 API" },
	{ "trustdecider.user.grant.session", "\u4f7f\u7528\u8005\u53ea\u5c0d\u6b64\u968e\u6bb5\u4f5c\u696d\u6388\u8207\u7a0b\u5f0f\u78bc\u7684\u5c08\u7528\u6b0a" },
	{ "trustdecider.user.grant.forever", "\u4f7f\u7528\u8005\u5c0d\u6b64\u7a0b\u5f0f\u78bc\u6388\u8207\u6c38\u4e45\u7684\u5c08\u7528\u6b0a" },
	{ "trustdecider.user.deny", "\u4f7f\u7528\u8005\u5df2\u62d2\u7d55\u5c0d\u7a0b\u5f0f\u78bc\u7684\u5c08\u7528\u6b0a" },
	{ "trustdecider.automation.trustcert", "\u81ea\u52d5\u5316\uff1a \u7c3d\u7ae0\u7684\u4fe1\u4efb RSA \u8b49\u66f8" },
	{ "trustdecider.code.type.applet", "Applet" },
	{ "trustdecider.code.type.application", "\u61c9\u7528\u7a0b\u5f0f" },
	{ "trustdecider.code.type.extension", "\u5ef6\u4f38" },
	{ "trustdecider.code.type.installer", "\u5b89\u88dd\u7a0b\u5f0f" },
	{ "trustdecider.user.cannot.grant.any", "\u60a8\u7684\u5b89\u5168\u914d\u7f6e\u4e0d\u5141\u8a31\u5c0d\u65b0\u8b49\u66f8\u6388\u8207\u8a31\u53ef\u6b0a" },
	{ "trustdecider.user.cannot.grant.notinca", "\u60a8\u7684\u5b89\u5168\u914d\u7f6e\u4e0d\u5141\u8a31\u5c0d\u81ea\u5df1\u7c3d\u7f72\u7684\u8b49\u66f8\u6388\u8207\u8a31\u53ef\u6b0a" },
	{ "x509trustmgr.automation.ignoreclientcert", "\u81ea\u52d5\u5316\uff1a \u5ffd\u7565\u672a\u4fe1\u4efb\u7684\u7528\u6236\u7aef\u8b49\u66f8" },
	{ "x509trustmgr.automation.ignoreservercert", "\u81ea\u52d5\u5316\uff1a \u5ffd\u7565\u672a\u4fe1\u4efb\u7684\u4f3a\u670d\u5668\u8b49\u66f8" },

	{ "net.proxy.text", "Proxy\ufe55" },
	{ "net.proxy.override.text", "Proxy \u7f6e\u63db\ufe55" },
	{ "net.proxy.configuration.text", "Proxy \u914d\u7f6e\ufe55" },
	{ "net.proxy.type.browser", "\u700f\u89bd\u5668 Proxy \u914d\u7f6e" },
	{ "net.proxy.type.auto", "\u81ea\u52d5 Proxy \u914d\u7f6e" },
	{ "net.proxy.type.manual", "\u624b\u52d5\u914d\u7f6e" },
	{ "net.proxy.type.none", "\u7121 Proxy" },
	{ "net.proxy.type.user", "\u4f7f\u7528\u8005\u5df2\u7f6e\u63db\u700f\u89bd\u5668\u7684 Proxy \u8a2d\u5b9a\u3002" },
	{ "net.proxy.loading.ie", "\u6b63\u5728\u5f9e Internet Explorer \u8f09\u5165 Proxy \u914d\u7f6e ..."},
	{ "net.proxy.loading.ns", "\u6b63\u5728\u5f9e Netscape Navigator \u8f09\u5165 Proxy \u914d\u7f6e ..."},
	{ "net.proxy.loading.userdef", "\u6b63\u5728\u8f09\u5165\u4f7f\u7528\u8005\u5b9a\u7fa9\u7684 Proxy \u914d\u7f6e ..."},
	{ "net.proxy.loading.direct", "\u6b63\u5728\u8f09\u5165\u76f4\u63a5 Proxy \u914d\u7f6e ..."},
	{ "net.proxy.loading.manual", "\u6b63\u5728\u8f09\u5165\u624b\u52d5 Proxy \u914d\u7f6e ..."},
	{ "net.proxy.loading.auto",   "\u6b63\u5728\u8f09\u5165\u81ea\u52d5 Proxy \u914d\u7f6e ..."},
	{ "net.proxy.loading.browser",   "\u6b63\u5728\u8f09\u5165\u700f\u89bd\u5668 Proxy \u914d\u7f6e ..."},
	{ "net.proxy.loading.manual.error", "\u7121\u6cd5\u4f7f\u7528\u624b\u52d5 Proxy \u914d\u7f6e - \u8fd4\u56de\u5230 DIRECT"},
	{ "net.proxy.loading.auto.error", "\u7121\u6cd5\u4f7f\u7528\u81ea\u52d5 Proxy \u914d\u7f6e - \u8fd4\u56de\u5230 MANUAL"},
	{ "net.proxy.loading.done", "\u5b8c\u6210\u3002"},
	{ "net.proxy.browser.pref.read", "\u6b63\u5728\u5f9e {0} \u8b80\u53d6\u4f7f\u7528\u8005\u500b\u4eba\u559c\u597d\u8a2d\u5b9a\u6a94"},
	{ "net.proxy.browser.proxyEnable", "    Proxy \u555f\u7528\uff1a {0}"},
	{ "net.proxy.browser.proxyList",     "    Proxy \u6e05\u55ae\uff1a {0}"},
	{ "net.proxy.browser.proxyOverride", "    Proxy \u7f6e\u63db\uff1a {0}"},
	{ "net.proxy.browser.autoConfigURL", "    \u81ea\u52d5\u914d\u7f6e URL\uff1a {0}"},
	{ "net.proxy.browser.smartConfig", "\u5075\u6e2c\u5728\u9023\u63a5\u57e0 {1} \u7684 Proxy \u4f3a\u670d\u5668 {0}"},
        { "net.proxy.browser.connectionException", "\u7121\u6cd5\u806f\u7e6b\u5728\u9023\u63a5\u57e0 {1} \u7684 Proxy \u4f3a\u670d\u5668 {0}"},
	{ "net.proxy.ns6.regs.exception", "\u8b80\u53d6\u8a3b\u518a\u6a94\u6848\u6642\u767c\u751f\u932f\u8aa4\uff1a {0}"},
	{ "net.proxy.pattern.convert", "\u5c07 Proxy \u65c1\u901a\u6e05\u55ae\u8f49\u63db\u6210\u6b63\u898f\u8868\u793a\u5f0f\uff1a "},
	{ "net.proxy.pattern.convert.error", "\u7121\u6cd5\u5c07 Proxy \u65c1\u901a\u6e05\u55ae\u8f49\u63db\u6210\u6b63\u898f\u8868\u793a\u5f0f - \u5ffd\u7565"},
	{ "net.proxy.auto.download.ins", "\u6b63\u5728\u5f9e {0} \u4e0b\u8f09 INS \u6a94\u6848" },
	{ "net.proxy.auto.download.js", "\u6b63\u5728\u5f9e {0} \u4e0b\u8f09\u81ea\u52d5 Proxy \u6a94\u6848" },
	{ "net.proxy.auto.result.error", "\u7121\u6cd5\u5f9e\u8a55\u4f30\u4f86\u6c7a\u5b9a Proxy \u8a2d\u5b9a -  \u5931\u6548\u6298\u8fd4\u6210 DIRECT"},
        { "net.proxy.service.not_available", "\u6c92\u6709 {0} \u53ef\u7528\u7684 Proxy \u670d\u52d9 - \u9810\u8a2d\u4f7f\u7528 DIRECT" },
	{ "net.proxy.error.caption", "\u932f\u8aa4 - Proxy \u914d\u7f6e" },
	{ "net.proxy.nsprefs.error", "<html><b>\u7121\u6cd5\u91cd\u65b0\u53d6\u5f97 Proxy \u8a2d\u5b9a\u503c</b></html>\u9000\u56de\u5230\u5176\u4ed6\u7684 proxy \u914d\u7f6e\u3002\n" },
	{ "net.proxy.connect", "\u4f7f\u7528 Proxy {1} \u4f86\u9023\u63a5 {0}" },

	{ "net.authenticate.caption", "\u9700\u8981\u5bc6\u78bc - \u7db2\u8def\u9023\u7dda"},
	{ "net.authenticate.label", "<html><b>\u8acb\u8f38\u5165\u60a8\u7684\u4f7f\u7528\u8005\u540d\u7a31\u548c\u5bc6\u78bc\uff1a</b></html>"},
	{ "net.authenticate.resource", "\u8cc7\u6e90\ufe55" },
	{ "net.authenticate.username", "\u4f7f\u7528\u8005\u540d\u7a31(U)\ufe55" },
        { "net.authenticate.username.mnemonic", "VK_U" },
	{ "net.authenticate.password", "\u5bc6\u78bc(P)\uff1a" },
        { "net.authenticate.password.mnemonic", "VK_P" },
	{ "net.authenticate.firewall", "\u4f3a\u670d\u5668\uff1a" },
	{ "net.authenticate.domain", "\u7db2\u57df(D)\uff1a"},
        { "net.authenticate.domain.mnemonic", "VK_D" },
	{ "net.authenticate.realm", "\u7bc4\u7587\uff1a" },
	{ "net.authenticate.scheme", "\u6a5f\u5236\uff1a" },
	{ "net.authenticate.unknownSite", "\u672a\u77e5\u7db2\u7ad9" },

	{ "net.cookie.cache", "Cookie \u5feb\u53d6\u8a18\u61b6\u9ad4\uff1a " },
	{ "net.cookie.server", "\u4f3a\u670d\u5668 {0} \u8981\u6c42\u4ee5 \"{1}\" \u4f86 set-cookie" },
	{ "net.cookie.connect", "\u4f7f\u7528 Cookie \"{1}\" \u4f86\u9023\u63a5 {0}" },
	{ "net.cookie.ignore.setcookie", "\u7121\u6cd5\u4f7f\u7528 Cookie \u670d\u52d9 - \u5ffd\u7565 \"Set-Cookie\"" },
	{ "net.cookie.noservice", "\u7121\u6cd5\u4f7f\u7528 Cookie \u670d\u52d9 - \u4f7f\u7528\u5feb\u53d6\u8a18\u61b6\u9ad4\u4f86\u6c7a\u5b9a \"Cookie\"" },

	{"about.java.version", "\u7248\u672c {0} (build {1})"},
	{"about.prompt.info", "\u5982\u9700\u95dc\u65bc Java \u6280\u8853\u7684\u8cc7\u8a0a\u4ee5\u53ca\u700f\u89bd Java \u61c9\u7528\u7a0b\u5f0f\uff0c\u8acb\u81f3"},
	{"about.home.link", "http://www.java.com"},
	{"about.option.close", "\u95dc\u9589(C)"},
	{"about.option.close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{"about.copyright", "Copyright 2004 Sun Microsystems, Inc."},
	{"about.legal.note", "\u7248\u6b0a\u6240\u6709\u3002\u5176\u4f7f\u7528\u9808\u7b26\u5408\u6388\u6b0a\u689d\u6b3e\u898f\u5b9a\u3002"},


	{ "cert.remove_button", "\u79fb\u9664(M)" },
        { "cert.remove_button.mnemonic", "VK_M" },
        { "cert.import_button", "\u532f\u5165(I)" },
        { "cert.import_button.mnemonic", "VK_I" },
        { "cert.export_button", "\u532f\u51fa(E)" },
        { "cert.export_button.mnemonic", "VK_E" },
        { "cert.details_button", "\u8a73\u7d30\u8cc7\u8a0a(D)" },
        { "cert.details_button.mnemonic", "VK_D" },
        { "cert.viewcert_button", "\u6aa2\u8996\u8b49\u66f8(V)" },
        { "cert.viewcert_button.mnemonic", "VK_V" },
        { "cert.close_button", "\u95dc\u9589(C)" },
        { "cert.close_button.mnemonic", "VK_C" },
        { "cert.type.trusted_certs", "\u4fe1\u4efb\u7684\u8b49\u66f8" },
        { "cert.type.secure_site", "\u5b89\u5168\u7db2\u7ad9" },
        { "cert.type.client_auth", "\u7528\u6236\u7aef\u8a8d\u8b49" },
        { "cert.type.signer_ca", "\u7c3d\u540d\u8005 CA" },
        { "cert.type.secure_site_ca", "\u5b89\u5168\u7db2\u7ad9 CA" },
        { "cert.rbutton.user", "\u4f7f\u7528\u8005" },
        { "cert.rbutton.system", "\u7cfb\u7d71" },
        { "cert.settings", "\u8b49\u66f8" },
        { "cert.dialog.import.error.caption", "\u932f\u8aa4 - \u532f\u5165\u8b49\u66f8" },
        { "cert.dialog.export.error.caption", "\u932f\u8aa4 - \u532f\u51fa\u8b49\u66f8" },
	{ "cert.dialog.import.format.text", "<html><b>\u7121\u6cd5\u8b58\u5225\u7684\u6a94\u6848\u683c\u5f0f</b></html>\u4e0d\u6703\u532f\u5165\u8b49\u66f8\u3002" },
	{ "cert.dialog.export.password.text", "<html><b>\u7121\u6548\u7684\u5bc6\u78bc</b></html>\u60a8\u8f38\u5165\u7684\u5bc6\u78bc\u4e0d\u6b63\u78ba\u3002" },
	{ "cert.dialog.import.file.text", "<html><b>\u6a94\u6848\u4e0d\u5b58\u5728</b></html>\u4e0d\u6703\u532f\u5165\u8b49\u66f8\u3002" },
	{ "cert.dialog.import.password.text", "<html><b>\u7121\u6548\u7684\u5bc6\u78bc</b></html>\u60a8\u8f38\u5165\u7684\u5bc6\u78bc\u4e0d\u6b63\u78ba\u3002" },
        { "cert.dialog.password.caption", "\u5bc6\u78bc" },
        { "cert.dialog.password.import.caption", "\u9700\u8981\u5bc6\u78bc - \u532f\u5165" },
        { "cert.dialog.password.export.caption", "\u9700\u8981\u5bc6\u78bc - \u532f\u51fa" },
        { "cert.dialog.password.text", "\u8f38\u5165\u5bc6\u78bc\u4ee5\u5b58\u53d6\u8a72\u6a94\u6848\uff1a\n" },
        { "cert.dialog.exportpassword.text", "\u8f38\u5165\u5bc6\u78bc\u4ee5\u5b58\u53d6\u7528\u6236\u7aef\u8a8d\u8b49\u5bc6\u9470\u5132\u5b58\u5eab\u4e2d\u7684\u8a72\u79c1\u5bc6\u91d1\u9470\uff1a\n" },
        { "cert.dialog.savepassword.text", "\u8f38\u5165\u5bc6\u78bc\u4ee5\u5132\u5b58\u8a72\u5bc6\u9470\u6a94\u6848\uff1a\n" },
        { "cert.dialog.password.okButton", "\u78ba\u5b9a" },
        { "cert.dialog.password.cancelButton", "\u53d6\u6d88" },
        { "cert.dialog.export.error.caption", "\u932f\u8aa4 - \u532f\u51fa\u8b49\u66f8" },
        { "cert.dialog.export.text", "<html><b>\u7121\u6cd5\u532f\u51fa</b></html>\u672a\u532f\u51fa\u8b49\u66f8\u3002" },
        { "cert.dialog.remove.text", "\u60a8\u78ba\u5be6\u8981\u522a\u9664\u8b49\u66f8\u55ce\uff1f" },
	{ "cert.dialog.remove.caption", "\u79fb\u9664\u8b49\u66f8" },
	{ "cert.dialog.issued.to", "\u767c\u51fa\u7d66" },
	{ "cert.dialog.issued.by", "\u767c\u51fa\u8005" },
	{ "cert.dialog.user.level", "\u4f7f\u7528\u8005" },
	{ "cert.dialog.system.level", "\u7cfb\u7d71" },
	{ "cert.dialog.certtype", "\u8b49\u66f8\u985e\u578b\uff1a "},

	{ "controlpanel.jre.platformTableColumnTitle","\u5e73\u53f0"},
	{ "controlpanel.jre.productTableColumnTitle","\u7522\u54c1" },
	{ "controlpanel.jre.locationTableColumnTitle","\u4f4d\u7f6e" },
	{ "controlpanel.jre.pathTableColumnTitle","\u8def\u5f91" },
	{ "controlpanel.jre.enabledTableColumnTitle", "\u5df2\u555f\u7528" },

	{ "jnlp.jre.title", "JNLP \u57f7\u884c\u968e\u6bb5\u8a2d\u5b9a" },
	{ "jnlp.jre.versions", "Java \u57f7\u884c\u968e\u6bb5\u7248\u672c" },
	{ "jnlp.jre.choose.button", "\u9078\u64c7(H)" },
	{ "jnlp.jre.find.button", "\u5c0b\u627e(F)" },
	{ "jnlp.jre.add.button", "\u65b0\u589e(A)" },
	{ "jnlp.jre.remove.button", "\u79fb\u9664(R)" },
	{ "jnlp.jre.ok.button", "\u78ba\u5b9a(O)" },
	{ "jnlp.jre.cancel.button", "\u53d6\u6d88(C)" },
	{ "jnlp.jre.choose.button.mnemonic", "VK_H" },
	{ "jnlp.jre.find.button.mnemonic", "VK_F" },
	{ "jnlp.jre.add.button.mnemonic", "VK_A" },
	{ "jnlp.jre.remove.button.mnemonic", "VK_R" },
	{ "jnlp.jre.ok.button.mnemonic", "VK_O" },
	{ "jnlp.jre.cancel.button.mnemonic", "VK_C" },

	{ "find.dialog.title", "JRE \u5c0b\u627e\u7a0b\u5f0f"},
	{ "find.title", "Java \u57f7\u884c\u968e\u6bb5\u74b0\u5883"},
	{ "find.cancelButton", "\u53d6\u6d88(C)"},
	{ "find.prevButton", "\u4e0a\u4e00\u6b65(P)"},
	{ "find.nextButton", "\u4e0b\u4e00\u6b65(N)"},
	{ "find.cancelButtonMnemonic", "VK_C"},
	{ "find.prevButtonMnemonic", "VK_P"},
	{ "find.nextButtonMnemonic", "VK_N"},
	{ "find.intro", "\u70ba\u4e86\u555f\u52d5\u61c9\u7528\u7a0b\u5f0f\uff0cJava Web Start \u9700\u8981\u77e5\u9053 Java \u57f7\u884c\u968e\u6bb5\u74b0\u5883\u7684\u5b89\u88dd\u4f4d\u7f6e\u3002\n\n\u60a8\u53ef\u4ee5\u9078\u53d6\u5df2\u77e5\u7684 JRE\uff0c\u6216\u5728\u6a94\u6848\u7cfb\u7d71\u4e2d\u9078\u53d6\u8981\u5728\u5176\u4e2d\u641c\u5c0b JRE \u7684\u76ee\u9304\u3002" },

	{ "find.searching.title", "\u6b63\u5728\u641c\u5c0b\u53ef\u7528\u7684 JRE\uff0c\u9019\u53ef\u80fd\u9700\u8981\u5e7e\u5206\u9418\u3002" },
	{ "find.searching.prefix", "\u6b63\u5728\u6aa2\u67e5\uff1a " },
	{ "find.foundJREs.title", "\u627e\u5230\u4ee5\u4e0b JRE\uff0c\u6309\u4e00\u4e0b\u300c\u4e0b\u4e00\u6b65\u300d\u4ee5\u65b0\u589e\u5b83\u5011" },
	{ "find.noJREs.title", "\u7121\u6cd5\u627e\u5230 JRE\uff0c\u6309\u4e00\u4e0b\u300c\u4e0a\u4e00\u6b65\u300d\u4ee5\u9078\u53d6\u5176\u4ed6\u641c\u5c0b\u4f4d\u7f6e" },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Java(tm) \u90e8\u7f72\u5c6c\u6027\n"
                        + "# \u8acb\u52ff\u7de8\u8f2f\u8a72\u6a94\u6848\u3002  \u5b83\u7531\u6a5f\u5668\u7522\u751f\u3002\n"
                        + "# \u4f7f\u7528 Java \u63a7\u5236\u9762\u677f\u4f86\u7de8\u8f2f\u5c6c\u6027\u3002" },
        { "config.unknownSubject", "\u4e0d\u660e\u7684\u4e3b\u984c" },
        { "config.unknownIssuer", "\u4e0d\u660e\u7684\u767c\u884c\u4eba" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>URL \u7570\u5e38</b></html>\u81ea\u52d5 Proxy \u914d\u7f6e URL \u7121\u6548\u3002" },
        { "config.proxy.autourl.invalid.caption", "\u932f\u8aa4 - Proxy" },
	// Java Web Start Properties
	 { "APIImpl.clipboard.message.read", "\u6b64\u61c9\u7528\u7a0b\u5f0f\u5df2\u8981\u6c42\u5c0d\u7cfb\u7d71\u526a\u8cbc\u7c3f\u7684\u552f\u8b80\u6b0a\u9650\u3002  \u61c9\u7528\u7a0b\u5f0f\u53ef\u80fd\u5b58\u53d6\u5132\u5b58\u5728\u526a\u8cbc\u7c3f\u7684\u6a5f\u5bc6\u8cc7\u8a0a\u3002  \u60a8\u8981\u540c\u610f\u6b64\u52d5\u4f5c\u55ce\uff1f" },
        { "APIImpl.clipboard.message.write", "\u6b64\u61c9\u7528\u7a0b\u5f0f\u5df2\u8981\u6c42\u5c0d\u7cfb\u7d71\u526a\u8cbc\u7c3f\u7684\u5beb\u5165\u6b0a\u9650\u3002  \u61c9\u7528\u7a0b\u5f0f\u53ef\u80fd\u6539\u5beb\u5132\u5b58\u5728\u526a\u8cbc\u7c3f\u7684\u8cc7\u8a0a\u3002  \u60a8\u8981\u540c\u610f\u6b64\u52d5\u4f5c\u55ce\uff1f" },
        { "APIImpl.file.open.message", "\u6b64\u61c9\u7528\u7a0b\u5f0f\u5df2\u8981\u6c42\u5c0d\u6a94\u6848\u7cfb\u7d71\u7684\u8b80\u53d6\u6b0a\u3002  \u61c9\u7528\u7a0b\u5f0f\u53ef\u5b58\u53d6\u5132\u5b58\u5728\u6a94\u6848\u7cfb\u7d71\u4e0a\u7684\u6a5f\u5bc6\u8cc7\u8a0a\u3002  \u60a8\u8981\u540c\u610f\u6b64\u52d5\u4f5c\u55ce\uff1f" },
        { "APIImpl.file.save.fileExist", "{0} \u5df2\u7d93\u5b58\u5728\u3002\n\u60a8\u8981\u53d6\u4ee3\u5b83\u55ce\uff1f" },
        { "APIImpl.file.save.fileExistTitle", "\u6a94\u6848\u5b58\u5728" },
        { "APIImpl.file.save.message", "\u6b64\u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u5c0d\u5340\u57df\u6a94\u6848\u7cfb\u7d71\u7684\u8b80\u53d6/\u5beb\u5165\u6b0a\u3002  \u63a5\u53d7\u6b64\u52d5\u4f5c\u53ea\u80fd\u8b93\u61c9\u7528\u7a0b\u5f0f\u5c0d\u4e0b\u5217\u6a94\u6848\u5c0d\u8a71\u65b9\u584a\u4e2d\u9078\u53d6\u7684\u6a94\u6848\u64c1\u6709\u5b58\u53d6\u6b0a\u3002  \u60a8\u63a5\u53d7\u6b64\u52d5\u4f5c\u55ce\uff1f" },
        { "APIImpl.persistence.accessdenied", "URL {0} \u7684\u6301\u7e8c\u5132\u5b58\u5b58\u53d6\u6b0a\u88ab\u62d2\u7d55" },
        { "APIImpl.persistence.filesizemessage", "\u8d85\u51fa\u6700\u5927\u6a94\u6848\u9577\u5ea6" },
        { "APIImpl.persistence.message", "\u6b64\u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u984d\u5916\u7684\u5340\u57df\u78c1\u789f\u5132\u5b58\u7a7a\u9593\u3002  \u4ee5\u76ee\u524d\u800c\u8a00\uff0c\u6240\u914d\u7f6e\u7684\u6700\u5927\u5132\u5b58\u7a7a\u9593\u70ba {1} \u4f4d\u5143\u7d44\u3002  \u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u589e\u52a0\u70ba {0} \u4f4d\u5143\u7d44\u3002  \u60a8\u63a5\u53d7\u6b64\u52d5\u4f5c\u55ce\uff1f" },
        { "APIImpl.print.message", "\u6b64\u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u5b58\u53d6\u9810\u8a2d\u7684\u5370\u8868\u6a5f\u3002  \u63a5\u53d7\u6b64\u52d5\u4f5c\u4fbf\u6703\u4f7f\u61c9\u7528\u7a0b\u5f0f\u64c1\u6709\u5370\u8868\u6a5f\u5beb\u5165\u6b0a\u3002  \u60a8\u63a5\u53d7\u6b64\u52d5\u4f5c\u55ce\uff1f" },
	{ "APIImpl.extended.fileOpen.message1", "\u6b64\u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u5c0d\u5340\u57df\u6a94\u6848\u7cfb\u7d71\u4e0a\u7684\u4ee5\u4e0b\u6a94\u6848\u7684\u8b80\u53d6/\u5beb\u5165\u6b0a\uff1a"},
	{ "APIImpl.extended.fileOpen.message2", "\u63a5\u53d7\u6b64\u52d5\u4f5c\u53ea\u80fd\u8b93\u61c9\u7528\u7a0b\u5f0f\u5c0d\u4ee5\u4e0a\u5217\u51fa\u7684\u6a94\u6848\u64c1\u6709\u5b58\u53d6\u6b0a\u3002  \u60a8\u63a5\u53d7\u6b64\u52d5\u4f5c\u55ce\uff1f"},
        { "APIImpl.securityDialog.no", "\u5426" },
        { "APIImpl.securityDialog.remember", "\u4e0d\u8981\u518d\u986f\u793a\u6b64\u8aee\u8a62" },
        { "APIImpl.securityDialog.title", "\u5b89\u5168\u8aee\u8a62" },
        { "APIImpl.securityDialog.yes", "\u662f" },
        { "Launch.error.installfailed", "\u5b89\u88dd\u5931\u6557" },
        { "aboutBox.closeButton", "\u95dc\u9589" },
        { "aboutBox.versionLabel", "\u7248\u672c {0} (build {1})" },
        { "applet.failedToStart", "\u7121\u6cd5\u555f\u52d5 Applet\uff1a {0}" },
        { "applet.initializing", "\u8d77\u59cb\u8a2d\u5b9a Applet" },
        { "applet.initializingFailed", "\u7121\u6cd5\u8d77\u59cb\u8a2d\u5b9a Applet\uff1a {0}" },
        { "applet.running", "\u57f7\u884c\u4e2d..." },
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "\u5fc5\u9808\u91cd\u65b0\u555f\u52d5 Windows \u624d\u80fd\u4f7f\u8b8a\u66f4\u751f\u6548\u3002\n\n\u60a8\u8981\u7acb\u5373\u91cd\u65b0\u555f\u52d5 Windows \u55ce\uff1f" },
        { "extensionInstall.rebootTitle", "\u91cd\u65b0\u555f\u52d5 Windows" }, 
        { "install.configButton", "\u914d\u7f6e(C)..." },
        { "install.configMnemonic", "VK_C" },
        { "install.errorInstalling", "\u5617\u8a66\u5b89\u88dd Java \u57f7\u884c\u968e\u6bb5\u74b0\u5883\u6642\u767c\u751f\u975e\u9810\u671f\u7684\u932f\u8aa4\uff0c\u8acb\u91cd\u8a66\u3002" },
        { "install.errorRestarting", "\u555f\u52d5\u6642\u767c\u751f\u975e\u9810\u671f\u7684\u932f\u8aa4\uff0c\u8acb\u91cd\u8a66\u3002" },
        { "install.title", "{0} - \u5efa\u7acb\u6377\u5f91" },

        { "install.windows.both.message", "\u60a8\u8981\u70ba {0} \u5efa\u7acb\u684c\u9762\u548c\u958b\u59cb\u529f\u80fd\u8868\u6377\u5f91\u55ce\n\uff1f" },
	{ "install.gnome.both.message", "\u60a8\u8981\u70ba {0} \u5efa\u7acb\u684c\u9762\u548c\u61c9\u7528\u7a0b\u5f0f\u529f\u80fd\u8868\u6377\u5f91\u55ce\n\uff1f" },
	{ "install.desktop.message", "\u60a8\u8981\u70ba {0} \u5efa\u7acb\u684c\u9762\u6377\u5f91\u55ce\n\uff1f" },
	{ "install.windows.menu.message", "\u60a8\u8981\u70ba {0} \u5efa\u7acb\u958b\u59cb\u529f\u80fd\u8868\u6377\u5f91\u55ce\n\uff1f" },
	{ "install.gnome.menu.message", "\u60a8\u8981\u70ba {0} \u5efa\u7acb\u61c9\u7528\u7a0b\u5f0f\u529f\u80fd\u8868\u6377\u5f91\u55ce\n\uff1f" },
        { "install.noButton", "\u5426(N)" },
        { "install.noMnemonic", "VK_N" },
        { "install.yesButton", "\u662f(Y)" },
        { "install.yesMnemonic", "VK_Y" },
        { "launch.cancel", "\u53d6\u6d88" },
        { "launch.downloadingJRE", "\u5f9e {1} \u8981\u6c42 JRE {0}" },
        { "launch.error.badfield", "\u6b04\u4f4d {0} \u6709\u7121\u6548\u503c\uff1a {1}" },
        { "launch.error.badfield-signedjnlp", "\u5728\u7c3d\u7f72\u7684\u555f\u52d5\u6a94\u6848\u4e2d\uff0c\u6b04\u4f4d {0} \u6709\u7121\u6548\u503c\uff1a {1}" },
        { "launch.error.badfield.download.https", "\u7121\u6cd5\u900f\u904e HTTPS \u4e0b\u8f09" },
        { "launch.error.badfield.https", "Java 1.4+ \u662f\u652f\u63f4 HTTPS \u6240\u5fc5\u9700\u7684" },
        { "launch.error.badjarfile", "\u5728 {0} \u4e0a JAR \u6a94\u6848\u5df2\u6bc0\u58de" },
        { "launch.error.badjnlversion", "\u555f\u52d5\u6a94\u6848 {0} \u4e2d\u6709\u4e0d\u53d7\u652f\u63f4\u7684 JNLP \u7248\u672c\u3002\u6b64\u7248\u672c\u50c5\u652f\u63f4 1.0 \u7248\u3002\u8acb\u5411\u61c9\u7528\u7a0b\u5f0f\u4f9b\u61c9\u5546\u5831\u544a\u6b64\u554f\u984c\u3002" },
        { "launch.error.badmimetyperesponse", "\u5b58\u53d6\u8cc7\u6e90 {0} - {1} \u6642\u5f9e\u4f3a\u670d\u5668\u50b3\u56de\u932f\u8aa4 MIME \u985e\u578b" },
        { "launch.error.badsignedjnlp", "\u7121\u6cd5\u9a57\u8b49\u555f\u52d5\u6a94\u6848\u7684\u7c3d\u7ae0\u3002 \u7c3d\u7f72\u7684\u7248\u672c\u8207\u4e0b\u8f09\u7684\u7248\u672c\u4e0d\u7b26\u5408\u3002" },
        { "launch.error.badversionresponse", "\u5b58\u53d6\u8cc7\u6e90 {0} - {1} \u6642\u4f3a\u670d\u5668\u56de\u61c9\u932f\u8aa4\u7248\u672c" },
        { "launch.error.canceledloadingresource", "\u4f7f\u7528\u8005\u5df2\u53d6\u6d88\u8cc7\u6e90 {0} \u7684\u8f09\u5165" },
        { "launch.error.category.arguments", "\u7121\u6548\u5f15\u6578\u932f\u8aa4" },
        { "launch.error.category.download", "\u4e0b\u8f09\u932f\u8aa4" },
        { "launch.error.category.launchdesc", "\u555f\u52d5\u6a94\u6848\u932f\u8aa4" },
        { "launch.error.category.memory", "\u8a18\u61b6\u9ad4\u4e0d\u8db3\u932f\u8aa4" },
        { "launch.error.category.security", "\u5b89\u5168\u932f\u8aa4" },
        { "launch.error.category.config", "\u7cfb\u7d71\u914d\u7f6e" },
        { "launch.error.category.unexpected", "\u975e\u9810\u671f\u7684\u932f\u8aa4" },
        { "launch.error.couldnotloadarg", "\u7121\u6cd5\u8f09\u5165\u6307\u5b9a\u7684\u6a94\u6848/URL\uff1a{0}" },
        { "launch.error.errorcoderesponse-known", "\u5b58\u53d6\u8cc7\u6e90 {0} \u6642\u5f9e\u4f3a\u670d\u5668\u50b3\u56de\u932f\u8aa4\u78bc {1} ({2})" },
        { "launch.error.errorcoderesponse-unknown", "\u5b58\u53d6\u8cc7\u6e90 {0} \u6642\u5f9e\u4f3a\u670d\u5668\u50b3\u56de\u932f\u8aa4\u78bc 99\uff08\u672a\u77e5\u932f\u8aa4\uff09" },
        { "launch.error.failedexec", "\u7121\u6cd5\u555f\u52d5 Java \u57f7\u884c\u968e\u6bb5\u74b0\u5883\u7248\u672c {0}" },
        { "launch.error.failedloadingresource", "\u7121\u6cd5\u8f09\u5165\u8cc7\u6e90\uff1a{0}" },
        { "launch.error.invalidjardiff", "\u7121\u6cd5\u5c07\u589e\u91cf\u66f4\u65b0\u5957\u7528\u5230\u8cc7\u6e90\uff1a{0}" },
        { "launch.error.jarsigning-badsigning", "\u7121\u6cd5\u9a57\u8b49\u8cc7\u6e90\u4e2d\u7684\u7c3d\u7ae0\uff1a {0}" },
        { "launch.error.jarsigning-missingentry", "\u4e0b\u5217\u8cc7\u6e90\u4e2d\u907a\u5931\u7c3d\u7f72\u7684\u767b\u9304\uff1a {0}" },
        { "launch.error.jarsigning-missingentryname", "\u907a\u5931\u7c3d\u7f72\u7684\u767b\u9304\uff1a {0}" },
        { "launch.error.jarsigning-multicerts", "\u7528\u4f86\u7c3d\u7f72\u8cc7\u6e90 {0} \u7684\u8a8d\u8b49\u4e0d\u53ea\u4e00\u500b" },
        { "launch.error.jarsigning-multisigners", "\u8cc7\u6e90 {0} \u4e2d\u767b\u9304\u7684\u7c3d\u540d\u4e0d\u53ea\u4e00\u500b" },
        { "launch.error.jarsigning-unsignedfile", "\u5728\u8cc7\u6e90 {0} \u4e2d\u627e\u5230\u672a\u7c3d\u7f72\u7684\u767b\u9304" },
        { "launch.error.missingfield", "\u555f\u52d5\u6a94\u6848\u7f3a\u5c11\u4e0b\u5217\u5fc5\u8981\u6b04\u4f4d\uff1a {0}" },
        { "launch.error.missingfield-signedjnlp", "\u7c3d\u7f72\u7684\u555f\u52d5\u6a94\u6848\u7f3a\u5c11\u4e0b\u5217\u5fc5\u8981\u6b04\u4f4d\uff1a {0}" },
        { "launch.error.missingjreversion", "\u5728\u6b64\u7cfb\u7d71\u7684\u555f\u52d5\u6a94\u6848\u4e2d\u627e\u4e0d\u5230 JRE \u7248\u672c" },
        { "launch.error.missingversionresponse", "\u5b58\u53d6\u8cc7\u6e90 {0} \u6642\u4f3a\u670d\u5668\u56de\u61c9\u7248\u672c\u6b04\u4f4d\u907a\u5931" },
        { "launch.error.multiplehostsreferences", "\u8cc7\u6e90\u4e2d\u6709\u591a\u91cd\u4e3b\u6a5f\u53c3\u7167" },
        { "launch.error.nativelibviolation", "\u4f7f\u7528\u539f\u59cb\u7a0b\u5f0f\u5eab\u9700\u8981\u7cfb\u7d71\u7684\u7121\u9650\u5236\u5b58\u53d6\u6b0a" },
        { "launch.error.noJre", "\u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u76ee\u524d\u672a\u5728\u5340\u57df\u74b0\u5883\u5b89\u88dd\u7684 JRE \u7248\u672c\u3002 Java Web Start \u7121\u6cd5\u81ea\u52d5\u4e0b\u8f09\u53ca\u5b89\u88dd\u5fc5\u8981\u7684\u7248\u672c\u3002 \u5fc5\u9808\u624b\u52d5\u5b89\u88dd JRE \u7248\u672c\u3002\n\n" },
        { "launch.error.wont.download.jre", "\u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u76ee\u524d\u672a\u5728\u5340\u57df\u74b0\u5883\u5b89\u88dd\u7684 JRE \u7248\u672c (\u7248\u672c {0})\u3002 Java Web Start \u4e0d\u5141\u8a31\u81ea\u52d5\u4e0b\u8f09\u548c\u5b89\u88dd\u5fc5\u8981\u7684\u7248\u672c\u3002 \u5fc5\u9808\u624b\u52d5\u5b89\u88dd JRE \u7248\u672c\u3002" },
        { "launch.error.cant.download.jre", "\u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u76ee\u524d\u672a\u5728\u5340\u57df\u74b0\u5883\u5b89\u88dd\u7684 JRE \u7248\u672c (\u7248\u672c {0})\u3002 Java Web Start \u7121\u6cd5\u81ea\u52d5\u4e0b\u8f09\u548c\u5b89\u88dd\u5fc5\u8981\u7684\u7248\u672c\u3002 \u5fc5\u9808\u624b\u52d5\u5b89\u88dd JRE \u7248\u672c\u3002" },
        { "launch.error.cant.access.system.cache", "\u76ee\u524d\u4f7f\u7528\u8005\u6c92\u6709\u5c0d\u7cfb\u7d71\u5feb\u53d6\u8a18\u61b6\u9ad4\u7684\u5beb\u5165\u5b58\u53d6\u6b0a\u3002" },
        { "launch.error.cant.access.user.cache", "\u76ee\u524d\u4f7f\u7528\u8005\u6c92\u6709\u5c0d\u5feb\u53d6\u8a18\u61b6\u9ad4\u7684\u5beb\u5165\u5b58\u53d6\u6b0a\u3002" },
        { "launch.error.noappresources", "\u672a\u5c0d\u6b64\u5e73\u53f0\u6307\u5b9a\u61c9\u7528\u7a0b\u5f0f\u8cc7\u6e90\u3002 \u8acb\u806f\u7d61\u61c9\u7528\u7a0b\u5f0f\u4f9b\u61c9\u5546\uff0c\u4ee5\u78ba\u5b9a\u6b64\u70ba\u53d7\u652f\u63f4\u7684\u5e73\u53f0\u3002" },
        { "launch.error.nomainclass", "\u5728 {1} \u4e2d\u627e\u4e0d\u5230\u4e3b\u8981\u985e\u5225 {0}" },
        { "launch.error.nomainclassspec", "\u672a\u5c0d\u61c9\u7528\u7a0b\u5f0f\u6307\u5b9a\u4e3b\u8981\u985e\u5225" },
        { "launch.error.nomainjar", "\u672a\u6307\u5b9a\u4e3b JAR \u6a94\u6848\u3002" },
        { "launch.error.nonstaticmainmethod", "main() \u65b9\u6cd5\u5fc5\u9808\u662f\u975c\u614b" },
        { "launch.error.offlinemissingresource", "\u61c9\u7528\u7a0b\u5f0f\u7121\u6cd5\u96e2\u7dda\u57f7\u884c\uff0c\u56e0\u70ba\u4e26\u672a\u5728\u672c\u6a5f\u4e0b\u8f09\u6240\u6709\u5fc5\u8981\u8cc7\u6e90" },
        { "launch.error.parse", "\u7121\u6cd5\u89e3\u6790\u555f\u52d5\u6a94\u6848\u3002 \u5728\u884c {0, number} \u767c\u751f\u932f\u8aa4\u3002" },
        { "launch.error.parse-signedjnlp", "\u7121\u6cd5\u89e3\u6790\u7c3d\u7f72\u7684\u555f\u52d5\u6a94\u6848\u3002 \u5728\u884c {0, number} \u767c\u751f\u932f\u8aa4\u3002" },
        { "launch.error.resourceID", "{0}" },
        { "launch.error.resourceID-version", "({0}, {1})" },
        { "launch.error.singlecertviolation", "JNLP \u6a94\u4e2d\u7684 JAR \u8cc7\u6e90\u672a\u4ee5\u76f8\u540c\u8a8d\u8b49\u7c3d\u7f72" },
        { "launch.error.toomanyargs", "\u63d0\u4f9b\u7684\u5f15\u6578\u592a\u591a\uff1a {0}" },
        { "launch.error.unsignedAccessViolation", "\u672a\u7c3d\u7f72\u7684\u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\u7121\u9650\u5236\u5b58\u53d6\u7cfb\u7d71" },
        { "launch.error.unsignedResource", "\u672a\u7c3d\u7f72\u7684\u8cc7\u6e90\uff1a {0}" },
        { "launch.estimatedTimeLeft", "\u5269\u9918\u9810\u4f30\u6642\u9593\uff1a {0,number,00}:{1,number,00}:{2,number,00}" },
        { "launch.extensiondownload", "\u4e0b\u8f09\u5ef6\u4f38\u63cf\u8ff0\u5143\uff08\u5269\u9918 {0}\uff09" },
        { "launch.extensiondownload-name", "\u4e0b\u8f09 {0} \u63cf\u8ff0\u5143\uff08\u5269\u9918 {1}\uff09" },
        { "launch.initializing", "\u8d77\u59cb\u8a2d\u5b9a..." },
        { "launch.launchApplication", "\u6b63\u5728\u555f\u52d5\u61c9\u7528\u7a0b\u5f0f..." },
        { "launch.launchInstaller", "\u555f\u52d5\u5b89\u88dd\u7a0b\u5f0f..." },
        { "launch.launchingExtensionInstaller", "\u6b63\u5728\u57f7\u884c\u5b89\u88dd\u7a0b\u5f0f\u3002 \u8acb\u7a0d\u5019..." },
        { "launch.loadingNetProgress", "\u8b80\u53d6 {0}" },
        { "launch.loadingNetProgressPercent", "\u8b80\u53d6 {1} \u7684 {0} ({2}%)" },
        { "launch.loadingNetStatus", "\u5f9e {1} \u8f09\u5165 {0}" },
        { "launch.loadingResourceFailed", "\u7121\u6cd5\u8f09\u5165\u8cc7\u6e90" },
        { "launch.loadingResourceFailedSts", "\u8981\u6c42\u7684 {0}" },
        { "launch.patchingStatus", "\u5f9e {1} \u4fee\u6b63 {0}" },
        { "launch.progressScreen", "\u6aa2\u67e5\u6700\u65b0\u7248\u672c..." },
        { "launch.stalledDownload", "\u7b49\u5f85\u8cc7\u6599..." },
        { "launch.validatingProgress", "\u6383\u63cf\u767b\u9304\uff08\u5df2\u5b8c\u6210 {0}%\uff09" },
        { "launch.validatingStatus", "\u5f9e {1} \u9a57\u8b49 {0}" },
        { "launcherrordialog.abort", "\u4e2d\u65b7(A)" },
        { "launcherrordialog.abortMnemonic", "VK_A" },
        { "launcherrordialog.brief.continue", "\u7121\u6cd5\u7e7c\u7e8c\u57f7\u884c" },
        { "launcherrordialog.brief.details", "\u8a73\u7d30\u8cc7\u8a0a" },
        { "launcherrordialog.brief.message", "\u7121\u6cd5\u555f\u52d5\u6307\u5b9a\u7684\u61c9\u7528\u7a0b\u5f0f\u3002" },
	{ "launcherrordialog.import.brief.message", "\u7121\u6cd5\u532f\u5165\u6307\u5b9a\u7684\u61c9\u7528\u7a0b\u5f0f\u3002" },
        { "launcherrordialog.brief.messageKnown", "\u7121\u6cd5\u555f\u52d5 {0}\u3002" },
	{ "launcherrordialog.import.brief.messageKnown", "\u7121\u6cd5\u532f\u5165 {0}\u3002" },
        { "launcherrordialog.brief.ok", "\u78ba\u5b9a" },
        { "launcherrordialog.brief.title", "Java Web Start - {0}" },
        { "launcherrordialog.consoleTab", "\u4e3b\u63a7\u53f0" },
        { "launcherrordialog.errorcategory", "\u985e\u5225\uff1a {0}\n\n" },
        { "launcherrordialog.errorintro", "\u555f\u52d5/\u57f7\u884c\u61c9\u7528\u7a0b\u5f0f\u6642\u767c\u751f\u932f\u8aa4\u3002\n\n" },
	{ "launcherrordialog.import.errorintro", "\u532f\u5165\u61c9\u7528\u7a0b\u5f0f\u6642\u767c\u751f\u932f\u8aa4\u3002\n\n" },
        { "launcherrordialog.errormsg", "{0}" },
        { "launcherrordialog.errortitle", "\u6a19\u984c\uff1a {0}\n" },
        { "launcherrordialog.errorvendor", "\u4f9b\u61c9\u5546\uff1a {0}\n" },
        { "launcherrordialog.exceptionTab", "\u4f8b\u5916" },
        { "launcherrordialog.generalTab", "\u4e00\u822c" },
        { "launcherrordialog.genericerror", "\u975e\u9810\u671f\u7684\u4f8b\u5916\uff1a {0}" },
        { "launcherrordialog.jnlpMainTab", "\u4e3b\u555f\u52d5\u6a94\u6848" },
        { "launcherrordialog.jnlpTab", "\u555f\u52d5\u6a94\u6848" },
        { "launcherrordialog.title", "Java Web Start - {0}" },
        { "launcherrordialog.wrappedExceptionTab", "\u5c01\u88dd\u4f8b\u5916" },

        { "uninstall.failedMessage", "\u7121\u6cd5\u5c0d\u61c9\u7528\u7a0b\u5f0f\u5b8c\u5168\u89e3\u9664\u5b89\u88dd\u3002" },
        { "uninstall.failedMessageTitle", "\u89e3\u9664\u5b89\u88dd" },
        { "install.alreadyInstalled", "\u5df2\u7d93\u6709 {0} \u7684\u6377\u5f91\u3002 \u9084\u8981\u5efa\u7acb\u6377\u5f91\u55ce\uff1f" },
        { "install.alreadyInstalledTitle", "\u5efa\u7acb\u6377\u5f91..." },
        { "install.desktopShortcutName", "{0}" },
        { "install.installFailed", "\u7121\u6cd5\u70ba {0} \u5efa\u7acb\u6377\u5f91\u3002" },
        { "install.installFailedTitle", "\u5efa\u7acb\u6377\u5f91" },
        { "install.startMenuShortcutName", "{0}" },
        { "install.startMenuUninstallShortcutName", "\u89e3\u9664\u5b89\u88dd {0}" },
        { "install.uninstallFailed", "\u7121\u6cd5\u79fb\u9664 {0} \u7684\u6377\u5f91\u3002 \u8acb\u91cd\u8a66\u3002" },
        { "install.uninstallFailedTitle", "\u79fb\u9664\u6377\u5f91" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "\u60a8\u7121\u6cd5\u57f7\u884c\u8a72\u7a0b\u5f0f\uff0c\u56e0\u70ba\u60a8\u7cfb\u7d71\u7684 deployment.config \u6a94\u6848\u8868\u660e\u4f01\u696d\u914d\u7f6e\u6a94\u6848\u662f\u5f37\u5236\u7684\uff0c\u4e26\u4e14\u6240\u9700\u7684 {0} \u4e0d\u53ef\u7528\u3002" },

	// Jnlp Cache Viewer:
	{ "jnlp.viewer.title", "Java \u61c9\u7528\u7a0b\u5f0f\u5feb\u53d6\u6aa2\u8996\u5668" },
	{ "jnlp.viewer.all", "\u5168\u90e8" },
	{ "jnlp.viewer.type", "{0}" },
	{ "jnlp.viewer.totalSize",  "\u7e3d\u7684\u8cc7\u6e90\u5927\u5c0f\uff1a{0}" },
 	{ "jnlp.viewer.emptyCache", "{0} \u5feb\u53d6\u70ba\u7a7a"},
 	{ "jnlp.viewer.noCache", "\u672a\u914d\u7f6e\u7cfb\u7d71\u5feb\u53d6"},

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

	{ "jnlp.viewer.remove.btn", "\u79fb\u9664(R)" },
	{ "jnlp.viewer.remove.1.btn", "\u79fb\u9664\u9078\u53d6\u7684 {0}(R)" },
	{ "jnlp.viewer.remove.2.btn", "\u79fb\u9664\u9078\u53d6\u7684\u767b\u9304(R)" },
	{ "jnlp.viewer.uninstall.btn", "\u89e3\u9664\u5b89\u88dd" },
	{ "jnlp.viewer.launch.offline.btn", "\u96e2\u7dda\u555f\u52d5(L)" },
	{ "jnlp.viewer.launch.online.btn", "\u7dda\u4e0a\u555f\u52d5(N)" },

        { "jnlp.viewer.file.menu", "\u6a94\u6848(F)" },
        { "jnlp.viewer.edit.menu", "\u7de8\u8f2f(E)" },
        { "jnlp.viewer.app.menu", "\u61c9\u7528\u7a0b\u5f0f(A)" },
        { "jnlp.viewer.view.menu", "\u6aa2\u8996(V)" },
        { "jnlp.viewer.help.menu", "\u8f14\u52a9\u8aaa\u660e(H)" },

	{ "jnlp.viewer.cpl.mi", "\u555f\u52d5 Java \u63a7\u5236\u9762\u677f(C)" },
	{ "jnlp.viewer.exit.mi", "\u9000\u51fa(X)" },

	{ "jnlp.viewer.reinstall.mi", "\u91cd\u65b0\u5b89\u88dd(R)..." },
	{ "jnlp.viewer.preferences.mi", "\u500b\u4eba\u559c\u597d(P)..." },

	{ "jnlp.viewer.launch.offline.mi", "\u96e2\u7dda\u555f\u52d5(L)" },
	{ "jnlp.viewer.launch.online.mi", "\u7dda\u4e0a\u555f\u52d5(N)" },
	{ "jnlp.viewer.install.mi", "\u5b89\u88dd\u6377\u5f91(I)" },
	{ "jnlp.viewer.uninstall.mi", "\u89e3\u9664\u5b89\u88dd\u6377\u5f91(U)" },
	{ "jnlp.viewer.remove.0.mi", "\u79fb\u9664" },
	{ "jnlp.viewer.remove.mi", "\u79fb\u9664 {0}(R)" },
	{ "jnlp.viewer.show.mi", "\u986f\u793a JNLP \u63cf\u8ff0\u5143(S)" },
	{ "jnlp.viewer.browse.mi", "\u700f\u89bd\u9996\u9801(B)" },

	{ "jnlp.viewer.view.0.mi", "\u6240\u6709\u985e\u578b(T)" },
	{ "jnlp.viewer.view.1.mi", "\u61c9\u7528\u7a0b\u5f0f(A)" },
	{ "jnlp.viewer.view.2.mi", "Applet(P)" },
	{ "jnlp.viewer.view.3.mi", "\u7a0b\u5f0f\u5eab(L)" },
	{ "jnlp.viewer.view.4.mi", "\u5b89\u88dd\u7a0b\u5f0f(I)" },

	{ "jnlp.viewer.view.0", "\u6240\u6709\u985e\u578b(T)" },
	{ "jnlp.viewer.view.1", "\u61c9\u7528\u7a0b\u5f0f(A)" },
	{ "jnlp.viewer.view.2", "Applet(P)" },
	{ "jnlp.viewer.view.3", "\u7a0b\u5f0f\u5eab(L)" },
	{ "jnlp.viewer.view.4", "\u5b89\u88dd\u7a0b\u5f0f(I)" },

	{ "jnlp.viewer.about.mi", "\u95dc\u65bc(A)" },
	{ "jnlp.viewer.help.java.mi", "J2SE \u9996\u9801(J)" },
	{ "jnlp.viewer.help.jnlp.mi", "JNLP \u9996\u9801(H)" },

        { "jnlp.viewer.app.column", "\u61c9\u7528\u7a0b\u5f0f" },
        { "jnlp.viewer.vendor.column", "\u4f9b\u61c9\u5546" },
        { "jnlp.viewer.type.column", "\u985e\u578b" },
        { "jnlp.viewer.size.column", "\u5927\u5c0f" },
        { "jnlp.viewer.date.column", "\u65e5\u671f" },
        { "jnlp.viewer.status.column", "\u72c0\u614b" },

        { "jnlp.viewer.app.column.tooltip", "\u6b64\u61c9\u7528\u7a0b\u5f0f\u3001Applet \u6216\u5ef6\u4f38\u7684\u5716\u793a\u548c\u6a19\u984c" },
        { "jnlp.viewer.vendor.column.tooltip", "\u90e8\u7f72\u6b64\u9805\u76ee\u7684\u516c\u53f8" },
        { "jnlp.viewer.type.column.tooltip", "\u6b64\u9805\u76ee\u7684\u985e\u578b" },
        { "jnlp.viewer.size.column.tooltip", "\u6b64\u9805\u76ee\u53ca\u5176\u6240\u6709\u8cc7\u6e90\u7684\u5927\u5c0f" },
        { "jnlp.viewer.date.column.tooltip", "\u6700\u5f8c\u4e00\u6b21\u57f7\u884c\u6b64\u61c9\u7528\u7a0b\u5f0f\u3001Applet \u6216\u5b89\u88dd\u7a0b\u5f0f\u7684\u65e5\u671f" },
        { "jnlp.viewer.status.column.tooltip", "\u986f\u793a\u5982\u4f55\u53ca\u53ef\u5426\u555f\u52d5\u6b64\u9805\u76ee\u7684\u5716\u793a" },


        { "jnlp.viewer.application", "\u61c9\u7528\u7a0b\u5f0f" },
        { "jnlp.viewer.applet", "Applet" },
        { "jnlp.viewer.extension", "\u7a0b\u5f0f\u5eab" },
        { "jnlp.viewer.installer", "\u5b89\u88dd\u7a0b\u5f0f" },

	{ "jnlp.viewer.offline.tooltip",
		 "\u53ef\u4ee5\u7dda\u4e0a\u555f\u52d5\u6216\u96e2\u7dda\u555f\u52d5\u6b64 {0}" },
	{ "jnlp.viewer.online.tooltip", "\u53ef\u4ee5\u7dda\u4e0a\u555f\u52d5\u6b64 {0}" },
	{ "jnlp.viewer.norun1.tooltip", 
        	"\u50c5\u53ef\u5f9e\u7db2\u9801\u700f\u89bd\u5668\u555f\u52d5\u6b64 {0}" },
        { "jnlp.viewer.norun2.tooltip", "\u7121\u6cd5\u555f\u52d5\u5ef6\u4f38" },

	{ "jnlp.viewer.show.title", "JNLP \u63cf\u8ff0\u5143\uff1a {0}" },

	{ "jnlp.viewer.removing", "\u6b63\u5728\u79fb\u9664..." },
	{ "jnlp.viewer.launching", "\u6b63\u5728\u555f\u52d5..." },
	{ "jnlp.viewer.browsing", "\u6b63\u5728\u555f\u52d5\u700f\u89bd\u5668..." },
	{ "jnlp.viewer.sorting", "\u6b63\u5728\u6392\u5e8f\u767b\u9304..." },
	{ "jnlp.viewer.searching", "\u6b63\u5728\u641c\u5c0b\u767b\u9304..." },
	{ "jnlp.viewer.installing", "\u6b63\u5728\u5b89\u88dd ..." },

        { "jnlp.viewer.reinstall.title", "\u91cd\u65b0\u5b89\u88dd\u5df2\u79fb\u9664\u7684 JNLP \u61c9\u7528\u7a0b\u5f0f" },
	{ "jnlp.viewer.reinstallBtn", "\u91cd\u65b0\u5b89\u88dd\u9078\u53d6\u7684\u61c9\u7528\u7a0b\u5f0f(R)" },
	{ "jnlp.viewer.reinstallBtn.mnemonic", "VK_R" },
        { "jnlp.viewer.closeBtn", "\u95dc\u9589(C)" },
        { "jnlp.viewer.closeBtn.mnemonic", "VK_C" },

	{ "jnlp.viewer.reinstall.column.title", "\u6a19\u984c\uff1a" },
	{ "jnlp.viewer.reinstall.column.location", "\u4f4d\u7f6e\uff1a" },

	// cache size warning
	{ "jnlp.cache.warning.title", "JNLP \u5feb\u53d6\u8a18\u61b6\u9ad4\u5927\u5c0f\u8b66\u544a" },
	{ "jnlp.cache.warning.message", "\u8b66\u544a\uff1a \n\n"+
		"\u60a8\u5df2\u8d85\u51fa\u5feb\u53d6\u8a18\u61b6\u9ad4\u4e2d\n"+
		"JNLP \u61c9\u7528\u7a0b\u5f0f\u548c\u8cc7\u6e90\u7684\u5efa\u8b70\u78c1\u789f\u7a7a\u9593\u5bb9\u91cf\u3002\n\n"+
		"\u60a8\u76ee\u524d\u4f7f\u7528\u7684\u5bb9\u91cf\u70ba\uff1a {0}\n"+
		"\u5efa\u8b70\u7684\u9650\u5236\u70ba\uff1a {1}\n\n"+
		"\u8acb\u4f7f\u7528 Java \u63a7\u5236\u9762\u677f\u79fb\u9664\u67d0\u4e9b\n"+
		"\u61c9\u7528\u7a0b\u5f0f\u6216\u8cc7\u6e90\uff0c\u6216\u8005\u8a2d\u5b9a\u66f4\u9ad8\u7684\u9650\u5236\u3002" },

        // Control Panel
        { "control.panel.title", "Java \u63a7\u5236\u9762\u677f" },
        { "control.panel.general", "\u4e00\u822c" },
        { "control.panel.security", "\u5b89\u5168" },
        { "control.panel.java", "Java" },
        { "control.panel.update", "\u66f4\u65b0" },
        { "control.panel.advanced", "\u9032\u968e" },

        // Common Strings used in different panels.
        { "common.settings", "\u8a2d\u5b9a" },
        { "common.ok_btn", "\u78ba\u5b9a(O)" },
        { "common.ok_btn.mnemonic", "VK_O" },
        { "common.cancel_btn", "\u53d6\u6d88(C)" },
        { "common.cancel_btn.mnemonic", "VK_C" },
        { "common.apply_btn", "\u5957\u7528(A)" },
        { "common.apply_btn.mnemonic", "VK_A" },
        { "common.add_btn", "\u65b0\u589e(A)" },
        { "common.add_btn.mnemonic", "VK_A" },
        { "common.remove_btn", "\u79fb\u9664(R)" },
        { "common.remove_btn.mnemonic", "VK_R" },

        // Network Settings Dialog
        { "network.settings.dlg.title", "\u7db2\u8def\u8a2d\u5b9a" },
        { "network.settings.dlg.border_title", " \u7db2\u8def Proxy \u8a2d\u5b9a " },
        { "network.settings.dlg.browser_rbtn", "\u4f7f\u7528\u700f\u89bd\u5668\u8a2d\u5b9a(B)" },
        { "browser_rbtn.mnemonic", "VK_B" },
        { "network.settings.dlg.manual_rbtn", "\u4f7f\u7528 Proxy \u4f3a\u670d\u5668(P)" },
        { "manual_rbtn.mnemonic", "VK_P" },
        { "network.settings.dlg.address_lbl", "\u4f4d\u5740\uff1a" },
	{ "network.settings.dlg.port_lbl", "\u9023\u63a5\u57e0\uff1a" },
        { "network.settings.dlg.advanced_btn", "\u9032\u968e(A)..." },
        { "network.settings.dlg.advanced_btn.mnemonic", "VK_A" },
        { "network.settings.dlg.bypass_text", "\u8fd1\u7aef\u4f4d\u5740\u4e0d\u4f7f\u7528 Proxy \u4f3a\u670d\u5668(Y)" },
        { "network.settings.dlg.bypass.mnemonic", "VK_Y" },
        { "network.settings.dlg.autoconfig_rbtn", "\u4f7f\u7528\u81ea\u52d5 Proxy \u914d\u7f6e\u7a0b\u5e8f\u6a94(T)" },
        { "autoconfig_rbtn.mnemonic", "VK_T" },
        { "network.settings.dlg.location_lbl", "\u7a0b\u5e8f\u6a94\u4f4d\u7f6e\uff1a " },
        { "network.settings.dlg.direct_rbtn", "\u76f4\u63a5\u9023\u7dda(D)" },
        { "direct_rbtn.mnemonic", "VK_D" },
        { "network.settings.dlg.browser_text", "\u81ea\u52d5\u914d\u7f6e\u53ef\u80fd\u6703\u7f6e\u63db\u624b\u52d5\u8a2d\u5b9a\u3002 \u82e5\u8981\u78ba\u4fdd\u4f7f\u7528\u624b\u52d5\u8a2d\u5b9a\uff0c\u8acb\u505c\u7528\u81ea\u52d5\u914d\u7f6e\u3002" },
        { "network.settings.dlg.proxy_text", "\u8986\u5beb\u700f\u89bd\u5668 Proxy \u8a2d\u5b9a\u3002" },
        { "network.settings.dlg.auto_text", "\u5728\u6307\u5b9a\u4f4d\u7f6e\u4f7f\u7528\u81ea\u52d5 Proxy \u914d\u7f6e\u7a0b\u5e8f\u6a94\u3002" },
        { "network.settings.dlg.none_text", "\u4f7f\u7528\u76f4\u63a5\u9023\u7dda\u3002" },

        // Advanced Network Settings Dialog
        { "advanced.network.dlg.title", "\u9032\u968e\u7db2\u8def\u8a2d\u5b9a" },
        { "advanced.network.dlg.servers", " \u4f3a\u670d\u5668 " },
        { "advanced.network.dlg.type", "\u985e\u578b" },
        { "advanced.network.dlg.http", "HTTP\uff1a" },
        { "advanced.network.dlg.secure", "Secure\uff1a" },
        { "advanced.network.dlg.ftp", "FTP\uff1a" },
        { "advanced.network.dlg.socks", "Socks\uff1a" },
        { "advanced.network.dlg.proxy_address", "Proxy \u4f4d\u5740" },
	{ "advanced.network.dlg.port", "\u9023\u63a5\u57e0" },
        { "advanced.network.dlg.same_proxy", " \u6240\u6709\u5354\u5b9a\u4f7f\u7528\u76f8\u540c\u7684 Proxy \u4f3a\u670d\u5668(U)" },
        { "advanced.network.dlg.same_proxy.mnemonic", "VK_U" },
        { "advanced.network.dlg.exceptions", " \u4f8b\u5916 " },
        { "advanced.network.dlg.no_proxy", " \u4e0b\u5217\u958b\u982d\u7684\u4f4d\u5740\u4e0d\u4f7f\u7528 Proxy \u4f3a\u670d\u5668" },
        { "advanced.network.dlg.no_proxy_note", " \u4f7f\u7528\u5206\u865f (;) \u5206\u9694\u4f4d\u5740\u3002" },

        // DeleteFilesDialog
        { "delete.files.dlg.title", "\u522a\u9664\u66ab\u5b58\u6a94\u6848" },
        { "delete.files.dlg.temp_files", "\u522a\u9664\u4ee5\u4e0b\u66ab\u5b58\u6a94\u6848\u55ce\uff1f" },
        { "delete.files.dlg.applets", "\u4e0b\u8f09\u7684 Applet" },
        { "delete.files.dlg.applications", "\u4e0b\u8f09\u7684\u61c9\u7528\u7a0b\u5f0f" },
        { "delete.files.dlg.other", "\u5176\u4ed6\u6a94\u6848" },

	// General
	{ "general.cache.border.text", " \u66ab\u5b58\u7db2\u969b\u7db2\u8def\u6a94\u6848 " },
	{ "general.cache.delete.text", "\u522a\u9664\u6a94\u6848(D)..." },
        { "general.cache.delete.text.mnemonic", "VK_D" },
	{ "general.cache.settings.text", "\u8a2d\u5b9a(S)..." },
        { "general.cache.settings.text.mnemonic", "VK_S" },
	{ "general.cache.desc.text", "\u60a8\u5728 Java \u61c9\u7528\u7a0b\u5f0f\u4e2d\u4f7f\u7528\u7684\u6a94\u6848\u5132\u5b58\u5728\u7279\u6b8a\u8cc7\u6599\u593e\u4e2d\uff0c\u4ee5\u4fbf\u7a0d\u5f8c\u5feb\u901f\u57f7\u884c\u3002 \u50c5\u9032\u968e\u4f7f\u7528\u8005\u624d\u9700\u522a\u9664\u6a94\u6848\u6216\u4fee\u6539\u9019\u4e9b\u8a2d\u5b9a\u3002" },
	{ "general.network.border.text", " \u7db2\u8def\u8a2d\u5b9a " },
	{ "general.network.settings.text", "\u7db2\u8def\u8a2d\u5b9a(N)..." },
        { "general.network.settings.text.mnemonic", "VK_N" },
	{ "general.network.desc.text", "\u7db2\u8def\u8a2d\u5b9a\u5728\u9032\u884c\u7db2\u969b\u7db2\u8def\u9023\u7dda\u6642\u4f7f\u7528\u3002 \u4f9d\u9810\u8a2d\uff0cJava \u5c07\u4f7f\u7528\u7db2\u9801\u700f\u89bd\u5668\u4e2d\u7684\u7db2\u8def\u8a2d\u5b9a\u3002 \u50c5\u9032\u968e\u4f7f\u7528\u8005\u624d\u9700\u4fee\u6539\u9019\u4e9b\u8a2d\u5b9a\u3002" },
        { "general.about.border", "\u95dc\u65bc" },
        { "general.about.text", "\u6aa2\u8996\u6709\u95dc Java \u63a7\u5236\u9762\u677f\u7684\u7248\u672c\u8cc7\u8a0a\u3002" },
        { "general.about.btn", "\u95dc\u65bc(B)..." },
        { "general.about.btn.mnemonic", "VK_B" },


	// Security
	{ "security.certificates.border.text", " \u8b49\u66f8 " },
	{ "security.certificates.button.text", "\u8b49\u66f8(E)..." },
        { "security.certificates.button.mnemonic", "VK_E" },
	{ "security.certificates.desc.text", "\u4f7f\u7528\u8b49\u66f8\u53ef\u4ee5\u660e\u78ba\u8b58\u5225\u500b\u4eba\u3001\u8a8d\u8b49\u3001\u6b0a\u9650\u4ee5\u53ca\u767c\u884c\u4eba\u3002" },
	{ "security.policies.border.text", " \u7b56\u7565 " },
	{ "security.policies.advanced.text", "\u9032\u968e(D)..." },
        { "security.policies.advanced.mnemonic", "VK_D" },
	{ "security.policies.desc.text", "\u4f7f\u7528\u5b89\u5168\u7b56\u7565\u53ef\u4ee5\u63a7\u5236\u61c9\u7528\u7a0b\u5f0f\u548c Applet \u5468\u570d\u7684\u5b89\u5168\u969c\u7919\u3002" },

	// Update
	{ "update.notify.border.text", " \u66f4\u65b0\u901a\u77e5 " }, // this one is not currently used.  See update panel!!!
	{ "update.updatenow.button.text", "\u7acb\u5373\u66f4\u65b0(U)" },
	{ "update.updatenow.button.mnemonic", "VK_U" },
	{ "update.advanced.button.text", "\u9032\u968e(D)..." },
	{ "update.advanced.button.mnemonic", "VK_D" },
	{ "update.desc.text", "Java Update \u6a5f\u5236\u78ba\u4fdd\u60a8\u64c1\u6709\u6700\u65b0\u7248\u672c\u7684 Java \u5e73\u53f0\u3002  \u4ee5\u4e0b\u9078\u9805\u53ef\u8b93\u60a8\u63a7\u5236\u5982\u4f55\u53d6\u5f97\u548c\u5957\u7528\u66f4\u65b0\u3002" },
        { "update.notify.text", "\u901a\u77e5\u6211\uff1a" },
        { "update.notify_install.text", "\u5b89\u88dd\u4e4b\u524d" },
        { "update.notify_download.text", "\u4e0b\u8f09\u4e4b\u524d\u548c\u5b89\u88dd\u4e4b\u524d" },
        { "update.autoupdate.text", "\u81ea\u52d5\u6aa2\u67e5\u66f4\u65b0" },
        { "update.advanced_title.text", "\u81ea\u52d5\u66f4\u65b0\u9032\u968e\u8a2d\u5b9a" },
        { "update.advanced_title1.text", "\u9078\u53d6\u9032\u884c\u6383\u63cf\u7684\u9593\u9694\u548c\u6642\u9593\u3002" },
        { "update.advanced_title2.text", "\u983b\u7387" },
        { "update.advanced_title3.text", "\u6642\u9593" },
        { "update.advanced_desc1.text", "\u5728\u6bcf\u5929 {0} \u6642\u57f7\u884c\u6383\u63cf" },
        { "update.advanced_desc2.text", "\u5728\u6bcf {0} \u7684 {1} \u6642\u57f7\u884c\u6383\u63cf" },
        { "update.advanced_desc3.text", "\u5728\u6bcf\u6708 {0} \u65e5\u7684 {1} \u6642\u57f7\u884c\u6383\u63cf" },
        { "update.check_daily.text", "\u6bcf\u5929" },
        { "update.check_weekly.text", "\u6bcf\u9031" },
        { "update.check_monthly.text", "\u6bcf\u6708" },
        { "update.check_date.text", "\u65e5\u671f\uff1a" },
        { "update.check_day.text", "\u6bcf\uff1a" },
        { "update.check_time.text", "\u5728\uff1a" },
        { "update.lastrun.text", "\u6700\u5f8c\u4e00\u6b21\u57f7\u884c Java Update \u7684\u6642\u9593\u662f {1} \u7684 {0}\u3002" },
        { "update.desc_autooff.text", "\u6309\u4e00\u4e0b\u4e0b\u9762\u7684\u300c\u7acb\u5373\u66f4\u65b0\u300d\u6309\u9215\u53ef\u4ee5\u6aa2\u67e5\u66f4\u65b0\u3002 \u5982\u679c\u66f4\u65b0\u53ef\u7528\uff0c\u7cfb\u7d71\u5217\u4e2d\u6703\u51fa\u73fe\u4e00\u500b\u5716\u793a\u3002 \u5c07\u6e38\u6a19\u79fb\u81f3\u5716\u793a\u4e0a\u53ef\u4ee5\u67e5\u770b\u66f4\u65b0\u7684\u72c0\u614b\u3002" },
        { "update.desc_check_daily.text", "\u6bcf\u5929 {0} \u6642\uff0cJava Update \u5c07\u6aa2\u67e5\u66f4\u65b0\u3002 " },
        { "update.desc_check_weekly.text", "\u6bcf {0} \u7684 {1} \u6642\uff0cJava Update \u5c07\u6aa2\u67e5\u66f4\u65b0\u3002 " },
        { "update.desc_check_monthly.text", "\u5728\u6bcf\u6708 {0} \u65e5\u7684 {1} \u6642\uff0cJava Update \u5c07\u6aa2\u67e5\u66f4\u65b0\u3002 " },
        { "update.desc_systrayicon.text", "\u5982\u679c\u66f4\u65b0\u53ef\u7528\uff0c\u7cfb\u7d71\u5217\u4e2d\u6703\u51fa\u73fe\u4e00\u500b\u5716\u793a\u3002 \u5c07\u6e38\u6a19\u79fb\u81f3\u5716\u793a\u4e0a\u53ef\u4ee5\u67e5\u770b\u66f4\u65b0\u7684\u72c0\u614b\u3002 " },
        { "update.desc_notify_install.text", "\u7cfb\u7d71\u6703\u5728\u5b89\u88dd\u66f4\u65b0\u524d\u901a\u77e5\u60a8\u3002" },
        { "update.desc_notify_download.text", "\u7cfb\u7d71\u6703\u5728\u4e0b\u8f09\u548c\u5b89\u88dd\u66f4\u65b0\u524d\u901a\u77e5\u60a8\u3002" },
	{ "update.launchbrowser.error.text", "\u7121\u6cd5\u555f\u52d5 Java Update Checker\u3002  \u82e5\u8981\u53d6\u5f97\u6700\u65b0\u7684 Java Update\uff0c\u8acb\u9020\u8a2a http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "\u932f\u8aa4 - \u66f4\u65b0" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "\u522a\u9664\u6a94\u6848(D)..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_D" },
        { "cache.settings.dialog.view_jws_btn", "\u6aa2\u8996\u61c9\u7528\u7a0b\u5f0f(V)..." },
        { "cache.settings.dialog.view_jws_btn.mnemonic", "VK_V" },
        { "cache.settings.dialog.view_jpi_btn", "\u6aa2\u8996 Applet(A)..." },
        { "cache.settings.dialog.view_jpi_btn.mnemonic", "VK_A" },
        { "cache.settings.dialog.chooser_title", "\u66ab\u5b58\u6a94\u6848\u4f4d\u7f6e" },
        { "cache.settings.dialog.select", "\u9078\u53d6(S)" },
        { "cache.settings.dialog.select_tooltip", "\u4f7f\u7528\u9078\u53d6\u7684\u4f4d\u7f6e" },
        { "cache.settings.dialog.select_mnemonic", "S" },
        { "cache.settings.dialog.title", "\u66ab\u5b58\u6a94\u6848\u8a2d\u5b9a" },
        { "cache.settings.dialog.cache_location", "\u4f4d\u7f6e\uff1a" },
        { "cache.settings.dialog.change_btn", "\u8b8a\u66f4(H)..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_H" },
        { "cache.settings.dialog.disk_space", "\u66ab\u5b58\u78c1\u789f\u7a7a\u9593\u5bb9\u91cf" },
        { "cache.settings.dialog.unlimited_btn", "\u4e0d\u9650" },
        { "cache.settings.dialog.max_btn", "\u6700\u591a" },
        { "cache.settings.dialog.compression", "Jar \u58d3\u7e2e\uff1a" },
        { "cache.settings.dialog.none", "\u7121" },
        { "cache.settings.dialog.high", "\u9ad8" },

	// JNLP File/MIME association dialog strings:
	{ "javaws.association.dialog.title", "JNLP \u6a94\u6848/MIME \u95dc\u806f" },
	{ "javaws.association.dialog.exist.command", "\u5df2\u7d93\u5b58\u5728\uff1a\n{0}"},
	{ "javaws.association.dialog.exist", "\u5df2\u7d93\u5b58\u5728\u3002"  },
	{ "javaws.association.dialog.askReplace", "\n\u60a8\u78ba\u5be6\u8981\u4f7f\u7528 {0} \u4f86\u8655\u7406\u5b83\u55ce\uff1f"},
	{ "javaws.association.dialog.ext", "\u526f\u6a94\u540d\uff1a {0}" },
        { "javaws.association.dialog.mime", "MIME \u985e\u578b\uff1a {0}" },
        { "javaws.association.dialog.ask", "\u60a8\u662f\u5426\u8981\u4f7f\u7528 {0} \u4f86\u8655\u7406\uff1a" },
        { "javaws.association.dialog.existAsk", "\u8b66\u544a\uff01 \u8207\u4ee5\u4e0b\u5167\u5bb9\u95dc\u806f\uff1a"},

        // Advanced panel strings:
        { "deployment.console.startup.mode", "Java \u4e3b\u63a7\u53f0" },
        { "deployment.console.startup.mode.SHOW", "\u986f\u793a\u4e3b\u63a7\u53f0" },
        { "deployment.console.startup.mode.SHOW.tooltip", "<html>" +
                                                          "\u6700\u5927\u5316\u555f\u52d5 Java \u4e3b\u63a7\u53f0" +
                                                          "</html>" },
        { "deployment.console.startup.mode.HIDE", "\u96b1\u85cf\u4e3b\u63a7\u53f0" },
        { "deployment.console.startup.mode.HIDE.tooltip", "<html>" +
                                                          "\u6700\u5c0f\u5316\u555f\u52d5 Java \u4e3b\u63a7\u53f0" +
                                                          "</html>" },
        { "deployment.console.startup.mode.DISABLE", "\u4e0d\u555f\u52d5\u4e3b\u63a7\u53f0" },
        { "deployment.console.startup.mode.DISABLE.tooltip", "<html>" +
                                                             "Java \u4e3b\u63a7\u53f0\u4e0d\u6703\u555f\u52d5" +
                                                             "</html>" },
        { "deployment.trace", "\u555f\u7528\u8ffd\u8e64" },
        { "deployment.trace.tooltip", "<html>" +
                                      "\u5efa\u7acb\u8ffd\u8e64\u6a94\u6848\u4ee5" +
                                      "<br>\u7528\u65bc\u9664\u932f" +
                                      "</html>" },
        { "deployment.log", "\u555f\u7528\u8a18\u9304" },
        { "deployment.log.tooltip", "<html>" +
                                    "\u5efa\u7acb\u65e5\u8a8c\u6a94\u4ee5" +
                                    "<br>\u8a18\u9304\u932f\u8aa4" +
                                    "</html>" },
        { "deployment.control.panel.log", "\u767b\u5165\u63a7\u5236\u9762\u677f" },
        { "deployment.javapi.lifecycle.exception", "\u986f\u793a Applet \u751f\u547d\u9031\u671f\u7570\u5e38" },
        { "deployment.javapi.lifecycle.exception.tooltip", "<html>" +
                                          "\u8f09\u5165 Applet \u767c\u751f\u932f\u8aa4\u6642"+
                                          "<br>\u986f\u793a\u8aaa\u660e\u7570\u5e38\u7684\u5c0d\u8a71\u65b9\u584a"+
                                          "<html>" },
        { "deployment.browser.vm.iexplorer", "Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
                                                     "\u5728 Internet Explorer \u700f\u89bd\u5668\u4e2d\u540c\u6642" +
                                                     "<br>\u4f7f\u7528 Sun Java \u548c APPLET \u6a19\u7c64" +
                                                     "</html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla \u548c Netscape" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
                                                   "\u5728 Mozilla \u6216 Netscape \u700f\u89bd\u5668\u4e2d\u540c\u6642\u4f7f\u7528" +
                                                   "<br>Sun Java \u548c APPLET \u6a19\u7c64" +
                                                   "</html>" },
        { "deployment.console.debugging", "\u9664\u932f" },
	{ "deployment.browsers.applet.tag", "<APPLET> \u6a19\u7c64\u652f\u63f4" },
        { "deployment.javaws.shortcut", "\u5efa\u7acb\u6377\u5f91" },
        { "deployment.javaws.shortcut.ALWAYS", "\u7e3d\u662f\u5141\u8a31" },
        { "deployment.javaws.shortcut.ALWAYS.tooltip", "<html>" +
                                                       "\u7e3d\u662f\u5efa\u7acb\u6377\u5f91" +
                                                       "</html>" },
        { "deployment.javaws.shortcut.NEVER" , "\u6c38\u4e0d\u5141\u8a31" },
        { "deployment.javaws.shortcut.NEVER.tooltip", "<html>" +
                                                      "\u4e0d\u5efa\u7acb\u6377\u5f91" +
                                                      "</html>" },
        { "deployment.javaws.shortcut.ASK_USER", "\u63d0\u793a\u4f7f\u7528\u8005" },
        { "deployment.javaws.shortcut.ASK_USER.tooltip", "<html>" +
                                                         "\u8a62\u554f\u4f7f\u7528\u8005\u662f\u5426" +
                                                         "<br>\u61c9\u5efa\u7acb\u6377\u5f91" +
                                                         "</html>" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED", "\u5982\u679c\u63d0\u793a\uff0c\u5247\u7e3d\u662f\u5141\u8a31" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED.tooltip", "<html>" +
                                                     "\u5982\u679c JNLP \u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\uff0c" +
                                                     "<br>\u5247\u7e3d\u662f\u5efa\u7acb\u6377\u5f91" +
                                                     "</html>" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED", "\u5982\u679c\u63d0\u793a\uff0c\u5247\u63d0\u793a\u4f7f\u7528\u8005" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED.tooltip", "<html>" +
                                                     "\u5982\u679c JNLP \u61c9\u7528\u7a0b\u5f0f\u8981\u6c42\uff0c" +
                                                     "<br>\u5247\u8a62\u554f\u4f7f\u7528\u8005\u662f\u5426\u61c9\u5efa\u7acb" +
                                                     "<br>\u6377\u5f91" +
                                                     "</html>" },
	{ "deployment.javaws.associations.NEVER", "\u6c38\u4e0d\u5141\u8a31" },
        { "deployment.javaws.associations.NEVER.tooltip", "<html>" +
                                                  "\u6c38\u4e0d\u5efa\u7acb\u526f\u6a94\u540d/MIME" +
                                                  "<br>\u95dc\u806f" +
                                                  "</html>" },
        { "deployment.javaws.associations.ASK_USER", "\u63d0\u793a\u4f7f\u7528\u8005" },
        { "deployment.javaws.associations.ASK_USER.tooltip", "<html>" +
                                                  "\u5728\u5efa\u7acb\u526f\u6a94\u540d/MIME \u95dc\u806f\u4e4b\u524d" +
                                                  "<br>\u8a62\u554f\u4f7f\u7528\u8005" +
                                                  "</html>" },
        { "deployment.javaws.associations.REPLACE_ASK", "\u63d0\u793a\u4f7f\u7528\u8005\u4ee5\u53d6\u4ee3" },
        { "deployment.javaws.associations.REPLACE_ASK.tooltip", "<html>" +
                                                  "\u50c5\u5728\u53d6\u4ee3\u73fe\u6709\u526f\u6a94\u540d/MIME" +
                                                  "<br>\u95dc\u806f\u6642\u8a62\u554f" +
                                                  "<br>\u4f7f\u7528\u8005" +
                                                  "</html>" },
        { "deployment.javaws.associations.NEW_ONLY", "\u5982\u679c\u95dc\u806f\u70ba\u65b0\u5247\u5141\u8a31" },
        { "deployment.javaws.associations.NEW_ONLY.tooltip", "<html>" +
                                                  "\u7e3d\u662f\u50c5\u5efa\u7acb\u65b0" +
                                                  "<br>\u526f\u6a94\u540d/MIME \u95dc\u806f" +
                                                  "</html>" },
        { "deployment.javaws.associations", "JNLP \u6a94\u6848/MIME \u95dc\u806f" },
        { "deployment.security.settings", "\u5b89\u5168" },
        { "deployment.security.askgrantdialog.show", "\u5141\u8a31\u4f7f\u7528\u8005\u5c0d\u5df2\u7c3d\u7f72\u7684\u5167\u5bb9\u6388\u8207\u8a31\u53ef\u6b0a" },
        { "deployment.security.askgrantdialog.notinca", "\u5141\u8a31\u4f7f\u7528\u8005\u5c0d\u4f86\u81ea\u4e0d\u53ef\u4fe1\u7684\u6388\u6b0a\u55ae\u4f4d\u7684\u5167\u5bb9\u6388\u8207\u8a31\u53ef\u6b0a" },

	{ "deployment.security.browser.keystore.use", "\u4f7f\u7528\u700f\u89bd\u5668\u5bc6\u9470\u5132\u5b58\u5eab\u4e2d\u7684\u8b49\u66f8\u548c\u5bc6\u9470" },
	{ "deployment.security.notinca.warning", "\u5982\u679c\u7121\u6cd5\u9a57\u8b49\u8b49\u66f8\u6388\u6b0a\u55ae\u4f4d\uff0c\u5247\u767c\u51fa\u8b66\u544a" },
        { "deployment.security.expired.warning", "\u5982\u679c\u8b49\u66f8\u904e\u671f\u6216\u5c1a\u672a\u751f\u6548\uff0c\u5247\u767c\u51fa\u8b66\u544a" },
        { "deployment.security.jsse.hostmismatch.warning", "\u5982\u679c\u7db2\u7ad9\u8b49\u66f8\u8207\u4e3b\u6a5f\u540d\u7a31\u4e0d\u76f8\u7b26\uff0c\u5247\u767c\u51fa\u8b66\u544a" },
        { "deployment.security.sandbox.awtwarningwindow", "\u986f\u793a\u6c99\u7bb1\u8b66\u544a\u6a19\u984c" },
        { "deployment.security.sandbox.jnlp.enhanced", "\u5141\u8a31\u4f7f\u7528\u8005\u63a5\u53d7 JNLP \u5b89\u5168\u8981\u6c42" },
        { "deploy.advanced.browse.title", "\u9078\u64c7\u555f\u52d5\u9810\u8a2d\u700f\u89bd\u5668\u7684\u6a94\u6848" },
        { "deploy.advanced.browse.select", "\u9078\u53d6" },
        { "deploy.advanced.browse.select_tooltip", "\u4f7f\u7528\u9078\u53d6\u7684\u6a94\u6848\u555f\u52d5\u700f\u89bd\u5668" },
        { "deploy.advanced.browse.select_mnemonic", "S" },
        { "deploy.advanced.browse.browse_btn", "\u700f\u89bd(B)..." },
        { "deploy.advanced.browse.browse_btn.mnemonic", "VK_B" },
        { "deployment.browser.default", "\u555f\u52d5\u9810\u8a2d\u700f\u89bd\u5668\u7684\u6307\u4ee4" },
        { "deployment.misc.label", "\u5176\u4ed6" },
        { "deployment.system.tray.icon", "\u5c07 Java \u5716\u793a\u653e\u7f6e\u5728\u7cfb\u7d71\u5217\u4e2d" },
        { "deployment.system.tray.icon.tooltip", "<html>" +
                                                 "\u9078\u53d6\u6b64\u9078\u9805\u5f8c\uff0c\u5728\u700f\u89bd\u5668\u4e2d\u57f7\u884c" +
                                                 "<br>Java \u6642\uff0c\u5247\u6703\u5728\u7cfb\u7d71\u5217\u4e2d\u986f\u793a" +
                                                 "<br>Java \u5496\u5561\u676f\u5716\u793a" +
                                                 "</html>" },

        //PluginJresDialog strings:
        { "jpi.jres.dialog.title", "Java \u57f7\u884c\u968e\u6bb5\u8a2d\u5b9a" },
        { "jpi.jres.dialog.border", " Java \u57f7\u884c\u968e\u6bb5\u7248\u672c " },
        { "jpi.jres.dialog.column1", "\u7522\u54c1\u540d\u7a31" },
        { "jpi.jres.dialog.column2", "\u7248\u672c" },
        { "jpi.jres.dialog.column3", "\u4f4d\u7f6e" },
        { "jpi.jres.dialog.column4", "Java \u57f7\u884c\u968e\u6bb5\u53c3\u6578" },
        { "jpi.jdk.string", "JDK" },
        { "jpi.jre.string", "JRE" },
        { "jpi.jres.dialog.product.tooltip", "\u70ba\u7522\u54c1\u540d\u7a31\u9078\u64c7 JRE \u6216 JDK" },

        // AboutDialog strings:
        { "about.dialog.title", "\u95dc\u65bc Java" },

        // JavaPanel strings:
        { "java.panel.plugin.border", " Java Applet \u57f7\u884c\u968e\u6bb5\u8a2d\u5b9a " }, 
        { "java.panel.plugin.text", "\u5728\u700f\u89bd\u5668\u4e2d\u57f7\u884c Applet \u6642\u4f7f\u7528\u57f7\u884c\u968e\u6bb5\u8a2d\u5b9a\u3002" },
        { "java.panel.jpi_view_btn", "\u6aa2\u8996(V)..." },
        { "java.panel.javaws_view_btn", "\u6aa2\u8996(I)..." },
        { "java.panel.jpi_view_btn.mnemonic", "VK_V" },
        { "java.panel.javaws_view_btn.mnemonic", "VK_I" },
        { "java.panel.javaws.border", " Java \u61c9\u7528\u7a0b\u5f0f\u57f7\u884c\u968e\u6bb5\u8a2d\u5b9a "},
        { "java.panel.javaws.text", "\u4f7f\u7528 Java \u7db2\u8def\u555f\u52d5\u5354\u5b9a (JNLP) \u555f\u52d5 Java \u61c9\u7528\u7a0b\u5f0f\u6216 Applet \u6642\uff0c\u4f7f\u7528\u57f7\u884c\u968e\u6bb5\u8a2d\u5b9a\u3002" },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "<html><b>\u5b58\u5728\u8f03\u65b0\u7684 Java \u57f7\u884c\u968e\u6bb5</b></html>Internet Explorer \u5df2\u64c1\u6709\u8f03\u65b0\u7248\u672c\u7684 Java \u57f7\u884c\u968e\u6bb5\u3002 \u60a8\u8981\u53d6\u4ee3\u5b83\u55ce\uff1f\n" },
        { "browser.settings.success.caption", "\u6210\u529f - \u700f\u89bd\u5668" },
        { "browser.settings.success.text", "<html><b>\u5df2\u8b8a\u66f4\u700f\u89bd\u5668\u8a2d\u5b9a</b></html>\u91cd\u65b0\u555f\u52d5\u700f\u89bd\u5668\u5f8c\uff0c\u8b8a\u66f4\u5c07\u751f\u6548\u3002\n" },
        { "browser.settings.fail.caption", "\u8b66\u544a - \u700f\u89bd\u5668" },
        { "browser.settings.fail.moz.text", "<html><b>\u7121\u6cd5\u8b8a\u66f4\u700f\u89bd\u5668\u8a2d\u5b9a</b></html>"
                                        + "\u8acb\u6aa2\u67e5 Mozilla \u6216 Netscape \u5df2\u6b63\u78ba\u5b89\u88dd\u5728\u7cfb\u7d71\u4e0a\u548c/\u6216 "
                                        + "\u60a8\u64c1\u6709 "
                                        + "\u8db3\u5920\u7684\u8a31\u53ef\u6b0a\uff0c\u53ef\u4ee5\u8b8a\u66f4\u7cfb\u7d71\u8a2d\u5b9a\u3002\n" },
        { "browser.settings.fail.ie.text", "<html><b>\u7121\u6cd5\u8b8a\u66f4\u700f\u89bd\u5668\u8a2d\u5b9a</b></html>\u8acb\u6aa2\u67e5\u60a8\u662f\u5426\u64c1\u6709 "
					+ "\u8db3\u5920\u7684\u8a31\u53ef\u6b0a\uff0c\u53ef\u4ee5\u8b8a\u66f4\u7cfb\u7d71\u8a2d\u5b9a\u3002\n" },


        // Tool tip strings.
        { "cpl.ok_btn.tooltip", "<html>" +
                                "\u95dc\u9589 Java \u63a7\u5236\u9762\u677f\u4e26\u5132\u5b58" +
                                "<br>\u60a8\u6240\u4f5c\u7684\u6240\u6709\u8b8a\u66f4" +
                                "</html>" },
        { "cpl.apply_btn.tooltip",  "<html>" +
                                    "\u5132\u5b58\u60a8\u6240\u4f5c\u7684\u6240\u6709\u8b8a\u66f4" +
                                    "<br>\u800c\u4e0d\u95dc\u9589 Java \u63a7\u5236\u9762\u677f" +
                                    "</html>" },
        { "cpl.cancel_btn.tooltip", "<html>" +
                                    "\u95dc\u9589 Java \u63a7\u5236\u9762\u677f\u800c\u4e0d" +
                                    "<br>\u5132\u5b58\u4efb\u4f55\u8b8a\u66f4" +
                                    "</html>" },

        {"network.settings.btn.tooltip", "<html>"+
                                         "\u4fee\u6539\u7db2\u969b\u7db2\u8def\u9023\u7dda\u8a2d\u5b9a" +
                                         "</html>"},

        {"temp.files.settings.btn.tooltip", "<html>"+
                                            "\u4fee\u6539\u66ab\u5b58\u6a94\u6848\u7684\u8a2d\u5b9a" +
                                            "</html>"},

        {"temp.files.delete.btn.tooltip", "<html>" +  // body bgcolor=\"#FFFFCC\">"+
                                          "\u522a\u9664\u66ab\u5b58 Java \u6a94\u6848" +
                                          "</html>"},

        {"delete.files.dlg.applets.tooltip", "<html>" +
                                          "\u6838\u53d6\u6b64\u9078\u9805\u53ef\u4ee5\u522a\u9664\u7531 Java" +
                                          "<br>Applet \u5efa\u7acb\u7684\u6240\u6709\u66ab\u5b58\u6a94\u6848" +
                                          "</html>" },

        {"delete.files.dlg.applications.tooltip", "<html>" +
                                          "\u6838\u53d6\u6b64\u9078\u9805\u53ef\u4ee5\u522a\u9664\u7531 Java Web Start" +
					  "<br>\u61c9\u7528\u7a0b\u5f0f\u5efa\u7acb\u7684\u6240\u6709\u66ab\u5b58\u6a94\u6848" +
                                          "</html>" },

        {"delete.files.dlg.other.tooltip", "<html>" +
                                          "\u6838\u53d6\u6b64\u9078\u9805\u53ef\u4ee5\u522a\u9664\u7531 Java \u5efa\u7acb\u7684" +
                                          "<br>\u6240\u6709\u5176\u4ed6\u66ab\u5b58\u6a94\u6848" +
                                          "</html>" },

        {"delete.files.dlg.temp_files.tooltip", "<html>" +
                                          "Java \u61c9\u7528\u7a0b\u5f0f\u53ef\u5728\u60a8\u7684\u96fb\u8166\u4e0a" +
                                          "<br>\u5132\u5b58\u67d0\u4e9b\u66ab\u5b58\u6a94\u6848\u3002  \u60a8\u53ef\u4ee5\u5b89\u5168\u5730" +
                                          "<br>\u522a\u9664\u9019\u4e9b\u6a94\u6848\u3002" +
                                          "<br>" +
                                          "<p>\u522a\u9664\u66ab\u5b58\u6a94\u6848\u4e4b\u5f8c\uff0c" +
                                          "<br>\u67d0\u4e9b Java \u61c9\u7528\u7a0b\u5f0f\u53ef\u80fd\u6703\u5728\u60a8\u9996\u6b21\u57f7\u884c\u5b83\u5011\u6642" +
                                          "<br>\u9700\u8981\u8f03\u9577\u6642\u9593\u4f86\u555f\u52d5\u3002" +
                                          "</html>" },

        {"cache.settings.dialog.view_jws_btn.tooltip", "<html>" +
                                          "\u6aa2\u8996\u7531 Java Web Start" +
                                          "<br>\u61c9\u7528\u7a0b\u5f0f\u5efa\u7acb\u7684\u66ab\u5b58\u6a94\u6848" +
                                          "</html>" },

        {"cache.settings.dialog.view_jpi_btn.tooltip", "<html>" +
                                          "\u6aa2\u8996\u7531 Java Applet" +
                                          "<br>\u5efa\u7acb\u7684\u66ab\u5b58\u6a94\u6848" +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "\u958b\u555f\u6a94\u6848\u700f\u89bd\u5668\u5c0d\u8a71\u65b9\u584a\uff0c\u4ee5\u6307\u5b9a\u60a8\u96fb\u8166\u4e0a\u7528\u65bc\u5132\u5b58" +
                                          "<br>\u7531 Java \u7a0b\u5f0f" +
                                          "<br>\u5efa\u7acb\u7684\u66ab\u5b58\u6a94\u6848" +
                                          "<br>\u7684\u76ee\u9304\u3002" +
                                          "</html>" },

        {"cache.settings.dialog.unlimited_btn.tooltip", "<html>" +
                                          "\u4e0d\u9650\u5236\u7528\u65bc\u5132\u5b58\u66ab\u5b58\u6a94\u6848\u7684" +
                                          "<br>\u78c1\u789f\u7a7a\u9593\u5bb9\u91cf" +
                                          "</html>" },

        {"cache.settings.dialog.max_btn.tooltip", "<html>" +
                                          "\u6307\u5b9a\u7528\u65bc\u5132\u5b58\u66ab\u5b58\u6a94\u6848\u7684" +
                                          "<br>\u6700\u5927\u78c1\u789f\u7a7a\u9593\u5bb9\u91cf\u3002" +
                                          "</html>" },

        {"cache.settings.dialog.compression.tooltip", "<html>" +
                                          "\u6307\u5b9a\u60a8\u7684\u66ab\u5b58\u6a94\u6848\u76ee\u9304\u4e2d\u7528\u65bc" +
                                          "<br>Java \u7a0b\u5f0f\u5132\u5b58\u7684 JAR \u6a94\u6848" +
                                          "<br>\u7684\u58d3\u7e2e\u5bb9\u91cf" +
                                          "<br>" +
                                          "<p>\u4f7f\u7528\u300c\u7121\u300d\uff0c\u53ef\u66f4\u5feb\u5730\u555f\u52d5\u60a8\u7684 Java \u7a0b\u5f0f\uff0c" +
                                          "<br>\u4f46\u5132\u5b58\u9019\u4e9b\u7a0b\u5f0f\u6240\u9700\u7684\u78c1\u789f\u7a7a\u9593\u5bb9\u91cf" +
					  "<br>\u6703\u6709\u6240\u589e\u52a0\uff0c" +
					  "<br>\u5bb9\u91cf\u503c\u8d8a\u5927\uff0c\u78c1\u789f\u7a7a\u9593\u7684\u9700\u6c42\u5c31\u8d8a\u4f4e\uff0c" + 
                                          "<br>\u4f46\u6703\u7a0d\u7a0d\u589e\u52a0\u555f\u52d5\u7684\u6642\u9593\u3002" +
                                          "</html>" },

        { "common.ok_btn.tooltip",  "<html>" +
                                    "\u5132\u5b58\u8b8a\u66f4\u4e26\u95dc\u9589\u5c0d\u8a71\u65b9\u584a" +
                                    "</html>" },

        { "common.cancel_btn.tooltip",  "<html>" +
                                        "\u53d6\u6d88\u8b8a\u66f4\u4e26\u95dc\u9589\u5c0d\u8a71\u65b9\u584a" +
                                        "</html>"},

	{ "network.settings.advanced_btn.tooltip",  "<html>" +
                                                    "\u6aa2\u8996\u4e26\u4fee\u6539\u9032\u968e Proxy \u8a2d\u5b9a"+
                                                    "</html>"},

        {"security.certs_btn.tooltip", "<html>" +
                                       "\u532f\u5165\u3001\u532f\u51fa\u6216\u79fb\u9664\u8b49\u66f8" +
                                       "</html>" },

        { "cert.import_btn.tooltip", "<html>" +
                                     "\u532f\u5165\u7684\u8b49\u66f8" +
                                     "<br>\u4e0d\u5728\u6e05\u55ae\u4e2d" +
				     "</html>"},

        { "cert.export_btn.tooltip",    "<html>" +
                                        "\u532f\u51fa\u9078\u53d6\u7684\u8b49\u66f8" +
                                        "</html>"},

        { "cert.remove_btn.tooltip",  "<html>" +
                                      "\u5f9e\u6e05\u55ae\u4e2d\u79fb\u9664\u9078\u53d6\u7684"+
                                      "<br>\u8b49\u66f8" +
        		      "</html>"},

        { "cert.details_btn.tooltip", "<html>" +
		      "\u6aa2\u8996\u95dc\u65bc\u9078\u53d6\u8b49\u66f8\u7684" +
                                      "<br>\u8a73\u7d30\u8cc7\u8a0a" +
		      "</html>"},

        { "java.panel.jpi_view_btn.tooltip",  "<html>" +
                                              "\u4fee\u6539 Applet \u7684 Java \u57f7\u884c\u968e\u6bb5\u8a2d\u5b9a"+
                                              "</html>" },

        { "java.panel.javaws_view_btn.tooltip",   "<html>" +
                                                  "\u4fee\u6539\u61c9\u7528\u7a0b\u5f0f\u7684 Java \u57f7\u884c\u968e\u6bb5\u8a2d\u5b9a" +
                                                  "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "\u6aa2\u8996\u95dc\u65bc\u6b64\u7248\u672c J2SE Runtime Environment \u7684" +
                                            "<br>\u8cc7\u8a0a\u3002" +
                                            "</html>" },

        { "update.notify_combo.tooltip",  "<html>" +
                                          "\u9078\u53d6\u60a8\u5e0c\u671b\u6536\u5230\u95dc\u65bc\u65b0 Java \u66f4\u65b0 " +
                                          "<br>\u901a\u77e5" +
                                          "<br>\u7684\u6642\u9593" +
                                          "</html>" },

        { "update.advanced_btn.tooltip",  "<html>" +
                                          "\u4fee\u6539\u81ea\u52d5\u66f4\u65b0\u7684" +
                                          "<br>\u6392\u7a0b\u7b56\u7565" +
                                          "</html>" },

        { "update.now_btn.tooltip",    "<html>" +
                                      "\u555f\u52d5 Java Update \u4ee5\u6aa2\u67e5" +
                                      "<br>\u6700\u65b0\u7684\u53ef\u7528 Java \u66f4\u65b0" +
                                      "</html>" },

        { "vm.options.add_btn.tooltip",   "<html>" +
                                          "\u5c07\u65b0\u7684 JRE \u65b0\u589e\u81f3\u6e05\u55ae" +
                                          "</html>" },

        { "vm.options.remove_btn.tooltip", "<html>" +
                                           "\u5f9e\u6e05\u55ae\u4e2d\u79fb\u9664\u9078\u53d6\u7684\u767b\u9304" +
                                           "</html>" },

        { "vm.optios.ok_btn.tooltip",    "<html>" +
		         "\u5132\u5b58\u5305\u542b" +
		         "<br>\u7522\u54c1\u540d\u7a31\u3001\u7248\u672c\u4ee5\u53ca" +
		         "<br>\u4f4d\u7f6e\u8cc7\u8a0a\u7684\u6240\u6709\u767b\u9304" +
		         "</html>" },

        { "jnlp.jre.find_btn.tooltip",  "<html>" +
		        "\u641c\u5c0b\u5df2\u5b89\u88dd\u7684 Java \u57f7\u884c\u968e\u6bb5" +
                                        "<br>\u74b0\u5883" +
		        "</html>" },

        { "jnlp.jre.add_btn.tooltip",   "<html>" +
                                        "\u5c07\u65b0\u7684\u767b\u9304\u65b0\u589e\u81f3\u6e05\u55ae" +
		                        "</html>" },

        { "jnlp.jre.remove_btn.tooltip",  "<html>" +
                                          "\u5f9e\u300c\u4f7f\u7528\u8005\u300d\u6e05\u55ae\u4e2d\u79fb\u9664" +
                                          "<br>\u9078\u53d6\u7684\u767b\u9304" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "\u5141\u8a31\u4e0b\u8f09 JRE" },
        { "download.jre.prompt.text1", "\u61c9\u7528\u7a0b\u5f0f \"{0}\" \u9700\u8981 "
                                     + "\u7684 JRE \u7248\u672c (\u7248\u672c {1}) "
                                     + "\u76ee\u524d\u5c1a\u672a\u5b89\u88dd\u5728\u60a8\u7684\u7cfb\u7d71\u4e0a\u3002" },
        { "download.jre.prompt.text2", "\u60a8\u5e0c\u671b Java Web Start \u81ea\u52d5 "
                                     + "\u4e0b\u8f09\u4e26\u5b89\u88dd\u6b64 JRE \u55ce\uff1f" },
        { "download.jre.prompt.okButton", "\u4e0b\u8f09(D)" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "download.jre.prompt.cancelButton", "\u53d6\u6d88(C)" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "autoupdatecheck.buttonYes", "\u662f(Y)" },
	{ "autoupdatecheck.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "autoupdatecheck.buttonNo", "\u5426(N)" },
	{ "autoupdatecheck.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "autoupdatecheck.buttonAskLater", "\u7a0d\u5f8c\u8a62\u554f\u6211(A)" },
	{ "autoupdatecheck.buttonAskLater.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "autoupdatecheck.caption", "\u81ea\u52d5\u6aa2\u67e5\u66f4\u65b0" },
	{ "autoupdatecheck.message", "\u65b0\u7248\u672c\u53ef\u7528\u6642\uff0cJava Update \u53ef\u81ea\u52d5\u66f4\u65b0\u60a8\u7684 Java \u8edf\u9ad4\u3002 \u60a8\u9858\u610f\u555f\u7528\u6b64\u9805\u670d\u52d9\u55ce\uff1f" },
    };
}

