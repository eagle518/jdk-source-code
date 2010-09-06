/*
 * @(#)Activator.java	1.135 04/04/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
/**
 * US English verison of Activator strings.
 *
 * @author Graham Hamilton
 */

public class Activator extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "loading", "Loading {0} ..." },
	{ "java_applet", "Java Applet" },
        { "failed", "Loading Java Applet Failed..." },
        { "image_failed", "Failed to create user-defined image.  Check image file name." },
	{ "java_not_enabled", "Java is not enabled" },
        { "exception", "Exception: {0}" },

	{ "bean_code_and_ser", "Bean cannot have both CODE and JAVA_OBJECT defined " },
	{ "status_applet", "Applet {0} {1}" },

	// Resources associated with SecurityManager print Dialog:
	{ "print.caption", "Confirmation Needed - Print" },
	{ "print.message", new String[]{
		"<html><b>Print Request</b></html>Applet would like to print. Do you want to proceed?"}},
	{ "print.checkBox", "Don't show this dialog box again" },
	{ "print.buttonYes", "Yes" },
	{ "print.buttonYes.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "print.buttonNo", "No" },
	{ "print.buttonNo.acceleratorKey", new Integer(KeyEvent.VK_N)},

	{ "optpkg.cert_expired", "<html><b>Certificate Expired</b></html>Optional package installation is aborted.\n" },
	{ "optpkg.cert_notyieldvalid", "<html><b>Certificate Not Valid</b></html>Optional package installation is aborted.\n" },
	{ "optpkg.cert_notverify", "<html><b>Certificate Not Verified</b></html>Optional package installation is aborted.\n" },
	{ "optpkg.general_error", "<html><b>General Exception</b></html>Optional package installation is aborted.\n" },
	{ "optpkg.caption", "Warning - Optional Package" },
	{ "optpkg.installer.launch.wait", "<html><b>Installating Optional Package</b></html>Click OK to continue applet loading after optional package installer exits.\n" },
	{ "optpkg.installer.launch.caption", "Installation in Progress - Optional Package"},
	{ "optpkg.prompt_user.new_spec.text", "<html><b>Download Request</b></html>The applet requires a newer version (specification {0}) of optional package \"{1}\" from {2}\n\nDo you want to continue?" },
	{ "optpkg.prompt_user.new_impl.text", "<html><b>Download Request</b></html>The applet requires a newer version (implementation {0}) of optional package \"{1}\" from {2}\n\nDo you want to continue?" },
	{ "optpkg.prompt_user.new_vendor.text", "<html><b>Download Request</b></html>The applet requires the ({0}) of optional package \"{1}\" {2} from {3}\n\nDo you want to continue?" },
	{ "optpkg.prompt_user.default.text", "<html><b>Download Request</b></html>The applet requires the installation of optional package \"{0}\" from {1}\n\nDo you want to continue?" },

	{ "cache.error.text", "<html><b>Caching Error</b></html>Unable to store or update files in the cache." },
	{ "cache.error.caption", "Error - Cache" },
	{ "cache.version_format_error", "{0} is not in the form xxxx.xxxx.xxxx.xxxx, where x is a hexadecimal digit" },
	{ "cache.version_attrib_error", "Number of attributes specified in \'cache_archive\' doesn't match those in \'cache_version\'" },
	{ "cache.header_fields_missing", "Last modified time and/or expiration value is not available.  Jar file will not be cached."},

	{ "applet.progress.load", "Loading applet ..." },
	{ "applet.progress.init", "Initializing applet ..." },
	{ "applet.progress.start", "Starting applet ..." },
	{ "applet.progress.stop", "Stopping applet ..." },
	{ "applet.progress.destroy", "Destroying applet ..." },
	{ "applet.progress.dispose", "Disposing applet ..." },
	{ "applet.progress.quit", "Quiting applet ..." },
	{ "applet.progress.stoploading", "Stopped loading ..." },
	{ "applet.progress.interrupted", "Interrupted thread ..." },
	{ "applet.progress.joining", "Joining applet thread ..." },
	{ "applet.progress.joined", "Joined applet thread ..." },
	{ "applet.progress.loadImage", "Loading image " },
	{ "applet.progress.loadAudio", "Loading audio " },
	{ "applet.progress.findinfo.0", "Finding information ..." },
	{ "applet.progress.findinfo.1", "Done ..." },
	{ "applet.progress.timeout.wait", "Waiting for timeout ..." },
	{ "applet.progress.timeout.jointing", "Doing a join ..." },
	{ "applet.progress.timeout.jointed", "Done with join ..." },

	{ "modality.register", "Registered modality listener" },
	{ "modality.unregister", "Unregistered modality listener" },
	{ "modality.pushed", "Modality pushed" },
	{ "modality.popped", "Modality popped" },

	{ "progress.listener.added", "Added progress listener: {0}" },
	{ "progress.listener.removed", "Removed progress listener: {0}" },

	{ "liveconnect.UniversalBrowserRead.enabled", "JavaScript: UniversalBrowserRead enabled" },
	{ "liveconnect.java.system", "JavaScript: calling Java system code" },
	{ "liveconnect.same.origin", "JavaScript: caller and callee have same origin" },
	{ "liveconnect.default.policy", "JavaScript: default security policy = {0}" },
	{ "liveconnect.UniversalJavaPermission.enabled", "JavaScript: UniversalJavaPermission enabled" },
	{ "liveconnect.wrong.securitymodel", "Netscape security model is no longer supported.\n"
					     + "Please migrate to the Java 2 security model instead.\n" },

        { "pluginclassloader.created_files", "Created {0} in cache." },
        { "pluginclassloader.deleting_files", "Deleting JAR files from cache." },
        { "pluginclassloader.file", "   deleting from cache {0}" },
        { "pluginclassloader.empty_file", "{0} is empty, deleting from cache." },

	{ "appletcontext.audio.loaded", "Loaded audio clip: {0}" },
	{ "appletcontext.image.loaded", "Loaded image: {0}" },

	{ "securitymgr.automation.printing", "Automation: Accept printing" },

	{ "classloaderinfo.referencing", "Referencing classloader: {0}, refcount={1}" },
	{ "classloaderinfo.releasing", "Releasing classloader: {0}, refcount={1}" },
	{ "classloaderinfo.caching", "Caching classloader: {0}" },
	{ "classloaderinfo.cachesize", "Current classloader cache size: {0}" },
	{ "classloaderinfo.num", "Number of cached classloaders over {0}, unreference {1}" },
        { "jsobject.call", "JSObject::call: name={0}" },
        { "jsobject.eval", "JSObject::eval({0})" },
        { "jsobject.getMember", "JSObject::getMember: name={0}" },
        { "jsobject.setMember", "JSObject::setMember: name={0}" },
        { "jsobject.removeMember", "JSObject::removeMember: name={0}" },
        { "jsobject.getSlot", "JSObject::getSlot: {0}" },
        { "jsobject.setSlot", "JSObject::setSlot: slot={0}" },
	{ "jsobject.invoke.url.permission", "the url of the applet is {0} and the permission is = {1}"},

	{ "optpkg.install.info", "Installing optional package {0}" },
	{ "optpkg.install.fail", "Optional package installation failed." },
	{ "optpkg.install.ok", "Optional package installation succeeded." },
	{ "optpkg.install.automation", "Automation: Accept optional package installation" },
	{ "optpkg.install.granted", "Optional package download granted by user, download from {0}" },
	{ "optpkg.install.deny", "Optional package download not granted by user" },
	{ "optpkg.install.begin", "Installing {0}" },
	{ "optpkg.install.java.launch", "Launching Java installer" },
	{ "optpkg.install.java.launch.command", "Launching Java installer through ''{0}''" },
	{ "optpkg.install.native.launch", "Launching native installer" },
	{ "optpkg.install.native.launch.fail.0", "Unable to execute {0}" },
	{ "optpkg.install.native.launch.fail.1", "Access to {0} failed" },
	{ "optpkg.install.raw.launch", "Installing raw optional package" },
	{ "optpkg.install.raw.copy", "Copying Raw Optional Package from {0} to {1}" },
	{ "optpkg.install.error.nomethod", "Dependent Extension Provider not installed : Cannot get the "
				         + " addExtensionInstallationProvider method" },
	{ "optpkg.install.error.noclass", "Dependent Extension Provider not installed : Cannot get the "
					 + "sun.misc.ExtensionDependency class" },

	{"progress_dialog.downloading", "Plug-in: Downloading ..."},
        {"progress_dialog.dismiss_button", "Dismiss"},
        {"progress_dialog.dismiss_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
        {"progress_dialog.from", "from"},

        {"applet_viewer.color_tag", "Incorrect number of components in {0}"},

        {"progress_info.downloading", "Downloading JAR file(s)"},
        {"progress_bar.preload", "Preloading JAR files: {0}"},

        {"cache.size", "Cache Size: {0}"},
        {"cache.cleanup", "Cache size is: {0} bytes, cleanup is necessary"},
        {"cache.full", "Cache is full: deleting file {0}"},
        {"cache.inuse", "Cannot delete file {0} since it is being used by this application"},
        {"cache.notdeleted", "Cannot delete file {0}, may be used by this and/or other application(s)"},
        {"cache.out_of_date", "Cached copy of {0} is out of date\n  Cached copy: {1}\n  Server copy: {2}"},
        {"cache.loading", "Loading {0} from cache"},
        {"cache.cache_warning", "WARNING: Unable to cache {0}"},
        {"cache.downloading", "Downloading {0} to cache"},
        {"cache.cached_name", "Cached file name: {0}"},
        {"cache.load_warning", "WARNING: error reading {0} from cache."},
        {"cache.disabled", "Cache is disabled by user"},
        {"cache.minSize", "Cache is disabled, cache limit is set to {0}, at least 5 MB should be specified"},
        {"cache.directory_warning", "WARNING: {0} is not a directory.  Cache will be disabled."},
        {"cache.response_warning", "WARNING: Unexpected response {0} for {1}.  File will be downloaded again."},
        {"cache.enabled", "Cache is enabled"},
        {"cache.location", "Location: {0}"},
        {"cache.maxSize", "Maximum size: {0}"},
        {"cache.create_warning", "WARNING: Could not create cache directory {0}.  Caching will be disabled."},
        {"cache.read_warning", "WARNING: Cannot read cache directory {0}.  Caching will be disabled."},
        {"cache.write_warning", "WARNING: Cannot write to cache directory {0}.  Caching will be disabled."},
        {"cache.compression", "Compression level: {0}"},
        {"cache.cert_load", "Certificates for {0} is read from JAR cache"},
	{"cache.jarjar.invalid_file", ".jarjar file contains a non .jar file"},
	{"cache.jarjar.multiple_jar", ".jarjar file contains more than one .jar file"},
        {"cache.version_checking", "Version checking for {0}, specified version is {1}"},
        {"cache.preloading", "Preloading file {0}"},

	{ "cache_viewer.caption", "Java Applet Cache Viewer" },
	{ "cache_viewer.refresh", "Refresh" },
	{ "cache_viewer.refresh.acceleratorKey", new Integer(KeyEvent.VK_R) },
        { "cache_viewer.remove", "Delete" },
	{ "cache_viewer.remove.acceleratorKey", new Integer(KeyEvent.VK_D) },
        { "cache_viewer.OK", "OK" },
	{ "cache_viewer.OK.acceleratorKey", new Integer(KeyEvent.VK_O) },
        { "cache_viewer.name", "Name" },
	{ "cache_viewer.type", "Type" },
	{ "cache_viewer.size", "Size" },
	{ "cache_viewer.modify_date", "Last Modified" },
	{ "cache_viewer.expiry_date", "Expiration Date" },
	{ "cache_viewer.url", "URL" },
	{ "cache_viewer.version", "Version" },
	{ "cache_viewer.help.name", "Cached file name" },
	{ "cache_viewer.help.type", "Cached file type" },
	{ "cache_viewer.help.size", "Cached file size" },
	{ "cache_viewer.help.modify_date", "Cached file last modify date" },
	{ "cache_viewer.help.expiry_date", "Cached file expiration date" },
	{ "cache_viewer.help.url", "Cached file download URL" },
	{ "cache_viewer.help.version", "Cached file version" },
	{ "cache_viewer.delete.text", "<html><b>File Not Deleted</b></html>{0} may be in use.\n" },
	{ "cache_viewer.delete.caption", "Error - Cache" },
	{ "cache_viewer.type.zip", "Jar" },
	{ "cache_viewer.type.class", "Class" },
	{ "cache_viewer.type.wav", "Wav Sound" },
	{ "cache_viewer.type.au", "Au Sound" },
	{ "cache_viewer.type.gif", "Gif Image" },
	{ "cache_viewer.type.jpg", "Jpeg Image" },
        { "cache_viewer.menu.file", "File" },
        { "cache_viewer.menu.file.acceleratorKey", new Integer(KeyEvent.VK_F) },
        { "cache_viewer.menu.options", "Options" },
        { "cache_viewer.menu.options.acceleratorKey", new Integer(KeyEvent.VK_P) },
        { "cache_viewer.menu.help", "Help" },
        { "cache_viewer.menu.help.acceleratorKey", new Integer(KeyEvent.VK_H) },
        { "cache_viewer.menu.item.exit", "Exit" },
        { "cache_viewer.menu.item.exit.acceleratorKey", new Integer(KeyEvent.VK_X) },
        { "cache_viewer.disable", "Enable Caching" },
        { "cache_viewer.disable.acceleratorKey", new Integer(KeyEvent.VK_N) },
        { "cache_viewer.menu.item.about", "About" },
        { "cache_viewer.menu.item.about.acceleratorKey", new Integer(KeyEvent.VK_A) },

	{ "net.proxy.auto.result.error", "Unable to determine proxy setting from evaluation - fallback to DIRECT"},

	{ "lifecycle.applet.found", "Found previous stopped applet from lifecycle cache" },
	{ "lifecycle.applet.support", "Applet supports legacy lifecycle model - add applet to lifecycle cache" },
	{ "lifecycle.applet.cachefull", "Lifecycle cache is full - prune least recently used applets" },

	{ "com.method.ambiguous", "Unable to select a method, ambiguous parameters" },
	{ "com.method.notexists", "{0} :no such method exists" },
	{ "com.notexists", "{0} :no such method/property exists" },
	{ "com.method.invoke", "Invoking method: {0}" },
	{ "com.method.jsinvoke", "Invoking JS method: {0}" },
	{ "com.method.argsTypeInvalid", "The parameters cannot be converted to the required types" },
	{ "com.method.argCountInvalid", "Number of arguments is not correct" },
	{ "com.field.needsConversion", "Needs conversion: {0} --> {1}" },
	{ "com.field.typeInvalid", " cannot be converted to type: {0}" },
	{ "com.field.get", "Getting property: {0}" },
	{ "com.field.set", "Setting property: {0}" },

	{ "rsa.cert_expired", "<html><b>Certificate Expired</b></html>Code will be treated as unsigned.\n" },
	{ "rsa.cert_notyieldvalid", "<html><b>Certificate Not Valid</b></html>Code will be treated as unsigned.\n" },
	{ "rsa.general_error", "<html><b>Certificate Not Verified</b></html>Code will be treated as unsigned.\n" },

	{ "dialogfactory.menu.show_console", "Show Java Console" },
	{ "dialogfactory.menu.hide_console", "Hide Java Console" },
	{ "dialogfactory.menu.about", "About Java Plug-in" },
	{ "dialogfactory.menu.copy", "Copy" },
	{ "dialogfactory.menu.open_console", "Open Java Console" },
	{ "dialogfactory.menu.about_java", "About Java(TM)" },
    };
}


