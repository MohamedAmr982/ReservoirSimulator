#include <Servo.h>

const int CAPACITY = 12000;
const int MAX_CHARGE_RATE = 200;
const int MAX_DISCHARGE_RATE = 40;
// 1 minute interval
const int INTERVAL = 1;
const int MAX_QUANTITY = (int) (0.95 * CAPACITY);
const int MIN_QUANTITY = (int) (0.05 * CAPACITY);

const int DELAY_INTERVAL = 1000;

//RGB LED
const int RED_PIN = 8;
const int GREEN_PIN = 9;
const int BLUE_PIN = 10;

//buzzer
const int BUZZER_PIN = 11;

//pushbutton
const int BUTTON_PIN = 3;

//potentiometer
const int POT_CHARGE_PIN = A1;
const int POT_DISCHARGE_PIN = A2;

//servo
const int SERVO_PIN = A0;


typedef struct Context Context;
typedef void (*ContextModifier) (Context* context);

typedef struct State{
  ContextModifier makeTransition;
  ContextModifier updateQuantity;
  ContextModifier updateRate;
  void (*setLED)(void);
  ContextModifier setBuzzer;
  ContextModifier setServoPosition;
  void (*printStateName)(void);
  float (*getRemainingTime) (Context* context);
} State;

struct Context{
  State* state;
  float quantity;
  volatile bool buttonInput;
  float rate;
  int servoPosition;
};

//-----------------------------------------------------------------
void updateQuantity(Context* context);
void updateRate(Context* context);
void setLED();
void setBuzzer(Context* context);
void setServoPosition(Context* context);
void printStateName();
float getRemainingTime(Context* context);
void makeTransition(Context* context);
void initState(State* state);
void idleSetLED();
void idleSetBuzzer(Context* context);
void printIdle();
void transitionFromIdle(Context* context);
void idle(State* state);
void ChargingUpdateRate(Context* context);
void chargingSetLED();
void chargingSetServoPosition(Context* context);
void printCharging();
void transitionFromCharging(Context* context);
void charging(State* state);
void dischargingUpdateRate(Context* context);
void dischargingSetLED();
void dischargingSetServoPosition(Context* context);
void printDischarging();
void transitionFromDischarging(Context* context);
void discharging(State* state);
void printStats(Context* context);
void interruptHandler();
//-----------------------------------------------------------------

//default state behavior

void updateQuantity(Context* context){
  //Quantity new = quantity old + old rate * interval
  context->quantity += context->rate * INTERVAL;
}

void updateRate(Context* context){context->rate = 0;}

void setLED(){
  digitalWrite(BLUE_PIN, HIGH);
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
}

void setBuzzer(Context* context){noTone(BUZZER_PIN);}

void setServoPosition(Context* context){
  context->servoPosition = 90;
}

void printStateName(){Serial.print("Undefined State");}

float getRemainingTime(Context* context){return INFINITY;}

void makeTransition(Context* context){}

//default constructor
void initState(State* state){
  state->makeTransition = makeTransition;
  state->updateQuantity = updateQuantity;
  state->updateRate = updateRate;
  state->setLED = setLED;
  state->setBuzzer = setBuzzer;
  state->setServoPosition = setServoPosition;
  state->printStateName = printStateName;
  state->getRemainingTime = getRemainingTime;
}

//---------------------------------------------------------------
//idle state
void idleSetLED(){
  //blue
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
}

void idleSetBuzzer(Context* context){
  if(!context->buttonInput &&
    (context->quantity >= MAX_QUANTITY 
    || context->quantity <= MIN_QUANTITY)
  ){
    tone(BUZZER_PIN, 1500, DELAY_INTERVAL);
  }
}

void printIdle(){Serial.print("Idle");}

void transitionFromIdle(Context* context){
  if(context->buttonInput){
    if(
      context->quantity >= MAX_QUANTITY ||
      (context->quantity <  MAX_QUANTITY && context->quantity > MIN_QUANTITY) 
      ){
        //similar to this.state = new Idle()
        discharging(context->state);
      }else{
        //this.state = new charging()
        charging(context->state);
      }
  }
}

//constructor
void idle(State* state){
  state->makeTransition = transitionFromIdle;
  state->updateQuantity = updateQuantity;
  state->updateRate = updateRate;
  state->setLED = idleSetLED;
  state->setBuzzer = idleSetBuzzer;
  state->setServoPosition = setServoPosition;
  state->printStateName = printIdle;
  state->getRemainingTime = getRemainingTime;
}

//---------------------------------------------------------------

