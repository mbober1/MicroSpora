# ZephyrBLDC

ZephyrBLDC is a Zephyr RTOS-based controller for small, high-performance BLDC and PMSM motors. The project prioritizes precise, high-quality control, automatic commissioning, and a broad set of motor-control and diagnostic capabilities.

## Features
- Field-oriented control (FOC)
- Automatic motor calibration
- Real-time diagnostics and monitoring
- Position and speed control

## Build targets
- [Native Simulator](https://docs.zephyrproject.org/latest/boards/native/native_sim/doc/index.html)
- [WeAct STM32G431 Core Board](https://docs.zephyrproject.org/latest/boards/weact/stm32g431_core/doc/index.html) (devboard for evaluation)
- [MicroSpora](https://oshwlab.com/rambros/nano-8316-motor-driver)

## Setup environment
```
west init -l app
west update
```

After setting up the workspace you can build the firmware using the appropriate VSCode task.