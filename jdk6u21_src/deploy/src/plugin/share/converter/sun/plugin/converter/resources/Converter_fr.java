/*
 * @(#)Converter_fr.java	1.44 10/04/22
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Java Plug-in HTML Converter strings.
 *
 * @author Stanley Man-Kit Ho
 */

public class Converter_fr extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "Erreur" },
	{ "caption.warning", "Avertissement" },
	{ "caption.absdirnotfound", "R\u00e9pertoire absolu introuvable" },
	{ "caption.reldirnotfound", "R\u00e9pertoire relatif introuvable" },
        { "about_dialog.info", "Convertisseur HTML v{0} du plug-in Java(TM)" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "A propos du Convertisseur HTML du plug-in Java(TM) " },
	{ "nottemplatefile_dialog.caption", "Ce n''est pas un fichier mod\u00e8le"},
	{ "nottemplatefile_dialog.info0", "Le fichier mod\u00e8le sp\u00e9cifi\u00e9 " + newline +
                                          " {0} " + newline + 
					  "n''est pas un fichier mod\u00e8le valide.  Ce fichier doit se terminer par" + newline +
					  "l''extention .tpl" + newline + newline +
                                          "Restauration du fichier mod\u00e8le par d\u00e9faut."},
	{ "warning_dialog.info", "Le dossier de sauvegarde et le dossier de destination ne peuvent pas " + newline +
	                         "avoir le m\u00eame chemin d''acc\u00e8s.  Voulez-vous remplacer le chemin d''acc\u00e8s du dossier" + newline +
	                         "de sauvegarde par ce qui suit : " + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "Fichier mod\u00e8le introuvable"},
        { "notemplate_dialog.info", "Le fichier mod\u00e8le par d\u00e9faut ({0})" + newline +
                                    "est introuvable.  Soit il ne figure pas dans le chemin de classe," + newline +
                                    "soit il ne se trouve pas dans le r\u00e9pertoire de travail."},
        { "file_unwritable.info", "Impossible d'\u00e9crire dans le fichier : "},
	{ "file_notexists.info", "Le fichier n'existe pas : "},
	{ "illegal_source_and_backup.info", "Les r\u00e9pertoires de destination et de sauvegarde doivent \u00eatre diff\u00e9rents !"},
	{ "button.reset", "Restaurer les valeurs par d\u00e9faut"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "OK"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "Annuler"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "button.done", "Termin\u00e9"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_T)},
	{ "button.browse.dir", "Parcourir..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_P)},
        { "button.browse.backup", "Parcourir..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "button.convert", "Convertir..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_C)},

	{ "advanced_dialog.caption", "Options avanc\u00e9es"},
	{ "advanced_dialog.cab", "Sp\u00e9cifier l'emplacement source du fichier ActiveX CAB :"},
	{ "advanced_dialog.plugin", "Sp\u00e9cifier l'emplacement source du plug-in Netscape :"},
	{ "advanced_dialog.smartupdate", "Sp\u00e9cifier l'emplacement source de Netscape SmartUpdate :"},
	{ "advanced_dialog.mimetype", "Sp\u00e9cifier le type MIME pour la conversion HTML du plug-in Java :"},
	{ "advanced_dialog.log", " Sp\u00e9cifier l'emplacement du fichier journal :"},
	{ "advanced_dialog.generate", "G\u00e9n\u00e9rer le fichier journal"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "Progression..."},
	{ "progress_dialog.processing", "Traitement en cours..."},
	{ "progress_dialog.folder", "Dossier :"},
	{ "progress_dialog.file", "Fichier :"},
	{ "progress_dialog.totalfile", "Nombre de fichiers trait\u00e9s :"},
	{ "progress_dialog.totalapplet", "Nombre d'applets trouv\u00e9s :"},
	{ "progress_dialog.totalerror", "Nombre total d'erreurs :"},

	{ "notdirectory_dialog.caption0", "Fichier non valide"},
	{ "notdirectory_dialog.caption1", "Dossier non valide"},
        { "notdirectory_dialog.info0", "Le dossier suivant n''existe pas" + newline + "{0}"},
        { "notdirectory_dialog.info1", "Le fichier suivant n''existe pas" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "Le dossier suivant n'existe pas " + newline + "<vide>"},
        
	{ "converter_gui.lablel0", "Sp\u00e9cifiez un fichier ou un chemin de r\u00e9pertoire :"},
	{ "converter_gui.lablel1", "Noms de fichiers correspondants :"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "Inclure les sous-dossiers"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "Un fichier :"},
	{ "converter_gui.lablel5", "Sauvegarder les fichiers dans le dossier :"},
	{ "converter_gui.lablel7", "Fichier mod\u00e8le :"},


	{ "template.default", "Standard (IE & Netscape Navigator) pour Windows & Solaris uniquement"},
	{ "template.extend",  "Etendu (standard + tous les navigateurs et toutes les plates-formes)"},
	{ "template.ieonly",  "Internet Explorer pour Windows & Solaris uniquement"},
	{ "template.nsonly",  "Netscape Navigator pour Windows uniquement"},
	{ "template.other",   "Autre mod\u00e8le..."},

        { "template_dialog.title", "S\u00e9lectionnez Fichier"},
	
        { "help_dialog.caption", "Aide"},
        { "help_dialog.error", "Impossible d'acc\u00e9der au fichier d'aide"},

	{ "menu.file", "Fichier"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "menu.exit", "Quitter"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_Q)},
	{ "menu.edit", "Edition"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.option", "Options"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "Aide"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "menu.about", "A propos de"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_A)},

        { "static.versioning.label", "Version Java pour les applets :"},
        { "static.versioning.radio.button", "Utilisez uniquement JRE {0}"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "Les applets n'utiliseront que cette version de JRE. Si elle n'est pas install\u00e9e, cette version sera t\u00e9l\u00e9charg\u00e9e automatiquement. Si ce n'est pas possible, l'utilisateur sera redirig\u00e9 vers une page o\u00f9 il pourra la t\u00e9l\u00e9charger manuellement. Veuillez consulter le site http://java.sun.com/products/plugin pour obtenir de plus amples renseignements sur le processus de t\u00e9l\u00e9chargement automatique et sur les politiques de fin de vie pour tous les produits Java."},
        { "dynamic.versioning.radio.button", "Utilisez uniquement JRE {0}, ou une ult\u00e9rieure"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "Si cette version n''est pas install\u00e9e, l''option de t\u00e9l\u00e9chargement par d\u00e9faut en cours pour la famille JRE {0} est t\u00e9l\u00e9charg\u00e9e automatiquement, quand c''est possible.  Autrement, l''utilisateur est redirig\u00e9 vers une page de t\u00e9l\u00e9chargement manuel."},
        
	{ "progress_event.preparing", "Pr\u00e9paration en cours"},
	{ "progress_event.converting", "Conversion en cours"},
	{ "progress_event.copying", "Copie en cours"},
	{ "progress_event.done", "Termin\u00e9"},
	{ "progress_event.destdirnotcreated", "Impossible de cr\u00e9er un r\u00e9pertoire de destination."},
	{ "progress_event.error", "Erreur"},
	
	{ "plugin_converter.logerror", "La sortie du fichier journal n'a pas pu \u00eatre \u00e9tablie"},
	{ "plugin_converter.saveerror", "Impossible d'enregistrer le fichier de propri\u00e9t\u00e9s :  "},
	{ "plugin_converter.appletconv", "Conversion d'applet "},
	{ "plugin_converter.failure", "Impossible de convertir le fichier "},
	{ "plugin_converter.overwrite1", "Une copie de sauvegarde existe d\u00e9j\u00e0 pour..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "Voulez-vous remplacer cette copie de sauvegarde ?"},
	{ "plugin_converter.done", "Termin\u00e9. Fichiers trait\u00e9s :  "},
	{ "plugin_converter.appletfound", "  Applets trouv\u00e9s :  "},
	{ "plugin_converter.processing", "  Traitement en cours..."},
	{ "plugin_converter.cancel", "Conversion annul\u00e9e"},
	{ "plugin_converter.files", "Fichiers \u00e0 convertir : "},
	{ "plugin_converter.converted", "Fichier d\u00e9j\u00e0 converti, aucune conversion n\u00e9cessaire. "},
	{ "plugin_converter.donefound", "Termin\u00e9. Applets trouv\u00e9s :  "},
	{ "plugin_converter.seetrace", "Erreur dans le fichier : voir le suivi ci-dessous"},
	{ "plugin_converter.noapplet", "Aucun applet dans le fichier "},
	{ "plugin_converter.nofiles", "Aucun fichier \u00e0 traiter "},
	{ "plugin_converter.nobackuppath", "Le chemin de sauvegarde n'a pas \u00e9t\u00e9 cr\u00e9\u00e9"},
	{ "plugin_converter.writelog", "Ecriture dans un fichier journal du m\u00eame nom"},
	{ "plugin_converter.backup_path", "Chemin de sauvegarde"},
	{ "plugin_converter.log_path", "Chemin d'acc\u00e8s aux journaux"},
	{ "plugin_converter.template_file", "Fichier mod\u00e8le"},
	{ "plugin_converter.process_subdirs", "Sous-r\u00e9pertoires de traitement"},
	{ "plugin_converter.show_progress", "Afficher la progression"},
	{ "plugin_converter.write_permission", "Vous devez disposer de droits d'\u00e9criture sur le r\u00e9pertoire de travail en cours"},
	{ "plugin_converter.overwrite", "Le fichier temporaire .tmpSource_stdin existe d\u00e9j\u00e0. Veuillez le supprimer ou le renommer."},
	{ "plugin_converter.help_message", newline +
                                      "Syntaxe : HtmlConverter [-option1 valeur1 [-option2 valeur2 [...]]] [-simulate]  [filespecs]" + newline + newline +
                                      "o\u00f9 les options sont :" + newline + newline +
                                      "    -source:    Chemin d'acc\u00e8s aux fichiers originaux.  Par d\u00e9faut : <reputil>" + newline +
                                          "    -source -:  lecture du fichier converti \u00e0 partir de l'entr\u00e9e standard" + newline +
                                      "    -dest:      Chemin d'acc\u00e8s aux fichiers convertis.  Par d\u00e9faut : <reputil>" + newline +
                                          "    -dest -:    \u00e9criture du fichier converti en sortie standard" + newline +
                                      "    -backup:    Chemin d'acc\u00e8s aux fichiers de sauvegarde.  Par d\u00e9faut : <nomrep>_BAK" + newline +
				      "    -f:         Force l'\u00e9crasement des fichiers de sauvegarde." + newline +
                                      "    -subdirs:   Si les fichiers situ\u00e9s dans les sous-r\u00e9pertoires doivent \u00eatre trait\u00e9s." + newline +
                                      "    -template:  Chemin d'acc\u00e8s aux fichier de mod\u00e8le.  En cas de doute, utilisez la valeur par d\u00e9faut." + newline +
                                      "    -log:       Chemin d'acc\u00e8s au journal d'\u00e9criture.  Doit \u00eatre sp\u00e9cifi\u00e9 pour que le journal soit cr\u00e9\u00e9." + newline +
                                      "    -progress:  Indique la progression de la conversion.  Par d\u00e9faut : false" + newline +
                                      "    -simulate:  D\u00e9crit la conversion sans l'ex\u00e9cuter." + newline +
				      "    -latest:    Utilise le dernier JRE prenant en charge le type MIME de la version." + newline +
                                      "    -gui:       Affiche l'interface graphique utilisateur du convertisseur." + newline + newline +
                                      "    filespecs:  Liste de sp\u00e9cifications de fichiers s\u00e9par\u00e9es par des espaces.  Par d\u00e9faut : \"*.html *.htm\" (guillemets obligatoires)" + newline},
	
	{ "product_name", "Java(TM) Plug-in - Convertisseur HTML" },
    };
}

