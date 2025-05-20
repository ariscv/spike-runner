
REPO_PATH = repo
ifeq ($(wildcard repo/spike_main),)
#   $(shell git clone --depth=1 git@github.com:NJU-ProjectN/riscv-isa-sim $(REPO_PATH))
#   $(shell git clone --depth=1 git@github.com:msuadOf/riscv-isa-sim.git $(REPO_PATH))
#   $(shell git clone --depth=1 git@github.com:riscv-software-src/riscv-isa-sim.git $(REPO_PATH))
  $(shell git clone --depth=1 git@github.com:ariscv/riscv-isa-sim.git $(REPO_PATH))
endif


ifeq ($(wildcard mini-gdbstub/src),)
  $(shell git clone --depth=1 git@github.com:RinHizakura/mini-gdbstub.git mini-gdbstub)
endif

REPO_BUILD_PATH = $(REPO_PATH)/build
REPO_MAKEFILE = $(REPO_BUILD_PATH)/Makefile
$(REPO_MAKEFILE):
	@mkdir -p $(@D)
	cd $(@D) && $(abspath $(REPO_PATH))/configure
	sed -i -e 's/-g -O2/-g -O0/' $@

SPIKE = $(REPO_BUILD_PATH)/spike
$(SPIKE): $(REPO_MAKEFILE)
	CFLAGS="-fvisibility=hidden" CXXFLAGS="-fvisibility=hidden" $(MAKE) -C $(^D) 

BUILD_DIR = ./build
$(shell mkdir -p $(BUILD_DIR))

inc_dependencies = fesvr riscv disasm customext fdt softfloat spike_main spike_dasm build
INC_PATH  = -I$(REPO_PATH) $(addprefix -I$(REPO_PATH)/, $(inc_dependencies))
# INC_PATH += -I$(NEMU_HOME)/include
lib_dependencies = libspike_main.a libriscv.a libdisasm.a libsoftfloat.a libfesvr.a libfdt.a
INC_LIBS  = $(addprefix $(REPO_PATH)/build/, $(lib_dependencies))

NAME = spike.elf
BINARY = $(BUILD_DIR)/$(NAME)
SRCS += main_official.cc
# SRCS = main.cc
SRCS += gdbstub.cc

$(BINARY): $(SPIKE) $(SRCS) ./mini-gdbstub/build/libgdbstub.a
	g++ -std=c++17 -O0 -g -fPIC -fvisibility=hidden -I./mini-gdbstub/include $(INC_PATH) $(SRCS) $(INC_LIBS) ./mini-gdbstub/build/libgdbstub.a -o $@

./mini-gdbstub/build/libgdbstub.a: Makefile
	make -C ./mini-gdbstub all -B

clean:
	rm -rf $(BUILD_DIR)
	make -C $(REPO_BUILD_PATH) clean

distclean:
	rm  -rf $(REPO_PATH)

all: $(BINARY)
.DEFAULT_GOAL = all

RUN_ARGS+=-i $(IMAGE)@0x80000000 $(ARGS)
run:$(BINARY)
	@ echo " - run $(NAME)"
	$(BINARY) $(RUN_ARGS)
debug:$(BINARY)
	@ echo " - debug $(NAME)"
	$(BINARY) $(RUN_ARGS) -g
opensbi:
	make run IMAGE=../riscv-linux/opensbi/build/platform/myspike/firmware/fw_payload.bin

openocd:
	./repo/build/spike --isa=rv32imafdc_zicsr_zifencei_zicntr_Sstc --rbb-port=1234 -m0x80000000:0x1000000 code/bao-demos/wrkdir/srcs/opensbi/build/platform/myspike/firmware/fw_payload.elf 
	openocd -f spike.cfg
	# ./repo/build/spike --halted -d  --isa=rv32imafdc_zicsr_zifencei_zicntr_Sstc --rbb-port=1234 -m0x80000000:0x1000000 code/bao-demos/wrkdir/srcs/opensbi/build/platform/myspike/firmware/fw_payload.elf 
	./repo/build/spike --isa=rv32imafdc_zicsr_zifencei_zicntr_Sstc --rbb-port=1234 -m0x80000000:0x1000000  -d  --log=`pwd`/spike.trace  code/bao-demos/wrkdir/srcs/opensbi/build/platform/myspike/firmware/fw_payload.elf 

gdb:
	/opt/riscv-rv64g-lp64/bin/riscv64-unknown-linux-gnu-gdb -ex "target remote localhost:1234" ../riscv-linux/opensbi/build/platform/myspike/firmware/fw_payload.elf 


.PHONY: all clean $(SPIKE) run
