/*
 * @(#)ConverterHelp_pt_BR.java	1.4 10/04/22
 *
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * US English version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_pt_BR extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Leiame do conversor HTML do plug-in Java(TM)" + newline + newline +
      "Vers\u00e3o:  " + j2seVersion + newline + newline + newline +
      "*****   FAZER BACKUP DE TODOS OS ARQUIVOS ANTES DE CONVERT\u00ca-LOS COM ESTA FERRAMENTA." + newline +
      "*****   O CANCELAMENTO DA COVERS\u00c3O N\u00c3O REVERTER\u00c1 AS ALTERA\u00c7\u00d5ES." + newline +
      "*****   OS COMENT\u00c1RIOS DENTRO DAS MARCAS DO APPLET S\u00c3O IGNORADOS." + newline + newline + newline +
      "Conte\u00fado:" + newline +
      "   1.  Novos recursos" + newline +
      "   2.  Corre\u00e7\u00f5es de erros" + newline +
      "   3.  Sobre o conversor HTML do plug-in Java(TM)" + newline +
      "   4.  O processo de convers\u00e3o" + newline +
      "   5.  Selecionando arquivos dentro das pastas para convers\u00e3o" + newline +
      "   6.  Selecionando a pasta de backup" + newline +
      "   7.  Gerando um arquivo de registro" + newline +
      "   8.  Selecionando um modelo de convers\u00e3o" + newline +
      "   9.  Convertendo" + newline +
      "  10.  Mais convers\u00f5es ou sair" + newline +
      "  11.  Detalhes sobre os modelos" + newline +
      "  12.  Executando o conversor HTML (Windows e Solaris)" + newline + newline +
      "1)  Novos recursos:" + newline + newline +
      "    o Modelos estendidos atualizados para oferecer suporte ao Netscape 6." + newline +
      "    o Todos os modelos atualizados para oferecer suporte a novos recursos de v\u00e1rias vers\u00f5es no plug-in Java." + newline +
      "    o Interface de usu\u00e1rio aprimorada com Swing 1.1 para suporte a i18n." + newline +
      "    o Caixa de di\u00e1logo Op\u00e7\u00f5es avan\u00e7adas aprimorada para oferecer suporte a novas marcas de modelo de" + newline +
      "      SmartUpadate e MimeType." + newline +
      "    o Conversor HTML aprimorado para ser usado com Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x, Java Plug-in 1.4.x" + newline +
      "      e Java Plugin-in 1.5.x." + newline +
      "    o Suporte aprimorado a SmartUpdate e MimeType em todos os modelos de" + newline +
      "     convers\u00e3o" + newline +
      "    o \"scriptable=false\" adicionado \u00e0 marca OBJECT/EMBED em todos os modelos." + newline + newline +
      "    \u00c9 usado para desabilitar a gera\u00e7\u00e3o de bibliotecas de tipos quando o" + newline +
      "     plug-in Java n\u00e3o estiver sendo usado para script." + newline + newline + newline +
      "2)  Erros corrigidos:" + newline + newline +
      "    o Tratamento aprimorado de erros quando os arquivos de propriedades n\u00e3o forem encontrados." + newline +
      "    o Convers\u00e3o HTML aprimorada de modo que a marca EMBED/OBJECT possa" + newline +
      "    ser usada no AppletViewer do JDK 1.2.x." + newline +
      "    o Eliminados os arquivos desnecess\u00e1rios que n\u00e3o utilizados do HTML Converter 1.1.x." + newline +
      "    o EMBED/OBJECT gerada com nomes de atributo CODE, CODEBASE, etc., em vez de JAVA_CODE," + newline +
      "    JAVA_CODEBASE, etc. Isso permitiu que a p\u00e1gina gerada" + newline +
      "     fosse usada no AppletViewer do JDK 1.2.x." + newline +
      "    o Suporte \u00e0 convers\u00e3o MAYSCRIPT se estiver presente na" + newline +
      "    marca APPLET." + newline + newline +
      "3)  Sobre o conversor HTML do plug-in Java(TM):" + newline + newline +
      "        O conversor HTML do plug-in Java(TM) \u00e9 um utilit\u00e1rio que permite converter qualquer" + newline +
      "        p\u00e1gina HTML que contenha applets em um formato que usar\u00e1 o" + newline +
      "        plug-in Java(TM)." + newline + newline +
      "4)  O processo de convers\u00e3o:" + newline + newline +
      "        O conversor HTML do plug-in Java(TM) converter\u00e1 quaisquer arquivos que contenham" + newline +
      "        applets em um formato que pode ser usado com o plug-in Java(TM)." + newline + newline +
      "        O processo de convers\u00e3o de cada arquivo ocorre da seguinte forma:" + newline +
      "        Primeiro, o HTML que n\u00e3o faz parte do applet \u00e9 transferido do arquivo de origem para um" + newline +
      "        arquivo tempor\u00e1rio.  Quando uma marca <APPLET for encontrada, o conversor analisar\u00e1 a primeira marca" + newline +
      "        </APPLET (n\u00e3o entre aspas) do applet e combinar\u00e1 os dados do applet com o modelo. (Consulte Detalhes" + newline +
      "        sobre os modelos, abaixo) Se esse processo for conclu\u00eddo sem erros, o arquivo html original ser\u00e1 movido" + newline +
      "        para a pasta de backup e o arquivo tempor\u00e1rio ser\u00e1 ent\u00e3o renomeado com o nome do arquivo original. " + newline +
      "        Assim, os arquivos originais nunca ser\u00e3o removidos do disco." + newline +
      "        Observe que o conversor converter\u00e1 eficazmente os arquivos no pr\u00f3prio local." + newline + newline +
      "        De modo que, depois que o conversor for executado, seus arquivos ser\u00e3o configurados" + newline +
      "        para usar o plug-in Java(TM)." + newline +


      "5)  Selecionando arquivos dentro das pastas para convers\u00e3o:" + newline + newline +
      "       Para converter todos os arquivos de uma pasta, voc\u00ea pode digitar o caminho para" + newline +
      "       a pasta ou pode optar pelo bot\u00e3o Explorar para selecionar uma pasta em uma caixa de di\u00e1logo." + newline  +
      "       Depois de selecionar um caminho, voc\u00ea pode fornecer qualquer n\u00famero aos especificadores" + newline +
      "       de arquivos em \"Nomes de arquivos correspondentes\".  Cada especificador deve estar separado por v\u00edrgula." + newline +
      "       Voc\u00ea pode usar * como coringa.  Se colocar um nome de arquivo com coringa, somente tal arquivo ser\u00e1 convertido." + newline  +
      "       Por \u00faltimo, selecione a caixa de sele\u00e7\u00e3o \"Incluir subpastas\" se quiser que sejam convertidos todos os" + newline +
      "       arquivos destas pastas aninhadas que correspondam ao nome do arquivo." + newline + newline +
      "6)  Selecionando a pasta de backup:" + newline +

      "       O caminho padr\u00e3o da pasta de backup \u00e9 o caminho de origem com um \"_BAK\" anexado" + newline +
      "       ao nome. Isto \u00e9,  se o caminho de origem fosse c:/html/applet.html (convertendo um arquivo)," + newline +
      "        o caminho de backup seria, ent\u00e3o, c:/html_BAK.  Se o caminho de origem fosse" + newline +
      "       c:/html (convertendo todos os arquivos do caminho), o caminho de backup seria, ent\u00e3o," + newline +
      "       c:/html_BAK. O caminho de backup pode ser alterado digitando um caminho no seguinte" + newline +
      "        campo \"Backup de arquivos na pasta:\" ou indo para uma pasta." + newline + newline +

      "       Unix(Solaris):" + newline +
      "       O caminho padr\u00e3o da pasta de backup \u00e9 o caminho de origem com um \"_BAK\" anexado" + newline +
      "       ao nome. Isto \u00e9,  se o caminho de origem fosse /home/user1/html/applet.html" + newline +
      "       (convertendo um arquivo), o caminho de backup seria, ent\u00e3o, /home/user1/html_BAK." + newline +
      "       Se o caminho de origem fosse /home/user1/html (convertendo todos os arquivos)," + newline +
      "       o caminho de backup seria, ent\u00e3o, /home/user1/html_BAK. O caminho de backup pode ser alterado" + newline +
      "       digitando um caminho no seguinte campo \"Backup de arquivos na pasta:\" ou indo para uma pasta." + newline + newline +
      "7)  Gerando um arquivo de registro:" + newline + newline +
      "       Se quiser que o arquivo de registro seja gerado, selecione a caixa de sele\u00e7\u00e3o" + newline +
      "       \"Generate Log File\". Voc\u00ea pode inserir o caminho e o nome do arquivo ou explorar para" + newline +
      "        selecionar uma pasta, em seguida, digitar o nome do arquivo e selecionar Abrir. " + newline +
      "       O arquivo de registro cont\u00e9m informa\u00e7\u00f5es b\u00e1sicas relacionadas ao" + newline +
      "        processo de convers\u00e3o." + newline + newline +
      "8)  Selecionando um modelo de convers\u00e3o:" + newline + newline +
      "       Se um modelo padr\u00e3o ser\u00e1 usado caso nenhum" + newline +
      "        modelo tenha sido selecionado." + newline +
      "         Este modelo criar\u00e1 arquivos html convertidos que funcionar\u00e3o com IE e Netscape." + newline +
      "       Se quiser usar outro modelo, voc\u00ea pode selecion\u00e1-lo no menu da tela principal. " + newline +
      "        Se selecionar outro, voc\u00ea poder\u00e1 escolher o arquivo que ser\u00e1 usado como modelo." + newline +
      "       Se selecionar um arquivo, CERTIFIQUE-SE DE QUE ELE SEJA UM MODELO." + newline + newline +
      "9)  Convertendo:" + newline + newline +
      "       Clique no bot\u00e3o \"Converter...\" para iniciar o processo de convers\u00e3o. Uma caixa de di\u00e1logo do" + newline +
      "       processo mostra os arquivos que est\u00e3o sendo processados, o n\u00famero do processo dos arquivos," + newline +
      "       o n\u00famero de applets encontrados e o n\u00famero de erros encontrados." + newline + newline +
      "10) Mais convers\u00f5es ou sair:" + newline + newline +
      "       Quando a convers\u00e3o for conclu\u00edda, o bot\u00e3o da caixa de di\u00e1logo do processo se alterar\u00e1" + newline +
      "       de \"Cancelar\" para \"Conclu\u00eddo\".  Voc\u00ea pode escolher \"Conclu\u00eddo\" para fechar a caixa de di\u00e1logo." + newline  +
      "       Neste momento, selecione \"Sair\" para fechar o conversor HTML do plug-in Java(TM)" + newline +
      "       ou selecione outro conjunto de arquivos para convers\u00e3o e escolha \"Converter...\" novamente." + newline + newline +
      "11)  Detalhes sobre os modelos:" + newline + newline +
      "       O arquivo de modelo \u00e9 a base da convers\u00e3o de applets." + newline +
      "       Trata-se simplesmente de um arquivo" + newline +
      "       de texto que cont\u00e9m marcas que representam partes do applet original." + newline +
      "       Ao adicionar/remover/mover as marcas em um arquivo de modelo, voc\u00ea pode alterar a sa\u00edda" + newline +
      "       do arquivo convertido." + newline + newline +
      "       Marcas suportadas:" + newline + newline +
      "        $OriginalApplet$    Esta marca \u00e9 substitu\u00edda por todo o texto" + newline +
      "        do applet original." + newline + newline +
      "        $AppletAttributes$   Esta marca \u00e9 substitu\u00edda por todos os" + newline +
      "        atributos dos applets. (c\u00f3digo, base de c\u00f3digo, largura, altura, etc.)" + newline + newline +
      "        $ObjectAttributes$   Esta marca \u00e9 substitu\u00edda por todos os" + newline + newline +
      "        atributos exigidos pela marca OBJECT." + newline + newline +
      "        $EmbedAttributes$   Esta marca \u00e9 substitu\u00edda por todos os" + newline +
      "        atributos exigidos pela marca EMBED." + newline + newline +
      "        $AppletParams$    Esta marca \u00e9 substitu\u00edda por todas as marcas" + newline +
      "        <param ...> do applet." + newline + newline +
      "        $ObjectParams$    Esta marca \u00e9 substitu\u00edda por todas as marcas" + newline +
      "        <param...> exigidas pela marca OBJECT." + newline + newline +
      "        $EmbedParams$     Esta marca \u00e9 substitu\u00edda por todas as marcas" + newline +
      "        <param...> exigidas pela marca EMBED na forma NAME=VALUE." + newline + newline +
      "        $AlternateHTML$  Esta marca \u00e9 substitu\u00edda por todo o texto na \u00e1rea" + newline +
      "        Sem suporte para applets do applet original." + newline + newline +
      "        $CabFileLocation$   Trata-se da URL do arquivo cab que deveria" + newline +
      "        ser usada em cada modelo com destino ao IE." + newline + newline +
      "        $NSFileLocation$    Trata-se da URL do plug-in Netscape que" + newline +
      "        ser\u00e1 usada em cada modelo com destino ao Netscape." + newline + newline +
      "        $SmartUpdate$   Trata-se da URL do Netscape SmartUpdate que ser\u00e1 usada" + newline +
      "        em cada modelo com destino ao Netscape Navigator 4.0 ou posterior." + newline + newline +
      "        $MimeType$    Trata-se do tipo MIME do objeto default.tpl Java" + newline + newline +
      "        (o modelo padr\u00e3o para o conversor) \u2013 a p\u00e1gina convertida pode ser usada no IE e no" + newline +
      "        Navigator do Windows para chamar o plug-in Java(TM)." + newline +
      "      Este modelo tamb\u00e9m pode ser usado com Netscape no Unix (Solaris)" + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl \u2013 a p\u00e1gina convertida pode ser usada para chamar o" + newline +
      "        plug-in Java(TM) no IE em Windows somente." + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- a p\u00e1gina convertida pode ser usada para chamar o" + newline +
      "        plug-in Java(TM) no Navigator em Windows e Solaris." + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl \u2013 a p\u00e1gina convertida pode ser usada em qualquer navegador e em qualquer plataforma." + newline +
      "      Se o navegador for IE ou Navigator no Windows (Navigator no Solaris), o plug-in Java(TM) ser\u00e1 chamado. Do contr\u00e1rio, ser\u00e1 usado o navegador padr\u00e3o da JVM." + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  Executando o conversor HTML:" + newline + newline +
      "      Executando a vers\u00e3o da GUI do conversor HTML" + newline + newline +
      "      O conversor HTML est\u00e1 no JDK, n\u00e3o no JRE. Para executar o conversor," + newline +
      "      v\u00e1 ao subdiret\u00f3rio lib do diret\u00f3rio de instala\u00e7\u00e3o do JDK. Por exemplo," + newline +
      "      se tiver instalado o JDK no Windows em C:\\jdk j2seVersion ," + newline + newline +
      "      ent\u00e3o v\u00e1 a C:\\jdk j2seVersion \\lib\\" + newline + newline +
      "      O conversor (htmlconverter.jar) est\u00e1 em tal diret\u00f3rio." + newline + newline +
      "      Para iniciar o tipo de conversor:" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      Iniciar o conversor no UNIX/Linux \u00e9 semelhante a usar os comandos acima." + newline +
      "      Encontram-se abaixo outras formas de iniciar o conversor" + newline + newline +
      "      No Windows" + newline +
      "      Para iniciar o conversor usando o Explorer." + newline +
      "      Use o Explorer para ir at\u00e9 o diret\u00f3rio seguinte." + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      Clique duas vezes no aplicativo HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Execute os seguintes comandos" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      Executando o conversor a partir da linha de comando:" + newline + newline +
      "      Formato:" + newline + newline +
      "      java -jar htmlconverter.jar [-options1 value1 [-option2 value2" + newline +
      "      [...]]] [-simulate] [filespecs]" + newline + newline +
      "      filespecs:  lista de espa\u00e7os limitados das especifica\u00e7\u00f5es do arquivo, * coringa. " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      Op\u00e7\u00f5es:" + newline + newline +
      "       source:    Caminho para os arquivos.  (c:\\htmldocs no Windows," + newline +
      "                  /home/user1/htmldocs no Unix) Padr\u00e3o: <userdir>" + newline +
      "                  Se o caminho for relativo, sup\u00f5e-se que \u00e9 relativo ao diret\u00f3rio em que o conversor" + newline +
      "                  HTML foi iniciado." + newline + newline +
      "       backup:    Caminho no qual gravar os arquivos de backup.  Padr\u00e3o:" + newline +
      "                  <userdir>/<source>_bak" + newline +
      "                  Se o caminho for relativo, sup\u00f5e-se que \u00e9 relativo ao diret\u00f3rio" + newline +
      "                  em que o conversor HTML foi iniciado." + newline +
      "       subdirs:   Se os arquivos dos subdiret\u00f3rios deveriam ser processados. " + newline +
      "                  Padr\u00e3o:  FALSE" + newline + newline +
      "       template:  Nome do arquivo de modelo.  Padr\u00e3o:  default.tpl-Standard " + newline +
      "                  (IE e Navigator) para Windows e Solaris somente. USAR PADR\u00c3O EM CASO DE D\u00daVIDA." + newline + newline +
      "       log:       Caminho e nome de arquivo para grava\u00e7\u00e3o do registro.  (Padr\u00e3o <userdir>/convert.log)" + newline + newline +
      "       progress:  Exibir progresso de sa\u00edda padr\u00e3o durante a convers\u00e3o. " + newline +
      "                  Padr\u00e3o: false" + newline + newline +
      "       simulate:  Exibir as especifica\u00e7\u00f5es da convers\u00e3o sem converter." + newline +
      "                  USAR ESTA OP\u00c7\u00c3O EM CASO DE D\u00daVIDA NA CONVERS\u00c3O." + newline +
      "                  SER\u00c1 OFERECIDA UMA LISTA DE DETALHES ESPEC\u00cdFICOS PARA A" + newline +
      "                  CONVERS\u00c3O." + newline + newline +
      "      Se somente \"java -jar htmlconverter.jar -gui\" estiver especificado" + newline +
      "      (somente a op\u00e7\u00e3o -gui sem filespecs), a vers\u00e3o da GUI do conversor ser\u00e1 iniciada." + newline  +
      "      Do contr\u00e1rio, a GUI ser\u00e1 omitida." + newline + newline +
      "      Para obter mais informa\u00e7\u00f5es, consulte a seguinte URL:" + newline + newline +
      "      http://java.sun.com/j2se/" +
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}
