dnl AC_LIB_VERSION(PREFIX, MAJOR, MINOR, REVISION, RELEASE)

AC_DEFUN([AC_LIB_VERSION],
[
  version_major=[$2]
  version_minor=[$3]
  version_revision=[$4]
  version_release=[$5]

  AC_SUBST(version_major)
  AC_SUBST(version_minor)
  AC_SUBST(version_revision)
  AC_SUBST(version_release)

])
