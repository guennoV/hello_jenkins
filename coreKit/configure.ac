AC_INIT([coreKit], 0.0.1, pierre.pele@uavia.eu,, uavia.eu)

# Get the Canonical System Type.
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AX_CHECK_ENABLE_DEBUG([info])

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])
LT_INIT

# Here are our macros.
AC_CONFIG_MACRO_DIR([m4])

################################################################################

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Enable C++11.
AX_CXX_COMPILE_STDCXX_11(, mandatory)

# C++ is our default language.
AC_LANG([C++])

################################################################################

# Revision control.
AC_LIB_VERSION(coreKit, 0, 0, 1, dev)

################################################################################

AC_LANG_PUSH([C])

# Check CC vendor / version.
AX_COMPILER_VENDOR
AX_COMPILER_VERSION

AC_SUBST(ax_cv_c_compiler_vendor)
AC_SUBST(ax_cv_c_compiler_version)

AC_LANG_POP([C])

# Check CXX vendor / version.
AX_COMPILER_VENDOR
AX_COMPILER_VERSION

AC_SUBST(ax_cv_cxx_compiler_vendor)
AC_SUBST(ax_cv_cxx_compiler_version)

################################################################################

# Configure compiler.
AX_CFLAGS_WARN_ALL
AX_CXXFLAGS_WARN_ALL

# Add -Wextra flag if available
CFLAGS_save="${CFLAGS}"
AC_CACHE_CHECK([if \$CC accepts -Wextra],
       [ac_cv_c_Wextra],
       [CFLAGS="-Wextra ${CFLAGS_save}"
       AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[ac_cv_c_Wextra=yes],[ac_cv_c_Wextra=no])])
CFLAGS="${CFLAGS_save}"
if test "${ac_cv_c_Wextra}" != "no"; then
    CFLAGS+=" -Wextra"
    CXXFLAGS+=" -Wextra"
fi

################################################################################

AC_CONFIG_HEADERS([coreKit_config.h])

################################################################################

# Checks for boost.
AX_BOOST_BASE(1.64)

# We use boost asio.
AX_BOOST_ASIO

# We use boost program options.
AX_BOOST_PROGRAM_OPTIONS

# We use boost thread.
AX_BOOST_THREAD

# We use boost filesystem.
AX_BOOST_FILESYSTEM

# Checks for PKG_CONFIG.
PKG_PROG_PKG_CONFIG

# Checks for Yaml-cpp.
PKG_CHECK_MODULES(YAML, yaml-cpp)

# Checks for Spdlog.
PKG_CHECK_MODULES(SPDLOG, spdlog)

# Checks for CPP-Netlib.
#PKG_CHECK_MODULES(NETWORKURI, networkuri)

################################################################################

# Should we compile sample apps ?
AC_ARG_ENABLE([samples],
              [AC_HELP_STRING([--disable-samples],
                              [Do not compile sample applications])],
              [],
              [enable_samples=yes])

AM_CONDITIONAL(HAS_SAMPLES, test "x$enable_samples" = "xyes")

################################################################################

# Should we run tests ?
AC_ARG_ENABLE([tests],
              [AC_HELP_STRING([--disable-tests],
                              [Do not run tests])],
              [],
              [enable_tests=yes])

AM_CONDITIONAL(HAS_TESTS, test "x$enable_tests" = "xyes")

################################################################################

# StackTrace support
AC_ARG_ENABLE([stacktrace],
              [AC_HELP_STRING([--disable-stacktrace],
                              [Do not enable StackTrace support])],
              [],
              [enable_stacktrace=yes])

AM_CONDITIONAL(HAS_STACKTRACE_SUPPORT, test "x$enable_stacktrace" = "xyes")

if test "x$enable_stacktrace" = "xyes"; then
		AX_BACKTRACE
fi

################################################################################

# Preprocessor flags.
CPPFLAGS+=" -I\${top_srcdir}/src"
CPPFLAGS+=" ${BOOST_CPPFLAGS}"
CPPFLAGS+=" ${SPDLOG_CFLAGS}"

