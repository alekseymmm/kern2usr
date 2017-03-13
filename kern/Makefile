#LK_SRCDIR ?= /lib/modules/$(shell uname -r)/build/
#LK_SRCDIR ?= /usr/src/kernels/$(shell uname -r)
LK_SRCDIR ?= /usr/src/kernels/4.8.13-1.el7.elrepo.x86_64/
ARCH    ?= x86_64

export ARCH

.PHONY: all clean

all :
	$(MAKE) -C $(LK_SRCDIR) M=$(CURDIR) modules

clean :
	$(MAKE) -C $(LK_SRCDIR) M=$(CURDIR) clean
