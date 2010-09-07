This directory contains the build infrastructure for a preliminary
and unsupported port of the new Java Plug-In to Mac OS X. 

What Is Here
------------

 - Ant-based build targets for:
   - The common deployment code and associated native code
   - The Java code for Java Web Start (but not the associated native
     code and launcher, yet)
   - The new Java Plug-In and associated native code

The compilation strategy for native code is to use the cpptasks Ant
library. The cpptasks build infrastructure was taken from the JOGL
project (http://jogl.dev.java.net/). (This is a Sun-owned project, so
there is no issue with repurposing the code or build mechanism.)

What Has Been Ported
--------------------

 - Rudimentary support in the Config class for detecting the available
   JREs on Mac OS X.
 - Enough structural support to allow Java Web Start to compile.
 - The new Java Plug-In in initial form.

What Has Not Been Ported
------------------------

 - Any interaction with the Apple-specific storage of deployment
   preferences, etc. The MacOSXConfig class subclasses UnixConfig and
   therefore will store the preferences in
   ~/.java/deployment/deployment.properties. It will ignore any
   settings made with Apple's Java Control Panel.
 - Java Web Start, including support for desktop integration, etc.
 - The Java Control Panel.

How to Build
------------

First, apply the deploy.patch in this directory to the root of your
workspace to comment out the Java Kernel dependencies.

Then source the "setup" script to get your shell set up to build. Note
that building the new plug-in requires Apple's JDK 6 to be installed,
although as on other platforms, it will run on top of earlier JDK
releases once compiled.

For each of deploy, javaws, and plugin (in that order), cd into the
directory and run "ant". The build results will go into deploy/build/ .

NOTE that the build produced by this process currently requires binary
plugs from Apple to be installed by hand in the build product in order
for the applets to show up in the correct place on the web page.

Installing the new Java Plug-In
-------------------------------

The new Java Plug-In can be installed in to Safari / WebKit and
Firefox for testing purposes.

The new Java Plug-In requires a very recent build of both browsers.
For WebKit, it requires a nightly build of WebKit dated 10/17 or later
from http://nightly.webkit.org/ .

For Firefox, a nightly build of Firefox 3.1 (called "Minefield") will
load the new Java Plug-In. Google for "firefox nightly builds" or go
to
http://ftp.mozilla.org/pub/mozilla.org/firefox/nightly/latest-trunk/
and download the latest .dmg.

To install the new Java Plug-In, follow these steps:

Quit all web browsers.
cd /Library/Internet\ Plug-Ins
sudo mkdir disabled
sudo mv JavaPluginCocoa.bundle disabled/
sudo mv ~/[...path to workspace]/deploy/build/plugin/obj/JavaPlugIn2.plugin .

Additionally, to allow Firefox 3.1 to recognize the new plug-in, you
must open the Firefox / Minefield app (in the Finder, "Show Package
Contents"), go into the folder "Contents/MacOS/plugins", make a new
folder called "disabled", and move the JavaEmbeddingPlugin.bundle and
MRJPlugin.plugin into it.

Then launch Safari or Firefox. The new Java Plug-In will be used to
execute applet content. Note that by default it will use JDK 6 (64-bit
only, at least at the present time) to execute applets. This may pose
problems for applets that pull in native code which is only compiled
for Mac OS X in 32-bit mode. It is possible to force the use of Java 5
by disabling the appropriate JRE entry in
~/.java/deployment/deployment.properties (which has to be hand-edited,
since there is no Java Control Panel which will edit it) -- for
example, set deployment.javaws.jre.0.enabled=false , but replace the
"0" with the JRE index corresponding to JRE 1.6.0.

To switch back to using the old Java Plug-In, quit Safari, remove the
JavaPlugIn2.plugin and move the JavaPluginCocoa.bundle back in to
place. Replace the JavaEmbeddingPlugin.bundle and MRJPlugin.plugin for
Firefox.

Known Issues
------------

 - Using Command-tab to switch between applications sometimes causes
   the applet's content to disappear. Clicking in the browser window
   should cause the applet's content to re-appear.

 - Cookie, proxy and browser authentication support is only working in
   Firefox on the Mac and not in Safari or any other browser.

 - Switching tabs in Safari while applets are visible does not work
   properly; applets on hidden tabs do not disappear. This is working
   correctly in Firefox 3.1, though. A bug has been filed to track
   this: https://bugs.webkit.org/show_bug.cgi?id=21651

 - The JavaScript syntax "new app.Packages.[Java class name]" is not
   working yet in Safari.

 - Jake2 does not work correctly apparently due to the AWT Robot being
   confused about where the applet window is on the screen.
