export V?=0

# If _HOST or _TA specific compilers are not specified, then use CROSS_COMPILE
# HOST_CROSS_COMPILE ?= $(CROSS_COMPILE)
# TA_CROSS_COMPILE ?= $(CROSS_COMPILE)

# .PHONY: all
# all:
#	$(MAKE) -C host CROSS_COMPILE="$(HOST_CROSS_COMPILE)" --no-builtin-variables
#	$(MAKE) -C ta CROSS_COMPILE="$(TA_CROSS_COMPILE)" LDFLAGS=""

#.PHONY: clean
#clean:
#	$(MAKE) -C host clean
#	$(MAKE) -C ta clean
# New stuff from article
TARGET_DIR := $(notdir $(shell pwd))

.PHONY: all
all:
	$(MAKE) -C ta \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		TA_DEV_KIT_DIR=$(TA_DEV_KIT_DIR) \
		CFG_TEE_TA_LOG_LEVEL=4 \
		O=$(O)/ta/$(TARGET_DIR)
	$(MAKE) -C host \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		OPTEE_CLIENT_EXPORT=$(OPTEE_CLIENT_EXPORT) \
		O=$(O)/ca/$(TARGET_DIR) \
		--no-builtin-variables

.PHONY: clean
clean:
	$(MAKE) -C ta \
		TA_DEV_KIT_DIR=$(TA_DEV_KIT_DIR) \
		O=$(O)/ta/$(TARGET_DIR) \
		clean
	$(MAKE) -C host \
		OPTEE_CLIENT_EXPORT=$(OPTEE_CLIENT_EXPORT) \
		O=$(O)/ca/$(TARGET_DIR) \
		clean
	rm -rf $(O)/ca/$(TARGET_DIR)
