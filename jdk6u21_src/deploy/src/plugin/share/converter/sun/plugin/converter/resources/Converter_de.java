/*
 * @(#)Converter_de.java	1.41 10/04/22
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

public class Converter_de extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "Fehler" },
	{ "caption.warning", "Warnung" },
	{ "caption.absdirnotfound", "Absolutes Verzeichnis nicht gefunden" },
	{ "caption.reldirnotfound", "Relatives Verzeichnis nicht gefunden" },
        { "about_dialog.info", "HTML-Converter f\u00fcr Java(TM)-Plug-In (Version{0})" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "Info zu Java(TM) Plug-in HTML-Konverter" },
	{ "nottemplatefile_dialog.caption", "Keine Schablonendatei"},
	{ "nottemplatefile_dialog.info0", "Die angegebene Schablonendatei " + newline +
                                          " {0} " + newline + 
					  "ist keine g\u00fcltige Schablonendatei.  Die Datei muss die" + newline +
					  "Erweiterung .tpl aufweisen." + newline + newline +
                                          "Schablonendatei wird auf Standarddatei zur\u00fcckgesetzt."},
	{ "warning_dialog.info", "Der Sicherungs- und Zielordner k\u00f6nnen nicht  " + newline +
	                         "denselben Pfad haben.  Soll der Pfad des " + newline +
	                         "Sicherungsordners ge\u00e4ndert werden in: " + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "Schablonendatei nicht gefunden"},
        { "notemplate_dialog.info", "Die Standard-Schablonendatei ({0})" + newline +
                                    "wurde nicht gefunden.  Sie befindet sich weder im classpath" + newline +
                                    "noch im Arbeitsverzeichnis."},
        { "file_unwritable.info", "In die Datei kann nicht geschrieben werden: "},
	{ "file_notexists.info", "Die Datei ist nicht vorhanden: "},
	{ "illegal_source_and_backup.info", "Ziel- und Sicherungsverzeichnisse d\u00fcrfen nicht identisch sein!"},
	{ "button.reset", "Auf Standardwerte zur\u00fccksetzen"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "button.okay", "OK"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "Abbrechen"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "button.done", "Fertig"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "button.browse.dir", "Durchsuchen..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_R)},
        { "button.browse.backup", "Durchsuchen..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.convert", "Konvertieren..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_K)},

	{ "advanced_dialog.caption", "Erweiterte Optionen"},
	{ "advanced_dialog.cab", "Geben Sie den Quellspeicherort f\u00fcr die ActiveX CAB-Datei an:"},
	{ "advanced_dialog.plugin", "Geben Sie den Quellspeicherort f\u00fcr das Netscape Plug-in an:"},
	{ "advanced_dialog.smartupdate", "Geben Sie das Quellverzeichnis f\u00fcr das Netscape-SmartUpdate an:"},
	{ "advanced_dialog.mimetype", "Geben Sie den MIME-Typ f\u00fcr die Java Plug-in HTML-Konvertierung an:"},
	{ "advanced_dialog.log", "Geben Sie den Speicherort f\u00fcr die Protokolldatei an:"},
	{ "advanced_dialog.generate", "Protokolldatei wird erzeugt"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_P)},

	{ "progress_dialog.caption", "Fortschritt..."},
	{ "progress_dialog.processing", "Verarbeitung l\u00e4uft..."},
	{ "progress_dialog.folder", "Ordner:"},
	{ "progress_dialog.file", "Datei:"},
	{ "progress_dialog.totalfile", "Gesamtzahl der verarbeiteten Dateien:"},
	{ "progress_dialog.totalapplet", "Gesamtzahl der gefundenen Applets:"},
	{ "progress_dialog.totalerror", "Gesamtzahl der Fehler:"},

	{ "notdirectory_dialog.caption0", "Keine g\u00fcltige Datei"},
	{ "notdirectory_dialog.caption1", "Kein g\u00fcltiger Ordner"},
        { "notdirectory_dialog.info0", "Der folgende Ordner ist nicht vorhanden" + newline + "{0}"},
        { "notdirectory_dialog.info1", "Die folgende Datei ist nicht vorhanden" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "Der folgende Ordner ist nicht vorhanden " + newline + "<leer>"},
        
	{ "converter_gui.lablel0", "Geben Sie eine Datei oder einen Verzeichnispfad an:"},
	{ "converter_gui.lablel1", "\u00dcbereinstimmende Dateinamen:"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "Untergeordnete Ordner aufnehmen"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_U)},
	{ "converter_gui.lablel4", "Eine Datei:"},
	{ "converter_gui.lablel5", "Sicherungsdateien in Ordner:"},
	{ "converter_gui.lablel7", "Schablonendatei:"},


	{ "template.default", "Standard (IE & Navigator) nur f\u00fcr Windows u. Solaris"},
	{ "template.extend",  "Erweitert (Standard + s\u00e4mtliche Browser/Plattformen)"},
	{ "template.ieonly",  "Internet Explorer nur f\u00fcr Windows & Solaris"},
	{ "template.nsonly",  "Navigator nur f\u00fcr Windows"},
	{ "template.other",   "Andere Schablone..."},

        { "template_dialog.title", "Datei ausw\u00e4hlen"},
	
        { "help_dialog.caption", "Hilfe"},
        { "help_dialog.error", "Zugriff auf die Hilfedatei nicht m\u00f6glich"},

	{ "menu.file", "Datei"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "menu.exit", "Beenden"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_B)},
	{ "menu.edit", "Bearbeiten"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_B)},
	{ "menu.option", "Optionen"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "Hilfe"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "menu.about", "Info zu"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_I)},

        { "static.versioning.label", "Java-Version f\u00fcr Applets:"},
        { "static.versioning.radio.button", "Nur JRE {0} verwenden"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "static.versioning.text", "Applets werden nur diese bestimmte JRE-Version verwenden. Falls diese Version nicht installiert ist, wird sie automatisch heruntergeladen, sofern dies m\u00f6glich ist. Andernfalls wird der Benutzer auf eine Seite zum manuellen Download weitergeleitet. Detaillierte Informationen zum automatischen Download-Prozess und den End-of-Life- (EOL) Grunds\u00e4tzen f\u00fcr alle Java-Releases finden Sie unter http://java.sun.com/products/plugin."},
        { "dynamic.versioning.radio.button", "Beliebiges JRE {0} oder h\u00f6her verwenden"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "Wenn keine derartige Version installiert ist, wird die aktuelle Standard-Download-Datei f\u00fcr die JRE {0}-Familie automatisch heruntergeladen, sofern dies m\u00f6glich ist.  Andernfalls wird der Benutzer auf eine Seite zum manuellen Download weitergeleitet."},
        
	{ "progress_event.preparing", "Vorbereitung"},
	{ "progress_event.converting", "Konvertierung"},
	{ "progress_event.copying", "Kopiervorgang"},
	{ "progress_event.done", "Fertig"},
	{ "progress_event.destdirnotcreated", "Das Zielverzeichnis konnte nicht erstellt werden."},
	{ "progress_event.error", "Fehler"},
	
	{ "plugin_converter.logerror", "Ausgabe der Protokolldatei nicht m\u00f6glich"},
	{ "plugin_converter.saveerror", "Eigenschaftendatei konnte nicht gespeichert werden:"},
	{ "plugin_converter.appletconv", "Applet-Konvertierung"},
	{ "plugin_converter.failure", "Die Datei konnte nicht konvertiert werden "},
	{ "plugin_converter.overwrite1", "Es existiert bereits eine Sicherungskopie von..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "M\u00f6chten Sie diese Sicherungskopie \u00fcberschreiben?"},
	{ "plugin_converter.done", "Alle verarbeiteten Dateien:"},
	{ "plugin_converter.appletfound", "Gefundene Applets:"},
	{ "plugin_converter.processing", "Verarbeitung l\u00e4uft..."},
	{ "plugin_converter.cancel", "Konvertierung abgebrochen"},
	{ "plugin_converter.files", "Zu konvertierende Dateien: "},
	{ "plugin_converter.converted", "Datei wurde bereits konvertiert, Konvertierung nicht erforderlich."},
	{ "plugin_converter.donefound", "Gefundene fertiggestellte Applets:"},
	{ "plugin_converter.seetrace", "Dateifehler - siehe nachstehende Ablaufverfolgung"},
	{ "plugin_converter.noapplet", "Keine Applets in der Datei vorhanden"},
	{ "plugin_converter.nofiles", "Es sind keine zu verarbeitende Dateien vorhanden. "},
	{ "plugin_converter.nobackuppath", "Sicherungspfad wurde nicht erstellt."},
	{ "plugin_converter.writelog", "Protokolldatei mit identischem Namen wird \u00fcberschrieben."},
	{ "plugin_converter.backup_path", "Sicherungspfad"},
	{ "plugin_converter.log_path", "Protokollpfad"},
	{ "plugin_converter.template_file", "Schablonendatei"},
	{ "plugin_converter.process_subdirs", "Verarbeitungsunterverzeichnisse"},
	{ "plugin_converter.show_progress", "Fortschrittsanzeige"},
	{ "plugin_converter.write_permission", "Berechtigung zum Schreiben in das aktuelle Arbeitsverzeichnis erforderlich"},
	{ "plugin_converter.overwrite", "Die tempor\u00e4re Datei .tmpSource_stdin ist bereits vorhanden. L\u00f6schen Sie diese Datei, oder \u00e4ndern Sie deren Namen."},
	{ "plugin_converter.help_message", newline +
                                      "Syntax: HtmlConverter [-option1 wert1 [-option2 wert2 [...]]] [-simulate]  [filespecs]" + newline + newline +
                                      "wobei folgende Optionen m\u00f6glich sind:" + newline + newline +
                                      "    -source:    Verzeichnis, aus dem die Ausgangsdateien gelesen werden sollen. Standardwert: <Benutzerverzeichnis>" + newline +
                                          "    -source -:  Konvertierungsdatei aus der Standard-Eingabe lesen." + newline +
                                      "    -dest:      Verzeichnis, in das die konvertierten Dateien geschrieben werden sollen. Standardwert: <Benutzerverzeichnis>" + newline +
                                          "    -dest -:    Konvertierte Datei in die Standard-Ausgabe schreiben." + newline +
                                      "    -backup:    Verzeichnis, in das die Sicherungsdateien geschrieben werden sollen. Standardwert: <Verzeichnisname>_BAK" + newline +
                                          "    -f:         Sicherungsdateien \u00fcberschreiben." + newline +
                                      "    -subdirs:   Falls Dateien in Unterverzeichnissen verarbeitet werden sollen." + newline +
                                      "    -template:  Verzeichnis der Schablonendatei. \u00dcbernehmen Sie die Standardvorgabe, wenn Sie unsicher sind." + newline +
                                      "    -log:       Verzeichnis, in das die Protokolldatei geschrieben werden soll. Wenn keine Angabe erfolgt, wird keine Protokolldatei erstellt." + newline +
                                      "    -progress:  Fortschrittsanzeige w\u00e4hrend der Umwandlung. Standardwert: false" + newline +
                                      "    -simulate:  Anzeige der Umwandlungsdetails ohne tats\u00e4chliche Umwandlung." + newline +
	                                  "    -latest:    Verwenden der neuesten JRE, die den Mimetype des Release unterst\u00fctzt." + newline +
	                        "    -gui:       Anzeige der grafischen Benutzeroberfl\u00e4che f\u00fcr das Umwandlungsprogramm." + newline + newline +
                                      "    filespecs:  Durch Leerzeichen getrennte Liste mit Dateierweiterungen. Standardwert: \"*.html *.htm\" (Anf\u00fchrungszeichen erforderlich)" + newline},
	
	{ "product_name", "Java(TM) Plug-in HTML-Konverter" },
    };
}

