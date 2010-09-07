/*
 * @(#)ConverterHelp_es.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * Spanish version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_es extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Archivo L\u00e9ame de Java(TM) Plug-in HTML Converter" + newline + newline +
      "Versi\u00f3n:  " + j2seVersion + newline + newline + newline +
      "*****   HAGA UNA COPIA DE SEGURIDAD DE LOS ARCHIVOS ANTES DE CONVERTIRLOS CON" + newline +
      "*****   ESTA HERRAMIENTA." + newline +
      "*****   LA CANCELACI\u00d3N DE UNA CONVERSI\u00d3N NO IMPLICA QUE SE RECUPERE" + newline +
      "*****   EL ESTADO PREVIO A LOS CAMBIOS." + newline +
      "*****   LOS COMENTARIOS SITUADOS DENTRO DE LAS ETIQUETAS APPLET NO SE TIENEN EN" + newline +
      "*****   CUENTA." + newline + newline + newline +
      "Contenido:" + newline +
      "   1.  Novedades" + newline +
      "   2.  Errores de c\u00f3digo corregidos" + newline +
      "   3.  Acerca de Java(TM) Plug-in HTML Converter" + newline +
      "   4.  Proceso de conversi\u00f3n" + newline +
      "   5.  Selecci\u00f3n de los archivos para la conversi\u00f3n" + newline +
      "   6.  Selecci\u00f3n de la carpeta de copias de seguridad" + newline +
      "   7.  Generaci\u00f3n de un archivo de registro" + newline +
      "   8.  Selecci\u00f3n de la plantilla de conversi\u00f3n" + newline +
      "   9.  Conversi\u00f3n" + newline +
      "  10.  M\u00e1s conversiones o salida" + newline +
      "  11.  Informaci\u00f3n sobre las plantillas" + newline +
      "  12.  Ejecuci\u00f3n del convertidor de HTML (Windows y Solaris)" + newline + newline +
      "1)  Novedades:" + newline + newline +
      "    o Ampliaci\u00f3n de las plantillas para funcionar con Netscape 6." + newline +
      "    o Actualizaci\u00f3n de todas las plantillas para poder utilizar nuevas" + newline +
      "      funciones multiversi\u00f3n de Java Plug-in." + newline +
      "    o Mejoras en la interfaz de usuario con Swing 1.1 para proporcionar" + newline +
      "      compatibilidad con i18n." + newline +
      "    o Mejoras en el cuadro de di\u00e1logo de opciones avanzadas para poder" + newline +
      "      utilizar las nuevas marcas de plantillas de SmartUpdate y MimeType." + newline +
      "    o Mejoras del Convertidor de HTML para poderlo utilizar con" + newline +
      "      Java Plug-in 1.1.x, Java Plug-in 1.2.x, Java Plug-in 1.3.x," + newline +
      "      Java Plug-in 1.4.x y Java Plug-in 1.5.x." + newline +
      "    o Mejoras de la compatibilidad con SmartUpdate y MimeType en todas" + newline +
      "      las plantillas de conversi\u00f3n." + newline +
      "    o Adici\u00f3n de \"scriptable=false\" a la etiqueta OBJECT/EMBED en todas" + newline +
      "      las plantillas." + newline + newline +
      "     Esto se utiliza para desactivar la generaci\u00f3n de typelib cuando Java" + newline +
      "     Plug-in no se utiliza para ejecutar secuencias de \u00f3rdenes." + newline + newline + newline +
      "2)  Errores de c\u00f3digo corregidos:" + newline + newline +
      "    o Mejoras en el manejo de errores cuando no se encuentran los" + newline +
      "      archivos de propiedades." + newline +
      "    o Mejoras en la conversi\u00f3n de HTML para que la etiqueta EMBED/OBJECT" + newline +
      "      resultante pueda utilizarse con AppletViewer en JDK 1.2.x." + newline +
      "    o Se han suprimido archivos innecesarios que quedaban del" + newline +
      "      Convertidor de HTML 1.1.x." + newline +
      "    o Posibilidad de generar EMBED/OBJECT con nombres de atributos CODE," + newline +
      "      CODEBASE, etc. en lugar de JAVA_CODE, JAVA_CODEBASE, etc. Esto permite" + newline +
      "      utilizar la p\u00e1gina generada en el AppletViewer de JDK 1.2.x." + newline +
      "    o Posibilidad de convertir MAYSCRIPT si est\u00e1 presente en la etiqueta" + newline +
      "      APPLET." + newline + newline +
      "3)  Acerca de Java(TM) Plug-in HTML Converter:" + newline + newline +
      "        El convertidor de HTML de Java(TM) Plug-in es una herramienta" + newline +
      "        que se utiliza para convertir cualquier p\u00e1gina HTML que contenga" + newline +
      "        miniaplicaciones en un formato legible para Java(TM) Plug-in." + newline + newline +
      "4)  Proceso de conversi\u00f3n:" + newline + newline +
      "        El convertidor transforma cualquier archivo que contenga miniaplicaciones" + newline +
      "        en un formato que pueda utilizarse con Java(TM) Plug-in." + newline + newline +
      "        El proceso de conversi\u00f3n de los archivos es como sigue:" + newline +
      "        En primer lugar, el c\u00f3digo HTML que no forma parte de la miniaplicaci\u00f3n" + newline +
      "        se traslada del archivo de origen a un archivo temporal. Cuando se" + newline +
      "        detecta una etiqueta <APPLET, el convertidor analiza todo el" + newline +
      "        c\u00f3digo hasta la siguiente etiqueta </APPLET (no incluida entre" + newline +
      "        comillas) y fusiona los datos de la miniaplicaci\u00f3n con la plantilla" + newline +
      "        (consulte la secci\u00f3n Informaci\u00f3n sobre las plantillas, m\u00e1s adelante)." + newline +
      "        Si esta operaci\u00f3n se realiza sin errores, el archivo html original se" + newline +
      "        traslada a la carpeta de copias de seguridad y el archivo temporal" + newline +
      "        pasa a denominarse como el archivo original. De esta forma, los" + newline +
      "        archivos originales nunca desaparecen del disco." + newline + newline +
      "        Tenga presente que la utilidad convertir\u00e1 los archivos de forma efectiva." + newline +
      "        Por tanto, una vez ejecutado el convertidor, los archivos quedar\u00e1n" + newline +
      "        configurados para utilizar Java(TM) Plug-in." + newline + newline +
      "5)  Selecci\u00f3n de los archivos para la conversi\u00f3n:" + newline + newline +
      "       Para convertir todos los archivos de una carpeta, puede escribir la" + newline +
      "       ruta de acceso a la misma o usar el bot\u00f3n Examinar para elegirla" + newline +
      "       desde el cuadro de di\u00e1logo." + newline +
      "       Despu\u00e9s de elegir la ruta de acceso, puede especificar los nombres de" + newline +
      "       los archivos en \"Correspondencia de nombres de archivo\". Estos nombres" + newline +
      "       deben ir separados por comas. Puede utilizar * como car\u00e1cter comod\u00edn. Si" + newline +
      "       escribe un nombre de archivo con un comod\u00edn, s\u00f3lo se convertir\u00e1n los" + newline +
      "       archivos que coincidan con este tipo. Por \u00faltimo, seleccione la casilla" + newline +
      "       \"Incluir subcarpetas\" si quiere que se conviertan los archivos de las" + newline +
      "       subcarpetas cuyo nombre coincida con el nombre de archivo especificado." + newline + newline +
      "6)  Selecci\u00f3n de la carpeta de copias de seguridad:" + newline +
      "       La carpeta de copias de seguridad predeterminada tiene la misma ruta" + newline +
      "       de acceso que la carpeta de origen, pero con el sufijo \"_BAK\". Por" + newline +
      "       ejemplo, si la ruta de acceso del archivo de origen es c:/html/applet.html" + newline +
      "       (lo que convierte s\u00f3lo un archivo), la carpeta de la copia de seguridad" + newline +
      "       ser\u00e1 c:/html_BAK. Si la carpeta de origen es c:/html (lo que convierte" + newline +
      "       todos los archivos que contenga), la carpeta de copia de seguridad ser\u00e1" + newline +
      "       c:/html_BAK. El nombre predeterminado de la carpeta de copias de" + newline +
      "       seguridad puede cambiarse escribiendo otro nombre en el campo" + newline +
      "       \"Archivos de copia de seguridad en carpeta:\" o buscando una carpeta" + newline +
      "       con el bot\u00f3n Examinar." + newline + newline +
      "       Unix(Solaris):" + newline +
      "       La ruta de acceso a la carpeta de copias de seguridad predeterminada es" + newline +
      "       la misma que la de la carpeta de origen, pero con el sufijo \"_BAK\"." + newline +
      "       Por ejemplo, si la carpeta original tiene la ruta de acceso" + newline +
      "       /home/user1/html/applet.html (lo que convierte s\u00f3lo un archivo)," + newline +
      "       la carpeta de la copia de seguridad ser\u00e1 /home/user1/html_BAK. Si la" + newline +
      "       carpeta de origen es /home/user1/html (lo que convierte todos los" + newline +
      "       archivos que contenga), la ruta de acceso a la copia de seguridad ser\u00e1" + newline +
      "       /home/user1/html_BAK. El nombre predeterminado de la carpeta de copias" + newline +
      "       de seguridad puede cambiarse escribiendo otro nombre en el campo" + newline +
      "       \"Archivos de copia de seguridad en carpeta:\" o buscando una carpeta" + newline +
      "       con el bot\u00f3n Examinar." + newline + newline +
      "7)  Generaci\u00f3n de un archivo de registro:" + newline + newline +
      "       Si desea generar un archivo de registro, marque la casilla" + newline +
      "       \"Generar archivo de registro\". Puede introducir la ruta de acceso" + newline +
      "       y el nombre del archivo directamente, o buscar una carpeta, escribir" + newline +
      "       el nombre del archivo y seleccionar Abrir. El archivo de registro" + newline +
      "       contiene informaci\u00f3n b\u00e1sica relativa al proceso de conversi\u00f3n." + newline + newline +
      "8)  Selecci\u00f3n de la plantilla de conversi\u00f3n:" + newline + newline +
      "       Si no se selecciona ninguna plantilla, se utilizar\u00e1 la plantilla" + newline +
      "       predeterminada. \u00c9sta genera los archivos HTML que funcionar\u00e1n con" + newline +
      "       IE y Netscape. Si desea utilizar otra plantilla, puede elegirla en" + newline +
      "       el men\u00fa de la pantalla principal. Si lo hace, podr\u00e1 seleccionar un" + newline +
      "       archivo que actuar\u00e1 como plantilla. En ese caso, al elegir el archivo" + newline +
      "       ASEG\u00daRESE DE QUE SEA UNA PLANTILLA." + newline + newline +
      "9)  Conversi\u00f3n:" + newline + newline +
      "       Pulse el bot\u00f3n \"Convertir...\" para iniciar el proceso de conversi\u00f3n." + newline +
      "       Un cuadro de di\u00e1logo mostrar\u00e1 los archivos que se est\u00e1n procesando, el" + newline +
      "       n\u00famero de proceso de los archivos, el n\u00famero de miniaplicaciones" + newline +
      "       encontradas y el n\u00famero de errores detectados." + newline + newline +
      "10) M\u00e1s conversiones o salida:" + newline + newline +
      "       Cuando termine la conversi\u00f3n, el bot\u00f3n del cuadro de di\u00e1logo del proceso" + newline +
      "       cambiar\u00e1 de \"Cancelar\" a \"Realizado\". Puede elegir \"Realizado\"" + newline +
      "       para cerrar el cuadro. A continuaci\u00f3n, elija \"Salir\" para cerrar el" + newline +
      "       convertidor o seleccione otro grupo de archivos para efectuar una" + newline +
      "       nueva conversi\u00f3n y elija \"Convertir...\"." + newline + newline +
      "11)  Informaci\u00f3n sobre las plantillas:" + newline + newline +
      "       El archivo de plantilla es la base que permite convertir las" + newline +
      "       miniaplicaciones. Es s\u00f3lo un archivo de texto con etiquetas" + newline +
      "       que representan partes de la miniaplicaci\u00f3n original." + newline +
      "       La salida del archivo convertido puede modificarse" + newline +
      "       agregando, suprimiendo o cambiando de lugar las etiquetas" + newline +
      "       de la plantilla." + newline + newline +
      "       Etiquetas admitidas:" + newline + newline +
      "        $OriginalApplet$    Esta etiqueta se sustituye por el texto completo de" + newline +
      "                            la miniaplicaci\u00f3n original." + newline + newline +
      "        $AppletAttributes$   Se sustituye por todos los atributos de la" + newline +
      "                             miniaplicaci\u00f3n (code, codebase, width, height, etc.)." + newline + newline +
      "        $ObjectAttributes$   Se sustituye por todos los atributos que necesita" + newline +
      "                             la etiqueta object." + newline + newline +
      "        $EmbedAttributes$   Se sustituye por todos los atributos que necesita" + newline +
      "                            la etiqueta embed." + newline + newline +
      "        $AppletParams$    Se sustituye por todas las etiquetas" + newline +
      "                          <param ...> de la miniaplicaci\u00f3n." + newline + newline +
      "        $ObjectParams$    Se sustituye por todas las etiquetas <param...>" + newline +
      "                          que necesita la etiqueta object." + newline + newline +
      "        $EmbedParams$     Se sustituye por todas las etiquetas <param...> que" + newline +
      "                          necesita la etiqueta embed en la forma NAME=VALUE." + newline + newline +
      "        $AlternateHTML$  Se sustituye por el texto de la secci\u00f3n No support for" + newline +
      "                         applets de la miniaplicaci\u00f3n original." + newline + newline +
      "        $CabFileLocation$   Es el URL del archivo cab que deber\u00eda utilizarse en" + newline +
      "                            cada plantilla que tenga IE como destino." + newline + newline +
      "        $NSFileLocation$    Es el URL del complemento de Netscape que se utilizar\u00e1" + newline +
      "                            en cada plantilla que tenga Netscape como destino." + newline + newline +
      "        $SmartUpdate$   Es el URL del SmartUpdate de Netscape que se utilizar\u00e1" + newline +
      "                        en cada plantilla que tenga Netscape Navigator 4.0 o una" + newline +
      "                        versi\u00f3n superior como destino." + newline + newline +
      "        $MimeType$    Es el tipo MIME del objeto Java." + newline + newline +
      "      default.tpl (plantilla predeterminada del convertidor) - la p\u00e1gina" + newline +
      "      convertida puede utilizarse en IE y Navigator en Windows para llamar" + newline +
      "      a Java(TM) Plug-in." + newline +
      "      Esta plantilla tambi\u00e9n puede utilizarse con Netscape en Unix (Solaris)" + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- la p\u00e1gina convertida puede utilizarse para llamar a" + newline +
      "      Java(TM) Plug-in en IE s\u00f3lo en Windows." + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- la p\u00e1gina convertida puede utilizarse para llamar a" + newline +
      "      Java(TM) Plug-in en Navigator en Windows y Solaris." + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- la p\u00e1gina convertida puede utilizarse en cualquier navegador" + newline +
      "      y plataforma. Si el navegador es IE o Navigator en Windows (Navigator en" + newline +
      "      Solaris), se ejecutar\u00e1 Java(TM) Plug-in. De lo contrario, se utilizar\u00e1" + newline +
      "      la JVM predeterminada del navegador." + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  Ejecuci\u00f3n del convertidor de HTML:" + newline + newline +
      "      Ejecuci\u00f3n del convertidor con la interfaz gr\u00e1fica" + newline + newline +
      "      El convertidor est\u00e1 incluido en el JDK, no en el JRE. Para" + newline +
      "      ejecutarlo, sit\u00faese en el subdirectorio lib del directorio de" + newline +
      "      instalaci\u00f3n del JDK. Por ejemplo, si ha instalado el JDK en el" + newline +
      "      directorio C:\\jdk " + j2seVersion + " de Windows, cambie al directorio" + newline + newline +
      "            C:\\jdk" + j2seVersion  + "\\lib\\" + newline + newline +
      "      El convertidor (htmlconverter.jar) est\u00e1 incluido en ese directorio." + newline + newline +
      "      Para ejecutar el tipo de convertidor:" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      El convertidor se ejecuta de forma parecida en UNIX/Linux utilizando" + newline +
      "      las \u00f3rdenes antes citadas." + newline +
      "      He aqu\u00ed otras formas de iniciar el convertidor." + newline + newline +
      "      En Windows" + newline +
      "      Para iniciar el convertidor con Internet Explorer." + newline +
      "      Utilice Internet Explorer para situarse en el siguiente directorio." + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      Pulse dos veces en la aplicaci\u00f3n HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Ejecute las \u00f3rdenes siguientes" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      Ejecuci\u00f3n del convertidor desde la l\u00ednea de comandos:" + newline + newline +
      "      Formato:" + newline + newline +
      "      java -jar htmlconverter.jar [-opciones1 valor1 [-opci\u00f3n2 valor2" + newline +
      "      [...]]] [-simulate] [archivos]" + newline + newline +
      "      archivos:  Lista delimitada por espacios donde se especifican" + newline +
      "                 los archivos; el asterisco (*) act\u00faa como comod\u00edn" + newline +
      "                 (*.html *.htm)." + newline + newline +
      "      Opciones:" + newline + newline +
      "       source:    Ruta de acceso a los archivos (c:\\htmldocs en Windows," + newline +
      "                  /home/user1/htmldocs en Unix). Predeterminada: <dirusuario>" + newline +
      "                  Si la ruta es relativa, se supone que lo es con respecto al" + newline +
      "                  directorio desde el que se ha ejecutado HTMLConverter." + newline + newline +
      "       backup:    Ruta de acceso a la carpeta de copias de seguridad." + newline +
      "                  Predeterminada: <dirusuario>/<origen>_bak" + newline +
      "                  Si la ruta es relativa, se supone que lo es con respecto al" + newline +
      "                  directorio desde el que se ha ejecutado HTMLConverter." + newline + newline +
      "       subdirs:   Indica si deben procesarse los subdirectorios de la carpeta." + newline +
      "                  Predeterminado:  FALSE" + newline + newline +
      "       template:  Nombre del archivo de plantilla." + newline +
      "                  Predeterminado:  default.tpl- Est\u00e1ndar (IE y Navigator)" + newline +
      "                  s\u00f3lo para Windows y Solaris. UTILICE LA PLANTILLA" + newline +
      "                  PREDETERMINADA SI NO EST\u00c1 SEGURO." + newline + newline +
      "       log:       Ruta de acceso y nombre del archivo de registro" + newline +
      "                  (predeterminado: <dirusuario>/convert.log)." + newline + newline +
      "       progress:  Muestra los resultados del proceso de conversi\u00f3n mientras" + newline +
      "                  se ejecuta. Predeterminado: false" + newline + newline +
      "       simulate:  Proporciona los datos de la conversi\u00f3n sin llevarla a cabo." + newline +
      "                  UTILICE ESTA OPCI\u00d3N SI NO EST\u00c1 SEGURO DE QUERER HACER LA" + newline +
      "                  CONVERSI\u00d3N. RECIBIR\u00c1 UNA LISTA DE DATOS RELATIVOS A LA" + newline +
      "                  CONVERSI\u00d3N." + newline + newline +
      "      Si especifica \u00fanicamente \"java -jar htmlconverter.jar -gui\" (s\u00f3lo la" + newline +
      "      opci\u00f3n -gui, sin especificar los archivos), se ejecuta la versi\u00f3n" + newline +
      "      gr\u00e1fica (GUI) del convertidor. De lo contrario, \u00e9sta no se ejecuta." + newline + newline +
      "      Encontrar\u00e1 m\u00e1s informaci\u00f3n en el siguiente url:" + newline + newline +
      "      http://java.sun.com/j2se/" +
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}

