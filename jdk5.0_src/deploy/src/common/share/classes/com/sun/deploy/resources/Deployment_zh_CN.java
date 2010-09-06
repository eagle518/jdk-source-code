/*
 * @(#)Deployment_zh_CN.java	1.28 04/07/16 
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

public final class Deployment_zh_CN extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
        { "product.javapi.name", "Java Plug-in {0}" },
        { "product.javaws.name", "Java Web Start {0}" },
	{ "console.version", "\u7248\u672c" },
	{ "console.default_vm_version", "\u7f3a\u7701\u865a\u62df\u673a\u7248\u672c" },
	{ "console.using_jre_version", "\u4f7f\u7528 JRE \u7248\u672c" },
	{ "console.user_home", "\u7528\u6237\u4e3b\u76ee\u5f55" },
        { "console.caption", "Java \u63a7\u5236\u53f0"},
        { "console.clear", "\u6e05\u9664(C)"},
        { "console.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "console.close", "\u5173\u95ed(E)"},
        { "console.close.acceleratorKey", new Integer(KeyEvent.VK_E) },
        { "console.copy", "\u590d\u5236(Y)"},
        { "console.copy.acceleratorKey", new Integer(KeyEvent.VK_Y) },
	{ "console.menu.text.top", "----------------------------------------------------\n" },
	{ "console.menu.text.c", "c:   \u6e05\u9664\u63a7\u5236\u53f0\u7a97\u53e3\n" },
	{ "console.menu.text.f", "f:   \u7ec8\u7ed3\u5728\u7ed3\u675f\u961f\u5217\u4e0a\u7684\u5bf9\u8c61\n" },
	{ "console.menu.text.g", "g:   \u5783\u573e\u6536\u96c6\n" },
	{ "console.menu.text.h", "h:   \u663e\u793a\u6b64\u5e2e\u52a9\u6d88\u606f\n" },
	{ "console.menu.text.j", "j:   \u8f6c\u50a8 jcov \u6570\u636e\n"},
	{ "console.menu.text.l", "l:   \u8f6c\u50a8\u7c7b\u8f7d\u5165\u7a0b\u5e8f\u5217\u8868\n" },
	{ "console.menu.text.m", "m:   \u6253\u5370\u5185\u5b58\u4f7f\u7528\n" },
	{ "console.menu.text.o", "o:   \u89e6\u53d1\u65e5\u5fd7\u8bb0\u5f55\n" },
	{ "console.menu.text.p", "p:   \u91cd\u65b0\u8f7d\u5165\u4ee3\u7406\u914d\u7f6e\n" },
	{ "console.menu.text.q", "q:   \u9690\u85cf\u63a7\u5236\u53f0\n" },
	{ "console.menu.text.r", "r:   \u91cd\u65b0\u8f7d\u5165\u7b56\u7565\u914d\u7f6e\n" },
	{ "console.menu.text.s", "s:   \u8f6c\u50a8\u7cfb\u7edf\u548c\u90e8\u7f72\u5c5e\u6027\n" },
	{ "console.menu.text.t", "t:   \u8f6c\u50a8\u7ebf\u7a0b\u5217\u8868\n" },
	{ "console.menu.text.v", "v:   \u8f6c\u50a8\u7ebf\u7a0b\u5806\u6808\n" },
	{ "console.menu.text.x", "x:   \u6e05\u9664\u7c7b\u8f7d\u5165\u7a0b\u5e8f\u9ad8\u901f\u7f13\u5b58\n" },
	{ "console.menu.text.0", "0-5: \u8bbe\u7f6e\u8ddf\u8e2a\u7ea7\u522b\u4e3a<n>\n" },
	{ "console.menu.text.tail", "----------------------------------------------------\n" },
	{ "console.done", "\u5b8c\u6210\u3002" },
	{ "console.trace.level.0", "\u8ddf\u8e2a\u7ea7\u522b\u8bbe\u7f6e\u4e3a 0\uff1a\u65e0 ... \u5b8c\u6210\u3002" },
	{ "console.trace.level.1", "\u8ddf\u8e2a\u7ea7\u522b\u8bbe\u7f6e\u4e3a 1\uff1a\u521d\u7ea7 ... \u5b8c\u6210\u3002" },
	{ "console.trace.level.2", "\u8ddf\u8e2a\u7ea7\u522b\u8bbe\u7f6e\u4e3a 2\uff1a\u521d\u7ea7\uff0c\u7f51\u7edc ... \u5b8c\u6210\u3002" },
	{ "console.trace.level.3", "\u8ddf\u8e2a\u7ea7\u522b\u8bbe\u7f6e\u4e3a 3\uff1a\u521d\u7ea7\uff0c\u7f51\u7edc\uff0c\u5b89\u5168 ... \u5b8c\u6210\u3002" },
	{ "console.trace.level.4", "\u8ddf\u8e2a\u7ea7\u522b\u8bbe\u7f6e\u4e3a 4\uff1a\u521d\u7ea7\uff0c\u7f51\u7edc\uff0c\u5b89\u5168\uff0c\u6269\u5c55 ... \u5b8c\u6210\u3002" },
	{ "console.trace.level.5", "\u8ddf\u8e2a\u7ea7\u522b\u8bbe\u7f6e\u4e3a 5\uff1a\u5168\u90e8 ... \u5b8c\u6210\u3002" },
	{ "console.log", "\u8bb0\u5f55\u8bbe\u7f6e\u4e3a\uff1a" },
	{ "console.completed", " .... \u5b8c\u6210\u3002" },
	{ "console.dump.thread", "\u8f6c\u50a8\u7ebf\u7a0b\u5217\u8868 ...\n" },
	{ "console.dump.stack", "\u8f6c\u50a8\u7ebf\u7a0b\u5806\u6808 ...\n" },
	{ "console.dump.system.properties", "\u8f6c\u50a8\u7cfb\u7edf\u5c5e\u6027 ...\n" },
	{ "console.dump.deployment.properties", "\u8f6c\u50a8\u90e8\u7f72\u5c5e\u6027 ...\n" },
	{ "console.clear.classloader", "\u6e05\u9664\u7c7b\u8f7d\u5165\u7a0b\u5e8f\u9ad8\u901f\u7f13\u5b58 .... \u5b8c\u6210\u3002" },
	{ "console.reload.policy", "\u91cd\u65b0\u8f7d\u5165\u7b56\u7565\u914d\u7f6e" },
	{ "console.reload.proxy", "\u91cd\u65b0\u8f7d\u5165\u4ee3\u7406\u914d\u7f6e ..." },
	{ "console.gc", "\u5783\u573e\u6536\u96c6" },
	{ "console.finalize", " \u7ec8\u7ed3\u5728\u7ed3\u675f\u961f\u5217\u4e0a\u7684\u5bf9\u8c61" },
	{ "console.memory", "\u5185\u5b58\uff1a{0}K  \u7a7a\u95f2\uff1a{1}K  ({2}%)" },
	{ "console.jcov.error", "Jcov Runtime\u9519\u8bef\uff1a\u8bf7\u68c0\u67e5\u662f\u5426\u6307\u5b9a\u4e86\u6b63\u786e\u7684 jcov \u9009\u9879\n"},
	{ "console.jcov.info", "Jcov \u6570\u636e\u8f6c\u50a8\u6210\u529f\n"},

	{ "https.dialog.caption", "\u8b66\u544a - HTTPS" },
	{ "https.dialog.text", "<html><b>\u4e3b\u673a\u540d\u4e0d\u5339\u914d</b></html>\u5728\u670d\u52a1\u5668\u5b89\u5168\u8bc1\u4e66\u4e2d\u7684\u4e3b\u673a\u540d\u4e0e\u670d\u52a1\u5668\u540d\u4e0d\u5339\u914d\u3002"	
			     + "\n\nURL \u7684\u4e3b\u673a\u540d\uff1a{0}"
			     + "\n\u8bc1\u4e66\u4e0a\u7684\u4e3b\u673a\u540d\uff1a{1}"
			     + "\n\n\u662f\u5426\u8981\u7ee7\u7eed\uff1f" },
	{ "https.dialog.unknown.host", "\u672a\u77e5\u4e3b\u673a" },

	{ "security.dialog.caption", "\u8b66\u544a - \u5b89\u5168" },
	{ "security.dialog.text0", "\u8981\u4fe1\u4efb\u7531 \"{1}\"\u5206\u53d1\u7b7e\u7f72\u7684 {0} \u5417\uff1f"
				 + "\n\n\u51fa\u7248\u5546\u7684\u771f\u5b9e\u6027\u5df2\u7531\"{2}\"\u9a8c\u8bc1" },
	{ "security.dialog.text0a", "\u8981\u4fe1\u4efb\u7531\u201c{1}\u201d\u5206\u53d1\u7b7e\u7f72\u7684 {0} \u5417\uff1f"
	                         + "\n\n\u65e0\u6cd5\u9a8c\u8bc1\u51fa\u7248\u5546\u7684\u771f\u5b9e\u6027\u3002" },
	{ "security.dialog.timestamp.text1", "\u5df2\u5728 {1} \u5206\u53d1\u7b7e\u7f72{0}\u3002" },
	{ "security.dialog_https.text0", "\u4e3a\u4e86\u4ea4\u6362\u52a0\u5bc6\u4fe1\u606f\uff0c\u60a8\u662f\u5426\u8981\u901a\u8fc7\u7f51\u7ad9\"{0}\"\u63a5\u53d7\u6b64\u8bc1\u4e66\uff1f"
				 + "\n\n\u51fa\u7248\u5546\u7684\u771f\u5b9e\u6027\u5df2\u7531\"{1}\"\u9a8c\u8bc1" },
	{ "security.dialog_https.text0a", "\u4e3a\u4e86\u4ea4\u6362\u52a0\u5bc6\u4fe1\u606f\uff0c\u60a8\u662f\u5426\u8981\u63a5\u53d7\u7f51\u7ad9\u201c{0}\u201d\u7684\u6b64\u8bc1\u4e66\uff1f"
 				 + "\n\n\u65e0\u6cd5\u9a8c\u8bc1\u51fa\u7248\u5546\u7684\u771f\u5b9e\u6027\u3002" },
	{ "security.dialog.text1", "\n\u6ce8\u610f\uff1a\"{0}\" \u58f0\u660e\u672c\u5185\u5bb9\u5b89\u5168\u3002\u53ea\u6709\u5f53\u60a8\u4fe1\u4efb \"{1}\" \u4f5c\u51fa\u6b64\u58f0\u660e\u540e\u624d\u53ef\u63a5\u53d7\u6b64\u5185\u5bb9\u3002\n\n" },
	{ "security.dialog.unknown.issuer", "\u672a\u77e5\u7b7e\u53d1\u4eba" },
	{ "security.dialog.unknown.subject", "\u672a\u77e5\u4e3b\u9898" },
	{ "security.dialog.certShowName", "{0}({1})" },
	{ "security.dialog.rootCANotValid", "\u6b64\u5b89\u5168\u8bc1\u4e66\u662f\u7531\u4e0d\u53ef\u4fe1\u7684\u516c\u53f8\u7b7e\u53d1\u7684\u3002" },
	{ "security.dialog.rootCAValid", "\u6b64\u5b89\u5168\u8bc1\u4e66\u662f\u7531\u53ef\u4fe1\u8d56\u7684\u516c\u53f8\u7b7e\u53d1\u7684\u3002" },
	{ "security.dialog.timeNotValid", "\u5b89\u5168\u8bc1\u4e66\u5df2\u5230\u671f\u6216\u5c1a\u672a\u751f\u6548\u3002" },
	{ "security.dialog.timeValid", "\u5b89\u5168\u8bc1\u4e66\u5c1a\u672a\u5230\u671f\uff0c\u4f9d\u7136\u6709\u6548\u3002" },
	{ "security.dialog.timeValidTS", "\u7b7e\u53d1{0}\u65f6\u5b89\u5168\u8bc1\u4e66\u4f9d\u7136\u6709\u6548\u3002" },
	{ "security.dialog.buttonAlways", "\u603b\u662f\u6709\u6548(A)" },
        { "security.dialog.buttonAlways.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "security.dialog.buttonYes", "\u662f(Y)" },
	{ "security.dialog.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "security.dialog.buttonNo", "\u5426(N)" },
	{ "security.dialog.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "security.dialog.buttonViewCert", "\u66f4\u591a\u8be6\u7ec6\u4fe1\u606f(M)" },
        { "security.dialog.buttonViewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

        { "security.badcert.caption", "\u8b66\u544a - \u5b89\u5168" },
        { "security.badcert.https.text", "\u65e0\u6cd5\u9a8c\u8bc1 SSL \u8bc1\u4e66\u3002\n\u5c06\u65e0\u6cd5\u8fd0\u884c\u6b64 {0}\u3002" },
        { "security.badcert.config.text", "\u60a8\u7684\u5b89\u5168\u914d\u7f6e\u4e0d\u5141\u8bb8\u60a8\u9a8c\u8bc1\u6b64\u8bc1\u4e66\u3002\u5c06\u65e0\u6cd5\u8fd0\u884c\u6b64 {0}\u3002" },
        { "security.badcert.text", "\u9a8c\u8bc1\u8bc1\u4e66\u5931\u8d25\u3002\u5c06\u65e0\u6cd5\u8fd0\u884c\u6b64 {0}\u3002" },
        { "security.badcert.viewException", "\u663e\u793a\u5f02\u5e38(S)" },
        { "security.badcert.viewException.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "security.badcert.viewCert", "\u66f4\u591a\u8be6\u7ec6\u4fe1\u606f(M)" },
        { "security.badcert.viewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

	{ "cert.dialog.caption", "\u8be6\u7ec6\u4fe1\u606f - \u8bc1\u4e66" },
	{ "cert.dialog.certpath", "\u8bc1\u4e66\u8def\u5f84" },
	{ "cert.dialog.field.Version", "\u7248\u672c" },
	{ "cert.dialog.field.SerialNumber", "\u5e8f\u5217\u53f7" },
	{ "cert.dialog.field.SignatureAlg", "\u7b7e\u540d\u7b97\u6cd5" },
	{ "cert.dialog.field.Issuer", "\u7b7e\u53d1\u4eba" },
	{ "cert.dialog.field.EffectiveDate", "\u751f\u6548\u65e5\u671f" },
	{ "cert.dialog.field.ExpirationDate", "\u5230\u671f\u65e5\u671f" },
	{ "cert.dialog.field.Validity", "\u6709\u6548\u6027" },
	{ "cert.dialog.field.Subject", "\u4e3b\u9898" },
	{ "cert.dialog.field.Signature", "\u7b7e\u5b57" },
	{ "cert.dialog.field", "\u5b57\u6bb5" },
	{ "cert.dialog.value", "\u503c" },
        { "cert.dialog.close", "\u5173\u95ed(C)" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_C) },

	{ "clientauth.password.dialog.buttonOK", "\u786e\u5b9a(O)" },
	{ "clientauth.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.password.dialog.buttonCancel", "\u53d6\u6d88(C)" },
	{ "clientauth.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.password.dialog.caption", "\u9700\u8981\u53e3\u4ee4 - \u5ba2\u6237\u9a8c\u8bc1\u5bc6\u94a5\u5e93" },
	{ "clientauth.password.dialog.text", "\u8f93\u5165\u53e3\u4ee4\u4ee5\u8bbf\u95ee\u5ba2\u6237\u9a8c\u8bc1\u5bc6\u94a5\u5e93:\n" },
	{ "clientauth.password.dialog.error.caption", "\u9519\u8bef - \u5ba2\u6237\u9a8c\u8bc1\u5bc6\u94a5\u5e93" },
	{ "clientauth.password.dialog.error.text", "<html><b>\u5bc6\u94a5\u5e93\u8bbf\u95ee\u9519\u8bef</b></html>\u5bc6\u94a5\u5e93\u635f\u574f\uff0c\u6216\u53e3\u4ee4\u4e0d\u6b63\u786e\u3002" },
		
	{ "clientauth.certlist.dialog.buttonOK", "\u786e\u5b9a(O)" },
	{ "clientauth.certlist.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.certlist.dialog.buttonCancel", "\u53d6\u6d88(C)" },
	{ "clientauth.certlist.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.certlist.dialog.buttonDetails", "\u8be6\u7ec6\u4fe1\u606f(D)" },
	{ "clientauth.certlist.dialog.buttonDetails.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "clientauth.certlist.dialog.caption", "\u5ba2\u6237\u9a8c\u8bc1" },
	{ "clientauth.certlist.dialog.text", "\u8981\u8fde\u63a5\u7684\u7f51\u7ad9\u9700\u8981\u6807\u8bc6\u3002\n\u9009\u62e9\u8fde\u63a5\u65f6\u8981\u4f7f\u7528\u7684\u8bc1\u4e66\u3002\n" },

	{ "dialogfactory.confirmDialogTitle", "\u9700\u8981\u786e\u8ba4 - Java" },
	{ "dialogfactory.inputDialogTitle", "\u9700\u8981\u4fe1\u606f - Java" },
	{ "dialogfactory.messageDialogTitle", "\u4fe1\u606f - Java" },
	{ "dialogfactory.exceptionDialogTitle", "\u9519\u8bef - Java" },
	{ "dialogfactory.optionDialogTitle", "\u9009\u9879 - Java" },
	{ "dialogfactory.aboutDialogTitle", "\u5173\u4e8e - Java" },
	{ "dialogfactory.confirm.yes", "\u662f(Y)" },
        { "dialogfactory.confirm.yes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "dialogfactory.confirm.no", "\u5426(N)" },
        { "dialogfactory.confirm.no.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "dialogfactory.moreInfo", "\u66f4\u591a\u8be6\u7ec6\u4fe1\u606f(M)" },
        { "dialogfactory.moreInfo.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "dialogfactory.lessInfo", "\u8f83\u5c11\u8be6\u7ec6\u4fe1\u606f(L)" },
        { "dialogfactory.lessInfo.acceleratorKey", new Integer(KeyEvent.VK_L)},
	{ "dialogfactory.java.home.link", "http://java.sun.com" },
	{ "dialogfactory.general_error", "<html><b>\u4e00\u822c\u5f02\u5e38</b></html>" },
	{ "dialogfactory.net_error", "<html><b>\u7f51\u7edc\u5f02\u5e38</b></html>" },
	{ "dialogfactory.security_error", "<html><b>\u5b89\u5168\u5f02\u5e38</b></html>" },
	{ "dialogfactory.ext_error", "<html><b>\u53ef\u9009\u8f6f\u4ef6\u5305\u5f02\u5e38</b></html>" },
	{ "dialogfactory.user.selected", "\u7528\u6237\u6240\u9009\u7684\uff1a{0}" },
	{ "dialogfactory.user.typed", "\u7528\u6237\u952e\u5165\u7684\uff1a{0}" },

	{ "deploycertstore.cert.loading", "\u6b63\u5728\u4ece {0} \u88c5\u5165\u90e8\u7f72\u8bc1\u4e66" },
	{ "deploycertstore.cert.loaded", "\u5df2\u4ece {0} \u88c5\u5165\u90e8\u7f72\u8bc1\u4e66" },
	{ "deploycertstore.cert.saving", "\u6b63\u5728\u5c06\u90e8\u7f72\u8bc1\u4e66\u4fdd\u5b58\u81f3 {0}" },
	{ "deploycertstore.cert.saved", "\u5df2\u5c06\u90e8\u7f72\u8bc1\u4e66\u4fdd\u5b58\u81f3 {0}" },
	{ "deploycertstore.cert.adding", "\u6b63\u5728\u5c06\u8bc1\u4e66\u6dfb\u52a0\u5230\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d", },
	{ "deploycertstore.cert.added", "\u5df2\u5c06\u8bc1\u4e66\u4ee5\u522b\u540d {0} \u6dfb\u52a0\u5230\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d" },
	{ "deploycertstore.cert.removing", "\u6b63\u5728\u5220\u9664\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66" },
	{ "deploycertstore.cert.removed", "\u5df2\u5220\u9664\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d\u522b\u540d\u4e3a {0} \u7684\u8bc1\u4e66" },
	{ "deploycertstore.cert.instore", "\u6b63\u5728\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u5728\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d" },
	{ "deploycertstore.cert.canverify", "\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u53ef\u4ee5\u4f7f\u7528\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66\u8fdb\u884c\u9a8c\u8bc1" },
	{ "deploycertstore.cert.iterator", "\u5728\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d\u83b7\u53d6\u8bc1\u4e66\u8fed\u4ee3\u7a0b\u5e8f" },
	{ "deploycertstore.cert.getkeystore", "\u83b7\u53d6\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u7684\u5bc6\u94a5\u5e93\u5bf9\u8c61" },

	{ "httpscertstore.cert.loading", "\u6b63\u5728\u4ece {0} \u88c5\u5165\u90e8\u7f72 SSL \u8bc1\u4e66" },
	{ "httpscertstore.cert.loaded", "\u5df2\u4ece {0} \u88c5\u5165\u90e8\u7f72 SSL \u8bc1\u4e66" },
	{ "httpscertstore.cert.saving", "\u6b63\u5728\u5c06\u90e8\u7f72 SSL \u8bc1\u4e66\u4fdd\u5b58\u81f3 {0}" },
	{ "httpscertstore.cert.saved", "\u5df2\u5c06\u90e8\u7f72 SSL \u8bc1\u4e66\u4fdd\u5b58\u81f3 {0}" },
	{ "httpscertstore.cert.adding", "\u6b63\u5728\u5c06 SSL \u8bc1\u4e66\u6dfb\u52a0\u5230\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d", },
	{ "httpscertstore.cert.added", "\u5df2\u5c06 SSL \u8bc1\u4e66\u4ee5\u522b\u540d {0} \u6dfb\u52a0\u5230\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d" },
	{ "httpscertstore.cert.removing", "\u6b63\u5728\u5220\u9664\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d\u7684 SSL \u8bc1\u4e66" },
	{ "httpscertstore.cert.removed", "\u5df2\u5220\u9664\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d\u522b\u540d\u4e3a {0} \u7684 SSL \u8bc1\u4e66" },
	{ "httpscertstore.cert.instore", "\u6b63\u5728\u68c0\u67e5 SSL \u8bc1\u4e66\u662f\u5426\u5728\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d" },
	{ "httpscertstore.cert.canverify", "\u68c0\u67e5 SSL \u8bc1\u4e66\u662f\u5426\u53ef\u4ee5\u4f7f\u7528\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66\u8fdb\u884c\u9a8c\u8bc1" },
	{ "httpscertstore.cert.iterator", "\u5728\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u4e2d\u83b7\u53d6 SSL \u8bc1\u4e66\u8fed\u4ee3\u7a0b\u5e8f" },
	{ "httpscertstore.cert.getkeystore", "\u83b7\u53d6\u90e8\u7f72\u6c38\u4e45\u8bc1\u4e66\u5e93\u7684\u5bc6\u94a5\u5e93\u5bf9\u8c61" },

	{ "rootcertstore.cert.loading", "\u6b63\u5728\u4ece {0} \u88c5\u5165\u6839 CA \u8bc1\u4e66" },
	{ "rootcertstore.cert.loaded", "\u5df2\u4ece {0} \u88c5\u5165\u6839 CA \u8bc1\u4e66" },
	{ "rootcertstore.cert.noload", "\u672a\u627e\u5230\u6839 CA \u8bc1\u4e66\u6587\u4ef6: {0}" },
	{ "rootcertstore.cert.saving", "\u6b63\u5728\u5c06\u6839 CA \u8bc1\u4e66\u4fdd\u5b58\u81f3 {0}" },
	{ "rootcertstore.cert.saved", "\u5df2\u5c06\u6839 CA \u8bc1\u4e66\u4fdd\u5b58\u81f3 {0}" },
	{ "rootcertstore.cert.adding", "\u6b63\u5728\u5c06\u8bc1\u4e66\u6dfb\u52a0\u5230\u6839 CA \u8bc1\u4e66\u5e93\u4e2d", },
	{ "rootcertstore.cert.added", "\u5df2\u5c06\u8bc1\u4e66\u4ee5\u522b\u540d {0} \u6dfb\u52a0\u5230\u6839 CA \u8bc1\u4e66\u5e93\u4e2d" },
	{ "rootcertstore.cert.removing", "\u6b63\u5728\u5220\u9664\u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66" },
	{ "rootcertstore.cert.removed", "\u5df2\u5220\u9664\u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u522b\u540d\u4e3a {0} \u7684\u8bc1\u4e66" },
	{ "rootcertstore.cert.instore", "\u6b63\u5728\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u5728\u6839 CA \u8bc1\u4e66\u5e93\u4e2d" },
	{ "rootcertstore.cert.canverify", "\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u53ef\u4ee5\u4f7f\u7528\u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66\u8fdb\u884c\u9a8c\u8bc1" },
	{ "rootcertstore.cert.iterator", "\u5728\u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u83b7\u53d6\u8bc1\u4e66\u8fed\u4ee3\u7a0b\u5e8f" },
	{ "rootcertstore.cert.getkeystore", "\u83b7\u53d6\u6839 CA \u8bc1\u4e66\u5e93\u7684\u5bc6\u94a5\u5e93\u5bf9\u8c61" },
	{ "rootcertstore.cert.verify.ok", "\u5df2\u4f7f\u7528\u6839 CA \u8bc1\u4e66\u6210\u529f\u9a8c\u8bc1\u8bc1\u4e66" },
	{ "rootcertstore.cert.verify.fail", "\u4f7f\u7528\u6839 CA \u8bc1\u4e66\u65e0\u6cd5\u6210\u529f\u9a8c\u8bc1\u8bc1\u4e66" },
	{ "rootcertstore.cert.tobeverified", "\u5c06\u8981\u9a8c\u8bc1\u4ee5\u4e0b\u8bc1\u4e66\uff1a\n {0}" },
	{ "rootcertstore.cert.tobecompared", "\u5c06\u8bc1\u4e66\u4e0e\u4ee5\u4e0b\u6839 CA \u8bc1\u4e66\u6bd4\u8f83:\n{0}" },

	{ "roothttpscertstore.cert.loading", "\u6b63\u5728\u4ece {0} \u8f7d\u5165 SSL \u6839 CA \u8bc1\u4e66" },
	{ "roothttpscertstore.cert.loaded", "\u5df2\u4ece {0} \u8f7d\u5165 SSL \u6839 CA \u8bc1\u4e66" },
	{ "roothttpscertstore.cert.noload", "\u672a\u627e\u5230 SSL \u6839 CA \u8bc1\u4e66\u6587\u4ef6\uff1a{0}" },
	{ "roothttpscertstore.cert.saving", "\u6b63\u5728\u5c06 SSL \u6839 CA \u8bc1\u4e66\u4fdd\u5b58\u5230 {0}" },
	{ "roothttpscertstore.cert.saved", "\u5df2\u5c06 SSL \u6839 CA \u8bc1\u4e66\u4fdd\u5b58\u5230 {0}" },
	{ "roothttpscertstore.cert.adding", "\u6b63\u5728\u5411 SSL \u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u6dfb\u52a0\u8bc1\u4e66", },
	{ "roothttpscertstore.cert.added", "\u5df2\u5c06\u8bc1\u4e66\u4ee5\u522b\u540d {0} \u6dfb\u52a0\u5230 SSL \u6839 CA \u8bc1\u4e66\u5e93\u4e2d" },
	{ "roothttpscertstore.cert.removing", "\u6b63\u5728\u4ece SSL \u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u5220\u9664\u8bc1\u4e66" },
	{ "roothttpscertstore.cert.removed", "\u5df2\u4ece SSL \u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u5220\u9664\u4ee5\u522b\u540d {0} \u5b58\u5728\u7684\u8bc1\u4e66" },
	{ "roothttpscertstore.cert.instore", "\u6b63\u5728\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u5b58\u5728\u4e8e SSL \u6839 CA \u8bc1\u4e66\u5e93\u4e2d" },
	{ "roothttpscertstore.cert.canverify", "\u68c0\u67e5\u662f\u5426\u53ef\u7528 SSL \u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66\u9a8c\u8bc1\u8bc1\u4e66" },
	{ "roothttpscertstore.cert.iterator", "\u83b7\u53d6 SSL \u6839 CA \u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66\u8fed\u4ee3" },
	{ "roothttpscertstore.cert.getkeystore", "\u83b7\u53d6 SSL \u6839 CA \u8bc1\u4e66\u5e93\u7684\u5bc6\u94a5\u5e93\u5bf9\u8c61" },
	{ "roothttpscertstore.cert.verify.ok", "\u8bc1\u4e66\u5df2\u6210\u529f\u901a\u8fc7 SSL \u6839 CA \u8bc1\u4e66\u5e93\u9a8c\u8bc1" },
	{ "roothttpscertstore.cert.verify.fail", "\u901a\u8fc7 SSL \u6839 CA \u8bc1\u4e66\u5e93\u9a8c\u8bc1\u7684\u8bc1\u4e66\u5df2\u5931\u8d25" },
	{ "roothttpscertstore.cert.tobeverified", "\u5c06\u8981\u9a8c\u8bc1\u4ee5\u4e0b\u8bc1\u4e66\uff1a\n {0}" },
	{ "roothttpscertstore.cert.tobecompared", "\u5c06\u8bc1\u4e66\u4e0e\u4ee5\u4e0b SSL \u6839 CA \u8bc1\u4e66\u6bd4\u8f83\uff1a\n {0}" },

	{ "sessioncertstore.cert.loading", "\u6b63\u5728\u4ece\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d\u88c5\u5165\u8bc1\u4e66" },
	{ "sessioncertstore.cert.loaded", "\u5df2\u4ece\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d\u88c5\u5165\u8bc1\u4e66" },
	{ "sessioncertstore.cert.saving", "\u6b63\u5728\u5c06\u8bc1\u4e66\u4fdd\u5b58\u81f3\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93" },
	{ "sessioncertstore.cert.saved", "\u5df2\u5c06\u8bc1\u4e66\u4fdd\u5b58\u81f3\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93" },
	{ "sessioncertstore.cert.adding", "\u6b63\u5728\u5c06\u8bc1\u4e66\u6dfb\u52a0\u5230\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d", },
	{ "sessioncertstore.cert.added", "\u5df2\u5c06\u8bc1\u4e66\u6dfb\u52a0\u5230\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d" },
	{ "sessioncertstore.cert.removing", "\u6b63\u5728\u5220\u9664\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66" },
	{ "sessioncertstore.cert.removed", "\u5df2\u5220\u9664\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66" },
	{ "sessioncertstore.cert.instore", "\u6b63\u5728\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u5728\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d" },
	{ "sessioncertstore.cert.canverify", "\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u53ef\u4ee5\u4f7f\u7528\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66\u8fdb\u884c\u9a8c\u8bc1" },
	{ "sessioncertstore.cert.iterator", "\u5728\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u4e2d\u83b7\u53d6\u8bc1\u4e66\u8fed\u4ee3\u7a0b\u5e8f" },
	{ "sessioncertstore.cert.getkeystore", "\u83b7\u53d6\u90e8\u7f72\u4f1a\u8bdd\u8bc1\u4e66\u5e93\u7684\u5bc6\u94a5\u5e93\u5bf9\u8c61" },

	{ "iexplorer.cert.loading", "\u6b63\u5728\u4ece Internet Explorer {0} \u8bc1\u4e66\u5e93\u88c5\u5165\u8bc1\u4e66" },
	{ "iexplorer.cert.loaded", "\u5df2\u4ece Internet Explorer {0} \u8bc1\u4e66\u5e93\u88c5\u5165\u8bc1\u4e66" },
	{ "iexplorer.cert.instore", "\u6b63\u5728\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u5728 Internet Explorer {0} \u8bc1\u4e66\u5e93\u4e2d" },
	{ "iexplorer.cert.canverify", "\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u53ef\u4ee5\u4f7f\u7528 Internet Explorer {0} \u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66\u8fdb\u884c\u9a8c\u8bc1" },
	{ "iexplorer.cert.iterator", "\u5728 Internet Explorer {0} \u8bc1\u4e66\u5e93\u4e2d\u83b7\u53d6\u8bc1\u4e66\u8fed\u4ee3\u7a0b\u5e8f" },
	{ "iexplorer.cert.verify.ok", "\u5df2\u4f7f\u7528 Internet Explorer {0} \u8bc1\u4e66\u6210\u529f\u9a8c\u8bc1\u8bc1\u4e66" },
	{ "iexplorer.cert.verify.fail", "\u4f7f\u7528 Internet Explorer {0} \u8bc1\u4e66\u65e0\u6cd5\u6210\u529f\u9a8c\u8bc1\u8bc1\u4e66" },
	{ "iexplorer.cert.tobeverified", "\u5c06\u8981\u9a8c\u8bc1\u4ee5\u4e0b\u8bc1\u4e66:\n{0}" },
	{ "iexplorer.cert.tobecompared", "\u5c06\u8bc1\u4e66\u4e0e\u4ee5\u4e0b Internet Explorer {0} \u8bc1\u4e66\u6bd4\u8f83:\n{1}" },

	{ "mozilla.cert.loading", "\u6b63\u5728\u4ece Mozilla {0} \u8bc1\u4e66\u5e93\u88c5\u5165\u8bc1\u4e66" },
        { "mozilla.cert.loaded", "\u5df2\u4ece Mozilla {0} \u8bc1\u4e66\u5e93\u88c5\u5165\u8bc1\u4e66" },
        { "mozilla.cert.instore", "\u6b63\u5728\u68c0\u67e5\u8bc1\u4e66\u662f\u5426\u5728 Mozilla {0} \u8bc1\u4e66\u5e93\u4e2d" },
        { "mozilla.cert.canverify", "\u68c0\u67e5\u662f\u5426\u53ef\u4ee5\u4f7f\u7528 Mozilla {0} \u8bc1\u4e66\u5e93\u4e2d\u7684\u8bc1\u4e66\u8fdb\u884c\u9a8c\u8bc1" },
        { "mozilla.cert.iterator", "\u5728 Mozilla {0} \u8bc1\u4e66\u5e93\u4e2d\u83b7\u53d6\u8bc1\u4e66\u8fed\u4ee3\u7a0b\u5e8f" },
        { "mozilla.cert.verify.ok", "\u4f7f\u7528 Mozilla {0} \u8bc1\u4e66\u8fdb\u884c\u9a8c\u8bc1\u6210\u529f" },
        { "mozilla.cert.verify.fail", "\u4f7f\u7528 Mozilla {0} \u8bc1\u4e66\u8fdb\u884c\u9a8c\u8bc1\u5931\u8d25" },
        { "mozilla.cert.tobeverified", "\u5c06\u8981\u9a8c\u8bc1\u4ee5\u4e0b\u8bc1\u4e66:\n{0}" },
        { "mozilla.cert.tobecompared", "\u5c06\u8bc1\u4e66\u4e0e\u4ee5\u4e0b Mozilla {0} \u8bc1\u4e66\u6bd4\u8f83:\n{1}" },

        { "browserkeystore.jss.no", "\u672a\u627e\u5230 JSS \u8f6f\u4ef6\u5305" },
        { "browserkeystore.jss.yes", "\u5df2\u88c5\u5165 JSS \u8f6f\u4ef6\u5305" },
        { "browserkeystore.jss.notconfig", "\u672a\u914d\u7f6e JSS" },
        { "browserkeystore.jss.config", "\u5df2\u914d\u7f6e JSS" },
        { "browserkeystore.mozilla.dir", "\u6b63\u5728\u8bbf\u95ee Mozilla \u7528\u6237\u914d\u7f6e\u6587\u4ef6\u4e2d\u7684\u5bc6\u94a5\u548c\u8bc1\u4e66: {0}" },
	{ "browserkeystore.password.dialog.buttonOK", "\u786e\u5b9a(O)" },
	{ "browserkeystore.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "browserkeystore.password.dialog.buttonCancel", "\u53d6\u6d88(C)" },
	{ "browserkeystore.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "browserkeystore.password.dialog.caption", "\u9700\u8981\u53e3\u4ee4" },
	{ "browserkeystore.password.dialog.text", "\u8bf7\u8f93\u5165 {0} \u7684\u53e3\u4ee4:\n" },
	{ "mozillamykeystore.priv.notfound", "\u672a\u627e\u5230\u4ee5\u4e0b\u8bc1\u4e66\u7684\u79c1\u4eba\u5bc6\u94a5: {0}" },
	{ "hostnameverifier.automation.ignoremismatch", "\u81ea\u52a8\uff1a\u5ffd\u7565\u4e3b\u673a\u540d\u7684\u4e0d\u5339\u914d" },

	{ "trustdecider.check.basicconstraints", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u57fa\u672c\u7ea6\u675f\u5931\u8d25" },
	{ "trustdecider.check.leafkeyusage", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u53f6\u5bc6\u94a5\u4f7f\u7528\u5931\u8d25" },
	{ "trustdecider.check.signerkeyusage", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u7b7e\u540d\u8005\u5bc6\u94a5\u4f7f\u7528\u5931\u8d25" },
	{ "trustdecider.check.extensions", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u5173\u952e\u6269\u5c55\u5931\u8d25" },
	{ "trustdecider.check.signature", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u7b7e\u540d\u5931\u8d25" },
	{ "trustdecider.check.basicconstraints.certtypebit", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1 netscape \u7c7b\u578b\u4f4d\u5931\u8d25" },
	{ "trustdecider.check.basicconstraints.extensionvalue", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1 netscape \u6269\u5c55\u503c\u5931\u8d25" },
	{ "trustdecider.check.basicconstraints.bitvalue", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1 netscape \u4f4d 5\u30016\u30017 \u503c\u5931\u8d25" },
	{ "trustdecider.check.basicconstraints.enduser", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u4f5c\u4e3a CA \u7684\u6700\u7ec8\u7528\u6237\u5931\u8d25" },
	{ "trustdecider.check.basicconstraints.pathlength", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u8def\u5f84\u957f\u5ea6\u7ea6\u675f\u5931\u8d25" },
	{ "trustdecider.check.leafkeyusage.length", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u5bc6\u94a5\u4f7f\u7528\u957f\u5ea6\u5931\u8d25" },
	{ "trustdecider.check.leafkeyusage.digitalsignature", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u6570\u5b57\u7b7e\u540d\u5931\u8d25" },
	{ "trustdecider.check.leafkeyusage.extkeyusageinfo", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u6269\u5c55\u5bc6\u94a5\u4f7f\u7528\u4fe1\u606f\u5931\u8d25" },
	{ "trustdecider.check.leafkeyusage.tsaextkeyusageinfo", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1 TSA \u6269\u5c55\u5bc6\u94a5\u4f7f\u7528\u4fe1\u606f\u5931\u8d25" },
	{ "trustdecider.check.leafkeyusage.certtypebit", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1 netscape \u7c7b\u578b\u4f4d\u5931\u8d25" },
	{ "trustdecider.check.signerkeyusage.lengthandbit", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u957f\u5ea6\u548c\u4f4d\u5931\u8d25" },
	{ "trustdecider.check.signerkeyusage.keyusage", "\u8ba4\u8bc1\u8fc7\u7a0b\u4e2d\u9a8c\u8bc1\u5bc6\u94a5\u4f7f\u7528\u5931\u8d25" },
	{ "trustdecider.check.canonicalize.updatecert", "\u4f7f\u7528 cacerts \u6587\u4ef6\u4e2d\u7684\u8bc1\u4e66\u66f4\u65b0\u6839\u8bc1\u4e66" },
	{ "trustdecider.check.canonicalize.missing", "\u6dfb\u52a0\u7f3a\u5c11\u7684\u6839\u8bc1\u4e66" },
	{ "trustdecider.check.gettrustedcert.find", "\u5728 cacerts \u6587\u4ef6\u4e2d\u67e5\u627e\u6709\u6548\u7684\u6839 CA" },
	{ "trustdecider.check.gettrustedissuercert.find", "\u5728 cacerts \u6587\u4ef6\u4e2d\u67e5\u627e\u7f3a\u5c11\u7684\u6709\u6548\u6839 CA" },
	{ "trustdecider.check.timestamping.no", "\u65e0\u53ef\u7528\u7684\u65f6\u95f4\u6807\u8bb0\u4fe1\u606f" },
	{ "trustdecider.check.timestamping.yes", "\u65f6\u95f4\u6807\u8bb0\u4fe1\u606f\u53ef\u7528" },
	{ "trustdecider.check.timestamping.tsapath", "\u5f00\u59cb\u9a8c\u8bc1 TSA \u8bc1\u4e66\u8def\u5f84" },
	{ "trustdecider.check.timestamping.inca", "\u5373\u4f7f\u8bc1\u4e66\u5df2\u5230\u671f\uff0c\u4f46\u5176\u65f6\u95f4\u6807\u8bb0\u5728\u6709\u6548\u671f\u5185\u5e76\u5177\u6709\u6709\u6548\u7684 TSA" },
	{ "trustdecider.check.timestamping.notinca", "\u8bc1\u4e66\u5df2\u5230\u671f\uff0c\u4f46 TSA \u65e0\u6548" },
	{ "trustdecider.check.timestamping.valid", "\u8bc1\u4e66\u5df2\u5230\u671f\uff0c\u4e14\u5176\u65f6\u95f4\u6807\u8bb0\u5728\u6709\u6548\u671f\u5185" },
	{ "trustdecider.check.timestamping.invalid", "\u8bc1\u4e66\u5df2\u5230\u671f\uff0c\u4e14\u5176\u65f6\u95f4\u6807\u8bb0\u4e0d\u5728\u6709\u6548\u671f\u5185" },
	{ "trustdecider.check.timestamping.need", "\u8bc1\u4e66\u5df2\u5230\u671f\uff0c\u9700\u8981\u9a8c\u8bc1\u65f6\u95f4\u6807\u8bb0\u4fe1\u606f" },
	{ "trustdecider.check.timestamping.noneed", "\u8bc1\u4e66\u5c1a\u672a\u5230\u671f\uff0c\u65e0\u9700\u9a8c\u8bc1\u65f6\u95f4\u6807\u8bb0\u4fe1\u606f" },
	{ "trustdecider.check.timestamping.notfound", "\u672a\u627e\u5230\u65b0\u7684\u65f6\u95f4\u6807\u8bb0 API" },
	{ "trustdecider.user.grant.session", "\u7528\u6237\u53ea\u5bf9\u6b64\u4f1a\u8bdd\u7684\u4ee3\u7801\u6388\u4e88\u6743\u9650" },
	{ "trustdecider.user.grant.forever", "\u7528\u6237\u5bf9\u4ee3\u7801\u6388\u4e88\u6c38\u4e45\u6743\u9650" },
	{ "trustdecider.user.deny", "\u7528\u6237\u62d2\u7edd\u5bf9\u4ee3\u7801\u6388\u4e88\u6743\u9650" },
	{ "trustdecider.automation.trustcert", "\u81ea\u52a8\uff1a\u4fe1\u4efb RSA \u8bc1\u4e66\u8fdb\u884c\u7b7e\u540d" },
	{ "trustdecider.code.type.applet", "\u5c0f\u5e94\u7528\u7a0b\u5e8f" },
	{ "trustdecider.code.type.application", "\u5e94\u7528\u7a0b\u5e8f" },
	{ "trustdecider.code.type.extension", "\u6269\u5c55" },
	{ "trustdecider.code.type.installer", "\u5b89\u88c5\u7a0b\u5e8f" },
	{ "trustdecider.user.cannot.grant.any", "\u60a8\u7684\u5b89\u5168\u914d\u7f6e\u4e0d\u5141\u8bb8\u4e3a\u65b0\u8bc1\u4e66\u6388\u4e88\u6743\u9650" },
	{ "trustdecider.user.cannot.grant.notinca", "\u60a8\u7684\u5b89\u5168\u914d\u7f6e\u4e0d\u5141\u8bb8\u4e3a\u81ea\u884c\u7b7e\u540d\u7684\u8bc1\u4e66\u6388\u4e88\u6743\u9650" },
	{ "x509trustmgr.automation.ignoreclientcert", "\u81ea\u52a8\uff1a\u5ffd\u7565\u4e0d\u53ef\u4fe1\u7684\u5ba2\u6237\u8bc1\u4e66" },
	{ "x509trustmgr.automation.ignoreservercert", "\u81ea\u52a8\uff1a\u5ffd\u7565\u4e0d\u53ef\u4fe1\u7684\u670d\u52a1\u5668\u8bc1\u4e66" },

	{ "net.proxy.text", "\u4ee3\u7406\uff1a" },
	{ "net.proxy.override.text", "\u4ee3\u7406\u8986\u76d6\uff1a" },
	{ "net.proxy.configuration.text", "\u4ee3\u7406\u914d\u7f6e\uff1a" },
	{ "net.proxy.type.browser", "\u6d4f\u89c8\u5668\u4ee3\u7406\u914d\u7f6e" },
	{ "net.proxy.type.auto", "\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e" },
	{ "net.proxy.type.manual", "\u624b\u52a8\u914d\u7f6e" },
	{ "net.proxy.type.none", "\u6ca1\u6709\u4ee3\u7406" },
	{ "net.proxy.type.user", "\u7528\u6237\u5df2\u8986\u76d6\u6d4f\u89c8\u5668\u7684\u4ee3\u7406\u8bbe\u7f6e\u3002" },
	{ "net.proxy.loading.ie", "\u6b63\u5728\u4ece Internet Explorer \u4e2d\u8f7d\u5165\u4ee3\u7406\u914d\u7f6e ..."},
	{ "net.proxy.loading.ns", "\u6b63\u5728\u4ece Netscape Navigator \u4e2d\u8f7d\u5165\u4ee3\u7406\u914d\u7f6e ..."},
	{ "net.proxy.loading.userdef", "\u6b63\u5728\u8f7d\u5165\u7528\u6237\u5b9a\u4e49\u7684\u4ee3\u7406\u914d\u7f6e ..."},
	{ "net.proxy.loading.direct", "\u6b63\u5728\u8f7d\u5165\u76f4\u63a5\u4ee3\u7406\u914d\u7f6e ..."},
	{ "net.proxy.loading.manual", "\u6b63\u5728\u8f7d\u5165\u624b\u52a8\u4ee3\u7406\u914d\u7f6e ..."},
	{ "net.proxy.loading.auto",   "\u6b63\u5728\u8f7d\u5165\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e ..."},
	{ "net.proxy.loading.browser",   "\u6b63\u5728\u8f7d\u5165\u6d4f\u89c8\u5668\u4ee3\u7406\u914d\u7f6e ..."},
	{ "net.proxy.loading.manual.error", "\u65e0\u6cd5\u4f7f\u7528\u624b\u52a8\u4ee3\u7406\u914d\u7f6e - \u540e\u9000\u5230 DIRECT"},
	{ "net.proxy.loading.auto.error", "\u65e0\u6cd5\u4f7f\u7528\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e - \u540e\u9000\u5230 MANUAL"},
	{ "net.proxy.loading.done", "\u5b8c\u6210\u3002"},
	{ "net.proxy.browser.pref.read", "\u6b63\u5728\u4ece {0} \u8bfb\u53d6\u7528\u6237\u9996\u9009\u8bbe\u7f6e\u6587\u4ef6 "},
	{ "net.proxy.browser.proxyEnable", "    \u4ee3\u7406\u542f\u7528\uff1a{0} "},
	{ "net.proxy.browser.proxyList",     "   \u4ee3\u7406\u5217\u8868\uff1a{0}"},
	{ "net.proxy.browser.proxyOverride", "    \u4ee3\u7406\u8986\u76d6\uff1a{0} "},
	{ "net.proxy.browser.autoConfigURL", "    \u81ea\u52a8\u914d\u7f6e URL\uff1a{0} "},
	{ "net.proxy.browser.smartConfig", "\u7528\u7aef\u53e3 {1} Ping \u4ee3\u7406\u670d\u52a1\u5668 {0}"},
        { "net.proxy.browser.connectionException", "\u65e0\u6cd5\u8bbf\u95ee\u7aef\u53e3 {1} \u4e0a\u7684\u4ee3\u7406\u670d\u52a1\u5668 {0}"},
	{ "net.proxy.ns6.regs.exception", "\u8bfb\u53d6\u6ce8\u518c\u6587\u4ef6\u65f6\u51fa\u9519\uff1a{0}"},
	{ "net.proxy.pattern.convert", "\u5c06\u4ee3\u7406\u89c4\u907f\u5217\u8868\u8f6c\u6362\u4e3a\u6b63\u5219\u8868\u8fbe\u5f0f\uff1a"},
	{ "net.proxy.pattern.convert.error", "\u65e0\u6cd5\u5c06\u4ee3\u7406\u89c4\u907f\u5217\u8868\u8f6c\u6362\u4e3a\u6b63\u5219\u8868\u8fbe\u5f0f - \u5ffd\u7565"},
	{ "net.proxy.auto.download.ins", "\u6b63\u5728\u4ece {0} \u4e0b\u8f7d INS \u6587\u4ef6" },
	{ "net.proxy.auto.download.js", "\u6b63\u5728\u4ece {0} \u4e0b\u8f7d\u81ea\u52a8\u4ee3\u7406\u6587\u4ef6" },
	{ "net.proxy.auto.result.error", "\u65e0\u6cd5\u4ece\u8bc4\u4f30\u4e2d\u786e\u5b9a\u4ee3\u7406\u8bbe\u7f6e - \u540e\u9000\u5230 DIRECT"},
        { "net.proxy.service.not_available", "{0} \u7684\u4ee3\u7406\u670d\u52a1\u4e0d\u53ef\u7528 - \u7f3a\u7701\u4e3a DIRECT" },
	{ "net.proxy.error.caption", "\u9519\u8bef - \u4ee3\u7406\u914d\u7f6e" },
	{ "net.proxy.nsprefs.error", "<html><b>\u4e0d\u80fd\u68c0\u7d22\u4ee3\u7406\u8bbe\u7f6e </b></html>\u9000\u56de\u5176\u5b83\u4ee3\u7406\u914d\u7f6e\u3002\n" },
	{ "net.proxy.connect", "\u6b63\u5728\u4f7f\u7528\u4ee3\u7406 {1} \u8fde\u63a5 {0}" },

	{ "net.authenticate.caption", "\u9700\u8981\u53e3\u4ee4 - \u7f51\u7edc"},
	{ "net.authenticate.label", "<html><b>\u8bf7\u8f93\u5165\u7528\u6237\u540d\u548c\u53e3\u4ee4:</b></html>"},
	{ "net.authenticate.resource", "\u8d44\u6e90:" },
	{ "net.authenticate.username", "\u7528\u6237\u540d(U):" },
        { "net.authenticate.username.mnemonic", "VK_U" },
	{ "net.authenticate.password", "\u53e3\u4ee4(P):" },
        { "net.authenticate.password.mnemonic", "VK_P" },
	{ "net.authenticate.firewall", "\u9632\u706b\u5899:" },
	{ "net.authenticate.domain", "\u57df(D):"},
        { "net.authenticate.domain.mnemonic", "VK_D" },
	{ "net.authenticate.realm", "\u9886\u57df\uff1a" },
	{ "net.authenticate.scheme", "\u65b9\u6848:" },
	{ "net.authenticate.unknownSite", "\u672a\u77e5\u7ad9\u70b9" },

	{ "net.cookie.cache", "Cookie \u9ad8\u901f\u7f13\u5b58\uff1a" },
	{ "net.cookie.server", "\u670d\u52a1\u5668 {0} \u8bf7\u6c42\u7528 \"{1}\" \u6267\u884c set-cookie" },
	{ "net.cookie.connect", "\u6b63\u5728\u8fde\u63a5 {0} \u4e0e cookie \"{1}\"" },
	{ "net.cookie.ignore.setcookie", "Cookie \u670d\u52a1\u4e0d\u53ef\u7528 - \u5ffd\u7565 \"Set-Cookie\"" },
	{ "net.cookie.noservice", "Cookie \u670d\u52a1\u4e0d\u53ef\u7528 - \u4f7f\u7528\u9ad8\u901f\u7f13\u5b58\u786e\u5b9a \"Cookie\"" },

	{"about.java.version", "\u7248\u672c {0} (build {1})"},
	{"about.prompt.info", "\u5982\u9700\u5173\u4e8e Java \u6280\u672f\u7684\u66f4\u591a\u8d44\u6599\u4ee5\u53ca\u67e5\u627e\u91cd\u8981\u7684 Java \u5e94\u7528\u7a0b\u5e8f\uff0c\u8bf7\u8bbf\u95ee "},
	{"about.home.link", "http://www.java.com"},
	{"about.option.close", "\u5173\u95ed(C)"},
	{"about.option.close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{"about.copyright", "\u7248\u6743 2004 Sun Microsystems, Inc.\u3002"},
	{"about.legal.note", "\u7248\u6743\u6240\u6709\u3002\u4f7f\u7528\u987b\u53d7\u8bb8\u53ef\u8bc1\u6761\u6b3e\u9650\u5236\u3002"},


	{ "cert.remove_button", "\u5220\u9664(M)" },
        { "cert.remove_button.mnemonic", "VK_M" },
        { "cert.import_button", "\u8f93\u5165(I)" },
        { "cert.import_button.mnemonic", "VK_I" },
        { "cert.export_button", "\u8f93\u51fa(E)" },
        { "cert.export_button.mnemonic", "VK_E" },
        { "cert.details_button", "\u8be6\u7ec6\u4fe1\u606f(D)" },
        { "cert.details_button.mnemonic", "VK_D" },
        { "cert.viewcert_button", "\u67e5\u770b\u8bc1\u4e66(V)" },
        { "cert.viewcert_button.mnemonic", "VK_V" },
        { "cert.close_button", "\u5173\u95ed(C)" },
        { "cert.close_button.mnemonic", "VK_C" },
        { "cert.type.trusted_certs", "\u53ef\u4fe1\u7684\u8bc1\u4e66" },
        { "cert.type.secure_site", "\u5b89\u5168\u7ad9\u70b9" },
        { "cert.type.client_auth", "\u5ba2\u6237\u9a8c\u8bc1" },
        { "cert.type.signer_ca", "\u7b7e\u540d\u8005 CA" },
        { "cert.type.secure_site_ca", "\u5b89\u5168\u7ad9\u70b9 CA" },
        { "cert.rbutton.user", "\u7528\u6237" },
        { "cert.rbutton.system", "\u7cfb\u7edf" },
        { "cert.settings", "\u8bc1\u4e66" },
        { "cert.dialog.import.error.caption", "\u9519\u8bef - \u8f93\u5165\u8bc1\u4e66" },
        { "cert.dialog.export.error.caption", "\u9519\u8bef - \u8f93\u51fa\u8bc1\u4e66" },
	{ "cert.dialog.import.format.text", "<html><b>\u65e0\u6cd5\u8bc6\u522b\u6587\u4ef6\u683c\u5f0f</b></html>\u5c06\u4e0d\u8f93\u5165\u4efb\u4f55\u8bc1\u4e66\u3002" },
	{ "cert.dialog.export.password.text", "<html><b>\u53e3\u4ee4\u65e0\u6548</b></html>\u60a8\u8f93\u5165\u7684\u53e3\u4ee4\u4e0d\u6b63\u786e\u3002" },
	{ "cert.dialog.import.file.text", "<html><b>\u6587\u4ef6\u4e0d\u5b58\u5728</b></html>\u5c06\u4e0d\u8f93\u5165\u4efb\u4f55\u8bc1\u4e66\u3002" },
	{ "cert.dialog.import.password.text", "<html><b>\u53e3\u4ee4\u65e0\u6548</b></html>\u60a8\u8f93\u5165\u7684\u53e3\u4ee4\u4e0d\u6b63\u786e\u3002" },
        { "cert.dialog.password.caption", "\u53e3\u4ee4" },
        { "cert.dialog.password.import.caption", "\u9700\u8981\u53e3\u4ee4 - \u8f93\u5165" },
        { "cert.dialog.password.export.caption", "\u9700\u8981\u53e3\u4ee4 - \u8f93\u51fa" },
        { "cert.dialog.password.text", "\u8f93\u5165\u53e3\u4ee4\u4ee5\u8bbf\u95ee\u6b64\u6587\u4ef6:\n" },
        { "cert.dialog.exportpassword.text", "\u8f93\u5165\u53e3\u4ee4\u4ee5\u8bbf\u95ee\u5ba2\u6237\u9a8c\u8bc1\u5bc6\u94a5\u5e93\u4e2d\u7684\u79c1\u6709\u5bc6\u94a5:\n" },
        { "cert.dialog.savepassword.text", "\u8f93\u5165\u53e3\u4ee4\u4ee5\u4fdd\u5b58\u6b64\u5bc6\u94a5\u6587\u4ef6:\n" },
        { "cert.dialog.password.okButton", "\u786e\u5b9a" },
        { "cert.dialog.password.cancelButton", "\u53d6\u6d88" },
        { "cert.dialog.export.error.caption", "\u9519\u8bef - \u8f93\u51fa\u8bc1\u4e66" },
        { "cert.dialog.export.text", "<html><b>\u65e0\u6cd5\u8f93\u51fa</b></html>\u672a\u8f93\u51fa\u4efb\u4f55\u8bc1\u4e66\u3002" },
        { "cert.dialog.remove.text", "\u662f\u5426\u786e\u5b9e\u8981\u5220\u9664\u8bc1\u4e66\uff1f" },
	{ "cert.dialog.remove.caption", "\u5220\u9664\u8bc1\u4e66" },
	{ "cert.dialog.issued.to", "\u53d1\u653e\u7ed9" },
	{ "cert.dialog.issued.by", "\u53d1\u653e\u8005" },
	{ "cert.dialog.user.level", "\u7528\u6237" },
	{ "cert.dialog.system.level", "\u7cfb\u7edf" },
	{ "cert.dialog.certtype", "\u8bc1\u4e66\u7c7b\u578b: "},

	{ "controlpanel.jre.platformTableColumnTitle","\u5e73\u53f0"},
	{ "controlpanel.jre.productTableColumnTitle","\u4ea7\u54c1" },
	{ "controlpanel.jre.locationTableColumnTitle","\u4f4d\u7f6e" },
	{ "controlpanel.jre.pathTableColumnTitle","\u8def\u5f84" },
	{ "controlpanel.jre.enabledTableColumnTitle", "\u542f\u7528" },

	{ "jnlp.jre.title", "JNLP Runtime \u8bbe\u7f6e" },
	{ "jnlp.jre.versions", "Java Runtime \u7248\u672c" },
	{ "jnlp.jre.choose.button", "\u9009\u62e9(H)" },
	{ "jnlp.jre.find.button", "\u67e5\u627e(F)" },
	{ "jnlp.jre.add.button", "\u6dfb\u52a0(A)" },
	{ "jnlp.jre.remove.button", "\u5220\u9664(R)" },
	{ "jnlp.jre.ok.button", "\u786e\u5b9a(O)" },
	{ "jnlp.jre.cancel.button", "\u53d6\u6d88(C)" },
	{ "jnlp.jre.choose.button.mnemonic", "VK_H" },
	{ "jnlp.jre.find.button.mnemonic", "VK_F" },
	{ "jnlp.jre.add.button.mnemonic", "VK_A" },
	{ "jnlp.jre.remove.button.mnemonic", "VK_R" },
	{ "jnlp.jre.ok.button.mnemonic", "VK_O" },
	{ "jnlp.jre.cancel.button.mnemonic", "VK_C" },

	{ "find.dialog.title", "JRE \u641c\u7d22\u5668"},
	{ "find.title", "Java Runtime Environment"},
	{ "find.cancelButton", "\u53d6\u6d88(C)"},
	{ "find.prevButton", "\u4e0a\u4e00\u6b65(P)"},
	{ "find.nextButton", "\u4e0b\u4e00\u6b65(N)"},
	{ "find.cancelButtonMnemonic", "VK_C"},
	{ "find.prevButtonMnemonic", "VK_P"},
	{ "find.nextButtonMnemonic", "VK_N"},
	{ "find.intro", "\u4e3a\u4e86\u542f\u52a8\u5e94\u7528\u7a0b\u5e8f\uff0cJava Web Start \u9700\u8981\u77e5\u9053 Java Runtime Environment \u7684\u5b89\u88c5\u4f4d\u7f6e\u3002\n\n\u60a8\u53ef\u4ee5\u9009\u62e9\u4e00\u4e2a\u5df2\u77e5\u7684 JRE\uff0c\u6216\u4ece\u6587\u4ef6\u7cfb\u7edf\u4e2d\u9009\u62e9\u53ef\u4ee5\u641c\u7d22\u5230 JRE \u7684\u76ee\u5f55\u3002" },

	{ "find.searching.title", "\u641c\u7d22\u53ef\u7528\u7684 JRE\uff0c\u8fd9\u53ef\u80fd\u9700\u8981\u6301\u7eed\u51e0\u5206\u949f\u65f6\u95f4\u3002" },
	{ "find.searching.prefix", "\u6b63\u5728\u68c0\u67e5:" },
	{ "find.foundJREs.title", "\u627e\u5230\u4ee5\u4e0b JRE\uff0c\u8bf7\u5355\u51fb\u201c\u4e0b\u4e00\u6b65\u201d\u6dfb\u52a0\u5b83\u4eec" },
	{ "find.noJREs.title", "\u65e0\u6cd5\u627e\u5230\u4efb\u4f55 JRE\uff0c\u8bf7\u5355\u51fb\u201c\u4e0a\u4e00\u6b65\u201d\u9009\u62e9\u53e6\u4e00\u4e2a\u641c\u7d22\u4f4d\u7f6e" },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Java(tm) \u90e8\u7f72\u5c5e\u6027\n"
                        + "# \u8bf7\u52ff\u7f16\u8f91\u672c\u6587\u4ef6\u3002\u5b83\u662f\u7531\u8ba1\u7b97\u673a\u751f\u6210\u7684\u3002\n"
                        + "# \u8bf7\u4f7f\u7528 Java \u63a7\u5236\u9762\u677f\u7f16\u8f91\u8fd9\u4e9b\u5c5e\u6027\u3002" },
        { "config.unknownSubject", "\u672a\u77e5\u4e3b\u9898" },
        { "config.unknownIssuer", "\u672a\u77e5\u53d1\u653e\u8005" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>URL \u683c\u5f0f\u9519\u8bef</b></html>\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e URL \u65e0\u6548\u3002" },
        { "config.proxy.autourl.invalid.caption", "\u9519\u8bef - \u4ee3\u7406" },
	// Java Web Start Properties
	 { "APIImpl.clipboard.message.read", "\u672c\u5e94\u7528\u7a0b\u5e8f\u5df2\u7ecf\u8bf7\u6c42\u5bf9\u7cfb\u7edf\u526a\u8d34\u677f\u7684\u53ea\u8bfb\u8bbf\u95ee\u3002\u5e94\u7528\u7a0b\u5e8f\u53ef\u4ee5\u8bbf\u95ee\u526a\u8d34\u677f\u4e0a\u5b58\u50a8\u7684\u673a\u5bc6\u4fe1\u606f\u3002\u662f\u5426\u5141\u8bb8\u8be5\u64cd\u4f5c\uff1f" },
        { "APIImpl.clipboard.message.write", "\u672c\u5e94\u7528\u7a0b\u5e8f\u5df2\u7ecf\u8bf7\u6c42\u5bf9\u7cfb\u7edf\u526a\u8d34\u677f\u7684\u5199\u5165\u8bbf\u95ee\u3002\u5e94\u7528\u7a0b\u5e8f\u53ef\u80fd\u8986\u76d6\u526a\u8d34\u677f\u4e0a\u5b58\u50a8\u7684\u4fe1\u606f\u3002\u662f\u5426\u5141\u8bb8\u8be5\u64cd\u4f5c\uff1f" },
        { "APIImpl.file.open.message", "\u672c\u5e94\u7528\u7a0b\u5e8f\u5df2\u7ecf\u8bf7\u6c42\u5bf9\u6587\u4ef6\u7cfb\u7edf\u7684\u8bfb\u53d6\u8bbf\u95ee\u3002\u5e94\u7528\u7a0b\u5e8f\u53ef\u4ee5\u8bbf\u95ee\u6587\u4ef6\u7cfb\u7edf\u4e2d\u5b58\u50a8\u7684\u673a\u5bc6\u4fe1\u606f\u3002\u662f\u5426\u5141\u8bb8\u8be5\u64cd\u4f5c\uff1f" },
        { "APIImpl.file.save.fileExist", "{0} \u5df2\u5b58\u5728\u3002\n\u662f\u5426\u8981\u66ff\u6362\uff1f" },
        { "APIImpl.file.save.fileExistTitle", "\u6587\u4ef6\u5df2\u5b58\u5728" },
        { "APIImpl.file.save.message", "\u672c\u5e94\u7528\u7a0b\u5e8f\u5df2\u7ecf\u8bf7\u6c42\u5bf9\u672c\u5730\u6587\u4ef6\u7cfb\u7edf\u4e2d\u6587\u4ef6\u7684\u8bfb\u5199\u8bbf\u95ee\u3002\u5141\u8bb8\u8be5\u64cd\u4f5c\u5c06\u53ea\u8d4b\u4e88\u5e94\u7528\u7a0b\u5e8f\u5bf9\u4e0b\u9762\u6587\u4ef6\u5bf9\u8bdd\u6846\u4e2d\u9009\u5b9a\u6587\u4ef6\u7684\u8bbf\u95ee\u3002\u662f\u5426\u5141\u8bb8\u8be5\u64cd\u4f5c\uff1f" },
        { "APIImpl.persistence.accessdenied", "\u62d2\u7edd\u8bbf\u95ee URL {0} \u7684\u6c38\u4e45\u5b58\u50a8\u5668" },
        { "APIImpl.persistence.filesizemessage", "\u8d85\u51fa\u6700\u5927\u6587\u4ef6\u957f\u5ea6" },
        { "APIImpl.persistence.message", "\u672c\u5e94\u7528\u7a0b\u5e8f\u5df2\u7ecf\u8bf7\u6c42\u9644\u52a0\u7684\u672c\u5730\u78c1\u76d8\u5b58\u50a8\u7a7a\u95f4\u3002\u5f53\u524d\u6700\u5927\u5206\u914d\u7a7a\u95f4\u4e3a {1} \u5b57\u8282\u3002\u6b64\u5e94\u7528\u7a0b\u5e8f\u8bf7\u6c42\u5c06\u6700\u5927\u5206\u914d\u7a7a\u95f4\u589e\u52a0\u81f3 {0} \u5b57\u8282\u3002\u662f\u5426\u5141\u8bb8\u8be5\u64cd\u4f5c\uff1f" },
        { "APIImpl.print.message", "\u672c\u5e94\u7528\u7a0b\u5e8f\u5df2\u7ecf\u8bf7\u6c42\u8bbf\u95ee\u9ed8\u8ba4\u6253\u5370\u673a\u3002\u5141\u8bb8\u8be5\u64cd\u4f5c\u5c06\u8d4b\u4e88\u5e94\u7528\u7a0b\u5e8f\u5bf9\u6253\u5370\u673a\u7684\u5199\u5165\u8bbf\u95ee\u3002\u662f\u5426\u5141\u8bb8\u8be5\u64cd\u4f5c\uff1f" },
	{ "APIImpl.extended.fileOpen.message1", "\u672c\u5e94\u7528\u7a0b\u5e8f\u5df2\u7ecf\u8bf7\u6c42\u8d4b\u4e88\u5bf9\u672c\u5730\u6587\u4ef6\u7cfb\u7edf\u4e2d\u4ee5\u4e0b\u6587\u4ef6\u7684\u8bfb/\u5199\u8bbf\u95ee\u6743\u9650\u3002"},
	{ "APIImpl.extended.fileOpen.message2", "\u5141\u8bb8\u6b64\u64cd\u4f5c\u5c06\u53ea\u8d4b\u4e88\u5e94\u7528\u7a0b\u5e8f\u5bf9\u4e0a\u9762\u5217\u51fa\u7684\u6587\u4ef6\u7684\u8bbf\u95ee\u6743\u9650\u3002\u662f\u5426\u5141\u8bb8\u6b64\u64cd\u4f5c\uff1f"},
        { "APIImpl.securityDialog.no", "\u5426" },
        { "APIImpl.securityDialog.remember", "\u8bf7\u4e0d\u8981\u518d\u663e\u793a\u8be5\u5efa\u8bae" },
        { "APIImpl.securityDialog.title", "\u5b89\u5168\u5efa\u8bae" },
        { "APIImpl.securityDialog.yes", "\u662f" },
        { "Launch.error.installfailed", "\u5b89\u88c5\u5931\u8d25" },
        { "aboutBox.closeButton", "\u5173\u95ed" },
        { "aboutBox.versionLabel", "\u7248\u672c {0} (build {1})" },
        { "applet.failedToStart", "\u542f\u52a8\u5c0f\u5e94\u7528\u7a0b\u5e8f\u5931\u8d25\uff1a{0}" },
        { "applet.initializing", "\u6b63\u5728\u521d\u59cb\u5316\u5c0f\u5e94\u7528\u7a0b\u5e8f" },
        { "applet.initializingFailed", "\u521d\u59cb\u5316\u5c0f\u5e94\u7528\u7a0b\u5e8f\u5931\u8d25\uff1a{0}" },
        { "applet.running", "\u6b63\u5728\u8fd0\u884c..." },
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "\u5fc5\u987b\u91cd\u65b0\u542f\u52a8 Windows\uff0c\u624d\u80fd\u4f7f\u66f4\u6539\u751f\u6548\u3002\n\n\u662f\u5426\u7acb\u5373\u91cd\u65b0\u542f\u52a8 Windows\uff1f" },
        { "extensionInstall.rebootTitle", "\u91cd\u65b0\u542f\u52a8 Windows" }, 
        { "install.configButton", "\u914d\u7f6e(C)..." },
        { "install.configMnemonic", "VK_C" },
        { "install.errorInstalling", "\u5c1d\u8bd5\u5b89\u88c5 Java Runtime Environment \u65f6\u53d1\u751f\u610f\u5916\u9519\u8bef\uff0c\u8bf7\u91cd\u8bd5\u3002" },
        { "install.errorRestarting", "\u542f\u52a8\u65f6\u53d1\u751f\u610f\u5916\u9519\u8bef\uff0c\u8bf7\u91cd\u8bd5\u3002" },
        { "install.title", "{0} - \u521b\u5efa\u5feb\u6377\u65b9\u5f0f" },

        { "install.windows.both.message", "\u662f\u5426\u8981\u4e3a {0} \n\u521b\u5efa\u684c\u9762\u548c\u5f00\u59cb\u83dc\u5355\u5feb\u6377\u65b9\u5f0f?" },
	{ "install.gnome.both.message", "\u662f\u5426\u8981\u4e3a {0} \n\u521b\u5efa\u684c\u9762\u548c\u5e94\u7528\u7a0b\u5e8f\u83dc\u5355\u5feb\u6377\u65b9\u5f0f?" },
	{ "install.desktop.message", "\u662f\u5426\u8981\u4e3a {0} \n\u521b\u5efa\u684c\u9762\u5feb\u6377\u65b9\u5f0f?" },
	{ "install.windows.menu.message", "\u662f\u5426\u8981\u4e3a {0} \n\u521b\u5efa\u5f00\u59cb\u83dc\u5355\u5feb\u6377\u65b9\u5f0f?" },
	{ "install.gnome.menu.message", "\u662f\u5426\u8981\u4e3a {0} \n\u521b\u5efa\u5e94\u7528\u7a0b\u5e8f\u83dc\u5355\u5feb\u6377\u65b9\u5f0f?" },
        { "install.noButton", "\u5426(N)" },
        { "install.noMnemonic", "VK_N" },
        { "install.yesButton", "\u662f(Y)" },
        { "install.yesMnemonic", "VK_Y" },
        { "launch.cancel", "\u53d6\u6d88" },
        { "launch.downloadingJRE", "\u6b63\u5728\u4ece {1} \u8bf7\u6c42 JRE {0}" },
        { "launch.error.badfield", "\u4e0b\u5217\u5b57\u6bb5 {0} \u7684\u503c\u65e0\u6548\uff1a{1}" },
        { "launch.error.badfield-signedjnlp", "\u4e0b\u5217\u7b7e\u540d\u542f\u52a8\u6587\u4ef6\u4e2d\u5b57\u6bb5 {0} \u7684\u503c\u65e0\u6548\uff1a{1}" },
        { "launch.error.badfield.download.https", "\u65e0\u6cd5\u901a\u8fc7 HTTPS \u4e0b\u8f7d" },
        { "launch.error.badfield.https", "Java 1.4+ \u662f\u5b9e\u73b0 HTTPS \u652f\u6301\u6240\u5fc5\u9700\u7684" },
        { "launch.error.badjarfile", "\u635f\u574f\u7684 JAR \u6587\u4ef6\u4f4d\u4e8e {0}" },
        { "launch.error.badjnlversion", "\u542f\u52a8\u6587\u4ef6 {0} \u4e2d\u6709\u4e0d\u652f\u6301\u7684 JNLP \u7248\u672c\u3002\u672c\u7248\u672c\u53ea\u652f\u6301 1.0 \u7248\u672c\u3002\u8bf7\u8054\u7cfb\u5e94\u7528\u7a0b\u5e8f\u4f9b\u5e94\u5546\u53cd\u6620\u8be5\u95ee\u9898\u3002" },
        { "launch.error.badmimetyperesponse", "\u5728\u8bbf\u95ee\u8d44\u6e90 {0} - {1} \u65f6\u4ece\u670d\u52a1\u5668\u8fd4\u56de\u9519\u8bef\u7684 MIME \u7c7b\u578b" },
        { "launch.error.badsignedjnlp", "\u9a8c\u8bc1\u542f\u52a8\u6587\u4ef6\u7b7e\u540d\u6709\u6548\u6027\u5931\u8d25\u3002\u7b7e\u540d\u7684\u7248\u672c\u4e0e\u4e0b\u8f7d\u7684\u7248\u672c\u4e0d\u5339\u914d\u3002" },
        { "launch.error.badversionresponse", "\u5728\u8bbf\u95ee\u8d44\u6e90 {0} - {1} \u65f6\u6765\u81ea\u670d\u52a1\u5668\u8fd4\u56de\u7684\u54cd\u5e94\u4e2d\u7684\u7248\u672c\u65e0\u6548" },
        { "launch.error.canceledloadingresource", "\u7528\u6237\u53d6\u6d88\u5bf9\u8d44\u6e90 {0} \u7684\u52a0\u8f7d" },
        { "launch.error.category.arguments", "\u65e0\u6548\u53c2\u6570\u9519\u8bef" },
        { "launch.error.category.download", "\u4e0b\u8f7d\u9519\u8bef" },
        { "launch.error.category.launchdesc", "\u542f\u52a8\u6587\u4ef6\u9519\u8bef" },
        { "launch.error.category.memory", "\u5185\u5b58\u4e0d\u8db3\u9519\u8bef" },
        { "launch.error.category.security", "\u5b89\u5168\u9519\u8bef" },
        { "launch.error.category.config", "\u7cfb\u7edf\u914d\u7f6e" },
        { "launch.error.category.unexpected", "\u610f\u5916\u9519\u8bef" },
        { "launch.error.couldnotloadarg", "\u65e0\u6cd5\u52a0\u8f7d\u6307\u5b9a\u7684\u6587\u4ef6/URL\uff1a{0}" },
        { "launch.error.errorcoderesponse-known", "\u5728\u8bbf\u95ee\u8d44\u6e90 {0} \u65f6\u4ece\u670d\u52a1\u5668\u8fd4\u56de\u9519\u8bef\u4ee3\u7801 {1} ({2})" },
        { "launch.error.errorcoderesponse-unknown", "\u5728\u8bbf\u95ee\u8d44\u6e90 {0} \u65f6\u4ece\u670d\u52a1\u5668\u8fd4\u56de\u9519\u8bef\u4ee3\u7801 99\uff08\u672a\u77e5\u9519\u8bef\uff09" },
        { "launch.error.failedexec", "\u65e0\u6cd5\u542f\u52a8 Java Runtime Environment {0} \u7248" },
        { "launch.error.failedloadingresource", "\u65e0\u6cd5\u52a0\u8f7d\u8d44\u6e90\uff1a{0}" },
        { "launch.error.invalidjardiff", "\u65e0\u6cd5\u5e94\u7528\u8d44\u6e90 {0} \u7684\u589e\u91cf\u5f0f\u66f4\u65b0" },
        { "launch.error.jarsigning-badsigning", "\u65e0\u6cd5\u9a8c\u8bc1\u8d44\u6e90 {0} \u4e2d\u7684\u7b7e\u540d" },
        { "launch.error.jarsigning-missingentry", "\u8d44\u6e90 {0} \u4e2d\u7f3a\u5c11\u7b7e\u540d\u9879" },
        { "launch.error.jarsigning-missingentryname", "\u7f3a\u5c11\u7b7e\u540d\u9879\uff1a{0}" },
        { "launch.error.jarsigning-multicerts", "\u7b7e\u7f72\u8d44\u6e90 {0} \u4f7f\u7528\u591a\u4e2a\u8bc1\u4e66" },
        { "launch.error.jarsigning-multisigners", "\u4e0b\u5217\u8d44\u6e90\u4e2d\u7684\u9879\u5177\u6709\u591a\u4e2a\u7b7e\u540d\uff1a{0}" },
        { "launch.error.jarsigning-unsignedfile", "\u8d44\u6e90 {0} \u4e2d\u53d1\u73b0\u672a\u7b7e\u540d\u9879" },
        { "launch.error.missingfield", "\u542f\u52a8\u6587\u4ef6\u4e2d\u7f3a\u5c11\u4e0b\u5217\u5fc5\u9700\u5b57\u6bb5\uff1a{0}" },
        { "launch.error.missingfield-signedjnlp", "\u7b7e\u540d\u7684\u542f\u52a8\u6587\u4ef6\u4e2d\u7f3a\u5c11\u4e0b\u5217\u5fc5\u9700\u5b57\u6bb5\uff1a{0}" },
        { "launch.error.missingjreversion", "\u542f\u52a8\u6587\u4ef6\u4e2d\u6ca1\u6709\u672c\u7cfb\u7edf\u7684 JRE \u7248\u672c" },
        { "launch.error.missingversionresponse", "\u5728\u8bbf\u95ee\u8d44\u6e90 {0} \u65f6\u670d\u52a1\u5668\u7684\u54cd\u5e94\u4e2d\u7f3a\u5c11\u7248\u672c\u5b57\u6bb5" },
        { "launch.error.multiplehostsreferences", "\u8d44\u6e90\u4e2d\u5f15\u7528\u591a\u4e2a\u4e3b\u673a" },
        { "launch.error.nativelibviolation", "\u4f7f\u7528\u672c\u5730\u5e93\u9700\u8981\u5bf9\u7cfb\u7edf\u8fdb\u884c\u65e0\u9650\u5236\u8bbf\u95ee" },
        { "launch.error.noJre", "\u5e94\u7528\u7a0b\u5e8f\u8bf7\u6c42\u4e86\u5f53\u524d\u672a\u672c\u5730\u5b89\u88c5\u7684 JRE \u7248\u672c\u3002Java Web Start \u4e0d\u80fd\u81ea\u52a8\u4e0b\u8f7d\u5e76\u5b89\u88c5\u6240\u9700\u7684\u7248\u672c\u3002JRE \u7248\u672c\u5fc5\u987b\u624b\u52a8\u5b89\u88c5\u3002" },
        { "launch.error.wont.download.jre", "\u5e94\u7528\u7a0b\u5e8f\u8bf7\u6c42\u4e86\u5f53\u524d\u672a\u5728\u672c\u5730\u5b89\u88c5\u7684 JRE \u7248\u672c (\u7248\u672c {0})\u3002Java Web Start \u4e0d\u80fd\u81ea\u52a8\u4e0b\u8f7d\u5e76\u5b89\u88c5\u6240\u9700\u7684\u7248\u672c\u3002JRE \u7248\u672c\u5fc5\u987b\u624b\u52a8\u5b89\u88c5\u3002" },
        { "launch.error.cant.download.jre", "\u5e94\u7528\u7a0b\u5e8f\u8bf7\u6c42\u4e86\u5f53\u524d\u672a\u5728\u672c\u5730\u5b89\u88c5\u7684 JRE \u7248\u672c (\u7248\u672c {0})\u3002Java Web Start \u4e0d\u80fd\u81ea\u52a8\u4e0b\u8f7d\u5e76\u5b89\u88c5\u6240\u9700\u7684\u7248\u672c\u3002JRE \u7248\u672c\u5fc5\u987b\u624b\u52a8\u5b89\u88c5\u3002" },
        { "launch.error.cant.access.system.cache", "\u5f53\u524d\u7528\u6237\u6ca1\u6709\u5bf9\u7cfb\u7edf\u7f13\u5b58\u7684\u5199\u5165\u8bbf\u95ee\u6743\u9650\u3002" },
        { "launch.error.cant.access.user.cache", "\u5f53\u524d\u7528\u6237\u6ca1\u6709\u5bf9\u7f13\u5b58\u7684\u5199\u5165\u8bbf\u95ee\u6743\u9650\u3002" },
        { "launch.error.noappresources", "\u6ca1\u6709\u4e3a\u8be5\u5e73\u53f0\u6307\u5b9a\u5e94\u7528\u7a0b\u5e8f\u8d44\u6e90\u3002\u8bf7\u8054\u7cfb\u5e94\u7528\u7a0b\u5e8f\u4f9b\u5e94\u5546\u786e\u8ba4\u662f\u5426\u652f\u6301\u8be5\u5e73\u53f0\u3002" },
        { "launch.error.nomainclass", "\u5728 {1} \u4e2d\u627e\u4e0d\u5230\u4e3b\u7c7b {0}" },
        { "launch.error.nomainclassspec", "\u672a\u6307\u5b9a\u5e94\u7528\u7a0b\u5e8f\u7684\u4e3b\u7c7b" },
        { "launch.error.nomainjar", "\u672a\u6307\u5b9a\u4e3b JAR \u6587\u4ef6" },
        { "launch.error.nonstaticmainmethod", "main() \u65b9\u6cd5\u5fc5\u987b\u4e3a\u9759\u6001" },
        { "launch.error.offlinemissingresource", "\u7531\u4e8e\u6ca1\u6709\u5c06\u6240\u6709\u6240\u9700\u7684\u8d44\u6e90\u4e0b\u8f7d\u5230\u672c\u5730\uff0c\u56e0\u6b64\u5e94\u7528\u7a0b\u5e8f\u4e0d\u80fd\u79bb\u7ebf\u8fd0\u884c" },
        { "launch.error.parse", "\u65e0\u6cd5\u89e3\u6790\u542f\u52a8\u6587\u4ef6\u3002\u7b2c {0, number} \u884c\u9519\u8bef\u3002" },
        { "launch.error.parse-signedjnlp", "\u65e0\u6cd5\u89e3\u6790\u7b7e\u540d\u7684\u542f\u52a8\u6587\u4ef6\u3002\u7b2c {0,number} \u884c\u9519\u8bef\u3002" },
        { "launch.error.resourceID", "{0}" },
        { "launch.error.resourceID-version", "({0}, {1})" },
        { "launch.error.singlecertviolation", "JNLP \u6587\u4ef6\u4e2d\u7684 JAR \u8d44\u6e90\u7b7e\u540d\u8bc1\u4e66\u4e0d\u4e00\u81f4" },
        { "launch.error.toomanyargs", "\u53c2\u6570\u592a\u591a\uff1a{0}" },
        { "launch.error.unsignedAccessViolation", "\u672a\u7b7e\u540d\u7684\u5e94\u7528\u7a0b\u5e8f\u8bf7\u6c42\u5bf9\u7cfb\u7edf\u8fdb\u884c\u65e0\u9650\u5236\u8bbf\u95ee" },
        { "launch.error.unsignedResource", "\u672a\u7b7e\u540d\u7684\u8d44\u6e90\uff1a{0}" },
        { "launch.estimatedTimeLeft", "\u4f30\u8ba1\u5269\u4f59\u65f6\u95f4\uff1a{0,number,00}:{1,number,00}:{2,number,00}" },
        { "launch.extensiondownload", "\u6b63\u5728\u4e0b\u8f7d\u6269\u5c55\u63cf\u8ff0\u7b26\uff08\u5269\u4f59 {0}\uff09" },
        { "launch.extensiondownload-name", "\u6b63\u5728\u4e0b\u8f7d {0} \u63cf\u8ff0\u7b26\uff08\u5269\u4f59 {1}\uff09" },
        { "launch.initializing", "\u6b63\u5728\u521d\u59cb\u5316..." },
        { "launch.launchApplication", "\u542f\u52a8\u5e94\u7528\u7a0b\u5e8f..." },
        { "launch.launchInstaller", "\u542f\u52a8\u5b89\u88c5\u7a0b\u5e8f..." },
        { "launch.launchingExtensionInstaller", "\u6b63\u5728\u8fd0\u884c\u5b89\u88c5\u7a0b\u5e8f\u3002\u8bf7\u7a0d\u5019..." },
        { "launch.loadingNetProgress", "\u8bfb\u53d6 {0}" },
        { "launch.loadingNetProgressPercent", "\u8bfb\u53d6 {1} \u4e2d\u7684 {0} ({2}%)" },
        { "launch.loadingNetStatus", "\u4ece {1} \u52a0\u8f7d {0}" },
        { "launch.loadingResourceFailed", "\u52a0\u8f7d\u8d44\u6e90\u5931\u8d25" },
        { "launch.loadingResourceFailedSts", "\u8bf7\u6c42\u7684 {0}" },
        { "launch.patchingStatus", "\u4fee\u8865\u6765\u81ea {1} \u7684 {0}" },
        { "launch.progressScreen", "\u68c0\u67e5\u6700\u65b0\u7248\u672c..." },
        { "launch.stalledDownload", "\u7b49\u5f85\u6570\u636e..." },
        { "launch.validatingProgress", "\u6b63\u5728\u626b\u63cf\u9879\uff08\u5df2\u5b8c\u6210 {0}%\uff09" },
        { "launch.validatingStatus", "\u9a8c\u8bc1\u6765\u81ea {1} \u7684 {0}" },
        { "launcherrordialog.abort", "\u653e\u5f03(A)" },
        { "launcherrordialog.abortMnemonic", "VK_A" },
        { "launcherrordialog.brief.continue", "\u65e0\u6cd5\u7ee7\u7eed\u6267\u884c" },
        { "launcherrordialog.brief.details", "\u8be6\u7ec6\u4fe1\u606f" },
        { "launcherrordialog.brief.message", "\u65e0\u6cd5\u542f\u52a8\u6307\u5b9a\u7684\u5e94\u7528\u7a0b\u5e8f\u3002" },
	{ "launcherrordialog.import.brief.message", "\u65e0\u6cd5\u5bfc\u5165\u6307\u5b9a\u7684\u5e94\u7528\u7a0b\u5e8f\u3002" },
        { "launcherrordialog.brief.messageKnown", "\u65e0\u6cd5\u542f\u52a8 {0}\u3002" },
	{ "launcherrordialog.import.brief.messageKnown", "\u65e0\u6cd5\u542f\u52a8 {0}\u3002" },
        { "launcherrordialog.brief.ok", "\u786e\u5b9a" },
        { "launcherrordialog.brief.title", "Java Web Start - {0}" },
        { "launcherrordialog.consoleTab", "\u63a7\u5236\u53f0" },
        { "launcherrordialog.errorcategory", "\u7c7b\u522b\uff1a{0}\n\n" },
        { "launcherrordialog.errorintro", "\u542f\u52a8/\u8fd0\u884c\u5e94\u7528\u7a0b\u5e8f\u65f6\u53d1\u751f\u9519\u8bef\u3002\n\n" },
	{ "launcherrordialog.import.errorintro", "\u5bfc\u5165\u5e94\u7528\u7a0b\u5e8f\u65f6\u53d1\u751f\u9519\u8bef\u3002\n\n" },
        { "launcherrordialog.errormsg", "{0}" },
        { "launcherrordialog.errortitle", "\u6807\u9898\uff1a{0}\n" },
        { "launcherrordialog.errorvendor", "\u4f9b\u5e94\u5546\uff1a{0}\n" },
        { "launcherrordialog.exceptionTab", "\u5f02\u5e38" },
        { "launcherrordialog.generalTab", "\u5e38\u89c4" },
        { "launcherrordialog.genericerror", "\u610f\u5916\u5f02\u5e38\uff1a{0}" },
        { "launcherrordialog.jnlpMainTab", "\u4e3b\u542f\u52a8\u6587\u4ef6" },
        { "launcherrordialog.jnlpTab", "\u542f\u52a8\u6587\u4ef6" },
        { "launcherrordialog.title", "Java Web Start - {0}" },
        { "launcherrordialog.wrappedExceptionTab", "\u5c01\u88c5\u5f02\u5e38" },

        { "uninstall.failedMessage", "\u65e0\u6cd5\u5b8c\u5168\u5378\u8f7d\u5e94\u7528\u7a0b\u5e8f\u3002" },
        { "uninstall.failedMessageTitle", "\u5378\u8f7d" },
        { "install.alreadyInstalled", "\u5df2\u7ecf\u6709\u4e00\u4e2a {0} \u7684\u5feb\u6377\u65b9\u5f0f\u4e86\u3002\u662f\u5426\u7ee7\u7eed\u521b\u5efa\u5feb\u6377\u65b9\u5f0f\uff1f" },
        { "install.alreadyInstalledTitle", "\u521b\u5efa\u5feb\u6377\u65b9\u5f0f..." },
        { "install.desktopShortcutName", "{0}" },
        { "install.installFailed", "\u65e0\u6cd5\u521b\u5efa {0} \u7684\u5feb\u6377\u65b9\u5f0f\u3002" },
        { "install.installFailedTitle", "\u521b\u5efa\u5feb\u6377\u65b9\u5f0f" },
        { "install.startMenuShortcutName", "{0}" },
        { "install.startMenuUninstallShortcutName", "\u5378\u8f7d {0}" },
        { "install.uninstallFailed", "\u65e0\u6cd5\u5220\u9664 {0} \u7684\u5feb\u6377\u65b9\u5f0f\u3002\u8bf7\u518d\u6b21\u5c1d\u8bd5\u3002" },
        { "install.uninstallFailedTitle", "\u5220\u9664\u5feb\u6377\u65b9\u5f0f" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "\u4e0d\u80fd\u8fd0\u884c\u6b64\u7a0b\u5e8f\uff0c\u56e0\u4e3a\u7cfb\u7edf\u4e2d\u7684 deployment.config \u6587\u4ef6\u8868\u660e\u4f01\u4e1a\u914d\u7f6e\u6587\u4ef6\u662f\u5fc5\u9700\u7684\uff0c\u4e14\u5fc5\u8981\u7684 {0} \u4e0d\u53ef\u7528\u3002" },

	// Jnlp Cache Viewer:
	{ "jnlp.viewer.title", "Java \u5e94\u7528\u7a0b\u5e8f\u9ad8\u901f\u7f13\u5b58\u67e5\u770b\u5668" },
	{ "jnlp.viewer.all", "\u5168\u90e8" },
	{ "jnlp.viewer.type", "{0}" },
	{ "jnlp.viewer.totalSize",  "\u5168\u90e8\u8d44\u6e90\u5927\u5c0f\uff1a{0}" },
 	{ "jnlp.viewer.emptyCache", "{0} \u9ad8\u901f\u7f13\u5b58\u4e3a\u7a7a"},
 	{ "jnlp.viewer.noCache", "\u672a\u914d\u7f6e\u7cfb\u7edf\u9ad8\u901f\u7f13\u5b58"},

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

	{ "jnlp.viewer.remove.btn", "\u5220\u9664(R)" },
	{ "jnlp.viewer.remove.1.btn", "\u5220\u9664\u9009\u5b9a\u7684 {0}(R)" },
	{ "jnlp.viewer.remove.2.btn", "\u5220\u9664\u9009\u5b9a\u7684\u6761\u76ee(R)" },
	{ "jnlp.viewer.uninstall.btn", "\u5378\u8f7d" },
	{ "jnlp.viewer.launch.offline.btn", "\u79bb\u7ebf\u542f\u52a8(L)" },
	{ "jnlp.viewer.launch.online.btn", "\u5728\u7ebf\u542f\u52a8(N)" },

        { "jnlp.viewer.file.menu", "\u6587\u4ef6(F)" },
        { "jnlp.viewer.edit.menu", "\u7f16\u8f91(E)" },
        { "jnlp.viewer.app.menu", "\u5e94\u7528\u7a0b\u5e8f(A)" },
        { "jnlp.viewer.view.menu", "\u89c6\u56fe(V)" },
        { "jnlp.viewer.help.menu", "\u5e2e\u52a9(H)" },

	{ "jnlp.viewer.cpl.mi", "\u542f\u52a8 Java \u63a7\u5236\u9762\u677f(C)" },
	{ "jnlp.viewer.exit.mi", "\u9000\u51fa(X)" },

	{ "jnlp.viewer.reinstall.mi", "\u91cd\u65b0\u5b89\u88c5(R)..." },
	{ "jnlp.viewer.preferences.mi", "\u9996\u9009\u9879(P)..." },

	{ "jnlp.viewer.launch.offline.mi", "\u79bb\u7ebf\u542f\u52a8(L)" },
	{ "jnlp.viewer.launch.online.mi", "\u5728\u7ebf\u542f\u52a8(N)" },
	{ "jnlp.viewer.install.mi", "\u5b89\u88c5\u5feb\u6377\u65b9\u5f0f(I)" },
	{ "jnlp.viewer.uninstall.mi", "\u5378\u8f7d\u5feb\u6377\u65b9\u5f0f(U)" },
	{ "jnlp.viewer.remove.0.mi", "\u5220\u9664(R)" },
	{ "jnlp.viewer.remove.mi", "\u5220\u9664 {0}(R)" },
	{ "jnlp.viewer.show.mi", "\u663e\u793a JNLP \u63cf\u8ff0\u7b26(S)" },
	{ "jnlp.viewer.browse.mi", "\u6d4f\u89c8\u4e3b\u9875(B)" },

	{ "jnlp.viewer.view.0.mi", "\u6240\u6709\u7c7b\u578b(T)" },
	{ "jnlp.viewer.view.1.mi", "\u5e94\u7528\u7a0b\u5e8f(A)" },
	{ "jnlp.viewer.view.2.mi", "\u5c0f\u5e94\u7528\u7a0b\u5e8f(P)" },
	{ "jnlp.viewer.view.3.mi", "\u5e93(L)" },
	{ "jnlp.viewer.view.4.mi", "\u5b89\u88c5\u7a0b\u5e8f(I)" },

	{ "jnlp.viewer.view.0", "\u6240\u6709\u7c7b\u578b" },
	{ "jnlp.viewer.view.1", "\u5e94\u7528\u7a0b\u5e8f" },
	{ "jnlp.viewer.view.2", "\u5c0f\u7a0b\u5e8f" },
	{ "jnlp.viewer.view.3", "\u5e93" },
	{ "jnlp.viewer.view.4", "\u5b89\u88c5\u7a0b\u5e8f" },

	{ "jnlp.viewer.about.mi", "\u5173\u4e8e(A)" },
	{ "jnlp.viewer.help.java.mi", "J2SE \u4e3b\u9875(J)" },
	{ "jnlp.viewer.help.jnlp.mi", "JNLP \u4e3b\u9875(H)" },

        { "jnlp.viewer.app.column", "\u5e94\u7528\u7a0b\u5e8f" },
        { "jnlp.viewer.vendor.column", "\u4f9b\u5e94\u5546" },
        { "jnlp.viewer.type.column", "\u7c7b\u578b" },
        { "jnlp.viewer.size.column", "\u5927\u5c0f" },
        { "jnlp.viewer.date.column", "\u65e5\u671f" },
        { "jnlp.viewer.status.column", "\u72b6\u6001" },

        { "jnlp.viewer.app.column.tooltip", "\u6b64\u5e94\u7528\u7a0b\u5e8f\u3001\u5c0f\u7a0b\u5e8f\u6216\u6269\u5c55\u7a0b\u5e8f\u7684\u56fe\u6807\u548c\u6807\u9898" },
        { "jnlp.viewer.vendor.column.tooltip", "\u516c\u53f8\u6b63\u5728\u90e8\u7f72\u6b64\u8f6f\u4ef6" },
        { "jnlp.viewer.type.column.tooltip", "\u6b64\u8f6f\u4ef6\u7684\u7c7b\u578b" },
        { "jnlp.viewer.size.column.tooltip", "\u6b64\u8f6f\u4ef6\u53ca\u5176\u6240\u6709\u8d44\u6e90\u7684\u5927\u5c0f" },
        { "jnlp.viewer.date.column.tooltip", "\u4e0a\u6b21\u8fd0\u884c\u6b64\u5e94\u7528\u7a0b\u5e8f\u3001\u5c0f\u7a0b\u5e8f\u6216\u5b89\u88c5\u7a0b\u5e8f\u7684\u65e5\u671f" },
        { "jnlp.viewer.status.column.tooltip", "\u7528\u4e8e\u663e\u793a\u662f\u5426\u53ef\u4ee5\u542f\u52a8\u4ee5\u53ca\u5982\u4f55\u542f\u52a8\u6b64\u8f6f\u4ef6\u7684\u56fe\u6807" },


        { "jnlp.viewer.application", "\u5e94\u7528\u7a0b\u5e8f" },
        { "jnlp.viewer.applet", "\u5c0f\u5e94\u7528\u7a0b\u5e8f" },
        { "jnlp.viewer.extension", "\u5e93" },
        { "jnlp.viewer.installer", "\u5b89\u88c5\u7a0b\u5e8f" },

	{ "jnlp.viewer.offline.tooltip",
		 "\u53ef\u5728\u7ebf\u6216\u79bb\u7ebf\u542f\u52a8\u6b64 {0}" },
	{ "jnlp.viewer.online.tooltip", "\u53ef\u5728\u7ebf\u542f\u52a8\u6b64 {0}" },
	{ "jnlp.viewer.norun1.tooltip", 
        	"\u53ea\u80fd\u4ece Web \u6d4f\u89c8\u5668\u542f\u52a8\u6b64 {0}" },
        { "jnlp.viewer.norun2.tooltip", "\u65e0\u6cd5\u542f\u52a8\u6269\u5c55\u7a0b\u5e8f" },

	{ "jnlp.viewer.show.title", "JNLP \u63cf\u8ff0\u7b26\u7528\u4e8e: {0}" },

	{ "jnlp.viewer.removing", "\u6b63\u5728\u5220\u9664..." },
	{ "jnlp.viewer.launching", "\u6b63\u5728\u542f\u52a8..." },
	{ "jnlp.viewer.browsing", "\u6b63\u5728\u542f\u52a8\u6d4f\u89c8\u5668..." },
	{ "jnlp.viewer.sorting", "\u6b63\u5728\u6392\u5e8f\u6761\u76ee..." },
	{ "jnlp.viewer.searching", "\u6b63\u5728\u641c\u7d22\u6761\u76ee..." },
	{ "jnlp.viewer.installing", "\u6b63\u5728\u5b89\u88c5..." },

        { "jnlp.viewer.reinstall.title", "\u91cd\u65b0\u5b89\u88c5\u5df2\u5220\u9664\u7684 JNLP \u5e94\u7528\u7a0b\u5e8f" },
	{ "jnlp.viewer.reinstallBtn", "\u91cd\u65b0\u5b89\u88c5\u5df2\u9009\u5b9a\u7684\u5e94\u7528\u7a0b\u5e8f(R)" },
	{ "jnlp.viewer.reinstallBtn.mnemonic", "VK_R" },
        { "jnlp.viewer.closeBtn", "\u5173\u95ed(C)" },
        { "jnlp.viewer.closeBtn.mnemonic", "VK_C" },

	{ "jnlp.viewer.reinstall.column.title", "\u6807\u9898:" },
	{ "jnlp.viewer.reinstall.column.location", "\u4f4d\u7f6e:" },

	// cache size warning
	{ "jnlp.cache.warning.title", "JNLP \u9ad8\u901f\u7f13\u5b58\u5927\u5c0f\u8b66\u544a" },
	{ "jnlp.cache.warning.message", "\u8b66\u544a\uff1a\n\n"+
		"\u9ad8\u901f\u7f13\u5b58\u4e2d\u7684 JNLP \u5e94\u7528\u7a0b\u5e8f\u548c\u8d44\u6e90\n"+
		"\u5df2\u7ecf\u8d85\u51fa\u5efa\u8bae\u7684\u78c1\u76d8\u7a7a\u95f4\u5927\u5c0f\u3002\n\n"+
		"\u60a8\u5f53\u524d\u6b63\u5728\u4f7f\u7528: {0}\n"+
		"\u5efa\u8bae\u7684\u9650\u5236\u4e3a: {1}\n\n"+
		"\u8bf7\u4f7f\u7528 Java \u63a7\u5236\u9762\u677f\u6765\u5220\u9664\u90e8\u5206\n"+
		"\u5e94\u7528\u7a0b\u5e8f\u6216\u8d44\u6e90\uff0c\u6216\u8005\u8bbe\u7f6e\u66f4\u9ad8\u7684\u9650\u5236\u3002" },

        // Control Panel
        { "control.panel.title", "Java \u63a7\u5236\u9762\u677f" },
        { "control.panel.general", "\u5e38\u89c4" },
        { "control.panel.security", "\u5b89\u5168" },
        { "control.panel.java", "Java" },
        { "control.panel.update", "\u66f4\u65b0" },
        { "control.panel.advanced", "\u9ad8\u7ea7" },

        // Common Strings used in different panels.
        { "common.settings", "\u8bbe\u7f6e" },
        { "common.ok_btn", "\u786e\u5b9a(O)" },
        { "common.ok_btn.mnemonic", "VK_O" },
        { "common.cancel_btn", "\u53d6\u6d88(C)" },
        { "common.cancel_btn.mnemonic", "VK_C" },
        { "common.apply_btn", "\u5e94\u7528(A)" },
        { "common.apply_btn.mnemonic", "VK_A" },
        { "common.add_btn", "\u6dfb\u52a0(A)" },
        { "common.add_btn.mnemonic", "VK_A" },
        { "common.remove_btn", "\u5220\u9664(R)" },
        { "common.remove_btn.mnemonic", "VK_R" },

        // Network Settings Dialog
        { "network.settings.dlg.title", "\u7f51\u7edc\u8bbe\u7f6e" },
        { "network.settings.dlg.border_title", " \u7f51\u7edc\u4ee3\u7406\u8bbe\u7f6e" },
        { "network.settings.dlg.browser_rbtn", "\u4f7f\u7528\u6d4f\u89c8\u5668\u8bbe\u7f6e" },
        { "browser_rbtn.mnemonic", "VK_B" },
        { "network.settings.dlg.manual_rbtn", "\u4f7f\u7528\u4ee3\u7406\u670d\u52a1\u5668" },
        { "manual_rbtn.mnemonic", "VK_P" },
        { "network.settings.dlg.address_lbl", "\u5730\u5740:" },
	{ "network.settings.dlg.port_lbl", "\u7aef\u53e3:" },
        { "network.settings.dlg.advanced_btn", "\u9ad8\u7ea7(A)..." },
        { "network.settings.dlg.advanced_btn.mnemonic", "VK_A" },
        { "network.settings.dlg.bypass_text", "\u5bf9\u672c\u5730\u5730\u5740\u4e0d\u4f7f\u7528\u4ee3\u7406\u670d\u52a1\u5668(Y)" },
        { "network.settings.dlg.bypass.mnemonic", "VK_Y" },
        { "network.settings.dlg.autoconfig_rbtn", "\u4f7f\u7528\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e\u811a\u672c(T)" },
        { "autoconfig_rbtn.mnemonic", "VK_T" },
        { "network.settings.dlg.location_lbl", "\u811a\u672c\u4f4d\u7f6e:" },
        { "network.settings.dlg.direct_rbtn", "\u76f4\u63a5\u8fde\u63a5(D)" },
        { "direct_rbtn.mnemonic", "VK_D" },
        { "network.settings.dlg.browser_text", "\u81ea\u52a8\u914d\u7f6e\u53ef\u80fd\u4f1a\u8986\u76d6\u624b\u52a8\u8bbe\u7f6e\u3002\u8981\u786e\u4fdd\u4f7f\u7528\u624b\u52a8\u8bbe\u7f6e\uff0c\u8bf7\u7981\u7528\u81ea\u52a8\u914d\u7f6e\u3002" },
        { "network.settings.dlg.proxy_text", "\u8986\u76d6\u6d4f\u89c8\u5668\u4ee3\u7406\u8bbe\u7f6e\u3002" },
        { "network.settings.dlg.auto_text", "\u5728\u6307\u5b9a\u7684\u4f4d\u7f6e\u4f7f\u7528\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e\u811a\u672c\u3002" },
        { "network.settings.dlg.none_text", "\u4f7f\u7528\u76f4\u63a5\u8fde\u63a5\u3002" },

        // Advanced Network Settings Dialog
        { "advanced.network.dlg.title", "\u9ad8\u7ea7\u7f51\u7edc\u8bbe\u7f6e" },
        { "advanced.network.dlg.servers", "\u670d\u52a1\u5668" },
        { "advanced.network.dlg.type", "\u7c7b\u578b" },
        { "advanced.network.dlg.http", "HTTP:" },
        { "advanced.network.dlg.secure", "\u5b89\u5168:" },
        { "advanced.network.dlg.ftp", "FTP:" },
        { "advanced.network.dlg.socks", "Socks:" },
        { "advanced.network.dlg.proxy_address", "\u4ee3\u7406\u5730\u5740" },
	{ "advanced.network.dlg.port", "\u7aef\u53e3" },
        { "advanced.network.dlg.same_proxy", "\u5bf9\u6240\u6709\u534f\u8bae\u4f7f\u7528\u540c\u4e00\u4e2a\u4ee3\u7406\u670d\u52a1\u5668(U)" },
        { "advanced.network.dlg.same_proxy.mnemonic", "VK_U" },
        { "advanced.network.dlg.exceptions", "\u5f02\u5e38" },
        { "advanced.network.dlg.no_proxy", "\u8bf7\u52ff\u5bf9\u4ee5\u4e0b\u5217\u6761\u76ee\u5f00\u5934\u7684\u5730\u5740\u4f7f\u7528\u4ee3\u7406\u670d\u52a1\u5668" },
        { "advanced.network.dlg.no_proxy_note", "\u4f7f\u7528\u5206\u53f7 (;) \u6765\u5206\u9694\u8fd9\u4e9b\u6761\u76ee\u3002" },

        // DeleteFilesDialog
        { "delete.files.dlg.title", "\u5220\u9664\u4e34\u65f6\u6587\u4ef6" },
        { "delete.files.dlg.temp_files", "\u662f\u5426\u5220\u9664\u4ee5\u4e0b\u4e34\u65f6\u6587\u4ef6\uff1f" },
        { "delete.files.dlg.applets", "\u5df2\u4e0b\u8f7d\u7684\u5c0f\u5e94\u7528\u7a0b\u5e8f" },
        { "delete.files.dlg.applications", "\u5df2\u4e0b\u8f7d\u7684\u5e94\u7528\u7a0b\u5e8f" },
        { "delete.files.dlg.other", "\u5176\u5b83\u6587\u4ef6" },

	// General
	{ "general.cache.border.text", "\u4e34\u65f6 Internet \u6587\u4ef6" },
	{ "general.cache.delete.text", "\u5220\u9664\u6587\u4ef6(D)..." },
        { "general.cache.delete.text.mnemonic", "VK_D" },
	{ "general.cache.settings.text", "\u8bbe\u7f6e(S)..." },
        { "general.cache.settings.text.mnemonic", "VK_S" },
	{ "general.cache.desc.text", "\u60a8\u5728 Java \u5e94\u7528\u7a0b\u5e8f\u4e2d\u4f7f\u7528\u7684\u6587\u4ef6\u5b58\u50a8\u5728\u4e00\u4e2a\u7279\u6b8a\u7684\u6587\u4ef6\u5939\u4e2d\uff0c\u4ee5\u4fbf\u5c06\u6765\u80fd\u591f\u5feb\u901f\u6267\u884c\u3002\u53ea\u6709\u9ad8\u7ea7\u7528\u6237\u624d\u80fd\u5220\u9664\u6587\u4ef6\u6216\u4fee\u6539\u8fd9\u4e9b\u8bbe\u7f6e\u3002" },
	{ "general.network.border.text", "\u7f51\u7edc\u8bbe\u7f6e" },
	{ "general.network.settings.text", "\u7f51\u7edc\u8bbe\u7f6e(N)..." },
        { "general.network.settings.text.mnemonic", "VK_N" },
	{ "general.network.desc.text", "\u5efa\u7acb Internet \u8fde\u63a5\u65f6\u9700\u8981\u4f7f\u7528\u8fd9\u4e9b\u7f51\u7edc\u8bbe\u7f6e\u3002\u9ed8\u8ba4\u60c5\u51b5\u4e0b\uff0cJava \u5c06\u4f7f\u7528 Web \u6d4f\u89c8\u5668\u4e2d\u7684\u7f51\u7edc\u8bbe\u7f6e\u3002\u53ea\u6709\u9ad8\u7ea7\u7528\u6237\u624d\u5e94\u8be5\u4fee\u6539\u8fd9\u4e9b\u8bbe\u7f6e\u3002" },
        { "general.about.border", "\u5173\u4e8e" },
        { "general.about.text", "\u67e5\u770b\u5173\u4e8e Java \u63a7\u5236\u9762\u677f\u7684\u7248\u672c\u4fe1\u606f\u3002" },
        { "general.about.btn", "\u5173\u4e8e(B)..." },
        { "general.about.btn.mnemonic", "VK_B" },


	// Security
	{ "security.certificates.border.text", "\u8bc1\u4e66" },
	{ "security.certificates.button.text", "\u8bc1\u4e66(E)..." },
        { "security.certificates.button.mnemonic", "VK_E" },
	{ "security.certificates.desc.text", "\u4f7f\u7528\u8bc1\u4e66\u6765\u660e\u786e\u5730\u6807\u8bc6\u60a8\u81ea\u5df1\u3001\u8bc1\u660e\u3001\u6388\u6743\u673a\u6784\u548c\u53d1\u653e\u673a\u6784\u3002" },
	{ "security.policies.border.text", "\u7b56\u7565" },
	{ "security.policies.advanced.text", "\u9ad8\u7ea7(D)..." },
        { "security.policies.advanced.mnemonic", "VK_D" },
	{ "security.policies.desc.text", "\u4f7f\u7528\u5b89\u5168\u7b56\u7565\u6765\u63a7\u5236\u5e94\u7528\u7a0b\u5e8f\u548c\u5c0f\u5e94\u7528\u7a0b\u5e8f\u7684\u5b89\u5168\u63aa\u65bd\u3002" },

	// Update
	{ "update.notify.border.text", "\u66f4\u65b0\u901a\u77e5" }, // this one is not currently used.  See update panel!!!
	{ "update.updatenow.button.text", "\u7acb\u5373\u66f4\u65b0(U)" },
	{ "update.updatenow.button.mnemonic", "VK_U" },
	{ "update.advanced.button.text", "\u9ad8\u7ea7(D)..." },
	{ "update.advanced.button.mnemonic", "VK_D" },
	{ "update.desc.text", "Java Update \u673a\u5236\u786e\u4fdd\u60a8\u62e5\u6709\u6700\u65b0\u7684 Java \u5e73\u53f0\u7248\u672c\u3002\u60a8\u53ef\u4ee5\u4f7f\u7528\u4ee5\u4e0b\u9009\u9879\u6765\u63a7\u5236\u83b7\u53d6\u548c\u5e94\u7528\u66f4\u65b0\u7684\u65b9\u5f0f\u3002" },
        { "update.notify.text", "\u901a\u77e5\u6211:" },
        { "update.notify_install.text", "\u5b89\u88c5\u4e4b\u524d" },
        { "update.notify_download.text", "\u4e0b\u8f7d\u548c\u5b89\u88c5\u4e4b\u524d" },
        { "update.autoupdate.text", "\u81ea\u52a8\u68c0\u67e5\u66f4\u65b0" },
        { "update.advanced_title.text", "\u81ea\u52a8\u66f4\u65b0\u9ad8\u7ea7\u8bbe\u7f6e" },
        { "update.advanced_title1.text", "\u9009\u62e9\u6267\u884c\u626b\u63cf\u7684\u9891\u7387\u548c\u65f6\u95f4\u3002" },
        { "update.advanced_title2.text", "\u9891\u7387" },
        { "update.advanced_title3.text", "\u65f6\u95f4" },
        { "update.advanced_desc1.text", "\u6bcf\u5929\u7684 {0} \u6267\u884c\u4e00\u6b21\u626b\u63cf" },
        { "update.advanced_desc2.text", "\u6bcf {0} \u7684 {1} \u6267\u884c\u4e00\u6b21\u626b\u63cf" },
        { "update.advanced_desc3.text", "\u6bcf\u6708\u7b2c {0} \u5929\u7684 {1} \u6267\u884c\u4e00\u6b21\u626b\u63cf" },
        { "update.check_daily.text", "\u6bcf\u5929\u4e00\u6b21" },
        { "update.check_weekly.text", "\u6bcf\u5468\u4e00\u6b21" },
        { "update.check_monthly.text", "\u6bcf\u6708\u4e00\u6b21" },
        { "update.check_date.text", "\u65e5\u671f:" },
        { "update.check_day.text", "\u6bcf:" },
        { "update.check_time.text", "\u65f6\u95f4:" },
        { "update.lastrun.text", "Java Update \u6700\u8fd1\u4e00\u6b21\u4e8e {1} \u7684 {0} \u8fd0\u884c\u3002" },
        { "update.desc_autooff.text", "\u5355\u51fb\u4e0b\u9762\u7684\u201c\u7acb\u5373\u66f4\u65b0\u201d\u6309\u94ae\u4ee5\u68c0\u67e5\u66f4\u65b0\u3002\u5982\u679c\u6709\u53ef\u7528\u7684\u66f4\u65b0\uff0c\u4efb\u52a1\u680f\u4e0a\u5c06\u663e\u793a\u4e00\u4e2a\u56fe\u6807\u3002\u5c06\u9f20\u6807\u79fb\u5230\u56fe\u6807\u4e0a\u53ef\u4ee5\u67e5\u770b\u8be5\u66f4\u65b0\u7684\u72b6\u6001\u3002" },
        { "update.desc_check_daily.text", "\u6bcf\u5929\u7684 {0}\uff0cJava Update \u5c06\u68c0\u67e5\u66f4\u65b0\u3002" },
        { "update.desc_check_weekly.text", "\u6bcf {0} \u7684 {1}\uff0cJava Update \u5c06\u68c0\u67e5\u66f4\u65b0\u3002" },
        { "update.desc_check_monthly.text", "\u6bcf\u6708\u7b2c {0} \u5929\u7684 {1}\uff0cJava Update \u5c06\u68c0\u67e5\u66f4\u65b0\u3002" },
        { "update.desc_systrayicon.text", "\u5982\u679c\u6709\u53ef\u7528\u7684\u66f4\u65b0\uff0c\u4efb\u52a1\u680f\u4e0a\u5c06\u663e\u793a\u4e00\u4e2a\u56fe\u6807\u3002\u5c06\u9f20\u6807\u79fb\u5230\u56fe\u6807\u4e0a\u53ef\u4ee5\u67e5\u770b\u8be5\u66f4\u65b0\u7684\u72b6\u6001\u3002" },
        { "update.desc_notify_install.text", "\u5728\u5b89\u88c5\u66f4\u65b0\u4e4b\u524d\uff0c\u7cfb\u7edf\u5c06\u53d1\u51fa\u901a\u77e5\u3002" },
        { "update.desc_notify_download.text", "\u5728\u4e0b\u8f7d\u548c\u5b89\u88c5\u66f4\u65b0\u4e4b\u524d\uff0c\u7cfb\u7edf\u5c06\u53d1\u51fa\u901a\u77e5\u3002" },
	{ "update.launchbrowser.error.text", "\u65e0\u6cd5\u542f\u52a8 Java Update \u68c0\u67e5\u7a0b\u5e8f\u3002\u8981\u83b7\u53d6\u6700\u65b0\u7684 Java Update\uff0c\u8bf7\u8bbf\u95ee http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "\u9519\u8bef - \u66f4\u65b0" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "\u5220\u9664\u6587\u4ef6(D)..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_D" },
        { "cache.settings.dialog.view_jws_btn", "\u67e5\u770b\u5e94\u7528\u7a0b\u5e8f(V)..." },
        { "cache.settings.dialog.view_jws_btn.mnemonic", "VK_V" },
        { "cache.settings.dialog.view_jpi_btn", "\u67e5\u770b\u5c0f\u5e94\u7528\u7a0b\u5e8f(A)..." },
        { "cache.settings.dialog.view_jpi_btn.mnemonic", "VK_A" },
        { "cache.settings.dialog.chooser_title", "\u4e34\u65f6\u6587\u4ef6\u4f4d\u7f6e" },
        { "cache.settings.dialog.select", "\u9009\u62e9(S)" },
        { "cache.settings.dialog.select_tooltip", "\u4f7f\u7528\u9009\u5b9a\u7684\u4f4d\u7f6e" },
        { "cache.settings.dialog.select_mnemonic", "S" },
        { "cache.settings.dialog.title", "\u4e34\u65f6\u6587\u4ef6\u8bbe\u7f6e" },
        { "cache.settings.dialog.cache_location", "\u4f4d\u7f6e:" },
        { "cache.settings.dialog.change_btn", "\u66f4\u6539(H)..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_H" },
        { "cache.settings.dialog.disk_space", "\u8981\u4f7f\u7528\u7684\u78c1\u76d8\u7a7a\u95f4\u91cf:" },
        { "cache.settings.dialog.unlimited_btn", "\u65e0\u9650" },
        { "cache.settings.dialog.max_btn", "\u6700\u5927\u503c" },
        { "cache.settings.dialog.compression", "Jar \u538b\u7f29:" },
        { "cache.settings.dialog.none", "\u65e0" },
        { "cache.settings.dialog.high", "\u9ad8" },

	// JNLP File/MIME association dialog strings:
	{ "javaws.association.dialog.title", "JNLP \u6587\u4ef6/MIME \u5173\u8054" },
	{ "javaws.association.dialog.exist.command", "\u5df2\u7ecf\u5b58\u5728\u4e8e:\n{0}"},
	{ "javaws.association.dialog.exist", "\u5df2\u7ecf\u5b58\u5728\u3002"  },
	{ "javaws.association.dialog.askReplace", "\n\u662f\u5426\u786e\u5b9e\u8981\u4f7f\u7528 {0} \u5904\u7406\u5b83\uff1f"},
	{ "javaws.association.dialog.ext", "\u6587\u4ef6\u6269\u5c55\u540d: {0}" },
        { "javaws.association.dialog.mime", "MIME \u7c7b\u578b: {0}" },
        { "javaws.association.dialog.ask", "\u662f\u5426\u5e0c\u671b\u4f7f\u7528 {0} \u6765\u5904\u7406:" },
        { "javaws.association.dialog.existAsk", "\u8b66\u544a\uff01\u5173\u8054\u4e8e:"},

        // Advanced panel strings:
        { "deployment.console.startup.mode", "Java \u63a7\u5236\u53f0" },
        { "deployment.console.startup.mode.SHOW", "\u663e\u793a\u63a7\u5236\u53f0" },
        { "deployment.console.startup.mode.SHOW.tooltip", "<html>" +
                                                          "\u542f\u52a8 Java \u63a7\u5236\u53f0\u65f6\u7a97\u53e3\u6700\u5927\u5316" +
                                                          "</html>" },
        { "deployment.console.startup.mode.HIDE", "\u9690\u85cf\u63a7\u5236\u53f0" },
        { "deployment.console.startup.mode.HIDE.tooltip", "<html>" +
                                                          "\u542f\u52a8 Java \u63a7\u5236\u53f0\u65f6\u7a97\u53e3\u6700\u5c0f\u5316" +
                                                          "</html>" },
        { "deployment.console.startup.mode.DISABLE", "\u4e0d\u542f\u52a8\u63a7\u5236\u53f0" },
        { "deployment.console.startup.mode.DISABLE.tooltip", "<html>" +
                                                             "\u4e0d\u542f\u52a8 Java \u63a7\u5236\u53f0" +
                                                             "</html>" },
        { "deployment.trace", "\u542f\u7528\u8ddf\u8e2a" },
        { "deployment.trace.tooltip", "<html>" +
                                      "\u521b\u5efa\u7528\u4e8e\u8c03\u8bd5\u7684" +
                                      "<br>\u8ddf\u8e2a\u6587\u4ef6" +
                                      "</html>" },
        { "deployment.log", "\u542f\u7528\u8bb0\u5f55" },
        { "deployment.log.tooltip", "<html>" +
                                    "\u521b\u5efa\u65e5\u5fd7\u6587\u4ef6\u4ee5\u6355\u83b7" +
                                    "<br>\u9519\u8bef" +
                                    "</html>" },
        { "deployment.control.panel.log", "\u5728\u63a7\u5236\u9762\u677f\u4e2d\u8bb0\u5f55" },
        { "deployment.javapi.lifecycle.exception", "\u663e\u793a\u5c0f\u7a0b\u5e8f\u751f\u547d\u5468\u671f\u5f02\u5e38" },
        { "deployment.javapi.lifecycle.exception.tooltip", "<html>" +
                                          "\u88c5\u5165\u5c0f\u7a0b\u5e8f\u7684\u8fc7\u7a0b\u4e2d\u51fa\u73b0\u9519\u8bef\u65f6"+
                                          "<br>\u663e\u793a\u5f02\u5e38\u5bf9\u8bdd\u6846"+
                                          "<html>" },
        { "deployment.browser.vm.iexplorer", "Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
                                                     "\u5728 Internet Explorer \u6d4f\u89c8\u5668\u4e2d" +
                                                     "<br>\u4f7f\u7528\u5e26\u6709 APPLET \u6807\u8bb0\u7684 Sun Java" +
                                                     "</html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla \u548c Netscape" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
                                                   "\u5728 Mozilla \u6216 Netscape \u6d4f\u89c8\u5668\u4e2d" +
                                                   "<br>\u4f7f\u7528\u5e26\u6709 APPLET \u6807\u8bb0\u7684 Sun Java" +
                                                   "</html>" },
        { "deployment.console.debugging", "\u8c03\u8bd5" },
	{ "deployment.browsers.applet.tag", "<APPLET> \u6807\u8bb0\u652f\u6301" },
        { "deployment.javaws.shortcut", "\u521b\u5efa\u5feb\u6377\u65b9\u5f0f" },
        { "deployment.javaws.shortcut.ALWAYS", "\u59cb\u7ec8\u5141\u8bb8" },
        { "deployment.javaws.shortcut.ALWAYS.tooltip", "<html>" +
                                                                 "\u59cb\u7ec8\u521b\u5efa\u5feb\u6377\u65b9\u5f0f" +
                                                                 "</html>" },
        { "deployment.javaws.shortcut.NEVER" , "\u4ece\u4e0d\u5141\u8bb8" },
        { "deployment.javaws.shortcut.NEVER.tooltip", "<html>" +
                                                                "\u4e0d\u521b\u5efa\u5feb\u6377\u65b9\u5f0f" +
                                                                "</html>" },
        { "deployment.javaws.shortcut.ASK_USER", "\u63d0\u9192\u7528\u6237" },
        { "deployment.javaws.shortcut.ASK_USER.tooltip", "<html>" +
                                                                   "\u8be2\u95ee\u7528\u6237\u662f\u5426\u521b\u5efa" +
                                                                   "<br>\u5feb\u6377\u65b9\u5f0f" +
                                                                   "</html>" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED", "\u63d0\u793a\u65f6\u59cb\u7ec8\u5141\u8bb8" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED.tooltip", "<html>" +
                                                                           "\u5982\u679c JNLP \u5e94\u7528\u7a0b\u5e8f\u8bf7\u6c42\uff0c" +
                                                                           "<br>\u59cb\u7ec8\u521b\u5efa\u5feb\u6377\u65b9\u5f0f" +
                                                                           "</html>" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED", "\u63d0\u793a\u65f6\u63d0\u9192\u7528\u6237" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED.tooltip", "<html>" +
                                                                        "JNLP \u5e94\u7528\u7a0b\u5e8f\u8bf7\u6c42\u65f6\uff0c" +
                                                                        "<br>\u8be2\u95ee\u7528\u6237\u662f\u5426\u521b\u5efa" +
                                                                        "<br>\u5feb\u6377\u65b9\u5f0f" +
                                                                        "</html>" },
	{ "deployment.javaws.associations.NEVER", "\u4ece\u4e0d\u5141\u8bb8" },
        { "deployment.javaws.associations.NEVER.tooltip", "<html>" +
                                                          "\u4ece\u4e0d\u521b\u5efa\u6587\u4ef6\u6269\u5c55\u540d/MIME" +
                                                          "<br>\u5173\u8054" +
                                                          "</html>" },
        { "deployment.javaws.associations.ASK_USER", "\u63d0\u9192\u7528\u6237" },
        { "deployment.javaws.associations.ASK_USER.tooltip", "<html>" +
                                                             "\u5728\u521b\u5efa\u6587\u4ef6\u6269\u5c55\u540d/MIME \u5173\u8054" +
                                                             "<br>\u4e4b\u524d\u8be2\u95ee\u7528\u6237" +
                                                             "</html>" },
        { "deployment.javaws.associations.REPLACE_ASK", "\u63d0\u9192\u7528\u6237\u66ff\u6362" },
        { "deployment.javaws.associations.REPLACE_ASK.tooltip", "<html>" +
                                                                "\u53ea\u5728\u66ff\u6362\u73b0\u6709\u6587\u4ef6\u6269\u5c55\u540d/MIME " +
                                                                "<br>\u5173\u8054\u65f6\u8be2\u95ee" +
                                                                "<br>\u7528\u6237" +
                                                                "</html>" },
        { "deployment.javaws.associations.NEW_ONLY", "\u5173\u8054\u4e3a\u65b0\u5173\u8054\u65f6\u5141\u8bb8" },
        { "deployment.javaws.associations.NEW_ONLY.tooltip", "<html>" +
                                                             "\u59cb\u7ec8\u53ea\u521b\u5efa\u65b0\u6587\u4ef6" +
                                                             "<br>\u6269\u5c55\u540d/MIME \u5173\u8054" +
                                                             "</html>" },
        { "deployment.javaws.associations", "JNLP \u6587\u4ef6/MIME \u5173\u8054" },
        { "deployment.security.settings", "\u5b89\u5168" },
        { "deployment.security.askgrantdialog.show", "\u5141\u8bb8\u7528\u6237\u4e3a\u7b7e\u540d\u7684\u5185\u5bb9\u6388\u4e88\u6743\u9650" },
        { "deployment.security.askgrantdialog.notinca", "\u5141\u8bb8\u7528\u6237\u4e3a\u6765\u81ea\u4e0d\u53ef\u4fe1\u8ba4\u8bc1\u673a\u6784\u7684\u5185\u5bb9\u6388\u4e88\u6743\u9650" },

	{ "deployment.security.browser.keystore.use", "\u4f7f\u7528\u6d4f\u89c8\u5668\u5bc6\u94a5\u5e93\u4e2d\u7684\u8bc1\u4e66\u548c\u5bc6\u94a5" },
	{ "deployment.security.notinca.warning", "\u65e0\u6cd5\u9a8c\u8bc1\u8ba4\u8bc1\u673a\u6784\u65f6\u53d1\u51fa\u8b66\u544a" },
        { "deployment.security.expired.warning", "\u8bc1\u4e66\u8fc7\u671f\u6216\u65e0\u6548\u65f6\u53d1\u51fa\u8b66\u544a" },
        { "deployment.security.jsse.hostmismatch.warning", "\u7ad9\u70b9\u8bc1\u4e66\u4e0e\u4e3b\u673a\u540d\u4e0d\u5339\u914d\u65f6\u53d1\u51fa\u8b66\u544a" },
        { "deployment.security.sandbox.awtwarningwindow", "\u663e\u793a\u6c99\u76d2\u72b6\u8b66\u544a\u6807\u9898" },
        { "deployment.security.sandbox.jnlp.enhanced", "\u5141\u8bb8\u7528\u6237\u63a5\u53d7 JNLP \u5b89\u5168\u8bf7\u6c42" },
        { "deploy.advanced.browse.title", "\u9009\u62e9\u7528\u4e8e\u542f\u52a8\u9ed8\u8ba4\u6d4f\u89c8\u5668\u7684\u6587\u4ef6" },
        { "deploy.advanced.browse.select", "\u9009\u62e9(S)" },
        { "deploy.advanced.browse.select_tooltip", "\u4f7f\u7528\u9009\u5b9a\u7684\u6587\u4ef6\u542f\u52a8\u6d4f\u89c8\u5668" },
        { "deploy.advanced.browse.select_mnemonic", "S" },
        { "deploy.advanced.browse.browse_btn", "\u6d4f\u89c8(B)..." },
        { "deploy.advanced.browse.browse_btn.mnemonic", "VK_B" },
        { "deployment.browser.default", "\u7528\u4e8e\u542f\u52a8\u9ed8\u8ba4\u6d4f\u89c8\u5668\u7684\u547d\u4ee4" },
        { "deployment.misc.label", "\u6742\u9879" },
        { "deployment.system.tray.icon", "\u5c06 Java \u56fe\u6807\u653e\u5728\u7cfb\u7edf\u4efb\u52a1\u680f\u4e2d" },
        { "deployment.system.tray.icon.tooltip", "<html>" +
                                                 "\u5982\u679c\u9009\u62e9\u6b64\u9009\u9879\uff0c\u5f53 Java \u5728\u6d4f\u89c8\u5668" +
                                                 "<br>\u4e2d\u8fd0\u884c\u65f6\uff0c\u5c06\u5728\u4efb\u52a1\u680f\u4e2d" +
                                                 "<br>\u663e\u793a Java \u5496\u5561\u676f\u56fe\u6807" +
                                                 "</html>" },

        //PluginJresDialog strings:
        { "jpi.jres.dialog.title", "Java Runtime \u8bbe\u7f6e" },
        { "jpi.jres.dialog.border", "Java Runtime \u7248\u672c" },
        { "jpi.jres.dialog.column1", "\u4ea7\u54c1\u540d\u79f0" },
        { "jpi.jres.dialog.column2", "\u7248\u672c" },
        { "jpi.jres.dialog.column3", "\u4f4d\u7f6e" },
        { "jpi.jres.dialog.column4", "Java Runtime \u53c2\u6570" },
        { "jpi.jdk.string", "JDK" },
        { "jpi.jre.string", "JRE" },
        { "jpi.jres.dialog.product.tooltip", "\u9009\u62e9 JRE \u6216 JDK \u4f5c\u4e3a\u4ea7\u54c1\u540d\u79f0" },

        // AboutDialog strings:
        { "about.dialog.title", "\u5173\u4e8e Java" },

        // JavaPanel strings:
        { "java.panel.plugin.border", "Java \u5c0f\u5e94\u7528\u7a0b\u5e8f Runtime \u8bbe\u7f6e" }, 
        { "java.panel.plugin.text", "\u5728\u6d4f\u89c8\u5668\u4e2d\u6267\u884c\u5c0f\u5e94\u7528\u7a0b\u5e8f\u65f6\uff0c\u5c06\u4f7f\u7528\u6b64 Runtime \u8bbe\u7f6e\u3002" },
        { "java.panel.jpi_view_btn", "\u67e5\u770b(V)..." },
        { "java.panel.javaws_view_btn", "\u67e5\u770b(I)..." },
        { "java.panel.jpi_view_btn.mnemonic", "VK_V" },
        { "java.panel.javaws_view_btn.mnemonic", "VK_I" },
        { "java.panel.javaws.border", "Java \u5e94\u7528\u7a0b\u5e8f Runtime \u8bbe\u7f6e "},
        { "java.panel.javaws.text", "\u4f7f\u7528 Java \u7f51\u7edc\u542f\u52a8\u534f\u8bae (JNLP) \u542f\u52a8 Java \u5e94\u7528\u7a0b\u5e8f\u6216\u5c0f\u5e94\u7528\u7a0b\u5e8f\u65f6\uff0c\u5c06\u4f7f\u7528\u6b64 Runtime \u8bbe\u7f6e\u3002" },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "<html><b>\u5b58\u5728\u66f4\u9ad8\u7248\u672c\u7684 Java Runtime</b></html>Internet Explorer \u4e2d\u5df2\u5177\u6709\u66f4\u9ad8\u7248\u672c\u7684 Java Runtime\u3002\u662f\u5426\u8981\u66ff\u6362\uff1f\n" },
        { "browser.settings.success.caption", "\u6210\u529f - \u6d4f\u89c8\u5668" },
        { "browser.settings.success.text", "<html><b>\u6d4f\u89c8\u5668\u8bbe\u7f6e\u5df2\u66f4\u6539</b></html>\u91cd\u65b0\u542f\u52a8\u6d4f\u89c8\u5668\u540e\uff0c\u6240\u505a\u7684\u66f4\u6539\u5c06\u751f\u6548\u3002\n" },
        { "browser.settings.fail.caption", "\u8b66\u544a - \u6d4f\u89c8\u5668" },
        { "browser.settings.fail.moz.text", "<html><b>\u65e0\u6cd5\u66f4\u6539\u6d4f\u89c8\u5668\u8bbe\u7f6e</b></html>"
                                        + "\u8bf7\u68c0\u67e5 Mozilla \u6216 Netscape \u662f\u5426\u5df2\u6b63\u786e\u5b89\u88c5\u5230\u7cfb\u7edf\u4e2d\u548c/\u6216 "
                                        + "\u60a8\u662f\u5426\u5177\u6709 "
                                        + "\u8db3\u591f\u7684\u6743\u9650\u6765\u66f4\u6539\u7cfb\u7edf\u8bbe\u7f6e\u3002\n" },
        { "browser.settings.fail.ie.text", "<html><b>\u65e0\u6cd5\u66f4\u6539\u6d4f\u89c8\u5668\u8bbe\u7f6e</b></html>\u8bf7\u68c0\u67e5\u60a8\u662f\u5426\u5177\u6709 "
					+ "\u8db3\u591f\u7684\u6743\u9650\u6765\u66f4\u6539\u7cfb\u7edf\u8bbe\u7f6e\u3002\n" },


        // Tool tip strings.
        { "cpl.ok_btn.tooltip", "<html>" +
                                "\u5173\u95ed Java \u63a7\u5236\u9762\u677f\u5e76\u4fdd\u5b58" +
                                "<br>\u60a8\u6240\u505a\u7684\u66f4\u6539" +
                                "</html>" },
        { "cpl.apply_btn.tooltip",  "<html>" +
                                    "\u4fdd\u5b58\u60a8\u6240\u505a\u7684\u66f4\u6539\uff0c" +
                                    "<br>\u4f46\u4e0d\u5173\u95ed Java \u63a7\u5236\u9762\u677f" +
                                    "</html>" },
        { "cpl.cancel_btn.tooltip", "<html>" +
                                    "\u5173\u95ed Java \u63a7\u5236\u9762\u677f\uff0c" +
                                    "<br>\u4f46\u4e0d\u4fdd\u5b58\u66f4\u6539" +
                                    "</html>" },

        {"network.settings.btn.tooltip", "<html>"+
                                         "\u4fee\u6539 Internet \u8fde\u63a5\u8bbe\u7f6e" +
                                         "</html>"},

        {"temp.files.settings.btn.tooltip", "<html>"+
                                            "\u4fee\u6539\u4e34\u65f6\u6587\u4ef6\u7684\u8bbe\u7f6e" +
                                            "</html>"},

        {"temp.files.delete.btn.tooltip", "<html>" +  // body bgcolor=\"#FFFFCC\">"+
                                          "\u5220\u9664\u4e34\u65f6 Java \u6587\u4ef6" +
                                          "</html>"},

        {"delete.files.dlg.applets.tooltip", "<html>" +
                                          "\u9009\u4e2d\u6b64\u9009\u9879\u4ee5\u5220\u9664\u6240\u6709\u7531 Java \u5c0f\u5e94\u7528\u7a0b\u5e8f" +
                                          "<br>\u521b\u5efa\u7684\u4e34\u65f6\u6587\u4ef6" +
                                          "</html>" },

        {"delete.files.dlg.applications.tooltip", "<html>" +
                                          "\u9009\u4e2d\u6b64\u9009\u9879\u4ee5\u5220\u9664\u6240\u6709" +
                                          "<br>\u7531 Java Web Start \u5e94\u7528\u7a0b\u5e8f" +
                                          "<br>\u521b\u5efa\u7684\u4e34\u65f6\u6587\u4ef6" +
                                          "</html>" },

        {"delete.files.dlg.other.tooltip", "<html>" +
                                          "\u9009\u4e2d\u6b64\u9009\u9879\u4ee5\u5220\u9664\u6240\u6709" +
                                          "<br>\u7531 Java \u521b\u5efa\u7684\u5176\u5b83\u4e34\u65f6\u6587\u4ef6" +
                                          "</html>" },

        {"delete.files.dlg.temp_files.tooltip", "<html>" +
                                          "Java \u5e94\u7528\u7a0b\u5e8f\u53ef\u80fd\u4f1a\u5728\u60a8\u7684\u8ba1\u7b97\u673a\u4e2d\u5b58\u50a8\u4e00 " +
				          "<br>\u4e9b\u4e34\u65f6\u6587\u4ef6\u3002\u5220\u9664\u8fd9\u4e9b\u6587\u4ef6\u4e0d\u4f1a\u5f15\u8d77\u5b89\u5168\u95ee\u9898\u3002" +
                                          "<br>\u5220\u9664\u4e34\u65f6\u6587\u4ef6\u540e\uff0c\u521d\u6b21\u8fd0\u884c\u67d0\u4e9b Java \u5e94\u7528\u7a0b\u5e8f " +
					  "<br>\u65f6\uff0c\u53ef\u80fd\u4f1a\u9700\u8981\u82b1\u8d39\u8f83\u957f\u7684\u65f6\u95f4\u624d\u80fd\u542f\u52a8" +
                                          "</html>" },

        {"cache.settings.dialog.view_jws_btn.tooltip", "<html>" +
                                          "\u67e5\u770b\u7531 Java Web Start \u5e94\u7528\u7a0b\u5e8f" +
                                          "<br>\u521b\u5efa\u7684\u4e34\u65f6\u6587\u4ef6" +
                                          "</html>" },

        {"cache.settings.dialog.view_jpi_btn.tooltip", "<html>" +
                                          "\u67e5\u770b\u7531 Java \u5c0f\u5e94\u7528\u7a0b\u5e8f" +
                                          "<br>\u521b\u5efa\u7684\u4e34\u65f6\u6587\u4ef6" +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "\u6307\u5b9a\u5b58\u50a8\u4e34\u65f6\u6587\u4ef6\u7684" +                                    
                                          "<br>\u76ee\u5f55"+
                                          "</html>" },

        {"cache.settings.dialog.unlimited_btn.tooltip", "<html>" +
                                          "\u8bf7\u52ff\u9650\u5236\u7528\u4e8e\u5b58\u50a8\u4e34\u65f6\u6587\u4ef6\u7684" +
                                          "<br>\u78c1\u76d8\u7a7a\u95f4\u91cf" +
                                          "</html>" },

        {"cache.settings.dialog.max_btn.tooltip", "<html>" +
                                          "\u6307\u5b9a\u7528\u4e8e\u5b58\u50a8\u4e34\u65f6\u6587\u4ef6\u7684\u6700\u5927" +
                                          "<br>\u78c1\u76d8\u7a7a\u95f4\u91cf" +
                                          "</html>" },

        {"cache.settings.dialog.compression.tooltip", "<html>" +
                                          "\u6307\u5b9a\u5728\u4e34\u65f6\u6587\u4ef6\u76ee\u5f55\u4e2d\u7531Java\u7a0b\u5e8f\u5b58\u50a8\u7684JAR " +
					  "<br>\u6587\u4ef6\u6240\u4f7f\u7528\u7684\u538b\u7f29\u91cf\u3002\u9009\u5b9a\u201c\u65e0\u201d\u65f6\uff0c\u60a8\u7684 " +
					  "<br>Java\u7a0b\u5e8f\u542f\u52a8\u8f83\u5feb\uff0c\u4f46\u5b58\u50a8\u5b83\u4eec\u6240\u9700\u8981\u7684\u78c1 " +
					  "<br>\u76d8\u7a7a\u95f4\u4e5f\u4f1a\u968f\u4e4b\u589e\u52a0\u3002\u503c\u8d8a\u5927\uff0c\u6240\u9700\u78c1\u76d8\u7a7a " +
					  "<br>\u95f4\u5c31\u8d8a\u5c0f\uff0c\u4f46\u4f1a\u7a0d\u7a0d\u589e\u52a0\u542f\u52a8\u6240\u9700\u7684\u65f6\u95f4" +
                                          "</html>" },

        { "common.ok_btn.tooltip",  "<html>" +
                                    "\u4fdd\u5b58\u66f4\u6539\u5e76\u5173\u95ed\u5bf9\u8bdd\u6846" +
                                    "</html>" },

        { "common.cancel_btn.tooltip",  "<html>" +
                                        "\u53d6\u6d88\u66f4\u6539\u5e76\u5173\u95ed\u5bf9\u8bdd\u6846" +
                                        "</html>"},

	{ "network.settings.advanced_btn.tooltip",  "<html>" +
                                                    "\u67e5\u770b\u5e76\u4fee\u6539\u9ad8\u7ea7\u4ee3\u7406\u8bbe\u7f6e"+
                                                    "</html>"},

        {"security.certs_btn.tooltip", "<html>" +
                                       "\u8f93\u5165\u3001\u8f93\u51fa\u6216\u5220\u9664\u8bc1\u4e66" +
                                       "</html>" },

        { "cert.import_btn.tooltip", "<html>" +
                                     "\u8f93\u5165\u5217\u8868\u4e2d\u4e0d\u5b58\u5728\u7684" +
                                     "<br>\u8bc1\u4e66" +
				     "</html>"},

        { "cert.export_btn.tooltip",    "<html>" +
                                        "\u8f93\u51fa\u9009\u5b9a\u7684\u8bc1\u4e66" +
                                        "</html>"},

        { "cert.remove_btn.tooltip",  "<html>" +
                                      "\u4ece\u5217\u8868\u4e2d\u5220\u9664\u9009\u5b9a\u7684"+
                                      "<br>\u8bc1\u4e66" +
        		      "</html>"},

        { "cert.details_btn.tooltip", "<html>" +
		      "\u67e5\u770b\u9009\u5b9a\u8bc1\u4e66\u7684" +
                                      "<br>\u8be6\u7ec6\u4fe1\u606f" +
		      "</html>"},

        { "java.panel.jpi_view_btn.tooltip",  "<html>" +
                                              "\u4fee\u6539 Java \u5c0f\u5e94\u7528\u7a0b\u5e8f\u7684\u8fd0\u884c\u65f6\u8bbe\u7f6e"+
                                              "</html>" },

        { "java.panel.javaws_view_btn.tooltip",   "<html>" +
                                                  "\u4fee\u6539 Java \u5e94\u7528\u7a0b\u5e8f\u7684\u8fd0\u884c\u65f6\u8bbe\u7f6e" +
                                                  "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "\u67e5\u770b\u6709\u5173\u6b64" +
                                            "<br> J2SE Runtime Environment \u7684\u4fe1\u606f" +
                                            "</html>" },

        { "update.notify_combo.tooltip",  "<html>" +
                                          "\u5982\u679c\u5e0c\u671b\u63a5\u5230\u6709\u5173\u65b0 " +
                                          "<br>\u7684 Java \u66f4\u65b0\u7684\u901a\u77e5\uff0c" +
                                          "<br>\u8bf7\u9009\u62e9\u6b64\u9009\u9879" +
                                          "</html>" },

        { "update.advanced_btn.tooltip",  "<html>" +
                                          "\u4fee\u6539\u81ea\u52a8\u66f4\u65b0\u7684" +
                                          "<br>\u8c03\u5ea6\u7b56\u7565" +
                                          "</html>" },

        { "update.now_btn.tooltip",    "<html>" +
                                      "\u8fd0\u884c Java Update \u4ee5\u68c0\u67e5\u6700\u65b0" +
                                      "<br>\u7684 Java \u66f4\u65b0" +
                                      "</html>" },

        { "vm.options.add_btn.tooltip",   "<html>" +
                                          "\u5c06\u65b0\u7684 JRE \u6dfb\u52a0\u5230\u5217\u8868\u4e2d" +
                                          "</html>" },

        { "vm.options.remove_btn.tooltip", "<html>" +
                                           "\u4ece\u5217\u8868\u4e2d\u5220\u9664\u9009\u5b9a\u7684\u6761\u76ee" +
                                           "</html>" },

        { "vm.optios.ok_btn.tooltip",    "<html>" +
		         "\u4fdd\u5b58\u6240\u6709\u5305\u542b\u4ea7\u54c1\u540d\u79f0\u3001" +
		         "<br>\u7248\u672c\u548c\u4f4d\u7f6e\u4fe1\u606f\u7684" +
		         "<br>\u6761\u76ee" +
		         "</html>" },

        { "jnlp.jre.find_btn.tooltip",  "<html>" +
		        "\u641c\u7d22\u5df2\u5b89\u88c5\u7684 Java Runtime" +
                                        "<br>Environment" +
		        "</html>" },

        { "jnlp.jre.add_btn.tooltip",   "<html>" +
                                        "\u5411\u5217\u8868\u6dfb\u52a0\u65b0\u6761\u76ee" +                                      
		        "</html>" },

        { "jnlp.jre.remove_btn.tooltip",  "<html>" +
                                          "\u4ece\u7528\u6237\u5217\u8868\u4e2d\u5220\u9664\u9009\u5b9a\u7684" +
                                          "<br>\u6761\u76ee" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "\u5141\u8bb8\u4e0b\u8f7d JRE" },
        { "download.jre.prompt.text1", "\u5e94\u7528\u7a0b\u5e8f\u201c{0}\u201d\u8981\u6c42\u60a8\u7684\u7cfb\u7edf\u4e2d "
                                     + "\u5df2\u7ecf\u5b89\u88c5\u4e86\u67d0\u4e2a\u7248\u672c\u7684 "
                                     + "JRE (\u7248\u672c {1})\u3002" },
        { "download.jre.prompt.text2", "\u662f\u5426\u5e0c\u671b Java Web Start \u81ea\u52a8\u4e0b\u8f7d "
                                     + "\u5e76\u5b89\u88c5 JRE\uff1f" },
        { "download.jre.prompt.okButton", "\u4e0b\u8f7d(D)" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "download.jre.prompt.cancelButton", "\u53d6\u6d88(C)" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "autoupdatecheck.buttonYes", "\u662f(Y)" },
	{ "autoupdatecheck.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "autoupdatecheck.buttonNo", "\u5426(N)" },
	{ "autoupdatecheck.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "autoupdatecheck.buttonAskLater", "\u7a0d\u540e\u518d\u95ee(A)" },
	{ "autoupdatecheck.buttonAskLater.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "autoupdatecheck.caption", "\u81ea\u52a8\u68c0\u67e5\u66f4\u65b0" },
	{ "autoupdatecheck.message", "\u5f53\u6709\u65b0\u7248\u672c\u7684 Java \u8f6f\u4ef6\u53ef\u7528\u65f6\uff0cJava Update \u53ef\u4ee5\u81ea\u52a8\u5bf9\u60a8\u7684 Java \u8f6f\u4ef6\u8fdb\u884c\u66f4\u65b0\u3002\u662f\u5426\u8981\u542f\u7528\u6b64\u670d\u52a1\uff1f" },
    };
}

