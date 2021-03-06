# Particle Electron

This is the code for the communication module Particle Electron. This uses arduino based code which is very similar to C. The particle Electron online IDE was used to develop and compile this code(link found below). The electron is the main communication between the ADCS and the web application, and is currently the only way to communicate to it. First the web application communicates to the electron, then the electron relays this message to the arduino. The same process works in the reverse so that the user can communicate to the ADCS. Look in the code for comments to see the specifics of the communication, or scroll to the bottom to view it in more detail.<br>
```
firmware.bin - This is the compiled binary file<br>
adcsv1.ino - This is the arduino code (fully commented)
```

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

The hardware that this uses is the Particle Electron. You also need to download the software to upload the binary file to the electron.<br>
The software is from the Particle Electron site and can be found <a href="https://docs.particle.io/guide/tools-and-features/cli/electron/">here</a>.

### Installing

Compile arduino code from Particle's development website into a binary file, and make sure that you have the software that can upload the file to the electron. Then go to the directory that the binary file is in and type:
```
particle flash --serial [your binary file]
```
Hold the "mode" button on the electron to make the LED start flashing blue, indicating that code can be uploaded.

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

Particle Electron online IDE - <a href="https://build.particle.io">Here</a>

## Authors

Slushyman

## Acknowledgments

Pebo & Stephen

## Communication
### Application Communication
When calling functions from the applicaion to the electron, use this format:
```
https://api.particle.io/v1/devices/[device ID]/[function name]?access_token=[access token value]
method: POST
args: [data to send to electron]
```
When calling variables from the application to the electron, use the format:
```
https://api.particle.io/v1/devices/[device ID]/[variable name]?access_token=[access token value]
method: GET
```
These are the current functions:<br>
  *cntrl_adcs - takes a string that will be used to control the adcs, current strings are: open, close, check, chargeon, chargeoff<br>
  updt_adcs - takes a string delimited by semi-colons that says what the drone size and battery is*<br><br>

These are the current variables:<br>
  *string state - displays the state of the ADCS, current states are: Closed, Closing, Opened, Opening, Limbo<br>
  int     batt - displays the value of the battery based on a number 0-10<br>
  int     error - displays how many errors there are<br>
  string  errno - list of error codes that are delimited by semi-colons, the codes are 4 digit format (0259) and correlate to an error.txt file<br>
  int     warn - displays how many warnings there are<br>
  string  warnno - list of warning codes that are delimited by semi-colons, the codes are 4 digit format (0259) and correlate to a warning.txt file<br>
  string  sig - displays the signal strength of the electron, those ratings are: Excellent, Good, Ok, Poor, Non-Existent<br>
  int     drnPos - displays if the position of the drone is good, 1 means the drone is seen, 0 means to reposition<br>
  int     charge - displays if the drone is charging, 1 means charging, 0 means not charging*
### Arduino Communication
The electron sends information through it's serial ports at a 9600 baud rate. After each message it sends a '\n' character to indicate that the message is finished. It also recieves data in a similar way and expects a '\n' to indicate that the message is finished.<br>
#### Different signals can be sent from the electron such as:
COMMAND - controls the ADCS<br>
The format it sends:<br>
```
COMMAND OPEN
```
Commands:<br>
  *OPEN - This will send a command to open the ADCS<br>
  CLOSE - This will send a command to close the ADCS<br>
  CHECK - This will send a command to check if the drone is in a good position to charge<br>
  CHARGE ON - This will send a command to charge the drone<br>
  CHARGE OFF - This will send a command to stop charging the drone*<br><br>
UPDATE - updates drone information for the ADCS
```
UPDATE DRONE SIZE 12
```
Updates:<br>
  *DRONE_SIZE - will send the drone size in cm, the number after drone_size will be the size<br>
  BATTERY_SIZE - will send the battery size, the number after battery_size will be the size<br>
  VOLTAGE - asks to get the voltage of the battery*

#### Different signals can be recieved to the electron such as:
COMMAND - controls the ADCS<br>
The format it recieves:<br>
```
ACK COMMAND OPEN
```
Commands:
  *OPEN - Tells the ADCS that it is open<br>
  CLOSE - Tells the ADCS that it is closed<br>
  CHECK GOOD - Tells the ADCS that the drone is in a good position<br>
  CHECK BAD - Tells the ADCS that the drone is in a bad position<br>
  CHARGE ON - Tells the ADCS that it has started charging.<br>
  CHARGE OFF - Tells the ADCS that is has stopped charging*<br><br>
UPDATE - updates status of ADCS<br>
The format it recieves:<br>
```
ACK UPDATE VOLTAGE 90%
```
Update:
 *VOLTAGE - takes a string value of the voltage and gives it to the user*<br><br>
ERROR - relays error messages to the user, set the specific number to error in the website txt file
```
ERROR ADD 1234
```
Error:<br>
  *ADD - Adds an error so the user can see it<br>
  RESOLVE - Removes the error*<br><br>
WARNING - relays warning messages to the user
```
WARNING ADD 1234
```
Warning:<br>
  *ADD - Adds a warning so the user can see it<br>
  RESOLVE - Removes the warning*<br>
