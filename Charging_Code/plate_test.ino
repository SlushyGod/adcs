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

void setup()
{
  Serial.begin(9600);
  for (int i = 1; i <= 16; i++)
  {
    pinMode(i+21,OUTPUT);
    digitalWrite(i+21,OFF);
  }
  pinMode(VOLT_M,OUTPUT);
  digitalWrite(VOLT_M,OFF);
  Serial.println("All plates are negative.");
  Serial.println("The voltmeter is active for checking mode.\n");
}

void loop()
{
  if (once)
  {
    Serial.println("Checking for contacts.....");
    value = check();
    if (value > 2) Serial.println("Check failed. Possible contact bridge.\n");
    else if (value < 2) Serial.println("Check failed. Not enough contacts found.\n");
    else 
    {
      Serial.println("Check passed. Clear to charge!");
      digitalWrite(VOLT_M,ON);
      Serial.println("The voltmeter is inactive. Switched to charging mode.");
      digitalWrite(conts[0]+21,ON);
      Serial.println("Plate #"+String(conts[0])+" is positive.");
      Serial.println("Plate #"+String(conts[1])+" is negative.\n");
    }
    once = false;
  }
}

int check()
{
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




