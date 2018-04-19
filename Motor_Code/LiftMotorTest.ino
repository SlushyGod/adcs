int Relay_One = 13;  //Relay one pin
int Relay_Two = 12;  //Relay two pin 
int UpButton = 2;    //lift up 
int DownButton = 4;  //lift down
int Motor1 =3;       //motor pin up 
int Motor2 = 5;      //motor pin down

bool R = true;
bool Rs = true;
bool L =true;
bool Ls = true;

void setup()
{
 Serial.begin(9600);
 pinMode(Relay_One, OUTPUT);
 pinMode (Relay_Two, OUTPUT);
 pinMode (UpButton, INPUT);
 pinMode (DownButton, INPUT);
 pinMode (Motor1, OUTPUT);
 pinMode (Motor2, OUTPUT);
}

void loop()
{

  /*
   THIS IS LIFT UP CODE
   */
   if (digitalRead(UpButton) == HIGH) {
      
      digitalWrite(Motor1, HIGH);
      digitalWrite (Relay_One, LOW);
      digitalWrite (Relay_Two, HIGH); //TESTING
      if (R) Serial.println("Raising up");
      R = false;
      Rs = true;
      } 
      
      else {
      
      digitalWrite(Motor1, LOW); //turn off motor
      digitalWrite (Relay_One, HIGH);  //turn off relay
      digitalWrite (Relay_Two, HIGH); //TESTING
      if (Rs) Serial.println("Raising stopped");
      Rs = false;
      R = true;
    }
  
  /*
   * THIS IS LIFT DOWN CODE
   */
  
  if (digitalRead(DownButton) == HIGH) {
   
      digitalWrite(Motor2, HIGH);
      digitalWrite (Relay_Two, LOW);
      digitalWrite (Relay_One, HIGH); //TESTING
      if (L) Serial.println("Lowering");
      L = false;
      Ls = true;
      } 
      
      else {
      
      digitalWrite(Motor2, LOW); //turn off motor
      digitalWrite (Relay_Two, HIGH);  //turn off relay
      digitalWrite (Relay_One, HIGH);//TESTING
      if (Ls) Serial.println("Lowering stopped");
      Ls = false;
      L = true;
  
    }
}

