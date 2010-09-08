# 
#  @(#)genhash.awk	1.5 10/03/23
# 
# Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
#  ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#  
#  With this script one can generate a new version XKeysym.java file out
#  of keysym2ucs.h prototype and UnicodeData.txt database.
#  Latter file should be fetched from a unicode.org site, most
#  probably http://www.unicode.org/Public/UNIDATA/UnicodeData.txt
#
BEGIN {   FS=";";
          while((getline < "UnicodeData.txt")){
              unic[$1]=$2;
          }
          FS=" ";
          print("// This is a generated file: do not edit! Edit keysym2ucs.h if necessary.\n");          
      }

/^0x/{
         if( $1 != "0x0000" ) {
             ndx =  toupper($1);
             sub(/0X/, "", ndx);
             printf("        keysym2UCSHash.put( (long)%s, (char)%s); // %s -->%s\n",
                        $4, $1, $3, (unic[ndx]=="" ? "" : " " unic[ndx]));
         }
     }
/tojava/ { sub(/tojava /, ""); sub(/tojava$/, ""); print}    
