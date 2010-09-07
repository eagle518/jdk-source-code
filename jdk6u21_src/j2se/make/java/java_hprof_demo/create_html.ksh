#!/bin/ksh

#
# @(#)create_html.ksh	1.5 10/04/01
# 
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


# KSH script to create a browseable page for the sources supplied to it

html=$1
README=$2
includes="$3"
sources="$4"
extern_funcs=$5

rm -f -r ${html}

tmpdir=/tmp/create_html.ksh.$$
rm -f -r ${tmpdir}
mkdir -p ${tmpdir}

constants=${tmpdir}/constants
macros=${tmpdir}/macros
types=${tmpdir}/types

sedfile_file_names=${tmpdir}/sedfile_file_names

sedfile_func_defs=${tmpdir}/sedfile_func_defs
sedfile_func_uses=${tmpdir}/sedfile_func_uses
sedfile_constant_defs=${tmpdir}/sedfile_constant_defs
sedfile_constant_uses=${tmpdir}/sedfile_constant_uses
sedfile_macro_defs=${tmpdir}/sedfile_macro_defs
sedfile_macro_uses=${tmpdir}/sedfile_macro_uses
sedfile_type_defs=${tmpdir}/sedfile_type_defs
sedfile_type_uses=${tmpdir}/sedfile_type_uses

( cat ${includes} \
        | egrep '^[\ \	]*[A-Z][a-z_A-Z0-9]*[\ \	]*=' \
	| sed -e 's@^[\ \	]*\([A-Z][a-z_A-Z0-9]*\).*@\1@' ) \
    | sort \
    | uniq > ${constants}

( cat ${includes} \
        | egrep '^[\ \	]*#define[\ \	]*[a-zA-Z][a-z_A-Z0-9]*' \
	| sed -e 's@.*#define[\ \	]*\([a-zA-Z][a-z_A-Z0-9]*\).*@\1@' ) \
    | egrep -v '[A-Z][A-Z_a-z0-9]*_H' \
    | sort \
    | uniq > ${macros}

(  ( cat ${includes} \
	| egrep '^[\ \	]*typedef[\ \	][\ \	]*[a-zA-Z][a-zA-Z_0-9]*[\ \	(*]*[A-Z][a-z_A-Z0-9]*[);]' \
	| sed -e 's@.*typedef[\ \	][\ \	]*[a-zA-Z][a-zA-Z_0-9]*[\ \	(*]*\([A-Z][a-z_A-Z0-9]*\)[;)].*@\1@' ) ; \
  ( cat ${includes} \
          | egrep '[\ \	]struct[\ \	][\ \	]*[A-Z][a-z_A-Z0-9]*' \
	  | sed -e 's@.*struct[\ \	][\ \	]*\([A-Z][a-z_A-Z0-9]*\).*@\1@' ) ) \
    | sort \
    | uniq > ${types}

echo "Extern function count: `cat ${extern_funcs} | wc -l`"
echo "Constant count: `cat ${constants} | wc -l`"
echo "Macro count: `cat ${macros} | wc -l`"
echo "Type count: `cat ${types} | wc -l`"

href()
{
    echo "s@\([^><#\"]\)\<${1}\>@\1<a HREF=\"#${1}\">${1}</a>@" >> ${2}
}

table()
{
    counter=0
    echo "<table>" >> ${html}
    echo "<tr>" >> ${html}
    for i in $* ; do
       echo "<td><a HREF=\"#${i}\"> ${i} </a></td>" >> ${html}
       counter="`expr ${counter} '+' 1`"
       if [ ${counter} -gt 4 ] ; then
	    echo "</tr>" >> ${html}
	    echo "<tr>" >> ${html}
	    counter=0
       fi
    done
    echo "</tr>" >> ${html}
    echo "</table>" >> ${html}
}

for name in `cat  ${extern_funcs}` ; do
    echo "s@^\<${name}\>@<a NAME=\"${name}\"><b>${name}</b></a>@" \
	>> ${sedfile_func_defs}
done
for name in `cat  ${constants}` ; do
    echo "s@\<${name}\>\([\ \	]*=\)@<a NAME=\"${name}\"><b>${name}</b></a>\1@" \
	>> ${sedfile_constant_defs}
done
for name in `cat  ${macros}` ; do
    echo "s@#define[\ \	][\ \	]*\<${name}\>@#define <a NAME=\"${name}\"><b>${name}</b></a>@" \
	>> ${sedfile_macro_defs}
