/*
 * @@(#)ConverterHelp_ko.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * US English version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_ko extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Java(TM) Plug-in HTML \ubcc0\ud658\uae30 \ucd94\uac00 \uc815\ubcf4" + newline + newline +
      "\ubc84\uc804:  " + j2seVersion + newline + newline + newline +
      "*****   \uc774 \ub3c4\uad6c\ub97c \uc0ac\uc6a9\ud558\uc5ec \ud30c\uc77c\uc744 \ubcc0\ud658\ud558\uae30 \uc804\uc5d0 \ubaa8\ub450 \ubc31\uc5c5\ud558\uc2ed\uc2dc\uc624." + newline +
      "*****   \ubcc0\ud658\uc744 \ucde8\uc18c\ud574\ub3c4 \ubcc0\uacbd \uc0ac\ud56d\uc740 \ub864\ubc31\ub418\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." + newline +
      "*****   \uc560\ud50c\ub9bf \ud0dc\uadf8 \ub0b4\uc758 \uc8fc\uc11d\uc740 \ubb34\uc2dc\ub429\ub2c8\ub2e4." + newline + newline + newline +
      "\ubaa9\ucc28:" + newline +
      "   1.  \uc0c8 \uae30\ub2a5" + newline +
      "   2.  \ubc84\uadf8 \uc218\uc815" + newline +
      "   3.  Java(TM) Plug-in HTML \ubcc0\ud658\uae30 \uc815\ubcf4" + newline +
      "   4.  \ubcc0\ud658 \ud504\ub85c\uc138\uc2a4" + newline +
      "   5.  \ud3f4\ub354\uc5d0\uc11c \ubcc0\ud658 \ub300\uc0c1 \ud30c\uc77c \uc120\ud0dd" + newline +
      "   6.  \ubc31\uc5c5 \ud3f4\ub354 \uc120\ud0dd" + newline +
      "   7.  \ub85c\uadf8 \ud30c\uc77c \uc0dd\uc131" + newline +
      "   8.  \ubcc0\ud658 \ud15c\ud50c\ub9bf \uc120\ud0dd" + newline +
      "   9.  \ubcc0\ud658" + newline +
      "  10.  \ubcc0\ud658 \uacc4\uc18d \ub610\ub294 \uc885\ub8cc" + newline +
      "  11.  \ud15c\ud50c\ub9bf \uc0c1\uc138 \uc815\ubcf4" + newline +
      "  12.  HTML Converter \uc2e4\ud589(Windows \ubc0f Solaris)" + newline + newline +
      "1)  \uc0c8 \uae30\ub2a5:" + newline + newline +     
      "    o Netscape 6\uc744 \uc9c0\uc6d0\ud558\ub3c4\ub85d \ud655\uc7a5 \ud15c\ud50c\ub9bf \uc5c5\ub370\uc774\ud2b8" + newline +
      "    o Java Plug-in\uc758 \uc0c8 \ub2e4\uc911 \ubc84\uc804 \uae30\ub2a5\uc744 \uc9c0\uc6d0\ud558\ub3c4\ub85d \ubaa8\ub4e0 \ud15c\ud50c\ub9bf \uc5c5\ub370\uc774\ud2b8" + newline +
      "    o \uad6d\uc81c\ud654 \uae30\ub2a5\uc744 \uc9c0\uc6d0\ud558\uae30 \uc704\ud574 Swing 1.1\uc744 \uc0ac\uc6a9\ud574\uc11c \uc0ac\uc6a9\uc790 \uc778\ud130\ud398\uc774\uc2a4 \ud5a5\uc0c1" + newline +
      "    o \uc0c8 Smartupdate \uc9c0\uc6d0\uc744 \uc704\ud574\uc11c \uace0\uae09 \uc635\uc158 \ub300\ud654 \uc0c1\uc790 \ubc0f" + newline +
      "      MimeType \ud15c\ud50c\ub9bf \ud0dc\uadf8 \ud5a5\uc0c1" + newline +
      "    o Java Plug-in 1.1.x, Java Plug-in 1.2.x, " + newline +
      "      Java Plug-in 1.3.x, Java Plug-in 1.4.x \ubc0f" + newline +
      "      Java Plug-in 1.5.x\uc5d0\uc11c \ubaa8\ub450 \uc0ac\uc6a9\ud560 \uc218 \uc788\ub3c4\ub85d HTML Converter \ud5a5\uc0c1" + newline +
      "    o \ubaa8\ub4e0 \ubcc0\ud658 \ud15c\ud50c\ub9bf\uc758 SmartUpdate \ubc0f MimeType \uc9c0\uc6d0" + newline +
      "      \uac15\ud654" + newline +
      "    o \ubaa8\ub4e0 \ud15c\ud50c\ub9bf\uc5d0\uc11c OBJECT/EMBED \ud0dc\uadf8\uc5d0 \"scriptable=false\" \ucd94\uac00" + newline + newline + 
      "     \uc774\uac83\uc740 Java Plug-in\uc744 \uc2a4\ud06c\ub9bd\ud305\uc5d0 \uc0ac\uc6a9\ud558\uc9c0 \uc54a\uc744 \ub54c" + newline +
      "    typelib\uc774 \uc0dd\uc131\ub418\uc9c0 \uc54a\uac8c \ud569\ub2c8\ub2e4." + newline + newline + newline +
      "2)  \uc218\uc815\ub41c \ubc84\uadf8:" + newline + newline +
      "    o \uc18d\uc131 \ud30c\uc77c\uc744 \ucc3e\uc9c0 \ubabb\ud560 \uacbd\uc6b0 \uc624\ub958 \ucc98\ub9ac \uae30\ub2a5 \ud5a5\uc0c1" + newline +
      "    o HTML \ubcc0\ud658\uc744 \ud5a5\uc0c1\ud558\uc5ec \uacb0\uacfc EMBED/OBJECT \ud0dc\uadf8\ub97c" + newline +
      "      JDK 1.2.x\uc758 AppleViewer\uc5d0 \uc0ac\uc6a9\ud560 \uc218 \uc788\uc74c" + newline +
      "    o HTML Converter 1.1.x\uc5d0\uc11c \ubd88\ud544\uc694\ud55c \ub0a8\uc740 \ud30c\uc77c \uc81c\uac70" + newline +
      "    o JAVA_CODE, JAVA_CODEBASECODE \ub300\uc2e0 CODE, CODEBASE \ub4f1\uc758 \ud2b9\uc131 \uc774\ub984\uc744" + newline +
      "      \uc0ac\uc6a9\ud558\uc5ec ENBED/OBJECT \uc0dd\uc131. \ub530\ub77c\uc11c" + newline +
      "      \uc0dd\uc131\ub41c \ud398\uc774\uc9c0\ub97c JDK 1.2.x AppletViewer\uc5d0 \uc0ac\uc6a9\ud560 \uc218 \uc788\uc74c." + newline +
      "    o APPLET \ud0dc\uadf8\uc5d0 \uc788\ub294 \uacbd\uc6b0 MAYSCRIPT" + newline +
      "      \ubcc0\ud658 \uc9c0\uc6d0." + newline + newline +
      "3)  Java(TM) Plug-in HTML \ubcc0\ud658\uae30 \uc815\ubcf4:" + newline + newline +
      "        Java(TM) Plug-in HTML \ubcc0\ud658\uae30\ub294 \uc560\ud50c\ub9bf\uc744 \ud3ec\ud568\ud558\ub294 \ubaa8\ub4e0 HTML" + newline +
      "        \ud398\uc774\uc9c0\ub97c Java(TM) Plug-in\uc744 \uc0ac\uc6a9\ud558\ub294 \ud615\uc2dd\uc73c\ub85c \ubcc0\ud658\ud560 \uc218 \uc788\ub294" + newline +
      "        \uc720\ud2f8\ub9ac\ud2f0\uc785\ub2c8\ub2e4." + newline + newline +       
      "4)  \ubcc0\ud658 \ud504\ub85c\uc138\uc2a4:" + newline + newline +
      "        Java(TM) Plug-in HTML \ubcc0\ud658\uae30\ub294 \uc560\ud50c\ub9bf\uc744 \ud3ec\ud568\ud558\ub294 \ubaa8\ub4e0 \ud30c\uc77c\uc744" + newline +
      "        Java(TM) Plug-in\uacfc \ud568\uaed8 \uc0ac\uc6a9\ud560 \uc218 \uc788\ub294 \ud615\uc2dd\uc73c\ub85c \ubcc0\ud658\ud569\ub2c8\ub2e4." + newline + newline +
      "        \uac01 \ud30c\uc77c\uc758 \ubcc0\ud658 \ud504\ub85c\uc138\uc2a4\ub294 \ub2e4\uc74c\uacfc \uac19\uc2b5\ub2c8\ub2e4:" + newline +
      "        \uccab\uc9f8, \uc560\ud50c\ub9bf\uc5d0 \uc18d\ud558\uc9c0 \uc54a\ub294 HTML\uc774 \uc18c\uc2a4 \ud30c\uc77c\uc5d0\uc11c" + newline +
      "        \uc784\uc2dc \ud30c\uc77c\ub85c \uc804\uc1a1\ub429\ub2c8\ub2e4. <APPLET \ud0dc\uadf8\uc5d0 \ub3c4\ub2ec\ud558\uba74 \ubcc0\ud658\uae30\uac00" + newline +
      "        \uc560\ud50c\ub9bf\uc744 \uccab\ubc88\uc9f8 </APPLET \ud0dc\uadf8(\ub530\uc634\ud45c\ub85c \ubb36\uc5ec \uc788\uc9c0 \uc54a\uc74c)\ub85c \uad6c\ubb38 \ubd84\uc11d\ud558\uace0" + newline +
      "        \uc560\ud50c\ub9bf \ub370\uc774\ud130\uc640 \ud15c\ud50c\ub9bf\uc744 \ud1b5\ud569\ud569\ub2c8\ub2e4(\ud15c\ud50c\ub9bf\uc5d0 \ub300\ud55c \uc790\uc138\ud55c \ub0b4\uc6a9\uc740" + newline +
      "        \uc544\ub798 \ucc38\uc870). \uc624\ub958 \uc5c6\uc774 \uc791\uc5c5\uc774 \uc644\ub8cc\ub418\uba74 \uc6d0\ubcf8 html \ud30c\uc77c\uc774" + newline +
      "        \ubc31\uc5c5 \ud3f4\ub354\ub85c \uc774\ub3d9\ud558\uace0 \uc784\uc2dc \ud30c\uc77c\uc774 \uc6d0\ubcf8 \ud30c\uc77c \uc774\ub984\uc73c\ub85c" + newline +
      "        \ubc14\ub01d\ub2c8\ub2e4. \ub530\ub77c\uc11c \uc6d0\ubcf8 \ud30c\uc77c\uc740 \ub514\uc2a4\ud06c\uc5d0\uc11c \uc81c\uac70\ub418\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." + newline + newline +
      "        \ubcc0\ud658\uae30\ub294 \ud574\ub2f9 \uc704\uce58\uc758 \ud30c\uc77c\uc744 \ud6a8\uacfc\uc801\uc73c\ub85c \ubcc0\ud658\ud569\ub2c8\ub2e4. \uadf8\ub798\uc11c" + newline +
      "        \ubcc0\ud658\uae30\ub97c \uc77c\ub2e8 \uc2e4\ud589\ud558\uba74 Java(TM) Plug-in\uc744 \uc0ac\uc6a9\ud558\ub3c4\ub85d \ud30c\uc77c\uc774 \uc124\uc815\ub429\ub2c8\ub2e4." + newline +
       

      "5)  \ud3f4\ub354\uc5d0\uc11c \ubcc0\ud658 \ub300\uc0c1 \ud30c\uc77c \uc120\ud0dd:" + newline + newline +
      "       \ud3f4\ub354\uc758 \ud30c\uc77c \uc804\uccb4\ub97c \ubcc0\ud658\ud558\ub824\uba74 \ud3f4\ub354 \uacbd\ub85c\ub97c \uc785\ub825\ud558\uac70\ub098" + newline +
      "       \ucc3e\uc544\ubcf4\uae30 \ubc84\ud2bc\uc744 \uc120\ud0dd\ud558\uc5ec \ub300\ud654 \uc0c1\uc790\uc5d0\uc11c \ud3f4\ub354\ub97c \uc120\ud0dd\ud558\uc2ed\uc2dc\uc624." + newline  +
      "       \uc77c\ub2e8 \uacbd\ub85c\ub97c \uc120\ud0dd\ud558\uba74 \"\ud30c\uc77c \uc774\ub984 \uc77c\uce58\"\uc758" + newline +
      "       \ud30c\uc77c \uc9c0\uc815\uc790\uc5d0 \ubc88\ud638\ub97c \ubd80\uc5ec\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4. \uac01 \uc9c0\uc815\uc790\ub294 \uc27c\ud45c\ub85c \uad6c\ubd84\ud569\ub2c8\ub2e4. *\ub97c" + newline +
      "       \uc640\uc77c\ub4dc\uce74\ub4dc\ub85c \uc0ac\uc6a9\ud574\ub3c4 \ub429\ub2c8\ub2e4. \ud30c\uc77c \uc774\ub984\uc5d0 \uc640\uc77c\ub4dc\uce74\ub4dc\ub97c \uc0ac\uc6a9\ud558\uba74 \uadf8 \ud30c\uc77c \ud55c \uac1c\ub9cc" + newline +
      "       \ubcc0\ud658\ub429\ub2c8\ub2e4. \ub9c8\uc9c0\ub9c9\uc73c\ub85c \ud30c\uc77c \uc774\ub984\uacfc \uc77c\uce58\ud558\ub294 \uc911\ucca9 \ud3f4\ub354\uc5d0 \uc788\ub294" + newline +
      "        \ud30c\uc77c\uc744 \ubaa8\ub450 \ubcc0\ud658\ud558\uace0 \uc2f6\uc740 \uacbd\uc6b0 \"\ud558\uc704 \ud3f4\ub354 \ud3ec\ud568\" \ud655\uc778\ub780\uc744 \uc120\ud0dd\ud569\ub2c8\ub2e4." + newline + newline +      
      "6)  \ubc31\uc5c5 \ud3f4\ub354 \uc120\ud0dd:" + newline +

      "       \uae30\ubcf8 \ubc31\uc5c5 \ud3f4\ub354 \uacbd\ub85c\ub294 \uc774\ub984\uc5d0 \"_BAK\"\uac00 \ucca8\ubd80\ub41c \uc18c\uc2a4" + newline +
      "       \uacbd\ub85c\uc785\ub2c8\ub2e4. \uc18c\uc2a4 \uacbd\ub85c\uac00 c:/html/applet.html(\ud55c \uac1c \ud30c\uc77c \ubcc0\ud658)\uc778" + newline +
      "       \uacbd\uc6b0 \ubc31\uc5c5 \uacbd\ub85c\ub294 c:/html_BAK\uc785\ub2c8\ub2e4. \uc18c\uc2a4 \uacbd\ub85c\uac00" + newline +
      "       c:/html(\uacbd\ub85c\uc5d0 \ud574\ub2f9\ud558\ub294 \ubaa8\ub4e0 \ud30c\uc77c \ubcc0\ud658)\uc778 \uacbd\uc6b0 \ubc31\uc5c5 \uacbd\ub85c\ub294" + newline +
      "       c:/html_BAK\uc785\ub2c8\ub2e4. \ubc31\uc5c5 \uacbd\ub85c\ub97c \ubcc0\uacbd\ud558\ub824\uba74 \"\ud3f4\ub354\uc5d0 \ud30c\uc77c \ubc31\uc5c5:\" \uc606\uc758 \ud544\ub4dc\uc5d0" + newline +
      "        \uacbd\ub85c\ub97c \uc785\ub825\ud558\uac70\ub098 \ud3f4\ub354\ub97c \ucc3e\uc544\ubcf4\uba74 \ub429\ub2c8\ub2e4." + newline + newline +

      "       Unix(Solaris):" + newline +
      "       \uae30\ubcf8 \ubc31\uc5c5 \ud3f4\ub354 \uacbd\ub85c\ub294 \uc774\ub984\uc5d0 \"_BAK\"\uac00 \ucca8\ubd80\ub41c \uc18c\uc2a4" + newline +
      "       \uacbd\ub85c\uc785\ub2c8\ub2e4. \uc18c\uc2a4 \uacbd\ub85c\uac00 /home/user1/html/applet.html(\ud55c \uac1c \ud30c\uc77c \ubcc0\ud658)\uc774\uba74" + newline +
      "       \ubc31\uc5c5 \uacbd\ub85c\ub294 /home/user1/html_BAK\uc785\ub2c8\ub2e4. \uc18c\uc2a4" + newline +
      "       \uacbd\ub85c\uac00 /home/user1/html(\uacbd\ub85c\uc5d0 \ud574\ub2f9\ud558\ub294 \ubaa8\ub4e0 \ud30c\uc77c \ubcc0\ud658)\uc774\uba74 \ubc31\uc5c5 \uacbd\ub85c\ub294" + newline +
      "       /home/user1/html_BAK\uc785\ub2c8\ub2e4. \ubc31\uc5c5 \uacbd\ub85c\ub97c \ubcc0\uacbd\ud558\ub824\uba74 \"\ud3f4\ub354 \ud30c\uc77c \ubc31\uc5c5:\" \uc606\uc5d0" + newline +
      "       \uc788\ub294 \ud544\ub4dc\uc5d0 \uacbd\ub85c\ub97c \uc785\ub825\ud558\uac70\ub098 \ud3f4\ub354\ub97c \ucc3e\uc544\ubcf4\uba74 \ub429\ub2c8\ub2e4." + newline + newline +      
      "7)  \ub85c\uadf8 \ud30c\uc77c \uc0dd\uc131:" + newline + newline +
      "       \ub85c\uadf8 \ud30c\uc77c\uc744 \uc0dd\uc131\ud558\ub294 \uacbd\uc6b0 \"\ub85c\uadf8 \ud30c\uc77c \uc0dd\uc131\" \ud655\uc778\ub780\uc744" + newline +
      "       \uc120\ud0dd\ud569\ub2c8\ub2e4. \uacbd\ub85c\uc640 \ud30c\uc77c \uc774\ub984\uc744 \uc785\ub825\ud558\uac70\ub098 \ucc3e\uc544\ubcf4\uae30\ub97c \uc774\uc6a9\ud574\uc11c" + newline +
      "       \ud3f4\ub354\ub97c \uc120\ud0dd\ud55c \ub2e4\uc74c \ud30c\uc77c \uc774\ub984\uc744 \uc785\ub825\ud558\uace0 \uc5f4\uae30\ub97c \uc120\ud0dd\ud558\uba74 \ub429\ub2c8\ub2e4." + newline +
      "       \ub85c\uadf8 \ud30c\uc77c\uc5d0\ub294 \ubcc0\ud658 \ud504\ub85c\uc138\uc2a4\uc640 \uad00\ub828\ub41c \uae30\ubcf8 \uc815\ubcf4\uac00 \ub4e4\uc5b4" + newline +
      "       \uc788\uc2b5\ub2c8\ub2e4." + newline + newline +        
      "8)  \ubcc0\ud658 \ud15c\ud50c\ub9bf \uc120\ud0dd:" + newline + newline +
      "       \ud15c\ud50c\ub9bf\uc744 \uc120\ud0dd\ud558\uc9c0 \uc54a\uc740 \uacbd\uc6b0\ub294 \uae30\ubcf8 \ud15c\ud50c\ub9bf\uc744 \uc0ac\uc6a9\ud569\ub2c8\ub2e4. \uc774 \ud15c\ud50c\ub9bf\uc5d0\uc11c" + newline +
      "       IE \ubc0f Netscape\uc5d0 \uc0ac\uc6a9\ud560 \ubcc0\ud658\ub41c html \ud30c\uc77c\uc744 \ub9cc\ub4ed\ub2c8\ub2e4." + newline  +
      "       \ub2e4\ub978 \ud15c\ud50c\ub9bf\uc744 \uc0ac\uc6a9\ud558\uace0 \uc2f6\uc740 \uacbd\uc6b0\uc5d0\ub294 \uae30\ubcf8 \ud654\uba74 \uba54\ub274\uc5d0\uc11c \uc120\ud0dd\ud558\uba74" + newline +
      "       \ub429\ub2c8\ub2e4. \uae30\ud0c0\ub97c \uc120\ud0dd\ud558\uba74 \ud15c\ud50c\ub9bf\uc73c\ub85c \uc0ac\uc6a9\ud560 \ud30c\uc77c\uc744 \uc120\ud0dd\ud560 \uc218" + newline +
      "       \uc788\uc2b5\ub2c8\ub2e4." + newline +
      "       \ud30c\uc77c\uc744 \uc120\ud0dd\ud55c \uacbd\uc6b0 \ud15c\ud50c\ub9bf\uc774 \ub9de\ub294\uc9c0 \ud655\uc778\ud558\uc2ed\uc2dc\uc624." + newline + newline +
      "9)  \ubcc0\ud658:" + newline + newline +
      "       \"...\ubcc0\ud658\" \ubc84\ud2bc\uc744 \ub20c\ub7ec\uc11c \ubcc0\ud658 \ud504\ub85c\uc138\uc2a4\ub97c \uc2dc\uc791\ud569\ub2c8\ub2e4. \ud504\ub85c\uc138\uc2a4" + newline +
      "       \ub300\ud654 \uc0c1\uc790\uac00 \uc9c4\ud589 \uc911\uc778 \ud30c\uc77c\uacfc \ud30c\uc77c \ud504\ub85c\uc138\uc2a4 \uc218, \ucc3e\uc740 \uc560\ud50c\ub9bf \uc218," + newline +
      "       \ubc0f \ucc3e\uc740 \uc624\ub958 \uc218\ub97c \ud45c\uc2dc\ud569\ub2c8\ub2e4." + newline + newline +       
      "10) \ubcc0\ud658 \uacc4\uc18d \ub610\ub294 \uc885\ub8cc:" + newline + newline +
      "       \ubcc0\ud658\uc774 \uc644\uc131\ub418\uba74 \ud504\ub85c\uc138\uc2a4 \ub300\ud654 \uc0c1\uc790\uc758 \ubc84\ud2bc\uc774 \"\ucde8\uc18c\"\uc5d0\uc11c" + newline +
      "       \"\uc644\ub8cc\"\ub85c \ubc14\ub01d\ub2c8\ub2e4. \"\uc644\ub8cc\"\ub97c \uc120\ud0dd\ud574\uc11c \ub300\ud654 \uc0c1\uc790\ub97c \ub2eb\uc2b5\ub2c8\ub2e4." + newline  +
      "       \uc774\ub54c \"\uc885\ub8cc\"\ub97c \ub20c\ub7ec\uc11c Java(TM) Plug-in HTML \ubcc0\ud658\uae30\ub97c \ub2eb\uac70\ub098" + newline +
      "       \ubcc0\ud658\ud560 \ub2e4\ub978 \ud30c\uc77c \uc138\ud2b8\ub97c \uc120\ud0dd\ud55c \ub2e4\uc74c \"...\ubcc0\ud658\"\uc744 \ub2e4\uc2dc \uc120\ud0dd\ud558\uc2ed\uc2dc\uc624." + newline + newline +       
      "11)  \ud15c\ud50c\ub9bf \uc0c1\uc138 \uc815\ubcf4:" + newline + newline +
      "       \ud15c\ud50c\ub9bf \ud30c\uc77c\uc740 \uc560\ud50c\ub9bf\uc744 \ubcc0\ud658\ud560 \ub54c \uae30\ubcf8\uc73c\ub85c \uc0ac\uc6a9\ud558\ub294 \ud30c\uc77c\ub85c\uc11c \uc6d0\ubcf8 \uc560\ud50c\ub9bf\uc758" + newline +
      "       \uc77c\ubd80\ub97c \ub098\ud0c0\ub0b4\ub294 \ud0dc\uadf8\ub97c \ud3ec\ud568\ud55c \ub2e8\uc21c \ud14d\uc2a4\ud2b8 \ud30c\uc77c\uc785\ub2c8\ub2e4." + newline +
      "       \ud15c\ud50c\ub9bf \ud30c\uc77c\uc5d0 \ud0dc\uadf8\ub97c \ucd94\uac00/\uc0ad\uc81c/\uc774\ub3d9\ud568\uc73c\ub85c\uc368 \ubcc0\ud658\ub41c \ud30c\uc77c\uc758 \ucd9c\ub825\uc744" + newline +
      "       \ubcc0\uacbd\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." + newline + newline +
      "       \uc9c0\uc6d0\ub418\ub294 \ud0dc\uadf8:" + newline + newline +
      "        $OriginalApplet$    \uc774 \ud0dc\uadf8\ub294 \uc6d0\ubcf8 \uc560\ud50c\ub9bf\uc758 \uc644\uc804 \ud14d\uc2a4\ud2b8\ub97c" + newline +
      "        \ub300\uccb4\ud569\ub2c8\ub2e4." + newline + newline +
      "        $AppletAttributes$   \uc774 \ud0dc\uadf8\ub294 \ubaa8\ub4e0 \uc560\ud50c\ub9bf \ubcc0\uc218\ub97c" + newline +
      "        \ub300\uccb4\ud569\ub2c8\ub2e4(\ucf54\ub4dc, \ucf54\ub4dc\ubca0\uc774\uc2a4, \ub108\ube44, \ub192\uc774 \ub4f1)" + newline + newline +
      "        $ObjectAttributes$   \uc774 \ud0dc\uadf8\ub294 \uac1d\uccb4 \ud0dc\uadf8\uc5d0 \ud544\uc694\ud55c \ubaa8\ub4e0 \ud2b9\uc131\uc744" + newline +
      "        \ub300\uccb4\ud569\ub2c8\ub2e4." + newline + newline +
      "        $EmbedAttributes$   \uc774 \ud0dc\uadf8\ub294 embed \ud0dc\uadf8\uc5d0 \ud544\uc694\ud55c \ubaa8\ub4e0 \ud2b9\uc131\uc744" + newline +
      "        \ub300\uccb4\ud569\ub2c8\ub2e4." + newline + newline +
      "        $AppletParams$    \uc774 \ud0dc\uadf8\ub294 \uc560\ud50c\ub9bf\uc758 \ubaa8\ub4e0" + newline +
      "        <param ...> \ud0dc\uadf8\ub97c \ub300\uccb4\ud569\ub2c8\ub2e4" + newline + newline +
      "        $ObjectParams$    \uc774 \ud0dc\uadf8\ub294 \uac1d\uccb4 \ud0dc\uadf8\uc5d0 \ud544\uc694\ud55c \ubaa8\ub4e0 <param...>" + newline +
      "        \ud0dc\uadf8\ub97c \ub300\uccb4\ud569\ub2c8\ub2e4." + newline + newline +
      "        $EmbedParams$     \uc774 \ud0dc\uadf8\ub294 NAME=VALUE \ud615\uc2dd\uc758 embed \ud0dc\uadf8\uc5d0 \ud544\uc694\ud55c \ubaa8\ub4e0 <param...>" + newline +
      "        \ud0dc\uadf8\ub97c \ub300\uccb4\ud569\ub2c8\ub2e4" + newline + newline +
      "        $AlternateHTML$  \uc774 \ud0dc\uadf8\ub294 \uc6d0\ubcf8 \uc560\ud50c\ub9bf\uc5d0\uc11c \uc560\ud50c\ub9bf\uc744 \uc9c0\uc6d0\ud558\uc9c0" + newline +
      "        \uc54a\ub294 \uc601\uc5ed\uc758 \ud14d\uc2a4\ud2b8\ub97c \ub300\uccb4\ud569\ub2c8\ub2e4." + newline + newline +
      "        $CabFileLocation$   \uc774\uac83\uc740 IE\ub97c \ub300\uc0c1\uc73c\ub85c \ud558\ub294 \uac01 \ud15c\ud50c\ub9bf\uc5d0" + newline +
      "        \uc0ac\uc6a9\ud574\uc57c \ud558\ub294 cab \ud30c\uc77c\uc758 URL\uc785\ub2c8\ub2e4." + newline + newline +
      "        $NSFileLocation$    \uc774\uac83\uc740 Netscape\ub97c \ub300\uc0c1\uc73c\ub85c \ud558\ub294 \uac01 \ud15c\ud50c\ub9bf\uc5d0" + newline +
      "        \uc0ac\uc6a9\ud574\uc57c \ud558\ub294 Netscape Plugin\uc758 URL\uc785\ub2c8\ub2e4." + newline + newline +
      "        $SmartUpdate$   \uc774\uac83\uc740 Netscape Navigator 4.0 \uc774\uc0c1\uc744 \ub300\uc0c1\uc73c\ub85c \ud558\ub294 \uac01 \ud15c\ud50c\ub9bf\uc5d0" + newline +
      "        \uc0ac\uc6a9\ud574\uc57c \ud558\ub294 Netscape SmartUpdate\uc758 URL\uc785\ub2c8\ub2e4." + newline + newline +
      "        $MimeType$    \uc774\uac83\uc740 Java \uac1d\uccb4\uc758 MIME \ud615\uc2dd\uc785\ub2c8\ub2e4." + newline + newline +     
      "      default.tpl (\ubcc0\ud658\uae30\uc758 \uae30\ubcf8 \ud15c\ud50c\ub9bf) -- \ubcc0\ud658\ub41c \ud398\uc774\uc9c0\ub97c" + newline +
      "      Windows\uc758 IE\uc640 Navigator\uc5d0\uc11c \uc0ac\uc6a9\ud558\uc5ec Java(TM) Plug-in\uc744 \ud638\ucd9c\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." + newline +
      "      \uc774 \ud15c\ud50c\ub9bf\uc740 Unix(Solaris) Netscape\uc5d0\ub3c4 \uc0ac\uc6a9\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- \ubcc0\ud658\ub41c \ud398\uc774\uc9c0\ub97c \uc0ac\uc6a9\ud558\uc5ec Windows \uc804\uc6a9 IE\uc5d0\uc11c Java(TM)" + newline +
      "      Plug-in\uc744 \ud638\ucd9c\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- \ubcc0\ud658\ub41c \ud398\uc774\uc9c0\ub97c \uc0ac\uc6a9\ud558\uc5ec Windows\uc640 Solaris\uc758 Navigator\uc5d0\uc11c Java(TM)" + newline +
      "      Plug-in\uc744 \ud638\ucd9c\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- \ubcc0\ud658\ub41c \ud398\uc774\uc9c0\ub294 \ubaa8\ub4e0 \ube0c\ub77c\uc6b0\uc800\ub098 \ud50c\ub7ab\ud3fc\uc5d0 \uc0ac\uc6a9\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4." + newline +
      "      \ube0c\ub77c\uc6b0\uc800\uac00 Windows\uc758 IE \ub610\ub294 Navigator(Solaris\uc758 Navigator)\uc778 \uacbd\uc6b0 Java(TM)" + newline +
      "      Plug-in\uc774 \ud638\ucd9c\ub429\ub2c8\ub2e4. \uadf8\ub807\uc9c0 \uc54a\uc73c\uba74 \ube0c\ub77c\uc6b0\uc800\uc758 \uae30\ubcf8 JVM\uc744 \uc0ac\uc6a9\ud569\ub2c8\ub2e4." + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  HTML Converter \uc2e4\ud589:" + newline + newline +
      "      HTML Converter\uc758 GUI \ubc84\uc804 \uc2e4\ud589 " + newline + newline +
      "      HTML Converter\ub294 JRE\uac00 \uc544\ub2c8\ub77c JDK\uc5d0 \ud3ec\ud568\ub429\ub2c8\ub2e4. \ubcc0\ud658\uae30\ub97c \uc2e4\ud589\ud558\ub824\uba74 JDK" + newline +
      "      \uc124\uce58 \ub514\ub809\ud1a0\ub9ac\uc758 lib \ud558\uc704 \ub514\ub809\ud1a0\ub9ac\ub85c \uc774\ub3d9\ud569\ub2c8\ub2e4. \uc608\ub97c \ub4e4\uc5b4" + newline +
      "      Windows\uc5d0 C:\\jdk" + j2seVersion + "\ub85c JDK\ub97c \uc124\uce58\ud55c \uacbd\uc6b0 \ub2e4\uc74c \uc704\uce58\ub85c \uc774\ub3d9\ud569\ub2c8\ub2e4." + newline + newline +
      "            C:\\jdk" + j2seVersion +  "\\lib\\" + newline + newline +
      "      \ubcc0\ud658\uae30(htmlconverter.jar)\ub294 \uadf8 \ub514\ub809\ud1a0\ub9ac\uc5d0 \ud3ec\ud568\ub418\uc5b4 \uc788\uc2b5\ub2c8\ub2e4." + newline + newline +
      "      \ubcc0\ud658\uae30\ub97c \uc2e4\ud589\ud558\ub824\uba74 \ub2e4\uc74c\uc744 \uc785\ub825\ud569\ub2c8\ub2e4." + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      UNIX/Linux\uc5d0\uc11c \ubcc0\ud658\uae30\ub97c \uc2e4\ud589\ud558\ub294 \uba85\ub839\uc740 \uc704\uc640 \ube44\uc2b7\ud569\ub2c8\ub2e4." + newline +
      "      \ub2e4\uc74c\uc740 Windows\uc5d0\uc11c \ubcc0\ud658\uae30\ub97c \uc2dc\uc791\ud558\ub294 \ub2e4\ub978" + newline + newline +
      "      \ubc29\ubc95\uc785\ub2c8\ub2e4." + newline +
      "      \ud0d0\uc0c9\uae30\ub97c \uc0ac\uc6a9\ud558\uc5ec \ubcc0\ud658\uae30\ub97c \uc2e4\ud589\ud558\ub824\uba74" + newline +
      "      \ud0d0\uc0c9\uae30\ub97c \uc0ac\uc6a9\ud558\uc5ec \ub2e4\uc74c \ub514\ub809\ud1a0\ub9ac\ub97c \ud0d0\uc0c9\ud558\uc2ed\uc2dc\uc624." + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      HtmlConverter \uc751\uc6a9 \ud504\ub85c\uadf8\ub7a8\uc744 \ub450 \ubc88 \ub204\ub974\uc2ed\uc2dc\uc624." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      \ub2e4\uc74c \uba85\ub839\uc744 \uc2e4\ud589\ud558\uc2ed\uc2dc\uc624." + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +             
      "      \uba85\ub839\uc904\uc5d0\uc11c \ubcc0\ud658\uae30 \uc2e4\ud589:" + newline + newline +
      "      \ud615\uc2dd:" + newline + newline +
      "      java -jar htmlconverter.jar [-options1 value1 [-option2 value2" + newline +
      "      [...]]] [-simulate] [filespecs]" + newline + newline +
      "      filespecs:  \ud30c\uc77c \uc0ac\uc591\uc5d0\uc11c \uacf5\uac04 \uc81c\ud55c\uc774 \uc5c6\ub294 \ubaa9\ub85d, * \uc640\uc77c\ub4dc\uce74\ub4dc. " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      \uc635\uc158:" + newline + newline +
      "       source:    \ud30c\uc77c \uacbd\ub85c. (Windows\uc758 \uacbd\uc6b0 c:\\htmldocs," + newline +
      "                  Unix\uc758 \uacbd\uc6b0 /home/user1/htmldocs) \uae30\ubcf8\uac12: <userdir>" + newline +
      "                  \uacbd\ub85c\uac00 \uc0c1\ub300\uc801\uc778 \uacbd\uc6b0 HTMLConverter\uac00 \uc2e4\ud589\ub41c \ub514\ub809\ud1a0\ub9ac\uc5d0 \ub300\ud574" + newline +
      "                  \uc0c1\ub300\uc801\uc778 \uacbd\ub85c\ub77c\uace0 \uac00\uc815\ud569\ub2c8\ub2e4." + newline + newline +
      "       backup:    \ubc31\uc5c5 \ud30c\uc77c \uacbd\ub85c.  \uae30\ubcf8\uac12:" + newline +
      "                  <userdir>/<source>_bak" + newline +
      "                  \uacbd\ub85c\uac00 \uc0c1\ub300\uc801\uc778 \uacbd\uc6b0 HTMLConverter\uac00 \uc2e4\ud589\ub41c \ub514\ub809\ud1a0\ub9ac\uc5d0 \ub300\ud574" + newline +
      "                  \uc0c1\ub300\uc801\uc778 \uacbd\ub85c\ub77c\uace0 \uac00\uc815\ud569\ub2c8\ub2e4." + newline + newline +
      "       subdirs:   \ud558\uc704 \ub514\ub809\ud1a0\ub9ac\uc758 \ud30c\uc77c\uc744 \ucc98\ub9ac\ud569\ub2c8\ub2e4. " + newline +
      "                  \uae30\ubcf8\uac12:  FALSE" + newline + newline +
      "       template:  \ud15c\ud50c\ub9bf \ud30c\uc77c\uc758 \uacbd\ub85c.  \uae30\ubcf8\uac12:  default.tpl-Standard " + newline +
      "                  Windows \ubc0f Solaris\uc6a9 (IE \ubc0f Navigator). \ubd88\ud655\uc2e4\ud55c \uacbd\uc6b0 \uae30\ubcf8\uac12\uc744 \uc0ac\uc6a9\ud558\uc2ed\uc2dc\uc624." + newline + newline +
      "       log:       \ub85c\uadf8\ub97c \uc791\uc131\ud560 \uacbd\ub85c\uc640 \ud30c\uc77c \uc774\ub984.  (\uae30\ubcf8\uac12 <userdir>/convert.log)" + newline + newline +
      "       process:   \ubcc0\ud658 \uc911 \ud45c\uc900 \uc9c4\ud589 \uacfc\uc815\uc744 \ud45c\uc2dc\ud569\ub2c8\ub2e4. " + newline +
      "                  \uae30\ubcf8\uac12: false" + newline + newline +
      "       simulate:  \ubcc0\ud658\ud558\uc9c0 \uc54a\uace0 \ubcc0\ud658 \uc0ac\uc591\uc744 \ud45c\uc2dc\ud569\ub2c8\ub2e4." + newline +
      "                  \ubcc0\ud658\uc5d0 \ub300\ud574 \uc798 \ubaa8\ub974\ub294 \uacbd\uc6b0 \uc774 \uc635\uc158\uc744 \uc0ac\uc6a9\ud558\uc2ed\uc2dc\uc624." + newline +
      "                  \ubcc0\ud658\uc5d0 \uad00\ud55c \uc0c1\uc138 \uc815\ubcf4\ub97c \uc5bb\uc744 \uc218" + newline +
      "                  \uc788\uc2b5\ub2c8\ub2e4." + newline + newline +
      "      \"java -jar htmlconverter.jar -gui\"\ub9cc \uc9c0\uc815\ud558\uba74(filespecs \uc5c6\uc774 gui\ub9cc \uc9c0\uc815)" + newline +
      "      \ubcc0\ud658\uae30\uc758 GUI \ubc84\uc804\uc774 \uc2e4\ud589\ub429\ub2c8\ub2e4." + newline  +
      "      \uadf8\ub807\uc9c0 \uc54a\uc73c\uba74 GUI\uac00 \uba48\ucda5\ub2c8\ub2e4." + newline + newline +
      "      \uc790\uc138\ud55c \uc815\ubcf4\ub294 \ub2e4\uc74c \uc8fc\uc18c\ub97c \ucc38\uace0\ud558\uc2ed\uc2dc\uc624." + newline + newline +
      "      http://java.sun.com/j2se/" + 
        (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
        "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
} 

 

