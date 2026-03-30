#
# FreeRTOS heap_2内存分配器示例 Makefile
# 学习要点：heap_2内存分配器、内存分配和释放、内存碎片
# 支持多目标: LM3S6965, STM32F103
#

# ========== 基础配置 ==========

# 交叉编译工具链前缀
CROSS_COMPILE = arm-none-eabi-

# 工具链命令
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE = $(CROSS_COMPILE)size

# FreeRTOS内核源文件路径（相对于当前项目目录）
FREERTOS_SRC = ../../../../../FreeRTOS/FreeRTOS/Source
FREERTOS_INC = $(FREERTOS_SRC)/include

# 基础编译选项
CFLAGS_BASE = -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable \
              -mcpu=cortex-m3 -mthumb \
              -Os -g \
              -ffunction-sections -fdata-sections \
              -Iinclude \
              -I$(FREERTOS_INC) \
              -I$(FREERTOS_SRC)/portable/GCC/ARM_CM3

# 基础链接选项
LDFLAGS_BASE = -mcpu=cortex-m3 -mthumb \
               -Wl,--gc-sections \
               -lm

# ========== 帮助目标 ==========

help:
	@echo "=================================================="
	@echo "  FreeRTOS heap_2内存分配器示例 - 编译帮助"
	@echo "=================================================="
	@echo ""
	@echo "可用的编译目标:"
	@echo "  make all-lm3s6965   - 编译 LM3S6965 版本"
	@echo "  make all-stm32f103  - 编译 STM32F103 版本"
	@echo "  make run-lm3s6965   - 在QEMU中运行 LM3S6965"
	@echo "  make run-stm32f103  - 在QEMU中运行 STM32F103"
	@echo "  make debug-lm3s6965 - LM3S6965调试模式（等待GDB）"
	@echo "  make debug-stm32f103 - STM32F103调试模式（等待GDB）"
	@echo "  make clean           - 清理所有编译产物"
	@echo ""
	@echo "学习要点："
	@echo "  - heap_2.c 内存分配器"
	@echo "  - pvPortMalloc() 内存分配"
	@echo "  - vPortFree() 内存释放"
	@echo "  - 内存碎片问题"
	@echo "  - heap_2 与 heap_1 的区别"
	@echo ""

# ========== LM3S6965 平台配置 ==========

TARGET_LM3S6965 = heap-2-lm3s6965
ELF_LM3S6965 = $(TARGET_LM3S6965).elf
BIN_LM3S6965 = $(TARGET_LM3S6965).bin
HEX_LM3S6965 = $(TARGET_LM3S6965).hex

# LM3S6965 编译选项：定义平台宏，使用默认FreeRTOSConfig.h
CFLAGS_LM3S6965 = $(CFLAGS_BASE) -DPLATFORM_LM3S6965

# LM3S6965 链接选项：使用LM3S6965链接脚本
LDFLAGS_LM3S6965 = $(LDFLAGS_BASE) -TLM3S6965.ld

# LM3S6965 FreeRTOS源文件 - 使用heap_2.c
FREERTOS_OBJS_LM3S6965 = \
    $(FREERTOS_SRC)/list-lm3s6965.o \
    $(FREERTOS_SRC)/queue-lm3s6965.o \
    $(FREERTOS_SRC)/tasks-lm3s6965.o \
    $(FREERTOS_SRC)/portable/GCC/ARM_CM3/port-lm3s6965.o \
    $(FREERTOS_SRC)/portable/MemMang/heap_2-lm3s6965.o

# LM3S6965 所有目标文件
OBJS_LM3S6965 = startup_lm3s6965.o src/main.o $(FREERTOS_OBJS_LM3S6965)

# ========== LM3S6965 编译目标 ==========

all-lm3s6965: $(ELF_LM3S6965) $(BIN_LM3S6965) $(HEX_LM3S6965)
	@echo "========================================"
	@echo "  LM3S6965 编译完成！"
	@echo "========================================"
	@$(SIZE) $(ELF_LM3S6965)
	@echo ""
	@echo "运行: make run-lm3s6965"

# 链接ELF文件
$(ELF_LM3S6965): $(OBJS_LM3S6965)
	@echo "链接 LM3S6965 $@..."
	@$(CC) $(CFLAGS_LM3S6965) $(OBJS_LM3S6965) $(LDFLAGS_LM3S6965) -o $@

# 编译启动文件
startup_lm3s6965.o: startup_lm3s6965.c
	@echo "编译 $@..."
	@$(CC) $(CFLAGS_LM3S6965) -c $< -o $@

# 编译主程序
src/main.o: src/main.c
	@echo "编译 $@..."
	@$(CC) $(CFLAGS_LM3S6965) -c $< -o $@

# FreeRTOS源文件编译规则（LM3S6965）
$(FREERTOS_SRC)/%-lm3s6965.o: $(FREERTOS_SRC)/%.c
	@echo "编译 FreeRTOS: $@..."
	@$(CC) $(CFLAGS_LM3S6965) -c $< -o $@

# 运行LM3S6965
run-lm3s6965: all-lm3s6965
	@echo "========================================"
	@echo "  启动 QEMU LM3S6965..."
	@echo "  按 Ctrl+A 然后 X 退出"
	@echo "========================================"
	qemu-system-arm -machine lm3s6965evb -kernel $(ELF_LM3S6965) -nographic -serial mon:stdio

