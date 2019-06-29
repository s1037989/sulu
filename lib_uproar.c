
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
#include <ctype.h>
#include <gtk/gtk.h>

#include <usb.h>

#include "main.h"
#include "lib_uproar.h"

#define MAX_PLAYLIST_DATA       8192
#define MAX_PLAYLIST_RETRY      5
#define USB_SHORT_TIMEOUT       30000
#define USB_LONG_TIMEOUT        60000
#define USB_EP_WRITE            0x02
#define USB_EP_READ             0x83
#define USB_BLOCK_SIZE          0x40

struct usb_device *Uproar_dev = NULL;
usb_dev_handle *Uproar_handle = NULL;

static int uproar_capacity;
static int uproar_available;
static unsigned char playlist_data[MAX_PLAYLIST_DATA];

#define TRACK_NAME_LEN 40
#define TRACK_ID_LEN 9

typedef struct {
    unsigned char track_name[TRACK_NAME_LEN];
    unsigned char track_id[TRACK_ID_LEN];
    int file_size;
} Track_Info;
static unsigned char cur_track_name[TRACK_NAME_LEN];

#define MAX_TRACKS 128
static Track_Info plarray[MAX_TRACKS];
static int plcount = 0;

#define SEND_TRACK_BLOCK_SIZE   0x80
#define SEND_TRACK_NAME_LEN     33
#define SEND_TRACK_DATA_LEN     0x8000


/* 
 *
 * internal functions
 *
 */

void error_message(unsigned char *hdr, unsigned char *txt)
{
    fprintf(stderr, "\nERROR: %-15.15s %s", hdr, txt);
}

void debug_message(unsigned char *hdr, unsigned char *txt)
{
#ifdef _DEBUG
    fprintf(stderr, "\nDEBUG: %-15.15s %s", hdr, txt);
#endif
}

void debug_buffer(unsigned char *hdr, unsigned char *buf)
{
#ifdef _DEBUG
    fprintf(stderr,
	    "\nDEBUG: %-15.15s %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
	    hdr, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
	    buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13],
	    buf[14], buf[15]);
#endif
}

int uproar_buf2int(unsigned char *buf)
{
    int retval;

    retval = ((unsigned char) buf[0]) * 0x1000000;
    retval += ((unsigned char) buf[1]) * 0x10000;
    retval += ((unsigned char) buf[2]) * 0x100;
    retval += ((unsigned char) buf[3]);

    return retval;
}


void uproar_make_title(char *title, char *outTit, int *len)
{
    int i, j, beglen, dot, outlen;
    char tmp[35];

    debug_message("uproar_make_title", "begin");

    beglen = strlen(title);

    dot = 0;
    while (dot < beglen) {
        if (title[dot] == '.') {
            break;
        }
        dot++;
    }


    if ((dot < beglen) && (dot <= 29)) {
        memcpy(tmp, title, dot);
        outlen = dot;
    } else if (beglen <= 29) {
        memcpy(tmp, title, beglen);
        outlen = beglen;
    } else {
        memcpy(tmp, title, 29);
        outlen = 29;
    }

    tmp[outlen] = '.';
    tmp[outlen+1] = 'm';
    tmp[outlen+2] = 'p';
    tmp[outlen+3] = '3';
    tmp[outlen+4] = '\0';
    outlen = strlen(tmp);

    outTit[1] = outlen * 2;
    j = 2;
    for (i = 0; i < outlen; i++) {
	outTit[j] = tmp[i];
	j += 2;
    }
    outTit[j] = 0x2e;

    *len = (outlen * 2) + 3;
}

void uproar_title_frame(int dst, int src)
{
    int i;
    cur_track_name[dst + 0] = playlist_data[src + 1];
    cur_track_name[dst + 1] = playlist_data[src + 3];
    cur_track_name[dst + 2] = playlist_data[src + 5];
    cur_track_name[dst + 3] = playlist_data[src + 7];
    cur_track_name[dst + 4] = playlist_data[src + 9];
    cur_track_name[dst + 5] = playlist_data[src + 14];
    cur_track_name[dst + 6] = playlist_data[src + 16];
    cur_track_name[dst + 7] = playlist_data[src + 18];
    cur_track_name[dst + 8] = playlist_data[src + 20];
    cur_track_name[dst + 9] = playlist_data[src + 22];
    cur_track_name[dst + 10] = playlist_data[src + 24];
    cur_track_name[dst + 11] = playlist_data[src + 28];
    cur_track_name[dst + 12] = playlist_data[src + 30];

    /* scan for ff and null them out */
    for (i = 0; i < 13; i++) {
	if (isprint(cur_track_name[dst + i]) == 0) {
	    cur_track_name[dst + i] = ' ';
	}
    }
}


