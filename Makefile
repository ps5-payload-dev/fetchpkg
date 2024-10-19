#   Copyright (C) 2024 John Törnblom
#
# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not see
# <http://www.gnu.org/licenses/>.

CFLAGS := --O2 -Wall -Werror -static
LDADD  := -lcurl

ELF := fetchpkg
all: $(ELF)

$(ELF): main.c dl.c parson.c sha256.c sha1.c
	$(CC) -o $@ $^ $(LDADD)

clean:
	rm -f $(ELF)
