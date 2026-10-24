#include <Wire.h>
#include <Adafruit_TCS34725.h>

// Motor and sensor setup
#define MAX_SPEED 170
#define WHEEL_DIAMETER 8.0
#define STOP_DISTANCE 20 // Distance to stop (cm)

// Pin definitions
int E1 = 5, M1 = 4;  // Motor 1 (Left)
int E2 = 6, M2 = 7;  // Motor 2 (Right)
int buttonPin = 2;   // Button to start/stop program
int LEDR = 9;        // Red LED (Right Turn)
int LEDL = 8;        // Green LED (Left Turn)
int trigPin = 10;    // Ultrasonic trigger pin
int echoPin = 11;    // Ultrasonic echo pin

// Global variables
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);  // Color sensor
float scaleR = 1, scaleG = 1, scaleB = 1;  // Calibration scales for the color sensor
boolean buttonState = false, lastButtonState = false;
boolean isRunning = false;  // Is the robot running or stopped
int distanceCount = 0;
int DISTANCE_TARGET = 0;
int state = 0;  // State machine step
boolean firstButtonPress = true;  // Flag for the first button press

void setup() {
  Serial.begin(9600);
  pinMode(E1, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(E2, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDL, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize the color sensor
  if (!tcs.begin()) {
    Serial.println("No TCS34725 found. Check wiring!");
    while (1);
  }
  stopMotors();
  Serial.println("Setup complete.");
}

void loop() {
  // Read button state to control the robot
  buttonState = digitalRead(buttonPin);

  // If the button is pressed
  if (buttonState == LOW && lastButtonState == HIGH) {
    if (firstButtonPress) {
      Serial.println("Starting White Calibration...");
      calibrateWhite();  // Perform white calibration
      flashLEDs();        // Flash LEDs once after calibration
      firstButtonPress = false;  // Disable further calibration
      isRunning = false; // Stop the robot after calibration
    } else {
      isRunning = !isRunning;  // Toggle running state
      state = 0;  // Reset state machine
      distanceCount = 0;
      Serial.println(isRunning ? "Program Started" : "Stopped");
    }
  }

  lastButtonState = buttonState;

  // If the robot is running
  if (isRunning) {
    int distance = getDistance();
    if (distance < STOP_DISTANCE) {
      stopMotors();
      flashLEDs();  // Flash LEDs if an obstacle is detected
      isRunning = false;  // Stop the program
    } else {
      executeStateMachine();  // Run the state machine for movements
    }

    // Check for color detection
    String detectedColor = detectColor();
    if (detectedColor == "RED") {
      stopMotors();  // Stop the car when red is detected
      flashLEDs();  // Flash both LEDs 4 times
      isRunning = false;  // Stop the program after detecting red
    }
  }

  delay(10);  // Small delay for responsiveness
}

// Measure distance using ultrasonic sensor
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.0344 / 2;  // Convert time to distance
  return distance;
}

// Flash LEDs a specified number of times
void flashLEDs() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(LEDR, HIGH);  // Turn on Red LED
    digitalWrite(LEDL, HIGH);  // Turn on Green LED
    delay(300);                // Wait for 300ms
    digitalWrite(LEDR, LOW);   // Turn off Red LED
    digitalWrite(LEDL, LOW);   // Turn off Green LED
    delay(300);                // Wait for 300ms
  }
}

// Detect color using the color sensor
String detectColor() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  // Apply calibration factors
  float R_cal = r * scaleR;
  float G_cal = g * scaleG;
  float B_cal = b * scaleB;

  // Calculate the sum and ratios
  float sum = R_cal + G_cal + B_cal;
  if (sum < 1.0) sum = 1.0;  // Prevent division by zero

  float rRatio = R_cal / sum;
  float gRatio = G_cal / sum;
  float bRatio = B_cal / sum;

  // Classify the color based on ratios
  String detectedColor = "UNDETERMINED";
  if (rRatio > 0.60 && gRatio < 0.20 && bRatio < 0.25) {
    detectedColor = "RED";
  } else if (gRatio > 0.42 && rRatio < 0.27 && bRatio < 0.33) {
    detectedColor = "GREEN";
  } else if (bRatio > 0.48 && rRatio < 0.20 && gRatio < 0.34) {
    detectedColor = "BLUE";
  }

  return detectedColor;
}

