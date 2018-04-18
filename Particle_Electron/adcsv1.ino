#include "Particle.h"


// Constants
const size_t READ_BUFF_SIZE = 64;
const unsigned long BATTERY_TIMEOUT = 300000;//5 minutes
const unsigned long LIFT_TIMEOUT = 300000;//5 minutes

// Used for loop
void process_buffer();
char read_buff[READ_BUFF_SIZE];
size_t buffer_i = 0;//counter, size_t is unsigned number so you cant use negatives, but perfect for counting
unsigned long last_time = 0;
unsigned long lift_time = 0;
bool adcs_moving;

// Cloud variables
String adcs_state;//can be Opened, Closed, Opening, Closing
String adcs_errno;
String adcs_warnno;
String adcs_signal;
int adcs_battery;//any number from 0-10
int adcs_error;
int adcs_warning;
int drone_pos;//0 is drone neutral, 1 is drone is good, -1 is drone is bad
int adcs_charge;

int battery_life=0;
char inChar;

// Used for process buffer function
char buffer[80];
char *separator=" ";
char *tokens;
char **token_array;
int token_i;

// Testing
int led = D7;
char test;
void setup() {
    //use a get request with this format: to get variable values/v1/devices/{DEVICE_ID}/{VARIABLE
    Particle.variable("state",adcs_state);
    Particle.variable("batt",adcs_battery);
    Particle.variable("error",adcs_error);
    Particle.variable("errno",adcs_errno);
    Particle.variable("warn",adcs_warning);
    Particle.variable("warnno",adcs_warnno);
    Particle.variable("sig",adcs_signal);
    Particle.variable("drnPos",drone_pos);
    Particle.variable("charge",adcs_charge);
    //Particle.variable("bub",bubble);
    adcs_state="Closed";
    adcs_battery=9;
    adcs_error=0;
    adcs_warning=0;
    adcs_errno="1234;8790;3428;";
    adcs_warnno="0000;4563;7686;";
    drone_pos=0;
    adcs_charge=0;

    //functions
    Particle.function("cntrl_adcs",control_adcs);
    Particle.function("updt_adcs",update_adcs);

    //set the baud rate
    Serial.begin(9600);//tx
    Serial1.begin(9600);//rx
   // For good measure, let's also make sure both LEDs are off when we start:
    digitalWrite(led, LOW);

    pinMode(led, OUTPUT);
    digitalWrite(led,LOW);
    adcs_moving=false;
    check_signal();
}