int uproar_control(usb_dev_handle * dh, int request, unsigned char lsb,
		   unsigned char msb, int timeout)
{
    int retval;
    unsigned char data_buffer[USB_BLOCK_SIZE];

    /* NOTE THAT WE MAY NEED TO SWAP THE INDEX MSB with LSB */
    // int new_idx = msb * 0x100 + lsb;
    int new_idx = lsb * 0x100 + msb;

#ifdef _DEBUG
    fprintf(stderr,
	    "\nDEBUG: %-15.15s %02x %02x %02x %02x %02x %02x %02x %02x, %x",
	    "Control", 0xc0, request, 0x00, 0x00, msb, lsb, 0x01, 0x00,
	    new_idx);
#endif


    /* clear the control msg return data buffer */
    memset(data_buffer, 0, USB_BLOCK_SIZE);

    retval = usb_control_msg(dh,	/* device handle */
			     0xc0,	/* bRequestType */
			     request,	/* bRequest */
			     0x0000,	/* wValue */
			     new_idx,	/* wIndex */
			     data_buffer,	/* return buffer */
			     1,	/* wLength */
			     timeout);

    if (retval < 0) {
	error_message("uproar_control", "usb_control_msg");
	error_message("uproar_control", usb_strerror());
	return -1;
    }

    if (data_buffer[0] != 0x40) {
	error_message("uproar_control", "did not get 0x40");
	return -1;
    }

    return 0;
}


int uproar_send(usb_dev_handle * dh, int request, unsigned char lsb,
		unsigned char msb, char *buffer, int size)
{
    int retval;
    debug_message("uproar_send", "begin");


    retval = uproar_control(dh, request, lsb, msb, USB_SHORT_TIMEOUT);
    if (retval < 0) {
	return retval;
    }
    debug_buffer("uproar_send", buffer);

    retval = usb_bulk_write(dh,	/* device handle */
			    USB_EP_WRITE,	/* end point */
			    buffer,	/* bytes */
			    size,	/* size */
			    USB_SHORT_TIMEOUT);

    if (retval < 0) {
	error_message("uproar_send", "usb_bulk_write");
	error_message("uproar_send", usb_strerror());
    }

    retval = uproar_control(dh, 0x0a, 0xff, 0xff, USB_SHORT_TIMEOUT);
    if (retval < 0) {
	return retval;
    }

    return 0;
}


int uproar_recv(usb_dev_handle * dh, int request, unsigned char lsb,
		unsigned char msb, unsigned char *buffer, int size)
{
    int retval;

    debug_message("uproar_recv", "begin");

    retval = uproar_control(dh, request, lsb, msb, USB_SHORT_TIMEOUT);
    if (retval < 0) {
	return retval;
    }
    retval = usb_bulk_read(dh,	/* device handle */
			   USB_EP_READ,	/* end point */
			   buffer,	/* bytes */
			   size,	/* size */
			   USB_SHORT_TIMEOUT);

    if (retval < 0) {
	error_message("uproar_recv", "usb_bulk_write");
	error_message("uproar_recv", usb_strerror());
	return retval;
    }
    debug_buffer("uproar_recv", buffer);

    retval = uproar_control(dh, 0x0a, 0xff, 0xff, USB_SHORT_TIMEOUT);
    if (retval < 0) {
	return retval;
    }

    return 0;
}


int uproar_file_exists(unsigned char *filename)
{
    int i;

    debug_message("uproar_file_exists", "begin");

    for (i = 0; i < plcount; i++) {
	if (strcasecmp(plarray[i].track_id, filename) == 0) {
	    return 1;
	}
    }
    return 0;
}


int uproar_open()
{
    int retval;

    debug_message("uproar_open", "begin");

    if (Uproar_dev == NULL) {
	error_message("uproar_open", "need to initialize first");
	return -1;
    }

    Uproar_handle = usb_open(Uproar_dev);
    if (Uproar_handle == NULL) {
	error_message("uproar_open", "usb_open failed");
	return -1;
    }

    retval = usb_claim_interface(Uproar_handle, 0);
    if (retval < 0) {
	error_message("uproar_open", "usb_claim_interface failed");
	return -1;
    }

    return 0;
}


