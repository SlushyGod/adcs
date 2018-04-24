#include <stdlib.h>

/*Laser Sensor Stuff*/
#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

bool fail = false;        // Determines if sensor setup was successful.
int max_height = 710;     // Default maximum height.
int min_height = 300;     // Default minimum height.
int ledPin = 13;          // LED port number.
int range = 0;            // Live range of height.
byte b1, b2;              // Bytes for conversion to an integer.

/* LIFT AND DOOR VARIABLES*/
#define LIFT_UP       39  // Raises the lift.
#define LIFT_DOWN     40  // Lowers the lift.
#define L_DOOR_OPEN   41  // Opens the left door.
#define L_DOOR_CLOSE  42  // Closes the right door.
#define R_DOOR_OPEN   43  // Opens the left door.
#define R_DOOR_CLOSE  44  // Closes the right door.

int CLOSE_FSR = A2;   // Stops the left door from closing.
int R_OPEN_FSR = A3;  // Stops the right door from opening.
int L_OPEN_FSR = A4;  // Stops the left door from opening.
int forceR, forceL;

 /*
Switches between checking and charging mode.
When the relay is high (off), the voltage sensor is active.
When the relay is low (on), the voltage sensor is inactive. 
*/
#define VOLT_M 38

// Defines the On/Off commands for the relays.
#define ON  LOW
#define OFF HIGH

int value = 10;   // Number of contacts found by check().
int conts[10];    // Holds plates with contacts on them.
char* voltage;   // Battery percentage.

const size_t READ_BUFF_SIZE = 64;
char read_buff[READ_BUFF_SIZE];
size_t buffer_i = 0;

void setup() {
  Serial.begin(9600);
  
  // Lift Setup Code
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin, LOW);

  for (int i = 39; i <= 44; i++)
  {
    pinMode(i,OUTPUT);
    digitalWrite(i,OFF);
  }
  
  // Laser Sensor Setup Code
  // Waits until serial port opens for native USB devices.
  while (!Serial) delay(1);
  
  // Checks if sensor object failed to be created.
  if (!lox.begin()) fail = true;

  // Charging plate setup.
  for (int i = 1; i <= 16; i++)
  {
    pinMode(i+21,OUTPUT);
    digitalWrite(i+21,OFF);
  }
  pinMode(VOLT_M,OUTPUT);
  digitalWrite(VOLT_M,OFF);
}

void loop() {
  /*Wait for data from serial1 and put that into a string, then a char array*/
	while(Serial.available()) {
		if (buffer_i < READ_BUFF_SIZE) {
			char c = Serial.read();
			if (c != '\n') {
				// Add character to buffer
				read_buff[buffer_i++] = c;
			}
			else {
				// End of line character found, process line
				read_buff[buffer_i] = 0;
				process_buffer();
				buffer_i = 0;
			}
		}
		else {
		  /*This shouldn't happen, but it checks for a buffer overflow just in case*/
			Serial.println("Buffer Overflow: Message longer than 64 characters.");
			buffer_i = 0;
		}
	}

}

/*Takes an integer that relates to an error and passes it to the communication module saying to add*/
void send_error(int errorno){
  Serial.print("ERROR ");
  Serial.print(errorno);
  Serial.print(" ADD");
  Serial.print('\n');
}

/*Takes an integer that relates to an error and passes it to the communication module saying to resolve*/
void fix_error(int errorno){
  Serial.print("ERROR ");
  Serial.print(errorno);
  Serial.print(" RESOLVE");
  Serial.print('\n');
}

/*Takes an integer that relates to a warning and passes it to the communication module saying to add*/
void send_warning(int warrno){
  Serial.print("WARNING ");
  Serial.print(warrno);
  Serial.print(" ADD");
  Serial.print('\n');
}

/*Takes an integer that relates to an error and passes it to the communication module saying to resolve*/
void fix_warning(int warrno){
  Serial.print("WARNING ");
  Serial.print(warrno);
  Serial.print(" RESOLVE");
  Serial.print('\n');
}

/*Take a double char pointer and deallocate the memory*/
void deallocate_mem(char **tokens){
    free(tokens);
}

