#!/bin/bash
set -e

script_dir=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)

if test "${UAVIA_EMBEDDED_SOFTWARE_DIR}" = ""; then
  echo "You must define 'UAVIA_EMBEDDED_SOFTWARE_DIR' environnement variable. Are you a true UAVIA developper ?!" ; exit 1;
fi

BASE_DIR=${UAVIA_EMBEDDED_SOFTWARE_DIR}/build_$(uname -m)-$(uname -s)$(uname -r)
BUILD_DIR=$BASE_DIR/build_coreKit

export BASE_DIR
export BUILD_DIR

$script_dir/build.sh $*
