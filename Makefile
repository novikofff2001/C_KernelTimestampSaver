# Variables
OBJ_NAME = main
KERNEL_DIR = /lib/modules/$(shell uname -r)/build
CURRENT_DIR = $(PWD)

# Rule to build the kernel module
obj-m += $(OBJ_NAME).o

all:
	make -C $(KERNEL_DIR) M=$(CURRENT_DIR) modules

clean:
	make -C $(KERNEL_DIR) M=$(CURRENT_DIR) clean