int uproar_close()
{
    debug_message("uproar_close", "begin");

    if (Uproar_dev == NULL) {
	error_message("uproar_close", "need to initialize first");
	return -1;
    }

    usb_close(Uproar_handle);
    return 0;
}


/* 
 *
 * external functions
 *
 */


int Uproar_initialize()
{
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_find_busses();
    usb_find_devices();

    debug_message("Uproar_initialize", "begin");

    /* initialize the usb stuff */
    if (Uproar_dev == NULL) {
	usb_init();
	usb_find_busses();
	usb_find_devices();
    }

    Uproar_dev = NULL;
    for (bus = usb_busses; bus; bus = bus->next) {
	for (dev = bus->devices; dev; dev = dev->next) {
	    if ((dev->descriptor.idVendor == UPROAR_VENDOR)
		&& (dev->descriptor.idProduct == UPROAR_PRODUCT)) {
		Uproar_dev = dev;
		debug_message("Uproar_initialize", "device found");
	    }
	}
    }

    if (Uproar_dev == NULL) {
	error_message("Uproar_initialize", "device not found");
	return UPROAR_DEVICE_NOT_FOUND;
    }

    return UPROAR_SUCCESS;
}


unsigned char *Uproar_get_device_info()
{
    usb_dev_handle *dh;
    static unsigned char device_info[USB_BLOCK_SIZE];
    unsigned char send_buffer[USB_BLOCK_SIZE];
    int retval;

    debug_message("Uproar_get_device_info", "begin");

    if (Uproar_dev == NULL) {
	error_message("Uproar_get_device_info",
		      "need to initialize first");
	return 0;
    }

    if (uproar_open() == -1) {
	error_message("Uproar_get_device_info", "uproar_open failed");
	return 0;
    }
    dh = Uproar_handle;

    memset(send_buffer, 0, USB_BLOCK_SIZE);
    send_buffer[0] = 0x23;
    send_buffer[1] = 0x00;
    send_buffer[2] = 0x03;
    send_buffer[3] = 0x3a;
    send_buffer[4] = 0x59;
    send_buffer[5] = 0x2e;
    retval =
	uproar_send(dh, 0x0a, 0x00, 0x00, send_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_get_device_info", "uproar_send failed");
	uproar_close();
	return 0;
    }

    memset(device_info, 0, USB_BLOCK_SIZE);
    retval =
	uproar_recv(dh, 0x0c, 0x00, 0x00, device_info, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_get_device_info", "uproar_recv failed");
	uproar_close();
	return 0;
    }

    uproar_close();
    return &(device_info[2]);
}


