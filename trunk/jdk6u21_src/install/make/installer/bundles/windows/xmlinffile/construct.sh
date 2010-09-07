#
# Construct the .xml and .inf file for the Java Plug-in installation
# bootstrap. 
# format is construct VERSION_VERSION HTTP_SERVER SIGNING_REQUIRED
#
if test $# != 3
then
	print usage: $0 plugin_version_number http_server sign_install_flag
	exit 2
fi

version=$1
server=$2


filever=`print $version| sed 's/\./_/g' | sed 's/\(\_0\)\+$//g'`
fn=jre$filever-win.exe
webver=`print $version| sed 's/\(\.0\)\+$//g'`

print Constructin Plug-in Installation bundles for version $version
print File version is $filever, Web pages version is $webver

if test -f ../../$fn 
then 
	print -n using file : 
	ls -lg ../../$fn
else
	print WARNING : JRE Bundles not found : $fn
fi

#construct the .inf and .xml file

print s@#PLUGIN_VERSION#@$version@g>tmp.tmp
print s@#HTTP_SERVER#@$server@g>>tmp.tmp
print s@#PLUGIN_VERSION_FOR_FILE#@$filever@g>>tmp.tmp
sed -f tmp.tmp plugin_jinstall_original.inf > ./jinstall_$filever.inf
sed -f tmp.tmp plugin_jinstall_original.xml > ./jinstall_$filever.xml
