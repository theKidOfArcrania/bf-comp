AC_PREREQ([2.69])
AC_INIT([bf-comp], [0.9])

AC_CONFIG_FILES([Makefile compile/Makefile utils/Makefile])
AC_CONFIG_SRCDIR([compile/bf.y])
AC_CONFIG_AUX_DIR([build-aux])
#AC_CONFIG_HEADER([config.h])

# Init automake
AM_INIT_AUTOMAKE([-Wall])

# Check for programs
AC_LANG(C)
AC_PROG_CC
AC_PROG_RANLIB

AM_PROG_CC_C_O
AM_PROG_AR
AM_PROG_LEX
AC_PROG_YACC

AM_SILENT_RULES([yes])

# Configure warnings
m4_include(m4/warnings.m4)
gl_WARN_ADD([-Wall], [WARN_CPPFLAGS])
gl_WARN_ADD([-Wextra], [WARN_CPPFLAGS])
gl_WARN_ADD([-Werror], [WARN_CPPFLAGS])

# Check for typedefs/structs/etc...
AC_CHECK_HEADER_STDBOOL
AC_CHECK_HEADERS([stdlib.h string.h stdio.h errno.h stdarg.h stdint.h stddef.h], [], 
                 [AC_MSG_ERROR([Couldn't find header])])
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Check for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove memset strdup strerror strtol], [], 
               [AC_MSG_ERROR([Standard library function not found])])

AC_OUTPUT
