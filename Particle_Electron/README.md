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
When calling functions from the application to the electron then you must use the format:
```
FORMAT
```
When calling variables from the application to the electron then you must use the format:
```
FORMAT
```
These are the current functions:
These are the current variables:
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
  CHARGE OFF - This will send a command to stop charging the drone*<br>
UPDATE - updates drone information for the ADCS
```
UPDATE DRONE SIZE 12
```
Updates:<br>
  *DRONE_SIZE<br>
  BATTERY_SIZE*

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
  CHARGE OFF - Tells the ADCS that is has stopped charging*<br>
ERROR - relays error messages to the user, set the specific number to error in the website txt file
```
ERROR ADD 1234
```
Error:<br>
  *ADD - Adds an error so the user can see it<br>
  RESOLVE - Removes the error*<br>
WARNING - relays warning messages to the user
```
WARNING ADD 1234
```
Warning:<br>
  *ADD - Adds a warning so the user can see it<br>
  RESOLVE - Removes the warning*<br>
