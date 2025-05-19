BAODEMO_PATH = bao-demos
ifeq ($(wildcard bao-demos/),)
  $(shell git clone git@github.com:ariscv/bao-demos.git $(BAODEMO_PATH))
endif

NAME = simpletest
SRCS += startup.S
SRCS += main.c ns16550.c ee_printf.c printk.c
# SRCS += clint.c

#######################################
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build
$(shell mkdir -p $(DST_DIR))
Q  = @

### Compilation targets (a binary image or archive)
IMAGE_REL = build/$(NAME)
IMAGE     = $(abspath $(IMAGE_REL))

### Collect the files to be linked: object files (`.o`) and libraries (`.a`)
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))

LINKAGE   = $(OBJS) 

## 3. General Compilation Flags

### (Cross) compilers, e.g., mips-linux-gnu-g++
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf

### Compilation flags
INC_PATH += $(WORK_DIR)/include 
INCFLAGS += $(addprefix -I, $(INC_PATH))


CFLAGS   += -O0 -MMD -Wall -Werror $(INCFLAGS) \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
            -Wno-main -U_FORTIFY_SOURCE
CXXFLAGS +=  $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -MMD $(INCFLAGS)
LDFLAGS  += -z noexecstack

##################################
CROSS_COMPILE := riscv64-linux-gnu-
# CROSS_COMPILE := /opt/riscv-rv32ima-ilp32/bin/riscv32-unknown-linux-gnu-
COMMON_CFLAGS := -fno-pic -mcmodel=medany -mstrict-align
CFLAGS        += $(COMMON_CFLAGS) -static
ASFLAGS       += $(COMMON_CFLAGS) -O0



######################
CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(WORK_DIR)/linker.ld \
             --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
#######################
COMMON_CFLAGS += -march=rv32ima_zicsr -mabi=ilp32  # overwrite
LDFLAGS       += -melf32lriscv                     # overwrite

all:image

$(DST_DIR)/%.o: %.c
	$(Q) mkdir -p $(dir $@) && echo + CC $<
	$(Q) $(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	$(Q) mkdir -p $(dir $@) && echo + CXX $<
	$(Q) $(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	$(Q) mkdir -p $(dir $@) && echo + CXX $<
	$(Q) $(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.S` -> `.o` (gcc, which preprocesses and calls as)
$(DST_DIR)/%.o: %.S
	$(Q) mkdir -p $(dir $@) && echo + AS $<
	$(Q) $(AS) $(ASFLAGS) -c -o $@ $(realpath $<)


### Rule (link): objects (`*.o`) and libraries (`*.a`) -> `IMAGE.elf`, the final ELF binary to be packed into image (ld)
$(IMAGE).elf: $(OBJS)
	$(Q) echo + LD "->" $(IMAGE_REL).elf
	$(Q) $(LD) $(LDFLAGS) -o $(IMAGE).elf --start-group $(OBJS)  --end-group

image: $(IMAGE).elf
	$(Q) $(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	$(Q) echo + OBJCOPY "->" $(IMAGE_REL).bin
	$(Q) $(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

clean:
	rm -rf $(DST_DIR)

run:image
	$(MAKE) -C .. run IMAGE=$(IMAGE).bin

bao:
	CROSS_COMPILE=$(CROSS_COMPILE) DEMO=baremetal PLATFORM=spike-riscv32 $(MAKE) -C $(BAODEMO_PATH) 
	
.PHONY: all run clean 
###################################

INC_PATH += $(WORK_DIR)