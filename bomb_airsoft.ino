/*Airsoft Bomb
 Una bomba fake para airsoft que se desactiva por codigo o cortando el cable correcto*/

#include <Keypad.h>
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>

const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
int second = 0, minute = 0, hour = 0; // declare time variables
int bombState = 0; //0 = Init, 1 = Ready, 2 = Detonada, 3 = Desactivada
int hooter = 11;
int wireCorrect; //Correct wire
int wire1 = 12;
int wire2 = 11;
int wire3 = 10;
int activationMethod = 0;//1 code; 2 wire
String desactivationCode = "";
String temporalCode = "";
int buzzer = 13;
boolean keepCountdown = true;

//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]=
{
  {'1', '2', '3', 'A'}
  ,
  {'4', '5', '6', 'B'}
  ,
  {'7', '8', '9', 'C'}
  ,
  {'*', '0', '#', 'D'}
};

//Code that shows the keypad connections to the arduino terminals
byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
byte colPins[numCols]= {5,4,3,2}; //Columns 0 to 3

//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

LiquidCrystal_I2C lcd(0x27,20,4);// Set the LCD I2C address (SDA =>A4, SCL => A5)

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(hooter, OUTPUT);
  pinMode(wire1, INPUT);
  pinMode(wire2, INPUT);
  pinMode(wire3, INPUT);
  lcd.init();
  lcd.backlight();
}

void resetAll() {
  desactivationCode = "";
  activationMethod = 0;
  temporalCode = "";
  bombState = 0;
  second=0;
  minute=0;
  hour=0;
  lcd.clear();
}

void loop() {
  char keypressed = myKeypad.getKey();
  char c = myKeypad.getKey();
  String minuteS;
  String secondS;

  switch(bombState) {
    case 0: //Init view
      resetAll();
      writeLCD(lcd, "Confirm: #");
      writeLCDTwo(lcd, "Erase/Back: C");
      delay(3000);
      lcd.clear();
      writeLCD(lcd, "Choose method:");
      writeLCDTwo(lcd, "A)Wire    B)Code");
      keypressed = myKeypad.waitForKey();
      if(keypressed == 'A') {
        activationMethod = 2;//Wire
        setWire(myKeypad);
      } else if(keypressed == 'B') {
        activationMethod = 1;//Code
        setCode(myKeypad);
      } else {
        lcd.clear();
        writeLCD(lcd, "Invalid option");
        delay(1000);
      }
      break;

    case 1: //Bomba armada
      startCountdown();
      c = myKeypad.getKey();
      if(activationMethod == 1) {
        if(c != NO_KEY) {
          if(c != '#') {
            temporalCode += c;
          } else if(c == '#') {
            if(temporalCode == desactivationCode) {
              bombState = 3;
            } else {
              temporalCode = "";
            }
          }
        }
      } else if(activationMethod == 2) {
        int statusWire1 = digitalRead(wire1);
        int statusWire2 = digitalRead(wire2);
        int statusWire3 = digitalRead(wire3);
        if((statusWire1 == 0 && wireCorrect == 1) || (statusWire2 == 0 && wireCorrect == 2) || (statusWire3 == 0 && wireCorrect == 3)) {
         bombState = 3;
        } else if((statusWire1 == 0 && wireCorrect != 1) || (statusWire2 == 0 && wireCorrect != 2) || (statusWire3 == 0 && wireCorrect != 3)) {
         bombState = 2;
        }
      }
      if(minute <=0 && second <= 0 && hour <= 0) {
        bombState = 2;
      }
      break;

    case 2: //Bomba detonada
      digitalWrite(buzzer, LOW);
      lcd.clear();
      writeLCD(lcd, "Boooom!! :(");
      digitalWrite(hooter, HIGH);
      delay(3000);
      writeLCDTwo(lcd, "Press #");
      keypressed = myKeypad.waitForKey();
      if(keypressed == '#') {
        bombState = 0;
        digitalWrite(hooter, LOW);
      }
      break;
    
    case 3: //Bomba desarmada
      digitalWrite(buzzer, LOW);
      lcd.clear();
      writeLCD(lcd, "Bomb deactivated");
      writeLCDTwo(lcd, "Press #");
      for(int i = 0; i<3; i++){
        for(int ii= 0; ii<5; ii++){
          digitalWrite(hooter, HIGH);
          delay(130);
          digitalWrite(hooter, LOW);
          delay(300);
        }
        delay(500);
      }
      keypressed = myKeypad.waitForKey();
      if(keypressed == '#') {
        bombState = 0;
      }
      break;
  }
  
}//void loop

