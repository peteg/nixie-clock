/*
 * Definitions shared between the PRU and ARM.
 */

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <stdint.h>

#define NUM_DIGITS 4

/* Nixie tube index. */
typedef uint8_t digit_t;
/* Nixie tube values. */
typedef uint8_t digit_val_t;

/* The Russian 74141-equiv blanks for digits A-F. */
#define BLANK_DIGIT 0xA
/* Set the top bit to make a digit flash. */
#define FLASH_DIGIT (1 << 7)

/* ARM tells PRU to halt by sending this message. */
#define HALT "halt"

/* The RPMSG end point on the ARM. */
#define RPMSG_EP "/dev/rpmsg_pru30"

#endif /* _CLOCK_H_ */
