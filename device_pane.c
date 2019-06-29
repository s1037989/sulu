
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
 

#include <stdio.h>
#include <dirent.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include <usb.h>

#include "main.h"
#include "device_pane.h"
#include "lib_uproar.h"

GtkWidget *Create_device_pane()
{
    GtkWidget *vertical_box;
    GtkWidget *scrolled_window;
    gchar *titles[2] = { "Filename", "MB" };
    int padding = 2;

    GtkWidget *menu_bar;

    GtkWidget *file_menu;
    GtkWidget *file_item;
    GtkWidget *selectall_item;
    GtkWidget *unselectall_item;
    GtkWidget *invert_item;

    GtkWidget *action_menu;
    GtkWidget *action_item;
    GtkWidget *refresh_item;
    GtkWidget *delete_item;
    GtkWidget *reformat_item;

    /* 
     * create a vertical box to hold everything in this pane 
     */
    vertical_box = gtk_vbox_new(FALSE, padding);
    gtk_widget_show(vertical_box);

    /* 
     * create the selection menu and link it with the top level item 
     */
    file_menu = gtk_menu_new();
    file_item = gtk_menu_item_new_with_label("File");
    gtk_widget_show(file_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

    /* selection->selectall */
    selectall_item = gtk_menu_item_new_with_label("Select all");
    gtk_menu_append(GTK_MENU(file_menu), selectall_item);
    gtk_signal_connect_object(GTK_OBJECT(selectall_item), "activate",
			      GTK_SIGNAL_FUNC(Selectall_device_pane),
			      (gpointer) "Selectall_device_pane");
    gtk_widget_show(selectall_item);

    /* selection->unselectall */
    unselectall_item = gtk_menu_item_new_with_label("Unselect all");
    gtk_menu_append(GTK_MENU(file_menu), unselectall_item);
    gtk_signal_connect_object(GTK_OBJECT(unselectall_item), "activate",
			      GTK_SIGNAL_FUNC(Unselectall_device_pane),
			      (gpointer) "Unselectall_device_pane");
    gtk_widget_show(unselectall_item);

    /* selection->invert */
    invert_item = gtk_menu_item_new_with_label("Invert Selection");
    gtk_menu_append(GTK_MENU(file_menu), invert_item);
    gtk_signal_connect_object(GTK_OBJECT(invert_item), "activate",
			      GTK_SIGNAL_FUNC(Invert_device_pane),
			      (gpointer) "Invert_device_pane");
    gtk_widget_show(invert_item);

    /* 
     * create the transfer menu and link it with the top level item 
     */
    action_menu = gtk_menu_new();
    action_item = gtk_menu_item_new_with_label("Action");
    gtk_widget_show(action_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(action_item), action_menu);

    /* action->refresh */
    refresh_item = gtk_menu_item_new_with_label("Refresh List");
    gtk_menu_append(GTK_MENU(action_menu), refresh_item);
    gtk_signal_connect_object(GTK_OBJECT(refresh_item), "activate",
			      GTK_SIGNAL_FUNC(Refresh_device_pane),
			      (gpointer) "Refresh_device_pane");
    gtk_widget_show(refresh_item);

    /* action->delete tracks */
    delete_item = gtk_menu_item_new_with_label("Delete tracks");
    gtk_menu_append(GTK_MENU(action_menu), delete_item);
    gtk_signal_connect_object(GTK_OBJECT(delete_item), "activate",
			      GTK_SIGNAL_FUNC(Delete_tracks),
			      (gpointer) "Delete_tracks");
    gtk_widget_show(delete_item);

    /* action->reformat memory */
    reformat_item = gtk_menu_item_new_with_label("Reformat memory");
    gtk_menu_append(GTK_MENU(action_menu), reformat_item);
    gtk_signal_connect_object(GTK_OBJECT(reformat_item), "activate",
			      GTK_SIGNAL_FUNC(Reformat_memory),
			      (gpointer) "Reformat_memory");
    gtk_widget_show(reformat_item);

    /* 
     * create the menu bar and add the top level items 
     */
    menu_bar = gtk_menu_bar_new();
    gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), file_item);
    gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), action_item);

    /* set the packing policy for resizes */
    gtk_container_add(GTK_CONTAINER(vertical_box), menu_bar);
    gtk_box_set_child_packing(GTK_BOX(vertical_box), menu_bar, FALSE,
			      FALSE, padding, GTK_PACK_START);
    gtk_widget_show(menu_bar);

    /*
     * create a status line
     */
    Status_device = gtk_label_new("");

    /* set the packing policy for resizes */
    gtk_container_add(GTK_CONTAINER(vertical_box), Status_device);
    gtk_box_set_child_packing(GTK_BOX(vertical_box), Status_device, FALSE,
			      FALSE, padding, GTK_PACK_START);
    gtk_widget_show(Status_device);

    /* 
     * create a new scrolled window, with scrollbars only if needed
     */
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_usize(GTK_WIDGET(scrolled_window), SYSTEM_PANE_X,
			 SYSTEM_PANE_Y);
    gtk_container_add(GTK_CONTAINER(vertical_box), scrolled_window);
    gtk_widget_show(scrolled_window);

    /* create a new clist and put it in the scrolled window */
    Clist_device = gtk_clist_new_with_titles(2, titles);
    gtk_clist_set_selection_mode(GTK_CLIST(Clist_device),
				 GTK_SELECTION_MULTIPLE);
    gtk_clist_set_column_width(GTK_CLIST(Clist_device), 0,
			       MIN_PANE_WIDTH + 10);
    gtk_clist_set_column_width(GTK_CLIST(Clist_device), 1, 10);
    gtk_clist_column_titles_passive(GTK_CLIST(Clist_device));

    /* add the file list to the window */
    Refresh_device_pane();

    gtk_container_add(GTK_CONTAINER(scrolled_window), Clist_device);
    gtk_widget_show(Clist_device);

    return vertical_box;
}

