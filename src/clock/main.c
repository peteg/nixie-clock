/*
 * FIXME describe
 */

// FIXME main goes first to define GLIBC features, ouch
#include "main.h"
#include "clock.h"
#include "event_loop.h"

#include <signal.h>
#include <stdlib.h>

#include <getopt.h>

#include <fcntl.h>
#include <unistd.h>

/* for umask(2) */
#include <sys/types.h>
#include <sys/stat.h>

/* ********************************************************************** */

static char *cmd_line_opts = "d";

static void
help(char *program_name)
{
  fprintf(stderr, "Usage: %s [-d]\n", program_name);
  /* FIXME document flags */
}

static state_t
configure(int argc, char *argv[])
{
  state_t s;
  int opt;

  if((s = (state_t)malloc(sizeof(struct state))) == NULL) {
    perror(argv[0]);
    exit(EXIT_FAILURE);
  }

  s->debug = false;
  s->prog_name = argv[0];
  s->display_rpmsg_path = RPMSG_EP;
  s->display_rpmsg_fd = -1;
  s->control_fifo_path = CONTROL_FIFO_PATH_DEFAULT;
  s->control_fifo_fd = -1;

  /* FIXME hardwired */
  s->pressure_path = "/sys/bus/i2c/drivers/bmp280/2-0077/iio:device0/in_pressure_input";
  s->temperature_path = "/sys/bus/i2c/drivers/bmp280/2-0077/iio:device0/in_temp_input";
  s->bmp085_sample_time = 5;

  // s->altitude = 885; /* FIXME elevation in metres at Orange, NSW */

  // Randwick, NSW
  s->lon = 151.243714;
  s->lat = -33.914109;
  s->elevation = 70; /* elevation in metres */

  while((opt = getopt(argc, argv, cmd_line_opts)) != -1) {
    switch (opt) {
    case 'd':
      s->debug = true;
      break;
    default:
      help(argv[0]);
      return NULL;
    }
  }

  return s;
}

/* ********************************************************************** */

// FIXME better way of passing state to signal handlers?
static state_t s;

static void
cleanup(void)
{
  event_loop_cleanup(s);
}

/* Courtesy of http://www.cons.org/cracauer/sigint.html */
static void
termination_handler(int sig)
{
  printf("Terminating on signal %d.\n", sig);

  cleanup();

  /* We received the signal, so it cannot have been ignored, right? */
  struct sigaction new_action;

  new_action.sa_handler = SIG_DFL;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction(sig, &new_action, NULL);
  kill(getpid(), sig);
}

int
main(int argc, char *argv[])
{
  int rv;

  if((s = configure(argc, argv)) == NULL) {
    exit(EXIT_FAILURE);
  }

  /* Daemonize. */
  if(!s->debug) {
    pid_t pid, sid;

    pid = fork();

    if(pid < 0) {
      exit(EXIT_FAILURE);
    } else if(pid > 0) {
      exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if(sid < 0) {
      exit(EXIT_FAILURE);
    }

    if((chdir("/")) < 0) {
      exit(EXIT_FAILURE);
    }

    /* Close the standard file descriptors. */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  lprintf(s, 1, "Nixie clock driver.\n");

  /* This complexity courtesy of GLIBC:
   * http://www.gnu.org/s/libc/manual/html_node/Sigaction-Function-Example.html
   * Respect our invoking shell's wishes wrt ignoring signals.
   *
   * This is probably irrelevant for a daemon.
   */
  struct sigaction new_action, old_action;

  new_action.sa_handler = termination_handler;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction(SIGINT, NULL, &old_action);
  if(old_action.sa_handler != SIG_IGN)
    sigaction(SIGINT, &new_action, NULL);
  sigaction(SIGHUP, NULL, &old_action);
  if(old_action.sa_handler != SIG_IGN)
    sigaction(SIGHUP, &new_action, NULL);
  sigaction(SIGTERM, NULL, &old_action);
  if(old_action.sa_handler != SIG_IGN)
    sigaction(SIGTERM, &new_action, NULL);

  /* Ignore SIGPIPE: the syscalls fail with EPIPE instead, letting us
   * handle them locally. */
  new_action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &new_action, NULL);

  /* Best-effort to switch the tubes off and nuke the control FIFO
   * when we terminate. */
  atexit(cleanup);

  dprintf(s, "Main thread init complete.\n");
  rv = event_loop(s);
  dprintf(s, "Main thread done.\n");

  return rv;
}
