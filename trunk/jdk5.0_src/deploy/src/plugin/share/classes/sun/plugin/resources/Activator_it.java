/*
 * @(#)Activator_it.java	1.49 04/03/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
/**
* Italian version of Activator strings.
*
* @author Jerome Dochez
*/

public class Activator_it extends ListResourceBundle {

   public Object[][] getContents() {
       return contents;
   }

   static final Object[][] contents = {
	{ "loading", "Caricamento di {0}" },
	{ "java_applet", "Applet Java" },
	{ "failed", "Caricamento Applet Java non riuscito..." },
	{ "image_failed", "Impossibile creare immagine definita dall'utente. Controllare nome file immagine." },
	{ "java_not_enabled", "Java non abilitato" },
	{ "exception", "Eccezione: {0}" },
       
	{ "bean_code_and_ser", "Impossibile definire sia CODE che JAVA_OBJECT per il Bean" },
	{ "status_applet", "Applet {0} {1}" },

   // Resources associated with SecurityManager print Dialog:
	{ "print.caption", "Conferma necessaria - Stampa" },
	{ "print.message", new String[]{
		"<html><b>Richiesta di stampa</b></html>L'applet sta tentando di stampare. Continuare?"}},
	{ "print.checkBox", "Non visualizzare pi\u00f9 questa finestra" },
	{ "print.buttonYes", "S\u00ec" },
	{ "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "print.buttonNo", "No" },
	{ "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},

	{ "optpkg.cert_expired", "<html><b>Certificato scaduto</b></html>Installazione del pacchetto opzionale interrotta.\n" },
	{ "optpkg.cert_notyieldvalid", "<html><b>Certificato non valido</b></html>Installazione del pacchetto opzionale interrotta.\n" },
	{ "optpkg.cert_notverify", "<html><b>Certificato non verificato</b></html>Installazione del pacchetto opzionale interrotta.\n" },
	{ "optpkg.general_error", "<html><b>Eccezione generale</b></html>Installazione del pacchetto opzionale interrotta.\n" },
	{ "optpkg.caption", "Attenzione - Pacchetto opzionale" },
	{ "optpkg.installer.launch.wait", "<html><b>Installazione del pacchetto opzionale</b></html>Fare clic su OK per continuare il caricamento dell'applet dopo la chiusura del programma di installazione del pacchetto opzionale.\n" },
	{ "optpkg.installer.launch.caption", "Installazione in corso - Pacchetto opzionale"},
	{ "optpkg.prompt_user.new_spec.text", "<html><b>Richiesta di download</b></html>Per questo applet \u00e8 necessaria una versione pi\u00f9 recente (specifica {0}) del pacchetto opzionale \"{1}\" da {2}\n\nContinuare?" },
	{ "optpkg.prompt_user.new_impl.text", "<html><b>Richiesta di download</b></html>Per questo applet \u00e8 necessaria versione pi\u00f9 recente (implementazione {0}) del pacchetto opzionale \"{1}\" da {2}\n\nContinuare?" },
	{ "optpkg.prompt_user.new_vendor.text", "<html><b>Richiesta di download</b></html>Per questo applet \u00e8 necessaria la ({0}) del pacchetto opzionale \"{1}\"  {2} da {3}\n\nContinuare?" },
	{ "optpkg.prompt_user.default.text", "<html><b>Richiesta di download</b></html>Per questo applet \u00e8 necessaria l''installazione del pacchetto opzionale \"{0}\" da {1}\n\nContinuare?" },

	{ "cache.error.text", "<html><b>Errore nella cache</b></html>Impossibile memorizzare o aggiornare i file nella cache." },
	{ "cache.error.caption", "Errore - Cache" },
	{ "cache.version_format_error", "{0} non \u00e8 nella forma xxxx.xxxx.xxxx.xxxx, dove x \u00e8 una cifra esadecimale" },
	{ "cache.version_attrib_error", "Il numero di attributi specificati in \'cache_archive\' non corrisponde a quelli presenti in \'cache_version\'" },
	{ "cache.header_fields_missing", "Ora ultima modifica e/o valore scadenza non disponibili. Il file Jar non verr\u00e0 inserito nella cache."},

	{ "applet.progress.load", "Caricamento applet..." },
	{ "applet.progress.init", "Inizializzazione applet..." },
	{ "applet.progress.start", "Avvio applet..." },
	{ "applet.progress.stop", "Interruzione applet..." },
	{ "applet.progress.destroy", "Distruzione applet..." },
	{ "applet.progress.dispose", "Eliminazione applet..." },
	{ "applet.progress.quit", "Chiusura applet..." },
	{ "applet.progress.stoploading", "Caricamento interrotto..." },
	{ "applet.progress.interrupted", "Thread interrotto..." },
	{ "applet.progress.joining", "Esecuzione join thread..." },
	{ "applet.progress.joined", "Join thread eseguito..." },
	{ "applet.progress.loadImage", "Caricamento immagine" },
	{ "applet.progress.loadAudio", "Caricamento audio" },
	{ "applet.progress.findinfo.0", "Ricerca informazioni..." },
	{ "applet.progress.findinfo.1", "Fine..." },
	{ "applet.progress.timeout.wait", "Attesa timeout..." },
	{ "applet.progress.timeout.jointing", "Esecuzione join..." },
	{ "applet.progress.timeout.jointed", "Join eseguito..." },


	{ "modality.register", "Ascoltatore in modalit\u00e0 registrata" },
	{ "modality.unregister", "Ascoltatore in modalit\u00e0 non registrata" },
	{ "modality.pushed", "Eseguito push della modalit\u00e0" },
	{ "modality.popped", "Eseguito pop della modalit\u00e0" },

	{ "progress.listener.added", "Aggiunto ascoltatore avanzamento: {0}" },
	{ "progress.listener.removed", "Rimosso ascoltatore avanzamento: {0}" },

	{ "liveconnect.UniversalBrowserRead.enabled", "JavaScript: UniversalBrowserRead attivato" },
	{ "liveconnect.java.system", "JavaScript: chiamata codice di sistema Java" },
	{ "liveconnect.same.origin", "JavaScript: stessa origine per chiamante e destinatario" },
	{ "liveconnect.default.policy", "JavaScript: policy di protezione predefinita = {0}" },
	{ "liveconnect.UniversalJavaPermission.enabled", "JavaScript: UniversalJavaPermission attivato" },
	{ "liveconnect.wrong.securitymodel", "Il modello di protezione Netscape non \u00e8 pi\u00f9 supportato.\n"
				      + "Eseguire la migrazione al modello di protezione Java 2.\n" },

       { "pluginclassloader.created_files", "Creato {0} nella cache." },
       { "pluginclassloader.deleting_files", "Eliminazione file JAR dalla cache." },
       { "pluginclassloader.file", "   eliminazione dalla cache {0}" },
       { "pluginclassloader.empty_file", "{0} non ha dati, eliminazione dalla cache." },

	{ "appletcontext.audio.loaded", "Caricato clip audio: " },
	{ "appletcontext.image.loaded", "Caricata immagine: " },

	{ "securitymgr.automation.printing", "Automazione: accetta la stampa" },

	{ "classloaderinfo.referencing", "Referenziazione classloader: {0}, refcount={1}" },
	{ "classloaderinfo.releasing", "Rilascio classloader: {0}, refcount={1}" },
	{ "classloaderinfo.caching", "Caching classloader: " },
	{ "classloaderinfo.cachesize", "Dimensioni cache classloader attuale: " },
	{ "classloaderinfo.num", "Numero di classloader memorizzati su {0}, senza referenziazione {1}" },

	{ "jsobject.eval", "JSObject::eval({0})" },
	{ "jsobject.call", "JSObject::call: name={0}" },
	{ "jsobject.getMember", "JSObject::getMember: name={0}" },
	{ "jsobject.setMember", "JSObject::setMember: name={0}" },
	{ "jsobject.removeMember", "JSObject::removeMember: name={0}" },
	{ "jsobject.getSlot", "JSObject::getSlot: {0}" },
	{ "jsobject.setSlot", "JSObject::setSlot: slot={0}" },
	{ "jsobject.invoke.url.permission", "L''url dell''applet \u00e8 {0} e l''autorizzazione \u00e8 = {1}"},

	{ "optpkg.install.info", "Installazione pacchetto opzionale {0}" },
	{ "optpkg.install.fail", "Installazione pacchetto opzionale non riuscita." },
	{ "optpkg.install.ok", "Installazione pacchetto opzionale eseguita." },
	{ "optpkg.install.automation", "Automazione: accetta installazione pacchetto opzionale" },
	{ "optpkg.install.granted", "Download pacchetto opzionale garantito dall''utente, scarica da {0}" },
	{ "optpkg.install.deny", "Download pacchetto opzionale non garantito dall'utente" },
	{ "optpkg.install.begin", "Installazione {0}" },
	{ "optpkg.install.java.launch", "Avvio programma di installazione Java" },
	{ "optpkg.install.java.launch.command", "Avvio programma di installazione Java attraverso {0}" },
	{ "optpkg.install.native.launch", "Avvio programma di installazione nativo" },
	{ "optpkg.install.native.launch.fail.0", "Impossibile eseguire {0}" },
	{ "optpkg.install.native.launch.fail.1", "Accesso a {0} non riuscito" },
	{ "optpkg.install.raw.launch", "Installazione package opzionale grezzo" },
	{ "optpkg.install.raw.copy", "Copia pacchetto opzionale grezzo da {0} a {1}" },
	{ "optpkg.install.error.nomethod", "Dependent Extension Provider non installato: impossibile ottenere "
				         + " il metodo addExtensionInstallationProvider" },
	{ "optpkg.install.error.noclass", "Dependent Extension Provider non installato: impossibile ottenere "
					 + "la classe sun.misc.ExtensionDependency" },

	{"progress_dialog.downloading", "Plug-in: download in corso..."},
       {"progress_dialog.dismiss_button", "Interrompi"},
       {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
       {"progress_dialog.from", "da"},                

       {"applet_viewer.color_tag", "Numero di componenti non corretto in {0}"},
               
       {"progress_info.downloading", "Download file JAR addizionali"},
       {"progress_bar.preload", "Precaricamento file JAR: {0}"},
       
       {"cache.size", "Dimensioni cache: {0}"},
       {"cache.cleanup", " Dimensioni cache: {0} byte, \u00e8 necessario eliminare file"},
       {"cache.full", "Cache piena: eliminazione del file {0}"},
       {"cache.inuse", "Impossibile eliminare il file {0} perch\u00e9 utilizzato da questa applicazione"},
       {"cache.notdeleted", "Impossibile eliminare il file {0}, il file potrebbe essere utilizzato da questa e/o altre applicazioni"},
       {"cache.out_of_date", "La copia di {0} nella cache non \u00e8 aggiornata\n  Copia nella cache: {1}\n  Copia nel server: {2}"},
       {"cache.loading", "Caricamento di {0} dalla cache"},
       {"cache.cache_warning", "AVVISO: Impossibile eseguire il caching di {0}"},
       {"cache.downloading", "Download  di {0} nella cache"},
       {"cache.cached_name", "Nome file nella cache: {0}"},
       {"cache.load_warning", "AVVISO: errore durante la lettura di {0} dalla cache."},
       {"cache.disabled", "Cache disabilitata dall'utente"},
       {"cache.minSize", "Cache disabilitata, limite cache impostato a {0}, specificare almeno 5 MB"},
       {"cache.directory_warning", "AVVISO: {0} non \u00e8 una directory.  Il caching verr\u00e0 disattivato."},
       {"cache.response_warning", "AVVISO: risposta inattesa {0} per {1}. Il file verr\u00e0 di nuovo scaricato."}, 
       {"cache.enabled", "Cache abilitata"},
       {"cache.location", "Posizione: {0}"},
       {"cache.maxSize", "Dimensione massima: {0}"},
       {"cache.create_warning", "AVVISO: impossibile creare la directory cache {0}. Il caching verr\u00e0 disabilitato."},
       {"cache.read_warning", " AVVISO: impossibile leggere la directory cache {0}.  Il caching verr\u00e0 disabilitato."},
       {"cache.write_warning", " AVVISO: impossibile scrivere sulla directory cache {0}.  Il caching verr\u00e0 disabilitato."},
       {"cache.compression", "Livello di compressione: {0}"},
       {"cache.cert_load", "I certificati per {0} sono letti dalla cache"},
       {"cache.jarjar.invalid_file", "Il file .jarjar contiene un file non .jar"},
       {"cache.jarjar.multiple_jar", "Il file .jarjar contiene pi\u00f9 di un file .jar"},
       {"cache.version_checking", "Controllo versione per {0}, la versione specificata \u00e8 {1}"},
       { "cache.preloading", "Precaricamento file {0}"},

	{ "cache_viewer.caption", "Visualizzatore cache applet Java" },
	{ "cache_viewer.refresh", "Aggiorna" },
	{ "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_A) },
	{ "cache_viewer.remove", "Elimina" },
	{ "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_E) },
	{ "cache_viewer.OK", "OK" },
	{ "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_O) },
	{ "cache_viewer.name", "Nome" },
	{ "cache_viewer.type", "Tipo" },
	{ "cache_viewer.size", "Dimensioni" },
	{ "cache_viewer.modify_date", "Ultima modifica" },
	{ "cache_viewer.expiry_date", "Data scadenza" },
	{ "cache_viewer.url", "URL" },
	{ "cache_viewer.version", "Versione" },
	{ "cache_viewer.help.name", "Nome file nella cache" },
	{ "cache_viewer.help.type", "Tipo file nella cache" },
	{ "cache_viewer.help.size", "Dimensioni file nella cache" },
	{ "cache_viewer.help.modify_date", "Data ultima modifica del file" },
	{ "cache_viewer.help.expiry_date", "Data scadenza del file" },
	{ "cache_viewer.help.url", "URL di scaricamento del file nella cache" },
	{ "cache_viewer.help.version", "Versione cache del file" },
	{ "cache_viewer.delete.text", "<html><b>File non eliminato</b></html>{0} potrebbe essere in uso.\n" },
	{ "cache_viewer.delete.caption", "Errore - Cache" },
	{ "cache_viewer.type.zip", "Jar" },
	{ "cache_viewer.type.class", "Classe" },
	{ "cache_viewer.type.wav", "File audio Wav" },
	{ "cache_viewer.type.au", " File audio Au " },
	{ "cache_viewer.type.gif", "Immagine Gif" },
	{ "cache_viewer.type.jpg", " Immagine Jpeg" },
        { "cache_viewer.menu.file", "File" },
        { "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_F) },
        { "cache_viewer.menu.options", "Opzioni" },
        { "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_P) },
        { "cache_viewer.menu.help", "Guida" },
        { "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_G) },
        { "cache_viewer.menu.item.exit", "Esci" },
        { "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_E) },
        { "cache_viewer.disable", "Abilita cache" },
        { "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_N) },
        { "cache_viewer.menu.item.about", "Informazioni su" },
        { "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_F) },

	{ "net.proxy.auto.result.error", "Impossibile determinare le impostazioni proxy dalla valutazione - fallback su DIRECT"},

	{ "lifecycle.applet.found", "\u00c8 stato trovato un applet precedente interrotto dalla cache del ciclo di vita" },
	{ "lifecycle.applet.support", "L'applet supporta il modello di ciclo di vita precedente. Aggiungere l'applet alla cache del ciclo di vita." },
	{ "lifecycle.applet.cachefull", "La cache del ciclo di vita \u00e8 piena. Eliminare gli applet meno recenti." },

	{ "com.method.ambiguous", "Impossibile selezionare un metodo, parametri non chiari" },
	{ "com.method.notexists", "{0}: metodo inesistente" },
	{ "com.notexists", "{0}: metodo/propriet\u00e0 inesistente" },
	{ "com.method.invoke", "Chiamata metodo: {0}" },
	{ "com.method.jsinvoke", "Chiamata metodo JS: {0}" },
	{ "com.method.argsTypeInvalid", "Impossibile convertire i parametri nei tipi richiesti" },
	{ "com.method.argCountInvalid", "Numero argomenti incorretto" },
	{ "com.field.needsConversion", "Richiesta conversione: {0} --> {1}" },
	{ "com.field.typeInvalid", " Impossibile convertire in tipo: {0}" },
	{ "com.field.get", "Recupero propriet\u00e0: {0}" },
	{ "com.field.set", "Impostazione propriet\u00e0: {0}" },

	{ "rsa.cert_expired", "<html><b>Certificato scaduto</b></html>Il codice verr\u00e0 trattato come non firmato.\n" },
	{ "rsa.cert_notyieldvalid", "<html><b>Certificato non valido</b></html>Il codice verr\u00e0 trattato come non firmato.\n" },
	{ "rsa.general_error", "<html><b>Certificato non verificato</b></html>Il codice verr\u00e0 trattato come non firmato.\n" },

	{ "dialogfactory.menu.show_console", "Mostra Console Java" },
	{ "dialogfactory.menu.hide_console", "Nascondi Console Java" },
	{ "dialogfactory.menu.about", "Informazioni su Java Plug-in" },
	{ "dialogfactory.menu.copy", "Copia" },
	{ "dialogfactory.menu.open_console", "Apri Console Java" },
	{ "dialogfactory.menu.about_java", "Informazioni su Java(TM)" },

   };
}




