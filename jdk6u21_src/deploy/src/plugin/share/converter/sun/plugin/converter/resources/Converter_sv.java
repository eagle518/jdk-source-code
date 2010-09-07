/*
 * @(#)Converter_sv.java	1.44 10/04/22
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

public class Converter_sv extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "Fel" },
	{ "caption.warning", "Varning" },
	{ "caption.absdirnotfound", "Hittar inte absolut katalog" },
	{ "caption.reldirnotfound", "Hittar inte relativ katalog" },
        { "about_dialog.info", "Java(TM) Plug-in HTML Converter {0}" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "Om Java(TM) instick-HTML-konverterare" },
	{ "nottemplatefile_dialog.caption", "Ingen mallfil"},
	{ "nottemplatefile_dialog.info0", "Den angivna mallfilen" + newline +
                                          " {0} " + newline + 
					  "\u00e4r inte giltig.  Filnamnet m\u00e5ste sluta " + newline +
					  "med suffixet .tpl" + newline + newline +
                                          "\u00c5terst\u00e4llning av mallfilen till standard."},
	{ "warning_dialog.info", "Backup-mappen och destinationsmappen kan inte" + newline +
	                         "ha samma s\u00f6kv\u00e4g.  Vill du \u00e4ndra" + newline +
	                         "backup-mappens s\u00f6kv\u00e4g till f\u00f6ljande: " + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "Hittar inte mallfil"},
        { "notemplate_dialog.info", "Standardmallfil({0})" + newline +
                                    "kunde inte hittas.  Antingen finns den inte i classpath" + newline +
                                    "eller s\u00e5 finns den inte i arbetsbiblioteket."},
        { "file_unwritable.info", "Kan inte skriva till filen: "},
	{ "file_notexists.info", "Filen finns inte: "},
	{ "illegal_source_and_backup.info", "Destinationsmappen och mappen f\u00f6r s\u00e4kerhetskopior kan inte vara samma!"},
	{ "button.reset", "\u00c5terst\u00e4ll standardv\u00e4rden"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "button.okay", "OK"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "Avbryt"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "button.done", "Klar"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_K)},
	{ "button.browse.dir", "Bl\u00e4ddra..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_B)},
        { "button.browse.backup", "Bl\u00e4ddra..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_L)},
	{ "button.convert", "Konvertera..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_K)},

	{ "advanced_dialog.caption", "Avancerade alternativ"},
	{ "advanced_dialog.cab", "Ange k\u00e4llplats f\u00f6r ActiveX CAB-fil:"},
	{ "advanced_dialog.plugin", "Ange k\u00e4llplats f\u00f6r Netscape Plug-In:"},
	{ "advanced_dialog.smartupdate", "Ange k\u00e4llplats f\u00f6r Netscape SmartUpdate:"},
	{ "advanced_dialog.mimetype", "Ange MIME-typ f\u00f6r Java Plug-In HTML-konvertering:"},
	{ "advanced_dialog.log", "Ange plats f\u00f6r logfil:"},
	{ "advanced_dialog.generate", "Generera loggfil"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "Status..."},
	{ "progress_dialog.processing", "Bearbetar..."},
	{ "progress_dialog.folder", "Mapp:"},
	{ "progress_dialog.file", "Fil:"},
	{ "progress_dialog.totalfile", "Antal bearbetade filer:"},
	{ "progress_dialog.totalapplet", "Antal funna appletprogram:"},
	{ "progress_dialog.totalerror", "Totalt antal fel:"},

	{ "notdirectory_dialog.caption0", "Ogiltig fil"},
	{ "notdirectory_dialog.caption1", "Ogiltig mapp"},
        { "notdirectory_dialog.info0", "F\u00f6ljande mapp existerar ej" + newline + "{0}"},
        { "notdirectory_dialog.info1", "F\u00f6ljande fil existerar ej" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "F\u00f6ljande mapp existerar ej" + newline + "<tom>"},
        
	{ "converter_gui.lablel0", "Ange fil eller s\u00f6kv\u00e4g:"},
	{ "converter_gui.lablel1", "\u00d6verensst\u00e4mmande filnamn:"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "Ta med underliggande mappar"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_T)},
	{ "converter_gui.lablel4", "En fil:"},
	{ "converter_gui.lablel5", "S\u00e4kerhetskopiera filer till mapp:"},
	{ "converter_gui.lablel7", "Mappfil:"},


	{ "template.default", "Endast standard (IE & Navigator) f\u00f6r Windows & Solaris"},
	{ "template.extend",  "Ut\u00f6kad (Standard + Alla webbl\u00e4sare/plattformar)"},
	{ "template.ieonly",  "Endast Internet Explorer f\u00f6r Windows & Solaris"},
	{ "template.nsonly",  "Endast Navigator f\u00f6r Windows"},
	{ "template.other",   "Annan mall..."},

        { "template_dialog.title", "V\u00e4lj fil"},
	
        { "help_dialog.caption", "Hj\u00e4lp"},
        { "help_dialog.error", "Det gick inte att \u00f6ppna hj\u00e4lpfilen."},

	{ "menu.file", "Arkiv"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "menu.exit", "Avsluta"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "menu.edit", "Redigera"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "menu.option", "Alternativ"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "menu.help", "Hj\u00e4lp"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "menu.about", "Om"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_O)},

        { "static.versioning.label", "Java-versionering f\u00f6r Appletprogram:"},
        { "static.versioning.radio.button", "Anv\u00e4nd endast JRE {0}"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "static.versioning.text", "Appletprogram kommer endast att anv\u00e4nda denna specifika versionen av JRE. Om den inte \u00e4r installerad, kommer den, om m\u00f6jligt, att automatiskt laddas ner. Annars kommer anv\u00e4ndaren till en sida f\u00f6r manuell nerladdning. Se  f\u00f6r detaljer om den automatiska nerladdningsprocessen och End of Life (EOL)-policy f\u00f6r alla Java-releaser."},
        { "dynamic.versioning.radio.button", "Anv\u00e4nd vilken JRE {0} som helst eller h\u00f6gre"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_V)},
        { "dynamic.versioning.text", "Om ingen s\u00e5dan version finns installerad kommer den nuvarande standardnerladdingen f\u00f6r JRE {0}-familjen, om m\u00f6jligt, att laddas ner automatiskt.  Annars kommer anv\u00e4ndaren att omdirigeras till en manuell nerladdnignssida."},
        
	{ "progress_event.preparing", "F\u00f6rbereder"},
	{ "progress_event.converting", "Konverterar"},
	{ "progress_event.copying", "Kopierar"},
	{ "progress_event.done", "Klar"},
	{ "progress_event.destdirnotcreated", "Kunde inte skapa m\u00e5lkatalog."},
	{ "progress_event.error", "Fel"},
	
	{ "plugin_converter.logerror", "Loggfilsutdata kunde inte fastst\u00e4llas"},
	{ "plugin_converter.saveerror", "Kunde inte spara egenskapsfilen: "},
	{ "plugin_converter.appletconv", "Konvertering av appletprogram"},
	{ "plugin_converter.failure", "Kunde inte konvertera filen "},
	{ "plugin_converter.overwrite1", "Det finns redan en s\u00e4kerhetskopia f\u00f6r..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "Vill du skriva \u00f6ver den h\u00e4r s\u00e4kerhetskopian?"},
	{ "plugin_converter.done", "Alla klara  Bearbetade filer: "},
	{ "plugin_converter.appletfound", "  Antal funna appletprogram: "},
	{ "plugin_converter.processing", "  Bearbetar..."},
	{ "plugin_converter.cancel", "Konverteringen avbr\u00f6ts"},
	{ "plugin_converter.files", "Filer som ska konverteras: "},
	{ "plugin_converter.converted", "Filen redan konverterad. Ingen konvertering beh\u00f6vs. "},
	{ "plugin_converter.donefound", "Klart  Antal funna appletprogram: "},
	{ "plugin_converter.seetrace", "Fel i fil - se sp\u00e5rning nedan"},
	{ "plugin_converter.noapplet", "Inga appletprogram i filen "},
	{ "plugin_converter.nofiles", "Finns inga filer att bearbeta "},
	{ "plugin_converter.nobackuppath", "Skapade inte backups\u00f6kv\u00e4g"},
	{ "plugin_converter.writelog", "Skriver \u00f6ver loggfilen med samma namn"},
	{ "plugin_converter.backup_path", "Backups\u00f6kv\u00e4g"},
	{ "plugin_converter.log_path", "Loggs\u00f6kv\u00e4g"},
	{ "plugin_converter.template_file", "Mallfil"},
	{ "plugin_converter.process_subdirs", "Underordnade kataloger som ska behandlas"},
	{ "plugin_converter.show_progress", "Visa f\u00f6rlopp"},
	{ "plugin_converter.write_permission", "Du m\u00e5ste ha beh\u00f6righet att skriva i den aktuella arbetskatalogen"},
	{ "plugin_converter.overwrite", "Den tempor\u00e4ra filen .tmpSource_stdin finns redan. Ta bort filen eller d\u00f6p om den."},
	{ "plugin_converter.help_message", newline +
                                      "G\u00f6r s\u00e5 h\u00e4r: HtmlConverter [-alternativ1 v\u00e4rde1 [-alternativ2 v\u00e4rde2 [...]]] [-simulate]  [fillista]" + newline + newline +
                                      "d\u00e4r alternativen omfattar:" + newline + newline +
                                      "    -source:    S\u00f6kv\u00e4g till originalfiler.  Standard: <anv\u00e4ndarkatalog>" + newline +
                                          "    -source -:  L\u00e4s konverteringsfil fr\u00e5n standardinmatning" + newline +
                                      "    -dest:      S\u00f6kv\u00e4g f\u00f6r att skriva konverterade filer.  Standard: <anv\u00e4ndarkatalog>" + newline +
                                          "    -dest -:    Skriv konverterad fil till standardutmatning" + newline +
                                      "    -backup:    S\u00f6kv\u00e4g f\u00f6r att spara backupfiler.  Standard: <katalognamn>_BAK" + newline +
                                      "    -f    Tvinga \u00f6verskrivning av backup-filer." + newline +
                                      "    -subdirs:   Ska filer i underordnade kataloger behandlas." + newline +
                                      "    -template:  S\u00f6kv\u00e4g till mallfil.  Anv\u00e4nd standard vid tveksamheter." + newline +
                                      "    -log:       S\u00f6kv\u00e4g till logg.  Om den inte anges, skrivs ingen logg." + newline +
                                      "    -progress:  Visa f\u00f6rlopp under konvertering.  Standard: false" + newline +
                                      "    -simulate:  Visa detaljerna i konverteringen utan att konvertera." + newline +
		          "    -latest:	 Anv\u00e4nd den senaste JRE som st\u00f6der release-mimetypen." + newline +
                                      "    -gui:       Visa det grafiska anv\u00e4ndargr\u00e4nssnittet f\u00f6r konverteringen." + newline + newline +
                                      "    filespecs:  Blankstegsformaterad lista \u00f6ver filspecifikationer.  Standard: \"*.html *.htm\" (citattecken kr\u00e4vs)" + newline},
	
	{ "product_name", "Java(TM) insticks-HTML-konverterare" },
    };
}

