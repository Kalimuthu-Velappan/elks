# State the current ELKS kernel version.

VERSION 	= 0	# (0-255)
PATCHLEVEL	= 0	# (0-255)
SUBLEVEL	= 88	# (0-255)
# PRE		= 1	# (0-255)	If not a pre, comment this line.

# Specify the architecture we will use.

ARCH		= i86

# ROOT_DEV specifies the default root-device when making the image.
# This does not yet work under ELKS. See include/linuxmt/config.h to
# change the root device.

ROOT_DEV	= FLOPPY

# Specify the target for `make nbImage`

TARGET_NB_IMAGE	= $(TOPDIR)/nbImage

#########################################################################
#									#
#   From here onwards, it is not normally necessary to edit this file   #
#									#
#########################################################################

# Specify the relative path from here to the root of the ELKS tree.
# Several other variables are defined based on the definition of this
# variable, so it needs to be accurate.

ELKSDIR		= .

#########################################################################
# Define variables directly dependant on the current ELKS version.

VSNCODE1	= $(shell printf '0x%02X%02X%02X' \
			$(VERSION) $(PATCHLEVEL) $(SUBLEVEL))

ifeq (x$(PRE), x)

DIST		= $(shell printf '%u.%u.%u' \
			$(VERSION) $(PATCHLEVEL) $(SUBLEVEL))

VSNCODE		= $(VSNCODE1)00

else

DIST		= $(shell printf '%u.%u.%u-pre%u' \
			$(VERSION) $(PATCHLEVEL) $(SUBLEVEL) $(PRE))

VSNCODE		= $(shell printf '0x%06X%02X' $$[$(VSNCODE1)-1] $(PRE))

endif

#########################################################################
# Specify the linuxMT root directory.

TOPDIR		= $(shell cd "$(ELKSDIR)" ; \
			  if [ -n "$$PWD" ]; \
				then echo $$PWD ; \
				else pwd ; \
			  fi)

#########################################################################
# Specify the various directories.

ARCH_DIR	= arch/$(ARCH)

DISTDIR 	= elks-$(DIST)

INCDIR		= $(TOPDIR)/include

#########################################################################
# Specify the standard definitions to be given to ELKS programs.

CCDATE		= $(shell date | tr ' ' ' ')

CCDEFS		= -DELKS_VERSION_CODE=$(VSNCODE)		\
		  -DUTS_RELEASE=\"$(DIST)\"			\
		  -DUTS_VERSION=\"\#$(DIST) $(CCDATE)\"		\
		  -D__KERNEL__

#########################################################################
# Specify the tools to use, with their flags.

CC		= bcc
CFLBASE 	= $(CCDEFS) -O
CFLAGS		= $(CFLBASE) -i
CPP		= $(CC) -I$(TOPDIR)/include -E $(CCDEFS)
CC_PROTO	= gcc -I$(TOPDIR)/include -M $(CCDEFS)

LINT		= lclint

CONFIG_SHELL	:= $(shell if [ -x "$$bash" ]; \
				then echo $$bash ; \
				else if [ -x /bin/bash ]; \
					then echo /bin/bash ; \
					else echo sh ; \
				     fi ; \
			   fi)

#########################################################################
# Export all variables.

.EXPORT_ALL_VARIABLES:

#########################################################################
# general construction rules

.c.s:
	$(CC) $(CFLAGS) -0 -nostdinc -I$(ELKSDIR)/include -S -o $*.s $<

.s.o:
	$(AS) -0 -I$(ELKSDIR)/include -c -o $*.o $<

.S.s:
	gcc -E -traditional $(CCDEFS) -o $*.s $<

.c.o:
	$(CC) $(CFLAGS) -0 -I$(ELKSDIR)/include -c -o $*.o $<

#########################################################################
# Targets

elks:
	make -C $(ELKSDIR) all

all:	do-it-all

# Make "config" the default target if there is no configuration file or
# "depend" the target if there is no top-level dependency information.

ifeq (.config,$(wildcard .config))
include .config
#ifeq (.depend,$(wildcard .depend))
#include .depend
do-it-all:      Image
#else
#CONFIGURATION = dep
#do-it-all:      dep
#endif
else
CONFIGURATION = config
do-it-all:      config
endif

#########################################################################
# What do we need this time?

ARCHIVES=kernel/kernel.a fs/fs.a lib/lib.a net/net.a

#########################################################################
# Check what filesystems to include

ifeq ($(CONFIG_ELKSFS_FS), y)
	ARCHIVES := $(ARCHIVES) fs/elksfs/elksfs.a
endif

ifeq ($(CONFIG_MINIX_FS), y)
	ARCHIVES := $(ARCHIVES) fs/minix/minixfs.a
endif

ifeq ($(CONFIG_ROMFS_FS), y)
	ARCHIVES := $(ARCHIVES) fs/romfs/romfs.a
endif

#########################################################################
# Define commands.

Image: $(ARCHIVES) init/main.o
	make -C $(ARCH_DIR) Image

nbImage: $(ARCHIVES) init/main.o
	make -C $(ARCH_DIR) nbImage

nb_install: nbImage
	cp -f $(ARCH_DIR)/boot/nbImage $(TARGET_NB_IMAGE)

