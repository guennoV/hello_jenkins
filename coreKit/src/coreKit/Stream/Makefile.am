AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES  = libstream.la
libstream_la_SOURCES   =   \
  \
  Internal/ClientImpl.cpp  \
  Internal/ClientImpl.hpp  \
  Internal/ClientImpl.ipp  \
  Internal/ServerImpl.cpp  \
  Internal/ServerImpl.hpp  \
  Internal/ServerImpl.ipp  \
  Internal/SessionImpl.hpp \
  Internal/SessionImpl.ipp \
  \
  Client.cpp  \
  Client.hpp  \
  Server.cpp  \
  Server.hpp  \
  Session.cpp \
  Session.hpp

src_stream_includedir      = $(includedir)/coreKit/Stream
src_stream_include_HEADERS = \
  Client.hpp  \
  Server.hpp  \
  Session.hpp

libstream_la_CXXFLAGS  = ${NETWORKURI_CFLAGS}
