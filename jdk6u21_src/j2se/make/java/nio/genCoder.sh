#! /bin/sh
# @(#)genCoder.sh	1.7 01/09/17
# Generate charset coder and decoder classes

# Environment variables required from make: SED SPP

what=$1
SRC=$2
DST=$3

if [ x$what = xdecoder ]; then

  echo '  '$SRC '--('$what')-->' $DST
  $SPP <$SRC >$DST \
    -K$what \
    -DA='A' \
    -Da='a' \
    -DCode='Decode' \
    -Dcode='decode' \
    -DitypesPhrase='bytes in a specific charset' \
    -DotypesPhrase='sixteen-bit Unicode characters' \
    -Ditype='byte' \
    -Dotype='character' \
    -DItype='Byte' \
    -DOtype='Char' \
    -Dcoder='decoder' \
    -DCoder='Decoder' \
    -Dcoding='decoding' \
    -DOtherCoder='Encoder' \
    -DreplTypeName='string' \
    -DdefaultRepl='"\\\\uFFFD"' \
    -DdefaultReplName='<tt>"\\\&#92;uFFFD"<\/tt>' \
    -DreplType='String' \
    -DreplFQType='java.lang.String' \
    -DreplLength='length()' \
    -DItypesPerOtype='CharsPerByte' \
    -DnotLegal='not legal for this charset' \
    -Dotypes-per-itype='chars-per-byte' \
    -DoutSequence='Unicode character'

elif [ x$what = xencoder ]; then

  echo '  '$SRC '--('$what')-->' $DST
  $SPP <$SRC >$DST \
    -K$what \
    -DA='An' \
    -Da='an' \
    -DCode='Encode' \
    -Dcode='encode' \
    -DitypesPhrase='sixteen-bit Unicode characters' \
    -DotypesPhrase='bytes in a specific charset' \
    -Ditype='character' \
    -Dotype='byte' \
    -DItype='Char' \
    -DOtype='Byte' \
    -Dcoder='encoder' \
    -DCoder='Encoder' \
    -Dcoding='encoding' \
    -DOtherCoder='Decoder' \
    -DreplTypeName='byte array' \
    -DdefaultRepl='new byte[] { (byte)'"'"\\?"'"' }' \
    -DdefaultReplName='<tt>{<\/tt>\\\&nbsp;<tt>(byte)'"'"\\?"'"'<\/tt>\\\&nbsp;<tt>}<\/tt>' \
    -DreplType='byte[]' \
    -DreplFQType='byte[]' \
    -DreplLength='length' \
    -DItypesPerOtype='BytesPerChar' \
    -DnotLegal='not a legal sixteen-bit Unicode sequence' \
    -Dotypes-per-itype='bytes-per-char' \
    -DoutSequence='byte sequence in the given charset'

else
  echo Illegal coder type: $what
  exit 1
fi
