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
EXTRA_CFLAGS += -Iinclude/generated/uapi

CONFIG_AUTOCFG_CP = y

CONFIG_MP_INCLUDED = y
CONFIG_POWER_SAVING = y
CONFIG_USB_AUTOSUSPEND = n
CONFIG_HW_PWRP_DETECTION = n
CONFIG_WIFI_TEST = n
CONFIG_BT_COEXIST = y
CONFIG_INTEL_WIDI = n
CONFIG_WAPI_SUPPORT = n
CONFIG_EFUSE_CONFIG_FILE = n
CONFIG_EXT_CLK = n
CONFIG_FTP_PROTECT = n
CONFIG_WOWLAN = n

CONFIG_PLATFORM_I386_PC = y

CONFIG_DRVEXT_MODULE = n

export TopDIR ?= $(shell pwd)

OUTSRC_FILES := hal/odm_debug.o	\
		hal/odm_interface.o\
		hal/odm_HWConfig.o\
		hal/odm.o\
		hal/HalPhyRf.o
										
HAL_COMM_FILES := hal/rtl8723a_xmit.o \
		hal/rtl8723a_sreset.o

OUTSRC_FILES += hal/Hal8723UHWImg_CE.o

OUTSRC_FILES += hal/HalHWImg8723A_BB.o\
		hal/HalHWImg8723A_MAC.o\
		hal/HalHWImg8723A_RF.o\
		hal/rtl8723a_bt-coexist.o\
		hal/odm_RegConfig8723A.o

OUTSRC_FILES += hal/HalDMOutSrc8192C_CE.o
clean_more ?=
clean_more += clean_odm-8192c

PWRSEQ_FILES := hal/HalPwrSeqCmd.o \
		hal/Hal8723PwrSeq.o

CHIP_FILES += $(HAL_COMM_FILES) $(OUTSRC_FILES) $(PWRSEQ_FILES)

ifeq ($(CONFIG_BT_COEXIST), y)
CHIP_FILES += hal/rtl8723a_bt-coexist.o
endif

_OS_INTFS_FILES :=	os_dep/osdep_service.o \
			os_dep/os_intfs.o \
			os_dep/usb_intf.o \
			os_dep/usb_ops_linux.o \
			os_dep/ioctl_linux.o \
			os_dep/xmit_linux.o \
			os_dep/mlme_linux.o \
			os_dep/recv_linux.o \
			os_dep/ioctl_cfg80211.o \
			os_dep/rtw_android.o

_HAL_INTFS_FILES :=	hal/hal_intf.o \
			hal/hal_com.o \
			hal/rtl8723a_hal_init.o \
			hal/rtl8723a_phycfg.o \
			hal/rtl8723a_rf6052.o \
			hal/rtl8723a_dm.o \
			hal/rtl8723a_rxdesc.o \
			hal/rtl8723a_cmd.o \
			hal/usb_halinit.o \
			hal/rtl8723au_led.o \
			hal/rtl8723au_xmit.o \
			hal/rtl8723au_recv.o

_HAL_INTFS_FILES += hal/usb_ops_linux.o

ifeq ($(CONFIG_MP_INCLUDED), y)
_HAL_INTFS_FILES += hal/rtl8723a_mp.o
endif

_HAL_INTFS_FILES += $(CHIP_FILES)

ifeq ($(CONFIG_USB_AUTOSUSPEND), y)
EXTRA_CFLAGS += -DCONFIG_USB_AUTOSUSPEND
endif

ifeq ($(CONFIG_MP_INCLUDED), y)
EXTRA_CFLAGS += -DCONFIG_MP_INCLUDED
endif

ifeq ($(CONFIG_POWER_SAVING), y)
EXTRA_CFLAGS += -DCONFIG_POWER_SAVING
endif

ifeq ($(CONFIG_HW_PWRP_DETECTION), y)
EXTRA_CFLAGS += -DCONFIG_HW_PWRP_DETECTION
endif

ifeq ($(CONFIG_WIFI_TEST), y)
EXTRA_CFLAGS += -DCONFIG_WIFI_TEST
endif

ifeq ($(CONFIG_BT_COEXIST), y)
EXTRA_CFLAGS += -DCONFIG_BT_COEXIST
endif

ifeq ($(CONFIG_WAPI_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_WAPI_SUPPORT
endif

ifeq ($(CONFIG_EFUSE_CONFIG_FILE), y)
EXTRA_CFLAGS += -DCONFIG_EFUSE_CONFIG_FILE
endif

ifeq ($(CONFIG_EXT_CLK), y)
EXTRA_CFLAGS += -DCONFIG_EXT_CLK
endif

ifeq ($(CONFIG_FTP_PROTECT), y)
EXTRA_CFLAGS += -DCONFIG_FTP_PROTECT
endif

ifeq ($(CONFIG_PLATFORM_I386_PC), y)
EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN
SUBARCH := $(shell uname -m | sed -e s/i.86/i386/)
ARCH ?= $(SUBARCH)
CROSS_COMPILE ?=
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/staging/
INSTALL_PREFIX :=
endif

ifneq ($(USER_MODULE_NAME),)
MODULE_NAME := $(USER_MODULE_NAME)
endif

ifneq ($(KERNELRELEASE),)

rtk_core :=	core/rtw_cmd.o \
		core/rtw_security.o \
		core/rtw_debug.o \
		core/rtw_io.o \
		core/rtw_ioctl_set.o \
		core/rtw_ieee80211.o \
		core/rtw_mlme.o \
		core/rtw_mlme_ext.o \
		core/rtw_wlan_util.o \
		core/rtw_pwrctrl.o \
		core/rtw_rf.o \
		core/rtw_recv.o \
		core/rtw_sta_mgt.o \
		core/rtw_ap.o \
		core/rtw_xmit.o	\
		core/rtw_p2p.o \
		core/rtw_tdls.o \
		core/rtw_br_ext.o \
		core/rtw_iol.o \
		core/rtw_led.o \
		core/rtw_sreset.o

8723au-y += $(rtk_core)

8723au-$(CONFIG_INTEL_WIDI) += core/rtw_intel_widi.o

8723au-$(CONFIG_WAPI_SUPPORT) += core/rtw_wapi.o	\
					core/rtw_wapi_sms4.o

8723au-y += core/rtw_efuse.o

8723au-y += $(_HAL_INTFS_FILES)

8723au-y += $(_OS_INTFS_FILES)

8723au-$(CONFIG_MP_INCLUDED) += core/rtw_mp.o \
					core/rtw_mp_ioctl.o
8723au-$(CONFIG_MP_INCLUDED)+= core/rtw_bt_mp.o

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

clean: $(clean_more)
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm -fr .tmp_versions
	rm -fr Module.symvers ; rm -fr Module.markers ; rm -fr modules.order
	cd core ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd hal ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
endif

