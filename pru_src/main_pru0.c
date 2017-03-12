/*
 * FIXME describe
 *
 * FIXME: perhaps halt other PRU?
 */

#include <stdlib.h>

#include <stdbool.h>
#include <stdint.h>

#include <pru_cfg.h>

#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>

// FIXME cleanup
#include "resource_table_pru0.h"
#include "pru_defs.h"

/* PRU GPIO registers. */
volatile register uint32_t __R30;
volatile register uint32_t __R31;

// printf is too big for the PRU. Library?
char *
itoa(int i, char *b)
{
  char *p = b;
  int j;

  if(i < 0) {
    *p = '-';
    p++;
    i *= -1;
  }

  p++;
  for(j = i; j != 0; j /= 10) {
    p++;
  };
  *p = '\0';

  do {
    *--p = '0' + i % 10;
    i /= 10;
  } while(i);

  return b;
}


/* FIXME shared with the ARM controller? */

#define NUM_DIGITS 4

/* The Russian 74141-equiv blanks for digits A-F */
#define BLANK_DIGIT 0xA

typedef uint8_t digit_t;
typedef uint8_t digit_val_t;

/* Set the top bit to make a digit flash. */
#define FLASHING (1 << 7)

/* **************************************** */
/* FIXME make the HT accessible to the PRU. */

static inline void
ht_on(void)
{
}

static inline void
ht_off(void)
{
}

/* **************************************** */

/* The Russian 74141-equiv is hooked up to the lowest nibble of the
 * PRU0 GPIO pins. The next greatest nibble sets which tube is on
 * (one-hot). */
#define DIGIT_VAL(x) ((x) & 0xF)
#define DIGIT_MASK   0xFFF0
#define TUBE_VAL(x)  (1 << (((x) & 0xF) + 4))
#define TUBE_MASK    0xFF0F

static inline void
tubes_all_off(void)
{
  __R30 = 0;
}

static inline void
tubes_set_val(digit_t digit, digit_val_t v, unsigned int delay)
{
  __R30 = DIGIT_VAL(v);
  /* FIXME sleep here long enough for the 74141-equiv to settle. 100us? */
  __delay_cycles(20000);
  __R30 |= TUBE_VAL(digit);

  /* FIXME cheesy hack: __delay_cycles requires a constant */
  while(delay-- > 0) {
    __delay_cycles(10000);
  }
}

/* **************************************** */

/* The brightness of the nixies is controlled by PWM. */

#define DIGIT_VAL_MASK 0x0F

typedef enum {
  ds_steady,
  ds_crossfading,
  ds_flashing_fade_out,
  ds_flashing_fade_in
} digit_state_t;

struct digit_data {
  digit_state_t digit_state;
  digit_val_t displayed_val;
  digit_val_t next_val;

  digit_val_t pending_val;
  uint8_t fade_index;
};

static struct digit_data
digit_data[NUM_DIGITS]; // FIXME initialize?

/* Last one wins semantics. */
static inline void
display_set_pending_val(digit_t digit, unsigned int new_val)
{
  digit_data[NUM_DIGITS - digit - 1].pending_val = new_val;
}

static inline uint8_t
display_get_displayed_val(digit_t digit)
{
  struct digit_data *d = &digit_data[digit];

  return d->displayed_val;
}

/* The maximum value here is DELAY_MAXIMUM.
 * Generated with fade.pl  */
#define DELAY_MAXIMUM 0x3F

static digit_val_t fade[] = {
  0x3F, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E,
  0x3E, 0x3E, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D,
  0x3D, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3B,
  0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3A, 0x3A, 0x3A, 0x3A,
  0x3A, 0x3A, 0x3A, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39,
  0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x37, 0x37, 0x37, 0x37,
  0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x35, 0x35, 0x35,
  0x35, 0x34, 0x34, 0x34, 0x34, 0x34, 0x33, 0x33, 0x33, 0x33,
  0x32, 0x32, 0x32, 0x31, 0x31, 0x31, 0x31, 0x30, 0x30, 0x30,
  0x2F, 0x2F, 0x2F, 0x2E, 0x2E, 0x2E, 0x2D, 0x2D, 0x2D, 0x2C,
  0x2C, 0x2B, 0x2B, 0x2A, 0x2A, 0x29, 0x29, 0x28, 0x28, 0x27,
  0x26, 0x26, 0x25, 0x24, 0x24, 0x23, 0x22, 0x21, 0x20, 0x1F,
  0x1D, 0x1C, 0x1B, 0x19, 0x17, 0x14, 0x12, 0x0E, 0x09, 0x00 };

