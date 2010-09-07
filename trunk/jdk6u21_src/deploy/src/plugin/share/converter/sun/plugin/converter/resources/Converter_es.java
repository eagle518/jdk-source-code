/*
 * @(#)Converter_es.java	1.36 10/04/22
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

public class Converter_es extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "caption.error", "Error" },
	{ "caption.warning", "Aviso" },
	{ "caption.absdirnotfound", "Directorio absoluto no encontrado" },
	{ "caption.reldirnotfound", "Directorio relativo no encontrado" },
        { "about_dialog.info", "Java(TM) Plug-in HTML Converter v{0}" + newline + 
          "Copyright (c) COPYRIGHT_YEAR Oracle and/or it's affiliates." },
        { "about_dialog.caption", "Acerca del Convertidor HTML de Java(TM) Plug-in" },
	{ "nottemplatefile_dialog.caption", "No es un archivo de plantillas"},
	{ "nottemplatefile_dialog.info0", "El archivo de plantillas especificado " + newline +
                                          " {0} " + newline + 
					  "no es v\u00e1lido. El archivo debe terminar" + newline +
					  "con la extensi\u00f3n .tpl" + newline + newline +
                                          "Restablecer el archivo de plantillas a sus valores predeterminados."},
	{ "warning_dialog.info", "La carpeta de copia de seguridad y la carpeta de destino no pueden " + newline +
	                         "tener la misma ruta. \u00bfDesea sustituir la ruta de la carpeta de copia" + newline +
	                         "de seguridad por la siguiente ruta?: " + newline +
                                 "{0}_BAK"},
	{ "notemplate_dialog.caption", "Archivo de plantillas no encontrado"},
        { "notemplate_dialog.info", "No se ha encontrado el archivo de plantillas({0})" + newline +
                                    "predeterminado. O no est\u00e1 en la ruta de clase" + newline +
                                    "o no est\u00e1 en el directorio de trabajo."},
        { "file_unwritable.info", "No es posible escribir en el archivo: "},
	{ "file_notexists.info", "El archivo no existe: "},
	{ "illegal_source_and_backup.info", "Los directorios de destino y de copia de seguridad no pueden ser el mismo."},
	{ "button.reset", "Restablecer a valores predeterminados"},
        { "button.reset.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.okay", "Aceptar"},
        { "button.okay.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "button.cancel", "Cancelar"}, 
        { "button.cancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "button.done", "Realizado"},
        { "button.done.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.browse.dir", "Examinar..."},
        { "button.browse.dir.acceleratorKey", new Integer(KeyEvent.VK_X)},
        { "button.browse.backup", "Examinar..."},
        { "button.browse.backup.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "button.convert", "Convertir..."},
        { "button.convert.acceleratorKey", new Integer(KeyEvent.VK_N)},

	{ "advanced_dialog.caption", "Opciones avanzadas"},
	{ "advanced_dialog.cab", "Especificar ubicaci\u00f3n de origen para archivo CAB ActiveX:"},
	{ "advanced_dialog.plugin", "Especificar ubicaci\u00f3n de origen para Netscape Plug-in:"},
	{ "advanced_dialog.smartupdate", "Especificar ubicaci\u00f3n de origen para Netscape SmartUpdate:"},
	{ "advanced_dialog.mimetype", "Especifique el tipo MIME para la conversi\u00f3n de HTML de Java Plug-In:"},
	{ "advanced_dialog.log", "Especificar ubicaci\u00f3n para archivo de registro:"},
	{ "advanced_dialog.generate", "Generar archivo de registro"},
        { "advanced_dialog.generate.acceleratorKey", new Integer(KeyEvent.VK_G)},

	{ "progress_dialog.caption", "En curso..."},
	{ "progress_dialog.processing", "Procesando..."},
	{ "progress_dialog.folder", "Carpeta:"},
	{ "progress_dialog.file", "Archivo:"},
	{ "progress_dialog.totalfile", "Total archivos procesados:"},
	{ "progress_dialog.totalapplet", "Total subprogramas encontrados:"},
	{ "progress_dialog.totalerror", "Total errores:"},

	{ "notdirectory_dialog.caption0", "No es un archivo v\u00e1lido"},
	{ "notdirectory_dialog.caption1", "No es una carpeta v\u00e1lida"},
        { "notdirectory_dialog.info0", "La siguiente carpeta no existe" + newline + "{0}"},
        { "notdirectory_dialog.info1", "El siguiente archivo no existe" + newline + "{0}"},
	{ "notdirectory_dialog.info5", "La siguiente carpeta no existe" + newline + "<vac\u00edo>"},
        
	{ "converter_gui.lablel0", "Especifique un archivo o una ruta de acceso a un directorio:"},
	{ "converter_gui.lablel1", "Correspondencia de nombres de archivo:"},
	{ "converter_gui.lablel2", "*.html, *.htm, *.asp"},
	{ "converter_gui.lablel3", "Incluir subcarpetas"},
        { "converter_gui.lablel3.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "converter_gui.lablel4", "Un archivo:"},
	{ "converter_gui.lablel5", "Archivos de copia de seguridad en carpeta:"},
	{ "converter_gui.lablel7", "Archivo de plantillas:"},


	{ "template.default", "Est\u00e1ndar (IE y Navigator) s\u00f3lo para Windows y Solaris"},
	{ "template.extend",  "Ampliado (Est\u00e1ndar + Todos los navegadores/plataformas)"},
	{ "template.ieonly",  "Internet Explorer s\u00f3lo para Windows y Solaris"},
	{ "template.nsonly",  "Navigator s\u00f3lo para Windows"},
	{ "template.other",   "Otra plantilla..."},

        { "template_dialog.title", "Seleccionar archivo"},
	
        { "help_dialog.caption", "Ayuda"},
        { "help_dialog.error", "No se ha podido abrir el archivo de ayuda."},

	{ "menu.file", "Archivo"},
        { "menu.file.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "menu.exit", "Salir"},
        { "menu.exit.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "menu.edit", "Editar"},
        { "menu.edit.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "menu.option", "Opciones"},
        { "menu.option.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "menu.help", "Ayuda"},
        { "menu.help.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "menu.about", "Acerca de"},
        { "menu.about.acceleratorKey", new Integer(KeyEvent.VK_A)},

        { "static.versioning.label", "Control de versiones de Java para subprogramas:"},
        { "static.versioning.radio.button", "Utilice solamente de JRE {0}"},
        { "static.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_U)},
        { "static.versioning.text", "Los subprogramas s\u00f3lo utilizar\u00e1n esta versi\u00f3n espec\u00edfica de JRE.  Si no est\u00e1 instalada y es posible, esta versi\u00f3n se descargar\u00e1 autom\u00e1ticamente.  De lo contrario, el usuario ser\u00e1 reencaminado a una p\u00e1gina de descarga manual.  Consulte la direcci\u00f3n http://java.sun.com/products/plugin para obtener m\u00e1s informaci\u00f3n acerca del proceso de descarga autom\u00e1tica y las pol\u00edticas de final de la vida \u00fatil (EOL) para todas las versiones de Java."},
        { "dynamic.versioning.radio.button", "Utilice de JRE {0}, o superior"},
        { "dynamic.versioning.radio.button.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dynamic.versioning.text", "Si no est\u00e1 instalada esta versi\u00f3n y es posible, se descargar\u00e1 autom\u00e1ticamente la versi\u00f3n actual predeterminada de la familia JRE {0}. De lo contrario, el usuario ser\u00e1 reencaminado a una p\u00e1gina de descarga manual."},
        
	{ "progress_event.preparing", "Preparar"},
	{ "progress_event.converting", "Convertir"},
	{ "progress_event.copying", "Copiar"},
	{ "progress_event.done", "Realizado"},
	{ "progress_event.destdirnotcreated", "No se ha podido crear el directorio de destino."},
	{ "progress_event.error", "Error"},
	
	{ "plugin_converter.logerror", "La salida del archivo de registro no pudo establecerse"},
	{ "plugin_converter.saveerror", "No se pudo guardar el archivo de propiedades:  "},
	{ "plugin_converter.appletconv", "Conversi\u00f3n de subprograma"},
	{ "plugin_converter.failure", "Imposible convertir el archivo "},
	{ "plugin_converter.overwrite1", "Ya hay una copia de seguridad de..." + newline + newline },
	{ "plugin_converter.overwrite2", newline + newline + "\u00bfDesea sobrescribir dicha copia de seguridad?"},
	{ "plugin_converter.done", "Se han procesado todos los archivos realizados:  "},
	{ "plugin_converter.appletfound", "Subprogramas encontrados:  "},
	{ "plugin_converter.processing", "  Procesando..."},
	{ "plugin_converter.cancel", "Conversi\u00f3n cancelada"},
	{ "plugin_converter.files", "Archivos que se deben convertir: "},
	{ "plugin_converter.converted", "Archivo convertido previamente, no es necesaria la conversi\u00f3n. "},
	{ "plugin_converter.donefound", "Subprogramas realizados encontrados:  "},
	{ "plugin_converter.seetrace", "Error en archivo - v\u00e9ase rastreo a continuaci\u00f3n"},
	{ "plugin_converter.noapplet", "No hay subprogramas en este archivo"},
	{ "plugin_converter.nofiles", "No hay archivos que procesar "},
	{ "plugin_converter.nobackuppath", "No se pudo crear la ruta de la copia de seguridad"},
	{ "plugin_converter.writelog", "Sobrescribir el archivo de registro con el mismo nombre"},
	{ "plugin_converter.backup_path", "Ruta de copia de seguridad"},
	{ "plugin_converter.log_path", "Ruta de registro"},
	{ "plugin_converter.template_file", "Archivo de plantillas"},
	{ "plugin_converter.process_subdirs", "Procesar subdirectorios"},
	{ "plugin_converter.show_progress", "Mostrar progreso"},
	{ "plugin_converter.write_permission", "El directorio de trabajo actual requiere permiso de escritura"},
	{ "plugin_converter.overwrite", "El archivo temporal .tmpSource_stdin ya existe. B\u00f3rrelo o c\u00e1mbielo de nombre."},
	{ "plugin_converter.help_message", newline +
                                      "Sintaxis: HtmlConverter [-opci\u00f3n1 valor1 [-opci\u00f3n2 valor2 [...]]] [-simulate]  [filespecs]" + newline + newline +
                                      "donde las opciones son:" + newline + newline +
                                      "    -source:    Ruta de acceso de los archivos originales. Predeterminada: <dir_de_usuario>" + newline +
                                          "    -source -:  leer archivo de conversi\u00f3n desde el dispositivo de entrada est\u00e1ndar" + newline +
                                      "    -dest:      Ruta de acceso para escribir los archivos convertidos. Predeterminada: <dir_de_usuario>" + newline +
                                          "    -dest -:    escribir el archivo convertido en el dispositivo de salida est\u00e1ndar" + newline +
                                      "    -backup:    Ruta de escritura de los archivos de copia de seguridad. Predeterminada: <nombre_directorio>_BAK" + newline +
                                          "    -f:         Fuerza la sobrescritura de los archivos de copia de seguridad." + newline +
                                      "    -subdirs:   Si hay que procesar los archivos de los subdirectorios." + newline +
                                      "    -template:  Ruta al archivo de plantillas. Use la predeterminada en caso de duda." + newline +
                                      "    -log:       Ruta de escritura del registro. Si no se indica, no se crear\u00e1 ning\u00fan registro." + newline +
                                      "    -progress:  Muestra el progreso durante la conversi\u00f3n. Predeterminada: false" + newline +
                                      "    -simulate:  Muestra detalles de la conversi\u00f3n, sin realizarla." + newline +
	                                  "    -latest:  Utiliza el JRE m\u00e1s avanzado que soporta el tipo mime de la versi\u00f3n." + newline + 
                                      "    -gui:       Muestra la interfaz gr\u00e1fica de usuario del conversor." + newline + newline +
                                      "    filespecs:  Lista de especificaciones de los archivos delimitada por espacios. Predeterminada: \"*.html *.htm\" (se requieren comillas)" + newline},
	
	{ "product_name", "Convertidor HTML de Java(TM) Plug-in" },
    };
}

