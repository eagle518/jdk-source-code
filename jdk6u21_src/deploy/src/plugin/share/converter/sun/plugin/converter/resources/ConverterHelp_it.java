/*
 * @(#)ConverterHelp_it.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * Italian version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_it extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Informazioni su Java(TM) Plug-in HTML Converter" + newline + newline +
      "Version: " + j2seVersion + newline + newline + newline +
      "*****   ESEGUIRE UNA COPIA DI BACKUP DEI FILE PRIMA DI PROCEDERE" + newline +
      "*****   ALLA CONVERSIONE CON QUESTO STRUMENTO. L'ANNULLAMENTO DELLA" + newline +
      "*****   CONVERSIONE NON ELIMINER\u00c0 LE MODIFICHE APPORTATE." + newline +
      "*****   I COMMENTI ALL'INTERNO DEL TAG APPLET VERRANNO IGNORATI." + newline + newline + newline +
      "Indice:" + newline +
      "   1.  Nuove funzioni" + newline +
      "   2.  Correzioni ai problemi" + newline +
      "   3.  Informazioni su Java(TM) Plug-in HTML Converter" + newline +
      "   4.  Il processo di conversione" + newline +
      "   5.  Scelta dei file da convertire all'interno delle cartelle" + newline +
      "   6.  Scelta di una cartella di backup" + newline +
      "   7.  Generazione di un file di log" + newline +
      "   8.  Scelta di un modello di conversione" + newline +
      "   9.  Conversione" + newline +
      "  10.  Ulteriori conversioni o uscita" + newline +
      "  11.  Informazioni sui modelli" + newline +
      "  12.  Esecuzione di HTML Converter (Windows e Solaris)" + newline + newline +
      "1)  Nuove funzioni:" + newline + newline +
      "    o Modelli estesi e aggiornati per il supporto di Netscape 6." + newline +
      "    o Modelli aggiornati per il supporto delle nuove funzioni di multi-versioning in Java Plug-in." + newline +
      "    o Interfaccia utente aggiornata con Swing 1.1 per il supporto dell'internazionalizzazione." + newline +
      "    o Finestra di dialogo con le opzioni avanzate per il supporto dei nuovi tag dei modelli" + newline +
      "      SmartUpdate e MimeType." + newline +
      "    o Nuove funzionalit\u00e0 di HTML Converter per l'uso con Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x, Java Plug-in 1.4.x" + newline +
      "      e Java Plugin-in 1.5.x." + newline +
      "    o Supporto migliorato per SmartUpdate e MimeType in tutti i modelli di conversione." + newline +
      "    o Aggiunta di \"scriptable=false\" al tag OBJECT/EMBED in tutti i modelli." + newline + newline +
      "     Questa istruzione viene usata per disabilitare la generazione di typelib quando Java" + newline +
      "    Plug-in non viene usato per lo scripting." + newline + newline + newline +
      "2)  Problemi corretti:" + newline + newline +
      "    o Gestione degli errori migliorata quando i file di propriet\u00e0 non vengono trovati." + newline +
      "    o Conversione HTML modificata in modo che il tag EMBED/OBJECT risultante possa" + newline +
      "      essere usato nell'AppletViewer del JDK 1.2.x." + newline +
      "    o Eliminazione dei file non necessari che non venivano cancellati lasciati da" + newline +
      "      HTML Converter 1.1.x." + newline +
      "    o Generazione di EMBED/OBJECT con attributi denominati CODE, CODEBASE, ecc." + newline +
      "      anzich\u00e9 JAVA_CODE, JAVA_CODEBASE, ecc. Questa modifica consente di utilizzare" + newline +
      "      le pagine generate nell'AppletViewer del JDK 1.2.x." + newline +
      "    o Supporto della conversione MAYSCRIPT se viene presentata nel tag APPLET." + newline + newline +
      "3)  Informazioni su Java(TM) Plug-in HTML Converter:" + newline + newline +
      "        Java(TM) Plug-in HTML Converter \u00e8 un'utility che permette di convertire" + newline +
      "        qualunque pagina HTML contenente un'applet in un formato compatibile con" + newline +
      "        Java(TM) Plug-in." + newline + newline +
      "4)  Il processo di conversione:" + newline + newline +
      "        Java(TM) Plug-in HTML Converter pu\u00f2 convertire qualunque file contenente" + newline +
      "        un'applet in un formato che possa essere utilizzato con Java(TM) Plug-in." + newline + newline +
      "        Il processo di conversione dei file si svolge come segue:" + newline +
      "        in primo luogo, il codice HTML che non fa parte di un'applet viene trasferito" + newline +
      "        dal file sorgente in un file temporaneo. Al raggiungimento di un tag <APPLET," + newline +
      "        il convertitore analizza l'applet fino al primo tag </APPLET (non racchiuso" + newline +
      "        tra virgolette) e unisce i dati dell'applet con il modello (per maggiori" + newline +
      "        informazioni sui modelli, vedere pi\u00f9 avanti). Se la procedura termina senza" + newline +
      "        errori, il file html originale viene spostato nella cartella di backup e il" + newline +
      "        file temporaneo viene rinominato con il nome del file originale. In questo" + newline +
      "        modo, i file originali non vengono mai rimossi dal disco." + newline + newline +
      "        Si osservi che il convertitore esegue la conversione direttamente sui file." + newline +
      "        Perci\u00f2, una volta eseguito il convertitore, i file saranno configurati" + newline +
      "        per l'uso di Java(TM) Plug-in." + newline + newline +
      "5)  Scelta dei file da convertire all'interno delle cartelle:" + newline + newline +
      "       Per convertire tutti i file all'interno di una cartella, \u00e8 possibile digitare" + newline +
      "       il percorso della cartella oppure premere il pulsante Sfoglia per selezionare" + newline +
      "       la cartella da una finestra di dialogo. Dopo aver selezionato il percorso," + newline +
      "       \u00e8 possibile inserire un numero qualsiasi di specificatori dei file" + newline +
      "       nel campo \"Nomi file corrispondenti\". Ogni specificatore deve essere separato da" + newline +
      "       una virgola. \u00c8 possibile usare il carattere jolly *. Se si inserisce il" + newline +
      "       nome di un file con un carattere jolly, verr\u00e0 convertito solo quel singolo" + newline +
      "       file. Infine, selezionare la casella di controllo \"Includi sottocartelle\" per" + newline +
      "       convertire tutti i file contenuti nelle cartelle di livello inferiore che" + newline +
      "       corrispondono al nome specificato." + newline + newline +
      "6)  Scelta della cartella di backup:" + newline +
      "       Il percorso predefinito per la cartella di backup \u00e8 uguale al percorso di origine" + newline +
      "       con l'aggiunta del suffisso \"_BAK\". Ad esempio, se il percorso di" + newline +
      "       origine \u00e8 c:/html/applet.html (per la conversione di un singolo file)," + newline +
      "       il percorso di backup sar\u00e0 c:/html_BAK. Se il percorso di origine \u00e8" + newline +
      "       c:/html (per la conversione di tutti i file in quella posizione), il percorso" + newline +
      "       di backup sar\u00e0 c:/html_BAK. Il percorso di backup pu\u00f2 essere modificato" + newline +
      "       digitando un percorso differente nel campo \"Backup dei file nella cartella:\"" + newline +
      "       o usando il pulsante Sfoglia per selezionare la cartella desiderata." + newline + newline +
      "       Unix(Solaris):" + newline +
      "       Il percorso predefinito per la cartella di backup \u00e8 uguale al percorso di origine" + newline +
      "       con l'aggiunta del suffisso \"_BAK\". Ad esempio, se il percorso di" + newline +
      "       origine \u00e8 /home/utente1/html/applet.html (per la conversione di un singolo file)," + newline +
      "       il percorso di backup sar\u00e0 /home/utente1/html_BAK. Se il percorso di origine \u00e8" + newline +
      "       /home/utente1/html (per la conversione di tutti i file in quella posizione), il" + newline +
      "       percorso di backup sar\u00e0 /home/utente1/html_BAK. Il percorso di backup pu\u00f2 essere" + newline +
      "       modificato digitando un percorso differente nel campo \"Backup dei file nella" + newline +
      "       cartella:\" o usando il pulsante Sfoglia per selezionare la cartella desiderata." + newline + newline +
      "7)  Generazione di un file di log:" + newline + newline +
      "       Se si desidera generare un file di log, selezionare la casella" + newline +
      "       \"Genera file di log\". \u00c8 possibile digitare il percorso e il nome del file" + newline +
      "       oppure usare il pulsante Sfoglia per scegliere la cartella desiderata, quindi" + newline +
      "       digitare il nome del file e selezionare Apri." + newline +
      "       Il file di log contiene informazioni di base relative al processo di conversione." + newline + newline +
      "8)  Scelta di un modello di conversione:" + newline + newline +
      "       Se non viene specificato un modello, viene usato quello predefinito. Questo" + newline +
      "       modello produce file html convertiti che possono essere utilizzati con IE e Netscape." + newline +
      "       Per usare un modello differente, \u00e8 possibile sceglierlo dal menu nella schermata" + newline +
      "       principale. Scegliendo la voce Altri modelli, verr\u00e0 offerta la possibilit\u00e0 di scegliere" + newline +
      "       un file da utilizzare come modello." + newline +
      "       Se si sceglie un file, ACCERTARSI CHE SI TRATTI DI UN MODELLO." + newline + newline +
      "9)  Conversione:" + newline + newline +
      "       Fare clic sul pulsante \"Converti...\" per iniziare il processo di conversione." + newline +
      "       Una finestra di dialogo mostrer\u00e0 i file in corso di elaborazione, il numero dei" + newline +
      "       file elaborati, il numero delle applet trovate e il numero di errori rilevato." + newline + newline +
      "10) Ulteriori conversioni o uscita:" + newline + newline +
      "       Al termine della conversione, l'etichetta sul pulsante nella finestra di dialogo" + newline +
      "       indicante le informazioni sul processo passa da \"Annulla\" a \"Chiudi\"." + newline +
      "       Scegliere \"Chiudi\" per chiudere la finestra di dialogo." + newline +
      "       A questo punto, scegliere \"Esci\" per chiudere Java(TM) Plug-in HTML Converter," + newline +
      "       oppure selezionare un altro gruppo di file da convertire e scegliere" + newline +
      "       nuovamente \"Converti...\"." + newline + newline +
      "11)  Informazioni sui modelli:" + newline + newline +
      "       Il file del modello \u00e8 la base per la conversione delle applet. Si tratta" + newline +
      "       di un semplice file di testo contenente tag che rappresentano parti" + newline +
      "       dell'applet originale. Aggiungendo, rimuovendo o spostando i tag in un" + newline +
      "       modello \u00e8 possibile modificare l'output del file convertito." + newline + newline +
      "       Tag supportati:" + newline + newline +
      "        $OriginalApplet$    Questo tag viene sostituito con il testo completo" + newline +
      "        dell'applet originale." + newline + newline +
      "        $AppletAttributes$   Questo tag viene sostituito con tutti gli attributi" + newline +
      "        dell'applet. (code, codebase, width, height, ecc.)" + newline + newline +
      "        $ObjectAttributes$   Questo tag viene sostituito con tutti gli" + newline +
      "        attributi richiesti dal tag object." + newline + newline +
      "        $EmbedAttributes$   Questo tag viene sostituito con tutti gli attributi" + newline +
      "        richiesti dal tag embed." + newline + newline +
      "        $AppletParams$    Questo tag viene sostituito con tutti i tag" + newline +
      "        <param ...> dell'applet" + newline + newline +
      "        $ObjectParams$    Questo tag viene sostituito con tutti i tag <param...>" + newline +
      "        richiesti dal tag object." + newline + newline +
      "        $EmbedParams$     Questo tag viene sostituito con tutti i tag <param...>" + newline +
      "        richiesti dal tag embed nel formato NOME=VALORE" + newline + newline +
      "        $AlternateHTML$  Questo tag viene sostituito con il testo contenuto" + newline +
      "        nell'area No support for applets dell'applet originale" + newline + newline +
      "        $CabFileLocation$   \u00c8 l'URL del file cab che dovrebbe essere" + newline +
      "        utilizzato in tutti i modelli orientati a IE." + newline + newline +
      "        $NSFileLocation$    \u00c8 l'URL del plugin Netscape che dovrebbe" + newline +
      "        essere usato in tutti i modelli orientati a Netscape" + newline + newline +
      "        $SmartUpdate$   \u00c8 l'URL di Netscape SmartUpdate che dovrebbe" + newline +
      "        essere usato in tutti i modelli orientati a Netscape Navigator 4.0 o" + newline +
      "        a una versione successiva." + newline + newline +
      "        $MimeType$    Il tipo MIME dell'oggetto Java" + newline + newline +
      "      default.tpl (modello predefinito per il convertitore) -- la pagina convertita" + newline +
      "      pu\u00f2 essere utilizzata in IE e in Navigator in Windows per richiamare Java(TM)" + newline +
      "      Plug-in. Il modello pu\u00f2 anche essere utilizzato con Netscape in Unix (Solaris)" + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- la pagina convertita pu\u00f2 essere utilizzata per richiamare Java(TM)" + newline +
      "      Plug-in solo in IE in Windows." + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- la pagina convertita pu\u00f2 essere utilizzata per richiamare Java(TM)" + newline +
      "      Plug-in in Navigator in Windows e Solaris." + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- la pagina convertita pu\u00f2 essere utilizzata in qualunque browser e su" + newline +
      "      qualunque piattaforma. Se il browser \u00e8 IE o Navigator in Windows (Navigator su" + newline +
      "      Solaris), verr\u00e0 richiamato Java(TM) Plug-in. Diversamente, verr\u00e0 usata la JVM" + newline +
      "      predefinita del browser." + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  Esecuzione di HTML Converter:" + newline + newline +
      "      Esecuzione della versione con interfaccia grafica di HTML Converter" + newline + newline +
      "      L'utility HTML Converter \u00e8 contenuta nell'JDK, non nel JRE. Per eseguire il" + newline +
      "      convertitore, accedere alla sottodirectory lib della directory di installazione" + newline +
      "      dell'JDK. Ad esempio, se si \u00e8 installato l'JDK su Windows delle C:\\jdk" + j2seVersion +"," + newline +
      "      spostarsi in" + newline + newline +
      "            C:\\jdk" + j2seVersion  + "\\lib\\" + newline + newline +
      "      Il convertitore (htmlconverter.jar) \u00e8 contenuto in questa directory." + newline + newline +
      "      Per avviare il convertitore, digitare:" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      La procedura di avvio del convertitore su UNIX/Linux \u00e8 analoga con l'uso dei" + newline +
      "      comandi sopra indicati." + newline +
      "      Qui di seguito sono descritte alcune procedure alternative per l'avvio del" + newline +
      "      convertitore" + newline + newline +
      "      Windows" + newline +
      "      Per avviare il convertitore usando Esplora risorse." + newline +
      "      Usare Esplora risorse per spostarsi nella directory seguente." + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      Fare doppio clic sull'applicazione HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Eseguire i comandi seguenti" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      Esecuzione del convertitore dalla riga di comando:" + newline + newline +
      "      Formato:" + newline + newline +
      "      java -jar htmlconverter.jar [-opzione1 valore1 [-opzione2 valore2" + newline +
      "      [...]]] [-simulate] [specificatori file]" + newline + newline +
      "      specificatori file: elenco di specificatori di file delimitati da spazi," + newline +
      "      carattere speciale *. (*.html *.htm)" + newline + newline +
      "      Opzioni:" + newline + newline +
      "       source:    Percorso dei file. (c:\\htmldocs in Windows," + newline +
      "                  /home/utente1/htmldocs in Unix) Valore predefinito: <dir_utente>" + newline +
      "                  Se il percorso \u00e8 relativo, viene usata come riferimento la" + newline +
      "                  directory da cui \u00e8 stato avviato HTMLConverter." + newline + newline +
      "       backup:    Percorso per la scrittura dei file di backup. Valore predefinito:" + newline +
      "                  <dir_utente>/<origine>_bak" + newline +
      "                  Se il percorso \u00e8 relativo, viene usata come punto di partenza la" + newline +
      "                  directory da cui \u00e8 stato avviato HTMLConverter." + newline + newline +
      "       subdirs:   Specifica l'elaborazione dei file contenuti nelle sottodirectory." + newline +
      "                  Valore predefinito: FALSE" + newline + newline +
      "       template:  Nome del file del modello. Valore predefinito: default.tpl-Standard" + newline +
      "                  (IE e Navigator) solo per Windows e Solaris. IN CASO DI DUBBIO," + newline +
      "                  USARE IL VALORE PREDEFINITO." + newline + newline +
      "       log:       Percorso e nome del file per la scrittura del log. (Valore" + newline +
      "                  predefinito: <dir_utente>/convert.log)" + newline + newline +
      "       progress:  Mostra l'avanzamento della conversione in forma standard." + newline +
      "                  Valore predefinito: false" + newline + newline +
      "       simulate:  Mostra le specifiche della conversione senza eseguirla." + newline +
      "                  USARE QUESTA OPZIONE IN CASO DI DUBBIO SULLA CONVERSIONE." + newline +
      "                  VERR\u00c0 FORNITO UN ELENCO DI INFORMAZIONI RIGUARDANTI LA" + newline +
      "                  CONVERSIONE." + newline + newline +
      "      Se viene specificato solo \"java -jar htmlconverter.jar -gui\" (cio\u00e8 solo" + newline +
      "      l'opzione -gui senza specificatori di file), viene avviata la versione GUI" + newline +
      "      del convertitore. Diversamente, la GUI non viene avviata." + newline + newline +
      "      Per maggiori informazioni, accedere al seguente url:" + newline + newline +
      "      http://java.sun.com/j2se/" +
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}