void loop() {
    /*Wait for data from serial1 and put that into a string, then a char array*/
	while(Serial1.available()) {
		if (buffer_i < READ_BUFF_SIZE) {
			char c = Serial1.read();
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
	
	if((millis() - lift_time >= LIFT_TIMEOUT) && (adcs_moving==true)){
	    adcs_state="Limbo";
	}
	
	/*This is to check the battery life and signal strength every 5 minutes*/
	if (millis() - last_time >= BATTERY_TIMEOUT) {
		last_time = millis();
		//check_battery();
		check_signal();
	}
}

/*This function processes the buffer read from the serial port and splits it into sections that are delimited by a space
After these sections are split, it then calls the appropriate function and sends whatever data is required
*/
void process_buffer(){
    Serial.printf("%s\n",read_buff);
    token_i=0;
    token_array=(char **) malloc(10*sizeof(char *));//create a pointer to an array of pointers
    tokens=strtok(read_buff,separator);//seperate the string by spaces
    token_array[token_i++]=tokens;
    while(tokens!=0){//loop through and put all the tokens into the array
        tokens = strtok(NULL,separator);
        token_array[token_i++]=tokens;
    }
    token_array[token_i]=(char *)NULL;//last value is a pointer to null
    
    /*Check to see if its an ack or error*/
    if(strcmp(token_array[0],"ERROR")==0){
        error_send();
    }
    else if(strcmp(token_array[0],"WARNING")==0){
        warning_send();
    }
    else if(strcmp(token_array[0],"ACK")==0){
        /*Check to see what value the ack was*/
        if(strcmp(token_array[1],"COMMAND")==0){
            command_ack();
        }
        if(strcmp(token_array[1],"UPDATE")==0){
            update_ack();
        }
    }
    memset(read_buff, 0, READ_BUFF_SIZE);
}

void deallocate_mem(){
    free(token_array);
}

/*error_send will update the error variable as well as update the string that contains all of the error numbers,
all of the numbers will be delimited with a semi-colon(;)
How it looks:   ERROR 1234 ADD
                ERROR 1234 RESOLVE
Current warnings:
*/
void error_send(){
    String tmp_string;
    int temp_int;
    if(strcmp(token_array[2],"ADD")==0){
        adcs_error+=1;
        tmp_string=String(token_array[1]) + ";";
        adcs_errno+=tmp_string;
    }
    if(strcmp(token_array[2],"RESOLVE")==0){
        adcs_error-=1;
            temp_int=adcs_errno.indexOf(token_array[1]);
            adcs_errno=adcs_errno.substring(0,temp_int) + adcs_errno.substring(temp_int+5,adcs_errno.length());
    }
    deallocate_mem();
}

/*warning_send will update the warning variable as well as update the string that contains all of the warning numbers,
all of the numbers will be delimited with a semi-colon(;)
How it looks:   WARNING 1234 ADD
                WARNING 1234 RESOLVE
Current warnings:
*/
void warning_send(){
    String tmp_string;
    int temp_int;
    if(strcmp(token_array[2],"ADD")==0){
        adcs_warning+=1;
        tmp_string=String(token_array[1]) + ";";
        adcs_warnno+=tmp_string;
    }
    if(strcmp(token_array[2],"RESOLVE")==0){
        adcs_warning-=1;
            temp_int=adcs_warnno.indexOf(token_array[1]);
            adcs_warnno=adcs_warnno.substring(0,temp_int) + adcs_warnno.substring(temp_int+5,adcs_warnno.length());
    }
    deallocate_mem();
}

/*command_ack will update the variables that was changed when it initially sent a command
How it looks:   ACK COMMAND OPEN
Current commands:
OPEN
CLOSE
CHECK GOOD:BAD
CHARGE ON:OFF
*/
void command_ack(){
    Serial.print("Im in command");
    if(strcmp(token_array[2],"OPEN")==0){
        adcs_state="Opened";
        adcs_moving=false;
        Serial.print("Adcs is opened");
    }
    if(strcmp(token_array[2],"CLOSE")==0){
        adcs_state="Closed";
        adcs_moving=false;
        Serial.print("Adcs is closed");
    }
    if(strcmp(token_array[2],"CHECK")==0){
        if(strcmp(token_array[3],"GOOD")==0){
            drone_pos=1;
        }
        else{
            drone_pos=-1;
        }
    }
    if(strcmp(token_array[2],"CHARGE")==0){
        if(strcmp(token_array[3],"ON")==0){
            adcs_charge=1;
        }
        else{
            adcs_charge=0;
        }
    }
    deallocate_mem();
}

void update_ack(){
    deallocate_mem();
}

void check_battery(){
    //do all your battery stuff then update the variable
}

/*This function will check the signal strength of the particle electron
This function uses the built in rssi function and gets back a number, it then uses this number to classify the "strength"
*/
void check_signal(){
    int sig_strength;
    CellularSignal sig = Cellular.RSSI();
    sig_strength = sig.rssi;
    if (sig_strength < 0) {
        if (sig_strength >= -57){
            adcs_signal = "Excellent";
        }
        else if (sig_strength > -68){
            adcs_signal = "Good";
        }
        else if (sig_strength > -80){
            adcs_signal = "OK";
        }
        else if (sig_strength > -92){
            adcs_signal = "POOR";
        }
        else if (sig_strength > -104){
            adcs_signal = "Almost Non Existent";
        }
    }
    else{
        adcs_signal="Error producing signal";
    }
}

/*This function takes a string with a set delimiter, splits it up into tokens, then sends the values
How it looks:   UPDATE DRONE_SIZE 50
Send/Recieve?   Send
Current update variables:
DRONE_SIZE(given in cm)
BATTERY_SIZE(given in volts)
*/
int update_adcs(String word){
    if(word.length()>30){
        return -1;
    }
    else{
        /*divides hte string into tokens then sends the tokens to the uC*/
        char word_buff[30];
        char *tok;
        char *delim=";";//this will set the delimiter
        char **tok_array;
        int i=0;

        tok_array=(char **) malloc(10*sizeof(char *));//create a pointer to an array of pointers
        word.toCharArray(word_buff, word.length());//turn string into a char array
        
        /*Start spliting the string by the delimiter*/
        tok=strtok(word_buff,delim);
        tok_array[i++]=tok;
        while(tok!=0){//loop through and put all the tokens into the array
            tok = strtok(NULL,delim);
            tok_array[i++]=tok;
        }
        tok_array[i]=(char *)NULL;
        i=0;
        
        /*Send the updates to the uC*/
        while(tok_array[i]!=NULL){
            Serial.printf("UPDATE %s",tok_array[i]);
            i++;
        }
        free(tok_array);
        return 1;
    }
}

/*Takes values "open" or "close" then checks to see the status of the ADCS, if its good then it will tell the uC
How it looks:   COMMAND OPEN
Send/Recieve?   Send
Current commands:
OPEN
CLOSE
CHECK
CHARGE
*/
int control_adcs(String word) {
    if(word=="open"){
        if(adcs_state=="Closed" || adcs_state=="Limbo"){
            Serial1.write("COMMAND OPEN");
            Serial1.write('\n');
            digitalWrite(led,HIGH);
            lift_time=millis();//this is for timing out the adcs if it does open in time
            adcs_moving=true;
            adcs_state="Opening";
            return 1;
        }
        else{
            return -1;
        }
    }
    else if(word=="close"){
        if(adcs_state=="Opened" || adcs_state=="Limbo"){
            Serial1.write("COMMAND CLOSE");
            Serial1.write('\n');
            digitalWrite(led,LOW);
            lift_time=millis();//this is for timing out the adcs if it does open in time
            adcs_moving=true;
            adcs_state="Closing";
            return 1;
        }
        else{
            return -1;
        }

    }
    else if(word=="check"){
        if(adcs_moving==false && adcs_charge==0){
            Serial1.write("COMMAND CHECK");
            Serial1.write('\n');
        }
        else{
            return -1;
        }
    }
    else if(word=="charge"){
        if(drone_pos==1){
            Serial1.write("COMMAND CHARGE");
            Serial1.write('\n');
        }
    }
    else{
        return 0;
    }
}