/*This function will turn the buffer that is read in, turn them into tokens, then send them to the correct function*/
void process_buffer(){
  char *separator=" ";
  char *tokens;
  char **token_array;
  int token_i;
  
  token_i=0;
  token_array=(char **) malloc(10*sizeof(char *));//max of 10 tokens
  tokens=strtok(read_buff,separator);//seperate the string by spaces
  token_array[token_i++]=tokens;
  while(tokens!=0){//loop through and put all the tokens into the array
      tokens = strtok(NULL,separator);
      token_array[token_i++]=tokens;
  }
  token_array[token_i]=(char *)NULL; //last value is a pointer to null
  
  /*Look to see what the first token was*/
  if(strcmp(token_array[0],"COMMAND")==0){
    process_command(token_array);
  }
  if(strcmp(token_array[0],"UPDATE")==0){
    process_update(token_array);
  }
  /*Add other things you are looking for*/
  
  memset(read_buff, 0, READ_BUFF_SIZE);//clear the buffer
}

/*This will look to see what update is sent and send it to the correct function*/
void process_update(char **tokens){
  if(strcmp(tokens[1],"DRONE_SIZE")==0){
    int drone_size=(int)tokens[2];
    update_size(drone_size);
  }
  if(strcmp(tokens[1],"BATTERY_SIZE")==0){
    double battery_size=strtod(tokens[2],NULL);
    update_battery(battery_size);
  }
  deallocate_mem(tokens);
}

/*An integer variable will be passed that represents the drone wingspan*/
void update_size(int size){
  /*Put code here that updates the drone size wingspan*/
}

/*A double variable will be passed that represents the voltage of the battery*/
void update_battery(double battery){
  /*Put code here that updates the battery voltage*/
}

/*This will look to see what command is sent and send it to the correct function*/
void process_command(char **tokens){
  int check_val;
  if(strcmp(tokens[1],"OPEN")==0){
    command_open();
  }
  if(strcmp(tokens[1],"CLOSE")==0){
    command_close();
  }
  if(strcmp(tokens[1],"CHECK")==0){
    check_val = command_check();
    if (check_val == 2) {
      ack_command("check good");//ack back if the check succeeds.
      delay(100);
      ack_command("check good");//ack back if the check succeeds.
      delay(100);
    }
    else {
      ack_command("check bad");//ack back if the check fails.
      delay(100);
      ack_command("check bad");//ack back if the check fails.
      delay(100);
    }
  }
  if(strcmp(tokens[1],"CHARGE")==0){
    if(strcmp(tokens[2],"ON")==0){
      check_val = command_check();
      if (check_val == 2) {
        ack_command("check good");//ack back if the check succeeds.
        delay(100);
        ack_command("check good");//ack back if the check succeeds.
        delay(100);
        digitalWrite(VOLT_M,ON);
        digitalWrite(conts[0]+21,ON);
        
        ack_command("charging");//ack back when adcs is charging.
        delay(100);
        ack_command("charging");//ack back when adcs is charging.
        delay(100);
      }
      else {
        ack_command("check bad");//ack back if the check fails.
        delay(100);
        ack_command("check bad");//ack back if the check fails.
        delay(100);
      }
    }
    
    else if(strcmp(tokens[2],"OFF")==0){
      for (int i = 1; i <= 16; i++) digitalWrite(i+21,OFF);
      digitalWrite(VOLT_M,OFF);
      
      ack_command("not charging");//ack back when adcs isn't charging.
      delay(100);
      ack_command("not charging");//ack back when adcs isn't charging.
      delay(100);
    }
  }
  deallocate_mem(tokens);
}

/*Function to check for contacts*/
int command_check(){
  digitalWrite(VOLT_M,OFF);
  float sensorValue;
  float avgVoltage = 0;
  int contact = 0;
  int R1 = 30000;
  int R2 = 7500;

  for (int i = 1; i <= 16; i++)
  {
    digitalWrite(i+21,ON);
    delay(50);
    sensorValue = analogRead(A0);
    if (sensorValue > 100)
    {
      conts[contact] = i;
      contact++;
      avgVoltage += sensorValue;
    }
    digitalWrite(i+21,OFF);
  }

  if (contact > 0)
  {
    avgVoltage = avgVoltage/contact;
    avgVoltage = .026*avgVoltage;
    battPercent(avgVoltage);
    ack_command("battery update");//ack back when voltage is found.
    delay(100);
    ack_command("battery update");//ack back when voltage is found.
    delay(100);
  }
  
  return contact;
}

/*Determines battery percentage*/
void battPercent(float volt)
{
  if (volt > 12.7) voltage = "100%";
  else if (volt > 12.5) voltage = "90%";
  else if (volt > 12.42) voltage = "80%";
  else if (volt > 12.32) voltage = "70%";
  else if (volt > 12.2) voltage = "60%";
  else if (volt > 12.06) voltage = "50%";
  else if (volt > 11.9) voltage = "40%";
  else if (volt > 11.79) voltage = "30%";
  else if (volt > 11.58) voltage = "20%";
  else if (volt > 11.31) voltage = "10%";
  else voltage = "0%";
}

