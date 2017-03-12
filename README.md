# A BeagleBone Black and PRU driver for a nixie clock


## Background

This sequence of posts is useful:

https://www.zeekhuge.me/post/a_handfull_of_commands_and_scripts_to_get_started_with_beagleboneblack/


## Complications

Use PRU0 and header P9 rather than PRU1 and P8
 - the pins we want to use on P8 seem to affect the boot up process of the BBB
  - see the BBB SRM, p96/97
 - only use the lower 8 bits of PRU0 GPIOs (those on P9)
  - the rest are on P8 and share with the eMMC.


## Licence

All files without explicit copyright notices are covered by the
licence in LICENCE.
