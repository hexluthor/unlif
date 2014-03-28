# unlif: Extracts individual images from LIF files.
# Copyright (C) 2014  Ian Martin
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

ALL += unlif

CLEAN += $(ALL)

EDIT += Makefile
EDIT += *.c

CFLAGS += -std=c99

# Enable debugging:
CFLAGS += -g

# Enable optimizations:
CFLAGS += -O2

all: $(ALL)

clean:
	rm -f $(CLEAN)

edit:
	kate -n $(EDIT) 2>/dev/null & disown

LICENSE:
	wget -O $@ http://www.gnu.org/licenses/gpl-2.0.txt