/*Function to open the ADCS*/
void command_open(){
  digitalWrite(ledPin, HIGH);
  doorOpen();
  doorWaitOpen();
  
  liftUp();
  liftWait("up");
  liftStop();
  digitalWrite(ledPin, LOW);

  ack_command("open");//ack back if the command succeeds
  delay(100);
  ack_command("open");//ack back if the command succeeds
  delay(100);
}

/*Function to close the ADCS*/
void command_close(){
  digitalWrite(ledPin, HIGH);
  liftDown();
  liftWait("down");
  liftStop();

  doorClose();
  doorWaitClose();
  digitalWrite(ledPin, LOW);

  ack_command("close");//ack back if the command succeeds
  delay(100);
  ack_command("close");//ack back if the command succeeds
  delay(100);
}

/*Function that opens the doors*/
void doorOpen(){
  digitalWrite(L_DOOR_OPEN,ON);
  digitalWrite(R_DOOR_OPEN,ON);
  digitalWrite(L_DOOR_CLOSE,OFF);
  digitalWrite(R_DOOR_CLOSE,OFF);
}

/*Function that closes the doors*/
void doorClose(){
  digitalWrite(L_DOOR_OPEN,OFF);
  digitalWrite(R_DOOR_OPEN,OFF);
  digitalWrite(L_DOOR_CLOSE,ON);
  digitalWrite(R_DOOR_CLOSE,ON);
}

void doorWaitOpen(){
  while (1)
  {
    forceR = analogRead(R_OPEN_FSR);
    forceL = analogRead(L_OPEN_FSR);
    if (forceR > 800) digitalWrite(R_DOOR_OPEN,OFF);
    if (forceL > 800) digitalWrite(L_DOOR_OPEN,OFF);
    if ((forceR > 800) && (forceL > 800)) break;
    delay(50);
  }
}

void doorWaitClose(){
  while (1)
  {
    forceL = analogRead(CLOSE_FSR);
    if (forceL > 800)
    {
      digitalWrite(L_DOOR_CLOSE,OFF);
      digitalWrite(R_DOOR_CLOSE,OFF);
      break;
    }
    delay(50);
  }
}

/*Function that raises the lift*/
void liftUp(){
  digitalWrite(LIFT_UP,ON);
  digitalWrite(LIFT_DOWN,OFF);
}

/*Function that lowers the lift*/
void liftDown(){
  digitalWrite(LIFT_UP,OFF);
  digitalWrite(LIFT_DOWN,ON);
}

/*Function that stops lift actions*/
void liftStop(){
  digitalWrite(LIFT_UP,OFF);
  digitalWrite(LIFT_DOWN,OFF);
}

void liftWait(String phrase){
  while(1)
  {
    VL53L0X_RangingMeasurementData_t measure; 

    // Gets measurement information.
    lox.rangingTest(&measure, false);
    
    // Executes if object is in range.
    if (measure.RangeStatus != 4)
    {
      // Gets the current height.
      range = measure.RangeMilliMeter;
  
      // Runs if maximum height is exceeded.
      if ((range >= max_height) && (phrase == "up")) return;

      // Runs if minimum height is exceeded.
      if ((range <= min_height) && (phrase == "down")) return;
    }
  }
}

/*Send a string to this command to ack back that the command succeded
Ack:open;close
*/
void ack_command(String command){
  if(command=="open"){
    Serial.write("ACK COMMAND OPEN");
    delay(100);
    Serial.write('\n');
  }
  if(command=="close"){
    Serial.write("ACK COMMAND CLOSE");
    delay(100);
    Serial.write('\n');
  }
  if(command=="check good"){
    Serial.write("ACK COMMAND CHECK GOOD");
    delay(100);
    Serial.write('\n');
  }
  if(command=="check bad"){
    Serial.write("ACK COMMAND CHECK BAD");
    delay(100);
    Serial.write('\n');
  }
  if(command=="charging"){
    Serial.write("ACK COMMAND CHARGE ON");
    delay(100);
    Serial.write('\n');
  }
  if(command=="not charging"){
    Serial.write("ACK COMMAND CHARGE OFF");
    delay(100);
    Serial.write('\n');
  }
  if(command=="battery update"){
    Serial.write("ACK UPDATE VOLTAGE ");
    Serial.write(voltage);
    delay(100);
    Serial.write('\n');
  }
}
