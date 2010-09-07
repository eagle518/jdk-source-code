#!/bin/sh -f

# javac must be from JDK 1.5 or greater
javac CookieAppletTest.java
# Test certificate was generated with the following:
# keytool -genkey -dname "cn=Cookie Applet Test, ou=Java Software, o=Sun Microsystems, c=US" -alias test -keystore CookieAppletTest.keystore -storepass testtest
jar cvf CookieAppletTest.jar CookieAppletTest*.class
jarsigner -keystore CookieAppletTest.keystore -storepass testtest CookieAppletTest.jar test
