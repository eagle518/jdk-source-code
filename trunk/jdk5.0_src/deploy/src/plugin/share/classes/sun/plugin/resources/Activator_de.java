/*
 * @(#)Activator_de.java	1.55 04/05/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
/**
 * German version of Activator strings.
 *
 * @author Jerome Dochez
 */

public class Activator_de extends ListResourceBundle {

    public Object[][] getContents() {
    return contents;
    }

    static final Object[][] contents = {
	{ "loading", "Ladevorgang l\u00e4uft {0}" },
	{ "java_applet", "Java-Applet" },
        { "failed", "Fehler beim Laden des Java-Applets..." },
        { "image_failed", "Fehler beim Erstellen des benutzerdefinierten Bildes. \u00dcberpr\u00fcfen Sie den Dateinamen des Bildes." },
	{ "java_not_enabled", "Java ist nicht aktiviert" },
        { "exception", "Ausnahme: {0}" },

	{ "bean_code_and_ser", "F\u00fcr Bean k\u00f6nnen nicht gleichzeitig CODE und JAVA_OBJECT definiert sein " },
	{ "status_applet", "Applet {0} {1}" },

    // Resources associated with SecurityManager print Dialog:
	{ "print.caption", "Best\u00e4tigung erforderlich - Drucken" },
	{ "print.message", new String[]{
		"<html><b>Druckanforderung</b></html>Applet m\u00f6chte drucken. M\u00f6chten Sie fortfahren?"}},
	{ "print.checkBox", "Dieses Dialogfeld nicht wieder anzeigen" },
	{ "print.buttonYes", "Ja" },
	{ "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_J)},
	{ "print.buttonNo", "Nein" },
	{ "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},

	{ "optpkg.cert_expired", "<html><b>Zertifikat abgelaufen</b></html>Installation des optionalen Pakets wurde abgebrochen.\n" },
	{ "optpkg.cert_notyieldvalid", "<html><b>Zertifikat nicht g\u00fcltig</b></html>Installation des optionalen Pakets wurde abgebrochen.\n" },
	{ "optpkg.cert_notverify", "<html><b>Zertifikat nicht verifiziert</b></html>Installation des optionalen Pakets wurde abgebrochen.\n" },
	{ "optpkg.general_error", "<html><b>Allgemeine Ausnahme</b></html>Installation des optionalen Pakets wurde abgebrochen.\n" },
	{ "optpkg.caption", "Warnung - Optionales Paket" },
	{ "optpkg.installer.launch.wait", "<html><b>Installation des optionalen Pakets</b></html>Klicken Sie auf OK, um das Laden des Applets fortzusetzen, wenn das Installationsprogramm des optionalen Pakets beendet ist.\n" },
	{ "optpkg.installer.launch.caption", "Installation wird durchgef\u00fchrt - Optionales Paket"},
	{ "optpkg.prompt_user.new_spec.text", "<html><b>Download-Anforderung</b></html>Das Applet erfordert eine neuere Version (Spezifikation {0}) des optionalen Pakets \"{1}\" von {2}\n\nM\u00f6chten Sie fortfahren?" },
	{ "optpkg.prompt_user.new_impl.text", "<html><b>Download-Anforderung</b></html>Das Applet erfordert eine neuere Version (Implementierung {0}) des optionalen Pakets \"{1}\" von {2}\n\nM\u00f6chten Sie fortfahren?" },
	{ "optpkg.prompt_user.new_vendor.text", "<html><b>Download-Anforderung</b></html>Das Applet erfordert ({0}) des optionalen Pakets \"{1}\" {2} von {3}\n\nM\u00f6chten Sie fortfahren?" },
	{ "optpkg.prompt_user.default.text", "<html><b>Download-Anforderung</b></html>Das Applet erfordert die Installation des optionalen Pakets \"{0}\" von {1}\n\nM\u00f6chten Sie fortfahren?" },

	{ "cache.error.text", "<html><b>Caching-Fehler</b></html>Im Cache k\u00f6nnen keine Dateien gespeichert oder aktualisiert werden." },
	{ "cache.error.caption", "Fehler - Cache" },
	{ "cache.version_format_error", "{0} hat nicht das Format xxxx.xxxx.xxxx.xxxx, wobei x eine hexadezimale Zahl ist" },
	{ "cache.version_attrib_error", "Anzahl der in \'cache_archive\' angegebenen Attribute entspricht nicht der Anzahl in \'cache_version\'" },
	{ "cache.header_fields_missing", "Wert f\u00fcr Datum der letzten \u00c4nderung und/oder Ablaufdatum ist nicht verf\u00fcgbar.  Die Jar-Datei wird nicht in den Cache geschrieben."},

	{ "applet.progress.load", "Applet wird geladen..." },
	{ "applet.progress.init", "Applet wird initialisiert..." },
	{ "applet.progress.start", "Applet wird gestartet..." },
	{ "applet.progress.stop", "Applet wird angehalten..." },
	{ "applet.progress.destroy", "Applet wird zerst\u00f6rt..." },
	{ "applet.progress.dispose", "Applet wird verworfen..." },
	{ "applet.progress.quit", "Applet wird beendet..." },
	{ "applet.progress.stoploading", "Ladevorgang abgebrochen..." },
	{ "applet.progress.interrupted", "Thread unterbrochen..." },
	{ "applet.progress.joining", "Applet-Thread wird beigetreten..." },
	{ "applet.progress.joined", "Applet-Thread wurde beigetreten..." },
	{ "applet.progress.loadImage", "Bild wird geladen " },
	{ "applet.progress.loadAudio", "Audiodatei wird geladen " },
	{ "applet.progress.findinfo.0", "Informationen werden gesucht..." },
	{ "applet.progress.findinfo.1", "Fertig..." },
	{ "applet.progress.timeout.wait", "Warten auf Timeout..." },
	{ "applet.progress.timeout.jointing", "Beitreten wird ausgef\u00fchrt..." },
	{ "applet.progress.timeout.jointed", "Beitreten abgeschlossen..." },


	{ "modality.register", "Registrierter Modality-Listener" },
	{ "modality.unregister", "Nicht registrierter Modality-Listener" },
	{ "modality.pushed", "Modality-Push durchgef\u00fchrt" },
	{ "modality.popped", "Modality-Pop durchgef\u00fchrt" },

	{ "progress.listener.added", "Fortschritts-Listener hinzugef\u00fcgt: {0}" },
	{ "progress.listener.removed", "Fortschritts-Listener entfernt: {0}" },

	{ "liveconnect.UniversalBrowserRead.enabled", "JavaScript: UniversalBrowserRead aktiviert" },
	{ "liveconnect.java.system", "JavaScript: Java-Systemcode wird aufgerufen" },
	{ "liveconnect.same.origin", "JavaScript: Aufrufer und Aufgerufener haben denselben Ursprung" },
	{ "liveconnect.default.policy", "JavaScript: Standard-Sicherheitsrichtlinie = {0}" },
	{ "liveconnect.UniversalJavaPermission.enabled", "JavaScript: UniversalJavaPermission aktiviert" },
	{ "liveconnect.wrong.securitymodel", "Netscape-Sicherheitsmodell wird nicht mehr unterst\u00fctzt.\n"
						 + "Bitte stellen Sie auf das Java 2-Sicherheitsmodell um.\n" },
        { "pluginclassloader.created_files", "{0} wurde im Cache erstellt." },
	{ "pluginclassloader.deleting_files", "JAR-Dateien werden aus Cache gel\u00f6scht." },
	{ "pluginclassloader.file", "   L\u00f6schen aus Cache {0}" },
	{ "pluginclassloader.empty_file", "{0} ist leer und wird aus Cache gel\u00f6scht." },

	{ "appletcontext.audio.loaded", "Audio-Clip geladen:{0} " },
	{ "appletcontext.image.loaded", "Bild geladen: {0}" },

	{ "securitymgr.automation.printing", "Automatisierung: Drucken akzeptieren" },

	{ "classloaderinfo.referencing", "ClassLoader wird referenziert: {0}, refcount={1}" },
	{ "classloaderinfo.releasing", "ClassLoader wird freigegeben: {0}, refcount={1}" },
	{ "classloaderinfo.caching", "ClassLoader wird im Cache abgelegt: {0}" },
	{ "classloaderinfo.cachesize", "Aktuelle Gr\u00f6\u00dfe des ClassLoader-Cache: {0}" },
	{ "classloaderinfo.num", "Anzahl der ClassLoader im Cache gr\u00f6\u00dfer als {0}, Referenz aufheben {1}" },

	{ "jsobject.eval", "JSObject::eval({0})" },
	{ "jsobject.call", "JSObject::call: Name={0}" },
	{ "jsobject.getMember", "JSObject::getMember: Name={0}" },
	{ "jsobject.setMember", "JSObject::setMember: Name={0}" },
	{ "jsobject.removeMember", "JSObject::removeMember: Name={0}" },
	{ "jsobject.getSlot", "JSObject::getSlot: {0}" },
	{ "jsobject.setSlot", "JSObject::setSlot: Slot={0}" },
        { "jsobject.invoke.url.permission", "Die URL des Applets lautet {0}, die Berechtigung = {1}"},

	{ "optpkg.install.info", "Optionales Paket wird installiert {0}" },
	{ "optpkg.install.fail", "Installation von optionalem Paket fehlgeschlagen." },
	{ "optpkg.install.ok", "Installation von optionalem Paket erfolgreich." },
	{ "optpkg.install.automation", "Automatisierung: Installation von optionalem Paket akzeptieren" },
	{ "optpkg.install.granted", "Download von optionalem Paket vom Benutzer gew\u00e4hrt, Download von {0}" },
	{ "optpkg.install.deny", "Download von optionalem Paket vom Benutzer nicht gew\u00e4hrt" },
	{ "optpkg.install.begin", "{0} wird installiert " },
	{ "optpkg.install.java.launch", "Java-Installationsprogramm wird gestartet" },
	{ "optpkg.install.java.launch.command", "Java-Installationsprogramm wird gestartet durch {0}" },
	{ "optpkg.install.native.launch", "Systemeigenes Installationsprogramm wird gestartet" },
	{ "optpkg.install.native.launch.fail.0", "Ausf\u00fchren von {0} nicht m\u00f6glich" },
	{ "optpkg.install.native.launch.fail.1", "Zugriff auf {0} nicht m\u00f6glich" },
	{ "optpkg.install.raw.launch", "Optionales Raw-Paket wird installiert" },
	{ "optpkg.install.raw.copy", "Optionales Raw-Paket wird von {0} nach {1} kopiert" },
	{ "optpkg.install.error.nomethod", "Dependent Extension Provider nicht installiert : Methode "
						 + " addExtensionInstallationProvider kann nicht abgerufen werden" },
	{ "optpkg.install.error.noclass", "Dependent Extension Provider nicht installiert : Klasse "
					 + "sun.misc.ExtensionDependency kann nicht abgerufen werden" },

	{"progress_dialog.downloading", "Plug-in: Download-Vorgang l\u00e4uft..."},
	{"progress_dialog.dismiss_button", "Verwerfen"},
	{"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_V)},
	{"progress_dialog.from", "von"},

	{"applet_viewer.color_tag", "Falsche Anzahl von Komponenten in {0}"},

	{"progress_info.downloading", "Download von JAR-Dateien"},
	{"progress_bar.preload", "JAR-Dateien werden vorab geladen: {0}"},

	{"cache.size", "Cache-Gr\u00f6\u00dfe: {0}"},
	{"cache.cleanup", "Gr\u00f6\u00dfe des Cache ist: {0} Byte, Bereinigung erforderlich"},
	{"cache.full", "Cache ist voll: Datei {0} wird gel\u00f6scht"},
	{"cache.inuse", "Datei {0} kann nicht gel\u00f6scht werden, da sie von dieser Anwendung verwendet wird"},
	{"cache.notdeleted", "Datei {0} kann nicht gel\u00f6scht werden, da sie m\u00f6glicherweise von dieser und/oder anderen Anwendungen verwendet wird"},
	{"cache.out_of_date", "Kopie von {0} im Cache ist nicht aktuell\n  Kopie im Cache: {1}\n  Kopie auf dem Server: {2}"},
	{"cache.loading", "{0} wird aus dem Cache geladen"},
	{"cache.cache_warning", "ACHTUNG: {0} kann nicht im Cache gespeichert werden"},
	{"cache.downloading", "Download von {0} in den Cache"},
	{"cache.cached_name", "Name der Datei im Cache: {0}"},
	{"cache.load_warning", "ACHTUNG: Fehler beim Lesen von {0} aus Cache."},
	{"cache.disabled", "Cache vom Benutzer deaktiviert"},
	{"cache.minSize", "Cache ist deaktiviert, Cache-Grenze auf {0} gesetzt, mindestens 5 MB sollten angegeben werden"},
	{"cache.directory_warning", "ACHTUNG: {0} ist kein Verzeichnis. Cache wird deaktiviert."},
 {"cache.response_warning", "ACHTUNG: Unerwartete Antwort {0} f\u00fcr {1}.  Die Datei wird erneut heruntergeladen."},
	{"cache.enabled", "Cache ist aktiviert"},
	{"cache.location", "Speicherort: {0}"},
	{"cache.maxSize", "Maximale Gr\u00f6\u00dfe: {0}"},
	{"cache.create_warning", "ACHTUNG: Cache-Verzeichnis {0} konnte nicht erstellt werden. Caching wird deaktiviert."},
	{"cache.read_warning", "ACHTUNG: Cache-Verzeichnis {0} kann nicht gelesen werden. Caching wird deaktiviert."},
	{"cache.write_warning", "ACHTUNG: In Cache-Verzeichnis {0} kann nicht geschrieben werden. Caching wird deaktiviert."},
	{"cache.compression", "Kompressionsstufe: {0}"},
	{"cache.cert_load", "Zertifikat f\u00fcr {0} wird aus JAR-Cache gelesen"},
	{"cache.jarjar.invalid_file", "Datei .jarjar enth\u00e4lt eine Datei, die keine .jar-Datei ist"},
	{"cache.jarjar.multiple_jar", "Datei .jarjar enth\u00e4lt mehr als eine .jar-Datei"},
	{"cache.version_checking", "Version von {0} wird \u00fcberpr\u00fcft, angegebene Version ist {1}"},
	{ "cache.preloading", "Datei {0} wird vorab geladen"},

	{ "cache_viewer.caption", "Cache-Anzeigeprogramm f\u00fcr Java-Applets" },
	{ "cache_viewer.refresh", "Aktualisieren" },
	{ "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_A) },
	{ "cache_viewer.remove", "L\u00f6schen" },
	{ "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_L) },
	   { "cache_viewer.OK", "OK" },
	{ "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_O) },
	{ "cache_viewer.name", "Name" },
	{ "cache_viewer.type", "Typ" },
	{ "cache_viewer.size", "Gr\u00f6\u00dfe" },
	{ "cache_viewer.modify_date", "Letzte \u00c4nderung" },
	{ "cache_viewer.expiry_date", "Ablaufdatum" },
	{ "cache_viewer.url", "URL" },
	{ "cache_viewer.version", "Version" },
	{ "cache_viewer.help.name", "Name der Datei im Cache" },
	{ "cache_viewer.help.type", "Typ der Datei im Cache" },
	{ "cache_viewer.help.size", "Gr\u00f6\u00dfe der Datei im Cache" },
	{ "cache_viewer.help.modify_date", "Datum der letzten \u00c4nderung der Datei im Cache" },
	{ "cache_viewer.help.expiry_date", "Ablaufdatum der Datei im Cache" },
	{ "cache_viewer.help.url", "URL f\u00fcr Download der Datei im Cache" },
	{ "cache_viewer.help.version", "Version der Datei im Cache" },
	{ "cache_viewer.delete.text", "<html><b>Datei nicht gel\u00f6scht</b></html>{0} wird m\u00f6glicherweise verwendet.\n" },
	{ "cache_viewer.delete.caption", "Fehler - Cache" },
	{ "cache_viewer.type.zip", "Jar" },
	{ "cache_viewer.type.class", "Klasse" },
	{ "cache_viewer.type.wav", "Wav-Sound" },
	{ "cache_viewer.type.au", "Au-Sound" },
	{ "cache_viewer.type.gif", "Gif-Bild" },
	{ "cache_viewer.type.jpg", "Jpeg-Bild" },
	{ "cache_viewer.menu.file", "Datei" },
        { "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_D) },
        { "cache_viewer.menu.options", "Optionen" },
        { "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_P) },
        { "cache_viewer.menu.help", "Hilfe" },
        { "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_H) },
        { "cache_viewer.menu.item.exit", "Beenden" },
        { "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_B) },
        { "cache_viewer.disable", "Caching aktivieren" },
        { "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_N) },
        { "cache_viewer.menu.item.about", "Anwendungsinfo" },
        { "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_I) },

	{ "net.proxy.auto.result.error", "Proxy-Einstellungen konnten nicht durch Auswertung bestimmt werden - R\u00fcckgriff auf DIREKTE"},

	{ "lifecycle.applet.found", "Zuvor angehaltenes Applet aus dem Lebenszyklus-Cache gefunden" },
	{ "lifecycle.applet.support", "Applet unterst\u00fctzt Legacy-Lebenszyklusmodell - Applet dem Lebenszyklus-Cache hinzuf\u00fcgen" },
	{ "lifecycle.applet.cachefull", "Lebenszyklus-Cache ist voll - die am l\u00e4ngsten nicht verwendeten Applets entfernen" },

	{ "com.method.ambiguous", "Methode kann nicht ausgew\u00e4hlt werden, keine eindeutigen Parameter" },
	{ "com.method.notexists", "{0} :Methode nicht vorhanden" },
	{ "com.notexists", "{0} :Methode/Eigenschaft nicht vorhanden" },
	{ "com.method.invoke", "Methode wird gestartet: {0}" },
	{ "com.method.jsinvoke", "JS-Methode wird gestartet: {0}" },
	{ "com.method.argsTypeInvalid", "Die Parameter konnten nicht zu den erforderlichen Typen konvertiert werden" },
	{ "com.method.argCountInvalid", "Anzahl der Argumente nicht korrekt" },
	{ "com.field.needsConversion", "Muss konvertiert werden: {0} --> {1}" },
	{ "com.field.typeInvalid", " kann nicht konvertiert werden zu Typ: {0}" },
	{ "com.field.get", "Eigenschaft wird gelesen: {0}" },
	{ "com.field.set", "Eigenschaft wird festgelegt: {0}" },

	{ "rsa.cert_expired", "<html><b>Zertifikat abgelaufen</b></html>Code wird als nicht signiert behandelt.\n" },
	{ "rsa.cert_notyieldvalid", "<html><b>Zertifikat nicht g\u00fcltig</b></html>Code wird als nicht signiert behandelt.\n" },
	{ "rsa.general_error", "<html><b>Zertifikat nicht verifiziert</b></html>Code wird als nicht signiert behandelt.\n" },

	{ "dialogfactory.menu.show_console", "Java-Konsole anzeigen" },
	{ "dialogfactory.menu.hide_console", "Java-Konsole verbergen" },
	{ "dialogfactory.menu.about",  "Anwendungsinfo Java Plug-in" },
	{ "dialogfactory.menu.copy", "Kopieren" },
	{ "dialogfactory.menu.open_console", "Java-Konsole \u00f6ffnen" },
	{ "dialogfactory.menu.about_java", "Anwendungsinfo Java(TM)" },

    };
}


