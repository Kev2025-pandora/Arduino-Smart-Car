# Arduino Smart Car

## Overview

This project is an Arduino-based smart car that uses a color sensor and an ultrasonic sensor to interact with its environment. It can follow a programmed movement sequence, detect specific colors, and stop when an obstacle is too close.

The project combines hardware control, sensor data processing, and state-based programming.

## Features

- **Color Detection:** Uses the Adafruit TCS34725 color sensor to identify red, green, and blue objects.
- **White Calibration:** Calibrates the color sensor using a white surface to improve color detection.
- **Obstacle Detection:** Uses an ultrasonic sensor to measure the distance to nearby objects.
- **Automatic Stop:** Stops the motors when an obstacle is detected within the configured distance.
- **Programmed Movement:** Executes a sequence of forward movements and left and right turns.
- **Button Control:** Uses a physical button to perform initial calibration and start or stop the program.
- **LED Indicators:** Flashes red and green LEDs to indicate calibration completion or a stop event.

## Hardware Components

- Arduino-compatible microcontroller
- Two DC motors and a motor driver
- Adafruit TCS34725 color sensor
- Ultrasonic distance sensor
- Push button
- Red LED and green LED
- Robot chassis and wheels
- Jumper wires and a suitable power supply

## Pin Configuration

| Component | Arduino Pin |
|---|---|
| Left motor speed (E1) | 5 |
| Left motor direction (M1) | 4 |
| Right motor speed (E2) | 6 |
| Right motor direction (M2) | 7 |
| Start/stop button | 2 |
| Red LED | 9 |
| Green LED | 8 |
| Ultrasonic trigger | 10 |
| Ultrasonic echo | 11 |
| TCS34725 color sensor | I2C (SDA/SCL) |

## Software Requirements

- Arduino IDE
- `Wire` library
- `Adafruit TCS34725` library

Install the Adafruit TCS34725 library through the Arduino IDE Library Manager before uploading the program.

## How It Works

1. **Initialization:** The Arduino initializes the motors, LEDs, ultrasonic sensor, and color sensor.
2. **White Calibration:** The first button press calibrates the color sensor using a white surface.
3. **Start the Program:** Press the button again to start the programmed movement sequence.
4. **Movement:** The car moves forward and performs turns according to the state machine.
5. **Obstacle Detection:** The ultrasonic sensor checks the distance ahead. If an obstacle is detected within the configured stopping distance, the car stops and flashes both LEDs.
6. **Color Detection:** The color sensor continuously checks the surface or object in front of it. When red is detected, the car stops and flashes both LEDs.
7. **Stop and Restart:** The button can be used to stop the program or start it again.

## Configuration

Several settings can be adjusted at the beginning of the code:

- `MAX_SPEED`: Controls the motor speed.
- `STOP_DISTANCE`: Sets the minimum obstacle distance in centimeters.
- `DISTANCE_TARGET`: Controls the duration of forward movement in the movement sequence.
- Color detection thresholds: Adjust these values in `detectColor()` to improve recognition under different lighting conditions.
- Turning delay: Adjust the `600` millisecond delay in `turnLeft()` and `turnRight()` to tune the turning angle.

**Note:** The current movement sequence uses loop iterations rather than wheel encoders to estimate travel distance. Therefore, `DISTANCE_TARGET` does not represent a precise physical distance. Turning angles and color thresholds may require testing and adjustment.

## Future Improvements

- Add wheel encoders for more accurate distance measurement.
- Improve color recognition under different lighting conditions.
- Add non-blocking motor control to improve sensor responsiveness during turns.
- Store calibration values for reuse after restarting the Arduino.
- Add more movement patterns and sensor-based behaviors.

## License

This project is intended for educational and personal learning purposes. A license can be added to specify how others may use, modify, and distribute the source code.
