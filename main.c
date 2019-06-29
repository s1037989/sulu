
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
#include <gtk/gtk.h>
#include <usb.h>

#include "main.h"
#include "system_pane.h"
#include "device_pane.h"
#include "lib_uproar.h"

gint Delete_event(GtkWidget * widget, GdkEvent * event, gpointer data)
{
    gtk_main_quit();
    return (FALSE);
}

int main(int argc, char **argv)
{
    GtkWidget *window;
    GtkWidget *vertical_layout;
    GtkWidget *hpaned;
    GtkWidget *frame1;
    GtkWidget *frame2;
    GtkWidget *system_pane;
    GtkWidget *device_pane;
    GdkColormap *cmap;
    GdkColor col1 = { 0, 0x1000, 0x7000, 0x1000 };
    GdkColor col2 = { 0, 0x7000, 0x1000, 0x1000 };
    int retval;
    char *devstr;

    retval = Uproar_initialize();
    if (retval < 0) {
	fprintf(stderr, "\n\nNO MP3 DEVICE FOUND.\n\n");
	exit(-1);
    }
    devstr = Uproar_get_device_info();
    if (devstr == 0) {
	fprintf(stderr, "\n\nNO MP3 DEVICE STRING.\n\n");
	exit(-1);
    }

    /* initialize the gtk */
    gtk_init(&argc, &argv);

    /* setup the default style */
    gtk_rc_parse(".sulurc");

    /* get some colors */
    cmap = gdk_colormap_get_system();
    if (gdk_colormap_alloc_color(cmap, &col1, FALSE, TRUE) &&
	gdk_colormap_alloc_color(cmap, &col2, FALSE, TRUE)) {
	FgFile = &col1;
	FgDir = &col2;
    } else {
	fprintf(stderr, "\nNO COLORS");
	FgDir = 0;
	FgFile = 0;
    }

    /* create the main window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(window, "main_window");
    gtk_window_set_title(GTK_WINDOW(window),
			 "Samsung Uproar Linux Utility (sulu) Version 0.04");
    gtk_signal_connect(GTK_OBJECT(window), "destroy",
		       GTK_SIGNAL_FUNC(Delete_event), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_widget_set_usize(GTK_WIDGET(window), STARTUP_X_SIZE,
			 STARTUP_Y_SIZE);

    /* create a vertical box layout */
    vertical_layout = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(window), vertical_layout);
    gtk_widget_show(vertical_layout);

    /* create the side by side pane */
    hpaned = gtk_hpaned_new();
    gtk_paned_set_gutter_size(GTK_PANED(hpaned), GUTTER_SIZE);
    gtk_widget_set_usize(hpaned, 2 * MIN_PANE_WIDTH + GUTTER_SIZE, -1);
    gtk_container_add(GTK_CONTAINER(vertical_layout), hpaned);
    gtk_widget_show(hpaned);

    /* create the system pane, left side */
    system_pane = Create_system_pane();
    gtk_widget_set_name(system_pane, "system_pane");
    frame1 = gtk_frame_new(NULL);
    gtk_paned_pack1(GTK_PANED(hpaned), frame1, TRUE, FALSE);
    gtk_widget_set_usize(frame1, MIN_PANE_WIDTH, -1);
    gtk_widget_show(frame1);
    gtk_container_add(GTK_CONTAINER(frame1), system_pane);
    gtk_widget_show(system_pane);

    /* create the device pane, right side */
    device_pane = Create_device_pane();
    gtk_widget_set_name(device_pane, "device_pane");
    frame2 = gtk_frame_new(NULL);
    gtk_paned_pack2(GTK_PANED(hpaned), frame2, TRUE, FALSE);
    gtk_widget_set_usize(frame2, MIN_PANE_WIDTH, -1);
    gtk_widget_show(frame2);
    gtk_container_add(GTK_CONTAINER(frame2), device_pane);
    gtk_widget_show(device_pane);

    // gtk_paned_add2(GTK_PANED(hpaned), device_pane);
    // gtk_widget_show(device_pane);

    /* show the window and start gtk processing */
    gtk_widget_show(window);
    gtk_main();

    return 0;
}
