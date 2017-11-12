#!/bin/bash
set -e

script_dir=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
previous_dir=$PWD

F="build"

if test "$*" = "--help" -o "$*" = "-h"; then
  echo "$F [OPTIONS]"
  echo ""
  echo "where :"
  echo ""
  echo "  OPTIONS    		Other options that will be passed directly to"
  echo "             		the configure script."
  echo ""
  echo " Environment variables :"
  echo ""
  echo "  ENABLE_PARALLEL_BUILD    "
  echo "  MAKE_CHECKS              "
  echo "  MAKE_INSTALL             "
  echo "  BASE_DIR                 "
  echo "  RELEASE_DIR              "
  echo "  BUILD_DIR                "
  echo ""

  exit 0
fi

if test "${ENABLE_PARALLEL_BUILD}" = ""; then
  # Enable parallel build
  ENABLE_PARALLEL_BUILD=1
fi

if test "${MAKE_CHECKS}" = ""; then
  # Run tests
  MAKE_CHECKS=1
fi

if test "${MAKE_INSTALL}" = ""; then
  # Install coreKit.
  MAKE_INSTALL=1
fi

if test "${BASE_DIR}" = ""; then
  # Use standard dirctory.
  BASE_DIR=$script_dir/build_$(uname -m)-$(uname -s)$(uname -r)
fi

if test "${RELEASE_DIR}" = ""; then
  # Use standard BASE_DIR.
  RELEASE_DIR=$BASE_DIR/release
fi

if test "${BUILD_DIR}" = ""; then
  # Use standard dirctory.
  BUILD_DIR=$BASE_DIR/build
fi

if [ -d $BUILD_DIR ]; then
  echo "Build folder already exists."
else
  echo "Build folder does not exist yet. Creating build folder ..."
  mkdir -p $BUILD_DIR
fi

# Configure PKG_CONFIG_PATH variable.
PKG_CONFIG_PATH="${RELEASE_DIR}/lib/pkgconfig:${PKG_CONFIG_PATH}"
export PKG_CONFIG_PATH

echo "Moving to build folder ..."
cd $BUILD_DIR

echo ""
echo "Running configure script ..."
echo ""
$script_dir/coreKit/configure --prefix ${RELEASE_DIR} --srcdir $script_dir/coreKit $*

echo ""
echo "Compiling coreKit ..."
echo ""
if test "${ENABLE_PARALLEL_BUILD}" == "1"; then
  make -j$(getconf _NPROCESSORS_ONLN)
else
  make
fi

if test "${MAKE_CHECKS}" == "1"; then
  echo ""
  echo "Running checks ..."
  echo ""
  if make check; then true;
  else
    echo ""
    echo "Checks failed : "
    cat tests/test-suite.log ; exit 1;
  fi
fi

if test "${MAKE_INSTALL}" == "1"; then
  echo ""
  echo "Installing ..."
  echo ""
  make install
fi

echo ""
echo "Moving to previous folder ..."
cd $previous_dir
