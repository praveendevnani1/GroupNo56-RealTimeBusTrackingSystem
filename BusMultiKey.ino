/* @file MultiKey.ino
|| @version 1.0
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
|| @description
|| | The latest version, 3.0, of the keypad library supports up to 10
|| | active keys all being pressed at the same time. This sketch is an
|| | example of how you can get multiple key presses from a keypad or
|| | keyboard.
|| #
*/

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display



int PASS=0;

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {10, 9, 8, 7}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {6, 5, 4, 3}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long loopCount;
unsigned long startTime;
String msg;
String number = "0", MODE = "SET", num = "";
char pressed;
int pressedX = 0;


void setup() {
  Serial.begin(9600);
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onRequest(requestEvent); /* register request event */
  Serial.println("start");
  loopCount = 0;
  startTime = millis();
  msg = "";

  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  
  lcd.home();
  
  lcd.print("Press * to set");
  Serial.print("Press * to set");
  lcd.setCursor(0, 1);
  lcd.print("Bus Number");
  Serial.print("Bus Number");
}


void loop() {
  loopCount++;
  if ( (millis()-startTime)>5000 ) {
    /*Serial.print("Average loops per second = ");
    Serial.println(loopCount/5);*/
    startTime = millis();
    loopCount = 0;
  }

  // Fills kpd.key[ ] array with up-to 10 active keys.
  // Returns true if there are ANY active keys.
  if (kpd.getKeys())
  {
    for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
          msg = "PRESSED";
          break;
          case HOLD:
          msg = "HOLD";
          break;
          case RELEASED:
          msg = "RELEASED";
          pressed = kpd.key[i].kchar;
          break;
          case IDLE:
          msg = " IDLE.";
        }


        if(pressed == '*' && MODE == "SET" && msg == "RELEASED"){
          MODE = "INPUT";
          num = "";
          lcd.setCursor(0, 0);
          lcd.print("Enter Bus Number : ");
        }
        else if(pressed == '*' && MODE == "INPUT" && msg == "RELEASED"){
          MODE = "SET";
          lcd.clear();
          lcd.setCursor(0, 0);
          String msg = "Bus Number : " +num;
          lcd.print(msg);

          lcd.setCursor(0, 1);
          lcd.print("Press * to change");
          number = num;
        }
        else if(pressed != '*' && MODE == "INPUT" && msg == "RELEASED"){
            lcd.clear();
            num += pressed;
            lcd.setCursor(0, 0);
            msg = "Bus Number : " +num;
            lcd.print(msg);
            
            lcd.setCursor(0, 1);
            lcd.print("Press * to set");
        }
      }
    }
  }
}  // End loop

void requestEvent() {
 Wire.write(number.c_str());  /*send string on request */
}


