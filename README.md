# ASCENDSat

Andes' Satellite for Celestial Exploration and Network Demonstration

> Uniandes, School of Engineering 2024-10

Welcome to the ASCENDSat repository! This repository hosts the codebase for ASCENDSat, a satellite developed by the Uniandes School of Engineering for celestial exploration and network demonstration purposes.

[//]: <> (##Mission Statement)

## Codebase structure

```
|── individual-tests/
| ├── accelerometer/accelerometer.ino
| ├── barometer/barometer.ino
| ├── camera/camera.ino
| ├── gps/gps.ino
| └── micro-sd/micro-sd.ino
|
|── sensors/
| ├── sensors.ino
| └── sample_stream.txt
|
└── documentation/
```

- `individual-tests/`: This directory contains individual test codes for each sensor required by the mission. Each sensor has its subdirectory containing the test code. The sensors utilized are:
  - *accelerometer and gyroscope*: [MPU-6050](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
  - *barometer*: [BMP280](https://cdn-shop.adafruit.com/datasheets/BST-BMP280-DS001-11.pdf)
  - *gps*: [GT-U7](https://images-na.ssl-images-amazon.com/images/I/91tuvtrO2jL.pdf)
  - *camera*: MJY20FF or other references adaptable to OV5640 module
  - *micro-sd*: Integrated within the OV5640 camera module
  
- `sensors/`: Contains the [`sensors.ino`](https://github.com/t-montes/CubeSat/blob/master/sensors/sensors.ino) file, which contains the combined code for all the sensors. This file measures all variables and prints a data stream in the same format as the sample file [`sample_stream.csv`](https://github.com/t-montes/CubeSat/blob/master/sensors/sample_stream.csv).

- `documentation/`: This directory contains documentation related to the _construction_ and _usage_ of ASCENDSat. 

