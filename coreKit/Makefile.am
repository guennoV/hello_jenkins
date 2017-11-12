SUBDIRS = src

if HAS_TESTS
  SUBDIRS += tests
endif

if HAS_SAMPLES
  SUBDIRS += samples
endif

pkgconfigdir = $(libdir)/pkgconfig
nodist_pkgconfig_DATA = coreKit.pc

coreKit_bannerdir   = $(sysconfdir)/@PACKAGE_NAME@
coreKit_banner_DATA = banner.txt

# Other dependencies.
EXTRA_DIST = banner.txt third-party
