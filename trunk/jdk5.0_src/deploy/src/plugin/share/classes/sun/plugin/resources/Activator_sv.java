/*
 * @(#)Activator_sv.java	1.51 04/05/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
/**
 * Swedish version of Activator strings.
 *
 * @author Jerome Dochez
 */

public class Activator_sv extends ListResourceBundle {

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
        { "loading", "L\u00e4ser in {0}...." },
        { "java_applet", "Java-applet" },
        { "failed", "Inl\u00e4sningen av Java-applet misslyckades..." },
        { "image_failed", "Kunde inte skapa anv\u00e4ndardefinierad bild.  Kontrollera bildfilens namn." },
        { "java_not_enabled", "Java har inte aktiverats" },
        { "exception", "Undantag: {0}" },

        { "bean_code_and_ser", "B\u00f6na kan inte ha b\u00e5de CODE och JAVA_OBJECT definierade " },
        { "status_applet", "Applet {0} {1}" },

        // Resources associated with SecurityManager print Dialog:
	{ "print.caption", "Beh\u00f6ver bekr\u00e4ftelse - skriv ut" },
        { "print.message", new String[]{
		"<html><b>F\u00f6rfr\u00e5gan om utskrivning</b></html>Applet vill skriva ut. Vill du forts\u00e4tta?\n"}},
	{ "print.checkBox", "Visa inte den h\u00e4r dialogrutan igen" },
	{ "print.buttonYes", "Ja" },
	{ "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_J)},
	{ "print.buttonNo", "Nej" },
	{ "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},

	{ "optpkg.cert_expired", "<html><b>Utg\u00e5nget certifikat</b></html>Installation av valfritt paket avbryts.\n" },
	{ "optpkg.cert_notyieldvalid", "<html><b>Certifikatet \u00e4r ej giltigt</b></html>Installation av valfritt paket avbryts.\n" },
	{ "optpkg.cert_notverify", "<html><b>Ej verifierat certifikat</b></html>Installation av valfritt paket avbryts.\n" },
	{ "optpkg.general_error", "<html><b>Generellt undantag</b></html>Installation av valfritt paket avbryts.\n" },
	{ "optpkg.caption", "Varning - Valfritt paket" },
	{ "optpkg.installer.launch.wait", "<html><b>Installerar valfritt paket</b></html>Klick OK f\u00f6r att forts\u00e4tta laddning av applet efter avslutad installation av valfritt paket.\n" },
	{ "optpkg.installer.launch.caption", "Installation p\u00e5g\u00e5r - Valfritt paket"},
	{ "optpkg.prompt_user.new_spec.text", "<html><b>F\u00f6rfr\u00e5gan om nerladdning</b></html>Applet beh\u00f6ver en nyare version (specifikation {0}) av valfritt paket \"{1}\" fr\u00e5n {2}\n\nVill du forts\u00e4tta?" },
	{ "optpkg.prompt_user.new_impl.text", "<html><b>F\u00f6rfr\u00e5gan om nerladdning</b></html>Applet beh\u00f6ver en nyare version (implementering {0}) av valfritt paket \"{1}\" fr\u00e5n {2}\n\nVill du forts\u00e4tta?" },
	{ "optpkg.prompt_user.new_vendor.text", "<html><b>F\u00f6rfr\u00e5gan om nerladdning</b></html>Applet kr\u00e4ver ({0}) av valfritt paket \"{1}\" {2} fr\u00e5n {3}\n\nVill fu forts\u00e4tta?" },
	{ "optpkg.prompt_user.default.text", "<html><b>F\u00f6rfr\u00e5gan om nerladdning</b></html>Applet kr\u00e4ver installation av valfritt paket \"{0}\" fr\u00e5n {1}\n\nVill du forts\u00e4tta?" },

	{ "cache.error.text", "<html><b>Cachar Fel</b></html>Kan inte spara eller uppdatera filer i cachen." },
	{ "cache.error.caption", "Fel - Cache" },
	{ "cache.version_format_error", "{0} \u00e4r inte i formen xxxx.xxxx.xxxx.xxxx d\u00e4r x \u00e4r ett hexadecimal tal" },
		{ "cache.version_attrib_error", "Antalet attribut som angetts i \'cache_archive\' \u00f6verensst\u00e4mmer inte med dem i \'cache_version\'" },
	{ "cache.header_fields_missing", "Uppgift om n\u00e4r filen \u00e4ndrades senast och/eller f\u00f6rfallodatum \u00e4r inte tillg\u00e4nglig.  Jar-filen kommer inte att lagras i cacheminnet."},

		{ "applet.progress.load", "Laddar applet..." },
		{ "applet.progress.init", "Initierar applet..." },
		{ "applet.progress.start", "Startar applet..." },
		{ "applet.progress.stop", "Stoppar applet..." },
		{ "applet.progress.destroy", "F\u00f6rst\u00f6r applet..." },
		{ "applet.progress.dispose", "Kasserar applet..." },
		{ "applet.progress.quit", "Avslutar applet..." },
		{ "applet.progress.stoploading", "Stoppade laddning..." },
		{ "applet.progress.interrupted", "Avbr\u00f6t tr\u00e5d..." },
		{ "applet.progress.joining", "Kopplar applet-tr\u00e5d..."},
		{ "applet.progress.joined", "Kopplade applet-tr\u00e5d..."},
		{ "applet.progress.loadImage", "Laddar bild " },
		{ "applet.progress.loadAudio", "Laddar ljud " },
		{ "applet.progress.findinfo.0", "S\u00f6ker efter information..." },
		{ "applet.progress.findinfo.1", "Klar..." },
		{ "applet.progress.timeout.wait", "V\u00e4ntar p\u00e5 tidsgr\u00e4ns..." },
		{ "applet.progress.timeout.jointing", "Utf\u00f6r en sammanslagning..."},
		{ "applet.progress.timeout.jointed", "Klar med sammanslagning..." },

		{ "modality.register", "Registrerad tillst\u00e5ndsavlyssnare" },
		{ "modality.unregister", "Oregistrerad tillst\u00e5ndsavlyssnare" },
		{ "modality.pushed", "Push-\u00e5tg\u00e4rd p\u00e5 tillst\u00e5nd" },
		{ "modality.popped", "Pop-\u00e5tg\u00e4rd p\u00e5 tillst\u00e5nd" },

	    { "progress.listener.added", "Lade till f\u00f6rloppsavlyssnare: {0}" },
	    { "progress.listener.removed", "Tog bort f\u00f6rloppsavlyssnare: {0}" },

	    { "liveconnect.UniversalBrowserRead.enabled", "JavaScript: UniversalBrowserRead aktiverad" },
	    { "liveconnect.java.system", "JavaScript: anropar Java-systemkod" },
	    { "liveconnect.same.origin", "JavaScript: anropare och mottagare har samma ursprung" },
	    { "liveconnect.default.policy", "JavaScript: standardinst\u00e4lld s\u00e4kerhetspolicy = {0}" },
	    { "liveconnect.UniversalJavaPermission.enabled", "JavaScript: UniversalJavaPermission aktiverad" },
	   	{ "liveconnect.wrong.securitymodel", "S\u00e4kerhetsmodell f\u00f6r Netscape kan inte l\u00e4ngre hanteras.\n"
		                     + "\u00d6verg\u00e5 till s\u00e4kerhetsmodellen f\u00f6r Java 2 i st\u00e4llet.\n" },

        { "pluginclassloader.created_files", "Skapade {0} i cache." },
	    { "pluginclassloader.deleting_files", "Tar bort JAR-filer fr\u00e5n cachen." },
	    { "pluginclassloader.file", "   tar bort fr\u00e5n cachen {0}" },
	    { "pluginclassloader.empty_file", "{0} \u00e4r tom, tar bort fr\u00e5n cachen." },

	    { "appletcontext.audio.loaded", "Laddade ljudklipp: {0}" },
	    { "appletcontext.image.loaded", "Laddade bild: {0}" },

	    { "securitymgr.automation.printing", "Automatisering: Acceptera utskrift" },

	    { "classloaderinfo.referencing", "Refererar till classloader: {0}, refcount={1}" },
	    { "classloaderinfo.releasing", "Frisl\u00e4pper classloader: {0}, refcount={1}" },
	    { "classloaderinfo.caching", "Placerar classloader i cache: {0}" },
	    { "classloaderinfo.cachesize", "Aktuell cachestorlek f\u00f6r classloader: {0}" },
	    { "classloaderinfo.num", "Antal cachelagrade classloaders \u00f6ver {0}, unreference {1}" },

	{ "jsobject.eval", "JSObject::eval({0})" },
	{ "jsobject.call", "JSObject::call: name={0}"},
	{ "jsobject.getMember", "JSObject::getMember: name={0}" },
	{ "jsobject.setMember", "JSObject::setMember: name={0}" },
	{ "jsobject.removeMember", "JSObject::removeMember: name={0}" },
	{ "jsobject.getSlot", "JSObject::getSlot: {0}" },
	{ "jsobject.setSlot", "JSObject::setSlot: slot={0}" },
	{ "jsobject.invoke.url.permission", "appletprogrammets url \u00e4r {0} och beh\u00f6righeten \u00e4r = {1}"},

	    { "optpkg.install.info", "Installera tillbeh\u00f6rspaket {0}" },
	    { "optpkg.install.fail", "Installationen av tillbeh\u00f6rspaketet misslyckades." },
	    { "optpkg.install.ok", "Installationen av tillbeh\u00f6rspaketet lyckades." },
	    { "optpkg.install.automation", "Automatisering: Acceptera installation av tillbeh\u00f6rspaket" },
	    { "optpkg.install.granted", "Anv\u00e4ndaren till\u00e5ter h\u00e4mtning av tillbeh\u00f6rspaket. H\u00e4mta fr\u00e5n {0}" },
	    { "optpkg.install.deny", "Anv\u00e4ndaren till\u00e5ter inte h\u00e4mtning av tillbeh\u00f6rspaket" },
	    { "optpkg.install.begin", "Installerar {0}" },
	    { "optpkg.install.java.launch", "Startar Java-installationsprogram" },
	    { "optpkg.install.java.launch.command", "Startar Java-installationsprogram via {0}" },
	    { "optpkg.install.native.launch", "Startar eget installationsprogram" },
	    { "optpkg.install.native.launch.fail.0", "Det g\u00e5r inte att k\u00f6ra {0}" },
	    { "optpkg.install.native.launch.fail.1", "\u00c5tkomst till {0} nekades" },
	    { "optpkg.install.raw.launch", "Installerar obearbetat tillbeh\u00f6rspaket" },
	    { "optpkg.install.raw.copy", "Kopierar obearbetat tillbeh\u00f6rspaket fr\u00e5n {0} till {1}" },
	    { "optpkg.install.error.nomethod", "Beroende tillbeh\u00f6rs-provider har inte installerats: Det g\u00e5r inte att h\u00e4mta "
	                         + " metoden addExtensionInstallationProvider" },
	    { "optpkg.install.error.noclass", "Beroende tillbeh\u00f6rs-provider har inte installerats: Det g\u00e5r inte att h\u00e4mta "
	                     + "klassen sun.misc.ExtensionDependency" },

	    {"progress_dialog.downloading", "Plug-in: H\u00e4mtar..."},
	    {"progress_dialog.dismiss_button", "Avf\u00e4rda"},
            {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_A)},
	    {"progress_dialog.from", "fr\u00e5n"},

          { "applet_viewer.color_tag", "Felaktigt antal komponenter i {0}"},

	    {"progress_info.downloading", "H\u00e4mtar ytterligare JAR-fil(er)"},
	    {"progress_bar.preload", "F\u00f6rladdar JAR-filer: {0}"},

	    {"cache.size", "Cachestorlek: {0}"},
	    {"cache.cleanup", "Cachestorlek \u00e4r: {0} bytes, det \u00e4r n\u00f6dv\u00e4ndigt att st\u00e4da upp"},
	    {"cache.full", "Cachen \u00e4r full: tar bort filen: {0}"},
	    {"cache.inuse", "Kan inte ta bort filen {0} eftersom den anv\u00e4nds av applikationen"},
	    {"cache.notdeleted", "Kan inte ta bort filen {0}, den anv\u00e4nds eventuellt av denna och/eller andra applikationer"},
	    {"cache.out_of_date", "Cache-kopia av {0} \u00e4r f\u00f6r gammal\n Cache-kopia: {1}\n Serverkopia: {2}"},
	    {"cache.loading", "Laddar {0} fr\u00e5n cache"},
	    {"cache.cache_warning", "VARNING: Kan inte lagra i cache"},
	    {"cache.downloading", "Laddar ner {0} till cache"},
	    {"cache.cached_name", "Filnamn i cache: {0}"},
	    {"cache.load_warning", "VARNING: fel intr\u00e4ffade n\u00e4r {0} l\u00e4stes fr\u00e5n cache."},
	    {"cache.disabled", "Anv\u00e4ndaren har st\u00e4ngt av cachen"},
	    {"cache.minSize", "Cachen \u00e4r avst\u00e4ngd, dess gr\u00e4ns \u00e4r satt till {0}, \u00e5tminstone 5 MB b\u00f6r vara angivet"},
	    {"cache.directory_warning", "VARNING: {0} \u00e4r inte ett bibliotek. Cachen kommer att st\u00e4ngas av."},
	    {"cache.response_warning", "VARNING: Ov\u00e4ntat svar {0} f\u00f6r {1}.  Filen kommer att laddas ned p\u00e5 nytt."},
	    {"cache.enabled", "Cachen \u00e4r aktiverad"},
	    {"cache.location", "Plats: {0}" },
	    {"cache.maxSize", "St\u00f6rsta storlek: {0}"},
	    {"cache.create_warning", "VARNING: Kunde inte skapa cache-bibliotek {0}. Cachen kommer att st\u00e4ngas av."},
	    {"cache.read_warning", "VARNING: Kunde inte l\u00e4sa cache-bibliotek {0}. Cachen kommer att st\u00e4ngas av."},
	    {"cache.write_warning", "VARNING: Kunde inte skriva till cache-bibliotek {0}. Cachen kommer att st\u00e4ngas av."},
	    {"cache.compression", "Komprimeringsniv\u00e5: {0}"},
	    {"cache.cert_load", "Certifikat f\u00f6r {0} l\u00e4ses fr\u00e5n JAR-cache"},
	    {"cache.jarjar.invalid_file", ".jarjar-fil inneh\u00e5ller en fil som inte \u00e4r en .jar-fil"},
	    {"cache.jarjar.multiple_jar", ".jarjar-fil inneh\u00e5ller fler \u00e4n en .jar-fil"},
	    {"cache.version_checking", "Versionkontroll f\u00f6r {0}, angiven version \u00e4r {1}"},
	    {"cache.preloading", "F\u00f6rladdar fil {0}"},

	{ "cache_viewer.caption", "Java Applet Cache Viewer" },
	    { "cache_viewer.refresh", "Uppdatera" },
	{ "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_U) },
	    { "cache_viewer.remove", "Ta bort" },
	{ "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_B) },
	    { "cache_viewer.OK", "OK" },
	{ "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_O) },
	    { "cache_viewer.name", "Namn" },
	    { "cache_viewer.type", "Typ"},
	    { "cache_viewer.size", "Storlek" },
	    { "cache_viewer.modify_date", "Senast \u00e4ndrad" },
	    { "cache_viewer.expiry_date", "F\u00f6rfallodatum" },
	    { "cache_viewer.url", "URL" },
	    { "cache_viewer.version", "Version" },
          { "cache_viewer.help.name", "Cachad jar-fils namn"},
	    { "cache_viewer.help.type", "Cachad jar-fils typ"},
          { "cache_viewer.help.size", "Cachad jar-fils storlek"},
          { "cache_viewer.help.modify_date", "Cachad jar-fils senaste \u00e4ndringsdatum"},
          { "cache_viewer.help.expiry_date", "Cachad jar-fils f\u00f6rfallodatum"},
          { "cache_viewer.help.url", "Jar-fils nerladdnings-URL"},
          { "cache_viewer.help.version","Jar-fils cache-version"},
	{ "cache_viewer.delete.text", "<html><b>Fil har inte tagits bort</b></html>{0} kan vara i bruk.\n" },
	{ "cache_viewer.delete.caption", "Fel - Cache" },
	    { "cache_viewer.type.zip", "Jar"},
	    { "cache_viewer.type.class", "Klass"},
	    { "cache_viewer.type.wav", "Wav-ljud"},
	    { "cache_viewer.type.au", "Au-ljud"},
	    { "cache_viewer.type.gif", "Gif-bild"},
	    { "cache_viewer.type.jpg", "Jpeg-bild"},
            { "cache_viewer.menu.file", "Arkiv" },
            { "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_A) },
            { "cache_viewer.menu.options", "Alternativ" },
            { "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_L) },
            { "cache_viewer.menu.help", "Hj\u00e4lp" },
            { "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_H) },
            { "cache_viewer.menu.item.exit", "Avsluta" },
            { "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_A) },
            { "cache_viewer.disable", "Aktivera cache-lagring" },
            { "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_N) },
            { "cache_viewer.menu.item.about", "Om" },
            { "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_M) },            

	    { "net.proxy.auto.result.error", "Det g\u00e5r inte att fastst\u00e4lla proxy-inst\u00e4llning fr\u00e5n utv\u00e4rdering - \u00e5terv\u00e4nder till DIRECT"},

          { "lifecycle.applet.found", "Hittade tidigare stoppad applet i livscykelcache"},
          { "lifecycle.applet.support", "Applet st\u00f6der modellen f\u00f6r \u00e4rvd livscykel - l\u00e4gg applet till livscykelcache"},
          { "lifecycle.applet.cachefull", "Livscykelcachen \u00e4r full - ta bort senast anv\u00e4nda appletar"},

	{ "com.method.ambiguous", "Det g\u00e5r inte att v\u00e4lja en metod, parametrarna \u00e4r inte entydiga" },
	{ "com.method.notexists", "{0} :det finns ingen s\u00e5dan metod" },
	{ "com.notexists", "{0} :det finns ingen s\u00e5dan metod/egenskap" },
	{ "com.method.invoke", "Anropar metod: {0}" },
	{ "com.method.jsinvoke", "Anropar JS-metod: {0}" },
	{ "com.method.argsTypeInvalid", "Parametrarna kan inte konverteras till de n\u00f6dv\u00e4ndiga typerna" },
	{ "com.method.argCountInvalid", "Antalet argument \u00e4r felaktigt" },
	{ "com.field.needsConversion", "M\u00e5ste konverteras: {0} --> {1}" },
	{ "com.field.typeInvalid", " kan inte konverteras till typ: {0}" },
	{ "com.field.get", "H\u00e4mtar egenskap: {0}" },
	{ "com.field.set", "St\u00e4ller in egenskap: {0}" },

	{ "rsa.cert_expired", "<html><b>Utg\u00e5nget certifikat</b></html>Kod kommer att behandlas som osignerad.\n" },
	{ "rsa.cert_notyieldvalid", "<html><b>Ej giltigt certifikat</b></html>Kod kommer att behandlas som osignerad.\n" },
	{ "rsa.general_error", "<html><b>Ej verifierat certifikat</b></html>Kod kommer att behandlas som osignerad.\n" },

        { "dialogfactory.menu.show_console", "Visa systemf\u00f6nstret f\u00f6r Java" },
        { "dialogfactory.menu.hide_console", "D\u00f6lj systemf\u00f6nstret f\u00f6r Java" },
        { "dialogfactory.menu.about", "Om Java Plug-in" },
        { "dialogfactory.menu.copy", "Kopiera" },
	{ "dialogfactory.menu.open_console", "\u00d6ppna Java-konsol" },
	{ "dialogfactory.menu.about_java", "Om Java(TM)" },


     };
}


