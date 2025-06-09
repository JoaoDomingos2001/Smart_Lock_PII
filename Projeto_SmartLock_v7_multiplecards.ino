/*
   --------------------------------------------------------------------------------------------------------------------
                                                       "Smart Lock"

                                                 Projeto para a cadeira de
                                                        Projeto II
                                                           LEEC

                                                         Turma PL4

                                                Professor Mahmound Tavakoli

                                                        Criado por:
                                                 João Domingos, 2019232053
                                                 João Ferreira, 2019230885
   --------------------------------------------------------------------------------------------------------------------
*/

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
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo sg90;

// Initialize Pins for led's, servo and buzzer
constexpr uint8_t greenLed = 9;
constexpr uint8_t redLed = 10;
constexpr uint8_t servoPin = 11;
constexpr uint8_t buzzerPin = 8;

String user = "User";
String master = "Joao";

char pass_encript[Password_Length] = "*******"; // Palavra-passe encriptada
char initial_password[Password_Length] = "123A456*";  // Variable to store initial password
char master_password[Password_Length] = "2704ABC*";
String tagUID_master = "00 AB 07 D0";
String tagUID = "F8 25 F5 2A";  // String to store UID of tag. Change it with your tag's UID
char password[Password_Length];   // Variable to store users password
boolean RFIDMode = true; // boolean to change modes
char key_pressed ; // Variable to store incoming keys
uint8_t i = 0;  // Variable used for counter for the password on keypad
uint8_t j = 0; //Another count for keypad

//variação dos angulos de abertura do servo
const int fechado = 90;
const int aberto = 0;

long int timer; // usado para o millis no loop
long int time_waiting = 0; //tempo de espera aplicado a quem falhar o codigo mais de 3 vezes
long int previousMillis = 0;
int tentativas = 0; // variavel que guarda o nº de tentativas

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
  sg90.write(fechado); //Declare the angle of servo

  lcd.init();   // LCD screen
  lcd.backlight();
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522

  lcd.clear(); // Clear LCD screen

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
}

void loop() {
  timer = millis();
  // System will first look for mode
  if (RFIDMode == true) {
    menu();
    j = 0;
    i = 0;
    key_pressed = keypad_key.getKey(); // Storing keys
    if (Looking4Keypad(key_pressed)) {
      password[i] = key_pressed;
      i++;
      j++;
      lcd.clear();
      return;
    }

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
      BemVindo(user);
      PortaAberta();
      lcd.clear();

      RFIDMode = true; // Make RFID mode false
    }
    else if (tag.substring(1) == tagUID_master) {
      BemVindo(master);
      key_pressed = keypad_key.getKey();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1-Entrar");
      lcd.setCursor(0,1);
      lcd.print("2-Nova tag");
      
      if(key_pressed = '1'){
       PortaAberta();
      lcd.clear(); 
      }
      else{
        lcd.clear();
      }

      RFIDMode = true; // Make RFID mode false
    }
    else
    {
      // If UID of tag is not matched.
      nBemVindo();
      PortaFechada();
      lcd.clear();

      RFIDMode = true;
    }
  }

  // If RFID mode is false, it will look for keys from keypad
  if (RFIDMode == false) {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
    lcd.setCursor(0, 1);
    key_pressed = keypad_key.getKey(); // Storing keys

    if (Looking4Cards() == true) {
      RFIDMode = true;
      return;
    }
    if (j == 1) {
      lcd.setCursor(i - 1, 1);
      lcd.print(pass_encript[i - 1]);
    }
    if (key_pressed)
    {
      password[i] = key_pressed; // Storing in password variable
      if (i < Password_Length) {
        lcd.setCursor(i, 1);
        lcd.print(pass_encript[i]);
      }
      i++;
    }
    if (i == Password_Length) // If 8 keys are completed
    {
      if (!(strncmp(password, initial_password, Password_Length))) // If password is matched
      {
        BemVindo(user);
        PortaAberta();
        lcd.clear();
        i = 0;
        RFIDMode = true; // Make RFID mode true
      }
      else if (!(strncmp(password, master_password, Password_Length))) {
        BemVindo(master);
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
        tentativas++;
        i = 0;
        waiting(timer);
        RFIDMode = true;  // Make RFID mode true
      }
    }
  }
}