nbrd_install: nbImage
	cp -f $(ARCH_DIR)/boot/nbImage $(ARCH_DIR)/boot/nbImage.rd
	cat $(ARCH_DIR)/boot/nbRamdisk >> $(ARCH_DIR)/boot/nbImage.rd
	cp -f $(ARCH_DIR)/boot/nbImage.rd $(TARGET_NB_IMAGE)

boot: Image
	make -C $(ARCH_DIR) boot

disk: Image
	make -C $(ARCH_DIR) disk

setup: $(ARCH_DIR)/boot/setup
	make -C $(ARCH_DIR) setup

#########################################################################
# library rules (all these are built even if they aren't used)

.PHONY: fs/fs.a fs/minix/minixfs.a fs/romfs/romfs.a fs/elksfs/elksfs.a \
        kernel/kernel.a lib/lib.a net/net.a

fs/fs.a:
	make -C fs

fs/elksfs/elksfs.a:
	make -C fs/elksfs

fs/minix/minixfs.a:
	make -C fs/minix

fs/romfs/romfs.a:
	make -C fs/romfs

kernel/kernel.a:
	make -C kernel

lib/lib.a:
	make -C lib

net/net.a:
	make -C net

#########################################################################
# lint rule

lint:
	$(LINT) -I$(TOPDIR)/include -c init/main.c
	@echo
	@echo Checking with lint is now complete.
	@echo

#########################################################################
# Specification files for archives.

elks.spec: Makefile
	@scripts/elksspec $(DISTDIR) $(DIST) $(shell date +%Y.%m.%d)

#########################################################################
# miscellaneous

clean:
	rm -f *~ Boot.map Setup.map System.map tmp_make core
	rm -f init/*~ init/*.o
	make -C $(ARCH_DIR) clean
	make -C fs clean
	make -C fs/elksfs clean
	make -C fs/minix clean
	make -C fs/romfs clean
	make -C kernel clean
	make -C lib clean
	make -C net clean
	make -C scripts clean
	@echo
	@echo The ELKS source tree has been cleaned.
	@echo

nodep:
	@for i in `find -name Makefile`; do \
		sed '/\#\#\# Dependencies/q' < $$i > tmp_make ; \
		if ! diff Makefile tmp_make > /dev/null ; then \
			mv tmp_make $$i ; \
		else \
			rm -f tmp_make ; \
		fi ; \
	done
	@echo
	@echo All dependencies have been removed.
	@echo

distclean: clean nodep
	rm -f .config* .menuconfig*
	rm -f scripts/lxdialog/*.o scripts/lxdialog/lxdialog
	@echo
	@echo This ELKS source tree has been cleaned ready for distribution.
	@echo

dist:
	-rm -rf $(DISTDIR)
	mkdir $(DISTDIR)
	-chmod 777 $(DISTDIR)
	cp -pf BUGS CHANGELOG COPYING Makefile $(DISTDIR)
	cp -pf nodeps README RELNOTES TODO $(DISTDIR)
	(cd $(DISTDIR); mkdir Documentation fs include init kernel lib net)
	(cd $(DISTDIR); mkdir -p $(ARCH_DIR) scripts)
	(cd $(DISTDIR)/fs; mkdir elksfs minix romfs)
	(cd $(DISTDIR)/include; mkdir arch linuxmt)
	make -C $(ARCH_DIR) distdir
	make -C fs distdir
	make -C fs/elksfs distdir
	make -C fs/minix distdir
	make -C fs/romfs distdir
	make -C kernel distdir
	make -C lib distdir
	make -C net distdir
	make -C scripts distdir
	cp -pf include/linuxmt/*.h $(DISTDIR)/include/linuxmt
	cp -pf include/arch/*.h $(DISTDIR)/include/arch
	cp -pf init/main.c $(DISTDIR)/init
	cp -apf Documentation $(DISTDIR)
	@echo
	@echo Directory $(DISTDIR) contains a clean ELKS distribution tree.
	@echo

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in init/*.c; do echo -n "init/"; $(CC_PROTO) $$i; echo; done) >> tmp_make
	mv tmp_make Makefile
	make -C $(ARCH_DIR) dep
	make -C fs dep
	make -C fs/minix dep
	make -C fs/romfs dep
	make -C fs/elksfs dep
	make -C kernel dep
	make -C lib dep
	make -C net dep
	@echo
	@echo All ELKS dependencies are now configured.
	@echo

#########################################################################
# Create distribution archives.

deb:	tar
	@echo
	@echo I do not yet know how to create *.deb archives, sorry.
	@echo

rpm:	tar rpm.spec
	@echo
	@echo I do not yet know how to create *.rpm archives, sorry.
	@echo I have, however, created the spec file required to do so.
	@echo

tar:	dist
	-chmod -R a+r $(DISTDIR)
	tar chozf $(DISTDIR).tar.gz $(DISTDIR)
	-rm -rf $(DISTDIR)

#########################################################################
# Configuration stuff

config:
	$(CONFIG_SHELL) scripts/Configure arch/$(ARCH)/config.in
	@echo
	@echo Configuration complete.
	@echo

defconfig:
	@yes '' | make config

menuconfig:
	make -C scripts/lxdialog all
	$(CONFIG_SHELL) scripts/Menuconfig arch/$(ARCH)/config.in
	@echo
	@echo COnfiguration complete.
	@echo

test:
	@printf '\t%s\n' $(CCDEFS)

#########################################################################
### Dependencies:
