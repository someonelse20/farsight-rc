# Farsight RC
![alt text](assets/controller-3d.png)

Custom long range remote controler with ESP32 to stream telemetry and an STM32 based lora module with the SX1262. The controller has a build in 18650 battery with an integrated charger. The transceiver is its own standalone board and can be used independenly of the controller. I made this project as a part of my larger goal to make a custom drone. I originaly was going to make it out of stock modules but decided to design custom pcbs.

## Usage

__--This is still under development and is subject to change--__

### Controller

The switch in the middle of the controller is the power switch and the led is the status of the battery charging. The usb connecter will charge the battery and can be used to communicate with the ESP32. The microsd card will log all the incoming telemetry.

The controller software is currently pretty basic and missing some fetures. The wifi network the esp32 will create can be configured by installing the espressive ide and running idf.py menuconfig and changing the wifi ssid and password under Wifi Configuration. Requesting /telemetry of the ip of the nextork will return a set of comma seperated values in the format of "altitude_meters,speed_meters_per_second,heading_degrees,latitude,longitude".

### Transceiver

UART pinout:
| 1   | 2   | 3  | 4  |
|-----|-----|----|----|
| VCC | GND | RX | TX |

Microcontroller pinout:
| 1     | 2    |
|-------|------|
| RESET | BOOT |

Debug pinout:
| 1      | 2     |
|--------|-------|
| SWDCLK | SWDIO |

The transciever has an internal regulator and can handle up to 5v input. The UART pinout is from the transceiver refrence so an external device would put its Tx to the transceiver Rx and vice versa. The buttons on the board will put the STM32 into boot and reset mode respectivly and the external pins are directly wired to the microcontroller. 

The transceiver is built on the [Semtech SX1262 driver](https://github.com/Lora-net/sx126x_driver). To communicate with the transceiver a host must first send a register bit and then the data to transmit or receive based on the table below.
