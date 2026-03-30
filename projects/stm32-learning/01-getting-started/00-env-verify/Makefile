#
# 环境验证项目 Makefile
# 用于验证开发环境是否可以正常编译STM32-FreeRTOS项目
# 支持多目标: LM3S6965, STM32F103
#

# 交叉编译工具链
CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE = $(CROSS_COMPILE)size

# FreeRTOS内核源文件
FREERTOS_SRC = ../../../../FreeRTOS/FreeRTOS/Source
FREERTOS_INC = $(FREERTOS_SRC)/include

# 编译选项基础
CFLAGS_BASE = -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable \
              -mcpu=cortex-m3 -mthumb \
              -Os -g \
              -ffunction-sections -fdata-sections \
              -Iinclude \
              -I$(FREERTOS_INC) \
              -I$(FREERTOS_SRC)/portable/GCC/ARM_CM3

LDFLAGS_BASE = -mcpu=cortex-m3 -mthumb \
               -Wl,--gc-sections \
               -lm

# 默认目标 - 显示帮助
help:
	@echo "可用的编译目标:"
	@echo "  make all-lm3s6965   - 编译 LM3S6965 版本"
	@echo "  make all-stm32f103  - 编译 STM32F103 版本"
	@echo "  make run-lm3s6965   - 运行 LM3S6965 QEMU"
	@echo "  make run-stm32f103  - 运行 STM32F103 QEMU"
	@echo "  make clean           - 清理所有编译产物"

# ========== LM3S6965 目标 ==========
TARGET_LM3S6965 = env-verify-lm3s6965
ELF_LM3S6965 = $(TARGET_LM3S6965).elf
BIN_LM3S6965 = $(TARGET_LM3S6965).bin
HEX_LM3S6965 = $(TARGET_LM3S6965).hex

CFLAGS_LM3S6965 = $(CFLAGS_BASE)
LDFLAGS_LM3S6965 = $(LDFLAGS_BASE) -TLM3S6965.ld
FREERTOS_OBJS_LM3S6965 = \
    $(FREERTOS_SRC)/list-lm3s6965.o \
    $(FREERTOS_SRC)/queue-lm3s6965.o \
    $(FREERTOS_SRC)/tasks-lm3s6965.o \
    $(FREERTOS_SRC)/portable/GCC/ARM_CM3/port-lm3s6965.o \
    $(FREERTOS_SRC)/portable/MemMang/heap_2-lm3s6965.o
OBJS_LM3S6965 = startup_lm3s6965.o src/main_lm3s6965.o $(FREERTOS_OBJS_LM3S6965)

all-lm3s6965: $(ELF_LM3S6965) $(BIN_LM3S6965) $(HEX_LM3S6965)
	@echo "=== LM3S6965 编译完成 ==="
	@$(SIZE) $(ELF_LM3S6965)

$(ELF_LM3S6965): $(OBJS_LM3S6965)
	@echo "Linking LM3S6965 $@..."
	@$(CC) $(CFLAGS_LM3S6965) $(OBJS_LM3S6965) $(LDFLAGS_LM3S6965) -o $@

startup_lm3s6965.o: startup_lm3s6965.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS_LM3S6965) -c $< -o $@

src/main_lm3s6965.o: src/main_lm3s6965.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS_LM3S6965) -c $< -o $@

# FreeRTOS LM3S6965编译
$(FREERTOS_SRC)/%-lm3s6965.o: $(FREERTOS_SRC)/%.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS_LM3S6965) -c $< -o $@

run-lm3s6965: all-lm3s6965
	@echo "Starting QEMU for LM3S6965..."
	qemu-system-arm -machine lm3s6965evb -kernel $(ELF_LM3S6965) -nographic -serial mon:stdio

debug-lm3s6965: all-lm3s6965
	@echo "Starting QEMU for GDB debugging (LM3S6965)..."
	qemu-system-arm -machine lm3s6965evb -kernel $(ELF_LM3S6965) -nographic -serial mon:stdio -s -S

# ========== STM32F103 目标 ==========
TARGET_STM32F103 = env-verify-stm32f103
ELF_STM32F103 = $(TARGET_STM32F103).elf
BIN_STM32F103 = $(TARGET_STM32F103).bin
HEX_STM32F103 = $(TARGET_STM32F103).hex

CFLAGS_STM32F103 = $(CFLAGS_BASE) -include FreeRTOSConfig_STM32F103.h
LDFLAGS_STM32F103 = $(LDFLAGS_BASE) -TSTM32F103RB.ld
FREERTOS_OBJS_STM32F103 = \
    $(FREERTOS_SRC)/list-stm32f103.o \
    $(FREERTOS_SRC)/queue-stm32f103.o \
    $(FREERTOS_SRC)/tasks-stm32f103.o \
    $(FREERTOS_SRC)/portable/GCC/ARM_CM3/port-stm32f103.o \
    $(FREERTOS_SRC)/portable/MemMang/heap_2-stm32f103.o
OBJS_STM32F103 = startup_stm32f103.o src/main_stm32f103.o $(FREERTOS_OBJS_STM32F103)

all-stm32f103: $(ELF_STM32F103) $(BIN_STM32F103) $(HEX_STM32F103)
	@echo "=== STM32F103 编译完成 ==="
	@$(SIZE) $(ELF_STM32F103)

$(ELF_STM32F103): $(OBJS_STM32F103)
	@echo "Linking STM32F103 $@..."
	@$(CC) $(CFLAGS_STM32F103) $(OBJS_STM32F103) $(LDFLAGS_STM32F103) -o $@

startup_stm32f103.o: startup_stm32f103.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS_STM32F103) -c $< -o $@

src/main_stm32f103.o: src/main_stm32f103.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS_STM32F103) -c $< -o $@

# FreeRTOS STM32F103编译
$(FREERTOS_SRC)/%-stm32f103.o: $(FREERTOS_SRC)/%.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS_STM32F103) -c $< -o $@

run-stm32f103: all-stm32f103
	@echo "Starting QEMU for STM32F103 (olimex-stm32-p103)..."
	qemu-system-arm -machine olimex-stm32-p103 -kernel $(ELF_STM32F103) -nographic -serial mon:stdio

debug-stm32f103: all-stm32f103
	@echo "Starting QEMU for GDB debugging (STM32F103)..."
	qemu-system-arm -machine olimex-stm32-p103 -kernel $(ELF_STM32F103) -nographic -serial mon:stdio -s -S

# ========== 通用规则 ==========
%.bin: %.elf
	@$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	@$(OBJCOPY) -O ihex $< $@

# ========== 清理目标 ==========
clean:
	@rm -f \
		$(ELF_LM3S6965) $(BIN_LM3S6965) $(HEX_LM3S6965) \
		$(ELF_STM32F103) $(BIN_STM32F103) $(HEX_STM32F103) \
		startup_lm3s6965.o startup_stm32f103.o \
		src/main_lm3s6965.o src/main_stm32f103.o \
		$(FREERTOS_SRC)/*-lm3s6965.o $(FREERTOS_SRC)/*-stm32f103.o \
		$(FREERTOS_SRC)/portable/GCC/ARM_CM3/*-lm3s6965.o \
		$(FREERTOS_SRC)/portable/GCC/ARM_CM3/*-stm32f103.o \
		$(FREERTOS_SRC)/portable/MemMang/*-lm3s6965.o \
		$(FREERTOS_SRC)/portable/MemMang/*-stm32f103.o
	@echo "Clean done."

.PHONY: help all-lm3s6965 all-stm32f103 run-lm3s6965 run-stm32f103 debug-lm3s6965 debug-stm32f103 clean
