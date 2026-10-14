# 第一阶段实施与验证报告

**日期：** 2026-10-14
**范围：** 软件框架、主机逻辑测试、C51双时钟构建、硬件测量审查页及报告素材。
**硬件状态：** 主板待焊接；没有实物上电或动作记录。

## 已交付文件

- `firmware/`：Keil C51兼容固件模块，包括Timer、UART、无GPIO写入的DryRun电机HAL、单字节协议、固定接收队列、安全状态、运动控制和非阻塞动作序列。
- `tests/test_core.c`、`tests/test-host.ps1`：用MSVC编译与运行共享固件核心逻辑的主机测试。
- `tools/build-keil.ps1`：调用Keil C51 9.59的C51、BL51和OH51编译器、链接器及HEX转换器；DryRun目标分别输出HEX、MAP和日志，Hardware目标验证板级安全门槛及实际电机HAL未完成并安全阻止。
- `tools/send-command.py`：PC单字节协议模拟器，默认只在终端显示命令；选择真实串口需要显式提供端口参数。
- `VoiceSmartCar.uvproj`：µVision C51工程文件，采用通用8052器件视图（8XC52/NXP），配置11.0592MHz和8KB ROM边界作保守编译设置；目标MCU准确尾缀仍以实物为准。
- `hardware/VoiceSmartCar.kicad_sch`、`.kicad_pro`：可编辑文字化连接审查页及KiCad项目；页上标准器件脚定义和待测事项清楚分开，没有任何推测的MCU GPIO网络。
- `hardware/VoiceSmartCar-block-diagram.pdf`、`VoiceSmartCar-review.net`、`VoiceSmartCar-erc.rpt`：KiCad导出文件。netlist为空，正反映审查页不包含虚构的电气连接。
- `hardware/PINOUT_TRACE.csv`、`docs/HARDWARE_AUDIT.md`：按测点记录的断电测量、电源审查、MAX232与语音UART隔离步骤。
- `docs/REQUIREMENTS_TRACE.md`：老师任务书条款与硬件、软件、测试和验收证据对应矩阵。
- `docs/KEIL_BUILD_AND_FLASH.md`：Keil C51构建与STC-ISP操作步骤。
- `docs/REPORT_TECHNICAL_DRAFT.md`：课程设计报告正文技术草稿，说明已实现与计划内容，留出实物数据实验表。
- `docs/DEV_LOG.md`、`TODO.md`、`README.md`：阶段日志、未完成事项和入口说明。

## 已实现的软件行为

UART应用层字节表为A1前进、A2后退、A3停车、A4加速、A5减速、A6左转、A7右转、B1动作一、B2动作二。状态机使用固定长度16字节队列，非法指令被丢弃。停止命令由接收中断置位停车锁存并清空旧命令；缓存溢出或接收错误进入停车故障，清理后才继续接受运动命令。运动命令超时为3秒，主循环停滞100ms也触发软件停车。软件PWM是32级，占空初值16/32，允许8/32至28/32；方向变换以100ms停车过渡。B1为前进1000ms、左转500ms、停止；B2为后退700ms、右转500ms、停止。参数均为可校准初值。

因PCB连线还未测量，主板电机GPIO、L293D使能极性和电机左右轮映射没有写入配置。当前DryRun电机HAL不写任何GPIO，Hardware目标构建需要端口映射、合规逻辑电源及复位期间驱动关闭三项确认，并需实现使用实测信息的电机HAL。此架构表示尚未达到可实际驱动电机的固件状态。

## 实际验证结果

| 验证 | 实际执行命令 | 结果 | 证据与范围 |
|---|---|---|---|
| 主机逻辑测试 | `.\tests\test-host.ps1` | PASS，82 assertions / 0 failures | `build/host-test.log`；只证明MSVC虚拟时间环境中共享C逻辑行为 |
| 12T C51构建 | `.\tools\build-keil.ps1 -TargetName DryRun_12T -Rebuild` | PASS，编译、链接、HEX转换0错误 | `build/DryRun_12T/`；MAP代码1896字节、data 82字节、xdata 0字节；17条unused segment警告 |
| 6T C51构建 | `.\tools\build-keil.ps1 -TargetName DryRun_6T -Rebuild` | PASS，编译、链接、HEX转换0错误 | `build/DryRun_6T/`；MAP代码1896字节、data 82字节、xdata 0字节；17条unused segment警告 |
| Hardware安全门槛 | `.\tools\build-keil.ps1 -TargetName Hardware -Rebuild` | EXPECTED BLOCK | `build/Hardware/hardware-guard.log`包含GPIO、电源、复位安全门槛和电机HAL未实现信息；不是DryRun失败 |
| KiCad PDF / netlist / ERC | `.\tools\check-hardware.ps1` | PASS，ERC 0 errors、0 warnings | KiCad 10.0.4；仅文件解析，图纸无器件和网络，空网表不能验证PCB |

本阶段的确定性测试与Keil构建不是Real Agent Eval，也不是硬件验收。

## 未测试或不可用

- `NOT TESTED`：焊接、稳定限流逻辑供电、RESET/RST与晶振实测、STC-ISP下载、串口收发波形、SU-03T识别、软件PWM波形、电机启动压降、电机方向、急停物理响应、两组动作和整车运行。
- `UNKNOWN`：STC89C52具体尾缀、芯片实际6T/12T模式、AA电芯种类、电池开路/带载电压、板上稳压器和电源路径、电机铭牌与堵转电流、MAX232至RXD实际连接、L293D控制GPIO及插座映射、USB下载线电平。
- `UNAVAILABLE`：原理图/PCB CAD源文件、Gerber和板级DRC；当前提供材料只有图片，因此不能自动交叉核对PCB铜线。

## 下一步实物工作

依次完成芯片完整丝印和下载线识别、断电测量表H01–H12、限流电源验证、MCU最小系统下载、串口回显、架空车轮逐路测试、语音模块版本及电平确认、动作计时校准。每一步记录仪表、设置、阻值/电压/电流、照片和结果，并据证据修订 `PINOUT_TRACE.csv`、`board_config.h`、需求矩阵与报告；在该证据形成前保持所有硬件功能未验收状态。
