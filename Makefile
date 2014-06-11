EXTRA_CFLAGS += $(USER_EXTRA_CFLAGS)
EXTRA_CFLAGS += -O1

EXTRA_CFLAGS += -Wno-unused-variable
EXTRA_CFLAGS += -Wno-unused-value
EXTRA_CFLAGS += -Wno-unused-label
EXTRA_CFLAGS += -Wno-unused-parameter
EXTRA_CFLAGS += -Wno-unused-function
EXTRA_CFLAGS += -Wno-unused

EXTRA_CFLAGS += -Wno-uninitialized

EXTRA_CFLAGS += -I$(src)/include

EXTRA_CFLAGE += -DCONFIG_8723AU_BT_COEXIST
EXTRA_CFLAGS += -DCONFIG_8723AU_AP_MODE
EXTRA_CFLAGS += -DCONFIG_8723AU_P2P

export TopDIR ?= $(shell pwd)


clean_more ?=
clean_more += clean_odm-8192c


SUBARCH := $(shell uname -m | sed -e s/i.86/i386/ | sed -e s/ppc/powerpc/)
ARCH ?= $(SUBARCH)
CROSS_COMPILE ?=
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/
INSTALL_PREFIX :=

ifneq ($(KERNELRELEASE),)


8723au-y :=				\
		core/rtw_ap.o		\
		core/rtw_cmd.o		\
		core/rtw_efuse.o	\
		core/rtw_ieee80211.o	\
		core/rtw_ioctl_set.o	\
		core/rtw_led.o		\
		core/rtw_mlme.o		\
		core/rtw_mlme_ext.o	\
		core/rtw_pwrctrl.o	\
		core/rtw_recv.o		\
		core/rtw_security.o	\
		core/rtw_sreset.o	\
		core/rtw_sta_mgt.o	\
		core/rtw_xmit.o		\
		core/rtw_wlan_util.o	\
		hal/hal_com.o		\
		hal/hal_intf.o		\
		hal/Hal8723PwrSeq.o	\
		hal/Hal8723UHWImg_CE.o	\
		hal/HalDMOutSrc8723A_CE.o \
		hal/HalHWImg8723A_BB.o	\
		hal/HalHWImg8723A_MAC.o	\
		hal/HalHWImg8723A_RF.o	\
		hal/HalPwrSeqCmd.o	\
		hal/odm_RegConfig8723A.o \
		hal/rtl8723a_bt-coexist.o \
		hal/odm_debug.o		\
		hal/odm_interface.o	\
		hal/odm_HWConfig.o	\
		hal/odm.o		\
		hal/rtl8723a_cmd.o	\
		hal/rtl8723a_dm.o	\
		hal/rtl8723a_hal_init.o	\
		hal/rtl8723a_phycfg.o	\
		hal/rtl8723a_rf6052.o	\
		hal/rtl8723a_rxdesc.o	\
		hal/rtl8723a_sreset.o	\
		hal/rtl8723a_xmit.o	\
		hal/rtl8723au_led.o	\
		hal/rtl8723au_recv.o	\
		hal/rtl8723au_xmit.o	\
		hal/usb_halinit.o	\
		hal/usb_ops_linux.o	\
		os_dep/ioctl_cfg80211.o	\
		os_dep/mlme_linux.o	\
		os_dep/os_intfs.o	\
		os_dep/recv_linux.o	\
		os_dep/usb_intf.o	\
		os_dep/usb_ops_linux.o	\
		os_dep/xmit_linux.o

CONFIG_RTL8723AS-VAU := m
obj-$(CONFIG_RTL8723AS-VAU) := 8723au.o

else

export CONFIG_RTL8723AS-VAU = m

all: modules

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules

strip:
	$(CROSS_COMPILE)strip 8723au.ko --strip-unneeded

install:
	install -p -m 644 8723au.ko  $(MODDESTDIR)
	/sbin/depmod -a ${KVER}

uninstall:
	rm -f $(MODDESTDIR)/8723au.ko
	/sbin/depmod -a ${KVER}

config_r:
	@echo "make config"
	/bin/bash script/Configure script/config.in

.PHONY: modules clean clean_odm-8192c

clean_odm-8192c:
clean: $(clean_more)
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm -fr .tmp_versions
	rm -fr Module.symvers ; rm -fr Module.markers ; rm -fr modules.order
	cd core ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd hal ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
endif

