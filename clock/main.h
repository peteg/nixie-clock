/*
 * FIXME
 */

#ifndef _MAIN_H_
#define _MAIN_H_

/* FIXME for lots of stuff. Use another flag? Use this globally,
 * hoping for consistency.*/
/* FIXME sunriset: we want the timezone variable. */
#define _XOPEN_SOURCE 600

/* FIXME for sigaction, timer_create
#define _POSIX_C_SOURCE 199309L*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <syslog.h>

/*
 * Configuration and state of a clock and remote instance.
 *
 * FIXME We would like to compose this, but that's too hard...
 */
typedef struct state {
  bool debug;
  char *prog_name;

  /* The RPMSG device for controlling the display. */
  char *display_rpmsg_device_name;

  /* The remote-control FIFO. */
  char *control_fifo_path;
  int control_fifo_fd;

  /* FIXME below here is configuration, should be persistent ?? */

  /* For computing the sunrise / sunset. */
  double lon, lat;

  /* Pressure, Temperature */
  char *temperature_path;
  char *pressure_path;
  double elevation; /* Elevation in metres, for correcting the pressure reading. */
  unsigned int bmp085_sample_time; /* Seconds between samples. */
  struct bmp085 *bmp085;
} *state_t;

#define CONTROL_FIFO_PATH_DEFAULT "/tmp/clock_control"

#define dprintf(s, ...) if(s->debug) { fprintf(stderr, __VA_ARGS__); fflush(stderr); }
#define lprintf(s, p, ...) if(s->debug) { dprintf(s, __VA_ARGS__); fflush(stderr); } else { syslog(p, __VA_ARGS__); }

#endif /* _MAIN_H_ */
