#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//creating the phases and the state of the buttons
typedef enum {SYNC,MAIN} PHASE;
typedef enum {PRESS,RELEASE} BUTTON_STATE;

//this is where I created the channel with all the variables
struct Channel{
  char Name;
  char* Description;
  int Min;
  int Max;
  int Value;
  bool Occupied; // check if the channel is empty or not
  int sum;
  int count;
  int avg;

//allocating the inputted values into the variables
  void setName(char Name){
    this -> Name = Name;
  }

  void setDescription(char* Description){
    if (not (this -> Description)){ // description doesnt exists
      this -> Description = (char*) malloc (sizeof(char)*16);// memory allocation consacutive free spaces find th start look for the pointer of the base of the size of a char * 16
    }
    strcpy(this -> Description,Description);
  }

  void setMin(int Min){
    this->Min = Min;
  }

  void setMax(int Max){
    this->Max = Max;
  }

  void setValue(int Value){
    this->Value = Value;
    //for RECENT extension only finding the overall value no holding of any values
    this -> sum = sum + Value;
    this -> count += 1;
    this -> avg = this -> sum / this->count;
  }

  void setOccupied(bool Occupied){
    this -> Occupied = Occupied;
  }

//actuallty getting the values for the variables
  void getDescription(char* Descrip){
    strcpy(Descrip, this -> Description); 
  }

  char getName(){
    return this->Name;
  }

  int getValue(){
    return this->Value;
  }

  int getMin(){
    return this->Min;
  }

  int getMax(){
    return this->Max;
  }

  int getAvg(){
    return this->avg;
  }
};

void cutDown15(String Descrip, char* Description){
  int mix = min(15, Descrip.length());
  for ( int i = 0; i < mix; i++){
    Description[i] = Descrip[i];
  }
  Description[mix] = '\0';
}


bool validate(String input){
  int check = input.length()-1;
  if (check >= 3){
    if (isAlpha(input[1]) and isUpperCase(input[1])){
      if (input[0] == 'C'){
        return true;
      }
      else if((input[0] == 'V') or (input[0] == 'X') or (input[0] == 'N')){
        char integer[check];
        for (int i = 2; i < check; i++){
          if(isDigit(input[i])){
            integer[i] = input[i];
          }
          else {
            return false;
          }
        }
        int number = atoi(integer);
        if ((number >=0) and (number <= 255)){
          return true;
        }
      }
    }
  }
  return false;
}

// this function was used to get the channel letter and to store it
int getChannel(char ID, Channel * channels){
  for (int i = 0; i < 26; i++){
    //Serial.println((int)channels[i].Name); DEBUG
    if (ID == channels[i].Name){ //checking through each channel name
      return i; // assume that the channel always exists 
    }
  }
}

//this was to check if the channel had already previously existed 
bool checkChannel(char ID, Channel * channels){
  for (int i = 0; i < 26; i++){
    if (ID == channels[i].Name){ //going through each channel
      return true; //returns true if the channel existed 
    } 
  }
  return false; //false if it does not
}

//this is a function that goes through what its suppose to depending on what you input
int process(String userInput, Channel * channels){
  int ChannelP;
  String Value = userInput.substring(2,userInput.length()-1);
  //this is if the user input is C
  if (userInput[0] == 'C'){
    char ArrayValue[16];
    cutDown15(Value,ArrayValue);
    //Serial.print(ArrayValue);
    //if channel didnt existed each assigned value to the default
    if (not checkChannel(userInput[1],channels)){
      channels[userInput[1] - 'A'].setName(userInput[1]);
      channels[userInput[1] - 'A'].setMax(255);
      channels[userInput[1] - 'A'].setMin(0);
      channels[userInput[1] - 'A'].setDescription(ArrayValue);
      channels[userInput[1] - 'A'].setOccupied(false);
      //this is if the channel already existed
    } else {
      ChannelP = getChannel(userInput[1],channels); 
      channels[ChannelP].setDescription(ArrayValue);
    }
    return userInput[1] - 'A'; //minused it with a to get the channel array 
    //if the user input V
  } else if (userInput[0] == 'V'){
    if (checkChannel(userInput[1],channels)){
      ChannelP = getChannel(userInput[1],channels); 
      channels[ChannelP].setValue(Value.toInt()); 
      channels[ChannelP].setOccupied(true);
    }
    //if the user input X
  } else if (userInput[0] == 'X'){
    if (checkChannel(userInput[1],channels)){
      ChannelP = getChannel(userInput[1],channels); 
      channels[ChannelP].setMax(Value.toInt()); 
    }
  } else {
    if (checkChannel(userInput[1],channels)){
      ChannelP = getChannel(userInput[1],channels); 
      channels[ChannelP].setMin(Value.toInt()); 
    }
  }
  return ChannelP;
}

