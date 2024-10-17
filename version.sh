#!/bin/sh

PRODUCT_NUMBER=T1196$2
PRODUCT_NAME=GMAKE_$1
COMMITTER_DATE=`date --date="@\`git show -s --format=%ct HEAD\`" +%d%b%g | tr '[:lower:]' '[:upper:]'`
VERSION_STRING=`git describe --tags --long | sed 's/-.*-/_/' | sed 's/\./_/g'`

echo ${PRODUCT_NUMBER}_${COMMITTER_DATE}_${PRODUCT_NAME}_${VERSION_STRING}
