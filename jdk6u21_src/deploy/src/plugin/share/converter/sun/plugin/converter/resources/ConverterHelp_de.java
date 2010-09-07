/*
 * @(#)ConverterHelp_de.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * German version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_de extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Java(TM) Plug-in HTML Converter Readme" + newline + newline +
      "Version:  " + j2seVersion + newline + newline + newline +
      "*****   SICHERN SIE ALLE DATEIEN, BEVOR SIE SIE MIT DIESEM TOOL KONVERTIEREN." + newline +
      "*****   DURCH EINEN ABBRUCH DER KONVERTIERUNG WERDEN DIE \u00c4NDERUNGEN NICHT" + newline +
      "*****   R\u00dcCKG\u00c4NGIG GEMACHT." + newline +
      "*****   KOMMENTARE INNERHALB DES APPLET-TAGs WERDEN IGNORIERT." + newline + newline + newline +
      "Inhalt:" + newline +
      "   1.  Neue Leistungsmerkmale" + newline +
      "   2.  Fehlerkorrekturen" + newline +
      "   3.  \u00dcber Java(TM) Plug-in HTML Converter" + newline +
      "   4.  Der Konvertierungsprozess" + newline +
      "   5.  Auswahl zu konvertierender Dateien aus Ordnern" + newline +
      "   6.  Auswahl eines Sicherungsordners" + newline +
      "   7.  Erzeugen einer Protokolldatei" + newline +
      "   8.  Auswahl einer Konvertierungsvorlage" + newline +
      "   9.  Konvertieren" + newline +
      "  10.  Weitere Konvertierungen oder Beenden" + newline +
      "  11.  N\u00e4heres zu Vorlagen" + newline +
      "  12.  Ausf\u00fchren von HTML Converter (Windows und Solaris)" + newline + newline +
      "1)  Neue Leistungsmerkmale:" + newline + newline +
      "    o Erweiterte Vorlagen unterst\u00fctzen jetzt Netscape 6." + newline +
      "    o Alle Vorlagen unterst\u00fctzen jetzt neue Java Plug-in-Leistungsmerkmale f\u00fcr" + newline +
      "      mehrere Versionen (MultiVersion)." + newline +
      "    o Verbesserte Benutzeroberfl\u00e4che mit Swing 1.1 zur Unterst\u00fctzung von" + newline +
      "      Internationalisierungen." + newline +
      "    o Verbessertes Dialogfeld f\u00fcr erweiterte Optionen zur Unterst\u00fctzung neuer" + newline +
      "      SmartUpdate- und MimeType-Vorlagen-Tags." + newline +
      "    o Verbesserter HTML-Konverter f\u00fcr den Einsatz mit Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x, Java Plug-in 1.4.x sowie" + newline +
      "      Java Plugin-in 1.5.x." + newline +
      "    o Verbesserte SmartUpdate- und MimeType-Unterst\u00fctzung in s\u00e4mtlichen" + newline +
      "      Konvertierungsvorlagen." + newline +
      "    o Erg\u00e4nzung des OBJECT/EMBED-Tags in allen Vorlagen um \"scriptable=false\"." + newline +
      "      Dadurch kann die typelib-Generierung deaktiviert werden, wenn Java" + newline +
      "      Plug-in nicht zum Scripting verwendet wird." + newline + newline + newline +
      "2)  Korrigierte Fehler:" + newline + newline +
      "    o Verbesserte Fehlerbehandlung beim Nichtauffinden von Eigenschaftendateien." + newline +
      "    o Verbesserte HTML-Konvertierung: Das resultierende EMBED/OBJECT-Tag kann in" + newline +
      "      JDK 1.2.x AppletViewer verwendet werden." + newline +
      "    o Aus HTML Converter 1.1.x stammende unn\u00f6tige Dateien wurden entfernt." + newline +
      "    o EMBED/OBJECT-Resultat erh\u00e4lt die Attributnamen CODE, CODEBASE etc. anstatt" + newline +
      "      JAVA_CODE, JAVA_CODEBASE etc. Dadurch kann die generierte Seite in" + newline +
      "      JDK 1.2.x AppletViewer verwendet werden." + newline +
      "    o Unterst\u00fctzung f\u00fcr MAYSCRIPT-Konvertierung, sofern im APPLET-Tag angegeben." + newline + newline +
      "3)  \u00dcber Java(TM) Plug-in HTML Converter:" + newline + newline +
      "        Java(TM) Plug-in HTML Converter ist ein Dienstprogramm zum Konvertieren" + newline +
      "        beliebiger HTML-Seiten, die Applets enthalten, in ein mit Java(TM) Plug-in" + newline +
      "        verwendbares Format." + newline + newline +
      "4)  Der Konvertierungsprozess:" + newline + newline +
      "        Java(TM) Plug-in HTML Converter konvertiert beliebige Dateien, die Applets" + newline +
      "        enthalten, in eine mit Java(TM) Plug-in verwendbare Form." + newline + newline +
      "        Bei der Konvertierung einer Datei geschieht Folgendes:" + newline +
      "        Zun\u00e4chst wird HTML-Code, der nicht Bestandteil eines Applets ist, aus der Quell- in" + newline +
      "        eine tempor\u00e4re Datei \u00fcbertragen. Beim Erreichen eines <APPLET-Tags analysiert der" + newline +
      "        Konverter das Applet bis zum ersten </APPLET-Tag (das nicht in Anf\u00fchrungszeichen" + newline +
      "        steht) und f\u00fchrt die Appletdaten mit der Vorlage zusammen (siehe N\u00e4heres zu Vorlagen)." + newline +
      "        L\u00e4uft dies ohne Fehler ab, wird die HTML-Originaldatei in den Sicherungsordner" + newline +
      "        verschoben, und die tempor\u00e4re Datei erh\u00e4lt den Namen der Originaldatei. Die" + newline +
      "        Originaldateien werden also nicht von der Festplatte gel\u00f6scht." + newline + newline +
      "        Beachten Sie bitte, dass der Konverter die Dateien effektiv an Ort und Stelle" + newline +
      "        konvertiert. Das hei\u00dft also, dass die Dateien nach der Ausf\u00fchrung des Konverters" + newline +
      "        f\u00fcr die Verwendung mit Java(TM) Plug-in bereit sind." + newline + newline +
      "5)  Auswahl zu konvertierender Dateien aus Ordnern:" + newline + newline +
      "       Wenn Sie alle Dateien in einem Ordner konvertieren m\u00f6chten, k\u00f6nnen Sie entweder den" + newline +
      "       Ordnerpfad eingeben oder auf \"Durchsuchen\" klicken und den Ordner \u00fcber" + newline +
      "       das dadurch aufgerufene Dialogfeld ausw\u00e4hlen." + newline +
      "       Nach der Angabe des Ordnerpfads geben Sie beliebig viele Dateiangaben in das" + newline +
      "       Feld \"\u00dcbereinstimmende Dateinamen\" ein. Trennen Sie die Angaben durch Kommata." + newline +
      "       Der Platzhalter * ist zul\u00e4ssig. Wenn Sie einen Dateinamen mit Platzhalter angeben," + newline +
      "       wird nur diese einzelne Datei konvertiert. Aktivieren Sie das Markierfeld" + newline +
      "       \"Untergeordnete Ordner aufnehmen\", wenn alle mit dem Dateinamen \u00fcbereinstimmenden Dateien aus" + newline +
      "       untergeordneten Ordnern konvertiert werden sollen." + newline + newline +
      "6)  Auswahl eines Sicherungsordners:" + newline +
      "       Der vorgegebene Sicherungspfad ist der Quellpfad, erg\u00e4nzt durch den" + newline +
      "       Anhang \"_BAK\". Wenn beispielsweise der Quellpfad c:/html/applet.html" + newline +
      "       lautet (Konvertierung nur einer Datei), hei\u00dft der vorgegebene Sicherungspfad" + newline +
      "       c:/html_BAK. Bei dem Quellpfad c:/html (Konvertierung aller Dateien im Pfad)" + newline +
      "       hei\u00dft der Sicherungspfad c:/html_BAK. Sie k\u00f6nnen den Sicherungspfad \u00e4ndern," + newline +
      "       indem Sie einen neuen Pfad in das Feld neben \"Sicherungsdateien in Ordner:\"" + newline +
      "       eingeben oder den gew\u00fcnschten Ordner im Auswahldialogfeld ausw\u00e4hlen." + newline + newline +
      "       Unix (Solaris):" + newline +
      "       Der vorgegebene Sicherungspfad ist der Quellpfad, erg\u00e4nzt durch den" + newline +
      "       Anhang \"_BAK\". Wenn beispielsweise der Quellpfad /home/user1/html/applet.html" + newline +
      "       lautet (Konvertierung nur einer Datei), hei\u00dft der vorgegebene Sicherungspfad" + newline +
      "       /home/user1/html_BAK. Bei dem Quellpfad /home/user1/html (Konvertierung aller" + newline +
      "       Dateien im Pfad) hei\u00dft der Sicherungspfad /home/user1/html_BAK." + newline +
      "       Sie k\u00f6nnen den Sicherungspfad \u00e4ndern, indem Sie einen neuen Pfad in das Feld" + newline +
      "       neben \"Sicherungsdateien in Ordner:\" eingeben oder den gew\u00fcnschten Ordner im" + newline +
      "       Auswahldialogfeld ausw\u00e4hlen." + newline + newline +
      "7)  Erzeugen einer Protokolldatei:" + newline + newline +
      "       Wenn Sie m\u00f6chten, dass eine Protokolldatei generiert wird, aktivieren Sie das" + newline +
      "       Markierfeld \"Protokolldatei wird erzeugt\". Sie k\u00f6nnen Pfad und Dateinamen entweder eingeben" + newline +
      "       oder einen Ordner ausw\u00e4hlen, den Dateinamen eingeben und dann \"\u00d6ffnen\" w\u00e4hlen." + newline +
      "       Die Protokolldatei enth\u00e4lt Grundinformationen \u00fcber den Konvertierungsprozess." + newline + newline +
      "8)  Auswahl einer Konvertierungsvorlage:" + newline + newline +
      "       Wenn Sie keine Vorlage ausw\u00e4hlen, wird eine Standardvorlage verwendet. Die damit" + newline +
      "       konvertierten HTML-Dateien sind f\u00fcr IE und Netscape geeignet." + newline +
      "       Andere Vorlagen k\u00f6nnen Sie \u00fcber das Men\u00fc im Hauptbildschirm ausw\u00e4hlen. Wenn Sie" + newline +
      "       eine Datei angeben, die als Vorlage verwendet werden soll, VERGEWISSERN SIE SICH," + newline +
      "       DASS ES SICH UM EINE VORLAGE HANDELT." + newline + newline +
      "9)  Konvertierung:" + newline + newline +
      "       Klicken Sie auf die Schaltfl\u00e4che \"Konvertieren...\", um die Konvertierung zu starten." + newline +
      "       In einem Fortschrittsdialogfeld werden die in Bearbeitung befindliche Datei, die" + newline +
      "       Anzahl an Dateien pro Prozess, die Anzahl gefundener Applets sowie die Anzahl" + newline +
      "       gefundener Fehler angezeigt." + newline + newline +
      "10) Weitere Konvertierungen oder Beenden:" + newline + newline +
      "       Nach Abschluss der Konvertierung wechselt die Beschriftung der Schaltfl\u00e4che im" + newline +
      "       Fortschrittsdialogfeld von \"Abbrechen\" zu \"Fertig\". W\u00e4hlen Sie \"Fertig\", wenn Sie" + newline +
      "       fertig sind und das Dialogfeld schlie\u00dfen m\u00f6chten. Dann w\u00e4hlen Sie \"Beenden\", um" + newline +
      "       Java(TM) Plug-in HTML Converter zu beenden. Anderenfalls w\u00e4hlen Sie einen weiteren" + newline +
      "       Satz zu konvertierender Dateien aus und w\u00e4hlen erneut \"Konvertieren...\"." + newline + newline +
      "11)  N\u00e4heres zu Vorlagen:" + newline + newline +
      "       Die Vorlagendatei ist die Grundlage f\u00fcr die Konvertierung von Applets. Sie ist" + newline +
      "       eine einfache Textdatei mit Tags, die Teile des Original-Applets darstellen." + newline +
      "       Durch Hinzuf\u00fcgen, Entfernen oder Verschieben von Tags in einer Vorlagendatei" + newline +
      "       l\u00e4sst sich die Ausgabe der konvertierten Datei beeinflussen." + newline + newline +
      "       Unterst\u00fctzte Tags:" + newline + newline +
      "        $OriginalApplet$    Dieses Tag wird durch den vollst\u00e4ndigen Text des" + newline +
      "        Original-Applets ersetzt." + newline + newline +
      "        $AppletAttributes$   Dieses Tag wird durch alle Attribute des Applets" + newline +
      "        ersetzt (Code, Codebase, Breite, H\u00f6he etc.)." + newline + newline +
      "        $ObjectAttributes$   Dieses Tag wird durch alle f\u00fcr das Object-Tag" + newline +
      "        erforderlichen Attribute ersetzt." + newline + newline +
      "        $EmbedAttributes$   Dieses Tag wird durch alle f\u00fcr das Embed-Tag" + newline +
      "        erforderlichen Attribute ersetzt." + newline + newline +
      "        $AppletParams$    Dieses Tag wird durch alle <param ...>-Tags des Applets" + newline +
      "        ersetzt." + newline + newline +
      "        $ObjectParams$    Dieses Tag wird durch alle f\u00fcr das Object-Tag" + newline +
      "        erforderlichen <param...>-Tags ersetzt." + newline + newline +
      "        $EmbedParams$     Dieses Tag wird durch alle f\u00fcr das Embed-Tag" + newline +
      "        erforderlichen <param...>-Tags in der Form NAME=WERT ersetzt." + newline + newline +
      "        $AlternateHTML$  Dieses Tag wird durch den Text im Bereich \"No" + newline +
      "        support for applets\" des Original-Applets ersetzt." + newline + newline +
      "        $CabFileLocation$   Dies ist die URL der cab-Datei, die in jeder auf" + newline +
      "        IE ausgerichteten Vorlage enthalten sein sollte." + newline + newline +
      "        $NSFileLocation$    Dies ist die URL des Netscape-Plugins, die in jeder" + newline +
      "        auf Netscape ausgerichteten Vorlage enthalten sein sollte." + newline + newline +
      "        $SmartUpdate$   Dies ist die URL f\u00fcr Netscape SmartUpdate, die in jeder" + newline +
      "        auf Netscape Navigator 4.0 oder h\u00f6her ausgerichteten Vorlage enthalten sein" + newline +
      "        sollte." + newline + newline +
      "        $MimeType$    Dies ist der MIME-Typ des Java-Objekts." + newline + newline +
      "      default.tpl (die Standardvorlage des Konverters) -- Mit der konvertierten Seite kann" + newline +
      "      Java(TM) Plug-in in IE und Navigator unter Windows aufgerufen werden. Die Vorlage" + newline +
      "      ist auch f\u00fcr Netscape unter Unix (Solaris) geeignet." + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- Mit der konvertierten Seite kann Java(TM) Plug-in nur in IE unter" + newline +
      "      Windows aufgerufen werden." + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- Mit der konvertierten Seite kann Java(TM) Plug-in in Navigator" + newline +
      "      unter Windows und Solaris aufgerufen werden." + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- Die konvertierte Seite kann f\u00fcr jeden Browser und jede Plattform" + newline +
      "      verwendet werden. Im Fall von IE oder Navigator unter Windows (Navigator unter Solaris)" + newline +
      "      wird Java(TM) Plug-in aufgerufen. Anderenfalls wird die Standard-JVM des Browsers" + newline +
      "      angesprochen." + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  Ausf\u00fchren von HTML Converter:" + newline + newline +
      "      Ausf\u00fchren der GUI-Version (graphische Benutzeroberfl\u00e4che) von HTML Converter" + newline + newline +
      "      HTML Converter ist nicht in JRE, sondern in JDK enthalten. Zum Ausf\u00fchren des" + newline +
      "      Konverters wechseln Sie in das Unterverzeichnis lib Ihres JDK-" + newline +
      "      Installationsverzeichnisses. Wenn Sie JDK beispielsweise unter Windows in" + newline +
      "      C:\\jdk" + j2seVersion + " installiert haben, wechseln Sie in das Verzeichnis:" + newline + newline +
      "            C:\\jdk" + j2seVersion  + "\\lib\\" + newline + newline +
      "      Der Konverter (htmlconverter.jar) befindet sich in diesem Verzeichnis." + newline + newline +
      "      Zum Starten des Konverters geben Sie folgenden Befehl ein:" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      Zum Starten des Konverters unter UNIX/Linux gehen Sie mit dem gleichen Befehl" + newline +
      "      analog vor." + newline +
      "      Der Konverter kann auch wie folgt gestartet werden:" + newline + newline +
      "      Unter Windows" + newline +
      "      Starten des Konverters \u00fcber Windows Explorer." + newline +
      "      Suchen Sie im Windows Explorer das folgende Verzeichnis:" + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      Doppelklicken Sie auf die Anwendung HtmlConverter." + newline + newline +
      "      Unter Unix/Linux" + newline +
      "      F\u00fchren Sie die folgenden Befehle aus:" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      Ausf\u00fchrung des Konverters \u00fcber die Befehlszeile:" + newline + newline +
      "      Format:" + newline + newline +
      "      java -jar htmlconverter.jar [-Option1 Wert1 [-Option2 Wert2" + newline +
      "      [...]]] [-simulate] [Dateiangaben]" + newline + newline +
      "      Dateiangaben: Eine Liste durch Leerzeichen getrennter Dateiangaben und" + newline +
      "      des Platzhalters *. (*.html *.htm)" + newline + newline +
      "      Optionen:" + newline + newline +
      "       source:    Dateipfade. (c:\\htmldocs unter Windows," + newline +
      "                  /home/user1/htmldocs unter Unix) Vorgabe: <Benutzerverzeichnis>." + newline +
      "                  Relative Pfade werden als relativ zu dem Verzeichnis interpretiert," + newline +
      "                  aus dem HTMLConverter gestartet wurde." + newline + newline +
      "       backup:    Pfad f\u00fcr Sicherungsdateien. Vorgabe:" + newline +
      "                  <Benutzerverzeichnis>/<Quelle>_bak." + newline +
      "                  Relative Pfade werden als relativ zu dem Verzeichnis interpretiert," + newline +
      "                  aus dem HTMLConverter gestartet wurde." + newline + newline +
      "       subdirs:   Bestimmt, ob Dateien aus Unterverzeichnissen verarbeitet werden." + newline +
      "                  Vorgabe:  FALSE" + newline + newline +
      "       template:  Name der Vorlagendatei. Vorgabe:  default.tpl-Standard" + newline +
      "                  (IE und Navigator) nur f\u00fcr Windows und Solaris. VERWENDEN SIE IM" + newline +
      "                  ZWEIFELSFALL DIE VORGABE." + newline + newline +
      "       log:       Pfad und Dateiname f\u00fcr die Generierung einer Protokolldatei." + newline +
      "                  (Vorgabe: <Benutzerverzeichnis>/convert.log)" + newline + newline +
      "       progress:  Standardfortschrittsanzeige w\u00e4hrend der Konvertierung." + newline +
      "                  Vorgabe: false" + newline + newline +
      "       simulate:  Anzeige vorgangsspezifischer Informationen ohne tats\u00e4chliche" + newline +
      "                  Konvertierung." + newline +
      "                  VERWENDEN SIE DIESE OPTION BEI ZWEIFELN BEZ\u00dcGLICH DER KONVERTIERUNG." + newline +
      "                  SIE ERHALTEN EINE LISTE VON INFORMATIONEN, DIE SICH SPEZIELL AUF DIE" + newline +
      "                  KONVERTIERUNG BEZIEHEN." + newline + newline +
      "      Wenn nur \"java -jar htmlconverter.jar -gui\" angegeben ist (nur die Option -gui" + newline +
      "      ohne Dateiangaben), wird die GUI-Version des Konverters gestartet." + newline +
      "      Anderenfalls wird die graphische Benutzeroberfl\u00e4che unterdr\u00fcckt." + newline + newline +
      "      Weitere Informationen stehen Ihnen unter folgender URL zur Verf\u00fcgung:" + newline + newline +
      "      http://java.sun.com/j2se/" +
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}
