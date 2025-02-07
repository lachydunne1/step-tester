# Unit Step Testing:

This project allows for users to perform basic unit step testing via an ESP32 interface. This example does use an ADS111x ADC
and LCD display that utilise driver libraries contained within the esp-idf-lib repository.

https://github.com/UncleRus/esp-idf-lib/tree/master


## How to use example
It is encouraged users to use the example as a template for derivation of simple plant models, such as DC motors, and calibration of sensors, (such as the ACS712, 
which was the original purpose of this file). 


## Example folder contents

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
|   └── unit_step_ADS1115.c : handles initialisation of relevant peripherals and external i2c devices, as starting main tasks.
|   └── uart.c : contains init_uart and  controls switch from datalogging to standby mode over USB interface with the script in py_uart.
|   └── display.c : displays real-time data on LCD: to disable rm display_task in app main and rm Queue logging from within unit_step_task
|   └── gpio.c : initialises relevant gpios
|   └── ads_helper.c : some helper functions that can be used with the ADS111x family of ADCs. This is optional, and the on-board
|                      ADC can be used, however for accurate results it is recommended to use an external ADC.
├──py_uart:
|  └── uart.py : script for real time animation of UART data and automated logging. To use, configure UART socket on line 150.
|                 Once the socket is setup, the user can customise monitored value names, total number (up to 4) and the file they 
|                 wish to save the data to. Once the test is complete, this script monitors for Ctrl+C to close uart. 
|                 'idf.py monitor' cannot be used simulataneously as it sporadic connections on UART.
|   
└── README.md                  This is the file you are currently reading, woah.
```

