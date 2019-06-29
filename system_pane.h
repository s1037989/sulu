
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
 

#ifndef system_pane_h
#define system_pane_h

#include <gtk/gtk.h>

GtkWidget *Create_system_pane();
GtkWidget *Clist_system;
GtkWidget *Status_system;
void Refresh_system_pane();
void Selectall_system_pane();
void Unselectall_system_pane();
void Invert_system_pane();
void Transfer_system_pane();
gint Is_regular_file(gint i);
void Select_row_callback(GtkWidget * widget, gint row, gint col,
			 GdkEventButton * ev, gpointer data);

gint Timeout_system_download(gpointer data);

#endif
