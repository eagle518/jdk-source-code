/*
 * @(#)Deployment_ko.java	1.29 04/07/16
 *
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;


/**
 * Korean verison of Deployment strings.
 *
 * @author Stanley Man-Kit Ho
 */

public final class Deployment_ko extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
        { "product.javapi.name", "Java Plug-in {0}" },
        { "product.javaws.name", "Java Web Start {0}" },
        { "console.version", "\ubc84\uc804" },
	{ "console.default_vm_version", "\uae30\ubcf8 \uac00\uc0c1 \uba38\uc2e0 \ubc84\uc804 " },
	{ "console.using_jre_version", "JRE \ubc84\uc804 \uc0ac\uc6a9" },
	{ "console.user_home", "\uc0ac\uc6a9\uc790 \ud648 \ub514\ub809\ud1a0\ub9ac" },
        { "console.caption", "Java \ucf58\uc194" },
        { "console.clear", "\uc9c0\uc6b0\uae30" },
        { "console.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "console.close", "\ub2eb\uae30" },
        { "console.close.acceleratorKey", new Integer(KeyEvent.VK_E) },
        { "console.copy", "\ubcf5\uc0ac" },
        { "console.copy.acceleratorKey", new Integer(KeyEvent.VK_Y) },
	{ "console.menu.text.top", "----------------------------------------------------\n" },
	{ "console.menu.text.c", "c:   \ucf58\uc194 \ucc3d \uc9c0\uc6b0\uae30\n" },
	{ "console.menu.text.f", "f:   \uc644\uc131 \ub300\uae30\uc5f4\uc5d0\uc11c \uac1d\uccb4 \uc644\uc131\n" },
	{ "console.menu.text.g", "g:   \uac00\ube44\uc9c0 \uceec\ub809\uc158\n" },
	{ "console.menu.text.h", "h:   \uc774 \ub3c4\uc6c0\ub9d0 \uba54\uc2dc\uc9c0 \ud45c\uc2dc\n" },
	{ "console.menu.text.j", "j:   jcov \ub370\uc774\ud130 \ub364\ud504\n"},
	{ "console.menu.text.l", "l:   \ud074\ub798\uc2a4\ub85c\ub354 \ubaa9\ub85d \ub364\ud504\n" },
	{ "console.menu.text.m", "m:   \uba54\ubaa8\ub9ac \uc0ac\uc6a9 \uc778\uc1c4\n" },
	{ "console.menu.text.o", "o:   \ub85c\uae45 \ud2b8\ub9ac\uac70\n" },
	{ "console.menu.text.p", "p:   \ud504\ub85d\uc2dc \uad6c\uc131 \uc7ac\ub85c\ub4dc\n" },
	{ "console.menu.text.q", "q:   \ucf58\uc194 \uc228\uae30\uae30\n" },
	{ "console.menu.text.r", "r:   \uc815\ucc45 \uad6c\uc131 \uc7ac\ub85c\ub4dc\n" },
	{ "console.menu.text.s", "s:   \uc2dc\uc2a4\ud15c \ubc0f \ubc30\ud3ec \ub4f1\ub85d \uc815\ubcf4 \ub364\ud504\n" },
	{ "console.menu.text.t", "t:   \uc2a4\ub808\ub4dc \ubaa9\ub85d \ub364\ud504\n" },
	{ "console.menu.text.v", "v:   \uc2a4\ub808\ub4dc \uc2a4\ud0dd \ub364\ud504\n" },
	{ "console.menu.text.x", "x:   \ud074\ub798\uc2a4\ub85c\ub354 \uce90\uc2dc \uc9c0\uc6b0\uae30\n" },
	{ "console.menu.text.0", "0-5: \ucd94\uc801 \uc218\uc900\uc744 <n>(\uc73c)\ub85c \uc124\uc815\n" },
	{ "console.menu.text.tail", "----------------------------------------------------\n" },
	{ "console.done", "\uc644\ub8cc" },
	{ "console.trace.level.0", "\ucd94\uc801 \uc218\uc900\uc744 0\uc73c\ub85c \uc124\uc815: none ... \uc644\ub8cc\ub428." },
	{ "console.trace.level.1", "\ucd94\uc801 \uc218\uc900\uc744 1\ub85c \uc124\uc815: basic ... \uc644\ub8cc\ub428." },
	{ "console.trace.level.2", "\ucd94\uc801 \uc218\uc900\uc744 2\ub85c \uc124\uc815: basic, net ... \uc644\ub8cc\ub428." },
	{ "console.trace.level.3", "\ucd94\uc801 \uc218\uc900\uc744 3\uc73c\ub85c \uc124\uc815: basic, net, security ... \uc644\ub8cc\ub428." },
	{ "console.trace.level.4", "\ucd94\uc801 \uc218\uc900\uc744 4\ub85c \uc124\uc815: basic, net, security, ext ... \uc644\ub8cc\ub428." },
	{ "console.trace.level.5", "\ucd94\uc801 \uc218\uc900\uc744 5\ub85c \uc124\uc815: all ... \uc644\ub8cc\ub428." },
	{ "console.log", "\ub85c\uae45 \uc124\uc815 : " },
	{ "console.completed", " ... \uc644\ub8cc\ub428." },
	{ "console.dump.thread", "\uc2a4\ub808\ub4dc \ubaa9\ub85d \ub364\ud504 ...\n" },
	{ "console.dump.stack", "\uc2a4\ub808\ub4dc \uc2a4\ud0dd \ub364\ud504 ...\n" },
	{ "console.dump.system.properties", "\uc2dc\uc2a4\ud15c \ub4f1\ub85d \uc815\ubcf4 \ub364\ud504 ...\n" },	
	{ "console.dump.deployment.properties", "\ubc30\ud3ec \ub4f1\ub85d \uc815\ubcf4 \ub364\ud504 ...\n" },	
	{ "console.clear.classloader", "\ud074\ub798\uc2a4\ub85c\ub354 \uce90\uc2dc \uc9c0\uc6b0\uae30 ... \uc644\ub8cc\ub428." },
	{ "console.reload.policy", "\uc815\ucc45 \uad6c\uc131 \uc7ac\ub85c\ub4dc" },
	{ "console.reload.proxy", "\ud504\ub85d\uc2dc \uad6c\uc131 \uc7ac\ub85c\ub4dc ..." },
	{ "console.gc", "\uac00\ube44\uc9c0 \ubaa8\uc73c\uae30" },
	{ "console.finalize", "\uc644\uc131 \ub300\uae30\uc5f4\uc5d0\uc11c \uac1d\uccb4 \uc644\uc131" },
	{ "console.memory", "\uba54\ubaa8\ub9ac: {0}K  \uc0ac\uc6a9 \uac00\ub2a5: {1}K  ({2}%)" },
	{ "console.jcov.error", "Jcov \ub7f0\ud0c0\uc784 \uc624\ub958: \uc62c\ubc14\ub978 jcov \uc635\uc158\uc744 \uc9c0\uc815\ud588\ub294\uc9c0 \ud655\uc778\ud558\uc2ed\uc2dc\uc624.\n"},
	{ "console.jcov.info", "Jcov \ub370\uc774\ud130\uac00 \uc131\uacf5\uc801\uc73c\ub85c \ub364\ud504\ub418\uc5c8\uc2b5\ub2c8\ub2e4.\n"},

	{ "https.dialog.caption", "\uacbd\uace0 - HTTPS" },
	{ "https.dialog.text", "<html><b>\ud638\uc2a4\ud2b8 \uc774\ub984 \ubd88\uc77c\uce58</b></html>\uc11c\ubc84 \ubcf4\uc548 \uc778\uc99d\uc11c\uc5d0 \uc788\ub294 \ud638\uc2a4\ud2b8 \uc774\ub984\uacfc \uc11c\ubc84 \uc774\ub984\uc774 \uc77c\uce58\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." 
			     + "\n\nURL\uc758 \ud638\uc2a4\ud2b8 \uc774\ub984: {0}"
			     + "\n\uc778\uc99d\uc11c\uc758 \ud638\uc2a4\ud2b8 \uc774\ub984: {1}"
			     + "\n\n\uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "https.dialog.unknown.host", "\uc54c \uc218 \uc5c6\ub294 \ud638\uc2a4\ud2b8" },

	{ "security.dialog.caption", "\uacbd\uace0 - \ubcf4\uc548" },
	{ "security.dialog.text0", "\ub2e4\uc74c\uc774 \ubc30\ud3ec\ud55c \uc11c\uba85\ub41c {0}\uc744(\ub97c) \uc2e0\ub8b0\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?:\"{1}\""
				 + "\n\n\ubc1c\ud589\uc790 \uc778\uc99d \ud655\uc778 \uae30\uc900: \"{2}\"" },
	{ "security.dialog.text0a", "\ub2e4\uc74c\uc774 \ubc30\ud3ec\ud55c \uc11c\uba85\ub41c {0}\uc744(\ub97c) \uc2e0\ub8b0\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?:\"{1}\""
 				 + "\n\n\ubc1c\ud589\uc790 \uc778\uc99d\uc744 \ud655\uc778\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "security.dialog.timestamp.text1", "{0}\ub294 {1}\uc5d0 \uc0ac\uc778\ub418\uc5c8\uc74d\ub2c8\ub2e4." },
	{ "security.dialog_https.text0", "\uc554\ud638\ud654\ub41c \uc815\ubcf4 \uad50\ud658\uc744 \ubaa9\uc801\uc73c\ub85c \uc6f9 \uc0ac\uc774\ud2b8 \"{0}\"\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \uc2b9\uc778\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?"
				 + "\n\n\ubc1c\ud589\uc790 \uc778\uc99d \ud655\uc778 \uae30\uc900: \"{1}\"" },
	{ "security.dialog_https.text0a", "\uc554\ud638\ud654\ub41c \uc815\ubcf4 \uad50\ud658\uc744 \ubaa9\uc801\uc73c\ub85c \uc6f9 \uc0ac\uc774\ud2b8 \"{0}\"\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \uc2b9\uc778\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?"
 				 + "\n\n\ubc1c\ud589\uc790 \uc778\uc99d\uc744 \ud655\uc778\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "security.dialog.text1", "\n\uc8fc\uc758: \uc774 \ub0b4\uc6a9\uc758 \uc548\uc804\uc131\uc740 \"{0}\"\uc5d0\uc11c \uba85\uc81c\ud654\ud569\ub2c8\ub2e4. \"{1}\"\uc744(\ub97c) \uc2e0\ub8b0\ud558\uc5ec \ud574\ub2f9 \uba85\uc81c\ub97c \uc791\uc131\ud558\ub824\ub294 \uacbd\uc6b0\uc5d0\ub9cc \uc774 \ub0b4\uc6a9\uc744 \uc2b9\uc778\ud558\uc2ed\uc2dc\uc624." },
	{ "security.dialog.unknown.issuer", "\uc54c \uc218 \uc5c6\ub294 \ubc1c\uae09\uc790" },
	{ "security.dialog.unknown.subject", "\uc54c \uc218 \uc5c6\ub294 \uc8fc\uc81c" },
	{ "security.dialog.certShowName", "{0} ({1})" },
	{ "security.dialog.rootCANotValid", "\uc774 \ubcf4\uc548 \uc778\uc99d\uc11c\ub294 \uc2e0\ub8b0\ud560 \uc218 \uc5c6\ub294 \ud68c\uc0ac\uc5d0\uc11c \ubc1c\ud589\ub41c \uac83\uc785\ub2c8\ub2e4." },
	{ "security.dialog.rootCAValid", "\uc774 \ubcf4\uc548 \uc778\uc99d\uc11c\ub294 \uc2e0\ub8b0\ud560 \uc218 \uc788\ub294 \ud68c\uc0ac\uc5d0\uc11c \ubc1c\ud589\ub41c \uac83\uc785\ub2c8\ub2e4." },
	{ "security.dialog.timeNotValid", "\uc774 \ubcf4\uc548 \uc778\uc99d\uc11c\ub294 \ub9cc\ub8cc\ub418\uc5c8\uac70\ub098 \uc544\uc9c1 \uc720\ud6a8\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "security.dialog.timeValid", "\uc774 \ubcf4\uc548 \uc778\uc99d\uc11c\ub294 \ub9cc\ub8cc\ub418\uc9c0 \uc54a\uc558\uac70\ub098 \uc544\uc9c1 \uc720\ud6a8\ud569\ub2c8\ub2e4." },
	{ "security.dialog.timeValidTS", "\uc774 \ubcf4\uc548 \uc778\uc99d\uc11c\ub294 {0}\uc774(\uac00) \uc0ac\uc778\ub418\uc5c8\uc744 \ub54c \uc720\ud6a8\ud569\ub2c8\ub2e4." }, 
	{ "security.dialog.buttonAlways", "\ud56d\uc0c1" },
        { "security.dialog.buttonAlways.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "security.dialog.buttonYes", "\uc608" },
	{ "security.dialog.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "security.dialog.buttonNo", "\uc544\ub2c8\uc624" },
	{ "security.dialog.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "security.dialog.buttonViewCert", "\uc138\ubd80 \uc815\ubcf4" },	
        { "security.dialog.buttonViewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

        { "security.badcert.caption", "\uacbd\uace0 - \ubcf4\uc548" },
        { "security.badcert.https.text", "SSL \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4.\n\uc774 {0}\uc744(\ub97c) \uc2e4\ud589\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "security.badcert.config.text", "\ubcf4\uc548 \uad6c\uc131\uc5d0\uc11c \uc774 \uc778\uc99d\uc11c\uc758 \ud655\uc778\uc744 \ud5c8\uc6a9\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4. \uc774 {0}\uc744(\ub97c) \uc2e4\ud589\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "security.badcert.text", "\uc778\uc99d\uc11c\ub97c \ud655\uc778\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4. \uc774 {0}\uc744(\ub97c) \uc2e4\ud589\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "security.badcert.viewException", "\uc608\uc678 \ud45c\uc2dc" },
        { "security.badcert.viewException.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "security.badcert.viewCert", "\uc138\ubd80 \uc815\ubcf4" },
        { "security.badcert.viewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

	{ "cert.dialog.caption", "\uc138\ubd80 \uc815\ubcf4 - \uc778\uc99d\uc11c" },
	{ "cert.dialog.certpath", "\uc778\uc99d\uc11c \uacbd\ub85c" },
	{ "cert.dialog.field.Version", "\ubc84\uc804" },
	{ "cert.dialog.field.SerialNumber", "\uc77c\ub828 \ubc88\ud638" },
	{ "cert.dialog.field.SignatureAlg", "\uc11c\uba85 \uc54c\uace0\ub9ac\uc998" },
	{ "cert.dialog.field.Issuer", "\ubc1c\uae09\uc790" },
	{ "cert.dialog.field.EffectiveDate", "\uac1c\uc2dc \ub0a0\uc9dc" },
	{ "cert.dialog.field.ExpirationDate", "\ub9cc\ub8cc \ub0a0\uc9dc" },
	{ "cert.dialog.field.Validity", "\uc720\ud6a8\uc131" },
	{ "cert.dialog.field.Subject", "\uc8fc\uc81c" },
	{ "cert.dialog.field.Signature", "\uc11c\uba85" },
	{ "cert.dialog.field", "\ud544\ub4dc" },
	{ "cert.dialog.value", "\uac12" },
        { "cert.dialog.close", "\ub2eb\uae30" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_C) },

	{ "clientauth.password.dialog.buttonOK", "\ud655\uc778" },
	{ "clientauth.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.password.dialog.buttonCancel", "\ucde8\uc18c" },
	{ "clientauth.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.password.dialog.caption", "\uc554\ud638 \ud544\uc694 - \ud074\ub77c\uc774\uc5b8\ud2b8 \uc778\uc99d \ud0a4 \uc800\uc7a5\uc18c" },
	{ "clientauth.password.dialog.text", "\ud074\ub77c\uc774\uc5b8\ud2b8 \uc778\uc99d \ud0a4 \uc800\uc7a5\uc18c\uc5d0 \uc561\uc138\uc2a4\ud558\ub824\uba74 \uc554\ud638\ub97c \uc785\ub825\ud569\ub2c8\ub2e4." },
	{ "clientauth.password.dialog.error.caption", "\uc624\ub958 - \ud074\ub77c\uc774\uc5b8\ud2b8 \uc778\uc99d \ud0a4 \uc800\uc7a5\uc18c" },
	{ "clientauth.password.dialog.error.text", "<html><b>\ud0a4 \uc800\uc7a5\uc18c \uc561\uc138\uc2a4 \uc624\ub958</b></html>\ud0a4 \uc800\uc7a5\uc18c\uac00 \uc190\uc0c1\ub418\uc5c8\uac70\ub098 \uc554\ud638\uac00 \uc62c\ubc14\ub974\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
		
	{ "clientauth.certlist.dialog.buttonOK", "\ud655\uc778" },
	{ "clientauth.certlist.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.certlist.dialog.buttonCancel", "\ucde8\uc18c" },
	{ "clientauth.certlist.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.certlist.dialog.buttonDetails", "\uc138\ubd80 \uc815\ubcf4" },
	{ "clientauth.certlist.dialog.buttonDetails.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "clientauth.certlist.dialog.caption", "\ud074\ub77c\uc774\uc5b8\ud2b8 \uc778\uc99d" },
	{ "clientauth.certlist.dialog.text", "\uc5f0\uacb0\ud558\ub824\ub294 \uc6f9 \uc0ac\uc774\ud2b8\uc5d0\ub294 \uc778\uc99d\uc774 \ud544\uc694\ud569\ub2c8\ub2e4.\n\uc5f0\uacb0\ud560 \ub54c \uc0ac\uc6a9\ud560 \uc778\uc99d\uc11c\ub97c \uc120\ud0dd\ud558\uc2ed\uc2dc\uc624.\n" },

	{ "dialogfactory.confirmDialogTitle", "\ud655\uc778 \ud544\uc694 - Java" },
	{ "dialogfactory.inputDialogTitle", "\uc815\ubcf4 \ud544\uc694 - Java" },
	{ "dialogfactory.messageDialogTitle", "\uba54\uc2dc\uc9c0 - Java" },
	{ "dialogfactory.exceptionDialogTitle", "\uc624\ub958 - Java" },
	{ "dialogfactory.optionDialogTitle", "\uc635\uc158 - Java" },
	{ "dialogfactory.aboutDialogTitle", "\uc815\ubcf4 - Java" },
	{ "dialogfactory.confirm.yes", "\uc608" },
        { "dialogfactory.confirm.yes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "dialogfactory.confirm.no", "\uc544\ub2c8\uc624" },
        { "dialogfactory.confirm.no.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "dialogfactory.moreInfo", "\uc138\ubd80 \uc815\ubcf4" },	
        { "dialogfactory.moreInfo.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "dialogfactory.lessInfo", "\ub35c \uc790\uc138\ud55c \uc815\ubcf4" },	
        { "dialogfactory.lessInfo.acceleratorKey", new Integer(KeyEvent.VK_L)},
	{ "dialogfactory.java.home.link", "http://java.sun.com" },
	{ "dialogfactory.general_error", "<html><b>\uc77c\ubc18 \uc608\uc678</b></html>" },
	{ "dialogfactory.net_error", "<html><b>\ub124\ud2b8\uc6cc\ud0b9 \uc608\uc678</b></html>" },
	{ "dialogfactory.security_error", "<html><b>\ubcf4\uc548 \uc608\uc678</b></html>" },
	{ "dialogfactory.ext_error", "<html><b>\uc120\ud0dd\uc801 \ud328\ud0a4\uc9c0 \uc608\uc678</b></html>" },
	{ "dialogfactory.user.selected", "\uc120\ud0dd\ub41c \uc0ac\uc6a9\uc790: {0}" },
	{ "dialogfactory.user.typed", "\uc785\ub825\ud55c \uc0ac\uc6a9\uc790: {0}" },

	{ "deploycertstore.cert.loading", "{0}\uc5d0\uc11c \ubc30\ud3ec \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud558\ub294 \uc911" },
	{ "deploycertstore.cert.loaded", "{0}\uc5d0\uc11c \ubc30\ud3ec \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud568" },
	{ "deploycertstore.cert.saving", "{0}\uc5d0 \ubc30\ud3ec \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud558\ub294 \uc911" },
	{ "deploycertstore.cert.saved", "{0}\uc5d0 \ubc30\ud3ec \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud568" },
	{ "deploycertstore.cert.adding", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \ucd94\uac00\ud558\ub294 \uc911", },
	{ "deploycertstore.cert.added", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \ubcc4\uce6d {0}(\uc73c)\ub85c \ucd94\uac00\ud568" },
	{ "deploycertstore.cert.removing", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud558\ub294 \uc911" },
	{ "deploycertstore.cert.removed", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \ubcc4\uce6d {0}(\uc73c)\ub85c \ub41c \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud568" }, 
	{ "deploycertstore.cert.instore", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\uac00 \uc788\ub294\uc9c0 \ud655\uc778\ud558\ub294 \uc911" },
	{ "deploycertstore.cert.canverify", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud560 \uc218 \uc788\ub294\uc9c0 \uac80\uc0ac" },
	{ "deploycertstore.cert.iterator", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c \ubc18\ubcf5\uae30 \uac00\uc838\uc624\uae30" },
	{ "deploycertstore.cert.getkeystore", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc758 \ud0a4 \uc800\uc7a5\uc18c \uac1d\uccb4 \uac00\uc838\uc624\uae30" },

	{ "httpscertstore.cert.loading", "{0}\uc5d0\uc11c \ubc30\ud3ec SSL \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud558\ub294 \uc911" },
	{ "httpscertstore.cert.loaded", "{0}\uc5d0\uc11c \ubc30\ud3ec SSL \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud568" },
	{ "httpscertstore.cert.saving", "{0}\uc5d0 \ubc30\ud3ec SSL \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud558\ub294 \uc911" },
	{ "httpscertstore.cert.saved", "{0}\uc5d0 \ubc30\ud3ec SSL \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud568" },
	{ "httpscertstore.cert.adding", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 SSL \uc778\uc99d\uc11c\ub97c \ucd94\uac00\ud558\ub294 \uc911", },
	{ "httpscertstore.cert.added", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 SSL \uc778\uc99d\uc11c\ub97c \ubcc4\uce6d {0}(\uc73c)\ub85c \ucd94\uac00\ud568" },
	{ "httpscertstore.cert.removing", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c SSL \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud558\ub294 \uc911" },
	{ "httpscertstore.cert.removed", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \ubcc4\uce6d {0}(\uc73c)\ub85c \ub41c SSL \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud568" }, 
	{ "httpscertstore.cert.instore", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 SSL \uc778\uc99d\uc11c\uac00 \uc788\ub294\uc9c0 \ud655\uc778\ud558\ub294 \uc911" },
	{ "httpscertstore.cert.canverify", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec SSL \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud560 \uc218 \uc788\ub294\uc9c0 \uac80\uc0ac" },
	{ "httpscertstore.cert.iterator", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 SSL \uc778\uc99d\uc11c \ubc18\ubcf5\uae30 \uac00\uc838\uc624\uae30" },
	{ "httpscertstore.cert.getkeystore", "\ubc30\ud3ec \uc601\uad6c \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc758 \ud0a4 \uc800\uc7a5\uc18c \uac1d\uccb4 \uac00\uc838\uc624\uae30" },

	{ "rootcertstore.cert.loading", "{0}\uc5d0\uc11c \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud558\ub294 \uc911" },
	{ "rootcertstore.cert.loaded", "{0}\uc5d0\uc11c \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud568" },
	{ "rootcertstore.cert.noload", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \ud30c\uc77c\uc744 \ucc3e\uc744 \uc218 \uc5c6\uc74c: {0}" },
	{ "rootcertstore.cert.saving", "{0}\uc5d0 \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud558\ub294 \uc911" },
	{ "rootcertstore.cert.saved", "{0}\uc5d0 \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud568" },
	{ "rootcertstore.cert.adding", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \ucd94\uac00\ud558\ub294 \uc911", },
	{ "rootcertstore.cert.added", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \ubcc4\uce6d {0}(\uc73c)\ub85c \ucd94\uac00\ud568" },
	{ "rootcertstore.cert.removing", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud558\ub294 \uc911" },
	{ "rootcertstore.cert.removed", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \ubcc4\uce6d {0}(\uc73c)\ub85c \ub41c \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud568" }, 
	{ "rootcertstore.cert.instore", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\uac00 \uc788\ub294\uc9c0 \ud655\uc778\ud558\ub294 \uc911" },
	{ "rootcertstore.cert.canverify", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud560 \uc218 \uc788\ub294\uc9c0 \uac80\uc0ac" },
	{ "rootcertstore.cert.iterator", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c \ubc18\ubcf5\uae30 \uac00\uc838\uc624\uae30" },
	{ "rootcertstore.cert.getkeystore", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc758 \ud0a4 \uc800\uc7a5\uc18c \uac1d\uccb4 \uac00\uc838\uc624\uae30" },
	{ "rootcertstore.cert.verify.ok", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud588\uc2b5\ub2c8\ub2e4." },
	{ "rootcertstore.cert.verify.fail", "\ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },
	{ "rootcertstore.cert.tobeverified", "\ud655\uc778\ud560 \uc778\uc99d\uc11c:\n{0}" },
	{ "rootcertstore.cert.tobecompared", "\ub2e4\uc74c \uc778\uc99d\uc11c\ub97c \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\uc640 \ube44\uad50\ud558\ub294 \uc911:\n{0}" },

	{ "roothttpscertstore.cert.loading", "{0}\uc5d0\uc11c SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud558\ub294 \uc911" },
	{ "roothttpscertstore.cert.loaded", "{0}\uc5d0\uc11c SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud568" },
	{ "roothttpscertstore.cert.noload", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \ud30c\uc77c\uc744 \ucc3e\uc744 \uc218 \uc5c6\uc74c: {0}" },
	{ "roothttpscertstore.cert.saving", "{0}\uc5d0 SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud558\ub294 \uc911" },
	{ "roothttpscertstore.cert.saved", "\ub2e4\uc74c \uc704\uce58\uc5d0 SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud568: {0}" },
	{ "roothttpscertstore.cert.adding", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \ucd94\uac00\ud558\ub294 \uc911", },
	{ "roothttpscertstore.cert.added", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \ubcc4\uce6d {0}(\uc73c)\ub85c \ucd94\uac00\ud568" },
	{ "roothttpscertstore.cert.removing", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud558\ub294 \uc911" },
	{ "roothttpscertstore.cert.removed", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \ubcc4\uce6d {0}(\uc73c)\ub85c \ub41c \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud568" }, 
	{ "roothttpscertstore.cert.instore", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\uac00 \uc788\ub294\uc9c0 \ud655\uc778\ud558\ub294 \uc911" },
	{ "roothttpscertstore.cert.canverify", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud560 \uc218 \uc788\ub294\uc9c0 \uac80\uc0ac" },
	{ "roothttpscertstore.cert.iterator", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c \ubc18\ubcf5\uae30 \uac00\uc838\uc624\uae30" },
	{ "roothttpscertstore.cert.getkeystore", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc758 \ud0a4 \uc800\uc7a5\uc18c \uac1d\uccb4 \uac00\uc838\uc624\uae30" },
	{ "roothttpscertstore.cert.verify.ok", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud588\uc2b5\ub2c8\ub2e4." },
	{ "roothttpscertstore.cert.verify.fail", "SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },
	{ "roothttpscertstore.cert.tobeverified", "\ud655\uc778\ud560 \uc778\uc99d\uc11c:\n{0}" },
	{ "roothttpscertstore.cert.tobecompared", "\ub2e4\uc74c \uc778\uc99d\uc11c\ub97c SSL \ub8e8\ud2b8 CA \uc778\uc99d\uc11c\uc640 \ube44\uad50\ud558\ub294 \uc911:\n{0}" },

	{ "sessioncertstore.cert.loading", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud558\ub294 \uc911" },
	{ "sessioncertstore.cert.loaded", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud568" },
	{ "sessioncertstore.cert.saving", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud558\ub294 \uc911" },
	{ "sessioncertstore.cert.saved", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \uc800\uc7a5\ud568" },
	{ "sessioncertstore.cert.adding", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \ucd94\uac00\ud558\ub294 \uc911", },
	{ "sessioncertstore.cert.added", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\ub97c \ucd94\uac00\ud568" },
	{ "sessioncertstore.cert.removing", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud558\ub294 \uc911" },
	{ "sessioncertstore.cert.removed", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud568" }, 
	{ "sessioncertstore.cert.instore", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\uac00 \uc788\ub294\uc9c0 \ud655\uc778\ud558\ub294 \uc911" },
	{ "sessioncertstore.cert.canverify", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud560 \uc218 \uc788\ub294\uc9c0 \uac80\uc0ac" },
	{ "sessioncertstore.cert.iterator", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c \ubc18\ubcf5\uae30 \uac00\uc838\uc624\uae30" },
	{ "sessioncertstore.cert.getkeystore", "\ubc30\ud3ec \uc138\uc158 \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc758 \ud0a4 \uc800\uc7a5\uc18c \uac1d\uccb4 \uac00\uc838\uc624\uae30" },

	{ "iexplorer.cert.loading", "Internet Explorer {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud558\ub294 \uc911" },
	{ "iexplorer.cert.loaded", "Internet Explorer {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud568" },
	{ "iexplorer.cert.instore", "Internet Explorer {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\uac00 \uc788\ub294\uc9c0 \ud655\uc778\ud558\ub294 \uc911" },
	{ "iexplorer.cert.canverify", "Internet Explorer {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud560 \uc218 \uc788\ub294\uc9c0 \uac80\uc0ac" },
	{ "iexplorer.cert.iterator", "Internet Explorer {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c \ubc18\ubcf5\uae30 \uac00\uc838\uc624\uae30" },
	{ "iexplorer.cert.verify.ok", "Internet Explorer {0} \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud588\uc2b5\ub2c8\ub2e4." },
	{ "iexplorer.cert.verify.fail", "Internet Explorer {0} \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },
	{ "iexplorer.cert.tobeverified", "\ud655\uc778\ud560 \uc778\uc99d\uc11c:\n{0}" },
	{ "iexplorer.cert.tobecompared", "\ub2e4\uc74c \uc778\uc99d\uc11c\ub97c Internet Explorer {0} \uc778\uc99d\uc11c\uc640 \ube44\uad50\ud558\ub294 \uc911:\n{1}" },

	{ "mozilla.cert.loading", "Mozilla {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud558\ub294 \uc911" },
        { "mozilla.cert.loaded", "Mozilla {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c\ub97c \ub85c\ub4dc\ud568" },
        { "mozilla.cert.instore", "Mozilla {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc778\uc99d\uc11c\uac00 \uc788\ub294\uc9c0 \ud655\uc778\ud558\ub294 \uc911" },
        { "mozilla.cert.canverify", "Mozilla {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud560 \uc218 \uc788\ub294\uc9c0 \uac80\uc0ac" },
        { "mozilla.cert.iterator", "Mozilla {0} \uc778\uc99d\uc11c \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c \ubc18\ubcf5\uae30 \uac00\uc838\uc624\uae30" },
        { "mozilla.cert.verify.ok", "Mozilla {0} \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud588\uc2b5\ub2c8\ub2e4." },
        { "mozilla.cert.verify.fail", "Mozilla {0} \uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc778\uc99d\uc11c\ub97c \ud655\uc778\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },
        { "mozilla.cert.tobeverified", "\ud655\uc778\ud560 \uc778\uc99d\uc11c:\n{0}" },
        { "mozilla.cert.tobecompared", "\ub2e4\uc74c \uc778\uc99d\uc11c\ub97c Mozilla {0} \uc778\uc99d\uc11c\uc640 \ube44\uad50\ud558\ub294 \uc911:\n{1}" },

        { "browserkeystore.jss.no", "JSS \ud328\ud0a4\uc9c0\ub97c \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "browserkeystore.jss.yes", "JSS \ud328\ud0a4\uc9c0\uac00 \ub85c\ub4dc\ub418\uc5c8\uc2b5\ub2c8\ub2e4." },
        { "browserkeystore.jss.notconfig", "JSS\uac00 \uad6c\uc131\ub418\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4." },
        { "browserkeystore.jss.config", "JSS\uac00 \uad6c\uc131\ub418\uc5b4 \uc788\uc2b5\ub2c8\ub2e4." },
        { "browserkeystore.mozilla.dir", "Mozilla \uc0ac\uc6a9\uc790 \ud504\ub85c\ud544\uc758 \ud0a4\uc640 \uc778\uc99d\uc11c\ub97c \uc561\uc138\uc2a4\ud558\ub294 \uc911: {0}" },
	{ "browserkeystore.password.dialog.buttonOK", "\ud655\uc778" },
	{ "browserkeystore.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "browserkeystore.password.dialog.buttonCancel", "\ucde8\uc18c" },
	{ "browserkeystore.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "browserkeystore.password.dialog.caption", "\uc554\ud638 \ud544\uc694" },
	{ "browserkeystore.password.dialog.text", "{0}\uc5d0 \ub300\ud55c \uc554\ud638\ub97c \uc785\ub825\ud558\uc2ed\uc2dc\uc624.\n" },
	{ "mozillamykeystore.priv.notfound", "\uc778\uc99d\uc11c\uc5d0 \ub300\ud55c \uac1c\uc778 \ud0a4\ub97c \ucc3e\uc744 \uc218 \uc5c6\uc74c: {0}" },
	{ "hostnameverifier.automation.ignoremismatch", "\uc790\ub3d9\ud654: \ud638\uc2a4\ud2b8 \uc774\ub984 \ubd88\uc77c\uce58 \ubb34\uc2dc" },

	{ "trustdecider.check.basicconstraints", "\uc778\uc99d\uc11c\uc758 \uae30\ubcf8 \uc81c\uc57d \uc870\uac74 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.leafkeyusage", "\uc778\uc99d\uc11c\uc758 \ub9ac\ud504 \ud0a4 \uc0ac\uc6a9 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.signerkeyusage", "\uc778\uc99d\uc11c\uc758 \uc11c\uba85\uc790 \ud0a4 \uc0ac\uc6a9 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.extensions", "\uc778\uc99d\uc11c\uc758 \uc911\uc694 \ud655\uc7a5 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.signature", "\uc778\uc99d\uc11c\uc758 \uc11c\uba85 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.basicconstraints.certtypebit", "\uc778\uc99d\uc11c\uc758 netscape \uc720\ud615 \ube44\ud2b8 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.basicconstraints.extensionvalue", "\uc778\uc99d\uc11c\uc758 netscape \ud655\uc7a5 \uac12 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.basicconstraints.bitvalue", "\uc778\uc99d\uc11c\uc758 netscape \ube44\ud2b8 5,6,7 \uac12 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.basicconstraints.enduser", "\uc778\uc99d\uc11c\uc5d0\uc11c CA \uc5ed\ud560\uc744 \ud558\ub294 \ucd5c\uc885 \uc0ac\uc6a9\uc790 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.basicconstraints.pathlength", "\uc778\uc99d\uc11c\uc758 \uacbd\ub85c \uae38\uc774 \uc81c\uc57d \uc870\uac74 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.leafkeyusage.length", "\uc778\uc99d\uc11c\uc758 \ud0a4 \uc0ac\uc6a9 \uae38\uc774 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.leafkeyusage.digitalsignature", "\uc778\uc99d\uc11c\uc758 \ub514\uc9c0\ud138 \uc11c\uba85 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.leafkeyusage.extkeyusageinfo", "\uc778\uc99d\uc11c\uc758 \ud655\uc7a5 \ud0a4 \uc0ac\uc6a9 \uc815\ubcf4 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.leafkeyusage.tsaextkeyusageinfo", "\uc778\uc99d\uc11c\uc758 TSA \ud655\uc7a5 \ud0a4 \uc0ac\uc6a9 \uc815\ubcf4 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.leafkeyusage.certtypebit", "\uc778\uc99d\uc11c\uc758 netscape \uc720\ud615 \ube44\ud2b8 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.signerkeyusage.lengthandbit", "\uc778\uc99d\uc11c\uc758 \uae38\uc774 \ubc0f \ube44\ud2b8 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.signerkeyusage.keyusage", "\uc778\uc99d\uc11c\uc758 \ud0a4 \uc0ac\uc6a9 \uac80\uc0ac \uc2e4\ud328" },
	{ "trustdecider.check.canonicalize.updatecert", "cacerts \ud30c\uc77c\uc5d0 \uc788\ub294 \uc778\uc99d\uc11c\ub85c \ub8e8\ud2b8 \uc778\uc99d\uc11c \uc5c5\ub370\uc774\ud2b8" },
	{ "trustdecider.check.canonicalize.missing", "\ube60\uc9c4 \ub8e8\ud2b8 \uc778\uc99d\uc11c \ucd94\uac00" },
	{ "trustdecider.check.gettrustedcert.find", "cacerts \ud30c\uc77c\uc5d0\uc11c \uc720\ud6a8\ud55c \ub8e8\ud2b8 CA \ucc3e\uae30" },        
	{ "trustdecider.check.gettrustedissuercert.find", "cacerts \ud30c\uc77c\uc5d0\uc11c \ube60\uc9c4 \uc720\ud6a8 \ub8e8\ud2b8 CA \ucc3e\uae30" },
	{ "trustdecider.check.timestamping.no", "\uc0ac\uc6a9 \uac00\ub2a5\ud55c \ud0c0\uc784\uc2a4\ud0ec\ud504 \uc815\ubcf4\uac00 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.check.timestamping.yes", "\ud0c0\uc784\uc2a4\ud0ec\ud504 \uc815\ubcf4\uac00 \uc0ac\uc6a9 \uac00\ub2a5\ud569\ub2c8\ub2e4." },
	{ "trustdecider.check.timestamping.tsapath", "TSA \uc778\uc99d\uc11c \uacbd\ub85c \uac80\uc0ac \uc2dc\uc791" },
	{ "trustdecider.check.timestamping.inca", "\uc778\uc99d\uc11c\uac00 \ub9cc\ub8cc\ub418\uc5c8\uc9c0\ub9cc \uc720\ud6a8 \uae30\uac04 \ub0b4\uc5d0 \ud0c0\uc784\uc2a4\ud0ec\ud504\ub418\uc5c8\uace0 TSA\uac00 \uc720\ud6a8\ud569\ub2c8\ub2e4." },
	{ "trustdecider.check.timestamping.notinca", "\uc778\uc99d\uc11c\uac00 \ub9cc\ub8cc\ub418\uc5c8\uc9c0\ub9cc TSA\ub294 \uc720\ud6a8\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.check.timestamping.valid", "\uc778\uc99d\uc11c\uac00 \ub9cc\ub8cc\ub418\uc5c8\uace0 \uc720\ud6a8 \uae30\uac04 \ub0b4\uc5d0 \ud0c0\uc784\uc2a4\ud0ec\ud504\ub418\uc5c8\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.check.timestamping.invalid", "\uc778\uc99d\uc11c\uac00 \ub9cc\ub8cc\ub418\uc5c8\uace0 \uc720\ud6a8\ud558\uc9c0 \uc54a\uc740 \uae30\uac04\uc5d0 \ud0c0\uc784\uc2a4\ud0ec\ud504\ub418\uc5c8\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.check.timestamping.need", "\uc778\uc99d\uc11c\uac00 \ub9cc\ub8cc\ub418\uc5c8\uc2b5\ub2c8\ub2e4. \uc778\uc99d\uc11c\uc758 \ud0c0\uc784\uc2a4\ud0ec\ud504 \uc815\ubcf4\ub97c \ud655\uc778\ud574\uc57c \ud569\ub2c8\ub2e4." },
	{ "trustdecider.check.timestamping.noneed", "\uc778\uc99d\uc11c\uac00 \ub9cc\ub8cc\ub418\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4. \uc778\uc99d\uc11c\uc758 \ud0c0\uc784\uc2a4\ud0ec\ud504 \uc815\ubcf4\ub97c \ud655\uc778\ud558\uc9c0 \uc54a\uc544\ub3c4 \ub429\ub2c8\ub2e4." },
	{ "trustdecider.check.timestamping.notfound", "\uc0c8 \ud0c0\uc784\uc2a4\ud0ec\ud504 API\ub97c \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.user.grant.session", "\uc0ac\uc6a9\uc790\uac00 \uc774 \uc138\uc158\uc758 \ucf54\ub4dc\uc5d0\ub9cc \uad8c\ud55c\uc744 \ubd80\uc5ec\ud588\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.user.grant.forever", "\uc0ac\uc6a9\uc790\uac00 \uc774 \ucf54\ub4dc\uc5d0 \uc601\uad6c \uad8c\ud55c\uc744 \ubd80\uc5ec\ud588\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.user.deny", "\uc0ac\uc6a9\uc790\uac00 \ucf54\ub4dc\uc5d0 \ub300\ud55c \uad8c\ud55c\uc744 \uac70\ubd80\ud588\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.automation.trustcert", "\uc790\ub3d9\ud654: \uc11c\uba85\uc6a9 \uc2e0\ub8b0 RSA \uc778\uc99d\uc11c" },
	{ "trustdecider.code.type.applet", "\uc560\ud50c\ub9bf" },
	{ "trustdecider.code.type.application", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8" },
	{ "trustdecider.code.type.extension", "\ud655\uc7a5\uc790" },
	{ "trustdecider.code.type.installer", "\uc124\uce58 \ud504\ub85c\uadf8\ub7a8" },
	{ "trustdecider.user.cannot.grant.any", "\ubcf4\uc548 \uad6c\uc131\uc5d0\uc11c \uc0c8 \uc778\uc99d\uc11c\uc5d0 \uad8c\ud55c \ubd80\uc5ec\ub97c \ud5c8\uc6a9\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "trustdecider.user.cannot.grant.notinca", "\ubcf4\uc548 \uad6c\uc131\uc5d0\uc11c \uc790\uccb4 \uc11c\uba85\ub41c \uc778\uc99d\uc11c\uc5d0 \uad8c\ud55c \ubd80\uc5ec\ub97c \ud5c8\uc6a9\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "x509trustmgr.automation.ignoreclientcert", "\uc790\ub3d9\ud654: \uc778\uc99d\ub418\uc9c0 \uc54a\uc740 \ud074\ub77c\uc774\uc5b8\ud2b8 \uc778\uc99d\uc11c \ubb34\uc2dc" },
	{ "x509trustmgr.automation.ignoreservercert", "\uc790\ub3d9\ud654: \uc778\uc99d\ub418\uc9c0 \uc54a\uc740 \uc11c\ubc84 \uc778\uc99d\uc11c \ubb34\uc2dc" },

	{ "net.proxy.text", "\ud504\ub85d\uc2dc: " },
	{ "net.proxy.override.text", "\ud504\ub85d\uc2dc \ub300\uccb4: " },
	{ "net.proxy.configuration.text", "\ud504\ub85d\uc2dc \uad6c\uc131: " },
	{ "net.proxy.type.browser", "\ube0c\ub77c\uc6b0\uc800 \ud504\ub85d\uc2dc \uad6c\uc131" },
	{ "net.proxy.type.auto", "\uc790\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131" },
	{ "net.proxy.type.manual", "\uc218\ub3d9 \uad6c\uc131" },
	{ "net.proxy.type.none", "\ud504\ub85d\uc2dc \uc5c6\uc74c" },
	{ "net.proxy.type.user", "\uc0ac\uc6a9\uc790\uac00 \ube0c\ub77c\uc6b0\uc800\uc758 \ud504\ub85d\uc2dc \uc124\uc815\uc744 \ub300\uccb4\ud588\uc2b5\ub2c8\ub2e4." },
	{ "net.proxy.loading.ie", "Internet Explorer\uc5d0\uc11c \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \ub85c\ub4dc\ud558\ub294 \uc911 ..."},
	{ "net.proxy.loading.ns", "Netscape Navigator\uc5d0\uc11c \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \ub85c\ub4dc\ud558\ub294 \uc911 ..."},
	{ "net.proxy.loading.userdef", "\uc0ac\uc6a9\uc790 \uc815\uc758 \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \ub85c\ub4dc\ud558\ub294 \uc911 ..."},
	{ "net.proxy.loading.direct", "\uc9c1\uc811 \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \ub85c\ub4dc\ud558\ub294 \uc911 ..."},
	{ "net.proxy.loading.manual", "\uc218\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \ub85c\ub4dc\ud558\ub294 \uc911 ..."},
	{ "net.proxy.loading.auto",   "\uc790\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \ub85c\ub4dc\ud558\ub294 \uc911 ..."},
	{ "net.proxy.loading.browser",   "\ube0c\ub77c\uc6b0\uc800 \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \ub85c\ub4dc\ud558\ub294 \uc911 ..."},
	{ "net.proxy.loading.manual.error", "\uc218\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \uc0ac\uc6a9\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4 - DIRECT\ub85c \ud3f4\ubc31"},
	{ "net.proxy.loading.auto.error", "\uc790\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131\uc744 \uc0ac\uc6a9\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4 - MANUAL\ub85c \ud3f4\ubc31"},
	{ "net.proxy.loading.done", "\uc644\ub8cc"},
	{ "net.proxy.browser.pref.read", "{0}\uc5d0\uc11c \uc0ac\uc6a9\uc790 \uae30\ubcf8 \uc124\uc815 \ud30c\uc77c\uc744 \uc77d\ub294 \uc911"},
	{ "net.proxy.browser.proxyEnable", "    \ud504\ub85d\uc2dc \uc0ac\uc6a9 \uac00\ub2a5: {0}"},
	{ "net.proxy.browser.proxyList",     "    \ud504\ub85d\uc2dc \ubaa9\ub85d: {0}"},
	{ "net.proxy.browser.proxyOverride", "    \ud504\ub85d\uc2dc \ub300\uccb4: {0}"},
	{ "net.proxy.browser.autoConfigURL", "    URL \uc790\ub3d9 \uad6c\uc131: {0}"},
	{ "net.proxy.browser.smartConfig", "\ud504\ub85d\uc2dc \uc11c\ubc84 {0}\uc744(\ub97c) \ud3ec\ud2b8 {1}\uc5d0\uc11c Ping"},
        { "net.proxy.browser.connectionException", "\ud504\ub85d\uc2dc \uc11c\ubc84 {0}\uc744(\ub97c) \ud3ec\ud2b8 {0}\uc5d0\uc11c \uc5f0\uacb0\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4."},
	{ "net.proxy.ns6.regs.exception", "\ub808\uc9c0\uc2a4\ud2b8\ub9ac \ud30c\uc77c\uc744 \uc77d\ub294 \uc911 \uc624\ub958 \ubc1c\uc0dd: {0}"},
	{ "net.proxy.pattern.convert", "\ud504\ub85d\uc2dc \ud1b5\uacfc \ubaa9\ub85d\uc744 \uc815\uaddc \ud45c\ud604\uc2dd\uc73c\ub85c \ubcc0\ud658: "},        
	{ "net.proxy.pattern.convert.error", "\ud504\ub85d\uc2dc \ud1b5\uacfc \ubaa9\ub85d\uc744 \uc815\uaddc \ud45c\ud604\uc2dd\uc73c\ub85c \ubcc0\ud658\ud560 \uc218 \uc5c6\uc74c - \ubb34\uc2dc"},        
	{ "net.proxy.auto.download.ins", "{0}\uc5d0\uc11c INS \ud30c\uc77c\uc744 \ub2e4\uc6b4\ub85c\ub4dc\ud558\ub294 \uc911" },
	{ "net.proxy.auto.download.js", "{0}\uc5d0\uc11c \uc790\ub3d9 \ud504\ub85d\uc2dc \ud30c\uc77c\uc744 \ub2e4\uc6b4\ub85c\ub4dc\ud558\ub294 \uc911" },
	{ "net.proxy.auto.result.error", "\ud3c9\uac00\ub85c\ubd80\ud130 \ud504\ub85d\uc2dc \uc124\uc815\uc744 \uacb0\uc815\ud560 \uc218 \uc5c6\uc74c - DIRECT\ub85c \ud3f4\ubc31"},
        { "net.proxy.service.not_available", "{0}\uc5d0 \ud504\ub85d\uc2dc \uc11c\ube44\uc2a4\ub97c \uc0ac\uc6a9\ud560 \uc218 \uc5c6\uc74c - \uae30\ubcf8\uac12\uc740 DIRECT" },
	{ "net.proxy.error.caption", "\uc624\ub958 - \ud504\ub85d\uc2dc \uad6c\uc131" },
	{ "net.proxy.nsprefs.error", "<html><b>\ud504\ub85c\uc2dc \uc124\uc815\uc744 \uac80\uc0c9\ud560 \uc218 \uc5c6\uc74c</b></html>\ub2e4\ub978 \ud504\ub85d\uc2dc \uad6c\uc131\uc73c\ub85c \ud3f4\ubc31\ud569\ub2c8\ub2e4.\n" },
	{ "net.proxy.connect", "{0}\uc744(\ub97c) proxy={1}\uacfc(\uc640) \uc5f0\uacb0 \uc911" },	

	{ "net.authenticate.caption", "\uc554\ud638 \ud544\uc694 - \ub124\ud2b8\uc6cc\ud0b9"},
	{ "net.authenticate.label", "<html><b>\uc0ac\uc6a9\uc790 \uc774\ub984 \ubc0f \uc554\ud638 \uc785\ub825:</b></html>"},
	{ "net.authenticate.resource", "\uc790\uc6d0:" },
	{ "net.authenticate.username", "\uc0ac\uc6a9\uc790 \uc774\ub984:" },
        { "net.authenticate.username.mnemonic", "VK_U" },
	{ "net.authenticate.password", "\uc554\ud638:" },
        { "net.authenticate.password.mnemonic", "VK_P" },
	{ "net.authenticate.firewall", "\uc11c\ubc84:" },
	{ "net.authenticate.domain", "\ub3c4\uba54\uc778:"},
        { "net.authenticate.domain.mnemonic", "VK_D" },
	{ "net.authenticate.realm", "\uc601\uc5ed:" },
	{ "net.authenticate.scheme", "\uccb4\uacc4:" },
	{ "net.authenticate.unknownSite", "\uc54c \uc218 \uc5c6\ub294 \uc0ac\uc774\ud2b8" },

	{ "net.cookie.cache", "\ucfe0\ud0a4 \uce90\uc2dc: " },
	{ "net.cookie.server", "\uc11c\ubc84 {0}\uc5d0\uc11c \"{1}\"(\uc73c)\ub85c \ucfe0\ud0a4 \uc124\uc815 \uc694\uccad \uc911" },
	{ "net.cookie.connect", "{0}\uc744(\ub97c) \"{1}\" \ucfe0\ud0a4\uc640 \uc5f0\uacb0 \uc911" },
	{ "net.cookie.ignore.setcookie", "\ucfe0\ud0a4 \uc11c\ube44\uc2a4\ub97c \uc0ac\uc6a9\ud560 \uc218 \uc5c6\uc74c - \"Set-Cookie\" \ubb34\uc2dc" },
	{ "net.cookie.noservice", "\ucfe0\ud0a4 \uc11c\ube44\uc2a4\ub97c \uc0ac\uc6a9\ud560 \uc218 \uc5c6\uc74c - \uce90\uc2dc\ub97c \uc0ac\uc6a9\ud558\uc5ec \"Cookie\" \uacb0\uc815" },

	{"about.java.version", "\ubc84\uc804 {0} (\ube4c\ub4dc {1})"},
	{"about.prompt.info", "Java \uae30\uc220\uc5d0 \uad00\ud55c \uc138\ubd80 \uc815\ubcf4\ub97c \ubcf4\uace0 Java \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \ud0d0\uc0c9\ud558\ub824\uba74 \ub2e4\uc74c\uc744 \ubc29\ubb38\ud558\uc2ed\uc2dc\uc624."},
	{"about.home.link", "http://www.java.com"},
	{"about.option.close", "\ub2eb\uae30"},
	{"about.option.close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{"about.copyright", "Copyright 2004 Sun Microsystems, Inc."},
	{"about.legal.note", "\ubaa8\ub4e0 \uad8c\ub9ac\ub294 \uc800\uc791\uad8c\uc790\uc758 \uc18c\uc720\uc785\ub2c8\ub2e4. \ubcf8 \uc81c\ud488\uc758 \uc0ac\uc6a9\uc740 \ub77c\uc774\uc13c\uc2a4 \uc870\ud56d\uc758 \uc801\uc6a9\uc744 \ubc1b\uc2b5\ub2c8\ub2e4."},


	{ "cert.remove_button", "\uc81c\uac70" },
        { "cert.remove_button.mnemonic", "VK_M" },
        { "cert.import_button", "\uac00\uc838\uc624\uae30" },
        { "cert.import_button.mnemonic", "VK_I" },
        { "cert.export_button", "\ub0b4\ubcf4\ub0b4\uae30" },
        { "cert.export_button.mnemonic", "VK_E" },
        { "cert.details_button", "\uc138\ubd80 \uc815\ubcf4" },
        { "cert.details_button.mnemonic", "VK_D" },
        { "cert.viewcert_button", "\uc778\uc99d\uc11c \ubcf4\uae30" },
        { "cert.viewcert_button.mnemonic", "VK_V" },
        { "cert.close_button", "\ub2eb\uae30" },
        { "cert.close_button.mnemonic", "VK_C" },
        { "cert.type.trusted_certs", "\uc2e0\ub8b0\ud560 \uc218 \uc788\ub294 \uc778\uc99d\uc11c" },
        { "cert.type.secure_site", "\ubcf4\uc548 \uc0ac\uc774\ud2b8" },
        { "cert.type.client_auth", "\ud074\ub77c\uc774\uc5b8\ud2b8 \uc778\uc99d" },
        { "cert.type.signer_ca", "\uc11c\uba85\uc790 CA" },
        { "cert.type.secure_site_ca", "\ubcf4\uc548 \uc0ac\uc774\ud2b8 CA" },
        { "cert.rbutton.user", "\uc0ac\uc6a9\uc790" },
        { "cert.rbutton.system", "\uc2dc\uc2a4\ud15c" },
        { "cert.settings", "\uc778\uc99d\uc11c" },
        { "cert.dialog.import.error.caption", "\uc624\ub958 - \uc778\uc99d\uc11c \uac00\uc838\uc624\uae30" },
        { "cert.dialog.export.error.caption", "\uc624\ub958 - \uc778\uc99d\uc11c \ub0b4\ubcf4\ub0b4\uae30" },
	{ "cert.dialog.import.format.text", "<html><b>\uc778\uc2dd\ud560 \uc218 \uc5c6\ub294 \ud30c\uc77c \ud615\uc2dd</b></html>\uc778\uc99d\uc11c\ub97c \uac00\uc838\uc62c \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "cert.dialog.export.password.text", "<html><b>\uc798\ubabb\ub41c \uc554\ud638</b></html>\uc785\ub825\ud55c \uc554\ud638\uac00 \uc62c\ubc14\ub974\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "cert.dialog.import.file.text", "<html><b>\ud30c\uc77c\uc774 \uc5c6\uc74c</b></html>\uc778\uc99d\uc11c\ub97c \uac00\uc838\uc62c \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "cert.dialog.import.password.text", "<html><b>\uc798\ubabb\ub41c \uc554\ud638</b></html>\uc785\ub825\ud55c \uc554\ud638\uac00 \uc62c\ubc14\ub974\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
        { "cert.dialog.password.caption", "\uc554\ud638" },
        { "cert.dialog.password.import.caption", "\uc554\ud638 \ud544\uc694 - \uac00\uc838\uc624\uae30" },
        { "cert.dialog.password.export.caption", "\uc554\ud638 \ud544\uc694 - \ub0b4\ubcf4\ub0b4\uae30" },
        { "cert.dialog.password.text", "\ub2e4\uc74c \ud30c\uc77c\uc5d0 \uc561\uc138\uc2a4\ud558\uae30 \uc704\ud55c \uc554\ud638 \uc785\ub825:\n" },
        { "cert.dialog.exportpassword.text", "\ud074\ub77c\uc774\uc5b8\ud2b8 \uc778\uc99d \ud0a4 \uc800\uc7a5\uc18c\uc5d0 \uc788\ub294 \ub2e4\uc74c \uac1c\uc778 \ud0a4\uc5d0 \uc561\uc138\uc2a4\ud558\uae30 \uc704\ud55c \uc554\ud638 \uc785\ub825:\n" },
        { "cert.dialog.savepassword.text", "\ub2e4\uc74c \ud0a4 \ud30c\uc77c\uc744 \uc800\uc7a5\ud558\uae30 \uc704\ud55c \uc554\ud638 \uc785\ub825:\n" },
        { "cert.dialog.password.okButton", "\ud655\uc778" },
        { "cert.dialog.password.cancelButton", "\ucde8\uc18c" },
        { "cert.dialog.export.error.caption", "\uc624\ub958 - \uc778\uc99d\uc11c \ub0b4\ubcf4\ub0b4\uae30" },
        { "cert.dialog.export.text", "<html><b>\ub0b4\ubcf4\ub0bc \uc218 \uc5c6\uc74c</b></html>\uc778\uc99d\uc11c\ub97c \ub0b4\ubcf4\ub0b4\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4." },
        { "cert.dialog.remove.text", "\uc778\uc99d\uc11c\ub97c \uc0ad\uc81c\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "cert.dialog.remove.caption", "\uc778\uc99d\uc11c \uc81c\uac70" },
	{ "cert.dialog.issued.to", "\ubc1c\uae09 \ub300\uc0c1" },
	{ "cert.dialog.issued.by", "\ubc1c\uae09\uc790" },
	{ "cert.dialog.user.level", "\uc0ac\uc6a9\uc790" },
	{ "cert.dialog.system.level", "\uc2dc\uc2a4\ud15c" },
	{ "cert.dialog.certtype", "\uc778\uc99d\uc11c \uc720\ud615: "},

	{ "controlpanel.jre.platformTableColumnTitle","\ud50c\ub7ab\ud3fc"},
	{ "controlpanel.jre.productTableColumnTitle","\uc81c\ud488" },
	{ "controlpanel.jre.locationTableColumnTitle","\uc704\uce58" },
	{ "controlpanel.jre.pathTableColumnTitle","\uacbd\ub85c" },
	{ "controlpanel.jre.enabledTableColumnTitle", "\uc0ac\uc6a9 \uac00\ub2a5" },

	{ "jnlp.jre.title", "JNLP \ub7f0\ud0c0\uc784 \uc124\uc815" },
	{ "jnlp.jre.versions", "Java \ub7f0\ud0c0\uc784 \ubc84\uc804" },
	{ "jnlp.jre.choose.button", "\uc120\ud0dd" },
	{ "jnlp.jre.find.button", "\ucc3e\uae30" },
	{ "jnlp.jre.add.button", "\ucd94\uac00" },
	{ "jnlp.jre.remove.button", "\uc81c\uac70" },
	{ "jnlp.jre.ok.button", "\ud655\uc778" },
	{ "jnlp.jre.cancel.button", "\ucde8\uc18c" },
	{ "jnlp.jre.choose.button.mnemonic", "VK_H" },
	{ "jnlp.jre.find.button.mnemonic", "VK_F" },
	{ "jnlp.jre.add.button.mnemonic", "VK_A" },
	{ "jnlp.jre.remove.button.mnemonic", "VK_R" },
	{ "jnlp.jre.ok.button.mnemonic", "VK_O" },
	{ "jnlp.jre.cancel.button.mnemonic", "VK_C" },

	{ "find.dialog.title", "JRE \uac80\uc0c9"},
	{ "find.title", "Java Runtime Environments"},
	{ "find.cancelButton", "\ucde8\uc18c"},
	{ "find.prevButton", "\uc774\uc804"},
	{ "find.nextButton", "\ub2e4\uc74c"},
	{ "find.cancelButtonMnemonic", "VK_C"},
	{ "find.prevButtonMnemonic", "VK_P"},
	{ "find.nextButtonMnemonic", "VK_N"},
	{ "find.intro", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \uc2dc\uc791\ud558\ub824\uba74 Java Web Start\uc5d0\uc11c \uc124\uce58\ub41c Java Runtime Environments\uc758 \uc704\uce58\ub97c \uc54c\uc544\uc57c \ud569\ub2c8\ub2e4.\n\n\uc0ac\uc6a9\ud55c JRE\ub97c \uc120\ud0dd\ud558\uac70\ub098 \ud30c\uc77c \uc2dc\uc2a4\ud15c\uc5d0\uc11c \ub514\ub809\ud1a0\ub9ac\ub97c \uc120\ud0dd\ud55c \ud6c4 JRE\ub97c \uac80\uc0c9\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },

	{ "find.searching.title", "\uc0ac\uc6a9 \uac00\ub2a5\ud55c JRE\ub97c \uac80\uc0c9\ud558\ub294 \uc911\uc785\ub2c8\ub2e4. \uba87 \ubd84 \uc815\ub3c4 \uac78\ub9b4 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
	{ "find.searching.prefix", "\uac80\uc0ac: " },
	{ "find.foundJREs.title", "\ub2e4\uc74c JRE\ub97c \ucc3e\uc558\uc2b5\ub2c8\ub2e4. \ucd94\uac00\ud558\ub824\uba74 \ub2e4\uc74c\uc744 \ub204\ub974\uc2ed\uc2dc\uc624." },
	{ "find.noJREs.title", "JRE\ub97c \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. \uac80\uc0c9\ud560 \ub2e4\ub978 \uc704\uce58\ub97c \uc120\ud0dd\ud558\ub824\uba74 \uc774\uc804\uc744 \ub204\ub974\uc2ed\uc2dc\uc624." },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Java(tm) \ubc30\ud3ec \ub4f1\ub85d \uc815\ubcf4\n"
                        + "# \uc774 \ud30c\uc77c\uc744 \ud3b8\uc9d1\ud558\uc9c0 \ub9c8\uc2ed\uc2dc\uc624. \uc774\uac83\uc740 \uc2dc\uc2a4\ud15c\uc774 \uc791\uc131\ud55c \uac83\uc785\ub2c8\ub2e4.\n"
                        + "# Java \uc81c\uc5b4\ud310\uc744 \uc0ac\uc6a9\ud558\uc5ec \ub4f1\ub85d \uc815\ubcf4\ub97c \ud3b8\uc9d1\ud558\uc2ed\uc2dc\uc624." },
        { "config.unknownSubject", "\uc54c \uc218 \uc5c6\ub294 \uc8fc\uc81c" },
        { "config.unknownIssuer", "\uc54c \uc218 \uc5c6\ub294 \ubc1c\uae09\uc790" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>\uc798\ubabb\ub41c URL</b></html>\uc790\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131 URL\uc774 \uc720\ud6a8\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
        { "config.proxy.autourl.invalid.caption", "\uc624\ub958 - \ud504\ub85d\uc2dc" },
	// Java Web Start Properties
	 { "APIImpl.clipboard.message.read", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \uc2dc\uc2a4\ud15c \ud074\ub9bd\ubcf4\ub4dc\uc5d0 \uc77d\uae30 \uc804\uc6a9 \uc561\uc138\uc2a4\ub97c \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc740 \ud074\ub9bd\ubcf4\ub4dc\uc5d0 \uc800\uc7a5\ub41c \uae30\ubc00 \uc815\ubcf4\uc5d0 \uc561\uc138\uc2a4\ud560 \uc218 \uc788\uac8c \ub429\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc744 \uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "APIImpl.clipboard.message.write", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \uc2dc\uc2a4\ud15c \ud074\ub9bd\ubcf4\ub4dc\uc5d0 \uc4f0\uae30 \uc561\uc138\uc2a4\ub97c \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc740 \ud074\ub9bd\ubcf4\ub4dc\uc5d0 \uc800\uc7a5\ub41c \uc815\ubcf4\ub97c \ub36e\uc5b4\uc4f8 \uc218 \uc788\uac8c \ub429\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc744 \uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "APIImpl.file.open.message", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \ud30c\uc77c \uc2dc\uc2a4\ud15c\uc73c\ub85c \uc77d\uae30 \uc561\uc138\uc2a4\ub97c \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc740 \ud30c\uc77c \uc2dc\uc2a4\ud15c\uc5d0 \uc800\uc7a5\ub41c \uae30\ubc00 \uc815\ubcf4\uc5d0 \uc561\uc138\uc2a4\ud560 \uc218 \uc788\uac8c \ub429\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc744 \uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "APIImpl.file.save.fileExist", "{0}\uc774(\uac00) \uc774\ubbf8 \uc788\uc2b5\ub2c8\ub2e4.\n \ubc14\uafb8\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "APIImpl.file.save.fileExistTitle", "\ud30c\uc77c \uc788\uc74c" },
        { "APIImpl.file.save.message", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \ub85c\uceec \ud30c\uc77c \uc2dc\uc2a4\ud15c\uc5d0 \uc788\ub294 \ud30c\uc77c\uc5d0 \uc77d\uae30/\uc4f0\uae30 \uc561\uc138\uc2a4\ub97c \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc73c\ub85c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc740 \ub2e4\uc74c \ud30c\uc77c \ub300\ud654 \uc0c1\uc790\uc5d0\uc11c \uc120\ud0dd\ub41c \ud30c\uc77c\uc5d0\ub9cc \uc561\uc138\uc2a4\ud560 \uc218 \uc788\uac8c \ub429\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc744 \uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "APIImpl.persistence.accessdenied", "{0} URL\uc5d0 \ub300\ud55c \uc601\uad6c \uc800\uc7a5\uc18c \uc561\uc138\uc2a4\uac00 \uac70\ubd80\ub418\uc5c8\uc2b5\ub2c8\ub2e4." },
        { "APIImpl.persistence.filesizemessage", "\ucd5c\ub300 \ud30c\uc77c \uae38\uc774\uac00 \ucd08\uacfc\ub428" },
        { "APIImpl.persistence.message", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \ucd94\uac00 \ub85c\uceec \ub514\uc2a4\ud06c \uc800\uc7a5 \uacf5\uac04\uc744 \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. \ud604\uc7ac \ud560\ub2f9\ub41c \ucd5c\ub300 \uc800\uc7a5\uc18c\ub294 {1} \ubc14\uc774\ud2b8\uc785\ub2c8\ub2e4. \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 {0} \ubc14\uc774\ud2b8\uc758 \ucd94\uac00 \uacf5\uac04\uc744 \uc694\uccad\ud569\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc744 \uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "APIImpl.print.message", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \uae30\ubcf8 \ud504\ub9b0\ud130\uc5d0 \uc561\uc138\uc2a4\ub97c \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc73c\ub85c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \ud504\ub9b0\ud130\uc5d0 \uc4f0\uae30 \uc561\uc138\uc2a4\ub97c \ud560 \uc218 \uc788\uac8c \ub429\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc744 \uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "APIImpl.extended.fileOpen.message1", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \ub85c\uceec \ud30c\uc77c \uc2dc\uc2a4\ud15c\uc5d0 \uc788\ub294 \ub2e4\uc74c \ud30c\uc77c\uc5d0 \ub300\ud55c \uc77d\uae30/\uc4f0\uae30 \uc561\uc138\uc2a4\ub97c \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4."},
	{ "APIImpl.extended.fileOpen.message2", "\uc774 \uc791\uc5c5\uc73c\ub85c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc740 \uc704\uc5d0 \ub098\uc5f4\ub41c \ud30c\uc77c\uc5d0\ub9cc \uc561\uc138\uc2a4\ud560 \uc218 \uc788\uac8c \ub429\ub2c8\ub2e4. \uc774 \uc791\uc5c5\uc744 \uacc4\uc18d\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?"},
        { "APIImpl.securityDialog.no", "\uc544\ub2c8\uc624" },
        { "APIImpl.securityDialog.remember", "\uc774 \uc815\ubcf4\ub97c \ub2e4\uc2dc \ud45c\uc2dc \uc548 \ud568" },
        { "APIImpl.securityDialog.title", "\ubcf4\uc548 \uc815\ubcf4" },
        { "APIImpl.securityDialog.yes", "\uc608" },
        { "Launch.error.installfailed", "\uc124\uce58\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },
        { "aboutBox.closeButton", "\ub2eb\uae30" },
        { "aboutBox.versionLabel", "\ubc84\uc804 {0} (\ube4c\ub4dc {1})" },
        { "applet.failedToStart", "\uc560\ud50c\ub9bf\uc744 \uc2dc\uc791\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4: {0}" },
        { "applet.initializing", "\uc560\ud50c\ub9bf\uc744 \ucd08\uae30\ud654\ud558\ub294 \uc911" },
        { "applet.initializingFailed", "\uc560\ud50c\ub9bf\uc744 \ucd08\uae30\ud654\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4: {0}" },
        { "applet.running", "\uc2e4\ud589 \uc911..." }, 
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "\ubcc0\uacbd \ub0b4\uc6a9\uc744 \uc801\uc6a9\ud558\ub824\uba74 Windows\ub97c \ub2e4\uc2dc \uc2dc\uc791\ud574\uc57c \ud569\ub2c8\ub2e4.\n\n\uc9c0\uae08 \ub2e4\uc2dc \uc2dc\uc791\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "extensionInstall.rebootTitle", "Windows \ub2e4\uc2dc \uc2dc\uc791" }, 
        { "install.configButton", "\uad6c\uc131 ..." },
        { "install.configMnemonic", "VK_C" },
        { "install.errorInstalling", "Java Runtime Environment\ub97c \uc124\uce58\ud558\ub824\uace0 \ud560 \ub54c \uc608\uae30\uce58 \uc54a\uc740 \uc624\ub958\uac00 \ubc1c\uc0dd\ud588\uc2b5\ub2c8\ub2e4. \ub2e4\uc2dc \uc2dc\ub3c4\ud558\uc2ed\uc2dc\uc624." },
        { "install.errorRestarting", "\uc2dc\uc791\ud558\ub294 \ub3d9\uc548 \uc608\uae30\uce58 \uc54a\uc740 \uc624\ub958\uac00 \ubc1c\uc0dd\ud588\uc2b5\ub2c8\ub2e4. \ub2e4\uc2dc \uc2dc\ub3c4\ud558\uc2ed\uc2dc\uc624." },
        { "install.title", "{0} - \ub2e8\ucd95\ud0a4 \uc791\uc131" },

        { "install.windows.both.message", "{0}\uc5d0 \ub300\ud55c \ub370\uc2a4\ud06c\ud0d1 \ubc0f \uc2dc\uc791 \uba54\ub274 \ub2e8\ucd95\ud0a4\ub97c\n\uc791\uc131\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "install.gnome.both.message", "{0}\uc5d0 \ub300\ud55c \ub370\uc2a4\ud06c\ud0d1 \ubc0f \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \uba54\ub274 \ub2e8\ucd95\ud0a4\ub97c\n\uc791\uc131\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "install.desktop.message", "{0}\uc5d0 \ub300\ud55c \ub370\uc2a4\ud06c\ud0d1 \ub2e8\ucd95\ud0a4\ub97c\n\uc791\uc131\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "install.windows.menu.message", "{0}\uc5d0 \ub300\ud55c \uc2dc\uc791 \uba54\ub274 \ub2e8\ucd95\ud0a4\ub97c\n\uc791\uc131\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "install.gnome.menu.message", "{0}\uc5d0 \ub300\ud55c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \uba54\ub274 \ub2e8\ucd95\ud0a4\ub97c\n\uc791\uc131\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "install.noButton", "\uc544\ub2c8\uc624" },
        { "install.noMnemonic", "VK_N" },
        { "install.yesButton", "\uc608" },
        { "install.yesMnemonic", "VK_Y" },
        { "launch.cancel", "\ucde8\uc18c" },
        { "launch.downloadingJRE", "JRE {0}\uc744(\ub97c) {1}\uc5d0\uc11c \uc694\uccad\ud558\ub294 \uc911" },
        { "launch.error.badfield", "\ud544\ub4dc {0}\uc5d0 \uc798\ubabb\ub41c \uac12\uc774 \uc788\uc2b5\ub2c8\ub2e4: {1}" },
        { "launch.error.badfield-signedjnlp", "\uc11c\uba85\ub41c \uc2dc\uc791 \ud30c\uc77c\uc758 \ud544\ub4dc {0}\uc5d0 \uc798\ubabb\ub41c \uac12\uc774 \uc788\uc2b5\ub2c8\ub2e4: {1}" },
        { "launch.error.badfield.download.https", "HTTPS\ub97c \ud1b5\ud574 \ub2e4\uc6b4\ub85c\ub4dc\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "launch.error.badfield.https", "HTTPS \uc9c0\uc6d0\uc5d0\ub294 Java 1.4+\uac00 \ud544\uc694\ud569\ub2c8\ub2e4." },
        { "launch.error.badjarfile", "{0}\uc5d0\uc11c JAR \ud30c\uc77c\uc774 \uc190\uc0c1\ub418\uc5c8\uc2b5\ub2c8\ub2e4" },
        { "launch.error.badjnlversion", "\uc2dc\uc791 \ud30c\uc77c\uc5d0 \uc788\ub294 JNLP\uc758 \ubc84\uc804\uc774 \uc9c0\uc6d0\ub418\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4: {0}. \uc774 \ubc84\uc804\uc5d0\ub294 \ubc84\uc804 1.0\uacfc 1.5\ub9cc \uc9c0\uc6d0\ub429\ub2c8\ub2e4. \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \uacf5\uae09\uc5c5\uccb4\uc5d0 \ubb38\uc758\ud558\uc5ec \uc774 \ubb38\uc81c\ub97c \ubcf4\uace0\ud558\uc2ed\uc2dc\uc624." },
        { "launch.error.badmimetyperesponse", "\uc790\uc6d0\uc5d0 \uc561\uc138\uc2a4\ud560 \ub54c \uc11c\ubc84\uc5d0\uc11c \uc798\ubabb\ub41c MIME \uc720\ud615\uc774 \ubc18\ud658\ub418\uc5c8\uc2b5\ub2c8\ub2e4: {0} - {1}" },
        { "launch.error.badsignedjnlp", "\uc2dc\uc791 \ud30c\uc77c\uc758 \uc11c\uba85\uc744 \ud655\uc778\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4. \uc11c\uba85\ud55c \ubc84\uc804\uc774 \ub2e4\uc6b4\ub85c\ub4dc\ud55c \ubc84\uc804\uacfc \uc77c\uce58\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
        { "launch.error.badversionresponse", "\uc790\uc6d0\uc5d0 \uc561\uc138\uc2a4\ud560 \ub54c \uc11c\ubc84\uc5d0\uc11c \uc798\ubabb\ub41c \ubc84\uc804 \uc751\ub2f5\uc774 \ud45c\uc2dc\ub418\uc5c8\uc2b5\ub2c8\ub2e4: {0} - {1}" },
        { "launch.error.canceledloadingresource", "\uc0ac\uc6a9\uc790\uac00 \uc790\uc6d0 {0} \ub85c\ub4dc\ub97c \ucde8\uc18c\ud588\uc2b5\ub2c8\ub2e4." },
        { "launch.error.category.arguments", "\uc798\ubabb\ub41c \uc778\uc218 \uc624\ub958" },
        { "launch.error.category.download", "\ub2e4\uc6b4\ub85c\ub4dc \uc624\ub958" },
        { "launch.error.category.launchdesc", "\uc2dc\uc791 \ud30c\uc77c \uc624\ub958" },
        { "launch.error.category.memory", "\uba54\ubaa8\ub9ac \ubd80\uc871 \uc624\ub958" },
        { "launch.error.category.security", "\ubcf4\uc548 \uc624\ub958" },
        { "launch.error.category.config", "\uc2dc\uc2a4\ud15c \uad6c\uc131" },
        { "launch.error.category.unexpected", "\uc608\uae30\uce58 \uc54a\uc740 \uc624\ub958" },
        { "launch.error.couldnotloadarg", "\uc9c0\uc815\ud55c \ud30c\uc77c/URL\uc744 \ub85c\ub4dc\ud560 \uc218 \uc5c6\uc74c: {0}" },
        { "launch.error.errorcoderesponse-known", "\uc790\uc6d0\uc5d0 \uc561\uc138\uc2a4\ud560 \ub54c \uc11c\ubc84\uc5d0\uc11c \uc624\ub958 \ucf54\ub4dc {1} ({2})\uc774(\uac00) \ubc18\ud658\ub428: {0}" },
        { "launch.error.errorcoderesponse-unknown", "\uc790\uc6d0\uc5d0 \uc561\uc138\uc2a4\ud560 \ub54c \uc11c\ubc84\uc5d0\uc11c \uc624\ub958 \ucf54\ub4dc 99(\uc54c \uc218 \uc5c6\ub294 \uc624\ub958)\uac00 \ubc18\ud658\ub428: {0}" },
        { "launch.error.failedexec", "Java Runtime Environment \ubc84\uc804 {0}\uc744(\ub97c) \uc2dc\uc791\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "launch.error.failedloadingresource", "\uc790\uc6d0\uc744 \ub85c\ub4dc\ud560 \uc218 \uc5c6\uc74c: {0}" },
        { "launch.error.invalidjardiff", "\uc790\uc6d0\uc5d0 \uc99d\ubd84 \uc5c5\ub370\uc774\ud2b8\ub97c \uc801\uc6a9\ud560 \uc218 \uc5c6\uc74c: {0}" },
        { "launch.error.jarsigning-badsigning", "\uc790\uc6d0\uc758 \uc11c\uba85\uc744 \ud655\uc778\ud560 \uc218 \uc5c6\uc74c: {0}" },
        { "launch.error.jarsigning-missingentry", "\uc790\uc6d0\uc5d0 \uc11c\uba85\ub41c \ud56d\ubaa9\uc774 \uc5c6\uc74c: {0}" },
        { "launch.error.jarsigning-missingentryname", "\uc11c\uba85\ub41c \ud56d\ubaa9\uc774 \uc5c6\uc74c: {0}" },
        { "launch.error.jarsigning-multicerts", "\uc790\uc6d0 \uc11c\uba85\uc5d0 \ub458 \uc774\uc0c1\uc758 \uc778\uc99d\uc11c\uac00 \uc0ac\uc6a9\ub428: {0}" },
        { "launch.error.jarsigning-multisigners", "\uc790\uc6d0\uc758 \ud56d\ubaa9\uc5d0 \ub458 \uc774\uc0c1\uc758 \uc11c\uba85\uc774 \uc788\uc74c: {0}" },
        { "launch.error.jarsigning-unsignedfile", "\uc790\uc6d0\uc5d0 \uc11c\uba85\ub418\uc9c0 \uc54a\uc740 \ud56d\ubaa9\uc774 \uc788\uc74c: {0}" },
        { "launch.error.missingfield", "\ub2e4\uc74c \ud544\uc218 \ud544\ub4dc\uac00 \uc2dc\uc791 \ud30c\uc77c\uc5d0 \uc5c6\uc74c: {0}" },
        { "launch.error.missingfield-signedjnlp", "\ub2e4\uc74c \ud544\uc218 \ud544\ub4dc\uac00 \uc11c\uba85\ub41c \uc2dc\uc791 \ud30c\uc77c\uc5d0 \uc5c6\uc74c: {0}" },
        { "launch.error.missingjreversion", "\uc774 \uc2dc\uc2a4\ud15c\uc5d0 \ub300\ud55c \uc2dc\uc791 \ud30c\uc77c\uc5d0 JRE \ubc84\uc804\uc774 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "launch.error.missingversionresponse", "\uc790\uc6d0\uc5d0 \uc561\uc138\uc2a4\ud560 \ub54c \uc11c\ubc84\uc5d0\uc11c \ubc84\uc804 \ud544\ub4dc\uac00 \uc5c6\ub2e4\ub294 \uc751\ub2f5\uc774 \ud45c\uc2dc\ub428: {0}" },
        { "launch.error.multiplehostsreferences", "\uc790\uc6d0\uc5d0\uc11c \uc5ec\ub7ec \ud638\uc2a4\ud2b8\uac00 \ucc38\uc870\ub428" },
        { "launch.error.nativelibviolation", "\uc6d0\uc2dc \ub77c\uc774\ube0c\ub7ec\ub9ac \uc0ac\uc6a9\uc5d0\ub294 \uc2dc\uc2a4\ud15c\uc73c\ub85c\uc758 \ubb34\uc81c\ud55c \uc561\uc138\uc2a4\uac00 \ud544\uc694\ud568" },
        { "launch.error.noJre", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c \ud604\uc7ac \ub85c\uceec\uc5d0 \uc124\uce58\ub418\uc9c0 \uc54a\uc740 JRE\uc758 \ubc84\uc804\uc744 \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. Java Web Start\uc5d0\uc11c \uc694\uccad\ub41c \ubc84\uc804\uc744 \uc790\ub3d9\uc73c\ub85c \ub2e4\uc6b4\ub85c\ub4dc\ud558\uace0 \uc124\uce58\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. JRE \ubc84\uc804\uc744 \uc218\ub3d9\uc73c\ub85c \uc124\uce58\ud574\uc57c \ud569\ub2c8\ub2e4.\n\n" },
        { "launch.error.wont.download.jre", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c \ud604\uc7ac \ub85c\uceec\uc5d0 \uc124\uce58\ub418\uc9c0 \uc54a\uc740 JRE (version {0})\uc744 \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. Java Web Start\uc5d0\uc11c\ub294 \uc694\uccad\ud55c \ubc84\uc804\uc744 \uc790\ub3d9\uc73c\ub85c \ub2e4\uc6b4\ub85c\ub4dc\ud558\uc5ec \uc124\uce58\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. JRE \ubc84\uc804\uc744 \uc218\ub3d9\uc73c\ub85c \uc124\uce58\ud574\uc57c \ud569\ub2c8\ub2e4." },
        { "launch.error.cant.download.jre", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c \ud604\uc7ac \ub85c\uceec\uc5d0 \uc124\uce58\ub418\uc9c0 \uc54a\uc740 JRE (version {0})\uc744 \uc694\uccad\ud588\uc2b5\ub2c8\ub2e4. Java Web Start\uc5d0\uc11c\ub294 \uc694\uccad\ud55c \ubc84\uc804\uc744 \uc790\ub3d9\uc73c\ub85c \ub2e4\uc6b4\ub85c\ub4dc\ud558\uc5ec \uc124\uce58\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. JRE \ubc84\uc804\uc744 \uc218\ub3d9\uc73c\ub85c \uc124\uce58\ud574\uc57c \ud569\ub2c8\ub2e4." },
        { "launch.error.cant.access.system.cache", "\ud604\uc7ac \uc0ac\uc6a9\uc790\ub294 \uc2dc\uc2a4\ud15c \uce90\uc2dc\uc5d0 \ub300\ud55c \uc4f0\uae30 \uc561\uc138\uc2a4 \uad8c\ud55c\uc744 \uac00\uc9c0\uace0 \uc788\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
        { "launch.error.cant.access.user.cache", "\ud604\uc7ac \uc0ac\uc6a9\uc790\ub294 \ud574\ub2f9 \uce90\uc2dc\uc5d0 \ub300\ud55c \uc4f0\uae30 \uc561\uc138\uc2a4 \uad8c\ud55c\uc744 \uac00\uc9c0\uace0 \uc788\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },        
        { "launch.error.noappresources", "\uc774 \ud50c\ub7ab\ud3fc\uc5d0 \ub300\ud55c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \uc790\uc6d0\uc774 \uc9c0\uc815\ub418\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4. \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \uacf5\uae09\uc5c5\uccb4\uc5d0 \ubb38\uc758\ud558\uc5ec \ud574\ub2f9 \ud50c\ub7ab\ud3fc\uc774 \uc9c0\uc6d0\ub418\ub294\uc9c0 \ud655\uc778\ud558\uc2ed\uc2dc\uc624." },
        { "launch.error.nomainclass", "\uae30\ubcf8 \ud074\ub798\uc2a4 {0}\uc744(\ub97c) {1}\uc5d0\uc11c \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "launch.error.nomainclassspec", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0 \ub300\ud55c \uae30\ubcf8 \ud074\ub798\uc2a4\uac00 \uc9c0\uc815\ub418\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4." },
        { "launch.error.nomainjar", "\uae30\ubcf8 JAR \ud30c\uc77c\uc774 \uc9c0\uc815\ub418\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4." },
        { "launch.error.nonstaticmainmethod", "main() \uba54\uc18c\ub4dc\ub294 \uc815\uc801\uc774\uc5b4\uc57c \ud569\ub2c8\ub2e4." },
        { "launch.error.offlinemissingresource", "\ud544\uc694\ud55c \ubaa8\ub4e0 \uc790\uc6d0\uc744 \ub85c\uceec\uc5d0 \ub2e4\uc6b4\ub85c\ub4dc\ud558\uc9c0 \uc54a\uc558\uae30 \ub54c\ubb38\uc5d0 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \uc624\ud504\ub77c\uc778\uc73c\ub85c \uc2e4\ud589\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "launch.error.parse", "\uc2dc\uc791 \ud30c\uc77c\uc744 \uad6c\ubb38 \ubd84\uc11d\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. {0, number} \ud589\uc5d0 \uc624\ub958\uac00 \uc788\uc2b5\ub2c8\ub2e4." },
        { "launch.error.parse-signedjnlp", "\uc11c\uba85\ub41c \uc2dc\uc791 \ud30c\uc77c\uc744 \uad6c\ubb38 \ubd84\uc11d\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. {0, number} \ud5f9\uc5d0 \uc624\ub958\uac00 \uc788\uc2b5\ub2c8\ub2e4." },
        { "launch.error.resourceID", "{0}" },
        { "launch.error.resourceID-version", "({0}, {1})" },
        { "launch.error.singlecertviolation", "JNLP \ud30c\uc77c\uc758 JAR \uc790\uc6d0\uc774 \ub3d9\uc77c\ud55c \uc778\uc99d\uc11c\ub85c \uc11c\uba85\ub418\uc9c0 \uc54a\uc558\uc2b5\ub2c8\ub2e4." },
        { "launch.error.toomanyargs", "\ub108\ubb34 \ub9ce\uc740 \uc778\uc218 \uacf5\uae09: {0}" },
        { "launch.error.unsignedAccessViolation", "\uc11c\uba85\ub418\uc9c0 \uc54a\uc740 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c \uc2dc\uc2a4\ud15c\uc5d0 \ubb34\uc81c\ud55c \uc561\uc138\uc2a4\ub97c \uc694\uccad\ud569\ub2c8\ub2e4." },
        { "launch.error.unsignedResource", "\uc11c\uba85\ub418\uc9c0 \uc54a\uc740 \uc790\uc6d0: {0}" },
        { "launch.estimatedTimeLeft", "\uc608\uc0c1\ub41c \ub0a8\uc740 \uc2dc\uac04: {0,number,00}:{1,number,00}:{2,number,00}" },
        { "launch.extensiondownload", "\ud655\uc7a5 \uc124\uba85\uc790\ub97c \ub2e4\uc6b4\ub85c\ub4dc\ud558\ub294 \uc911({0} \ub0a8\uc74c)" },
        { "launch.extensiondownload-name", "{0} \uc124\uba85\uc790\ub97c \ub2e4\uc6b4\ub85c\ub4dc\ud558\ub294 \uc911({1} \ub0a8\uc74c)" },
        { "launch.initializing", "\ucd08\uae30\ud654\ud558\ub294 \uc911..." },
        { "launch.launchApplication", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \uc2dc\uc791\ud558\ub294 \uc911..." },
        { "launch.launchInstaller", "\uc124\uce58 \ud504\ub85c\uadf8\ub7a8\uc744 \uc2dc\uc791\ud558\ub294 \uc911..." },
        { "launch.launchingExtensionInstaller", "\uc124\uce58 \ud504\ub85c\uadf8\ub7a8\uc744 \uc2e4\ud589 \uc911\uc785\ub2c8\ub2e4. \uc7a0\uc2dc \uae30\ub2e4\ub824 \uc8fc\uc2ed\uc2dc\uc624..." },
        { "launch.loadingNetProgress", "{0} \uc77d\uae30" },
        { "launch.loadingNetProgressPercent", "={1}\uc758 {0} \uc77d\uae30 ({2}%)." },
        { "launch.loadingNetStatus", "{1}\uc5d0\uc11c {0}\uc744(\ub97c) \ub85c\ub4dc\ud558\ub294 \uc911" },
        { "launch.loadingResourceFailed", "\uc790\uc6d0\uc744 \ub85c\ub4dc\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },
        { "launch.loadingResourceFailedSts", "{0} \uc694\uccad" },
        { "launch.patchingStatus", "{1}\uc5d0\uc11c {0} \ud328\uce58 \uc911" },
        { "launch.progressScreen", "\ucd5c\uc2e0 \ubc84\uc804 \ud655\uc778 \uc911..." },
        { "launch.stalledDownload", "\ub370\uc774\ud130\ub97c \uae30\ub2e4\ub9ac\ub294 \uc911..." },
        { "launch.validatingProgress", "=\ud56d\ubaa9\uc744 \uc2a4\uce90\ub2dd\ud558\ub294 \uc911({0}% \uc644\ub8cc)" },
        { "launch.validatingStatus", "{1}\uc5d0\uc11c {0}\uc744(\ub97c) \ud655\uc778\ud558\ub294 \uc911" },
        { "launcherrordialog.abort", "\uc911\uc9c0" },
        { "launcherrordialog.abortMnemonic", "VK_A" },
        { "launcherrordialog.brief.continue", "\uacc4\uc18d \uc2e4\ud589\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "launcherrordialog.brief.details", "\uc138\ubd80 \uc815\ubcf4" },
        { "launcherrordialog.brief.message", "\uc9c0\uc815\ud55c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \uc2dc\uc791\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "launcherrordialog.import.brief.message", "\uc9c0\uc815\ud55c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \uac00\uc838\uc62c \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "launcherrordialog.brief.messageKnown", "{0}\uc744(\ub97c) \uc2dc\uc791\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
	{ "launcherrordialog.import.brief.messageKnown", "{0}\uc744(\ub97c) \uac00\uc838\uc62c \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "launcherrordialog.brief.ok", "\ud655\uc778" },
        { "launcherrordialog.brief.title", "Java Web Start - {0}" },
        { "launcherrordialog.consoleTab", "\ucf58\uc194" },
        { "launcherrordialog.errorcategory", "\ubc94\uc8fc: {0}\n\n" },
        { "launcherrordialog.errorintro", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \uc2dc\uc791/\uc2e4\ud589\ud558\ub294 \ub3c4\uc911 \uc624\ub958\uac00 \ubc1c\uc0dd\ud588\uc2b5\ub2c8\ub2e4.\n\n" },
	{ "launcherrordialog.import.errorintro", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \uac00\uc838\uc624\ub294 \ub3c4\uc911 \uc624\ub958\uac00 \ubc1c\uc0dd\ud588\uc2b5\ub2c8\ub2e4.\n\n" },
        { "launcherrordialog.errormsg", "{0}" },
        { "launcherrordialog.errortitle", "\uc81c\ubaa9: {0}\n" },
        { "launcherrordialog.errorvendor", "\uacf5\uae09\uc5c5\uccb4: {0}\n" },
        { "launcherrordialog.exceptionTab", "\uc608\uc678" },
        { "launcherrordialog.generalTab", "\uc77c\ubc18" },
        { "launcherrordialog.genericerror", "\uc608\uae30\uce58 \uc54a\uc740 \uc608\uc678: {0}" },
        { "launcherrordialog.jnlpMainTab", "\uae30\ubcf8 \uc2dc\uc791 \ud30c\uc77c" },
        { "launcherrordialog.jnlpTab", "\ud30c\uc77c \uc2dc\uc791" },
        { "launcherrordialog.title", "Java Web Start - {0}" },
        { "launcherrordialog.wrappedExceptionTab", "\uc904\ubc14\uafc8 \uc608\uc678" },

        { "uninstall.failedMessage", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \uc81c\uac70\ub97c \uc644\ub8cc\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "uninstall.failedMessageTitle", "\uc81c\uac70" },
        { "install.alreadyInstalled", "{0}\uc5d0 \ub300\ud55c \ub2e8\ucd95\ud0a4\uac00 \uc774\ubbf8 \uc788\uc2b5\ub2c8\ub2e4. \uadf8\ub798\ub3c4 \ub2e8\ucd95\ud0a4\ub97c \ub9cc\ub4dc\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "install.alreadyInstalledTitle", "\ub2e8\ucd95\ud0a4 \ub9cc\ub4e4\uae30..." },
        { "install.desktopShortcutName", "{0}" },
        { "install.installFailed", "{0}\uc758 \ub2e8\ucd95\ud0a4\ub97c \ub9cc\ub4e4 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },
        { "install.installFailedTitle", "\ub2e8\ucd95\ud0a4 \ub9cc\ub4e4\uae30" },
        { "install.startMenuShortcutName", "{0}" },
        { "install.startMenuUninstallShortcutName", "{0} \uc81c\uac70" },        
        { "install.uninstallFailed", "{0}\uc758 \ub2e8\ucd95\ud0a4\ub97c \uc81c\uac70\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. \ub2e4\uc2dc \uc2dc\ub3c4\ud558\uc2ed\uc2dc\uc624." },
        { "install.uninstallFailedTitle", "\ub2e8\ucd95\ud0a4 \uc81c\uac70" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "\uc2dc\uc2a4\ud15c\uc758 deployment.config \ud30c\uc77c\uc5d0\uc11c \uc5d4\ud130\ud504\ub77c\uc774\uc988 \uad6c\uc131 \ud30c\uc77c\uc774 \ud544\uc218\ub85c \uc9c0\uc815\ub418\uc5b4 \uc788\uc73c\uba70 \ud544\uc218 {0}\uc744(\ub97c) \uc0ac\uc6a9\ud560 \uc218 \uc5c6\uae30 \ub54c\ubb38\uc5d0 \uc774 \ud504\ub85c\uadf8\ub7a8\uc744 \uc2e4\ud589\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },

	// Jnlp Cache Viewer:
	{ "jnlp.viewer.title", "Java \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \uce90\uc2dc \ubdf0\uc5b4" },
	{ "jnlp.viewer.all", "\ubaa8\ub450" },
	{ "jnlp.viewer.type", "{0}" },
	{ "jnlp.viewer.totalSize",  "\uc804\uccb4 \uc790\uc6d0 \ud06c\uae30:  {0}" },
 	{ "jnlp.viewer.emptyCache", "{0} \uce90\uc2dc\uac00 \ube44\uc5b4 \uc788\uc2b5\ub2c8\ub2e4."},
 	{ "jnlp.viewer.noCache", "\uad6c\uc131\ub41c \uc2dc\uc2a4\ud15c \uce90\uc2dc\uac00 \uc5c6\uc2b5\ub2c8\ub2e4."},

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

	{ "jnlp.viewer.remove.btn", "\uc81c\uac70(R)" },
	{ "jnlp.viewer.remove.1.btn", "\uc120\ud0dd\ud55c {0} \uc81c\uac70(R)" },
	{ "jnlp.viewer.remove.2.btn", "\uc120\ud0dd\ud55c \ud56d\ubaa9 \uc81c\uac70(R)" },
	{ "jnlp.viewer.uninstall.btn", "\uc81c\uac70" },
	{ "jnlp.viewer.launch.offline.btn", "\uc624\ud504\ub77c\uc778 \uc2dc\uc791(L)" },
	{ "jnlp.viewer.launch.online.btn", "\uc628\ub77c\uc778 \uc2dc\uc791(N)" },

        { "jnlp.viewer.file.menu", "\ud30c\uc77c(F)" },
        { "jnlp.viewer.edit.menu", "\ud3b8\uc9d1(E)" },
        { "jnlp.viewer.app.menu", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8(A)" },
        { "jnlp.viewer.view.menu", "\ubcf4\uae30(V)" },
        { "jnlp.viewer.help.menu", "\ub3c4\uc6c0\ub9d0(H)" },

	{ "jnlp.viewer.cpl.mi", "Java \uc81c\uc5b4\ud310 \uc2dc\uc791(C)" },
	{ "jnlp.viewer.exit.mi", "\uc885\ub8cc(X)" },

	{ "jnlp.viewer.reinstall.mi", "\ub2e4\uc2dc \uc124\uce58(R) ..." },
	{ "jnlp.viewer.preferences.mi", "\uae30\ubcf8 \uc124\uc815(P) ..." },

	{ "jnlp.viewer.launch.offline.mi", "\uc624\ud504\ub77c\uc778 \uc2dc\uc791(L)" },
	{ "jnlp.viewer.launch.online.mi", "\uc628\ub77c\uc778 \uc2dc\uc791(N)" },
	{ "jnlp.viewer.install.mi", "\ub2e8\ucd95\ud0a4 \uc124\uce58(I)" },
	{ "jnlp.viewer.uninstall.mi", "\ub2e8\ucd95\ud0a4 \uc81c\uac70(U)" },
	{ "jnlp.viewer.remove.0.mi", "\uc81c\uac70(R)" },
	{ "jnlp.viewer.remove.mi", "{0} \uc81c\uac70(R)" },
	{ "jnlp.viewer.show.mi", "JNLP \uc124\uba85\uc790 \ud45c\uc2dc(S)" },
	{ "jnlp.viewer.browse.mi", "\ud648 \ud398\uc774\uc9c0 \ucc3e\uc544\ubcf4\uae30(B)" },

	{ "jnlp.viewer.view.0.mi", "\ubaa8\ub4e0 \uc720\ud615(T)" },
	{ "jnlp.viewer.view.1.mi", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8(A)" },
	{ "jnlp.viewer.view.2.mi", "\uc560\ud50c\ub9bf(P)" },
	{ "jnlp.viewer.view.3.mi", "\ub77c\uc774\ube0c\ub7ec\ub9ac(L)" },
	{ "jnlp.viewer.view.4.mi", "\uc124\uce58 \ud504\ub85c\uadf8\ub7a8(I)" },

	{ "jnlp.viewer.view.0", "\ubaa8\ub4e0 \uc720\ud615" },
	{ "jnlp.viewer.view.1", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8" },
	{ "jnlp.viewer.view.2", "\uc560\ud50c\ub9bf" },
	{ "jnlp.viewer.view.3", "\ub77c\uc774\ube0c\ub7ec\ub9ac" },
	{ "jnlp.viewer.view.4", "\uc124\uce58 \ud504\ub85c\uadf8\ub7a8" },

	{ "jnlp.viewer.about.mi", "\uc815\ubcf4(A)" },
	{ "jnlp.viewer.help.java.mi", "J2SE \ud648 \ud398\uc774\uc9c0(J)" },
	{ "jnlp.viewer.help.jnlp.mi", "JNLP \ud648 \ud398\uc774\uc9c0(H)" },

        { "jnlp.viewer.app.column", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8" },
        { "jnlp.viewer.vendor.column", "\uacf5\uae09\uc5c5\uccb4" },
        { "jnlp.viewer.type.column", "\uc720\ud615" },
        { "jnlp.viewer.size.column", "\ud06c\uae30" },
        { "jnlp.viewer.date.column", "\ub0a0\uc9dc" },
        { "jnlp.viewer.status.column", "\uc0c1\ud0dc" },

        { "jnlp.viewer.app.column.tooltip", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8, \uc560\ud50c\ub9bf \ub610\ub294 \ud655\uc7a5\uc758 \uc774\ub984 \ubc0f \uc544\uc774\ucf58" },
        { "jnlp.viewer.vendor.column.tooltip", "\uc774 \ud56d\ubaa9\uc744 \uc0ac\uc6a9\ud558\uace0 \uc788\ub294 \ud68c\uc0ac" },
        { "jnlp.viewer.type.column.tooltip", "\uc774 \ud56d\ubaa9\uc758 \uc720\ud615" },
        { "jnlp.viewer.size.column.tooltip", "\uc774 \ud56d\ubaa9\uc758 \ud06c\uae30 \ubc0f \ubaa8\ub4e0 \uc790\uc6d0" },
        { "jnlp.viewer.date.column.tooltip", "\uc774 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8, \uc560\ud50c\ub9bf \ub610\ub294 \uc124\uce58 \ud504\ub85c\uadf8\ub7a8\uc774 \ub9c8\uc9c0\ub9c9\uc73c\ub85c \uc2e4\ud589\ub41c \ub0a0\uc9dc" },
        { "jnlp.viewer.status.column.tooltip", "\uc774 \ud56d\ubaa9\uc744 \uc2dc\uc791\ud560 \uc218 \uc788\ub294\uc9c0\uc640 \uadf8 \uc2dc\uc791 \ubc29\ubc95\uc744 \ubcf4\uc5ec\uc8fc\ub294 \uc544\uc774\ucf58" },


        { "jnlp.viewer.application", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8" },
        { "jnlp.viewer.applet", "\uc560\ud50c\ub9bf" },
        { "jnlp.viewer.extension", "\ub77c\uc774\ube0c\ub7ec\ub9ac" },
        { "jnlp.viewer.installer", "\uc124\uce58 \ud504\ub85c\uadf8\ub7a8" },

	{ "jnlp.viewer.offline.tooltip",
		 "\uc774 {0}\uc740(\ub294) \uc628\ub77c\uc778 \ub610\ub294 \uc624\ud504\ub77c\uc778\uc73c\ub85c \uc2dc\uc791\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
	{ "jnlp.viewer.online.tooltip", "\uc774 {0}\uc740(\ub294) \uc628\ub77c\uc778\uc73c\ub85c \uc2dc\uc791\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
	{ "jnlp.viewer.norun1.tooltip", 
        	"\uc774 {0}\uc740(\ub294) \uc6f9 \ube0c\ub77c\uc6b0\uc800\uc5d0\uc11c\ub9cc \uc2dc\uc791\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
        { "jnlp.viewer.norun2.tooltip", "\ud655\uc7a5\uc744 \uc2dc\uc791\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4." },

	{ "jnlp.viewer.show.title", "JNLP \uc124\uba85\uc790 \ub300\uc0c1: {0}" },

	{ "jnlp.viewer.removing", "\uc81c\uac70 \uc911 ..." },
	{ "jnlp.viewer.launching", "\uc2dc\uc791 \uc911 ..." },
	{ "jnlp.viewer.browsing", "\ube0c\ub77c\uc6b0\uc800 \uc2dc\uc791 \uc911 ..." },
	{ "jnlp.viewer.sorting", "\ud56d\ubaa9 \uc815\ub82c \uc911 ..." },
	{ "jnlp.viewer.searching", "\ud56d\ubaa9 \uac80\uc0c9 \uc911 ..." },
	{ "jnlp.viewer.installing", "\uc124\uce58\ud558\ub294 \uc911 ..." },

        { "jnlp.viewer.reinstall.title", "\uc81c\uac70\ub41c JNLP \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \ub2e4\uc2dc \uc124\uce58" },
	{ "jnlp.viewer.reinstallBtn", "\uc120\ud0dd\ud55c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \ub2e4\uc2dc \uc124\uce58(R)" },
	{ "jnlp.viewer.reinstallBtn.mnemonic", "VK_R" },
        { "jnlp.viewer.closeBtn", "\ub2eb\uae30(C)" },
        { "jnlp.viewer.closeBtn.mnemonic", "VK_C" },

	{ "jnlp.viewer.reinstall.column.title", "\uc81c\ubaa9:" },
	{ "jnlp.viewer.reinstall.column.location", "\uc704\uce58:" },

	// cache size warning
	{ "jnlp.cache.warning.title", "JNLP \uce90\uc2dc \ud06c\uae30 \uacbd\uace0" },
	{ "jnlp.cache.warning.message", "\uacbd\uace0: \n\n"+
		"\uce90\uc2dc\uc5d0\uc11c JNLP \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \ubc0f \uc790\uc6d0\uc5d0 \uad8c\uc7a5\ub418\ub294\n"+
		"\ub514\uc2a4\ud06c \uacf5\uac04\uc758 \ud06c\uae30\ub97c \ucd08\uacfc\ud588\uc2b5\ub2c8\ub2e4.\n\n"+
		"\ud604\uc7ac \uc0ac\uc6a9 \ud06c\uae30: {0}\n"+
		"\uad8c\uc7a5 \uc81c\ud55c: {1}\n\n"+
		"Java \uc81c\uc5b4\ud310\uc744 \uc0ac\uc6a9\ud558\uc5ec \uc77c\ubd80 \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \ub610\ub294 \uc790\uc6d0\uc744\n"+
		"\uc81c\uac70\ud558\uac70\ub098 \ub354 \ub192\uc740 \uc81c\ud55c \uac12\uc744 \uc124\uc815\ud558\uc2ed\uc2dc\uc624." },

        // Control Panel
        { "control.panel.title", "Java \uc81c\uc5b4\ud310" },
        { "control.panel.general", "\uc77c\ubc18" },
        { "control.panel.security", "\ubcf4\uc548" },
        { "control.panel.java", "Java" },
        { "control.panel.update", "\uc5c5\ub370\uc774\ud2b8" },
        { "control.panel.advanced", "\uace0\uae09" },

        // Common Strings used in different panels.
        { "common.settings", "\uc124\uc815" },
        { "common.ok_btn", "\ud655\uc778" },
        { "common.ok_btn.mnemonic", "VK_O" },
        { "common.cancel_btn", "\ucde8\uc18c" },
        { "common.cancel_btn.mnemonic", "VK_C" },
        { "common.apply_btn", "\uc801\uc6a9" },
        { "common.apply_btn.mnemonic", "VK_A" },
        { "common.add_btn", "\ucd94\uac00" },
        { "common.add_btn.mnemonic", "VK_A" },
        { "common.remove_btn", "\uc81c\uac70" },
        { "common.remove_btn.mnemonic", "VK_R" },

        // Network Settings Dialog
        { "network.settings.dlg.title", "\ub124\ud2b8\uc6cc\ud06c \uc124\uc815" },
        { "network.settings.dlg.border_title", " \ub124\ud2b8\uc6cc\ud06c \ud504\ub85d\uc2dc \uc124\uc815 " },
        { "network.settings.dlg.browser_rbtn", "\ube0c\ub77c\uc6b0\uc800 \uc124\uc815 \uc0ac\uc6a9" },
        { "browser_rbtn.mnemonic", "VK_B" },
        { "network.settings.dlg.manual_rbtn", "\ud504\ub85d\uc2dc \uc11c\ubc84 \uc0ac\uc6a9" },
        { "manual_rbtn.mnemonic", "VK_P" },
        { "network.settings.dlg.address_lbl", "\uc8fc\uc18c:" },
	{ "network.settings.dlg.port_lbl", "\ud3ec\ud2b8:" },
        { "network.settings.dlg.advanced_btn", "\uace0\uae09..." },
        { "network.settings.dlg.advanced_btn.mnemonic", "VK_A" },
        { "network.settings.dlg.bypass_text", "\ub85c\uceec \uc8fc\uc18c\uc5d0 \ub300\ud574 \ud504\ub85d\uc2dc \uc11c\ubc84 \uc6b0\ud68c" },
        { "network.settings.dlg.bypass.mnemonic", "VK_Y" },
        { "network.settings.dlg.autoconfig_rbtn", "\uc790\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131 \uc2a4\ud06c\ub9bd\ud2b8 \uc0ac\uc6a9" },
        { "autoconfig_rbtn.mnemonic", "VK_T" },
        { "network.settings.dlg.location_lbl", "\uc2a4\ud06c\ub9bd\ud2b8 \uc704\uce58: " },
        { "network.settings.dlg.direct_rbtn", "\uc9c1\uc811 \uc5f0\uacb0" },
        { "direct_rbtn.mnemonic", "VK_D" },
        { "network.settings.dlg.browser_text", "\uc790\ub3d9 \uad6c\uc131\uc740 \uc218\ub3d9 \uc124\uc815\uc744 \ub300\uccb4\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4. \uc218\ub3d9 \uc124\uc815\uc744 \uc0ac\uc6a9\ud558\ub824\uba74 \uc790\ub3d9 \uad6c\uc131\uc744 \ube44\ud65c\uc131\ud654\ud558\uc2ed\uc2dc\uc624." },
        { "network.settings.dlg.proxy_text", "\ube0c\ub77c\uc6b0\uc800 \ud504\ub85d\uc2dc \uc124\uc815\uc744 \ub36e\uc5b4\uc501\ub2c8\ub2e4." },
        { "network.settings.dlg.auto_text", "\uc9c0\uc815\ub41c \uc704\uce58\uc5d0 \uc788\ub294 \uc790\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131 \uc2a4\ud06c\ub9bd\ud2b8\ub97c \uc0ac\uc6a9\ud569\ub2c8\ub2e4." },
        { "network.settings.dlg.none_text", "\uc9c1\uc811 \uc5f0\uacb0\uc744 \uc0ac\uc6a9\ud569\ub2c8\ub2e4." },

        // Advanced Network Settings Dialog
        { "advanced.network.dlg.title", "\uace0\uae09 \ub124\ud2b8\uc6cc\ud06c \uc124\uc815" },
        { "advanced.network.dlg.servers", " \uc11c\ubc84 " },
        { "advanced.network.dlg.type", "\uc720\ud615" },
        { "advanced.network.dlg.http", "HTTP:" },
        { "advanced.network.dlg.secure", "\ubcf4\uc548:" },
        { "advanced.network.dlg.ftp", "FTP:" },
        { "advanced.network.dlg.socks", "Socks:" },
        { "advanced.network.dlg.proxy_address", "\ud504\ub85d\uc2dc \uc8fc\uc18c" },
	{ "advanced.network.dlg.port", "\ud3ec\ud2b8" },
        { "advanced.network.dlg.same_proxy", " \ubaa8\ub4e0 \ud504\ub85c\ud1a0\ucf5c\uc5d0 \uac19\uc740 \ud504\ub85d\uc2dc \uc11c\ubc84 \uc0ac\uc6a9" },
        { "advanced.network.dlg.same_proxy.mnemonic", "VK_U" },
        { "advanced.network.dlg.exceptions", " \uc608\uc678 " },
        { "advanced.network.dlg.no_proxy", " \ub2e4\uc74c\uc73c\ub85c \uc2dc\uc791\ud558\ub294 \uc8fc\uc18c\uc5d0 \ud504\ub85d\uc2dc \uc11c\ubc84 \uc0ac\uc6a9 \uc548 \ud568" },
        { "advanced.network.dlg.no_proxy_note", " \uc138\ubbf8\ucf5c\ub860(;)\uc744 \uc0ac\uc6a9\ud558\uc5ec \ud56d\ubaa9\uc744 \uad6c\ubd84\ud569\ub2c8\ub2e4." },

        // DeleteFilesDialog
        { "delete.files.dlg.title", "\uc784\uc2dc \ud30c\uc77c \uc0ad\uc81c" },
        { "delete.files.dlg.temp_files", "\ub2e4\uc74c \uc784\uc2dc \ud30c\uc77c\uc744 \uc0ad\uc81c\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "delete.files.dlg.applets", "\ub2e4\uc6b4\ub85c\ub4dc\ub41c \uc560\ud50c\ub9bf" },
        { "delete.files.dlg.applications", "\ub2e4\uc6b4\ub85c\ub4dc\ub41c \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8" },
        { "delete.files.dlg.other", "\uae30\ud0c0 \ud30c\uc77c" },

	// General
	{ "general.cache.border.text", " \uc784\uc2dc \uc778\ud130\ub137 \ud30c\uc77c " },
	{ "general.cache.delete.text", "\ud30c\uc77c \uc0ad\uc81c..." },
        { "general.cache.delete.text.mnemonic", "VK_D" },
	{ "general.cache.settings.text", "\uc124\uc815..." },
        { "general.cache.settings.text.mnemonic", "VK_S" },
	{ "general.cache.desc.text", "Java \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c \uc0ac\uc6a9\ud558\ub294 \ud30c\uc77c\uc740 \ub098\uc911\uc5d0 \ube68\ub9ac \uc2e4\ud589\ud560 \uc218 \uc788\ub3c4\ub85d \ud2b9\uc218 \ud3f4\ub354\uc5d0 \uc800\uc7a5\ub429\ub2c8\ub2e4. \uace0\uae09 \uc0ac\uc6a9\uc790\ub9cc \ud30c\uc77c\uc744 \uc0ad\uc81c\ud558\uac70\ub098 \uc774 \uc124\uc815\uc744 \uc218\uc815\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
	{ "general.network.border.text", " \ub124\ud2b8\uc6cc\ud06c \uc124\uc815 " },
	{ "general.network.settings.text", "\ub124\ud2b8\uc6cc\ud06c \uc124\uc815..." },
        { "general.network.settings.text.mnemonic", "VK_N" },
	{ "general.network.desc.text", "\ub124\ud2b8\uc6cc\ud06c \uc124\uc815\uc740 \uc778\ud130\ub137 \uc5f0\uacb0 \uc2dc\uc5d0 \uc0ac\uc6a9\ub429\ub2c8\ub2e4. \uae30\ubcf8\uc801\uc73c\ub85c, Java\ub294 \uc6f9 \ube0c\ub77c\uc6b0\uc800\uc758 \ub124\ud2b8\uc6cc\ud06c \uc124\uc815\uc744 \uc0ac\uc6a9\ud569\ub2c8\ub2e4. \uace0\uae09 \uc0ac\uc6a9\uc790\ub9cc \uc774 \uc124\uc815\uc744 \uc218\uc815\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
        { "general.about.border", "\uc815\ubcf4" },
        { "general.about.text", "Java \uc81c\uc5b4\ud310\uc758 \ubc84\uc804 \uc815\ubcf4\ub97c \ubd05\ub2c8\ub2e4." },
        { "general.about.btn", "\uc815\ubcf4..." },
        { "general.about.btn.mnemonic", "VK_B" },


	// Security
	{ "security.certificates.border.text", " \uc778\uc99d\uc11c " },
	{ "security.certificates.button.text", "\uc778\uc99d\uc11c..." },
        { "security.certificates.button.mnemonic", "VK_E" },
	{ "security.certificates.desc.text", "\uc778\uc99d\uc11c\ub97c \uc0ac\uc6a9\ud558\uc5ec \uc0ac\uc6a9\uc790, \uc778\uc99d\uc11c, \uad8c\ud55c \ubc0f \ubc1c\ud589\uc778\uc744 \uba85\ud655\ud788 \ud655\uc778\ud569\ub2c8\ub2e4." },
	{ "security.policies.border.text", " \uc815\ucc45 " },
	{ "security.policies.advanced.text", "\uace0\uae09..." },
        { "security.policies.advanced.mnemonic", "VK_D" },
	{ "security.policies.desc.text", "\ubcf4\uc548 \uc815\ucc45\uc744 \uc0ac\uc6a9\ud558\uc5ec \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uacfc \uc560\ud50c\ub9bf\uc744 \ub458\ub7ec\uc2fc \ubcf4\uc548 \uc601\uc5ed\uc744 \uc81c\uc5b4\ud569\ub2c8\ub2e4." },

	// Update
	{ "update.notify.border.text", " \uc5c5\ub370\uc774\ud2b8 \uc54c\ub9bc " }, // this one is not currently used.  See update panel!!!
	{ "update.updatenow.button.text", "\uc9c0\uae08 \uc5c5\ub370\uc774\ud2b8" },
	{ "update.updatenow.button.mnemonic", "VK_U" },
	{ "update.advanced.button.text", "\uace0\uae09..." },
	{ "update.advanced.button.mnemonic", "VK_D" },
	{ "update.desc.text", "Java \uc5c5\ub370\uc774\ud2b8 \uba54\ucee4\ub2c8\uc998\uc744 \uc0ac\uc6a9\ud558\uba74 \ud56d\uc0c1 \ucd5c\uc2e0 \ubc84\uc804\uc758 Java \ud50c\ub7ab\ud3fc\uc744 \uad6c\ube44\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4. \uc544\ub798 \uc635\uc158\uc744 \uc0ac\uc6a9\ud558\uba74 \uc5c5\ub370\uc774\ud2b8\ub97c \uc5bb\uace0 \uc801\uc6a9\ud558\ub294 \ubc29\ubc95\uc744 \uc81c\uc5b4\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
        { "update.notify.text", "\uc0ac\uc6a9\uc790 \uc54c\ub9bc:" },
        { "update.notify_install.text", "\uc124\uce58 \uc804" },
        { "update.notify_download.text", "\ub2e4\uc6b4\ub85c\ub4dc \uc804 \ubc0f \uc124\uce58 \uc804" },
        { "update.autoupdate.text", "\uc790\ub3d9 \uc5c5\ub370\uc774\ud2b8 \ud655\uc778" },
        { "update.advanced_title.text", "\uc790\ub3d9 \uc5c5\ub370\uc774\ud2b8 \uace0\uae09 \uc124\uc815" },
        { "update.advanced_title1.text", "\uc2a4\uce94 \uc218\ud589 \ube48\ub3c4 \ubc0f \uc2dc\uae30\ub97c \uc120\ud0dd\ud569\ub2c8\ub2e4." },
        { "update.advanced_title2.text", "\ube48\ub3c4" },
        { "update.advanced_title3.text", "\uc2dc\uae30" },
        { "update.advanced_desc1.text", "\ub9e4\uc77c {0}\uc5d0 \uc2a4\uce94" },
        { "update.advanced_desc2.text", "\ub9e4 {0}\ub9c8\ub2e4 {1}\uc5d0 \uc2a4\uce94" },
        { "update.advanced_desc3.text", "\ub9e4\uc6d4 {0}\uc77c {1}\uc5d0 \uc2a4\uce94" },
        { "update.check_daily.text", "\uc77c\ubcc4" },
        { "update.check_weekly.text", "\uc8fc\ubcc4" },
        { "update.check_monthly.text", "\uc6d4\ubcc4" },
        { "update.check_date.text", "\ub0a0\uc9dc:" },
        { "update.check_day.text", "\uc694\uc77c:" },
        { "update.check_time.text", "\uc2dc\uac04:" },
        { "update.lastrun.text", "Java Update\uac00 {1} {0}\uc5d0  \ub9c8\uc9c0\ub9c9\uc73c\ub85c \uc2e4\ud589\ub418\uc5c8\uc2b5\ub2c8\ub2e4." },
        { "update.desc_autooff.text", "\uc5c5\ub370\uc774\ud2b8\ub97c \ud655\uc778\ud558\ub824\uba74 \uc544\ub798\uc758 \"\uc9c0\uae08 \uc5c5\ub370\uc774\ud2b8\" \ubc84\ud2bc\uc744 \ub204\ub974\uc2ed\uc2dc\uc624. \uc5c5\ub370\uc774\ud2b8\uac00 \uac00\ub2a5\ud55c \uacbd\uc6b0\uc5d0\ub294 \uc2dc\uc2a4\ud15c \ud2b8\ub808\uc774\uc5d0 \uc544\uc774\ucf58\uc774 \ub098\ud0c0\ub0a9\ub2c8\ub2e4. \ucee4\uc11c\ub97c \uc544\uc774\ucf58 \uc704\ub85c \uac00\uc838\uac00\uba74 \uc5c5\ub370\uc774\ud2b8 \uc0c1\ud0dc\ub97c \ubcfc \uc218 \uc788\uc2b5\ub2c8\ub2e4." },
        { "update.desc_check_daily.text", "\ub9e4\uc77c {0}\uc5d0 Java Update\uc5d0\uc11c \uc5c5\ub370\uc774\ud2b8\ub97c \ud655\uc778\ud569\ub2c8\ub2e4. " },
        { "update.desc_check_weekly.text", "\ub9e4 {0}\ub9c8\ub2e4 {1}\uc5d0 Java Update\uc5d0\uc11c \uc5c5\ub370\uc774\ud2b8\ub97c \ud655\uc778\ud569\ub2c8\ub2e4. " },
        { "update.desc_check_monthly.text", "\ub9e4\uc6d4 {0}\uc77c {1}\uc5d0 Java Update\uc5d0\uc11c \uc5c5\ub370\uc774\ud2b8\ub97c \ud655\uc778\ud569\ub2c8\ub2e4. " },
        { "update.desc_systrayicon.text", "\uc5c5\ub370\uc774\ud2b8\uac00 \uac00\ub2a5\ud55c \uacbd\uc6b0\uc5d0\ub294 \uc2dc\uc2a4\ud15c \ud2b8\ub808\uc774\uc5d0 \uc544\uc774\ucf58\uc774 \ub098\ud0c0\ub0a9\ub2c8\ub2e4. \ucee4\uc11c\ub97c \uc544\uc774\ucf58 \uc704\ub85c \uac00\uc838\uac00\uba74 \uc5c5\ub370\uc774\ud2b8\uc758 \uc0c1\ud0dc\ub97c \ubcfc \uc218 \uc788\uc2b5\ub2c8\ub2e4. " },
        { "update.desc_notify_install.text", "\uc5c5\ub370\uc774\ud2b8\ub97c \uc124\uce58\ud558\uae30 \uc804\uc5d0 \uc54c\ub9bc \uba54\uc2dc\uc9c0\uac00 \ud45c\uc2dc\ub429\ub2c8\ub2e4." },
        { "update.desc_notify_download.text", "\uc5c5\ub370\uc774\ud2b8\uac00 \ub2e4\uc6b4\ub85c\ub4dc\ub418\uace0 \uc124\uce58\ub418\uae30 \uc804\uc5d0 \uc54c\ub9bc \uba54\uc2dc\uc9c0\uac00 \ud45c\uc2dc\ub429\ub2c8\ub2e4." },
	{ "update.launchbrowser.error.text", "Java Update Checker\ub97c \uc2dc\uc791\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4. \ucd5c\uc2e0 Java \uc5c5\ub370\uc774\ud2b8\ub97c \uc5bb\uc73c\ub824\uba74 http://java.sun.com/getjava/javaupdate\ub85c \uc774\ub3d9\ud558\uc2ed\uc2dc\uc624." },
	{ "update.launchbrowser.error.caption", "\uc624\ub958 - \uc5c5\ub370\uc774\ud2b8" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "\ud30c\uc77c \uc0ad\uc81c..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_D" },
        { "cache.settings.dialog.view_jws_btn", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \ubcf4\uae30..." },
        { "cache.settings.dialog.view_jws_btn.mnemonic", "VK_V" },
        { "cache.settings.dialog.view_jpi_btn", "\uc560\ud50c\ub9bf \ubcf4\uae30..." },
        { "cache.settings.dialog.view_jpi_btn.mnemonic", "VK_A" },
        { "cache.settings.dialog.chooser_title", "\uc784\uc2dc \ud30c\uc77c \uc704\uce58" },
        { "cache.settings.dialog.select", "\uc120\ud0dd" },
        { "cache.settings.dialog.select_tooltip", "\uc120\ud0dd\ud55c \uc704\uce58 \uc0ac\uc6a9" },
        { "cache.settings.dialog.select_mnemonic", "S" },
        { "cache.settings.dialog.title", "\uc784\uc2dc \ud30c\uc77c \uc124\uc815" },
        { "cache.settings.dialog.cache_location", "\uc704\uce58:" },
        { "cache.settings.dialog.change_btn", "\ubcc0\uacbd..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_H" },
        { "cache.settings.dialog.disk_space", "\uc0ac\uc6a9\ud560 \ub514\uc2a4\ud06c \uacf5\uac04 \ud06c\uae30:" },
        { "cache.settings.dialog.unlimited_btn", "\uc81c\ud55c \uc5c6\uc74c" },
        { "cache.settings.dialog.max_btn", "\ucd5c\ub300" },
        { "cache.settings.dialog.compression", "Jar \uc555\ucd95:" },
        { "cache.settings.dialog.none", "\uc5c6\uc74c" },
        { "cache.settings.dialog.high", "\ub192\uc74c" },

	// JNLP File/MIME association dialog strings:
	{ "javaws.association.dialog.title", "JNLP \ud30c\uc77c/MIME \uc5f0\uad00" },
	{ "javaws.association.dialog.exist.command", "\uc774\ubbf8 \uc788\uc2b5\ub2c8\ub2e4:\n{0}"},
	{ "javaws.association.dialog.exist", "\uc774\ubbf8 \uc788\uc2b5\ub2c8\ub2e4." },
	{ "javaws.association.dialog.askReplace", "\n\ub300\uc2e0 {0}\uc744(\ub97c) \uc0ac\uc6a9\ud558\uc5ec \ucc98\ub9ac\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?"},
	{ "javaws.association.dialog.ext", "\ud30c\uc77c \ud655\uc7a5\uc790: {0}" },
        { "javaws.association.dialog.mime", "MIME \uc720\ud615: {0}" },
        { "javaws.association.dialog.ask", "{0}\uc744(\ub97c) \uc0ac\uc6a9\ud558\uc5ec \ucc98\ub9ac\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c:" },
        { "javaws.association.dialog.existAsk", "\uacbd\uace0! \uc5f0\uad00 \ub300\uc0c1:"},

        // Advanced panel strings:
        { "deployment.console.startup.mode", "Java \ucf58\uc194" },
        { "deployment.console.startup.mode.SHOW", "\ucf58\uc194 \ud45c\uc2dc" },
        { "deployment.console.startup.mode.SHOW.tooltip", "<html>" +
                                                          "<font size=-1>" +
                                                          "Java \ucf58\uc194\uc744 \ucd5c\ub300\ud654\ud569\ub2c8\ub2e4." +
                                                          "</font>" +
                                                          "</html>" },
        { "deployment.console.startup.mode.HIDE", "\ucf58\uc194 \uc228\uae30\uae30" },
        { "deployment.console.startup.mode.HIDE.tooltip", "<html>" +
                                                          "<font size=-1>" +        
                                                          "Java \ucf58\uc194\uc744 \ucd5c\uc18c\ud654\ud569\ub2c8\ub2e4." +
                                                          "</font>" +
                                                          "</html>" },
        { "deployment.console.startup.mode.DISABLE", "\ucf58\uc194\uc744 \uc2dc\uc791\ud558\uc9c0 \ub9c8\uc2ed\uc2dc\uc624." },
        { "deployment.console.startup.mode.DISABLE.tooltip", "<html>" +
                                                             "<font size=-1>" +        
                                                             "Java \ucf58\uc194\uc774 \uc2dc\uc791\ub418\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." +
                                                             "</font>" +
                                                             "</html>" },
        { "deployment.trace", "\ucd94\uc801 \uc0ac\uc6a9 \uac00\ub2a5" },
        { "deployment.trace.tooltip", "<html>" +
                                      "<font size=-1>" +        
                                      "\ub514\ubc84\uae45\uc744 \uc704\ud574 \ucd94\uc801 \ud30c\uc77c\uc744" +
                                      "<br>\ub9cc\ub4ed\ub2c8\ub2e4." +
                                      "</font>" +
                                      "</html>" },
        { "deployment.log", "\ub85c\uae45 \uc0ac\uc6a9 \uac00\ub2a5" },
        { "deployment.log.tooltip", "<html>" +
                                    "<font size=-1>" +
                                    "\uc624\ub958\ub97c \ucea1\ucc98\ud558\uae30 \uc704\ud574 \ub85c\uadf8 \ud30c\uc77c\uc744" +
                                    "<br>\ub9cc\ub4ed\ub2c8\ub2e4." +
                                    "</font>" +
                                    "</html>" },
        { "deployment.control.panel.log", "\uc81c\uc5b4\ud310\uc5d0 \ub85c\uae45" },
        { "deployment.javapi.lifecycle.exception", "\uc560\ud50c\ub9bf \uc21c\ud658 \uc608\uc678 \ud45c\uc2dc" },
        { "deployment.javapi.lifecycle.exception.tooltip", "<html>" +
                                          "\uc560\ud50c\ub9bf\uc744 \ub85c\ub4dc\ud558\ub294 \uc911\uc5d0 \uc624\ub958\uac00"+
                                          "<br>\ubc1c\uc0dd\ud55c \uacbd\uc6b0 \uc608\uc678 \uc0ac\ud56d\uc774 \uc788\ub294"+
                                          "<br>\ub300\ud654 \uc0c1\uc790\ub97c \ud45c\uc2dc\ud569\ub2c8\ub2e4.<html>" },
        { "deployment.browser.vm.iexplorer", "Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
                                                     "<font size=-1>" +
                                                     "Internet Explorer \ube0c\ub77c\uc6b0\uc800\uc5d0\uc11c" +
                                                     "<br>APPLET \ud0dc\uadf8\uac00 \uc788\ub294 Sun Java\ub97c" +
                                                     "<br>\uc0ac\uc6a9\ud569\ub2c8\ub2e4.</font></html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla \ubc0f Netscape" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
                                                   "<font size=-1>" +        
                                                   "Mozilla \ub610\ub294 Netscape \ube0c\ub77c\uc6b0\uc800\uc5d0\uc11c APPLET \ud0dc\uadf8\uac00" +
                                                   "<br>\uc788\ub294 Sun Java\ub97c \uc0ac\uc6a9\ud569\ub2c8\ub2e4." +
                                                   "</font>" +
                                                   "</html>" },
        { "deployment.console.debugging", "\ub514\ubc84\uae45" },
	{ "deployment.browsers.applet.tag", "<APPLET> \ud0dc\uadf8 \uc9c0\uc6d0" },
        { "deployment.javaws.shortcut", "\ub2e8\ucd95\ud0a4 \uc791\uc131" },
        { "deployment.javaws.shortcut.ALWAYS", "\ud56d\uc0c1 \ud5c8\uc6a9" },
        { "deployment.javaws.shortcut.ALWAYS.tooltip", "<html>" +
                                                   "<font size=-1>" +        
                                                   "\ud56d\uc0c1 \ubc14\ub85c \uac00\uae30 \uc791\uc131" +
                                                   "</font>" +
                                                   "</html>" },
        { "deployment.javaws.shortcut.NEVER" , "\ud5c8\uc6a9 \uc548 \ud568" },
        { "deployment.javaws.shortcut.NEVER.tooltip", "<html>" +
                                                      "<font size=-1>" +
                                                      "\ubc14\ub85c \uac00\uae30\ub97c \ub9cc\ub4e4\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." +
                                                      "</font>" +
                                                      "</html>" },
        { "deployment.javaws.shortcut.ASK_USER", "\uc0ac\uc6a9\uc790 \ud504\ub86c\ud504\ud2b8" },
        { "deployment.javaws.shortcut.ASK_USER.tooltip", "<html>" +
                                                      "<font size=-1>" +
                                                      "\ubc14\ub85c \uac00\uae30\ub97c \ub9cc\ub4e4\uc9c0 \uc0ac\uc6a9\uc790\uc5d0\uac8c" +
                                                      "<br>\ubb3b\uc2b5\ub2c8\ub2e4." +
                                                      "</font>" +
                                                      "</html>" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED", "\ud78c\ud2b8\uac00 \uc788\uc73c\uba74 \ud56d\uc0c1 \ud5c8\uc6a9" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED.tooltip", "<html>" +
                                                      "<font size=-1>" +
                                                      "JNLP \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \uc694\uccad\ud560 \uacbd\uc6b0" +
                                                      "<br>\ud56d\uc0c1 \ubc14\ub85c \uac00\uae30\ub97c \ub9cc\ub4ed\ub2c8\ub2e4." +
                                                      "</font>" +
                                                      "</html>" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED", "\ud78c\ud2b8\uac00 \uc788\uc73c\uba74 \uc0ac\uc6a9\uc790 \ud504\ub86c\ud504\ud2b8" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED.tooltip", "<html>" +
                                                      "<font size=-1>" +
                                                      "JNLP \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc774 \uc694\uccad\ud560 \uacbd\uc6b0" +
                                                      "<br>\ubc14\ub85c \uac00\uae30\ub97c \ub9cc\ub4e4\uc9c0 \uc0ac\uc6a9\uc790\uc5d0\uac8c" +
                                                      "<br>\ubb3b\uc2b5\ub2c8\ub2e4." +
                                                      "</font>" +
                                                      "</html>" },
	{ "deployment.javaws.associations.NEVER", "\ud5c8\uc6a9 \uc548 \ud568" },
        { "deployment.javaws.associations.NEVER.tooltip", "<html>" +
                                                  "<font size=-1>" +
                                                  "\ud30c\uc77c \ud655\uc7a5\uc790/MIME \uc5f0\uad00\uc744 \ub9cc\ub4e4\uc9c0" +
                                                  "<br>\uc54a\uc2b5\ub2c8\ub2e4." +
                                                  "</font>" +
                                                  "</html>" },
        { "deployment.javaws.associations.ASK_USER", "\uc0ac\uc6a9\uc790 \ud504\ub86c\ud504\ud2b8" },
        { "deployment.javaws.associations.ASK_USER.tooltip", "<html>" +
                                                  "<font size=-1>" +
                                                  "\ud30c\uc77c \ud655\uc7a5\uc790/MIME \uc5f0\uad00\uc744 \ub9cc\ub4e4\uae30 \uc804\uc5d0" +
                                                  "<br>\uc0ac\uc6a9\uc790\uc5d0\uac8c \ubb3b\uc2b5\ub2c8\ub2e4." +
                                                  "</font>" +
                                                  "</html>" },
        { "deployment.javaws.associations.REPLACE_ASK", "\ubc14\uafc0 \ub54c \uc0ac\uc6a9\uc790 \ud504\ub86c\ud504\ud2b8" },
        { "deployment.javaws.associations.REPLACE_ASK.tooltip", "<html>" +
                                                  "<font size=-1>" +
                                                  "\uae30\uc874 \ud30c\uc77c \ud655\uc7a5\uc790/MIME \uc5f0\uad00\uc744" +
                                                  "<br>\ubc14\uafc0 \uacbd\uc6b0\uc5d0\ub9cc \uc0ac\uc6a9\uc790\uc5d0\uac8c" +
                                                  "<br>\ubb3b\uc2b5\ub2c8\ub2e4." +
                                                  "</font>" +
                                                  "</html>" },
        { "deployment.javaws.associations.NEW_ONLY", "\uc0c8 \uc5f0\uad00\uc778 \uacbd\uc6b0\uc5d0\ub9cc \ud5c8\uc6a9" },
        { "deployment.javaws.associations.NEW_ONLY.tooltip", "<html>" +
                                                  "<font size=-1>" +
                                                  "\ud56d\uc0c1 \ud30c\uc77c \ud655\uc7a5\uc790/MIME \uc5f0\uad00\uc744" +
                                                  "<br>\uc0c8\ub85c \ub9cc\ub4ed\ub2c8\ub2e4." +
                                                  "</font>" +
                                                  "</html>" },
        { "deployment.javaws.associations", "JNLP \ud30c\uc77c/MIME \uc5f0\uad00" },
        { "deployment.security.settings", "\ubcf4\uc548" },
        { "deployment.security.askgrantdialog.show", "\uc0ac\uc6a9\uc790\uc5d0\uac8c \uc11c\uba85\ub41c \ub0b4\uc6a9\uc5d0 \ub300\ud55c \uad8c\ud55c \ubd80\uc5ec" },
        { "deployment.security.askgrantdialog.notinca", "\uc2e0\ub8b0\ud560 \uc218 \uc5c6\ub294 \ub0b4\uc6a9\uc5d0 \ub300\ud574 \uc0ac\uc6a9\uc790\uc5d0\uac8c \uad8c\ud55c \ubd80\uc5ec" },

	{ "deployment.security.browser.keystore.use", "\ube0c\ub77c\uc6b0\uc800 \ud0a4 \uc800\uc7a5\uc18c\uc5d0\uc11c \uc778\uc99d\uc11c \ubc0f \ud0a4 \uc0ac\uc6a9" },
	{ "deployment.security.notinca.warning", "\uc778\uc99d\uc11c\uac00 \ub9cc\ub8cc\ub418\uc5c8\uac70\ub098 \ud655\uc778\ud560 \uc218 \uc5c6\ub294 \uacbd\uc6b0 \uacbd\uace0" },
        { "deployment.security.expired.warning", "\uc778\uc99d\uc11c\uac00 \ub9cc\ub8cc\ub418\uc5c8\uac70\ub098 \uc720\ud6a8\ud558\uc9c0 \uc54a\uc740 \uacbd\uc6b0 \uacbd\uace0" },
        { "deployment.security.jsse.hostmismatch.warning", "\uc0ac\uc774\ud2b8 \uc778\uc99d\uc11c\uac00 \ud638\uc2a4\ud2b8 \uc774\ub984\uacfc \uc77c\uce58\ud558\uc9c0 \uc54a\ub294 \uacbd\uc6b0 \uacbd\uace0" },
        { "deployment.security.sandbox.awtwarningwindow", "\uc0cc\ub4dc \ubc15\uc2a4 \uacbd\uace0 \ubc30\ub108 \ud45c\uc2dc" },
        { "deployment.security.sandbox.jnlp.enhanced", "\uc0ac\uc6a9\uc790\uc5d0\uac8c JNLP \ubcf4\uc548 \uc694\uccad \ub3d9\uc758 \ud5c8\uc6a9" },
        { "deploy.advanced.browse.title", "\uae30\ubcf8 \ube0c\ub77c\uc6b0\uc800\ub97c \uc2dc\uc791\ud560 \ud30c\uc77c \uc120\ud0dd" },
        { "deploy.advanced.browse.select", "\uc120\ud0dd" },
        { "deploy.advanced.browse.select_tooltip", "\ube0c\ub77c\uc6b0\uc800 \uc2dc\uc791\uc5d0 \uc120\ud0dd\ud55c \ud30c\uc77c \uc0ac\uc6a9" },
        { "deploy.advanced.browse.select_mnemonic", "S" },
        { "deploy.advanced.browse.browse_btn", "\ucc3e\uc544\ubcf4\uae30..." },
        { "deploy.advanced.browse.browse_btn.mnemonic", "VK_B" },
        { "deployment.browser.default", "\uae30\ubcf8 \ube0c\ub77c\uc6b0\uc800\ub97c \uc2dc\uc791\ud558\ub294 \uba85\ub839" },
        { "deployment.misc.label", "\uae30\ud0c0" },
        { "deployment.system.tray.icon", "\uc2dc\uc2a4\ud15c \ud2b8\ub808\uc774\uc5d0 Java \uc544\uc774\ucf58 \ub193\uae30" },
        { "deployment.system.tray.icon.tooltip", "<html>" +
                                                 "<font size=-1>" +
                                                 "\ube0c\ub77c\uc6b0\uc800\uc5d0\uc11c Java\uac00 \uc2e4\ud589 \uc911\uc778 \uacbd\uc6b0" +
                                                 "<br>\uc2dc\uc2a4\ud15c \ud2b8\ub808\uc774\uc5d0 Java \ucef5 \uc544\uc774\ucf58\uc744" +
                                                 "<br>\ud45c\uc2dc\ud558\ub824\uba74 \uc774 \uc635\uc158\uc744 \uc120\ud0dd\ud569\ub2c8\ub2e4." +
                                                 "</font>" +
                                                 "</html>" },

        //PluginJresDialog strings:
        { "jpi.jres.dialog.title", "Java \ub7f0\ud0c0\uc784 \uc124\uc815" },
        { "jpi.jres.dialog.border", " Java \ub7f0\ud0c0\uc784 \ubc84\uc804 " },
        { "jpi.jres.dialog.column1", "\uc81c\ud488 \uc774\ub984" },
        { "jpi.jres.dialog.column2", "\ubc84\uc804" },
        { "jpi.jres.dialog.column3", "\uc704\uce58" },
        { "jpi.jres.dialog.column4", "Java \ub7f0\ud0c0\uc784 \ub9e4\uac1c\ubcc0\uc218" },
        { "jpi.jdk.string", "JDK" },
        { "jpi.jre.string", "JRE" },
        { "jpi.jres.dialog.product.tooltip", "\uc81c\ud488 \uc774\ub984\uc73c\ub85c JRE \ub610\ub294 JDK \uc120\ud0dd" },

        // AboutDialog strings:
        { "about.dialog.title", "Java \uc815\ubcf4" },

        // JavaPanel strings:
        { "java.panel.plugin.border", " Java \uc560\ud50c\ub9bf \ub7f0\ud0c0\uc784 \uc124\uc815 " }, 
        { "java.panel.plugin.text", "\ub7f0\ud0c0\uc784 \uc124\uc815\uc740 \ube0c\ub77c\uc6b0\uc800\uc5d0\uc11c \uc560\ud50c\ub9bf\uc744 \uc2e4\ud589\ud560 \ub54c \uc0ac\uc6a9\ub429\ub2c8\ub2e4." },
        { "java.panel.jpi_view_btn", "\ubcf4\uae30..." },
        { "java.panel.javaws_view_btn", "\ubcf4\uae30..." },
        { "java.panel.jpi_view_btn.mnemonic", "VK_V" },
        { "java.panel.javaws_view_btn.mnemonic", "VK_I" },
        { "java.panel.javaws.border", " Java \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \ub7f0\ud0c0\uc784 \uc124\uc815 "},
        { "java.panel.javaws.text", "\ub7f0\ud0c0\uc784 \uc124\uc815\uc740 JNLP(Java Network Launching Protocol)\uc744 \uc0ac\uc6a9\ud558\uc5ec Java \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \ub610\ub294 \uc560\ud50c\ub9bf\uc744 \uc2dc\uc791\ud560 \ub54c \uc0ac\uc6a9\ub429\ub2c8\ub2e4." },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "<html><b>Java \ub7f0\ud0c0\uc784 \uc0c8 \ubc84\uc804 \uc788\uc74c</b></html>Internet Explorer\uc5d0 \uc774\ubbf8 \uc0c8 \ubc84\uc804\uc758 Java \ub7f0\ud0c0\uc784\uc774 \uc788\uc2b5\ub2c8\ub2e4. \ubc14\uafb8\uc2dc\uaca0\uc2b5\ub2c8\uae4c?\n" },
        { "browser.settings.success.caption", "\uc131\uacf5 - \ube0c\ub77c\uc6b0\uc800" },
        { "browser.settings.success.text", "<html><b>\ube0c\ub77c\uc6b0\uc800 \uc124\uc815 \ubcc0\uacbd</b></html>\ube0c\ub77c\uc6b0\uc800\ub97c \ub2e4\uc2dc \uc2dc\uc791\ud558\uba74 \ubcc0\uacbd \ub0b4\uc6a9\uc774 \uc801\uc6a9\ub429\ub2c8\ub2e4.\n" },
        { "browser.settings.fail.caption", "\uacbd\uace0 - \ube0c\ub77c\uc6b0\uc800" },
        { "browser.settings.fail.moz.text", "<html><b>\ube0c\ub77c\uc6b0\uc800 \uc124\uc815\uc744 \ubcc0\uacbd\ud560 \uc218 \uc5c6\uc74c</b></html>"
                                        + "Mozilla \ub610\ub294 Netscape\uac00 \uc2dc\uc2a4\ud15c\uc5d0 \uc81c\ub300\ub85c \uc124\uce58\ub418\uc5b4 \uc788\uace0 "
                                        + "\uc2dc\uc2a4\ud15c \uc124\uc815\uc744 "
                                        + "\ubcc0\uacbd\ud558\uae30\uc5d0 \ucda9\ubd84\ud55c \uad8c\ud55c\uc774 \uc788\ub294\uc9c0 \ud655\uc778\ud558\uc2ed\uc2dc\uc624.\n" },
        { "browser.settings.fail.ie.text", "<html><b>\ube0c\ub77c\uc6b0\uc800 \uc124\uc815\uc744 \ubcc0\uacbd\ud560 \uc218 \uc5c6\uc74c</b></html>\uc2dc\uc2a4\ud15c \uc124\uc815\uc744 "
					+ "\ubcc0\uacbd\ud558\uae30\uc5d0 \ucda9\ubd84\ud55c \uad8c\ud55c\uc774 \uc788\ub294\uc9c0 \ud655\uc778\ud558\uc2ed\uc2dc\uc624.\n" },


        // Tool tip strings.
        { "cpl.ok_btn.tooltip", "<html>" +
                                "<font size=-1>" +
                                "Java \uc81c\uc5b4\ud310\uc744 \ub2eb\uace0 \ubcc0\uacbd " +
                                "<br>\ub0b4\uc6a9\uc744 \ubaa8\ub450 \uc800\uc7a5\ud569\ub2c8\ub2e4." +
                                "</font>" +
                                "</html>" },
        { "cpl.apply_btn.tooltip",  "<html>" +
                                    "<font size=-1>" +
                                    "Java \uc81c\uc5b4\ud310\uc744 \ub2eb\uc9c0 \uc54a\uace0 " +
                                    "<br>\ubcc0\uacbd \ub0b4\uc6a9\uc744 \ubaa8\ub450 \uc800\uc7a5\ud569\ub2c8\ub2e4." +
                                    "</font>" +
                                    "</html>" },
        { "cpl.cancel_btn.tooltip", "<html>" +
                                    "<font size=-1>" +
                                    "\ubcc0\uacbd \ub0b4\uc6a9\uc744 \uc800\uc7a5\ud558\uc9c0 \uc54a\uace0" +
                                    "<br>Java \uc81c\uc5b4\ud310\uc744 \ub2eb\uc2b5\ub2c8\ub2e4." +
                                    "</font>" +
                                    "</html>" },

        {"network.settings.btn.tooltip", "<html>"+
                                         "<font size=-1>" +
                                         "\uc778\ud130\ub137 \uc5f0\uacb0 \uc124\uc815\uc744 \uc218\uc815\ud569\ub2c8\ub2e4." +
                                         "</font>" +
                                         "</html>"},

        {"temp.files.settings.btn.tooltip", "<html>"+
                                            "<font size=-1>" +
                                            "\uc784\uc2dc \ud30c\uc77c\uc5d0 \ub300\ud55c \uc124\uc815\uc744 \uc218\uc815\ud569\ub2c8\ub2e4." +
                                            "</font>" +
                                            "</html>"},

        {"temp.files.delete.btn.tooltip", "<html>" +  // body bgcolor=\"#FFFFCC\">"+
                                          "<font size=-1>" +
                                          "\uc784\uc2dc Java \ud30c\uc77c\uc744 \uc0ad\uc81c\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>"},

        {"delete.files.dlg.applets.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java \uc560\ud50c\ub9bf\uc5d0\uc11c \ub9cc\ub4e0 \uc784\uc2dc \ud30c\uc77c\uc744 \ubaa8\ub450" +
                                          "<br>\uc0ad\uc81c\ud558\ub824\uba74 \uc774 \uc635\uc158\uc744 \uc120\ud0dd\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        {"delete.files.dlg.applications.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java Web Start \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c \ub9cc\ub4e0" +
                                          "<br>\uc784\uc2dc \ud30c\uc77c\uc744 \ubaa8\ub450 \uc0ad\uc81c\ud558\ub824\uba74 \uc774 \uc635\uc158\uc744" +
                                          "<br>\uc120\ud0dd\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        {"delete.files.dlg.other.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java\uc5d0\uc11c \ub9cc\ub4e0 \ub2e4\ub978 \ubaa8\ub4e0 \uc784\uc2dc \ud30c\uc77c\uc744" +
                                          "<br>\uc0ad\uc81c\ud558\ub824\uba74 \uc774 \uc635\uc158\uc744 \uc120\ud0dd\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        {"delete.files.dlg.temp_files.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c\ub294 \ucef4\ud4e8\ud130\uc5d0 " +
                                      "<br>\uc784\uc2dc \ud30c\uc77c\uc744 \uc800\uc7a5\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." +
                                      "<br>\uc774 \ud30c\uc77c\uc744 \uc0ad\uc81c\ud574\ub3c4 \uc548\uc804\ud569\ub2c8\ub2e4." +
                                          "<br>" +
                                          "<p>\uc77c\ubd80 Java \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc740 \uc784\uc2dc \ud30c\uc77c\uc744" +
                                         "<br>\uc0ad\uc81c\ud55c \ud6c4 \ucc98\uc74c\uc73c\ub85c \uc2dc\uc791\ud560 \ub54c " +
                                         "<br>\uc2dc\uac04\uc774 \ub354 \uc624\ub798 \uac78\ub9b4 \uc218 \uc788\uc2b5\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.view_jws_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java Web Start \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c " +
                                          "<br>\ub9cc\ub4e0 \uc784\uc2dc \ud30c\uc77c\uc744 \ubd05\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.view_jpi_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java \uc560\ud50c\ub9bf\uc5d0\uc11c \ub9cc\ub4e0 " +
                                          "<br>\uc784\uc2dc \ud30c\uc77c\uc744\ubd05\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "\uc784\uc2dc \ud30c\uc77c\uc774 \uc800\uc7a5\ub418\ub294 " +
                                          "<br>\ub514\ub809\ud1a0\ub9ac\ub97c \uc9c0\uc815\ud569\ub2c8\ub2e4."+
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.unlimited_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "\uc784\uc2dc \ud30c\uc77c \uc800\uc7a5\uc5d0 \uc0ac\uc6a9\ub418\ub294 \ub514\uc2a4\ud06c \uacf5\uac04\uc758" +
                                      "<br>\ud06c\uae30\ub97c \uc81c\ud55c\ud558\uc9c0 \ub9c8\uc2ed\uc2dc\uc624." +
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.max_btn.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "\uc784\uc2dc \ud30c\uc77c \uc800\uc7a5\uc5d0 \uc0ac\uc6a9\ub418\ub294 \ub514\uc2a4\ud06c \uacf5\uac04\uc758 " +
                                      "<br>\ucd5c\ub300 \ud06c\uae30\ub97c \uc9c0\uc815\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        {"cache.settings.dialog.compression.tooltip", "<html>" +
                                          "<font size=-1>" +
                                          "Java \ud504\ub85c\uadf8\ub7a8\uc5d0\uc11c \uc784\uc2dc \ud30c\uc77c \ub514\ub809\ud1a0\ub9ac\uc5d0" +
                                          "<br>\uc800\uc7a5\ud558\ub294 JAR \ud30c\uc77c\uc5d0 \uc0ac\uc6a9\ud560 \uc555\ucd95\ub960\uc744" +
                                          "<br>\uc9c0\uc815\ud569\ub2c8\ub2e4." +
                                          "<br>" +
                                          "<p>\"\uc5c6\uc74c\"\uc744 \uc9c0\uc815\ud558\uba74 Java \ud504\ub85c\uadf8\ub7a8\uc774 \ub354 \ube68\ub9ac" +
                                          "<br>\uc2dc\uc791\ub418\uc9c0\ub9cc \uc800\uc7a5\uc5d0 \ud544\uc694\ud55c \ub514\uc2a4\ud06c" +
                                          "<br>\uacf5\uac04\uc740 \ub298\uc5b4\ub0a9\ub2c8\ub2e4. \ub192\uc740 \uac12\uc744 \uc9c0\uc815\ud558\uba74" +
                                          "<br>\ub514\uc2a4\ud06c \uacf5\uac04 \uc694\uad6c \uc0ac\ud56d\uc774 \uc904\uc5b4\ub4e4\uc9c0\ub9cc" +
                                          "<br>\uc2dc\uc791 \uc2dc\uac04\uc774 \uc57d\uac04 \uae38\uc5b4\uc9d1\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        { "common.ok_btn.tooltip",  "<html>" +
                                    "<font size=-1>" +
                                    "\ubcc0\uacbd \ub0b4\uc6a9\uc744 \uc800\uc7a5\ud558\uace0 \ub300\ud654 \uc0c1\uc790\ub97c \ub2eb\uc2b5\ub2c8\ub2e4." +
                                    "</font>" +
                                    "</html>" },

        { "common.cancel_btn.tooltip",  "<html>" +
                                        "<font size=-1>" +
                                        "\ubcc0\uacbd\uc744 \ucde8\uc18c\ud558\uace0 \ub300\ud654 \uc0c1\uc790\ub97c \ub2eb\uc2b5\ub2c8\ub2e4." +
                                        "</font>" +
                                        "</html>"},

	{ "network.settings.advanced_btn.tooltip",  "<html>" +
                                                    "<font size=-1>" +
                                                    "\uace0\uae09 \ud504\ub85d\uc2dc \uc124\uc815\uc744 \ubcf4\uace0 \uc218\uc815\ud569\ub2c8\ub2e4."+
                                                    "</font>" +
                                                    "</html>"},

        {"security.certs_btn.tooltip", "<html>" +
                                       "<font size=-1>" +
                                       "\uc778\uc99d\uc11c\ub97c \uac00\uc838\uc624\uac70\ub098 \ub0b4\ubcf4\ub0b4\uac70\ub098 \uc81c\uac70\ud569\ub2c8\ub2e4." +
                                       "</font>" +
                                       "</html>" },

        { "cert.import_btn.tooltip", "<html>" +
                                     "<font size=-1>" +
                                     "\uc544\uc9c1 \ubaa9\ub85d\uc5d0 \uc5c6\ub294 " +
                                     "<br>\uc778\uc99d\uc11c\ub97c \uac00\uc838\uc635\ub2c8\ub2e4." +
				     "</font>" +
				     "</html>"},

        { "cert.export_btn.tooltip",    "<html>" +
                                        "<font size=-1>" +
                                        "\uc120\ud0dd\ud55c \uc778\uc99d\uc11c\ub97c \ub0b4\ubcf4\ub0c5\ub2c8\ub2e4." +
                                        "</font>" +
                                        "</html>"},

        { "cert.remove_btn.tooltip",  "<html>" +
                                      "<font size=-1>" +
                                      "\ubaa9\ub85d\uc5d0\uc11c \uc120\ud0dd\ud55c "+
                                  "<br>\uc778\uc99d\uc11c\ub97c \uc81c\uac70\ud569\ub2c8\ub2e4." +
        		      "</font>" +
        		      "</html>"},

        { "cert.details_btn.tooltip", "<html>" +
		      "<font size=-1>" +
		      "\uc120\ud0dd\ud55c \uc778\uc99d\uc11c\uc5d0 \uad00\ud55c " +
                      "<br>\uc138\ubd80 \uc815\ubcf4\ub97c \ubd05\ub2c8\ub2e4." +
		      "</font>" +
		      "</html>"},

        { "java.panel.jpi_view_btn.tooltip",  "<html>" +
                                              "<font size=-1>" +
                                              "\uc560\ud50c\ub9bf\uc758 Java \ub7f0\ud0c0\uc784 \uc124\uc815\uc744 \uc218\uc815\ud569\ub2c8\ub2e4." +
                                              "</font>" +
                                              "</html>" },

        { "java.panel.javaws_view_btn.tooltip",   "<html>" +
                                                  "<font size=-1>" +
                                                  "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc758 Java \ub7f0\ud0c0\uc784 \uc124\uc815\uc744 \uc218\uc815\ud569\ub2c8\ub2e4." +
                                                  "</font>" +
                                                  "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "<font size=-1>" +
                                            "\uc774 \ubc84\uc804\uc758 J2SE Runtime Environment \uc815\ubcf4\ub97c \ubd05\ub2c8\ub2e4." +
                                            "</font>" +
                                            "</html>" },

        { "update.notify_combo.tooltip",  "<html>" +
                                          "<font size=-1>" +
                                          "\uc0c8 Java \uc5c5\ub370\uc774\ud2b8 " +
                                          "<br>\uc54c\ub9bc\uc744 \ubc1b\uace0 \uc2f6\uc740 " +
                                          "<br>\uc2dc\uae30\ub97c \uc120\ud0dd\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        { "update.advanced_btn.tooltip",  "<html>" +
                                          "<font size=-1>" +
                                          "\uc790\ub3d9 \uc5c5\ub370\uc774\ud2b8\ub97c \uc704\ud55c \uc608\uc57d \uc815\ucc45\uc744" +
					  "<br>\uc218\uc815\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        { "update.now_btn.tooltip",   "<html>" +
                                      "<font size=-1>" +
                                      "Java Update\ub97c \uc2dc\uc791\ud558\uc5ec \uc0ac\uc6a9 \uac00\ub2a5\ud55c " +
                                      "<br>\ucd5c\uc2e0 Java \uc5c5\ub370\uc774\ud2b8\ub97c \ud655\uc778\ud569\ub2c8\ub2e4." +
                                      "</font>" +
                                      "</html>" },

        { "vm.options.add_btn.tooltip",   "<html>" +
                                          "<font size=-1>" +
                                          "\ubaa9\ub85d\uc5d0 \uc0c8 JRE\ub97c \ucd94\uac00\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },

        { "vm.options.remove_btn.tooltip", "<html>" +
                                           "<font size=-1>" +
                                           "\uc120\ud0dd\ud55c \ud56d\ubaa9\uc744 \ubaa9\ub85d\uc5d0\uc11c \uc81c\uac70\ud569\ub2c8\ub2e4." +
                                           "</font>" +
                                           "</html>" },

        { "vm.optios.ok_btn.tooltip",    "<html>" +
		         "<font size=-1>" +
		         "\uc81c\ud488 \uc774\ub984, \ubc84\uc804, " +
		     "<br>\uc704\uce58 \uc815\ubcf4\uac00 \ud3ec\ud568\ub41c " +
		     "<br>\ubaa8\ub4e0 \ud56d\ubaa9\uc744 \uc800\uc7a5\ud569\ub2c8\ub2e4." +
		         "</font>" +
		         "</html>" },

        { "jnlp.jre.find_btn.tooltip",  "<html>" +
		        "<font size=-1>" +
		        "\uc124\uce58\ub41c Java Runtime " +
                        "<br>Environment\ub97c \uac80\uc0c9\ud569\ub2c8\ub2e4." +
		        "</font>" +
		        "</html>" },

        { "jnlp.jre.add_btn.tooltip",   "<html>" +
                                        "<font size=-1>" +
                                        "\ubaa9\ub85d\uc5d0 \uc0c8 \ud56d\ubaa9\uc744 \ucd94\uac00\ud569\ub2c8\ub2e4." +
		        "</font>" +
		        "</html>" },

        { "jnlp.jre.remove_btn.tooltip",  "<html>" +
                                          "<font size=-1>" +
                                          "\uc0ac\uc6a9\uc790 \ubaa9\ub85d\uc5d0\uc11c \uc120\ud0dd\ud55c " +
                                      "<br>\ud56d\ubaa9\uc744 \uc81c\uac70\ud569\ub2c8\ub2e4." +
                                          "</font>" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "JRE \ub2e4\uc6b4\ub85c\ub4dc \ud5c8\uc6a9" },
        { "download.jre.prompt.text1", "\uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8 \"{0}\"\uc5d0\ub294 \ud604\uc7ac \uc2dc\uc2a4\ud15c\uc5d0 "
                                     + "\uc124\uce58\ub418\uc5b4 \uc788\uc9c0 \uc54a\uc740 "
                                     + "JRE (version {1}) \ubc84\uc804\uc774 \ud544\uc694\ud569\ub2c8\ub2e4." },
        { "download.jre.prompt.text2", "Java Web Start\uc5d0\uc11c \uc790\ub3d9\uc73c\ub85c \uc774 JRE\ub97c \ub2e4\uc6b4\ub85c\ub4dc\ud558\uc5ec "
                                     + "\uc124\uce58\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
        { "download.jre.prompt.okButton", "\ub2e4\uc6b4\ub85c\ub4dc" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "download.jre.prompt.cancelButton", "\ucde8\uc18c" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "autoupdatecheck.buttonYes", "\uc608" },
	{ "autoupdatecheck.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "autoupdatecheck.buttonNo", "\uc544\ub2c8\uc624" },
	{ "autoupdatecheck.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "autoupdatecheck.buttonAskLater", "\ub098\uc911\uc5d0 \ub2e4\uc2dc \ubb3b\uae30" },
	{ "autoupdatecheck.buttonAskLater.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "autoupdatecheck.caption", "\uc5c5\ub370\uc774\ud2b8 \uc790\ub3d9 \uac80\uc0c9" },
	{ "autoupdatecheck.message", "Java Update\ub294 Java \uc18c\ud504\ud2b8\uc6e8\uc5b4\ub97c \uc0ac\uc6a9 \uac00\ub2a5\ud55c \uc0c8 \ubc84\uc804\uc73c\ub85c \uc790\ub3d9\uc73c\ub85c \uc5c5\ub370\uc774\ud2b8\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4. \uc774 \uc11c\ube44\uc2a4\ub97c \uc0ac\uc6a9\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
    };
}