//SE ESTABLECE EL CODIGO DE DESACTIVACION
void setCode(Keypad myKeyPad) {
  desactivationCode = "";
  boolean okCode = false;
  lcd.clear();
  writeLCD(lcd, "Set code:");
  while(okCode == false) {
    char temp = myKeyPad.waitForKey();
    if(temp == '#' && desactivationCode.length() > 2) {
      okCode = true;
    } else if(temp != '#') {
      desactivationCode += temp;
      writeLCDTwo(lcd, desactivationCode);
    }
  }
  setCountdown(myKeyPad);
}

//ESTABLECE EL TIEMPO DE LA BOMBA
void setCountdown(Keypad myKeyPad) {
  String bombTimeString = "";
  boolean okTime = false;
  lcd.clear();
  writeLCD(lcd, "Time (minutes):");
  while(okTime == false) {
    char temp = myKeyPad.waitForKey();
    if(temp == '#' && bombTimeString.length()>0) {
      okTime = true;
    } else if(temp == 'C') {
      bombTimeString = bombTimeString.substring(0,bombTimeString.length() - 1);
      lcd.clear();
      writeLCD(lcd, "Time (minutes):");
      writeLCDTwo(lcd, bombTimeString);
    } else {
      if(temp != 'A' && temp != 'B' && temp != 'C' && temp != 'D' && temp != '*') {
        bombTimeString += temp;
        writeLCDTwo(lcd, bombTimeString);
      }
    }
  }
  minute = bombTimeString.toInt();
  //Bomba armada
  bombState = 1;
}

//SE ELIGE EL CABLE CORRECTO
void setWire(Keypad myKeyPad) {
  boolean selectedWire = false;
  lcd.clear();
  writeLCD(lcd, "Choose wire:");
  writeLCDTwo(lcd, " 1, 2 ,3");
  while(selectedWire == false) {
    char wire = myKeyPad.waitForKey();
    if(wire == 'C') {
      return;
    } else if(wire == '1' ) {
      wireCorrect = 1;
      selectedWire = true;
    } else if(wire == '2') {
      wireCorrect = 2;
      selectedWire = true;
    } else if(wire == '3'){
      wireCorrect = 3;
      selectedWire = true;
    } else {
      lcd.clear();
      writeLCD(lcd, "Invalid option");
      delay(1000);
      lcd.clear();
      writeLCD(lcd, "Choose wire:");
      writeLCDTwo(lcd, " 1, 2 ,3");
    }
  }
  setCountdown(myKeyPad);
}

//INICIA EL CONTADOR
void startCountdown() {
  static unsigned long lastTick = 0;
  if (second > 0) {
    if (millis() - lastTick >= 1000) {
      digitalWrite(buzzer, HIGH);
      lastTick = millis();
      second--;
      String minuteS = String(minute);
      String secondS = String(second);
      lcd.clear();
      writeLCD(lcd, "Time: "+ minuteS + ":" + secondS);
    }
  }

  if (millis() - lastTick >= 300) {
    digitalWrite(buzzer, LOW);
  }

 String title = "Code: " + temporalCode;
  if(activationMethod == 2) title = "Enter wire:";
  writeLCDTwo(lcd, title);

  // decrement one minute every 60 seconds
  if (minute > 0) {
    if (second <= 0) {
      minute--;
      second = 60; // reset seconds to 60
    }
  }
  // decrement one hour every 60 minutes
  if (hour > 0) {
    if (minute <= 0) {
      hour--;
      minute = 60; // reset minutes to 60
    }//closes if
  }//closes if
  //digitalWrite(buzzer, LOW);
} //close countdown();

void writeLCD(LiquidCrystal_I2C lcd, String message) {
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void writeLCDTwo(LiquidCrystal_I2C lcd, String message) {
  lcd.setCursor(0, 1);
  lcd.print(message);
}
