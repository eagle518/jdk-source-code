/*
 * @(#)Activator_es.java	1.52 04/05/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
/**
 * Spanish version of Activator strings.
 *
 * @author Jerome Dochez
 */

public class Activator_es extends ListResourceBundle {

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    	{ "loading", "Cargando {0}" },
    	{ "java_applet", "Miniaplicaci\u00f3n de Java" },
        { "failed", "Ha fallado la carga de la miniaplicaci\u00f3n de Java..." },
        { "image_failed", "No se ha podido crear la imagen definida por el usuario.  Comprobar nombre del archivo de imagen." },
    	{ "java_not_enabled", "Java no habilitado" },
        { "exception", "Excepci\u00f3n: {0}" },

    	{ "bean_code_and_ser", "Bean no puede tener definido CODE y JAVA_OBJECT " },
    	{ "status_applet", "Miniaplicaci\u00f3n {0} {1}" },

    	// Resources associated with SecurityManager print Dialog:
	{ "print.caption", "Se necesita confirmaci\u00f3n - Imprimir" },
    	{ "print.message", new String[]{
		"<html><b>Solicitud de impresi\u00f3n</b></html>La miniaplicaci\u00f3n va a iniciar la impresi\u00f3n. \u00bfDesea proseguir?"}},
	{ "print.checkBox", "No mostrar de nuevo este cuadro de di\u00e1logo" },
	{ "print.buttonYes", "S\u00ed" },
	{ "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "print.buttonNo", "No" },
	{ "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},

	{ "optpkg.cert_expired", "<html><b>El certificado ha caducado</b></html>Se ha cancelado la instalaci\u00f3n del paquete opcional.\n" },
	{ "optpkg.cert_notyieldvalid", "<html><b>El certificado no es v\u00e1lido</b></html>Se ha cancelado la instalaci\u00f3n del paquete opcional.\n" },
	{ "optpkg.cert_notverify", "<html><b>El certificado no se ha verificado</b></html>Se ha cancelado la instalaci\u00f3n del paquete opcional.\n" },
	{ "optpkg.general_error", "<html><b>Excepci\u00f3n general</b></html>Se ha cancelado la instalaci\u00f3n del paquete opcional.\n" },
	{ "optpkg.caption", "Advertencia - Paquete opcional" },
	{ "optpkg.installer.launch.wait", "<html><b>Instalaci\u00f3n del paquete opcional</b></html>Haga clic en Aceptar para continuar la carga de la miniaplicaci\u00f3n cuando se haya instalado el instalador del paquete opcional.\n" },
	{ "optpkg.installer.launch.caption", "Instalaci\u00f3n en curso - Paquete opcional"},
	{ "optpkg.prompt_user.new_spec.text", "<html><b>Solicitud de descarga</b></html>La miniaplicaci\u00f3n necesita una versi\u00f3n m\u00e1s reciente (especificaci\u00f3n {0}) del paquete opcional \"{1}\" de {2}\n\nDesea proseguir?" },
	{ "optpkg.prompt_user.new_impl.text", "<html><b>Solicitud de descarga</b></html>La miniaplicaci\u00f3n necesita una versi\u00f3n m\u00e1s reciente (implementaci\u00f3n {0}) del paquete opcional \"{1}\" de {2}\n\n\u00bfDesea proseguir?" },
	{ "optpkg.prompt_user.new_vendor.text", "<html><b>Solicitud de descarga</b></html>La miniaplicaci\u00f3n necesita el ({0}) de paquete opcional \"{1}\" {2} de {3}\n\n\u00bfDesea proseguir?" },
	{ "optpkg.prompt_user.default.text", "<html><b>Solicitud de descarga</b></html>La miniaplicaci\u00f3n necesita que se instale el paquete opcional \"{0}\" de {1}\n\n\u00bfDesea proseguir?" },

	{ "cache.error.text", "<html><b>Error de antememoria</b></html>No se puede guardar ni actualizar archivos en la antememoria"},
	{ "cache.error.caption", "Error - Antememoria" },
	{ "cache.version_format_error", "{0} no est\u00e1 en el formato xxxx.xxxx.xxxx.xxxx, donde x es un d\u00edgito hexadecimal" },
    { "cache.version_attrib_error", "El n\u00famero de atributos especificados en \'cache_archive\' no coincide con los de \'cache_version\'" },
	{ "cache.header_fields_missing", "No se conoce la hora de modificaci\u00f3n ni/o el valor de caducidad.  El archivo JAR no se colocar\u00e1 en la antememoria."},

    { "applet.progress.load", "Cargando miniaplicaci\u00f3n..." },
    { "applet.progress.init", "Inicializando miniaplicaci\u00f3n..." },
    { "applet.progress.start", "Iniciando miniaplicaci\u00f3n..." },
    { "applet.progress.stop", "Deteniendo miniaplicaci\u00f3n..." },
    { "applet.progress.destroy", "Destruyendo miniaplicaci\u00f3n..." },
    { "applet.progress.dispose", "Desechando miniaplicaci\u00f3n..." },
    { "applet.progress.quit", "Saliendo de miniaplicaci\u00f3n..." },
    { "applet.progress.stoploading", "Se detuvo la carga..." },
    { "applet.progress.interrupted", "Se interrumpi\u00f3 el subproceso..." },
    { "applet.progress.joining", "Uniendo el subproceso de la miniaplicaci\u00f3n..." },
    { "applet.progress.joined", "Unido el subproceso de la miniaplicaci\u00f3n..." },
    { "applet.progress.loadImage", "Carga de imagen " },
    { "applet.progress.loadAudio", "Carga de audio " },
    { "applet.progress.findinfo.0", "Buscando informaci\u00f3n..." },
    { "applet.progress.findinfo.1", "Terminado..." },
    { "applet.progress.timeout.wait", "Tiempo de espera..." },
    { "applet.progress.timeout.jointing", "Uniendo..." },
    { "applet.progress.timeout.jointed", "Uni\u00f3n terminada..." },

    { "modality.register", "Modalidad de receptor registrada" },
    { "modality.unregister", "Modalidad de receptor no registrada" },
    { "modality.pushed", "Modalidad impuesta" },
    { "modality.popped", "Modalidad extra\u00edda" },

    { "progress.listener.added", "Receptor de progreso agregado: {0}" },
    { "progress.listener.removed", "Receptor de progreso suprimido: {0}" },

    { "liveconnect.UniversalBrowserRead.enabled", "JavaScript: UniversalBrowserRead habilitado" },
    { "liveconnect.java.system", "JavaScript: llamada de c\u00f3digo del sistema Java" },
    { "liveconnect.same.origin", "JavaScript: el llamante y el destinatario de la llamada tienen el mismo origen" },
    { "liveconnect.default.policy", "JavaScript: norma de seguridad predeterminada = {0}" },
    { "liveconnect.UniversalJavaPermission.enabled", "JavaScript: UniversalJavaPermission habilitado" },
    { "liveconnect.wrong.securitymodel", "Ya no se admite el modelo de seguridad de Netscape.\n"
                         + "En su lugar, migre al modelo de seguridad de Java 2.\n" },

    { "pluginclassloader.created_files", "Creado (0) en la antememoria." },
    { "pluginclassloader.deleting_files", "Supresi\u00f3n de archivos JAR de la antememoria." },
    { "pluginclassloader.file", "   supresi\u00f3n desde la antememoria {0}" },
    { "pluginclassloader.empty_file", "{0} est\u00e1 vac\u00edo, suprimir de la antememoria." },

    { "appletcontext.audio.loaded", "Clip de audio cargado: {0}" },
    { "appletcontext.image.loaded", "Imagen cargada:{0} " },

    { "securitymgr.automation.printing", "Automatizaci\u00f3n: Aceptar impresi\u00f3n" },

    { "classloaderinfo.referencing", "Referencia a cargador de clases: {0}, contador de referencia={1}" },
    { "classloaderinfo.releasing", "Liberaci\u00f3n de cargador de clases: {0}, contador de referencia={1}" },
    { "classloaderinfo.caching", "Colocaci\u00f3n del cargador de clases en la antememoria: {0}" },
    { "classloaderinfo.cachesize", "Tama\u00f1o de la antememoria del cargador de clases actual: {0}" },
    { "classloaderinfo.num", "El n\u00famero de cargadores de clases colocados en la antememoria es superior a {0}, sin referencia {1}" },

	{ "jsobject.eval", "JSObject::eval({0})" },
	{ "jsobject.call", "JSObject::llamar: nombre={0}" },
	{ "jsobject.getMember", "JSObject::getMember: nombre={0}" },
	{ "jsobject.setMember", "JSObject::setMember: nombre={0}" },
	{ "jsobject.removeMember", "JSObject::removeMember: nombre={0}" },
	{ "jsobject.getSlot", "JSObject::getSlot: {0}" },
	{ "jsobject.setSlot", "JSObject::setSlot: ranura={0}" },
	{ "jsobject.invoke.url.permission", "el url de la miniaplicaci\u00f3n es {0} y el permiso es = {1}"},

    { "optpkg.install.info", "Instalaci\u00f3n de paquete opcional {0}" },
    { "optpkg.install.fail", "Instalaci\u00f3n del paquete opcional no satisfactoria." },
    { "optpkg.install.ok", "Instalaci\u00f3n del paquete opcional realizada." },
    { "optpkg.install.automation", "Automatizaci\u00f3n: Aceptar instalaci\u00f3n del paquete opcional" },
    { "optpkg.install.granted", "El usuario ha concedido la descarga del paquete opcional, descargar desde {0}" },
    { "optpkg.install.deny", "El usuario no ha concedido la descarga del paquete opcional" },
    { "optpkg.install.begin", "Instalaci\u00f3n {0}" },
    { "optpkg.install.java.launch", "Iniciar el instalador de Java" },
    { "optpkg.install.java.launch.command", "Iniciar el instalador de Java a trav\u00e9s de {0}" },
    { "optpkg.install.native.launch", "Iniciar instalador nativo" },
    { "optpkg.install.native.launch.fail.0", "Imposible ejecutar {0}" },
    { "optpkg.install.native.launch.fail.1", "Acceso a {0} no satisfactorio" },
    { "optpkg.install.raw.launch", "Instalaci\u00f3n del paquete opcional b\u00e1sico" },
    { "optpkg.install.raw.copy", "Copiar el paquete opcional b\u00e1sico desde {0} en {1}" },
    { "optpkg.install.error.nomethod", "No se ha instalado el proveedor de ampliaci\u00f3n dependiente : no se puede obtener el "
                         + " m\u00e9todo addExtensionInstallationProvider" },
    { "optpkg.install.error.noclass", "No se ha instalado el proveedor de ampliaci\u00f3n dependiente : no se puede obtener la "
                     + "clase sun.misc.ExtensionDependency" },

    {"progress_dialog.downloading", "M\u00f3dulo: descargando..."},
    {"progress_dialog.dismiss_button", "Descartar"},
    {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
    {"progress_dialog.from", "desde"},

    {"applet_viewer.color_tag", "N\u00famero de componentes incorrecto en {0}"},

    {"progress_info.downloading", "Descarga de archivos JAR"},
    {"progress_bar.preload", "Carga previa de archivos JAR: {0}"},

    {"cache.size", "Tama\u00f1o de la antememoria: {0}"},
    {"cache.cleanup", "El tama\u00f1o de la antememoria es: {0} bytes, es necesario limpiarla"},
    {"cache.full", "La antememoria est\u00e1 completa: supresi\u00f3n del archivo {0}"},
    {"cache.inuse", "No se puede suprimir el archivo {0} porque se est\u00e1 utilizando en esta aplicaci\u00f3n"},
    {"cache.notdeleted", "No se puede suprimir el archivo {0}, puede que se utilice en esta y/o otras aplicaciones"},
    {"cache.out_of_date", "La copia de {0} guardada en la antememoria est\u00e1 caducada\n  Copia guardada en la antememoria: {1}\n  Copia del servidor: {2}"},
    {"cache.loading", "Carga de {0} desde la antememoria"},
    {"cache.cache_warning", "ADVERTENCIA: no se puede guardar {0} en la antememoria"},
    {"cache.downloading", "Descarga de {0} a la antememoria"},
    {"cache.cached_name", "Nombre de archivo guardado en la antememoria: {0}"},
    {"cache.load_warning", "ADVERTENCIA: error al leer {0} de la antememoria."},
    {"cache.disabled", "Antememoria inhabilitada por usuario"},
    {"cache.minSize", "Antememoria inhabilitada, el l\u00edmite de la antememoria es {0}, debe especificarse al menos 5 MB "},
    {"cache.directory_warning", "ADVERTENCIA: {0} no es un directorio. Se inhabilitar\u00e1 la antememoria."},
    {"cache.response_warning", "ADVERTENCIA: Respuesta inesperada {0} para {1}.  El archivo se descargar\u00e1 de nuevo."},
    {"cache.enabled", "Antememoria habilitada"},
    {"cache.location", "Ubicaci\u00f3n: {0}"},
    {"cache.maxSize", "Tama\u00f1o m\u00e1ximo: {0}"},
    {"cache.create_warning", "ADVERTENCIA: no ha sido posible crear el directorio {0} de la antememoria. se inhabilitar\u00e1 la colocaci\u00f3n en la antememoria."},
    {"cache.read_warning", "ADVERTENCIA: no se puede leer el directorio {0} de la antememoria. se inhabilitar\u00e1 la colocaci\u00f3n en la antememoria."},
    {"cache.write_warning", "ADVERTENCIA: no se puede escribir en el directorio {0} de la antememoria. Se inhabilitar\u00e1 la colocaci\u00f3n en la antememoria."},
    {"cache.compression", "Nivel de compresi\u00f3n: {0}"},
    {"cache.cert_load", "Los certificados para {0} se leen desde la antememoria JAR"},
    {"cache.jarjar.invalid_file", "el archivo .jarjar contiene un archivo distinto del .jar"},
    {"cache.jarjar.multiple_jar", "el archivo .jarjar contiene m\u00e1s de un archivo .jar"},
    {"cache.version_checking", "Comprobaci\u00f3n de la versi\u00f3n de {0}, la versi\u00f3n especificada es {1}"},
    { "cache.preloading", "Carga previa del archivo {0}"},

    { "cache_viewer.caption", "Visualizador de la antememoria de miniaplicaciones de Java" },
    { "cache_viewer.refresh", "Renovar" },
    { "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_R) },
    { "cache_viewer.remove", "Borrar" },
    { "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_B) },
    { "cache_viewer.OK", "Aceptar" },
    { "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_A) },
    { "cache_viewer.name", "Nombre" },
    { "cache_viewer.type", "Tipo" },
    { "cache_viewer.size", "Tama\u00f1o" },
    { "cache_viewer.modify_date", "\u00daltima modificaci\u00f3n" },
    { "cache_viewer.expiry_date", "Fecha de caducidad" },
    { "cache_viewer.url", "URL" },
    { "cache_viewer.version", "Versi\u00f3n" },
    { "cache_viewer.help.name", "Nombre de archivo guardado en la antememoria" },
    { "cache_viewer.help.type", "Tipo de archivo guardado en la antememoria" },
    { "cache_viewer.help.size", "Tama\u00f1o de archivo guardado en la antememoria" },
    { "cache_viewer.help.modify_date", "Fecha de la \u00faltima modificaci\u00f3n de archivo" },
    { "cache_viewer.help.expiry_date", "Fecha de caducidad de archivo" },
    { "cache_viewer.help.url", "Descarga de URL de archivo" },
    { "cache_viewer.help.version", "Versiones de antememoria de archivo" },
    { "cache_viewer.delete.text", "<html><b>El archivo se ha borrado</b></html>{0} tal vez est\u00e9 en uso.\n" },
    { "cache_viewer.delete.caption", "Error - Antememoria" },
    { "cache_viewer.type.zip", "Jar" },
    { "cache_viewer.type.class", "Clase" },
    { "cache_viewer.type.wav", "Sonido Wav" },
    { "cache_viewer.type.au", "Sonido Au" },
    { "cache_viewer.type.gif", "Imagen Gif" },
    { "cache_viewer.type.jpg", "Imagen Jpeg" },
        { "cache_viewer.menu.file", "Archivo" },
        { "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_C) },
        { "cache_viewer.menu.options", "Opciones" },
        { "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_P) },
        { "cache_viewer.menu.help", "Ayuda" },
        { "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_Y) },
        { "cache_viewer.menu.item.exit", "Salir" },
        { "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_S) },
        { "cache_viewer.disable", "Activar almacenamiento en la antememoria" },
        { "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_N) },
        { "cache_viewer.menu.item.about", "Acerca de" },
        { "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_R) },

	{ "net.proxy.auto.result.error", "Imposible determinar valor de proxy desde evaluaci\u00f3n - retroceder a DIRECTA"},

	{ "com.method.ambiguous", "Imposible seleccionar un m\u00e9todo, par\u00e1metros ambiguos." },
	{ "com.method.notexists", "{0} :no existe ese m\u00e9todo" },
	{ "com.notexists", "{0} :no existe ese m\u00e9todo/esa propiedad" },
	{ "com.method.invoke", "Invocando m\u00e9todo: {0}" },
	{ "com.method.jsinvoke", "Invocando m\u00e9todo JS: {0}" },
	{ "com.method.argsTypeInvalid", "Los par\u00e1metros no pueden convertirse en los tipos requeridos" },
	{ "com.method.argCountInvalid", "El n\u00famero de argumentos no es correcto" },
	{ "com.field.needsConversion", "Se requiere conversi\u00f3n: {0} --> {1}" },
	{ "com.field.typeInvalid", " no puede convertirse al tipo: {0}" },
	{ "com.field.get", "Obteniendo propiedad: {0}" },
	{ "com.field.set", "Definiendo propiedad: {0}" },

    { "lifecycle.applet.found", "Se ha encontrado la miniaplicaci\u00f3n previa detenida desde la antememoria del ciclo de vida \u00fatil" },
    { "lifecycle.applet.support", "La miniaplicaci\u00f3n admite el modelo de ciclo de vida \u00fatil heredado - a\u00f1adir miniaplicaci\u00f3n a la antememoria de ciclo de vida \u00fatil" },
    { "lifecycle.applet.cachefull", "La antememoria de ciclo de vida \u00fatil est\u00e1 completa - reducir las miniaplicaciones de uso menos reciente" },

	{ "rsa.cert_expired", "<html><b>El certificado ha caducado</b></html>El c\u00f3digo se tratar\u00e1 como no firmado.\n" },
	{ "rsa.cert_notyieldvalid", "<html><b>El certificado no es v\u00e1lido</b></html>El c\u00f3digo se tratar\u00e1 como no firmado.\n" },
	{ "rsa.general_error", "<html><b>El certificado no se ha verificado</b></html>El c\u00f3digo se tratar\u00e1 como no firmado.\n" },

	{ "dialogfactory.menu.show_console", "Mostrar consola de Java" },
	{ "dialogfactory.menu.hide_console", "Ocultar consola de Java" },
	{ "dialogfactory.menu.about", "Acerca de Java Plugin" },
	{ "dialogfactory.menu.copy", "Copiar" },
	{ "dialogfactory.menu.open_console", "Abrir consola de Java" },
	{ "dialogfactory.menu.about_java", "Acerca de Java(TM)" },


   };
}


