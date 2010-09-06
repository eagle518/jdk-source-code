/*
 * @(#)SearchPath.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.File;
import java.lang.Object;
import java.lang.String;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.StringTokenizer;


public class SearchPath extends Object {
    public SearchPath(String path) {
        _path = path;
    }


    public File find(String  filename) {
        String [] filenames = { filename };

        return (findOne(filenames));
    }

    public File findOne(String  [] filenames) {
        return (find(filenames, null));
    }

    public File [] findAll(String  [] filenames) {
        ArrayList list   = new ArrayList(filenames.length * 4);
        File      found  = null;
        boolean   first  = true;

        do {
            found = find(filenames, found);
            if (found != null) {
                list.add(found);
            }
            // no else required; search complete
        } while (found != null);

        return ((File []) list.toArray((Object []) new File[list.size()]));
    }

    private File find(String  [] filenames,
                      File       last) {
        StringTokenizer tokenizer = new StringTokenizer(_path, File.pathSeparator);
        List            names     = Arrays.asList(filenames);
        int             index     = filenames.length;
        File            dir       = null;
        File            result    = null;

        // find where to start
        if (last != null) {
            // get the index of the next filename to find is
            index = names.indexOf(last.getName()) + 1;

            // find the last directory searched
            while (tokenizer.hasMoreTokens()) {
                dir = new File(tokenizer.nextToken());
                if (dir.equals(last.getParentFile())) {
                    break;
                }
            }
        }

        do {
            // check if there are more files to search for in the current
            // directory
            if (index >= filenames.length) {
                index = 0;    // start with first name in list

                // get the next directory in the path to search
                if (tokenizer.hasMoreTokens()) {
                    dir = new File(tokenizer.nextToken());
                }
                else {
                    dir = null;
                }
            }
            // no else required; still have files to look for in current
            // directory

            // is there still a directory in the path to search
            if (dir != null) {
                File current = new File(dir, (String) names.get(index++));

                if (current.exists()) {
                    result = current;
                }
                // no else required; file not in dir, keep searching
            }
            // no else required; search complete, nothing found this pass
        } while ((result == null) && (dir != null));

        return (result);
    }

    private String _path;
}