void Selectall_device_pane()
{

    gtk_clist_select_all(GTK_CLIST(Clist_device));
}

void Unselectall_device_pane()
{

    gtk_clist_unselect_all(GTK_CLIST(Clist_device));
}

void Invert_device_pane()
{
    int selected;
    gint i;
    gint row = -1;
    gint total_rows = GTK_CLIST(Clist_device)->rows;
    GList *glist;

    for (i = 0; i < total_rows; i++) {
	selected = 0;
	glist = GTK_CLIST(Clist_device)->selection;
	while (glist != NULL) {
	    row = (gint) glist->data;
	    if (row == i) {
		selected = 1;
		break;
	    }
	    glist = glist->next;
	}
	if (selected == 0) {
	    gtk_clist_select_row(GTK_CLIST(Clist_device), i, 0);
	} else {
	    gtk_clist_unselect_row(GTK_CLIST(Clist_device), i, 0);
	}
    }
}

void Delete_tracks()
{
    gtk_label_set_text(GTK_LABEL(Status_device), "DELETING...");
    gtk_timeout_add(10, Timeout_device_delete, 0);
}

gint Timeout_device_delete(gpointer gdata)
{
    int retval;
    int selected;
    gint i;
    gint row = -1;
    gint total_rows;
    GList *glist;

    total_rows = GTK_CLIST(Clist_device)->rows;
    for (i = 0; i < total_rows; i++) {
	selected = 0;
	glist = GTK_CLIST(Clist_device)->selection;
	while (glist != NULL) {
	    row = (gint) glist->data;
	    if (row == i) {
		selected = 1;
		break;
	    }
	    glist = glist->next;
	}
	if (selected) {
	    retval = Uproar_delete_track(i);
	    if (retval != UPROAR_SUCCESS) {
                fprintf(stderr, "\nError deleting file from device.");
	    }
	}

    }
    gtk_clist_unselect_all(GTK_CLIST(Clist_device));
    Refresh_device_pane();
    return FALSE;
}


void Reformat_memory()
{
    gtk_label_set_text(GTK_LABEL(Status_device), "REFORMATING...");
    gtk_timeout_add(10, Timeout_device_reformat, 0);
}

gint Timeout_device_reformat(gpointer gdata)
{
    int retval;

    retval = Uproar_reformat_memory();
    if (retval != UPROAR_SUCCESS) {
        fprintf(stderr, "\nError reformatting device.");
    }
    gtk_clist_unselect_all(GTK_CLIST(Clist_device));
    Refresh_device_pane();
    return FALSE;
}

void Refresh_device_pane()
{
    int retval;
    int tracknum;
    char *trackname;
    gchar *clist_txt[2];
    int filesize;
    float megabytes;
    char status[80];

    /* load the playlist */
    retval = Uproar_load_playlist();
    if (retval != UPROAR_SUCCESS) {
	gtk_label_set_text(GTK_LABEL(Status_device),
			   "Error loading playlist.");
	return;
    }

    gtk_clist_freeze(GTK_CLIST(Clist_device));

    gtk_clist_clear(GTK_CLIST(Clist_device));

    tracknum = 0;
    while (1) {
	trackname = Uproar_get_track_info(tracknum, &filesize);
	if (trackname == 0) {
	    break;
	}
	megabytes = ((float) filesize) / ((float) 1024 * 1024);
	clist_txt[0] = g_strdup_printf("%s", trackname);
	clist_txt[1] = g_strdup_printf("%.2f", megabytes);
	gtk_clist_append(GTK_CLIST(Clist_device), clist_txt);
	g_free(clist_txt[0]);
	g_free(clist_txt[1]);
	tracknum++;
    }

    gtk_clist_thaw(GTK_CLIST(Clist_device));

    megabytes = ((float) Uproar_get_available()) / ((float) 1024 * 1024);
    sprintf(status, "Space Available %.2fMB", megabytes);
    gtk_label_set_text(GTK_LABEL(Status_device), status);
}

