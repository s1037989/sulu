
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

#include "main.h"
#include "system_pane.h"
#include "device_pane.h"
#include "lib_uproar.h"

GtkWidget *Create_system_pane()
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
    GtkWidget *transfer_item;

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
			      GTK_SIGNAL_FUNC(Selectall_system_pane),
			      (gpointer) "Selectall_system_pane");
    gtk_widget_show(selectall_item);

    /* selection->unselectall */
    unselectall_item = gtk_menu_item_new_with_label("Unselect all");
    gtk_menu_append(GTK_MENU(file_menu), unselectall_item);
    gtk_signal_connect_object(GTK_OBJECT(unselectall_item), "activate",
			      GTK_SIGNAL_FUNC(Unselectall_system_pane),
			      (gpointer) "Unselectall_system_pane");
    gtk_widget_show(unselectall_item);

    /* selection->invert */
    invert_item = gtk_menu_item_new_with_label("Invert Selection");
    gtk_menu_append(GTK_MENU(file_menu), invert_item);
    gtk_signal_connect_object(GTK_OBJECT(invert_item), "activate",
			      GTK_SIGNAL_FUNC(Invert_system_pane),
			      (gpointer) "Invert_system_pane");
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
			      GTK_SIGNAL_FUNC(Refresh_system_pane),
			      (gpointer) "Refresh_system_pane");
    gtk_widget_show(refresh_item);

    /* action->transfer */
    transfer_item = gtk_menu_item_new_with_label("Transfer Selected");
    gtk_menu_append(GTK_MENU(action_menu), transfer_item);
    gtk_signal_connect_object(GTK_OBJECT(transfer_item), "activate",
			      GTK_SIGNAL_FUNC(Transfer_system_pane),
			      (gpointer) "Transfer_system_pane");
    gtk_widget_show(transfer_item);

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
    Status_system = gtk_label_new("");

    /* set the packing policy for resizes */
    gtk_container_add(GTK_CONTAINER(vertical_box), Status_system);
    gtk_box_set_child_packing(GTK_BOX(vertical_box), Status_system, FALSE,
			      FALSE, padding, GTK_PACK_START);
    gtk_widget_show(Status_system);

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
    Clist_system = gtk_clist_new_with_titles(2, titles);
    gtk_clist_set_selection_mode(GTK_CLIST(Clist_system),
				 GTK_SELECTION_MULTIPLE);
    gtk_clist_set_column_width(GTK_CLIST(Clist_system), 0,
			       MIN_PANE_WIDTH - 10);
    gtk_clist_set_column_width(GTK_CLIST(Clist_system), 1, 10);
    gtk_clist_column_titles_passive(GTK_CLIST(Clist_system));

    /* connect the callback */
    gtk_signal_connect(GTK_OBJECT(Clist_system), "select_row",
		       GTK_SIGNAL_FUNC(Select_row_callback), NULL);
    /* add the file list to the window */
    Refresh_system_pane();

    gtk_container_add(GTK_CONTAINER(scrolled_window), Clist_system);
    gtk_widget_show(Clist_system);

    return vertical_box;
}

void Refresh_system_pane()
{
    gchar *clist_txt[2];
    int i;
    struct dirent **namelist;
    int n;
    struct stat statbuf;
    int retval;
    float megabytes;
    gint total_rows;
    gchar cwd[MAX_FILENAME_LEN];

    /* set the status line */
    if (getcwd(cwd, MAX_FILENAME_LEN) == 0) {
	/* ERROR */
	return;
    }
    gtk_label_set_text(GTK_LABEL(Status_system), cwd);

    /* update the list pane */
    gtk_clist_freeze(GTK_CLIST(Clist_system));

    gtk_clist_clear(GTK_CLIST(Clist_system));

    n = scandir(".", &namelist, 0, alphasort);
    if (n < 0) {
	/* ERROR */
	gtk_clist_thaw(GTK_CLIST(Clist_system));
	return;
    }

    /* limit the number of files we will look at */
    if (n > MAX_SYSTEM_FILES) {
	n = MAX_SYSTEM_FILES;
    }

    for (i = 0; i < n; i++) {
	/* skip all dot entries */
	if (strcmp(namelist[i]->d_name, ".") == 0) {
	    continue;
	}

	/* stat the file */
	retval = stat(namelist[i]->d_name, &statbuf);
	if (retval != 0) {
	    /* ERROR */
	    continue;
	}

	/* load the filename */
	clist_txt[0] = g_strdup(namelist[i]->d_name);

	/* check if it is a directory */
	if (S_ISDIR(statbuf.st_mode)) {
	    clist_txt[1] = g_strdup(" ");
	} else {
	    megabytes = ((float) statbuf.st_size) / ((float) 1024 * 1024);
	    if (megabytes < 0.01f) {
		megabytes = 0.01f;
	    }
	    clist_txt[1] = g_strdup_printf("%.2f", megabytes);
	}
	gtk_clist_append(GTK_CLIST(Clist_system), clist_txt);

	g_free(clist_txt[0]);
	g_free(clist_txt[1]);
	free(namelist[i]);
    }
    free(namelist);

    /* set the color */
    if ((FgFile != 0) && (FgDir != 0)) {
	total_rows = GTK_CLIST(Clist_system)->rows;
	for (i = 0; i < total_rows; i++) {
	    if (Is_regular_file(i) == 1) {
		gtk_clist_set_foreground(GTK_CLIST(Clist_system), i,
					 FgFile);
	    } else {
		gtk_clist_set_foreground(GTK_CLIST(Clist_system), i,
					 FgDir);
	    }
	}
    }

    gtk_clist_thaw(GTK_CLIST(Clist_system));
}

