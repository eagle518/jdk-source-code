/*
 *  @(#)Deployment.java	1.250 10/03/29
 *
 *
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;


/**
 * Deployment strings.
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

	{ "https.dialog.caption", "Warning - Hostname Mismatch" },  
	{ "https.dialog.masthead", "The name of the site " +
                  "does not match the name on the certificate.  " +
                  "Do you want to continue?" },
        { "security.dialog.hostname.mismatch.sub", "The name of the site, " +
                  "\"{0}\", does not match the name on the " +
                  "certificate, \"{1}\"." },
        { "security.dialog.hostname.mismatch.moreinfo", "The application is " +
                  "being downloaded from a site other than the one specified " +
                  "in the security certificate." +
                  // Translators, please note a newline and 4 spaces indentation
                  "\n     Downloading from \"{0}\" " + 
                  "\n     Expecting \"{1}\" "},
	{ "https.dialog.unknown.host", "Unknown host" },

	{ "security.dialog.extension.title", "Install Java Extension" },
	{ "security.dialog.extension.caption", "Do you want to install the following software?" },
	{ "security.dialog.extension.buttonInstall", "Install" },
	{ "security.dialog.extension.sub", "The application " +
		"requires this software to continue. Note that installing this software " +
		"has risk, click on the More Information link for details." }, 

	{ "security.dialog.extension.warning", "When you are installing Java extensions, " +
		"you should be aware of the following items:" + 
		"\n\nThis extension contains software that will have unrestricted " +
		"access to your computer." +
		"\n\n\"{0}\" asserts " + 
		"that this extension software is safe. You should install this " +
		"extension only if you trust \"{1}\"." +
		"\n\nThe digital signature from \"{2}\" has been verified." },

	{ "security.dialog.caption", "Warning - Security" },
                
	{ "security.dialog.unknown.issuer", "Unknown issuer" },
	{ "security.dialog.unknown.subject", "Unknown subject" },
	{ "security.dialog.certShowName", "{0} ({1})" },
        { "security.dialog.verified.valid.signed.caption", "The application's " +
                  "digital signature has been verified.  Do you want to run " +
                  "the application?" },
        { "security.dialog.verified.valid.https.caption", "The web site's " +
                  "certificate has been verified.  Do you want to continue?"},
        { "security.dialog.verified.valid.signed.sub", "The digital " +
                  "signature has been validated by a trusted source." }, 
        { "security.dialog.verified.valid.https.sub", "The certificate " +
                  "has been validated by a trusted source."},
        { "security.dialog.signed.moreinfo.generic",  "This application " +
                  "will be run without the security restrictions normally " +
                  "provided by Java." },
        { "security.dialog.verified.https.publisher", "The certificate was " +
                  "issued by a trusted source." },
        { "security.dialog.verified.signed.publisher", "The digital " +
                  "signature was generated with a trusted certificate."},
	{ "security.dialog.verified.valid.warning", "Caution: \"{0}\" asserts " +
                  "that this application is safe. You should only run this " +
                  "application if you trust \"{1}\" to make that assertion." },
        { "security.dialog.timestamp", "The digital signature was valid at " +
                  "the time of signing on {0}." },
        { "security.dialog.unverified.signed.caption", "The application's " +
                  "digital signature cannot be verified.  Do you want to run the " +
                  "application?" },
        { "security.dialog.unverified.https.caption", "The web site's " +
                  "certificate cannot be verified.  Do you want to continue?" },
        { "security.dialog.unverified.signed.sub", "The digital signature " +
                  "cannot be verified by a trusted source.  Only run if " +
                  "you trust the origin of the application." },
        { "security.dialog.jnlpunsigned.sub", "Part of the application is missing a " +
                  "digital signature.  Only run if " + "you trust the origin of the application." },
        { "security.dialog.jnlpunsigned.more", "Although the application has a digital " +
                  "signature, the application's associated file(JNLP) does not " + 
		  "have one.  A digital signature ensures that a file is from " +
		  "the vendor and that it has not been altered." },
        { "security.dialog.unverified.https.sub", "The certificate " +
                  "cannot be verified by a trusted source." },
        { "security.dialog.unverified.signed.publisher", "The digital signature " +
                  "was generated with an untrusted certificate."},
        { "security.dialog.unverified.https.publisher", "The certificate was " +
                  "issued by a source that is not trusted." },                          
        { "security.dialog.invalid.time.signed.caption", "The application's " +
                  "digital signature has an error.  Do you want to run the " +
                  "application?" },
        { "security.dialog.invalid.time.https.caption", "The web site's " +
                  "certificate has an error.  Do you want to continue?" },
        { "security.dialog.expired.signed.sub", "The digital signature was " +
                  "generated with a trusted certificate but has expired." },
        { "security.dialog.expired.https.sub", "The certificate was " +
                  "issued by a trusted source but has expired." },   
        { "security.dialog.notyet.signed.sub", "The digital signature was " +
                  "generated with a trusted certificate but is not yet valid."},                          
        { "security.dialog.notyet.https.sub", "The certificate was " +
                  "issued by a trusted source but is not yet valid."},   
        { "security.dialog.expired.signed.time", "The digital signature has " +
                  "expired." },
        { "security.dialog.expired.https.time", "The certificate has expired."},
        { "security.dialog.notyetvalid.signed.time", "The digital signature is " +
                  "not yet valid." },
        { "security.dialog.notyetvalid.https.time", "The certificate is not " +
                  "yet valid." },
                          
        { "security.dialog.exception.message", "No certificate validation messages."},                
        { "security.dialog.ocsp.datetime.msg", "This warning can occur if the date and time " +
		"on your system is not set correctly. If it is not currently {0}, then adjust " +
		"the date and time. You should also confirm that the time zone setting is " +
		"correct. Load the application again after you make adjustments."},
                
        { "security.dialog.always", "Always trust content from this publisher." },
        { "security.dialog.always.mnemonic", "VK_A" },
        { "security.dialog.signed.buttonContinue", "Run" },
        { "security.dialog.signed.buttonCancel", "Cancel" },
        { "security.dialog.https.buttonContinue", "Yes" },
        { "security.dialog.https.buttonCancel", "No" },
                
        { "security.dialog.mixcode.title", "Warning - Security" },
        { "security.dialog.mixcode.header", "Java has discovered application components " +
                        "that could indicate a security concern." },
        { "security.dialog.mixcode.question", "Block potentially unsafe components " +
                        "from being run? (recommended)" },
        { "security.dialog.mixcode.alert", "The application contains both signed and " +
                        "unsigned code. \nContact the application vendor to ensure that " + 
                        "it has not been tampered with." },

        { "security.dialog.mixcode.info1", "This application has been found to contain " +
                        "a potentially unsafe combination of both signed and unsigned " + 
                        "components(code and/or resources)." + 
                        "\n\nThis situation may indicate a security risk unless it was " +
                        "intended by the application vendor(unusual)." },
        { "security.dialog.mixcode.info2", "Blocking potentially unsafe components from " +
                        "running (by clicking the Yes button) will protect the data on " +
                        "your computer, but may cause the application to fail." +
                        "\n\nThis action is recommended if you are unfamiliar with the " +
                        "application or the website from which it was accessed." },
        { "security.dialog.mixcode.info3", "Allowing potentially unsafe components to run " +
                        "(by clicking the No button) could compromise data on your computer." +
                        "\n\nTo reduce risk, Java will run trusted components where available."},

        { "security.dialog.mixcode.buttonYes", "Yes" },
        { "security.dialog.mixcode.buttonNo", "No" },

        { "security.more.info.title", "More Information" },
        { "security.more.info.details", "Certificate Details..." },
        { "security.more.info.details.acceleratorKey", new Integer(KeyEvent.VK_C)},
                
        { "password.dialog.title", "Authentication Required" },
        { "pwd-masthead.png", "image/pwd-masthead.png" },
        { "password.dialog.username", "User name:" },
        { "password.dialog.username.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "password.dialog.password", "Password:" }, 
        { "password.dialog.password.acceleratorKey", new Integer(KeyEvent.VK_P)},
        { "password.dialog.domain", "Domain:" },
        { "password.dialog.domain.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "password.dialog.save", "Save this password in your password list" },
        { "password.dialog.scheme", "Authentication scheme: {0}" },
                
        { "security.badcert.caption", "Warning - Security" },
        { "security.badcert.https.text", "Cannot validate SSL certificate.\n" +
		"The application will not be executed." },
        { "security.badcert.config.text", "Your security configuration will " +
		"not allow you to validate this certificate.\n" + 
		"The application will not be executed." },
        { "security.badcert.text", "Failed to validate certificate.\n" +
		"The application will not be executed." },

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
	{ "cert.dialog.field.md5Fingerprint", "MD5 Fingerprint" },
        { "cert.dialog.field.sha1Fingerprint", "SHA1 Fingerprint" },
	{ "cert.dialog.field", "Field" },
	{ "cert.dialog.value", "Value" },
        { "cert.dialog.close", "Close" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_C) },

	{ "clientauth.user.password.dialog.text", "Enter a password to access your personal keystore:" },
	{ "clientauth.system.password.dialog.text", "Enter a password to access the system's keystore:" },
	{ "clientauth.password.dialog.error.caption", "Error - Client Authentication Keystore" },
	{ "clientauth.password.dialog.error.text", "Keystore Access Error \n" +
	  "Keystore was tampered with, or password was incorrect." },

	{ "clientauth.certlist.dialog.caption", "Request Authentication" },
	{ "clientauth.certlist.dialog.text", "Identification required.  " +
                  "Please select certificate to be used for authentication." },
	{ "clientauth.certlist.dialog.javaKS", "{0} (from personal keystore)" },
	{ "clientauth.certlist.dialog.browserKS", "{0} (from browser keystore)" },
	{ "clientauth.checkTLSClient.checkKeyUsage", "KeyUsage does not allow digital signatures" },
	{ "clientauth.checkTLSClient.checkEKU", "Extended key usage does not " + 
		  "permit use for TLS client authentication" },
	{ "clientauth.checkTLSClient.failed", "Certificate {0} cannot be used for client authentication." },
	{ "clientauth.readFromCache.failed", "Can't read client certificate from cache, exception has been thrown." },
        { "clientauth.readFromCache.success", "Reading client certificate {0} from cache." },

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
	{ "dialogfactory.general_error", "General Exception" },
	{ "dialogfactory.net_error", "Networking Exception" },
	{ "dialogfactory.security_error", "Security Exception" },
	{ "dialogfactory.ext_error", "Optional Package Exception" },
	{ "dialogfactory.user.selected", "User selected: {0}" },
	{ "dialogfactory.user.typed", "User typed: {0}" },

	{ "deploycertstore.cert.loading", "Loading Deployment certificates from {0}" },
	{ "deploycertstore.cert.loaded", "Loaded Deployment certificates from {0}" },
	{ "deploycertstore.cert.saving", "Saving Deployment certificates in {0}" },
	{ "deploycertstore.cert.saved", "Saved Deployment certificates in {0}" },
	{ "deploycertstore.cert.adding", "Adding certificate in Deployment permanent certificate store" },
	{ "deploycertstore.cert.added", "Added certificate in Deployment permanent certificate store as alias {0}" },
	{ "deploycertstore.cert.removing", "Removing certificate in Deployment permanent certificate store" },
	{ "deploycertstore.cert.removed", "Removed certificate in Deployment permanent certificate store as alias {0}" },
	{ "deploycertstore.cert.instore", "Checking if certificate is in Deployment permanent certificate store" },
	{ "deploycertstore.cert.canverify", "Check if certificate can be verified using certificates in Deployment permanent certificate store" },
	{ "deploycertstore.cert.getcertificates", "Obtain certificate collection in Deployment permanent certificate store" },
	{ "deploycertstore.password.dialog.text", "Enter a password to access your trusted signer keystore:" },

	{ "httpscertstore.cert.loading", "Loading Deployment SSL certificates from {0}" },
	{ "httpscertstore.cert.loaded", "Loaded Deployment SSL certificates from {0}" },
	{ "httpscertstore.cert.saving", "Saving Deployment SSL certificates in {0}" },
	{ "httpscertstore.cert.saved", "Saved Deployment SSL certificates in {0}" },
	{ "httpscertstore.cert.adding", "Adding SSL certificate in Deployment permanent certificate store" },
	{ "httpscertstore.cert.added", "Added SSL certificate in Deployment permanent certificate store as alias {0}" },
	{ "httpscertstore.cert.removing", "Removing SSL certificate in Deployment permanent certificate store" },
	{ "httpscertstore.cert.removed", "Removed SSL certificate in Deployment permanent certificate store as alias {0}" },
	{ "httpscertstore.cert.instore", "Checking if SSL certificate is in Deployment permanent certificate store" },
	{ "httpscertstore.cert.canverify", "Check if SSL certificate can be verified using certificates in Deployment permanent certificate store" },
	{ "httpscertstore.cert.getcertificates", "Obtain SSL certificate collection in Deployment permanent certificate store" },
	{ "httpscertstore.password.dialog.text", "Enter a password to access your trusted SSL keystore:" },

	{ "rootcertstore.cert.loading", "Loading Root CA certificates from {0}" },
	{ "rootcertstore.cert.loaded", "Loaded Root CA certificates from {0}" },
	{ "rootcertstore.cert.noload", "Root CA certificates file not found: {0}" },
	{ "rootcertstore.cert.saving", "Saving Root CA certificates in {0}" },
	{ "rootcertstore.cert.saved", "Saved Root CA certificates in {0}" },
	{ "rootcertstore.cert.adding", "Adding certificate in Root CA certificate store" },
	{ "rootcertstore.cert.added", "Added certificate in Root CA certificate store as alias {0}" },
	{ "rootcertstore.cert.removing", "Removing certificate in Root CA certificate store" },
	{ "rootcertstore.cert.removed", "Removed certificate in Root CA certificate store as alias {0}" },
	{ "rootcertstore.cert.instore", "Checking if certificate is in Root CA certificate store" },
	{ "rootcertstore.cert.canverify", "Check if certificate can be verified using certificates in Root CA certificate store" },
	{ "rootcertstore.cert.getcertificates", "Obtain certificate collection in Root CA certificate store" },
	{ "rootcertstore.cert.verify.ok", "Certificate has been verified with Root CA certificates successfully" },
	{ "rootcertstore.cert.verify.fail", "Certificate has failed the verification with the Root CA certificates" },
	{ "rootcertstore.cert.tobeverified", "Certificate to be verified:\n{0}" },
	{ "rootcertstore.cert.tobecompared", "Comparing certificate against Root CA certificate:\n{0}" },
	{ "rootcertstore.password.dialog.text", "Enter a password to access your signer's CA keystore:" },

	{ "roothttpscertstore.cert.loading", "Loading SSL Root CA certificates from {0}" },
	{ "roothttpscertstore.cert.loaded", "Loaded SSL Root CA certificates from {0}" },
	{ "roothttpscertstore.cert.noload", "SSL Root CA certificates file not found: {0}" },
	{ "roothttpscertstore.cert.saving", "Saving SSL Root CA certificates in {0}" },
	{ "roothttpscertstore.cert.saved", "Saved SSL Root CA certificates in {0}" },
	{ "roothttpscertstore.cert.adding", "Adding certificate in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.added", "Added certificate in SSL Root CA certificate store as alias {0}" },
	{ "roothttpscertstore.cert.removing", "Removing certificate in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.removed", "Removed certificate in SSL Root CA certificate store as alias {0}" },
	{ "roothttpscertstore.cert.instore", "Checking if certificate is in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.canverify", "Check if certificate can be verified using certificates in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.getcertificates", "Obtain certificate collection in SSL Root CA certificate store" },
	{ "roothttpscertstore.cert.verify.ok", "Certificate has been verified with SSL Root CA certificates successfully" },
	{ "roothttpscertstore.cert.verify.fail", "Certificate has failed the verification with the SSL Root CA certificates" },
	{ "roothttpscertstore.cert.tobeverified", "Certificate to be verified:\n{0}" },
	{ "roothttpscertstore.cert.tobecompared", "Comparing certificate against SSL Root CA certificate:\n{0}" },
	{ "roothttpscertstore.password.dialog.text", "Enter a password to access your SSL's CA keystore:" },

	{ "sessioncertstore.cert.loading", "Loading certificates from Deployment session certificate store" },
	{ "sessioncertstore.cert.loaded", "Loaded certificates from Deployment session certificate store" },
	{ "sessioncertstore.cert.saving", "Saving certificates in Deployment session certificate store" },
	{ "sessioncertstore.cert.saved", "Saved certificates in Deployment session certificate store" },
	{ "sessioncertstore.cert.adding", "Adding certificate in Deployment session certificate store" },
	{ "sessioncertstore.cert.added", "Added certificate in Deployment session certificate store" },
	{ "sessioncertstore.cert.removing", "Removing certificate in Deployment session certificate store" },
	{ "sessioncertstore.cert.removed", "Removed certificate in Deployment session certificate store" },
	{ "sessioncertstore.cert.instore", "Checking if certificate is in Deployment session certificate store" },
	{ "sessioncertstore.cert.canverify", "Check if certificate can be verified using certificates in Deployment session certificate store" },
	{ "sessioncertstore.cert.getcertificates", "Obtain certificate collection in Deployment session certificate store" },

	{ "deniedcertstore.cert.adding", "Adding certificate in Deployment denied certificate store" },
        { "deniedcertstore.cert.added", "Added certificate in Deployment denied certificate store" },
        { "deniedcertstore.cert.removing", "Removing certificate in Deployment denied certificate store" },
        { "deniedcertstore.cert.removed", "Removed certificate in Deployment denied certificate store" },
        { "deniedcertstore.cert.instore", "Checking if certificate is in Deployment denied certificate store" },
	{ "deniedcertstore.cert.getcertificates", "Obtain certificate collection in Deployment denied certificate store" },

	{ "iexplorer.cert.loading", "Loading certificates from Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.loaded", "Loaded certificates from Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.instore", "Checking if certificate is in Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.canverify", "Check if certificate can be verified using certificates in Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.getcertificates", "Obtain certificate collection in Internet Explorer {0} certificate store" },
	{ "iexplorer.cert.verify.ok", "Certificate has been verified with Internet Explorer {0} certificates successfully" },
	{ "iexplorer.cert.verify.fail", "Certificate has failed the verification with the Internet Explorer {0} certificates" },
	{ "iexplorer.cert.tobeverified", "Certificate to be verified:\n{0}" },
	{ "iexplorer.cert.tobecompared", "Comparing certificate against Internet Explorer {0} certificate:\n{1}" },

	{ "mozilla.cert.loading", "Loading certificates from Mozilla {0} certificate store" },
        { "mozilla.cert.loaded", "Loaded certificates from Mozilla {0} certificate store" },
        { "mozilla.cert.instore", "Checking if certificate is in Mozilla {0} certificate store" },
        { "mozilla.cert.canverify", "Check if certificate can be verified using certificates in Mozilla {0} certificate store" },
        { "mozilla.cert.getcertificates", "Obtain certificate collection in Mozilla {0} certificate store" },
        { "mozilla.cert.verify.ok", "Certificate has been verified with Mozilla {0} certificates successfully" },
        { "mozilla.cert.verify.fail", "Certificate has failed the verification with the Mozilla {0} certificates" },
        { "mozilla.cert.tobeverified", "Certificate to be verified:\n{0}" },
        { "mozilla.cert.tobecompared", "Comparing certificate against Mozilla {0} certificate:\n{1}" },

        { "browserkeystore.jss.no", "JSS package is not found" },
        { "browserkeystore.jss.yes", "JSS package is loaded" },
        { "browserkeystore.jss.notconfig", "JSS is not configured" },
        { "browserkeystore.jss.config", "JSS is configured" },
        { "browserkeystore.mozilla.dir", "Accessing keys and certificate in Mozilla user profile: {0}" },
	{ "browserkeystore.password.dialog.text", "Enter password to access {0} in browser keystore:" },
	{ "mozillamykeystore.priv.notfound", "private key not found for cert : {0}" },
	{ "hostnameverifier.automation.ignoremismatch", "Automation: Ignore hostname mismatch" },

	{ "trustdecider.check.validate.legacy.algorithm", "Validate the certificate chain using legacy algorithm" },
	{ "trustdecider.check.validate.certpath.algorithm", "Validate the certificate chain using CertPath API" },
	{ "trustdecider.check.validate.notfound", "sun.security.validator.Validator API is not available" },
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
	{ "trustdecider.check.jurisdiction.found", "Found jurisdiction list file" },
	{ "trustdecider.check.jurisdiction.notfound", "Cannot find jurisdiction list file" },
	{ "trustdecider.check.trustextension.on", "Start checking trusted extension for this certificate" },
	{ "trustdecider.check.trustextension.off", "No need to checking trusted extension for this certificate" },
	{ "trustdecider.check.trustextension.add", "Trusted extension certificate has been added to trusted store automatically" },
	{ "trustdecider.check.trustextension.jurisdiction", "Start comparing to jurisdiction list with this certificate" },
	{ "trustdecider.check.trustextension.jurisdiction.found", "Found match certificate in jurisdiction list" },
	{ "trustdecider.check.extensioninstall.on", "Start checking revocation for extension installation for this certificate" },
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
	{ "trustdecider.check.validation.revoked", "This certificate has been revoked" },
        { "trustdecider.check.validation.crl.on", "The CRL support is enabled" },
        { "trustdecider.check.validation.crl.off", "The CRL support is disabled" },
        { "trustdecider.check.validation.crl.system.on", "Use CRL setting from system config file" },
        { "trustdecider.check.validation.crl.system.off", "Use CRL setting from certificate" },
	{ "trustdecider.check.validation.crl.notfound", "This certificate does not have CRL extension" },
        { "trustdecider.check.reset.denystore", "Reset deny session certificate store" },
        { "trustdecider.check.validation.ocsp.on", "The OCSP support is enabled" },
        { "trustdecider.check.validation.ocsp.off", "The OCSP support is disabled" },
        { "trustdecider.check.validation.ocsp.system.on", "Use OCSP setting from system config file" },
        { "trustdecider.check.validation.ocsp.system.off", "Use OCSP setting from certificate" },
	{ "trustdecider.check.validation.ocsp.notfound", "This certificate does not have AIA extension" },
	{ "trustdecider.check.revocation.succeed", "Certificate validation succeeded using OCSP/CRL" },

	{ "trustdecider.check.ocsp.ee.on", "This OCSP End Entity validation is enabled" },
	{ "trustdecider.check.ocsp.ee.off", "This OCSP End Entity validation is disabled" },
	{ "trustdecider.check.ocsp.ee.start", "Start OCSP End Entity validation check" },
	{ "trustdecider.check.ocsp.ee.bad", "OCSP End Entity validation status is bad" },
	{ "trustdecider.check.ocsp.ee.good", "OCSP End Entity validation status is good" },
	{ "trustdecider.check.ocsp.ee.responderURI.no", "No valid OCSP responder, return good status" },
	{ "trustdecider.check.ocsp.ee.return.status", "The OCSP return status is: {0}" },
	{ "trustdecider.check.ocsp.ee.responderURI.value", "The OCSP responder URI is: {0}" },
	{ "trustdecider.check.ocsp.ee.responderURI.no", "No OCSP responder URI found" },
	{ "trustdecider.check.ocsp.ee.revoked", "Certificate is revoked or revocation status is unknown" },

	{ "trustdecider.check.replacedCA.start", "Start to check whether root CA is replaced" },
	{ "trustdecider.check.replacedCA.succeed", "The root CA has been replaced" },
	{ "trustdecider.check.replacedCA.failed", "The root CA hasn't been replaced" },

	{ "downloadengine.check.blacklist.enabled", "Blacklist revocation check is enabled" },
	{ "downloadengine.check.blacklist.notexist", "Blacklist file not found or revocation check is disabled" },
	{ "downloadengine.check.blacklist.notfound", "The jar file isn't on a blacklist" },
	{ "downloadengine.check.blacklist.found", "The jar file is on a blacklist and will not be loaded" },
	{ "downloadengine.check.blacklist.notsigned", "The jar file isn't signed so the blacklist check will be skipped" },     	

	{ "downloadengine.check.trustedlibraries.enabled", "Trusted libraries list check is enabled" },
	{ "downloadengine.check.trustedlibraries.notexist", "Trusted libraries list file not found" },
	{ "downloadengine.check.trustedlibraries.notfound", "The jar file isn't on a trusted libraries list" },
	{ "downloadengine.check.trustedlibraries.found", "The jar file is on a trusted libraries list" },
	{ "downloadengine.check.trustedlibraries.notsigned", "The jar file isn't signed so the trusted libraries list check will be skipped" },     	

	{ "x509trustmgr.automation.ignoreclientcert", "Automation: Ignore untrusted client certificate" },
	{ "x509trustmgr.automation.ignoreservercert", "Automation: Ignore untrusted server certificate" },
	{ "x509trustmgr.check.validcert", "Valid certificate from HTTPS server" },
	{ "x509trustmgr.check.invalidcert", "Invalid certificate from HTTPS server" },

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
        { "net.proxy.browser.pDetectionError", "Unable to perform Auto Proxy Detection, domain name too short: {0}"},     
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
	{ "net.proxy.nsprefs.error", "Unable to Retrieve Proxy Settings. \n" +
	  "Fallback to other proxy configuration.\n" },
	{ "net.proxy.connect", "Connecting {0} with proxy={1}" },
        { "net.proxy.connectionFailure", "Connection {0} failed: removed from proxy cache" },

        // Below message is of form "Enter login details to access <realm> on <server>".
	{ "net.authenticate.text", "Enter login details to access {0} on {1}:"},
	{ "net.authenticate.unknownSite", "Unknown Site" },
        { "net.authenticate.ntlm.display.string", "Integrated Windows" },
        { "net.authenticate.basic.display.string", "Basic" },
        { "net.authenticate.digest.display.string", "Digest" },
        { "net.authenticate.unknown.display.string", "Unknown" },

	{ "net.cookie.cache", "Cookie Cache: " },
	{ "net.cookie.server", "Server {0} requesting to set-cookie with \"{1}\"" },
	{ "net.cookie.connect", "Connecting {0} with cookie \"{1}\"" },
	{ "net.cookie.ignore.setcookie", "Cookie service is unavailable - ignore \"Set-Cookie\"" },
	{ "net.cookie.noservice", "Cookie service is not available - use cache to determine \"Cookie\"" },

	{ "about.java.version", "Version {0}" },
        { "about.java.version.update", "Version {0} Update {1}" },
        { "about.java.build", "(build {0})" },
	{ "about.prompt.info", "For more information about Java technology and to explore great Java applications, visit"},
	{ "about.home.link", "http://www.java.com"},
	{ "about.option.close", "Close"},
	{ "about.copyright", "Copyright (c) @@COPYRIGHT_YEAR@@ Oracle and/or it's affiliates."},
	{ "about.legal.note", "All rights reserved. Use is subject to license terms."},
        { "sun.logo.image", "image/about-OracleLogo.png"},


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
	{ "cert.dialog.import.format.masthead", "Unrecognized file format." },
        { "cert.dialog.import.format.text",  "No certificate will be imported." },
	{ "cert.dialog.export.password.masthead", "Invalid password." },
        { "cert.dialog.export.password.text", "The password you entered " +
                  "is incorrect.  Certificate export failed." },
	{ "cert.dialog.import.file.masthead", "File does not exist." },
        { "cert.dialog.import.file.text", 
                  "The certificate will not be imported." },
	{ "cert.dialog.import.password.masthead", "Invalid password." },
        { "cert.dialog.import.password.text", "The password you entered is " +
                  "incorrect.  Certificate import failed." },
        { "cert.dialog.password.text", "Enter password to access file:" },
        { "cert.dialog.exportpassword.text", "Enter password to access private key in client authentication keystore:" },
        { "cert.dialog.savepassword.text", "Enter password to protect key file:" },
        { "cert.dialog.export.error.caption", "Error - Export Certificate" },
        { "cert.dialog.export.masthead", "Unable to export." },
        { "cert.dialog.export.text", "The certificate will not be exported." },
        { "cert.dialog.remove.masthead", "Are you sure you want to remove " +
                  "the selected certificates?" }, 
        { "cert.dialog.remove.text", "The selected certificates will be " +
                  "permanently removed." },
	{ "cert.dialog.remove.caption", "Confirmation - Remove Certificate?" },
	{ "cert.dialog.issued.to", "Issued To" },
	{ "cert.dialog.issued.by", "Issued By" },
	{ "cert.dialog.user.level", "User" },
	{ "cert.dialog.system.level", "System" },
	{ "cert.dialog.certtype", "Certificate type: "},

	{ "controlpanel.jre.platformTableColumnTitle","Platform"},
	{ "controlpanel.jre.productTableColumnTitle","Product" },
	{ "controlpanel.jre.locationTableColumnTitle","Location" },
	{ "controlpanel.jre.pathTableColumnTitle","Path" },
	{ "controlpanel.jre.vmargsTableColumnTitle", "Runtime Parameters" },
	{ "controlpanel.jre.enabledTableColumnTitle", "Enabled" },

	{ "deploy.jre.title", "Java Runtime Environment Settings" },
	{ "deploy.jre.versions", "Java Runtime Versions" },
	{ "deploy.jre.find.button", "Find" },
	{ "deploy.jre.add.button", "Add" },
	{ "deploy.jre.remove.button", "Remove" },
	{ "deploy.jre.ok.button", "OK" },
	{ "deploy.jre.cancel.button", "Cancel" },
	{ "deploy.jre.find.button.mnemonic", "VK_F" },
	{ "deploy.jre.add.button.mnemonic", "VK_A" },
	{ "deploy.jre.remove.button.mnemonic", "VK_R" },

	{ "jretable.platform.tooltip", "Java Platform Version" },
	{ "jretable.product.tooltip", "Java Product Version"},
	{ "jretable.location.tooltip", "Java Download Location"},
	{ "jretable.path.tooltip", "Path to Java Runtime"},
	{ "jretable.vmargs.tooltip", "Java Runtime Parameters For Applets"},
	{ "jretable.enable.tooltip", "Enable Java Runtime For Applets and Applications"},

	{ "find.dialog.title", "JRE Finder"},
	{ "find.title", "Java Runtime Environments"},
	{ "find.cancelButton", "Cancel"},
	{ "find.prevButton", "Previous"},
	{ "find.nextButton", "Next"},
	{ "find.finishButton", "Finish"},
	{ "find.cancelButtonMnemonic", "VK_C"},
	{ "find.prevButtonMnemonic", "VK_P"},
	{ "find.nextButtonMnemonic", "VK_X"},
	{ "find.intro", "In order to launch applications or applets, Java needs to know the locations of installed Java Runtime Environments.\n\nYou can either select a known JRE, or select a directory in the file system from which to search for JREs." },

	{ "find.searching.title", "Searching for available JREs, this may take several minutes." },
	{ "find.searching.prefix", "checking: " },
	{ "find.foundJREs.title", "The following JREs were found, click Finish to add them" },
	{ "find.noJREs.title", "Unable to locate a JRE, click Previous to select a different location to search from" },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Java(tm) Deployment Properties\n"
                        + "# DO NOT EDIT THIS FILE.  It is machine generated.\n"
                        + "# Use the Java Control Panel to edit properties." },
        { "config.unknownSubject", "Unknown Subject" },
        { "config.unknownIssuer", "Unknown Issuer" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "Malformed URL \n" +
	  "Automatic proxy configuration URL is invalid." },
        { "config.proxy.autourl.invalid.caption", "Error - Proxies" },

	// Java Web Start ApiDialog
	{ "api.clipboard.title", "Security Warning" },
	{ "api.clipboard.message.read", "The application has requested " +
	   "read-only clipboard access.  " +
	   "Do you want to allow this action?" },
        { "api.clipboard.message.write", "The application has requested " +
	   "write-only clipboard access.  " +
	   "Do you want to allow this action?" },
	{ "api.clipboard.write.always", 
	   "Always allow this application to access the clipboard." },
	{ "api.clipboard.read.always", 
	   "Always allow this application to access the clipboard." },

	{ "api.file.open.title", "Security Warning" },
	{ "api.file.open.always", 
	   "Always allow this action." },
        { "api.file.open.message", 
          "The application has requested read/write access to a file on the " +
	  "machine.  Do you want to allow this action?" },

	{ "api.file.save.title", "Security Warning" },
	{ "api.file.save.always", 
	   "Always allow this action." },
        { "api.file.save.message", 
	  "The application has requested write access to a file on the " +
	  "machine.  Do you want to allow this action?" },

        { "api.file.save.fileExistTitle", "File Exists" },
        { "api.file.save.fileExist", "{0} already exists.\n" +
	  "Do you want to replace it?" },

        { "api.persistence.title", "Security Warning" },
        { "api.persistence.accessdenied", 
	  "Access to persistent storage denied for URL {0}" },
        { "api.persistence.filesizemessage", "Maximum file length exceeded" },
        { "api.persistence.message", "This application has requested " +
	  "additional local disk space. "  +
	  "Do you want to allow this action?" },
	{ "api.persistence.detail", 
	  "The maximum size of allotted storage is {1} bytes.  The " +
	  "application has requested to increese it to {0} bytes." },

	{ "plugin.print.title", "Security Warning" },
	{ "plugin.print.message", "The applet has requested access to the " +
	   "printer.  Do you want to allow this action?" },
	{ "plugin.print.always", 
	   "Always allow this applet to access the printer." },

	{ "api.print.title", "Security Warning" },
        { "api.print.message", "The application has requested access to the " +
	   "printer.  Do you want to allow this action?" },
	{ "api.print.always", 
	   "Always allow this application to access the printer." },

	{ "api.extended.open.title", "Security Warning" },
	{ "api.extended.open.label", "Files to open:" },
	{ "api.extended.open.message",
	  "The application has requested read-write access to the listed files.  " +
	  "Do you want to allow this action?" },
	{ "api.extended.open.lable", "Files to modify:" },

	{ "api.ask.host.title", "Security Warning" },
        { "api.ask.connect", 
	  "The application has requested permission to " +
	  "establish connections to {0}.  " +
	  "Do you want to allow this action?" },
        { "api.ask.accept", 
	  "The application has requested permission to " +
	  "accept connections from {0}.  " +
	  "Do you want to allow this action?" },

	{ "update.dialog.title", "Application Update" },
	{ "update.dialog.prompt-run", "A mandatory update is available.\n" +
				"Do you want to continue?" },
	{ "update.dialog.prompt-update","An optional update is available.\n"+
				"Do you want to update the application? \n" },

        { "Launch.error.installfailed", "Installation failed" },
        { "aboutBox.closeButton", "Close" },
        { "aboutBox.versionLabel", "Version {0} (build {1})" },
        { "applet.failedToStart", "Failed to start Applet: {0}" },
        { "applet.initializing", "Initializing Applet" },
        { "applet.initializingFailed", "Failed to initialize Applet: {0}" },
        { "applet.running", "Running..." },
        { "error32.image", "image/error32.png" },
        { "error48.image", "image/error48.png" },
        { "warning32.image", "image/warning32.png" },
        { "warning48.image", "image/warning48.png" },
	{ "mixcode.image", "image/mixcode.png" },
        { "major-warning48.image", "image/major-warning48.png" },
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "Windows must restart for the changes to take effect.\n\nDo you want to restart Windows now?" },
        { "extensionInstall.rebootTitle", "Restart Windows" },
        { "install.errorInstalling", "Unexpected error trying to install Java Runtime Environment, please try again." },
        { "install.errorRestarting", "Unexpected error starting, please try again." },
	{ "integration.title", "Desktop Integration Warning" },
	{ "integration.skip.button", "Skip" },
	{ "integration.text.both", 
	    "The application can be integrated into the desktop. " +
	    "Do you want to continue?" },
	{ "integration.text.shortcut", 
	    "The application would like to create shortcuts. " +
	    "Do you want to continue?" },
	{ "integration.text.association", 
	    "The application would like to become the default for certain file types. " +
	    "Do you want to continue?" },
	{ "install.windows.both.message", "The application will add " +
		"shortcuts to the desktop and the start menu." },
	{ "install.gnome.both.message", "The application will add " +
		"shortcuts to the desktop and the applications menu." },
	{ "install.desktop.message", "The application will add " +
		"a shortcut to the desktop." },
	{ "install.windows.menu.message", "The application will add " +
		"a shortcut to the start menu." },
	{ "install.gnome.menu.message", "The application will add " +
		"a shortcut to the applications menu." },
        { "progress.title.app", "Starting application..." },
        { "progress.title.installer", "Starting installer..." },
	{ "progress.downloading", "Downloading application." },
	{ "progress.verifying", "Verifying application." },
	{ "progress.patching", "Patching application." },
	{ "progress.launching", "Launching application." },
	{ "progress.download.failed", "Download Failed." },
        { "progress.download.jre", "Requesting JRE {0}." },
        { "progress.stalled", "Download stalled" },
        { "progress.time.left.minute.second", 
              "Estimated time remaining: {0} minute, {1} second "},
        { "progress.time.left.minute.seconds", 
              "Estimated time remaining: {0} minute, {1} seconds "},
        { "progress.time.left.minutes.second", 
              "Estimated time remaining: {0} minutes, {1} second "},
        { "progress.time.left.minutes.seconds", 
              "Estimated time remaining: {0} minutes, {1} seconds "},
        { "progress.time.left.second", "Estimated time remaining: {0} second "},
        { "progress.time.left.seconds", "Estimated time remaining: {0} seconds "},

        { "progress.background.image", "image/progress.png" },
        { "launch.error.dateformat", "Please specify date with format \"MM/dd/yy hh:mm a\"." },
        { "launch.error.offline", "Cannot download resource.  System is offline." },
        { "launch.error.badfield", "The field {0} has an invalid value: {1}" },
        { "launch.error.badfield-signedjnlp", "The field {0} has an invalid value in the signed launch file: {1}" },
        { "launch.error.badfield.download.https", "Unable to Download via HTTPS" },
        { "launch.error.badfield.https", "Java 1.4+ is required for HTTPS support" },
        { "launch.error.offline.noofflineallowed", "System is offline and the application does not specify <offline-allowed/>" },
        { "launch.error.badfield.nocache", "Cache must be enabled for nativelib or installer-desc support" },
        { "launch.error.badjarfile", "Corrupted JAR file at {0}" },
        { "launch.error.badjnlversion", "Unsupported JNLP version in launch file: {0}. Only versions 6.0, 1.5, and 1.0 are supported with this version. Please contact the application vendor to report this problem." },
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
        { "launch.error.noJre", "The application requires a version of the JRE that is not installed on this computer. Java Web Start was unable to download and install the required version. This JRE must be installed manually.\n\n" },
        { "launch.error.wont.download.jre", "The application has requested a version of the JRE (version {0}) that currently is not locally installed. Java Web Start was not allowed to automatically download and install the requested version. This JRE must be installed manually." },
        { "launch.error.cant.download.jre", "The application has requested a version of the JRE (version {0}) that currently is not locally installed. Java Web Start is unable to automatically download and install the requested version. This JRE must be installed manually." },
        { "launch.error.cant.access.system.cache", "The current user does not have write access to the system cache." },
        { "launch.error.cant.access.user.cache", "The current user does not have write access to the cache." },
	{ "launch.error.disabled.system.cache", "Caching is disabled.  You cannot access the system cache." },
	{ "launch.error.disabled.user.cache", "Caching is disabled.  You cannot access the cache." },
        { "launch.error.nocache", "The {0} cache does not exist and cannot be created.  Check that the configuration is valid, and that you have permission to write to the configured cache location." },
        { "launch.error.nocache.config", "Invalid argument.  \"-system\" used when no system cache is configured. " },
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
        { "launch.error.toomanyargs", "Invalid arguments supplied: {0}" },
        { "launch.error.unsignedAccessViolation", "Unsigned application requesting unrestricted access to system" },
        { "launch.error.unsignedResource", "Unsigned resource: {0}" },
        { "launch.warning.cachedir", "Warning: Directory of system cache must be different than user cache. System cache is ignored." },
        { "launch.estimatedTimeLeft", "Estimated time left: {0,number,00}:{1,number,00}:{2,number,00}" },

	{ "launcherrordialog.error.label", "Error: " },
        { "launcherrordialog.brief.details", "Details" },
        { "launcherrordialog.brief.message", 
		"Unable to launch the application." },
        { "launcherrordialog.brief.message.applet", 
		"Unable to find specified configuration file." },
	{ "launcherrordialog.import.brief.message", 
		"Unable to import the application." },
        { "launcherrordialog.uninstall.brief.message", 
		"Unable to uninstall application(s)." },
        { "launcherrordialog.brief.ok", "Ok" },
        { "launcherrordialog.exceptionTab", "Exception" },
        { "launcherrordialog.genericerror", "Unexpected exception: {0}" },
        { "launcherrordialog.jnlpMainTab", "Main Launch File" },
        { "launcherrordialog.jnlpTab", "Launch File" },
        { "launcherrordialog.consoleTab", "Console" },
        { "launcherrordialog.wrappedExceptionTab", "Wrapped Exception" },

	{ "exception.details.label", "General Exception details:" },

        { "uninstall.failedMessage", "Unable to completely uninstall application." },
        { "uninstall.failedMessageTitle", "Uninstall" },
        { "install.alreadyInstalled", "There is already a shortcut for {0}. Would you like to create a shortcut anyway?" },
        { "install.alreadyInstalledTitle", "Create Shortcut..." },
        { "install.installFailed", "Unable to create a shortcut for {0}." },
        { "install.installFailedTitle", "Create Shortcut" },
        { "install.startMenuUninstallShortcutName", "Uninstall {0}" },
        { "install.uninstallFailed", "Unable to remove the shortcuts for {0}. Please try again." },
        { "install.uninstallFailedTitle", "Remove Shortcuts" },
	{ "error.default.title", "Application Error" },
	{ "error.default.title.applet", "Configuration Error" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "You can not run this program because your system deployment.config file states that an enterprise configuration file is mandatory, and the required: {0} is not available." },

	{ "enterprize.cfg.mandatory.applet", "You cannot view applets in your browser because a required configuration file could not be found at the specified location: {0}. You will need to restart your browser when the configuration problem is resolved." },

	// Combined Cache Viewer:
	{ "viewer.title", "Java Cache Viewer" },
	{ "viewer.view.label", "Show:" },
	{ "viewer.view.jnlp", "Applications" },
	{ "viewer.view.res", "Resources" },
	{ "viewer.view.import", "Deleted Applications" },
	{ "viewer.sys.view.jnlp", "System Applications" },
	{ "viewer.sys.view.res", "System Resources" },
	{ "viewer.size", "Cache Size: {0}" },
	{ "viewer.size.system", "System Cache Size: {0}" },

	{ "viewer.close", "Close" },
	{ "viewer.close.mnemonic", "VK_C"},
	{ "viewer.close.tooltip", "Close the Java Cache Viewer"},
	{ "viewer.help", "Help"},
	{ "viewer.help.mnemonic", "VK_H"},

        // action buttons
	{ "viewer.run.online.mi", "Run Online" },
	{ "viewer.run.online.mi.icon", "image/run-on-menu16.png" },
	{ "viewer.run.offline.mi", "Run Offline" },
	{ "viewer.run.offline.mi.icon", "image/run-off-menu16.png" },
	{ "viewer.run.online.icon", "image/run-online24.png" },
	{ "viewer.run.online.tooltip", 
			"Run the selected application online"},
	{ "viewer.run.offline.icon", "image/run-offline24.png" },
	{ "viewer.run.offline.tooltip", 
			"Run the selected application offline"},
	{ "viewer.remove.icon", "image/delete24.png" },
	{ "viewer.remove.tooltip", 
			"Remove selected items"},
	{ "viewer.remove.res.icon", "image/delete24.png" },
	{ "viewer.remove.res.tooltip", 
			"Remove selected resources"},
	{ "viewer.remove.removed.icon", "image/delete24.png" },
	{ "viewer.remove.removed.tooltip", 
		"Remove application from list of deleted applications"},
	{ "viewer.install.icon", "image/addshortcut24.png" },
	{ "viewer.install.tooltip", 
			"Install shortcuts to the selected application"},
	{ "viewer.show.icon", "image/showfile24.png" },
	{ "viewer.show.tooltip", 
			"Show the selected application or applet's JNLP file"},
	{ "viewer.info.icon", "image/showfile24.png" },
	{ "viewer.info.tooltip", 
			"Show the selected item"},
	{ "viewer.home.icon", "image/home24.png" },
	{ "viewer.home.tooltip", 
		"Show the home page of the selected item in the browser"},
	{ "viewer.import.icon", "image/installapp24.png" },
	{ "viewer.import.tooltip", 
			"Install selected items"},
	// popup menu
	{ "viewer.run.online.menuitem",  "Run Online" },
	{ "viewer.run.online.menuitem.mnemonic",    "VK_R" },

	{ "viewer.run.offline.menuitem", "Run Offline" },
	{ "viewer.run.offline.menuitem.mnemonic",    "VK_O" },

	{ "viewer.remove.menuitem",      "Delete" },
	{ "viewer.remove.menuitem.mnemonic",         "VK_D" },

	{ "viewer.install.menuitem",     "Install Shortcuts" },
	{ "viewer.install.menuitem.mnemonic",        "VK_I" },

	{ "viewer.show.menuitem",        "Show JNLP File" },
	{ "viewer.show.menuitem.mnemonic",           "VK_S" },

	{ "viewer.show.resource.menuitem",   "Show JNLP File" },
	{ "viewer.show.resource.menuitem.mnemonic",  "VK_S" },

	{ "viewer.home.menuitem",        "Go to Homepage" },
	{ "viewer.home.menuitem.mnemonic",           "VK_H" },

	{ "viewer.import.menuitem",         "Install" },
	{ "viewer.import.menuitem.mnemonic",         "VK_I" },


        { "jnlp.viewer.app.column", "Application" },
        { "jnlp.viewer.vendor.column", "Vendor" },
        { "jnlp.viewer.type.column", "Type" },
        { "jnlp.viewer.size.column", "Size" },
        { "jnlp.viewer.date.column", "Date" },
        { "jnlp.viewer.status.column", "Status" },

        { "jnlp.viewer.app.column.tooltip", 
		"Title of this application or applet" },
        { "jnlp.viewer.vendor.column.tooltip", 
		"Company information of this application or applet" },
        { "jnlp.viewer.type.column.tooltip", "Type of this item" },
        { "jnlp.viewer.size.column.tooltip", 
		"Total size of this application or applet" },
        { "jnlp.viewer.date.column.tooltip", 
		"Last execution date of this application or applet" },
        { "jnlp.viewer.status.column.tooltip", 
		"Launching mode of this application or applet" },
	{ "res.viewer.name.column", "Name"},
	{ "res.viewer.name.column.tooltip", "Name of this resource"},
	{ "res.viewer.size.column", "Size"},
	{ "res.viewer.size.column.tooltip", "Total size of this resource"},
	{ "res.viewer.modified.column", "Modified"},
	{ "res.viewer.modified.column.tooltip", 
		"Last modification date of this resource"},
	{ "res.viewer.expired.column", "Expired"},
	{ "res.viewer.expired.column.tooltip", 
		"Expiration date of this resource"},
	{ "res.viewer.version.column", "Version"},
	{ "res.viewer.version.column.tooltip", "Version of this resource"},
	{ "res.viewer.url.column", "URL"},
	{ "res.viewer.url.column.tooltip", "URL of this resource"},
	{ "del.viewer.app.column", "Title"},
	{ "del.viewer.app.column.tooltip", "Title of this deleted application"},
	{ "del.viewer.url.column", "URL"},
	{ "del.viewer.url.column.tooltip", "URL of this deleted application"},
	{ "viewer.offline.tooltip", "This {0} can be launched offline" },
	{ "viewer.online.tooltip", "This {0} can be launched online" },
	{ "viewer.onlineoffline.tooltip",
		 "This {0} can be launched online or offline" },
	{ "viewer.norun1.tooltip",
        	"This {0} can only be launched from a web browser" },
        { "viewer.norun2.tooltip", "Extensions can not be launched" },

	{ "viewer.application", "Application" },
	{ "viewer.applet", "Applet" },
	{ "viewer.extension", "Extension" },
	{ "viewer.installer", "Installer" },

	{ "viewer.show.title", "JNLP File" },

	{ "viewer.wait.remove", "Please wait while the selected \n" +
				"applications are removed." },
	{ "viewer.wait.remove.single", "Please wait while the selected \n" +
				"application is removed." },
	{ "viewer.wait.remove.title", "Cache Viewer" },
	{ "viewer.wait.import", 
		"Please wait while the selected \n" +
		"applications are re-installed." },
	{ "viewer.wait.import.single", 
		"Please wait while the selected \n" +
		"application is re-installed." },
	{ "viewer.wait.import.title", "Cache Viewer" },

	// system cache not configured warning
	{ "jnlp.systemcache.warning.title", "JNLP System Cache Warning" },
	{ "jnlp.systemcache.warning.message", "Warning: \n\n"+
	  "There is no system cache configured, \"-system\" option ignored."},


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
	{ "common.continue_btn", "Continue" },
        { "common.apply_btn", "Apply" },
        { "common.apply_btn.mnemonic", "VK_A" },
        { "common.add_btn", "Add" },
        { "common.add_btn.mnemonic", "VK_A" },
        { "common.remove_btn", "Remove" },
        { "common.remove_btn.mnemonic", "VK_R" },                
        { "common.close_btn", "Close" },
        { "common.detail.button", "Details" },

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
        { "network.settings.dlg.browser_text", "Use proxy settings from your " + 
                  "default browser to connect to the Internet." },
        { "network.settings.dlg.proxy_text", "Override browser proxy settings." },
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
        { "delete.files.dlg.trace", "Trace and Log Files" },
        { "delete.files.dlg.applications", "Applications and Applets" },

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

	{ "general.cache.view.text", "View..." },
	{ "general.cache.view.text.mnemonic", "VK_V"},
	{ "general.cache.view.tooltip", "<html>" +
                         "Show the Java Cache Viewer" + "</html>" },
        { "general.cache.view.tooltip.unapplied", "<html>" +
                         "The Java Cache Viewer cannot<br>" +
                         "be shown before the change is applied" + "</html>" },
	{ "general.cache.view.tooltip.disabled", "<html>" +
                         "The Java Cache Viewer cannot<br>" +
                         "be shown when the cache is disabled" + "</html>" },
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
        { "update.notify_download.text", "Before downloading" },
        { "update.autoupdate.text", "Check for Updates Automatically" },
        { "update.autoupdate.disable.monthlyCheck", "Check Monthly" },
        { "update.autoupdate.disable.weeklyCheck", "Check Weekly" },
        { "update.autoupdate.disable.dailyCheck", "Check Daily" },
        { "update.autoupdate.disable.neverCheck", "Never Check" },
        { "update.autoupdate.disable.info", "We strongly recommend letting " +
                  "Java periodically check for newer versions to ensure you " +
                  "have the most secure and fastest Java experience." },
        { "update.autoupdate.disable.message", "You have chosen to never " +
                  "check for updates and may miss future security updates." },
        { "update.warning", "Java Update - Warning" },                
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
        { "update.desc_notify_download.text", "You will be notified before the update is downloaded." },
	{ "update.launchbrowser.error.text", "Unable to launch Java Update checker.  To obtain the latest Java Update, please go to http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Error - Update" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "Delete Files..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_D" },
        { "cache.settings.dialog.restore_btn", "Restore Defaults" },
        { "cache.settings.dialog.restore_btn.mnemonic", "VK_R" },
        { "cache.settings.dialog.chooser_title", "Temporary Files Location" },
        { "cache.settings.dialog.select", "Select" },
        { "cache.settings.dialog.select_tooltip", "Use selected location" },
        { "cache.settings.dialog.select_mnemonic", "S" },
        { "cache.settings.dialog.title", "Temporary Files Settings" },
        { "cache.settings.dialog.cache_location", "Location" },
        { "cache.settings.dialog.change_btn", "Change..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_H" },
        { "cache.settings.dialog.disk_space", "Disk Space" },
        { "cache.settings.dialog.diskSpaceLbl", "Set the amount of disk space for storing temporary files:"},                
        { "cache.settings.dialog.compression", "Select the compression level for JAR files:" },
        { "cache.settings.dialog.none", "None" },
        { "cache.settings.dialog.low", "Low" },
        { "cache.settings.dialog.medium", "Medium" },
        { "cache.settings.dialog.high", "High" },
        { "cache.settings.dialog.location_label", "Select the location where temporary files are kept:"},
        { "cache.settings.dialog.cacheEnabled", "Keep temporary files on my computer."},
        { "cache.settings.dialog.cacheEnabled.mnemonic", "VK_K" },
	{ "cache.settings.dialog.directory_masthead", "Directory does not exist." },
	{ "cache.settings.dialog.directory_body", "The specified directory does not exist. Please check the spelling or click the Change... button to choose a directory." },
                
        // Strings for the DialogTemplate
        { "dialog.template.name", "Name:" },
        { "dialog.template.publisher", "Publisher:" },
        { "dialog.template.from", "From:" },
        { "dialog.template.more.info", "More Information..." },
        { "dialog.template.more.info.acceleratorKey", new Integer(KeyEvent.VK_M) },
        { "dialog.template.name.unknown", "Unknown" },
        { "info48.image", "image/icon-info48.png" },

	// JNLP File/MIME association dialog strings:
	{ "association.replace.ext", "Association already exists with " +
	  "extension {0}.  Do you want to replace?"},
	{ "association.replace.mime", "Association already exists with " +
	  "MIME type {0}.  Do you want to replace?"},
	{ "association.replace.info", 
	  "The association is currently used by {0}." },
	{ "association.replace.title", "Association Warning" },
        { "association.dialog.ask", "The application will be associated " +
	  "with MIME type \"{0}\", and with file extensions \"{1}\"." },


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
        { "deployment.browser.vm.iexplorer", "Microsoft Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
                                                     "Use Sun Java with APPLET tag" +
                                                     "<br>in Internet Explorer browser" +
                                                     "</html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla family" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
                                                   "Use Sun Java with APPLET tag in" +
                                                   "<br>Mozilla or Firefox or Netscape browser(s)" +
                                                   "</html>" },
	{ "deployment.jpi.mode", "Java Plug-in" },
	{ "deployment.jpi.mode.new", "Enable the next-generation Java Plug-in (requires browser restart)" },
        { "deployment.console.debugging", "Debugging" },
	{ "deployment.browsers.applet.tag", "Default Java for browsers" },
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
        { "deployment.javaws.associations.ALWAYS", "Always allow" },
        { "deployment.javaws.associations.ALWAYS.tooltip", "<html>" +
          "Always create extension/MIME association" +
                                                  "</html>" },


        { "deployment.javaws.autodownload", "JRE Auto-Download" },
        { "deployment.javaws.autodownload.ALWAYS", "Always Auto-Download" },
        { "deployment.javaws.autodownload.ALWAYS.tooltip", "<html>" +
          "Always Auto-Download the required JRE" + "</html>" },
        { "deployment.javaws.autodownload.PROMPT", "Prompt user" },
        { "deployment.javaws.autodownload.PROMPT.tooltip", "<html>" +
          "Prompt user before Auto-Downloading the required JRE" + "</html>" },
        { "deployment.javaws.autodownload.NEVER", "Never Auto-Download" },
        { "deployment.javaws.autodownload.NEVER.tooltip", "<html>" +
          "Never Auto-Download the required JRE" + "</html>" },
        { "deployment.security.settings", "Security" },
	{ "deployment.security.general", "General" },
        { "deployment.security.askgrantdialog.show", "Allow user to grant permissions to signed content" },
        { "deployment.security.askgrantdialog.notinca", "Allow user to grant permissions to content from an untrusted authority" },

	{ "deployment.security.browser.keystore.use", "Use certificates and keys in browser keystore" },
	{ "deployment.security.clientauth.keystore.auto", "Use personal certificate automatically if only one matches server request" },
	{ "deployment.security.notinca.warning", "Warn if certificate authority can not be verified" },
        { "deployment.security.expired.warning", "Warn if certificate is expired or not yet valid" },
        { "deployment.security.jsse.hostmismatch.warning", "Warn if site certificate does not match hostname" },
        { "deployment.security.https.warning.show", "Show site certificate from server even if it is valid" },
        { "deployment.security.sandbox.awtwarningwindow", "Show sandbox warning banner" },
        { "deployment.security.sandbox.jnlp.enhanced", "Allow user to accept JNLP security requests" },
	{ "deployment.security.validation.crl", "Check certificates for revocation using Certificate Revocation Lists (CRLs)" },
        { "deployment.security.validation.ocsp", "Enable online certificate validation" },
        { "deployment.security.validation.ocsp.tooltip", "Check certificates for revocation using the Online Certificate Status Protocol (OCSP)" },
        { "deployment.security.validation.ocsp.publisher", "Enable online certificate validation for publisher certificate only" },
        { "deployment.security.validation.ocsp.publisher.tooltip", "Check publisher certificate for revocation using the Online Certificate Status Protocol (OCSP)" },
        { "deployment.security.pretrust.list", "Enable list of trusted publishers" },
        { "deployment.security.blacklist.check", "Enable blacklist revocation check" },
        { "deployment.security.password.cache", "Enable caching password for authentication" },
        { "deployment.security.TLSv1", "Use TLS 1.0" },
        { "deployment.security.SSLv2Hello", "Use SSL 2.0 compatible ClientHello format" },
        { "deployment.security.SSLv3", "Use SSL 3.0" },
	{ "deployment.security.mixcode", "Mixed code (sandboxed vs. trusted) security verification" },
	{ "deployment.security.mixcode.ENABLE", "Enable - show warning if needed" },
	{ "deployment.security.mixcode.HIDE_RUN", "Enable - hide warning and run with protections" },
	{ "deployment.security.mixcode.HIDE_CANCEL", "Enable - hide warning and don't run untrusted code" },
	{ "deployment.security.mixcode.DISABLE", "Disable verification (not recommended)" },
	{ "deployment.security.mixcode.enabled", "Mixed code security verification is enabled" },
	{ "deployment.security.mixcode.disabled", "Mixed code security verification is disabled" },
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
        { "java.quick.starter", "Java Quick Starter" },
        { "java.quick.starter.tooltip", "<html>" +
                  "Allow faster startup of " +
                  "Java applets and applications" +
                  "</html>" },

        // AboutDialog strings:
        { "about.dialog.title", "About Java" },

        // JavaPanel strings:
        { "java.panel.jre_view_btn", "View..." },
        { "java.panel.jre_view_btn.mnemonic", "VK_V" },
        { "java.panel.jre.border", " Java Runtime Environment Settings "},
        { "java.panel.jre.text", "View and manage Java Runtime versions and settings for Java applications and applets." },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "Newer Java Runtime exists \n" +
	  "Internet Explorer already has a newer version of Java Runtime. " +
	  "Would you like to replace it?\n" },
        { "browser.settings.success.caption", "Success - Browser" },
        { "browser.settings.success.masthead", "Browser settings changed." },
        { "browser.settings.success.text", "Changes will be in effect after restart of browser(s)." },
        { "browser.settings.fail.caption", "Error - Browser" },
        { "browser.settings.fail.masthead", "Unable to change browser settings."},
        { "browser.settings.fail.moz.text", "Please check that Mozilla or " +
                  "Firefox or Netscape is properly installed on the system " +
                  "and/or that you have sufficient permissions to change " +
                  "system settings." },
        { "browser.settings.fail.ie.text", "Please check if you have " +
                  "sufficient permissions to change system settings." },

        // strings in the dialogs for Java Plug-in settings
        { "jpi.settings.success.caption", "Success - Java Plug-in"},
        { "jpi.settings.success.masthead", "Java Plug-in settings changed."},
	{ "jpi.settings.success.text", "Changes to the next-generation Java Plug-in option will be in effect after restart of browser(s)." },
	{ "jpi.settings.fail.caption", "Error - Java Plug-in" },
	{ "jpi.settings.fail.masthead", "Unable to change Java Plug-in settings."},
					   { "jpi.settings.fail.text", "The next-generation Java Plug-in option cannot be changed at this time " +
					       "because one or more browsers are running. " +
					       "Please close all browser windows before changing the next-generation Java Plug-in option." },

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

        {"delete.files.dlg.applications.tooltip.enabled", "<html>" +
                          "Check this option to delete all resources, " +
                          "<br>applications, and applets cached by " +
                          "<br>Java Web Start and Java Plug-in." +
                                          "</html>" },
                                                  
        {"delete.files.dlg.applications.tooltip.disabled", "<html>" +
                          "Applications and applets cached by " +
                          "<br>Java Web Start and Java Plug-in " +
                          "<br>cannot be deleted when cache is disabled." +
                                          "</html>" },

        {"delete.files.dlg.trace.tooltip", "<html>" +
                          "Check this option to delete all trace " +
                          "<br>and log files created by " +
                          "<br>Java Web Start and Java Plug-in." +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "Specify the directory where temporary" +
                                          "<br>files are stored"+
                                          "</html>" },
                                                  
        { "cache.settings.dialog.restore_btn.tooltip", "<html>"  +
                                          "Restore default values for" +
                                          "<br>temporary files settings" +
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

        { "java.panel.jre_view_btn.tooltip",   "<html>" +
	              "Modify Java runtime settings for applications and applets" +
	              "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "View information about this version of the JRE" +
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

        { "deploy.jre.find_btn.tooltip",  "<html>" +
		        "Search for an installed Java Runtime" +
                                        "<br>Environment" +
		        "</html>" },

        { "deploy.jre.add_btn.tooltip",   "<html>" +
                                        "Add a new entry to the list" +
		                        "</html>" },

        { "deploy.jre.remove_btn.tooltip",  "<html>" +
                                          "Remove the selected entry from the" +
                                          "<br>user list" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "Allow JRE Download" },
        { "download.jre.prompt", "The application requires a JRE that is not " +
	  "installed on the system (version {0}). \n" +
	  "Do you want this JRE to be downloaded and installed?" },
        { "download.jre.prompt.okButton", "Download" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "download.jre.prompt.cancelButton", "Cancel" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "autoupdatecheck.buttonYes", "Yes" },
	{ "autoupdatecheck.buttonYes.mnemonic", "VK_Y" },
	{ "autoupdatecheck.buttonNo", "No" },
	{ "autoupdatecheck.buttonNo.mnemonic", "VK_N" },
	{ "autoupdatecheck.buttonAskLater", "Ask Me Later" },
	{ "autoupdatecheck.buttonAskLater.mnemonic", "VK_A" },
	{ "autoupdatecheck.caption", "Check for Updates Automatically" },
        { "autoupdatecheck.message", "Keeping Java up to date will "+
			"improve the security and performance of your "+
			"Java applications.  Enabling this feature will "+
			"provide you with access to Java updates as they "+
			"become available." },
	{ "autoupdatecheck.masthead", "Enable automatic Java(TM) updates?" },

        { "uninstall.app.prompt.title", "Confirm File Deletion" },
        { "uninstall.app.prompt.message", "Are you sure you want to completely "
                                        + "remove ''{0}'' and all of its components?" },
        // jardiff
        { "jardiff.error.create", "Unable to create jardiff: {0}" },
        { "jardiff.error.apply", "Unable to apply jardiff: {0}" },
        { "jardiff.error.noindex", "Invalid jardiff, no index! {0}" },
        { "jardiff.error.badheader", "Invalid jardiff header: {0}" },
        { "jardiff.error.badremove", "Invalid jardiff remove command: {0}" },
        { "jardiff.error.badmove", "Invalid jardiff move command: {0}" },
        { "jardiff.error.badcommand", "Invalid jardiff command {0}:" },
        // cache
        { "cache.removeCacheEntry", "Remove cache entry: {0}" },
        { "cache.getCacheEntry.return.found", "Cache entry found [url: {0}, version: {1}]" },
        { "cache.getCacheEntry.return.notfound", "Cache entry not found [url: {0}, version: {1}]" },
        // cache entry
        { "cacheEntry.applyJarDiff", "Apply jardiff for {0} between {1} and {2}"},
        { "cacheEntry.unsignedJar", "No certificate info for unsigned JAR file: {0}"},
        // BasicHttpRequest
        { "basicHttpRequest.responseCode", "ResponseCode for {0} : {1}"},
        { "basicHttpRequest.encoding", "Encoding for {0} : {1}"},
        // BasicHttpResponse
        { "basicHttpResponse.disconnect", "Disconnect connection to {0}"},
        // DownloadEngine
        { "downloadEngine.serverResponse", "Sever response: (length: {0}, lastModified: {1}, downloadVersion: {2}, mimeType: {3})"},
        // HttpDownloadHelper
        { "httpDownloadHelper.doingDownload", "Downloading resource: {0}\n\tContent-Length: {1}\n\tContent-Encoding: {2}"},
        { "httpDownloadHelper.wroteUrlToFile", "Wrote URL {0} to File {1}"},
        // DeployOfflineManager
        { "deployOfflineManager.askUserGoOnline.title", 
              "Application unavailable while offline"},
        { "deployOfflineManager.askUserGoOnline.message", "The application " +
              "has requested to go online. Do you want to continue?"},	
        { "cache.upgrade.title.javaws", "Java Web Start" },
        { "cache.upgrade.title.javapi", "Java Plug-in" },
        { "cache.upgrade.masthead.javaws", "Upgrading Java application cache."},
        { "cache.upgrade.masthead.javapi", "Upgrading Java applet cache."},
        { "cache.upgrade.message.javaws", "Please wait while your stored Java applications are updated for Java Web Start 6."},
        { "cache.upgrade.message.javapi", "Please wait while your stored Java applets are updated for Java SE 6."},
//	Specify minimum font size to be used in UI for different locales.
//        { "ui.min.font.size", new Integer(10) },

	{ "javaws.ssv.title", "Java Security Warning" },

	{ "javaws.ssv.runold.masthead", "The application requires an " +
	  "earlier version of Java.  Do you want to continue?" },
	{ "javaws.ssv.runold.bullet", "The required version of Java, {0}, " +
	  "is not the latest and may not " + 
	  "contain the latest security updates." },

	{ "javaws.ssv.download.masthead", "The application needs to " +
	  "download an earlier version of Java.  " + 
	  "Do you want to continue?" },
	{ "javaws.ssv.download.bullet", "The required version of Java, {0} " +
	  "from {1}, is not the latest and may not " +
	  "contain the latest security updates." },

	{ "javaws.ssv.download.button", "Download" },
	{ "javaws.ssv.run.button", "Run" },
        { "applet.error.details.btn", "Details" },
        { "applet.error.details.btn.mnemonic", "VK_D" },
        { "applet.error.ignore.btn", "Ignore" },
        { "applet.error.ignore.btn.mnemonic", "VK_I" },
        { "applet.error.reload.btn", "Reload" },
        { "applet.error.reload.btn.mnemonic", "VK_R" },

        // Java system tray menu strings -- copied from C++ up to Java
        { "systray.open.console",      "Open {0} &Console" },
        { "systray.hide.console",      "Hide {0} &Console" },
        { "systray.about.java",        "&About Java Technology" },
        { "systray.disable",           "&Hide Icon" },
        { "systray.goto.java",         "&Go to Java.com" },
        { "systray.tooltip",           "Java(TM) Platform, Standard Edition" },
        { "systray.balloon.title",     "Java(TM) Platform, Standard Edition" },
        { "systray.balloon.tooltip",   "Visit us for more information at:\r\nhttp://www.java.com" },
        { "systray.open.controlpanel", "&Open Control Panel" },

	// Plugin Activator strings
	{ "loading", "Loading {0} ..." },
	{ "java_applet", "Java Applet" },
        { "failed", "Loading Java Applet Failed..." },
        { "image_failed", "Failed to create user-defined image.  Check image file name." },
	{ "java_not_enabled", "Java is not enabled" },
        { "exception", "Exception: {0}" },

	{ "bean_code_and_ser", "Bean cannot have both CODE and JAVA_OBJECT defined " },
	{ "status_applet", "Applet {0} {1}" },

	{ "optpkg.cert_expired", "The security certificate has expired." +
                  "  Optional package not installed." },
	{ "optpkg.cert_notyieldvalid", "The security certificate is not valid." +
                  "  Optional package not installed." },
	{ "optpkg.cert_notverify", "The publisher cannot be verified by a " +
                  "trusted source.  Optional package not installed." },
	{ "optpkg.general_error", "General exception.  " +
                  "Optional package not installed." },
	{ "optpkg.caption", "Security Warning" },
	{ "optpkg.installer.launch.wait", "Click OK to close this dialog " +
                  "and continue applet loading after optional package " +
                  "installer exits." },
	{ "optpkg.installer.launch.caption", 
                  "Installing Optional Package"},
	{ "optpkg.prompt_user.text", "The applet requires a newer " +
                  "version of optional package.  Do you want to continue?" },
        { "optpkg.prompt_user.specification", " ({0} specification)" },                           
        { "optpkg.prompt_user.implementation", " ({0} implementation)" },
	{ "optpkg.prompt_user.default.text", "The applet requires " +
                  "installation of optional package.  Do you want to continue?" },
        { "optpkg.prompt_user.caption", "Request Download" },

	{ "cache.error.text", "Unable to update files in cache." },
	{ "cache.error.caption", "Error - Cache" },
	{ "cache.version_format_error", "{0} is not in the form xxxx.xxxx.xxxx.xxxx, where x is a hexadecimal digit" },
	{ "cache.version_attrib_error", "Number of attributes specified in \'cache_archive\' doesn't match those in \'cache_version\'" },
	{ "cache.header_fields_missing", "Last modified time and/or expiration value is not available.  Jar file will not be cached."},

	{ "applet.progress.load", "Loading applet ..." },
	{ "applet.progress.init", "Initializing applet ..." },
	{ "applet.progress.start", "Starting applet ..." },
	{ "applet.progress.stop", "Stopping applet ..." },
	{ "applet.progress.destroy", "Destroying applet ..." },
	{ "applet.progress.dispose", "Disposing applet ..." },
	{ "applet.progress.quit", "Quiting applet ..." },
	{ "applet.progress.stoploading", "Stopped loading ..." },
	{ "applet.progress.interrupted", "Interrupted thread ..." },
	{ "applet.progress.joining", "Joining applet thread ..." },
	{ "applet.progress.joined", "Joined applet thread ..." },
	{ "applet.progress.loadImage", "Loading image " },
	{ "applet.progress.loadAudio", "Loading audio " },
	{ "applet.progress.findinfo.0", "Finding information ..." },
	{ "applet.progress.findinfo.1", "Done ..." },
	{ "applet.progress.timeout.wait", "Waiting for timeout ..." },
	{ "applet.progress.timeout.jointing", "Doing a join ..." },
	{ "applet.progress.timeout.jointed", "Done with join ..." },

	{ "modality.register", "Registered modality listener" },
	{ "modality.unregister", "Unregistered modality listener" },
	{ "modality.pushed", "Modality pushed" },
	{ "modality.popped", "Modality popped" },

	{ "progress.listener.added", "Added progress listener: {0}" },
	{ "progress.listener.removed", "Removed progress listener: {0}" },

	{ "liveconnect.UniversalBrowserRead.enabled", "JavaScript: UniversalBrowserRead enabled" },
	{ "liveconnect.java.system", "JavaScript: calling Java system code" },
	{ "liveconnect.same.origin", "JavaScript: caller and callee have same origin" },
	{ "liveconnect.default.policy", "JavaScript: default security policy = {0}" },
	{ "liveconnect.UniversalJavaPermission.enabled", "JavaScript: UniversalJavaPermission enabled" },
	{ "liveconnect.wrong.securitymodel", "Netscape security model is no longer supported.\n"
					     + "Please migrate to the Java 2 security model instead.\n" },
	{ "liveconnect.insecurejvm.warning", "This application is going to perform an insecure operation."
	    + " Do you want to continue?" },

        { "pluginclassloader.created_files", "Created {0} in cache." },
        { "pluginclassloader.deleting_files", "Deleting JAR files from cache." },
        { "pluginclassloader.file", "   deleting from cache {0}" },
        { "pluginclassloader.empty_file", "{0} is empty, deleting from cache." },

	{ "appletcontext.audio.loaded", "Loaded audio clip: {0}" },
	{ "appletcontext.image.loaded", "Loaded image: {0}" },

	{ "securitymgr.automation.printing", "Automation: Accept printing" },

	{ "classloaderinfo.referencing", "Referencing classloader: {0}, refcount={1}" },
	{ "classloaderinfo.releasing", "Releasing classloader: {0}, refcount={1}" },
	{ "classloaderinfo.caching", "Caching classloader: {0}" },
	{ "classloaderinfo.cachesize", "Current classloader cache size: {0}" },
	{ "classloaderinfo.num", "Number of cached classloaders over {0}, unreference {1}" },
        { "jsobject.call", "JSObject::call: name={0}" },
        { "jsobject.eval", "JSObject::eval({0})" },
        { "jsobject.getMember", "JSObject::getMember: name={0}" },
        { "jsobject.setMember", "JSObject::setMember: name={0}" },
        { "jsobject.removeMember", "JSObject::removeMember: name={0}" },
        { "jsobject.getSlot", "JSObject::getSlot: {0}" },
        { "jsobject.setSlot", "JSObject::setSlot: slot={0}" },
	{ "jsobject.invoke.url.permission", "the url of the applet is {0} and the permission is = {1}"},

	{ "optpkg.install.info", "Installing optional package {0}" },
	{ "optpkg.install.fail", "Optional package installation failed." },
	{ "optpkg.install.ok", "Optional package installation succeeded." },
	{ "optpkg.install.automation", "Automation: Accept optional package installation" },
	{ "optpkg.install.granted", "Optional package download granted by user, download from {0}" },
	{ "optpkg.install.deny", "Optional package download not granted by user" },
	{ "optpkg.install.begin", "Installing {0}" },
	{ "optpkg.install.java.launch", "Launching Java installer" },
	{ "optpkg.install.java.launch.command", "Launching Java installer through ''{0}''" },
	{ "optpkg.install.native.launch", "Launching native installer" },
	{ "optpkg.install.native.launch.fail.0", "Unable to execute {0}" },
	{ "optpkg.install.native.launch.fail.1", "Access to {0} failed" },
	{ "optpkg.install.raw.launch", "Installing raw optional package" },
	{ "optpkg.install.raw.copy", "Copying Raw Optional Package from {0} to {1}" },
	{ "optpkg.install.error.nomethod", "Dependent Extension Provider not installed : Cannot get the "
				         + " addExtensionInstallationProvider method" },
	{ "optpkg.install.error.noclass", "Dependent Extension Provider not installed : Cannot get the "
					 + "sun.misc.ExtensionDependency class" },

	{"progress_dialog.downloading", "Plug-in: Downloading ..."},
        {"progress_dialog.dismiss_button", "Dismiss"},
        {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
        {"progress_dialog.from", "from"},

        {"applet_viewer.color_tag", "Incorrect number of components in {0}"},

        {"progress_info.downloading", "Downloading JAR file(s)"},
        {"progress_bar.preload", "Preloading JAR files: {0}"},

        {"cache.size", "Cache Size: {0}"},
        {"cache.cleanup", "Cache size is: {0} bytes, cleanup is necessary"},
        {"cache.full", "Cache is full: deleting file {0}"},
        {"cache.inuse", "Cannot delete file {0} since it is being used by this application"},
        {"cache.notdeleted", "Cannot delete file {0}, may be used by this and/or other application(s)"},
        {"cache.out_of_date", "Cached copy of {0} is out of date\n  Cached copy: {1}\n  Server copy: {2}"},
        {"cache.loading", "Loading {0} from cache"},
        {"cache.cache_warning", "WARNING: Unable to cache {0}"},
        {"cache.downloading", "Downloading {0} to cache"},
        {"cache.cached_name", "Cached file name: {0}"},
        {"cache.load_warning", "WARNING: error reading {0} from cache."},
        {"cache.disabled", "Cache is disabled by user"},
        {"cache.minSize", "Cache is disabled, cache limit is set to {0}, at least 5 MB should be specified"},
        {"cache.directory_warning", "WARNING: {0} is not a directory.  Cache will be disabled."},
        {"cache.response_warning", "WARNING: Unexpected response {0} for {1}.  File will be downloaded again."},
        {"cache.enabled", "Cache is enabled"},
        {"cache.location", "Location: {0}"},
        {"cache.maxSize", "Maximum size: {0}"},
        {"cache.create_warning", "WARNING: Could not create cache directory {0}.  Caching will be disabled."},
        {"cache.read_warning", "WARNING: Cannot read cache directory {0}.  Caching will be disabled."},
        {"cache.write_warning", "WARNING: Cannot write to cache directory {0}.  Caching will be disabled."},
        {"cache.compression", "Compression level: {0}"},
        {"cache.cert_load", "Certificates for {0} is read from JAR cache"},
	{"cache.jarjar.invalid_file", ".jarjar file contains a non .jar file"},
	{"cache.jarjar.multiple_jar", ".jarjar file contains more than one .jar file"},
        {"cache.version_checking", "Version checking for {0}, specified version is {1}"},
        {"cache.preloading", "Preloading file {0}"},

	{ "lifecycle.applet.found", "Found previous stopped applet from lifecycle cache" },
	{ "lifecycle.applet.support", "Applet supports legacy lifecycle model - add applet to lifecycle cache" },
	{ "lifecycle.applet.cachefull", "Lifecycle cache is full - prune least recently used applets" },

	{ "com.method.ambiguous", "Unable to select a method, ambiguous parameters" },
	{ "com.method.notexists", "{0} :no such method exists" },
	{ "com.notexists", "{0} :no such method/property exists" },
	{ "com.method.invoke", "Invoking method: {0}" },
	{ "com.method.jsinvoke", "Invoking JS method: {0}" },
	{ "com.method.argsTypeInvalid", "The parameters cannot be converted to the required types" },
	{ "com.method.argCountInvalid", "Number of arguments is not correct" },
	{ "com.field.needsConversion", "Needs conversion: {0} --> {1}" },
	{ "com.field.typeInvalid", " cannot be converted to type: {0}" },
	{ "com.field.get", "Getting property: {0}" },
	{ "com.field.set", "Setting property: {0}" },

	{ "rsa.cert_expired", "The security certificate has expired.  " +
                  "Code will be treated as unsigned." },
	{ "rsa.cert_notyieldvalid", "The security certificate is not valid." +
                  "  Code will be treated as unsigned." },
	{ "rsa.general_error", "The publisher cannot be verified by a " +
                  "trusted source.  Code will be treated as unsigned." },
	{ "ocsp.general_error", "Unable to verify the publisher information. Check" +
                  " the date and time on your system." },

	{ "dialogfactory.menu.show_console", "Show Java Console" },
	{ "dialogfactory.menu.hide_console", "Hide Java Console" },
	{ "dialogfactory.menu.about", "About Java Plug-in" },
	{ "dialogfactory.menu.copy", "Copy" },
	{ "dialogfactory.menu.open_console", "Open Java Console" },
	{ "dialogfactory.menu.about_java", "About Java(TM)" },
	{ "applet.error.message", "Error.  Click for details" },
	{ "applet.error.security.masthead", "The application is not allowed to run."},
	{ "applet.error.security.body", "You did not accept the security " +
                  "certificate required to run the application.  " +
                  "Click \"Reload\" to review the security certificate and " +
                  "reload the application."},
	{ "applet.error.generic.masthead", "The application failed to run." },
	{ "applet.error.generic.body", "There was an error while executing " +
                  "the application.  Click \"Details\" for more information." },
	{ "applet.error.details.btn", "Details" },
	{ "applet.error.ignore.btn", "Ignore" },
	{ "applet.error.reload.btn", "Reload" },
    };
}