# Compiler flags.
CXXFLAGS+=" ${YAML_CFLAGS}"

# Linker flags.
LDFLAGS+=" ${BOOST_LDFLAGS}"

################################################################################

AC_DEFINE_DIR(
  [COREKIT_BANNERFILE],
	[sysconfdir/$PACKAGE_NAME/banner.txt], [coreKit banner file])

################################################################################

# Source files.
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(src/Makefile)
AC_CONFIG_FILES(src/coreKit/Makefile)
AC_CONFIG_FILES(src/coreKit/App/Makefile)
AC_CONFIG_FILES(src/coreKit/Config/Makefile)
AC_CONFIG_FILES(src/coreKit/Log/Makefile)
AC_CONFIG_FILES(src/coreKit/Network/Makefile)
AC_CONFIG_FILES(src/coreKit/Service/Makefile)
AC_CONFIG_FILES(src/coreKit/Stream/Makefile)
AC_CONFIG_FILES(src/coreKit/Utils/Makefile)
AC_CONFIG_FILES(src/coreKit/Version/Makefile)

# Tests.
AC_CONFIG_FILES(tests/Makefile)

# Samples.
AC_CONFIG_FILES(samples/Makefile)
AC_CONFIG_FILES(samples/app/Makefile)
AC_CONFIG_FILES(samples/config/Makefile)
AC_CONFIG_FILES(samples/log/Makefile)
AC_CONFIG_FILES(samples/service/Makefile)
AC_CONFIG_FILES(samples/stacktrace/Makefile)
AC_CONFIG_FILES(samples/stream/Makefile)
AC_CONFIG_FILES(samples/yaml/Makefile)

# Others.
AC_CONFIG_FILES(src/coreKit_Version.cpp)
AC_CONFIG_FILES(coreKit.pc)

AC_OUTPUT

################################################################################

echo "
coreKit ${coreKit_version}
==============================================

version_major        : ${version_major}
version_minor        : ${version_minor}
version_revision     : ${version_revision}
version_release      : ${version_release}"

echo "
Build variables :
==============================================

Prefix   :  ${prefix}

CC       :  ${CC}
CXX      :  ${CXX}

CPPFLAGS :  ${CPPFLAGS}

CFLAGS   :  ${CFLAGS}
CXXFLAGS :  ${CXXFLAGS}

LIBS     :  ${LIBS}
LDFLAGS  :  ${LDFLAGS}

Dependencies :
==============================================

PTHREAD_LIBS              :  ${PTHREAD_LIBS}
PTHREAD_CFLAGS            :  ${PTHREAD_CFLAGS}
PTHREAD_CC                :  ${PTHREAD_CC}

BOOST_CPPFLAGS            :  ${BOOST_CPPFLAGS}
BOOST_LDFLAGS             :  ${BOOST_LDFLAGS}

BOOST_ASIO_LIB            :  ${BOOST_ASIO_LIB}
BOOST_PROGRAM_OPTIONS_LIB :  ${BOOST_PROGRAM_OPTIONS_LIB}
BOOST_THREAD_LIB          :  ${BOOST_THREAD_LIB}
BOOST_FILESYSTEM_LIB      :  ${BOOST_FILESYSTEM_LIB}

YAML_CFLAGS               :  ${YAML_CFLAGS}
YAML_LIBS                 :  ${YAML_LIBS}

SPDLOG_CFLAGS             :  ${SPDLOG_CFLAGS}

NETWORKURI_CFLAGS         :  ${NETWORKURI_CFLAGS}
NETWORKURI_LIBS           :  ${NETWORKURI_LIBS}"

if test "x$enable_stacktrace" = "xyes"; then
	echo "
StackTrace support :
==============================================

BACKTRACE_CPPFLAGS    :  ${BACKTRACE_CPPFLAGS}
BACKTRACE_LDFLAGS     :  ${BACKTRACE_LDFLAGS}
BACKTRACE_LIB         :  ${BACKTRACE_LIB}"

	echo ""
fi

echo ""
