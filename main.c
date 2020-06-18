/* main.c
To be used with producer.h and consumer.h.
Version 1.0 -- Steven Wheeler
Target operating system: Linux
Last updated: 25/11/2019
w012922i@student.staffs.ac.uk

This program is a simulation of a printer queue with a producer to create jobs and a consumer to destroy jobs.
This program WILL require the producer.h and consumer.h files to run.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h> // This file does not exist in Windows. Linux only!
#include <sys/shm.h> // This file does not exist in Windows. Linux only!
#include <unistd.h> // This file does not exist in Windows. Linux only!
#include "producer.h"
#include "consumer.h"

int main(int argc, char *argv[])
{
	int i;
	char programMode[2] = {"NA"}; //Program mode is set to NA if not defined, C if consumer, P if producer.
	int maxCreatePerCycle = 0;
	int maxDeletePerCycle = 0;
	int maxSystem = 0;
	int cycles = 0;

	printf("\n Simulation of printer queue with Producer and Consumer.\n  C Programming assignment\n  Steven Wheeler\n  w012922i@student.staffs.ac.uk\n\n--PROGRAM STARTING--\n");

	for (i = 1; i < argc; i++){ //RUN THROUGH ARGUMENTS TO FIND MODE
		// FIND PROGRAM MODE
		if (strcmp(argv[i], "-producer") == 0) {
			if (strcmp(programMode, "NA") != 0) {
				printf("\n    [ WARNING ]: Program mode is already set, ignoring %s flag.", argv[i]);
			}
			else {
				printf("\n    [ MODE ]: PROGRAM LAUNCHING IN PRODUCER MODE");
				strcpy(programMode, "P");
			}
		}
		else if (strcmp(argv[i], "-consumer") == 0) {
			if (strcmp(programMode, "NA") != 0) {
				printf("\n    [ WARNING ]: Program mode is already set, ignoring %s flag.", argv[i]);
			}
			else {
				printf("\n    [ MODE ]: PROGRAM LAUNCHING IN CONSUMER MODE");
				strcpy(programMode, "C");
			}
		}
	}
	if (strcmp(programMode, "NA") == 0) { //CHECK THAT PROGRAM MODE IS DEFINED, IF NOT CLOSE
		printf("\n    [ FATAL ERROR ]: NO PROGRAM MODE WAS SUPPLIED, PLEASE RUN THE PROGRAM WITH THE -producer OR -consumer ARGUMENT.\n\n");
		exit(0);
	}

	for (i=1;i < argc;i++) { //RUN THROUGH ARGUMENTS TO SET SETTINGS
		if (strcmp(programMode, "P") == 0) { //IF SYSTEM IS RUNNING IN PRODUCER MODE
			// MAX CREATED BY PRODUCER PER CYCLE
			if (strcmp(argv[i], "-maxcreatecycle") == 0) {
				if (atoi(argv[i + 1]) > 0) {
					printf("\n    [ INFO ]: Max jobs created by producer per cycle set to %s", argv[i + 1]);
					maxCreatePerCycle = atoi(argv[i + 1]);
				}
				else {
					printf("\n    [ WARNING ]: -maxcreatecycle requires a numerical input, such as '-maxcreatecycle 20'. Continuing with default max of 2.");
					maxCreatePerCycle = 2;
				}
			}
			// MAX FOR SYSTEM
			else if (strcmp(argv[i], "-maxsystem") == 0) {
				if (atoi(argv[i + 1]) > 500) {
					printf("\n    [ WARNING ]: -maxsystem cannot be above 500. Continuing with default max of 50.");
					maxSystem = 50;
				}
				else if (atoi(argv[i + 1]) > 0 && atoi(argv[i + 1]) <= 500) {
					printf("\n    [ INFO ]: Max jobs for system set to %s", argv[i + 1]);
					maxSystem = atoi(argv[i + 1]);
				}
				else {
					printf("\n    [ WARNING ]: -maxsystem requires a numerical input, such as '-maxsystem 100'. Continuing with default max of 50.");
					maxSystem = 50;
				}
			}
			// WARN NON PRODUCER ARGUMENTS
			if (strcmp(argv[i], "-maxdeletecycle") == 0) {
				printf("\n    [ WARNING ]: -maxdeletecycle is a consumer mode command. Ignoring -maxdeletecycle flag.");
			}
		}
		if (strcmp(programMode, "C") == 0) { //IF SYSTEM IS RUNNING IN CONSUMER MODE
			// MAX DELETED BY CONSUMER PER CYCLE
			if (strcmp(argv[i], "-maxdeletecycle") == 0) {
				if (atoi(argv[i + 1]) > 0) {
					printf("\n    [ INFO ]: Max jobs deleted by consumer per cycle set to %s", argv[i + 1]);
					maxDeletePerCycle = atoi(argv[i + 1]);
				}
				else {
					printf("\n    [ WARNING ]: -maxdeletecycle requires a numerical input, such as '-maxdeletecycle 20'. Continuing with default max of 2.");
					maxDeletePerCycle = 2;
				}
			}
			// WARN NON CONSUMER ARGUMENTS
			if (strcmp(argv[i], "-maxcreatecycle") == 0) {
				printf("\n    [ WARNING ]: -maxcreatecycle is a producer mode command. Ignoring -maxcreatecycle flag.");
			}
			if (strcmp(argv[i], "-maxsystem") == 0) {
				printf("\n    [ WARNING ]: -maxsystem is a producer mode command. Ignoring -maxsystem flag.");
			}
		}
	}
	if (strcmp(programMode, "P") == 0) {// IF COMMAND WASNT SPECIFIED
		if (maxCreatePerCycle == 0) { //DEFAULT MAXCREATE ARGUMENT
			printf("\n    [ INFO ]: No max jobs created by producer per cycle was specified. Defaulting to 2.");
			maxCreatePerCycle = 2;
		}
		if (maxSystem == 0) { //DEFAULT MAXSYSTEM ARGUMENT
			printf("\n    [ INFO ]: No max jobs for system was specified. Defaulting to 50.");
			maxSystem = 50;
		}
	}
	if (strcmp(programMode, "C") == 0) {// IF COMMAND WASNT SPECIFIED
		if (maxDeletePerCycle == 0) { //DEFAULT MAXDELETE ARGUMENT
			printf("\n    [ INFO ]: No max jobs deleted by consumer per cycle was specified. Defaulting to 2.");
			maxDeletePerCycle = 2;
		}
	}
	
	if (strcmp(programMode, "P") == 0) { // PRODUCER PROCESS
		printf("\n    [ INITIALIZE ]: Starting producer...");
		waitForConsumer();
		producer(maxCreatePerCycle, maxSystem);
	}

	if (strcmp(programMode, "C") == 0) { // CONSUMER PROCESS
		printf("\n    [ INITIALIZE ]: Starting consumer...");
		lookForProducer();
		consumer(maxDeletePerCycle);
	}

	printf("\n\n"); // END OF PROGRAM
}