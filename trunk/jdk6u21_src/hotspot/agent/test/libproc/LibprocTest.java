/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
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

/**
  This is test case run by debuggee for running LibprocClient.java.
*/

public class LibprocTest {
   public static void main(String[] args) throws Exception {
      String myStr = "";
      System.out.println("main start");
      synchronized(myStr) {
         try {
            myStr.wait();
         } catch (InterruptedException ee) {
         }
      }
      System.out.println("main end");
   }
}
