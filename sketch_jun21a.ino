// Include required libraries
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>

#define Password_Length 8

constexpr uint8_t RST_PIN = 5;
constexpr uint8_t SS_PIN = 53;

// Create instances
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN); // MFRC522 mfrc522(SS_PIN, RST_PIN)
Servo sg90;

// Initialize Pins for led's, servo and buzzer
// Blue LED is connected to 5V
constexpr uint8_t greenLed = 9;
constexpr uint8_t redLed = 10;
constexpr uint8_t servoPin = 11;
constexpr uint8_t buzzerPin = 8;

char pass_encript[Password_Length] = "*******";
char initial_password[Password_Length] = "123A456*";  // Variable to store initial password
String tagUID = "F8 25 F5 2A";  // String to store UID of tag. Change it with your tag's UID
char password[Password_Length];   // Variable to store users password
boolean RFIDMode = true; // boolean to change modes
char key_pressed ; // Variable to store incoming keys
uint8_t i = 0;  // Variable used for counter
const int fechado = 90;
const int aberto = 0;

// defining how many rows and columns our keypad have
const byte ROWS = 4;
const byte COLS = 4;

// Keypad pin map
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Initializing pins for keypad
byte rowPins[ROWS] = {22, 24, 26, 28};
byte colPins[COLS] = {30, 32, 34, 36};

// Create instance for keypad
Keypad keypad_key = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  // Arduino Pin configuration
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  sg90.attach(servoPin);  //Declare pin 11 for servo
  sg90.write(fechado);

  lcd.init();   // LCD screen
  lcd.backlight();
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522

  lcd.clear(); // Clear LCD screen

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
}

void loop() {
  // System will first look for mode
  if (RFIDMode == true) {
    lcd.setCursor(0, 0);
    lcd.print("   Door Lock");
    lcd.setCursor(0, 1);
    lcd.print(" Scan Your Tag ");

    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    //Reading from the card
    String tag = "";
    for (byte j = 0; j < mfrc522.uid.size; j++)
    {
      tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
      tag.concat(String(mfrc522.uid.uidByte[j], HEX));
    }
    tag.toUpperCase();

    //Checking the card
    if (tag.substring(1) == tagUID)
    {
      // If UID of tag is matched.
      BemVindo();
      PortaAberta();
      lcd.clear();

      RFIDMode = true; // Make RFID mode false
    }

    else
    {
      // If UID of tag is not matched.
      nBemVindo();
      PortaFechada();
      lcd.clear();

      RFIDMode = false;
    }
  }

  // If RFID mode is false, it will look for keys from keypad
  if (RFIDMode == false) {
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
    lcd.setCursor(0, 1);
    key_pressed = keypad_key.getKey(); // Storing keys
    if (key_pressed)
    {
      password[i] = key_pressed; // Storing in password variable
      if(i<Password_Length){
        lcd.setCursor(i, 1);
        lcd.print(pass_encript[i]);
      }
      i++;
    }
    if (i == Password_Length) // If 4 keys are completed
    {
      if (!(strncmp(password, initial_password, Password_Length))) // If password is matched
      {
        BemVindo();
        PortaAberta();
        lcd.clear();
        i = 0;
        RFIDMode = true; // Make RFID mode true
      }
      else    // If password is not matched
      {
        nBemVindo();
        PortaFechada();
        lcd.clear();
        i = 0;
        RFIDMode = false;  // Make RFID mode true
      }
    }
  }
}

void BemVindo() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tag/Pass Right");
  lcd.setCursor(0, 1);
  lcd.print("Bem-vindo!");
}

void nBemVindo() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tag/Pass Wrong");
  lcd.setCursor(0, 1);
  lcd.print("Try again !");
}

void PortaAberta() {
  sg90.write(aberto); // Door Opened
  tone(buzzerPin, 500);
  digitalWrite(greenLed, HIGH);
  delay(3000);
  digitalWrite(greenLed, LOW);
  noTone(buzzerPin);
  sg90.write(fechado); // Door Closed
}

void PortaFechada() {
  tone(buzzerPin, 5);
  digitalWrite(redLed, HIGH);
  delay(3000);
  digitalWrite(redLed, LOW);
  noTone(buzzerPin);
}