done
for name in `cat  ${types}` ; do
    echo "s@struct \<${name}\>[\ \	]*\([{]\)@struct <a NAME=\"${name}\"><b>${name}</b></a> \1@" \
	>> ${sedfile_type_defs}
    echo "s@\(typedef[\ \	]*[a-zA-Z].*[\ \	(*]*\)\<${name}\>@\1<a NAME=\"${name}\"><b>${name}</b></a>@" \
	>> ${sedfile_type_defs}
done

for name in `cat  ${extern_funcs}` ; do
    href ${name} ${sedfile_func_uses}
done
for name in `cat  ${constants}` ; do
    href ${name} ${sedfile_constant_uses}
done
for name in `cat  ${macros}` ; do
    href ${name} ${sedfile_macro_uses}
done
for name in `cat  ${types}` ; do
    href ${name} ${sedfile_type_uses}
done

pwd="`pwd`"
basedir="`basename ${pwd}`"

echo "<html>" >> ${html}
echo "<head><title>Source Browsing Page for ${basedir}</title><head>" >> ${html}
echo "<body>" >> ${html}
echo "<h1><center>Source Browsing Page for ${basedir}</center><br></h1>" >> ${html}
echo "<h1><center>`date`</center></h1>" >> ${html}

for filename in ${includes} ; do
   base="`basename ${filename}`"
   link="`basename ${base} .h`_INCLUDE"
   echo "s@\<${base}\>@<a HREF=\"#${link}\">${base}</a>@" \
	>> ${sedfile_file_names}
done

for filename in ${sources} ; do
   base="`basename ${filename}`"
   link="`basename ${base} .c`_SOURCE"
   echo "s@\<${base}\>@<a HREF=\"#${link}\">${base}</a>@" \
	>> ${sedfile_file_names}
done

base="`basename ${README}`"
echo "<p><hr><p><h2>${base}</h2>" >> ${html}
echo "<p><pre>" >> ${html}
sed -e 's@<@\&lt;@g' -e 's@>@\&gt;@g' ${README} \
    | sed -f ${sedfile_func_uses} \
    | sed -f ${sedfile_constant_uses} \
    | sed -f ${sedfile_macro_uses} \
    | sed -f ${sedfile_type_uses} \
    | sed -f ${sedfile_file_names} >> ${html}
echo "</pre></p>" >> ${html}

for filename in ${includes} ; do
   base="`basename ${filename}`"
   link="`basename ${base} .h`_INCLUDE"
   echo "Processing ${base}:"
   echo "<p><hr><p><h2>Include File: <a NAME=\"${link}\">${base}</a></h2>" >> ${html}
   echo "<p><pre><tt>" >> ${html}
   sed -e 's@<@\&lt;@g' -e 's@>@\&gt;@g' ${filename} \
       | sed -f ${sedfile_func_uses} \
       | sed -f ${sedfile_constant_defs} \
       | sed -f ${sedfile_macro_defs} \
       | sed -f ${sedfile_type_defs} \
       | sed -f ${sedfile_constant_uses} \
       | sed -f ${sedfile_macro_uses} \
       | sed -f ${sedfile_type_uses} \
       | cat -n >> ${html}
   echo "</tt></pre></p>" >> ${html}
done

for filename in ${sources} ; do
   base="`basename ${filename}`"
   link="`basename ${base} .c`_SOURCE"
   echo "Processing ${base}:"
   echo "<p><hr><p><h2>Source File: <a NAME=\"${link}\">${base}</a></h2>" >> ${html}
   echo "<p><pre><tt>" >> ${html}
   sed -e 's@<@\&lt;@g' -e 's@>@\&gt;@g' ${filename} \
       | sed -f ${sedfile_func_defs} \
       | sed -f ${sedfile_constant_defs} \
       | sed -f ${sedfile_macro_defs} \
       | sed -f ${sedfile_type_defs} \
       | sed -f ${sedfile_func_uses} \
       | sed -f ${sedfile_constant_uses} \
       | sed -f ${sedfile_macro_uses} \
       | sed -f ${sedfile_type_uses} \
       | cat -n >> ${html}
   echo "</tt></pre></p>" >> ${html}
done

echo "Creating Index"

echo "<p><h2>Functions</h2>" >> ${html}
table `cat ${extern_funcs}`
echo "<p><h2>Constants</h2>" >> ${html}
table `cat ${constants}`
echo "<p><h2>Macros</h2>" >> ${html}
table `cat ${macros}`
echo "<p><h2>Types</h2>" >> ${html}
table `cat ${types}`


echo "</body>" >> ${html}
echo "</html>" >> ${html}

echo "Html file is ${html}"

rm -f -r ${tmpdir}

exit 0
