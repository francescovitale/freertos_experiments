# default target and name of image and executable to generate
TARGET = freeRTOSdemo

# path to the root folder of STM32F3Cube platform
STM_DIR = ../../Materiale_STM_per_STM32F303

# board and MCU names in the directory
BOARD_UC = STM32F3-Discovery_Demo
MCU_UC = STM32F303VC

# board name in the drivers and BSP folders
BSP_BOARD = STM32F3-Discovery

# MCU name used in the .s file
MCU_LC = stm32f303xc

# pre-processor symbol to be defined for the compilation
MCU_MC = STM32F303xC

###################################################################################
# Directories

BSP_DIR	= $(STM_DIR)/Drivers/BSP
HAL_DIR	= $(STM_DIR)/Drivers/STM32f3xx_HAL_Driver
CMSIS_DIR = $(STM_DIR)/Drivers/CMSIS
DEV_DIR	= $(CMSIS_DIR)/Device/ST/STM32F3xx   
USB_DIR	= $(STM_DIR)/Middlewares/ST/STM32_USB_Device_Library
DEM_DIR	= $(STM_DIR)/Projects/STM32F3-Discovery/Demonstrations

###################################################################################
# Sources

HALBASE = $(HAL_DIR)/Src/
HALC = $(HALBASE)STM32F3xx_hal
SRCS = $(HALC).c
SRCS += $(HALC)_rcc.c
SRCS += $(HALC)_gpio.c
SRCS += $(HALC)_cortex.c
SRCS += $(HALC)_tim.c
SRCS += $(HALC)_tim_ex.c
SRCS += $(BSP_DIR)/$(BSP_BOARD)/stm32f3_discovery.c
SRCS += Src/main.c
SRCS += Src_freeRTOS/cmsis_os.c
SRCS += Src_freeRTOS/heap_4.c
SRCS += Src_freeRTOS/list.c
SRCS += Src_freeRTOS/port.c
SRCS += Src_freeRTOS/queue.c
SRCS += Src_freeRTOS/tasks.c
SRCS += Src_freeRTOS/timers.c
SRCS += Src_STM/stm32f3xx_hal_timebase_tim.c
SRCS += Src_STM/stm32f3xx_it.c
SRCS += Src_STM/system_stm32f3xx.c


# remove paths from the file names
SRCS_FN = $(notdir $(SRCS))

#LDFILE = $(DEM_DIR)/SW4STM32/$(BOARD_UC)/$(MCU_UC)Tx_FLASH.ld
LDFILE = STM32F303VCTx_FLASH.ld

###################################################################################
# Tools

PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
AR = $(PREFIX)-ar
OBJCOPY = $(PREFIX)-objcopy
OBJDUMP = $(PREFIX)-objdump
SIZE = $(PREFIX)-size
GDB = $(PREFIX)-gdb-py
ODC = openocd

###################################################################################
# Options

# Defines
DEFS = -D$(MCU_MC) -DUSE_HAL_DRIVER
DEFS += -DUSE_DBPRINTF

INCS = -I$(STM_DIR)/Drivers/CMSIS/Include
INCS += -I$(STM_DIR)/Drivers/CMSIS/Device/ST/STM32F3xx/Include
INCS += -I$(STM_DIR)/Drivers/STM32F3xx_HAL_Driver/Inc
INCS += -I$(HAL_DIR)/Inc
INCS += -I$(STM_DIR)/Drivers/BSP/STM32F3-Discovery
INCS += -IInc
INCS += -IInc_freeRTOS
INCS += -IInc_STM
INCS += -IOptional_Inc

# library search paths
LIBS = -L$(CMSIS_DIR)/Lib

# compiler flags
CFLAGS = -Wall -g  -std=c99 -Os
CFLAGS += -mlittle-endian -mcpu=cortex-m4 -march=armv7e-m -mthumb
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(INCS) $(DEFS)

# linker flags
LDFLAGS = -Wl,--gc-sections -Wl,-Map=$(TARGET).map $(LIBS) -Ttmp/linkerScript.ld

# enable semihosting
LDFLAGS += --specs=rdimon.specs -lc -lrdimon
#LDFLAGS += --specs=noys.specs --specs=nano.specs --specs=rdimon.specs -lc -lrdimon

# source search paths
VPATH = ./tmp
VPATH += $(BSP_DIR)
VPATH += $(HAL_DIR)/Src
VPATH += $(DEV_DIR)/Source/

# debugger flags
OCDFLAGS = -F board/stm32f3discovery.cfg
GDBFLAGS = 

# generate objs and deps target lists by prepending obj/ and dep prefixes
OBJS = $(addprefix obj/,$(SRCS_FN:.c=.o))
DEPS = $(addprefix dep/,$(SRCS_FN:.c=.d))


###################################################################################

.PHONY: all dirs program debug template clean

all: $(TARGET).bin

-include $(DEPS)

dirs: dep obj tmp

dep obj tmp:
	echo "[MKDIR]	$@"
	mkdir -p $@

obj/%.o : %.c | dirs
	echo "Generating \"$@\" from \"$<\""
	$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF dep/$(*F).d

$(TARGET).elf: $(OBJS)
	echo "[LD]	$(TARGET).elf"
	$(CC) $(CFLAGS) $(LDFLAGS) tmp/startup_$(MCU_LC).s $^ -o $@
	echo "[OBJDUMP]	$(TARGET).lst"
	$(OBJDUMP) -St $(TARGET).elf >$(TARGET).lst
	echo "[SIZE]	$(TARGET).elf"
	$(SIZE) $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	echo "[OBJCOPY] $(TARGET).bin"
	$(OBJCOPY) -O binary $< $@

debug:
	$(GDB)	-ex "target extended localhost:3333" \
			-ex "monitor arm semihosting enable" \
			-ex "monitor reset halt" \
			-ex "load" \
			-ex "monitor reset init" \
			$(GDBFLAGS) $(TARGET).elf; \

prepare: tmp
	cp $(SRCS) tmp/
	cp $(STM_DIR)/Drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/gcc/startup_$(MCU_LC).s tmp/
	cp $(LDFILE) tmp/linkerScript.ld

clean:
	echo "[RM]	$(TARGET).bin"; rm -f $(TARGET).bin
	echo "[RM]	$(TARGET).elf"; rm -f $(TARGET).elf
	echo "[RM]	$(TARGET).map"; rm -f $(TARGET).map
	echo "[RM]	$(TARGET).lst"; rm -f $(TARGET).lst
	echo "[RM]	tmp files"; rm -f tmp/*
	echo "[RM]	ld script"; rm -f tmp/linkerScript.ld
	echo "[RMDIR]	dep"; rm -fr dep
	echo "[RMDIR]	obj"; rm -fr obj
	echo "[RMDIR]	tmp"; rm -fr tmp