# Keil C51 构建与STC-ISP操作记录

## 工具链及构建

已发现本机 Keil C51 9.59.0.0：`C:\Keil_v5\C51\BIN\C51.exe`、`BL51.EXE`、`OH51.EXE`。工程保留 `.uvproj` 文件供 µVision 打开；自动脚本直接调用这三个 Keil C51 工具，分别编译、链接并转换Intel HEX。当前脚本的构建目标是 C51 编译/链接配置，不包含硬件端口实现。

在工程根目录打开 PowerShell，执行：

```powershell
Set-Location '<工程目录>\VoiceSmartCar'
.\tools\build-keil.ps1 -TargetName DryRun_12T -Rebuild
.\tools\build-keil.ps1 -TargetName DryRun_6T -Rebuild
```

每个目标生成独立的 `build\<目标名>\` 文件夹，包含 `VoiceSmartCar.hex`、链接MAP `VoiceSmartCar.m51` 和完整 `build.log`。日志记录编译器诊断和链接代码/数据空间统计。链接器的“未调用函数”警告来自用于测试和查询的公开API没有在嵌入式入口中调用；当前结果为0链接错误。MAP中 `data` 只统计静态数据，不含硬件实机栈峰值测量。STC89C52不同尾缀的ROM容量必须以实际芯片为准；当前摘要代码1896字节，低于常见8KB规格，但编程前仍须核对完整型号。

也可以在µVision 5中打开 `VoiceSmartCar.uvproj`，确认选择 `DryRun_12T` 或 `DryRun_6T`，点击 Rebuild。不同版本µVision若未配置STC器件包，可将Keil内核选择理解为通用8052编译设置；本项目的自动构建不依赖µVision GUI设备数据库。不要选择Hardware目标烧录当前待测端口配置。

## 6T/12T时钟设置

两个目标都以11.0592MHz为编译时钟。12T串口Timer1重装值FDh、Timer2约每256个机器周期中断一次；6T目标分别使用FAh与512计数重装值。Timer2中断频率设计值为3600Hz，36次生成一次10ms系统节拍。Timer1用于9600bps串口波特率，Timer2提供软件PWM和系统计时。Keil宏只能让代码选择一套参数，不能改变STC芯片熔丝/ISP配置。烧录前须按完整芯片型号检查STC-ISP中的时钟模式，并以逻辑分析仪或示波器核对串口波特率与PWM频率。

## STC-ISP下载步骤（未执行）

1. 先确认板卡实际芯片完整丝印、ISP支持情况、USB下载线型号及其输出是RS-232还是TTL；不要把DB9的±RS-232电平直接接到MCU UART。
2. 断开电池、电机和语音板，先完成 `HARDWARE_AUDIT.md` 的断电检查与安全逻辑供电核验。避免把未确认的DC6V输入连接到单片机VCC。
3. 依据芯片手册准备适配该型号的STC-ISP版本、串口端口、波特率和时钟模式。第一次下载使用最小化的静态指示/串口回显固件，不接L293D电机负载。
4. 在STC-ISP中打开对应目标的HEX文件，点击下载后按软件提示给目标板上电或复位。只在端口、电平、地线和供电已确认后做此操作。本工程脚本不会枚举、打开COM口或发送数据。
5. 保存ISP软件版本、COM端口、芯片识别结果、时钟选项、HEX SHA-256、下载结果截图和供电电流。下载成功仅证明编程链路，不等于电机或语音功能验收。
6. 连接电机前先将车体架空，分通道短时测试；观察电机方向、L293D温升、供电电压跌落和电流。之后再进行完整动作测试。

## 当前状态

- C51 9.59编译、BL51链接、OH51 HEX导出：已执行，详见 `build\DryRun_12T\build.log` 和 `build\DryRun_6T\build.log`。
- Keil GUI完整工程Build：未执行；自动调用C51工具链是本阶段实际编译证据。
- HEX烧录、UART实际收发、芯片6T/12T运行模式、车轮运动：NOT TESTED。
- `Hardware`目标在缺少实测引脚/供电/复位断驱动确认时预期失败；此失败是配置安全锁，不是DryRun构建失败。
