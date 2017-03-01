# On BBB Debian, after installing these packages:
#    ti-pru-cgt-installer
#    pru-software-support-package
PRU_CGT=/usr/share/ti/cgt-pru
PRU_SSP=/usr/lib/ti/pru-software-support-package

SUDO=sudo
CONFIG_PIN=/opt/beaglebone-universal-io/config-pin

HEADER=P9_
# For PRU0 we use P9_91/P9_92 to set pruout for pins 41 and 42
#   see the config-pin script
PRU_PIN_NUMBERS=25 27 28 29 30 31 91 92
HT_GPIO_NUMBER=26
PRU_CORE=0

########################################

GEN_DIR=gen

LINKER_COMMAND_FILE=./AM335x_PRU.cmd
LIBS=--library=$(PRU_SSP)/lib/rpmsg_lib.lib
INCLUDE=--include_path=$(PRU_SSP)/include --include_path=$(PRU_SSP)/include/am335x
STACK_SIZE=0x100
HEAP_SIZE=0x100

CFLAGS = --c99
CFLAGS += -v3 --endian=little --hardware_mac=on --obj_directory=$(GEN_DIR) --pp_directory=$(GEN_DIR) -ppd -ppa
CFLAGS += -O2
CFLAGS += --issue_remarks --display_error_number

LFLAGS = --reread_libs --warn_sections --stack_size=$(STACK_SIZE) --heap_size=$(HEAP_SIZE)

########################################

PRU0_FW	= $(GEN_DIR)/main_pru0_fw.out

TARGETS = $(PRU0_FW)

# LINK_PRU0_FW = (GEN_DIR)/pru0-inline-asm.object (GEN_DIR)/code2_pru1.object
LINK_PRU0_FW=

########################################

.PHONY: all clean install install-pru0 copy_pru0_fw reboot_pru_0

all: $(TARGETS)
	@echo '-	generated firmware: $^'

$(GEN_DIR)/main_pru0.object: main_pru0.c resource_table_pru0.h
	@mkdir -p $(GEN_DIR)
	$(PRU_CGT)/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

$(PRU0_FW): $(GEN_DIR)/main_pru0.object $(LINK_PRU0_FW)
	$(PRU_CGT)/bin/lnkpru -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(LFLAGS) -o $@ $^  $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS) $^

########################################
# FIXME rip out the patsubst?
# FIXME deps are sequential

install: $(patsubst $(GEN_DIR)/main_pru%_fw.out, install_pru%, $(TARGETS))

install_pru0: $(PRU0_FW) copy_pru0_fw config_pinmux reboot_pru0

config_pinmux:
	@echo "-	configuring pinmux"
	$(SUDO) $(CONFIG_PIN) -a $(HEADER)$(HT_GPIO_NUMBER) hi
	$(SUDO) $(CONFIG_PIN) -q $(HEADER)$(HT_GPIO_NUMBER)

	for PIN in $(PRU_PIN_NUMBERS); do \
		$(SUDO) $(CONFIG_PIN) -a $(HEADER)$$PIN pruout ; \
		$(SUDO) $(CONFIG_PIN) -q $(HEADER)$$PIN ; \
	done

copy_pru0_fw: $(PRU0_FW)
	$(SUDO) cp $(PRU0_FW) /lib/firmware/am335x-pru0-fw

halt_pru0:
	@echo "-	stopping pru core 0"
	-echo "4a334000.pru0" | $(SUDO) tee /sys/bus/platform/drivers/pru-rproc/unbind > /dev/null

reboot_pru0: halt_pru0
	@echo "-	starting pru core 0"
	echo "4a334000.pru0" | $(SUDO) tee /sys/bus/platform/drivers/pru-rproc/bind > /dev/null
	@echo "-	pru core 0 is now loaded with $(PRU0_FW)"

clean:
	rm -rf $(GEN_DIR)