# 调试LM3S6965（等待GDB连接）
debug-lm3s6965: all-lm3s6965
	@echo "========================================"
	@echo "  启动 QEMU LM3S6965 调试模式..."
	@echo "  等待 GDB 连接（端口 1234）"
	@echo "  按 Ctrl+A 然后 X 退出"
	@echo "========================================"
	qemu-system-arm -machine lm3s6965evb -kernel $(ELF_LM3S6965) -nographic -serial mon:stdio -s -S

# ========== STM32F103 平台配置 ==========

TARGET_STM32F103 = heap-2-stm32f103
ELF_STM32F103 = $(TARGET_STM32F103).elf
BIN_STM32F103 = $(TARGET_STM32F103).bin
HEX_STM32F103 = $(TARGET_STM32F103).hex

# STM32F103 编译选项：定义平台宏，使用STM32F103配置文件
CFLAGS_STM32F103 = $(CFLAGS_BASE) -DPLATFORM_STM32F103 -include FreeRTOSConfig_STM32F103.h

# STM32F103 链接选项：使用STM32F103链接脚本
LDFLAGS_STM32F103 = $(LDFLAGS_BASE) -TSTM32F103RB.ld

# STM32F103 FreeRTOS源文件 - 使用heap_2.c
FREERTOS_OBJS_STM32F103 = \
    $(FREERTOS_SRC)/list-stm32f103.o \
    $(FREERTOS_SRC)/queue-stm32f103.o \
    $(FREERTOS_SRC)/tasks-stm32f103.o \
    $(FREERTOS_SRC)/portable/GCC/ARM_CM3/port-stm32f103.o \
    $(FREERTOS_SRC)/portable/MemMang/heap_2-stm32f103.o

# STM32F103 所有目标文件
OBJS_STM32F103 = startup_stm32f103.o src/main.o $(FREERTOS_OBJS_STM32F103)

# ========== STM32F103 编译目标 ==========

all-stm32f103: $(ELF_STM32F103) $(BIN_STM32F103) $(HEX_STM32F103)
	@echo "========================================"
	@echo "  STM32F103 编译完成！"
	@echo "========================================"
	@$(SIZE) $(ELF_STM32F103)
	@echo ""
	@echo "运行: make run-stm32f103"

# 链接ELF文件
$(ELF_STM32F103): $(OBJS_STM32F103)
	@echo "链接 STM32F103 $@..."
	@$(CC) $(CFLAGS_STM32F103) $(OBJS_STM32F103) $(LDFLAGS_STM32F103) -o $@

# 编译启动文件
startup_stm32f103.o: startup_stm32f103.c
	@echo "编译 $@..."
	@$(CC) $(CFLAGS_STM32F103) -c $< -o $@

# 编译主程序（STM32F103版本，复用同一个源文件但用不同编译选项）
src/main-stm32f103.o: src/main.c
	@echo "编译 $@..."
	@$(CC) $(CFLAGS_STM32F103) -c $< -o $@

# FreeRTOS源文件编译规则（STM32F103）
$(FREERTOS_SRC)/%-stm32f103.o: $(FREERTOS_SRC)/%.c
	@echo "编译 FreeRTOS: $@..."
	@$(CC) $(CFLAGS_STM32F103) -c $< -o $@

# 运行STM32F103
run-stm32f103: all-stm32f103
	@echo "========================================"
	@echo "  启动 QEMU STM32F103 (Olimex-P103)..."
	@echo "  按 Ctrl+A 然后 X 退出"
	@echo "========================================"
	qemu-system-arm -machine olimex-stm32-p103 -kernel $(ELF_STM32F103) -nographic -serial mon:stdio

# 调试STM32F103（等待GDB连接）
debug-stm32f103: all-stm32f103
	@echo "========================================"
	@echo "  启动 QEMU STM32F103 调试模式..."
	@echo "  等待 GDB 连接（端口 1234）"
	@echo "  按 Ctrl+A 然后 X 退出"
	@echo "========================================"
	qemu-system-arm -machine olimex-stm32-p103 -kernel $(ELF_STM32F103) -nographic -serial mon:stdio -s -S

# ========== 通用规则 ==========

# 生成二进制文件 (.bin)
%.bin: %.elf
	@$(OBJCOPY) -O binary $< $@

# 生成Intel Hex文件 (.hex)
%.hex: %.elf
	@$(OBJCOPY) -O ihex $< $@

# ========== 清理目标 ==========

clean:
	@echo "清理编译产物..."
	@rm -f \
		$(ELF_LM3S6965) $(BIN_LM3S6965) $(HEX_LM3S6965) \
		$(ELF_STM32F103) $(BIN_STM32F103) $(HEX_STM32F103) \
		startup_lm3s6965.o startup_stm32f103.o \
		src/main.o src/main-stm32f103.o \
		$(FREERTOS_SRC)/*-lm3s6965.o $(FREERTOS_SRC)/*-stm32f103.o \
		$(FREERTOS_SRC)/portable/GCC/ARM_CM3/*-lm3s6965.o \
		$(FREERTOS_SRC)/portable/GCC/ARM_CM3/*-stm32f103.o \
		$(FREERTOS_SRC)/portable/MemMang/*-lm3s6965.o \
		$(FREERTOS_SRC)/portable/MemMang/*-stm32f103.o
	@echo "清理完成！"

# ========== 声明伪目标（不对应实际文件） ==========
.PHONY: help all-lm3s6965 all-stm32f103 run-lm3s6965 run-stm32f103 \
        debug-lm3s6965 debug-stm32f103 clean
