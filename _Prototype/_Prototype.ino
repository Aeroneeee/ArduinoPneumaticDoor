#if defined(__AVR__)
  #define imagedatatype  unsigned int
#elif defined(__PIC32MX__)
  #define imagedatatype  unsigned short
#elif defined(__arm__)
  #define imagedatatype  unsigned short
#endif

#include <UTFT.h>
#include <URTouch.h>

// For thermal sensor
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

Adafruit_AMG88xx amg;

float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

extern uint8_t BigFont[];
extern uint8_t SmallFont[];

extern imagedatatype eyebtn[];
extern imagedatatype backspace[];

UTFT myGLCD(ILI9341_16,38,39,40,41);

URTouch myTouch(6,5,4,3,2);

int x, y, z, pinLen=0;
bool doorState = false, runkey = true, eyePressed = false; 
char inputPin[4], pin[]={'1', '2', '3', '4'};

unsigned long previousMillis = 0;
const long interval = 5000;

void inputBox(){    //Draw the Grey box with white border
  myGLCD.setColor(97, 97, 97);
  myGLCD.fillRoundRect(10, 20, 230, 70);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 20, 230, 70);
  myGLCD.setColor(127, 127, 127);
  myGLCD.drawRoundRect (11, 21, 229, 69);
}

void updatePin(int val){
  if(pinLen<=3 && pinLen>=0){
    inputPin[pinLen]=val;      //Append to pin
    inputPin[pinLen+1]='\0';   //Clear anything after the appended value
    pinLen++;
    showhide(pinLen, eyePressed);
  }
  if(pinLen==4){                  //Test the input pin
    if((pin[0]==inputPin[0]) && (pin[1]==inputPin[1]) && (pin[2]==inputPin[2]) && (pin[3]==inputPin[3])){ //Correct pin
      if(doorState==true){
        doorState = false;
        digitalWrite(12, LOW);
      }else{
        doorState = true;
        digitalWrite(12, HIGH);
      }
      blinking(0, 255, 0);    //Green
      myGLCD.clrScr();
      runkey = false;
      titlePage();
    }else{                          //Wrong pin
      blinking(255, 0, 0);     //Red
      inputBox();
    }
    inputPin[0]='\0';
    pinLen=0;
  }
}

void blinking(int r, int g, int b){   // Blinking box for 3 times
  for(int j=0; j<=3; j++){ 
    myGLCD.setColor(r, g, b);
    myGLCD.drawRoundRect (10, 20, 230, 70);
    myGLCD.setColor(r, g, b);
    myGLCD.drawRoundRect (11, 21, 229, 69);
    digitalWrite(13, HIGH);
    delay(250);
    myGLCD.setColor(255, 255, 255); //White box
    myGLCD.drawRoundRect (10, 20, 230, 70);
    myGLCD.setColor(127, 127, 127); //Grey Box
    myGLCD.drawRoundRect (11, 21, 229, 69);
    digitalWrite(13, LOW);
    delay(250);
  }
}

void showhide(int l, bool show){
  inputBox();
  myGLCD.setFont(BigFont);
  for(int i = l; i>0; i--){   //Hide the pin code
    if(show){
      myGLCD.setColor(0, 255, 0);
      myGLCD.printChar(inputPin[i-1], 35+((i-1)*50), 38);
    }else{
      myGLCD.setColor(255, 255, 255);
      myGLCD.print("*", 35+((i-1)*50), 38);
    }
  }
}

int pressingBtn(int x1, int y1, int x2, int y2){ 
  //RED box button while pressing  
  myGLCD.setColor(255, 0, 0);     //RED
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable()){
    myTouch.read();
    digitalWrite(13, HIGH); //buzzer
  }
  myGLCD.setColor(255, 255, 255); //White
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  digitalWrite(13, LOW);
}

void titlePage(){
  myGLCD.clrScr();        
  myGLCD.setBackColor(VGA_TRANSPARENT);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Accreditation", CENTER, 5);
  myGLCD.print("Office", CENTER, 21);
  myGLCD.print("Door Lock using", CENTER, 50);
  myGLCD.print("Pneumatic", CENTER, 66);
  myGLCD.print("Cylinder with", CENTER, 82);
  myGLCD.print("Thermal Camera", CENTER, 98);
  myGLCD.print("and", CENTER, 114);
  myGLCD.print("Alert System", CENTER, 130);
  myGLCD.setFont(SmallFont);
  myGLCD.print(" Proponents:", LEFT, 180);
  myGLCD.print("LOREN, Trizzia Fhidez L.", CENTER, 192);
  myGLCD.print("MULINGBAYAN, Chris John B.", CENTER, 204);
  myGLCD.print("PAMINTUAN, Aerone G.:", CENTER, 216);
  myGLCD.print("QUILAPIO, Clarisse C.", CENTER, 228);
  
  if(doorState){
    myGLCD.print("The door is OPEN", CENTER, 275);
  }else{
    myGLCD.print("The door is CLOSE", CENTER, 275);
  }
  myGLCD.print("Press anywhere to enter pin...", CENTER, 308);
}

