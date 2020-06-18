/* producer.h
To be used with main.c and consumer.h.
Version 1.0 -- Steven Wheeler
Target operating system: Linux
Last updated: 05/12/2019
w012922i@student.staffs.ac.uk

This program is a simulation of a printer queue with a producer to create jobs and a consumer to destroy jobs.
This file is a part of the program and is required by main.c to operate.
*/

#include <unistd.h> // This file does not exist in Windows. Linux only!
#include <time.h>

int consumerprocessfound = 0, cycles = -1, lock = 0, currentProcesses = 0, priority = 0, forkWaste = 0, lookingForSpace = 0, currentArrayEntry;
int i = 0, j = 0, h = 0;
char input[10];
int(*jobIDArray)[499]; // Yes I acknowledge this is a STUPID way to do this. I now know a much better way at the end of the project but to redo this would require rewriting the ENTIRE PROGRAM, and it works up to 500 so realistically someone would not be trying to run this program past 500 active forks unless they dont like their processor.
pid_t forkArray[1024];
int logcreated = 0;
char producerlogentry[128];

struct ProducerNode
{
	char message[128];
	struct ProducerNode* next;
};

void createproducerlog(struct ProducerNode** head_ref, char *msg)
{
	struct ProducerNode* new_node = (struct ProducerNode*) malloc(sizeof(struct ProducerNode));
	struct ProducerNode* last = *head_ref;
	strcpy(new_node->message, msg);
	new_node->next = NULL;
	if (*head_ref == NULL)
	{
		*head_ref = new_node;
		return;
	}
	while (last->next != NULL)
		last = last->next;
	last->next = new_node;
	return;
}

void displayproducerlog(struct ProducerNode* node)
{
	while (node != NULL)
	{
		printf("\n%s", node->message);
		node = node->next;
	}
}

int producer(int maxCreatePerCycle, int maxSystem) {
	srand(time(0));

	key_t jobidarray = ftok("jobidarray", 65);
	int idforjobidarray = shmget(jobidarray, sizeof(int)*40000000, 0666 | IPC_CREAT);
	jobIDArray = shmat(idforjobidarray, 0, 0);

	key_t turntrader = ftok("turntrader", 65);
	int idforturntrader = shmget(turntrader, 32, 0666 | IPC_CREAT);
	char* turn = (char*)shmat(idforturntrader, (void*)0, 0);
	sprintf(turn, "producer");

	key_t producerkill = ftok("kill", 65);
	int idforkill = shmget(producerkill, 32, 0666 | IPC_CREAT);
	char* killcommand = (char*)shmat(idforkill, (void*)0, 0);

	printf("\n    [ INFO ]: Starting cycles...\n"); // STARTING CYCLE ENGINE
	struct ProducerNode* producerlog = NULL;

	time_t producer_cur_time;
	char* producer_cur_t_string;
	char* producer_timestring;
	
	while (1 == 1) {
		if (strcmp(turn, "producer") == 0) {
			cycles++;
			if (cycles % 5 == 0 && cycles != 0) {
				lock = 1;
				producer_cur_time = time(NULL);
				producer_cur_t_string = ctime(&producer_cur_time);
				sprintf(producerlogentry, "    [ %.19s ]: Producer has completed %d cycles so far.", producer_cur_t_string, cycles);
				createproducerlog(&producerlog, producerlogentry);
				printf("\n%d CYCLES COMPLETED. Would you like to do another 5? <Y/N>\nPressing N will terminate the producer and consumer.\nYou can also press L to view the log.\n> ", cycles);
				while (lock == 1) {
					scanf("%s", input);
					if (strcmp(input, "y") == 0 | strcmp(input, "Y") == 0) {
						printf("\n    [ INFO ]: Continuing cycles...");
						producer_cur_time = time(NULL);
						producer_cur_t_string = ctime(&producer_cur_time);
						sprintf(producerlogentry, "    [ %.19s ]: User decided to do another 5 cycles.", producer_cur_t_string);
						createproducerlog(&producerlog, producerlogentry);
						lock = 0;
					}
					else if (strcmp(input, "n") == 0 | strcmp(input, "N") == 0) {
						printf("\n    [ INFO ]: Terminating processes..."); // TERMINATIONS
						sprintf(killcommand, "kill");
						printf("\n    [ TERMINATION ]: Termination request sent to consumer...");
						printf("\n    [ TERMINATION ]: Terminating producer. Goodbye!\n\n");
						exit(0);
					}
					else if (strcmp(input, "l") == 0 | strcmp(input, "L") == 0) {
						printf("\n    [ INFO ]: Displaying log...");
						sprintf(turn, "log");
						displayproducerlog(producerlog);
						printf("\n    [ INFO ]: End of log.\n");
						printf("\n%d CYCLES COMPLETED. Would you like to do another 5? <Y/N>\nPressing N will terminate the producer and consumer.\nYou can also press L to view the log.\n> ", cycles);
					}
					else {
						printf("EXPECTED AN INPUT OF Y OR N.\nYou can also press L to view the log.\n> ");
					}
				}
			}
			if (cycles != 0) {
				sleep(1);
			}
			printf("\n    [ INFO ]: Beginning cycle %d", cycles + 1);
			for (i = 1; i <= maxCreatePerCycle; i++) {
				lookingForSpace = 1;
				currentProcesses = 0;
				for (j = 0; j <= 500; j++)
				{
					if (jobIDArray[j][0] == 0 && lookingForSpace == 1) {
						currentArrayEntry = j;
						lookingForSpace = 0;
					}
				}

				for (h = 0; h <= 500; h++)
				{
					if (jobIDArray[h][0] != 0) {
						currentProcesses++;
					}
				}

				if (currentProcesses < maxSystem) {
					forkArray[currentProcesses] = fork();
					if (forkArray[currentProcesses] == 0) {
						while (1 == 1) {
							forkWaste++;
							forkWaste--;
						}
					}
					else if (forkArray[currentProcesses] > 0) {
						priority = (rand() % 10 + 1);
						printf("\n    [ CYCLE %d ]: Job created with PID: %d, priority level: %d", cycles + 1, forkArray[currentProcesses], priority);
						jobIDArray[currentArrayEntry][0] = forkArray[currentProcesses];
						jobIDArray[currentArrayEntry][1] = priority;
					}
				}
			}
			if (currentProcesses == maxSystem) {
				printf("\n    [ CYCLE %d ]: Max job limit for system reached.", cycles + 1);
			}
			printf("\n    [ CYCLE %d ]: Cycle completed, Active processes: %d\n", cycles + 1, currentProcesses);
			sprintf(turn, "consumer");
		}
	}
}

int waitForConsumer() {
	key_t producerwaiting = ftok("wait", 65);
	int id = shmget(producerwaiting, 32, 0666 | IPC_CREAT);
	char* str = (char*)shmat(id, (void*)0, 0);
	sprintf(str, "wait");
	printf("\n\n    [ INFO ]: Producer ready.");
	printf("\n    [ INFO ]: Standing by for a Consumer process to be launched.\n"); // READY TO RUN
	
	while (consumerprocessfound == 0) {
		if (strcmp(str, "here") == 0) {
			// IF CONSUMER FOUND:
			sprintf(str, "produceracknowledge");
			shmdt(str);
			printf("\n    [ INFO ]: Consumer process found.");
			consumerprocessfound = 1;
		}
	}
}