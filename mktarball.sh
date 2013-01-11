#!/bin/bash

[ -f `basename "$0"` ] || {
    echo "ERROR: This script should be run from the source directory"
    exit 1
}

# Get project name from PWD
PROJ_NAME=`basename "$PWD" | awk -F'/' '{print $NF}'`

[ -f deliverables ] || {
    echo "ERROR: deliverables file not present"
    exit 1
}
PROJ_FILES=`cat deliverables`

[ -f version ] || {
    echo "ERROR: version file not present"
    exit 1
}
PROJ_VERSION=`cat version`

# Prepend a project-version/ to the sources in the tarball
eval tar --transform "s,^,${PROJ_NAME}-${PROJ_VERSION}/,S" -cvzf ${PROJ_NAME}-${PROJ_VERSION}.tar.gz ${PROJ_FILES}
[ $? -eq 0 ] || {
    echo "ERROR: tar command failed"
    exit 1
}

