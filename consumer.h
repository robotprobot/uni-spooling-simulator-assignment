/* consumer.h
To be used with main.c and producer.h.
Version 1.0 -- Steven Wheeler
Target operating system: Linux
Last updated: 05/12/2019
w012922i@student.staffs.ac.uk

This program is a simulation of a printer queue with a producer to create jobs and a consumer to destroy jobs.
This file is a part of the program and is required by main.c to operate.
*/

#include <unistd.h> // This file does not exist in Windows. Linux only!
#include <signal.h>
#include <time.h>

int producerprocessfound = 0, produceracknowledgement = 0, currentJobArray, currentJobPriority, currentJobID, cycle;
int k = 0, l = 0, m = 0, n = 0;
int(*jobIDArray)[499];
char consumerlogentry[128];

struct ConsumerNode
{
	char message[128];
	struct ConsumerNode* next;
};

void createconsumerlog(struct ConsumerNode** head_ref, char* msg)
{
	struct ConsumerNode* new_node = (struct ConsumerNode*) malloc(sizeof(struct ConsumerNode));
	struct ConsumerNode* last = *head_ref;
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

void displayconsumerlog(struct ConsumerNode* node)
{
	while (node != NULL)
	{
		printf("\n%s", node->message);
		node = node->next;
	}
}

int consumer(int maxDeletePerCycle) {
	key_t jobidarray = ftok("jobidarray", 65);
	int idforjobidarray = shmget(jobidarray, sizeof(int)*40000000, 0666 | IPC_CREAT);
	jobIDArray = shmat(idforjobidarray, 0, 0);

	key_t turntrader = ftok("turntrader", 65);
	int idforturntrader = shmget(turntrader, 32, 0666 | IPC_CREAT);
	char* turn = (char*)shmat(idforturntrader, (void*)0, 0);

	key_t producerkill = ftok("kill", 65);
	int idforkill = shmget(producerkill, 32, 0666 | IPC_CREAT);
	char* killcommand = (char*)shmat(idforkill, (void*)0, 0);

	printf("\n    [ INFO ]: Starting cycles...\n");
	struct ConsumerNode* consumerlog = NULL;

	time_t consumer_cur_time;
	char* consumer_cur_t_string;
	char* consumer_timestring;

	while (1 == 1) {
		if (strcmp(turn, "log") == 0) {
			sprintf(turn, "producer");
			printf("\n    [ INFO ]: Displaying log...");
			displayconsumerlog(consumerlog);
			printf("\n    [ INFO ]: End of log.\n    [ INFO ]: Waiting for producer to decide next action...\n");

		}
		// AFTER DOING CYCLE DO THE FOLLOWING TO CHECK FOR TERMINATION REQUESTS FROM PRODUCER
		else if (strcmp(killcommand, "kill") == 0) {
			shmdt(killcommand);
			shmdt(turn);
			shmctl(idforkill, IPC_RMID, NULL);
			shmctl(idforturntrader, IPC_RMID, NULL);
			printf("\n    [ TERMINATION ]: Termination request recieved from producer...");
			printf("\n    [ TERMINATION ]: Clearing up system...");
			for (n = 1; n <= 500; n++) {
				// kill ALL forks
				currentJobID = jobIDArray[n][0];
				if (currentJobID != 0) {
					kill(currentJobID, SIGKILL);
				}
			}
			shmdt(jobIDArray);
			shmctl(idforjobidarray, IPC_RMID, NULL);
			printf("\n    [ TERMINATION ]: Terminating consumer. Goodbye!\n\n");
			exit(0);
		}
		if (strcmp(turn, "consumer") == 0) {
			cycle++;
			if (cycle != 0) {
				sleep(1);
			}
			printf("\n    [ INFO ]: Beginning cycle %d", cycle);
			m = 0;
			for (l = 1; l <= maxDeletePerCycle; l++) {
				// LOOK FOR AND DELETE JOBS
				currentJobArray = 0;
				currentJobPriority = 0;
				currentJobID = 0;
				for (k = 1; k <= 500; k++)
				{
					if (jobIDArray[k][1] > currentJobPriority) {
						currentJobArray = k;
						currentJobID = jobIDArray[k][0];
						currentJobPriority = jobIDArray[k][1];
					}
				}
				if (currentJobID == 0) {
					if (m == 0) {
						printf("\n    [ CYCLE %d ]: No jobs were found.", cycle);
					}
					m = 1;
				}
				else {
					printf("\n    [ CYCLE %d ]: Found job with id %d and priority %d.", cycle, currentJobID, currentJobPriority);
					jobIDArray[currentJobArray][0] = 0;
					jobIDArray[currentJobArray][1] = 0;
					kill(currentJobID, SIGKILL);
					consumer_cur_time = time(NULL);
					consumer_cur_t_string = ctime(&consumer_cur_time);
					sprintf(consumerlogentry, "    [ %.19s ]: Consumer successfully resolved job %d with priority %d", consumer_cur_t_string, currentJobID, currentJobPriority);
					createconsumerlog(&consumerlog, consumerlogentry);
					printf("\n    [ CYCLE %d ]: Job %d resolved.", cycle, currentJobID);
				}
			}
			printf("\n    [ CYCLE %d ]: Cycle completed.\n", cycle);
			sprintf(turn, "producer");
		}
	}
}

int lookForProducer() {
	key_t producerwaiting = ftok("wait", 65);
	int id = shmget(producerwaiting, 32, 0666 | IPC_CREAT);
	char* str = (char*)shmat(id, (void*)0, 0);
	printf("\n\n    [ INFO ]: Consumer ready.");
	printf("\n    [ INFO ]: Standing by for a Producer process to be launched.\n");

	while (producerprocessfound == 0) {
		if (strcmp(str, "wait") == 0) {
			// IF PRODUCER FOUND:
			sprintf(str, "here");
			producerprocessfound = 1;
		}
	}
	while (produceracknowledgement == 0) {
		if (strcmp(str, "produceracknowledge") == 0) {
			// IF PRODUCER ACKNOWLEDGES:
			shmdt(str);
			shmctl(id, IPC_RMID, NULL);
			printf("\n    [ INFO ]: Producer process found.");
			produceracknowledgement = 1;
		}
	}
}