static void
digit_task(digit_t digit)
{
  struct digit_data *d = &digit_data[digit];
  int delay;

  tubes_all_off();

  switch(d->digit_state) {
  case ds_steady:
    if(d->next_val != d->pending_val) {
      d->next_val = d->pending_val;
      d->digit_state = d->pending_val & FLASHING ? ds_flashing_fade_out : ds_crossfading;
      d->fade_index = 0;
    }
    tubes_set_val(digit, d->displayed_val, DELAY_MAXIMUM);
    break;

  case ds_crossfading:
    delay = fade[d->fade_index++];

    if(delay == 0) {
      d->displayed_val = d->next_val;
      d->digit_state = ds_steady;
    }
    tubes_set_val(digit, d->displayed_val, delay);
    tubes_set_val(digit, d->next_val, DELAY_MAXIMUM - delay);
    break;

  case ds_flashing_fade_out:
    delay = fade[d->fade_index++];

    if(delay == 0) {
      d->digit_state = ds_flashing_fade_in;
      /* Pick up a new value if any. */
      d->displayed_val = d->next_val;
    } else {
      tubes_set_val(digit, d->displayed_val, delay);
    }
    tubes_set_val(digit, BLANK_DIGIT, DELAY_MAXIMUM - delay);
    break;

  case ds_flashing_fade_in:
    delay = fade[d->fade_index++];

    if(delay == 0) {
      if(d->displayed_val != d->next_val) {
        d->digit_state = ds_crossfading;
      } else {
        d->digit_state = ds_flashing_fade_out;
      }
    } else {
      tubes_set_val(digit, d->displayed_val, delay);
    }
    tubes_set_val(digit, BLANK_DIGIT, DELAY_MAXIMUM - delay);
    break;
  }

  return;
}

static inline void
display_init(void)
{
  for(digit_t i = 0; i < NUM_DIGITS; i++) {
    struct digit_data *d = &digit_data[i];

    d->digit_state = ds_steady;
    d->displayed_val = BLANK_DIGIT;
    d->next_val = BLANK_DIGIT;
    d->pending_val = 0;
  }
}

/* **************************************** */
/* RPMSG communication with the ARM host. */

// FIXME only one instance so just use globals

// FIXME cannot be defined inside main (on the stack?)
static uint8_t payload[RPMSG_BUF_SIZE];
static struct pru_rpmsg_transport transport;

#define HOST_ARM_TO_PRU0 HOST0_INT

static void
rpmsg_init(void)
{
  volatile uint8_t *status;

  /* allow OCP master port access by the PRU so the PRU can read external memories */
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

  /* clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
  CT_INTC.SICR_bit.STS_CLR_IDX = SE_ARM_TO_PRU0;

  /* Make sure the Linux drivers are ready for RPMsg communication */
  status = &resourceTable.rpmsg_vdev.status;
  while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

  /* Initialize pru_virtqueue corresponding to vring0 (PRU to ARM Host direction) */
  pru_virtqueue_init(&transport.virtqueue0, &resourceTable.rpmsg_vring0, SE_PRU0_TO_ARM, SE_ARM_TO_PRU0);

  /* Initialize pru_virtqueue corresponding to vring1 (ARM Host to PRU direction) */
  pru_virtqueue_init(&transport.virtqueue1, &resourceTable.rpmsg_vring1, SE_PRU0_TO_ARM, SE_ARM_TO_PRU0);

  /* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
  while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, RPMSG_CHAN_NAME, PRU0_RPMSG_CHAN_DESC, PRU0_RPMSG_CHAN_PORT) != PRU_RPMSG_SUCCESS);
}

static bool
rpmsg_poll(void)
{
  /* Check bit 30 of register R31 to see if the ARM has kicked us */
  if(check_host_int(HOST_ARM_TO_PRU0)) {
    uint16_t src, dst, len;

    /* Clear the event status */
    CT_INTC.SICR_bit.STS_CLR_IDX = SE_ARM_TO_PRU0;

    /* Receive all available messages, multiple messages can be sent per kick */
    while(pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS) {
      static uint32_t xxx = 0;
      char *buf = "xxxxxxxxxx";

      itoa(xxx, buf);
      xxx++;
      pru_rpmsg_send(&transport, dst, src, buf, 10);
      pru_rpmsg_send(&transport, dst, src, payload, len);

      for(int i = 0; i < 4; i++) {
        // FIXME take care of flashing top bit
        display_set_pending_val(i, payload[i] - '0');
      }

      ht_off();
      tubes_all_off();
      /* FIXME set the PRU GPIO to output if we can. */
      tubes_all_off();
      ht_on();

    }

    return true;
  } else {
    return false;
  }
}

/* **************************************** */
// FIXME experiments

void
main(void)
{
  ht_off();
  tubes_all_off();
  ht_on();

  display_init();
  rpmsg_init();

  /* FIXME initial display */
  for(unsigned int i = 0; i < NUM_DIGITS; i++) {
    display_set_pending_val(i, i);
  }

  while(1) {
    for(int i = 0; i < NUM_DIGITS; i++) {
      rpmsg_poll();
      digit_task(i);
    }
  }

  __halt();
}
