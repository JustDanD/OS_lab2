obj-m := proc_module.o
KERNEL_PATH  := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	make -C $(KERNEL_PATH) M=$(PWD) modules
	sudo insmod ./proc_module.ko
	chmod 777 /proc/lab2
	make -C $(KERNEL_PATH) M=$(PWD) clean
	rm -f Module.symvers
clean:
	rmmod proc_module
# obj-m += proc_module.o
 
# PWD := $(CURDIR)
 
# all:
# 	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# clean:
# 	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean