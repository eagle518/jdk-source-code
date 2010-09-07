/*
 * @(#)Converter_it.java	1.42 10/04/22
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

public class Converter_it extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "Errore" },
	{ "caption.warning", "Avviso" },
	{ "caption.absdirnotfound", "Impossibile trovare la cartella assoluta" },
	{ "caption.reldirnotfound", "Impossibile trovare la cartella relativa" },
        { "about_dialog.info", "Java(TM) Plug-in HTML Converter v{0}" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "Informazioni sul convertitore HTML Java(TM) Plug-in" },
	{ "nottemplatefile_dialog.caption", "File modello non valido"},
	{ "nottemplatefile_dialog.info0", "Il file modello specificato " + newline +
                                          " {0} " + newline + 
					  "non \u00e8 un file modello valido.  Il file deve terminare" + newline +
					  "con l''estensione .tpl" + newline + newline +
                                          "Ripristino del file modello predefinito."},
	{ "warning_dialog.info", "La cartella di backup e quella di destinazione non possono " + newline +
	                         "coincidere.  Modificare" + newline +
	                         "il percorso della cartella di backup e impostarlo su: " + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "File modello non trovato"},
        { "notemplate_dialog.info", "Impossibile trovare il file modello ({0})" + newline +
                                    "specificato.  Non \u00e8 nella classpath" + newline +
                                    "oppure non \u00e8 nella directory di lavoro."},
        { "file_unwritable.info", "Impossibile scrivere sul file: "},
	{ "file_notexists.info", "File inesistente: "},
	{ "illegal_source_and_backup.info", "La directory di destinazione e quella di backup non possono coincidere!"},
	{ "button.reset", "Ripristina valori predefiniti"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "OK"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "Annulla"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "button.done", "Chiudi"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.browse.dir", "Sfoglia..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "button.browse.backup", "Sfoglia..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_G)},
	{ "button.convert", "Converti..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_C)},

	{ "advanced_dialog.caption", "Opzioni avanzate"},
	{ "advanced_dialog.cab", "Specifica posizione di origine per file CAB ActiveX:"},
	{ "advanced_dialog.plugin", "Specifica posizione di origine per Netscape Plug-in:"},
	{ "advanced_dialog.smartupdate", "Specificare la posizione di origine di Netscape SmartUpdate:"},
	{ "advanced_dialog.mimetype", "Specificare il tipo MIME per la conversione HTML di Java Plug-In:"},
	{ "advanced_dialog.log", "Specifica posizione per file di log:"},
	{ "advanced_dialog.generate", "Genera file di log"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "Avanzamento..."},
	{ "progress_dialog.processing", "Elaborazione in corso..."},
	{ "progress_dialog.folder", "Cartella:"},
	{ "progress_dialog.file", "File:"},
	{ "progress_dialog.totalfile", "Totale file elaborati:"},
	{ "progress_dialog.totalapplet", "Totale applet trovati:"},
	{ "progress_dialog.totalerror", "Totale errori:"},

	{ "notdirectory_dialog.caption0", "File non valido"},
	{ "notdirectory_dialog.caption1", "Cartella non valida"},
        { "notdirectory_dialog.info0", "La seguente cartella non esiste" + newline + "{0}"},
        { "notdirectory_dialog.info1", "Il seguente file non esiste" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "La seguente cartella non esiste " + newline + "<vuota>"},
        
	{ "converter_gui.lablel0", "Specificare un file o una directory:"},
	{ "converter_gui.lablel1", "Nomi file corrispondenti:"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "Includi sottocartelle"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "File singolo:"},
	{ "converter_gui.lablel5", "Backup dei file nella cartella:"},
	{ "converter_gui.lablel7", "File modello:"},


	{ "template.default", "Standard (IE e Navigator) solo per Windows e Solaris"},
	{ "template.extend",  "Esteso (Standard + tutti i browser/piattaforme)"},
	{ "template.ieonly",  "Internet Explorer solo per Windows e Solaris"},
	{ "template.nsonly",  "Solo Navigator per Windows"},
	{ "template.other",   "Altri modelli..."},

        { "template_dialog.title", "Seleziona file"},
	
        { "help_dialog.caption", "Aiuto"},
        { "help_dialog.error", "Impossibile accedere al file della Guida"},

	{ "menu.file", "File"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "menu.exit", "Esci"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.edit", "Modifica"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_M)},
	{ "menu.option", "Opzioni"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "Aiuto"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "menu.about", "Informazioni su"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_I)},

        { "static.versioning.label", "Versioni Java per gli applet:"},
        { "static.versioning.radio.button", "Utilizza solo JRE {0}"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "Gli applet utilizzano solo questa particolare versione di JRE. Se non \u00e8 gi\u00e0 installata, la versione sar\u00e0 scaricata automaticamente, se possibile. Altrimenti l'utente sar\u00e0 indirizzato a una pagina di download manuale. Per dettagli sulle procedure di download automatico e le politiche di durata di tutti i rilasci Java, visitare http://java.sun.com/products/plugin."},
        { "dynamic.versioning.radio.button", "Utilizza qualsiasi JRE {0} o successiva"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_T)},
        { "dynamic.versioning.text", "Se tale versione non \u00e8 installata, il download predefinito corrente per la famiglia JRE {0} verr\u00e0 autoscaricato, se possibile.  Altrimenti, l''utente verr\u00e0 reindirizzato alla pagina del download manuale."},
        
	{ "progress_event.preparing", "Preparazione"},
	{ "progress_event.converting", "Conversione"},
	{ "progress_event.copying", "Copia"},
	{ "progress_event.done", "Chiudi"},
	{ "progress_event.destdirnotcreated", "Impossibile creare la directory di destinazione"},
	{ "progress_event.error", "Errore"},
	
	{ "plugin_converter.logerror", "Impossibile eseguire l'output del file di log"},
	{ "plugin_converter.saveerror", "Impossibile salvare il file delle propriet\u00e0:  "},
	{ "plugin_converter.appletconv", "Conversione dell'applet "},
	{ "plugin_converter.failure", "Impossibile convertire il file "},
	{ "plugin_converter.overwrite1", "Esiste gi\u00e0 una copia di backup per..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "Sovrascriverla?"},
	{ "plugin_converter.done", "Operazione eseguita. File elaborati:  "},
	{ "plugin_converter.appletfound", "Applet trovati:  "},
	{ "plugin_converter.processing", "Elaborazione in corso..."},
	{ "plugin_converter.cancel", "Conversione annullata"},
	{ "plugin_converter.files", "File da convertire: "},
	{ "plugin_converter.converted", "File gi\u00e0 convertito. Conversione non necessaria. "},
	{ "plugin_converter.donefound", "Operazione eseguita. Applet trovati:  "},
	{ "plugin_converter.seetrace", "Errore nel file - vedere il file traccia riportato di seguito"},
	{ "plugin_converter.noapplet", "Nessun applet nel file "},
	{ "plugin_converter.nofiles", "Nessun file da elaborare "},
	{ "plugin_converter.nobackuppath", "Percorso di backup non creato"},
	{ "plugin_converter.writelog", "Scrittura su un file di log con lo stesso nome"},
	{ "plugin_converter.backup_path", "Percorso di backup"},
	{ "plugin_converter.log_path", "Percorso del file di log"},
	{ "plugin_converter.template_file", "File modello"},
	{ "plugin_converter.process_subdirs", "Elabora sottocartelle"},
	{ "plugin_converter.show_progress", "Mostra avanzamento"},
	{ "plugin_converter.write_permission", "Necessario il permesso di scrittura nella directory di lavoro corrente"},
	{ "plugin_converter.overwrite", "Il file temporaneo .tmpSource_stdin esiste gi\u00e0. Eliminarlo o assegnare un altro nome."},
	{ "plugin_converter.help_message", newline +
                                      "Utilizzo: HtmlConverter [-opzione1 valore1 [-opzione2 valore2 [...]]] [-simulate]  [filespecs]" + newline + newline +
                                      "dove le opzioni includono:" + newline + newline +
                                      "    -source:    percorso da cui leggere i file originari.  Default: <dirutente>" + newline +
                                          "    -source -:  legge il file di conversione dall'input standard" + newline +
                                      "    -dest:      percorso in cui scrivere i file convertiti.  Default: <dirutente>" + newline +
                                          "    -dest -:    scrive il file convertito nell'ouput standard" + newline +
                                      "    -backup:    percorso in cui scrivere i file di backup.  Default: <nomedir>_BAK" + newline +
                                          "    -f:         forza la sovrascrittura dei file di backup." + newline +
                                      "    -subdirs:   elabora i file nelle sottocartelle." + newline +
                                      "    -template:  percorso del file modello.  In caso di dubbio utilizzare il percorso di default." + newline +
                                      "    -log:       percorso in cui scrivere il file di log.  Se non specificato, il file non verr\u00e0 scritto." + newline +
                                      "    -progress:  visualizza l'avanzamento durante la conversione.  Default: false" + newline +
                                      "    -simulate:  visualizza le specifiche della conversione senza effettuarla." + newline +
	                                  "    -latest:    utilizza l'ultimo JRE che supporta il tipo MIME del rilascio." + newline + 
                                      "    -gui:       visualizza l'interfaccia utente grafica per il converter." + newline + newline +
                                      "    filespecs:  elenco delimitato da spazi delle specifiche dei file.  Default: \"*.html *.htm\" (virgolette necessarie)" + newline},
	
	{ "product_name", "Convertitore HTML di Java(TM) Plug-in" },
    };
}

