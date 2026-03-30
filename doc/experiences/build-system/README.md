# 编译构建系统经验

此目录存放所有与编译构建相关的经验：
- Makefile经验
- CMake配置
- 链接脚本问题
- 编译错误排查
- 优化选项经验

---

## 多平台 Makefile 设计经验 ⭐

### 设计思路

在为多个目标平台（如LM3S6965和STM32F103）编写Makefile时，我们采用了以下设计原则：

#### 1. 分离共享配置和平台特定配置

**核心思想**：将通用的编译选项、工具链配置与平台特定的配置分离。

```makefile
# 共享配置
CFLAGS_BASE = -Wall -Wextra -mcpu=cortex-m3 -mthumb -Os -g
LDFLAGS_BASE = -mcpu=cortex-m3 -mthumb -Wl,--gc-sections

# 平台特定配置（LM3S6965）
CFLAGS_LM3S6965 = $(CFLAGS_BASE)
LDFLAGS_LM3S6965 = $(LDFLAGS_BASE) -TLM3S6965.ld

# 平台特定配置（STM32F103）
CFLAGS_STM32F103 = $(CFLAGS_BASE) -include FreeRTOSConfig_STM32F103.h
LDFLAGS_STM32F103 = $(LDFLAGS_BASE) -TSTM32F103RB.ld
```

**优点**：
- 减少重复代码
- 便于维护和更新通用选项
- 平台差异清晰可见

#### 2. 使用目标前缀区分中间文件

**问题**：不同平台编译相同的源文件会产生同名的 `.o` 文件，导致冲突。

**解决方案**：为不同平台的目标文件添加后缀。

```makefile
# LM3S6965的FreeRTOS目标文件
FREERTOS_OBJS_LM3S6965 = \
    $(FREERTOS_SRC)/list-lm3s6965.o \
    $(FREERTOS_SRC)/queue-lm3s6965.o \
    $(FREERTOS_SRC)/tasks-lm3s6965.o

# STM32F103的FreeRTOS目标文件
FREERTOS_OBJS_STM32F103 = \
    $(FREERTOS_SRC)/list-stm32f103.o \
    $(FREERTOS_SRC)/queue-stm32f103.o \
    $(FREERTOS_SRC)/tasks-stm32f103.o

# 模式规则处理带后缀的目标文件
$(FREERTOS_SRC)/%-lm3s6965.o: $(FREERTOS_SRC)/%.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS_LM3S6965) -c $< -o $@

$(FREERTOS_SRC)/%-stm32f103.o: $(FREERTOS_SRC)/%.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS_STM32F103) -c $< -o $@
```

**优点**：
- 避免了文件冲突
- 可以并行编译多个平台
- 清晰区分不同平台的编译产物

#### 3. 提供独立的目标名称

**设计**：每个平台有自己独立的目标名称，格式为 `动作-平台`。

```makefile
# 默认目标 - 显示帮助
help:
	@echo "可用的编译目标:"
	@echo "  make all-lm3s6965   - 编译 LM3S6965 版本"
	@echo "  make all-stm32f103  - 编译 STM32F103 版本"
	@echo "  make run-lm3s6965   - 运行 LM3S6965 QEMU"
	@echo "  make run-stm32f103  - 运行 STM32F103 QEMU"
	@echo "  make clean           - 清理所有编译产物"

# 平台目标
all-lm3s6965: $(ELF_LM3S6965) $(BIN_LM3S6965) $(HEX_LM3S6965)
all-stm32f103: $(ELF_STM32F103) $(BIN_STM32F103) $(HEX_STM32F103)

run-lm3s6965: all-lm3s6965
	qemu-system-arm -machine lm3s6965evb -kernel $(ELF_LM3S6965) ...

run-stm32f103: all-stm32f103
	qemu-system-arm -machine lm3s6965evb -kernel $(ELF_STM32F103) ...
```

**优点**：
- 用户明确知道自己在编译哪个平台
- 不会因为默认目标选错平台
- 便于自动化和脚本调用

#### 4. 统一的清理目标

**设计**：一个 `clean` 目标清理所有平台的编译产物。

```makefile
clean:
	@rm -f \
		$(ELF_LM3S6965) $(BIN_LM3S6965) $(HEX_LM3S6965) \
		$(ELF_STM32F103) $(BIN_STM32F103) $(HEX_STM32F103) \
		startup_lm3s6965.o startup_stm32f103.o \
		src/main_lm3s6965.o src/main_stm32f103.o \
		$(FREERTOS_SRC)/*-lm3s6965.o $(FREERTOS_SRC)/*-stm32f103.o \
		...
	@echo "Clean done."
```

### 完整的文件命名规范

为了配合多平台Makefile，项目文件也应该遵循清晰的命名规范：

```
00-env-verify/
├── Makefile                    # 编译脚本（支持多平台）
├── LM3S6965.ld                 # LM3S6965链接脚本
├── STM32F103RB.ld              # STM32F103链接脚本
├── startup_lm3s6965.c          # LM3S6965启动文件
├── startup_stm32f103rb.s       # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig_LM3S6965.h  # LM3S6965 FreeRTOS配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103 FreeRTOS配置
└── src/
    ├── main_lm3s6965.c         # LM3S6965主程序
    └── main_stm32f103.c        # STM32F103主程序
```

### 经验总结

1. **分离关注点**：共享配置和平台配置分开
2. **避免冲突**：使用目标后缀区分不同平台的编译产物
3. **明确目标**：提供清晰的目标名称和帮助信息
4. **统一清理**：一个clean目标处理所有平台
5. **文件命名**：项目文件也应遵循平台后缀的命名规范

### 扩展建议

如果将来需要添加更多平台，可以考虑：

1. **使用变量循环**：对于大量平台，可以考虑使用Make的循环功能
2. **配置文件分离**：将每个平台的配置分离到单独的 `.mk` 文件
3. **CMake过渡**：对于更复杂的项目，考虑迁移到CMake

### 相关文档

- [环境验证项目Makefile](../../projects/stm32-learning/01-getting-started/00-env-verify/Makefile)
- [环境验证项目README](../../projects/stm32-learning/01-getting-started/00-env-verify/README.md)
