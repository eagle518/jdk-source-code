/*
 * @(#)Deployment.java	1.101 04/07/15
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

public final class Deployment extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
        { "product.javapi.name", "Java Plug-in {0}" },
        { "product.javaws.name", "Java Web Start {0}" },
	{ "console.version", "Version" },
	{ "console.default_vm_version", "Default Virtual Machine Version " },
	{ "console.using_jre_version", "Using JRE version" },
	{ "console.user_home", "User home directory" },
        { "console.caption", "Java Console" },
        { "console.clear", "Clear" },
        { "console.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "console.close", "Close" },
        { "console.close.acceleratorKey", new Integer(KeyEvent.VK_E) },
        { "console.copy", "Copy" },
        { "console.copy.acceleratorKey", new Integer(KeyEvent.VK_Y) },
	{ "console.menu.text.top", "----------------------------------------------------\n" },
	{ "console.menu.text.c", "c:   clear console window\n" },
	{ "console.menu.text.f", "f:   finalize objects on finalization queue\n" },
	{ "console.menu.text.g", "g:   garbage collect\n" },
	{ "console.menu.text.h", "h:   display this help message\n" },
	{ "console.menu.text.j", "j:   dump jcov data\n"},
	{ "console.menu.text.l", "l:   dump classloader list\n" },
	{ "console.menu.text.m", "m:   print memory usage\n" },
	{ "console.menu.text.o", "o:   trigger logging\n" },
	{ "console.menu.text.p", "p:   reload proxy configuration\n" },
	{ "console.menu.text.q", "q:   hide console\n" },
	{ "console.menu.text.r", "r:   reload policy configuration\n" },
	{ "console.menu.text.s", "s:   dump system and deployment properties\n" },
	{ "console.menu.text.t", "t:   dump thread list\n" },
	{ "console.menu.text.v", "v:   dump thread stack\n" },
	{ "console.menu.text.x", "x:   clear classloader cache\n" },
	{ "console.menu.text.0", "0-5: set trace level to <n>\n" },
	{ "console.menu.text.tail", "----------------------------------------------------\n" },
	{ "console.done", "Done." },
	{ "console.trace.level.0", "Trace level set to 0: none ... completed." },
	{ "console.trace.level.1", "Trace level set to 1: basic ... completed." },
	{ "console.trace.level.2", "Trace level set to 2: basic, net ... completed." },
	{ "console.trace.level.3", "Trace level set to 3: basic, net, security ... completed." },
	{ "console.trace.level.4", "Trace level set to 4: basic, net, security, ext ... completed." },
	{ "console.trace.level.5", "Trace level set to 5: all ... completed." },
	{ "console.log", "Logging set to : " },
	{ "console.completed", " ... completed." },
	{ "console.dump.thread", "Dump thread list ...\n" },
	{ "console.dump.stack", "Dump thread stack ...\n" },
	{ "console.dump.system.properties", "Dump system properties ...\n" },
	{ "console.dump.deployment.properties", "Dump deployment properties ...\n" },
	{ "console.clear.classloader", "Clear classloader cache ... completed." },
	{ "console.reload.policy", "Reload policy configuration" },
	{ "console.reload.proxy", "Reload proxy configuration ..." },
	{ "console.gc", "Garbage collect" },
	{ "console.finalize", "Finalize objects on finalization queue" },
	{ "console.memory", "Memory: {0}K  Free: {1}K  ({2}%)" },
	{ "console.jcov.error", "Jcov runtime error: check that if you specified the right jcov option\n"},
	{ "console.jcov.info", "Jcov data dumped successfully\n"},

	{ "https.dialog.caption", "Warning - HTTPS" },
	{ "https.dialog.text", "<html><b>Hostname Mismatch</b></html>The hostname in the server security certificate does not match the name of the server."
			     + "\n\nHostname of the URL: {0}"
			     + "\nHostname from the certificate: {1}"
			     + "\n\nDo you want to proceed?" },
	{ "https.dialog.unknown.host", "Unknown host" },

	{ "security.dialog.caption", "Warning - Security" },
	{ "security.dialog.text0", "Do you want to trust the signed {0} distributed by \"{1}\"?"
				 + "\n\nPublisher authenticity verified by: \"{2}\"" },
	{ "security.dialog.text0a", "Do you want to trust the signed {0} distributed by \"{1}\"?"
 				 + "\n\nPublisher authenticity can not be verified." },
	{ "security.dialog.timestamp.text1", "The {0} was signed on {1}." },
	{ "security.dialog_https.text0", "Do you want to accept the certificate from web site \"{0}\" for the purpose of exchanging encrypted information?"
				 + "\n\nPublisher authenticity verified by: \"{1}\"" },
	{ "security.dialog_https.text0a", "Do you want to accept the certificate from web site \"{0}\" for the purpose of exchanging encrypted information?"
 				 + "\n\nPublisher authenticity can not be verified." },
	{ "security.dialog.text1", "\nCaution: \"{0}\" asserts that this content is safe. You should only accept this content if you trust \"{1}\" to make that assertion." },
	{ "security.dialog.unknown.issuer", "Unknown issuer" },
	{ "security.dialog.unknown.subject", "Unknown subject" },
	{ "security.dialog.certShowName", "{0} ({1})" },
	{ "security.dialog.rootCANotValid", "The security certificate was issued by a company that is not trusted." },
	{ "security.dialog.rootCAValid", "The security certificate was issued by a company that is trusted." },
	{ "security.dialog.timeNotValid", "The security certificate has expired or is not yet valid." },
	{ "security.dialog.timeValid", "The security certificate has not expired and is still valid." },
	{ "security.dialog.timeValidTS", "The security certificate was valid when the {0} was signed." },
	{ "security.dialog.buttonAlways", "Always" },
        { "security.dialog.buttonAlways.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "security.dialog.buttonYes", "Yes" },
	{ "security.dialog.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "security.dialog.buttonNo", "No" },
	{ "security.dialog.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "security.dialog.buttonViewCert", "More Details" },
        { "security.dialog.buttonViewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

        { "security.badcert.caption", "Warning - Security" },
        { "security.badcert.https.text", "Cannot validate SSL certificate.\nThis {0} will not be run." },
        { "security.badcert.config.text", "Your security configuration will not allow you to validate this certificate.  This {0} will not be run." },
        { "security.badcert.text", "Failed to validate certificate.  This {0} will not be run." },
        { "security.badcert.viewException", "Show Exception" },
        { "security.badcert.viewException.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "security.badcert.viewCert", "More Details" },
        { "security.badcert.viewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

	{ "cert.dialog.caption", "Details - Certificate" },
	{ "cert.dialog.certpath", "Certificate Path" },
	{ "cert.dialog.field.Version", "Version" },
	{ "cert.dialog.field.SerialNumber", "Serial Number" },
	{ "cert.dialog.field.SignatureAlg", "Signature Algorithm" },
	{ "cert.dialog.field.Issuer", "Issuer" },
	{ "cert.dialog.field.EffectiveDate", "Effective Date" },
	{ "cert.dialog.field.ExpirationDate", "Expiration Date" },
	{ "cert.dialog.field.Validity", "Validity" },
	{ "cert.dialog.field.Subject", "Subject" },
	{ "cert.dialog.field.Signature", "Signature" },
	{ "cert.dialog.field", "Field" },
	{ "cert.dialog.value", "Value" },
        { "cert.dialog.close", "Close" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_C) },

	{ "clientauth.password.dialog.buttonOK", "OK" },
	{ "clientauth.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.password.dialog.buttonCancel", "Cancel" },
	{ "clientauth.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.password.dialog.caption", "Password Needed - Client Authentication Keystore" },
	{ "clientauth.password.dialog.text", "Enter a password to access client authentication keystore:\n" },
	{ "clientauth.password.dialog.error.caption", "Error - Client Authentication Keystore" },
	{ "clientauth.password.dialog.error.text", "<html><b>Keystore Access Error</b></html>Keystore was tampered with, or password was incorrect." },
		
	{ "clientauth.certlist.dialog.buttonOK", "OK" },
	{ "clientauth.certlist.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.certlist.dialog.buttonCancel", "Cancel" },
	{ "clientauth.certlist.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.certlist.dialog.buttonDetails", "Details" },
	{ "clientauth.certlist.dialog.buttonDetails.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "clientauth.certlist.dialog.caption", "Client Authentication" },
	{ "clientauth.certlist.dialog.text", "The web site you want to connect requests identification.\nSelect the certificate to use when connecting.\n" },

	{ "dialogfactory.confirmDialogTitle", "Confirmation Needed - Java" },
	{ "dialogfactory.inputDialogTitle", "Information Needed - Java" },
	{ "dialogfactory.messageDialogTitle", "Message - Java" },
	{ "dialogfactory.exceptionDialogTitle", "Error - Java" },
	{ "dialogfactory.optionDialogTitle", "Option - Java" },
	{ "dialogfactory.aboutDialogTitle", "About - Java" },
	{ "dialogfactory.confirm.yes", "Yes" },
        { "dialogfactory.confirm.yes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
        { "dialogfactory.confirm.no", "No" },
        { "dialogfactory.confirm.no.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "dialogfactory.moreInfo", "More Details" },
        { "dialogfactory.moreInfo.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "dialogfactory.lessInfo", "Less Details" },
        { "dialogfactory.lessInfo.acceleratorKey", new Integer(KeyEvent.VK_L)},
	{ "dialogfactory.java.home.link", "http://www.java.com" },
	{ "dialogfactory.general_error", "<html><b>General Exception</b></html>" },
	{ "dialogfactory.net_error", "<html><b>Networking Exception</b></html>" },
	{ "dialogfactory.security_error", "<html><b>Security Exception</b></html>" },
	{ "dialogfactory.ext_error", "<html><b>Optional Package Exception</b></html>" },
	{ "dialogfactory.user.selected", "User selected: {0}" },
	{ "dialogfactory.user.typed", "User typed: {0}" },

	{ "deploycertstore.cert.loading", "Loading Deployment certificates from {0}" },
	{ "deploycertstore.cert.loaded", "Loaded Deployment certificates from {0}" },
	{ "deploycertstore.cert.saving", "Saving Deployment certificates in {0}" },
	{ "deploycertstore.cert.saved", "Saved Deployment certificates in {0}" },
	{ "deploycertstore.cert.adding", "Adding certificate in Deployment permanent certificate store", },
	{ "deploycertstore.cert.added", "Added certificate in Deployment permanent certificate store as alias {0}" },
	{ "deploycertstore.cert.removing", "Removing certificate in Deployment permanent certificate store" },
	{ "deploycertstore.cert.removed", "Removed certificate in Deployment permanent certificate store as alias {0}" },
	{ "deploycertstore.cert.instore", "Checking if certificate is in Deployment permanent certificate store" },
	{ "deploycertstore.cert.canverify", "Check if certificate can be verified using certificates in Deployment permanent certificate store" },
	{ "deploycertstore.cert.iterator", "Obtain certificate iterator in Deployment permanent certificate store" },
	{ "deploycertstore.cert.getkeystore", "Obtain key store object of Deployment permanent certificate store" },

	{ "httpscertstore.cert.loading", "Loading Deployment SSL certificates from {0}" },
	{ "httpscertstore.cert.loaded", "Loaded Deployment SSL certificates from {0}" },
	{ "httpscertstore.cert.saving", "Saving Deployment SSL certificates in {0}" },
	{ "httpscertstore.cert.saved", "Saved Deployment SSL certificates in {0}" },
	{ "httpscertstore.cert.adding", "Adding SSL certificate in Deployment permanent certificate store", },
	{ "httpscertstore.cert.added", "Added SSL certificate in Deployment permanent certificate store as alias {0}" },
	{ "httpscertstore.cert.removing", "Removing SSL certificate in Deployment permanent certificate store" },
	{ "httpscertstore.cert.removed", "Removed SSL certificate in Deployment permanent certificate store as alias {0}" },
	{ "httpscertstore.cert.instore", "Checking if SSL certificate is in Deployment permanent certificate store" },
	{ "httpscertstore.cert.canverify", "Check if SSL certificate can be verified using certificates in Deployment permanent certificate store" },
	{ "httpscertstore.cert.iterator", "Obtain SSL certificate iterator in Deployment permanent certificate store" },
	{ "httpscertstore.cert.getkeystore", "Obtain key store object of Deployment permanent certificate store" },

	{ "rootcertstore.cert.loading", "Loading Root CA certificates from {0}" },
	{ "rootcertstore.cert.loaded", "Loaded Root CA certificates from {0}" },
	{ "rootcertstore.cert.noload", "Root CA certificates file not found: {0}" },
	{ "rootcertstore.cert.saving", "Saving Root CA certificates in {0}" },
	{ "rootcertstore.cert.saved", "Saved Root CA certificates in {0}" },
	{ "rootcertstore.cert.adding", "Adding certificate in Root CA certificate store", },
	{ "rootcertstore.cert.added", "Added certificate in Root CA certificate store as alias {0}" },
	{ "rootcertstore.cert.removing", "Removing certificate in Root CA certificate store" },
	{ "rootcertstore.cert.removed", "Removed certificate in Root CA certificate store as alias {0}" },
	{ "rootcertstore.cert.instore", "Checking if certificate is in Root CA certificate store" },
	{ "rootcertstore.cert.canverify", "Check if certificate can be verified using certificates in Root CA certificate store" },
	{ "rootcertstore.cert.iterator", "Obtain certificate iterator in Root CA certificate store" },
	{ "rootcertstore.cert.getkeystore", "Obtain key store object of Root CA certificate store" },
	{ "rootcertstore.cert.verify.ok", "Certificate has been verified with Root CA certificates successfully" },
	{ "rootcertstore.cert.verify.fail", "Certificate has failed the verification with the Root CA certificates" },
	{ "rootcertstore.cert.tobeverified", "Certificate to be verified:\n{0}" },
	{ "rootcertstore.cert.tobecompared", "Comparing certificate against Root CA certificate:\n{0}" },

	{ "roothttpscertstore.cert.loading", "Loading SSL Root CA certificates from {0}" },
	{ "roothttpscertstore.cert.loaded", "Loaded SSL Root CA certificates from {0}" },
	{ "roothttpscertstore.cert.noload", "SSL Root CA certificates file not found: {0}" },
	{ "roothttpscertstore.cert.saving", "Saving SSL Root CA certificates in {0}" },
	{ "roothttpscertstore.cert.saved", "Saved SSL Root CA certificates in {0}" },
	{ "roothttpscertstore.cert.adding", "Adding certificate in SSL Root CA certificate store", },
	{ "roothttpscertstore.cert.added", "Added certificate in SSL Root CA certificate store as alias {0}" },
	{ "roothttpscertstore.cert.removing", "Removing certificate in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.removed", "Removed certificate in SSL Root CA certificate store as alias {0}" },
	{ "roothttpscertstore.cert.instore", "Checking if certificate is in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.canverify", "Check if certificate can be verified using certificates in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.iterator", "Obtain certificate iterator in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.getkeystore", "Obtain key store object of SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.verify.ok", "Certificate has been verified with SSL Root CA certificates successfully" },
	{ "roothttpscertstore.cert.verify.fail", "Certificate has failed the verification with the SSL Root CA certificates" },
	{ "roothttpscertstore.cert.tobeverified", "Certificate to be verified:\n{0}" },
	{ "roothttpscertstore.cert.tobecompared", "Comparing certificate against SSL Root CA certificate:\n{0}" },

	{ "sessioncertstore.cert.loading", "Loading certificates from Deployment session certificate store" },
	{ "sessioncertstore.cert.loaded", "Loaded certificates from Deployment session certificate store" },
	{ "sessioncertstore.cert.saving", "Saving certificates in Deployment session certificate store" },
	{ "sessioncertstore.cert.saved", "Saved certificates in Deployment session certificate store" },
	{ "sessioncertstore.cert.adding", "Adding certificate in Deployment session certificate store", },
	{ "sessioncertstore.cert.added", "Added certificate in Deployment session certificate store" },
	{ "sessioncertstore.cert.removing", "Removing certificate in Deployment session certificate store" },
	{ "sessioncertstore.cert.removed", "Removed certificate in Deployment session certificate store" },
	{ "sessioncertstore.cert.instore", "Checking if certificate is in Deployment session certificate store" },
	{ "sessioncertstore.cert.canverify", "Check if certificate can be verified using certificates in Deployment session certificate store" },
	{ "sessioncertstore.cert.iterator", "Obtain certificate iterator in Deployment session certificate store" },
	{ "sessioncertstore.cert.getkeystore", "Obtain key store object of Deployment session certificate store" },

	{ "iexplorer.cert.loading", "Loading certificates from Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.loaded", "Loaded certificates from Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.instore", "Checking if certificate is in Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.canverify", "Check if certificate can be verified using certificates in Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.iterator", "Obtain certificate iterator in Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.verify.ok", "Certificate has been verified with Internet Explorer {0} certificates successfully" },
	{ "iexplorer.cert.verify.fail", "Certificate has failed the verification with the Internet Explorer {0} certificates" },
	{ "iexplorer.cert.tobeverified", "Certificate to be verified:\n{0}" },
	{ "iexplorer.cert.tobecompared", "Comparing certificate against Internet Explorer {0} certificate:\n{1}" },

	{ "mozilla.cert.loading", "Loading certificates from Mozilla {0} certificate store" },
        { "mozilla.cert.loaded", "Loaded certificates from Mozilla {0} certificate store" },
        { "mozilla.cert.instore", "Checking if certificate is in Mozilla {0} certificate store" },
        { "mozilla.cert.canverify", "Check if certificate can be verified using certificates in Mozilla {0} certificate store" },
        { "mozilla.cert.iterator", "Obtain certificate iterator in Mozilla {0} certificate store" },
        { "mozilla.cert.verify.ok", "Certificate has been verified with Mozilla {0} certificates successfully" },
        { "mozilla.cert.verify.fail", "Certificate has failed the verification with the Mozilla {0} certificates" },
        { "mozilla.cert.tobeverified", "Certificate to be verified:\n{0}" },
        { "mozilla.cert.tobecompared", "Comparing certificate against Mozilla {0} certificate:\n{1}" },

        { "browserkeystore.jss.no", "JSS package is not found" },
        { "browserkeystore.jss.yes", "JSS package is loaded" },
        { "browserkeystore.jss.notconfig", "JSS is not configured" },
        { "browserkeystore.jss.config", "JSS is configured" },
        { "browserkeystore.mozilla.dir", "Accessing keys and certificate in Mozilla user profile: {0}" },
	{ "browserkeystore.password.dialog.buttonOK", "OK" },
	{ "browserkeystore.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "browserkeystore.password.dialog.buttonCancel", "Cancel" },
	{ "browserkeystore.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "browserkeystore.password.dialog.caption", "Password Needed" },
	{ "browserkeystore.password.dialog.text", "Please enter the password for the {0}:\n" },
	{ "mozillamykeystore.priv.notfound", "private key not found for cert : {0}" },
	{ "hostnameverifier.automation.ignoremismatch", "Automation: Ignore hostname mismatch" },

	{ "trustdecider.check.basicconstraints", "Check basic constraints failed in certificate" },
	{ "trustdecider.check.leafkeyusage", "Check leaf key usage failed in certificate" },
	{ "trustdecider.check.signerkeyusage", "Check signer key usage failed in certificate" },
	{ "trustdecider.check.extensions", "Check critical extensions failed in certificate" },
	{ "trustdecider.check.signature", "Check signature failed in certificate" },
	{ "trustdecider.check.basicconstraints.certtypebit", "Check netscape type bit failed in certificate" },
	{ "trustdecider.check.basicconstraints.extensionvalue", "Check netscape extension value failed in certificate" },
	{ "trustdecider.check.basicconstraints.bitvalue", "Check netscape bits 5,6,7 value failed in certificate" },
	{ "trustdecider.check.basicconstraints.enduser", "Check end user act as a CA failed in certificate" },
	{ "trustdecider.check.basicconstraints.pathlength", "Check path length constraints failed in certificate" },
	{ "trustdecider.check.leafkeyusage.length", "Check length of key usage failed in certificate" },
	{ "trustdecider.check.leafkeyusage.digitalsignature", "Check digital signature failed in certificate" },
	{ "trustdecider.check.leafkeyusage.extkeyusageinfo", "Check extension key usage info failed in certificate" },
	{ "trustdecider.check.leafkeyusage.tsaextkeyusageinfo", "Check TSA extension key usage info failed in certificate" },
	{ "trustdecider.check.leafkeyusage.certtypebit", "Check netscape type bit failed in certificate" },
	{ "trustdecider.check.signerkeyusage.lengthandbit", "Check length and bit failed in certificate" },
	{ "trustdecider.check.signerkeyusage.keyusage", "Check key usage failed in certificate" },
	{ "trustdecider.check.canonicalize.updatecert", "Update root certificate with certificate in cacerts file" },
	{ "trustdecider.check.canonicalize.missing", "Add missing root certificate" },
	{ "trustdecider.check.gettrustedcert.find", "Find the valid root CA in cacerts file" },
	{ "trustdecider.check.gettrustedissuercert.find", "Find the missing valid root CA in cacerts file" },
	{ "trustdecider.check.timestamping.no", "No timestamping info available" },
	{ "trustdecider.check.timestamping.yes", "Timestamping info is available" },
	{ "trustdecider.check.timestamping.tsapath", "Start checking TSA certificate path" },
	{ "trustdecider.check.timestamping.inca", "Even though certificate has expired, but it timestamped in valid period and has valid TSA" },
	{ "trustdecider.check.timestamping.notinca", "The certificate has expired, but TSA is not valid" },
	{ "trustdecider.check.timestamping.valid", "The certificate has expired, and it timestamped in valid period" },
	{ "trustdecider.check.timestamping.invalid", "The certificate has expired, and it timestamped in invalid period" },
	{ "trustdecider.check.timestamping.need", "The certificate has been expired, need to check timestamping info" },
	{ "trustdecider.check.timestamping.noneed", "The certificate hasn't been expired, no need to check timestamping info" },
	{ "trustdecider.check.timestamping.notfound", "The new timestamping API is not found" },
	{ "trustdecider.user.grant.session", "User has granted the priviledges to the code for this session only" },
	{ "trustdecider.user.grant.forever", "User has granted the priviledges to the code forever" },
	{ "trustdecider.user.deny", "User has denied the priviledges to the code" },
	{ "trustdecider.automation.trustcert", "Automation: Trust RSA certificate for signing" },
	{ "trustdecider.code.type.applet", "applet" },
	{ "trustdecider.code.type.application", "application" },
	{ "trustdecider.code.type.extension", "extension" },
	{ "trustdecider.code.type.installer", "installer" },
	{ "trustdecider.user.cannot.grant.any", "Your security configuration will not allow granting permission to new certificates" },
	{ "trustdecider.user.cannot.grant.notinca", "Your security configuration will not allow granting permission to self signed certificates" },
	{ "x509trustmgr.automation.ignoreclientcert", "Automation: Ignore untrusted client certificate" },
	{ "x509trustmgr.automation.ignoreservercert", "Automation: Ignore untrusted server certificate" },

	{ "net.proxy.text", "Proxy: " },
	{ "net.proxy.override.text", "Proxy Overrides: " },
	{ "net.proxy.configuration.text", "Proxy Configuration: " },
	{ "net.proxy.type.browser", "Browser Proxy Configuration" },
	{ "net.proxy.type.auto", "Automatic Proxy Configuration" },
	{ "net.proxy.type.manual", "Manual Configuration" },
	{ "net.proxy.type.none", "No proxy" },
	{ "net.proxy.type.user", "User has overriden browser's proxy settings." },
	{ "net.proxy.loading.ie", "Loading proxy configuration from Internet Explorer ..."},
	{ "net.proxy.loading.ns", "Loading proxy configuration from Netscape Navigator ..."},
	{ "net.proxy.loading.userdef", "Loading user-defined proxy configuration ..."},
	{ "net.proxy.loading.direct", "Loading direct proxy configuration ..."},
	{ "net.proxy.loading.manual", "Loading manual proxy configuration ..."},
	{ "net.proxy.loading.auto",   "Loading auto proxy configuration ..."},
	{ "net.proxy.loading.browser",   "Loading browser proxy configuration ..."},
	{ "net.proxy.loading.manual.error", "Unable to use manual proxy configuration - fallback to DIRECT"},
	{ "net.proxy.loading.auto.error", "Unable to use auto proxy configuration - fallback to MANUAL"},
	{ "net.proxy.loading.done", "Done."},
	{ "net.proxy.browser.pref.read", "Reading user preference file from {0}"},
	{ "net.proxy.browser.proxyEnable", "    Proxy enable: {0}"},
	{ "net.proxy.browser.proxyList",     "    Proxy list: {0}"},
	{ "net.proxy.browser.proxyOverride", "    Proxy override: {0}"},
	{ "net.proxy.browser.autoConfigURL", "    Auto config URL: {0}"},
	{ "net.proxy.browser.smartConfig", "Ping the proxy server {0} on port {1}"},
        { "net.proxy.browser.connectionException", "Proxy server {0} on port {1} can not be reached"},
	{ "net.proxy.ns6.regs.exception", "Error reading registry file: {0}"},
	{ "net.proxy.pattern.convert", "Convert proxy bypass list to regular expression: "},
	{ "net.proxy.pattern.convert.error", "Unable to convert proxy bypass list to regular expression - ignore"},
	{ "net.proxy.auto.download.ins", "Downloading INS file from {0}" },
	{ "net.proxy.auto.download.js", "Downloading auto proxy file from {0}" },
	{ "net.proxy.auto.result.error", "Unable to determine proxy setting from evaluation - fallback to DIRECT"},
        { "net.proxy.service.not_available", "Proxy service not available for {0} - default to DIRECT" },
	{ "net.proxy.error.caption", "Error - Proxy Configuration" },
	{ "net.proxy.nsprefs.error", "<html><b>Unable to Retrieve Proxy Settings</b></html>Fallback to other proxy configuration.\n" },
	{ "net.proxy.connect", "Connecting {0} with proxy={1}" },

	{ "net.authenticate.caption", "Password Needed - Networking"},
	{ "net.authenticate.label", "<html><b>Enter your username and password:</b></html>"},
	{ "net.authenticate.resource", "Resource:" },
	{ "net.authenticate.username", "User name:" },
        { "net.authenticate.username.mnemonic", "VK_U" },
	{ "net.authenticate.password", "Password:" },
        { "net.authenticate.password.mnemonic", "VK_P" },
	{ "net.authenticate.firewall", "Server:" },
	{ "net.authenticate.domain", "Domain:"},
        { "net.authenticate.domain.mnemonic", "VK_D" },
	{ "net.authenticate.realm", "Realm:" },
	{ "net.authenticate.scheme", "Scheme:" },
	{ "net.authenticate.unknownSite", "Unknown Site" },

	{ "net.cookie.cache", "Cookie Cache: " },
	{ "net.cookie.server", "Server {0} requesting to set-cookie with \"{1}\"" },
	{ "net.cookie.connect", "Connecting {0} with cookie \"{1}\"" },
	{ "net.cookie.ignore.setcookie", "Cookie service is unavailable - ignore \"Set-Cookie\"" },
	{ "net.cookie.noservice", "Cookie service is not available - use cache to determine \"Cookie\"" },

	{"about.java.version", "Version {0} (build {1})"},
	{"about.prompt.info", "For more information about Java technology and to explore great Java applications, visit"},
	{"about.home.link", "http://www.java.com"},
	{"about.option.close", "Close"},
	{"about.option.close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{"about.copyright", "Copyright 2004 Sun Microsystems, Inc."},
	{"about.legal.note", "All rights reserved. Use is subject to license terms."},


	{ "cert.remove_button", "Remove" },
        { "cert.remove_button.mnemonic", "VK_M" },
        { "cert.import_button", "Import" },
        { "cert.import_button.mnemonic", "VK_I" },
        { "cert.export_button", "Export" },
        { "cert.export_button.mnemonic", "VK_E" },
        { "cert.details_button", "Details" },
        { "cert.details_button.mnemonic", "VK_D" },
        { "cert.viewcert_button", "View Certificate" },
        { "cert.viewcert_button.mnemonic", "VK_V" },
        { "cert.close_button", "Close" },
        { "cert.close_button.mnemonic", "VK_C" },
        { "cert.type.trusted_certs", "Trusted Certificates" },
        { "cert.type.secure_site", "Secure Site" },
        { "cert.type.client_auth", "Client Authentication" },
        { "cert.type.signer_ca", "Signer CA" },
        { "cert.type.secure_site_ca", "Secure Site CA" },
        { "cert.rbutton.user", "User" },
        { "cert.rbutton.system", "System" },
        { "cert.settings", "Certificates" },
        { "cert.dialog.import.error.caption", "Error - Import Certificate" },
        { "cert.dialog.export.error.caption", "Error - Export Certificate" },
	{ "cert.dialog.import.format.text", "<html><b>Unrecognized File Format</b></html>No certificate will be imported." },
	{ "cert.dialog.export.password.text", "<html><b>Invalid Password</b></html>The password you entered is incorrect." },
	{ "cert.dialog.import.file.text", "<html><b>File Doesn't Exist</b></html>No certificate will be imported." },
	{ "cert.dialog.import.password.text", "<html><b>Invalid Password</b></html>The password you entered is incorrect." },
        { "cert.dialog.password.caption", "Password" },
        { "cert.dialog.password.import.caption", "Password Needed - Import" },
        { "cert.dialog.password.export.caption", "Password Needed - Export" },
        { "cert.dialog.password.text", "Enter a password to access this file:\n" },
        { "cert.dialog.exportpassword.text", "Enter a password to access this private key in client authentication keystore:\n" },
        { "cert.dialog.savepassword.text", "Enter a password to save this key file:\n" },
        { "cert.dialog.password.okButton", "OK" },
        { "cert.dialog.password.cancelButton", "Cancel" },
        { "cert.dialog.export.error.caption", "Error - Export Certificate" },
        { "cert.dialog.export.text", "<html><b>Unable to Export</b></html>No certificate is exported." },
        { "cert.dialog.remove.text", "Do you really want to delete the certificate(s)?" },
	{ "cert.dialog.remove.caption", "Remove Certificate" },
	{ "cert.dialog.issued.to", "Issued To" },
	{ "cert.dialog.issued.by", "Issued By" },
	{ "cert.dialog.user.level", "User" },
	{ "cert.dialog.system.level", "System" },
	{ "cert.dialog.certtype", "Certificate type: "},

	{ "controlpanel.jre.platformTableColumnTitle","Platform"},
	{ "controlpanel.jre.productTableColumnTitle","Product" },
	{ "controlpanel.jre.locationTableColumnTitle","Location" },
	{ "controlpanel.jre.pathTableColumnTitle","Path" },
	{ "controlpanel.jre.enabledTableColumnTitle", "Enabled" },

	{ "jnlp.jre.title", "JNLP Runtime Settings" },
	{ "jnlp.jre.versions", "Java Runtime Versions" },
	{ "jnlp.jre.choose.button", "Choose" },
	{ "jnlp.jre.find.button", "Find" },
	{ "jnlp.jre.add.button", "Add" },
	{ "jnlp.jre.remove.button", "Remove" },
	{ "jnlp.jre.ok.button", "OK" },
	{ "jnlp.jre.cancel.button", "Cancel" },
	{ "jnlp.jre.choose.button.mnemonic", "VK_H" },
	{ "jnlp.jre.find.button.mnemonic", "VK_F" },
	{ "jnlp.jre.add.button.mnemonic", "VK_A" },
	{ "jnlp.jre.remove.button.mnemonic", "VK_R" },
	{ "jnlp.jre.ok.button.mnemonic", "VK_O" },
	{ "jnlp.jre.cancel.button.mnemonic", "VK_C" },

	{ "find.dialog.title", "JRE Finder"},
	{ "find.title", "Java Runtime Environments"},
	{ "find.cancelButton", "Cancel"},
	{ "find.prevButton", "Previous"},
	{ "find.nextButton", "Next"},
	{ "find.cancelButtonMnemonic", "VK_C"},
	{ "find.prevButtonMnemonic", "VK_P"},
	{ "find.nextButtonMnemonic", "VK_N"},
	{ "find.intro", "In order to launch applications, Java Web Start needs to know the locations of installed Java Runtime Environments.\n\nYou can either select a known JRE, or select a directory in the file system from which to search for JREs." },

	{ "find.searching.title", "Searching for available JREs, this may take several minutes." },
	{ "find.searching.prefix", "checking: " },
	{ "find.foundJREs.title", "The following JREs were found, click Next to add them" },
	{ "find.noJREs.title", "Unable to locate a JRE, click Previous to select a different location to search from" },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Java(tm) Deployment Properties\n"
                        + "# DO NOT EDIT THIS FILE.  It is machine generated.\n"
                        + "# Use the Java Control Panel to edit properties." },
        { "config.unknownSubject", "Unknown Subject" },
        { "config.unknownIssuer", "Unknown Issuer" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>Malformed URL</b></html>Automatic proxy configuration URL is invalid." },
        { "config.proxy.autourl.invalid.caption", "Error - Proxies" },
	// Java Web Start Properties
	 { "APIImpl.clipboard.message.read", "This application has requested read-only access to the system clipboard.  The application might gain access to confidential information stored on the clipboard.  Do you want to allow this action?" },
        { "APIImpl.clipboard.message.write", "This application has requested write access to the system clipboard.  The application might overwrite information stored on the clipboard.  Do you want to allow this action?" },
        { "APIImpl.file.open.message", "This application has requested read access to the filesystem.  The application might gain access to confidential information stored on the filesystem.  Do you want to allow this action?" },
        { "APIImpl.file.save.fileExist", "{0} already exists.\n Do you want to replace it?" },
        { "APIImpl.file.save.fileExistTitle", "File Exists" },
        { "APIImpl.file.save.message", "This application has requested read/write access to a file on the local filesystem.  Allowing this action will only give the application access to the file(s) selected in the following file dialog box.  Do you want to allow this action?" },
        { "APIImpl.persistence.accessdenied", "Access to persistent storage denied for URL {0}" },
        { "APIImpl.persistence.filesizemessage", "Maximum file length exceeded" },
        { "APIImpl.persistence.message", "This application has requested additional local disk storage space.  Currently, the maximum allotted storage is {1} bytes.  The application is requesting that this be increased to {0} bytes.  Do you wish to allow this action?" },
        { "APIImpl.print.message", "This application has requested access to the default printer.  Allowing this action will give the application write access to the printer.  Do you want to allow this action?" },
	{ "APIImpl.extended.fileOpen.message1", "This application has requested read/write access to the following file(s) on the local file system:"},
	{ "APIImpl.extended.fileOpen.message2", "Allowing this action will only give the application access to the file(s) listed above.  Do you want to allow this action?"},
        { "APIImpl.securityDialog.no", "No" },
        { "APIImpl.securityDialog.remember", "Do not show this advisory again" },
        { "APIImpl.securityDialog.title", "Security Advisory" },
        { "APIImpl.securityDialog.yes", "Yes" },
        { "Launch.error.installfailed", "Installation failed" },
        { "aboutBox.closeButton", "Close" },
        { "aboutBox.versionLabel", "Version {0} (build {1})" },
        { "applet.failedToStart", "Failed to start Applet: {0}" },
        { "applet.initializing", "Initializing Applet" },
        { "applet.initializingFailed", "Failed to initialize Applet: {0}" },
        { "applet.running", "Running..." },
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "Windows must restart for the changes to take effect.\n\nDo you wish to restart Windows now?" },
        { "extensionInstall.rebootTitle", "Restart Windows" }, 
        { "install.configButton", "Configure ..." },
        { "install.configMnemonic", "VK_C" },
        { "install.errorInstalling", "Unexpected error trying to install Java Runtime Environment, please try again." },
        { "install.errorRestarting", "Unexpected error starting, please try again." },
        { "install.title", "{0} - Create shortcut(s)" },

        { "install.windows.both.message", "Would you like to create desktop and start menu shortcuts for\n{0}?" },
	{ "install.gnome.both.message", "Would you like to create desktop and application menu shortcuts for\n{0}?" },
	{ "install.desktop.message", "Would you like to create desktop shortcut(s) for\n{0}?" },
	{ "install.windows.menu.message", "Would you like to create start menu shortcut(s) for\n{0}?" },
	{ "install.gnome.menu.message", "Would you like to create application menu shortcut(s) for\n{0}?" },
        { "install.noButton", "No" },
        { "install.noMnemonic", "VK_N" },
        { "install.yesButton", "Yes" },
        { "install.yesMnemonic", "VK_Y" },
        { "launch.cancel", "Cancel" },
        { "launch.downloadingJRE", "Requesting JRE {0} from {1}" },
        { "launch.error.badfield", "The field {0} has an invalid value: {1}" },
        { "launch.error.badfield-signedjnlp", "The field {0} has an invalid value in the signed launch file: {1}" },
        { "launch.error.badfield.download.https", "Unable to Download via HTTPS" },
        { "launch.error.badfield.https", "Java 1.4+ is required for HTTPS support" },
        { "launch.error.badjarfile", "Corrupted JAR file at {0}" },
        { "launch.error.badjnlversion", "Unsupported JNLP version in launch file: {0}. Only versions 1.0 and 1.5 are supported with this version. Please contact the application vendor to report this problem." },
        { "launch.error.badmimetyperesponse", "Bad MIME type returned from server when accessing resource: {0} - {1}" },
        { "launch.error.badsignedjnlp", "Failed to validate signing of launch file. The signed version does not match the downloaded version." },
        { "launch.error.badversionresponse", "Bad version in response from server when accessing resource: {0} - {1}" },
        { "launch.error.canceledloadingresource", "Loading of resource {0} was canceled by user" },
        { "launch.error.category.arguments", "Invalid Argument Error" },
        { "launch.error.category.download", "Download Error" },
        { "launch.error.category.launchdesc", "Launch File Error" },
        { "launch.error.category.memory", "OutOfMemory Error" },
        { "launch.error.category.security", "Security Error" },
        { "launch.error.category.config", "System Configuration" },
        { "launch.error.category.unexpected", "Unexpected Error" },
        { "launch.error.couldnotloadarg", "Could not load file/URL specified: {0}" },
        { "launch.error.errorcoderesponse-known", "Error Code {1} ({2}) returned from server when accessing resource: {0}" },
        { "launch.error.errorcoderesponse-unknown", "Error Code 99 (Unknown error) returned from server when accessing resource: {0}" },
        { "launch.error.failedexec", "Could not launch the Java Runtime Environment version {0}" },
        { "launch.error.failedloadingresource", "Unable to load resource: {0}" },
        { "launch.error.invalidjardiff", "Unable to apply incremental update for resource: {0}" },
        { "launch.error.jarsigning-badsigning", "Could not verify signing in resource: {0}" },
        { "launch.error.jarsigning-missingentry", "Missing signed entry in resource: {0}" },
        { "launch.error.jarsigning-missingentryname", "Missing signed entry: {0}" },
        { "launch.error.jarsigning-multicerts", "More than one certificate used to sign resource: {0}" },
        { "launch.error.jarsigning-multisigners", "More than one signature on entry in resource: {0}" },
        { "launch.error.jarsigning-unsignedfile", "Found unsigned entry in resource: {0}" },
        { "launch.error.missingfield", "The following required field is missing from the launch file: {0}" },
        { "launch.error.missingfield-signedjnlp", "The following required field is missing from the signed launch file: {0}" },
        { "launch.error.missingjreversion", "No JRE version found in launch file for this system" },
        { "launch.error.missingversionresponse", "Missing version field in response from server when accessing resource: {0}" },
        { "launch.error.multiplehostsreferences", "Multiple hosts referenced in resources" },
        { "launch.error.nativelibviolation", "Use of native libraries requires unrestricted access to system" },
        { "launch.error.noJre", "The application has requested a version of the JRE that is currently not locally installed. Java Web Start was unable to automatically download and install the requested version. This JRE must be installed manually.\n\n" },
        { "launch.error.wont.download.jre", "The application has requested a version of the JRE (version {0}) that currently is not locally installed. Java Web Start was not allowed to automatically download and install the requested version. This JRE must be installed manually." },
        { "launch.error.cant.download.jre", "The application has requested a version of the JRE (version {0}) that currently is not locally installed. Java Web Start is unable to automatically download and install the requested version. This JRE must be installed manually." },
        { "launch.error.cant.access.system.cache", "The current user does not have write access to the system cache." },
        { "launch.error.cant.access.user.cache", "The current user does not have write access to the cache." },
        { "launch.error.noappresources", "No application resources are specified for this platform. Please, contact the vendor of the application to make sure that this is a supported platform." },
        { "launch.error.nomainclass", "Could not find main-class {0} in {1}" },
        { "launch.error.nomainclassspec", "No main class specified for application" },
        { "launch.error.nomainjar", "No main JAR file specified." },
        { "launch.error.nonstaticmainmethod", "The main() method must be static" },
        { "launch.error.offlinemissingresource", "The application can not be run off-line, since not all of the needed resources have been downloaded locally" },
        { "launch.error.parse", "Could not parse launch file. Error at line {0, number}." },
        { "launch.error.parse-signedjnlp", "Could not parse the signed launch file. Error at line {0, number}." },
        { "launch.error.resourceID", "{0}" },
        { "launch.error.resourceID-version", "({0}, {1})" },
        { "launch.error.singlecertviolation", "JAR resources in JNLP file are not signed by same certificate" },
        { "launch.error.toomanyargs", "Too many arguments supplied: {0}" },
        { "launch.error.unsignedAccessViolation", "Unsigned application requesting unrestricted access to system" },
        { "launch.error.unsignedResource", "Unsigned resource: {0}" },
        { "launch.estimatedTimeLeft", "Estimated time left: {0,number,00}:{1,number,00}:{2,number,00}" },
        { "launch.extensiondownload", "Downloading extension descriptor ({0} remaining)" },
        { "launch.extensiondownload-name", "Downloading {0} descriptor ({1} remaining)" },
        { "launch.initializing", "Initializing..." },
        { "launch.launchApplication", "Starting application..." },
        { "launch.launchInstaller", "Starting installer..." },
        { "launch.launchingExtensionInstaller", "Running installer. Please wait..." },
        { "launch.loadingNetProgress", "Read {0}" },
        { "launch.loadingNetProgressPercent", "Read {0} of {1} ({2}%)" },
        { "launch.loadingNetStatus", "Loading {0} from {1}" },
        { "launch.loadingResourceFailed", "Failed to load resource" },
        { "launch.loadingResourceFailedSts", "Requested {0}" },
        { "launch.patchingStatus", "Patching {0} from {1}" },
        { "launch.progressScreen", "Checking for latest version..." },
        { "launch.stalledDownload", "Waiting for data..." },
        { "launch.validatingProgress", "Scanning entries ({0}% done)" },
        { "launch.validatingStatus", "Validating {0} from {1}" },
        { "launcherrordialog.abort", "Abort" },
        { "launcherrordialog.abortMnemonic", "VK_A" },
        { "launcherrordialog.brief.continue", "Unable to continue execution" },
        { "launcherrordialog.brief.details", "Details" },
        { "launcherrordialog.brief.message", "Unable to launch the specified application." },
	{ "launcherrordialog.import.brief.message", "Unable to import the specified application." },
        { "launcherrordialog.brief.messageKnown", "Unable to launch {0}." },
	{ "launcherrordialog.import.brief.messageKnown", "Unable to import {0}." },
        { "launcherrordialog.brief.ok", "Ok" },
        { "launcherrordialog.brief.title", "Java Web Start - {0}" },
        { "launcherrordialog.consoleTab", "Console" },
        { "launcherrordialog.errorcategory", "Category: {0}\n\n" },
        { "launcherrordialog.errorintro", "An error occurred while launching/running the application.\n\n" },
	{ "launcherrordialog.import.errorintro", "An error occurred while importing the application.\n\n" },
        { "launcherrordialog.errormsg", "{0}" },
        { "launcherrordialog.errortitle", "Title: {0}\n" },
        { "launcherrordialog.errorvendor", "Vendor: {0}\n" },
        { "launcherrordialog.exceptionTab", "Exception" },
        { "launcherrordialog.generalTab", "General" },
        { "launcherrordialog.genericerror", "Unexpected exception: {0}" },
        { "launcherrordialog.jnlpMainTab", "Main Launch File" },
        { "launcherrordialog.jnlpTab", "Launch File" },
        { "launcherrordialog.title", "Java Web Start - {0}" },
        { "launcherrordialog.wrappedExceptionTab", "Wrapped Exception" },

        { "uninstall.failedMessage", "Unable to completely uninstall application." },
        { "uninstall.failedMessageTitle", "Uninstall" },
        { "install.alreadyInstalled", "There is already a shortcut for {0}. Would you like to create a shortcut anyway?" },
        { "install.alreadyInstalledTitle", "Create Shortcut..." },
        { "install.desktopShortcutName", "{0}" },
        { "install.installFailed", "Unable to create a shortcut for {0}." },
        { "install.installFailedTitle", "Create Shortcut" },
        { "install.startMenuShortcutName", "{0}" },
        { "install.startMenuUninstallShortcutName", "Uninstall {0}" },
        { "install.uninstallFailed", "Unable to remove the shortcuts for {0}. Please try again." },
        { "install.uninstallFailedTitle", "Remove Shortcuts" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "You can not run this program because your system deployment.config file states that an enterprise configuration file is mandatory, and the required: {0} is not available." },

	// Jnlp Cache Viewer:
	{ "jnlp.viewer.title", "Java Application Cache Viewer" },
	{ "jnlp.viewer.all", "All" },
	{ "jnlp.viewer.type", "{0}" },
	{ "jnlp.viewer.totalSize",  "Total resources size:  {0}" },
 	{ "jnlp.viewer.emptyCache", "{0} cache is empty"},
 	{ "jnlp.viewer.noCache", "System cache is not configured"},

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

	{ "jnlp.viewer.remove.btn", "Remove" },
	{ "jnlp.viewer.remove.1.btn", "Remove Selected {0}" },
	{ "jnlp.viewer.remove.2.btn", "Remove Selected Entries" },
	{ "jnlp.viewer.uninstall.btn", "Uninstall" },
	{ "jnlp.viewer.launch.offline.btn", "Launch Offline" },
	{ "jnlp.viewer.launch.online.btn", "Launch Online" },

        { "jnlp.viewer.file.menu", "File" },
        { "jnlp.viewer.edit.menu", "Edit" },
        { "jnlp.viewer.app.menu", "Application" },
        { "jnlp.viewer.view.menu", "View" },
        { "jnlp.viewer.help.menu", "Help" },

	{ "jnlp.viewer.cpl.mi", "Launch Java Control Panel" },
	{ "jnlp.viewer.exit.mi", "Exit" },

	{ "jnlp.viewer.reinstall.mi", "Reinstall ..." },
	{ "jnlp.viewer.preferences.mi", "Preferences ..." },

	{ "jnlp.viewer.launch.offline.mi", "Launch Offline" },
	{ "jnlp.viewer.launch.online.mi", "Launch Online" },
	{ "jnlp.viewer.install.mi", "Install Shortcuts" },
	{ "jnlp.viewer.uninstall.mi", "Uninstall Shortcuts" },
	{ "jnlp.viewer.remove.0.mi", "Remove" },
	{ "jnlp.viewer.remove.mi", "Remove {0}" },
	{ "jnlp.viewer.show.mi", "Show JNLP Descriptor" },
	{ "jnlp.viewer.browse.mi", "Browse Home Page" },

	{ "jnlp.viewer.view.0.mi", "All Types" },
	{ "jnlp.viewer.view.1.mi", "Applications" },
	{ "jnlp.viewer.view.2.mi", "Applets" },
	{ "jnlp.viewer.view.3.mi", "Libraries" },
	{ "jnlp.viewer.view.4.mi", "Installers" },

	{ "jnlp.viewer.view.0", "All Types" },
	{ "jnlp.viewer.view.1", "Applications" },
	{ "jnlp.viewer.view.2", "Applets" },
	{ "jnlp.viewer.view.3", "Libraries" },
	{ "jnlp.viewer.view.4", "Installers" },

	{ "jnlp.viewer.about.mi", "About" },
	{ "jnlp.viewer.help.java.mi", "J2SE Homepage" },
	{ "jnlp.viewer.help.jnlp.mi", "JNLP Homepage" },

        { "jnlp.viewer.app.column", "Application" },
        { "jnlp.viewer.vendor.column", "Vendor" },
        { "jnlp.viewer.type.column", "Type" },
        { "jnlp.viewer.size.column", "Size" },
        { "jnlp.viewer.date.column", "Date" },
        { "jnlp.viewer.status.column", "Status" },

        { "jnlp.viewer.app.column.tooltip", "Icon and title of this Application, Applet, or Extension" },
        { "jnlp.viewer.vendor.column.tooltip", "Company deploying this item" },
        { "jnlp.viewer.type.column.tooltip", "Type of this item" },
        { "jnlp.viewer.size.column.tooltip", "Size of this item and all it's resources" },
        { "jnlp.viewer.date.column.tooltip", "Date this Application, Applet or Installer was last run" },
        { "jnlp.viewer.status.column.tooltip", "Icon showing how and if this item can be launched" },


        { "jnlp.viewer.application", "Application" },
        { "jnlp.viewer.applet", "Applet" },
        { "jnlp.viewer.extension", "Library" },
        { "jnlp.viewer.installer", "Installer" },

	{ "jnlp.viewer.offline.tooltip",
		 "This {0} can be launched online or offline" },
	{ "jnlp.viewer.online.tooltip", "This {0} can be launched online" },
	{ "jnlp.viewer.norun1.tooltip", 
        	"This {0} can only be launched from a web browser" },
        { "jnlp.viewer.norun2.tooltip", "Extensions can not be launched" },

	{ "jnlp.viewer.show.title", "JNLP Descriptor for: {0}" },

	{ "jnlp.viewer.removing", "Removing ..." },
	{ "jnlp.viewer.launching", "Launching ..." },
	{ "jnlp.viewer.browsing", "Launching Browser ..." },
	{ "jnlp.viewer.sorting", "Sorting entries ..." },
	{ "jnlp.viewer.searching", "Searching entries ..." },
	{ "jnlp.viewer.installing", "Installing ..." },

        { "jnlp.viewer.reinstall.title", "Reinstall Removed JNLP Applications" },
	{ "jnlp.viewer.reinstallBtn", "Reinstall Selected Application(s)" },
	{ "jnlp.viewer.reinstallBtn.mnemonic", "VK_R" },
        { "jnlp.viewer.closeBtn", "Close" },
        { "jnlp.viewer.closeBtn.mnemonic", "VK_C" },

	{ "jnlp.viewer.reinstall.column.title", "Title:" },
	{ "jnlp.viewer.reinstall.column.location", "Location:" },

	// cache size warning
	{ "jnlp.cache.warning.title", "JNLP Cache Size Warning" },
	{ "jnlp.cache.warning.message", "Warning: \n\n"+
		"You have exceeded the recommended amount of disk space for\n"+
		"JNLP Applications and Resources in your cache.\n\n"+
		"You are currently using: {0}\n"+
		"The recommended limit is: {1}\n\n"+
		"Please use the Java Control Panel to remove some \n"+
		"Applications or Resources or to set a higher limit." },

        // Control Panel
        { "control.panel.title", "Java Control Panel" },
        { "control.panel.general", "General" },
        { "control.panel.security", "Security" },
        { "control.panel.java", "Java" },
        { "control.panel.update", "Update" },
        { "control.panel.advanced", "Advanced" },

        // Common Strings used in different panels.
        { "common.settings", "Settings" },
        { "common.ok_btn", "OK" },
        { "common.ok_btn.mnemonic", "VK_O" },
        { "common.cancel_btn", "Cancel" },
        { "common.cancel_btn.mnemonic", "VK_C" },
        { "common.apply_btn", "Apply" },
        { "common.apply_btn.mnemonic", "VK_A" },
        { "common.add_btn", "Add" },
        { "common.add_btn.mnemonic", "VK_A" },
        { "common.remove_btn", "Remove" },
        { "common.remove_btn.mnemonic", "VK_R" },

        // Network Settings Dialog
        { "network.settings.dlg.title", "Network Settings" },
        { "network.settings.dlg.border_title", " Network Proxy Settings " },
        { "network.settings.dlg.browser_rbtn", "Use browser settings" },
        { "browser_rbtn.mnemonic", "VK_B" },
        { "network.settings.dlg.manual_rbtn", "Use proxy server" },
        { "manual_rbtn.mnemonic", "VK_P" },
        { "network.settings.dlg.address_lbl", "Address:" },
	{ "network.settings.dlg.port_lbl", "Port:" },
        { "network.settings.dlg.advanced_btn", "Advanced..." },
        { "network.settings.dlg.advanced_btn.mnemonic", "VK_A" },
        { "network.settings.dlg.bypass_text", "Bypass proxy server for local addresses" },
        { "network.settings.dlg.bypass.mnemonic", "VK_Y" },
        { "network.settings.dlg.autoconfig_rbtn", "Use automatic proxy configuration script" },
        { "autoconfig_rbtn.mnemonic", "VK_T" },
        { "network.settings.dlg.location_lbl", "Script location: " },
        { "network.settings.dlg.direct_rbtn", "Direct connection" },
        { "direct_rbtn.mnemonic", "VK_D" },
        { "network.settings.dlg.browser_text", "Automatic configuration may override manual settings. To ensure the use of manual settings, disable automatic configuration." },
        { "network.settings.dlg.proxy_text", "Overwrite browser proxy settings." },
        { "network.settings.dlg.auto_text", "Use automatic proxy configuration script at the specified location." },
        { "network.settings.dlg.none_text", "Use direct connection." },

        // Advanced Network Settings Dialog
        { "advanced.network.dlg.title", "Advanced Network Settings" },
        { "advanced.network.dlg.servers", " Servers " },
        { "advanced.network.dlg.type", "Type" },
        { "advanced.network.dlg.http", "HTTP:" },
        { "advanced.network.dlg.secure", "Secure:" },
        { "advanced.network.dlg.ftp", "FTP:" },
        { "advanced.network.dlg.socks", "Socks:" },
        { "advanced.network.dlg.proxy_address", "Proxy Address" },
	{ "advanced.network.dlg.port", "Port" },
        { "advanced.network.dlg.same_proxy", " Use same proxy server for all protocols" },
        { "advanced.network.dlg.same_proxy.mnemonic", "VK_U" },
        { "advanced.network.dlg.exceptions", " Exceptions " },
        { "advanced.network.dlg.no_proxy", " Do not use proxy server for addresses beginning with" },
        { "advanced.network.dlg.no_proxy_note", " Use semicolon (;) to separate entries." },

        // DeleteFilesDialog
        { "delete.files.dlg.title", "Delete Temporary Files" },
        { "delete.files.dlg.temp_files", "Delete the following temporary files?" },
        { "delete.files.dlg.applets", "Downloaded Applets" },
        { "delete.files.dlg.applications", "Downloaded Applications" },
        { "delete.files.dlg.other", "Other Files" },

	// General
	{ "general.cache.border.text", " Temporary Internet Files " },
	{ "general.cache.delete.text", "Delete Files..." },
        { "general.cache.delete.text.mnemonic", "VK_D" },
	{ "general.cache.settings.text", "Settings..." },
        { "general.cache.settings.text.mnemonic", "VK_S" },
	{ "general.cache.desc.text", "Files you use in Java applications are stored in a special folder for quick execution later.  Only advanced users should delete files or modify these settings." },
	{ "general.network.border.text", " Network Settings " },
	{ "general.network.settings.text", "Network Settings..." },
        { "general.network.settings.text.mnemonic", "VK_N" },
	{ "general.network.desc.text", "Network settings are used when making Internet connections.  By default, Java will use the network settings in your web browser.  Only advanced users should modify these settings." },
        { "general.about.border", "About" },
        { "general.about.text", "View version information about Java Control Panel." },
        { "general.about.btn", "About..." },
        { "general.about.btn.mnemonic", "VK_B" },


	// Security
	{ "security.certificates.border.text", " Certificates " },
	{ "security.certificates.button.text", "Certificates..." },
        { "security.certificates.button.mnemonic", "VK_E" },
	{ "security.certificates.desc.text", "Use certificates to positively identify yourself, certifications, authorities, and publishers." },
	{ "security.policies.border.text", " Policies " },
	{ "security.policies.advanced.text", "Advanced..." },
        { "security.policies.advanced.mnemonic", "VK_D" },
	{ "security.policies.desc.text", "Use security policies to control the security barriers around applications and applets." },

	// Update
	{ "update.notify.border.text", " Update Notification " }, // this one is not currently used.  See update panel!!!
	{ "update.updatenow.button.text", "Update Now" },
	{ "update.updatenow.button.mnemonic", "VK_U" },
	{ "update.advanced.button.text", "Advanced..." },
	{ "update.advanced.button.mnemonic", "VK_D" },
	{ "update.desc.text", "The Java Update mechanism ensures you have the most updated version of the Java platform.  The options below let you control how updates are obtained and applied." },
        { "update.notify.text", "Notify Me:" },
        { "update.notify_install.text", "Before installing" },
        { "update.notify_download.text", "Before downloading and before installing" },
        { "update.autoupdate.text", "Check for Updates Automatically" },
        { "update.advanced_title.text", "Automatic Update Advanced Settings" },
        { "update.advanced_title1.text", "Select how often and when you want the scan to occur." },
        { "update.advanced_title2.text", "Frequency" },
        { "update.advanced_title3.text", "When" },
        { "update.advanced_desc1.text", "Perform scan every day at {0}" },
        { "update.advanced_desc2.text", "Perform scan every {0} at {1}" },
        { "update.advanced_desc3.text", "Perform scan on day {0} of every month at {1}" },
        { "update.check_daily.text", "Daily" },
        { "update.check_weekly.text", "Weekly" },
        { "update.check_monthly.text", "Monthly" },
        { "update.check_date.text", "Day:" },
        { "update.check_day.text", "Every:" },
        { "update.check_time.text", "At:" },
        { "update.lastrun.text", "Java Update was last run at {0} on {1}." },
        { "update.desc_autooff.text", "Click the \"Update Now\" button below to check for updates. An icon will appear in the system tray if an update is available. Move the cursor over the icon to see the status of the update." },
        { "update.desc_check_daily.text", "Every day at {0}, Java Update will check for updates. " },
        { "update.desc_check_weekly.text", "Every {0} at {1}, Java Update will check for updates. " },
        { "update.desc_check_monthly.text", "On the day {0} of each month at {1}, Java Update will check for updates. " },
        { "update.desc_systrayicon.text", "An icon will appear in the system tray if an update is available. Move the cursor over the icon to see the status of the update. " },
        { "update.desc_notify_install.text", "You will be notified before the update is installed." },
        { "update.desc_notify_download.text", "You will be notified before the update is downloaded and before it is installed." },
	{ "update.launchbrowser.error.text", "Unable to launch Java Update checker.  To obtain the latest Java Update, please go to http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Error - Update" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "Delete Files..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_D" },
        { "cache.settings.dialog.view_jws_btn", "View Applications..." },
        { "cache.settings.dialog.view_jws_btn.mnemonic", "VK_V" },
        { "cache.settings.dialog.view_jpi_btn", "View Applets..." },
        { "cache.settings.dialog.view_jpi_btn.mnemonic", "VK_A" },
        { "cache.settings.dialog.chooser_title", "Temporary Files Location" },
        { "cache.settings.dialog.select", "Select" },
        { "cache.settings.dialog.select_tooltip", "Use selected location" },
        { "cache.settings.dialog.select_mnemonic", "S" },
        { "cache.settings.dialog.title", "Temporary Files Settings" },
        { "cache.settings.dialog.cache_location", "Location:" },
        { "cache.settings.dialog.change_btn", "Change..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_H" },
        { "cache.settings.dialog.disk_space", "Amount of disk space to use:" },
        { "cache.settings.dialog.unlimited_btn", "Unlimited" },
        { "cache.settings.dialog.max_btn", "Maximum" },
        { "cache.settings.dialog.compression", "Jar compression:" },
        { "cache.settings.dialog.none", "None" },
        { "cache.settings.dialog.high", "High" },

	// JNLP File/MIME association dialog strings:
	{ "javaws.association.dialog.title", "JNLP File/MIME Association" },
	{ "javaws.association.dialog.exist.command", "already exists with:\n{0}"},
	{ "javaws.association.dialog.exist", "already exists."  },
	{ "javaws.association.dialog.askReplace", "\nDo you really want to use {0} to handle it instead?"},
	{ "javaws.association.dialog.ext", "File extensions: {0}" },
        { "javaws.association.dialog.mime", "MIME type: {0}" },
        { "javaws.association.dialog.ask", "Do you want to use {0} to handle:" },
        { "javaws.association.dialog.existAsk", "WARNING! Association with:"},

        // Advanced panel strings:
        { "deployment.console.startup.mode", "Java console" },
        { "deployment.console.startup.mode.SHOW", "Show console" },
        { "deployment.console.startup.mode.SHOW.tooltip", "<html>" +
                                                          "Start Java Console maximized" +
                                                          "</html>" },
        { "deployment.console.startup.mode.HIDE", "Hide console" },
        { "deployment.console.startup.mode.HIDE.tooltip", "<html>" +
                                                          "Start Java Console minimized" +
                                                          "</html>" },
        { "deployment.console.startup.mode.DISABLE", "Do not start console" },
        { "deployment.console.startup.mode.DISABLE.tooltip", "<html>" +
                                                             "Java Console will not be started" +
                                                             "</html>" },
        { "deployment.trace", "Enable tracing" },
        { "deployment.trace.tooltip", "<html>" +
                                      "Create trace file for debugging" +
                                      "<br>purposes" +
                                      "</html>" },
        { "deployment.log", "Enable logging" },
        { "deployment.log.tooltip", "<html>" +
                                    "Create log file to capture" +
                                    "<br>errors" +
                                    "</html>" },
        { "deployment.control.panel.log", "Logging in control panel" },
        { "deployment.javapi.lifecycle.exception", "Show applet lifecycle exceptions" },
        { "deployment.javapi.lifecycle.exception.tooltip", "<html>" +
                                          "Show Dialog with Exceptions when"+
                                          "<br>errors occur during applet loading"+
                                          "<html>" },
        { "deployment.browser.vm.iexplorer", "Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
                                                     "Use Sun Java with APPLET tag" +
                                                     "<br>in Internet Explorer browser" +
                                                     "</html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla and Netscape" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
                                                   "Use Sun Java with APPLET tag in" +
                                                   "<br>Mozilla or Netscape browser(s)" +
                                                   "</html>" },
        { "deployment.console.debugging", "Debugging" },
	{ "deployment.browsers.applet.tag", "<APPLET> tag support" },
        { "deployment.javaws.shortcut", "Shortcut Creation" },
        { "deployment.javaws.shortcut.ALWAYS", "Always allow" },
        { "deployment.javaws.shortcut.ALWAYS.tooltip", "<html>" +
                                                       "Always create shortcuts" +
                                                       "</html>" },
        { "deployment.javaws.shortcut.NEVER" , "Never allow" },
        { "deployment.javaws.shortcut.NEVER.tooltip", "<html>" +
                                                      "Do not create shortcuts" +
                                                      "</html>" },
        { "deployment.javaws.shortcut.ASK_USER", "Prompt user" },
        { "deployment.javaws.shortcut.ASK_USER.tooltip", "<html>" +
                                                         "Ask user if shortcut should" +
                                                         "<br>be created" +
                                                         "</html>" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED", "Always allow if hinted" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED.tooltip", "<html>" +
                                                     "Always create shortcuts if" +
                                                     "<br>JNLP application requested" +
                                                     "</html>" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED", "Prompt user if hinted" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED.tooltip", "<html>" +
                                                     "Ask user if shortcut should" +
                                                     "<br>be created if requested by" +
                                                     "<br>JNLP application" +
                                                     "</html>" },
	{ "deployment.javaws.associations.NEVER", "Never allow" },
        { "deployment.javaws.associations.NEVER.tooltip", "<html>" +
                                                  "Never create File extension/MIME" +
                                                  "<br>association" +
                                                  "</html>" },
        { "deployment.javaws.associations.ASK_USER", "Prompt user" },
        { "deployment.javaws.associations.ASK_USER.tooltip", "<html>" +
                                                  "Ask user before creating File" +
                                                  "<br>extension/MIME association" +
                                                  "</html>" },
        { "deployment.javaws.associations.REPLACE_ASK", "Prompt user to replace" },
        { "deployment.javaws.associations.REPLACE_ASK.tooltip", "<html>" +
                                                  "Ask user only if replacing" +
                                                  "<br>existing File extension/MIME" +
                                                  "<br>association" +
                                                  "</html>" },
        { "deployment.javaws.associations.NEW_ONLY", "Allow if association is new" },
        { "deployment.javaws.associations.NEW_ONLY.tooltip", "<html>" +
                                                  "Always create only new File" +
                                                  "<br>extension/MIME association" +
                                                  "</html>" },
        { "deployment.javaws.associations", "JNLP File/MIME Association" },
        { "deployment.security.settings", "Security" },
        { "deployment.security.askgrantdialog.show", "Allow user to grant permissions to signed content" },
        { "deployment.security.askgrantdialog.notinca", "Allow user to grant permissions to content from an untrusted authority" },

	{ "deployment.security.browser.keystore.use", "Use certificates and keys in browser keystore" },
	{ "deployment.security.notinca.warning", "Warn if certificate authority can not be verified" },
        { "deployment.security.expired.warning", "Warn if certificate is expired or not yet valid" },
        { "deployment.security.jsse.hostmismatch.warning", "Warn if site certificate does not match hostname" },
        { "deployment.security.sandbox.awtwarningwindow", "Show sandbox warning banner" },
        { "deployment.security.sandbox.jnlp.enhanced", "Allow user to accept JNLP security requests" },
        { "deploy.advanced.browse.title", "Choose file to launch default browser" },
        { "deploy.advanced.browse.select", "Select" },
        { "deploy.advanced.browse.select_tooltip", "Use selected file to launch browser" },
        { "deploy.advanced.browse.select_mnemonic", "S" },
        { "deploy.advanced.browse.browse_btn", "Browse..." },
        { "deploy.advanced.browse.browse_btn.mnemonic", "VK_B" },
        { "deployment.browser.default", "Command to launch default browser" },
        { "deployment.misc.label", "Miscellaneous" },
        { "deployment.system.tray.icon", "Place Java icon in system tray" },
        { "deployment.system.tray.icon.tooltip", "<html>" +
                                                 "Select this option to show Java" +
                                                 "<br>cup icon in the system tray when" +
                                                 "<br>Java is running in the browser" +
                                                 "</html>" },

        //PluginJresDialog strings:
        { "jpi.jres.dialog.title", "Java Runtime Settings" },
        { "jpi.jres.dialog.border", " Java Runtime Versions " },
        { "jpi.jres.dialog.column1", "Product Name" },
        { "jpi.jres.dialog.column2", "Version" },
        { "jpi.jres.dialog.column3", "Location" },
        { "jpi.jres.dialog.column4", "Java Runtime Parameters" },
        { "jpi.jdk.string", "JDK" },
        { "jpi.jre.string", "JRE" },
        { "jpi.jres.dialog.product.tooltip", "Choose JRE or JDK for product name" },

        // AboutDialog strings:
        { "about.dialog.title", "About Java" },

        // JavaPanel strings:
        { "java.panel.plugin.border", " Java Applet Runtime Settings " }, 
        { "java.panel.plugin.text", "Runtime settings are used when an applet is executed in the browser." },
        { "java.panel.jpi_view_btn", "View..." },
        { "java.panel.javaws_view_btn", "View..." },
        { "java.panel.jpi_view_btn.mnemonic", "VK_V" },
        { "java.panel.javaws_view_btn.mnemonic", "VK_I" },
        { "java.panel.javaws.border", " Java Application Runtime Settings "},
        { "java.panel.javaws.text", "Runtime settings are used when a Java application or applet is launched using the Java Network Launching Protocol (JNLP)." },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "<html><b>Newer Java Runtime exists</b></html>Internet Explorer already has a newer version of Java Runtime. Would you like to replace it?\n" },
        { "browser.settings.success.caption", "Success - Browser" },
        { "browser.settings.success.text", "<html><b>Browser Settings Changed</b></html>Changes will be in effect after restart of browser(s).\n" },
        { "browser.settings.fail.caption", "Warning - Browser" },
        { "browser.settings.fail.moz.text", "<html><b>Unable to change Browser Settings</b></html>"
                                        + "Please check that Mozilla or Netscape is properly installed on the system and/or "
                                        + "that you have "
                                        + "sufficient permissions to change system settings.\n" },
        { "browser.settings.fail.ie.text", "<html><b>Unable to Change Browser Settings</b></html>Please check if you have "
					+ "sufficient permissions to change system settings.\n" },


        // Tool tip strings.
        { "cpl.ok_btn.tooltip", "<html>" +
                                "Close the Java Control Panel and" +
                                "<br>save any changes you have made" +
                                "</html>" },
        { "cpl.apply_btn.tooltip",  "<html>" +
                                    "Save all changes you have made" +
                                    "<br>without closing the Java Control Panel" +
                                    "</html>" },
        { "cpl.cancel_btn.tooltip", "<html>" +
                                    "Close the Java Control Panel" +
                                    "<br>without saving any changes" +
                                    "</html>" },

        {"network.settings.btn.tooltip", "<html>"+
                                         "Modify Internet connection settings" +
                                         "</html>"},

        {"temp.files.settings.btn.tooltip", "<html>"+
                                            "Modify settings for temporary files" +
                                            "</html>"},

        {"temp.files.delete.btn.tooltip", "<html>" +  // body bgcolor=\"#FFFFCC\">"+
                                          "Delete temporary Java files" +
                                          "</html>"},

        {"delete.files.dlg.applets.tooltip", "<html>" +
                                          "Check this option to delete all temporary" +
                                          "<br>files created by Java applets" +
                                          "</html>" },

        {"delete.files.dlg.applications.tooltip", "<html>" +
                                          "Check this option to delete all temporary" +
                                          "<br>files created by Java Web Start" +
                                          "<br>applications" +
                                          "</html>" },

        {"delete.files.dlg.other.tooltip", "<html>" +
                                          "Check this option to delete all other" +
                                          "<br>temporary files created by Java" +
                                          "</html>" },

        {"delete.files.dlg.temp_files.tooltip", "<html>" +
                                          "Java applications may store some temporary" +
                                          "<br>files on your computer.  It is safe to" +
                                          "<br>delete these files." +
                                          "<br>" +
                                          "<p>Some Java applications might take longer" +
                                          "<br>to start up the first time you run them" +
                                          "<br>after deleting the temporary files." +
                                          "</html>" },

        {"cache.settings.dialog.view_jws_btn.tooltip", "<html>" +
                                          "View temporary files created by" +
                                          "<br>Java Web Start applications" +
                                          "</html>" },

        {"cache.settings.dialog.view_jpi_btn.tooltip", "<html>" +
                                          "View temporary files created by" +
                                          "<br>Java Applets" +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "Specify the directory where temporary" +
                                          "<br>files are stored"+
                                          "</html>" },

        {"cache.settings.dialog.unlimited_btn.tooltip", "<html>" +
                                          "Do not limit the amount of disk space" +
                                          "<br>to use for storing temporary files" +
                                          "</html>" },

        {"cache.settings.dialog.max_btn.tooltip", "<html>" +
                                          "Specify the maximum amount of disk space" +
                                          "<br>to use for storing temporary files." +
                                          "</html>" },

        {"cache.settings.dialog.compression.tooltip", "<html>" +
                                          "Specify the amount of compression to use" +
                                          "<br>for JAR files stored by Java programs" +
                                          "<br>in your temporary files directory" +
                                          "<br>" +
                                          "<p>With \"None\", your Java programs start up" +
                                          "<br>faster, but the amount of disk space" +
                                          "<br>required to store them increases.  Higher" +
                                          "<br>values lower disk space requirements, while" +
                                          "<br>slightly increasing start up times." +
                                          "</html>" },

        { "common.ok_btn.tooltip",  "<html>" +
                                    "Save changes and close the dialog" +
                                    "</html>" },

        { "common.cancel_btn.tooltip",  "<html>" +
                                        "Cancel changes and close the dialog" +
                                        "</html>"},

	{ "network.settings.advanced_btn.tooltip",  "<html>" +
                                                    "View and modify advanced proxy settings"+
                                                    "</html>"},

        {"security.certs_btn.tooltip", "<html>" +
                                       "Import, export, or remove certificates" +
                                       "</html>" },

        { "cert.import_btn.tooltip", "<html>" +
                                     "Import a certificate that is not already" +
                                     "<br>on the list" +
				     "</html>"},

        { "cert.export_btn.tooltip",    "<html>" +
                                        "Export the selected certificate" +
                                        "</html>"},

        { "cert.remove_btn.tooltip",  "<html>" +
                                      "Remove the selected certificate"+
                                      "<br>from the list" +
        		      "</html>"},

        { "cert.details_btn.tooltip", "<html>" +
		      "View detailed information about" +
                                      "<br>the selected certificate" +
		      "</html>"},

        { "java.panel.jpi_view_btn.tooltip",  "<html>" +
                                              "Modify Java runtime settings for applets"+
                                              "</html>" },

        { "java.panel.javaws_view_btn.tooltip",   "<html>" +
                                                  "Modify Java runtime settings for applications" +
                                                  "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "View information about this version" +
                                            "<br>of the J2SE Runtime Environment" +
                                            "</html>" },

        { "update.notify_combo.tooltip",  "<html>" +
                                          "Select when you would like " +
                                          "<br>to be notified about new Java" +
                                          "<br>updates" +
                                          "</html>" },

        { "update.advanced_btn.tooltip",  "<html>" +
                                          "Modify the scheduling policy" +
					  "<br>for automatic update" +
                                          "</html>" },

        { "update.now_btn.tooltip",    "<html>" +
                                      "Launch Java Update to check for" +
                                      "<br>the latest available Java updates" +
                                      "</html>" },

        { "vm.options.add_btn.tooltip",   "<html>" +
                                          "Add a new JRE to the list" +
                                          "</html>" },

        { "vm.options.remove_btn.tooltip", "<html>" +
                                           "Remove the selected entry from the list" +
                                           "</html>" },

        { "vm.optios.ok_btn.tooltip",    "<html>" +
		         "Save all entries containing" +
		         "<br>product name, version and" +
		         "<br>location information" +
		         "</html>" },

        { "jnlp.jre.find_btn.tooltip",  "<html>" +
		        "Search for an installed Java Runtime" +
                                        "<br>Environment" +
		        "</html>" },

        { "jnlp.jre.add_btn.tooltip",   "<html>" +
                                        "Add a new entry to the list" +
		                        "</html>" },

        { "jnlp.jre.remove_btn.tooltip",  "<html>" +
                                          "Remove the selected entry from the" +
                                          "<br>user list" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "Allow JRE Download" },
        { "download.jre.prompt.text1", "The application \"{0}\" requires a version of "
                                     + "the JRE (version {1}), which "
                                     + "is not currently installed on your system." },
        { "download.jre.prompt.text2", "Would you like Java Web Start to automatically "
                                     + "download and install this JRE for you?" },
        { "download.jre.prompt.okButton", "Download" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "download.jre.prompt.cancelButton", "Cancel" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "autoupdatecheck.buttonYes", "Yes" },
	{ "autoupdatecheck.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "autoupdatecheck.buttonNo", "No" },
	{ "autoupdatecheck.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "autoupdatecheck.buttonAskLater", "Ask Me Later" },
	{ "autoupdatecheck.buttonAskLater.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "autoupdatecheck.caption", "Check for Updates Automatically" },
	{ "autoupdatecheck.message", "Java Update can automatically update your Java software as new versions become available. Would you like to enable this service?" },
    };
}

