/*
 * "@(#)ConverterHelp_sv.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * Swedish version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_sv extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Readme-fil om Java(TM) Plug-in HTML Converter" + newline + newline +
      "Version:  " + j2seVersion + newline + newline + newline +
      "*****   S\u00c4KERHETSKOPIERA ALLA FILER INNAN DU KONVERTERAR DEM MED DET H\u00c4R VERKTYGET." + newline +
      "*****   OM DU AVBRYTER KONVERTERINGEN \u00c5TERST\u00c4LLS FILERNA INTE." + newline +
      "*****   KOMMENTARER INUTI APPLET-TAGGAR IGNORERAS." + newline + newline + newline +
      "Inneh\u00e5ll:" + newline +
      "   1.  Nya funktioner" + newline +
      "   2.  Felkorrigeringar" + newline +
      "   3.  Om Java(TM) Plug-in HTML Converter" + newline +
      "   4.  Konverteringsprocessen" + newline +
      "   5.  V\u00e4lja filer och mappar som ska konverteras" + newline +
      "   6.  V\u00e4lja mapp f\u00f6r s\u00e4kerhetskopior" + newline +
      "   7.  Skapa en loggfil" + newline +
      "   8.  V\u00e4lja en konverteringsmall" + newline +
      "   9.  Konvertering" + newline +
      "  10.  Konvertera mera eller avsluta" + newline +
      "  11.  Information om mallar" + newline +
      "  12.  K\u00f6ra HTML Converter (Windows & Solaris)" + newline + newline +
      "1)  Nya funktioner:" + newline + newline +
      "    o Mallarna har uppdaterats och ut\u00f6kats med st\u00f6d f\u00f6r Netscape 6." + newline +
      "    o Alla mallar har uppdaterats med st\u00f6d f\u00f6r de nya flerversionsfunktionerna i Java Plug-in." + newline +
      "    o Anv\u00e4ndargr\u00e4nssnittet har f\u00f6rb\u00e4ttrats med Swing 1.1 f\u00f6r i18n-st\u00f6d." + newline +
      "    o Dialogrutan Avancerade alternativ har f\u00f6rb\u00e4ttrats och f\u00e5tt st\u00f6d f\u00f6r nya SmartUpdate- och" + newline +
      "      MimeType-malltaggar." + newline +
      "    o HTML Converter har f\u00f6rb\u00e4ttrats f\u00f6r anv\u00e4ndning med b\u00e5de Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x, Java Plug-in 1.4.x" + newline +
      "      och Java Plug-in 1.5.x." + newline +
      "    o SmartUpdate- och MimeType-st\u00f6det har f\u00f6rb\u00e4ttrats i alla konverterings-" + newline +
      "      mallar." + newline +
      "    o \"scriptable=false\" har lagts till i taggen OBJECT/EMBED i alla mallar." + newline + newline +
      "     Detta anv\u00e4nds f\u00f6r att inaktivera typelib-generering n\u00e4r Java" + newline +
      "    Plug-in inte anv\u00e4nds f\u00f6r skript." + newline + newline + newline +
      "2)  Fel som har korrigerats:" + newline + newline +
      "    o Felhanteringen d\u00e5 egenskapsfiler inte hittas har f\u00f6rb\u00e4ttrats." + newline +
      "    o HTML-konverteringen har f\u00f6rb\u00e4ttrats s\u00e5 att den resulterande EMBED/OBJECT-taggen kan" + newline +
      "      anv\u00e4ndas i AppletViewer i JDK 1.2.x." + newline +
      "    o On\u00f6diga filer, som l\u00e4mnas kvar efter HTML Converter 1.1.x, har tagits bort." + newline +
      "    o Genererade EMBED/OBJECT med bl.a. CODE- och CODEBASE-attributnamn" + newline +
      "      i st\u00e4llet f\u00f6r JAVA_CODE, JAVA_CODEBASE osv. P\u00e5 s\u00e5 s\u00e4tt kan" + newline +
      "      den genererade sidan anv\u00e4ndas i JDK 1.2.x AppletViewer." + newline +
      "    o St\u00f6d f\u00f6r MAYSCRIPT-konvertering om detta finns i" + newline +
      "      APPLET-taggen." + newline + newline +
      "3)  Om Java(TM) Plug-in HTML Converter:" + newline + newline +
      "        Java(TM) Plug-in HTML Converter \u00e4r ett verktyg som du anv\u00e4nder f\u00f6r att konvertera" + newline +
      "        HTML-sidor som inneh\u00e5ller appletprogram till ett format som anv\u00e4nder Java(TM)" + newline +
      "        Plug-in." + newline + newline +
      "4) Konverteringsprocessen:" + newline + newline +
      "        Java(TM) Plug-in HTML Converter konverterar alla filer som inneh\u00e5ller" + newline +
      "        appletprogram till ett format som kan anv\u00e4ndas med Java(TM) Plug-in." + newline + newline +
      "        Konverteringsprocessen f\u00f6r varje fil g\u00e5r till s\u00e5 h\u00e4r:" + newline +
      "        F\u00f6rst \u00f6verf\u00f6rs den HTML-kod som inte ing\u00e5r i n\u00e5gon applet fr\u00e5n k\u00e4llan" + newline +
      "        till en tempor\u00e4r fil.  N\u00e4r programmet kommer till en <APPLET-tagg" + newline +
      "        tolkar konverteraren appleten till den f\u00f6rsta </APPLET-taggen (som inte st\u00e5r inom citattecken)" + newline +
      "        och sl\u00e5r ihop applet-data med mallen. (Se avsnittet Information om mallar" + newline +
      "        nedan). Om detta slutf\u00f6rs utan fel flyttas den ursprungliga html-filen" + newline +
      "        till mappen f\u00f6r s\u00e4kerhetskopior och den tempor\u00e4ra filens namn \u00e4ndras" + newline +
      "        till den ursprungliga filens namn.  De ursprungliga filerna tas allts\u00e5 aldrig bort fr\u00e5n disken." + newline + newline +
      "        Konverteraren konverterar filerna p\u00e5 plats.  D\u00e4rf\u00f6r" + newline +
      "        \u00e4r filerna klara f\u00f6r anv\u00e4ndning med Java(TM) Plug-in s\u00e5 snart du har k\u00f6rt konverteraren." + newline +


      "5)  V\u00e4lja filer och mappar som ska konverteras:" + newline + newline +
      "       Om du vill konvertera alla filer i en mapp skriver du in s\u00f6kv\u00e4gen till mappen" + newline +
      "       eller anv\u00e4nder bl\u00e4ddringsknappen f\u00f6r att v\u00e4lja en mapp i en dialogruta." + newline  +
      "       N\u00e4r du har angett s\u00f6kv\u00e4gen kan du ange valfritt antal filspecificerare i" + newline +
      "       \"\u00d6verensst\u00e4mmande filnamn\".  Varje specificerare m\u00e5ste s\u00e4rskiljas med ett komma.  Du kan anv\u00e4nda * som" + newline +
      "       jokertecken.  Om du anger ett filnamn med jokertecken konverteras bara just den" + newline +
      "       filen. Markera slutligen kryssrutan \"Ta med underliggande mappar\" om du vill" + newline +
      "       att alla filer, som matchar filnamnet, i undermapparna ocks\u00e5 ska konverteras." + newline + newline +
      "6)  V\u00e4lja mapp f\u00f6r s\u00e4kerhetskopior:" + newline +

      "       Standards\u00f6kv\u00e4gen till mappen f\u00f6r s\u00e4kerhetskopior \u00e4r k\u00e4lls\u00f6kv\u00e4gen med till\u00e4gget \"_BAK\" " + newline +
      "       i namnet.  Om k\u00e4lls\u00f6kv\u00e4gen till exempel \u00e4r c:/html/applet.html (konvertering av en fil)" + newline +
      "       blir s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopiorna c:/html_BAK.  Om k\u00e4lls\u00f6kv\u00e4gen \u00e4r" + newline +
      "       c:/html (konvertering av alla filer i s\u00f6kv\u00e4gen) blir s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopiorna" + newline +
      "       c:/html_BAK. Du kan \u00e4ndra s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopiorna genom att skriva in en annan s\u00f6kv\u00e4g i f\u00e4ltet" + newline +
      "       bredvid \"S\u00e4kerhetskopiera filer till mapp:\" eller genom att bl\u00e4ddra dig fram till \u00f6nskad mapp." + newline + newline +

      "       Unix (Solaris):" + newline +
      "       Standards\u00f6kv\u00e4gen till mappen f\u00f6r s\u00e4kerhetskopior \u00e4r k\u00e4lls\u00f6kv\u00e4gen med till\u00e4gget \"_BAK\" " + newline +
      "       i namnet.  Om k\u00e4lls\u00f6kv\u00e4gen till exempel \u00e4r /home/user1/html/applet.html (konvertering av en fil)" + newline +
      "       blir s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopiorna /home/user1/html_BAK. Om k\u00e4lls\u00f6kv\u00e4gen" + newline +
      "       \u00e4r /home/user1/html (konvertering av alla filer i s\u00f6kv\u00e4gen) blir s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopiorna" + newline +
      "       /home/user1/html_BAK. Du kan \u00e4ndra s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopiorna genom att skriva" + newline +
      "       in en annan s\u00f6kv\u00e4g i f\u00e4ltet bredvid \"S\u00e4kerhetskopiera filer till mapp:\" eller genom att bl\u00e4ddra dig fram till \u00f6nskad mapp." + newline + newline +
      "7)  Skapa en loggfil:" + newline + newline +
      "       Om du vill skapa en loggfil markerar du kryssrutan" + newline +
      "       \"Generera loggfil\". Du kan ange s\u00f6kv\u00e4g och filnamn eller bl\u00e4ddra" + newline +
      "       dig fram till \u00f6nskad mapp och sedan skriva filnamnet och \u00f6ppna filen." + newline +
      "       Loggfilen inneh\u00e5ller grundl\u00e4ggande information om konverterings-" + newline +
      "       processen." + newline + newline +
      "8)  V\u00e4lja en konverteringsmall:" + newline + newline +
      "       Om du inte anger n\u00e5gon mall anv\u00e4nds standardmallen.  Den h\u00e4r mallen" + newline +
      "       producerar konverterade html-filer som fungerar med IE och Netscape." + newline  +
      "       Om du vill anv\u00e4nda en annan mall v\u00e4ljer du \u00f6nskad mall p\u00e5 menyn p\u00e5" + newline +
      "       huvudsk\u00e4rmen.  Om du v\u00e4ljer en annan mall kan du ocks\u00e5 v\u00e4lja en fil" + newline +
      "       som ska anv\u00e4ndas som mall." + newline +
      "       Om du v\u00e4ljer en fil M\u00c5STE DU SE TILL ATT DET \u00c4R EN MALL." + newline + newline +
      "9)  Konvertering:" + newline + newline +
      "       Klicka p\u00e5 knappen \"Konvertera...\" n\u00e4r du vill p\u00e5b\u00f6rja konverteringsprocessen.  En dialogruta" + newline +
      "       f\u00f6r processen visar de filer som bearbetas, antalet filer i processen," + newline +
      "       antalet appletprogram som hittas samt antalet fel som p\u00e5tr\u00e4ffas." + newline + newline +
      "10) Konvertera mera eller avsluta:" + newline + newline +
      "       N\u00e4r konverteringen \u00e4r klar \u00e4ndras knappen i dialogrutan f\u00f6r processen" + newline +
      "       fr\u00e5n \"Avbryt\" till \"Klar\".  Du kan v\u00e4lja \"Klar\" om du vill st\u00e4nga dialogrutan." + newline  +
      "       Sedan v\u00e4ljer du \"Avsluta\" f\u00f6r att st\u00e4nga Java(TM) Plug-in HTML Converter" + newline +
      "       eller s\u00e5 anger du en annan upps\u00e4ttning filer som ska konverteras och v\u00e4ljer sedan \"Konvertera...\" igen." + newline + newline +
      "11)  Information om mallar:" + newline + newline +
      "       Mallfilen \u00e4r grunden i konverteringen av appletprogram.  Det \u00e4r en textfil" + newline +
      "       med taggar, som representerar delar av den ursprungliga appleten." + newline +
      "       Genom att l\u00e4gga till/ta bort/flytta taggarna i en mallfil kan du \u00e4ndra utdata" + newline +
      "       f\u00f6r den konverterade filen." + newline + newline +
      "       Taggar som st\u00f6ds:" + newline + newline +
      "        $OriginalApplet$    Den h\u00e4r taggen ers\u00e4tts med den fullst\u00e4ndiga texten" + newline +
      "        i den ursprungliga appleten." + newline + newline +
      "        $AppletAttributes$   Den h\u00e4r taggen ers\u00e4tts med alla" + newline +
      "        applet-attribut (kod, kodbas, bredd, h\u00f6jd etc)." + newline + newline +
      "        $ObjectAttributes$   Den h\u00e4r taggen ers\u00e4tts med alla" + newline +
      "        attribut som objekttaggen kr\u00e4ver." + newline + newline +
      "        $EmbedAttributes$   Den h\u00e4r taggen ers\u00e4tts med alla attribut" + newline +
      "        som den inb\u00e4ddade taggen kr\u00e4ver." + newline + newline +
      "        $AppletParams$   Den h\u00e4r taggen ers\u00e4tts med alla appletens" + newline +
      "        <param ...>-taggar." + newline + newline +
      "        $ObjectParams$   Den h\u00e4r taggen ers\u00e4tts med alla <param...>-" + newline +
      "        taggar som objekttaggen kr\u00e4ver." + newline + newline +
      "        $EmbedParams$   Den h\u00e4r taggen ers\u00e4tts med alla <param...>-" + newline +
      "        taggar som den inb\u00e4ddade taggen kr\u00e4ver med formatet NAMN=V\u00c4RDE" + newline + newline +
      "        $AlternateHTML$  Den h\u00e4r taggen ers\u00e4tts med texten i omr\u00e5det Inget" + newline +
      "        st\u00f6d f\u00f6r appletprogram i den ursprungliga appleten." + newline + newline +
      "        $CabFileLocation$   Det h\u00e4r \u00e4r URL:en f\u00f6r den cab-fil som ska" + newline +
      "        anv\u00e4ndas i alla mallar som \u00e4r avsedda f\u00f6r IE." + newline + newline +
      "        $NSFileLocation$   Det h\u00e4r \u00e4r URL:en f\u00f6r den Netscape-insticksmodul som ska" + newline +
      "        anv\u00e4ndas i alla mallar som \u00e4r avsedda f\u00f6r Netscape." + newline + newline +
      "        $SmartUpdate$   Det h\u00e4r \u00e4r URL:en f\u00f6r den Netscape SmartUpdate" + newline +
      "        som ska anv\u00e4ndas i alla mallar som \u00e4r avsedda f\u00f6r Netscape Navigator 4.0 eller senare." + newline + newline +
      "        $MimeType$    Det h\u00e4r \u00e4r MIME-typen f\u00f6r Java-objektet" + newline + newline +
      "      default.tpl (standardmallen f\u00f6r konverteraren). Den konverterade sidan kan" + newline +
      "      anv\u00e4ndas i IE och Navigator f\u00f6r Windows f\u00f6r att anropa Java(TM) Plug-in." + newline +
      "      Den h\u00e4r mallen kan ocks\u00e5 anv\u00e4ndas med Netscape f\u00f6r Unix (Solaris)" + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- Den konverterade sidan kan anv\u00e4ndas f\u00f6r att anropa Java(TM)" + newline +
      "      Plug-in i IE endast f\u00f6r Windows." + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- Den konverterade sidan kan anv\u00e4ndas f\u00f6r att anropa Java(TM)" + newline +
      "      Plug-in i Navigator f\u00f6r Windows och Solaris." + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- Den konverterade sidan kan anv\u00e4ndas i alla webbl\u00e4sare och p\u00e5 alla plattformar." + newline +
      "      Om webbl\u00e4saren \u00e4r IE eller Navigator f\u00f6r Windows (Navigator f\u00f6r Solaris) anropas" + newline +
      "      Java (TM) Plug-in. Annars anv\u00e4nds webbl\u00e4sarens standard-JVM." + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  K\u00f6ra HTML Converter:" + newline + newline +
      "      K\u00f6ra GUI-versionen av HTML Converter" + newline + newline +
      "      HTML Converter finns i JDK, inte i JRE. Om du vill k\u00f6ra konverteraren g\u00e5r du till" + newline +
      "      lib-underkatalogen i den katalog som JDK-installationen finns i. Om du till exempel" + newline +
      "      har installerat JDK f\u00f6r Windows i C:\\jdk" + j2seVersion + " g\u00e5r du till katalogen" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\" + newline + newline +
      "      Konverteraren (htmlconverter.jar) finns i den h\u00e4r katalogen." + newline + newline +
      "      N\u00e4r du vill starta konverteraren skriver du:" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      Du startar konverteraren f\u00f6r UNIX/Linux p\u00e5 samma s\u00e4tt som beskrivs ovan." + newline +
      "      Nedan beskrivs hur du kan starta konverteraren p\u00e5 n\u00e5gra andra s\u00e4tt." + newline + newline +
      "      Windows" + newline +
      "      Starta konverteraren med Utforskaren." + newline +
      "      Anv\u00e4nd Utforskaren f\u00f6r att g\u00e5 till f\u00f6ljande katalog:" + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      Dubbelklicka p\u00e5 programmet HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      K\u00f6r f\u00f6ljande kommandon:" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      K\u00f6ra konverteraren fr\u00e5n kommandoraden:" + newline + newline +
      "      Format:" + newline + newline +
      "      java -jar htmlconverter.jar [-alternativ1 v\u00e4rde1 [-alternativ2 v\u00e4rde2" + newline +
      "      [...]]] [-simulate] [fillista]" + newline + newline +
      "      fillista:  blankteckenavgr\u00e4nsad lista \u00f6ver filspecifikationer, * jokertecken. " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      Alternativ:" + newline + newline +
      "       k\u00e4lla:    S\u00f6kv\u00e4g till filer.  (c:\\htmldocs i Windows," + newline +
      "                  /home/user1/htmldocs i Unix) Standard: <anv\u00e4ndarkatalog>" + newline +
      "                  Om s\u00f6kv\u00e4gen \u00e4r relativ antas den anges i f\u00f6rh\u00e5llande till" + newline +
      "                  den katalog som HTMLConverter startades fr\u00e5n." + newline + newline +
      "       backup:    S\u00f6kv\u00e4g d\u00e4r s\u00e4kerhetskopior ska sparas.  Standard:" + newline +
      "                  <anv\u00e4ndarkatalog>/<k\u00e4lla>_bak" + newline +
      "                  Om s\u00f6kv\u00e4gen \u00e4r relativ antas den anges i f\u00f6rh\u00e5llande till" + newline +
      "                  den katalog som HTMLConverter startades fr\u00e5n." + newline + newline +
      "       subdirs:   Om filer i underkataloger ska bearbetas. " + newline +
      "                  Standard:  FALSE" + newline + newline +
      "       template:  Namn p\u00e5 mallfil.  Standard:  default.tpl-Standard " + newline +
      "                  (IE & Navigator) endast f\u00f6r Windows & Solaris. ANV\u00c4ND STANDARD OM DU \u00c4R OS\u00e4KER." + newline + newline +
      "       log:       S\u00f6kv\u00e4g och filnamn f\u00f6r loggfilen.  (Standard <anv\u00e4ndarkatalog>/convert.log)" + newline + newline +
      "       progress:  Visa standardf\u00f6rlopp under konvertering. " + newline +
      "                  Standard: false" + newline + newline +
      "       simulate:  Visar detaljer om konverteringen utan att konvertera." + newline +
      "                  ANV\u00c4ND DET H\u00c4R ALTERNATIVET OM DU \u00c4R OS\u00c4KER P\u00c5 KONVERTERINGEN." + newline +
      "                  DET VISAS EN LISTA MED UPPGIFTER OM" + newline +
      "                  KONVERTERINGEN." + newline + newline +
      "      Om endast \"java -jar htmlconverter.jar -gui\" anges (endast alternativet -gui" + newline +
      "      utan filspecifikation) startas GUI-versionen av konverteraren." + newline  +
      "      I annat fall startas inte GUI-versionen." + newline + newline +
      "      Du hittar mer information p\u00e5 f\u00f6ljande webbplats:" + newline + newline +
      "      http://java.sun.com/j2se/" +
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}

