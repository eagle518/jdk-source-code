/*
 * @(#)Converter_pt_BR.java	1.4 10/04/22
 *
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
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

public class Converter_pt_BR extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "Erro" },
	{ "caption.warning", "Aviso" },
	{ "caption.absdirnotfound", "Diret\u00f3rio absoluto n\u00e3o encontrado" },
	{ "caption.reldirnotfound", "Diret\u00f3rio relativo n\u00e3o encontrado" },
        { "about_dialog.info", "Conversor HTML do plug-in Java(TM) v{0}" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "Sobre o conversor HTML do plug-in Java(TM)" },
	{ "nottemplatefile_dialog.caption", "N\u00e3o \u00e9 um arquivo de modelo"},
	{ "nottemplatefile_dialog.info0", "O arquivo de modelo especificado " + newline +
                                          " {0} " + newline + 
					  "n\u00e3o \u00e9 um arquivo de modelo v\u00e1lido. O arquivo deve terminar" + newline +
					  "com a extens\u00e3o .tpl" + newline + newline +
                                          "Redefinindo o arquivo de modelo com o padr\u00e3o."},
	{ "warning_dialog.info", "A pasta de backup e a pasta de destino n\u00e3o podem " + newline +
	                         "ter o mesmo caminho. Gostaria que o caminho da" + newline +
	                         "pasta de backup fosse alterado para o seguinte: " + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "Arquivo de modelo n\u00e3o encontrado"},
        { "notemplate_dialog.info", "N\u00e3o foi poss\u00edvel encontrar o arquivo de modelo padr\u00e3o ({0})." + newline +
                                    "N\u00e3o est\u00e1 no classpath" + newline +
                                    "ou n\u00e3o est\u00e1 no diret\u00f3rio de trabalho."},
        { "file_unwritable.info", "O arquivo n\u00e3o pode ser gravado: "},
	{ "file_notexists.info", "O arquivo n\u00e3o existe: "},
	{ "illegal_source_and_backup.info", "Os diret\u00f3rios de destino e backup n\u00e3o podem ser os mesmos!"},
	{ "button.reset", "Restaurar padr\u00f5es"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "OK"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "button.cancel", "Cancelar"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.done", "Conclu\u00eddo"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "button.browse.dir", "Explorar..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_B)},
        { "button.browse.backup", "Explorar..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.convert", "Converter..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_C)},

	{ "advanced_dialog.caption", "Op\u00e7\u00f5es avan\u00e7adas"},
	{ "advanced_dialog.cab", "Especificar local de origem do arquivo ActiveX CAB:"},
	{ "advanced_dialog.plugin", "Especificar local de origem do plug-in Netscape:"},
	{ "advanced_dialog.smartupdate", "Especificar local de origem do Netscape SmartUpdate:"},
	{ "advanced_dialog.mimetype", "Especificar tipo MIME da convers\u00e3o HTML do plug-in Java:"},
	{ "advanced_dialog.log", "Especificar local do arquivo de registro:"},
	{ "advanced_dialog.generate", "Gerar arquivo de registro"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "Progresso..."},
	{ "progress_dialog.processing", "Processando..."},
	{ "progress_dialog.folder", "Pasta:"},
	{ "progress_dialog.file", "Arquivo:"},
	{ "progress_dialog.totalfile", "Total de arquivos processados:"},
	{ "progress_dialog.totalapplet", "Total de applets encontrados:"},
	{ "progress_dialog.totalerror", "Total de erros:"},

	{ "notdirectory_dialog.caption0", "N\u00e3o \u00e9 um arquivo v\u00e1lido"},
	{ "notdirectory_dialog.caption1", "N\u00e3o \u00e9 uma pasta v\u00e1lida"},
        { "notdirectory_dialog.info0", "A pasta seguinte n\u00e3o existe" + newline + "{0}"},
        { "notdirectory_dialog.info1", "O arquivo seguinte n\u00e3o existe" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "A pasta seguinte n\u00e3o existe " + newline + "<empty>"},
        
	{ "converter_gui.lablel0", "Especificar um arquivo ou um caminho de diret\u00f3rio:"},
	{ "converter_gui.lablel1", "Nomes de arquivos correspondentes:"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "Incluir subpastas"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "Um arquivo:"},
	{ "converter_gui.lablel5", "Backup de arquivos na pasta:"},
	{ "converter_gui.lablel7", "Arquivo de modelo:"},


	{ "template.default", "Padr\u00e3o (IE & Navigator) para Windows & Solaris somente"},
	{ "template.extend",  "Estendido (Padr\u00e3o + Todos os navegadores/plataformas)"},
	{ "template.ieonly",  "Internet Explorer para Windows & Solaris somente"},
	{ "template.nsonly",  "Navigator para Windows somente"},
	{ "template.other",   "Outro modelo..."},

        { "template_dialog.title", "Selecionar arquivo"},
	
        { "help_dialog.caption", "Ajuda"},
        { "help_dialog.error", "N\u00e3o foi poss\u00edvel acessar o arquivo de ajuda"},

	{ "menu.file", "Arquivo"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_F)},
	{ "menu.exit", "Sair"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_X)},
	{ "menu.edit", "Editar"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.option", "Op\u00e7\u00f5es"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "Ajuda"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "menu.about", "Sobre"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_A)},

        { "static.versioning.label", "Vers\u00e3o Java para applets:"},
        { "static.versioning.radio.button", "Usar somente JRE vers\u00e3o {0}"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "Os applets usar\u00e3o somente esta vers\u00e3o em particular do JRE.  Se n\u00e3o estiver instalada, esta vers\u00e3o ser\u00e1 baixada automaticamente, se poss\u00edvel.  Do contr\u00e1rio, o usu\u00e1rio ser\u00e1 enviado a uma p\u00e1gina de download manual.  Consulte http://java.sun.com/products/plugin para obter detalhes sobre o processo de download autom\u00e1tico e as pol\u00edticas de fim de vida (EOL) de todas as vers\u00f5es do Java."},
        { "dynamic.versioning.radio.button", "Usar qualquer JRE vers\u00e3o {0}, ou superior"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "Se tal vers\u00e3o n\u00e3o estiver instalada, o download padr\u00e3o atual da fam\u00edlia JRE vers\u00e3o {0} ser\u00e1 baixado automaticamente, se poss\u00edvel.  Do contr\u00e1rio, o usu\u00e1rio ser\u00e1 enviado a uma p\u00e1gina de download manual."},
        
	{ "progress_event.preparing", "Preparando"},
	{ "progress_event.converting", "Convertendo"},
	{ "progress_event.copying", "Copiando"},
	{ "progress_event.done", "Conclu\u00eddo"},
	{ "progress_event.destdirnotcreated", "N\u00e3o foi poss\u00edvel criar o diret\u00f3rio de destino."},
	{ "progress_event.error", "Erro"},
	
	{ "plugin_converter.logerror", "N\u00e3o foi poss\u00edvel estabelecer a sa\u00edda do arquivo de registro"},
	{ "plugin_converter.saveerror", "N\u00e3o foi poss\u00edvel salvar o arquivo de propriedades:  "},
	{ "plugin_converter.appletconv", "Convers\u00e3o do applet "},
	{ "plugin_converter.failure", "N\u00e3o \u00e9 poss\u00edvel converter o arquivo "},
	{ "plugin_converter.overwrite1", "J\u00e1 existe uma c\u00f3pia de backup de..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "Deseja substituir esta c\u00f3pia de backup?"},
	{ "plugin_converter.done", "Todos conclu\u00eddos  Arquivos processados:  "},
	{ "plugin_converter.appletfound", "  Applets encontrados:  "},
	{ "plugin_converter.processing", "  Processando..."},
	{ "plugin_converter.cancel", "Convers\u00e3o cancelada"},
	{ "plugin_converter.files", "Arquivos a serem convertidos: "},
	{ "plugin_converter.converted", "Arquivo convertido anteriormente, convers\u00e3o n\u00e3o necess\u00e1ria. "},
	{ "plugin_converter.donefound", "Conclu\u00eddo  Applets encontrados:  "},
	{ "plugin_converter.seetrace", "Erro no arquivo - consultar rastreamento abaixo"},
	{ "plugin_converter.noapplet", "Sem applets no arquivo "},
	{ "plugin_converter.nofiles", "Sem arquivos a serem processados "},
	{ "plugin_converter.nobackuppath", "Caminho de backup n\u00e3o criado"},
	{ "plugin_converter.writelog", "Grava\u00e7\u00e3o no arquivo de registro com o mesmo nome"},
	{ "plugin_converter.backup_path", "Caminho do backup"},
	{ "plugin_converter.log_path", "Caminho do registro"},
	{ "plugin_converter.template_file", "Arquivo de modelo"},
	{ "plugin_converter.process_subdirs", "Processar subdiret\u00f3rios"},
	{ "plugin_converter.show_progress", "Exibir progresso"},
	{ "plugin_converter.write_permission", "\u00c9 necess\u00e1rio ter permiss\u00e3o de grava\u00e7\u00e3o no diret\u00f3rio de trabalho atual"},
	{ "plugin_converter.overwrite", "O arquivo tempor\u00e1rio .tmpSource_stdin j\u00e1 existe. Exclua o arquivo ou renomeie-o."},
	{ "plugin_converter.help_message", newline + 
	                                  "Uso: HtmlConverter [-option1 value1 [-option2 value2 [...]]] [-simulate]  [filespecs]" + newline + newline +
	                                  "em que option inclui:" + newline + newline +
	                                  "    -source:    caminho para obter os arquivos originais.  Padr\u00e3o: <userdir>" + newline +
                                          "    -source -:  l\u00ea o arquivo convertido na entrada padr\u00e3o" + newline + 
	                                  "    -dest:      caminho para gravar arquivos convertidos.  Padr\u00e3o: <userdir>" + newline + 
                                          "    -dest -:    grava o arquivo convertido na sa\u00edda padr\u00e3o" + newline +
	                                  "    -backup:    caminho para gravar arquivos de backup.  Padr\u00e3o: <dirname>_BAK" + newline +
                                          "    -f:         for\u00e7a a substitui\u00e7\u00e3o dos arquivos de backup." + newline +
	                                  "    -subdirs:   os arquivos dos subdiret\u00f3rios devem ser processados." + newline +
	                                  "    -template:  caminho para o arquivo de modelo.  Usar padr\u00e3o em caso de d\u00favida." + newline +
	                                  "    -log:       caminho para gravar o registro.  Se n\u00e3o for fornecido, nenhum registro \u00e9 gravado." + newline +
	                                  "    -progress:  exibe o progresso da convers\u00e3o.  Padr\u00e3o: false" + newline +
	                                  "    -simulate:  exibe as especifica\u00e7\u00f5es para a convers\u00e3o sem converter." + newline + 
	                                  "    -latest:    usa o JRE mais recente que oferece suporte ao tipo MIME da vers\u00e3o." + newline + 
                                          "    -gui:       exibe a interface gr\u00e1fica de usu\u00e1rio do conversor." + newline + newline +
	                                  "    filespecs:  lista de espa\u00e7os delimitados das especifica\u00e7\u00f5es dos arquivos.  Padr\u00e3o: \"*.html *.htm\" (aspas necess\u00e1rias)" + newline},
	
	{ "product_name", "Conversor HTML do plug-in Java(TM)" },
    };
}

