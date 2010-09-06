/*
 * @(#)Activator_fr.java	1.60 04/07/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
/**
 * French version of Activator strings.
 *
 * @author Jerome Dochez
 */

public class Activator_fr extends ListResourceBundle {

    public Object[][] getContents() {
         return contents;
    }

    static final Object[][] contents = {
        { "loading", "Chargement de {0}" },
        { "java_applet", "Applet Java" },
        { "failed", "Echec du chargement de l'applet Java..." },
        { "image_failed", "Echec de cr\u00e9ation d'une image d\u00e9finie par l'utilisateur.  V\u00e9rifiez le nom de fichier de l'image." },
        { "java_not_enabled", "Java n'est pas activ\u00e9" },
        { "exception", "Exception : {0}" },

        { "bean_code_and_ser", "CODE et JAVA_OBJECT ne peuvent pas \u00eatre tous les deux d\u00e9finis pour Bean " },
        { "status_applet", "Applet {0} {1}" },

        // Resources associated with SecurityManager print Dialog:
	{ "print.caption", "Confirmation requise - Impression" },
        { "print.message", new String[]{
		"<html><b>Demande d'impression</b></html>L'applet souhaite imprimer. Voulez-vous continuer ?"}},
        { "print.checkBox", "Ne plus afficher cette bo\u00eete de dialogue" },
      { "print.buttonYes", "Oui" },
        { "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_O)},
        { "print.buttonNo", "Non" },
        { "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},

	{ "optpkg.cert_expired", "<html><b>Le certificat a expir\u00e9</b></html>L'installation du package facultatif a \u00e9t\u00e9 abandonn\u00e9e.\n" },
	{ "optpkg.cert_notyieldvalid", "<html><b>Le certificat n'est pas valide</b></html>L'installation du package facultatif a \u00e9t\u00e9 abandonn\u00e9e.\n" },
	{ "optpkg.cert_notverify", "<html><b>Le certificat n'a pas \u00e9t\u00e9 v\u00e9rifi\u00e9</b></html>L'installation du package facultatif a \u00e9t\u00e9 abandonn\u00e9e.\n" },
	{ "optpkg.general_error", "<html><b>Exception g\u00e9n\u00e9rale</b></html>L'installation du package facultatif a \u00e9t\u00e9 abandonn\u00e9e.\n" },
	{ "optpkg.caption", "Avertissement - Package facultatif" },
	{ "optpkg.installer.launch.wait", "<html><b>Installation du package facultatif en cours</b></html>Cliquez sur OK pour continuer le chargement de l'applet apr\u00e8s l'arr\u00eat du programme d'installation du package facultatif.\n" },
	{ "optpkg.installer.launch.caption", "Installation en cours - Package facultatif"},
	{ "optpkg.prompt_user.new_spec.text", "<html><b>Demande de t\u00e9l\u00e9chargement</b></html>L''applet requiert une version plus r\u00e9cente (sp\u00e9cification {0}) du package facultatif \"{1}\" \u00e0 partir de {2}\n\nVoulez-vous continuer ?" },
	{ "optpkg.prompt_user.new_impl.text", "<html><b>Demande de t\u00e9l\u00e9chargement</b></html>L''applet requiert une version plus r\u00e9cente (impl\u00e9mentation {0}) du package facultatif \"{1}\" \u00e0 partir de {2}\n\nVoulez-vous continuer ?" },
	{ "optpkg.prompt_user.new_vendor.text", "<html><b>Demande de t\u00e9l\u00e9chargement</b></html>L''applet requiert ({0})du package facultatif \"{1}\" {2} \u00e0 partir de {3}\n\nVoulez-vous continuer ?" },
	{ "optpkg.prompt_user.default.text", "<html><b>Demande de t\u00e9l\u00e9chargement</b></html>L''applet requiert l''installation du package facultatif \"{0}\" \u00e0 partir de {1}\n\nVoulez-vous continuer ?" },

	{ "cache.error.text", "<html><b>Erreur de mise en cache</b></html>Impossible de stocker ou de mettre \u00e0 jour des fichiers dans le cache." },
	{ "cache.error.caption", "Erreur - Cache" },
	{ "cache.version_format_error", "{0} n''a pas le format xxxx.xxxx.xxxx.xxxx, o\u00f9 x est un nombre hexad\u00e9cimal" },
		{ "cache.version_attrib_error", "Le nombre d'attributs sp\u00e9cifi\u00e9 dans \'cache_archive\' ne correspond pas \u00e0 celui de \'cache_version\'" },
	{ "cache.header_fields_missing", "L'heure de la derni\u00e8re modification et/ou le d\u00e9lai d'expiration n'est pas disponible.  Le fichier Jar ne sera pas mis en cache."},

		{ "applet.progress.load", "Chargement de l'applet..." },
		{ "applet.progress.init", "Initialisation de l'applet..." },
		{ "applet.progress.start", "D\u00e9marrage de l'applet..." },
		{ "applet.progress.stop", "Arr\u00eat de l'applet..." },
		{ "applet.progress.destroy", "Destruction de l'applet..." },
		{ "applet.progress.dispose", "Elimination de l'applet..." },
		{ "applet.progress.quit", "Sortie de l'applet..." },
		{ "applet.progress.stoploading", "Chargement arr\u00eat\u00e9..." },
		{ "applet.progress.interrupted", "Thread interrompu..." },
		{ "applet.progress.joining", "Jonction du thread d'applet..." },
		{ "applet.progress.joined", "Thread d'applet joint..." },
		{ "applet.progress.loadImage", "Chargement d'image " },
		{ "applet.progress.loadAudio", "Chargement audio " },
		{ "applet.progress.findinfo.0", "Recherche d'informations..." },
		{ "applet.progress.findinfo.1", "Termin\u00e9..." },
		{ "applet.progress.timeout.wait", "Attente de fin de d\u00e9lai..." },
		{ "applet.progress.timeout.jointing", "Jointure en cours..." },
		{ "applet.progress.timeout.jointed", "Jointure termin\u00e9e..." },

		{ "modality.register", "R\u00e9cepteur de modalit\u00e9s enregistr\u00e9" },
		{ "modality.unregister", "R\u00e9cepteur de modalit\u00e9s non enregistr\u00e9" },
		{ "modality.pushed", "Modalit\u00e9 empil\u00e9e" },
		{ "modality.popped", "Modalit\u00e9 d\u00e9sempil\u00e9e" },

		{ "progress.listener.added", "R\u00e9cepteur de progression ajout\u00e9 : {0}" },
		{ "progress.listener.removed", "R\u00e9cepteur de progression supprim\u00e9 : {0}" },

		{ "liveconnect.UniversalBrowserRead.enabled", "JavaScript : UniversalBrowserRead activ\u00e9" },
		{ "liveconnect.java.system", "JavaScript : appel du code syst\u00e8me Java" },
		{ "liveconnect.same.origin", "JavaScript : appelant et appel\u00e9 ont la m\u00eame origine" },
		{ "liveconnect.default.policy", "JavaScript : politique de s\u00e9curit\u00e9 par d\u00e9faut = {0}" },
		{ "liveconnect.UniversalJavaPermission.enabled", "JavaScript : UniversalJavaPermission activ\u00e9" },
		{ "liveconnect.wrong.securitymodel", "Le mod\u00e8le de s\u00e9curit\u00e9 de Netscape n'est plus pris en charge.\n"
		                         + "Migrez vers le mod\u00e8le de s\u00e9curit\u00e9 Java 2.\n" },

        { "pluginclassloader.created_files", "{0} a \u00e9t\u00e9 cr\u00e9\u00e9 dans le cache." },
		    { "pluginclassloader.deleting_files", "Suppression des fichiers JAR du cache." },
		    { "pluginclassloader.file", "   suppression du cache {0}" },
		    { "pluginclassloader.empty_file", "{0} est vide, suppression du cache." },

		{ "appletcontext.audio.loaded", "Clip audio charg\u00e9 : {0}" },
		{ "appletcontext.image.loaded", "Image charg\u00e9e : {0}" },

		{ "securitymgr.automation.printing", "Automatisation : Accepter l'impression" },

		{ "classloaderinfo.referencing", "R\u00e9f\u00e9rence au chargeur de classes : {0}, refcount={1}" },
		{ "classloaderinfo.releasing", "Lib\u00e9ration du chargeur de classes : {0}, refcount={1}" },
		{ "classloaderinfo.caching", "Mise en cache du chargeur de classes : {0}" },
		{ "classloaderinfo.cachesize", "Taille de cache du chargeur de classes courant : {0}" },
		{ "classloaderinfo.num", "Nombre de chargeurs de classes mis en cache sur {0}, sans r\u00e9f\u00e9rence {1}" },

	{ "jsobject.eval", "JSObject::eval({0})" },
	{ "jsobject.call", "JSObject::call: name={0}" },
	{ "jsobject.getMember", "JSObject::getMember: name={0}" },
	{ "jsobject.setMember", "JSObject::setMember: name={0}" },
	{ "jsobject.removeMember", "JSObject::removeMember: name={0}" },
	{ "jsobject.getSlot", "JSObject::getSlot: {0}" },
	{ "jsobject.setSlot", "JSObject::setSlot: slot={0}" },
	{ "jsobject.invoke.url.permission", "l''url de l''applet est {0} et l''autorisation est \u00e9gale \u00e0 {1}"},

		{ "optpkg.install.info", "Installation du package facultatif {0}" },
		{ "optpkg.install.fail", "L'installation du package facultatif a \u00e9chou\u00e9." },
		{ "optpkg.install.ok", "L'installation du package facultatif a r\u00e9ussi." },
		{ "optpkg.install.automation", "Automatisation : Accepter l'installation de modules Java optionnels" },
		{ "optpkg.install.granted", "T\u00e9l\u00e9chargement du package facultatif autoris\u00e9 par l''utilisateur, t\u00e9l\u00e9charger \u00e0 partir de {0}" },
		{ "optpkg.install.deny", "T\u00e9l\u00e9chargement du package facultatif non autoris\u00e9 par l'utilisateur" },
		{ "optpkg.install.begin", "Installation {0}" },
		{ "optpkg.install.java.launch", "D\u00e9marrage du programme d'installation Java" },
		{ "optpkg.install.java.launch.command", "D\u00e9marrage du programme d''installation Java par {0}" },
		{ "optpkg.install.native.launch", "D\u00e9marrage du programme d'installation natif" },
		{ "optpkg.install.native.launch.fail.0", "Impossible d''ex\u00e9cuter {0}" },
		{ "optpkg.install.native.launch.fail.1", "L''acc\u00e8s \u00e0 {0} a \u00e9chou\u00e9" },
		{ "optpkg.install.raw.launch", "Installation du package facultatif brut" },
		{ "optpkg.install.raw.copy", "Copie du package facultatif brut \u00e0 partir de {0} vers {1}" },
		{ "optpkg.install.error.nomethod", "Fournisseur d'extension d\u00e9pendant non install\u00e9 : Impossible d'obtenir la m\u00e9thode "
		  	                     + " addExtensionInstallationProvider" },
		{ "optpkg.install.error.noclass", "Fournisseur d'extension d\u00e9pendant non install\u00e9 : Impossible d'obtenir la classe "
		                     + "sun.misc.ExtensionDependency" },

		{"progress_dialog.downloading", "Plug-in : T\u00e9l\u00e9chargement..."},
        {"progress_dialog.dismiss_button", "Abandonner"},
        {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_A)},
        {"progress_dialog.from", "\u00e0 partir de"},

        {"applet_viewer.color_tag", "Nombre de composants incorrect dans {0}"},

        {"progress_info.downloading", "T\u00e9l\u00e9chargement de fichiers JAR"},
        {"progress_bar.preload", "Pr\u00e9-chargement des fichiers JAR : {0}"},

        {"cache.size", "Taille du cache : {0}"},
        {"cache.cleanup", "La taille du cache est : {0} octets. Vous devez lib\u00e9rer de l''espace"},
        {"cache.full", "Le cache est plein : suppression du fichier {0}"},
        {"cache.inuse", "Impossible de supprimer le fichier {0} parce qu''il est utilis\u00e9 par cette application"},
        {"cache.notdeleted", "Impossible de supprimer le fichier {0} car il peut \u00eatre en cours d''utilisation par cette et/ou une autre application"},
        {"cache.out_of_date", "La copie du cache de {0} n''est pas actualis\u00e9e\n  Copie du cache : {1}\n  Copie Serveur : {2}"},
        {"cache.loading", "Chargement de {0} \u00e0 partir du cache"},
        {"cache.cache_warning", "ATTENTION : Impossible de mettre {0} en cache"},
        {"cache.downloading", "T\u00e9l\u00e9chargement de {0} dans le cache"},
        {"cache.cached_name", "Nom du fichier mis en cache : {0}"},
        {"cache.load_warning", "ATTENTION : erreur de lecture de {0} \u00e0 partir du cache."},
        {"cache.disabled", "Le cache est d\u00e9sactiv\u00e9 par l'utilisateur"},
        {"cache.minSize", "Le cache est d\u00e9sactiv\u00e9, la limite du cache est d\u00e9finie \u00e0 {0}, au moins 5 Mo doivent \u00eatre sp\u00e9cifi\u00e9s"},
        {"cache.directory_warning", "ATTENTION : {0} n''est pas un r\u00e9pertoire. La fonction de cache sera d\u00e9sactiv\u00e9e."},
       	{"cache.response_warning", "ATTENTION : R\u00e9ponse inattendue {0} pour {1}.  Le fichier sera de nouveau t\u00e9l\u00e9charg\u00e9."},
        {"cache.enabled", "Le cache est activ\u00e9"},
        {"cache.location", "Emplacement : {0}"},
        {"cache.maxSize", "Taille maximale : {0}"},
        {"cache.create_warning", "ATTENTION : Impossible de cr\u00e9er le r\u00e9pertoire du cache {0}.  La mise en cache sera d\u00e9sactiv\u00e9e."},
        {"cache.read_warning", "ATTENTION : Impossible de lire le r\u00e9pertoire du cache {0}.  La mise en cache sera d\u00e9sactiv\u00e9e."},
        {"cache.write_warning", "ATTENTION : Impossible d''\u00e9crire dans le r\u00e9pertoire du cache {0}.  La mise en cache sera d\u00e9sactiv\u00e9e."},
        {"cache.compression", "Niveau de compression : {0}"},
        {"cache.cert_load", "Les certificats pour {0} sont lus \u00e0 partir du cache JAR"},
	{"cache.jarjar.invalid_file", "Le fichier .jarjar contient un fichier qui n'est pas au format .jar"},
	{"cache.jarjar.multiple_jar", "Le fichier .jarjar contient plusieurs fichiers au format .jar"},
        {"cache.version_checking", "V\u00e9rification de version pour {0}. La version sp\u00e9cifi\u00e9e est {1}"},
        {"cache.preloading", "Pr\u00e9-chargement du fichier {0}"},

	{ "cache_viewer.caption", "Visualiseur du cache de l'applet Java" },
	{ "cache_viewer.refresh", "Actualiser" },
	{ "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_A) },
	{ "cache_viewer.remove", "Supprimer" },
	{ "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_S) },
	{ "cache_viewer.OK", "OK" },
	{ "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_O) },
	{ "cache_viewer.name", "Nom" },
	{ "cache_viewer.type", "Type" },
	{ "cache_viewer.size", "Taille" },
	{ "cache_viewer.modify_date", "Derni\u00e8re modification" },
	{ "cache_viewer.expiry_date", "Date d'expiration" },
	{ "cache_viewer.url", "URL" },
	{ "cache_viewer.version", "Version" },
	{ "cache_viewer.help.name", "Nom du fichier mis en cache" },
	{ "cache_viewer.help.type", "Type du fichier mis en cache" },
	{ "cache_viewer.help.size", "Taille du fichier mis en cache" },
	{ "cache_viewer.help.modify_date", "Derni\u00e8re modification du fichier mis en cache" },
	{ "cache_viewer.help.expiry_date", "Date d'expiration du fichier mis en cache" },
	{ "cache_viewer.help.url", "URL de t\u00e9l\u00e9chargement du fichier mis en cache" },
	{ "cache_viewer.help.version", "Version du fichier mis en cache" },
	{ "cache_viewer.delete.text", "<html><b>Fichier non supprim\u00e9</b></html>{0} est peut-\u00eatre en cours d''utilisation.\n" },
	{ "cache_viewer.delete.caption", "Erreur - Cache" },
	{ "cache_viewer.type.zip", "Jar" },
	{ "cache_viewer.type.class", "Classe" },
	{ "cache_viewer.type.wav", "Son Wav" },
	{ "cache_viewer.type.au", "Son Au" },
	{ "cache_viewer.type.gif", "Image Gif" },
	{ "cache_viewer.type.jpg", "Image Jpeg" },
	{ "cache_viewer.menu.file", "Fichier" },
        { "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_F) },
        { "cache_viewer.menu.options", "Options" },
        { "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_P) },
        { "cache_viewer.menu.help", "Aide" },
        { "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_D) },
        { "cache_viewer.menu.item.exit", "Quitter" },
        { "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_Q) },
        { "cache_viewer.disable", "Activer la mise en cache" },
        { "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_H) },
        { "cache_viewer.menu.item.about", "A propos de" },
        { "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_D) },

	{ "net.proxy.auto.result.error", "Impossible de d\u00e9terminer le r\u00e9glage proxy de l''\u00e9valuation - repli sur DIRECTE"},

	{ "lifecycle.applet.found", "Le pr\u00e9c\u00e9dent applet interrompu a \u00e9t\u00e9 trouv\u00e9 dans le cache de cycle de vie" },
	{ "lifecycle.applet.support", "L'applet prend en charge les mod\u00e8les de cycle de vie h\u00e9rit\u00e9s - ajout de l'applet au cache de cycle de vie" },
	{ "lifecycle.applet.cachefull", "Le cache de cycle de vie est plein - veuillez supprimer au moins les applets utilis\u00e9s r\u00e9cemment" },

	{ "com.method.ambiguous", "Impossible de s\u00e9lectionner une m\u00e9thode, param\u00e8tres ambigus" },
	{ "com.method.notexists", "{0} : cette m\u00e9thode n''existe pas" },
	{ "com.notexists", "{0} : cette m\u00e9thode/propri\u00e9t\u00e9  n''existe pas" },
	{ "com.method.invoke", "Appel de la m\u00e9thode : {0}" },
	{ "com.method.jsinvoke", "Appel de la m\u00e9thode JS : {0}" },
	{ "com.method.argsTypeInvalid", "Les param\u00e8tres ne peuvent pas \u00eatre convertis selon les types requis" },
	{ "com.method.argCountInvalid", "Le nombre d'arguments est incorrect" },
	{ "com.field.needsConversion", "Conversion requise : {0} --> {1}" },
	{ "com.field.typeInvalid", " impossible de convertir selon le type : {0}" },
	{ "com.field.get", "R\u00e9cup\u00e9ration de la propri\u00e9t\u00e9 : {0}" },
	{ "com.field.set", "D\u00e9finition de la propri\u00e9t\u00e9 : {0}" },

	{ "rsa.cert_expired", "<html><b>Le certificat a expir\u00e9</b></html>Le code sera consid\u00e9r\u00e9 comme \u00e9tant non sign\u00e9.\n" },
	{ "rsa.cert_notyieldvalid", "<html><b>Le certificat n'est pas valide</b></html>Le code sera consid\u00e9r\u00e9 comme \u00e9tant non sign\u00e9.\n" },
	{ "rsa.general_error", "<html><b>Le certificat n'a pas \u00e9t\u00e9 v\u00e9rifi\u00e9</b></html>Le code sera consid\u00e9r\u00e9 comme \u00e9tant non sign\u00e9.\n" },

        { "dialogfactory.menu.show_console", "Afficher la console Java" },
        { "dialogfactory.menu.hide_console", "Masquer la console Java" },
        { "dialogfactory.menu.about", "A propos du plug-in Java" },
        { "dialogfactory.menu.copy", "Copier" },
	{ "dialogfactory.menu.open_console", "Ouvrir la console Java" },
	{ "dialogfactory.menu.about_java", "A propos de Java(TM)" },

    };
}


