/*
 * @(#)ConverterHelp_fr.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * French version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_fr extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "" + newline +
      "Java(TM) Plug-in HTML Converter - Lisez-moi" + newline + newline +
      "Version :  " + j2seVersion + newline + newline + newline +
      "*****   SAUVEGARDEZ TOUS LES FICHIERS AVANT DE LES CONVERTIR AVEC CET OUTIL." + newline +
      "*****   ANNULER UNE CONVERSION NE RESTAURE PAS LE FICHIER D'ORIGINE." + newline +
      "*****   LES COMMENTAIRES INCLUS DANS LA BALISE APPLET SERONT IGNORES." + newline + newline + newline +
      "Contenu :" + newline +
      "   1.  Nouvelles fonctionnalit\u00e9s" + newline +
      "   2.  Bugs corrig\u00e9s" + newline +
      "   3.  A propos de Java(TM) Plug-in HTML Converter" + newline +
      "   4.  Le processus de conversion" + newline +
      "   5.  Choix des fichiers \u00e0 convertir dans les dossiers" + newline +
      "   6.  Choix d'un dossier de sauvegarde" + newline +
      "   7.  G\u00e9n\u00e9ration d'un fichier journal" + newline +
      "   8.  Choix d'un mod\u00e8le de conversion" + newline +
      "   9.  Conversion" + newline +
      "  10.  Conversions suppl\u00e9mentaires ou Quitter" + newline +
      "  11.  D\u00e9tails sur les mod\u00e8les" + newline +
      "  12.  Ex\u00e9cution du convertisseur HTML (Windows et Solaris)" + newline + newline +
      "1)  Nouvelles fonctionnalit\u00e9s :" + newline + newline +
      "    o Mod\u00e8les \u00e9tendus mis \u00e0 jour pour prendre en charge Netscape 6." + newline +
      "    o Mise \u00e0 jour de tous les mod\u00e8les pour la prise en charge des nouvelles fonctionnalit\u00e9s multi-version de Java Plug-in." + newline +
      "    o Interface utilisateur am\u00e9lior\u00e9e avec Swing 1.1 pour la prise en charge d'i18n." + newline +
      "    o Bo\u00eete de dialogue d'options avanc\u00e9es am\u00e9lior\u00e9e pour prendre en charge les balises des mod\u00e8les SmartUpdate et" + newline +
      "      MimeType." + newline +
      "    o Convertisseur HTML am\u00e9lior\u00e9 \u00e0 utiliser avec Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x, Java Plug-in 1.4.x" + newline +
      "      et Java Plug-in 1.5.x." + newline +
      "    o Prise en charge de SmartUpdate et MimeType am\u00e9lior\u00e9e dans tous les mod\u00e8les" + newline +
      "      de conversion." + newline +
      "    o Ajout de \"scriptable=false\" \u00e0 la balise OBJECT/EMBED dans tous les mod\u00e8les." + newline + newline +
      "     Ceci est utilis\u00e9 pour d\u00e9sactiver la g\u00e9n\u00e9ration de typelib quand Java" + newline +
      "    Plug-in n'est pas utilis\u00e9 pour les scripts." + newline + newline + newline +
      "2)  Bugs corrig\u00e9s :" + newline + newline +
      "    o Gestion d'erreurs am\u00e9lior\u00e9e en cas de fichiers de propri\u00e9t\u00e9s introuvables." + newline +
      "    o Conversion HTML am\u00e9lior\u00e9e de sorte que la balise EMBED/OBJECT obtenue puisse" + newline +
      "      \u00eatre utilis\u00e9e dans l'AppletViewer de JDK 1.2.x." + newline +
      "    o Elimination des fichiers inutiles qui figuraient dans HTML Converter 1.1.x." + newline +
      "    o Les balises EMBED/OBJECT sont g\u00e9n\u00e9r\u00e9es avec les noms d'attributs CODE, CODEBASE," + newline +
      "      etc. et non plus JAVA_CODE, JAVA_CODEBASE, etc., ce qui permet" + newline +
      "      d'utiliser la page g\u00e9n\u00e9r\u00e9e dans l'AppletViewer de JDK 1.2.x." + newline +
      "    o Prise en charge de la conversion MAYSCRIPT si elle est pr\u00e9sente dans" + newline +
      "      la balise APPLET." + newline + newline +
      "3)  A propos de Java(TM) Plug-in HTML Converter :" + newline + newline +
      "        Java(TM) Plug-in HTML Converter est un utilitaire qui vous permet de" + newline +
      "        convertir toute page HTML contenant des applets dans un format" + newline +
      "        appropri\u00e9 \u00e0 Java(TM) Plug-in." + newline + newline +
      "4)  Le processus de conversion :" + newline + newline +
      "        Le convertisseur HTML Java(TM) Plug-in HTML Converter convertit tout" + newline +
      "        fichier contenant des applets dans une forme utilisable avec Java(TM) Plug-in." + newline + newline +
      "        Le processus de conversion d'un fichier est le suivant :" + newline +
      "        Pour commencer, le contenu HTML qui ne fait partie d'aucune applet est" + newline +
      "        transf\u00e9r\u00e9 du fichier source \u00e0 un fichier temporaire. Lorsque qu'une balise" + newline +
      "        <APPLET est rencontr\u00e9e, le convertisseur analyse l'applet jusqu'\u00e0 la" + newline +
      "        premi\u00e8re balise </APPLET (qui n'est pas entre guillemets) et fusionne" + newline +
      "        les donn\u00e9es de l'applet avec le mod\u00e8le (voir D\u00e9tails sur les mod\u00e8les" + newline +
      "        plus loin). Si cela s'effectue sans erreur, le fichier html d'origine" + newline +
      "        est d\u00e9plac\u00e9 dans le dossier de sauvegarde et le fichier temporaire est" + newline +
      "        ensuite renomm\u00e9 avec le nom du fichier d'origine. Vos fichiers originaux" + newline +
      "        ne sont, par cons\u00e9quent, jamais supprim\u00e9s du disque." + newline + newline +
      "        Vous remarquerez que le convertisseur convertit effectivement les fichiers" + newline +
      "        en place. Du coup, une fois le convertisseur ex\u00e9cut\u00e9, vos fichiers sont" + newline +
      "        configur\u00e9s pour utiliser Java(TM) Plug-in." + newline + newline +
      "5)  Choix des fichiers \u00e0 convertir dans un dossier :" + newline + newline +
      "       Pour convertir tous les fichiers d'un dossier, vous pouvez saisir le chemin de ce" + newline +
      "       dossier ou choisir le bouton Parcourir pour s\u00e9lectionner un dossier dans" + newline +
      "       une bo\u00eete de dialogue. Une fois un chemin choisi, vous pouvez saisir un nombre" + newline +
      "       quelconque de sp\u00e9cificateurs de fichier dans \"Noms de fichiers correspondants\"." + newline +
      "       Ces sp\u00e9cificateurs doivent \u00eatre s\u00e9par\u00e9s par une virgule. Vous pouvez utiliser un *" + newline +
      "       en tant que caract\u00e8re joker. Si vous indiquez un nom de fichier avec un joker," + newline +
      "       seul ce fichier sera converti. Enfin, s\u00e9lectionnez la case \u00e0 cocher \"Inclure les" + newline +
      "       sous-dossiers\", si vous voulez que tous les fichiers des dossiers imbriqu\u00e9s qui" + newline +
      "       correspondent au nom de fichier saisi soient convertis." + newline + newline +
      "6)  Choix d'un dossier de sauvegarde :" + newline +
      "       Le chemin du dossier de sauvegarde par d\u00e9faut est le chemin de la source auquel" + newline +
      "       l'extension \"_BAK\" est ajout\u00e9e. Par ex., si le chemin de la source est" + newline +
      "       c:/html/applet.html (un seul fichier est converti), celui de sauvegarde sera" + newline +
      "       c:/html_BAK. Si le chemin de la source est c:/html (tous les fichiers du chemin" + newline +
      "       sont convertis), celui de sauvegarde sera alors c:/html_BAK. Le chemin de" + newline +
      "       sauvegarde peut \u00eatre modifi\u00e9 en tapant un chemin dans le champ en regard de" + newline +
      "       \"Sauvegarder les fichiers dans le dossier :\", ou en parcourant l'arborescence" + newline +
      "       \u00e0 la recherche d'un dossier." + newline + newline +
      "       Unix(Solaris):" + newline +
      "       Le chemin du dossier de sauvegarde par d\u00e9faut est le chemin de la source auquel" + newline +
      "       l'extension \"_BAK\" est ajout\u00e9e. Par ex., si le chemin de la source est" + newline +
      "       /home/user1/html/applet.html (un seul fichier est converti), celui de sauvegarde" + newline +
      "       sera /home/user1/html_BAK. Si le chemin de la source est /home/user1/html" + newline +
      "       (tous les fichiers du chemin sont convertis), celui de sauvegarde sera" + newline +
      "       /home/user1/html_BAK. Le chemin de sauvegarde peut \u00eatre modifi\u00e9 en tapant un" + newline +
      "       chemin dans le champ en regard de \"Sauvegarder les fichiers dans le dossier:\"," + newline +
      "       ou en parcourant l'arborescence \u00e0 la recherche d'un dossier." + newline + newline +
      "7)  G\u00e9n\u00e9ration d'un fichier journal :" + newline + newline +
      "       Si vous voulez qu'un fichier journal soit g\u00e9n\u00e9r\u00e9, s\u00e9lectionnez la case \u00e0 cocher" + newline +
      "       \"G\u00e9n\u00e9rer le fichier journal\". Vous pouvez entrer le chemin et le nom du fichier" + newline +
      "       ou parcourir l'arborescence \u00e0 la recherche d'un dossier, tapez ensuite le nom" + newline +
      "       du fichier et s\u00e9lectionnez Ouvrir. Le fichier journal ainsi obtenu contiendra" + newline +
      "       des informations de base relatives au processus de conversion." + newline + newline +
      "8)  Choix d'un mod\u00e8le de conversion :" + newline + newline +
      "       Un mod\u00e8le par d\u00e9faut sera utilis\u00e9 si vous n'en choisissez aucun. Ce mod\u00e8le" + newline +
      "       produira des fichiers html convertis qui fonctionneront avec IE et Netscape." + newline +
      "       Si vous voulez utiliser un autre mod\u00e8le, vous pouvez le choisir dans le menu" + newline +
      "       sur l'\u00e9cran principal. Si vous choisissez Autre mod\u00e8le, vous serez autoris\u00e9 \u00e0" + newline +
      "       choisir un fichier qui sera utilis\u00e9 en tant que mod\u00e8le." + newline +
      "       Si vous choisissez un fichier, VEILLEZ A CE QU'IL S'AGISSE D'UN MODELE." + newline + newline +
      "9)  Conversion :" + newline + newline +
      "       Cliquez sur le bouton \"Convertir...\" pour lancer le processus de conversion." + newline +
      "       Une bo\u00eete de dialogue relative au traitement indiquera les fichiers trait\u00e9s," + newline +
      "       le nombre des applets trouv\u00e9es et le nombre des erreurs d\u00e9cel\u00e9es." + newline + newline +
      "10) Conversions suppl\u00e9mentaires ou Quitter :" + newline + newline +
      "       A la fin de la conversion, le bouton de la bo\u00eete de dialogue relative au" + newline +
      "       traitement passera de \"Annuler\" \u00e0 \"Termin\u00e9\". Vous pouvez choisir \"Termin\u00e9\"" + newline +
      "       pour fermer la bo\u00eete de dialogue." + newline +
      "       Choisissez ensuite \"Quitter\" pour fermer Java(TM) Plug-in HTML Converter," + newline +
      "       ou s\u00e9lectionnez un autre ensemble de fichiers \u00e0 convertir et choisissez de" + newline +
      "       nouveau \"Convertir...\"." + newline + newline +
      "11)  D\u00e9tails sur les mod\u00e8les :" + newline + newline +
      "       Le fichier mod\u00e8le est \u00e0 la base de la conversion des applets. Il s'agit" + newline +
      "       simplement d'un fichier de texte contenant des balises qui repr\u00e9sentent des" + newline +
      "       parties de l'applet d'origine." + newline +
      "       En ajoutant/supprimant/d\u00e9pla\u00e7ant les balises dans un fichier mod\u00e8le, vous" + newline +
      "       pouvez modifier la sortie du fichier converti." + newline + newline +
      "       Balises prises en charge :" + newline + newline +
      "        $OriginalApplet$    Cette balise est remplac\u00e9e par le texte complet de" + newline +
      "        l'applet d'origine." + newline + newline +
      "        $AppletAttributes$   Cette balise est remplac\u00e9e par tous les attributs" + newline +
      "        de l'applet (code, codebase, largeur, hauteur, etc.)" + newline + newline +
      "        $ObjectAttributes$   Cette balise est remplac\u00e9e par tous les attributs" + newline +
      "        requis par la balise object." + newline + newline +
      "        $EmbedAttributes$   Cette balise est remplac\u00e9e par tous les attributs" + newline +
      "        requis par la balise embed." + newline + newline +
      "        $AppletParams$    Cette balise est remplac\u00e9e par toutes les balises" + newline +
      "        <param ...> de l'applet." + newline + newline +
      "        $ObjectParams$    Cette balise est remplac\u00e9e par toutes les balises" + newline +
      "        <param...> requises par la balise object." + newline + newline +
      "        $EmbedParams$     Cette balise est remplac\u00e9e par toutes les balises" + newline +
      "        <param...> requises par la balise embed, sous la forme  NOM=VALEUR." + newline + newline +
      "        $AlternateHTML$  Cette balise est remplac\u00e9e par le texte de la zone" + newline +
      "        No support for applets de l'applet d'origine." + newline + newline +
      "        $CabFileLocation$   Ceci est l'URL du fichier cab qui devrait \u00eatre" + newline +
      "        utilis\u00e9 dans chaque mod\u00e8le ayant pour cible IE." + newline + newline +
      "        $NSFileLocation$    Ceci est l'URL du plugin Netscape qui devrait" + newline +
      "        \u00eatre utilis\u00e9 dans chaque mod\u00e8le ayant pour cible Netscape." + newline + newline +
      "        $SmartUpdate$   Ceci est l'URL de Netscape SmartUpdate qui devrait" + newline +
      "        \u00eatre utilis\u00e9 dans chaque mod\u00e8le ayant pour cible Netscape Navigator" + newline +
      "        4.0 ou sup." + newline + newline +
      "        $MimeType$    Ceci est le type MIME de l'objet Java." + newline + newline +
      "      default.tpl (mod\u00e8le par d\u00e9faut pour le convertisseur) -- La page" + newline +
      "      convertie peut \u00eatre utilis\u00e9e dans IE et Navigator sous Windows pour" + newline +
      "      appeler Java(TM) Plug-in." + newline +
      "      Ce mod\u00e8le peut aussi \u00eatre utilis\u00e9 avec Netscape sous Unix (Solaris)" + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- La page convertie peut \u00eatre utilis\u00e9e pour appeler uniquement" + newline +
      "      Java(TM)Plug-in dans IE sous Windows." + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- La page convertie peut \u00eatre utilis\u00e9e pour appeler Java(TM)" + newline +
      "      Plug-in dans Navigator sous Windows et Solaris." + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- La page convertie peut \u00eatre utilis\u00e9e dans n'importe quel navigateur" + newline +
      "      et plate-forme. Si le navigateur est IE ou Navigator sous Windows (Navigator sous" + newline +
      "      Solaris), Java(TM) Plug-in sera appel\u00e9. Sinon le JVM par d\u00e9faut du navigateur" + newline +
      "      sera utilis\u00e9." + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  Ex\u00e9cution du convertisseur HTML :" + newline + newline +
      "      Ex\u00e9cution de la version IUG du convertisseur HTML" + newline + newline +
      "      Le convertisseur HTML est contenu dans le JDK, pas dans le JRE. Pour l'ex\u00e9cuter," + newline +
      "      allez au sous-r\u00e9pertoire lib de votre r\u00e9pertoire d'installation de JDK. Par" + newline +
      "      exemple, si vous avez install\u00e9 le JDK sous Windows dans C:\\jdk" + j2seVersion + ", passez au" + newline +
      "      moyen de cd \u00e0 :" + newline + newline +
      "            C:\\jdk" + j2seVersion  + "\\lib\\" + newline + newline +
      "      Le convertisseur (htmlconverter.jar) est contenu dans ce r\u00e9pertoire." + newline + newline +
      "      Pour lancer le convertisseur, tapez :" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      Le lancement du convertisseur sous UNIX/Linux est analogue et se fait au moyen" + newline +
      "      des commandes ci-dessus." + newline +
      "      Voici d'autres m\u00e9thodes permettant de d\u00e9marrer le convertisseur." + newline + newline +
      "      Sous Windows" + newline +
      "      Lancement du convertisseur en utilisant Explorer." + newline +
      "      Utilisez Explorer pour naviguer vers le r\u00e9pertoire suivant :" + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      Double-cliquez sur l'application HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Ex\u00e9cutez les commandes suivantes :" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      Lancement du convertisseur \u00e0 partir de la ligne de commande :" + newline + newline +
      "      Format :" + newline + newline +
      "      java -jar htmlconverter.jar [-options1 valeur1 [-option2 valeur2" + newline +
      "      [...]]] [-simulate] [sp\u00e9cfichiers]" + newline + newline +
      "      sp\u00e9cfichiers:  liste de sp\u00e9cificateurs s\u00e9par\u00e9s par des virgules, caract\u00e8re" + newline +
      "      joker * (*.html *.htm)." + newline + newline +
      "      Options:" + newline + newline +
      "       source:    Chemin des fichiers (c:\\htmldocs sous Windows," + newline +
      "                  /home/user1/htmldocs sous Unix) Par d\u00e9faut : <r\u00e9p_utilisateur>" + newline +
      "                  S'il s'agit d'un chemin relatif, on assume qu'il est" + newline +
      "                  relatif au r\u00e9pertoire duquel HTMLConverter a \u00e9t\u00e9 lanc\u00e9." + newline + newline +
      "       backup:    Chemin pour l'\u00e9criture des fichiers de sauvegarde. Valeur" + newline +
      "                  par d\u00e9faut : <r\u00e9p_utilisateur>/<source>_bak." + newline +
      "                  S'il s'agit d'un chemin relatif, on assume qu'il est" + newline +
      "                  relatif au r\u00e9pertoire duquel HTMLConverter a \u00e9t\u00e9 lanc\u00e9." + newline + newline +
      "       subdirs:   Les fichiers des sous-r\u00e9pertoires doivent-ils \u00eatre trait\u00e9s ?" + newline +
      "                  Valeur par d\u00e9faut :  FALSE" + newline + newline +
      "       template:  Nom du fichier mod\u00e8le. Valeur par d\u00e9faut :  default.tpl-Standard" + newline +
      "                  (IE et Navigator) pour Windows et Solaris uniquement. UTILISEZ LA" + newline +
      "                  VALEUR PAR DEFAUT EN CAS DE DOUTE." + newline + newline +
      "       log:       Chemin et nom de fichier pour le journal (valeur par d\u00e9faut <r\u00e9p_utilisateur>/convert.log)" + newline + newline +
      "       progress:  Affichage de la progression de la sortie standard pendant la conversion." + newline +
      "                  Valeur par d\u00e9faut : false" + newline + newline +
      "       simulate:  Affichage des d\u00e9tails sp\u00e9cifiques de la conversion sans convertir." + newline +
      "                  UTILISEZ CETTE OPTION EN CAS DE DOUTE SUR LA CONVERSION." + newline +
      "                  VOUS OBTIENDREZ UNE LISTE DE DETAILS SPECIFIQUES DE LA" + newline +
      "                  CONVERSION." + newline + newline +
      "      Si seul \"java -jar htmlconverter.jar -gui\" est sp\u00e9cifi\u00e9 (uniquement l'option" + newline +
      "      -gui sans sp\u00e9cification), la version IUG du convertisseur sera lanc\u00e9e. Sinon," + newline +
      "      l'IUG sera supprim\u00e9e." + newline + newline +
      "      Pour plus d'informations, consultez l'url suivant :" + newline + newline +
      "      http://java.sun.com/j2se/" +
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}
