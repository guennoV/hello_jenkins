AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = coreKit

lib_LTLIBRARIES = libcoreKit.la

libcoreKit_la_SOURCES = \
  @builddir@/coreKit_Version.cpp

libcoreKit_la_LDFLAGS = \
  -no-undefined

libcoreKit_la_LDFLAGS += \
  ${NETWORKURI_LIBS} \
  ${BOOST_ASIO_LIB} \
  ${BOOST_PROGRAM_OPTIONS_LIB} \
  ${BOOST_THREAD_LIB} \
  ${BOOST_FILESYSTEM_LIB} \
  ${YAML_LIBS}

libcoreKit_la_LIBADD  = \
  coreKit/App/libapp.la \
  coreKit/Config/libconfig.la \
  coreKit/Log/liblog.la \
  coreKit/Network/libnetwork.la \
  coreKit/Service/libservice.la \
  coreKit/Stream/libstream.la \
  coreKit/Utils/libutils.la \
  coreKit/Version/libversion.la