//this is the fucntion to print out a line in the lcd for both top and bottom
void printFunction(int index,Channel * channels,char* PrintArray){
  char Descrip[16];
  channels[index].getDescription(Descrip);
  //Descrip[10-arrowPrinted] = '\0';
  //Serial.println(strlen(Descrip));
  sprintf(PrintArray,"%c%3d,%3d %s",channels[index].getName(),channels[index].getValue(),channels[index].getAvg(),Descrip);
}

//helps find the next (increase) pointer
int nextPointer(int Pointer, Channel * channels){
  for (int i = Pointer + 1; i < 26; i++){
    if (channels[i].Name and channels[i].Occupied){
      return i;
    }
  }
  return -1; //this is for if the value is the last channel 
}

//helps find the previous (decrease) pointer
int previousPointer(int Pointer, Channel * channels){
  for (int i = Pointer - 1; i > -1; i--){
    if (channels[i].Name and channels[i].Occupied){
      return i;
    }
  }
  return -1;//this is for if the value is the last channel
}

//this prints the arrows and also sets the cursor for where it should be printed
void printPointer(int Pointer,Channel * channels){
  if (Pointer != -1) {
    char PrintArray[22];
    lcd.clear();
    lcd.setCursor(0,0);
    //Serial.println("----------------");
    if (previousPointer(Pointer,channels) != -1){
      //Serial.print('^');
      lcd.write(8);
    }
    printFunction(Pointer,channels, PrintArray);
    //Serial.println(PrintArray);
    lcd.print(PrintArray);
    int secondPointer = nextPointer(Pointer,channels);
    if (secondPointer != -1){
      lcd.setCursor(0,1);
      printFunction(secondPointer,channels, PrintArray);
      //Serial.print('v');
      //Serial.println(PrintArray);
      lcd.write(4);
      lcd.print(PrintArray);
    }
  }
  setBackLight(channels);
}

//this is to set the backlight if there is value above or below the minimum 
void setBackLight(Channel * channels){
  int backlight = 7;
  for (int i = 0;i < 26; i++){
    if (channels[i].getName() and (channels[i].getMin() <= channels[i].getMax()) and channels[i].Occupied){
      if (channels[i].getMax() < channels[i].getValue()){
        if (backlight == 2){
          backlight = 3;
          break;
        }else{
          backlight = 1;
        }
      }else if(channels[i].getMin() > channels[i].getValue()) {
         if (backlight == 1){
           backlight = 3;
           break;
         } else {
          backlight = 2;
         }
      }
    } 
  }
  lcd.setBacklight(backlight);
}

//below are the three functiond for the SCROLL extension
void scrollingOne(String scrollMessage){
  scrollMessage = scrollMessage + " ";
  String scroller = scrollMessage;
  static unsigned int scrollpos = 0;
  static unsigned long now = millis();

  if (millis() - now > 500) {
    now = millis();
    scrollpos ++;
    if (scrollpos >= scroller.length()) {
      scrollpos = 0;
    }
  }
  lcd.setCursor(9, 0);
  lcd.print(scroller.substring(scrollpos, scrollpos + 16));
  //lcd.setCursor(9, 0);
}

void scrollingTop(String scrollMessage){
  scrollMessage = scrollMessage + " ";
  String scroller = scrollMessage;
  static unsigned int scrollpos = 0;
  static unsigned long now = millis();

  if (millis() - now > 500) {
    now = millis();
    scrollpos ++;
    if (scrollpos >= scroller.length()) {
      scrollpos = 0;
    }
  }
  lcd.setCursor(10, 0);
  lcd.print(scroller.substring(scrollpos, scrollpos + 16));
  lcd.setCursor(10, 0);
}

