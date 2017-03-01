/*
 * IR remote control daemon.
 *
 * Peter Gammie, peteg42 at gmail dot com
 * Commenced March 2010.
 *
 * Yes, this is somewhat like what lirc does: funnel events from the
 * remote HID to a named pipe.
 *
 * Hardwired for the DViCO USB remote, using the /dev/[usb/]hiddev
 * interface.
 *

FIXME probably gets invoked by udev/mdev, so the device name wired in
here is not so critical.

What happens if we get going before the clock sets up the FIFO? Do we
care?

 */

#include "DViCO.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/hiddev.h>

/* ********************************************************************** */

int
main(int argc, char *argv[])
{
  int debug = 1;
  int remote_fd;
  int control_fifo_fd;

  // FIXME generalise: dev name is in the environment ??
  //        char *getenv(const char *name);
  char *control_fifo_path = CONTROL_FIFO_PATH_DEFAULT;
  char *device = "/dev/usb/hiddev0";

  switch(argc) {
  case 1:
    /* No arguments. */
    break;

  case 2:
    device = argv[1];
    break;

  default:
    {
      char *mdev = getenv("MDEV");
      if(mdev != NULL) {
        printf("Using mdev = '%s'\n", mdev);
        device = mdev;
      }
    }
    break;
  }

  if(debug) {
    printf("Initialising remote control driver.\n");
    printf("Using device '%s'\n", device);
  }

  if((remote_fd = open(device, O_RDWR)) < 0) {
    printf("Failed to open the device '%s'.\n", device);
    perror(argv[0]);

    exit(EXIT_FAILURE);
  }

  if((control_fifo_fd = open(control_fifo_path, O_WRONLY)) < 0) {
    printf("Failed to open control FIFO device '%s'.\n", control_fifo_path);
    perror(argv[0]);

    exit(EXIT_FAILURE);
  }

  if(debug) {
    int version;
    struct hiddev_devinfo info;

    ioctl(remote_fd, HIDIOCGVERSION, &version);
    printf("HID driver version %d\n", version);

    ioctl(remote_fd, HIDIOCGDEVINFO, &info);
    printf("device info:\n");
    printf("   bus type:     %d\n", info.bustype);
    printf("   bus num:      %d\n", info.busnum);
    printf("   dev num:      %d\n", info.devnum);
    printf("   if num:       %d\n", info.ifnum);
    printf("   vendor:       0x%04x\n", info.vendor);
    printf("   product:      0x%04x\n", info.product & 0x0000FFFF);
    printf("   version:      %x\n", info.version);
    printf("   num apps:     %d\n", info.num_applications);

    /* FIXME explain what we expect, device + manufacturer id's */
  }

  while(1) {
    struct hiddev_event event;

    if(read(remote_fd, &event, sizeof(struct hiddev_event)) == sizeof(struct hiddev_event)) {
      if(debug) {
        printf("hid: 0x%x value: 0x%04x\n", event.hid, event.value);
      }

      if(event.hid == significant_hid_event_id) {
        if(write(control_fifo_fd, &event.value, sizeof(control_event_t)) != sizeof(control_event_t)) {
          fprintf(stderr, "Write to control FIFO '%s' failed.\n", control_fifo_path);

          exit(EXIT_FAILURE);
        }
      }
    } else {
      /* FIXME device disconnect? */
      fprintf(stderr, "Read from remote device '%s' failed.\n", device);

      exit(EXIT_FAILURE);
    }
  }

  return EXIT_SUCCESS;
}
