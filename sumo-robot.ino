#include <Servo.h>

// L298N დრაივერის კონფიგურაცია
const int in1 = 7;  // მარცხენა ძრავის პირველი ფეხი
const int in2 = 8;  // მარცხენა ძრავის მეორე ფეხი
const int in3 = 9;  // მარჯვენა ძრავის პირველი ფეხი
const int in4 = 10; // მარჯვენა ძრავის მეორე ფეხი

// IR სენსორების პინები
const int IR_PIN1 = A0;  // ყველაზე მარცხენა სენსორი
const int IR_PIN2 = A1;  // მარცხენა ცენტრალური სენსორი
const int IR_PIN3 = A2;  // მარჯვენა ცენტრალური სენსორი
const int IR_PIN4 = A3;  // ყველაზე მარჯვენა სენსორი

// ულტრასონიკური სენსორის პინები
const int trigPin = 11; // ტრიგერის პინი
const int echoPin = 12; // ექოს პინი

// სერვო ძრავის პინი
const int servoPin = 13;

// პარამეტრები
const int threshold = 500;      // IR სენსორების ზღვრული მნიშვნელობა (რეგულირდება ზედაპირის მიხედვით)
const int motorSpeed = 200;     // ძრავების სიჩქარე (0-255)
const int printDelay = 100;     // დიაგნოსტიკისთვის დაყოვნება
const int detectionDistance = 30; // მანძილი სანტიმეტრებში, როდესაც რობოტი რეაგირებს
const int scanDelay = 15;       // სკანირებისთვის დაყოვნება

Servo myServo;
int servoAngle = 90;  // საწყისი პოზიცია (პირდაპირ)
int scanDirection = 1; // 1 მარჯვნივ, -1 მარცხნივ

void setup() {
  Serial.begin(9600);
  
  // ძრავების პინების კონფიგურაცია
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // IR სენსორების პინები
  pinMode(IR_PIN1, INPUT);
  pinMode(IR_PIN2, INPUT);
  pinMode(IR_PIN3, INPUT);
  pinMode(IR_PIN4, INPUT);
  
  // ულტრასონიკური სენსორის პინები
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // სერვო ძრავის ინიციალიზაცია
  myServo.attach(servoPin);
  myServo.write(servoAngle);
  
  stopMotors();
}

void loop() {
  // სკანირების მიმართულების განახლება
  servoAngle += scanDirection * 5;
  
  // სერვო ძრავის პოზიციის შეზღუდვა
  if (servoAngle >= 180) {
    servoAngle = 180;
    scanDirection = -1;
  } else if (servoAngle <= 0) {
    servoAngle = 0;
    scanDirection = 1;
  }
  
  myServo.write(servoAngle);
  
  // მანძილის გაზომვა მიმდინარე მიმართულებით
  long distance = measureDistance();
  
  // თუ მოწინააღმდეგე აღმოჩენილია მოცემულ მანძილზე
  if (distance > 0 && distance < detectionDistance) {
    // მივუძღვნით მოწინააღმდეგეს
    approachTarget(servoAngle);
    delay(100); // მცირე დაყოვნება მოძრაობისთვის
    return; // არ ვამოწმებთ ხაზის სენსორებს ამ იტერაციაზე
  }

  // IR სენსორების წაკითხვა (შებრუნებული ლოგიკა შავი არენისთვის თეთრი საზღვრით)
  int s1 = analogRead(IR_PIN1) > threshold ? 1 : 0;  // 1 თეთრი (საზღვარი), 0 შავი
  int s2 = analogRead(IR_PIN2) > threshold ? 1 : 0;
  int s3 = analogRead(IR_PIN3) > threshold ? 1 : 0;
  int s4 = analogRead(IR_PIN4) > threshold ? 1 : 0;

  if (s1 || s2) {    // თეთრი საზღვარი მარცხნივ - მარჯვნივ მობრუნება
    turnRight();
  }
  else if (s3 || s4) { // თეთრი საზღვარი მარჯვნივ - მარცხნივ მობრუნება
    turnLeft();
  } 
  else {              // საზღვარი არ ჩანს - წინ სვლა
    moveForward();
  }

  // დიაგნოსტიკა
  Serial.print("IR: ");
  Serial.print(s1); Serial.print(",");
  Serial.print(s2); Serial.print(",");
  Serial.print(s3); Serial.print(",");
  Serial.print(s4);
  Serial.print(" | კუთხე: ");
  Serial.print(servoAngle);
  Serial.print("° | მანძილი: ");
  Serial.print(distance);
  Serial.println(" სმ");

  delay(scanDelay);
}

long measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

// ===== ძრავების კონტროლის ფუნქციები =====
void moveForward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void turnRight() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void turnLeft() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void approachTarget(int angle) {
  // მიმართულების განსაზღვრა სერვოს მიმდინარე კუთხის მიხედვით
  if (angle < 80) {
    // მოწინააღმდეგე მარცხნივაა - მარცხნივ მობრუნება
    turnLeft();
    Serial.println("მოწინააღმდეგე მარცხნივ! მობრუნება...");
  } else if (angle > 100) {
    // მოწინააღმდეგე მარჯვნივაა - მარჯვნივ მობრუნება
    turnRight();
    Serial.println("მოწინააღმდეგე მარჯვნივ! მობრუნება...");
  } else {
    // მოწინააღმდეგე წინაა - წინ სვლა
    moveForward();
    Serial.println("მოწინააღმდეგე წინ! წინსვლა...");
  }
}