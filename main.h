
/*

    This file is part of the Samsung Uproar Linux Utility (sulu).
 
    Sulu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    Sulu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with sulu.  See the file COPYING.  If you haven't received
    a copy of the GNU General Public License, write to:
 
        Free Software Foundation, Inc., 
        59 Temple Place, Suite 330, Boston, MA  
        02111-1307  USA
 
  kal@users.sourceforge.net
 
*/
 

#ifndef main_h
#define main_h

#define MAX_SYSTEM_FILES 1024
#define MAX_FILENAME_LEN 1024
#define STARTUP_X_SIZE   520
#define STARTUP_Y_SIZE   360
#define DEVICE_PANE_X    (STARTUP_X_SIZE / 2)
#define DEVICE_PANE_Y    (STARTUP_Y_SIZE)
#define SYSTEM_PANE_X    (STARTUP_X_SIZE - DEVICE_PANE_X)
#define SYSTEM_PANE_Y    (STARTUP_Y_SIZE)
#define GUTTER_SIZE      20
#define MIN_PANE_WIDTH   200

gint Delete_event(GtkWidget * widget, GdkEvent * event, gpointer data);

GdkColor *FgDir;
GdkColor *FgFile;

#endif
