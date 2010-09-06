/*
 * @(#)Deployment_it.java	1.26 04/07/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;


/**
 * Italian version of Deployment strings.
 *
 * @author Stanley Man-Kit Ho
 */

public final class Deployment_it extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
        { "product.javapi.name", "Java Plug-in {0}" },
        { "product.javaws.name", "Java Web Start {0}" },

	{ "console.version", "Versione" },
	{ "console.default_vm_version", "Versione predefinita di Virtual Machine" },
	{ "console.using_jre_version", "Uso della versione JRE" },
	{ "console.user_home", "Directory principale utente" },
	{ "console.caption", "Console Java" },
	{ "console.clear", "Cancella" },
	{ "console.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "console.close", "Chiudi" },
	{ "console.close.acceleratorKey", new Integer(KeyEvent.VK_H) },
	{ "console.copy", "Copia" },
	{ "console.copy.acceleratorKey", new Integer(KeyEvent.VK_P) },
	{ "console.menu.text.top", "----------------------------------------------------\n" },
	{ "console.menu.text.c", "c:   cancella finestra console\n" },
	{ "console.menu.text.f", "f:   finalizza oggetti nella coda di finalizzazione\n" },
	{ "console.menu.text.g", "g:   recupera spazio\n" },
	{ "console.menu.text.h", "h:   visualizza questo messaggio di aiuto\n" },
	{ "console.menu.text.j", "j:   esegui dump dei dati jcov\n"},
	{ "console.menu.text.l", "l:   esegui dump dell'elenco classloader\n" },
	{ "console.menu.text.m", "m:   stampa utilizzo memoria\n" },
	{ "console.menu.text.o", "o:   attiva registrazione eventi\n" },
	{ "console.menu.text.p", "p:   ricarica configurazione proxy\n" },
	{ "console.menu.text.q", "q:   nascondi console\n" },
	{ "console.menu.text.r", "r:   ricarica configurazione criteri\n" },
	{ "console.menu.text.s", "s:   esegui dump delle propriet\u00e0 del sistema e dell'installazione\n" },
	{ "console.menu.text.t", "t:   esegui dump dell'elenco thread\n" },
	{ "console.menu.text.v", "v:   esegui dump dello stack del thread\n" },
	{ "console.menu.text.x", "x:   cancella cache classloader\n" },
	{ "console.menu.text.0", "0-5: imposta livello di traccia su <n>\n" },
	{ "console.menu.text.tail", "----------------------------------------------------\n" },
	{ "console.done", "Fine." },
	{ "console.trace.level.0", "Livello di traccia impostato su 0: nessuno ... completato." },
	{ "console.trace.level.1", "Livello di traccia impostato su 1: base ... completato." },
	{ "console.trace.level.2", "Livello di traccia impostato su 2: base, rete ... completato." },
	{ "console.trace.level.3", "Livello di traccia impostato su 3: base, rete, protezione ... completato." },
	{ "console.trace.level.4", "Livello di traccia impostato su 4: base, rete, protezione, est ... completato." },
	{ "console.trace.level.5", "Livello di traccia impostato su 5: tutto ... completato." },
	{ "console.log", "Registrazione eventi impostata su : " },
	{ "console.completed", " .... completato." },
	{ "console.dump.thread", "Esegui dump dell'elenco thread ...\n" },
	{ "console.dump.stack", "Esegui dump dello stack del thread...\n" },
	{ "console.dump.system.properties", "Esegui dump delle propriet\u00e0 del sistema ...\n" },
        { "console.dump.deployment.properties", "Esegui dump delle propriet\u00e0 dell'installazione ...\n" },
	{ "console.clear.classloader", "Cancella cache classloader .... completato." },
	{ "console.reload.policy", "Ricarica configurazione policy" },
	{ "console.reload.proxy", "Ricarica configurazione proxy...." },
	{ "console.gc", "Recupera spazio" },
	{ "console.finalize", "Finalizza oggetti nella coda di finalizzazione" },
	{ "console.memory", "Memoria:  {0}K  liberi: {1}K  ({2}%)" },
	{ "console.jcov.error", "Errore runtime jcov: assicurarsi di aver specificato la corretta opzione jcov\n"},
	{ "console.jcov.info", "Esecuzione dump dei dati jcov corretta\n"},

	{ "https.dialog.caption", "Attenzione - HTTPS" },
	{ "https.dialog.text", "<html><b a>Nome host non corrispondente</b></html>Il nome host nel certificato di protezione del server non corrisponde al nome del server."
				+ "\n\nNome host dell''URL: {0}"
				+ "\n\nNome host dal certificato: {1}"
				+ "\n\nContinuare?" },
	{ "https.dialog.unknown.host", "Host sconosciuto" },

	{ "security.dialog.caption", "Attenzione - protezione" },
	{ "security.dialog.text0", "Considerare attendibile {0} firmato e distribuito da \"{1}\"?"
				+ "\n\nAutenticit\u00e0 dell''autore verificata da: \"{2}\"" },
        { "security.dialog.text0a", "Considerare attendibile {0} firmato e distribuito da \"{1}\"?"
                                 + "\n\nL''autenticit\u00e0 dell''autore non pu\u00f2 essere verificata." },
  	{ "security.dialog.timestamp.text1", "The {0} was signed on {1}."},
	{ "security.dialog_https.text0", "Si desidera accettare il certificato del sito Web \"{0}\" per effettuare lo scambio di informazioni codificate?"
				+ "\n\nAutenticit\u00e0 dell''autore verificata da: \"{1}\"" },
        { "security.dialog_https.text0a", "Si desidera accettare il certificato del sito Web \"{0}\" per effettuare lo scambio di informazioni codificate?"
                                 + "\n\nL''autenticit\u00e0 dell''autore non pu\u00f2 essere verificata." },
	{ "security.dialog.text1", "\nAttenzione: \"{0}\" dichiara che il contenuto \u00e8 sicuro. Installare o visualizzare il contenuto solo se \"{1}\" \u00e8 considerato attendibile.\n\n" },
	{ "security.dialog.unknown.issuer", "Autorit\u00e0 emittente sconosciuta" },
	{ "security.dialog.unknown.subject", "Soggetto sconosciuto" },
	{ "security.dialog.certShowName", "{0} ({1})" },
	{ "security.dialog.rootCANotValid", "Il certificato di protezione \u00e8 stato rilasciato da una societ\u00e0 non attendibile." },
	{ "security.dialog.rootCAValid", "Il certificato di protezione \u00e8 stato rilasciato da una societ\u00e0 attendibile." },
	{ "security.dialog.timeNotValid", "Il certificato di protezione \u00e8 scaduto oppure non \u00e8 ancora valido." },
	{ "security.dialog.timeValid", " Il certificato di protezione non \u00e8 scaduto ed \u00e8 ancora valido." },
	{ "security.dialog.timeValidTS", "The security certificate was valid when the {0} was signed." },
	{ "security.dialog.buttonAlways", "Sempre" },
	{ "security.dialog.buttonAlways.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "security.dialog.buttonYes", "S\u00ec" },
	{ "security.dialog.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "security.dialog.buttonNo", "No" },
	{ "security.dialog.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "security.dialog.buttonViewCert", "Pi\u00f9 dettagli" },
	{ "security.dialog.buttonViewCert.acceleratorKey", new Integer(KeyEvent.VK_P)},

        { "security.badcert.caption", "Attenzione - Protezione" },
        { "security.badcert.https.text", "Impossibile convalidare il certificato SSL.\nQuesto {0} non verr\u00e0 eseguito." },
        { "security.badcert.config.text", "La configurazione di protezione non consente di convalidare questo certificato. Questo {0} non verr\u00e0 eseguito." },
        { "security.badcert.text", "Impossibile convalidare il certificato. Questo {0} non verr\u00e0 eseguito." },
        { "security.badcert.viewException", "Mostra eccezione" },
        { "security.badcert.viewException.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "security.badcert.viewCert", "Pi\u00f9 dettagli" },
        { "security.badcert.viewCert.acceleratorKey", new Integer(KeyEvent.VK_P)},

	{ "cert.dialog.caption", "Dettagli - Certificato" },
	{ "cert.dialog.certpath", "Percorso certificato" },
	{ "cert.dialog.field.Version", "Versione" },
	{ "cert.dialog.field.SerialNumber", "Numero di serie" },
	{ "cert.dialog.field.SignatureAlg", "Algoritmo della firma" },
	{ "cert.dialog.field.Issuer", "Emesso da" },
	{ "cert.dialog.field.EffectiveDate", "Data effettiva" },
	{ "cert.dialog.field.ExpirationDate", "Data di scadenza" },
	{ "cert.dialog.field.Validity", "Validit\u00e0" },
	{ "cert.dialog.field.Subject", "Soggetto" },
	{ "cert.dialog.field.Signature", "Firma" },
	{ "cert.dialog.field", "Campo" },
	{ "cert.dialog.value", "Valore" },
	{ "cert.dialog.close", "Chiudi" },
	{ "cert.dialog.close.acceleratorKey", new Integer(KeyEvent.VK_C) },

	{ "clientauth.password.dialog.buttonOK", "OK" },
	{ "clientauth.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.password.dialog.buttonCancel", "Annulla" },
	{ "clientauth.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "clientauth.password.dialog.caption", "\u00c8 richiesta una password - Archivio chiavi per l'autenticazione del client" },
	{ "clientauth.password.dialog.text", "Immettere una password per accedere all'archivio chiavi per l'autenticazione del client:\n" },
        { "clientauth.password.dialog.error.caption", "Errore - Archivio chiavi per l'autenticazione del client" },
        { "clientauth.password.dialog.error.text", "<html><b>Errore nell'accesso all'archivio chiavi</b></html>L'archivio chiavi \u00e8 stato manomesso o \u00e8 stata specificata una password non corretta." },

	{ "clientauth.certlist.dialog.buttonOK", "OK" },
	{ "clientauth.certlist.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
	{ "clientauth.certlist.dialog.buttonCancel", "Annulla" },
	{ "clientauth.certlist.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "clientauth.certlist.dialog.buttonDetails", "Dettagli" },
	{ "clientauth.certlist.dialog.buttonDetails.acceleratorKey", new Integer(KeyEvent.VK_P)},
	{ "clientauth.certlist.dialog.caption", "Autenticazione client" },
	{ "clientauth.certlist.dialog.text", "Il sito Web a cui si cerca di connettersi richiede un'identificazione.\nSelezionare il certificato da usare per la connessione.\n" },

	{ "dialogfactory.confirmDialogTitle", "Conferma necessaria - Java" },
	{ "dialogfactory.inputDialogTitle", "Informazione necessaria - Java" },
	{ "dialogfactory.messageDialogTitle", "Messaggio - Java" },
	{ "dialogfactory.exceptionDialogTitle", "Errore - Java" },
	{ "dialogfactory.optionDialogTitle", "Opzione - Java" },
	{ "dialogfactory.aboutDialogTitle", "Informazioni su - Java" },
	{ "dialogfactory.confirm.yes", "S\u00ec" },
	{ "dialogfactory.confirm.yes.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "dialogfactory.confirm.no", "No" },
	{ "dialogfactory.confirm.no.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "dialogfactory.moreInfo", "Pi\u00f9 dettagli" },
	{ "dialogfactory.moreInfo.acceleratorKey", new Integer(KeyEvent.VK_P)},
	{ "dialogfactory.lessInfo", "Meno dettagli" },
	{ "dialogfactory.lessInfo.acceleratorKey", new Integer(KeyEvent.VK_M)},
	{ "dialogfactory.java.home.link", "http://www.java.com" },
	{ "dialogfactory.general_error", "<html><b>Eccezione generale</b></html>" },
	{ "dialogfactory.net_error", "<html><b>Eccezione di rete</b></html>" },
	{ "dialogfactory.security_error", "<html><b>Eccezione di protezione</b></html>" },
	{ "dialogfactory.ext_error", "<html><b>Eccezione del pacchetto opzionale</b></html>" },
	{ "dialogfactory.user.selected", "Utente selezionato: {0}" },
	{ "dialogfactory.user.typed", "Utente immesso: {0}" },

        { "deploycertstore.cert.loading", "Caricamento certificati Deployment da {0}" },
        { "deploycertstore.cert.loaded", "Certificati Deployment caricati da {0}" },
        { "deploycertstore.cert.saving", "Salvataggio certificati Deployment in {0}" },
        { "deploycertstore.cert.saved", "Certificati Deployment salvati in {0}" },
        { "deploycertstore.cert.adding", "Aggiunta del certificato nell''archivio certificati Deployment permanenti" },
        { "deploycertstore.cert.added", "Aggiunto alias {0} del certificato all''archivio certificati Deployment permanenti" },
        { "deploycertstore.cert.removing", "Rimozione del certificato dall''archivio certificati Deployment permanenti" },
        { "deploycertstore.cert.removed", "Rimosso alias {0} del certificato dall''archivio certificati Deployment permanenti" },
        { "deploycertstore.cert.instore", "Controllo del certificato nell''archivio certificati Deployment permanenti" },
        { "deploycertstore.cert.canverify", "Controllare se \u00e8 possibile verificare il certificato utilizzando i certificati nell''archivio certificati Deployment permanenti" },
        { "deploycertstore.cert.iterator", "Ottenere l''iteratore certificati nell'archivio certificati Deployment permanenti" },
        { "deploycertstore.cert.getkeystore", "Ottenere l''oggetto archivio chiavi dell''archivio certificati Deployment permanenti" },

        { "httpscertstore.cert.loading", "Caricamento dei certificati Deployment SSL da {0}" },
        { "httpscertstore.cert.loaded", "Certificati Deployment SSL caricati da {0}" },
        { "httpscertstore.cert.saving", "Salvataggio certificati Deployment SSL in {0}" },
        { "httpscertstore.cert.saved", "Certificati Deployment SSL salvati in {0}" },
        { "httpscertstore.cert.adding", "Aggiunta certificato SSL nell''archivio certificati Deployment permanenti" },
        { "httpscertstore.cert.added", "Aggiunto alias {0} del certificato SSL nell''archivio certificati Deployment permanenti" },
        { "httpscertstore.cert.removing", "Rimozione certificato SSL dall''archivio certificati Deployment permanenti" },
        { "httpscertstore.cert.removed", "Rimosso alias {0} del certificato SSL dall''archivio certificati Deployment permanenti" },
        { "httpscertstore.cert.instore", "Controllo del certificato SSL nell''archivio certificati Deployment permanenti" },
        { "httpscertstore.cert.canverify", "Controllare se \u00e8 possibile verificare il certificato SSL utilizzando i certificati nell''archivio certificati Deployment permanenti" },
        { "httpscertstore.cert.iterator", "Ottenere l''iteratore certificati SSL nell''archivio certificati Deployment permanenti" },
        { "httpscertstore.cert.getkeystore", "Ottenere l''oggetto archivio chiavi dell''archivio certificati Deployment permanenti" },

	{ "rootcertstore.cert.loading", "Caricamento certificati della CA principale da {0}" },
	{ "rootcertstore.cert.loaded", "Certificati della CA principale caricati da {0}" },
	{ "rootcertstore.cert.noload", "Impossibile trovare il file dei certificati della CA principale: {0}" },
	{ "rootcertstore.cert.saving", "Salvataggio certificati della CA principale in {0}" },
	{ "rootcertstore.cert.saved", "Certificati della CA principale salvati in {0}" },
	{ "rootcertstore.cert.adding", "Aggiunta certificato nell''archivio certificati della CA principale" },
	{ "rootcertstore.cert.added", "Aggiunto alias {0} del certificato nell''archivio certificati della CA principale" },
	{ "rootcertstore.cert.removing", "Rimozione certificato dall''archivio certificati della CA principale" },
        { "rootcertstore.cert.removed", "Rimosso alias {0} del certificato dall''archivio certificati della CA principale" },
	{ "rootcertstore.cert.instore", "Controllo del certificato nell''archivio certificati della CA principale" },
	{ "rootcertstore.cert.canverify", "Controllare se \u00e8 possibile verificare il certificato utilizzando i certificati nell''archivio certificati della CA principale" },
	{ "rootcertstore.cert.iterator", "Ottenere l''iteratore certificati nell''archivio certificati della CA principale" },
	{ "rootcertstore.cert.getkeystore", "Ottenere l''oggetto archivio chiavi dell''archivio certificati della CA principale" },
	{ "rootcertstore.cert.verify.ok", "Il certificato \u00e8 stato verificato con i certificati della CA principale" },
	{ "rootcertstore.cert.verify.fail", "Impossibile verificare il certificato con i certificati della CA principale" },
	{ "rootcertstore.cert.tobeverified", "Certificato da verificare:\n{0}" },
	{ "rootcertstore.cert.tobecompared", "Confronto certificato con certificato della CA principale:\n{0}" },

        { "roothttpscertstore.cert.loading", "Caricamento dei certificati SSL della CA principale da {0}" },
        { "roothttpscertstore.cert.loaded", "Certificati SSL della CA principale caricati da {0}" },
        { "roothttpscertstore.cert.noload", "Impossibile trovare il file dei certificati SSL della CA principale: {0}" },
        { "roothttpscertstore.cert.saving", "Salvataggio certificati SSL della CA principale in {0}" },
        { "roothttpscertstore.cert.saved", "Certificati SSL della CA principale salvati in {0}" },
        { "roothttpscertstore.cert.adding", "Aggiunta certificato nell''archivio dei certificati SSL della CA principale" },
        { "roothttpscertstore.cert.added", "Aggiunto certificato nell''archivio certificati SSL della CA principale come alias {0}" },
        { "roothttpscertstore.cert.removing", "Rimozione certificato dall''archivio certificati SSL della CA principale" },
        { "roothttpscertstore.cert.removed", "Rimosso certificato dall''archivio certificati SSL della CA principale come alias {0}" },
        { "roothttpscertstore.cert.instore", "Controllo del certificato nell''archivio certificati SSL della CA principale" },
        { "roothttpscertstore.cert.canverify", "Controllare se \u00e8 possibile verificare il certificato utilizzando i certificati nell''archivio certificati SSL della CA principale" },
        { "roothttpscertstore.cert.iterator", "Ottenere l''iteratore certificati nell''archivio certificati SSL della CA principale" },
        { "roothttpscertstore.cert.getkeystore", "Ottenere l''oggetto archivio chiavi dell''archivio certificati SSL della CA principale" },
        { "roothttpscertstore.cert.verify.ok", "Il certificato \u00e8 stato verificato con i certificati SSL della CA principale" },
        { "roothttpscertstore.cert.verify.fail", "Impossibile verificare il certificato con i certificati SSL della CA principale" },
	{ "roothttpscertstore.cert.tobeverified", "Certificato da verificare:\n{0}" },
	{ "roothttpscertstore.cert.tobecompared", "Confronto certificato con certificato SSL della CA principale:\n{0}" },

        { "sessioncertstore.cert.loading", "Caricamento certificati dall''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.loaded", "Certificati caricati dall''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.saving", "Salvataggio certificati nell''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.saved", "Certificati salvati nell''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.adding", "Aggiunta del certificato nell''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.added", "Certificato aggiunto nell''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.removing", "Rimozione del certificato dall''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.removed", "Certificato rimosso dall''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.instore", "Controllo del certificato nell''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.canverify", "Controllare se \u00e8 possibile verificare il certificato utilizzando i certificati nell''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.iterator", "Ottenere l''iteratore certificati nell''archivio certificati della sessione Deployment" },
        { "sessioncertstore.cert.getkeystore", "Ottenere l''oggetto archivio chiavi dell''archivio certificati della sessione Deployment" },

        { "iexplorer.cert.loading", "Caricamento certificati dall''archivio certificati di Internet Explorer {0}" },
        { "iexplorer.cert.loaded", "Certificati caricati dall''archivio certificati di Internet Explorer {0}" },
        { "iexplorer.cert.instore", "Controllo della presenza del certificato nell''archivio certificati di Internet Explorer {0}" },
        { "iexplorer.cert.canverify", "Controllare se \u00e8 possibile verificare il certificato utilizzando i certificati nell''archivio certificati di Internet Explorer {0}" },
        { "iexplorer.cert.iterator", "Ottenere l''iteratore certificati nell''archivio certificati di Internet Explorer {0}" },
        { "iexplorer.cert.verify.ok", "Il certificato \u00e8 stato verificato con i certificati di Internet Explorer {0}" },
        { "iexplorer.cert.verify.fail", "Impossibile verificare il certificato con i certificati di Internet Explorer {0}" },
        { "iexplorer.cert.tobeverified", "Certificato da verificare:\n{0}" },
        { "iexplorer.cert.tobecompared", "Confronto certificato con certificato di Internet Explorer {0}:\n{1}" },
        { "mozilla.cert.loading", "Caricamento certificati dall''archivio certificati di Mozilla {0}" },
        { "mozilla.cert.loaded", "Certificati caricati dall''archivio certificati di Mozilla {0}" },
        { "mozilla.cert.instore", "Controllo della presenza del certificato nell''archivio certificati di Mozilla {0}" },
        { "mozilla.cert.canverify", "Controllare se \u00e8 possibile verificare il certificato utilizzando i certificati nell''archivio certificati di Mozilla {0}" },
        { "mozilla.cert.iterator", "Ottenere l''iteratore certificati nell''archivio certificati di Mozilla {0}" },
        { "mozilla.cert.verify.ok", "Il certificato \u00e8 stato verificato con i certificati di Mozilla {0}" },
        { "mozilla.cert.verify.fail", "Impossibile verificare il certificato con i certificati di Mozilla {0}" },
        { "mozilla.cert.tobeverified", "Certificato da verificare:\n{0}" },
        { "mozilla.cert.tobecompared", "Confronto certificato con certificato di Mozilla {0}:\n{1}" },

        { "browserkeystore.jss.no", "Pacchetto JSS non trovato" },
        { "browserkeystore.jss.yes", "Pacchetto JSS caricato" },
        { "browserkeystore.jss.notconfig", "JSS non \u00e8 configurato" },
        { "browserkeystore.jss.config", "JSS \u00e8 configurato" },
        { "browserkeystore.mozilla.dir", "Accesso alle chiavi e al certificato nel profilo utente di Mozilla: {0}" },
        { "browserkeystore.password.dialog.buttonOK", "OK" },
        { "browserkeystore.password.dialog.buttonOK.acceleratorKey", new Integer(KeyEvent.VK_O)},
        { "browserkeystore.password.dialog.buttonCancel", "Annulla" },
        { "browserkeystore.password.dialog.buttonCancel.acceleratorKey", new Integer(KeyEvent.VK_A)},
        { "browserkeystore.password.dialog.caption", "Password richiesta" },
        { "browserkeystore.password.dialog.text", "Inserire la password per {0}:\n" },
        { "mozillamykeystore.priv.notfound", "impossibile trovare la chiave privata per il certificato: {0}" },

	{ "hostnameverifier.automation.ignoremismatch", "Automazione: ignora mancata corrispondenza nome host" },

	{ "trustdecider.check.basicconstraints", "Controllo limitazioni di base non riuscito nel certificato" },
	{ "trustdecider.check.leafkeyusage", "Controllo utilizzo chiave foglia non riuscito nel certificato" },
	{ "trustdecider.check.signerkeyusage", "Controllo uso chiave firmatario non riuscito nel certificato" },
	{ "trustdecider.check.extensions", "Controllo estensioni critiche non riuscito nel certificato" },
	{ "trustdecider.check.signature", "Controllo firma non riuscito nel certificato" },
	{ "trustdecider.check.basicconstraints.certtypebit", "Controllo bit tipo Netscape non riuscito nel certificato" },
	{ "trustdecider.check.basicconstraints.extensionvalue", "Controllo valore estensione Netscape non riuscito nel certificato" },
	{ "trustdecider.check.basicconstraints.bitvalue", "Controllo valore 5,6,7 bit Netscape non riuscito nel certificato" },
	{ "trustdecider.check.basicconstraints.enduser", "Controllo azione utente finale come CA non riuscito nel certificato" },
	{ "trustdecider.check.basicconstraints.pathlength", "Controllo limitazioni lunghezza percorso non riuscito nel certificato" },
	{ "trustdecider.check.leafkeyusage.length", "Controllo lunghezza uso chiave non riuscito nel certificato" },
	{ "trustdecider.check.leafkeyusage.digitalsignature", "Controllo firma digitale non riuscito nel certificato" },
	{ "trustdecider.check.leafkeyusage.extkeyusageinfo", "Controllo info uso chiave estensione non riuscito nel certificato" },
        { "trustdecider.check.leafkeyusage.tsaextkeyusageinfo", "Controllo info uso chiave estensione TSA non riuscito nel certificato"},
	{ "trustdecider.check.leafkeyusage.certtypebit", "Controllo bit tipo Netscape non riuscito nel certificato" },
	{ "trustdecider.check.signerkeyusage.lengthandbit", "Controllo lunghezza e bit non riuscito nel certificato" },
	{ "trustdecider.check.signerkeyusage.keyusage", "Controllo uso chiave non riuscito nel certificato" },
	{ "trustdecider.check.canonicalize.updatecert", "Aggiornamento certificato principale con il certificato nel file cacerts" },
	{ "trustdecider.check.canonicalize.missing", "Aggiunta certificato principale mancante" },
	{ "trustdecider.check.gettrustedcert.find", "Ricerca CA principale valida nel file cacerts" },
	{ "trustdecider.check.gettrustedissuercert.find", "Ricerca CA principale valida mancante nel file cacerts" },
        { "trustdecider.check.timestamping.no", "Informazioni su data e ora non disponibili" },
        { "trustdecider.check.timestamping.yes", "Sono disponibili informazioni su data e ora" },
        { "trustdecider.check.timestamping.tsapath", "Iniziare a controllare il percorso del certificato TSA" },
        { "trustdecider.check.timestamping.inca", "Anche se il certificato \u00e8 scaduto, la sua indicazione di data e ora rientra in un periodo valido e possiede un TSA valido" },
        { "trustdecider.check.timestamping.notinca", "Il certificato \u00e8 scaduto, ma il TSA non \u00e8 valido" },
        { "trustdecider.check.timestamping.valid", "Il certificato \u00e8 scaduto ma l''indicazione di data e ora rientra in un periodo valido" },
        { "trustdecider.check.timestamping.invalid", "Il certificato \u00e8 scaduto e l''indicazione di data e ora rientra in un periodo nonvalido" },
        { "trustdecider.check.timestamping.need", "Il certificato \u00e8 scaduto, \u00e8 necessario controllare le informazioni di data e ora" },
        { "trustdecider.check.timestamping.noneed", "Il certificato non \u00e8 scaduto, non \u00e8 necessario controllare le informazioni di data e ora" },
        { "trustdecider.check.timestamping.notfound", "Nuova API per le indicazioni di data e ora non trovata" },
	{ "trustdecider.user.grant.session", "L''utente ha concesso i privilegi al codice solo per questa sessione" },
	{ "trustdecider.user.grant.forever", "L''utente ha concesso i privilegi al codice senza limitazioni di tempo" },
	{ "trustdecider.user.deny", "L''utente ha negato i privilegi al codice" },
	{ "trustdecider.automation.trustcert", "Automazione: certificato RSA attendibile per la firma" },
	{ "trustdecider.code.type.applet", "applet" },
	{ "trustdecider.code.type.application", "applicazione" },
	{ "trustdecider.code.type.extension", "estensione" },
	{ "trustdecider.code.type.installer", "programma di installazione" },
	{ "trustdecider.user.cannot.grant.any", "La configurazione di protezione non consente di concedere le autorizzazioni per nuovi certificati" },
	{ "trustdecider.user.cannot.grant.notinca", "La configurazione di protezione non consente di concedere le autorizzazioni per i certificati autofirmati" },
	{ "x509trustmgr.automation.ignoreclientcert", "Automazione: ignora certificato client non attendibile" },
	{ "x509trustmgr.automation.ignoreservercert", "Automazione: ignora certificato server non attendibile" },

	{ "net.proxy.text", "Proxy: " },
	{ "net.proxy.override.text", "Sostituzioni proxy: " },
	{ "net.proxy.configuration.text", "Configurazione proxy: " },
	{ "net.proxy.type.browser", "Configurazione del proxy del browser" },
	{ "net.proxy.type.auto", "Configurazione automatica proxy" },
	{ "net.proxy.type.manual", "Configurazione manuale" },
	{ "net.proxy.type.none", "Nessun proxy" },
	{ "net.proxy.type.user", "Le impostazioni proxy del browser sono state sostituite dall'utente." },
	{ "net.proxy.loading.ie", "Caricamento della configurazione proxy da Internet Explorer ..."},
	{ "net.proxy.loading.ns", "Caricamento della configurazione proxy da Netscape Navigator ..."},
	{ "net.proxy.loading.userdef", "Caricamento della configurazione proxy definita dall''utente ..."},
	{ "net.proxy.loading.direct", "Caricamento della configurazione proxy diretta ..."},
	{ "net.proxy.loading.manual", "Caricamento della configurazione proxy manuale ..."},
	{ "net.proxy.loading.auto",   "Caricamento della configurazione proxy automatica ..."},
	{ "net.proxy.loading.browser",   "Caricamento della configurazione proxy del browser ..."},
	{ "net.proxy.loading.manual.error", "Impossibile utilizzare la configurazione proxy manuale - viene usata la configurazione diretta"},
	{ "net.proxy.loading.auto.error", "Impossibile utilizzare la configurazione proxy automatica - viene usata la configurazione manuale"},
	{ "net.proxy.loading.done", "Fine."},
	{ "net.proxy.browser.pref.read", "Lettura del file preferenze utente da {0}"},
	{ "net.proxy.browser.proxyEnable", "    Abilita proxy: {0}"},
	{ "net.proxy.browser.proxyList",     "    Elenco proxy: {0}"},
	{ "net.proxy.browser.proxyOverride", "    Ignora proxy: {0}"},
	{ "net.proxy.browser.autoConfigURL", "    URL configurazione automatica: {0}"},
	{ "net.proxy.browser.smartConfig", "Eseguire il ping sul server proxy {0} sulla porta {1}"},
	{ "net.proxy.browser.connectionException", "Impossibile raggiungere il server proxy {0} sulla porta {1} "},
	{ "net.proxy.ns6.regs.exception", "Errore durante la lettura del file di registro: {0}"},
	{ "net.proxy.pattern.convert", "Conversione dell''elenco bypass proxy in espressione regolare: "},
	{ "net.proxy.pattern.convert.error", "Impossibile convertire l'elenco bypass proxy in espressione regolare - ignorare"},
	{ "net.proxy.auto.download.ins", "Download file INS da {0}" },
	{ "net.proxy.auto.download.js", "Download file proxy automatico da {0}" },
	{ "net.proxy.auto.result.error", "Impossibile determinare le impostazioni proxy dalla valutazione - viene usata la configurazione diretta"},
	{ "net.proxy.service.not_available", "Servizio proxy non disponibile per {0} - predefinito su DIRECT" },
	{ "net.proxy.error.caption", "Errore - Configurazione proxy" },
	{ "net.proxy.nsprefs.error", "<html><b>Impossibile recuperare le impostazioni proxy</b></html>Viene usata un'altra configurazione proxy.\n" },
	{ "net.proxy.connect", "Connessione a {0} con proxy={1}" },

	{ "net.authenticate.caption", "Password necessaria - Rete"},
	{ "net.authenticate.label", "<html><b>Digitare nome utente e password:</b></html>"},
	{ "net.authenticate.resource", "Risorsa:" },
	{ "net.authenticate.username", "Nome utente:" },
	{ "net.authenticate.username.mnemonic", "VK_U" },
	{ "net.authenticate.password", "Password:" },
	{ "net.authenticate.password.mnemonic", "VK_P" },
	{ "net.authenticate.firewall", "Server:" },
	{ "net.authenticate.domain", "Dominio:"},
	{ "net.authenticate.domain.mnemonic", "VK_D" },
	{ "net.authenticate.realm", "Settore:" },
	{ "net.authenticate.scheme", "Schema:" },
	{ "net.authenticate.unknownSite", "Sito sconosciuto" },

	{ "net.cookie.cache", "Cache cookie: " },
	{ "net.cookie.server", "Richiesta del server {0} di impostare i cookie con \"{1}\"" },
	{ "net.cookie.connect", "Connessione {0} con cookie \"{1}\"" },
	{ "net.cookie.ignore.setcookie", "Servizio cookie non disponibile - ignora \"Set-Cookie\"" },
	{ "net.cookie.noservice", " Servizio cookie non disponibile - utilizza cache per determinare \"Cookie\"" },

	{ "about.java.version", "Versione {0} (build {1})"},
	{ "about.prompt.info", "Per ulteriori informazioni sulla tecnologia Java e per informazioni sulle applicazioni Java disponibili, visitare il sito"},
	{ "about.home.link", "http://www.java.com"},
	{ "about.option.close", "Chiudi"},
	{ "about.option.close.acceleratorKey", new Integer(KeyEvent.VK_C) },
        {"about.copyright", "Copyright 2004 Sun Microsystems, Inc."},
	{ "about.legal.note", "Tutti i diritti riservati. L'uso \u00e8 soggetto ai termini della licenza. "},


	{ "cert.remove_button", "Rimuovi" },
        { "cert.remove_button.mnemonic", "VK_M" },
        { "cert.import_button", "Importa" },
        { "cert.import_button.mnemonic", "VK_I" },
        { "cert.export_button", "Esporta" },
        { "cert.export_button.mnemonic", "VK_E" },
        { "cert.details_button", "Dettagli" },
        { "cert.details_button.mnemonic", "VK_D" },
        { "cert.viewcert_button", "Visualizza certificato" },
        { "cert.viewcert_button.mnemonic", "VK_V" },
        { "cert.close_button", "Chiudi" },
        { "cert.close_button.mnemonic", "VK_C" },
        { "cert.type.trusted_certs", "Certificati accreditati" },
        { "cert.type.secure_site", "Sito sicuro" },
        { "cert.type.client_auth", "Autenticazione client" },
        { "cert.type.signer_ca", "CA firmataria" },
        { "cert.type.secure_site_ca", "CA sito sicuro" },
        { "cert.rbutton.user", "Utente" },
        { "cert.rbutton.system", "Sistema" },
        { "cert.settings", "Certificati" },
        { "cert.dialog.import.error.caption", "Errore - importazione certificato" },
        { "cert.dialog.export.error.caption", "Errore - esportazione certificato" },
        { "cert.dialog.import.format.text", "<html><b>Formato di file non riconosciuto</b></html>Nessun certificato verr\u00e0 importato." },
        { "cert.dialog.export.password.text", "<html><b>Password non corretta</b></html>La password inserita non \u00e8 corretta." },
        { "cert.dialog.import.file.text", "<html><b>Il file non esiste</b></html>Nessun certificato verr\u00e0 importato." },
        { "cert.dialog.import.password.text", "<html><b>Password non corretta</b></html>La password inserita non \u00e8 corretta." },
        { "cert.dialog.password.caption", "Password" },
        { "cert.dialog.password.import.caption", "Password richiesta - Importazione" },
        { "cert.dialog.password.export.caption", "Password richiesta - Esportazione" },
        { "cert.dialog.password.text", "Inserire una password per accedere al file:\n" },
        { "cert.dialog.exportpassword.text", "Inserire una password per accedere alla chiave privata nell'archivio chiavi per l'autenticazione client:\n" },
        { "cert.dialog.savepassword.text", "Inserire una password per il salvataggio del file delle chiavi:\n" },
        { "cert.dialog.password.okButton", "OK" },
        { "cert.dialog.password.cancelButton", "Annulla" },
        { "cert.dialog.export.error.caption", "Errore - esportazione certificato" },
        { "cert.dialog.export.text", "<html><b>Impossibile esportare</b></html>Nessun certificato \u00e8 stato esportato." },
        { "cert.dialog.remove.text", "Eliminare i certificati?" },
	{ "cert.dialog.remove.caption", "Rimuovi certificato" },
	{ "cert.dialog.issued.to", "Rilasciato a" },
	{ "cert.dialog.issued.by", "Rilasciato da" },
	{ "cert.dialog.user.level", "Utente" },
	{ "cert.dialog.system.level", "Sistema" },
	{ "cert.dialog.certtype", "Tipo certificato: "},

	{ "controlpanel.jre.platformTableColumnTitle","Piattaforma"},
	{ "controlpanel.jre.productTableColumnTitle","Prodotto" },
	{ "controlpanel.jre.locationTableColumnTitle","Posizione" },
	{ "controlpanel.jre.pathTableColumnTitle","Percorso" },
	{ "controlpanel.jre.enabledTableColumnTitle", "Abilitato" },

	{ "jnlp.jre.title", "Impostazioni runtime JNLP" },
	{ "jnlp.jre.versions", "Versioni del runtime Java" },
	{ "jnlp.jre.choose.button", "Scegli" },
	{ "jnlp.jre.find.button", "Trova" },
	{ "jnlp.jre.add.button", "Aggiungi" },
	{ "jnlp.jre.remove.button", "Rimuovi" },
	{ "jnlp.jre.ok.button", "OK" },
	{ "jnlp.jre.cancel.button", "Annulla" },
	{ "jnlp.jre.choose.button.mnemonic", "VK_S" },
	{ "jnlp.jre.find.button.mnemonic", "VK_T" },
	{ "jnlp.jre.add.button.mnemonic", "VK_G" },
	{ "jnlp.jre.remove.button.mnemonic", "VK_R" },
	{ "jnlp.jre.ok.button.mnemonic", "VK_O" },
	{ "jnlp.jre.cancel.button.mnemonic", "VK_A" },

	{ "find.dialog.title", "Ricerca JRE"},
	{ "find.title", "Ambienti runtime Java"},
	{ "find.cancelButton", "Annulla"},
	{ "find.prevButton", "Indietro"},
	{ "find.nextButton", "Avanti"},
	{ "find.cancelButtonMnemonic", "VK_A"},
	{ "find.prevButtonMnemonic", "VK_I"},
	{ "find.nextButtonMnemonic", "VK_V"},
	{ "find.intro", "Per avviare le applicazioni, Java Web Start deve conoscere la posizione in cui sono installati gli ambienti runtime Java (JRE).\n\n\u00c8 possibile selezionare un ambiente JRE tra quelli individuati, oppure selezionare la directory del file system in cui ricercare gli ambienti JRE." },

	{ "find.searching.title", "Ricerca degli ambienti runtime Java disponibili, pu\u00f2 richiedere diversi minuti." },
	{ "find.searching.prefix", "controllo: " },
	{ "find.foundJREs.title", "Sono stati individuati i seguenti ambienti runtime Java, fare clic su Avanti per aggiungerli" },
	{ "find.noJREs.title", "Impossibile individuare un ambiente runtime Java, fare clic su Indietro per selezionare una differente posizione per la ricerca" },

	// Each line in the property_file_header must start with "#"
        { "config.property_file_header", "# Propriet\u00e0 di distribuzione Java(tm)\n"
                        + "# NON MODIFICARE QUESTO FILE. Il file viene generato automaticamente.\n"
                        + "# Usare il pannello di controllo Java per modificare le propriet\u00e0." },
        { "config.unknownSubject", "Soggetto sconosciuto" },
        { "config.unknownIssuer", "Autorit\u00e0 emittente sconosciuta" },
        { "config.certShowName", "{0} ({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>URL non corretto</b></html>L'URL per la configurazione automatica del proxy non \u00e8 corretto." },
        { "config.proxy.autourl.invalid.caption", "Errore - proxy" },
	// Java Web Start Properties
	{ "APIImpl.clipboard.message.read", "L'applicazione ha richiesto l'accesso in sola lettura agli Appunti del sistema e potrebbe quindi accedere a informazioni riservate memorizzate negli Appunti. Consentire l'accesso?" },
        { "APIImpl.clipboard.message.write", "L'applicazione ha richiesto l'accesso in scrittura agli Appunti del sistema e potrebbe quindi sovrascrivere le informazioni memorizzate negli Appunti. Consentire l'accesso?" },
        { "APIImpl.file.open.message", "L'applicazione ha richiesto l'accesso in lettura al file system e potrebbe quindi accedere a informazioni riservate memorizzate nel file system. Consentire l'accesso?" },
        { "APIImpl.file.save.fileExist", "{0} esiste gi\u00e0.\n Sostituirlo?" },
        { "APIImpl.file.save.fileExistTitle", "Il file esiste gi\u00e0" },
        { "APIImpl.file.save.message", "L'applicazione ha richiesto l'accesso in lettura e scrittura a un file del file system locale e pertanto, se autorizzata, avrebbe accesso solamente al/i file selezionati nella finestra di dialogo seguente. Consentire l'accesso?" },
        { "APIImpl.persistence.accessdenied", "Accesso negato all''area di memorizzazione permanente per l''URL {0}" },
        { "APIImpl.persistence.filesizemessage", "Lunghezza massima file superata" },
        { "APIImpl.persistence.message", "L''applicazione ha richiesto spazio di memorizzazione aggiuntivo sul disco locale. Attualmente l''area di memorizzazione massima \u00e8 di {1} byte: l''applicazione ha richiesto di aumentarla a {0} byte. Consentire l''aumento?" },
        { "APIImpl.print.message", "L'applicazione ha richiesto l'accesso alla stampante predefinita e pertanto, se autorizzata, potrebbe accedere in scrittura alla stampante. Consentire l'accesso?" },
	{ "APIImpl.extended.fileOpen.message1", "L'applicazione ha richiesto l'accesso in lettura e scrittura ai seguenti file del file system locale:"},
	{ "APIImpl.extended.fileOpen.message2", "Se autorizzata, l'applicazione avrebbe accesso solamente ai file elencati qui sopra. Consentire l'accesso?"},
        { "APIImpl.securityDialog.no", "No" },
        { "APIImpl.securityDialog.remember", "Non visualizzare pi\u00f9 il controllo" },
        { "APIImpl.securityDialog.title", "Controllo protezione" },
        { "APIImpl.securityDialog.yes", "S\u00ec" },
        { "Launch.error.installfailed", "Installazione non riuscita" },
        { "aboutBox.closeButton", "Chiudi" },
        { "aboutBox.versionLabel", "Versione {0} (build {1})" },
        { "applet.failedToStart", "Impossibile avviare l''applet: {0}" },
        { "applet.initializing", "Inizializzazione applet in corso" },
        { "applet.initializingFailed", "Impossibile inizializzare l''applet: {0}" },
        { "applet.running", "Esecuzione in corso..." },
        { "java48.image", "image/java48.png" },
        { "java32.image", "image/java32.png" },
        { "extensionInstall.rebootMessage", "Riavviare Windows per rendere effettive le modifiche.\n\nRiavviare Windows adesso?" },
        { "extensionInstall.rebootTitle", "Riavvia Windows" },
        { "install.configButton", "Configura ..." },
        { "install.configMnemonic", "VK_C" },
        { "install.errorInstalling", "Errore imprevisto durante il tentativo di installazione dell'ambiente runtime Java. Riprovare." },
        { "install.errorRestarting", "Errore imprevisto durante l'avvio. Riprovare." },
        { "install.title", "{0} - Crea collegamenti" },
        { "install.windows.both.message", "Creare un collegamento sul desktop e nel menu di avvio per\n{0}?" },
        { "install.gnome.both.message", "Creare un collegamento sul desktop e nel menu delle applicazioni per\n{0}?" },
        { "install.desktop.message", "Creare un collegamento sul desktop per\n{0}?" },
        { "install.windows.menu.message", "Creare un collegamento nel menu di avvio per\n{0}?" },
        { "install.gnome.menu.message", "Creare un collegamento nel menu delle applicazioni per\n{0}?" },
        { "install.noButton", "No" },
        { "install.noMnemonic", "VK_N" },
        { "install.yesButton", "S\u00ec" },
        { "install.yesMnemonic", "VK_S" },
        { "launch.cancel", "Annulla" },
        { "launch.downloadingJRE", "Richiesta del JRE {0} da {1} in corso" },
        { "launch.error.badfield", "Il campo {0} presenta un valore non valido: {1}" },
        { "launch.error.badfield-signedjnlp", "Il campo {0} presenta un valore non valido nel file di avvio firmato: {1}" },
        { "launch.error.badfield.download.https", "Impossibile scaricare tramite HTTPS" },
        { "launch.error.badfield.https", "\u00c8 richiesto Java 1.4+ per il supporto HTTPS" },
        { "launch.error.badjarfile", "File JAR danneggiato in {0}" },
        { "launch.error.badjnlversion", "Versione JNLP non supportata nel file di avvio: {0}. Questa versione supporta solamente la versione 1.0 e 1.5. Comunicare il problema al fornitore dell''applicazione." },
        { "launch.error.badmimetyperesponse", "Il server ha restituito un tipo MIME errato durante l''accesso alla risorsa: {0} - {1}" },
        { "launch.error.badsignedjnlp", "Convalida della firma del file di avvio non riuscita. La versione firmata non corrisponde alla versione scaricata." },
        { "launch.error.badversionresponse", "Il server ha restituito una versione errata durante l''accesso alla risorsa: {0} - {1}" },
        { "launch.error.canceledloadingresource", "Il caricamento della risorsa {0} \u00e8 stato annullato dall''utente" },
        { "launch.error.category.arguments", "Errore di argomento non valido" },
        { "launch.error.category.download", "Errore di download" },
        { "launch.error.category.launchdesc", "Errore di avvio file" },
        { "launch.error.category.memory", "Errore di memoria esaurita" },
        { "launch.error.category.security", "Errore di protezione" },
        { "launch.error.category.config", "Configurazione del sistema" },
        { "launch.error.category.unexpected", "Errore imprevisto" },
        { "launch.error.couldnotloadarg", "Impossibile caricare il file/l''URL specificato: {0}" },
        { "launch.error.errorcoderesponse-known", "Il server ha restituito il codice errore {1} ({2}) durante l''accesso alla risorsa: {0}" },
        { "launch.error.errorcoderesponse-unknown", "Il server ha restituito il codice errore 99 (errore sconosciuto) durante l''accesso alla risorsa: {0}" },
        { "launch.error.failedexec", "Impossibile avviare la versione {0} dell''ambiente runtime Java" },
        { "launch.error.failedloadingresource", "Impossibile caricare la risorsa: {0}" },
        { "launch.error.invalidjardiff", "Impossibile applicare l''aggiornamento incrementale per la risorsa: {0}" },
        { "launch.error.jarsigning-badsigning", "Impossibile verificare la firma nella risorsa: {0}" },
        { "launch.error.jarsigning-missingentry", "Voce firmata mancante nella risorsa: {0}" },
        { "launch.error.jarsigning-missingentryname", "Voce firmata mancante: {0}" },
        { "launch.error.jarsigning-multicerts", "\u00c8 stato utilizzato pi\u00f9 di un certificato per firmare la risorsa: {0}" },
        { "launch.error.jarsigning-multisigners", "\u00c8 presente pi\u00f9 di una firma nella voce della risorsa: {0}" },
        { "launch.error.jarsigning-unsignedfile", "Trovata una voce non firmata nella risorsa: {0}" },
        { "launch.error.missingfield", "Il seguente campo obbligatorio non \u00e8 contenuto nel file di avvio: {0}" },
        { "launch.error.missingfield-signedjnlp", "Il seguente campo obbligatorio non \u00e8 contenuto nel file di avvio firmato: {0}" },
        { "launch.error.missingjreversion", "Nessuna versione JRE trovata nel file di avvio per questo sistema" },
        { "launch.error.missingversionresponse", "Il server non ha restituito il campo versione durante l''accesso alla risorsa: {0}" },
        { "launch.error.multiplehostsreferences", "Le risorse fanno riferimento a pi\u00f9 host" },
        { "launch.error.nativelibviolation", "L'uso delle librerie native richiede l'accesso illimitato al sistema" },
        { "launch.error.noJre", "L'applicazione ha richiesto una versione della piattaforma JRE che non \u00e8 attualmente installata in locale. Impossibile scaricare e installare automaticamente la versione richiesta. La versione del JRE deve essere installata manualmente.\n\n" },
        { "launch.error.wont.download.jre", "L''applicazione ha richiesto una versione della piattaforma JRE (versione {0}) che non \u00e8 attualmente installata in locale. Java Web Start non possiede le autorizzazioni per scaricare e installare automaticamente la versione richiesta. Questa versione del JRE deve essere installata manualmente." },
        { "launch.error.cant.download.jre", "L''applicazione ha richiesto una versione della piattaforma JRE (versione {0}) che non \u00e8 attualmente installata in locale. Java Web Start non \u00e8 in grado di scaricare e installare automaticamente la versione richiesta. Questa versione del JRE deve essere installata manualmente." },
        { "launch.error.cant.access.system.cache", "L'utente non possiede l'accesso in scrittura alla cache del sistema." },
	{ "launch.error.cant.access.user.cache", "L'utente non possiede l'accesso in scrittura alla cache." },
        { "launch.error.noappresources", "Nessuna risorsa di applicazione specificata per questa piattaforma. Contattare il fornitore dell'applicazione per accertarsi che la piattaforma in uso sia supportata." },
        { "launch.error.nomainclass", "Impossibile trovare la classe principale {0} in {1}" },
        { "launch.error.nomainclassspec", "Nessuna classe principale specificata per l'applicazione" },
        { "launch.error.nomainjar", "Nessun file JAR principale specificato." },
        { "launch.error.nonstaticmainmethod", "Il metodo main() deve essere statico" },
        { "launch.error.offlinemissingresource", "L'applicazione deve essere eseguita in linea in quanto alcune risorse necessarie non sono state scaricate in locale" },
        { "launch.error.parse", "Impossibile analizzare il file di avvio. Errore alla riga {0, number}." },
        { "launch.error.parse-signedjnlp", "Impossibile analizzare il file di avvio firmato. Errore alla riga {0, number}." },
        { "launch.error.resourceID", "{0}" },
        { "launch.error.resourceID-version", "({0}, {1})" },
        { "launch.error.singlecertviolation", "Le risorse JAR nel file JNLP non sono firmate dallo stesso certificato" },
        { "launch.error.toomanyargs", "Sono stati forniti troppi argomenti: {0}" },
        { "launch.error.unsignedAccessViolation", "Un'applicazione non firmata ha richiesto l'accesso illimitato al sistema" },
        { "launch.error.unsignedResource", "Risorsa non firmata: {0}" },
        { "launch.estimatedTimeLeft", "Tempo residuo stimato: {0,number,00}:{1,number,00}:{2,number,00}" },
        { "launch.extensiondownload", "Download descrittore estensione in corso ({0} rimanente)" },
        { "launch.extensiondownload-name", "Download descrittore {0} in corso ({1} rimanente)" },
        { "launch.initializing", "Inizializzazione in corso..." },
        { "launch.launchApplication", "Avvio applicazione in corso..." },
        { "launch.launchInstaller", "Avvio programma di installazione in corso..." },
        { "launch.launchingExtensionInstaller", "Esecuzione del programma di installazione in corso. Attendere prego." },
        { "launch.loadingNetProgress", "Letto {0}" },
        { "launch.loadingNetProgressPercent", "Letti {0} di {1} ({2}%)" },
        { "launch.loadingNetStatus", "Caricamento di {0} da {1}" },
        { "launch.loadingResourceFailed", "Caricamento risorsa non riuscito" },
        { "launch.loadingResourceFailedSts", "Richiesto {0}" },
        { "launch.patchingStatus", "Aggiunta della patch {0} da {1}" },
        { "launch.progressScreen", "Ricerca versione pi\u00f9 recente in corso..." },
        { "launch.stalledDownload", "Attesa inserimento dati..." },
        { "launch.validatingProgress", "Lettura voci ({0}% completato)" },
        { "launch.validatingStatus", "Convalida {0} da {1}" },
        { "launcherrordialog.abort", "Annulla" },
        { "launcherrordialog.abortMnemonic", "VK_A" },
        { "launcherrordialog.brief.continue", "Impossibile continuare l'esecuzione" },
        { "launcherrordialog.brief.details", "Dettagli" },
        { "launcherrordialog.brief.message", "Impossibile avviare l'applicazione specificata." },
        { "launcherrordialog.import.brief.message", "Impossibile importare l'applicazione specificata." },
        { "launcherrordialog.brief.messageKnown", "Impossibile avviare {0}." },
        { "launcherrordialog.import.brief.messageKnown", "Impossibile importare {0}." },
        { "launcherrordialog.brief.ok", "OK" },
        { "launcherrordialog.brief.title", "Java Web Start - {0}" },
        { "launcherrordialog.consoleTab", "Console" },
        { "launcherrordialog.errorcategory", "Categoria: {0}\n\n" },
        { "launcherrordialog.errorintro", "Si \u00e8 verificato un errore durante l'avvio/l'esecuzione dell'applicazione.\n\n" },
        { "launcherrordialog.import.errorintro", "Si \u00e8 verificato un errore durante l'importazione dell'applicazione.\n\n" },
        { "launcherrordialog.errormsg", "{0}" },
        { "launcherrordialog.errortitle", "Titolo: {0}\n" },
        { "launcherrordialog.errorvendor", "Fornitore: {0}\n" },
        { "launcherrordialog.exceptionTab", "Eccezione" },
        { "launcherrordialog.generalTab", "Generale" },
        { "launcherrordialog.genericerror", "Eccezione imprevista: {0}" },
        { "launcherrordialog.jnlpMainTab", "File di avvio principale" },
        { "launcherrordialog.jnlpTab", "File di avvio" },
        { "launcherrordialog.title", "Java Web Start - {0}" },
        { "launcherrordialog.wrappedExceptionTab", "Eccezione wrapped" },

        { "uninstall.failedMessage", "Impossibile disinstallare completamente l'applicazione." },
        { "uninstall.failedMessageTitle", "Disinstalla" },
        { "install.alreadyInstalled", "\u00c8 gi\u00e0 presente un collegamento per {0}. Creare ugualmente un collegamento?" },
        { "install.alreadyInstalledTitle", "Crea collegamento..." },
        { "install.desktopShortcutName", "{0}" },
        { "install.installFailed", "Impossibile creare un collegamento per {0}." },
        { "install.installFailedTitle", "Crea collegamento" },
        { "install.startMenuShortcutName", "{0}" },
	{ "install.startMenuUninstallShortcutName", "Disinstalla {0}" },
        { "install.uninstallFailed", "Impossibile rimuovere i collegamenti per {0}. Riprovare." },
        { "install.uninstallFailedTitle", "Rimuovi collegamenti" },

	// Mandatory Enterprize configuration not available.
	{ "enterprize.cfg.mandatory", "Impossibile eseguire il programma. Il file di sistema deployment.config stabilisce che occorre utilizzare un file di configurazione aziendale ma quello richiesto ({0}) non \u00e8 disponibile." },

	// Jnlp Cache Viewer:
	{ "jnlp.viewer.title", "Visualizzatore cache applicazioni Java" },
	{ "jnlp.viewer.all", "Tutto" },
	{ "jnlp.viewer.type", "{0}" },
	{ "jnlp.viewer.totalSize",  "Dimensione totale risorse:  {0}" },
        { "jnlp.viewer.emptyCache", "La cache {0} \u00e8 vuota"},
        { "jnlp.viewer.noCache", "La cache di sistema non \u00e8 configurata"},

	{ "jnlp.viewer.remove.btn.mnemonic", "VK_R" },
	{ "jnlp.viewer.launch.offline.btn.mnemonic", "VK_N" },
	{ "jnlp.viewer.launch.online.btn.mnemonic", "VK_L" },

	{ "jnlp.viewer.file.menu.mnemonic", "VK_F" },
	{ "jnlp.viewer.edit.menu.mnemonic", "VK_M" },
	{ "jnlp.viewer.app.menu.mnemonic", "VK_A" },
	{ "jnlp.viewer.view.menu.mnemonic", "VK_V" },
	{ "jnlp.viewer.help.menu.mnemonic", "VK_G" },

	{ "jnlp.viewer.cpl.mi.mnemonic", "VK_C" },
	{ "jnlp.viewer.exit.mi.mnemonic", "VK_E" },

	{ "jnlp.viewer.reinstall.mi.mnemonic", "VK_R" },
	{ "jnlp.viewer.preferences.mi.mnemonic", "VK_P" },

	{ "jnlp.viewer.launch.offline.mi.mnemonic", "VK_N" },
	{ "jnlp.viewer.launch.online.mi.mnemonic", "VK_L" },
	{ "jnlp.viewer.install.mi.mnemonic", "VK_I" },
	{ "jnlp.viewer.uninstall.mi.mnemonic", "VK_D" },
	{ "jnlp.viewer.remove.mi.mnemonic", "VK_R" },
	{ "jnlp.viewer.show.mi.mnemonic", "VK_M" },
	{ "jnlp.viewer.browse.mi.mnemonic", "VK_S" },

	{ "jnlp.viewer.view.0.mi.mnemonic", "VK_T" },
	{ "jnlp.viewer.view.1.mi.mnemonic", "VK_A" },
	{ "jnlp.viewer.view.2.mi.mnemonic", "VK_P" },
	{ "jnlp.viewer.view.3.mi.mnemonic", "VK_L" },
	{ "jnlp.viewer.view.4.mi.mnemonic", "VK_I" },

        { "jnlp.viewer.view.0", "Tutti i tipi" },
        { "jnlp.viewer.view.1", "Applicazioni" },
        { "jnlp.viewer.view.2", "Applet" },
        { "jnlp.viewer.view.3", "Librerie" },
        { "jnlp.viewer.view.4", "Programmi di installazione" },

	{ "jnlp.viewer.about.mi.mnemonic", "VK_F" },
	{ "jnlp.viewer.help.java.mi.mnemonic", "VK_J" },
	{ "jnlp.viewer.help.jnlp.mi.mnemonic", "VK_H" },

	{ "jnlp.viewer.remove.btn", "Rimuovi" },
	{ "jnlp.viewer.remove.1.btn", "Rimuovi {0} selezionati" },
	{ "jnlp.viewer.remove.2.btn", "Rimuovi voci selezionate" },
	{ "jnlp.viewer.uninstall.btn", "Disinstalla" },
	{ "jnlp.viewer.launch.offline.btn", "Avvia non in linea" },
	{ "jnlp.viewer.launch.online.btn", "Avvia in linea" },

        { "jnlp.viewer.file.menu", "File" },
        { "jnlp.viewer.edit.menu", "Modifica" },
        { "jnlp.viewer.app.menu", "Applicazione" },
        { "jnlp.viewer.view.menu", "Visualizza" },
        { "jnlp.viewer.help.menu", "Guida" },

	{ "jnlp.viewer.cpl.mi", "Avvia pannello di controllo Java" },
	{ "jnlp.viewer.exit.mi", "Esci" },

	{ "jnlp.viewer.reinstall.mi", "Reinstalla..." },
	{ "jnlp.viewer.preferences.mi", "Preferenze..." },

	{ "jnlp.viewer.launch.offline.mi", "Avvia non in linea" },
	{ "jnlp.viewer.launch.online.mi", "Avvia in linea" },
	{ "jnlp.viewer.install.mi", "Installa collegamenti" },
	{ "jnlp.viewer.uninstall.mi", "Disinstalla collegamenti" },
        { "jnlp.viewer.remove.0.mi", "Rimuovi" },
	{ "jnlp.viewer.remove.mi", "Rimuovi {0}" },
	{ "jnlp.viewer.show.mi", "Mostra descrittore JNLP" },
	{ "jnlp.viewer.browse.mi", "Sfoglia home page" },

	{ "jnlp.viewer.view.0.mi", "Tutti i tipi" },
	{ "jnlp.viewer.view.1.mi", "Applicazioni" },
	{ "jnlp.viewer.view.2.mi", "Applet" },
	{ "jnlp.viewer.view.3.mi", "Librerie" },
	{ "jnlp.viewer.view.4.mi", "Programmi di installazione" },

	{ "jnlp.viewer.about.mi", "Informazioni su" },
	{ "jnlp.viewer.help.java.mi", "Home page di J2SE" },
	{ "jnlp.viewer.help.jnlp.mi", "Home page di JNLP" },

        { "jnlp.viewer.app.column", "Applicazione" },
        { "jnlp.viewer.vendor.column", "Fornitore" },
        { "jnlp.viewer.type.column", "Tipo" },
        { "jnlp.viewer.size.column", "Dimensione" },
        { "jnlp.viewer.date.column", "Data" },
        { "jnlp.viewer.status.column", "Stato" },

        { "jnlp.viewer.app.column.tooltip", "L'icona e il titolo di questa applicazione, applet o estensione" },
        { "jnlp.viewer.vendor.column.tooltip", "La societ\u00e0 che distribuisce questo elemento" },
        { "jnlp.viewer.type.column.tooltip", "Il tipo di elemento" },
        { "jnlp.viewer.size.column.tooltip", "Dimensioni e risorse dell'elemento" },
        { "jnlp.viewer.date.column.tooltip", "La data in cui l'applicazione, l'applet o il programma di installazione sono stati eseguiti l'ultima volta" },
        { "jnlp.viewer.status.column.tooltip", "Un'icona che indica se e come l'elemento pu\u00f2 essere avviato" },

        { "jnlp.viewer.application", "Applicazione" },
        { "jnlp.viewer.applet", "Applet" },
        { "jnlp.viewer.extension", "Libreria" },
        { "jnlp.viewer.installer", "Programma di installazione" },

        { "jnlp.viewer.offline.tooltip",
                "Questa {0} pu\u00f2 essere avviata in modalit\u00e0 in linea o non in linea" },
        { "jnlp.viewer.online.tooltip", "Questa {0} pu\u00f2 essere avviata in linea" },
        { "jnlp.viewer.norun1.tooltip", 
                "Questa {0} pu\u00f2 essere avviata solo da un browser Web" },
        { "jnlp.viewer.norun2.tooltip", "Impossibile avviare le estensioni" },

	{ "jnlp.viewer.show.title", "Descrittore JNLP per: {0}" },

	{ "jnlp.viewer.removing", "Rimozione in corso..." },
	{ "jnlp.viewer.launching", "Avvio in corso..." },
	{ "jnlp.viewer.browsing", "Avvio del browser in corso..." },
	{ "jnlp.viewer.sorting", "Ordinamento voci in corso..." },
	{ "jnlp.viewer.searching", "Ricerca voci in corso..." },
        { "jnlp.viewer.installing", "Installazione in corso..." },

        { "jnlp.viewer.reinstall.title", "Reinstallazione applicazioni JNLP rimosse" },
	{ "jnlp.viewer.reinstallBtn", "Reinstalla applicazioni selezionate" },
	{ "jnlp.viewer.reinstallBtn.mnemonic", "VK_R" },
        { "jnlp.viewer.closeBtn", "Chiudi" },
        { "jnlp.viewer.closeBtn.mnemonic", "VK_C" },

	{ "jnlp.viewer.reinstall.column.title", "Titolo:" },
	{ "jnlp.viewer.reinstall.column.location", "Posizione:" },

	// cache size warning
	{ "jnlp.cache.warning.title", "Avvertenza dimensione cache JNLP" },
	{ "jnlp.cache.warning.message", "Attenzione: \n\nLo spazio su disco consigliato per le applicazioni\ne le risorse JNLP nella cache \u00e8 stato superato.\n\nLo spazio utilizzato attualmente \u00e8 di: {0}\nIl limite consigliato \u00e8 di: {1}\n\nUsare il pannello di controllo Java per rimuovere\nalcune applicazioni o risorse o per impostare un limite pi\u00f9 elevato." },

        // Control Panel
        { "control.panel.title", "Pannello di controllo Java" },
        { "control.panel.general", "Generale" },
        { "control.panel.security", "Protezione" },
        { "control.panel.java", "Java" },
        { "control.panel.update", "Aggiornamento" },
        { "control.panel.advanced", "Avanzate" },

        // Common Strings used in different panels.
        { "common.settings", "Impostazioni" },
        { "common.ok_btn", "OK" },
        { "common.ok_btn.mnemonic", "VK_O" },
        { "common.cancel_btn", "Annulla" },
        { "common.cancel_btn.mnemonic", "VK_A" },
        { "common.apply_btn", "Applica" },
        { "common.apply_btn.mnemonic", "VK_P" },
        { "common.add_btn", "Aggiungi" },
        { "common.add_btn.mnemonic", "VK_G" },
        { "common.remove_btn", "Rimuovi" },
        { "common.remove_btn.mnemonic", "VK_R" },

        // Network Settings Dialog
        { "network.settings.dlg.title", "Impostazioni di rete" },
        { "network.settings.dlg.border_title", " Impostazioni proxy di rete " },
        { "network.settings.dlg.browser_rbtn", "Usa impostazioni del browser" },
        { "browser_rbtn.mnemonic", "VK_B" },
        { "network.settings.dlg.manual_rbtn", "Usa server proxy" },
        { "manual_rbtn.mnemonic", "VK_P" },
        { "network.settings.dlg.address_lbl", "Indirizzo:" },
        { "network.settings.dlg.port_lbl", "Porta:" },
        { "network.settings.dlg.advanced_btn", "Avanzate..." },
        { "network.settings.dlg.advanced_btn.mnemonic", "VK_V" },
        { "network.settings.dlg.bypass_text", "Ignora server proxy per indirizzi locali" },
        { "network.settings.dlg.bypass.mnemonic", "VK_I" },
        { "network.settings.dlg.autoconfig_rbtn", "Usa script di configurazione automatica proxy" },
        { "autoconfig_rbtn.mnemonic", "VK_T" },
        { "network.settings.dlg.location_lbl", "Posizione script: " },
        { "network.settings.dlg.direct_rbtn", "Connessione diretta" },
        { "direct_rbtn.mnemonic", "VK_D" },
        { "network.settings.dlg.browser_text", "La configurazione automatica pu\u00f2 prevalere sulle impostazioni manuali. Per essere certi di utilizzare le impostazioni manuali, disabilitarla." },
        { "network.settings.dlg.proxy_text", "Sovrascrivi impostazioni proxy del browser." },
        { "network.settings.dlg.auto_text", "Usa script di configurazione automatica del proxy nella posizione specificata." },
        { "network.settings.dlg.none_text", "Usa connessione diretta." },

        // Advanced Network Settings Dialog
        { "advanced.network.dlg.title", "Impostazioni di rete avanzate" },
        { "advanced.network.dlg.servers", " Server " },
        { "advanced.network.dlg.type", "Tipo" },
        { "advanced.network.dlg.http", "HTTP:" },
        { "advanced.network.dlg.secure", "Sicuro:" },
        { "advanced.network.dlg.ftp", "FTP:" },
        { "advanced.network.dlg.socks", "Socks:" },
        { "advanced.network.dlg.proxy_address", "Indirizzo proxy" },
        { "advanced.network.dlg.port", "Porta" },
        { "advanced.network.dlg.same_proxy", " Usa lo stesso server proxy per tutti i protocolli" },
        { "advanced.network.dlg.same_proxy.mnemonic", "VK_U" },
        { "advanced.network.dlg.exceptions", " Eccezioni " },
        { "advanced.network.dlg.no_proxy", " Non usare il server proxy per gli indirizzi che iniziano con" },
        { "advanced.network.dlg.no_proxy_note", " Usare il punto e virgola (;) per separare le voci." },

        // DeleteFilesDialog
        { "delete.files.dlg.title", "Elimina file temporanei" },
        { "delete.files.dlg.temp_files", "Eliminare i seguenti file temporanei?" },
        { "delete.files.dlg.applets", "Applet scaricate" },
        { "delete.files.dlg.applications", "Applicazioni scaricate" },
        { "delete.files.dlg.other", "Altri file" },

	// General
	{ "general.cache.border.text", " File temporanei Internet " },
	{ "general.cache.delete.text", "Elimina file..." },
        { "general.cache.delete.text.mnemonic", "VK_E" },
	{ "general.cache.settings.text", "Impostazioni..." },
        { "general.cache.settings.text.mnemonic", "VK_I" },
	{ "general.cache.desc.text", "I file utilizzati nelle applicazioni Java vengono memorizzati in una speciale cartella per una successiva esecuzione pi\u00f9 rapida. La modifica di queste impostazioni o l'eliminazione dei file sono destinate ai soli utenti avanzati." },
	{ "general.network.border.text", " Impostazioni di rete " },
	{ "general.network.settings.text", "Impostazioni di rete..." },
        { "general.network.settings.text.mnemonic", "VK_R" },
	{ "general.network.desc.text", "Le impostazioni di rete vengono utilizzate per effettuare le connessioni. Normalmente, Java utilizza le impostazioni di rete del browser Web. La modifica di queste impostazioni \u00e8 destinata ai soli utenti avanzati." },
        { "general.about.border", "Informazioni su" },
	{ "general.about.btn", "Informazioni su..." },
	{ "general.about.btn.mnemonic", "VK_F" },
	{ "general.about.text", "Visualizza le informazioni sulla versione del pannello di controllo Java." },

	// Security
	{ "security.certificates.border.text", " Certificati " },
	{ "security.certificates.button.text", "Certificati..." },
        { "security.certificates.button.mnemonic", "VK_E" },
	{ "security.certificates.desc.text", "Usa i certificati per identificare con sicurezza gli utenti, le certificazioni le autorit\u00e0 e gli editori." },
	{ "security.policies.border.text", " Criteri " },
	{ "security.policies.advanced.text", "Avanzati..." },
        { "security.policies.advanced.mnemonic", "VK_V" },
	{ "security.policies.desc.text", "Usa i criteri di protezione per controllare le barriere di protezione delle applicazioni e delle applet." },

	// Update
	{ "update.notify.border.text", " Aggiorna notifica " }, // this one is not currently used.  See update panel!!!
	{ "update.updatenow.button.text", "Aggiorna adesso" },
	{ "update.updatenow.button.mnemonic", "VK_G" },
	{ "update.advanced.button.text", "Avanzate..." },
	{ "update.advanced.button.mnemonic", "VK_V" },
	{ "update.desc.text", "La funzione Aggiornamento Java garantisce sempre l'utilizzo della versione pi\u00f9 aggiornata della piattaforma Java. Le opzioni indicate qui sotto permettono di configurare i metodi per ottenere e installare gli aggiornamenti." },
        { "update.notify.text", "Notifica:" },
        { "update.notify_install.text", "Prima dell'installazione" },
        { "update.notify_download.text", "Prima di scaricare e prima dell'installazione" },
        { "update.autoupdate.text", "Controlla automaticamente la presenza di aggiornamenti" },
        { "update.advanced_title.text", "Impostazioni avanzate di aggiornamento automatico" },
        { "update.advanced_title1.text", "Selezionare quando e con che frequenza eseguire il controllo." },
        { "update.advanced_title2.text", "Frequenza" },
        { "update.advanced_title3.text", "Data" },
        { "update.advanced_desc1.text", "Esegui controllo ogni giorno alle {0}" },
        { "update.advanced_desc2.text", "Esegui controllo ogni {0} alle {1}" },
        { "update.advanced_desc3.text", "Esegui controllo il {0} di ogni mese alle {1}" },
        { "update.check_daily.text", "Quotidiana" },
        { "update.check_weekly.text", "Settimanale" },
        { "update.check_monthly.text", "Mensile" },
        { "update.check_date.text", "Giorno:" },
        { "update.check_day.text", "Ogni:" },
        { "update.check_time.text", "Alle:" },
        { "update.lastrun.text", "L''Aggiornamento Java \u00e8 stato eseguito l''ultima volta alle {0} del {1}." },
        { "update.desc_autooff.text", "Fare clic sul pulsante \"Aggiorna adesso\" qui sotto per controllare la presenza di un aggiornamento. Se \u00e8 disponibile un aggiornamento verr\u00e0 visualizzata un'icona nella barra delle applicazioni. Spostare il puntatore sull'icona per visualizzare lo stato dell'aggiornamento." },
        { "update.desc_check_daily.text", "L''Aggiornamento Java controller\u00e0 gli aggiornamenti ogni giorno alle {0}. " },
        { "update.desc_check_weekly.text", "L''Aggiornamento Java controller\u00e0 gli aggiornamenti ogni {0} alle {1}. " },
        { "update.desc_check_monthly.text", "L''Aggiornamento Java controller\u00e0 gli aggiornamenti il {0} di ogni mese alle {1}. " },
        { "update.desc_systrayicon.text", "Se \u00e8 disponibile un aggiornamento verr\u00e0 visualizzata un'icona nella barra delle applicazioni. Spostare il puntatore sull'icona per visualizzare lo stato dell'aggiornamento. " },
        { "update.desc_notify_install.text", "L'utente verr\u00e0 avvisato prima dell'installazione dell'aggiornamento." },
        { "update.desc_notify_download.text", "L'utente verr\u00e0 avvisato prima che l'aggiornamento venga scaricato e prima che venga installato." },
	{ "update.launchbrowser.error.text", "Impossibile avviare il controllo degli aggiornamenti Java. Per ottenere il pi\u00f9 recente aggiornamento di Java, visitare la pagina http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Errore - aggiornamento" },

        // CacheSettingsDialog strings:
        { "cache.settings.dialog.delete_btn", "Elimina file..." },
        { "cache.settings.dialog.delete_btn.mnemonic", "VK_E" },
        { "cache.settings.dialog.view_jws_btn", "Visualizza applicazioni..." },
        { "cache.settings.dialog.view_jws_btn.mnemonic", "VK_V" },
        { "cache.settings.dialog.view_jpi_btn", "Visualizza applet..." },
        { "cache.settings.dialog.view_jpi_btn.mnemonic", "VK_S" },
        { "cache.settings.dialog.chooser_title", "Posizione file temporanei" },
        { "cache.settings.dialog.select", "Seleziona" },
        { "cache.settings.dialog.select_tooltip", "Usa posizione selezionata" },
        { "cache.settings.dialog.select_mnemonic", "S" },
        { "cache.settings.dialog.title", "Impostazioni file temporanei" },
        { "cache.settings.dialog.cache_location", "Posizione:" },
        { "cache.settings.dialog.change_btn", "Modifica..." },
        { "cache.settings.dialog.change_btn.mnemonic", "VK_M" },
        { "cache.settings.dialog.disk_space", "Spazio su disco da usare:" },
        { "cache.settings.dialog.unlimited_btn", "Illimitato" },
        { "cache.settings.dialog.max_btn", "Massimo" },
        { "cache.settings.dialog.compression", "Compressione Jar:" },
        { "cache.settings.dialog.none", "Nessuna" },
        { "cache.settings.dialog.high", "Elevata" },

	// JNLP File/MIME association dialog strings:
	{ "javaws.association.dialog.title", "File JNLP/associazione MIME" },
        { "javaws.association.dialog.exist.command", "gi\u00e0 esistente con:\n{0}"},
	{ "javaws.association.dialog.exist", "gi\u00e0 esistente." },
        { "javaws.association.dialog.askReplace", "\nConfermare l''utilizzo di {0} al suo posto per la gestione?"},
	{ "javaws.association.dialog.ext", "Estensioni file: {0}" },
        { "javaws.association.dialog.mime", "Tipo MIME: {0}" },
        { "javaws.association.dialog.ask", "Usare {0} per gestire:" },
        { "javaws.association.dialog.existAsk", "ATTENZIONE! Associazione con:"},

        // Advanced panel strings:
        { "deployment.console.startup.mode", "Console Java" },
        { "deployment.console.startup.mode.SHOW", "Mostra console" },
        { "deployment.console.startup.mode.SHOW.tooltip", "<html>" +
          "Avvia la console Java ingrandita" +
          "</html>" },
        { "deployment.console.startup.mode.HIDE", "Nascondi console" },
        { "deployment.console.startup.mode.HIDE.tooltip", "<html>" +
          "Avvia la console Java ridotta a icona" +
          "</html>" },
        { "deployment.console.startup.mode.DISABLE", "Non avviare la console" },
        { "deployment.console.startup.mode.DISABLE.tooltip", "<html>" +
          "La console Java non verr\u00e0 avviata" +
          "</html>" },
        { "deployment.trace", "Abilita tracing" },
        { "deployment.trace.tooltip", "<html>" +
          "Crea un file di traccia per consentire" +
          "<br>il debugging" +
          "</html>" },
        { "deployment.log", "Abilita registrazione eventi" },
	{ "deployment.log.tooltip", "<html>" +
                                    "Crea un file di log per" +
                                    "<br>l'identificazione degli errori" +
                                    "</html>" },
        { "deployment.control.panel.log", "Registrazione eventi nel pannello di controllo" },
        { "deployment.javapi.lifecycle.exception", "Mostra eccezioni nel ciclo di vita dell'applet" },
        { "deployment.javapi.lifecycle.exception.tooltip", "<html>" +
                                          "Mostra la finestra delle eccezioni quando si"+
                                          "<br>verificano errori durante il caricamento dell'applet"+
                                          "<html>" },
        { "deployment.browser.vm.iexplorer", "Internet Explorer" },
        { "deployment.browser.vm.iexplorer.tooltip", "<html>" +
          "Usa Java Sun con il tag APPLET" +
          "<br>nel browser Internet Explorer" +
          "</html>" },
        { "deployment.browser.vm.mozilla",   "Mozilla e Netscape" },
        { "deployment.browser.vm.mozilla.tooltip", "<html>" +
          "Usa Java Sun con il tag APPLET" +
          "<br>nei browser Mozilla o Netscape" +
          "</html>" },
        { "deployment.console.debugging", "Debugging" },
        { "deployment.browsers.applet.tag", "Supporto del tag <APPLET>" },
        { "deployment.javaws.shortcut", "Creazione collegamenti" },
        { "deployment.javaws.shortcut.ALWAYS", "Consenti sempre" },
        { "deployment.javaws.shortcut.ALWAYS.tooltip", "<html>" +
          "Crea sempre un collegamento." +
	  "</html>" },
        { "deployment.javaws.shortcut.NEVER" , "Non consentire mai" },
        { "deployment.javaws.shortcut.NEVER.tooltip", "<html>" +
          "Non crea un collegamento" +
          "</html>" },
        { "deployment.javaws.shortcut.ASK_USER", "Richiedi conferma" },
        { "deployment.javaws.shortcut.ASK_USER.tooltip", "<html>" +
          "Chiede all'utente prima di creare" +
          "<br>un collegamento" +
          "</html>" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED", "Consenti sempre se suggerito" },
        { "deployment.javaws.shortcut.ALWAYS_IF_HINTED.tooltip", "<html>" +
          "Crea sempre un collegamento se" +
          "<br>lo richiede l'applicazione JNLP" +
          "</html>" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED", "Richiedi all'utente se suggerito" },
        { "deployment.javaws.shortcut.ASK_IF_HINTED.tooltip", "<html>" +
          "Chiede conferma all'utente prima di" +
          "<br>creare una scelta rapida se" +
          "<br>l'applicazione JNLP lo richiede" +
          "</html>" },
	{ "deployment.javaws.associations.NEVER", "Non consentire mai" },
        { "deployment.javaws.associations.NEVER.tooltip", "<html>" +
          "Non crea mai l'associazione tra estensione e" +
          "<br>tipo MIME del file" +
          "</html>" },
        { "deployment.javaws.associations.ASK_USER", "Richiedi conferma" },
        { "deployment.javaws.associations.ASK_USER.tooltip", "<html>" +
          "Chiede conferma all'utente prima di" +
          "<br>creare l'associazione tra" +
          "<br>estensione e tipo MIME del file" +
          "</html>" },
        { "deployment.javaws.associations.REPLACE_ASK", "Richiedi conferma per sostituzione" },
        { "deployment.javaws.associations.REPLACE_ASK.tooltip", "<html>" +
          "Chiede conferma all'utente solo prima" +
          "<br>di sostituire l'associazione tra" +
          "<br>estensione e tipo MIME esistente" +
          "</html>" },
        { "deployment.javaws.associations.NEW_ONLY", "Consenti se l'associazione \u00e8 nuova" },
        { "deployment.javaws.associations.NEW_ONLY.tooltip", "<html>" +
          "Crea solo le nuove associazioni tra" +
          "<br>estensione e tipo MIME del file" +
          "</html>" },
        { "deployment.javaws.associations", "File JNLP/associazione MIME" },
        { "deployment.security.settings", "Protezione" },
        { "deployment.security.askgrantdialog.show", "Consenti all'utente di concedere autorizzazioni per i contenuti firmati" },
        { "deployment.security.askgrantdialog.notinca", "Consenti all'utente di concedere autorizzazioni per i contenuti provenienti da autorit\u00e0 non attendibili" },
        { "deployment.security.browser.keystore.use", "Usa i certificati e le chiavi dell'archivio chiavi del browser" },
        { "deployment.security.notinca.warning", "Avvisa se non \u00e8 possibile verificare l'autorit\u00e0 del certificato" },
        { "deployment.security.expired.warning", "Avvisa se il certificato \u00e8 scaduto o non \u00e8 ancora valido" },
        { "deployment.security.jsse.hostmismatch.warning", "Avvisa se il certificato del sito non corrisponde al nome host" },
        { "deployment.security.sandbox.awtwarningwindow", "Mostra il segnale di avvertimento sandbox" },
        { "deployment.security.sandbox.jnlp.enhanced", "Consenti all'utente di accettare le richieste di sicurezza JNLP" },
        { "deploy.advanced.browse.title", "Scegli file per avviare il browser predefinito" },
        { "deploy.advanced.browse.select", "Seleziona" },
        { "deploy.advanced.browse.select_tooltip", "Usa file selezionato per avviare il browser" },
        { "deploy.advanced.browse.select_mnemonic", "S" },
        { "deploy.advanced.browse.browse_btn", "Sfoglia..." },
        { "deploy.advanced.browse.browse_btn.mnemonic", "VK_F" },
        { "deployment.browser.default", "Comando per avviare il browser predefinito" },
        { "deployment.misc.label", "Varie" },
        { "deployment.system.tray.icon", "Inserisci l'icona di Java nella barra delle applicazioni" },
        { "deployment.system.tray.icon.tooltip", "<html>" +
                             "Mostra l'icona della tazza di Java" +
                             "<br>nella barra delle applicazioni quando" +
                             "<br>Java \u00e8 in esecuzione nel browser" +
                             "</html>" },

        //PluginJresDialog strings:
        { "jpi.jres.dialog.title", "Impostazioni runtime Java" },
        { "jpi.jres.dialog.border", " Versioni runtime Java" },
        { "jpi.jres.dialog.column1", "Nome prodotto" },
        { "jpi.jres.dialog.column2", "Versione" },
        { "jpi.jres.dialog.column3", "Posizione" },
        { "jpi.jres.dialog.column4", "Parametri runtime Java" },
        { "jpi.jdk.string", "JDK" },
        { "jpi.jre.string", "JRE" },
        { "jpi.jres.dialog.product.tooltip", "Scegliere JRE o JDK come nome del prodotto" },

        // AboutDialog strings:
        { "about.dialog.title", "Informazioni su Java" },

        // JavaPanel strings:
        { "java.panel.plugin.border", " Impostazioni runtime applet Java " },
        { "java.panel.plugin.text", "Le impostazioni runtime sono utilizzate quando l'applet viene eseguita nel browser." },
        { "java.panel.jpi_view_btn", "Visualizza..." },
        { "java.panel.javaws_view_btn", "Visualizza..." },
        { "java.panel.jpi_view_btn.mnemonic", "VK_V" },
        { "java.panel.javaws_view_btn.mnemonic", "VK_I" },
        { "java.panel.javaws.border", " Impostazioni runtime applicazioni Java"},
        { "java.panel.javaws.text", "Le impostazioni runtime sono utilizzate quando si avvia un'applicazione o un'applet Java usando JNLP (Java Network Launching Protocol)." },

        // Strings in the confirmation dialogs for APPLET tag in browsers.
        { "browser.settings.alert.text", "<html><b>\u00c8 presente una versione pi\u00f9 recente di Java</b></html>Internet Explorer dispone gi\u00e0 di una versione pi\u00f9 recente del runtime Java. Sostituirla?\n" },
        { "browser.settings.success.caption", "Operazione riuscita - browser" },
        { "browser.settings.success.text", "<html><b>Impostazioni del browser modificate</b></html>Le modifiche avranno effetto al prossimo riavvio del browser.\n" },
        { "browser.settings.fail.caption", "Attenzione - browser" },
        { "browser.settings.fail.moz.text", "<html><b>Impossibile modificare le impostazioni del browser</b></html>"
        		+ "Accertarsi che Mozilla o Netscape siano stati installati correttamente sul sistema e di "
        		+ "disporre delle autorizzazioni appropriate "
        		+ "per modificare le impostazioni di sistema.\n" },
        { "browser.settings.fail.ie.text", "<html><b>Impossibile modificare le impostazioni del browser</b></html>Accertarsi di disporre delle "
        		+ "autorizzazioni appropriate per modificare le impostazioni di sistema.\n" },


        // Tool tip strings.
        { "cpl.ok_btn.tooltip", "<html>" +
                                "Chiude il pannello di controllo e salva" +
                                "<br>le modifiche apportate" +
                                "</html>" },
        { "cpl.apply_btn.tooltip",  "<html>" +
                                    "Salva tutte le modifiche" +
                                    "<br>senza chiudere il pannello di controllo Java" +
                                    "</html>" },
        { "cpl.cancel_btn.tooltip", "<html>" +
                                    "Chiude il pannello di controllo Java senza" +
                                    "<br>salvare le modifiche" +
                                    "</html>" },

        {"network.settings.btn.tooltip", "<html>"+
                                         "Modifica le impostazioni di connessione a Internet"+
                                         "</html>"},

        {"temp.files.settings.btn.tooltip", "<html>"+
                                            "Modifica le impostazioni per i file temporanei"+
                                            "</html>"},

        {"temp.files.delete.btn.tooltip", "<html>" +  // body bgcolor=\"#FFFFCC\">"+
                                          "Elimina i file Java temporanei" +
                                          "</html>"},

        {"delete.files.dlg.applets.tooltip", "<html>" +
                                          "Selezionare questa opzione per eliminare tutti i file" +
                                          "<br>temporanei creati dalle applet Java" +
                                          "</html>" },

        {"delete.files.dlg.applications.tooltip", "<html>" +
                                          "Selezionare questa opzione per eliminare tutti i file" +
                                          "<br>temporanei creati dalle applicazioni" +
                                          "<br>Java Web Start" +
                                          "</html>" },

        {"delete.files.dlg.other.tooltip", "<html>" +
                                          "Selezionare questa opzione per eliminare tutti gli" +
                                          "<br>altri file temporanei creati da Java" +
                                          "</html>" },

        {"delete.files.dlg.temp_files.tooltip", "<html>" +
                                          "Le applicazioni Java possono memorizzare alcuni file" +
                                          "<br>temporanei sul computer, che possono essere" +
                                          "<br>eliminati senza rischi." +
                                          "<br>" +
                                          "<p>Alla prima esecuzione dopo l'eliminazione dei file" +
                                          "<br>temporanei, alcune applicazioni Java possono" +
                                          "<br>richiedere pi\u00f9 tempo per avviarsi." +
                                          "</html>" },

        {"cache.settings.dialog.view_jws_btn.tooltip", "<html>" +
                                          "Visualizza i file temporanei creati dalle" +
                                          "<br>applicazioni Java Web Start" +
                                          "</html>" },

        {"cache.settings.dialog.view_jpi_btn.tooltip", "<html>" +
                                          "Visualizza i file temporanei creati dalle" +
                                          "<br>applet Java" +
                                          "</html>" },

        {"cache.settings.dialog.change_btn.tooltip", "<html>" +
                                          "Specifica la directory in cui sono" +
                                          "<br>memorizzati i file temporanei" +
                                          "</html>" },

        {"cache.settings.dialog.unlimited_btn.tooltip", "<html>" +
                                          "Non limita lo spazio su disco utilizzabile" +
                                          "<br>per memorizzare i file temporanei" +
                                          "</html>" },

        {"cache.settings.dialog.max_btn.tooltip", "<html>" +
                                          "Specifica lo spazio massimo su disco utilizzabile" +
                                          "<br>per memorizzare i file temporanei." +
                                          "</html>" },

        {"cache.settings.dialog.compression.tooltip", "<html>" +
                                          "Specifica il tipo di compressione da utilizzare per" +
                                          "<br>i file JAR memorizzati dai programmi Java" +
                                          "<br>nella directory dei file temporanei." +
                                          "<br>" +
                                          "<p>Selezionando \"Nessuna\", l'avvio dei programmi Java" +
                                          "<br>\u00e8 pi\u00f9 rapido, ma lo spazio su disco richiesto \u00e8" +
                                          "<br>superiore. Valori superiori riducono lo spazio" +
                                          "<br>occupato sul disco ma aumentano leggermente i" +
                                          "<br>tempi di avvio." +
                                          "</html>" },

        { "common.ok_btn.tooltip",  "<html>" +
                                    "Salva le modifiche e chiude la finestra di dialogo" +
                                    "</html>" },

        { "common.cancel_btn.tooltip",  "<html>" +
                                        "Annulla le modifiche e chiude la finestra di dialogo" +
                                        "</html>"},

	{ "network.settings.advanced_btn.tooltip",  "<html>" +
                                                    "Visualizza e modifica le impostazioni avanzate del proxy"+
                                                    "</html>"},

        {"security.certs_btn.tooltip", "<html>" +
                                       "Importa, esporta o rimuove un certificato" +
                                       "</html>" },

        { "cert.import_btn.tooltip", "<html>" +
                                     "Importa un certificato ancora non presente" +
                                     "<br>nell'elenco" +
				     "</html>"},

        { "cert.export_btn.tooltip",    "<html>" +
                                        "Esporta il certificato selezionato" +
                                        "</html>"},

        { "cert.remove_btn.tooltip",  "<html>" +
                                      "Rimuove il certificato selezionato"+
                                      "<br>dall'elenco" +
        		      "</html>"},

        { "cert.details_btn.tooltip", "<html>" +
		      "Visualizza informazioni dettagliate sul" +
                      "<br>certificato selezionato" +
		      "</html>"},

        { "java.panel.jpi_view_btn.tooltip",  "<html>" +
                                              "Modifica le impostazioni runtime Java per le applet"+
                                              "</html>" },

        { "java.panel.javaws_view_btn.tooltip",   "<html>" +
                                                  "Modifica le impostazioni runtime Java per le applicazioni"+
                                                  "</html>" },

        { "general.about.btn.tooltip",   "<html>" +
                                            "Visualizza informazioni su questa versione" +
                                            "<br>di J2SE Runtime Environment" +
                                            "</html>" },

        { "update.notify_combo.tooltip",  "<html>" +
                                          "Seleziona le modalit\u00e0 di notifica " +
                                          "<br>dei nuovi aggiornamenti di" +
                                          "<br>Java" +
                                          "</html>" },

        { "update.advanced_btn.tooltip",  "<html>" +
                                          "Modifica i criteri di pianificazione" +
                                          "<br>per l'aggiornamento automatico" +
                                          "</html>" },

        { "update.now_btn.tooltip",    "<html>" +
                                      "Avvia l'Aggiornamento Java per il controllo della" +
                                      "<br>disponibilit\u00e0 di nuovi aggiornamenti per Java" +
                                      "</html>" },

        { "vm.options.add_btn.tooltip",   "<html>" +
                                          "Aggiunge un nuovo JRE all'elenco" +
                                          "</html>" },

        { "vm.options.remove_btn.tooltip", "<html>" +
                                           "Rimuove la voce selezionata dall'elenco" +
                                           "</html>" },

        { "vm.optios.ok_btn.tooltip",    "<html>" +
		         "Salva tutte le voci contenenti informazioni sul" +
		         "<br>nome del prodotto, la versione e" +
		         "<br>la posizione" +
		         "</html>" },

        { "jnlp.jre.find_btn.tooltip",  "<html>" +
		        "Verifica la presenza di un ambiente runtime Java" +
                        "<br>installato" +
		        "</html>" },

        { "jnlp.jre.add_btn.tooltip",   "<html>" +
                                        "Aggiunge una nuova voce all'elenco" +
		        "</html>" },

        { "jnlp.jre.remove_btn.tooltip",  "<html>" +
                                          "Rimuove la voce selezionata dall'elenco" +
                                          "<br>dell'utente" +
                                          "</html>" },


        // JaWS Auto Download JRE Prompt
        { "download.jre.prompt.title", "Consenti download JRE" },
        { "download.jre.prompt.text1", "L''applicazione \"{0}\" richiede una versione"
                                     + "dell''ambiente runtime JRE (versione {1}), che"
                                     + "non \u00e8 attualmente installata sul sistema." },
        { "download.jre.prompt.text2", "Consentire a Java Web Start di scaricare "
                                     + "e installare questo JRE automaticamente?" },
        { "download.jre.prompt.okButton", "Scarica" },
        { "download.jre.prompt.okButton.acceleratorKey", new Integer(KeyEvent.VK_S)},
        { "download.jre.prompt.cancelButton", "Annulla" },
        { "download.jre.prompt.cancelButton.acceleratorKey", new Integer(KeyEvent.VK_A)},
        { "autoupdatecheck.buttonYes", "S\u00ec" },
        { "autoupdatecheck.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "autoupdatecheck.buttonNo", "No" },
	{ "autoupdatecheck.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},
        { "autoupdatecheck.buttonAskLater", "Richiedi in seguito" },
        { "autoupdatecheck.buttonAskLater.acceleratorKey", new Integer(KeyEvent.VK_R)},
        { "autoupdatecheck.caption", "Controlla automaticamente gli aggiornamenti" },
        { "autoupdatecheck.message", "L'Aggiornamento Java pu\u00f2 aggiornare automaticamente Java quando si rende disponibile una nuova versione. Abilitare questo servizio?" },
    };
}

