#include <stdlib.h>

/*Laser Sensor Stuff*/
#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

bool fail = false;        // Determines if sensor setup was successful.
int max_height = 700;     // Default maximum height.
int min_height = 300;
int ledPin = 13;          // LED port number.
int range = 0;            // Live range of height.
byte b1, b2;              // Bytes for conversion to an integer.

/* LIFT AND DOOR VARIABLES
#define RED_D   pin for a relay
#define BLACK_D pin for a relay*/
#define RELAY1  2 //pin for a relay
#define RELAY2  4 //pin for a relay

#define L_ON    LOW
#define L_OFF   HIGH

 /*
Switches between checking and charging mode.
When the relay is high (off), the voltage sensor is active.
When the relay is low (on), the voltage sensor is inactive. 
*/
#define VOLT_M 38 

// Defines the On/Off commands for the relays.
#define ON  LOW
#define OFF HIGH

bool once = true; // Stops the scan from running continuosly.
int value = 10;   // Number of contacts found by check().
int conts[10];    // Holds plates with contacts on them.

const size_t READ_BUFF_SIZE = 64;
char read_buff[READ_BUFF_SIZE];
size_t buffer_i = 0;

void setup() {
  Serial.begin(9600);
  
  // Lift Setup Code
  pinMode(ledPin,OUTPUT);
  pinMode(RELAY1,OUTPUT);
  pinMode(RELAY2,OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(RELAY1,L_OFF);
  digitalWrite(RELAY2,L_OFF);
  
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
    if (check_val == 2) ack_command("check good");//ack back if the check succeeds.
    else ack_command("check bad");//ack back if the check fails.
  }
  if(strcmp(tokens[1],"CHARGE")==0){
    if(strcmp(tokens[2],"ON")==0){
      check_val = command_check();
      if (check_val == 2) {
        ack_command("check good");//ack back if the check succeeds.
        digitalWrite(VOLT_M,ON);
        digitalWrite(conts[0]+21,ON);
        ack_command("charging");//ack back when adcs is charging.
      }
      else ack_command("check bad");//ack back if the check fails.
    }
    
    else if(strcmp(tokens[2],"OFF")==0){
      for (int i = 1; i <= 16; i++) digitalWrite(i+21,OFF);
      digitalWrite(VOLT_M,OFF);
      ack_command("not charging");//ack back when adcs isn't charging.
    }
  }
  
  /*if(strcmp(tokens[1],"CHARGE OFF")==0){
    for (int i = 1; i <= 16; i++) digitalWrite(i+21,OFF);
    digitalWrite(VOLT_M,OFF);
    ack_command("not charging");//ack back when adcs isn't charging.
  }*/
  deallocate_mem(tokens);
}

/*Function to check for contacts*/
int command_check(){
  digitalWrite(VOLT_M,OFF);
  float sensorValue;
  int contact = 0;
  for (int i = 1; i <= 16; i++)
  {
    digitalWrite(i+21,ON);
    delay(50);
    sensorValue = analogRead(A0);
    if (sensorValue > 100)
    {
      conts[contact] = i;
      contact++;
    }
    digitalWrite(i+21,OFF);
  }
  return contact;
}

/*Function to open the ADCS*/
void command_open(){
  /*Put code here that opens ADCS*/
  /*
   * 1. Open the doors.
   *  -Sends signal to device that opens doors to begin.
   *  -Some way of waiting for doors to finish opening.
   *  -Sends signal to device that opens doors to stop.
   *  Actual Code:
   *    doorOpen();
   *    doorWaitOpen();
   *    doorStop();
   * 2. Raise the platform.
   *  -Sends signal to device that raises platform to begin.
   *  -While loop that checks the height of the platform.
   *  -Sends signal to device that raises platform to stop.
   *  Actual Code:
   *    liftUp();
   *    liftWaitUp();
   *    liftStop();
  */
  /*End of opening sequence*/
  digitalWrite(ledPin, HIGH);
  liftUp();
  /*liftWaitUp();*/
  liftWait("up");
  liftStop();
  digitalWrite(ledPin, LOW);
  Serial.println("Up");
  ack_command("open");//ack back if the command succeeds
}

/*Function to close the ADCS*/
void command_close(){
   /*Put code here that closes ADCS*/
  /*
   * 1. Lowers the platform.
   *  -Sends signal to device that lowers the platform to begin.
   *  -While loop that checks the height of the platform.
   *  -Sends signal to device that lowers platform to stop.
   *  Actual Code:
   *    liftDown();
   *    liftWaitDown();
   *    liftStop();
   * 2. Closes the doors.
   *  -Sends signal to device that closes the doors to begin.
   *  -Some way of waiting for doors to finish opening.
   *  -Sends signal to device that closes the doors to stop.
   *  Actual Code:
   *    doorClose();
   *    doorWaitClose();
   *    doorStop();
   
  */
  /*End of opening sequence*/
  digitalWrite(ledPin, HIGH);
  liftDown();
  /*liftWaitDown();*/
  liftWait("down");
  liftStop();
  digitalWrite(ledPin, LOW);
  Serial.println("Down");
   
  ack_command("close");//ack back if the command succeeds
}

/*Function that opens the doors*/
void doorOpen(){
  /*
   * Actual Code:
   *  digitalWrite(RED_D,LOW);
   *  digitalWrite(BLACK_D,HIGH);
  */
}

/*Function that closes the doors*/
void doorClose(){
  /*
   * Actual Code:
   *  digitalWrite(RED_D,HIGH);
   *  digitalWrite(BLACK_D,LOW);
  */
}

/*Function that stops door actions*/
void doorStop(){
  /*
   * Actual Code:
   *  digitalWrite(RED_D,LOW);
   *  digitalWrite(BLACK_D,LOW);
  */
}

void doorWaitOpen(){
  /*
   * Actual Code:
   * 
  */
}

void doorWaitClose(){
  /*
   * Actual Code:
   * 
  */
}

/*Function that raises the lift*/
void liftUp(){
  digitalWrite(RELAY1,L_ON);
  digitalWrite(RELAY2,L_OFF);
}

/*Function that lowers the lift*/
void liftDown(){
  digitalWrite(RELAY1,L_OFF);
  digitalWrite(RELAY2,L_ON);
}

/*Function that stops lift actions*/
void liftStop(){
  digitalWrite(RELAY1,L_OFF);
  digitalWrite(RELAY2,L_OFF);
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
    Serial.write('\n');
  }
  if(command=="close"){
    Serial.write("ACK COMMAND CLOSE");
    Serial.write('\n');
  }
  if(command=="check good"){
    Serial.write("ACK COMMAND CHECK GOOD");
    Serial.write('\n');
  }
  if(command=="check bad"){
    Serial.write("ACK COMMAND CHECK BAD");
    Serial.write('\n');
  }
  if(command=="charging"){
    Serial.write("ACK COMMAND CHARGE ON");
    Serial.write('\n');
  }
  if(command=="not charging"){
    Serial.write("ACK COMMAND CHARGE OFF");
    Serial.write('\n');
  }
}
