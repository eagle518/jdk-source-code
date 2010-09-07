/*
 * @(#)RemoveCommentReader.java	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.net.proxy;

import java.io.FilterReader;
import java.io.Reader;
import java.io.IOException;

/**
 * RemoveCommentReader is a simple FilterReader that strips comment out of a
 * stream of characters. It will strip the '//' comment and the '/*' comment.
 */
public final class RemoveCommentReader extends FilterReader  {

      /** A trivial constructor. Just initialize our superclass */
      public RemoveCommentReader(Reader in)   {
          super(in);
      }

      // Use to remember whether we are "inside" the '//'
      boolean inComment1 = false;

      // Use to remember whether we are "inside" the '/*'
      boolean inComment2 = false;

      // Use to remember whether we are "inside" the quote (", ')
      boolean inQuote = false;


      /**
       * This is the implementation of the no-op read() method of FilterReader.
       * It calls in.read() to get a buffer full of characters, then strips
       * out the comments.
       **/
      public int read(char[] buf, int from, int len) throws IOException   {
          int numchars = 0;  // how many characters have been read

          // Loop, because we might read a bunch of characters, then strip them
          // all out, leaving us with zero characters to return.
          while (numchars == 0)  {
              numchars = in.read(buf, from, len);     // Read characters
              if (numchars == -1)                     // Check if EOF
                 return -1;

              // Loop through the characters we read, stripping out comments.
              // Characters not in tags are copied over any previous comment in
              // the buffer.
              int last = from;           // Index of last non-comment character
              for (int i=from; i < from + numchars; i++)  {
                  if (!inComment1 && !inComment2)  {

		      // Not in comment
		      if (buf[i] == '"' || buf[i] == '\'')    {
			   // found a quote
			   inQuote = !inQuote;
		      }
		      else if (!inQuote && buf[i] == '/')    {
                           if (buf[i+1] == '/')    {      // Comment '//'
                               inComment1 = true;
                               i++;
                               continue;
                           }
                           else if (buf[i+1] == '*')    { // Comment '/*'
                               inComment2 = true;
                               i++;
                               continue;
                           }
                      }

                      if (Character.isWhitespace(buf[i]))   // Skip whitespaces
                          buf[last++] = ' ';
                      else
                          buf[last++] = buf[i];
                  }
                  else
                  {
                      // inComment1 || inComment2
                      if (inComment1)    {       // End Comment '//'
                         if (buf[i] == '\n')
                            inComment1 = false;
                      }
                      else if (inComment2)
                      {
                         if (buf[i] == '*' && buf[i+1] == '/') // End Comment '/*'
                         {
                            inComment2 = false;
                            i++;
                         }
                      }
                  }
              }
              numchars = last - from;  // Figure out how many characters remain
                                       // And if it is more than zero characters
                                       // Then return that number
          }
          return numchars;
      }

      /**
       * This is another no-op read() method we have to implement. We implement
       * it in terms of the method above. Our superclass implements the
       * remaining read() methods in terms of these two.
       **/
      public int read() throws IOException  {
          char[] buf = new char[1];
          int result = read(buf, 0, 1);
          if (result == -1)
              return -1;
          else
              return (int)buf[0];
      }
}



