# dsPIC-MotorVibrationAnalyzer
Complete hardware and bare-metal firmware for real-time industrial motor vibration diagnostics using the dsPIC33F DSP engine.

Industrial motors vibrate in specific ways before they break down. This project acts as an early warning system. It is a custom-built embedded device that listens to motor vibrations, mathematically analyzes the frequencies in real-time, and points out mechanical issues before a failure happens.

<img width="1724" height="938" alt="Vibration_dsPIC33" src="https://github.com/user-attachments/assets/83ec5619-0517-47d3-a38c-295055ac3dff" />

**Used ICs:** dsPIC33FJ64MC204 (MCU), ADXL335 (Analog Accelerometer), MAX3232 (UART-RS232 Bridge), NCP716 & TLF50241ELV50 (Voltage Regulators).

This project does not use an operating system or HAL libraries. It is entirely written in bare-metal C.

To save energy, the system defaults to a deep hardware sleep mode. The operational workflow is executed as follows:

1. The user presses the physical **START BUTTON**. The processor wakes up and activates the status LED.
2. 10-second timer is started. During this time, ADC captures 4,000 samples of the vibration every single second.
3. To move data, DMA controller is used. The DMA continuously streams incoming data into a primary memory.
4. Once the first memory array is full, the DMA redirects data to a secondary array (Ping-Pong buffering). While the secondary array fills, the main processor executes FFT on the first one. Working in parallel ensures the system never misses a sample.
5. After 10 seconds, the device stops sampling. It averages the magnitudes and transmits a list of frequencies to a terminal via the RS232 port before returning to sleep.


This repository contains:
* **Firmware (`.c` & `.h` files):** Full bare-metal C source code, including register-level configurations for DMA, dual-channel ADC, Timers, UART, and the Complex FFT algorithms.
* **Schematic PDF:** The complete circuit diagram.
* **Gerber Files:** Production-ready PCB manufacturing files.
