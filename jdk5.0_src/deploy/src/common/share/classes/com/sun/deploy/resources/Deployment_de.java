/*
 * @(#)Deployment_de.java	1.25 04/07/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;


/**
 * German version of Deployment strings.
 *
 * @author Stanley Man-Kit Ho
 */

public final class Deployment_de extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
        { "product.javapi.name", "Java Plug-in {0}" },
        { "product.javaws.name", "Java Web Start {0}" },

	{ "console.version", "Version" },
	{ "console.default_vm_version", "Standardversion der virtuellen Maschine " },
	{ "console.using_jre_version", "Verwendung der JRE-Version" },
	{ "console.user_home", "Home-Verzeichnis des Benutzers" },
        { "console.caption", "Java Console" },
        { "console.clear", "L\u00f6schen" },
        { "console.clear.acceleratorKey", new Integer(KeyEvent.VK_L)},
        { "console.close", "Schlie\u00dfen" },
        { "console.close.acceleratorKey", new Integer(KeyEvent.VK_S) },
        { "console.copy", "Kopieren" },
        { "console.copy.acceleratorKey", new Integer(KeyEvent.VK_K) },
	{ "console.menu.text.top", "----------------------------------------------------\n" },
	{ "console.menu.text.c", "c:   Konsolenfenster schlie\u00dfen\n" },
	{ "console.menu.text.f", "f:   Objekte in Finalisierungswarteschlange finalisieren\n" },
	{ "console.menu.text.g", "g:   Speicherbereinigung\n" },
	{ "console.menu.text.h", "h:   Diese Hilfemeldung anzeigen\n" },
	{ "console.menu.text.j", "j:   JCOV-Daten ausgeben\n"},
	{ "console.menu.text.l", "l:   ClassLoader-Liste ausgeben\n" },
	{ "console.menu.text.m", "m:   Speicherbelegung anzeigen\n" },
	{ "console.menu.text.o", "o:   Protokollierung ausl\u00f6sen\n" },
	{ "console.menu.text.p", "p:   Proxy-Konfiguration neu laden\n" },
	{ "console.menu.text.q", "q:   Konsole ausblenden\n" },
	{ "console.menu.text.r", "r:   Richtlinien-Konfiguration neu laden\n" },
	{ "console.menu.text.s", "s:   System- und Bereitstellungseigenschaften ausgeben\n" },
	{ "console.menu.text.t", "t:   Threadliste ausgeben\n" },
	{ "console.menu.text.v", "v:   Thread-Stack ausgeben\n" },
	{ "console.menu.text.x", "x:   ClassLoader-Cache l\u00f6schen\n" },
	{ "console.menu.text.0", "0-5: Trace-Stufe auf <n> setzen\n" },
	{ "console.menu.text.tail", "----------------------------------------------------\n" },
	{ "console.done", "Fertig." },
	{ "console.trace.level.0", "Trace-Stufe auf 0 (kein) setzen ... abgeschlossen." },
	{ "console.trace.level.1", "Trace-Stufe auf 1 (einfach) setzen ... abgeschlossen." },
	{ "console.trace.level.2", "Trace-Stufe auf 2 (einfach, Netz) setzen ... abgeschlossen." },
	{ "console.trace.level.3", "Trace-Stufe auf 3 (einfach, Netz, Sicherheit) setzen ... abgeschlossen." },
	{ "console.trace.level.4", "Trace-Stufe auf 4 (einfach, Netz, Sicherheit, erweitert) setzen ... abgeschlossen." },
	{ "console.trace.level.5", "Trace-Stufe auf 5 (alle) setzen ... abgeschlossen." },
	{ "console.log", "Protokollieren : " },
	{ "console.completed", " ... abgeschlossen." },
	{ "console.dump.thread", "Threadliste ausgeben...\n" },
	{ "console.dump.stack", "Thread-Stack ausgeben...\n" },
	{ "console.dump.system.properties", "Systemeigenschaften ausgeben...\n" },
        { "console.dump.deployment.properties", "Bereitstellungseigenschaften ausgeben ...\n" },
	{ "console.clear.classloader", "ClassLoader-Cache l\u00f6schen... abgeschlossen." },
	{ "console.reload.policy", "Richtlinien-Konfiguration neu laden" },
	{ "console.reload.proxy", "Proxy-Konfiguration neu laden..." },
	{ "console.gc", "Speicherbereinigung" },
	{ "console.finalize", "Objekte in Finalisierungswarteschlange finalisieren" },
	{ "console.memory", "Speicher: {0}K  Frei: {1}K  ({2}%)" },
	{ "console.jcov.error", "JCOV-Laufzeitfehler: Pr\u00fcfen Sie, ob Sie die richtige JCOV-Option angegeben haben.\n"},
	{ "console.jcov.info", "Die JCOV-Daten wurden erfolgreich ausgegeben.\n"},

	{ "https.dialog.caption", "Warnung - HTTPS" },
	{ "https.dialog.text", "<html><b>Hostnamen-Konflikt</b></html>Der Hostname im Sicherheitszertifikat des Servers stimmt nicht mit dem Namen des Servers \u00fcberein."
			     + "\n\nHostname der URL: {0}"
			     + "\nHostname des Zertifikats: {1}"
			     + "\n\nM\u00f6chten Sie fortfahren?" },
	{ "https.dialog.unknown.host", "Unbekannter Host" },

	{ "security.dialog.caption", "Warnung - Sicherheit" },
	{ "security.dialog.text0", "Soll das signierte {0}, das von \"{1}\" \u00fcbertragen wird, akzeptiert werden?"
				 + "\n\nAuthentizit\u00e4t des Herausgebers \u00fcberpr\u00fcft durch: \"{2}\"" },
        { "security.dialog.text0a", "Soll das signierte {0}, das von \"{1}\" \u00fcbertragen wird, akzeptiert werden?"
                                + "\n\nDie Authentizit\u00e4t des Herausgebers kann nicht \u00fcberpr\u00fcft werden." },
  	{ "security.dialog.timestamp.text1", "The {0} was signed on {1}." },
	{ "security.dialog_https.text0", "M\u00f6chten Sie das Zertifikat von der Website \"{0}\" zum Austausch von verschl\u00fcsselten Informationen annehmen?"
				 + "\n\nAuthentizit\u00e4t des Herausgebers \u00fcberpr\u00fcft durch: \"{1}\"" },
        { "security.dialog_https.text0a", "M\u00f6chten Sie das Zertifikat von der Website \"{0}\" zum Austausch von verschl\u00fcsselten Informationen annehmen?"
                         + "\n\nDie Authentizit\u00e4t des Herausgebers kann nicht \u00fcberpr\u00fcft werden." },
	{ "security.dialog.text1", "\nVorsicht: \"{0}\" gibt an, dass dieser Inhalt sicher ist. Sie sollten diesen Inhalt nur akzeptieren, wenn \"{1}\" vertrauensw\u00fcrdig ist." },
	{ "security.dialog.unknown.issuer", "Unbekannter Aussteller" },
	{ "security.dialog.unknown.subject", "Unbekannter Betreff" },
	{ "security.dialog.certShowName", "{0} ({1})" },
	{ "security.dialog.rootCANotValid", "Das Sicherheitszertifikat stammt von einem Unternehmen, das nicht als vertrauensw\u00fcrdig eingestuft ist." },
	{ "security.dialog.rootCAValid", "Das Sicherheitszertifikat stammt von einem Unternehmen, das als vertrauensw\u00fcrdig eingestuft ist." },
	{ "security.dialog.timeNotValid", "Das Sicherheitszertifikat ist abgelaufen oder noch nicht g\u00fcltig." },
	{ "security.dialog.timeValid", "Das Sicherheitszertifikat ist nicht abgelaufen und immer noch g\u00fcltig." },
	{ "security.dialog.timeValidTS", "The security certificate was valid when the {0} was signed." },
	{ "security.dialog.buttonAlways", "Immer" },
        { "security.dialog.buttonAlways.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "security.dialog.buttonYes", "Ja" },
	{ "security.dialog.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_J)},
        { "security.dialog.buttonNo", "Nein" },
	{ "security.dialog.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "security.dialog.buttonViewCert", "Mehr Details" },
        { "security.dialog.buttonViewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

        { "security.badcert.caption", "Warnung - Sicherheit" },
        { "security.badcert.https.text", "SSL-Zertifikat kann nicht validiert werden.\n{0} wird nicht ausgef\u00fchrt." },
        { "security.badcert.config.text", "Ihre Sicherheitskonfiguration l\u00e4sst eine Validierung dieses Zertifikats nicht zu. {0} wird nicht ausgef\u00fchrt." },
        { "security.badcert.text", "Zertifikat konnte nicht validiert werden. {0} wird nicht ausgef\u00fchrt." },
        { "security.badcert.viewException", "Ausnahme anzeigen" },
        { "security.badcert.viewException.acceleratorKey", new Integer(KeyEvent.VK_A)},
        { "security.badcert.viewCert", "Mehr Details" },
        { "security.badcert.viewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

	{ "cert.dialog.caption", "Details - Zertifikat" },
	{ "cert.dialog.certpath", "Zertifikatspfad" },
	{ "cert.dialog.field.Version", "Version" },
	{ "cert.dialog.field.SerialNumber", "Seriennummer" },
	{ "cert.dialog.field.SignatureAlg", "Signatur-Algorithmus" },
	{ "cert.dialog.field.Issuer", "Aussteller" },
	{ "cert.dialog.field.EffectiveDate", "Tag des Inkrafttretens" },
	{ "cert.dialog.field.ExpirationDate", "Ablaufdatum" },
	{ "cert.dialog.field.Validity", "G\u00fcltigkeit" },
	{ "cert.dialog.field.Subject", "Betreff" },
	{ "cert.dialog.field.Signature", "Signatur" },
	{ "cert.dialog.field", "Feld" },
	{ "cert.dialog.value", "Wert" },
        { "cert.dialog.close", "Schlie\u00dfen" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_S) },

	{ "clientauth.password.dialog.buttonOK", "OK" },
	{ "clientauth.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.password.dialog.buttonCancel", "Abbrechen" },
	{ "clientauth.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "clientauth.password.dialog.caption", "Passwort erforderlich - Keystore f\u00fcr die Client-Authentifizierung" },
	{ "clientauth.password.dialog.text", "Geben Sie ein Passwort zum Zugriff auf den Keystore f\u00fcr die Client-Authentifizierung ein:\n" },
        { "clientauth.password.dialog.error.caption", "Fehler - Keystore f\u00fcr die Client-Authentifizierung" },
        { "clientauth.password.dialog.error.text", "<html><b>Fehler beim Keystore-Zugriff</b></html>Entweder wurde der Keystore manipuliert, oder das Passwort ist falsch." },

	{ "clientauth.certlist.dialog.buttonOK", "OK" },
	{ "clientauth.certlist.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.certlist.dialog.buttonCancel", "Abbrechen" },
	{ "clientauth.certlist.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "clientauth.certlist.dialog.buttonDetails", "Details" },
	{ "clientauth.certlist.dialog.buttonDetails.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "clientauth.certlist.dialog.caption", "Client-Authentifizierung" },
	{ "clientauth.certlist.dialog.text", "F\u00fcr die Website, zu der Sie eine Verbindung herstellen m\u00f6chten, ist eine Identifikation erforderlich.\nW\u00e4hlen Sie das f\u00fcr die Verbindung zu verwendende Zertifikat aus.\n" },

	{ "dialogfactory.confirmDialogTitle", "Best\u00e4tigung erforderlich - Java" },
	{ "dialogfactory.inputDialogTitle", "Informationen erforderlich - Java" },
	{ "dialogfactory.messageDialogTitle", "Mitteilung - Java" },
	{ "dialogfactory.exceptionDialogTitle", "Fehler - Java" },
	{ "dialogfactory.optionDialogTitle", "Option - Java" },
	{ "dialogfactory.aboutDialogTitle", "Info - Java" },
	{ "dialogfactory.confirm.yes", "Ja" },
        { "dialogfactory.confirm.yes.acceleratorKey", new Integer(KeyEvent.VK_J)},
        { "dialogfactory.confirm.no", "Nein" },
        { "dialogfactory.confirm.no.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "dialogfactory.moreInfo", "Mehr Details" },
        { "dialogfactory.moreInfo.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "dialogfactory.lessInfo", "Weniger Details" },
        { "dialogfactory.lessInfo.acceleratorKey", new Integer(KeyEvent.VK_W)},
	{ "dialogfactory.java.home.link", "http://www.java.com" },
	{ "dialogfactory.general_error", "<html><b>Allgemeine Ausnahme</b></html>" },
	{ "dialogfactory.net_error", "<html><b>Netzwerkausnahme</b></html>" },
	{ "dialogfactory.security_error", "<html><b>Sicherheitsausnahme</b></html>" },
	{ "dialogfactory.ext_error", "<html><b>Ausnahme bei optionalem Paket</b></html>" },
	{ "dialogfactory.user.selected", "Benutzerauswahl: {0}" },
	{ "dialogfactory.user.typed", "Benutzereingabe: {0}" },

	{ "deploycertstore.cert.loading", "Bereitstellungszertifikate werden geladen aus {0}" },
	{ "deploycertstore.cert.loaded", "Bereitstellungszertifikate wurden geladen aus {0}" },
	{ "deploycertstore.cert.saving", "Bereitstellungszertifikate werden gespeichert in {0}" },
	{ "deploycertstore.cert.saved", "Bereitstellungszertifikate wurden gespeichert in {0}" },
	{ "deploycertstore.cert.adding", "Zertifikat wird permanentem Bereitstellungszertifikatspeicher hinzugef\u00fcgt" },
	{ "deploycertstore.cert.added", "Zertifikat wurde permanentem Bereitstellungszertifikatspeicher als Alias {0} hinzugef\u00fcgt" },
	{ "deploycertstore.cert.removing", "Zertifikat wird aus permanentem Bereitstellungszertifikatspeicher entfernt" },
	{ "deploycertstore.cert.removed", "Zertifikat wurde als Alias {0} aus permanentem Bereitstellungszertifikatspeicher entfernt" },
	{ "deploycertstore.cert.instore", "Zertifikat wird im permanenten Bereitstellungszertifikatspeicher gesucht" },
	{ "deploycertstore.cert.canverify", "\u00dcberpr\u00fcfung, ob Zertifikat durch Zertifikate im permanenten Bereitstellungszertifikatspeicher verifiziert werden kann" },
	{ "deploycertstore.cert.iterator", "Zertifikat-Iterator aus permanentem Bereitstellungszertifikatspeicher abrufen" },
	{ "deploycertstore.cert.getkeystore", "Keystore-Objekt von Bereitstellungszertifikatspeicher abrufen" },

	{ "httpscertstore.cert.loading", "Bereitstellungs-SSL-Zertifikate werden geladen aus {0}" },
	{ "httpscertstore.cert.loaded", "Bereitstellungs-SSL-Zertifikate wurden geladen aus {0}" },
	{ "httpscertstore.cert.saving", "Bereitstellungs-SSL-Zertifikate werden gespeichert in {0}" },
	{ "httpscertstore.cert.saved", "Bereitstellungs-SSL-Zertifikate wurden gespeichert in {0}" },
	{ "httpscertstore.cert.adding", "SSL-Zertifikat wird permanentem Bereitstellungszertifikatspeicher hinzugef\u00fcgt" },
	{ "httpscertstore.cert.added", "SSL-Zertifikat wurde permanentem Bereitstellungszertifikatspeicher als Alias {0} hinzugef\u00fcgt" },
	{ "httpscertstore.cert.removing", "SSL-Zertifikat wird aus permanentem Bereitstellungszertifikatspeicher entfernt" },
	{ "httpscertstore.cert.removed", "SSL-Zertifikat wurde als Alias {0} aus permanentem Bereitstellungszertifikatspeicher entfernt" },
	{ "httpscertstore.cert.instore", "SSL-Zertifikat wird im permanenten Bereitstellungszertifikatspeicher gesucht" },
	{ "httpscertstore.cert.canverify", "\u00dcberpr\u00fcfung, ob SSL-Zertifikat durch Zertifikate im permanenten Bereitstellungszertifikatspeicher verifiziert werden kann" },
	{ "httpscertstore.cert.iterator", "SSL-Zertifikat-Iterator aus permanentem Bereitstellungszertifikatspeicher abrufen" },
	{ "httpscertstore.cert.getkeystore", "Keystore-Objekt von Bereitstellungszertifikatspeicher abrufen" },

	{ "rootcertstore.cert.loading", "Zertifizierungsstellen-Stammzertifikate werden geladen aus {0}" },
	{ "rootcertstore.cert.loaded", "Zertifizierungsstellen-Stammzertifikate wurden geladen aus {0}" },
	{ "rootcertstore.cert.noload", "Datei f\u00fcr Zertifizierungsstellen-Stammzertifikat wurde nicht gefunden: {0}" },
	{ "rootcertstore.cert.saving", "Zertifizierungsstellen-Stammzertifikate werden gespeichert in {0}" },
	{ "rootcertstore.cert.saved", "Zertifizierungsstellen-Stammzertifikate wurden gespeichert in {0}" },
	{ "rootcertstore.cert.adding", "Zertifikat wird Zertifizierungsstellen-Stammzertifikatspeicher hinzugef\u00fcgt" },
	{ "rootcertstore.cert.added", "Zertifikat wurde Zertifizierungsstellen-Stammzertifikatspeicher als Alias {0} hinzugef\u00fcgt" },
	{ "rootcertstore.cert.removing", "Zertifikat wird aus Zertifizierungsstellen-Stammzertifikatspeicher entfernt" },
	{ "rootcertstore.cert.removed", "Zertifikat wurde als Alias {0} aus Zertifizierungsstellen-Stammzertifikatspeicher entfernt" },
	{ "rootcertstore.cert.instore", "Zertifikat wird in Zertifizierungsstellen-Stammzertifikatspeicher gesucht" },
	{ "rootcertstore.cert.canverify", "\u00dcberpr\u00fcfung, ob Zertifikat durch Zertifikate in Zertifizierungsstellen-Stammzertifikatspeicher verifiziert werden kann" },
	{ "rootcertstore.cert.iterator", "Zertifikat-Iterator aus Zertifizierungsstellen-Stammzertifikatspeicher abrufen" },
	{ "rootcertstore.cert.getkeystore", "Keystore-Objekt von Zertifizierungsstellen-Stammzertifikatspeicher abrufen" },
	{ "rootcertstore.cert.verify.ok", "Zertifikat wurde erfolgreich durch Zertifizierungsstellen-Stammzertifikate verifiziert" },
	{ "rootcertstore.cert.verify.fail", "Zertifikat konnte durch Zertifizierungsstellen-Stammzertifikate nicht verifiziert werden" },
	{ "rootcertstore.cert.tobeverified", "Zu verifizierendes Zertifikat:\n{0}" },
	{ "rootcertstore.cert.tobecompared", "Zertifikat wird mit Zertifizierungsstellen-Stammzertifikat verglichen:\n{0}" },

	{ "roothttpscertstore.cert.loading", "Zertifizierungsstellen-SSL-Stammzertifikate werden geladen aus {0}" },
	{ "roothttpscertstore.cert.loaded", "Zertifizierungsstellen-SSL-Stammzertifikate wurden geladen aus {0}" },
	{ "roothttpscertstore.cert.noload", "Datei f\u00fcr Zertifizierungsstellen-SSL-Stammzertifikat wurde nicht gefunden: {0}" },
	{ "roothttpscertstore.cert.saving", "Zertifizierungsstellen-SSL-Stammzertifikate werden gespeichert in {0}" },
	{ "roothttpscertstore.cert.saved", "Zertifizierungsstellen-SSL-Stammzertifikate wurden in {0} gespeichert " },
	{ "roothttpscertstore.cert.adding", "Zertifikat wird Zertifizierungsstellen-SSL-Stammzertifikatspeicher hinzugef\u00fcgt" },
	{ "roothttpscertstore.cert.added", "Zertifikat wurde Zertifizierungsstellen-SSL-Stammzertifikatspeicher als Alias {0} hinzugef\u00fcgt" },
	{ "roothttpscertstore.cert.removing", "Zertifikat wird aus Zertifizierungsstellen-SSL-Stammzertifikatspeicher entfernt" },
	{ "roothttpscertstore.cert.removed", "Zertifikat wurde als Alias {0} aus Zertifizierungsstellen-SSL-Stammzertifikatspeicher entfernt " },
	{ "roothttpscertstore.cert.instore", "Zertifikat wird in Zertifizierungsstellen-SSL-Stammzertifikatspeicher gesucht" },
	{ "roothttpscertstore.cert.canverify", "\u00dcberpr\u00fcfung, ob Zertifikat durch Zertifikate in Zertifizierungsstellen-SSL-Stammzertifikatspeicher verifiziert werden kann" },
	{ "roothttpscertstore.cert.iterator", "Zertifikat-Iterator aus Zertifizierungsstellen-SSL-Stammzertifikatspeicher abrufen" },
	{ "roothttpscertstore.cert.getkeystore", "Keystore-Objekt von Zertifizierungsstellen-SSL-Stammzertifikatspeicher abrufen" },
	{ "roothttpscertstore.cert.verify.ok", "Zertifikat wurde erfolgreich durch Zertifizierungsstellen-SSL-Stammzertifikate verifiziert" },
	{ "roothttpscertstore.cert.verify.fail", "Zertifikat konnte durch Zertifizierungsstellen-SSL-Stammzertifikate nicht verifiziert werden" },
	{ "roothttpscertstore.cert.tobeverified", "Zu verifizierendes Zertifikat:\n{0}" },
	{ "roothttpscertstore.cert.tobecompared", "Zertifikat wird mit Zertifizierungsstellen-SSL-Stammzertifikat verglichen:\n{0}" },

	{ "sessioncertstore.cert.loading", "Zertifikate werden aus Bereitstellungssitzungs-Zertifikatspeicher geladen" },
	{ "sessioncertstore.cert.loaded", "Zertifikate wurden aus Bereitstellungssitzungs-Zertifikatspeicher geladen" },
	{ "sessioncertstore.cert.saving", "Zertifikate werden in Bereitstellungssitzungs-Zertifikatspeicher gespeichert" },
	{ "sessioncertstore.cert.saved", "Zertifikate wurden in Bereitstellungssitzungs-Zertifikatspeicher gespeichert" },
	{ "sessioncertstore.cert.adding", "Zertifikat wird Bereitstellungssitzungs-Zertifikatspeicher hinzugef\u00fcgt" },
	{ "sessioncertstore.cert.added", "Zertifikat wurde Bereitstellungssitzungs-Zertifikatspeicher hinzugef\u00fcgt" },
	{ "sessioncertstore.cert.removing", "Zertifikat wird aus Bereitstellungssitzungs-Zertifikatspeicher entfernt" },
	{ "sessioncertstore.cert.removed", "Zertifikat wurde aus Bereitstellungssitzungs-Zertifikatspeicher entfernt" },
	{ "sessioncertstore.cert.instore", "Zertifikat wird in Bereitstellungssitzungs-Zertifikatspeicher gesucht" },
	{ "sessioncertstore.cert.canverify", "\u00dcberpr\u00fcfung, ob Zertifikat durch Zertifikate in Bereitstellungssitzungs-Zertifikatspeicher verifiziert werden kann" },
	{ "sessioncertstore.cert.iterator", "Zertifikat-Iterator aus Bereitstellungssitzungs-Zertifikatspeicher abrufen" },
	{ "sessioncertstore.cert.getkeystore", "Keystore-Objekt von Bereitstellungssitzungs-Zertifikatspeicher abrufen" },

        { "iexplorer.cert.loading", "Zertifikate werden aus Internet Explorer {0}-Zertifikatspeicher geladen" },
        { "iexplorer.cert.loaded", "Zertifikate wurden aus Internet Explorer {0}-Zertifikatspeicher geladen" },
        { "iexplorer.cert.instore", "Zertifikat wird in Internet Explorer {0}-Zertifikatspeicher gesucht" },
        { "iexplorer.cert.canverify", "\u00dcberpr\u00fcfung, ob Zertifikat durch Zertifikate in Internet Explorer {0}Zertifikatspeicher verifiziert werden kann" },
        { "iexplorer.cert.iterator", "Zertifikat-Iterator aus Internet Explorer {0}-Zertifikatspeicher abrufen" },
        { "iexplorer.cert.verify.ok", "Zertifikat wurde erfolgreich durch Internet Explorer {0}-Zertifikate verifiziert" },
        { "iexplorer.cert.verify.fail", "Zertifikat konnte durch Internet Explorer {0}-Zertifikate nicht verifiziert werden" },
        { "iexplorer.cert.tobeverified", "Zu verifizierendes Zertifikat:\n{0}" },
        { "iexplorer.cert.tobecompared", "Zertifikat wird mit Internet Explorer {0}-Zertifikat verglichen:\n{1}" },
        { "mozilla.cert.loading", "Zertifikate werden aus dem Mozilla {0}-Zertifikatspeicher geladen" },
        { "mozilla.cert.loaded", "Zertifikate wurden aus dem Mozilla {0}-Zertifikatspeicher geladen" },
        { "mozilla.cert.instore", "Zertifikat wird im Mozilla {0}-Zertifikatspeicher gesucht" },
        { "mozilla.cert.canverify", "\u00dcberpr\u00fcfung, ob Zertifikat durch Zertifikate im Mozilla {0}-Zertifikatspeicher verifiziert werden kann" },
        { "mozilla.cert.iterator", "Zertifikat-Iterator aus Mozilla {0}-Zertifikatspeicher abrufen" },
        { "mozilla.cert.verify.ok", "Zertifikat wurde erfolgreich durch Mozilla {0}-Zertifikate verifiziert" },
        { "mozilla.cert.verify.fail", "Zertifikat konnte durch Mozilla {0}-Zertifikate nicht verifiziert werden" },
        { "mozilla.cert.tobeverified", "Zu verifizierendes Zertifikat:\n{0}" },
        { "mozilla.cert.tobecompared", "Zertifikat wird mit Mozilla {0}-Zertifikat verglichen:\n{1}" },

        { "browserkeystore.jss.no", "JSS-Package wurde nicht gefunden" },
        { "browserkeystore.jss.yes", "JSS-Package ist geladen" },
        { "browserkeystore.jss.notconfig", "JSS ist nicht konfiguriert" },
        { "browserkeystore.jss.config", "JSS ist konfiguriert" },
        { "browserkeystore.mozilla.dir", "Zugriff auf Schl\u00fcssel und Zertifikat im Mozilla-Benutzerprofil: {0}" },
        { "browserkeystore.password.dialog.buttonOK", "OK" },
        { "browserkeystore.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
        { "browserkeystore.password.dialog.buttonCancel", "Abbrechen" },
        { "browserkeystore.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
        { "browserkeystore.password.dialog.caption", "Passwort erforderlich" },
        { "browserkeystore.password.dialog.text", "Geben Sie das Passwort f\u00fcr {0} ein:\n" },
        { "mozillamykeystore.priv.notfound", "Privater Schl\u00fcssel f\u00fcr Zertifikat {0} wurde nicht gefunden" },

	{ "hostnameverifier.automation.ignoremismatch", "Automatisierung: Hostnamen-Konflikt ignorieren" },

	{ "trustdecider.check.basicconstraints", "Pr\u00fcfung der grundlegenden Einschr\u00e4nkungen im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.leafkeyusage", "Pr\u00fcfung der Verwendung des Blattschl\u00fcssels im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.signerkeyusage", "Pr\u00fcfung der Verwendung des Signiererschl\u00fcssels im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.extensions", "Pr\u00fcfung der kritischen Erweiterungen im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.signature", "Pr\u00fcfung der Signatur im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.basicconstraints.certtypebit", "Pr\u00fcfung des Netscape-Typbits im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.basicconstraints.extensionvalue", "Pr\u00fcfung des Werts f\u00fcr die Netscape-Erweiterung im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.basicconstraints.bitvalue", "Pr\u00fcfung der Werte f\u00fcr die Netscape-Bits 5, 6 und 7 im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.basicconstraints.enduser", "Pr\u00fcfung der Funktion des Endbenutzers als Zertifizierungsstelle im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.basicconstraints.pathlength", "Pr\u00fcfung der Pfadl\u00e4ngenbeschr\u00e4nkungen im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.leafkeyusage.length", "Pr\u00fcfung der L\u00e4nge der Schl\u00fcsselverwendung im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.leafkeyusage.digitalsignature", "Pr\u00fcfung der digitalen Signatur im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.leafkeyusage.extkeyusageinfo", "Pr\u00fcfung der Verwendungsinformationen f\u00fcr den Erweiterungsschl\u00fcssel im Zertifikat fehlgeschlagen" },
        { "trustdecider.check.leafkeyusage.tsaextkeyusageinfo", "Pr\u00fcfung der Verwendungsinformationen f\u00fcr den TSA-Erweiterungsschl\u00fcssel im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.leafkeyusage.certtypebit", "Pr\u00fcfung des Netscape-Typbits im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.signerkeyusage.lengthandbit", "Pr\u00fcfung der L\u00e4nge und des Bits im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.signerkeyusage.keyusage", "Pr\u00fcfung der Schl\u00fcsselverwendung im Zertifikat fehlgeschlagen" },
	{ "trustdecider.check.canonicalize.updatecert", "Stammzertifikat durch Zertifikat in Datei cacerts aktualisieren" },
	{ "trustdecider.check.canonicalize.missing", "Fehlendes Stammzertifikat hinzuf\u00fcgen" },
	{ "trustdecider.check.gettrustedcert.find", "G\u00fcltige Stammzertifizierungsstelle in Datei cacerts suchen" },
	{ "trustdecider.check.gettrustedissuercert.find", "Fehlende g\u00fcltige Stammzertifizierungsstelle in Datei cacerts suchen" },
        { "trustdecider.check.timestamping.no", "Keine Zeitstempelinformationen verf\u00fcgbar" },
        { "trustdecider.check.timestamping.yes", "Zeitstempelinformationen verf\u00fcgbar" },
        { "trustdecider.check.timestamping.tsapath", "Pr\u00fcfung des TSA-Zertifikatpfads starten" },
        { "trustdecider.check.timestamping.inca", "Das Zertifikat ist zwar abgelaufen, tr\u00e4gt aber einen Zeitstempel aus dem g\u00fcltigen Zeitraum und weist g\u00fcltigen TSA auf." },
        { "trustdecider.check.timestamping.notinca", "Das Zertifikat ist abgelaufen, TSA ist aber nicht vertrauensw\u00fcrdig" },
        { "trustdecider.check.timestamping.valid", "Das Zertifikat ist abgelaufen und tr\u00e4gt einen Zeitstempel des g\u00fcltigen Zeitraums" },
        { "trustdecider.check.timestamping.invalid", "Das Zertifikat ist abgelaufen und tr\u00e4gt einen Zeitstempel eines ung\u00fcltigen Zeitraums" },
        { "trustdecider.check.timestamping.need", "Das Zertifikat ist abgelaufen; Zeitstempelinformationen m\u00fcssen \u00fcberpr\u00fcft werden" },
        { "trustdecider.check.timestamping.noneed", "Das Zertifikat ist nicht abgelaufen; Zeitstempelinformationen m\u00fcssen nicht \u00fcberpr\u00fcft werden" },
        { "trustdecider.check.timestamping.notfound", "Die neue Zeitstempel-API kann nicht gefunden werden" },
	{ "trustdecider.user.grant.session", "Berechtigungen f\u00fcr den Code wurden vom Benutzer nur f\u00fcr diese Sitzung gew\u00e4hrt" },
	{ "trustdecider.user.grant.forever", "Berechtigungen f\u00fcr den Code wurden vom Benutzer unbegrenzt gew\u00e4hrt" },
	{ "trustdecider.user.deny", "Benutzer hat keine Berechtigungen f\u00fcr den Code gew\u00e4hrt" },
	{ "trustdecider.automation.trustcert", "Automatisierung: RSA-Zertifikat zum Signieren vertrauen" },
	{ "trustdecider.code.type.applet", "Applet" },
	{ "trustdecider.code.type.application", "Anwendung" },
	{ "trustdecider.code.type.extension", "Erweiterung" },
	{ "trustdecider.code.type.installer", "Installationsprogramm" },
	{ "trustdecider.user.cannot.grant.any", "Ihre Sicherheitskonfiguration l\u00e4sst die Erteilung von Berechtigungen an neue Zertifikate nicht zu" },
	{ "trustdecider.user.cannot.grant.notinca", "Ihre Sicherheitskonfiguration l\u00e4sst die Erteilung von Berechtigungen an selbst signierte Zertifikate nicht zu" },
	{ "x509trustmgr.automation.ignoreclientcert", "Automatisierung: Nicht als vertrauensw\u00fcrdig eingestuftes Client-Zertifikat ignorieren" },
	{ "x509trustmgr.automation.ignoreservercert", "Automatisierung: Nicht als vertrauensw\u00fcrdig eingestuftes Server-Zertifikat ignorieren" },

	{ "net.proxy.text", "Proxy: " },
	{ "net.proxy.override.text", "Proxy-\u00dcberschreibungen: " },
	{ "net.proxy.configuration.text", "Proxy-Konfiguration: " },
	{ "net.proxy.type.browser", "Proxy-Konfiguration des Browsers" },
	{ "net.proxy.type.auto", "Automatische Proxy-Konfiguration" },
	{ "net.proxy.type.manual", "Manuelle Konfiguration" },
	{ "net.proxy.type.none", "Kein Proxy" },
	{ "net.proxy.type.user", "Proxy-Einstellungen des Browsers wurden vom Benutzer \u00fcberschrieben." },
	{ "net.proxy.loading.ie", "Proxy-Konfiguration wird aus Internet Explorer geladen ..."},
	{ "net.proxy.loading.ns", "Proxy-Konfiguration wird aus Netscape Navigator geladen ..."},
	{ "net.proxy.loading.userdef", "Benutzerdefinierte Proxy-Konfiguration wird geladen ..."},
	{ "net.proxy.loading.direct", "Direkte Proxy-Konfiguration wird geladen ..."},
	{ "net.proxy.loading.manual", "Manuelle Proxy-Konfiguration wird geladen ..."},
	{ "net.proxy.loading.auto",   "Automatische Proxy-Konfiguration wird geladen ..."},
	{ "net.proxy.loading.browser",   "Browser-Proxy-Konfiguration wird geladen ..."},
	{ "net.proxy.loading.manual.error", "Manuelle Proxy-Konfiguration kann nicht verwendet werden - R\u00fcckgriff auf DIREKTE"},
	{ "net.proxy.loading.auto.error", "Automatische Proxy-Konfiguration kann nicht verwendet werden - R\u00fcckgriff auf MANUELLE"},
	{ "net.proxy.loading.done", "Fertig."},
	{ "net.proxy.browser.pref.read", "Datei mit Benutzereinstellungen wird geladen von {0}"},
	{ "net.proxy.browser.proxyEnable", "    Proxy aktivieren: {0}"},
	{ "net.proxy.browser.proxyList",     "    Proxy-Liste: {0}"},
	{ "net.proxy.browser.proxyOverride", "    Proxy-\u00dcberschreibungen: {0}"},
	{ "net.proxy.browser.autoConfigURL", "    Autokonfigurations-URL: {0}"},
	{ "net.proxy.browser.smartConfig", "Proxyserver {0} an Port {1} anpingen"},
        { "net.proxy.browser.connectionException", "Proxyserver {0} an Port {1} kann nicht erreicht werden"},
	{ "net.proxy.ns6.regs.exception", "Fehler beim Lesen der Registrierungsdatei: {0}"},
	{ "net.proxy.pattern.convert", "Umwandeln von Proxy-Umgehungsliste in regul\u00e4ren Ausdruck: "},
	{ "net.proxy.pattern.convert.error", "Proxy-Umgehungsliste konnte nicht in regul\u00e4ren Ausdruck umgewandelt werden - Ignorieren"},
	{ "net.proxy.auto.download.ins", "INS-Datei wird heruntergeladen von {0}" },
	{ "net.proxy.auto.download.js", "Automatische Proxy-Datei wird heruntergeladen von {0}" },
	{ "net.proxy.auto.result.error", "Proxy-Einstellungen konnten nicht durch Auswertung bestimmt werden - R\u00fcckgriff auf DIREKTE"},
        { "net.proxy.service.not_available", "Proxy-Dienst nicht verf\u00fcgbar f\u00fcr {0} - R\u00fcckgriff auf DIREKTE" },
	{ "net.proxy.error.caption", "Fehler - Proxy-Konfiguration" },
	{ "net.proxy.nsprefs.error", "<html><b>Proxy-Einstellungen k\u00f6nnen nicht abgerufen werden</b></html>R\u00fcckgriff auf andere Proxy-Konfiguration.\n" },
	{ "net.proxy.connect", "Verbindung von {0} mit Proxy={1} wird hergestellt" },

	{ "net.authenticate.caption", "Passwort erforderlich - Netzwerk"},
	{ "net.authenticate.label", "<html><b>Geben Sie Ihren Benutzernamen und Ihr Passwort ein:</b></html>"},
	{ "net.authenticate.resource", "Ressource:" },
	{ "net.authenticate.username", "Benutzername:" },
        { "net.authenticate.username.mnemonic", "VK_B" },
	{ "net.authenticate.password", "Passwort:" },
        { "net.authenticate.password.mnemonic", "VK_P" },
	{ "net.authenticate.firewall", "Server:" },
	{ "net.authenticate.domain", "Domain:"},
        { "net.authenticate.domain.mnemonic", "VK_D" },
	{ "net.authenticate.realm", "Bereich:" },
	{ "net.authenticate.scheme", "Schema:" },
	{ "net.authenticate.unknownSite", "Unbekannte Site" },

	{ "net.cookie.cache", "Cookie-Cache: " },
	{ "net.cookie.server", "Server {0} sendet Anfrage nach Set-Cookie mit \"{1}\"" },
	{ "net.cookie.connect", "Verbindung {0} mit Cookie \"{1}\"" },
	{ "net.cookie.ignore.setcookie", "Cookie-Service nicht verf\u00fcgbar - ignorieren \"Set-Cookie\"" },
	{ "net.cookie.noservice", "Cookie-Service nicht verf\u00fcgbar - Cache zum Ermitteln von \"Cookie\" verwenden" },

	{"about.java.version", "Version {0} (Build {1})"},
	{"about.prompt.info", "Weitere Informationen \u00fcber die Java-Technologie und hervorragende Java-Anwendungen erhalten Sie unter"},
	{"about.home.link", "http://www.java.com"},
	{"about.option.close", "Schlie\u00dfen"},
	{"about.option.close.acceleratorKey", new Integer(KeyEvent.VK_S)},
        {"about.copyright", "Copyright 2004 Sun Microsystems Inc."},
	{"about.legal.note", "Alle Rechte vorbehalten. Die Verwendung unterliegt den Lizenzbestimmungen."},

	{ "cert.remove_button", "Entfernen" },
        { "cert.remove_button.mnemonic", "VK_N" },
        { "cert.import_button", "Importieren" },
        { "cert.import_button.mnemonic", "VK_I" },
        { "cert.export_button", "Exportieren" },
        { "cert.export_button.mnemonic", "VK_E" },
        { "cert.details_button", "Details" },
        { "cert.details_button.mnemonic", "VK_D" },
        { "cert.viewcert_button", "Zertifikat zeigen" },
        { "cert.viewcert_button.mnemonic", "VK_Z" },
        { "cert.close_button", "Schlie\u00dfen" },
        { "cert.close_button.mnemonic", "VK_S" },
        { "cert.type.trusted_certs", "Vertrauensw\u00fcrdige Zertifikate" },
        { "cert.type.secure_site", "Sichere Site" },
        { "cert.type.client_auth", "Client-Authentifizierung" },
        { "cert.type.signer_ca", "Zertifizierungsstelle f\u00fcr Signierer" },
        { "cert.type.secure_site_ca", "Zertifizierungsstelle f\u00fcr sichere Site" },
        { "cert.rbutton.user", "Benutzer" },
        { "cert.rbutton.system", "System" },
        { "cert.settings", "Zertifikate" },
        { "cert.dialog.import.error.caption", "Fehler - Zertifikat importieren" },
        { "cert.dialog.export.error.caption", "Fehler - Zertifikat exportieren" },
        { "cert.dialog.import.format.text", "<html><b>Unbekanntes Dateiformat</b></html>Es wird kein Zertifikat importiert." },
        { "cert.dialog.export.password.text", "<html><b>Ung\u00fcltiges Passwort</b></html>Das eingegebene Passwort ist falsch." },
        { "cert.dialog.import.file.text", "<html><b>Datei existiert nicht</b></html>Es wird kein Zertifikat importiert." },
        { "cert.dialog.import.password.text", "<html><b>Ung\u00fcltiges Passwort</b></html>Das eingegebene Passwort ist falsch." },
        { "cert.dialog.password.caption", "Passwort" },
        { "cert.dialog.password.import.caption", "Passwort erforderlich - Import" },
        { "cert.dialog.password.export.caption", "Passwort erforderlich - Export" },
        { "cert.dialog.password.text", "Geben Sie ein Passwort f\u00fcr den Zugriff auf diese Datei ein:" },
        { "cert.dialog.exportpassword.text", "Geben Sie ein Passwort f\u00fcr den Zugriff auf diesen privaten Schl\u00fcssel in der Keystore-Datei f\u00fcr die Client-Authentifizierung ein:" },
        { "cert.dialog.savepassword.text", "Geben Sie ein Passwort zum Speichern dieser Schl\u00fcsseldatei ein:" },
        { "cert.dialog.password.okButton", "OK" },
        { "cert.dialog.password.cancelButton", "Abbrechen" },
        { "cert.dialog.export.error.caption", "Fehler - Zertifikat exportieren" },
        { "cert.dialog.export.text", "<html><b>Exportieren nicht m\u00f6glich</b></html>Es wird kein Zertifikat exportiert." },
        { "cert.dialog.remove.text", "Sollen diese Zertifikate wirklich gel\u00f6scht werden?" },
	{ "cert.dialog.remove.caption", "Zertifikat l\u00f6schen" },
	{ "cert.dialog.issued.to", "Ausgestellt f\u00fcr" },
	{ "cert.dialog.issued.by", "Ausgestellt von" },
	{ "cert.dialog.user.level", "Benutzer" },
	{ "cert.dialog.system.level", "System" },
	{ "cert.dialog.certtype", "Zertifikattyp: "},

	{ "controlpanel.jre.platformTableColumnTitle","Plattform"},
	{ "controlpanel.jre.productTableColumnTitle","Produkt" },
	{ "controlpanel.jre.locationTableColumnTitle","Adresse" },
	{ "controlpanel.jre.pathTableColumnTitle","Pfad" },
	{ "controlpanel.jre.enabledTableColumnTitle", "Aktiviert" },

	{ "jnlp.jre.title", "Einstellungen f\u00fcr JNLP Runtime" },
	{ "jnlp.jre.versions", "Java Runtime-Versionen" },
	{ "jnlp.jre.choose.button", "Auswahl" },
	{ "jnlp.jre.find.button", "Suchen" },
	{ "jnlp.jre.add.button", "Hinzuf\u00fcgen" },
	{ "jnlp.jre.remove.button", "Entfernen" },
	{ "jnlp.jre.ok.button", "OK" },
	{ "jnlp.jre.cancel.button", "Abbrechen" },
	{ "jnlp.jre.choose.button.mnemonic", "VK_W" },
	{ "jnlp.jre.find.button.mnemonic", "VK_S" },
	{ "jnlp.jre.add.button.mnemonic", "VK_H" },
	{ "jnlp.jre.remove.button.mnemonic", "VK_E" },
	{ "jnlp.jre.ok.button.mnemonic", "VK_O" },
	{ "jnlp.jre.cancel.button.mnemonic", "VK_A" },

	{ "find.dialog.title", "JRE-Suchdienst"},
	{ "find.title", "Java Runtime Environments"},
	{ "find.cancelButton", "Abbrechen"},
	{ "find.prevButton", "Zur\u00fcck"},
	{ "find.nextButton", "Weiter"},
	{ "find.cancelButtonMnemonic", "VK_A"},
	{ "find.prevButtonMnemonic", "VK_Z"},
	{ "find.nextButtonMnemonic", "VK_W"},
	{ "find.intro", "Damit Java Web Start Anwendungen starten kann, m\u00fcssen die Adressen der installierten Java Runtime Environments bekannt sein.\n\nSie k\u00f6nnen entweder eine bekannte JRE oder ein Verzeichnis im Dateisystem ausw\u00e4hlen, in dem nach JREs gesucht werden soll." },

	{ "find.searching.title", "Verf\u00fcgbare JREs werden gesucht. Dies kann einige Minuten dauern." },
	{ "find.searching.prefix", "Suche in: " },
	{ "find.foundJREs.title", "Die folgenden JREs wurden gefunden. Klicken Sie auf \"Weiter\", um sie hinzuzuf\u00fcgen" },
	{ "find.noJREs.title", "Es kann keine JRE gefunden werden. Klicken Sie auf \"Zur\u00fcck\", um ein anderes Verzeichnis f\u00fcr die Suche auszuw\u00e4hlen" },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Java(tm) Deployment Properties\n"
                        + "# BEARBEITEN SIE DIESE DATEI NICHT. Die Datei wurde vom System generiert.\n"
                        + "# Zum Bearbeiten der Eigenschaften verwenden Sie Java Control Panel." },
        { "config.unknownSubject", "Unbekannter Betreff" },
        { "config.unknownIssuer", "Unbekannter Aussteller" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>Falsches URL-Format</b></html>URL f\u00fcr automatische Proxy-Konfiguration ist ung\u00fcltig." },
        { "config.proxy.autourl.invalid.caption", "Fehler - Proxies" },
	// Java Web Start Properties
	 { "APIImpl.clipboard.message.read", "Diese Anwendung hat Lesezugriff auf die Systemzwischenablage angefordert. Die Anwendung kann unter Umst\u00e4nden in der Zwischenablage gespeicherte vertrauliche Informationen auslesen. M\u00f6chten Sie diesen Vorgang zulassen?" },
        { "APIImpl.clipboard.message.write", "Diese Anwendung hat Schreibzugriff auf die Systemzwischenablage angefordert. Die Anwendung kann unter Umst\u00e4nden in der Zwischenablage gespeicherte Informationen \u00fcberschreiben. M\u00f6chten Sie diesen Vorgang zulassen?" },
        { "APIImpl.file.open.message", "Diese Anwendung hat Lesezugriff auf das Dateisystem angefordert. Die Anwendung kann unter Umst\u00e4nden im Dateisystem gespeicherte vertraulichen Informationen auslesen. M\u00f6chten Sie diesen Vorgang zulassen?" },
        { "APIImpl.file.save.fileExist", "{0} ist bereits vorhanden.\n Soll die Datei ersetzt werden?" },
        { "APIImpl.file.save.fileExistTitle", "Datei vorhanden" },
        { "APIImpl.file.save.message", "Diese Anwendung hat Lese-/Schreibzugriff auf eine Datei im lokalen Dateisystem angefordert. Wenn Sie diesen Vorgang zulassen, wird der Anwendung lediglich der Zugriff auf die im folgenden Dateidialogfeld ausgew\u00e4hlte(n) Datei(en) erm\u00f6glicht. M\u00f6chten Sie diesen Vorgang zulassen?" },
        { "APIImpl.persistence.accessdenied", "URL {0} wurde der Zugriff auf st\u00e4ndigen Speicher verweigert" },
        { "APIImpl.persistence.filesizemessage", "Maximale Dateil\u00e4nge \u00fcberschritten" },
        { "APIImpl.persistence.message", "Diese Anwendung hat zus\u00e4tzlichen Speicherplatz auf der lokalen Festplatte angefordert. Zurzeit betr\u00e4gt der maximal zugeteilte Speicherplatz {1} Byte. Die Anwendung fordert eine Vergr\u00f6\u00dferung dieses Werts auf {0} Byte an. M\u00f6chten Sie diesen Vorgang zulassen?" },
        { "APIImpl.print.message", "Diese Anwendung hat Zugriff auf den Standarddrucker angefordert. Wenn Sie diesen Vorgang zulassen, erh\u00e4lt die Anwendung Schreibzugriff auf den Drucker. M\u00f6chten Sie diesen Vorgang zulassen?" },
	{ "APIImpl.extended.fileOpen.message1", "Diese Anwendung hat Lese-/Schreibzugriff auf die folgenden Dateien im lokalen Dateisystem angefordert:"},
	{ "APIImpl.extended.fileOpen.message2", "Wenn Sie diesen Vorgang zulassen, erh\u00e4lt die Anwendung nur Zugriff auf die oben aufgef\u00fchrten Dateien. M\u00f6chten Sie diesen Vorgang zulassen?"},
        { "APIImpl.securityDialog.no", "Nein" },
        { "APIImpl.securityDialog.remember", "Diese Warnung nicht wieder anzeigen" },
        { "APIImpl.securityDialog.title", "Sicherheitswarnung" },
        { "APIImpl.securityDialog.yes", "Ja" },
        { "Launch.error.installfailed", "Installation fehlgeschlagen" },
        { "aboutBox.closeButton", "Schlie\u00dfen" },
        { "aboutBox.versionLabel", "Version {0} (Build {1})" },
        { "applet.failedToStart", "Applet konnte nicht gestartet werden: {0}" },
        { "applet.initializing", "Applet wird initialisiert" },
        { "applet.initializingFailed", "Applet konnte nicht initialisiert werden: {0}" },
        { "applet.running", "Vorgang l\u00e4uft..." },
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "Windows muss neu gestartet werden, damit die \u00c4nderungen wirksam werden.\n\nSoll Windows jetzt neu gestartet werden?" },
        { "extensionInstall.rebootTitle", "Windows neu starten" },
        { "install.configButton", "Konfigurieren ..." },
        { "install.configMnemonic", "VK_K" },
        { "install.errorInstalling", "Unerwarteter Fehler bei der Installation von Java Runtime Environment, bitte versuchen Sie es noch einmal." },
        { "install.errorRestarting", "Unerwarteter Fehler beim Starten, bitte versuchen Sie es noch einmal." },
        { "install.title", "{0} - Verkn\u00fcpfungen erstellen" },
	{ "install.windows.both.message", "M\u00f6chten Sie Desktop- und Startmen\u00fc-Verkn\u00fcpfungen f\u00fcr\n{0} erstellen?" },
        { "install.gnome.both.message", "M\u00f6chten Sie Desktop- und Anwendungsmen\u00fc-Verkn\u00fcpfungen f\u00fcr\n{0} erstellen?" },
        { "install.desktop.message", "M\u00f6chten Sie Desktop-Verkn\u00fcpfungen f\u00fcr\n{0} erstellen?" },
        { "install.windows.menu.message", "M\u00f6chten Sie Startmen\u00fc-Verkn\u00fcpfungen f\u00fcr\n{0} erstellen?" },
        { "install.gnome.menu.message", "M\u00f6chten Sie Anwendungsmen\u00fc-Verkn\u00fcpfungen f\u00fcr\n{0} erstellen?" },
        { "install.noButton", "Nein" },
        { "install.noMnemonic", "VK_N" },
        { "install.yesButton", "Ja" },
        { "install.yesMnemonic", "VK_J" },
        { "launch.cancel", "Abbrechen" },
        { "launch.downloadingJRE", "JRE {0} wird von {1} angefordert" },
        { "launch.error.badfield", "Das Feld {0} weist einen ung\u00fcltigen Wert auf: {1}" },
        { "launch.error.badfield-signedjnlp", "Das Feld {0} weist einen ung\u00fcltigen Wert in der signierten Startdatei auf: {1}" },
        { "launch.error.badfield.download.https", "Download \u00fcber HTTPS nicht m\u00f6glich" },
        { "launch.error.badfield.https", "F\u00fcr HTTPS-Unterst\u00fctzung ist Java 1.4+ erforderlich" },
        { "launch.error.badjarfile", "Besch\u00e4digte JAR-Datei bei {0}" },
        { "launch.error.badjnlversion", "Nicht unterst\u00fctzte JNLP-Version in Startdatei: {0}. Nur die Versionen 1.0 und 1.5 werden in dieser Version unterst\u00fctzt. Bitte informieren Sie den Anwendungshersteller \u00fcber dieses Problem." },
        { "launch.error.badmimetyperesponse", "Server lieferte fehlerhaften MIME-Typ beim Zugriff auf Ressource: {0} - {1}" },
        { "launch.error.badsignedjnlp", "Signatur der Startdatei konnte nicht verifiziert werden. Die signierte Version stimmt nicht mit der heruntergeladenen Version \u00fcberein." },
        { "launch.error.badversionresponse", "Fehlerhafte Version in Server-Antwort beim Zugriff auf Ressource: {0} - {1}" },
        { "launch.error.canceledloadingresource", "Laden der Ressource {0} wurde vom Benutzer abgebrochen" },
        { "launch.error.category.arguments", "Fehler: ung\u00fcltiges Argument" },
        { "launch.error.category.download", "Download-Fehler" },
        { "launch.error.category.launchdesc", "Startdateifehler" },
        { "launch.error.category.memory", "Fehler: nicht gen\u00fcgend Speicherplatz" },
        { "launch.error.category.security", "Sicherheitsfehler" },
        { "launch.error.category.config", "Systemkonfiguration" },
        { "launch.error.category.unexpected", "Unerwarteter Fehler" },
        { "launch.error.couldnotloadarg", "Angegebene Datei/URL konnte nicht geladen werden: {0}" },
        { "launch.error.errorcoderesponse-known", "Server lieferte Fehlercode {1} ({2}) beim Zugriff auf Ressource: {0}" },
        { "launch.error.errorcoderesponse-unknown", "Server lieferte Fehlercode 99 (unbekannter Fehler) beim Zugriff auf Ressource: {0}" },
        { "launch.error.failedexec", "Java Runtime Environment Version {0} konnte nicht gestartet werden" },
        { "launch.error.failedloadingresource", "Ressource konnte nicht geladen werden: {0}" },
        { "launch.error.invalidjardiff", "Inkrementelles Update nicht anwendbar bei Ressource: {0}" },
        { "launch.error.jarsigning-badsigning", "Signatur konnte nicht verifiziert werden in Ressource: {0}" },
        { "launch.error.jarsigning-missingentry", "Signierter Eintrag fehlt in Ressource: {0}" },
        { "launch.error.jarsigning-missingentryname", "Fehlender signierter Eintrag: {0}" },
        { "launch.error.jarsigning-multicerts", "Mehr als ein Zertifikat zum Signieren von Ressource: {0} verwendet" },
        { "launch.error.jarsigning-multisigners", "Mehr als eine Signatur in Eintrag in Ressource: {0}" },
        { "launch.error.jarsigning-unsignedfile", "Unsignierter Eintrag in Ressource: {0} gefunden" },
        { "launch.error.missingfield", "Das folgende erforderliche Feld fehlt in der Startdatei: {0}" },
        { "launch.error.missingfield-signedjnlp", "Das folgende erforderliche Feld fehlt in der signierten Startdatei: {0}" },
        { "launch.error.missingjreversion", "Es wurde keine JRE-Version in der Startdatei f\u00fcr dieses System gefunden" },
        { "launch.error.missingversionresponse", "Versionsfeld fehlt in Server-Antwort beim Zugriff auf Ressource: {0}" },
        { "launch.error.multiplehostsreferences", "In Ressourcen wird auf mehrere Hosts verwiesen" },
        { "launch.error.nativelibviolation", "Verwendung von systemeigenen Bibliotheken erfordert unbeschr\u00e4nkten Zugang zum System" },
        { "launch.error.noJre", "Die Anwendung hat eine Version der JRE angefordert, die zurzeit nicht auf dem lokalen Host installiert ist. Java Web Start konnte die angeforderte Version nicht automatisch herunterladen und installieren. Die JRE-Version muss manuell installiert werden.\n\n" },
        { "launch.error.wont.download.jre", "Die Anwendung hat eine Version der JRE (Version {0}) angefordert, die zurzeit nicht auf dem lokalen Host installiert ist. Java Web Start hatte keine Erlaubnis, die angeforderte Version automatisch herunterzuladen und zu installieren. Die JRE-Version muss manuell installiert werden." },
        { "launch.error.cant.download.jre", "Die Anwendung hat eine Version der JRE (Version {0}) angefordert, die zurzeit nicht auf dem lokalen Host installiert ist. Java Web Start kann die angeforderte Version nicht automatisch herunterladen und installieren. Die JRE-Version muss manuell installiert werden." },
        { "launch.error.cant.access.system.cache", "Der aktuelle Benutzer hat keine Schreibberechtigung f\u00fcr den System-Cache." },
	{ "launch.error.cant.access.user.cache", "Der aktuelle Benutzer hat keine Schreibberechtigung f\u00fcr den Cache." },
        { "launch.error.noappresources", "F\u00fcr diese Plattform sind keine Anwendungsressourcen angegeben. Bitte wenden Sie sich an den Anwendungshersteller, um sicherzustellen, dass diese Plattform unterst\u00fctzt wird." },
        { "launch.error.nomainclass", "Hauptklasse {0} konnte nicht in {1} gefunden werden" },
        { "launch.error.nomainclassspec", "F\u00fcr die Anwendung wurde keine Hauptklasse angegeben" },
        { "launch.error.nomainjar", "Es wurde keine Haupt-JAR-Datei angegeben." },
        { "launch.error.nonstaticmainmethod", "Die Methode main() muss statisch sein" },
        { "launch.error.offlinemissingresource", "Die Anwendung kann nicht offline ausgef\u00fchrt werden, da nicht alle der ben\u00f6tigten Ressourcen auf den lokalen Host heruntergeladen wurden" },
        { "launch.error.parse", "Die Startdatei konnte nicht analysiert werden. Fehler in Zeile {0, number}." },
        { "launch.error.parse-signedjnlp", "Die signierte Startdatei konnte nicht analysiert werden. Fehler in Zeile {0, number}." },
        { "launch.error.resourceID", "{0}" },
        { "launch.error.resourceID-version", "({0}, {1})" },
        { "launch.error.singlecertviolation", "JAR-Ressourcen in JNLP-Datei sind nicht von demselben Zertifikat signiert" },
        { "launch.error.toomanyargs", "Zu viele Argumente angegeben: {0}" },
        { "launch.error.unsignedAccessViolation", "Nicht signierte Anwendung fordert uneingeschr\u00e4nkten Zugang zum System" },
        { "launch.error.unsignedResource", "Unsignierte Ressource: {0}" },
        { "launch.estimatedTimeLeft", "Gesch\u00e4tzter restlicher Zeitbedarf: {0,number,00}:{1,number,00}:{2,number,00}" },
        { "launch.extensiondownload", "Erweiterungsdeskriptor wird heruntergeladen ({0} verbleibend)" },
        { "launch.extensiondownload-name", "{0}-Deskriptor wird heruntergeladen ({1} verbleibend)" },
        { "launch.initializing", "Initialisierung..." },
        { "launch.launchApplication", "Anwendung wird gestartet..." },
        { "launch.launchInstaller", "Installationsprogramm wird gestartet..." },
        { "launch.launchingExtensionInstaller", "Installationsprogramm wird ausgef\u00fchrt. Bitte warten..." },
        { "launch.loadingNetProgress", "{0} gelesen" },
        { "launch.loadingNetProgressPercent", "{0} von {1} gelesen ({2}%)" },
        { "launch.loadingNetStatus", "{0} von {1} wird geladen" },
        { "launch.loadingResourceFailed", "Laden der Ressource war nicht m\u00f6glich" },
        { "launch.loadingResourceFailedSts", "{0} angefordert" },
        { "launch.patchingStatus", "Teile von {0} werden von {1} geladen" },
        { "launch.progressScreen", "Neueste Version wird gesucht..." },
        { "launch.stalledDownload", "Warten auf Daten..." },
        { "launch.validatingProgress", "Eintr\u00e4ge werden durchsucht ({0}% fertig)" },
        { "launch.validatingStatus", "{0} von {1} wird validiert" },
        { "launcherrordialog.abort", "Abbruch" },
        { "launcherrordialog.abortMnemonic", "VK_A" },
        { "launcherrordialog.brief.continue", "Ausf\u00fchrung kann nicht fortgesetzt werden" },
        { "launcherrordialog.brief.details", "Details" },
        { "launcherrordialog.brief.message", "Die angegebene Anwendung kann nicht gestartet werden." },
        { "launcherrordialog.import.brief.message", "Die angegebene Anwendung kann nicht importiert werden." },
        { "launcherrordialog.brief.messageKnown", "{0} kann nicht gestartet werden." },
        { "launcherrordialog.import.brief.messageKnown", "{0} kann nicht importiert werden." },
        { "launcherrordialog.brief.ok", "Ok" },
        { "launcherrordialog.brief.title", "Java Web Start - {0}" },
        { "launcherrordialog.consoleTab", "Konsole" },
        { "launcherrordialog.errorcategory", "Kategorie: {0}\n\n" },
        { "launcherrordialog.errorintro", "Beim Starten/Ausf\u00fchren der Anwendung ist ein Fehler aufgetreten.\n\n" },
        { "launcherrordialog.import.errorintro", "Beim Importieren der Anwendung ist ein Fehler aufgetreten.\n\n" },
        { "launcherrordialog.errormsg", "{0}" },
        { "launcherrordialog.errortitle", "Titel: {0}\n" },
        { "launcherrordialog.errorvendor", "Hersteller: {0}\n" },
        { "launcherrordialog.exceptionTab", "Ausnahme" },
        { "launcherrordialog.generalTab", "Allgemein" },
        { "launcherrordialog.genericerror", "Unerwartete Ausnahme: {0}" },
        { "launcherrordialog.jnlpMainTab", "Hauptstartdatei" },
        { "launcherrordialog.jnlpTab", "Startdatei" },
        { "launcherrordialog.title", "Java Web Start - {0}" },
        { "launcherrordialog.wrappedExceptionTab", "Gekapselte Ausnahme" },

        { "uninstall.failedMessage", "Anwendung konnte nicht vollst\u00e4ndig deinstalliert werden." },
        { "uninstall.failedMessageTitle", "Deinstallation" },
        { "install.alreadyInstalled", "Es ist bereits eine Verkn\u00fcpfung zu {0} vorhanden. M\u00f6chten Sie trotzdem eine Verkn\u00fcpfung herstellen?" },
        { "install.alreadyInstalledTitle", "Verkn\u00fcpfung herstellen..." },
        { "install.desktopShortcutName", "{0}" },
        { "install.installFailed", "Es konnte keine Verkn\u00fcpfung zu {0} hergestellt werden." },
        { "install.installFailedTitle", "Verkn\u00fcpfung herstellen" },
        { "install.startMenuShortcutName", "{0}" },
	{ "install.startMenuUninstallShortcutName", "{0} deinstallieren" },
        { "install.uninstallFailed", "Die Verkn\u00fcpfungen zu {0} konnten nicht gel\u00f6scht werden. Bitte versuchen Sie es erneut." },
        { "install.uninstallFailedTitle", "Verkn\u00fcpfungen l\u00f6schen" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "Sie k\u00f6nnen dieses Programm nicht ausf\u00fchren, da aus der Datei deployment.config Ihres Systems hervorgeht, dass eine Unternehmenskonfigurationsdatei obligatorisch ist. Die erforderliche: {0} ist jedoch nicht verf\u00fcgbar." },

	// Jnlp Cache Viewer:
	{ "jnlp.viewer.title", "Cache-Anzeigeprogramm f\u00fcr Java-Anwendungen" },
	{ "jnlp.viewer.all", "Alle" },
	{ "jnlp.viewer.type", "{0}" },
	{ "jnlp.viewer.totalSize",  "Gesamtressourcengr\u00f6\u00dfe:  {0}" },
	{ "jnlp.viewer.emptyCache", "Cache f\u00fcr {0} ist leer"},
        { "jnlp.viewer.noCache", "Systemcache ist nicht konfiguriert"},

	{ "jnlp.viewer.remove.btn.mnemonic", "VK_E" },
	{ "jnlp.viewer.launch.offline.btn.mnemonic", "VK_F" },
	{ "jnlp.viewer.launch.online.btn.mnemonic", "VK_N" },

	{ "jnlp.viewer.file.menu.mnemonic", "VK_D" },
	{ "jnlp.viewer.edit.menu.mnemonic", "VK_B" },
	{ "jnlp.viewer.app.menu.mnemonic", "VK_W" },
	{ "jnlp.viewer.view.menu.mnemonic", "VK_A" },
	{ "jnlp.viewer.help.menu.mnemonic", "VK_H" },

	{ "jnlp.viewer.cpl.mi.mnemonic", "VK_C" },
	{ "jnlp.viewer.exit.mi.mnemonic", "VK_B" },

	{ "jnlp.viewer.reinstall.mi.mnemonic", "VK_N" },
	{ "jnlp.viewer.preferences.mi.mnemonic", "VK_E" },

	{ "jnlp.viewer.launch.offline.mi.mnemonic", "VK_F" },
	{ "jnlp.viewer.launch.online.mi.mnemonic", "VK_N" },
	{ "jnlp.viewer.install.mi.mnemonic", "VK_I" },
	{ "jnlp.viewer.uninstall.mi.mnemonic", "VK_D" },
	{ "jnlp.viewer.remove.mi.mnemonic", "VK_E" },
	{ "jnlp.viewer.show.mi.mnemonic", "VK_Z" },
	{ "jnlp.viewer.browse.mi.mnemonic", "VK_S" },

	{ "jnlp.viewer.view.0.mi.mnemonic", "VK_A" },
	{ "jnlp.viewer.view.1.mi.mnemonic", "VK_W" },
	{ "jnlp.viewer.view.2.mi.mnemonic", "VK_P" },
	{ "jnlp.viewer.view.3.mi.mnemonic", "VK_B" },
	{ "jnlp.viewer.view.4.mi.mnemonic", "VK_I" },

        { "jnlp.viewer.view.0", "Alle Typen" },
        { "jnlp.viewer.view.1", "Anwendungen" },
        { "jnlp.viewer.view.2", "Applets" },
        { "jnlp.viewer.view.3", "Bibliotheken" },
        { "jnlp.viewer.view.4", "Installationsprogramme" },

	{ "jnlp.viewer.about.mi.mnemonic", "VK_I" },
	{ "jnlp.viewer.help.java.mi.mnemonic", "VK_S" },
	{ "jnlp.viewer.help.jnlp.mi.mnemonic", "VK_J" },

	{ "jnlp.viewer.remove.btn", "Entfernen" },
	{ "jnlp.viewer.remove.1.btn", "Ausgew\u00e4hlte {0} entfernen" },
	{ "jnlp.viewer.remove.2.btn", "Ausgew\u00e4hlte Eintr\u00e4ge entfernen" },
	{ "jnlp.viewer.uninstall.btn", "Deinstallieren" },
	{ "jnlp.viewer.launch.offline.btn", "Offline starten" },
	{ "jnlp.viewer.launch.online.btn", "Online starten" },

        { "jnlp.viewer.file.menu", "Datei" },
        { "jnlp.viewer.edit.menu", "Bearbeiten" },
        { "jnlp.viewer.app.menu", "Anwendung" },
        { "jnlp.viewer.view.menu", "Ansicht" },
        { "jnlp.viewer.help.menu", "Hilfe" },

	{ "jnlp.viewer.cpl.mi", "Java Control Panel starten" },
	{ "jnlp.viewer.exit.mi", "Beenden" },

	{ "jnlp.viewer.reinstall.mi", "Neuinstallation ..." },
	{ "jnlp.viewer.preferences.mi", "Einstellungen ..." },

	{ "jnlp.viewer.launch.offline.mi", "Offline starten" },
	{ "jnlp.viewer.launch.online.mi", "Online starten" },
	{ "jnlp.viewer.install.mi", "Verkn\u00fcpfungen installieren" },
	{ "jnlp.viewer.uninstall.mi", "Verkn\u00fcpfungen deinstallieren" },
        { "jnlp.viewer.remove.0.mi", "Entfernen" },
	{ "jnlp.viewer.remove.mi", "{0} entfernen" },
	{ "jnlp.viewer.show.mi", "JNLP-Deskriptor zeigen" },
	{ "jnlp.viewer.browse.mi", "Homepage durchsuchen" },

	{ "jnlp.viewer.view.0.mi", "Alle Typen" },
	{ "jnlp.viewer.view.1.mi", "Anwendungen" },
	{ "jnlp.viewer.view.2.mi", "Applets" },
	{ "jnlp.viewer.view.3.mi", "Bibliotheken" },
	{ "jnlp.viewer.view.4.mi", "Installationsprogramme" },

	{ "jnlp.viewer.about.mi", "Anwendungsinfo" },
	{ "jnlp.viewer.help.java.mi", "J2SE-Homepage" },
	{ "jnlp.viewer.help.jnlp.mi", "JNLP-Homepage" },

        { "jnlp.viewer.app.column", "Anwendung" },
        { "jnlp.viewer.vendor.column", "Hersteller" },
        { "jnlp.viewer.type.column", "Typ" },
        { "jnlp.viewer.size.column", "Gr\u00f6\u00dfe" },
        { "jnlp.viewer.date.column", "Datum" },
        { "jnlp.viewer.status.column", "Status" },

        { "jnlp.viewer.app.column.tooltip", "Symbol und Titel f\u00fcr diese Anwendung, Erweiterung oder dieses Applet" },
        { "jnlp.viewer.vendor.column.tooltip", "Die dieses Objekt bereitstellende Firma" },
        { "jnlp.viewer.type.column.tooltip", "Typ dieses Objekts" },
        { "jnlp.viewer.size.column.tooltip", "Gr\u00f6\u00dfe dieses Objekts und all seiner Ressourcen" },
        { "jnlp.viewer.date.column.tooltip", "Datum der letzten Ausf\u00fchrung dieser Anwendung bzw. dieses Applets oder Installationsprogramms" },
        { "jnlp.viewer.status.column.tooltip", "Symbol zur Anzeige der Aufrufart und -f\u00e4higkeit dieses Objekts" },

        { "jnlp.viewer.application", "Anwendung" },
        { "jnlp.viewer.applet", "Applet" },
        { "jnlp.viewer.extension", "Bibliothek" },
        { "jnlp.viewer.installer", "Installationsprogramm" },

	{ "jnlp.viewer.offline.tooltip",
		 "Diese/s {0} kann online oder offline aufgerufen werden" },
	{ "jnlp.viewer.online.tooltip", "Diese/s {0} kann online aufgerufen werden" },
        { "jnlp.viewer.norun1.tooltip", 
                "Diese/s {0} kann nur aus einem Webbrowser aufgerufen werden" },
        { "jnlp.viewer.norun2.tooltip", "Erweiterungen k\u00f6nnen nicht aufgerufen werden" },

	{ "jnlp.viewer.show.title", "JNLP-Deskriptor f\u00fcr: {0}" },

	{ "jnlp.viewer.removing", "Entfernen ..." },
	{ "jnlp.viewer.launching", "Starten ..." },
	{ "jnlp.viewer.browsing", "Browser wird gestartet ..." },
	{ "jnlp.viewer.sorting", "Eintr\u00e4ge werden sortiert ..." },
	{ "jnlp.viewer.searching", "Eintr\u00e4ge werden gesucht ..." },
        { "jnlp.viewer.installing", "Installation ..." },

        { "jnlp.viewer.reinstall.title", "Entfernte JNLP-Anwendungen neu installieren"
},
  	{ "jnlp.viewer.reinstallBtn", "Ausgew\u00e4hlte Anwendungen neu installieren" },
	{ "jnlp.viewer.reinstallBtn.mnemonic", "VK_A" },
	{ "jnlp.viewer.closeBtn", "Schlie\u00dfen" },
	{ "jnlp.viewer.closeBtn.mnemonic", "VK_S" },

	{ "jnlp.viewer.reinstall.column.title", "Titel:" },
	{ "jnlp.viewer.reinstall.column.location", "Adresse:" },

	// cache size warning
	{ "jnlp.cache.warning.title", "Warnung \u00fcber JNLP-Cache-Gr\u00f6\u00dfe" },
	{ "jnlp.cache.warning.message", "Warnung: \n\n"+
		"Die empfohlene Speicherplatzgr\u00f6\u00dfe f\u00fcr JNLP-Anwendungen\n"+
		"und Ressourcen im Cache wurde \u00fcberstiegen.\n\n"+
		"Derzeit werden genutzt: {0}\n"+
		"Der empfohlene Grenzwert ist: {1}\n\n"+
		"Bitte greifen Sie auf Java Control Panel zur\u00fcck, um einige \n"+
		"Anwendungen oder Ressourcen zu entfernen oder den Grenzwert zu erh\u00f6hen." },

        // Control Panel
        { "control.panel.title", "Java Control Panel" },
        { "control.panel.general", "Allgemein" },
        { "control.panel.security", "Sicherheit" },
        { "control.panel.java", "Java" },
        { "control.panel.update", "Aktualisierung" },
        { "control.panel.advanced", "Erweitert" },

        // Common Strings used in different panels.
        { "common.settings", "Einstellungen" },
        { "common.ok_btn", "OK" },
        { "common.ok_btn.mnemonic", "VK_O" },
        { "common.cancel_btn", "Abbrechen" },
        { "common.cancel_btn.mnemonic", "VK_A" },
        { "common.apply_btn", "Anwenden" },
        { "common.apply_btn.mnemonic", "VK_W" },
        { "common.add_btn", "Hinzuf\u00fcgen" },
        { "common.add_btn.mnemonic", "VK_H" },
        { "common.remove_btn", "Entfernen" },
        { "common.remove_btn.mnemonic", "VK_E" },

        // Network Settings Dialog
        { "network.settings.dlg.title", "Netzwerkeinstellungen" },
        { "network.settings.dlg.border_title", " Proxy-Einstellungen f\u00fcr das Netzwerk " },
        { "network.settings.dlg.browser_rbtn", "Browser-Einstellungen verwenden" },
        { "browser_rbtn.mnemonic", "VK_R" },
        { "network.settings.dlg.manual_rbtn", "Proxyserver verwenden" },
        { "manual_rbtn.mnemonic", "VK_P" },
        { "network.settings.dlg.address_lbl", "Adresse:" },
        { "network.settings.dlg.port_lbl", "Port:" },
        { "network.settings.dlg.advanced_btn", "Erweitert..." },
        { "network.settings.dlg.advanced_btn.mnemonic", "VK_E" },
        { "network.settings.dlg.bypass_text", "Proxyserver f\u00fcr lokale Adressen umgehen" },
        { "network.settings.dlg.bypass.mnemonic", "VK_U" },
        { "network.settings.dlg.autoconfig_rbtn", "Skript f\u00fcr automatische Proxy-Konfiguration verwenden" },
        { "autoconfig_rbtn.mnemonic", "VK_T" },
        { "network.settings.dlg.location_lbl", "Skript-Adresse: " },
        { "network.settings.dlg.direct_rbtn", "Direktverbindung" },
        { "direct_rbtn.mnemonic", "VK_D" },
        { "network.settings.dlg.browser_text", "Die automatische Konfiguration kann unter Umst\u00e4nden manuelle Einstellungen \u00fcberschreiben. Wenn manuelle Einstellungen verwendet werden sollen, deaktivieren Sie die automatische Konfiguration." },
        { "network.settings.dlg.proxy_text", "Proxy-Einstellungen des Browsers \u00fcberschreiben." },
        { "network.settings.dlg.auto_text", "Skript f\u00fcr die automatische Proxy-Konfiguration an angegebener Adresse verwenden." },
        { "network.settings.dlg.none_text", "Direktverbindung verwenden." },

        // Advanced Network Settings Dialog
        { "advanced.network.dlg.title", "Erweiterte Netzwerkeinstellungen" },
        { "advanced.network.dlg.servers", " Server " },
        { "advanced.network.dlg.type", "Typ" },
        { "advanced.network.dlg.http", "HTTP:" },
        { "advanced.network.dlg.secure", "Sicher:" },
        { "advanced.network.dlg.ftp", "FTP:" },
        { "advanced.network.dlg.socks", "Socks:" },
        { "advanced.network.dlg.proxy_address", "Proxy-Adresse" },
        { "advanced.network.dlg.port", "Port" },
        { "advanced.network.dlg.same_proxy", " Denselben Proxyserver f\u00fcr alle Protokolle verwenden" },
        { "advanced.network.dlg.same_proxy.mnemonic", "VK_D" },
        { "advanced.network.dlg.exceptions", " Ausnahmen " },
        { "advanced.network.dlg.no_proxy", " Keinen Proxyserver verwenden f\u00fcr Adressen mit dem Anfang" },
        { "advanced.network.dlg.no_proxy_note", " Trennen Sie mehrere Eintr\u00e4ge mit Strichpunkten (;) voneinander." },

        // DeleteFilesDialog
        { "delete.files.dlg.title", "Tempor\u00e4re Dateien l\u00f6schen" },
        { "delete.files.dlg.temp_files", "Die folgenden tempor\u00e4ren Dateien l\u00f6schen?" },
        { "delete.files.dlg.applets", "Applets herunterladen" },
        { "delete.files.dlg.applications", "Anwendungen herunterladen" },
        { "delete.files.dlg.other", "Sonstige Dateien" },

	// General
	{ "general.cache.border.text", " Tempor\u00e4re Internet-Dateien " },
	{ "general.cache.delete.text", "Dateien l\u00f6schen..." },
        { "general.cache.delete.text.mnemonic", "VK_D" },
	{ "general.cache.settings.text", "Einstellungen..." },
        { "general.cache.settings.text.mnemonic", "VK_E" },
	{ "general.cache.desc.text", "Dateien, die Sie in Java-Anwendungen verwenden, werden in einem speziellen Ordner gespeichert, um sp\u00e4ter schneller wieder aufgerufen werden zu k\u00f6nnen. Nur erfahrene Benutzer sollten Dateien l\u00f6schen oder diese Einstellungen \u00e4ndern." },
	{ "general.network.border.text", " Netzwerkeinstellungen " },
	{ "general.network.settings.text", "Netzwerkeinstellungen..." },
        { "general.network.settings.text.mnemonic", "VK_N" },
	{ "general.network.desc.text", "Die Netzwerkeinstellungen werden beim Aufbau einer Internet-Verbindung verwendet. Java verwendet standardm\u00e4\u00dfig die Netzwerkeinstellungen Ihres Webbrowsers. Diese Einstellungen sollten nur erfahrene Benutzer \u00e4ndern." },
        { "general.about.border", "Anwendungsinfo" },
	{ "general.about.btn", "Anwendungsinfo..."},
	{ "general.about.btn.mnemonic", "VK_I" },
	{ "general.about.text", "Versionsinformationen zu Java Control Panel anzeigen." },

	// Security
	{ "security.certificates.border.text", " Zertifikate " },
	{ "security.certificates.button.text", "Zertifikate..." },
        { "security.certificates.button.mnemonic", "VK_E" },
	{ "security.certificates.desc.text", "Mit Zertifikaten k\u00f6nnen Sie sich selbst, andere Zertifikate, Zertifizierungsstellen und Aussteller eindeutig ausweisen." },
	{ "security.policies.border.text", " Richtlinien " },
	{ "security.policies.advanced.text", "Erweitert..." },
        { "security.policies.advanced.mnemonic", "VK_E" },
	{ "security.policies.desc.text", "Mit Sicherheitsrichtlinien bestimmen Sie den Schutz f\u00fcr Anwendungen und Applets." },

	// Update
	{ "update.notify.border.text", " Benachrichtigung \u00fcber Aktualisierungen " }, // this one is not currently used.  See update panel!!!
	{ "update.updatenow.button.text", "Jetzt aktualisieren" },
	{ "update.updatenow.button.mnemonic", "VK_J" },
	{ "update.advanced.button.text", "Erweitert..." },
	{ "update.advanced.button.mnemonic", "VK_E" },
	{ "update.desc.text", "Java-Update gew\u00e4hrleistet, dass Sie stets \u00fcber die neueste Version der Java-Plattform verf\u00fcgen. Mit den nachfolgenden Optionen k\u00f6nnen Sie festlegen, wie Aktualisierungen abgerufen und angewendet werden." },
        { "update.notify.text", "Benachrichtigung ausgeben:" },
        { "update.notify_install.text", "Vor der Installation" },
        { "update.notify_download.text", "Vor dem Herunterladen und vor der Installation" },
        { "update.autoupdate.text", "Automatisch nach Aktualisierungen suchen" },
        { "update.advanced_title.text", "Erweiterte Einstellungen f\u00fcr die automatische Aktualisierung" },
        { "update.advanced_title1.text", "Geben Sie an, wie h\u00e4ufig und wann die Suche stattfinden soll." },
        { "update.advanced_title2.text", "H\u00e4ufigkeit" },
        { "update.advanced_title3.text", "Wann" },
        { "update.advanced_desc1.text", "Suche jeden Tag um {0} durchf\u00fchren" },
        { "update.advanced_desc2.text", "Suche jeden {0} um {1} durchf\u00fchren" },
        { "update.advanced_desc3.text", "Suche am {0} jedes Monats um {1} durchf\u00fchren" },
        { "update.check_daily.text", "T\u00e4glich" },
        { "update.check_weekly.text", "W\u00f6chentlich" },
        { "update.check_monthly.text", "Monatlich" },
        { "update.check_date.text", "Tag:" },
        { "update.check_day.text", "Jeden:" },
        { "update.check_time.text", "Um:" },
        { "update.lastrun.text", "Java-Update wurde zuletzt am {0} um {1} ausgef\u00fchrt." },
        { "update.desc_autooff.text", "Klicken Sie unten auf \"Jetzt aktualisieren\", um die Suche nach Aktualisierungen zu starten. Wenn eine Aktualisierung verf\u00fcgbar ist, wird in der Taskleiste ein Symbol eingeblendet. Den Status der Aktualisierung sehen Sie, wenn Sie den Mauszeiger \u00fcber das Symbol setzen." },
        { "update.desc_check_daily.text", "Java-Update sucht jeden Tag um {0} nach Aktualisierungen. " },
        { "update.desc_check_weekly.text", "Java-Update sucht jeden {0} um {1} nach Aktualisierungen. " },
        { "update.desc_check_monthly.text", "Java-Update sucht am {0} jedes Monats um {1} nach Aktualisierungen. " },
        { "update.desc_systrayicon.text", "Wenn eine Aktualisierung verf\u00fcgbar ist, wird in der Taskleiste ein Symbol eingeblendet. Den Status der Aktualisierung sehen Sie, wenn Sie den Mauszeiger \u00fcber das Symbol setzen. " },
        { "update.desc_notify_install.text", "Sie werden vor der Installation der Aktualisierung benachrichtigt." },
        { "update.desc_notify_download.text", "Sie werden vor dem Herunterladen und vor der Installation der Aktualisierung benachrichtigt." },
	{ "update.launchbrowser.error.text", "Java Update Checker kann nicht gestartet werden. Bitte laden Sie die neueste Version von Java-Update von http://java.sun.com/getjava/javaupdate herunter" },
	{ "update.launchbrowser.error.caption", "Fehler - Aktualisierung" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "Dateien l\u00f6schen..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_D" },
        { "cache.settings.dialog.view_jws_btn", "Anwendungen anzeigen..." },
        { "cache.settings.dialog.view_jws_btn.mnemonic", "VK_N" },
        { "cache.settings.dialog.view_jpi_btn", "Applets anzeigen..." },
        { "cache.settings.dialog.view_jpi_btn.mnemonic", "VK_P" },
        { "cache.settings.dialog.chooser_title", "Adresse tempor\u00e4rer Dateien" },
        { "cache.settings.dialog.select", "Auswahl" },
        { "cache.settings.dialog.select_tooltip", "Ausgew\u00e4hlte Adresse verwenden" },
        { "cache.settings.dialog.select_mnemonic", "S" },
        { "cache.settings.dialog.title", "Einstellungen f\u00fcr tempor\u00e4re Dateien" },
        { "cache.settings.dialog.cache_location", "Adresse:" },
        { "cache.settings.dialog.change_btn", "\u00c4ndern..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_R" },
        { "cache.settings.dialog.disk_space", "Zu verwendender Speicherplatz:" },
        { "cache.settings.dialog.unlimited_btn", "Unbegrenzt" },
        { "cache.settings.dialog.max_btn", "Maximum" },
        { "cache.settings.dialog.compression", "Jar-Kompression:" },
        { "cache.settings.dialog.none", "Keine" },
        { "cache.settings.dialog.high", "Hoch" },

	// JNLP File/MIME association dialog strings:
	{ "javaws.association.dialog.title", "JNLP-Datei/MIME-Zuordnung" },
        { "javaws.association.dialog.exist.command", "ist bereits vorhanden mit:\n{0}"},
	{ "javaws.association.dialog.exist", "ist bereits vorhanden." },
        { "javaws.association.dialog.askReplace", "\nSoll stattdessen {0} zur Behandlung verwendet werden?"},
	{ "javaws.association.dialog.ext", "Dateierweiterungen: {0}" },
        { "javaws.association.dialog.mime", "MIME-Typ: {0}" },
        { "javaws.association.dialog.ask", "M\u00f6chten Sie {0} zur Behandlung verwenden von:" },
        { "javaws.association.dialog.existAsk", "WARNUNG! Zuordnung zu:"},

        // Advanced panel strings:
        { "deployment.console.startup.mode", "Java-Konsole" },
        { "deployment.console.startup.mode.SHOW", "Konsole einblenden" },
        { "deployment.console.startup.mode.SHOW.tooltip", "<html>" +
                                                          "Java Console wird in ganzer Gr\u00f6\u00dfe  gestartet" +
                                                          "</html>" },
        { "deployment.console.startup.mode.HIDE", "Konsole ausblenden" },
        { "deployment.console.startup.mode.HIDE.tooltip", "<html>" +
                                                          "Java Console wird minimiert gestartet" +
                                                          "</html>" },
        { "deployment.console.startup.mode.DISABLE", "Konsole nicht starten" },
        { "deployment.console.startup.mode.DISABLE.tooltip", "<html>" +
                                                             "Java Console wird nicht gestartet" +
                                                             "</html>" },
        { "deployment.trace", "Tracing aktivieren" },
        { "deployment.trace.tooltip", "<html>" +
          "Erzeugt eine Trace-Datei f\u00fcr" +
          "<br>Debugging-Zwecke" +
          "</html>" },
        { "deployment.log", "Protokollierung aktivieren" },
	{ "deployment.log.tooltip", "<html>" +
                                    "Erzeugt eine Protokolldatei zur" +
                                    "<br>Aufzeichnung von Fehlern" +
                                    "</html>" },
        { "deployment.control.panel.log", "Protokollausgabe in Steuerbereich" },
        { "deployment.javapi.lifecycle.exception", "Lebenszyklusausnahmen des Applets anzeigen" },
        { "deployment.javapi.lifecycle.exception.tooltip", "<html>" +
                                    "Dialogfeld mit Ausnahmen anzeigen," +
                                    "<br>falls beim Laden des Applets Fehler auftreten" +
                                    "<html>" },
        { "deployment.browser.vm.iexplorer", "Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
          "Bei APPLET-Tags im Browser Internet Explorer" +
          "<br>Sun Java verwenden" +
          "</html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla und Netscape" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
          "Bei APPLET-Tags in Mozilla- oder" +
          "<br>Netscape-Browsern Sun Java verwenden" +
          "</html>" },
        { "deployment.console.debugging", "Debugging" },
	{ "deployment.browsers.applet.tag", "<APPLET>-Tag-Unterst\u00fctzung" },
        { "deployment.javaws.shortcut", "Erstellung von Verkn\u00fcpfungen" },
        { "deployment.javaws.shortcut.ALWAYS", "Immer zulassen" },
        { "deployment.javaws.shortcut.ALWAYS.tooltip", "<html>" +
          "Es werden immer Verkn\u00fcpfungen erzeugt" +
          "</html>" },
        { "deployment.javaws.shortcut.NEVER" , "Nie zulassen" },
        { "deployment.javaws.shortcut.NEVER.tooltip", "<html>" +
          "Es werden keine Verkn\u00fcpfungen erzeugt" +
          "</html>" },
        { "deployment.javaws.shortcut.ASK_USER", "Benutzer fragen" },
        { "deployment.javaws.shortcut.ASK_USER.tooltip", "<html>" +
          "Benutzer wird gefragt, ob eine" +
          "<br>Verkn\u00fcpfung erzeugt werden soll" +
          "</html>" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED", "Bei Hervorhebung immer zulassen" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED.tooltip", "<html>" +
          "Bei Anforderung durch JNLP-Anwendung" +
          "<br>werden immer Verkn\u00fcpfungen erzeugt" +
          "</html>" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED", "Bei Hervorhebung Benutzer fragen" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED.tooltip", "<html>" +
          "Bei Anforderung durch JNLP-Anwendung" +
          "<br>wird der Benutzer gefragt, ob" +
          "<br>Verkn\u00fcpfungen erzeugt werden sollen" +
          "</html>" },
	{ "deployment.javaws.associations.NEVER", "Nie zulassen" },
        { "deployment.javaws.associations.NEVER.tooltip", "<html>" +
          "Es erfolgen keine Zuordnungen zwischen" +
          "<br>Dateierweiterungen und MIME" +
          "</html>" },
        { "deployment.javaws.associations.ASK_USER", "Benutzer fragen" },
        { "deployment.javaws.associations.ASK_USER.tooltip", "<html>" +
          "Benutzer wird gefragt, ob Zuordnungen zwischen" +
          "<br>Dateierweiterung und MIME erfolgen sollen" +
          "</html>" },
        { "deployment.javaws.associations.REPLACE_ASK", "Zur Ersetzung Benutzer fragen" },
        { "deployment.javaws.associations.REPLACE_ASK.tooltip", "<html>" +
          "Benutzer wird nur beim Ersetzen einer" +
          "<br>vorhandenen Zuordnung zwischen Dateierweiterung" +
          "<br>und MIME gefragt" +
          "</html>" },
        { "deployment.javaws.associations.NEW_ONLY", "Neue Zuordnungen zulassen" },
        { "deployment.javaws.associations.NEW_ONLY.tooltip", "<html>" +
          "Es erfolgen stets nur neue Zuordnungen" +
          "<br>zwischen Dateierweiterungen und MIME" +
          "</html>" },
        { "deployment.javaws.associations", "JNLP-Datei/MIME-Zuordnung" },
        { "deployment.security.settings", "Sicherheit" },
        { "deployment.security.askgrantdialog.show", "Benutzer darf Berechtigungen f\u00fcr signierte Inhalte erteilen" },
        { "deployment.security.askgrantdialog.notinca", "Benutzer darf Berechtigungen f\u00fcr Inhalte von nicht vertrauensw\u00fcrdigen Zertifizierungsstellen erteilen" },
        { "deployment.security.browser.keystore.use", "Zertifikate und Schl\u00fcssel in Browser-Keystore verwenden" },
        { "deployment.security.notinca.warning", "Warnung, wenn Zertifikat nicht in Stamm-Zertifizierungsstelle vorhanden ist" },
        { "deployment.security.expired.warning", "Warnung bei abgelaufenen oder noch nicht g\u00fcltigen Zertifikaten" },
        { "deployment.security.jsse.hostmismatch.warning", "Warnung, wenn Site-Zertifikat nicht mit Host-Namen \u00fcbereinstimmt" },
        { "deployment.security.sandbox.awtwarningwindow", "Sandbox-Warnbanner zeigen" },
        { "deployment.security.sandbox.jnlp.enhanced", "Annahme von JNLP-Sicherheitsanfragen durch Benutzer zulassen" },
        { "deploy.advanced.browse.title", "W\u00e4hlen Sie eine Datei zum Starten des Standardbrowsers" },
        { "deploy.advanced.browse.select", "Auswahl" },
        { "deploy.advanced.browse.select_tooltip", "Ausgew\u00e4hlte Datei zum Starten des Browsers verwenden" },
        { "deploy.advanced.browse.select_mnemonic", "S" },
        { "deploy.advanced.browse.browse_btn", "Durchsuchen..." },
        { "deploy.advanced.browse.browse_btn.mnemonic", "VK_D" },
        { "deployment.browser.default", "Befehl zum Starten des Standardbrowsers" },
        { "deployment.misc.label", "Diverses" },
        { "deployment.system.tray.icon", "Java-Symbol in Taskleiste anzeigen" },
	{ "deployment.system.tray.icon.tooltip", "<html>" +
                             "Ist diese Option aktiviert, wird die" +
                             "<br>Java-Kaffeetasse in der Taskleiste" +
                             "<br>angezeigt, wenn im Browser Java ausgef\u00fchrt wird" +
                             "</html>" },

        //PluginJresDialog strings:
        { "jpi.jres.dialog.title", "Java Runtime-Einstellungen" },
        { "jpi.jres.dialog.border", " Java Runtime-Versionen " },
        { "jpi.jres.dialog.column1", "Produktname" },
        { "jpi.jres.dialog.column2", "Version" },
        { "jpi.jres.dialog.column3", "Adresse" },
        { "jpi.jres.dialog.column4", "Java Runtime-Parameter" },
        { "jpi.jdk.string", "JDK" },
        { "jpi.jre.string", "JRE" },
        { "jpi.jres.dialog.product.tooltip", "W\u00e4hlen Sie JRE oder JDK als Produktnamen" },

        // AboutDialog strings:
        { "about.dialog.title", "Anwendungsinfo Java" },

        // JavaPanel strings:
        { "java.panel.plugin.border", " Java-Applet-Laufzeiteinstellungen " },
        { "java.panel.plugin.text", "Die Laufzeiteinstellungen werden bei der Ausf\u00fchrung eines Applets im Browser verwendet." },
        { "java.panel.jpi_view_btn", "Anzeigen..." },
        { "java.panel.javaws_view_btn", "Anzeigen..." },
        { "java.panel.jpi_view_btn.mnemonic", "VK_N" },
        { "java.panel.javaws_view_btn.mnemonic", "VK_I" },
        { "java.panel.javaws.border", " Laufzeiteinstellungen f\u00fcr Java-Anwendungen "},
        { "java.panel.javaws.text", "Die Laufzeiteinstellungen werden beim Starten von Java-Anwendungen oder -Applets per Java Network Launching Protocol (JNLP) verwendet." },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "<html><b>Neuere Version von Java Runtime vorhanden</b></html>Internet Explorer verf\u00fcgt bereits \u00fcber eine neuere Version von Java Runtime. M\u00f6chten Sie diese ersetzen?\n" },
        { "browser.settings.success.caption", "Erfolgreich - Browser" },
        { "browser.settings.success.text", "<html><b>Browser-Einstellungen ge\u00e4ndert</b></html>Die \u00c4nderungen werden nach einem Neustart des Browsers g\u00fcltig.\n" },
        { "browser.settings.fail.caption", "Warnung - Browser" },
        { "browser.settings.fail.moz.text", "<html><b>\u00c4nderung der Browser-Einstellungen nicht m\u00f6glich</b></html>"
                                        + "Vergewissern Sie sich bitte, dass Mozilla bzw. Netscape richtig auf dem System installiert ist "
                                        + "und dass Sie \u00fcber "
                                        + "die Berechtigung zum \u00c4ndern von Systemeinstellungen verf\u00fcgen.\n" },
        { "browser.settings.fail.ie.text", "<html><b>\u00c4nderung der Browser-Einstellungen nicht m\u00f6glich</b></html>\u00dcberpr\u00fcfen Sie bitte, ob Sie "
					+ "\u00fcber die Berechtigung zum \u00c4ndern von Systemeinstellungen verf\u00fcgen.\n" },


        // Tool tip strings.
        { "cpl.ok_btn.tooltip", "<html>" +
                                "Schlie\u00dft Java Control Panel und speichert" +
                                "<br>alle vorgenommenen \u00c4nderungen" +
                                "</html>" },
        { "cpl.apply_btn.tooltip",  "<html>" +
                                    "Speichert alle vorgenommenen \u00c4nderungen," +
                                    "<br>ohne Java Control Panel zu schlie\u00dfen" +
                                    "</html>" },
        { "cpl.cancel_btn.tooltip", "<html>" +
                                    "Schlie\u00dft Java Control Panel, ohne etwaige" +
                                    "<br>\u00c4nderungen zu speichern" +
                                    "</html>" },

        {"network.settings.btn.tooltip", "<html>"+
                                         "Dient zum \u00c4ndern der Einstellungen" +
                                         "<br>f\u00fcr die Internetverbindung"+
                       	                 "</html>"},

        {"temp.files.settings.btn.tooltip", "<html>"+
                                            "Dient zum \u00c4ndern der" +
                                            "<br>Einstellungen f\u00fcr tempor\u00e4re Dateien" +
                                            "</html>"},

        {"temp.files.delete.btn.tooltip", "<html>" +  // body bgcolor=\"#FFFFCC\">"+
                                          "Dient zum L\u00f6schen von tempor\u00e4ren Java-Dateien" +
                                          "</html>"},

        {"delete.files.dlg.applets.tooltip", "<html>" +
                                          "Aktivieren Sie diese Option, um alle von Java-Applets" +
                                          "<br>erzeugten tempor\u00e4ren Dateien zu l\u00f6schen" +
                                          "</html>" },

        {"delete.files.dlg.applications.tooltip", "<html>" +
                                          "Aktivieren Sie diese Option, um alle von" +
                                          "<br>Java Web Start-Anwendungen erzeugten tempor\u00e4ren" +
                                          "<br>Dateien zu l\u00f6schen" +
                                          "</html>" },

        {"delete.files.dlg.other.tooltip", "<html>" +
                                          "Aktivieren Sie diese Option, um alle sonstigen" +
                                          "<br>von Java erzeugten tempor\u00e4ren Dateien zu l\u00f6schen" +
                                          "</html>" },

        {"delete.files.dlg.temp_files.tooltip", "<html>" +
                                          "Java-Anwendungen speichern mitunter tempor\u00e4re" +
                                          "<br>Dateien auf Ihrem System. Aus" +
                                          "<br>Sicherheitsgr\u00fcnden sollten Sie diese Dateien l\u00f6schen." +
                                          "<br>" +
                                          "<p>Der erste Start einiger Java-Anwendungen nach" +
                                          "<br>dem L\u00f6schen der tempor\u00e4ren Dateien kann unter" +
                                          "<br>Umst\u00e4nden l\u00e4nger dauern als gewohnt." +
                                          "</html>" },

        {"cache.settings.dialog.view_jws_btn.tooltip", "<html>" +
                                          "Zeigt die von Java Web Start-Anwendungen erzeugten" +
                                          "<br>tempor\u00e4ren Dateien an" +
                                          "</html>" },

        {"cache.settings.dialog.view_jpi_btn.tooltip", "<html>" +
                                          "Zeigt die von Java-Applets erzeugten tempor\u00e4ren" +
                                          "<br>Dateien an" +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "Geben Sie das Verzeichnis an, in dem" +
                                          "<br>tempor\u00e4re Dateien gespeichert werden sollen" +
                                          "</html>" },

        {"cache.settings.dialog.unlimited_btn.tooltip", "<html>" +
                                          "Es wird keine Speicherplatzbegrenzung f\u00fcr die" +
                                          "<br>Speicherung der tempor\u00e4ren Dateien angegeben" +
                                          "</html>" },

        {"cache.settings.dialog.max_btn.tooltip", "<html>" +
                                          "Sie geben an, wie viel Speicherplatz maximal f\u00fcr die" +
                                          "<br>Speicherung der tempor\u00e4ren Dateien verwendet werden darf" +
                                          "</html>" },

        {"cache.settings.dialog.compression.tooltip", "<html>" +
                                          "Geben Sie die Kompressionsrate f\u00fcr die von" +
                                          "<br>Java-Programmen in dem Verzeichnis f\u00fcr" +
                                          "<br>tempor\u00e4re Dateien gespeicherten JAR-Dateien an." +
                                          "<br>" +
                                          "<p>Mit \"Keine\" starten die Java-Programme zwar" +
                                          "<br>schneller, doch sie ben\u00f6tigen mehr" +
                                          "<br>Speicherplatz. H\u00f6here Werte bedeuten weniger" +
                                          "<br>Speicherplatzbedarf und einen etwas" +
                                          "<br>langsameren Programmstart." +
                                          "</html>" },

        { "common.ok_btn.tooltip",  "<html>" +
                                    "Speichert die \u00c4nderungen und schlie\u00dft das Dialogfeld" +
                                    "</html>" },

        { "common.cancel_btn.tooltip",  "<html>" +
                                        "Verwirft die \u00c4nderungen und schlie\u00dft das Dialogfeld" +
                                        "</html>"},

	{ "network.settings.advanced_btn.tooltip",  "<html>" +
                                                    "Zeigt erweiterte Proxy-Einstellungen zur Bearbeitung an"+
                                                    "</html>"},

        {"security.certs_btn.tooltip", "<html>" +
                                       "Importiert, exportiert oder entfernt Zertifikate" +
                                       "</html>" },

        { "cert.import_btn.tooltip", "<html>" +
                                     "Importiert ein noch nicht in der Liste befindliches" +
                                     "<br>Zertifikat" +
				     "</html>"},

        { "cert.export_btn.tooltip",    "<html>" +
                                        "Exportiert das ausgew\u00e4hlte Zertifikat" +
                                        "</html>"},

        { "cert.remove_btn.tooltip",  "<html>" +
                                      "Entfernt das ausgew\u00e4hlte Zertifikat aus"+
                                      "<br>der Liste" +
        		      "</html>"},

        { "cert.details_btn.tooltip", "<html>" +
		      "Zeigt ausf\u00fchrliche Angaben \u00fcber das ausgew\u00e4hlte" +
                      "<br>Zertifikat an" +
		      "</html>"},

        { "java.panel.jpi_view_btn.tooltip",  "<html>" +
                                              "\u00c4ndert die \"Java Runtime-Einstellungen f\u00fcr Applets\"" +
                                              "</html>" },

        { "java.panel.javaws_view_btn.tooltip",   "<html>" +
                                                  "\u00c4ndert die \"Java Runtime-Einstellungen f\u00fcr Anwendungen\"" +
                                                  "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "Zeigt Informationen \u00fcber diese Version von" +
                                            "<br>J2SE Runtime Environment an" +
                                            "</html>" },

        { "update.notify_combo.tooltip",  "<html>" +
                                          "Geben Sie an, wann Sie \u00fcber neue " +
                                          "<br>Java-Aktualisierungen benachrichtigt" +
                                          "<br>werden m\u00f6chten" +
                                          "</html>" },

        { "update.advanced_btn.tooltip",  "<html>" +
                                          "\u00c4ndert die Zeitplanrichtlinie" +
                                          "<br>f\u00fcr die automatische Aktualisierung" +
                                          "</html>" },

        { "update.now_btn.tooltip",    "<html>" +
                                      "Startet Java-Update, um nach den neuesten" +
                                      "<br>verf\u00fcgbaren Java-Aktualisierungen zu suchen" +
                                      "</html>" },

        { "vm.options.add_btn.tooltip",   "<html>" +
                                          "F\u00fcgt eine neue JRE-Version in die Liste ein" +
                                          "</html>" },

        { "vm.options.remove_btn.tooltip", "<html>" +
                                           "Entfernt den ausgew\u00e4hlten Eintrag aus der Liste" +
                                           "</html>" },

        { "vm.optios.ok_btn.tooltip",    "<html>" +
		         "Speichert alle Eintr\u00e4ge mit Informationen" +
		         "<br>\u00fcber Produktnamen, Version und" +
		         "<br>Adresse" +
		         "</html>" },

        { "jnlp.jre.find_btn.tooltip",  "<html>" +
		        "Sucht installierte Version von Java Runtime" +
                        "<br>Environment" +
		        "</html>" },

        { "jnlp.jre.add_btn.tooltip",   "<html>" +
                                        "F\u00fcgt einen neuen Eintrag in die Liste ein" +
		                        "</html>" },

        { "jnlp.jre.remove_btn.tooltip",  "<html>" +
                                          "Entfernt den ausgew\u00e4hlten Eintrag aus der" +
                                          "<br>Benutzerliste" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "JRE-Download zulassen" },
        { "download.jre.prompt.text1", "Die Anwendung \"{0}\" ben\u00f6tigt eine Version von "
                                     + "JRE (Version {1}), die "
                                     + "derzeit nicht auf Ihrem System installiert ist." },
        { "download.jre.prompt.text2", "Soll Java Web Start diese JRE-Version automatisch "
                                     + "herunterladen und installieren?" },
        { "download.jre.prompt.okButton", "Herunterladen" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_H)},
        { "download.jre.prompt.cancelButton", "Abbrechen" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "autoupdatecheck.buttonYes", "Ja" },
	{ "autoupdatecheck.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_J)},
	{ "autoupdatecheck.buttonNo", "Nein" },
	{ "autoupdatecheck.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "autoupdatecheck.buttonAskLater", "Sp\u00e4ter fragen" },
	{ "autoupdatecheck.buttonAskLater.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "autoupdatecheck.caption", "Automatisch nach Aktualisierungen suchen" },
        { "autoupdatecheck.message", "Java Update kann Ihre Java-Software automatisch aktualisieren, sobald neue Versionen verf\u00fcgbar werden. M\u00f6chten Sie diesen Dienst aktivieren?" },
    };
}
