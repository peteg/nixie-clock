/*
 * Shared definitions.
 *
 * Peter Gammie, peteg42 at gmail dot com
 * Commenced March 2010.
 *
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

#include <semaphore.h>

#include <syslog.h>

/* FIXME really belongs in display.h */
#define NUM_DIGITS 4

/* FIXME forward-reference. Gotta break the loop somewhere. */
struct mpd;

/* Esterel type for display values. FIXME clunky. */
typedef struct {
  unsigned int vals[NUM_DIGITS];
} time_vals_t;

/*
 * Configuration and state of a clock and remote instance.
 *
 * FIXME We would like to compose this, but that's too hard...
 */
typedef struct state {
  bool debug;
  char *prog_name;

  /* The i2c device for controlling the display. */
  char *display_i2c_device_name;

  /* The remote-control FIFO. */
  char *control_fifo_path;
  int control_fifo_fd;

  /* Tell the main thread the gig is over. FIXME well, it doesn't get control back... */
  sem_t term_sem;

  /* mpd */
  struct mpd *mpd;

  /* FIXME below here is configuration, should be persistent ?? */

  /* For computing the sunrise / sunset. */
  double lon, lat;

  /* Alarm in 24hr BCD-ish format. */
  bool alarm_active;
  time_vals_t alarm_time;

  /* mpd */
  char *mpd_host;
  unsigned mpd_port;
  int mpd_timeout; /* FIXME */
  int mpd_reconnect_timeout; /* Seconds between reconnection attempts. FIXME ms? */

  /* Temperature */
  char *temperature_path;
  char *pressure_path;
  double elevation; /* Elevation in metres, for correcting the pressure reading. */
  unsigned int bmp085_sample_time; /* Seconds between samples. */
  struct bmp085 *bmp085;
} *state_t;

#define NSECS_PER_SEC 1000000000L

#define CONTROL_FIFO_PATH_DEFAULT "/tmp/clock_control"

typedef enum {
  OK = 0,
  NOT_OK = 1 /* FIXME */
} init_fn_ret_t;

/* FIXME shared between the two programs. The type of events coming
   from the remote control. */
typedef uint32_t control_event_t;

#define dprintf(s, ...) if(s->debug) { fprintf(stderr, __VA_ARGS__); fflush(stderr); }
#define lprintf(s, p, ...) if(s->debug) { dprintf(s, __VA_ARGS__); fflush(stderr); } else { syslog(p, __VA_ARGS__); }

#endif /* _MAIN_H_ */