int Uproar_load_playlist()
{
    usb_dev_handle *dh;
    unsigned char send_buffer[USB_BLOCK_SIZE];
    unsigned char recv_buffer[USB_BLOCK_SIZE];
    int retval;
    int data_len;
    int playlist_len;
    int maxtrycnt = MAX_PLAYLIST_RETRY;
    int trycnt;
    int i;
    int frames;

    debug_message("Uproar_load_playlist", "begin");

    if (Uproar_dev == NULL) {
	error_message("Uproar_load_playlist", "need to initialize first");
	return UPROAR_ERROR;
    }

    /* try a few times to get the response we like */
    trycnt = 1;
    while ((trycnt < maxtrycnt) && (trycnt > 0)) {

	memset(send_buffer, 0, USB_BLOCK_SIZE);
	send_buffer[0] = 0x23;
	send_buffer[1] = 0x00;
	send_buffer[2] = 0x04;
	send_buffer[3] = 0x3a;
	send_buffer[4] = 0x4d;
	send_buffer[5] = 0x47;
	send_buffer[6] = 0x2e;

	if (uproar_open() == -1) {
	    error_message("Uproar_load_playlist", "uproar_open failed");
	    return UPROAR_ERROR;
	}
	dh = Uproar_handle;

	retval =
	    uproar_send(dh, 0x0a, 0x00, 0x00, send_buffer, USB_BLOCK_SIZE);
	if (retval < 0) {
	    error_message("Uproar_load_playlist", "uproar_send failed");
	    uproar_close();
	    return UPROAR_ERROR;
	}

	memset(recv_buffer, 0, USB_BLOCK_SIZE);
	retval =
	    uproar_recv(dh, 0x0c, 0x00, 0x00, recv_buffer, USB_BLOCK_SIZE);
	if (retval < 0) {
	    error_message("Uproar_load_playlist", "uproar_recv failed");
	    uproar_close();
	    return UPROAR_ERROR;
	}

	if ((recv_buffer[2] == 0x06)
	    && (recv_buffer[3] == 0x3a)
	    && (recv_buffer[4] == 0x4d)
	    && (recv_buffer[5] == 0x47)) {
	    debug_buffer("accept", recv_buffer);
	    break;
	} else {
	    debug_buffer("reject", recv_buffer);
	}

	uproar_close();
	trycnt++;
    }

    if (trycnt >= maxtrycnt) {
	error_message("Uproar_load_playlist", "too many retries");
	return UPROAR_ERROR;
    }

    data_len = (recv_buffer[6] * 256) + recv_buffer[7];
#ifdef _DEBUG
    fprintf(stderr, "\nDEBUG: Uproar_load_playlist() data_len %d",
	    data_len);
#endif

    memset(send_buffer, 0, USB_BLOCK_SIZE);
    send_buffer[0] = 0x23;
    send_buffer[1] = 0x00;
    send_buffer[2] = 0x03;
    send_buffer[3] = 0x3a;
    send_buffer[4] = 0x46;
    send_buffer[5] = 0x2e;
    retval =
	uproar_send(dh, 0x0a, 0x01, 0x00, send_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_load_playlist", "uproar_send failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    playlist_len = (data_len / USB_BLOCK_SIZE) * USB_BLOCK_SIZE;
    playlist_len += USB_BLOCK_SIZE;
#ifdef _DEBUG
    fprintf(stderr, "\nDEBUG: Uproar_load_playlist() playlist_len %d",
	    playlist_len);
#endif
    if (playlist_len < data_len) {
	error_message("Uproar_load_playlist", "length failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    if (playlist_len > MAX_PLAYLIST_DATA) {
	error_message("Uproar_load_playlist", "buffer overflow");
	uproar_close();
	return UPROAR_ERROR;
    }
    memset(playlist_data, 0, MAX_PLAYLIST_DATA);

    retval =
	uproar_recv(dh, 0x0c, 0x01, 0x00, playlist_data, playlist_len);
    if (retval < 0) {
	error_message("Uproar_load_playlist", "uproar_recv failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    /** OKAY transfer was good now close and parse the buffer **/
    uproar_close();

    i = 0;
    while (i < playlist_len) {
	debug_buffer("playlist_data", &(playlist_data[i]));
	i += 16;
    }

    uproar_capacity = uproar_buf2int(&(playlist_data[0]));
    uproar_available = uproar_buf2int(&(playlist_data[4]));

#ifdef _DEBUG
    fprintf(stderr,
	    "\nDEBUG: Uproar_load_playlist() capacity:%d, available:%d",
	    uproar_capacity, uproar_available);
#endif

    /* 
     * Now parse the data into the array 
     */
    i = 8;
    plcount = 0;
    while (playlist_data[i] != 0) {
	frames = playlist_data[i] - 0x40;
#ifdef _DEBUG
	fprintf(stderr, "\nDEBUG: 0x%x Frames:%d, ", playlist_data[i],
		frames);
#endif
	memset(cur_track_name, 0, TRACK_NAME_LEN - 1);
	if (frames == 1) {
	    uproar_title_frame(0, i);
	    cur_track_name[13] = 0;
	    i += 32;
	} else if (frames == 2) {
	    uproar_title_frame(13, i);
	    uproar_title_frame(0, i + 32);
	    cur_track_name[25] = 0;
	    i += 64;
	} else if (frames == 3) {
	    uproar_title_frame(0, i + 64);
	    uproar_title_frame(13, i + 32);
	    uproar_title_frame(26, i);
	    cur_track_name[37] = 0;
	    i += 96;
	} else {
	    /*** all done when frames is not what we expect ***/
	    return UPROAR_SUCCESS;
	}
	strcpy(plarray[plcount].track_name, cur_track_name);

#ifdef _DEBUG
	fprintf(stderr, "Track:%s ", cur_track_name);
#endif

	plarray[plcount].file_size = playlist_data[i + 31] * 0x1000000;
	plarray[plcount].file_size += playlist_data[i + 30] * 0x10000;
	plarray[plcount].file_size += playlist_data[i + 29] * 0x100;
	plarray[plcount].file_size += playlist_data[i + 28];

#ifdef _DEBUG
	fprintf(stderr, "\nDEBUG: Filesize:%d, ",
		plarray[plcount].file_size);
#endif

	memcpy(plarray[plcount].track_id, &(playlist_data[i]),
	       TRACK_ID_LEN - 1);
	plarray[plcount].track_id[TRACK_ID_LEN - 1] = 0;
#ifdef _DEBUG
	fprintf(stderr, "Track ID:%s, ", plarray[plcount].track_id);
#endif

	i += 32;
	plcount++;
    }
    return UPROAR_SUCCESS;
}


unsigned char *Uproar_get_track_info(int tracknum, int *filesize)
{

    if (tracknum < 0) {
	return 0;
    }

    if (tracknum >= plcount) {
	return 0;
    }

    *filesize = plarray[tracknum].file_size;
    return plarray[tracknum].track_name;
}


int Uproar_get_capacity()
{
    return uproar_capacity;
}


int Uproar_get_available()
{
    return uproar_available;
}


int Uproar_num_tracks()
{
    return plcount;
}



int Uproar_delete_track(int tracknum)
{
    usb_dev_handle *dh;
    unsigned char send_buffer[USB_BLOCK_SIZE];
    unsigned char recv_buffer[USB_BLOCK_SIZE];
    int retval;
    unsigned char ufile[TRACK_ID_LEN];
    int ulen;

    debug_message("Uproar_delete_track", "begin");

    if (Uproar_dev == NULL) {
	error_message("Uproar_delete_track", "need to initialize first");
	return UPROAR_ERROR;
    }

    strcpy(ufile, plarray[tracknum].track_id);
    ulen = strlen(ufile);

    if (uproar_open() == -1) {
	error_message("Uproar_delete_track", "uproar_open failed");
	return UPROAR_ERROR;
    }
    dh = Uproar_handle;

    memset(send_buffer, 0, USB_BLOCK_SIZE);
    send_buffer[0] = 0x23;
    send_buffer[1] = 0x00;
    send_buffer[2] = ulen + 8;
    send_buffer[3] = 0x3a;
    send_buffer[4] = 0x4d;
    send_buffer[5] = 0x45;
    memcpy(&(send_buffer[6]), ufile, ulen);
    strcpy(&(send_buffer[6 + ulen]), ".MP3");
    send_buffer[ulen + 10] = 0x2e;

    debug_buffer("send_buffer", send_buffer);

    retval =
	uproar_send(dh, 0x0a, 0x00, 0x00, send_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_delete_track", "uproar_send failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    memset(recv_buffer, 0, USB_BLOCK_SIZE);
    retval =
	uproar_recv(dh, 0x0c, 0x00, 0x00, recv_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_delete_track", "uproar_recv failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    debug_buffer("recv_buffer", recv_buffer);

    if (memcmp(send_buffer, recv_buffer, USB_BLOCK_SIZE) != 0) {
	error_message("Uproar_delete_track", "unexpected recv buffer");
	uproar_close();
	return UPROAR_ERROR;
    }

    memset(send_buffer, 0, USB_BLOCK_SIZE);
    send_buffer[0] = 0x23;
    send_buffer[1] = 0x00;
    send_buffer[2] = 0x03;
    send_buffer[3] = 0x3a;
    send_buffer[4] = 0x46;
    send_buffer[5] = 0x2e;
    retval =
	uproar_send(dh, 0x0a, 0x01, 0x00, send_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_delete_track", "uproar_send failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    memset(recv_buffer, 0, USB_BLOCK_SIZE);
    retval =
	uproar_recv(dh, 0x0c, 0x01, 0x00, recv_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_delete_track", "uproar_recv failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    if ((recv_buffer[4] != 0x00)
	|| (recv_buffer[5] != 0x2e)) {
	error_message("Uproar_delete_track", "unexpected recv buffer 2");
	uproar_close();
	return UPROAR_ERROR;
    }

    uproar_close();
    return UPROAR_SUCCESS;
}


int Uproar_reformat_memory()
{
    usb_dev_handle *dh;
    unsigned char send_buffer[USB_BLOCK_SIZE];
    unsigned char recv_buffer[USB_BLOCK_SIZE];
    int retval;

    debug_message("Uproar_reformat_memory", "begin");

    if (Uproar_dev == NULL) {
	error_message("Uproar_reformat_memory",
		      "need to initialize first");
	return UPROAR_ERROR;
    }

    if (uproar_open() == -1) {
	error_message("Uproar_reformat_memory", "uproar_open failed");
	return UPROAR_ERROR;
    }
    dh = Uproar_handle;

    memset(send_buffer, 0, USB_BLOCK_SIZE);
    send_buffer[0] = 0x23;
    send_buffer[1] = 0x00;
    send_buffer[2] = 0x04;
    send_buffer[3] = 0x3a;
    send_buffer[4] = 0x4d;
    send_buffer[5] = 0x46;
    send_buffer[6] = 0x2e;

    retval =
	uproar_send(dh, 0x0a, 0x00, 0x01, send_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_reformat_memory", "uproar_send failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    memset(recv_buffer, 0, USB_BLOCK_SIZE);
    retval =
	uproar_recv(dh, 0x0c, 0x00, 0x01, recv_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_reformat_memory", "uproar_recv failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    if (memcmp(send_buffer, recv_buffer, USB_BLOCK_SIZE) != 0) {
	error_message("Uproar_reformat_memory", "unexpected recv buffer");
	uproar_close();
	return UPROAR_ERROR;
    }

    memset(send_buffer, 0, USB_BLOCK_SIZE);
    send_buffer[0] = 0x23;
    send_buffer[1] = 0x00;
    send_buffer[2] = 0x03;
    send_buffer[3] = 0x3a;
    send_buffer[4] = 0x46;
    send_buffer[5] = 0x2e;
    retval =
	uproar_send(dh, 0x0a, 0x00, 0x01, send_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_reformat_memory", "uproar_send failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    memset(recv_buffer, 0, USB_BLOCK_SIZE);
    retval =
	uproar_recv(dh, 0x0c, 0x01, 0x01, recv_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_reformat_memory", "uproar_recv failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    if ((recv_buffer[4] != 0x00)
	|| (recv_buffer[5] != 0x2e)) {
	error_message("Uproar_reformat_memory",
		      "unexpected recv buffer 2");
	uproar_close();
	return UPROAR_ERROR;
    }

    uproar_close();
    return UPROAR_SUCCESS;
}

int Uproar_download(unsigned char *filepath)
{
    usb_dev_handle *dh;
    unsigned char send_buffer[SEND_TRACK_BLOCK_SIZE];
    unsigned char recv_buffer[SEND_TRACK_BLOCK_SIZE];
    unsigned char temp_buffer[SEND_TRACK_BLOCK_SIZE];
    int retval;
    unsigned char ufile[TRACK_ID_LEN];
    int ulen;
    int title_len;
    int total_len;
    int nread;

    FILE *fp;
    struct stat statbuf;
    unsigned char data_buffer[SEND_TRACK_DATA_LEN];
    int data_ptr;

    debug_message("Uproar_download", "begin");

    if (Uproar_dev == NULL) {
	error_message("Uproar_download", "need to initialize first");
	return UPROAR_ERROR;
    }

    if (filepath == NULL) {
	error_message("Uproar_download", "NULL Filename");
	return UPROAR_ERROR;
    }

    debug_message("filepath", filepath);
    retval = stat(filepath, &statbuf);
    if (retval != 0) {
	error_message("Uproar_download", "stat file error");
	return UPROAR_ERROR;
    }

    if (statbuf.st_size > uproar_available) {
	error_message("Uproar_download", "not enough space");
	return UPROAR_NOT_ENOUGH_SPACE;
    }

    /* generate a track ID */
    ulen = 0;
    sprintf(ufile, "T~%06d", ulen);

    while (uproar_file_exists(ufile) == 1) {
	ulen++;
	sprintf(ufile, "T~%06d", ulen);
    }
    ulen = strlen(ufile);
    debug_message("Track ID", ufile);

    if (uproar_open() == -1) {
	error_message("Uproar_download", "uproar_open failed");
	return UPROAR_ERROR;
    }
    dh = Uproar_handle;

    memset(send_buffer, 0, SEND_TRACK_BLOCK_SIZE);
    send_buffer[0] = 0x23;
    send_buffer[1] = 0x00;
    send_buffer[2] = 0x00;   /*** len to be filled in later ***/
    send_buffer[3] = 0x3a;
    send_buffer[4] = 0x4d;
    send_buffer[5] = 0x90;
    send_buffer[6] = 0x17;
    send_buffer[7] = 0x02;
    send_buffer[8] = 0x20;
    send_buffer[9] = 0x28;
    send_buffer[10] = 0x61;
    send_buffer[11] = 0x63;
    send_buffer[12] = 0xcd;

    memcpy(temp_buffer, &statbuf.st_size, 4);
    send_buffer[13] = temp_buffer[3];
    send_buffer[14] = temp_buffer[2];
    send_buffer[15] = temp_buffer[1];
    send_buffer[16] = temp_buffer[0];

    send_buffer[17] = 0x32;
    send_buffer[18] = 0x00;
    send_buffer[19] = ulen + 4;

    // Track ID
    memcpy(&(send_buffer[20]), ufile, ulen);
    send_buffer[28] = '.';
    send_buffer[29] = 'M';
    send_buffer[30] = 'P';
    send_buffer[31] = '3';

    // Track Title
    memset(temp_buffer, 0, SEND_TRACK_BLOCK_SIZE);
    uproar_make_title(filepath, temp_buffer, &title_len);
    memcpy(&(send_buffer[ulen + 24]), temp_buffer, title_len);

    total_len = ulen + 24 + title_len;
    send_buffer[2] = total_len - 3;

    debug_buffer("send_buffer", &(send_buffer[0]));
    debug_buffer("send_buffer", &(send_buffer[16]));
    debug_buffer("send_buffer", &(send_buffer[32]));
    debug_buffer("send_buffer", &(send_buffer[48]));
    debug_buffer("send_buffer", &(send_buffer[64]));
    debug_buffer("send_buffer", &(send_buffer[80]));
    debug_buffer("send_buffer", &(send_buffer[96]));
    debug_buffer("send_buffer", &(send_buffer[112]));

    retval = uproar_send(dh, 0x0a, 0x00, 0x00, send_buffer, total_len);
    if (retval < 0) {
	error_message("Uproar_download", "uproar_send failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    retval =
	uproar_recv(dh, 0x0c, 0x00, 0x00, recv_buffer, USB_BLOCK_SIZE);
    if (retval < 0) {
	error_message("Uproar_download", "uproar_recv failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    if ((recv_buffer[2] != 0x08)
	|| (recv_buffer[3] != 0x3a)
	|| (recv_buffer[9] != 0x06)
	|| (recv_buffer[10] != 0x2e)) {
	error_message("Uproar_download", "unexpected recv buffer");
	uproar_close();
	return UPROAR_ERROR;
    }

    retval = uproar_control(dh, 0x0b, 0x00, 0x00, USB_SHORT_TIMEOUT);
    if (retval < 0) {
	error_message("Uproar_download", "uproar_control failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    data_ptr = 0;
    fp = fopen(filepath, "r");
    if (fp == NULL) {
	error_message("Uproar_download", "file open failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    while (data_ptr < statbuf.st_size) {
#ifdef _DEBUG
	fprintf(stderr, "\nDEBUG: Uproar_download() data_ptr %d",
		data_ptr);
#endif
	memset(data_buffer, 0, SEND_TRACK_DATA_LEN);
	nread =
	    fread(data_buffer, sizeof(unsigned char), SEND_TRACK_DATA_LEN,
		  fp);
	data_ptr += nread;
	retval =
	    usb_bulk_write(dh, USB_EP_WRITE, data_buffer,
			   SEND_TRACK_DATA_LEN, USB_LONG_TIMEOUT);
	if (retval < 0) {
	    error_message("Uproar_download", "usb_bulk_write failed");
	    fclose(fp);
	    uproar_close();
	    return UPROAR_ERROR;
	}
    }

    debug_message("Uproar_download", "final control message");
    retval = uproar_control(dh, 0x0a, 0xff, 0xff, USB_LONG_TIMEOUT);
    if (retval < 0) {
	error_message("Uproar_download", "uproar_control failed");
	uproar_close();
	return UPROAR_ERROR;
    }

    uproar_close();

    /*** reload the playlist so we can generate new track IDs ***/
    Uproar_load_playlist();

    return UPROAR_SUCCESS;
}
