#
# This file is part of the Samsung Uproar Linux Utility (Sulu).
#
#   Sulu is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   Sulu is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Sulu.  See the file COPYING.  If you haven't received
#   a copy of the GNU General Public License, write to:
#
#       Free Software Foundation, Inc., 
#       59 Temple Place, Suite 330, Boston, MA  
#       02111-1307  USA
#
# kal@users.sourceforge.net
#
# CVS Version Information
# $Header$
#


#
# commands
#
CC          = /usr/bin/gcc
RM          = /bin/rm -f
CP          = /bin/cp -f
IN          = /usr/bin/indent -kr -i4


#
# flags
#
#CPPFLAGS    = -D_DEBUG 
CPPFLAGS    = 
CFLAGS      = -Wall 
INCLUDES    = `gtk-config --cflags`
LDFLAGS     =  -lusb
LIBRARIES   = `gtk-config --libs` 


#
# file defs
#
TARGET_sulu  = sulu
SOURCE_sulu  = \
	main.c \
	lib_uproar.c \
	system_pane.c \
	device_pane.c 
OBJECT_sulu  = $(SOURCE_sulu:.c=.o)
SOURCES     = $(SOURCE_sulu)


#
# targets
#
default: all

all: $(TARGET_sulu) 

clean:
	$(RM) $(TARGET_sulu)
	$(RM) *.o

indent:
	$(IN) *.h *.c
	$(RM) *.h~ *.c~

depend: $(SOURCES)
	$(CC) -MM $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $(SOURCES) > makedepend
	
$(TARGET_sulu): $(OBJECT_sulu)
	$(CC) -o $(TARGET_sulu) $(OBJECT_sulu) $(LDFLAGS) $(LIBRARIES)


#
# rules
#
%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $<


#
# makedepend 
#
# DO NOT DELETE THIS LINE -- make depend depends on it.

include makedepend