void BemVindo(String nome) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tag/Pass Right");
  lcd.setCursor(0, 1);
  lcd.print("Bem-vindo!");
  lcd.print(nome);
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

//CONVERSÃO DO TEMPO QUE PASSA DESDE QUE O ARDUINO FOI LIGADO (USANDO A FUNÇÃO MILLIS())
long int convertToSeconds(long int t) {
  long int seconds = t / 1000;
  return seconds;
}

long int convertToMinutes(long int t) {
  long int minutes = convertToSeconds(t) / 60 ;
  return minutes;
}

long int convertToHours(long int t) {
  long int hours = convertToMinutes(t) / 60;
  return hours;
}
//-----------------------------------------------------------------------//

void scroll() {
  for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(750);
  }
}

void horas() { //Reprodução das horas no Smart Lock
  unsigned int horas = 11; //Por não termos acesso ao Wifi e o RTC falhar ao usar o LCD
  unsigned int minutos = 00; //As horas e minutos são distribuidos automáticamente
  minutos = minutos + convertToMinutes(timer);
  if (minutos >= 60) {
    horas = horas + (minutos / 60);
    minutos = minutos % 60;
  }
  if (horas >= 24) {
    horas = horas - 24;
  }
  if (horas < 10) { // Formatação das horas para ficar HH:MM
    lcd.setCursor(11, 0);
    lcd.print("0");
    lcd.setCursor(12, 0);
    lcd.print(horas);
  }
  else{
    lcd.setCursor(11, 0);
    lcd.print(horas);
  }
  if (minutos < 10) { // Formatação das horas para ficar HH:MM
    lcd.setCursor(13, 0);
    lcd.print(":0");
    lcd.setCursor(15, 0);
    lcd.print(minutos);
  }
  else {
    lcd.setCursor(13, 0);
    lcd.print(":");
    lcd.setCursor(14, 0);
    lcd.print(minutos);
  }
}

void menu() { // Menu do Smart Lock quando um utilizador se aproxima
  //lcd.clear();
  horas();
  lcd.setCursor(0, 0);
  lcd.print("Smart Lock");
  lcd.setCursor(0, 1);
  lcd.print("Pass tag or code");
  //scroll();
}

void waiting(long int t) { // Método de segurança implentado que obriga utilizador a esperar se errar várias vezes
  previousMillis = t;
  if (tentativas >= 3) {
    time_waiting = (tentativas - 2) * 60000;
  }
  else
    return;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("To unlock wait...");
  lcd.setCursor(0, 1);
  lcd.print( convertToMinutes(time_waiting) );
  lcd.setCursor(4, 1);
  if (convertToMinutes(time_waiting) == 1) {
    lcd.print("Minute");
  }
  else {
    lcd.print("Minutes");
  }
  delay(time_waiting);
  lcd.clear();
}

bool Looking4Cards() { //Procura alguma aproximação de um cartão
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  return true;
}

bool Looking4Keypad(char aux) { // Verifica se algum botão do teclado foi pressionado
  switch (aux) {
    case '1':
      RFIDMode = false;
      return true;
      break;
    case '2':
      RFIDMode = false;
      return true;
      break;
    case '3':
      RFIDMode = false;
      return true;
      break;
    case '4':
      RFIDMode = false;
      return true;
      break;
    case '5':
      RFIDMode = false;
      return true;
      break;
    case '6':
      RFIDMode = false;
      return true;
      break;
    case '7':
      RFIDMode = false;
      return true;
      break;
    case '8':
      RFIDMode = false;
      return true;
      break;
    case '9':
      RFIDMode = false;
      return true;
      break;
    case '0':
      RFIDMode = false;
      return true;
      break;
    case 'A':
      RFIDMode = false;
      return true;
      break;
    case 'B':
      RFIDMode = false;
      return true;
      break;
    case 'C':
      RFIDMode = false;
      return true;
      break;
    case 'D':
      RFIDMode = false;
      return true;
      break;
    case '*':
      RFIDMode = false;
      return true;
      break;
    case '#':
      RFIDMode = false;
      return true;
      break;
    default:
      RFIDMode = true;
      return false;
      break;
  }
}
