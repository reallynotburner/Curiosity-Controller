# Curiosity-Controller

## Goal
###
I want to drive around a highly functional model of the Mars Science Laboratory (MSL), named [Curiosity](https://www.jpl.nasa.gov/missions/mars-science-laboratory-curiosity-rover-msl/).  It will have working rocker-bogie suspension, and 4 steerable motors so it can spin in place without skid steering like a tank.  I'm making it as small as I can using [Sanyo-style micro gearmotors](https://www.amazon.com/gp/aw/d/B07FVMVGM3/?_encoding=UTF8), which are inexpensive, strong, and fairly small.  The motors have to fit inside the wheel hubs and the steering mechanism without altering the proportions of the model too much for my taste.  Therefore the smallest I can make it is 1/6 scale, I could go smaller but it would mean more expensive smaller motors.

## Hardware
### Controller
[ESP32 Devkit](https://www.microcenter.com/product/613822/inland-esp32-wroom-32d-module) as a central controller has many input-output pins, nearly all of which can do motor controlling functions like PWM.  This is vital when it needs 10 independent motors just to drive around.  The ESP32 also has WiFi and Bluetooth built in, providing an inexpensive method of remote control.

### Motors
[Sanyo-style micro gearmotors](https://www.amazon.com/gp/aw/d/B07FVMVGM3/?_encoding=UTF8) provide the traction and steering functions of the model.  They may even provide animation of the [Mast Camera](https://science.nasa.gov/mission/msl-curiosity/science-instruments/#h-mastcam) and Robotic Arm, but that is a project more complex than just driving the model around.

### Motor Driver
[DRV8833 H-Bridges](https://www.amazon.com/VKLSVAN-DRV8833-h-Bridge-Arduino-Microcontroller/dp/B0DQGQ1V1C) provide control of 2 motors per module, so there will be 5 of these on the model for driving and steering,

### Hex Inverters
[Texas Instruments low-voltage 6 channel inverters](https://www.mouser.com/ProductDetail/Texas-Instruments/SN74LVC04APWRG3?qs=EuM%2FBx4ov4R3TYLIBa%252BMkA%3D%3D) are used to reduce the number of ESP32 pins I need to control my motor drivers.

### 10k Potentiometers
[Potentiometers](https://www.amazon.com/Taiss-Potentiometer-Variable-Resistors-Terminals/dp/B09XDR799P) are often used as volume controls.  I'm using them as a position sensor for the steerable wheels.  They convert an angle of the steering bracket into a voltage, which can be read by the ESP32, which then controls the steering motor that can move the bracket to the desired position.
