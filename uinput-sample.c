#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define die(str, args...) do {                  \
      perror(str);                              \
      exit(EXIT_FAILURE);                       \
   } while(0)

#define UINPUT_PATH "/dev/uinput"

int
main(void)
{
   int                    fd;
   struct uinput_user_dev uidev;
   struct input_event     ev;
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
   snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
   uidev.id.bustype = BUS_USB;
   uidev.id.vendor  = 0x1;
   uidev.id.product = 0x1;
   uidev.id.version = 1;

   if(write(fd, &uidev, sizeof(uidev)) < 0)
      die("error: write");

   if(ioctl(fd, UI_DEV_CREATE) < 0)
      die("error: ioctl");
   
   printf("wait 5 seconds\n");
   sleep(5);
   printf("start uinput sample\n");
   
    
   for(i = BTN_MISC; i <= BTN_TASK; ++i) {
      printf("button code is 0x%04x\n", i);
      

      memset(&ev, 0, sizeof(struct input_event));
      ev.type = EV_KEY;
      ev.code = i;
      ev.value = 1;
      if(write(fd, &ev, sizeof(struct input_event)) < 0)
         die("error: write");

      /* memset(&ev, 0, sizeof(struct input_event)); */
      /* ev.type = EV_KEY; */
      /* ev.code = i; */
      /* ev.value = 0; */
      /* if(write(fd, &ev, sizeof(struct input_event)) < 0) */
      /*    die("error: write"); */

      memset(&ev, 0, sizeof(struct input_event));
      ev.type = EV_SYN;
      ev.code = 0;
      ev.value = 0;
      if(write(fd, &ev, sizeof(struct input_event)) < 0)
         die("error: write");

      sleep(2);

      memset(&ev, 0, sizeof(struct input_event));
      ev.type = EV_KEY;
      ev.code = i;
      ev.value = 0;
      if(write(fd, &ev, sizeof(struct input_event)) < 0)
         die("error: write");

      memset(&ev, 0, sizeof(struct input_event));
      ev.type = EV_SYN;
      ev.code = 0;
      ev.value = 0;
      if(write(fd, &ev, sizeof(struct input_event)) < 0)
         die("error: write");

      sleep(2);
   }
    
   /* usleep(15000); */

   if(ioctl(fd, UI_DEV_DESTROY) < 0)
      die("error: ioctl");

   close(fd);

   return 0;
}
