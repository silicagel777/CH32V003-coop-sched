###############################################################################
# Project makefile
###############################################################################

TARGET = blink
DEBUG = 1
OPT = -Os -flto
BUILD_DIR = build

###############################################################################
# C++ sources / includes / defines
###############################################################################

CPP_SOURCES = \
$(shell find src/ -name "*.cpp") \
$(shell find vendor/ -name "*.cpp")

CPP_INCLUDES = \
-Isrc \
-Ivendor/Core \
-Ivendor/Debug \
-Ivendor/Peripheral/inc

CPP_DEFS = \
-DDEBUG=2

###############################################################################
# C sources / includes / defines
###############################################################################

C_SOURCES = \
$(shell find src/ -name "*.c") \
$(shell find vendor/ -name "*.c")

C_INCLUDES = $(CPP_INCLUDES)

C_DEFS = $(CPP_DEFS)

###############################################################################
# ASM sources / includes / defines
###############################################################################

# ASM sources
ASM_SOURCES = \
vendor/Startup/startup_ch32v00x.s

AS_INCLUDES = 

AS_DEFS = 

###############################################################################
# Binaries
###############################################################################

PREFIX = riscv32-wch-elf-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CXX = $(GCC_PATH)/$(PREFIX)g++
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CXX = $(PREFIX)g++
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
###############################################################################
# Compiler flags
###############################################################################

# mcu
MCU = -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore
# set to rv32eczicsr for non-WCH compiler

# compile gcc flags
CPPFLAGS = $(MCU) $(CPP_DEFS) $(CPP_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -fno-exceptions -fno-rtti

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
CPPFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"
CPPFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

###############################################################################
# Linker flags
###############################################################################

# link script
LDSCRIPT = vendor/Ld/Link.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -nostartfiles -Xlinker --gc-sections -Wl,-Map,"$(BUILD_DIR)/$(TARGET).map" --specs=nano.specs --specs=nosys.specs

###############################################################################
# build the application
###############################################################################

# list of ASM program objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))
# list of C program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
# list of C++ program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(CPP_SOURCES:.cpp=.o))
# list of generated dependecy files
DEPENDENCIES = $(OBJECTS:.o=.d)

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.cpp Makefile
	mkdir -p $(dir $@)
	$(CXX) -c $(CPPFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile
	mkdir -p $(dir $@)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(BIN) $< $@	

###############################################################################
# clean up
###############################################################################

clean:
	-rm -rf $(BUILD_DIR)
  
###############################################################################
# dependencies
###############################################################################

-include $(DEPENDENCIES)
