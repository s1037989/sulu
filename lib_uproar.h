
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

 
#ifndef lib_uproar_h
#define lib_uproar_h

#include <usb.h>

#define UPROAR_VENDOR   0x04E8
#define UPROAR_PRODUCT  0x6600

#define UPROAR_SUCCESS                0
#define UPROAR_ERROR                 -1
#define UPROAR_DEVICE_NOT_FOUND      -1000
#define UPROAR_NOT_ENOUGH_SPACE      -1010

extern struct usb_device *Uproar_dev;

int Uproar_initialize();
unsigned char *Uproar_get_device_info();
int Uproar_load_playlist();
unsigned char *Uproar_get_track_info(int tracknum, int *filesize);
int Uproar_get_capacity();
int Uproar_get_available();
int Uproar_num_tracks();
int Uproar_delete_track(int tracknum);
int Uproar_reformat_memory();
int Uproar_download(unsigned char *filepath);


#endif