void Selectall_system_pane()
{
    gint i;
    gint total_rows;

    total_rows = GTK_CLIST(Clist_system)->rows;
    for (i = 0; i < total_rows; i++) {
	if (Is_regular_file(i) == 1) {
	    gtk_clist_select_row(GTK_CLIST(Clist_system), i, 0);
	} else {
	    gtk_clist_unselect_row(GTK_CLIST(Clist_system), i, 0);
	}
    }
}

void Unselectall_system_pane()
{

    gtk_clist_unselect_all(GTK_CLIST(Clist_system));
}

void Invert_system_pane()
{
    int selected;
    gint i;
    gint row = -1;
    gint total_rows;
    GList *glist;

    total_rows = GTK_CLIST(Clist_system)->rows;
    for (i = 0; i < total_rows; i++) {
	selected = 0;
	glist = GTK_CLIST(Clist_system)->selection;
	while (glist != NULL) {
	    row = (gint) glist->data;
	    if (row == i) {
		selected = 1;
		break;
	    }
	    glist = glist->next;
	}
	if (Is_regular_file(i) == 1) {
	    if (selected == 0) {
		gtk_clist_select_row(GTK_CLIST(Clist_system), i, 0);
	    } else {
		gtk_clist_unselect_row(GTK_CLIST(Clist_system), i, 0);
	    }
	} else {
	    gtk_clist_unselect_row(GTK_CLIST(Clist_system), i, 0);
	}
    }
}

void Transfer_system_pane()
{
    gtk_label_set_text(GTK_LABEL(Status_system), "DOWNLOADING...");
    gtk_timeout_add(10, Timeout_system_download, 0);
}

/* returns -1 on error, 1 if row i is a regular file, 0 otherwise */
int Is_regular_file(gint i)
{
    gint total_rows;
    gint gretval;
    gchar *fname;
    struct stat statbuf;
    int retval;

    total_rows = GTK_CLIST(Clist_system)->rows;
    if (i >= total_rows) {
	/* ERROR index out of range */
	return -1;
    }

    /* get the clist text for the file name */
    gretval = gtk_clist_get_text(GTK_CLIST(Clist_system), i, 0, &fname);
    if (gretval != 1) {
	/* ERROR */
	return -1;
    }

    /* stat the file */
    retval = stat(fname, &statbuf);
    if (retval != 0) {
	/* ERROR */
	return -1;
    }

    /* check if it is a regular file */
    if (S_ISREG(statbuf.st_mode)) {
	return 1;
    }

    return 0;
}

void Select_row_callback(GtkWidget * widget, gint row, gint col,
			 GdkEventButton * ev, gpointer data)
{
    gint gretval;
    gchar *fname;
    struct stat statbuf;
    int retval;

    /* get the clist text for the file name */
    gretval = gtk_clist_get_text(GTK_CLIST(Clist_system), row, 0, &fname);
    if (gretval != 1) {
	/* ERROR */
	return;
    }

    /* stat the file */
    retval = stat(fname, &statbuf);
    if (retval != 0) {
	/* ERROR */
	return;
    }

    /* check if it is a directory */
    if (S_ISDIR(statbuf.st_mode)) {
	retval = chdir(fname);
	if (retval == 0) {
	    Refresh_system_pane();
	}
    }
}

gint Timeout_system_download(gpointer data) 
{
    int selected;
    gint i;
    gint row = -1;
    gint total_rows;
    GList *glist;
    gchar *fname;
    gint gretval;

    total_rows = GTK_CLIST(Clist_system)->rows;
    for (i = 0; i < total_rows; i++) {
	selected = 0;
	glist = GTK_CLIST(Clist_system)->selection;
	while (glist != NULL) {
	    row = (gint) glist->data;
	    if (row == i) {
		selected = 1;
		break;
	    }
	    glist = glist->next;
	}
	if (Is_regular_file(i) == 1) {
	    if (selected) {
		gretval =
		    gtk_clist_get_text(GTK_CLIST(Clist_system), i, 0,
				       &fname);
		if (gretval != 1) {
		    return FALSE;
		}
		Uproar_download(fname);
	    }
	}
    }
    gtk_clist_unselect_all(GTK_CLIST(Clist_system));
    Refresh_device_pane();
    Refresh_system_pane();
    return FALSE;
}
