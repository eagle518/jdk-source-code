var javawsInstalled = 0;
var javawsJuniorInstalled = 0;

isIE = "false";

if (navigator.mimeTypes && navigator.mimeTypes.length) {
  x = navigator.mimeTypes['application/x-java-jnlp-file'];
  if (x) {
	javawsInstalled = 1;
	javawsJuniorInstalled = 1;
  }
} else { 
  isIE = "true";
}

function writeHTML(realUrl, imageUrl) {
  document.write("<center><div align=center>");
  document.write("<a href=" + realUrl + "><img src=" + imageUrl + " width=\"130\" height=\"107\" BORDER=\"0\"></a></div>");
  document.write("</center>");
}    

function launchLink(imageUrl, jnlpUrl) {
  launchAILink(imageUrl, jnlpUrl, "http://java.sun.com/cgi-bin/javawebstart-platform.sh?");
}

function launchLink2(imageUrl, jnlpUrl) {
  launchAILink2(imageUrl, jnlpUrl, "http://java.sun.com/cgi-bin/javawebstart-platform.sh?");
}

function launchAILink(imageUrl, jnlpUrl, AIUrl) {
  var realUrl = AIUrl;
  if (javawsInstalled) {
    realUrl = jnlpUrl;
  }
  writeHTML(realUrl, imageUrl);
}

function launchAILink2(imageUrl, jnlpUrl, AIUrl) {       
  var realUrl = AIUrl;
  if (javawsJuniorInstalled) {
    realUrl = jnlpUrl;
  }
  writeHTML(realUrl, imageUrl);
}
