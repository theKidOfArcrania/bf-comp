# 
# High-level compiler to brainf**k
# Copyright (C) 2019 theKidOfArcrania
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

noinst_LIBRARIES = libcompile.a

BUILT_SOURCES = bf.h bf_lex.h
AM_YFLAGS = -d
AM_LFLAGS = --header-file=bf_lex.h
libcompile_a_CPPFLAGS = -I.. $(WARN_CPPFLAGS)
libcompile_a_SOURCES = bf.y bf_lex.l ast.c main.c il_parser.c
MOSTLYCLEANFILES = bf.h bf_lex.h

# Recover from removal of header.
bf_lex.h: bf_lex.c
	test -f $@ || rm -f $<
	test -f $@ || $(MAKE) $(AM_MAKEFLAGS) $<


