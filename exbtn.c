/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009
 
 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/
#define _DEFAULT_SOURCE 1

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include <hidapi.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/types.h>

#define VendorID 0x056e
#define ProductID 0x00e6

#define die(str, args...) do {                  \
      perror(str);                              \
      exit(EXIT_FAILURE);                       \
   } while(0)

#define UINPUT_PATH "/dev/uinput"

#define FN1_KEY_CODE BTN_FORWARD
#define FN2_KEY_CODE BTN_BACK
#define FN3_KEY_CODE BTN_TASK

/* #define FN2_KEY_CODE BTN_7 */
/* #define FN3_KEY_CODE BTN_9 */

int uinput_initialize()
{
   int                    fd;
   struct uinput_user_dev uidev;
   int                    i;

   fd = open(UINPUT_PATH, O_WRONLY | O_NONBLOCK);
   if(fd < 0)
      die("error: open");

   if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
      die("error: ioctl");

   for(i = BTN_MISC; i <=  BTN_9; ++i) {
      if(ioctl(fd, UI_SET_KEYBIT, i) < 0)
         die("error: ioctl");
   }

   for(i = BTN_MOUSE; i <= BTN_TASK; ++i) {
      if(ioctl(fd, UI_SET_KEYBIT, i) < 0)
         die("error: ioctl");
   }

   if(ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
      die("error: ioctl");
   if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)
      die("error: ioctl");
   if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)
      die("error: ioctl");

   memset(&uidev, 0, sizeof(uidev));
   snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Extended Mouse Button Mapper");
   uidev.id.bustype = BUS_USB;
   uidev.id.vendor  = 0x1;
   uidev.id.product = 0x1;
   uidev.id.version = 1;

   if(write(fd, &uidev, sizeof(uidev)) < 0)
      die("error: write");

   if(ioctl(fd, UI_DEV_CREATE) < 0)
      die("error: ioctl");
   
   return fd;
}

void uinput_emit(int fd, __u16 type, __u16 code, __s32 value)
{
   struct input_event ev;
   memset(&ev, 0, sizeof(struct input_event));
   ev.type = type;
   ev.code = code;
   ev.value = value;
   if(write(fd, &ev, sizeof(struct input_event)) < 0)
      die("error: write");
}

void uinput_button_press(int fd, __u16 code)
{
   uinput_emit(fd, EV_KEY, code, 1);
   uinput_emit(fd, EV_SYN, 0, 0);
   usleep(10000);
}

void uinput_button_release(int fd, __u16 code)
{
   uinput_emit(fd, EV_KEY, code, 0);
   uinput_emit(fd, EV_SYN, 0, 0);
   usleep(10000);
}

void button_check(int fd, unsigned char* buf, const unsigned char btn_bit, const int key_code)
{
   static char btn_state = 0;
   
   if (buf[2] & btn_bit) {
      uinput_button_press(fd, key_code);
      btn_state |= btn_bit;

#ifdef DEBUG
      fprintf(stderr, "btn 0x%02X pressed\n", btn_bit);
#endif /* Def: DEBUG */
   } else if (btn_state & btn_bit){
      uinput_button_release(fd, key_code);
      btn_state &= ~btn_bit;

#ifdef DEBUG
      fprintf(stderr, "btn 0x%02X released\n", btn_bit);
#endif /* Def: DEBUG */
   }
}


int main(int argc, char* argv[])
{
   int res;
   unsigned char buf[256];
#define MAX_STR 255
   wchar_t wstr[MAX_STR];
   hid_device *handle;
   int i;
   char path[16];
   int                    fd;

   struct hid_device_info *devs, *cur_dev;
        
   if (hid_init())
      return -1;

   devs = hid_enumerate(VendorID, ProductID);
   cur_dev = devs;	
   while (cur_dev) {
#ifdef DEBUG
      fprintf(stderr, "Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
      fprintf(stderr, "\n");
      fprintf(stderr, "  Manufacturer: %ls\n", cur_dev->manufacturer_string);
      fprintf(stderr, "  Product:      %ls\n", cur_dev->product_string);
      fprintf(stderr, "  Release:      %hx\n", cur_dev->release_number);
      fprintf(stderr, "  Interface:    %d\n",  cur_dev->interface_number);
      fprintf(stderr, "\n");
#endif /* Def: DEBUG */      
      if (cur_dev->interface_number == 2) {
         strncpy(path, cur_dev->path, sizeof(path));
         break;
      }
      
      cur_dev = cur_dev->next;
   }
   hid_free_enumeration(devs);

   // Set up the command buffer.
   memset(buf,0x00,sizeof(buf));
   buf[0] = 0x01;
   buf[1] = 0x81;

   handle = hid_open_path(path);
   if (!handle) {
      fprintf(stderr, "unable to open device\n");
      return 1;
   }

   // Set the hid_read() function to be non-blocking.
   /* hid_set_nonblocking(handle, 1); */

   do {
      /* res = hid_read(handle, buf, sizeof(buf)); */
      res = hid_read_timeout(handle, buf, sizeof(buf), 0);
      fprintf(stderr, "flush: hid_read res is %d\n", res);
   } while (res != 0) ;
   
   
   hid_set_nonblocking(handle, 0);
        
   // Read requested state. hid_read() has been set to be
   // non-blocking by the call to hid_set_nonblocking() above.
   // This loop demonstrates the non-blocking nature of hid_read().
   res = 0;

   fd = uinput_initialize();
   
   /* for (int i=0; i < 10; ++i){ */
   while (1) {
      res = hid_read(handle, buf, sizeof(buf));

      fprintf(stderr, "hid_read res is %d\n", res);
      
      if (res != 16) continue;

      button_check(fd, buf, 0x20, FN1_KEY_CODE);
      button_check(fd, buf, 0x40, FN2_KEY_CODE);
      button_check(fd, buf, 0x80, FN3_KEY_CODE);
   }

   /* close for hidapi */
   hid_close(handle);

   /* Free static HIDAPI objects. */
   hid_exit();

   /* close for uinput */
   if(ioctl(fd, UI_DEV_DESTROY) < 0)
      die("error: ioctl");

   close(fd);

   return 0;
}
