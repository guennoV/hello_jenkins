/* coreKit_config.h.  Generated from coreKit_config.h.in by configure.  */
/* coreKit_config.h.in.  Generated from configure.ac by autoheader.  */

/* Provide C++ backtraces with backward code. */
#define BACKTRACE_USE_BACKWARD 1

/* Provide C++ backtraces with booster code. */
/* #undef BACKTRACE_USE_BOOSTER */

/* Define if execinfo.h is usable. */
/* #undef BACKWARD_HAS_BACKTRACE */

/* Define if execinfo.h is usable. */
/* #undef BACKWARD_HAS_BACKTRACE_SYMBOL */

/* Define if libbfd is usable. */
/* #undef BACKWARD_HAS_BFD */

/* Define if libdw is usable. */
#define BACKWARD_HAS_DW 1

/* Define if libgcc has _Unwind_GetIP(). */
#define BACKWARD_HAS_UNWIND 1

/* Specify linux support. */
#define BACKWARD_SYSTEM_LINUX 1

/* Define if cxxabi.h is usable. */
/* #undef BOOSTER_HAVE_ABI_CXA_DEMANGLE */

/* Define if dladdr() is usable. */
/* #undef BOOSTER_HAVE_DLADDR */

/* Define if execinfo.h is usable. */
/* #undef BOOSTER_HAVE_EXECINFO */

/* Define if libgcc has _Unwind_GetIP(). */
/* #undef BOOSTER_HAVE_UNWIND_BACKTRACE */

/* coreKit banner file */
#define COREKIT_BANNERFILE "/usr/local/etc/coreKit/banner.txt"

/* define if the Boost library is available */
#define HAVE_BOOST /**/

/* define if the Boost::ASIO library is available */
#define HAVE_BOOST_ASIO /**/

/* define if the Boost::Filesystem library is available */
#define HAVE_BOOST_FILESYSTEM /**/

/* define if the Boost::PROGRAM_OPTIONS library is available */
#define HAVE_BOOST_PROGRAM_OPTIONS /**/

/* define if the Boost::Thread library is available */
#define HAVE_BOOST_THREAD /**/

/* define if the compiler supports basic C++11 syntax */
#define HAVE_CXX11 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define if debugging is disabled */
#define NDEBUG /**/

/* Name of package */
#define PACKAGE "corekit"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "pierre.pele@uavia.eu"

/* Define to the full name of this package. */
#define PACKAGE_NAME "coreKit"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "coreKit 0.0.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "corekit"

/* Define to the home page for this package. */
#define PACKAGE_URL "uavia.eu"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.0.1"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.0.1"
