# VoiceSmartCar / 51语音控制小车

2026-10-14 第一阶段软件与硬件审查工程。工程以用户提供的《语音小车.docx》任务书文本、BOM和照片为资料来源，不替代独立签署任务书或 PCB 导通测量。

## 当前状态

- 主机端固件共用逻辑和测试：82条断言通过；结果见 `docs/STAGE1_REPORT.md`。
- Keil C51 9.59：DryRun_12T、DryRun_6T均编译、链接并导出HEX；Hardware目标因硬件门槛及电机HAL未实现而预期拒绝。
- KiCad 10.0.4：文字化连接审查页可编辑，PDF导出及ERC已执行；页上不画未知PCB网络。
- 物理样板：未焊接；未上电、未烧录、未接语音模块、未测试电机。
- IO 映射、模块实物版本与最终电源轨：待实测。`Hardware` Keil 目标在映射确认前会主动拒绝编译。

## 工程结构

- `firmware/` C51 固件，协议/状态机/动作队列为可移植 C 逻辑。
- `tests/` 使用 Microsoft C 编译器构建并运行共用逻辑的确定性主机测试。
- `tools/send-command.py` 默认只打印一条单字节指令；显式添加 `--port COMx` 才会打开串口。
- `hardware/` KiCad文字化连接审查页、PDF、空网络表、ERC报告和引脚追踪表。该审查页不是已测原理图。
- `docs/` 追踪矩阵、测量记录表、Keil操作指南、开发日志和课程报告技术草稿。
- `tools/` 主机测试及 Keil CLI 构建脚本。
- `build/` Keil 生成物和验证日志。

## 本机检查

在 PowerShell 中，从本目录运行：

推荐直接用 Keil 打开 `VoiceSmartCar.uvproj`，选择 `DryRun_12T` 或 `DryRun_6T`。在第一次打开此工程的Keil实例中，请核对Target器件显示并从串口和定时选项中检查振荡器/12T或6T配置。自动主机测试：

```powershell
.\tools\test-host.ps1
py .\tools\send-command.py A1
```

Keil CLI 构建：

```powershell
.\tools\build-keil.ps1 -TargetName DryRun_12T -Rebuild
.\tools\build-keil.ps1 -TargetName DryRun_6T -Rebuild
```

待 PCB 测量记录填完、在 `firmware/board_config.h` 明确配置并审核后，才可选择 `Hardware`。最终 HEX 上传请使用与芯片完整型号匹配的 STC-ISP；本工程不会自动打开串口或烧录。

## 安全边界

收到这套未焊接板照片的事实不足以推导端口映射或供电铜箔连接。首次硬件步骤应是拔掉电机、电池并断电测通断；按 `docs/HARDWARE_AUDIT.md` 记录后，由稳定限流 5V 电源单独验证逻辑电源。在确定 MCU VCC 安全前，不接标为 DC6V 的电池盒。
