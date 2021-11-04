// constant values
const unsigned short ROWS = 4; // four rows
const unsigned short COLS = 4; // four columns
const unsigned short CLOSED = LOW;
const unsigned short OPEN = HIGH;

bool starPressed;
bool passwordChecked;
char attempt[4];
char password[3] = { '1', '2', '3' };
int passwordSize = sizeof(password)/sizeof(password[0]);
int inputPos = 0;
int arrayRef1 = 0;
int arrayRef2 = 1;
int attemptIndex = 1;

// Keypads have several states
typedef enum { IDLE, PRESSED, HOLD, RELEASED } KeyState; 

// global variables to track state of keypad
KeyState state = IDLE;
bool isButtonDown = false;
bool stateChanged = false;

// array which stores pins associated with rows of keypad matrix
short rowPins[ROWS] = { 2, 3, 4, 5 };

// array which stores pins associated with columns of keypad matrix
short colPins[COLS] = { 6, 7, 8, 9 };

// initialization of keypad matrix characters
char keys[ROWS][COLS] = { {'1', '2', '3', 'A'},
                          {'4', '5', '6', 'B'},
                          {'7', '8', '9', 'C'},
                          {'*', '0', '#', 'D'} };

// function prototype needed because 
// this function will be called before providing code for it
void executeStateMachine(short, short);

void setup() {
  // configure pins associated with colums of keypad matrix for output
  for(unsigned short int columnIndex = 0; columnIndex < COLS; columnIndex++){
    pinMode( colPins[columnIndex], OUTPUT);
  }

  // configure pins associated with rows of keypad matrix for input
  for(unsigned short int rowIndex = 0; rowIndex < ROWS; rowIndex++){
    pinMode( rowPins[rowIndex], INPUT );
    // writing to an input port enable the internal pull-up resistors. 
    // The input signal in these pins will be normally HIGH
    digitalWrite( rowPins[rowIndex], HIGH);
  }
  // initialize the serial port for viewing the keypad input
  Serial.begin(9600);

}

void loop() {
  
  // flag to determine if a button down had been detected
  isButtonDown = false;

  // variables to keep track of rows and columns during the scanning process
  unsigned int curCol, curRow;

  // variable keep track of key detected by specified row and col value
  unsigned char curKey = 0;

  // flag indicates a state change occured. Assume on every scan the state has not changed
  stateChanged = false;

  // begin the scanning process. for each column we will scan every row
  for( int columnIndex = 0; columnIndex < COLS; columnIndex++ ){
    // First, set column identified by columnIndex to LOW. With the column grounded
    // we scan each row to look for a zero (0) value. If a zero value is returned
    // from any one row, that means that a row/column connection has been made
    digitalWrite(colPins[columnIndex], LOW);

    // begin scanning each row, looking for zero value
    for(int rowIndex = 0; rowIndex < ROWS; rowIndex++){
      // row pins have pull-up resistors activated, it should be 1 for unpressed
      // if curKey is not 1, then that means one button at column (columnIndex) and
      // row (rowIndex) has been pressed. Read the signal of row pin (rowIndex).
      curKey = digitalRead(rowPins[rowIndex]);

      // determine if a connection between row and column has been made
      if(curKey == 0){
        curCol = columnIndex;
        curRow = rowIndex;
        isButtonDown = true;
        break;
      }
    }

    // return the column value to its original form
    digitalWrite(colPins[columnIndex], HIGH);

    // row/column connection button has been identitifed, exit for loop to execute
    // state machine
    if(isButtonDown)
      break;
  }

  // execute state machine
  executeStateMachine(curRow, curCol);
  
}

void executeStateMachine(short curRow, short curCol){
  static unsigned long timer;
  unsigned int debounceTime = 2;
  unsigned int holdTime = 500;
  char currentKey;

  // execute the state machine
  switch(state){
    case IDLE:
      // waiting for a debounced keypress
      if(isButtonDown && (millis() - timer) > debounceTime){
        currentKey = keys[curRow][curCol];

        // print the current key to the console
        Serial.print(currentKey);

        // start recording input when * is pressed
        if(currentKey == '*'){ 
          starPressed = true;
        }

        if(starPressed){
          attempt[inputPos] = currentKey;
        }
             
        // check input when # is pressed
        if((currentKey == '#') && (starPressed)){

          passwordChecked = false;
 
          // calculate the size of the attempt to compare size to password
          int attemptSize = sizeof(attempt)/sizeof(attempt[0]);

          // check if pin attempt is correct length, if not its incorrect
          if ( ((attemptSize) - 1) == passwordSize ){

            // check each index
            do{
              // index comparison between both arrays
              char num1 = password[arrayRef1];
              char num2 = attempt[arrayRef2];

              // if the current indexes are correct
              if ( num1 == num2 ){
                // check the next
                arrayRef1++;
                arrayRef2++;
                attemptIndex++;
                
                // if you make it to the end of the attempt and all indexes match, then password is correct
                if ( attemptIndex == ((passwordSize) + 1 )){
                  Serial.println(" CORRECT PIN ");
                  passwordChecked = true;
                  starPressed = false;
                  for (int i = 0; i < 4; i++){
                    attempt[i] = 0;
                  }
                  inputPos = 0;
                  arrayRef1 = 0;
                  arrayRef2 = 1;
                  attemptIndex = 1;
                }
              }else{    // an index does not match
                Serial.println(" INCORRECT PIN ");
                passwordChecked = true;
                starPressed = false;
                for (int i = 0; i < 4; i++){
                  attempt[i] = 0;
                }
                inputPos = 0;
                arrayRef1 = 0;
                arrayRef2 = 1;
                attemptIndex = 1;
              }          
            }while(passwordChecked == false);
     
          }else{      // incorrect length
            Serial.println(" INCORRECT PIN ");
            starPressed = false;
            for (int i = 0; i < 4; i++){
              attempt[i] = 0;
            }
            inputPos = 0;
            arrayRef1 = 0;
            arrayRef2 = 1;
            attemptIndex = 1;
          }
        }  
        
        state = PRESSED; // change the state
        stateChanged = true;
        timer = millis();
        
        if(starPressed){
          inputPos++;
        }
        
      }
      break;
    case PRESSED:
      // waiting for a key hold
      if((millis() - timer) > holdTime){
        state = HOLD; // move to next state
        stateChanged = true;
        timer = millis(); // reset debounce timer
      }
      // or for the key to be released
      else if(!isButtonDown && (millis() - timer) > debounceTime){
        state = RELEASED;
        stateChanged = true;
        timer = millis();
      }
      break;
    case HOLD: 
      // waiting for the key to be released.
      if((!isButtonDown) && (millis() - timer) > debounceTime){
        state = RELEASED;
        stateChanged = true;
        timer = millis();
      }
      break;
    case RELEASED:
      state = IDLE;
      break;
    }
}
