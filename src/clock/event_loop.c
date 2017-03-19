/*
 * FIXME describe.
 *
 */

#include "main.h"
#include "event_loop.h"
#include "remote.h"

#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <time.h>
#include <unistd.h>

/* mkfifo(3) */
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/select.h>

/* ********************************************************************** */

static void
event_loop_update_time(state_t s)
{
  /* Poll the RTC. */
  time_t current_time;
  struct tm tm;
  char current_time_str[5];
  int rv;

  current_time = time(NULL);
  localtime_r(&current_time, &tm);

  sprintf(current_time_str, "%d%d%d%d",
          tm.tm_hour / 10, tm.tm_hour % 10,
          tm.tm_min / 10, tm.tm_min % 10);

  dprintf(s, "event_loop_update_time: '%s'\n", current_time_str);

  if((rv = write(s->display_rpmsg_fd, current_time_str, sizeof(current_time_str) / sizeof(char))) < 0) {
    perror("event_loop_update_time()/write()");
    exit(EXIT_FAILURE);
  }
}

static int
event_loop_FIXME(state_t s)
{
  for(bool cont = true; cont; ) {
    int rv;
    int nfds = 0;
    fd_set rd, wr, er;
    struct timeval utimeout;

    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&er);
    FD_SET(s->control_fifo_fd, &rd);
    nfds = s->control_fifo_fd;

    // FIXME compute this: time to next minute
    utimeout.tv_sec = 0;
    utimeout.tv_usec = 500000;

    if((rv = select(nfds + 1, &rd, &wr, &er, &utimeout)) < 0) {
      if(errno == EINTR) {
        continue;
      } else {
        perror("event_loop_FIXME()/select()");
        exit(EXIT_FAILURE);
      }
    }

    if(rv > 0 && FD_ISSET(s->control_fifo_fd, &rd)) {
      int n;
      control_event_t e;

      dprintf(s, "FIXME got an event from the remote\n");

      while((n = read(s->control_fifo_fd, &e, sizeof(e))) > 0) {
        if(n == sizeof(unsigned int)) {
          dprintf(s, "got control event: 0x%04x\n", e);
          // control_event_dispatch(s, e);
        } else {
          dprintf(s, "DISCARDING %d bytes\n", n);
        }
      }
    } else {
      // FIXME for now, but later the controller will need to do this.
      dprintf(s, "FIXME timeout\n");
      event_loop_update_time(s);
    }
  }

  return 0;
}

int
event_loop(state_t s)
{
  dprintf(s, "event_loop()\n");

  /* FIXME permissions */
  dprintf(s, "Creating control fifo: '%s'\n", s->control_fifo_path);
  if(mkfifo(s->control_fifo_path, 0777)) {
    perror(s->prog_name);
    if(errno == EEXIST) {
      fprintf(stderr, "Control fifo '%s' already exists.\n", s->control_fifo_path);
    } else {
      return 1;
    }
  }

  dprintf(s, "Opening control fifo: '%s'\n", s->control_fifo_path);
  // FIXME grot: avoid nasty pipe behaviour: no writers, we get an infinity of EOFs
  if((s->control_fifo_fd = open(s->control_fifo_path, O_NONBLOCK | O_RDWR /*| O_RDONLY*/)) < 0) {
    perror(s->prog_name);
    fprintf(stderr, "Problem opening control fifo '%s'.\n", s->control_fifo_path);
    return 1;
  }

  dprintf(s, "Opening display rpmsg: '%s'\n", s->display_rpmsg_path);
  if((s->display_rpmsg_fd = open(s->display_rpmsg_path, O_WRONLY)) < 0) {
    perror(s->prog_name);
    fprintf(stderr, "Problem opening display rpmsg '%s'.\n", s->display_rpmsg_path);
    return 1;
  }

  event_loop_FIXME(s);

  return 0;
}

void
event_loop_cleanup(state_t s)
{
  dprintf(s, "event_loop() cleanup.\n");

  dprintf(s, "Best-effort removal of control fifo: '%s'\n", s->control_fifo_path);
  unlink(s->control_fifo_path);

  close(s->control_fifo_fd);
  close(s->display_rpmsg_fd);

  s->control_fifo_fd = -1;
  s->display_rpmsg_fd = -1;

  dprintf(s, "event_loop() cleanup finished.\n");
}
