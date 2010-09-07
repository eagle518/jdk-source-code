/*
 * Copyright (c) 2001, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package sun.jvm.hotspot.debugger.win32.coff;

public class DumpExports {
  private static void usage() {
    System.err.println("usage: java DumpExports [.dll name]");
    System.exit(1);
  }

  public static void main(String[] args) {
    if (args.length != 1) {
      usage();
    }

    String filename = args[0];
    COFFFile file   = COFFFileParser.getParser().parse(filename);
    ExportDirectoryTable exports =
      file.getHeader().
        getOptionalHeader().
          getDataDirectories().
            getExportDirectoryTable();
    if (exports == null) {
      System.out.println("No exports found.");
    } else {
      System.out.println(file.getHeader().getNumberOfSections() + " sections in file");
      for (int i = 0; i < file.getHeader().getNumberOfSections(); i++) {
        System.out.println("  Section " + i + ": " + file.getHeader().getSectionHeader(1 + i).getName());
      }

      DataDirectory dir = file.getHeader().getOptionalHeader().getDataDirectories().getExportTable();
      System.out.println("Export table: RVA = 0x" + Integer.toHexString(dir.getRVA()) +
                         ", size = 0x" + Integer.toHexString(dir.getSize()));

      System.out.println("DLL name: " + exports.getDLLName());
      System.out.println("Time/date stamp 0x" + Integer.toHexString(exports.getTimeDateStamp()));
      System.out.println("Major version 0x" + Integer.toHexString(exports.getMajorVersion() & 0xFFFF));
      System.out.println("Minor version 0x" + Integer.toHexString(exports.getMinorVersion() & 0xFFFF));
      System.out.println(exports.getNumberOfNamePointers() + " functions found");
      for (int i = 0; i < exports.getNumberOfNamePointers(); i++) {
        System.out.println("  0x" +
                           Integer.toHexString(exports.getExportAddress(exports.getExportOrdinal(i))) +
                           "  " +
                           (exports.isExportAddressForwarder(exports.getExportOrdinal(i))  ?
                            ("Forwarded to " + exports.getExportAddressForwarder(exports.getExportOrdinal(i))) :
                            exports.getExportName(i)));
      }
    }
  }
}
