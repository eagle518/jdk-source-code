/*
 * @(#)Deployment_es.java	1.29 04/07/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;


/**
 * Spanish version of Deployment strings.
 *
 * @author Stanley Man-Kit Ho
 */

public final class Deployment_es extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
        { "product.javapi.name", "Java Plug-in {0}" },
        { "product.javaws.name", "Java Web Start {0}" },

    	{ "console.version", "Versi\u00f3n" },
    	{ "console.default_vm_version", "Versi\u00f3n de la m\u00e1quina virtual predeterminada" },
    	{ "console.using_jre_version", "Usar versi\u00f3n JRE" },
    	{ "console.user_home", "Directorio local del usuario" },
	{ "console.caption", "Consola de Java" },
	{ "console.clear", "Borrar" },
	{ "console.clear.acceleratorKey", new Integer(KeyEvent.VK_B)},
	{ "console.close", "Cerrar" },
	{ "console.close.acceleratorKey", new Integer(KeyEvent.VK_C) },
	{ "console.copy", "Copiar" },
	{ "console.copy.acceleratorKey", new Integer(KeyEvent.VK_O) },
	{ "console.menu.text.top", "----------------------------------------------------\n" },
	{ "console.menu.text.c", "c:   borrar ventana de consola\n" },
	{ "console.menu.text.f", "f:   finalizar objetos en la cola de finalizaci\u00f3n\n" },
	{ "console.menu.text.g", "g:   liberaci\u00f3n de recursos\n" },
	{ "console.menu.text.h", "h:   presentar este mensaje de ayuda\n" },
	{ "console.menu.text.j", "j:   volcar datos jcov\n"},
	{ "console.menu.text.l", "l:   volcar lista del cargador de clases\n" },
	{ "console.menu.text.m", "m:   imprimir sintaxis de memoria\n" },
	{ "console.menu.text.o", "o:   activar registro\n" },
	{ "console.menu.text.p", "p:   recargar configuraci\u00f3n de proxy\n" },
	{ "console.menu.text.q", "q:   ocultar consola\n" },
	{ "console.menu.text.r", "r:   recargar configuraci\u00f3n de norma\n" },
	{ "console.menu.text.s", "s:   volcar propiedades del sistema y de despliegue\n" },
	{ "console.menu.text.t", "t:   volcar lista de subprocesos\n" },
	{ "console.menu.text.v", "v:   volcar pila de subprocesos\n" },
	{ "console.menu.text.x", "x:   borrar antememoria del cargador de clases\n" },
	{ "console.menu.text.0", "0-5: establecer nivel de rastreo en <n>\n" },
	{ "console.menu.text.tail", "----------------------------------------------------\n" },
	{ "console.done", "Terminado." },
	{ "console.trace.level.0", "Nivel de rastreo establecido en 0: ninguno ... terminado." },
	{ "console.trace.level.1", "Nivel de rastreo establecido en 1: b\u00e1sico ... terminado." },
	{ "console.trace.level.2", "Nivel de rastreo establecido en 2: b\u00e1sico, red ... terminado." },
	{ "console.trace.level.3", "Nivel de rastreo establecido en 3: b\u00e1sico, red, seguridad ... terminado." },
	{ "console.trace.level.4", "Nivel de rastreo establecido en 4: b\u00e1sico, red, seguridad, ext ... terminado." },
	{ "console.trace.level.5", "Nivel de rastreo establecido en 5: todos... terminado." },
	{ "console.log", "Registro establecido en : " },
	{ "console.completed", " ... terminado." },
	{ "console.dump.thread", "Volcar lista de subprocesos ...\n" },
	{ "console.dump.stack", "Volcar pila de subprocesos ...\n" },
	{ "console.dump.system.properties", "Volcar propiedades del sistema ...\n" },
        { "console.dump.deployment.properties", "Volcar propiedades de despliegue...\n" },
	{ "console.clear.classloader", "Borrar antememoria del cargador de clases .... terminado." },
	{ "console.reload.policy", "Recargar configuraci\u00f3n de norma" },
	{ "console.reload.proxy", "Recargar configuraci\u00f3n de proxy ..." },
	{ "console.gc", "Liberaci\u00f3n de recursos" },
	{ "console.finalize", "Finalizar objetos en la cola de finalizaci\u00f3n" },
	{ "console.memory", "Memoria: {0}K  Libre: {1}K  ({2}%)" },
	{ "console.jcov.error", "Error de tiempo de ejecuci\u00f3n de Jcov: compruebe si ha especificado la opci\u00f3n jcov adecuada\n"},
	{ "console.jcov.info", "Datos Jcov volcados con \u00e9xito\n"},

	{ "https.dialog.caption", "Advertencia - HTTPS" },
	{ "https.dialog.text", "<html><b>Discordancia de nombres de sistema</b></html>El nombre de sistema en el certificado de seguridad del servidor no coincide con el nombre del servidor."
			     + "\n\nNombre de sistema del URL: {0}"
			     + "\nNombre de sistema en el certificado: {1}"
			     + "\n\n\u00bfDesea proseguir?" },
	{ "https.dialog.unknown.host", "Sistema desconocido" },

	{ "security.dialog.caption", "Advertencia - Seguridad" },
	{ "security.dialog.text0", "\u00bfDesea instalar y ejecutar el {0} firmado distribuido por \"{1}\"?"
				 + "\n\nAutenticidad del editor verificada por: \"{2}\"" },
        { "security.dialog.text0a", "\u00bfDesea instalar y ejecutar el {0} firmado distribuido por \"{1}\"?"
                                 + "\n\nNo puede verificarse la autenticidad del editor." },
  	{ "security.dialog.timestamp.text1", "The {0} was signed on {1}." },
	{ "security.dialog_https.text0", "\u00bfDesea aceptar este certificado del sitio web \"{0}\" para el intercambio de informaci\u00f3n cifrada?"
				 + "\n\nAutenticidad del editor verificada por: \"{1}\"" },
        { "security.dialog_https.text0a", "\u00bfDesea aceptar este certificado del sitio web \"{0}\" para el intercambio de informaci\u00f3n cifrada?"
                                 + "\n\nNo se puede verificar la autenticidad del editor." },
	{ "security.dialog.text1", "\nPrecauci\u00f3n: \"{0}\" afirma que este contenido es seguro. S\u00f3lo debe aceptar el contenido si conf\u00eda en \"{1}\"." },
	{ "security.dialog.unknown.issuer", "Emisor desconocido" },
	{ "security.dialog.unknown.subject", "Asunto desconocido" },
	{ "security.dialog.certShowName", "{0} ({1})" },
	{ "security.dialog.rootCANotValid", "El certificado de seguridad lo emiti\u00f3 una compa\u00f1\u00eda que no es de confianza." },
	{ "security.dialog.rootCAValid", "El certificado de seguridad lo emiti\u00f3 una compa\u00f1\u00eda de confianza." },
	{ "security.dialog.timeNotValid", "El certificado de seguridad ha caducado o a\u00fan no es v\u00e1lido." },
	{ "security.dialog.timeValid", "El certificado de seguridad no ha caducado o a\u00fan es v\u00e1lido." },
	{ "security.dialog.timeValidTS", "The security certificate was valid when the {0} was signed." },
	{ "security.dialog.buttonAlways", "Siempre" },
        { "security.dialog.buttonAlways.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "security.dialog.buttonYes", "S\u00ed" },
	{ "security.dialog.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "security.dialog.buttonNo", "No" },
	{ "security.dialog.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "security.dialog.buttonViewCert", "M\u00e1s detalles" },
        { "security.dialog.buttonViewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

        { "security.badcert.caption", "Advertencia - Seguridad" },
        { "security.badcert.https.text", "No se puede validar el certificado SSL.\n{0} no se ejecutar\u00e1." },
        { "security.badcert.config.text", "La configuraci\u00f3n de seguridad no permitir\u00e1 validar este certificado. {0} no se ejecutar\u00e1." },
        { "security.badcert.text", "Ha fallado la validaci\u00f3n del certificado. {0} no se ejecutar\u00e1." },
        { "security.badcert.viewException", "Ver excepci\u00f3n" },
        { "security.badcert.viewException.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "security.badcert.viewCert", "M\u00e1s detalles" },
        { "security.badcert.viewCert.acceleratorKey", new Integer(KeyEvent.VK_M)},

	{ "cert.dialog.caption", "Detalles - Certificado" },
	{ "cert.dialog.certpath", "Ruta del certificado" },
	{ "cert.dialog.field.Version", "Versi\u00f3n" },
	{ "cert.dialog.field.SerialNumber", "N\u00famero de serie" },
	{ "cert.dialog.field.SignatureAlg", "Algoritmo de firma" },
	{ "cert.dialog.field.Issuer", "Emisor" },
	{ "cert.dialog.field.EffectiveDate", "Fecha efectiva" },
	{ "cert.dialog.field.ExpirationDate", "Fecha de caducidad" },
	{ "cert.dialog.field.Validity", "Validez" },
	{ "cert.dialog.field.Subject", "Asunto" },
	{ "cert.dialog.field.Signature", "Firma" },
	{ "cert.dialog.field", "Campo" },
	{ "cert.dialog.value", "Valor" },
        { "cert.dialog.close", "Cerrar" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_C) },

	{ "clientauth.password.dialog.buttonOK", "Aceptar" },
	{ "clientauth.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "clientauth.password.dialog.buttonCancel", "Cancelar" },
	{ "clientauth.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.password.dialog.caption", "Se necesita una contrase\u00f1a - Archivo de claves de autenticaci\u00f3n de clientes" },
	{ "clientauth.password.dialog.text", "Especifique una contrase\u00f1a para acceder al archivo de claves de autenticaci\u00f3n de clientes:\n" },
        { "clientauth.password.dialog.error.caption", "Error - Archivo de claves de autenticaci\u00f3n de clientes" },
        { "clientauth.password.dialog.error.text", "<html><b>Error de acceso al archivo de claves</b></html>Se ha manipulado el archivo de claves o la contrase\u00f1a era incorrecta." },

	{ "clientauth.certlist.dialog.buttonOK", "Aceptar" },
	{ "clientauth.certlist.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "clientauth.certlist.dialog.buttonCancel", "Cancelar" },
	{ "clientauth.certlist.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "clientauth.certlist.dialog.buttonDetails", "Detalles" },
	{ "clientauth.certlist.dialog.buttonDetails.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "clientauth.certlist.dialog.caption", "Autenticaci\u00f3n de cliente" },
	{ "clientauth.certlist.dialog.text", "Es preciso identificarse para conectarse a ese sitio web. \nSeleccione el certificado que utilizar\u00e1 al contectarse.\n" },

	{ "dialogfactory.confirmDialogTitle", "Se necesita confirmaci\u00f3n - Java" },
	{ "dialogfactory.inputDialogTitle", "Se necesita informaci\u00f3n - Java" },
	{ "dialogfactory.messageDialogTitle", "Mensaje - Java" },
	{ "dialogfactory.exceptionDialogTitle", "Error - Java" },
	{ "dialogfactory.optionDialogTitle", "Opci\u00f3n - Java" },
	{ "dialogfactory.aboutDialogTitle", "Acerca de - Java" },
	{ "dialogfactory.confirm.yes", "S\u00ed" },
        { "dialogfactory.confirm.yes.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "dialogfactory.confirm.no", "No" },
        { "dialogfactory.confirm.no.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "dialogfactory.moreInfo", "M\u00e1s detalles" },
        { "dialogfactory.moreInfo.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "dialogfactory.lessInfo", "Menos detalles" },
        { "dialogfactory.lessInfo.acceleratorKey", new Integer(KeyEvent.VK_L)},
	{ "dialogfactory.java.home.link", "http://www.java.com" },
	{ "dialogfactory.general_error", "<html><b>Excepci\u00f3n general</b></html>" },
	{ "dialogfactory.net_error", "<html><b>Excepci\u00f3n de conexi\u00f3n en red</b></html>" },
	{ "dialogfactory.security_error", "<html><b>Excepci\u00f3n de seguridad</b></html>" },
	{ "dialogfactory.ext_error", "<html><b>Excepci\u00f3n de paquete opcional</b></html>" },
	{ "dialogfactory.user.selected", "Usuario seleccionado: {0}" },
	{ "dialogfactory.user.typed", "Usuario escrito: {0}" },

	{ "deploycertstore.cert.loading", "Carga de certificados de despliegue desde {0}" },
	{ "deploycertstore.cert.loaded", "Certificados de despliegue cargados desde {0}" },
	{ "deploycertstore.cert.saving", "Guardar certificados de despliegue en {0}" },
	{ "deploycertstore.cert.saved", "Certificados de despliegue guardados en {0}" },
	{ "deploycertstore.cert.adding", "Adici\u00f3n de certificado al almac\u00e9n permanente de certificados de despliegue" },
	{ "deploycertstore.cert.added", "Certificado agregado como alias {0} al almac\u00e9n permanente de certificados de despliegue" },
	{ "deploycertstore.cert.removing", "Supresi\u00f3n de certificado del almac\u00e9n permanente de certificados de despliegue" },
	{ "deploycertstore.cert.removed", "Certificado suprimido como alias {0} del almac\u00e9n permanente de certificados de despliegue" },
	{ "deploycertstore.cert.instore", "Comprobar si el certificado est\u00e1 en el almac\u00e9n permanente de certificados de despliegue" },
	{ "deploycertstore.cert.canverify", "Comprobar si se puede verificar el certificado por medio del almac\u00e9n permanente de certificados de despliegue" },
	{ "deploycertstore.cert.iterator", "Obtener el iterador de certificados del almac\u00e9n permanente de certificados de despliegue" },
	{ "deploycertstore.cert.getkeystore", "Obtener el objeto de archivo de claves del almac\u00e9n permanente de certificados de despliegue" },

	{ "httpscertstore.cert.loading", "Carga de certificados SSL para despliegue desde {0}" },
	{ "httpscertstore.cert.loaded", "Certificados SSL para despliegue cargados desde {0}" },
	{ "httpscertstore.cert.saving", "Guardar certificados SSL para despliegue en {0}" },
	{ "httpscertstore.cert.saved", "Certificados SSL para despliegue guardados en {0}" },
	{ "httpscertstore.cert.adding", "Adici\u00f3n de certificado SSL al almac\u00e9n permanente de certificados de despliegue" },
	{ "httpscertstore.cert.added", "Certificado SSL agregado como alias {0} del almac\u00e9n permanente de certificados para despliegue" },
	{ "httpscertstore.cert.removing", "Supresi\u00f3n de certificado SSL del almac\u00e9n permanente de certificados de despliegue" },
	{ "httpscertstore.cert.removed", "Certificado SSL suprimido como alias {0} del almac\u00e9n permanente de certificados de despliegue" },
	{ "httpscertstore.cert.instore", "Comprobar si el certificado SSL est\u00e1 en el almac\u00e9n permanente de certificados de despliegue" },
	{ "httpscertstore.cert.canverify", "Comprobar si se puede verificar el certificado SSL por medio del almac\u00e9n permanente de certificados de despliegue" },
	{ "httpscertstore.cert.iterator", "Obtener iterador de certificados SSL en el almac\u00e9n permanente de certificados de despliegue" },
	{ "httpscertstore.cert.getkeystore", "Obtener objeto del archivo de claves del almac\u00e9n permanente de certificados de despliegue" },

	{ "rootcertstore.cert.loading", "Carga de certificados CA ra\u00edz desde {0}" },
	{ "rootcertstore.cert.loaded", "Certificados CA ra\u00edz cargados desde {0}" },
	{ "rootcertstore.cert.noload", "No se ha encontrado el archivo de los certificados CA ra\u00edz: {0}" },
	{ "rootcertstore.cert.saving", "Guardar certificados CA ra\u00edz en {0}" },
	{ "rootcertstore.cert.saved", "Certificados CA ra\u00edz guardados en {0}" },
	{ "rootcertstore.cert.adding", "Adici\u00f3n de certificado en el almac\u00e9n de certificados CA ra\u00edz" },
	{ "rootcertstore.cert.added", "Certificado agregado como alias {0} en el almac\u00e9n de certificados CA ra\u00edz" },
	{ "rootcertstore.cert.removing", "Supresi\u00f3n de certificado del almac\u00e9n de certificados CA ra\u00edz" },
	{ "rootcertstore.cert.removed", "Certificado suprimido como alias {0} en el almac\u00e9n de certificados CA ra\u00edz" },
	{ "rootcertstore.cert.instore", "Comprobar si el certificado est\u00e1 en el almac\u00e9n de certificados CA ra\u00edz" },
	{ "rootcertstore.cert.canverify", "Comprobar si se puede verificar el certificado por medio de los certificados del almac\u00e9n de certificados CA ra\u00edz" },
	{ "rootcertstore.cert.iterator", "Obtener iterador de certificados en el almac\u00e9n de certificados CA ra\u00edz" },
	{ "rootcertstore.cert.getkeystore", "Obtener objeto de archivo de claves del almac\u00e9n de certificados CA ra\u00edz" },
	{ "rootcertstore.cert.verify.ok", "El certificado se ha verificado correctamente con certificados CA ra\u00edz" },
	{ "rootcertstore.cert.verify.fail", "El certificado no se ha verificado satisfactoriamente con los certificados CA ra\u00edz" },
	{ "rootcertstore.cert.tobeverified", "Certificado que debe verificarse:\n{0}" },
	{ "rootcertstore.cert.tobecompared", "Comparando certificado con el certificado CA ra\u00edz:\n{0}" },

	{ "roothttpscertstore.cert.loading", "Carga de certificados SSL CA ra\u00edz desde {0}" },
	{ "roothttpscertstore.cert.loaded", "Certificados SSL CA ra\u00edz cargados desde {0}" },
	{ "roothttpscertstore.cert.noload", "No se ha encontrado el archivo de los certificados SSL CA ra\u00edz: {0}" },
	{ "roothttpscertstore.cert.saving", "Guardar certificados SSL CA ra\u00edz en {0}" },
	{ "roothttpscertstore.cert.saved", "Certificados SSL CA ra\u00edz guardados en {0}" },
	{ "roothttpscertstore.cert.adding", "Adici\u00f3n de certificado en el almac\u00e9n de certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.added", "Certificado agregado como alias {0} en el almac\u00e9n de certificados SSL CA ra\u00edz " },
	{ "roothttpscertstore.cert.removing", "Supresi\u00f3n de certificado del almac\u00e9n de certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.removed", "Certificado suprimido como alias {0} en el almac\u00e9n de certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.instore", "Comprobar si el certificado est\u00e1 en el almac\u00e9n de certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.canverify", "Comprobar si se puede verificar el certificado por medio de los certificados del almac\u00e9n de certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.iterator", "Obtener iterador de certificados en el almac\u00e9n de certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.getkeystore", "Obtener objeto de archivo de claves del almac\u00e9n de certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.verify.ok", "El certificado se ha verificado correctamente con certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.verify.fail", "El certificado no se ha verificado satisfactoriamente con los certificados SSL CA ra\u00edz" },
	{ "roothttpscertstore.cert.tobeverified", "Certificado que debe verificarse:\n{0}" },
	{ "roothttpscertstore.cert.tobecompared", "Comparando certificado con el certificado SSL CA ra\u00edz:\n{0}" },

        { "sessioncertstore.cert.loading", "Carga de certificados del almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.loaded", "Certificados cargados del almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.saving", "Guardar certificados en el almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.saved", "Certificados guardados en el almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.adding", "Adici\u00f3n de certificados en el almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.added", "Certificados agregados en el almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.removing", "Supresi\u00f3n de certificado en el almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.removed", "Certificado suprimido en el almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.instore", "Comprobar si el certificado est\u00e1 en el almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.canverify", "Comprobar si se puede verificar el certificado por medio de los certificados del almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.iterator", "Obtener iterador de certificados en el almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },
        { "sessioncertstore.cert.getkeystore", "Obtener objeto de archivo de claves del almac\u00e9n de certificados de la sesi\u00f3n de despliegue" },

        { "iexplorer.cert.loading", "Cargando certificados del almac\u00e9n de certificados {0} de Internet Explorer" },
        { "iexplorer.cert.loaded", "Certificados cargados desde el almac\u00e9n de certificados {0} de Internet Explorer" },
        { "iexplorer.cert.instore", "Comprobando si el certificado est\u00e1 en el almac\u00e9n de certificados {0} de Internet Explorer" },
        { "iexplorer.cert.canverify", "Comprobar si el certificado puede verificarse mediante los certificados del almac\u00e9n de certificados {0} de Internet Explorer" },
        { "iexplorer.cert.iterator", "Obtener el iterador de certificados del almac\u00e9n de certificados {0} de Internet Explorer" },
        { "iexplorer.cert.verify.ok", "El certificado se ha verificado correctamente mediante los certificados {0} de Internet Explorer" },
        { "iexplorer.cert.verify.fail", "No se ha podido verificar el certificado mediante los certificados {0} de Internet Explorer" },
        { "iexplorer.cert.tobeverified", "Certificado para verificar:\n{0}" },
        { "iexplorer.cert.tobecompared", "Comparando el certificado con el certificado {0} de Internet Explorer:\n{1}" },
        { "mozilla.cert.loading", "Cargando los certificados del almac\u00e9n de certificados {0} de Mozilla" },
        { "mozilla.cert.loaded", "Certificados cargados del almac\u00e9n de certificados {0} de Mozilla" },
        { "mozilla.cert.instore", "Comprobando si el certificado se encuentra en el almac\u00e9n de certificados {0} de Mozilla" },
        { "mozilla.cert.canverify", "Comprobar si el certificado puede verificarse mediante el almac\u00e9n de certificados {0} de Mozilla" },
        { "mozilla.cert.iterator", "Obtener el iterador de certificados en el almac\u00e9n de certificados {0} de Mozilla" },
        { "mozilla.cert.verify.ok", "El certificado se ha verificado correctamente con los certificados {0} de Mozilla" },
        { "mozilla.cert.verify.fail", "No se ha podido verificar el certificado con los certificados {0} de Mozilla" },
        { "mozilla.cert.tobeverified", "Certificado para Verificar:\n{0}" },
        { "mozilla.cert.tobecompared", "Comparando el certificado con el certificado {0} de Mozilla:\n{1}" },

        { "browserkeystore.jss.no", "No se encuentra el paquete de JSS" },
        { "browserkeystore.jss.yes", "El paquete de JSS est\u00e1 cargado" },
        { "browserkeystore.jss.notconfig", "JSS no est\u00e1 configurado" },
        { "browserkeystore.jss.config", "JSS est\u00e1 configurado" },
        { "browserkeystore.mozilla.dir", "Accediendo a las claves y el certificado del perfil de usuario de Mozilla: {0}" },
        { "browserkeystore.password.dialog.buttonOK", "Aceptar" },
        { "browserkeystore.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_A)},
        { "browserkeystore.password.dialog.buttonCancel", "Cancelar" },
        { "browserkeystore.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "browserkeystore.password.dialog.caption", "Se necesita una contrase\u00f1a" },
        { "browserkeystore.password.dialog.text", "Introduzca la contrase\u00f1a para {0}:\n" },
        { "mozillamykeystore.priv.notfound", "no se ha encontrado la clave privada del certificado: {0}" },

	{ "hostnameverifier.automation.ignoremismatch", "Automatizaci\u00f3n: Ignorar discrepancia de nombres del sistema" },

	{ "trustdecider.check.basicconstraints", "La comprobaci\u00f3n de restricciones b\u00e1sicas ha fallado en el certificado" },
	{ "trustdecider.check.leafkeyusage", "La comprobaci\u00f3n del uso de la clave de hoja ha fallado en el certificado" },
	{ "trustdecider.check.signerkeyusage", "La comprobaci\u00f3n del uso de la clave del firmante ha fallado en el certificado" },
	{ "trustdecider.check.extensions", "La comprobaci\u00f3n de extensiones cr\u00edticas ha fallado en el certificado" },
	{ "trustdecider.check.signature", "La comprobaci\u00f3n de firma ha fallado en el certificado" },
	{ "trustdecider.check.basicconstraints.certtypebit", "La comprobaci\u00f3n de bits de tipo netscape ha fallado en el certificado" },
	{ "trustdecider.check.basicconstraints.extensionvalue", "La comprobaci\u00f3n del valor de la extensi\u00f3n de netscape ha fallado en el certificado" },
	{ "trustdecider.check.basicconstraints.bitvalue", "La comprobaci\u00f3n del valor de los bits 5,6,7 de netscape ha fallado en el certificado" },
	{ "trustdecider.check.basicconstraints.enduser", "La comprobaci\u00f3n del usuario final como CA ha fallado en el certificado" },
	{ "trustdecider.check.basicconstraints.pathlength", "La comprobaci\u00f3n de restricciones de longitud de ruta ha fallado en el certificado" },
	{ "trustdecider.check.leafkeyusage.length", "La comprobaci\u00f3n del uso de la longitud de clave ha fallado en el certificado" },
	{ "trustdecider.check.leafkeyusage.digitalsignature", "La comprobaci\u00f3n de la firma digital ha fallado en el certificado" },
	{ "trustdecider.check.leafkeyusage.extkeyusageinfo", "La comprobaci\u00f3n de informaci\u00f3n sobre el uso de la clave de extensi\u00f3n ha fallado en el certificado" },
        { "trustdecider.check.leafkeyusage.tsaextkeyusageinfo", "La comprobaci\u00f3n de informaci\u00f3n sobre el uso de la clave de la extensi\u00f3n de TSA ha fallado en el certificado" },
	{ "trustdecider.check.leafkeyusage.certtypebit", "La comprobaci\u00f3n de bits de tipo netscape ha fallado en el certificado" },
	{ "trustdecider.check.signerkeyusage.lengthandbit", "La comprobaci\u00f3n de longitud y bit ha fallado en el certificado" },
	{ "trustdecider.check.signerkeyusage.keyusage", "La comprobaci\u00f3n del uso de claves ha fallado en el certificado" },
	{ "trustdecider.check.canonicalize.updatecert", "Actualizar el certificado ra\u00edz con el certificado del archivo cacerts" },
	{ "trustdecider.check.canonicalize.missing", "Agregar el certificado ra\u00edz que falta" },
	{ "trustdecider.check.gettrustedcert.find", "Localizar el certificado CA ra\u00edz v\u00e1lido en el archivo cacerts" },
	{ "trustdecider.check.gettrustedissuercert.find", "Localizar en el archivo cacerts el certificado CA ra\u00edz v\u00e1lido que falta" },
        { "trustdecider.check.timestamping.no", "No hay informaci\u00f3n de fechado digital disponible" },
        { "trustdecider.check.timestamping.yes", "Hay informaci\u00f3n de fechado digital disponible" },
        { "trustdecider.check.timestamping.tsapath", "Empezar a comprobar la ruta de acceso a los certificados de TSA" },
        { "trustdecider.check.timestamping.inca", "Aunque el certificado ha caducado, su fecha digital es de un periodo v\u00e1lido y tiene una TSA v\u00e1lida" },
        { "trustdecider.check.timestamping.notinca", "El certificado ha caducado, pero la TSA no es v\u00e1lida" },
        { "trustdecider.check.timestamping.valid", "El certificado ha caducado y su fecha digital es de un periodo v\u00e1lido" },
        { "trustdecider.check.timestamping.invalid", "El certificado ha caducado y su fecha digital es de un periodo no v\u00e1lido" },
        { "trustdecider.check.timestamping.need", "El certificado ha caducado. Es preciso comprobar la informaci\u00f3n sobre fechado digital" },
        { "trustdecider.check.timestamping.noneed", "El certificado no ha caducado. No es preciso comprobar la informaci\u00f3n sobre el fechado digital" },
        { "trustdecider.check.timestamping.notfound", "No se encuentra la nueva API de gesti\u00f3n de fechado digital" },
	{ "trustdecider.user.grant.session", "El usuario ha concedido privilegios de c\u00f3digo s\u00f3lo para esta sesi\u00f3n" },
	{ "trustdecider.user.grant.forever", "El usuario ha concedido privilegios permanentes de c\u00f3digo" },
	{ "trustdecider.user.deny", "El usuario ha denegado los privilegios de c\u00f3digo" },
	{ "trustdecider.automation.trustcert", "Automatizaci\u00f3n: Confiar en el certificado RSA para la firma" },
	{ "trustdecider.code.type.applet", "miniaplicaci\u00f3n" },
	{ "trustdecider.code.type.application", "aplicaci\u00f3n" },
	{ "trustdecider.code.type.extension", "extensi\u00f3n" },
	{ "trustdecider.code.type.installer", "instalador" },
	{ "trustdecider.user.cannot.grant.any", "La configuraci\u00f3n de seguridad no permitir\u00e1 dar permisos a nuevos certificados" },
	{ "trustdecider.user.cannot.grant.notinca", "La configuraci\u00f3n de seguridad no permitir\u00e1 dar permisos a certificados generados por el propio usuario" },
	{ "x509trustmgr.automation.ignoreclientcert", "Automatizaci\u00f3n: Ignorar certificado de cliente que no sea de confianza" },
	{ "x509trustmgr.automation.ignoreservercert", "Automatizaci\u00f3n: Ignorar certificado de servidor que no sea de confianza" },

	{ "net.proxy.text", "Proxy: " },
	{ "net.proxy.override.text", "Alteraciones del proxy: " },
	{ "net.proxy.configuration.text", "Configuraci\u00f3n del proxy: " },
	{ "net.proxy.type.browser", "Configuraci\u00f3n del proxy del navegador" },
	{ "net.proxy.type.auto", "Configuraci\u00f3n autom\u00e1tica del proxy" },
	{ "net.proxy.type.manual", "Configuraci\u00f3n manual" },
	{ "net.proxy.type.none", "Sin proxy" },
	{ "net.proxy.type.user", "El usuario ha alterado los valores de proxy del navegador." },
	{ "net.proxy.loading.ie", "Cargando configuraci\u00f3n del proxy desde Internet Explorer ..."},
	{ "net.proxy.loading.ns", "Cargando configuraci\u00f3n del proxy desde Netscape Navigator ..."},
	{ "net.proxy.loading.userdef", "Cargando configuraci\u00f3n del proxy definida por el usuario ..."},
	{ "net.proxy.loading.direct", "Cargando configuraci\u00f3n directa de proxy ..."},
	{ "net.proxy.loading.manual", "Cargando configuraci\u00f3n manual de proxy ..."},
	{ "net.proxy.loading.auto",   "Cargando configuraci\u00f3n autom\u00e1tica de proxy ..."},
	{ "net.proxy.loading.browser",   "Cargando configuraci\u00f3n de proxy del navegador ..."},
	{ "net.proxy.loading.manual.error", "Imposible utilizar la configuraci\u00f3n manual de proxy - retroceder a DIRECTA"},
	{ "net.proxy.loading.auto.error", "Imposible utilizar la configuraci\u00f3n autom\u00e1tica de proxy - retroceder a MANUAL"},
	{ "net.proxy.loading.done", "Terminado."},
	{ "net.proxy.browser.pref.read", "Leer archivo de preferencia de usuario en {0}"},
	{ "net.proxy.browser.proxyEnable", "    Habilitar proxy: {0}"},
	{ "net.proxy.browser.proxyList",     "    Lista de proxy: {0}"},
	{ "net.proxy.browser.proxyOverride", "    Anular proxy: {0}"},
	{ "net.proxy.browser.autoConfigURL", "    Configuraci\u00f3n de URL autom\u00e1tica: {0}"},
	{ "net.proxy.browser.smartConfig", "Hacer un ping al servidor del proxy {0} en el puerto {1}"},
        { "net.proxy.browser.connectionException", "No se puede acceder al servidor del proxy {0} desde el puerto {1}"},
	{ "net.proxy.ns6.regs.exception", "Error al leer el archivo de registro: {0}"},
	{ "net.proxy.pattern.convert", "Convertir lista de proxies omitidos en expresi\u00f3n regular: "},
	{ "net.proxy.pattern.convert.error", "Imposible convertir lista de proxies omitidos en expresi\u00f3n regular - ignorar"},
	{ "net.proxy.auto.download.ins", "Descargar archivo INS desde {0}" },
	{ "net.proxy.auto.download.js", "Descargar archivo proxy autom\u00e1ticamente desde {0}" },
	{ "net.proxy.auto.result.error", "Imposible determinar valor de proxy desde evaluaci\u00f3n - retroceder a DIRECTA"},
        { "net.proxy.service.not_available", "Proxy no disponible para {0} - el proxy predeterminado es DIRECT" },
	{ "net.proxy.error.caption", "Error - Configuraci\u00f3n del proxy" },
	{ "net.proxy.nsprefs.error", "<html><b>No se pueden recuperar los valores del proxy</b></html>Retroceder a otra configuraci\u00f3n del proxy.\n" },
	{ "net.proxy.connect", "Conectando {0} con proxy={1}" },

	{ "net.authenticate.caption", "Se necesita contrase\u00f1a - Conexi\u00f3n en red"},
	{ "net.authenticate.label", "<html><b>Introduzca el nombre de usuario y la contrase\u00f1a:</b></html>"},
	{ "net.authenticate.resource", "Recurso:" },
	{ "net.authenticate.username", "Nombre de usuario:" },
        { "net.authenticate.username.mnemonic", "VK_U" },
	{ "net.authenticate.password", "Contrase\u00f1a:" },
        { "net.authenticate.password.mnemonic", "VK_P" },
	{ "net.authenticate.firewall", "Servidor:" },
	{ "net.authenticate.domain", "Dominio:"},
        { "net.authenticate.domain.mnemonic", "VK_D" },
	{ "net.authenticate.realm", "\u00c1mbito:" },
	{ "net.authenticate.scheme", "Esquema:" },
	{ "net.authenticate.unknownSite", "Sitio desconocido" },

	{ "net.cookie.cache", "Antememoria de cookie: " },
	{ "net.cookie.server", "Servidor {0} solicita configuraci\u00f3n de cookie con \"{1}\"" },
	{ "net.cookie.connect", "Conectar {0} con cookie \"{1}\"" },
	{ "net.cookie.ignore.setcookie", "Cookie no est\u00e1 disponible - no tener en cuenta \"Definir Cookie\"" },
	{ "net.cookie.noservice", "Cookie no est\u00e1 disponible - utilice la antememoria para determinar \"Cookie\"" },

	{"about.java.version", "Versi\u00f3n {0} (build {1})"},
	{"about.prompt.info", "Para obtener m\u00e1s informaci\u00f3n sobre Java y examinar algunas buenas aplicaciones de Java, visite"},
	{"about.home.link", "http://www.java.com"},
	{"about.option.close", "Cerrar"},
	{"about.option.close.acceleratorKey", new Integer(KeyEvent.VK_C)},
        {"about.copyright", "Copyright 2004 Sun Microsystems, Inc."},
	{"about.legal.note", "Reservados todos los derechos. Uso sujeto a los t\u00e9rminos de la licencia."},


	{ "cert.remove_button", "Suprimir" },
        { "cert.remove_button.mnemonic", "VK_R" },
        { "cert.import_button", "Importar" },
        { "cert.import_button.mnemonic", "VK_I" },
        { "cert.export_button", "Exportar" },
        { "cert.export_button.mnemonic", "VK_E" },
        { "cert.details_button", "Detalles" },
        { "cert.details_button.mnemonic", "VK_D" },
        { "cert.viewcert_button", "Ver certificado" },
        { "cert.viewcert_button.mnemonic", "VK_V" },
        { "cert.close_button", "Cerrar" },
        { "cert.close_button.mnemonic", "VK_C" },
        { "cert.type.trusted_certs", "Certificados de confianza" },
        { "cert.type.secure_site", "Sitio seguro" },
        { "cert.type.client_auth", "Autenticaci\u00f3n de cliente" },
        { "cert.type.signer_ca", "CA de firmante" },
        { "cert.type.secure_site_ca", "CA de sitio seguro" },
        { "cert.rbutton.user", "Usuario" },
        { "cert.rbutton.system", "Sistema" },
        { "cert.settings", "Certificados" },
        { "cert.dialog.import.error.caption", "Error - Importaci\u00f3n de certificado" },
        { "cert.dialog.export.error.caption", "Error - Exportaci\u00f3n de certificado" },
        { "cert.dialog.import.format.text", "<html><b>Formato de archivo desconocido</b></html>No se importar\u00e1 ning\u00fan certificado." },
        { "cert.dialog.export.password.text", "<html><b>Contrase\u00f1a no v\u00e1lida</b></html>La contrase\u00f1a especificada no es correcta." },
        { "cert.dialog.import.file.text", "<html><b>El archivo no existe</b></html>No se importar\u00e1 ning\u00fan certificado." },
        { "cert.dialog.import.password.text", "<html><b>Contrase\u00f1a no v\u00e1lida</b></html>La contrase\u00f1a especificada no es correcta." },
        { "cert.dialog.password.caption", "Contrase\u00f1a" },
        { "cert.dialog.password.import.caption", "Se necesita una contrase\u00f1a - Importar" },
        { "cert.dialog.password.export.caption", "Se necesita una contrase\u00f1a - Exportar" },
        { "cert.dialog.password.text", "Introduzca una contrase\u00f1a para acceder a este archivo:\n" },
        { "cert.dialog.exportpassword.text", "Introduzca una contrase\u00f1a para acceder a esta clave privada del archivo de claves de autenticaci\u00f3n de clientes:\n" },
        { "cert.dialog.savepassword.text", "Introduzca una contrase\u00f1a para guardar el archivo de claves:\n" },
        { "cert.dialog.password.okButton", "Aceptar" },
        { "cert.dialog.password.cancelButton", "Cancelar" },
        { "cert.dialog.export.error.caption", "Error - Exportaci\u00f3n de certificado" },
        { "cert.dialog.export.text", "<html><b>Imposible exportar</b></html>No se ha exportado ning\u00fan certificado." },
        { "cert.dialog.remove.text", "Confirme la supresi\u00f3n de certificado(s)" },
	{ "cert.dialog.remove.caption", "Suprimir certificado" },
	{ "cert.dialog.issued.to", "Emitido para" },
	{ "cert.dialog.issued.by", "Emitido por" },
	{ "cert.dialog.user.level", "Usuario" },
	{ "cert.dialog.system.level", "Sistema" },
	{ "cert.dialog.certtype", "Tipo de certificado: "},

	{ "controlpanel.jre.platformTableColumnTitle","Plataforma"},
	{ "controlpanel.jre.productTableColumnTitle","Producto" },
	{ "controlpanel.jre.locationTableColumnTitle","Ubicaci\u00f3n" },
	{ "controlpanel.jre.pathTableColumnTitle","Ruta" },
	{ "controlpanel.jre.enabledTableColumnTitle", "Activado" },

	{ "jnlp.jre.title", "Configuraci\u00f3n del entorno de ejecuci\u00f3n de JNLP" },
	{ "jnlp.jre.versions", "Versiones del entorno de ejecuci\u00f3n de Java" },
	{ "jnlp.jre.choose.button", "Elegir" },
	{ "jnlp.jre.find.button", "Buscar" },
	{ "jnlp.jre.add.button", "Agregar" },
	{ "jnlp.jre.remove.button", "Suprimir" },
	{ "jnlp.jre.ok.button", "Aceptar" },
	{ "jnlp.jre.cancel.button", "Cancelar" },
	{ "jnlp.jre.choose.button.mnemonic", "VK_E" },
	{ "jnlp.jre.find.button.mnemonic", "VK_B" },
	{ "jnlp.jre.add.button.mnemonic", "VK_G" },
	{ "jnlp.jre.remove.button.mnemonic", "VK_R" },
	{ "jnlp.jre.ok.button.mnemonic", "VK_A" },
	{ "jnlp.jre.cancel.button.mnemonic", "VK_C" },

	{ "find.dialog.title", "Buscador de JRE"},
	{ "find.title", "Entornos de ejecuci\u00f3n de Java"},
	{ "find.cancelButton", "Cancelar"},
	{ "find.prevButton", "Anterior"},
	{ "find.nextButton", "Siguiente"},
	{ "find.cancelButtonMnemonic", "VK_C"},
	{ "find.prevButtonMnemonic", "VK_A"},
	{ "find.nextButtonMnemonic", "VK_S"},
	{ "find.intro", "Para iniciar las aplicaciones, Java Web Start necesita conocer la ubicaci\u00f3n de los entornos de ejecuci\u00f3n de Java (JRE) instalados.\n\nPuede seleccionar un JRE conocido o elegir un directorio del sistema de archivos para buscarlo." },

	{ "find.searching.title", "Buscando los JRE disponibles; la b\u00fasqueda puede tardar varios minutos." },
	{ "find.searching.prefix", "comprobaci\u00f3n: " },
	{ "find.foundJREs.title", "Se han encontrado los siguientes JRE, haga clic en Siguiente para agregarlos" },
	{ "find.noJREs.title", "No se ha encontrado ning\u00fan JRE, haga clic en Anterior para seleccionar otro directorio de b\u00fasqueda" },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Propiedades de despliegue de Java(tm)\n"
                        + "# NO MODIFICAR ESTE ARCHIVO. Se genera autom\u00e1ticamente.\n"
                        + "# Usar el Panel de control de Java para modificar propiedades." },
        { "config.unknownSubject", "Asunto desconocido" },
        { "config.unknownIssuer", "Emisor desconocido" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>URL con formato no v\u00e1lido</b></html>El URL de configuraci\u00f3n autom\u00e1tica de proxy no es v\u00e1lido." },
        { "config.proxy.autourl.invalid.caption", "Error - Proxies" },
	// Java Web Start Properties
	 { "APIImpl.clipboard.message.read", "Esta aplicaci\u00f3n ha solicitado acceso de s\u00f3lo lectura al portapapeles del sistema. Puede que la aplicaci\u00f3n tenga acceso a informaci\u00f3n confidencial almacenada en el portapapeles. \u00bfDesea permitir esta acci\u00f3n?" },
        { "APIImpl.clipboard.message.write", "Esta aplicaci\u00f3n ha solicitado acceso de escritura al portapapeles del sistema. Puede que la aplicaci\u00f3n sobrescriba informaci\u00f3n almacenada en el portapapeles. \u00bfDesea permitir esta acci\u00f3n?" },
        { "APIImpl.file.open.message", "Esta aplicaci\u00f3n ha solicitado acceso de lectura al sistema de archivos. Puede que la aplicaci\u00f3n obtenga acceso a la informaci\u00f3n confidencial almacenada en el sistema de archivos. \u00bfDesea permitir esta acci\u00f3n?" },
        { "APIImpl.file.save.fileExist", "{0} ya existe.\n \u00bfDesea reemplazarlo?" },
        { "APIImpl.file.save.fileExistTitle", "El archivo ya existe" },
        { "APIImpl.file.save.message", "Esta aplicaci\u00f3n ha solicitado acceso de lectura/escritura a un archivo del sistema de archivos local. Si autoriza esta acci\u00f3n, la aplicaci\u00f3n s\u00f3lo obtendr\u00e1 acceso a los archivos seleccionados en el cuadro de di\u00e1logo siguiente. \u00bfDesea permitir esta acci\u00f3n?" },
        { "APIImpl.persistence.accessdenied", "Denegado el acceso al espacio de almacenamiento permanente para el URL {0}" },
        { "APIImpl.persistence.filesizemessage", "Se ha superado la longitud m\u00e1xima del archivo" },
        { "APIImpl.persistence.message", "Esta aplicaci\u00f3n ha solicitado espacio de almacenamiento adicional en el disco local. Actualmente, el almacenamiento m\u00e1ximo asignado es de {1} bytes. La aplicaci\u00f3n solicita incrementarlo hasta {0} bytes. \u00bfDesea permitir esta acci\u00f3n?" },
        { "APIImpl.print.message", "Esta aplicaci\u00f3n ha solicitado acceso a la impresora predeterminada. Si autoriza esta acci\u00f3n conceder\u00e1 a la aplicaci\u00f3n acceso de escritura a la impresora. \u00bfDesea permitir esta acci\u00f3n?" },
	{ "APIImpl.extended.fileOpen.message1", "Esta aplicaci\u00f3n ha solicitado permiso de lectura/escritura a los siguientes archivos del sistema de archivos local:"},
	{ "APIImpl.extended.fileOpen.message2", "Si autoriza esta acci\u00f3n conceder\u00e1 a la aplicaci\u00f3n acceso a los archivos de la lista anterior. \u00bfDesea permitir esta acci\u00f3n?"},
        { "APIImpl.securityDialog.no", "No" },
        { "APIImpl.securityDialog.remember", "No volver a mostrar esta asesor\u00eda" },
        { "APIImpl.securityDialog.title", "Asesor\u00eda de seguridad" },
        { "APIImpl.securityDialog.yes", "S\u00ed" },
        { "Launch.error.installfailed", "Instalaci\u00f3n no satisfactoria" },
        { "aboutBox.closeButton", "Cerrar" },
        { "aboutBox.versionLabel", "Versi\u00f3n {0} (build {1})" },
        { "applet.failedToStart", "No se ha podido iniciar la miniaplicaci\u00f3n: {0}" },
        { "applet.initializing", "Inicializando miniaplicaci\u00f3n" },
        { "applet.initializingFailed", "No se ha podido inicializar la miniaplicaci\u00f3n: {0}" },
        { "applet.running", "Ejecutando..." },
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "Debe reiniciar Windows para que se apliquen los cambios.\n\n\u00bfDesea reiniciar Windows ahora?" },
        { "extensionInstall.rebootTitle", "Reiniciar Windows" },
        { "install.configButton", "Configurar ..." },
        { "install.configMnemonic", "VK_C" },
        { "install.errorInstalling", "Error no esperado al tratar de instalar el entorno de ejecuci\u00f3n de Java, vuelva a intentarlo." },
        { "install.errorRestarting", "Error no esperado al iniciar, vuelva a intentarlo." },
        { "install.title", "{0} - Creaci\u00f3n de accesos directos" },
        { "install.windows.both.message", "\u00bfDesea crear los accesos directos de {0} en el escritorio y el\nmen\u00fa de inicio?" },
        { "install.gnome.both.message", "\u00bfDesea crear los accesos directos de {0} en el escritorio y el men\u00fa de\naplicaciones?" },
        { "install.desktop.message", "\u00bfDesea crear los accesos directos de {0}\nen el escritorio?" },
        { "install.windows.menu.message", "\u00bfDesea crear los accesos directos de {0}\nen el men\u00fa de inicio?" },
        { "install.gnome.menu.message", "\u00bfDesea crear los accesos directos de {0}\nen el men\u00fa de aplicaciones?" },
        { "install.noButton", "No" },
        { "install.noMnemonic", "VK_N" },
        { "install.yesButton", "S\u00ed" },
        { "install.yesMnemonic", "VK_S" },
        { "launch.cancel", "Cancelar" },
        { "launch.downloadingJRE", "Solicitando JRE {0} de {1}" },
        { "launch.error.badfield", "El campo {0} contiene un valor no v\u00e1lido: {1}" },
        { "launch.error.badfield-signedjnlp", "El campo {0} contiene un valor no v\u00e1lido en el archivo de ejecuci\u00f3n firmado: {1}" },
        { "launch.error.badfield.download.https", "No se puede realizar la descarga a trav\u00e9s de HTTPS" },
        { "launch.error.badfield.https", "HTTPS necesita Java 1.4 o una versi\u00f3n superior" },
        { "launch.error.badjarfile", "Archivo JAR deteriorado en {0}" },
        { "launch.error.badjnlversion", "Versi\u00f3n de JNLP no admitida en el archivo de ejecuci\u00f3n: {0}. Esta versi\u00f3n s\u00f3lo admite las versiones 1.0 y 1.5. P\u00f3ngase en contacto con el proveedor de la aplicaci\u00f3n para informarle del problema." },
        { "launch.error.badmimetyperesponse", "El servidor ha devuelto un tipo MIME incorrecto al acceder al recurso: {0} - {1}" },
        { "launch.error.badsignedjnlp", "No se ha podido validar la firma del archivo de ejecuci\u00f3n. La versi\u00f3n firmada no coincide con la versi\u00f3n descargada." },
        { "launch.error.badversionresponse", "La respuesta del servidor contiene una versi\u00f3n incorrecta al acceder al recurso: {0} - {1}" },
        { "launch.error.canceledloadingresource", "El usuario ha cancelado la carga del recurso {0}" },
        { "launch.error.category.arguments", "Error de argumento no v\u00e1lido" },
        { "launch.error.category.download", "Error de descarga" },
        { "launch.error.category.launchdesc", "Error de ejecuci\u00f3n de archivo" },
        { "launch.error.category.memory", "Error OutOfMemory" },
        { "launch.error.category.security", "Error de seguridad" },
        { "launch.error.category.config", "Configuraci\u00f3n del sistema" },
        { "launch.error.category.unexpected", "Error no esperado" },
        { "launch.error.couldnotloadarg", "No se ha podido cargar el URL ni el archivo especificado: {0}" },
        { "launch.error.errorcoderesponse-known", "El servidor ha devuelto el c\u00f3digo de error {1} ({2}) al acceder al recurso: {0}" },
        { "launch.error.errorcoderesponse-unknown", "El servidor ha devuelto el c\u00f3digo de error 99 (error desconocido) al acceder al recurso: {0}" },
        { "launch.error.failedexec", "No se ha podido ejecutar el entorno de ejecuci\u00f3n de Java versi\u00f3n {0}" },
        { "launch.error.failedloadingresource", "No se puede cargar el recurso: {0}" },
        { "launch.error.invalidjardiff", "No se puede aplicar la actualizaci\u00f3n incremental para el recurso: {0}" },
        { "launch.error.jarsigning-badsigning", "No se ha podido verificar la firma del recurso: {0}" },
        { "launch.error.jarsigning-missingentry", "Falta una entrada firmada en el recurso: {0}" },
        { "launch.error.jarsigning-missingentryname", "Entrada firmada que falta: {0}" },
        { "launch.error.jarsigning-multicerts", "Se ha usado m\u00e1s de un certificado para firmar el recurso: {0}" },
        { "launch.error.jarsigning-multisigners", "Hay m\u00e1s de una firma en una entrada del recurso: {0}" },
        { "launch.error.jarsigning-unsignedfile", "Se ha encontrado una entrada sin firma en el recurso: {0}" },
        { "launch.error.missingfield", "Falta el siguiente campo requerido del archivo de ejecuci\u00f3n: {0}" },
        { "launch.error.missingfield-signedjnlp", "Falta el siguiente campo requerido del archivo de ejecuci\u00f3n firmado: {0}" },
        { "launch.error.missingjreversion", "No se ha encontrado ninguna versi\u00f3n de JRE en el archivo de ejecuci\u00f3n para este sistema" },
        { "launch.error.missingversionresponse", "La respuesta del servidor no contiene ning\u00fan campo de versi\u00f3n al acceder al recurso: {0}" },
        { "launch.error.multiplehostsreferences", "Los recursos hacen referencia a varios sistemas" },
        { "launch.error.nativelibviolation", "El uso de bibliotecas nativas requiere acceso sin restricciones al sistema" },
        { "launch.error.noJre", "La aplicaci\u00f3n ha solicitado una versi\u00f3n de la plataforma JRE que no est\u00e1 instalada en el sistema local actualmente. Java Web Start no ha podido descargar e instalar autom\u00e1ticamente la versi\u00f3n solicitada. La versi\u00f3n de JRE debe instalarse manualmente.\n\n" },
        { "launch.error.wont.download.jre", "La aplicaci\u00f3n ha solicitado una versi\u00f3n de la plataforma JRE (versi\u00f3n {0}) que no est\u00e1 instalada en el sistema local actualmente. Java Web Start no est\u00e1 autorizada a descargar e instalar autom\u00e1ticamente la versi\u00f3n solicitada. La versi\u00f3n de JRE debe instalarse manualmente." },
        { "launch.error.cant.download.jre", "La aplicaci\u00f3n ha solicitado una versi\u00f3n de la plataforma JRE (versi\u00f3n {0}) que no est\u00e1 instalada en el sistema local actualmente. Java Web Start no puede descargar e instalar autom\u00e1ticamente la versi\u00f3n solicitada. La versi\u00f3n de JRE debe instalarse manualmente." },
        { "launch.error.cant.access.system.cache", "El usuario actual no tiene acceso de escritura a la antememoria del sistema." },
	{ "launch.error.cant.access.user.cache", "El usuario actual no tiene acceso de escritura a la antememoria." },
        { "launch.error.noappresources", "No se han especificado recursos de aplicaci\u00f3n para esta plataforma. P\u00f3ngase en contacto con el proveedor de la aplicaci\u00f3n para asegurarse de que la plataforma est\u00e1 admitida." },
        { "launch.error.nomainclass", "No se ha podido encontrar la clase principal {0} en {1}" },
        { "launch.error.nomainclassspec", "No se ha especificado ninguna clase principal para la aplicaci\u00f3n" },
        { "launch.error.nomainjar", "No se ha especificado un archivo JAR principal." },
        { "launch.error.nonstaticmainmethod", "El m\u00e9todo main() debe ser est\u00e1tico." },
        { "launch.error.offlinemissingresource", "La aplicaci\u00f3n no puede ejecutarse si no est\u00e1 en l\u00ednea, porque no todos los recursos necesarios se han descargado localmente" },
        { "launch.error.parse", "No se ha podido analizar el archivo de ejecuci\u00f3n. Error en la l\u00ednea  {0, number}." },
        { "launch.error.parse-signedjnlp", "No se ha podido analizar el archivo de ejecuci\u00f3n firmado. Error en la l\u00ednea  {0, number}." },
        { "launch.error.resourceID", "{0}" },
        { "launch.error.resourceID-version", "({0}, {1})" },
        { "launch.error.singlecertviolation", "Los recursos JAR del archivo JNLP no est\u00e1n firmados por el mismo certificado" },
        { "launch.error.toomanyargs", "Demasiados argumentos: {0}" },
        { "launch.error.unsignedAccessViolation", "Una aplicaci\u00f3n sin firma solicita acceso sin restricciones al sistema" },
        { "launch.error.unsignedResource", "Recurso sin firma: {0}" },
        { "launch.estimatedTimeLeft", "Tiempo estimado restante: {0,number,00}:{1,number,00}:{2,number,00}" },
        { "launch.extensiondownload", "Descargando extensi\u00f3n de descriptor ({0} restante)" },
        { "launch.extensiondownload-name", "Descargando descriptor {0} ({1} restante)" },
        { "launch.initializing", "Inicializando..." },
        { "launch.launchApplication", "Iniciando aplicaci\u00f3n..." },
        { "launch.launchInstaller", "Iniciando instalador..." },
        { "launch.launchingExtensionInstaller", "Ejecutando instalador. Espere..." },
        { "launch.loadingNetProgress", "Le\u00eddo {0}" },
        { "launch.loadingNetProgressPercent", "Le\u00eddo {0} de {1} ({2}%)" },
        { "launch.loadingNetStatus", "Cargando {0} de {1}" },
        { "launch.loadingResourceFailed", "No se ha podido cargar el recurso" },
        { "launch.loadingResourceFailedSts", "Solicitado {0}" },
        { "launch.patchingStatus", "Modificando {0} desde {1}" },
        { "launch.progressScreen", "Comprobando \u00faltima versi\u00f3n..." },
        { "launch.stalledDownload", "Esperando datos..." },
        { "launch.validatingProgress", "Explorando entradas ({0}% terminado)" },
        { "launch.validatingStatus", "Validando {0} desde {1}" },
        { "launcherrordialog.abort", "Cancelar" },
        { "launcherrordialog.abortMnemonic", "VK_A" },
        { "launcherrordialog.brief.continue", "Imposible continuar la ejecuci\u00f3n" },
        { "launcherrordialog.brief.details", "Detalles" },
        { "launcherrordialog.brief.message", "Imposible ejecutar la aplicaci\u00f3n especificada." },
        { "launcherrordialog.import.brief.message", "Imposible importar la aplicaci\u00f3n especificada." },
        { "launcherrordialog.brief.messageKnown", "Imposible ejecutar {0}." },
        { "launcherrordialog.import.brief.messageKnown", "Imposible importar {0}." },
        { "launcherrordialog.brief.ok", "Aceptar" },
        { "launcherrordialog.brief.title", "Java Web Start - {0}" },
        { "launcherrordialog.consoleTab", "Consola" },
        { "launcherrordialog.errorcategory", "Categor\u00eda: {0}\n\n" },
        { "launcherrordialog.errorintro", "Ha habido un error al ejecutar la aplicaci\u00f3n.\n\n" },
        { "launcherrordialog.import.errorintro", "Se ha producido un error al importar la aplicaci\u00f3n.\n\n" },
        { "launcherrordialog.errormsg", "{0}" },
        { "launcherrordialog.errortitle", "T\u00edtulo: {0}\n" },
        { "launcherrordialog.errorvendor", "Proveedor: {0}\n" },
        { "launcherrordialog.exceptionTab", "Excepci\u00f3n" },
        { "launcherrordialog.generalTab", "General" },
        { "launcherrordialog.genericerror", "Excepci\u00f3n no esperada: {0}" },
        { "launcherrordialog.jnlpMainTab", "Archivo de ejecuci\u00f3n principal" },
        { "launcherrordialog.jnlpTab", "Archivo de ejecuci\u00f3n" },
        { "launcherrordialog.title", "Java Web Start - {0}" },
        { "launcherrordialog.wrappedExceptionTab", "Excepci\u00f3n con envoltorio" },

        { "uninstall.failedMessage", "No se puede desinstalar completamente la aplicaci\u00f3n." },
        { "uninstall.failedMessageTitle", "Desinstalar" },
        { "install.alreadyInstalled", "Ya hay un acceso directo para {0}. \u00bfDesea crearlo de todos modos?" },
        { "install.alreadyInstalledTitle", "Crear acceso directo..." },
        { "install.desktopShortcutName", "{0}" },
        { "install.installFailed", "No se puede crear un acceso directo para {0}." },
        { "install.installFailedTitle", "Crear acceso directo" },
        { "install.startMenuShortcutName", "{0}" },
	{ "install.startMenuUninstallShortcutName", "Desinstalar {0}" },
        { "install.uninstallFailed", "No se pueden suprimir los accesos directos para {0}. Int\u00e9ntelo de nuevo." },
        { "install.uninstallFailedTitle", "Suprimir accesos directos" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "El programa no puede ejecutarse porque el archivo deployment.config del sistema indica que debe haber un archivo de configuraci\u00f3n corporativo, y el archivo necesario: {0}, no est\u00e1 disponible." },

	// Jnlp Cache Viewer:
	{ "jnlp.viewer.title", "Visualizador de la antememoria de aplicaciones de Java" },
	{ "jnlp.viewer.all", "Todo" },
	{ "jnlp.viewer.type", "{0}" },
	{ "jnlp.viewer.totalSize",  "Tama\u00f1o total de los recursos:  {0}" },
        { "jnlp.viewer.emptyCache", "La antememoria de {0} est\u00e1 vac\u00eda"},
        { "jnlp.viewer.noCache", "La antememoria del sistema no est\u00e1 configurada"},

	{ "jnlp.viewer.remove.btn.mnemonic", "VK_R" },
	{ "jnlp.viewer.launch.offline.btn.mnemonic", "VK_S" },
	{ "jnlp.viewer.launch.online.btn.mnemonic", "VK_N" },

	{ "jnlp.viewer.file.menu.mnemonic", "VK_C" },
	{ "jnlp.viewer.edit.menu.mnemonic", "VK_E" },
	{ "jnlp.viewer.app.menu.mnemonic", "VK_P" },
	{ "jnlp.viewer.view.menu.mnemonic", "VK_V" },
	{ "jnlp.viewer.help.menu.mnemonic", "VK_Y" },

	{ "jnlp.viewer.cpl.mi.mnemonic", "VK_C" },
	{ "jnlp.viewer.exit.mi.mnemonic", "VK_S" },

	{ "jnlp.viewer.reinstall.mi.mnemonic", "VK_R" },
	{ "jnlp.viewer.preferences.mi.mnemonic", "VK_P" },

	{ "jnlp.viewer.launch.offline.mi.mnemonic", "VK_S" },
	{ "jnlp.viewer.launch.online.mi.mnemonic", "VK_N" },
	{ "jnlp.viewer.install.mi.mnemonic", "VK_I" },
	{ "jnlp.viewer.uninstall.mi.mnemonic", "VK_U" },
	{ "jnlp.viewer.remove.mi.mnemonic", "VK_R" },
	{ "jnlp.viewer.show.mi.mnemonic", "VK_V" },
	{ "jnlp.viewer.browse.mi.mnemonic", "VK_P" },

	{ "jnlp.viewer.view.0.mi.mnemonic", "VK_T" },
	{ "jnlp.viewer.view.1.mi.mnemonic", "VK_A" },
	{ "jnlp.viewer.view.2.mi.mnemonic", "VK_P" },
	{ "jnlp.viewer.view.3.mi.mnemonic", "VK_L" },
	{ "jnlp.viewer.view.4.mi.mnemonic", "VK_I" },

        { "jnlp.viewer.view.0", "Todos los tipos" },
        { "jnlp.viewer.view.1", "Aplicaciones" },
        { "jnlp.viewer.view.2", "Miniaplicaciones" },
        { "jnlp.viewer.view.3", "Bibliotecas" },
        { "jnlp.viewer.view.4", "Instaladores" },

	{ "jnlp.viewer.about.mi.mnemonic", "VK_R" },
	{ "jnlp.viewer.help.java.mi.mnemonic", "VK_J" },
	{ "jnlp.viewer.help.jnlp.mi.mnemonic", "VK_N" },

	{ "jnlp.viewer.remove.btn", "Suprimir" },
	{ "jnlp.viewer.remove.1.btn", "Suprimir selecci\u00f3n: {0}" },
	{ "jnlp.viewer.remove.2.btn", "Suprimir entradas seleccionadas" },
	{ "jnlp.viewer.uninstall.btn", "Desinstalar" },
	{ "jnlp.viewer.launch.offline.btn", "Ejecutar sin conexi\u00f3n" },
	{ "jnlp.viewer.launch.online.btn", "Ejecutar en l\u00ednea" },

        { "jnlp.viewer.file.menu", "Archivo" },
        { "jnlp.viewer.edit.menu", "Editar" },
        { "jnlp.viewer.app.menu", "Aplicaci\u00f3n" },
        { "jnlp.viewer.view.menu", "Ver" },
        { "jnlp.viewer.help.menu", "Ayuda" },

	{ "jnlp.viewer.cpl.mi", "Iniciar el Panel de control de Java" },
	{ "jnlp.viewer.exit.mi", "Salir" },

	{ "jnlp.viewer.reinstall.mi", "Reinstalar ..." },
	{ "jnlp.viewer.preferences.mi", "Preferencias ..." },

	{ "jnlp.viewer.launch.offline.mi", "Ejecutar sin conexi\u00f3n" },
	{ "jnlp.viewer.launch.online.mi", "Ejecutar en l\u00ednea" },
	{ "jnlp.viewer.install.mi", "Instalar accesos directos" },
	{ "jnlp.viewer.uninstall.mi", "Desinstalar accesos directos" },
        { "jnlp.viewer.remove.0.mi", "Suprimir" },
	{ "jnlp.viewer.remove.mi", "Suprimir {0}" },
	{ "jnlp.viewer.show.mi", "Ver descriptor JNLP" },
	{ "jnlp.viewer.browse.mi", "Ver p\u00e1gina inicial" },

	{ "jnlp.viewer.view.0.mi", "Todos los tipos" },
	{ "jnlp.viewer.view.1.mi", "Aplicaciones" },
	{ "jnlp.viewer.view.2.mi", "Miniaplicaciones" },
	{ "jnlp.viewer.view.3.mi", "Bibliotecas" },
	{ "jnlp.viewer.view.4.mi", "Instaladores" },

	{ "jnlp.viewer.about.mi", "Acerca de" },
	{ "jnlp.viewer.help.java.mi", "P\u00e1gina inicial de J2SE" },
	{ "jnlp.viewer.help.jnlp.mi", "P\u00e1gina inicial de JNLP" },

        { "jnlp.viewer.app.column", "Aplicaci\u00f3n" },
        { "jnlp.viewer.vendor.column", "Proveedor" },
        { "jnlp.viewer.type.column", "Tipo" },
        { "jnlp.viewer.size.column", "Tama\u00f1o" },
        { "jnlp.viewer.date.column", "Fecha" },
        { "jnlp.viewer.status.column", "Estado" },

        { "jnlp.viewer.app.column.tooltip", "Icono y t\u00edtulo de esta aplicaci\u00f3n, miniaplicaci\u00f3n o extensi\u00f3n" },
        { "jnlp.viewer.vendor.column.tooltip", "Empresa que despliega el componente" },
        { "jnlp.viewer.type.column.tooltip", "Tipo de componente" },
        { "jnlp.viewer.size.column.tooltip", "Tama\u00f1o del componente y todos sus recursos" },
        { "jnlp.viewer.date.column.tooltip", "Fecha de la \u00faltima vez que se ejecut\u00f3 la aplicaci\u00f3n, la miniaplicaci\u00f3n o el instalador" },
        { "jnlp.viewer.status.column.tooltip", "Icono que indica si el componente puede ejecutarse y la forma de hacerlo" },

        { "jnlp.viewer.application", "Aplicaci\u00f3n" },
        { "jnlp.viewer.applet", "Miniaplicaci\u00f3n" },
        { "jnlp.viewer.extension", "Biblioteca" },
        { "jnlp.viewer.installer", "Instalador" },

        { "jnlp.viewer.offline.tooltip",
                 "Esta {0} puede ejecutarse con conexi\u00f3n o sin conexi\u00f3n" },
        { "jnlp.viewer.online.tooltip", "Esta {0} puede ejecutarse con conexi\u00f3n" },
        { "jnlp.viewer.norun1.tooltip", 
                "Esta {0} s\u00f3lo puede ejecutarse desde un navegador" },
        { "jnlp.viewer.norun2.tooltip", "No se pueden ejecutar las extensiones" },

	{ "jnlp.viewer.show.title", "Descriptor JNLP para: {0}" },

	{ "jnlp.viewer.removing", "Suprimiendo ..." },
	{ "jnlp.viewer.launching", "Iniciando ..." },
	{ "jnlp.viewer.browsing", "Ejecutando navegador ..." },
	{ "jnlp.viewer.sorting", "Ordenando entradas ..." },
	{ "jnlp.viewer.searching", "Examinando entradas ..." },
        { "jnlp.viewer.installing", "Instalando..." },

        { "jnlp.viewer.reinstall.title", "Reinstalar las aplicaciones JNLP suprimidas" },
	{ "jnlp.viewer.reinstallBtn", "Reinstalar las aplicaciones seleccionadas" },
	{ "jnlp.viewer.reinstallBtn.mnemonic", "VK_R" },
        { "jnlp.viewer.closeBtn", "Cerrar" },
        { "jnlp.viewer.closeBtn.mnemonic", "VK_C" },

	{ "jnlp.viewer.reinstall.column.title", "T\u00edtulo:" },
	{ "jnlp.viewer.reinstall.column.location", "Ubicaci\u00f3n:" },

	// cache size warning
	{ "jnlp.cache.warning.title", "Advertencia sobre el tama\u00f1o de la antememoria de JNLP" },
	{ "jnlp.cache.warning.message", "Advertencia: \n\n"+
		"Se ha superado el espacio de disco recomendado para\n"+
		"aplicaciones y recursos JNLP en la antememoria.\n\n"+
		"Ahora est\u00e1 utilizando: {0}\n"+
		"El l\u00edmite recomendado es: {1}\n\n"+
		"Utilice el Panel de control de Java para suprimir \n"+
		"aplicaciones o recursos, o para ampliar el l\u00edmite." },

        // Control Panel
        { "control.panel.title", "Panel de control de Java" },
        { "control.panel.general", "General" },
        { "control.panel.security", "Seguridad" },
        { "control.panel.java", "Java" },
        { "control.panel.update", "Actualizaci\u00f3n" },
        { "control.panel.advanced", "Avanzado" },

        // Common Strings used in different panels.
        { "common.settings", "Configuraci\u00f3n" },
        { "common.ok_btn", "Aceptar" },
        { "common.ok_btn.mnemonic", "VK_A" },
        { "common.cancel_btn", "Cancelar" },
        { "common.cancel_btn.mnemonic", "VK_C" },
        { "common.apply_btn", "Aplicar" },
        { "common.apply_btn.mnemonic", "VK_P" },
        { "common.add_btn", "Agregar" },
        { "common.add_btn.mnemonic", "VK_G" },
        { "common.remove_btn", "Suprimir" },
        { "common.remove_btn.mnemonic", "VK_R" },

        // Network Settings Dialog
        { "network.settings.dlg.title", "Configuraci\u00f3n de red" },
        { "network.settings.dlg.border_title", " Configuraci\u00f3n de proxy de red " },
        { "network.settings.dlg.browser_rbtn", "Usar configuraci\u00f3n de navegador" },
        { "browser_rbtn.mnemonic", "VK_N" },
        { "network.settings.dlg.manual_rbtn", "Usar servidor proxy" },
        { "manual_rbtn.mnemonic", "VK_P" },
        { "network.settings.dlg.address_lbl", "Direcci\u00f3n:" },
        { "network.settings.dlg.port_lbl", "Puerto:" },
        { "network.settings.dlg.advanced_btn", "Avanzada..." },
        { "network.settings.dlg.advanced_btn.mnemonic", "VK_V" },
        { "network.settings.dlg.bypass_text", "No se tiene en cuenta el servidor proxy para direcciones locales" },
        { "network.settings.dlg.bypass.mnemonic", "VK_Y" },
        { "network.settings.dlg.autoconfig_rbtn", "Usar secuencia de \u00f3rdenes de configuraci\u00f3n autom\u00e1tica de proxy" },
        { "autoconfig_rbtn.mnemonic", "VK_T" },
        { "network.settings.dlg.location_lbl", "Ubicaci\u00f3n de la secuencia de \u00f3rdenes: " },
        { "network.settings.dlg.direct_rbtn", "Conexi\u00f3n directa" },
        { "direct_rbtn.mnemonic", "VK_D" },
        { "network.settings.dlg.browser_text", "La configuraci\u00f3n autom\u00e1tica puede anular los valores de configuraci\u00f3n manual. Para asegurarse de que utiliza la configuraci\u00f3n manual, desactive la configuraci\u00f3n autom\u00e1tica." },
        { "network.settings.dlg.proxy_text", "No usa el navegador para configurar el proxy." },
        { "network.settings.dlg.auto_text", "Usa la secuencia de configuraci\u00f3n autom\u00e1tica del proxy en la ubicaci\u00f3n especificada." },
        { "network.settings.dlg.none_text", "Usa conexi\u00f3n directa." },

        // Advanced Network Settings Dialog
        { "advanced.network.dlg.title", "Configuraci\u00f3n de red avanzada" },
        { "advanced.network.dlg.servers", " Servidores " },
        { "advanced.network.dlg.type", "Tipo" },
        { "advanced.network.dlg.http", "HTTP:" },
        { "advanced.network.dlg.secure", "Segura:" },
        { "advanced.network.dlg.ftp", "FTP:" },
        { "advanced.network.dlg.socks", "Socks:" },
        { "advanced.network.dlg.proxy_address", "Direcci\u00f3n de proxy" },
        { "advanced.network.dlg.port", "Puerto" },
        { "advanced.network.dlg.same_proxy", " Usar el mismo servidor proxy para todos los protocolos" },
        { "advanced.network.dlg.same_proxy.mnemonic", "VK_U" },
        { "advanced.network.dlg.exceptions", " Excepciones " },
        { "advanced.network.dlg.no_proxy", " No usar servidor proxy para las direcciones que empiecen por" },
        { "advanced.network.dlg.no_proxy_note", " Usar punto y coma (;) para separar las entradas." },

        // DeleteFilesDialog
        { "delete.files.dlg.title", "Borrar archivos temporales" },
        { "delete.files.dlg.temp_files", "\u00bfBorrar los siguientes archivos temporales?" },
        { "delete.files.dlg.applets", "Miniaplicaciones descargadas" },
        { "delete.files.dlg.applications", "Aplicaciones descargadas" },
        { "delete.files.dlg.other", "Otros archivos" },

	// General
	{ "general.cache.border.text", " Archivos temporales de Internet " },
	{ "general.cache.delete.text", "Borrar archivos..." },
        { "general.cache.delete.text.mnemonic", "VK_B" },
	{ "general.cache.settings.text", "Configuraci\u00f3n..." },
        { "general.cache.settings.text.mnemonic", "VK_F" },
	{ "general.cache.desc.text", "Los archivos utilizados en las aplicaciones de Java se guardan en una carpeta especial para acelerar su ejecuci\u00f3n posterior. S\u00f3lo los usuarios con conocimientos avanzados deber\u00edan borrar archivos o modificar esta configuraci\u00f3n." },
	{ "general.network.border.text", " Configuraci\u00f3n de red " },
	{ "general.network.settings.text", "Configuraci\u00f3n de red..." },
        { "general.network.settings.text.mnemonic", "VK_N" },
	{ "general.network.desc.text", "La configuraci\u00f3n de red se utiliza cuando se establece la conexi\u00f3n con Internet. Java utilizar\u00e1 la configuraci\u00f3n de red del navegador de forma predeterminada. Esta configuraci\u00f3n s\u00f3lo debe ser modificada por usuarios con conocimientos avanzados." },
        { "general.about.border", "Acerca de" },
	{ "general.about.btn", "Acerca de..." },
	{ "general.about.btn.mnemonic", "VK_R" },
	{ "general.about.text", "Muestra informaci\u00f3n sobre la versi\u00f3n del Panel de control." },

	// Security
	{ "security.certificates.border.text", " Certificados " },
	{ "security.certificates.button.text", "Certificados..." },
        { "security.certificates.button.mnemonic", "VK_E" },
	{ "security.certificates.desc.text", "Los certificados se utilizan para la correcta identificaci\u00f3n de usuarios, certificaciones, autoridades y editores." },
	{ "security.policies.border.text", " Normas " },
	{ "security.policies.advanced.text", "Avanzadas..." },
        { "security.policies.advanced.mnemonic", "VK_D" },
	{ "security.policies.desc.text", "Las normas de seguridad se utilizan para controlar las restricciones de seguridad impuestas a las aplicaciones y las miniaplicaciones." },

	// Update
	{ "update.notify.border.text", " Notificaci\u00f3n de actualizaci\u00f3n " }, // this one is not currently used.  See update panel!!!
	{ "update.updatenow.button.text", "Actualizar ahora" },
	{ "update.updatenow.button.mnemonic", "VK_U" },
	{ "update.advanced.button.text", "Avanzada..." },
	{ "update.advanced.button.mnemonic", "VK_D" },
        { "update.desc.text", "El mecanismo de actualizaci\u00f3n de Java le garantiza que tendr\u00e1 la \u00faltima versi\u00f3n de la plataforma Java.  Las opciones siguientes permiten controlar la forma de obtener y aplicar las actualizaciones." },
        { "update.notify.text", "Recibir notificaci\u00f3n:" },
        { "update.notify_install.text", "Antes de la instalaci\u00f3n" },
        { "update.notify_download.text", "Antes de la descarga y la instalaci\u00f3n" },
        { "update.autoupdate.text", "Comprobar actualizaciones autom\u00e1ticamente" },
        { "update.advanced_title.text", "Actualizaci\u00f3n autom\u00e1tica: configuraci\u00f3n avanzada" },
        { "update.advanced_title1.text", "Seleccionar la frecuencia y el momento de la b\u00fasqueda." },
        { "update.advanced_title2.text", "Frecuencia" },
        { "update.advanced_title3.text", "Cu\u00e1ndo" },
        { "update.advanced_desc1.text", "Buscar cada d\u00eda a las {0}" },
        { "update.advanced_desc2.text", "Buscar cada {0} a las {1}" },
        { "update.advanced_desc3.text", "Buscar el d\u00eda {0} de cada mes a las {1}" },
        { "update.check_daily.text", "Diariamente" },
        { "update.check_weekly.text", "Semanalmente" },
        { "update.check_monthly.text", "Mensualmente" },
        { "update.check_date.text", "D\u00eda:" },
        { "update.check_day.text", "Cada:" },
        { "update.check_time.text", "Hora:" },
        { "update.lastrun.text", "\u00daltima ejecuci\u00f3n de Java Update: {0}, d\u00eda {1}." },
        { "update.desc_autooff.text", "Haga clic en el bot\u00f3n \"Actualizar ahora\" para comprobar si hay actualizaciones. Si se detecta alguna, aparecer\u00e1 un icono en la bandeja del sistema. Sit\u00fae el cursor sobre el icono para ver el estado de la actualizaci\u00f3n." },
        { "update.desc_check_daily.text", "Java Update comprobar\u00e1 las actualizaciones cada d\u00eda a las {0}. " },
        { "update.desc_check_weekly.text", "Java Update comprobar\u00e1 las actualizaciones cada {0} a las {1}. " },
        { "update.desc_check_monthly.text", "Java Update comprobar\u00e1 las actualizaciones el d\u00eda {0} de cada mes a las {1}. " },
        { "update.desc_systrayicon.text", "Si hay alguna actualizaci\u00f3n disponible, aparecer\u00e1 un icono en la bandeja del sistema. Sit\u00fae el cursor sobre el icono para ver el estado de la actualizaci\u00f3n. " },
        { "update.desc_notify_install.text", "Recibir\u00e1 una notificaci\u00f3n antes de que se instale la actualizaci\u00f3n." },
        { "update.desc_notify_download.text", "Recibir\u00e1 una notificaci\u00f3n antes de que se descargue e instale la actualizaci\u00f3n." },
        { "update.launchbrowser.error.text", "No se puede ejecutar el programa de comprobaci\u00f3n de Java Update. Para obtener la \u00faltima versi\u00f3n de Java Update, vaya a http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Error - Actualizaci\u00f3n" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "Borrar archivos..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_B" },
        { "cache.settings.dialog.view_jws_btn", "Ver aplicaciones..." },
        { "cache.settings.dialog.view_jws_btn.mnemonic", "VK_V" },
        { "cache.settings.dialog.view_jpi_btn", "Ver miniaplicaciones..." },
        { "cache.settings.dialog.view_jpi_btn.mnemonic", "VK_P" },
        { "cache.settings.dialog.chooser_title", "Ubicaci\u00f3n de archivos temporales" },
        { "cache.settings.dialog.select", "Seleccionar" },
        { "cache.settings.dialog.select_tooltip", "Usar la ubicaci\u00f3n seleccionada" },
        { "cache.settings.dialog.select_mnemonic", "S" },
        { "cache.settings.dialog.title", "Configuraci\u00f3n de archivos temporales" },
        { "cache.settings.dialog.cache_location", "Ubicaci\u00f3n:" },
        { "cache.settings.dialog.change_btn", "Cambiar..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_M" },
        { "cache.settings.dialog.disk_space", "Cantidad de espacio de disco utilizable:" },
        { "cache.settings.dialog.unlimited_btn", "Sin l\u00edmite" },
        { "cache.settings.dialog.max_btn", "M\u00e1ximo" },
        { "cache.settings.dialog.compression", "Compresi\u00f3n de Jar:" },
        { "cache.settings.dialog.none", "Ninguna" },
        { "cache.settings.dialog.high", "Alta" },

	// JNLP File/MIME association dialog strings:
	{ "javaws.association.dialog.title", "Asociaci\u00f3n MIME/archivo JNLP" },
        { "javaws.association.dialog.exist.command", "ya existe con:\n{0}"},
	{ "javaws.association.dialog.exist", "ya existe." },
        { "javaws.association.dialog.askReplace", "\n\u00bfEst\u00e1 seguro de que prefiere utilizar {0} para manejarla?"},
	{ "javaws.association.dialog.ext", "Extensiones de archivos: {0}" },
        { "javaws.association.dialog.mime", "Tipo MIME: {0}" },
        { "javaws.association.dialog.ask", "\u00bfDesea usar {0} para la gesti\u00f3n:" },
        { "javaws.association.dialog.existAsk", "\u00a1ADVERTENCIA! Asociaci\u00f3n con:"},

        // Advanced panel strings:
        { "deployment.console.startup.mode", "Consola de Java" },
        { "deployment.console.startup.mode.SHOW", "Ver consola" },
        { "deployment.console.startup.mode.SHOW.tooltip", "<html>" +
          "Inicia la Consola de Java maximizada" +
          "</html>" },
        { "deployment.console.startup.mode.HIDE", "Ocultar consola" },
        { "deployment.console.startup.mode.HIDE.tooltip", "<html>" +
          "Inicia la Consola de Java minimizada" +
          "</html>" },
        { "deployment.console.startup.mode.DISABLE", "No iniciar la consola" },
        { "deployment.console.startup.mode.DISABLE.tooltip", "<html>" +
          "Impide qe se inicie la Consola de Java" +
          "</html>" },
        { "deployment.trace", "Activar rastreo" },
        { "deployment.trace.tooltip", "<html>" +
          "Crea un archivo de rastreo para efectuar" +
          "<br>la depuraci\u00f3n" +
          "</html>" },
        { "deployment.log", "Activar registro de eventos" },
	{ "deployment.log.tooltip", "<html>" +
                                    "Crea un archivo de registro para" +
                                    "<br>detectar errores" +
                                    "</html>" },
        { "deployment.control.panel.log", "Registro en panel de control" },
        { "deployment.javapi.lifecycle.exception", "Ver excepciones al cargar la miniaplicaci\u00f3n" },
        { "deployment.javapi.lifecycle.exception.tooltip", "<html>" +
                                         "Muestra un cuadro de di\u00e1logo con excepciones" +
                                         "<br>cuando se producen errores al cargar la miniaplicaci\u00f3n" +
                                         "<html>" },
        { "deployment.browser.vm.iexplorer", "Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
          "Permite usar Sun Java con la etiqueta APPLET" +
          "<br>en Internet Explorer" +
          "</html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla y Netscape" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
          "Permite usar Sun Java con la etiqueta APPLET en" +
          "<br>los navegadores Mozilla y Netscape" +
          "</html>" },
        { "deployment.console.debugging", "Depuraci\u00f3n" },
	{ "deployment.browsers.applet.tag", "Etiqueta <APPLET> permitida" },
        { "deployment.javaws.shortcut", "Creaci\u00f3n de accesos directos" },
        { "deployment.javaws.shortcut.ALWAYS", "Permitir siempre" },
        { "deployment.javaws.shortcut.ALWAYS.tooltip", "<html>" +
          "Siempre se crean accesos" +
	  "</html>" },
        { "deployment.javaws.shortcut.NEVER" , "No permitir nunca" },
        { "deployment.javaws.shortcut.NEVER.tooltip", "<html>" +
          "Nunca se crean accesos directos" +
          "</html>" },
        { "deployment.javaws.shortcut.ASK_USER", "Preguntar a usuario" },
        { "deployment.javaws.shortcut.ASK_USER.tooltip", "<html>" +
          "Pregunta al usuario si debe crearse" +
          "<br>un acceso directo" +
          "</html>" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED", "Permitir siempre, si se ha indicado" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED.tooltip", "<html>" +
          "Siempre se crean accesos directos si" +
          "<br>se ha indicado en la aplicaci\u00f3n JNLP" +
          "</html>" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED", "Preguntar al usuario, si se ha indicado" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED.tooltip", "<html>" +
          "Pregunta al usuario si debe crearse un" +
          "<br>acceso directo en caso de que" +
          "<br>lo solicite la aplicaci\u00f3n JNLP" +
          "</html>" },
	{ "deployment.javaws.associations.NEVER", "No permitir nunca" },
        { "deployment.javaws.associations.NEVER.tooltip", "<html>" +
          "Nunca se crea una asociaci\u00f3n de" +
          "<br>Extensi\u00f3n de archivo/MIME" +
          "</html>" },
        { "deployment.javaws.associations.ASK_USER", "Preguntar a usuario" },
        { "deployment.javaws.associations.ASK_USER.tooltip", "<html>" +
          "Pregunta al usuario antes de crear una asociaci\u00f3n de" +
          "<br>Extensi\u00f3n de archivo/MIME" +
          "</html>" },
        { "deployment.javaws.associations.REPLACE_ASK", "Preguntar al usuario para reemplazar" },
        { "deployment.javaws.associations.REPLACE_ASK.tooltip", "<html>" +
          "Preguntar al usuario s\u00f3lo si se va a reemplazar" +
          "<br>una asociaci\u00f3n de Extensi\u00f3n de archivo/MIME" +
          "<br>existente" +
          "</html>" },
        { "deployment.javaws.associations.NEW_ONLY", "S\u00f3lo si la asociaci\u00f3n es nueva" },
        { "deployment.javaws.associations.NEW_ONLY.tooltip", "<html>" +
          "Crear \u00fanicamente asociaciones de" +
          "<br>Extensiones de archivo/MIME nuevas" +
          "</html>" },
        { "deployment.javaws.associations", "Asociaci\u00f3n MIME/archivo JNLP" },
        { "deployment.security.settings", "Seguridad" },
        { "deployment.security.askgrantdialog.show", "Autorizar al usuario a dar permisos para contenido firmado" },
        { "deployment.security.askgrantdialog.notinca", "Autorizar al usuario a dar permisos para contenido procedente de una autoridad que no es de confianza" },
        { "deployment.security.browser.keystore.use", "Usar los certificados y claves del almac\u00e9n de claves del navegador" },
        { "deployment.security.notinca.warning", "Avisar si no se puede verificar la autoridad de certificaci\u00f3n" },
        { "deployment.security.expired.warning", "Avisar si el certificado ha caducado o no es v\u00e1lido" },
        { "deployment.security.jsse.hostmismatch.warning", "Avisar si el certificado del sitio no corresponde al nombre del sistema" },
        { "deployment.security.sandbox.awtwarningwindow", "Ver indicaci\u00f3n de zona protegida" },
        { "deployment.security.sandbox.jnlp.enhanced", "Permitir al usuario aceptar las peticiones de seguridad de JNLP" },
        { "deploy.advanced.browse.title", "Elegir un archivo para ejecutar el navegador predeterminado" },
        { "deploy.advanced.browse.select", "Seleccionar" },
        { "deploy.advanced.browse.select_tooltip", "Usar archivo seleccionado para ejecutar navegador" },
        { "deploy.advanced.browse.select_mnemonic", "S" },
        { "deploy.advanced.browse.browse_btn", "Explorar..." },
        { "deploy.advanced.browse.browse_btn.mnemonic", "VK_E" },
        { "deployment.browser.default", "Orden para ejecutar el navegador predeterminado" },
        { "deployment.misc.label", "Varios" },
        { "deployment.system.tray.icon", "Colocar el icono de Java en la bandeja del sistema" },
	{ "deployment.system.tray.icon.tooltip", "<html>" +
                             "Seleccione esta opci\u00f3n para ver el icono de la" +
                             "<br>taza de Java en la bandeja del sistema" +
                             "<br>cuando Java se est\u00e1 ejecutando en el navegador" +
                             "</html>" },

        //PluginJresDialog strings:
        { "jpi.jres.dialog.title", "Configuraci\u00f3n del entorno de ejecuci\u00f3n de Java" },
        { "jpi.jres.dialog.border", " Versiones del entorno de ejecuci\u00f3n de Java " },
        { "jpi.jres.dialog.column1", "Nombre del producto" },
        { "jpi.jres.dialog.column2", "Versi\u00f3n" },
        { "jpi.jres.dialog.column3", "Ubicaci\u00f3n" },
        { "jpi.jres.dialog.column4", "Par\u00e1metros del entorno de ejecuci\u00f3n de Java" },
        { "jpi.jdk.string", "JDK" },
        { "jpi.jre.string", "JRE" },
        { "jpi.jres.dialog.product.tooltip", "Elegir JRE o JDK para nombre del producto." },

        // AboutDialog strings:
        { "about.dialog.title", "Acerca de Java" },

        // JavaPanel strings:
        { "java.panel.plugin.border", " Configuraci\u00f3n de tiempo de ejecuci\u00f3n de la miniaplicaci\u00f3n de Java" },
        { "java.panel.plugin.text", "La configuraci\u00f3n de tiempo de ejecuci\u00f3n se utiliza cuando una miniaplicaci\u00f3n se ejecuta en el navegador." },
        { "java.panel.jpi_view_btn", "Ver..." },
        { "java.panel.javaws_view_btn", "Ver..." },
        { "java.panel.jpi_view_btn.mnemonic", "VK_V" },
        { "java.panel.javaws_view_btn.mnemonic", "VK_E" },
        { "java.panel.javaws.border", " Configuraci\u00f3n de tiempo de ejecuci\u00f3n de la aplicaci\u00f3n de Java "},
        { "java.panel.javaws.text", "La configuraci\u00f3n de tiempo de ejecuci\u00f3n se utiliza cuando se ejecuta una aplicaci\u00f3n de Java mediante JNLP (Java Network Launching Protocol)." },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "<html><b>Existe una versi\u00f3n m\u00e1s reciente del entorno de ejecuci\u00f3n de Java</b></html>Internet Explorer ya tiene una versi\u00f3n m\u00e1s reciente del entorno de ejecuci\u00f3n de Java. \u00bfDesea sustituirla?\n" },
        { "browser.settings.success.caption", "Realizado - Navegador" },
        { "browser.settings.success.text", "<html><b>La configuraci\u00f3n del navegador ha cambiado</b></html>Los cambios tendr\u00e1n efecto cuando se reinicie el navegador.\n" },
        { "browser.settings.fail.caption", "Advertencia - Navegador" },
        { "browser.settings.fail.moz.text", "<html><b>No se puede cambiar la configuraci\u00f3n del navegador</b></html>"
                                        + "Compruebe si Mozilla o Netscape est\u00e1n bien instalados en el sistema o "
                                        + "si dispone de "
                                        + "suficientes permisos para cambiar la configuraci\u00f3n del sistema.\n" },

        { "browser.settings.fail.ie.text", "<html><b>No se puede cambiar la configuraci\u00f3n del navegador</b></html>Compruebe si dispone de suficientes "
					+ "permisos para cambiar la configuraci\u00f3n del sistema.\n" },


        // Tool tip strings.
        { "cpl.ok_btn.tooltip", "<html>" +
                                "Cierra el Panel del control de Java y" +
                                "<br>guarda los cambios realizados" +
                                "</html>" },
        { "cpl.apply_btn.tooltip",  "<html>" +
                                    "Aplica los cambios realizados sin" +
                                    "<br>cerrar el Panel de control de Java" +
                                    "</html>" },
        { "cpl.cancel_btn.tooltip", "<html>" +
                                    "Cierra el Panel de control de Java" +
                                    "<br>sin guardar los cambios" +
                                    "</html>" },

        {"network.settings.btn.tooltip", "<html>"+
                                         "Permite modificar la configuraci\u00f3n de la conexi\u00f3n a Internet" +
                                         "</html>"},

        {"temp.files.settings.btn.tooltip", "<html>"+
                                            "Permite modificar la configuraci\u00f3n de los archivos temporales" +
                                            "</html>"},

        {"temp.files.delete.btn.tooltip", "<html>" +  // body bgcolor=\"#FFFFCC\">"+
                                          "Borra los archivos temporales de Java" +
                                          "</html>"},

        {"delete.files.dlg.applets.tooltip", "<html>" +
                                          "Marque esta opci\u00f3n para borrar todos los archivos" +
                                          "<br>temporales creados por las miniaplicaciones de Java" +
                                          "</html>" },

        {"delete.files.dlg.applications.tooltip", "<html>" +
                                          "Marque esta opci\u00f3n para borrar todos los archivos" +
                                          "<br>temporales creados por las aplicaciones de" +
                                          "<br>Java Web Start" +
                                          "</html>" },

        {"delete.files.dlg.other.tooltip", "<html>" +
                                          "Marque esta opci\u00f3n para borrar otros archivos" +
                                          "<br>temporales creados por Java" +
                                          "</html>" },

        {"delete.files.dlg.temp_files.tooltip", "<html>" +
                                          "Las aplicaciones de Java pueden guardar algunos archivos" +
                                          "<br>temporales en el sistema. Pueden borrarse " +
                                          "<br>sin problemas." +
                                          "<br>" +
                                          "<p>Despu\u00e9s de borrarlos, algunas aplicaciones" +
                                          "<br>Java pueden tardar m\u00e1s en iniciarse" +
                                          "<br>la primera vez que se ejecutan." +
                                          "</html>" },

        {"cache.settings.dialog.view_jws_btn.tooltip", "<html>" +
                                          "Muestra los archivos temporales creados" +
                                          "<br>por las aplicaciones de Java Web Start" +
                                          "</html>" },

        {"cache.settings.dialog.view_jpi_btn.tooltip", "<html>" +
                                          "Muestra los archivos termporales creados" +
                                          "<br>por las miniaplicaciones de Java" +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "Permite especificar el directorio donde" +
                                          "<br>se almacenar\u00e1n los archivos temporales"+
                                          "</html>" },


        {"cache.settings.dialog.unlimited_btn.tooltip", "<html>" +
                                          "No limita la cantidad de espacio disponible en el" +
                                          "<br>disco para guardar archivos temporales" +
                                          "</html>" },

        {"cache.settings.dialog.max_btn.tooltip", "<html>" +
                                          "Permite especificar la cantidad m\u00e1xima de espacio" +
                                          "<br>disponible en el disco para guardar archivos temporales" +
                                          "</html>" },

        {"cache.settings.dialog.compression.tooltip", "<html>" +
                                          "Permite especificar la cantidad de compresi\u00f3n usada" +
                                          "<br>para los archivos JAR guardados por los programas" +
                                          "<br>Java en el directorio de archivos temporales" +
                                          "<br>" +
                                          "<p>Con \"Ninguna\", los programas de Java se inician con" +
                                          "<br>m\u00e1s rapidez, pero se precisa m\u00e1s espacio" +
                                          "<br>en el disco para almacenarlos.  Cuanto m\u00e1s altos son" +
                                          "<br>los valores, menos espacio se necesita en el disco, " +
                                          "<br>pero m\u00e1s tardan en iniciarse las aplicaciones." +
                                          "</html>" },

        { "common.ok_btn.tooltip",  "<html>" +
                                    "Guarda los cambios y cierra el cuadro de di\u00e1logo" +
                                    "</html>" },

        { "common.cancel_btn.tooltip",  "<html>" +
                                        "Anula los cambios y cierra el cuadro de di\u00e1logo" +
                                        "</html>"},

	{ "network.settings.advanced_btn.tooltip",  "<html>" +
                                                    "Permite ver y modificar la configuraci\u00f3n avanzada del servidor proxy"+
                                                    "</html>"},

        {"security.certs_btn.tooltip", "<html>" +
                                       "Permite importar, exportar o suprimir los certificados disponibles" +
                                       "</html>" },

        { "cert.import_btn.tooltip", "<html>" +
                                     "Importa un certificado que no se encuentra" +
                                     "<br>en la lista" +
				     "</html>"},

        { "cert.export_btn.tooltip",    "<html>" +
                                        "Exporta el certificado seleccionado" +
                                        "</html>"},

        { "cert.remove_btn.tooltip",  "<html>" +
                                      "Suprime el certificado seleccionado"+
                                      "<br>de la lista" +
        		      "</html>"},

        { "cert.details_btn.tooltip", "<html>" +
		      "Muestra informaci\u00f3n detallada sobre" +
                      "<br>el certificado seleccionado." +
		      "</html>"},


        { "java.panel.jpi_view_btn.tooltip",  "<html>" +
                                              "Permite modificar la configuraci\u00f3n del entorno"+
                                              "<br>de ejecuci\u00f3n de Java para miniaplicaciones" +
                                              "</html>" },

        { "java.panel.javaws_view_btn.tooltip",   "<html>" +
                                                  "Permite modificar la configuraci\u00f3n del entorno"+
                                                  "<br>de ejecuci\u00f3n de Java para aplicaciones" +
                                                  "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "Muestra informaci\u00f3n sobre esta versi\u00f3n del" +
                                            "<br>entorno de ejecuci\u00f3n de J2SE Runtime Environment" +
                                            "</html>" },

        { "update.notify_combo.tooltip",  "<html>" +
                                          "Indique cu\u00e1ndo desea recibir notificaci\u00f3n " +
                                          "<br>de las nuevas actualizaciones de Java" +
                                          "<br>disponibles" +
                                          "</html>" },

        { "update.advanced_btn.tooltip",  "<html>" +
                                          "Permite modificar las normas de programaci\u00f3n" +
                                          "<br>de la actualizaci\u00f3n autom\u00e1tica" +
                                          "</html>" },

        { "update.now_btn.tooltip",    "<html>" +
                                      "Ejecuta Java Update para buscar las \u00faltimas" +
                                      "<br>actualizaciones disponibles de Java" +
                                      "</html>" },

        { "vm.options.add_btn.tooltip",   "<html>" +
                                          "Agrega un JRE nuevo a la lista" +
                                          "</html>" },

        { "vm.options.remove_btn.tooltip", "<html>" +
                                           "Suprime la entrada seleccionada de la lista" +
                                           "</html>" },

        { "vm.optios.ok_btn.tooltip",    "<html>" +
		         "Guarda todas las entradas que contengan" +
                         "<br>informaci\u00f3n sobre el nombre, la versi\u00f3n" +
                         "<br>y la ubicaci\u00f3n del producto" +
		         "</html>" },

        { "jnlp.jre.find_btn.tooltip",  "<html>" +
                        "Busca una versi\u00f3n del entorno de ejecuci\u00f3n de Java" +
                        "<br>instalada" +
		        "</html>" },

        { "jnlp.jre.add_btn.tooltip",   "<html>" +
                        "Agrega una nueva entrada a la lista" +
		        "</html>" },

        { "jnlp.jre.remove_btn.tooltip",  "<html>" +
                                          "Suprime la entrada seleccionada de la " +
                                          "<br>lista del usuario" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "Autorizaci\u00f3n para descargar JRE" },
        { "download.jre.prompt.text1", "La aplicaci\u00f3n \"{0}\" necesita una versi\u00f3n del entorno"
                                     + "de ejecuci\u00f3n de JRE (versi\u00f3n {1}), que "
                                     + "no se encuentra instalada en el sistema." },
        { "download.jre.prompt.text2", "\u00bfDesea que Java Web Start descargue e instale "
                                     + "autom\u00e1ticamente este JRE?" },
        { "download.jre.prompt.okButton", "Descargar" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "download.jre.prompt.cancelButton", "Cancelar" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "autoupdatecheck.buttonYes", "S\u00ed" },
	{ "autoupdatecheck.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "autoupdatecheck.buttonNo", "No" },
	{ "autoupdatecheck.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "autoupdatecheck.buttonAskLater", "Preguntar m\u00e1s adelante" },
	{ "autoupdatecheck.buttonAskLater.acceleratorKey", new Integer(KeyEvent.VK_P)},
        { "autoupdatecheck.caption", "Comprobar las actualizaciones autom\u00e1ticamente" },
        { "autoupdatecheck.message", "Java Update puede actualizar autom\u00e1ticamente el software de Java cuando se publican nuevas versiones. \u00bfQuiere activar este servicio?" },
    };
}

