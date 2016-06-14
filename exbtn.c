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

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi.h>
#include <unistd.h>

#define VendorID 0x056e
#define ProductID 0x00e6

int main(int argc, char* argv[])
{
   int res;
   unsigned char buf[256];
#define MAX_STR 255
   wchar_t wstr[MAX_STR];
   hid_device *handle;
   int i;
   char path[16];
   

   struct hid_device_info *devs, *cur_dev;
        
   if (hid_init())
      return -1;

   devs = hid_enumerate(VendorID, ProductID);
   cur_dev = devs;	
   while (cur_dev) {
      printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
      printf("\n");
      printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
      printf("  Product:      %ls\n", cur_dev->product_string);
      printf("  Release:      %hx\n", cur_dev->release_number);
      printf("  Interface:    %d\n",  cur_dev->interface_number);
      printf("\n");
      
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
      printf("unable to open device\n");
      return 1;
   }

   // Set the hid_read() function to be non-blocking.
   hid_set_nonblocking(handle, 1);

   do {
      res = hid_read(handle, buf, sizeof(buf));
      printf("flush: hid_read res is %d\n", res);
   } while (res != 0) ;
   
   
   hid_set_nonblocking(handle, 0);
        
   // Read requested state. hid_read() has been set to be
   // non-blocking by the call to hid_set_nonblocking() above.
   // This loop demonstrates the non-blocking nature of hid_read().
   res = 0;
   /* while (res == 0) { */
   for (int i=0; i < 10; ++i){
      
      res = hid_read(handle, buf, sizeof(buf));

      printf("hid_read res is %d\n", res);
      
      if (res != 16) continue;

      if (buf[2] == 0x20) {
         printf("fn1\n");
      }
      if (buf[2] == 0x40) {
         printf("fn2\n");
      }
      if (buf[2] == 0x80) {
         printf("fn3\n");
      }
      if (buf[2] == 0x00) {
         printf("Release\n");
      }
   }

   hid_close(handle);

   /* Free static HIDAPI objects. */
   hid_exit();

   return 0;
}
