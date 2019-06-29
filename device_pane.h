
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


#ifndef device_pane_h
#define device_pane_h

#include <gtk/gtk.h>

GtkWidget *Create_device_pane();
GtkWidget *Clist_device;
GtkWidget *Status_device;
void Refresh_device_pane();
void Selectall_device_pane();
void Unselectall_device_pane();
void Invert_device_pane();
void Delete_tracks();
void Reformat_memory();

gint Timeout_device_delete(gpointer data);
gint Timeout_device_reformat(gpointer data);

#endif
