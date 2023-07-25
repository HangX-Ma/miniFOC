# miniFOC

This repository records my development of miniFOC. I have an immature design and those ideas haven't been verified.

So _**DON'T USE ANY MATERIALS DIRECTLY!**_ I won't response for any mistakes.

> I will try to use sensor-less control strategy if the fundamental functions all perform well.

<div align="center">
    <img src="assets/miniFOC.svg" alt="miniFOC design(concept), HangX-Ma" width=600 />
    <br>
    <font size="2" color="#999"><u>miniFOC design(concept), HangX-Ma</u></font>
</div>

<div align="center">
    <img src="assets/cubemx-settings.png" alt="Current STM32CubeMX settings, HangX-Ma" width=400 />
    <br>
    <font size="2" color="#999"><u>Current STM32CubeMX settings, HangX-Ma</u></font>
</div>

## Problem Found

- SPI output pins sequence needs to be `CS`, `SCLK`, `MISO`, `MOSI`.
- Magnetic attached to motor will affect the motor itself, my friend suggests me to use sensorless control strategy.

## TODO

- [x] Add Basic _SVPWM_ control algorithm.
- [ ] Design OLED display library. Prepare to take a reference to [WouoUI](https://github.com/RQNG/WouoUI).
- [x] Use UART to debug and control motor. [[Vofa+]](https://www.vofa.plus/downloads/?v=7/17/2023)

## Development Logs

### 2023-07-24

- SC60228DC magnetic encoder works well. I brought _5 mm x 2 mm (d x h)_ magnetic but can not stick it to the motor bearing using hot-melt adhesive. I will use AB glue instead.
    > Waiting for essential materials again~

### 2023-07-23

- Add SVPWM control module and change PWM to center aligned mode.
- <span style="color: #98C379; font-weight: bold;">[Pass TEST2]:</span> SVPWM control test passes.

### 2023-07-22

- <span style="color: #98C379; font-weight: bold;">[Pass TEST1]:</span> Direct given PWM can successfully drive BLDC motor.
- Fix transmission error of SPI2, which is linked to magnetic encoder. But I forget to buy the magnetic, so fully check hasn't been done.

### 2023-07-21

- Add [u8g2](https://github.com/olikraus/u8g2) OLED library and add OLED demo code.
    >
    > - :x: OLED can not display normally, which needs further checking.
    > - :heavy_check_mark: Remove SPI1 DMA and the SPI OLED can display normally using u8g2.

- Reconstruct PWMx configurations and merges it into BLDC configurations.
- Add BLDC motor tests for checking different BLDC motor working conditions.
- Add USART control to start or stop BLDC motor.
- Driver board has been prepared.

### 2023-07-19

- Add [Qfplib-M3](https://www.quinapalus.com/qfplib-m3.html) float-point library and it works well!
    > I use cortex-debug plugin and it shows that Qfplib-M3 is visibly faster than software float-point calculation.
- Add Vofa+ communication protocol and configure USART1.
    >
    > - :question: Cough... It seems that my CH340 serial communication device breaks down. Need further checking.
    > - :heavy_check_mark: I use logic analyzer to capture output of _**PB6(Tx)**_ and it actually gives me different waveforms, which means that the CH340 is actually broken.

### 2023-07-18

- Change SPI mode to **(Receive Only Master)** and finish the SC60228DC software logic.
    > Use OOP concept to write SC60228DC, use method in `g_encoder` can satisfy the user's need.

### 2023-07-17

- :book: Create a mind map about _miniFOC_ design.
- :rocket: Finish the board design but haven't check the details. Prepare for further checking and BOM selections.
    > :heavy_check_mark: BOM and board have been checked. Waiting for all materials to arrive.
- Remove `TIM1_CH1N`, `TIM1_CH2N`, `TIM1_CH3N` settings， because they are not used in this driver.

### 2023-07-15

- :x: I tried to use proteus 8.15 to develop the software using online simulation, but the NMOS in proteus performed abnormally. I will use **Logic Analyzer** instead and design the modular board.

## License

Apache License Version 2
