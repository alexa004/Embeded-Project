#include <SPI.h>

#include <MFRC522.h>

#include <LiquidCrystal_I2C.h>

#include <Servo.h>

 

// RFID

#define SS_PIN 10

#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);

 

// LCD

LiquidCrystal_I2C lcd(0x27, 16, 2);

 

// Servo

Servo servo;

#define SERVO_PIN 3

 

// TCS3200 Color Sensor Pins

#define S0 4

#define S1 5

#define S2 6

#define S3 7

#define COLOR_OUT 8

 

// Parking Slot Setup

const int normalSlotStart = 1;

const int evSlotStart = 6;

const int taxiSlotStart = 9;

 

const int totalNormal = 5;

const int totalEV = 3;

const int totalTaxi = 2;

 

byte occupiedUIDNormal[5][4] = {{0}};

byte occupiedUIDEV[3][4] = {{0}};

byte occupiedUIDTaxi[2][4] = {{0}};

 

bool slotOccupiedNormal[5] = {false};

bool slotOccupiedEV[3] = {false};

bool slotOccupiedTaxi[2] = {false};

 

void setup() {

 Serial.begin(9600);

 SPI.begin();

 mfrc522.PCD_Init();

 

 lcd.begin(16, 2);

 lcd.setBacklight(1);

 lcd.print("Smart Parking");

 delay(2000);

 lcd.clear();

 

 servo.attach(SERVO_PIN);

 servo.write(0);

 

 pinMode(S0, OUTPUT); pinMode(S1, OUTPUT);

 pinMode(S2, OUTPUT); pinMode(S3, OUTPUT);

 pinMode(COLOR_OUT, INPUT);

 digitalWrite(S0, HIGH); digitalWrite(S1, LOW);

}

 

void loop() {

 if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

   byte* uid = mfrc522.uid.uidByte;

   Serial.print("UID: ");

   for (int i = 0; i < mfrc522.uid.size; i++) {

     Serial.print(uid[i], HEX); Serial.print(" ");

   }

   Serial.println();

 

   String vehicleType = detectColor();

   Serial.println("Detected: " + vehicleType);

 

   int assignedSlot = -1;

 

   if (vehicleType == "EV") {

     int index = findCard(uid, occupiedUIDEV, totalEV);

     if (index == -1) {

       assignedSlot = assignSlot(uid, occupiedUIDEV, slotOccupiedEV, totalEV, evSlotStart, "EV Entry");

     } else {

       clearSlot(index, occupiedUIDEV, slotOccupiedEV);

       assignedSlot = evSlotStart + index;

       showLCD("EV Exit", "Slot " + String(assignedSlot));

       openGate();

     }

   } else if (vehicleType == "Taxi") {

     int index = findCard(uid, occupiedUIDTaxi, totalTaxi);

     if (index == -1) {

       assignedSlot = assignSlot(uid, occupiedUIDTaxi, slotOccupiedTaxi, totalTaxi, taxiSlotStart, "Taxi Entry");

     } else {

       clearSlot(index, occupiedUIDTaxi, slotOccupiedTaxi);

       assignedSlot = taxiSlotStart + index;

       showLCD("Taxi Exit", "Slot " + String(assignedSlot));

       openGate();

     }

   } else {

     int index = findCard(uid, occupiedUIDNormal, totalNormal);

     if (index == -1) {

       assignedSlot = assignSlot(uid, occupiedUIDNormal, slotOccupiedNormal, totalNormal, normalSlotStart, "Normal Entry");

     } else {

       clearSlot(index, occupiedUIDNormal, slotOccupiedNormal);

       assignedSlot = normalSlotStart + index;

       showLCD("Normal Exit", "Slot " + String(assignedSlot));

       openGate();

     }

   }

 

   mfrc522.PICC_HaltA();

   mfrc522.PCD_StopCrypto1();

 }

}

 

String detectColor() {

 digitalWrite(S2, LOW); digitalWrite(S3, LOW);

 int red = pulseIn(COLOR_OUT, LOW);

 digitalWrite(S2, HIGH); digitalWrite(S3, HIGH);

 int green = pulseIn(COLOR_OUT, LOW);

 digitalWrite(S2, LOW); digitalWrite(S3, HIGH);

 int blue = pulseIn(COLOR_OUT, LOW);

 

 Serial.print("R:"); Serial.print(red);

 Serial.print(" G:"); Serial.print(green);

 Serial.print(" B:"); Serial.println(blue);

 

 if (green < red && green < blue) return "EV";        // Green is dominant

 else if (red < green && red < blue) return "Taxi";   // Yellow is mostly red + green, but here red wins

 else if (abs(red - green) < 30 && abs(blue - green) < 30 && red < 150) return "Normal";  // White

 else return "Normal";

}

 

int findCard(byte* uid, byte occupied[][4], int size) {

 for (int i = 0; i < size; i++) {

   if (memcmp(uid, occupied[i], 4) == 0) return i;

 }

 return -1;

}

 

int assignSlot(byte* uid, byte occupied[][4], bool occupiedFlags[], int size, int startSlot, String typeText) {

 for (int i = 0; i < size; i++) {

   if (!occupiedFlags[i]) {

     memcpy(occupied[i], uid, 4);

     occupiedFlags[i] = true;

     int slotNumber = startSlot + i;

     showLCD(typeText, "Slot " + String(slotNumber));

     openGate();

     return slotNumber;

   }

 }

 showLCD("Parking Full", typeText);

 return -1;

}

 

void clearSlot(int index, byte occupied[][4], bool occupiedFlags[]) {

 memset(occupied[index], 0, 4);

 occupiedFlags[index] = false;

}

 

void openGate() {

 servo.write(90);

 delay(3000);

 servo.write(0);

}

 

void showLCD(String line1, String line2) {

 lcd.clear();

 lcd.setCursor(0, 0);

 lcd.print(line1);

 lcd.setCursor(0, 1);

 lcd.print(line2);

 delay(2000);

}
