/*
 * FIXME describe
 */

#ifndef _PRU_DEFS_
#define _PRU_DEFS_

#define SE_PRU0_TO_ARM			16
#define SE_ARM_TO_PRU0			17

#define HOST1_INT			((uint32_t) 1<<31)
#define HOST0_INT			((uint32_t) 1<<30)

#define R31_VECTOR_VALID_STROBE_BIT	5

#define RPMSG_CHAN_NAME			"rpmsg-pru"

#define PRU0_RPMSG_CHAN_DESC		"Channel 30"
#define PRU0_RPMSG_CHAN_PORT		30

#define VIRTIO_CONFIG_S_DRIVER_OK	4

static inline uint32_t
check_host_int(uint32_t host)
{
  extern volatile uint32_t __R31;
  return (__R31 & host);
}

/* static inline void */
/* generate_sys_eve(sys_eve) */
/* { */
/*   extern volatile uint32_t __R31; */
/*   __R31 = ( (1 << R31_VECTOR_VALID_STROBE_BIT) | (SE_PRU1_TO_PRU0-16)); */
/* } */

#endif /* _PRU_DEFS_ */
