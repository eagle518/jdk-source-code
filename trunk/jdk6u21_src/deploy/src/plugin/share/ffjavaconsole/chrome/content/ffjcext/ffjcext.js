const gJavaConsole_JDK_UNDERSCORE_VERSION =  {

	id	: "javaconsole_JDK_VERSION",

	mimeType: "application/x-java-applet;jpi-version=_PLUGIN_MAJOR_MIMETYPE",

	install	: function() {
		window.addEventListener("load",this.init,false);
	},

	init	: function() { 
		if (navigator.mimeTypes[gJavaConsole_JDK_UNDERSCORE_VERSION.mimeType]) {
			var toolsPopup = document.getElementById("menu_ToolsPopup");	
			toolsPopup.addEventListener("popupshowing",gJavaConsole_JDK_UNDERSCORE_VERSION.enable,false);
			var element = document.getElementById(gJavaConsole_JDK_UNDERSCORE_VERSION.id);
			element.setAttribute( "oncommand" , "gJavaConsole_JDK_UNDERSCORE_VERSION.show();");
		} else {
			var element = document.getElementById(gJavaConsole_JDK_UNDERSCORE_VERSION.id);
			element.setAttribute("style", "display: none");
		}
	},

	enable	: function() {
		var element = document.getElementById(gJavaConsole_JDK_UNDERSCORE_VERSION.id);
    		if (navigator.javaEnabled()) {
			element.removeAttribute("disabled");
    		} else {
      			element.setAttribute("disabled", "true");
    		}
	},

	show	: function() {
     		var jvmMgr = Components.classes['@mozilla.org/oji/jvm-mgr;1']
	                   .getService(Components.interfaces.nsIJVMManager)
    		jvmMgr.showJavaConsole();
	}
	
};

gJavaConsole_JDK_UNDERSCORE_VERSION.install();