// Execute state machine for robot movements
void executeStateMachine() {
  switch (state) {
    case 0: // Move forward
      DISTANCE_TARGET = 300; // Move forward for a set distance
      if (distanceCount < DISTANCE_TARGET) {
        moveForward();
        distanceCount++;
      } else {
        stopMotors();
        Serial.println("Step 1: Move Forward");
        state++;  // Move to the next step
        distanceCount = 0;
      }
      break;

    case 1: // Turn Left 90 degrees
      turnLeft();
      Serial.println("Step 2: Turn Left 90 degrees");
      state++;
      distanceCount = 0;
      break;

    case 2: // Move forward again
      DISTANCE_TARGET = 150; // Shorter forward movement
      if (distanceCount < DISTANCE_TARGET) {
        moveForward();
        distanceCount++;
      } else {
        stopMotors();
        Serial.println("Step 3: Move Forward after Left Turn");
        state++;
        distanceCount = 0;
      }
      break;

    case 3: // Turn Left 180 degrees
      turnLeft();
      turnLeft(); // Two 90° turns for 180° turn
      Serial.println("Step 4: Turn Left 180 degrees");
      state++;
      distanceCount = 0;
      break;

    case 4: // Move forward longer distance
      DISTANCE_TARGET = 300;
      if (distanceCount < DISTANCE_TARGET) {
        moveForward();
        distanceCount++;
      } else {
        stopMotors();
        Serial.println("Step 5: Move Forward Longer Distance");
        state++;
        distanceCount = 0;
      }
      break;

    case 5: // Turn Right 180 degrees
      turnRight();
      turnRight();
      Serial.println("Step 6: Turn Right 180 degrees");
      state++;
      distanceCount = 0;
      break;

    case 6: // Move forward back to the middle
      DISTANCE_TARGET = 150;
      if (distanceCount < DISTANCE_TARGET) {
        moveForward();
        distanceCount++;
      } else {
        stopMotors();
        Serial.println("Step 7: Return to Middle");
        state = 0;  // Restart the sequence
        isRunning = false;  // Stop after completion
      }
      break;

    default:
      stopMotors();
      break;
  }
}

// Move forward at the defined speed
void moveForward() {
  digitalWrite(M1, HIGH); digitalWrite(M2, HIGH);
  analogWrite(E1, MAX_SPEED); analogWrite(E2, MAX_SPEED);
}

// Turn left (90 degrees)
void turnLeft() {
  digitalWrite(M1, LOW); digitalWrite(M2, HIGH);
  analogWrite(E1, MAX_SPEED); analogWrite(E2, MAX_SPEED);
  delay(600); // Adjust delay for 90° turn
  stopMotors();
}

// Turn right (90 degrees)
void turnRight() {
  digitalWrite(M1, HIGH); digitalWrite(M2, LOW);
  analogWrite(E1, MAX_SPEED); analogWrite(E2, MAX_SPEED);
  delay(600); // Adjust delay for 90° turn
  stopMotors();
}

// Stop motors
void stopMotors() {
  analogWrite(E1, 0); analogWrite(E2, 0);
  digitalWrite(M1, LOW); digitalWrite(M2, LOW);
}

// Perform white calibration for the color sensor
void calibrateWhite() {
  uint16_t r, g, b, c;
  tcs.setInterrupt(false);
  delay(60); // Wait for fresh sensor data
  tcs.getRawData(&r, &g, &b, &c);
  tcs.setInterrupt(true);

  // Avoid division by zero
  if (r == 0) r = 1;
  if (g == 0) g = 1;
  if (b == 0) b = 1;

  // Calibration: Adjust scale factors so white maps to 255
  scaleR = 255.0 / (float)r;
  scaleG = 255.0 / (float)g;
  scaleB = 255.0 / (float)b;

  Serial.println("Calibration complete!");
  Serial.print("scaleR = "); Serial.println(scaleR);
  Serial.print("scaleG = "); Serial.println(scaleG);
  Serial.print("scaleB = "); Serial.println(scaleB);
}



