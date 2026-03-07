FORTE is a mid powered rocket with an estimated flight alitude of 137m initially, and a planned 1000m flight on a high powered motor later. 
It features a miniature onboard flight computer to verify data logging and live telemetry capabilties. 
The Flight computer has an inertial measurement unit and barometer to sense it's height and orientation. A LoRa Chip is placed onboard to stream data over 915 mHz to a ground station. 

The PCB is mainly for telemetry and datalogging, running custom firmware to achieve this. To get the PCB, you can send the gerber files to your manufacturer of choice, and buy components from online services such as LCSC, digikey or mouser. The rocket kit is available from LOC Precision and costs about $90.

This Rocket is my next step in my rocketry journey, allowing me to explore telemetry onboard rockets, efficient datalogging and building up to something more complex such as active control.

Render of Final PCB
<img width="584" height="678" alt="Screenshot 2026-03-08 at 9 48 38 am" src="https://github.com/user-attachments/assets/b15c7bcb-8a0b-4ab8-b4f9-5d3b0472f02e" />

Model of Rocket
<img width="943" height="248" alt="Screenshot 2026-03-08 at 9 41 17 am" src="https://github.com/user-attachments/assets/6fec367b-423c-4a66-864c-a905bbb93628" />

Avionics Bay CAD
<img width="337" height="450" alt="Screenshot 2026-03-08 at 9 33 35 am" src="https://github.com/user-attachments/assets/ea96758a-54da-45c3-9228-cb077fd5ed29" />

## Bill of Materials (BOM)

| Designator | Part Name | Qty | Value / Part Number | Link |
|---|---|---|---|---|
| C14, C15, C16, C17, C18, C27, C28, C29, C30, C31, C35, C36 | Capacitor | 12 | 100 nF 0603 | https://www.digikey.com/en/products/detail/murata-electronics/GRM188R71C104KA01D |
| C13, C20, C21 | Capacitor | 3 | 1 µF 0603 | https://www.digikey.com/en/products/detail/murata-electronics/GRM188R61A105KA61D |
| C24, C25 | Capacitor | 2 | 2.2 µF 0603 | https://www.digikey.com/en/products/detail/murata-electronics/GRM188R61A225KE15 |
| C22, C23 | Capacitor | 2 | 10 pF 0603 | https://www.digikey.com/en/products/detail/murata-electronics/GRM1885C1H100JA01D |
| C34 | Capacitor | 1 | 22 µF 0603 | https://www.digikey.com/en/products/detail/murata-electronics/GRM188R60J226MEA0 |
| C26, C38, C39 | Capacitor | 3 | 100 nF 0603 | https://www.digikey.com/en/products/detail/murata-electronics/GRM188R71C104KA01D |
| R2, R3, R4, R8, R9, R10, R11, R12, R13, R14, R15, R18 | Resistor | 12 | 10 kΩ 0603 | https://www.digikey.com/en/products/detail/yageo/RC0603FR-0710KL |
| R16, R17 | Resistor | 2 | 0603 220Ω | https://www.digikey.com/en/products/detail/yageo/RC0603FR |
| U3 | Microcontroller | 1 | STM32F405RGT6 | https://www.digikey.com/en/products/detail/stmicroelectronics/STM32F405RGT6 |
| IC1 | SPI Flash Memory | 1 | W25Q128JVFIQ | https://www.digikey.com/en/products/detail/winbond-electronics/W25Q128JVFIQ |
| U2 | Barometric Pressure Sensor | 1 | MS5611-01BA | https://www.digikey.com/en/products/detail/te-connectivity/MS5611-01BA |
| U4 | GNSS Receiver | 1 | u-blox MAX-M10S | https://www.digikey.com/en/products/detail/u-blox/MAX-M10S-00B |
| U5 | USB ESD Protection | 1 | USBLC6-2SC6 | https://www.digikey.com/en/products/detail/stmicroelectronics/USBLC6-2SC6 |
| U13 | Buck Converter | 1 | TPS54525PWPR | https://www.digikey.com/en/products/detail/texas-instruments/TPS54525PWPR |
| Y2 | Crystal | 1 | 16 MHz SMD | https://www.digikey.com/en/products/detail/ecs-inc/ECS-160-20-33Q |
| J2 | RF Connector | 1 | U.FL-R-SMT-01 | https://www.digikey.com/en/products/detail/hirose-electric/U-FL-R-SMT-01 |
| J4 | USB-C Connector | 1 | USB4105-GF-A | https://www.digikey.com/en/products/detail/gct/USB4105-GF-A |
| TP1, TP2 | Test Point | 2 | PCB Test Pad | https://www.digikey.com/en/products/detail/keystone-electronics/5015 |
| SW2 | Slide Switch | 1 | SPDT (PCM12 style) | https://www.digikey.com/en/products/detail/c-k/PCM12SMTR |
