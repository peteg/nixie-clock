/*
 * FIXME describe
 */

#include <stdbool.h>
#include <stdint.h>
#include <pru_cfg.h>

#include "resource_table_pru0.h"

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

volatile register uint32_t __R30;

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
tubes_set_val(digit_t digit, digit_val_t v)
{
  __R30 = DIGIT_VAL(v);
  /* FIXME sleep here long enough for the 74141-equiv to settle. 100us? */
  __delay_cycles(200000);
  __R30 |= TUBE_VAL(digit);
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
  digit_data[digit].pending_val = new_val;
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
    tubes_set_val(digit, d->displayed_val);
    __delay_cycles(DELAY_MAXIMUM * 1000000); // FIXME abstract
    break;

  case ds_crossfading:
    delay = fade[d->fade_index++];

    if(delay == 0) {
      d->displayed_val = d->next_val;
      d->digit_state = ds_steady;
    }
    tubes_set_val(digit, d->displayed_val);
    __delay_cycles(delay * 1000000);
    tubes_set_val(digit, d->next_val);
    __delay_cycles((DELAY_MAXIMUM - delay) * 1000000);
    break;

  case ds_flashing_fade_out:
    delay = fade[d->fade_index++];

    if(delay == 0) {
      d->digit_state = ds_flashing_fade_in;
      /* Pick up a new value if any. */
      d->displayed_val = d->next_val;
    } else {
      tubes_set_val(digit, d->displayed_val);
      __delay_cycles(delay * 1000000);
    }
    tubes_set_val(digit, BLANK_DIGIT);
    __delay_cycles((DELAY_MAXIMUM - delay) * 1000000);
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
      tubes_set_val(digit, d->displayed_val);
      __delay_cycles(delay * 1000000);
    }
    tubes_set_val(digit, BLANK_DIGIT);
    __delay_cycles((DELAY_MAXIMUM - delay) * 1000000);
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

  /* FIXME initial display */
  for(unsigned int i = 0; i < NUM_DIGITS; i++) {
    display_set_pending_val(i, i);
  }
}

/* **************************************** */

void
main(void)
{
  int vals[4] = {1, 2, 3, 4};

  ht_off();
  tubes_all_off();
  /* FIXME set the PRU GPIO to output if we can. */
  tubes_all_off();
  ht_on();

  while (1) {
    for(int i = 0; i < 100; i++) {
      for(int j = 0; j < 4; j++) {
        __R30 = DIGIT_VAL(vals[j]) | TUBE_VAL(j);
        __delay_cycles(100000);
      }
    }
    for(int j = 0; j < 4; j++) {
      vals[j] = (vals[j] + 1) % 10;
    }
  }

  tubes_all_off();
  ht_off();

  __halt();
}
