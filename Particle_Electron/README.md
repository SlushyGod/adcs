# Particle Electron

This is the code for the communication module Particle Electron. This uses arduino based code which is very similar to C. The particle Electron online IDE was used to develop and compile this code(link found below). The electron is the main communication between the ADCS and the web application, and is currently the only way to communicate to it. Look in the code for comments on how to communicate to the electron, or scroll to the bottom to see how the ways to communicate to it.<br>
firmware.bin - This is the compiled binary file<br>
adcsv1.ino - This is the arduino code (fully commented)

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
