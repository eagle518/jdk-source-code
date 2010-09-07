/*
 * @(#)ConverterHelp_zh_TW.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * Traditional Chinese version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_zh_TW extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Java(TM) Plug-in HTML Converter \u8b80\u6211" + newline + newline +
      "\u7248\u672c\uff1a  " + j2seVersion + newline + newline + newline +
      "*****   \u8acb\u5728\u4f7f\u7528\u6b64\u5de5\u5177\u8f49\u63db\u4e4b\u524d\u5099\u4efd\u6240\u6709\u6a94\u6848\u3002" + newline +
      "*****   \u53d6\u6d88\u8f49\u63db\u4e0d\u6703\u5fa9\u539f\u8b8a\u66f4\u3002" + newline +
      "*****   APPLET \u6a19\u7c64\u4e2d\u7684\u8a3b\u91cb\u88ab\u5ffd\u7565\u3002" + newline + newline + newline +
      "\u76ee\u9304\uff1a" + newline +
      "   1.  \u65b0\u589e\u529f\u80fd" + newline +
      "   2.  \u932f\u8aa4\u4fee\u6b63" + newline +
      "   3.  \u95dc\u65bc Java(TM) Plug-in HTML Converter" + newline +
      "   4.  \u8f49\u63db\u7a0b\u5e8f" + newline +
      "   5.  \u5728\u8cc7\u6599\u593e\u4e2d\u9078\u64c7\u8981\u8f49\u63db\u7684\u6a94\u6848" + newline +
      "   6.  \u9078\u64c7\u5099\u4efd\u8cc7\u6599\u593e" + newline +
      "   7.  \u7522\u751f\u65e5\u8a8c\u6a94" + newline +
      "   8.  \u9078\u64c7\u8f49\u63db\u7bc4\u672c" + newline +
      "   9.  \u8f49\u63db" + newline +
      "  10.  \u7e7c\u7e8c\u8f49\u63db\u6216\u9000\u51fa" + newline +
      "  11.  \u95dc\u65bc\u7bc4\u672c\u7684\u8a73\u7d30\u8cc7\u8a0a" + newline +
      "  12.  \u57f7\u884c HTML Converter (Windows \u548c Solaris)" + newline + newline +
      "1)  \u65b0\u589e\u529f\u80fd\uff1a" + newline + newline +
      "    o \u5df2\u66f4\u65b0\u5ef6\u4f38\u7bc4\u672c\uff0c\u53ef\u652f\u63f4 Netscape 6\u3002" + newline +
      "    o \u5df2\u66f4\u65b0\u6240\u6709\u7bc4\u672c\uff0c\u53ef\u652f\u63f4 Java Plug-in \u4e2d\u65b0\u7684\u591a\u7248\u672c\u529f\u80fd\u3002" + newline +
      "    o \u5df2\u4f7f\u7528 Swing 1.1 \u589e\u5f37\u4f7f\u7528\u8005\u4ecb\u9762\uff0c\u53ef\u652f\u63f4 i18n\u3002" + newline +
      "    o \u5df2\u589e\u5f37\u300c\u9032\u968e\u9078\u9805\u300d\u5c0d\u8a71\u65b9\u584a\uff0c\u53ef\u652f\u63f4\u65b0\u7684 SmartUpdate \u548c" + newline +
      "       MimeType \u7bc4\u672c\u6a19\u7c64\u3002" + newline +
      "    o \u5df2\u589e\u5f37 HTML Converter\uff0c\u53ef\u8207 Java Plug-in 1.1.x\u3001" + newline +
      "      Java Plug-in 1.2.x\u3001Java Plug-in 1.3.x\u3001Java Plug-in 1.4.x" + newline +
      "       \u548c Java Plug-in 1.5.x \u914d\u5408\u4f7f\u7528\u3002" + newline +
      "    o \u5df2\u589e\u5f37\u6240\u6709\u8f49\u63db\u7bc4\u672c\u4e2d\u7684 SmartUpdate \u548c MimeType" + newline +
      "       \u652f\u63f4\u3002" + newline +
      "    o \u5df2\u5c07\u300cscriptable=false\u300d\u65b0\u589e\u81f3\u6240\u6709\u7bc4\u672c\u4e2d\u7684 OBJECT/EMBED \u6a19\u7c64\u3002" + newline + newline +
      "     \u7576 Java Plug-in \u672a\u7528\u65bc\u7a0b\u5e8f\u6a94\u7de8\u8f2f\u6642\uff0c\u53ef\u4f7f\u7528\u8a72\u529f\u80fd\u4f86" + newline +
      "    \u505c\u7528 typelib \u7684\u7522\u751f\u3002" + newline + newline + newline +
      "2)  \u932f\u8aa4\u4fee\u6b63\uff1a" + newline + newline +
      "    o \u5df2\u589e\u5f37\u627e\u4e0d\u5230\u5c6c\u6027\u6a94\u6848\u6642\u7684\u932f\u8aa4\u8655\u7406\u529f\u80fd\u3002" + newline +
      "    o \u5df2\u589e\u5f37 HTML \u8f49\u63db\u529f\u80fd\uff0c\u5f9e\u800c\u4f7f\u7d50\u679c EMBED/OBJECT \u6a19\u7c64" + newline +
      "      \u53ef\u7528\u65bc JDK 1.2.x \u4e2d\u7684 AppletViewer\u3002" + newline +
      "    o \u5df2\u79fb\u9664 HTML Converter 1.1.x \u907a\u7559\u4e0b\u4f86\u7684\u4e0d\u5fc5\u8981\u6a94\u6848\u3002" + newline +
      "    o \u7522\u751f\u7684 EMBED/OBJECT \u5e36\u6709 CODE\u3001CODEBASE \u7b49\u5c6c\u6027\u540d\u7a31\uff0c" + newline +
      "      \u800c\u975e JAVA_CODE\u3001JAVA_CODEBASE \u7b49\u540d\u7a31\u3002\u9019\u4f7f\u5f97" + newline +
      "      \u7522\u751f\u7684\u9801\u9762\u53ef\u7528\u65bc JDK 1.2.x AppletViewer\u3002" + newline +
      "    o \u5982\u679c MAYSCRIPT \u51fa\u73fe\u5728" + newline +
      "       APPLET \u6a19\u7c64\u4e2d\uff0c\u5247\u652f\u63f4  MAYSCRIPT \u8f49\u63db\u3002" + newline + newline +
      "3)  \u95dc\u65bc Java(TM) Plug-in HTML Converter\uff1a" + newline + newline +
      "        Java(TM) Plug-in HTML Converter \u662f\u4e00\u7a2e\u516c\u7528\u7a0b\u5f0f\uff0c\u53ef\u8b93\u60a8\u5c07\u4efb\u4f55" + newline +
      "        \u5305\u542b applet \u7684 HTML \u9801\u9762\u8f49\u63db\u70ba\u4f7f\u7528 Java(TM)" + newline +
      "         Plug-in \u7684\u683c\u5f0f\u3002" + newline + newline +
      "4)  \u8f49\u63db\u7a0b\u5e8f\uff1a" + newline + newline +
      "        Java(TM) Plug-in HTML Converter \u53ef\u5c07\u4efb\u4f55\u5305\u542b applet \u7684\u6a94\u6848\u8f49\u63db\u70ba" + newline +
      "        \u53ef\u8207 Java(TM) Plug-in \u914d\u5408\u4f7f\u7528\u7684\u683c\u5f0f\u3002" + newline + newline +
      "        \u8f49\u63db\u6bcf\u500b\u6a94\u6848\u7684\u7a0b\u5e8f\u5982\u4e0b\uff1a" + newline +
      "        \u9996\u5148\uff0c\u5c07\u4e0d\u5c6c\u65bc applet \u7684 HTML \u5f9e\u4f86\u6e90\u6a94\u50b3\u9001\u81f3" + newline +
      "        \u66ab\u6642\u6a94\u6848\u3002\u7576\u9047\u5230 <APPLET \u6a19\u7c64\u6642\uff0c\u8f49\u63db\u7a0b\u5f0f" + newline +
      "        \u6703\u5256\u6790 applet \u76f4\u81f3\u7b2c\u4e00\u500b </APPLET \u6a19\u7c64 (\u4e0d\u5305\u542b\u5728\u5f15\u865f\u4e2d)\uff0c" + newline +
      "        \u4e26\u5c07 applet \u8cc7\u6599\u8207\u7bc4\u672c\u5408\u4f75\u3002(\u8acb\u53c3\u95b1\u4e0b\u9762\u95dc\u65bc\u7bc4\u672c\u7684\u8a73\u7d30\u8cc7\u8a0a)\uff0c" + newline +
      "        \u5982\u679c\u6210\u529f\u5b8c\u6210\uff0c\u5247\u539f\u59cb\u7684 html \u6a94\u6848\u5c07\u88ab\u79fb\u81f3" + newline +
      "        \u5099\u4efd\u8cc7\u6599\u593e\uff0c\u66ab\u6642\u6a94\u6848\u5c07\u88ab\u91cd\u65b0\u547d\u540d\u70ba\u539f\u59cb\u6a94\u6848\u7684" + newline +
      "        \u540d\u7a31\u3002\u9019\u6a23\uff0c\u60a8\u7684\u539f\u59cb\u6a94\u6848\u4fbf\u6c38\u9060\u4e0d\u6703\u5f9e\u78c1\u789f\u4e2d\u88ab\u79fb\u9664\u3002" + newline + newline +
      "        \u8acb\u6ce8\u610f\uff0c\u8f49\u63db\u7a0b\u5f0f\u6703\u5728\u9069\u7576\u7684\u4f4d\u7f6e\u6709\u6548\u5730\u8f49\u63db\u6a94\u6848\u3002\u56e0\u6b64\uff0c" + newline +
      "        \u4e00\u65e6\u57f7\u884c\u8f49\u63db\u7a0b\u5f0f\uff0c\u60a8\u7684\u6a94\u6848\u4fbf\u6703\u88ab\u8a2d\u5b9a\u70ba\u4f7f\u7528 Java(TM) Plug-in\u3002" + newline +


      "5)  \u5728\u8cc7\u6599\u593e\u4e2d\u9078\u64c7\u8981\u8f49\u63db\u7684\u6a94\u6848\uff1a" + newline + newline +
      "       \u82e5\u8981\u8f49\u63db\u8cc7\u6599\u593e\u4e2d\u7684\u6240\u6709\u6a94\u6848\uff0c\u60a8\u53ef\u4ee5\u9375\u5165\u8a72\u8cc7\u6599\u593e\u7684\u8def\u5f91\uff0c" + newline +
      "       \u6216\u9078\u64c7\u300c\u700f\u89bd\u300d\u6309\u9215\uff0c\u5f9e\u5c0d\u8a71\u65b9\u584a\u4e2d\u9078\u53d6\u8cc7\u6599\u593e\u3002" + newline  +
      "       \u4e00\u65e6\u9078\u64c7\u8def\u5f91\uff0c\u60a8\u4fbf\u53ef\u4ee5\u5728\u300c\u76f8\u7b26\u7684\u6a94\u6848\u540d\u7a31\u300d\u4e2d\u8f38\u5165\u4efb\u610f\u6578\u76ee\u7684" + newline +
      "       \u6a94\u6848\u6307\u5b9a\u689d\u4ef6\u3002\u5404\u500b\u6307\u5b9a\u689d\u4ef6\u5fc5\u9808\u7528\u9017\u865f\u5206\u958b\u3002\u60a8\u53ef\u4ee5\u5c07 * \u7528\u4f5c" + newline +
      "       \u842c\u7528\u5b57\u5143\u3002\u5982\u679c\u60a8\u8f38\u5165\u7684\u6a94\u6848\u540d\u7a31\u5e36\u6709\u842c\u7528\u5b57\u5143\uff0c\u5247\u53ea\u6709\u8a72\u6a94\u6848" + newline +
      "       \u6703\u88ab\u8f49\u63db\u3002\u6700\u5f8c\uff0c\u5982\u679c\u60a8\u8981\u8f49\u63db\u5d4c\u5957\u8cc7\u6599\u593e\u4e2d\u7b26\u5408\u8a72\u6a94\u6848\u540d\u7a31" + newline +
      "       \u7684\u6240\u6709\u6a94\u6848\uff0c\u8acb\u9078\u53d6\u300c\u5305\u62ec\u5b50\u8cc7\u6599\u593e\u300d\u6838\u53d6\u65b9\u584a\u3002" + newline + newline +
      "6)  \u9078\u64c7\u5099\u4efd\u8cc7\u6599\u593e\uff1a" + newline +

      "       \u9810\u8a2d\u7684\u5099\u4efd\u8cc7\u6599\u593e\u8def\u5f91\u662f\u5728\u4f86\u6e90\u8def\u5f91\u540d\u7a31\u5f8c\u9644\u52a0\u300c_BAK\u300d\u3002" + newline +
      "       \u5373\uff0c\u5982\u679c\u4f86\u6e90\u8def\u5f91\u70ba c:/html/applet.html (\u8f49\u63db\u4e00\u500b\u6a94\u6848)\uff0c" + newline +
      "       \u5247\u5099\u4efd\u8def\u5f91\u70ba c:/html_BAK\u3002\u5982\u679c\u4f86\u6e90\u8def\u5f91" + newline +
      "       \u70ba c:/html (\u8f49\u63db\u8def\u5f91\u4e2d\u7684\u6240\u6709\u6a94\u6848)\uff0c\u5247\u5099\u4efd\u8def\u5f91\u70ba" + newline +
      "        c:/html_BAK\u3002\u53ef\u4ee5\u900f\u904e\u5728\u300c\u5c07\u6a94\u6848\u5099\u4efd\u81f3\u8cc7\u6599\u593e\uff1a\u300d\u65c1\u908a\u7684\u6b04\u4f4d\u4e2d" + newline +
      "       \u9375\u5165\u8def\u5f91\u4f86\u8b8a\u66f4\u5099\u4efd\u8def\u5f91\uff0c\u6216\u900f\u904e\u700f\u89bd\u8cc7\u6599\u593e\u4f86\u8b8a\u66f4\u5099\u4efd\u8def\u5f91\u3002" + newline + newline +

      "       Unix(Solaris)\uff1a" + newline +
      "       \u9810\u8a2d\u7684\u5099\u4efd\u8cc7\u6599\u593e\u8def\u5f91\u662f\u5728\u4f86\u6e90\u8def\u5f91\u540d\u7a31\u5f8c\u9644\u52a0\u300c_BAK\u300d\u3002" + newline +
      "       \u5373\uff0c\u5982\u679c\u4f86\u6e90\u8def\u5f91\u70ba /home/user1/html/applet.html (\u8f49\u63db\u4e00\u500b\u6a94\u6848)\uff0c" + newline +
      "       \u5247\u5099\u4efd\u8def\u5f91\u70ba /home/user1/html_BAK\u3002\u5982\u679c\u4f86\u6e90\u8def\u5f91" + newline +
      "       \u70ba /home/user1/html (\u8f49\u63db\u8def\u5f91\u4e2d\u7684\u6240\u6709\u6a94\u6848)\uff0c\u5247\u5099\u4efd\u8def\u5f91\u70ba" + newline +
      "        /home/user1/html_BAK\u3002\u53ef\u4ee5\u900f\u904e\u5728\u300c\u5c07\u6a94\u6848\u5099\u4efd\u81f3\u8cc7\u6599\u593e\uff1a\u300d\u65c1\u908a\u7684\u6b04\u4f4d\u4e2d" + newline +
      "       \u9375\u5165\u8def\u5f91\u4f86\u8b8a\u66f4\u5099\u4efd\u8def\u5f91\uff0c\u6216\u900f\u904e\u700f\u89bd\u8cc7\u6599\u593e\u4f86\u8b8a\u66f4\u5099\u4efd\u8def\u5f91\u3002" + newline + newline +
      "7)  \u7522\u751f\u65e5\u8a8c\u6a94\uff1a" + newline + newline +
      "       \u5982\u679c\u60a8\u8981\u7522\u751f\u65e5\u8a8c\u6a94\uff0c\u8acb\u9078\u53d6" + newline +
      "       \u300c\u7522\u751f\u65e5\u8a8c\u6a94\u300d\u6838\u53d6\u65b9\u584a\u3002\u60a8\u53ef\u4ee5\u8f38\u5165\u8def\u5f91\u548c\u6a94\u6848\u540d\u7a31\uff0c\u6216\u900f\u904e\u700f\u89bd" + newline +
      "       \u4f86\u9078\u64c7\u8cc7\u6599\u593e\uff0c\u7136\u5f8c\u9375\u5165\u6a94\u6848\u540d\u7a31\u4e26\u9078\u53d6\u300c\u958b\u555f\u300d\u3002" + newline +
      "       \u65e5\u8a8c\u6a94\u5305\u542b\u8207\u8f49\u63db\u7a0b\u5e8f\u6709\u95dc\u7684\u57fa\u672c" + newline +
      "       \u8cc7\u8a0a\u3002" + newline + newline +
      "8)  \u9078\u64c7\u8f49\u63db\u7bc4\u672c\uff1a" + newline + newline +
      "       \u5982\u679c\u672a\u9078\u64c7\u7bc4\u672c\uff0c\u5247\u5c07\u4f7f\u7528\u9810\u8a2d\u7bc4\u672c\u3002\u6b64\u7bc4\u672c" + newline +
      "       \u5c07\u751f\u6210\u53ef\u4ee5\u8207 IE \u548c Netscape \u914d\u5408\u4f7f\u7528\u7684\u5df2\u8f49\u63db html \u6a94\u6848\u3002" + newline  +
      "       \u5982\u679c\u60a8\u8981\u4f7f\u7528\u5176\u4ed6\u7bc4\u672c\uff0c\u53ef\u4ee5\u5f9e\u4e3b\u87a2\u5e55\u7684\u529f\u80fd\u8868" + newline +
      "       \u4e2d\u9078\u64c7\u3002\u5982\u679c\u60a8\u9078\u64c7\u300c\u5176\u4ed6\u300d\uff0c\u7cfb\u7d71\u5c07\u5141\u8a31\u60a8\u9078\u64c7\u4e00\u500b\u6a94\u6848\uff0c" + newline +
      "       \u4ee5\u4f5c\u70ba\u7bc4\u672c\u4f7f\u7528\u3002" + newline +
      "       \u5982\u679c\u60a8\u9078\u64c7\u6a94\u6848\uff0c\u8acb\u78ba\u4fdd\u8a72\u6a94\u6848\u70ba\u7bc4\u672c\u3002" + newline + newline +
      "9)  \u8f49\u63db\uff1a" + newline + newline +
      "       \u6309\u4e00\u4e0b\u300c\u8f49\u63db...\u300d\u6309\u9215\u4ee5\u958b\u59cb\u57f7\u884c\u8f49\u63db\u7a0b\u5e8f\u3002\u9032\u7a0b" + newline +
      "        \u5c0d\u8a71\u65b9\u584a\u5c07\u986f\u793a\u6b63\u5728\u8655\u7406\u7684\u6a94\u6848\u3001\u6a94\u6848\u8655\u7406\u7684\u6578\u76ee\u3001" + newline +
      "       \u627e\u5230\u7684 applet \u6578\u76ee\u4ee5\u53ca\u627e\u5230\u7684\u932f\u8aa4\u6578\u76ee\u3002" + newline + newline +
      "10) \u7e7c\u7e8c\u8f49\u63db\u6216\u9000\u51fa\uff1a" + newline + newline +
      "       \u8f49\u63db\u5b8c\u6210\u5f8c\uff0c\u9032\u7a0b\u5c0d\u8a71\u65b9\u584a\u4e2d\u7684\u6309\u9215\u5c07\u5f9e" + newline +
      "       \u300c\u53d6\u6d88\u300d\u8b8a\u66f4\u70ba\u300c\u5b8c\u6210\u300d\u3002\u60a8\u53ef\u4ee5\u9078\u64c7\u300c\u5b8c\u6210\u300d\u4ee5\u95dc\u9589\u5c0d\u8a71\u65b9\u584a\u3002" + newline  +
      "       \u6b64\u6642\uff0c\u9078\u64c7\u300c\u9000\u51fa\u300d\u4ee5\u95dc\u9589 Java(TM) Plug-in HTML Converter\uff0c" + newline +
      "       \u6216\u9078\u53d6\u53e6\u4e00\u7d44\u8981\u8f49\u63db\u7684\u6a94\u6848\uff0c\u4e26\u518d\u6b21\u9078\u64c7\u300c\u8f49\u63db...\u300d\u3002" + newline + newline +
      "11)  \u95dc\u65bc\u7bc4\u672c\u7684\u8a73\u7d30\u8cc7\u8a0a\uff1a" + newline + newline +
      "       \u7bc4\u672c\u6a94\u662f\u8f49\u63db applet \u7684\u57fa\u790e\u3002\u5b83\u50c5\u70ba\u6587\u5b57\u6a94\u6848\uff0c" + newline +
      "       \u5305\u542b\u8868\u793a\u90e8\u5206\u539f\u59cb applet \u7684\u6a19\u7c64\u3002" + newline +
      "       \u900f\u904e\u5728\u7bc4\u672c\u6a94\u4e2d\u65b0\u589e/\u79fb\u9664/\u79fb\u52d5\u6a19\u7c64\uff0c\u60a8\u53ef\u4ee5\u6539\u8b8a\u8f49\u63db\u6a94\u6848\u7684" + newline +
      "       \u8f38\u51fa\u3002" + newline + newline +
      "       \u652f\u63f4\u7684\u6a19\u7c64\uff1a" + newline + newline +
      "        $OriginalApplet$    \u6b64\u6a19\u7c64\u5c07\u88ab\u539f\u59cb applet \u7684" + newline +
      "        \u5168\u90e8\u6587\u5b57\u66ff\u4ee3\u3002" + newline + newline +
      "        $AppletAttributes$   \u6b64\u6a19\u7c64\u5c07\u88ab\u6240\u6709\u7684 applet" + newline +
      "         \u5c6c\u6027\u66ff\u4ee3\u3002(\u4ee3\u78bc\u3001\u4ee3\u78bc\u5eab\u3001\u5bec\u5ea6\u548c\u9ad8\u5ea6\u7b49\u3002)" + newline + newline +
      "        $ObjectAttributes$   \u6b64\u6a19\u7c64\u5c07\u88ab\u7269\u4ef6\u6a19\u7c64\u6240\u9700\u7684\u6240\u6709" + newline +
      "        \u5c6c\u6027\u66ff\u4ee3\u3002" + newline + newline +
      "        $EmbedAttributes$   \u6b64\u6a19\u7c64\u5c07\u88ab\u5d4c\u5165\u6a19\u7c64\u6240\u9700\u7684\u6240\u6709" + newline +
      "        \u5c6c\u6027\u66ff\u4ee3\u3002" + newline + newline +
      "        $AppletParams$    \u6b64\u6a19\u7c64\u5c07\u88ab applet \u7684\u6240\u6709" + newline +
      "         <param ...> \u6a19\u7c64\u66ff\u4ee3\u3002" + newline + newline +
      "        $ObjectParams$    \u6b64\u6a19\u7c64\u5c07\u88ab\u7269\u4ef6\u6a19\u7c64\u6240\u9700\u7684\u6240\u6709" + newline +
      "         <param...> \u6a19\u7c64\u66ff\u4ee3\u3002" + newline + newline +
      "        $EmbedParams$     \u6b64\u6a19\u7c64\u5c07\u88ab\u5d4c\u5165\u6a19\u7c64\u6240\u9700\u7684\u6240\u6709" + newline +
      "         <param...> \u6a19\u7c64\u4ee5 NAME=VALUE \u683c\u5f0f\u66ff\u4ee3\u3002" + newline + newline +
      "        $AlternateHTML$  \u6b64\u6a19\u7c64\u5c07\u88ab\u539f\u59cb applet \u7684\u4e0d\u652f\u63f4 applet\u5340\u57df" + newline +
      "        \u4e2d\u7684\u6587\u5b57\u66ff\u4ee3\u3002" + newline + newline +
      "        $CabFileLocation$   \u9019\u662f\u61c9\u5728\u76ee\u6a19\u70ba IE \u7684\u5404\u500b\u7bc4\u672c\u4e2d\u4f7f\u7528\u7684 cab \u6a94\u6848" + newline +
      "        \u7684 URL\u3002" + newline + newline +
      "        $NSFileLocation$    \u9019\u662f\u5728\u76ee\u6a19\u70ba Netscape \u7684\u5404\u500b\u7bc4\u672c\u4e2d\u4f7f\u7528\u7684 Netscape" + newline +
      "         \u63d2\u4ef6\u7684 URL" + newline + newline +
      "        $SmartUpdate$   \u9019\u662f\u5728\u76ee\u6a19\u70ba Netscape Navigator 4.0 \u6216\u66f4\u9ad8\u7248\u672c\u7684\u5404\u500b\u7bc4\u672c\u4e2d\u4f7f\u7528\u7684" + newline +
      "         Netscape SmartUpdate \u7684 URL\u3002" + newline + newline +
      "        $MimeType$    \u9019\u662f Java \u7269\u4ef6\u7684 MIME \u985e\u578b" + newline + newline +
      "      default.tpl (\u8f49\u63db\u7a0b\u5f0f\u7684\u9810\u8a2d\u7bc4\u672c) - \u8f49\u63db\u7684\u9801\u9762\u53ef" + newline +
      "      \u7528\u65bc Windows \u4e0a\u7684 IE \u548c Navigator\uff0c\u4ee5\u555f\u52d5 Java(TM) Plug-in\u3002" + newline +
      "      \u6b64\u7bc4\u672c\u4ea6\u53ef\u8207 Unix(Solaris) \u4e0a\u7684 Netscape \u914d\u5408\u4f7f\u7528" + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- \u8f49\u63db\u7684\u9801\u9762\u50c5\u53ef\u7528\u65bc\u555f\u52d5 Windows \u4e0a IE \u4e2d\u7684" + newline +
      "       Java(TM) Plug-in\u3002" + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- \u8f49\u63db\u7684\u9801\u9762\u53ef\u7528\u65bc\u555f\u52d5 Windows \u548c Solaris \u4e0a Navigator \u4e2d\u7684" + newline +
      "       Java(TM) Plug-in\u3002" + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- \u8f49\u63db\u7684\u9801\u9762\u53ef\u7528\u65bc\u4efb\u4f55\u700f\u89bd\u5668\u548c\u4efb\u4f55\u5e73\u53f0\u3002" + newline +
      "      \u5982\u679c\u700f\u89bd\u5668\u70ba Windows \u4e0a\u7684 IE \u6216 Navigator (Solaris \u4e0a\u7684 Navigator)\uff0c" + newline +
      "      \u5247\u6703\u555f\u52d5 Java(TM) Plug-in\u3002\u5426\u5247\uff0c\u5c07\u4f7f\u7528\u700f\u89bd\u5668\u7684\u9810\u8a2d JVM\u3002" + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  \u57f7\u884c HTML Converter\uff1a" + newline + newline +
      "      \u57f7\u884c HTML Converter \u7684 GUI \u7248\u672c" + newline + newline +
      "      HTML Converter \u5305\u542b\u5728 JDK \u4e2d\uff0c\u800c\u975e JRE \u4e2d\u3002\u82e5\u8981\u57f7\u884c\u8f49\u63db\u7a0b\u5f0f\uff0c\u8acb\u79fb\u81f3" + newline +
      "      \u60a8\u7684 JDK \u5b89\u88dd\u76ee\u9304\u7684 lib \u5b50\u76ee\u9304\u3002\u4f8b\u5982\uff0c" + newline +
      "      \u5982\u679c\u60a8\u5c07 JDK \u5b89\u88dd\u5728 Windows \u4e0a\u7684 C:\\jdk" + j2seVersion + "\uff0c\u8acb\u4f7f\u7528 cd \u6307\u4ee4\u8f49\u5230" + newline + newline +
      "             C:\\jdk" + j2seVersion + "\\lib\\" + newline + newline +
      "      \u8f49\u63db\u7a0b\u5f0f (htmlconverter.jar) \u5305\u542b\u5728\u8a72\u76ee\u9304\u4e2d\u3002" + newline + newline +
      "      \u82e5\u8981\u555f\u52d5\u8f49\u63db\u7a0b\u5f0f\uff0c\u8acb\u9375\u5165\uff1a" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      \u5728 UNIX/Linux \u4e0a\u555f\u52d5\u8f49\u63db\u7a0b\u5f0f\u540c\u6a23\u4f7f\u7528\u4e0a\u8ff0\u6307\u4ee4\u3002" + newline +
      "      \u4e0b\u9762\u662f\u555f\u52d5\u8f49\u63db\u7a0b\u5f0f\u7684\u4e00\u4e9b\u5176\u4ed6\u65b9\u6cd5" + newline + newline +
      "      \u5728 Windows \u4e0a" + newline +
      "      \u4f7f\u7528\u6a94\u6848\u7e3d\u7ba1\u555f\u52d5\u8f49\u63db\u7a0b\u5f0f\u3002" + newline +
      "      \u4f7f\u7528\u6a94\u6848\u7e3d\u7ba1\u5c0e\u822a\u81f3\u4ee5\u4e0b\u76ee\u9304\u3002" + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      \u9023\u6309\u5169\u4e0b HtmlConverter \u61c9\u7528\u7a0b\u5f0f\u3002" + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      \u57f7\u884c\u4ee5\u4e0b\u6307\u4ee4" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      \u5f9e\u4ee5\u4e0b\u6307\u4ee4\u884c\u57f7\u884c\u8f49\u63db\u7a0b\u5f0f\uff1a" + newline + newline +
      "      \u683c\u5f0f\uff1a" + newline + newline +
      "      java -jar htmlconverter.jar [-options1 value1 [-option2 value2" + newline +
      "      [...]]] [-simulate] [filespecs]" + newline + newline +
      "      filespecs\uff1a  \u4ee5\u7a7a\u683c\u5206\u9694\u7684\u6a94\u6848\u8aaa\u660e\u6e05\u55ae\uff0c* \u70ba\u842c\u7528\u5b57\u5143\u3002" + newline +
      "      (*.html *.htm)" + newline + newline +
      "      \u9078\u9805\uff1a" + newline + newline +
      "       source\uff1a    \u6a94\u6848\u8def\u5f91\u3002(\u5728 Windows \u4e2d\u70ba c:\\htmldocs\uff0c" + newline +
      "                  \u5728 Unix \u4e2d\u70ba /home/user1/htmldocs) \u9810\u8a2d\u503c\uff1a <userdir>" + newline +
      "                  \u5982\u679c\u662f\u76f8\u5c0d\u8def\u5f91\uff0c\u5247\u5047\u5b9a\u5176\u76f8\u5c0d\u65bc\u5df2\u555f\u52d5 HTMLConverter" + newline +
      "                   \u7684\u76ee\u9304\u3002" + newline + newline +
      "       backup\uff1a    \u5beb\u5165\u5099\u4efd\u6a94\u6848\u7684\u8def\u5f91\u3002\u9810\u8a2d\u503c\uff1a" + newline +
      "                  <userdir>/<source>_bak" + newline +
      "                  \u5982\u679c\u662f\u76f8\u5c0d\u8def\u5f91\uff0c\u5247\u5047\u5b9a\u5176\u76f8\u5c0d\u65bc\u5df2\u555f\u52d5 HTMLConverter" + newline +
      "                   \u7684\u76ee\u9304\u3002" + newline + newline +
      "       subdirs\uff1a   \u5b50\u76ee\u9304\u4e2d\u7684\u6a94\u6848\u61c9\u88ab\u8655\u7406\u3002" + newline +
      "                  \u9810\u8a2d\u503c\uff1a  FALSE" + newline + newline +
      "       template\uff1a  \u7bc4\u672c\u6a94\u540d\u7a31\u3002\u9810\u8a2d\u503c\uff1a  default.tpl-Standard " + newline +
      "                  (IE \u8207 Navigator) \u50c5\u7528\u65bc Windows \u8207 Solaris\u3002\u5982\u679c\u7121\u6cd5\u78ba\u5b9a\uff0c\u8acb\u4f7f\u7528\u9810\u8a2d\u503c\u3002" + newline + newline +
      "       log\uff1a       \u5beb\u5165\u8a18\u9304\u7684\u8def\u5f91\u548c\u6a94\u6848\u540d\u7a31\u3002(\u9810\u8a2d\u503c <userdir>/convert.log)" + newline + newline +
      "       progress\uff1a  \u986f\u793a\u8f49\u63db\u6642\u6a19\u6e96\u8f38\u51fa\u9032\u7a0b\u3002" + newline +
      "                  \u9810\u8a2d\u503c\uff1a false" + newline + newline +
      "       simulate\uff1a  \u986f\u793a\u5c0d\u8f49\u63db\u7684\u8aaa\u660e\uff0c\u800c\u4e0d\u9032\u884c\u8f49\u63db\u3002" + newline +
      "                  \u5982\u679c\u672a\u78ba\u5b9a\u8f49\u63db\uff0c\u8acb\u4f7f\u7528\u6b64\u9078\u9805\u3002" + newline +
      "                  \u7cfb\u7d71\u5c07\u70ba\u60a8\u63d0\u4f9b\u95dc\u65bc\u8f49\u63db\u7684" + newline +
      "                  \u8a73\u7d30\u8aaa\u660e\u6e05\u55ae\u3002" + newline + newline +
      "      \u5982\u679c\u50c5\u6307\u5b9a\u300cjava -jar htmlconverter.jar -gui\u300d(\u50c5\u5305\u542b -gui" + newline +
      "       \u9078\u9805\uff0c\u800c\u7121\u6a94\u6848\u8aaa\u660e)\uff0c\u5247\u5c07\u555f\u52d5\u8f49\u63db\u7a0b\u5f0f\u7684 GUI \u7248\u672c\u3002" + newline  +
      "      \u5426\u5247\uff0c\u5c07\u4e0d\u6703\u555f\u52d5 GUI\u3002" + newline + newline +
      "      \u5982\u9700\u66f4\u591a\u8cc7\u8a0a\uff0c\u8acb\u53c3\u95b1\u4ee5\u4e0b URL\uff1a" + newline + newline +
      "      http://java.sun.com/j2se/" + 
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html\u3002"}
};
}