void thermal(){
  if(!doorState){            //scan only if door is closed state
    amg.readPixels(pixels);
    for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
      if (pixels[i-1] >= 31){    //32 degree celsius
        //Insert AT commands
        myGLCD.clrScr();
        myGLCD.fillScr(VGA_RED);
        myGLCD.setFont(BigFont);
        myGLCD.setColor(0, 0, 0);
        myGLCD.print("SENDING", CENTER, 150);
        myGLCD.print("MESSAGE", CENTER, 165);
        myGLCD.setColor(255, 255, 255);
        for(int j=0; j<=20; j++){
          myGLCD.setColor(255, 255, 255);
          myGLCD.print("INTRUDER ALERT!", CENTER, 100);
          delay(100);
          myGLCD.setColor(0, 0, 0);
          myGLCD.print("INTRUDER ALERT!", CENTER, 100);
          delay(100);
        }
        titlePage();
        runkey = false;
      }
    }
  }
}

void setup() {
  myGLCD.InitLCD(PORTRAIT);
  myGLCD.clrScr();

  myTouch.InitTouch(PORTRAIT);
  myTouch.setPrecision(PREC_MEDIUM);
  titlePage();

  pinMode(12, OUTPUT); // door open signal
  pinMode(13, OUTPUT); // buzzer
  
  // test the thermal cam
  bool status = amg.begin();
 
  if (!status){
      Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
      while (1);
  }
  delay(100); // let sensor boot up
}

void loop() { 
  if(!myTouch.dataAvailable()){
    thermal();
  }
  
  else{
    myGLCD.clrScr();

    myGLCD.setFont(SmallFont);
    if(doorState){
      myGLCD.print("Enter PIN to CLOSE", CENTER, 5);
    }else{
      myGLCD.print("Enter PIN to OPEN", CENTER, 5);
    }
    
    myGLCD.setFont(BigFont);
    inputBox();           // Draw the input box, then
    
    z=0;
    for(y=0; y<4; y++){   // Draw the keypad
      for(x=0; x<3; x++){
        myGLCD.setColor(0, 0, 255);
        myGLCD.fillRoundRect (28+(x*62), 72+(y*62), 88+(x*62), 132+(y*62));
        myGLCD.setColor(255, 255, 255);
        myGLCD.drawRoundRect (28+(x*62), 72+(y*62), 88+(x*62), 132+(y*62));
        myGLCD.setBackColor(VGA_TRANSPARENT);
        if(y!=3)myGLCD.printNumI(z+x+1, 51+(x*62), 95+(y*62));
      }
      z+=3;
    }
    myGLCD.printNumI(0, 113, 282);

    //Draw icons
    myGLCD.drawBitmap(34, 263, 50, 50, eyebtn);   
    myGLCD.drawBitmap(158, 263, 50, 50, backspace);
    
    runkey = true;
    eyePressed = false;
    unsigned long currentMillis = millis();
    
    while(runkey){      //Read the touch input
      thermal();
      if (!myTouch.dataAvailable()){
        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
          runkey = false;
          titlePage();
        }
      }
      else {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
    
        if(y>=72 && y<=132){
          if(x>=28 && x<=88){
            pressingBtn(28, 72, 88, 132);
            updatePin('1');
          }
          if(x>=90 && x<=150){
            pressingBtn(90, 72, 150, 132);
            updatePin('2');
          }
          if(x>=152 && x<=212){
            pressingBtn(152, 72, 212, 132);
            updatePin('3');
          }
        }
        
        if(y>=134 && y<=194){
          if(x>=28 && x<=88){
            pressingBtn(28, 134, 88, 194);
            updatePin('4');
          }
          if(x>=90 && x<=150){
            pressingBtn(90, 134, 150,194);
            updatePin('5');
          }
          if(x>=152 && x<=212){
            pressingBtn(152, 134, 212, 194);
            updatePin('6');
          }
        }
          
        if(y>=196 && y<=256){
          if(x>=28 && x<=88){
            pressingBtn(28, 196, 88, 256);
            updatePin('7');
          }
          if(x>=90 && x<=150){
            pressingBtn(90, 196, 150, 256);
            updatePin('8');
          }
          if(x>=152 && x<=212){
            pressingBtn(152, 196, 212, 256);
            updatePin('9');
          }
        }
    
        if(y>=258 && y<=318){
          if(x>=28 && x<=88){   //Show PIN button
            pressingBtn(28, 258, 88, 318);
            if(eyePressed){
              myGLCD.setColor(255, 255, 255);     //White
              myGLCD.drawRoundRect (28, 258, 88, 318);
              eyePressed = false;
              showhide(pinLen, eyePressed);
            }else{
              myGLCD.setColor(255, 0, 0);         //RED
              myGLCD.drawRoundRect (28, 258, 88, 318);
              eyePressed = true;
              showhide(pinLen, eyePressed);
            }
          }
          if(x>=90 && x<=150){
            pressingBtn(90, 258, 150, 318);
            updatePin('0');
          }
          if(x>=152 && x<=212){   //Backspace or Exit
            pressingBtn(152, 258, 212, 318);
            if(!(pinLen==0)){
              pinLen--;
              inputPin[pinLen]='\0';
              showhide(pinLen, eyePressed);
            }else{
              titlePage();
              runkey = false;
            }
          }
        }
      }      
    }
  }
}
