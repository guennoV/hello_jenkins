noinst_LTLIBRARIES  = libutils.la
libutils_la_SOURCES = \
  Colors.cpp \
  Colors.hpp \
  Context.cpp \
  Context.hpp \
  Crc.cpp \
  Crc.hpp \
  Functional.hpp \
  Singleton.hpp \
  Singleton.ipp \
  StackTrace.cpp \
  StackTrace.hpp \
  iziDeclarations.hpp \
  iziDeclarations.cpp

src_utils_includedir      = $(includedir)/coreKit/Utils
src_utils_include_HEADERS = \
  Colors.hpp \
  Context.hpp \
  Crc.hpp \
  Functional.hpp \
  Singleton.hpp \
  Singleton.ipp \
  StackTrace.hpp \
  iziDeclarations.hpp

libutils_la_CXXFLAGS  =
libutils_la_LDFLAGS   =

if HAS_STACKTRACE_SUPPORT

  libutils_la_CXXFLAGS += \
    ${BACKTRACE_CPPFLAGS}

  libutils_la_LDFLAGS += \
    ${BACKTRACE_LDFLAGS} \
    ${BACKTRACE_LIB}

endif
