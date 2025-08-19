//Libraries for motor, screen, and calculations
#include <Servo.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <math.h>

// Pin definitions and objects initialization

//Ultrasonic HC-SRO4
const int trigPin = 10;
const int echoPin = 11;

//Servomotor micro sg90
const int servoPin = 12;
Servo myServo;

//Nokia LCD 5110 PCD8544 display
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

//Trail lines configurations
int prevPixelX = 0;
int prevPixelY = 0;
#define MAX_TRAIL_LINES 100  //notice the max is just backup, the trailCount is resets for each half rotation (see line 55 & 60)
int trailX[MAX_TRAIL_LINES]; // Array to store X positions of trail lines
int trailY[MAX_TRAIL_LINES]; // Array to store Y positions of trail lines
int trailCount = 0;

// Constants for screen pixels
const int screenWidth = 83;
const int screenHeight = 47;


void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  myServo.attach(servoPin);
  display.clearDisplay();
  display.begin();
}

void loop() {
  //Half rotation anti-clockwise 
  trailCount = 0; //Clear the trail for each scan
  for (int angle = 0; angle <= 180; angle++) {
    scanEnvironment(angle);
  }
//Half rotation clocwise
  trailCount = 0;
  for (int angle = 180; angle >= 0; angle--) {
    scanEnvironment(angle);
  }
}

// This function gets for each itteration of the loop diffrenant angle to set the motor and preform scan + display line
void scanEnvironment(int angle) {
  myServo.write(angle);
  delay(30); //To allow smooth rotation

//To allow smooth display, doing the calc not for each degree but in steps of 5, hance the if check
if(angle%5==0){
  
  int distance = calculateDistance();
  float angleRad = angle * (M_PI / 180.0); //convert deg to rad

 //converting distance and angle to X and Y elements
  float x = distance * (cos(angleRad));
  float y = distance * (sin(angleRad));

  // limiting the distance to 1.5 meter 
  const float maxDistance = 150.0;
  if(y > maxDistance){
    y = maxDistance;
  }
  if (x > maxDistance){
    x = maxDistance;
  }

  //Converting the x,y point in cm to pixeleted point (the point later use in displayData to draw a line to it)
  int pixelX = (screenWidth/2.0) + round(x * ((screenWidth/2.0)/maxDistance)); //staring in the middle of the screen, adding rounded distance x in cm convered to pixels number
  int pixelY = screenHeight - round(y * (screenHeight/maxDistance)); //starting at the bottom of the screen, subtracting rounded distance y in cm convered to pixels number

  printSerialData(angle, distance, pixelX, pixelY);
  displayData(distance, pixelX, pixelY);
  }
}

// Function to calculate distance using the ultrasonic sensor
int calculateDistance() {
  // Send a short LOW pulse to ensure a clean pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a HIGH pulse for 10 microseconds to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the pulse sent by the sensor
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance based on the duration of the pulse
  // Speed of sound is approximately 0.0344 cm per microsecond, and we need to halve the distance
  int distance = round(duration * 0.0344 / 2);

  // Return the calculated distance
  return distance;
}

//Serial prints, helpful for debug
void printSerialData(int angle, int distance, int pixelX, int pixelY) {
  Serial.print(angle);
  Serial.print(",");
  Serial.print(distance);
  Serial.print(".\n");
  Serial.print(pixelX);
  Serial.print(",");
  Serial.print(pixelY);
  Serial.print(".\n");
}

void displayData(int distance, int pixelX, int pixelY) {

  //Configuration 
  display.setContrast(57);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.clearDisplay();

  //Draw base line and current distance measured
  display.drawLine(0, 47, 83, 47, BLACK);
  display.println(distance);

  //Draw new line
  display.drawLine(pixelX, (pixelY), 41, 47, BLACK);

  //Draw trail lines
  for (int i = 0; i < trailCount; i++) {
    display.drawLine((trailX[i]), (trailY[i]), 41, 47, BLACK);
  }

  // Store the current pixel coordinates in the trail arrays
  trailX[trailCount] = pixelX;
  trailY[trailCount] = pixelY;
  
  // Increment trail count and checks max
  trailCount++;
  if (trailCount >= MAX_TRAIL_LINES) {
    trailCount = 0;
  }

  // Update the physical display with the buffered graphics
  display.display();

}