//charging state
void ChargingUpdateRate(Context* context){
  int reading = analogRead(POT_CHARGE_PIN);
  context->rate = (reading / 1023.0f) * MAX_CHARGE_RATE;
}

void chargingSetLED(){
  //green
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
}

void chargingSetServoPosition(Context* context){
  context->servoPosition = (90 - 5) * context->rate / MAX_CHARGE_RATE;
}

void printCharging(){Serial.print("Charging");}

float chargingGetRemainingTime(Context* context){
  return (MAX_QUANTITY - context->quantity) / context->rate;
}

void transitionFromCharging(Context* context){
  if(context->buttonInput || context->quantity >= MAX_QUANTITY){
    idle(context->state);
  }
}

//constructor
void charging(State* state){
  state->makeTransition = transitionFromCharging;
  state->updateQuantity = updateQuantity;
  state->updateRate = ChargingUpdateRate;
  state->setLED = chargingSetLED;
  state->setBuzzer = setBuzzer;
  state->setServoPosition = chargingSetServoPosition;
  state->printStateName = printCharging;
  state->getRemainingTime = chargingGetRemainingTime;
}

//---------------------------------------------------------------
//discharging state

void dischargingUpdateRate(Context* context){
  int reading = analogRead(POT_CHARGE_PIN);
  context->rate = - (reading / 1023.0f) * MAX_DISCHARGE_RATE;
}

void dischargingSetLED(){
  //red
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
}

void dischargingSetServoPosition(Context* context){
  context->servoPosition = 180 + (90 - 5) * context->rate / MAX_DISCHARGE_RATE;  
}

void printDischarging(){Serial.print("Discharging");}

float dischargingGetRemainingTime(Context* context){
  return (MIN_QUANTITY - context->quantity) / context->rate;  
}

void transitionFromDischarging(Context* context){
  if(context->buttonInput || context->quantity <= MIN_QUANTITY){
    idle(context->state);
  }
}

//constructor
void discharging(State* state){
  state->makeTransition = transitionFromDischarging;
  state->updateQuantity = updateQuantity;
  state->updateRate = dischargingUpdateRate;
  state->setLED = dischargingSetLED;
  state->setBuzzer = setBuzzer;
  state->setServoPosition = dischargingSetServoPosition;
  state->printStateName = printDischarging;
  state->getRemainingTime = dischargingGetRemainingTime;
}

//---------------------------------------------------------------

void printStats(Context* context){  
  Serial.print("Action         ");
  context->state->printStateName();

  Serial.println();

  Serial.print("Rate           ");
  Serial.print(context->rate);
  Serial.print(" /minute");

  Serial.println();

  Serial.print("Quantity       ");
  Serial.print(context->quantity);

  Serial.println();

  Serial.print("Percentage     ");
  Serial.print((context->quantity / MAX_QUANTITY) * 100);
  Serial.print("%");

  Serial.println();

  Serial.print("Remaining time ");
  Serial.print(context->state->getRemainingTime(context));
  Serial.println(" minutes");
  Serial.println("*******************************");
}

//----------------------------------------------------------------------
//global vars
Servo servo;
Context context;
State state;

//ISR
void interruptHandler(){
  cli(); // Disable interrupts
  EIMSK &= 0x00; // Mask external interrupt
  sei(); // Enable interrupts  
  
  context.buttonInput = true;
}

//---------------------------------------------------------------

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT);

  pinMode(POT_CHARGE_PIN, INPUT);
  pinMode(POT_DISCHARGE_PIN, INPUT);

  //initializing context
  context.state = &state;
  context.rate = 0;
  context.buttonInput = false;
  context.quantity = 0;
  context.servoPosition = 0;

  //initializing state as idle
  idle(&state);

  //initializing servo
  servo.attach(SERVO_PIN);
  servo.write(context.servoPosition);

  Serial.begin(9600);

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    interruptHandler,
    RISING
  );
}

void loop() {
  //update quantity based on old state and  old rate
  state.updateQuantity(&context);

  //update state
  state.makeTransition(&context);

  //update led based on new state
  state.setLED();
  //update rate based on new state
  state.updateRate(&context);
  //update rate based on new state and new quantity
  state.setServoPosition(&context);
  servo.write(context.servoPosition);
  //update buzzer based on new state and new quantity
  state.setBuzzer(&context);

  printStats(&context);

  //reset button input
  context.buttonInput = false;
  // Unmask external interrupts
  cli(); // Disable interrupts
  EIMSK |= 0x03; // Unmask external interrupts
  sei(); // Enable interrupts

  delay(DELAY_INTERVAL);
}