void scrollingBottom(String scrollMessage){
  scrollMessage = scrollMessage + " ";
  String scroller = scrollMessage;
  static unsigned int scrollpos = 0;
  static unsigned long now = millis();

  if (millis() - now > 500) {
    now = millis();
    scrollpos ++;
    if (scrollpos >= scroller.length()) {
      scrollpos = 0;
    }
  }
  lcd.setCursor(10, 1);
  lcd.print(scroller.substring(scrollpos, scrollpos + 16));
  //lcd.setCursor(10, 1);
}


//this is get the free memory for the FREESRAM extension 
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else // __ARM__
extern char *__brkval;
#endif // __arm__
int freeMemory() {
char top;
#ifdef __arm__
return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
return &top - __brkval;
#else // __arm__
return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.setBacklight(5);
  //this is how the arrow characters are created
  byte upArrow[] = {
    B00100,
    B01110,
    B11111,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100
  };
  byte downArrow[] = {
    B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B11111,
    B01110,
    B00100
  };
  lcd.createChar(8,upArrow);
  lcd.createChar(4,downArrow);
}

void loop() {
  // put your main code here, to run repeatedly:
  static PHASE currentPHASE = SYNC;
  static Channel channels[26];
  static long timer;
  static String userInput;
  static int Pointer = -1;
  static int StoredIndex;
  static BUTTON_STATE currentSTATE = RELEASE;
  static int state;
  static long timestamp;
  static bool lastState = false;
  static bool Display = false;
  static unsigned int scrollpos = 0;
  static unsigned long now = millis();
  timer = millis();
  switch (currentPHASE){
   case SYNC:
    if (timer % 1000 == 0){
      Serial.print('Q');
      delay(1);
    }
    if (Serial.available()){
      userInput = Serial.readString();
      if ((2 == userInput.length()) and (userInput[0] == 'X')){
        Serial.println("BASIC");
        Serial.println("RECENT,FREERAM,SCROLL,UDCHARS,NAMES");
        currentPHASE = MAIN;
        lcd.setBacklight(7);
      }
    }
    break;
   
   case MAIN:
    if (Serial.available()){
      userInput = Serial.readString();
      //Serial.println(userInput);
      if (validate(userInput)){
        //Serial.println(userInput);
        StoredIndex = process(userInput,channels);
        if ((Pointer == -1) and (userInput[0] == 'V') and channels[StoredIndex].Occupied){
          Pointer = StoredIndex;
        }
        if (not lastState){
          printPointer(Pointer,channels);
        }
      } else {
        Serial.println("ERROR: " + userInput);
      }
    }

    //this is to implement the SCROLL extension 
    if (not lastState){
      char Descrip[16];
      channels[Pointer].getDescription(Descrip);
      if (strlen(Descrip) > 6){
        if (Pointer == 0){
          scrollingOne(Descrip); 
        } else {
          scrollingTop(Descrip);
        }
      }
 
      char Descrip2[16];
      int secondPointer = nextPointer(Pointer,channels);
      channels[secondPointer].getDescription(Descrip2);
      if (strlen(Descrip2) > 6){
        scrollingBottom(Descrip2);
      }
    }

    
    
    //process button 
    state = lcd.readButtons();
    switch(currentSTATE){
      case RELEASE:
        if (state == 4){ //down button
          int secondPointer = nextPointer(Pointer,channels);
          if (secondPointer != -1){
            Pointer = secondPointer;
            currentSTATE = PRESS;
            printPointer(Pointer,channels);
          }
        } else if (state == 8){ //up button
          int secondPointer = previousPointer(Pointer,channels);
          if (secondPointer != -1){
            Pointer = secondPointer;
            currentSTATE = PRESS;
            printPointer(Pointer,channels);
          }
        } else if (state == 1){
          timestamp = timer;
          Display = true;
          currentSTATE = PRESS;
          lastState = true;
        }
        break;
          
        
      case PRESS:
        if (not state){
          currentSTATE = RELEASE;
          if (lastState == true){
            lcd.clear();
            printPointer(Pointer,channels);
            Display = false;
            //Serial.println("DEBUG");
            //delay(4000);
            lastState = false;
          }
        } 
        else if ((timestamp + 1000 <= millis()) and (Display == true)){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.setBacklight(5);
          lcd.print("F119883");
          lcd.setCursor(0,1);
          lcd.print("Free SRAM: " + String(freeMemory()));
          Display = false;
        } 
    }
  }
}
