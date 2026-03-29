# Git 开发规范

## 分支命名规范

### 主分支
- `main` - 主分支，稳定版本代码
- `develop` - 开发分支，集成所有功能开发

### 功能分支
分支命名格式：`类型/描述`

类型包括：
- `feature/` - 新功能开发
  - 示例：`feature/add-riscv-port`
  - 示例：`feature/stm32f103-demo`
- `bugfix/` - Bug修复
  - 示例：`bugfix/fix-scheduler-crash`
  - 示例：`bugfix/fix-linker-script`
- `docs/` - 文档更新
  - 示例：`docs/update-experiences`
  - 示例：`docs/add-tutorial`
- `hotfix/` - 紧急修复生产环境问题
  - 示例：`hotfix/fix-critical-security`

## 提交信息规范

提交信息要求：
1. 使用简洁中文描述
2. 一句话说明清楚修改内容
3. 不超过50个汉字
4. 开头说明修改类型，后面跟上具体描述

### 提交类型前缀
- `feat`: 新增功能
- `fix`: 修复问题
- `docs`: 更新文档
- `refactor`: 重构代码
- `perf`: 性能优化
- `test`: 添加测试
- `build`: 构建相关
- `chore`: 其他修改

### 示例
```
feat: 添加RISC-V架构支持
fix: 修复调度器在高负载下崩溃问题
docs: 更新FreeRTOS队列操作经验
refactor: 重写HAL驱动代码结构
```

## 工作流程
1. 从最新`develop`分支创建功能分支
2. 在功能分支上完成开发
3. 提交代码时遵循提交信息规范
4. 合并回`develop`分支前确保代码可编译
