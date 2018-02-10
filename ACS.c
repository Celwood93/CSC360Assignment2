
/*
Cameron Elwood
Unversity of Victoria
CSC 360
V00152812
Assignment 2
*/

#define _POSIX_SOURCE
#define _BSD_SOURCE	
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>


struct customer_info{ 
    int user_id;
	int service_time;
	int arrival_time;
	int which_queue;
	int whereIn_queue;
	int server;
};

struct clerk_info{
	int clerk_id;
};

//mutex for all the queues
pthread_mutex_t queues_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t clerk1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerk1_convar = PTHREAD_COND_INITIALIZER;

pthread_mutex_t clerk2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerk2_convar = PTHREAD_COND_INITIALIZER;

//mutexs and convars for each queue
pthread_mutex_t q1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t q2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t q3_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t q4_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t q1_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q2_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q3_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q4_convar = PTHREAD_COND_INITIALIZER;

pthread_mutex_t q1_custConflict_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t q2_custConflict_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t q3_custConflict_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t q4_custConflict_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t q1_custConflict_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q2_custConflict_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q3_custConflict_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q4_custConflict_convar = PTHREAD_COND_INITIALIZER;

 
static struct timeval init_time;
int numCustomersGlob;
double overall_waiting_time; 
int queue_length[4]; 
int queue_countDown[4];

//add 2 more variables, c1 and c2, which will be integers. these will store values from 0-3 representing which queue they are working in.
int c1 = -1; //clerk1
int c2 = -1; //clerk2

int c1_busy = 0;// 0 not busy, 1 busy, 2 requesting customer
int c2_busy = 0;



int conVarCommands(int whichCommand, int currentQueue){
	int test = 1;
	if(whichCommand == 1){ //wait for a queue is 1
		if(currentQueue == 0){
			test = pthread_cond_wait(&q1_convar, &q1_mutex);
		}else if(currentQueue == 1){
			test = pthread_cond_wait(&q2_convar, &q2_mutex);
		}else if(currentQueue == 2){
			test = pthread_cond_wait(&q3_convar, &q3_mutex);
		}else if(currentQueue == 3){
			test = pthread_cond_wait(&q4_convar, &q4_mutex);
		}
		if(test != 0){
			printf("error cond wait");
			exit(0);
		}
	}else if(whichCommand == 2){ //convar broadcase to queues is 2 (for a specfic queued customers, from clerks)
		if(currentQueue == 0){
			test = pthread_cond_broadcast(&q1_convar);
		}else if(currentQueue == 1){
			test = pthread_cond_broadcast(&q2_convar);
		}else if(currentQueue == 2){
			test = pthread_cond_broadcast(&q3_convar);
		}else if(currentQueue == 3){
			test = pthread_cond_broadcast(&q4_convar);
		}
		if(test != 0){
			printf("error broadcase");
			exit(0);
		}
	}else if(whichCommand == 3){ //customer sends a signal to a potentially waiting clerk.
		if(currentQueue == 0){
			test = pthread_cond_broadcast(&q1_custConflict_convar);
		}else if(currentQueue == 1){
			test = pthread_cond_broadcast(&q2_custConflict_convar);
		}else if(currentQueue == 2){
			test = pthread_cond_broadcast(&q3_custConflict_convar);
		}else if(currentQueue == 3){
			test = pthread_cond_broadcast(&q4_custConflict_convar);
		}
		if(test != 0){
			printf("error broadcast");
			exit(0);
		}
	}
}


int lockUnlockMutex(int lockOrUnlock, int currentQueue){
	int test = 1;
	if(lockOrUnlock == 0){
		if(currentQueue == 0){
			test = pthread_mutex_lock(&q1_mutex);
		}else if(currentQueue == 1){
			test = pthread_mutex_lock(&q2_mutex);
		}else if(currentQueue == 2){
			test = pthread_mutex_lock(&q3_mutex);
		}else if(currentQueue == 3){
			test = pthread_mutex_lock(&q4_mutex);
		}
		if(test != 0){
			printf("error locking mutex");
			exit(0);
		}
	}else{
		if(currentQueue == 0){
			test = pthread_mutex_unlock(&q1_mutex);
		}else if(currentQueue == 1){
			test = pthread_mutex_unlock(&q2_mutex);
		}else if(currentQueue == 2){
			test = pthread_mutex_unlock(&q3_mutex);
		}else if(currentQueue == 3){
			test = pthread_mutex_unlock(&q4_mutex);
		}
		if(test != 0){
			printf("error unlocking mutex");
			exit(0);
		}
	}
	return 0;
}

int lockUnlockMutexClerks(int lockOrUnlock, int currentQueue){
	int test = 1;
	if(lockOrUnlock == 0){
		if(currentQueue == 0){
			test = pthread_mutex_lock(&q1_custConflict_mutex);
		}else if(currentQueue == 1){
			test = pthread_mutex_lock(&q2_custConflict_mutex);
		}else if(currentQueue == 2){
			test = pthread_mutex_lock(&q3_custConflict_mutex);
		}else if(currentQueue == 3){
			test = pthread_mutex_lock(&q4_custConflict_mutex);
		}
		if(test != 0){
			printf("error locking mutex");
			exit(0);
		}
	}else{
		if(currentQueue == 0){
			test = pthread_mutex_unlock(&q1_custConflict_mutex);
		}else if(currentQueue == 1){
			test = pthread_mutex_unlock(&q2_custConflict_mutex);
		}else if(currentQueue == 2){
			test = pthread_mutex_unlock(&q3_custConflict_mutex);
		}else if(currentQueue == 3){
			test = pthread_mutex_unlock(&q4_custConflict_mutex);
		}
		if(test != 0){
			printf("error unlocking mutex");
			exit(0);
		}
	}
	return 0;
}

//0 is clerk, 1 is customer
int checkQueue(int cuOrCl){
	if(queue_length[0] == 0 && queue_length[1]==0 && queue_length[2]==0 && queue_length[3]==0 && cuOrCl == 0){ //when the queues are all empty and the clerk is requesting return -1
		return -1;
	}
	if((queue_length[0] == queue_length[1] && queue_length[1]==queue_length[2] && queue_length[2] == queue_length[3])){
		int r = rand();
		r = r % 4;
		return r;
	}

	int largest = 0;
	int largestSpot = 0;
	int smallest = 15;
	int smallestSpot = 0;

	for(int i = 0; i< 4; i++){
		if(queue_length[i] > largest){
			largest = queue_length[i];
			largestSpot = i;
		}
		if(queue_length[i] < smallest){
			smallest = queue_length[i];
			smallestSpot = i;
		}
	}
	if(cuOrCl == 1){
		return smallestSpot;
	}else{
		return largestSpot;
	}

	
}


void *clerk_entry(void * clerkInfo){
	int test = 1;

	struct clerk_info * p_clerkInfo = clerkInfo;
	//printf("clerk %d created\n", p_clerkInfo->clerk_id);
	while(numCustomersGlob != 0){
		test = pthread_mutex_lock(&queues_mutex);
		if(test != 0){
			printf("error locking mutex");
			exit(0);
		}	
		int queueTake = checkQueue(0);
		if(queueTake == -1){
			test = pthread_mutex_unlock(&queues_mutex);
			if(test != 0){
				printf("error locking mutex");
				exit(0);
			}
			if(queueTake == -1 && numCustomersGlob < 1){
				break;
			}
			usleep(100);
			continue;
		}
		
 		
 		
	// 	printf("clerk wanting to get into queue: %d\n", queueTake);
 		lockUnlockMutex(0, queueTake); //lock that queue
 		//check which clerk this is, using its p_clerkInfo->clerk_id
 		queue_length[queueTake]--;
	 	numCustomersGlob--; //these two -- guys used to be above the printf statement just above. moved so that queue stuff wouldnt conflict.
 		
	 	//need to also see if a clerk is 
	 	if(p_clerkInfo->clerk_id == 1){
	 		if(c2 == queueTake){
	 			//release the mutex and wait
	 			lockUnlockMutex(1, queueTake);
	 			lockUnlockMutexClerks(0, queueTake);
	 			conVarCommands(3, queueTake);
	 			lockUnlockMutexClerks(1, queueTake);
	 			lockUnlockMutex(0, queueTake);
	 		}
	 		c1 = queueTake;
	 		c1_busy = 2;
	 	}else if(p_clerkInfo->clerk_id == 2){
	 		if(c1 == queueTake){
	 			//release the mutex and wait
	 			lockUnlockMutex(1, queueTake);
	 			lockUnlockMutexClerks(0, queueTake);
	 			conVarCommands(3, queueTake);
	 			lockUnlockMutexClerks(1, queueTake);
	 			lockUnlockMutex(0, queueTake);
	 		}
	 		c2 = queueTake;
	 		c2_busy = 2;
	 	}

	 	test = pthread_mutex_unlock(&queues_mutex); //unlock the amount of values in a queue
	 	if(test != 0){
			printf("error unlocking mutex");
			exit(0);
		}
	 	

	 	queue_countDown[queueTake] = queue_length[queueTake]+1;
 		conVarCommands(2, queueTake); //broadcast to processes BROADCAST--BROADCAST
	 	lockUnlockMutex(1, queueTake); //unlock the current queue

	 	//based on which clerk it is. lock its mutex
	 	if(p_clerkInfo->clerk_id == 1){
	 		test = pthread_mutex_lock(&clerk1_mutex);
	 		if(test != 0){
				printf("error locking mutex");
				exit(0);
			}
	 		test = pthread_cond_wait(&clerk1_convar, &clerk1_mutex);
	 		if(test != 0){
				printf("error waiting convar");
				exit(0);
			}
	 		test = pthread_mutex_unlock(&clerk1_mutex);
	 		if(test != 0){
				printf("error unlocking mutex");
				exit(0);
			}
	 	}else if(p_clerkInfo->clerk_id == 2){
	 		test = pthread_mutex_lock(&clerk2_mutex);
	 		if(test != 0){
				printf("error locking mutex");
				exit(0);
			}
	 		test = pthread_cond_wait(&clerk2_convar, &clerk2_mutex);
	 		if(test != 0){
				printf("error waiting convcar");
				exit(0);
			}
	 		test = pthread_mutex_unlock(&clerk2_mutex);
	 		if(test != 0){
				printf("error unlocking mutex");
				exit(0);
			}
	 	}

	 	//do convar wait, using the convar specific to the clerk_id
	 	//unlock after

	}
	
	pthread_exit(NULL);
	
	return NULL;
}


void * customer_entry(void * cus_info){

	struct customer_info * p_myInfo = cus_info;
 	struct timeval cur_time; //time structs, one for when the customer enters the queue, one for when the clerk takes the customer
 	double enterQueue_time;
	usleep((p_myInfo->arrival_time)*100000);
 	fprintf(stdout, "A customer arrives: customer ID %2d. \n",  p_myInfo->user_id);
 	int test = 1;

 	test = pthread_mutex_lock(&queues_mutex);
 	if(test != 0){
		printf("error locking mutex");
		exit(0);
	}
 	int queueEnter = checkQueue(1);
 	lockUnlockMutex(0, queueEnter);
 	p_myInfo->which_queue = queueEnter; //0-3, after the indexes moved this to be with the other update stuff
 	p_myInfo->whereIn_queue = queue_length[queueEnter]+1;//this will return the number in the queue, so 1 will be the "first" in the queue
 	queue_length[queueEnter]++; //moved this to be before the time stuff
 	fprintf(stdout, "Customer %d enters a queue: the queue ID %1d, and queue location %2d. \n",p_myInfo->user_id, queueEnter+1, p_myInfo->whereIn_queue);
 	gettimeofday(&cur_time,NULL); //taking the time of when the customer enters the queue
	enterQueue_time = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
 	//lockUnlockMutex(1, queueEnter);
 	//lockUnlockMutex(0, queueEnter);
 	test = pthread_mutex_unlock(&queues_mutex);
 	if(test != 0){
		printf("error locking mutex");
		exit(0);
	}
 	do{
 	//	lockUnlockMutex(0, queueEnter); //lock the mutex while in the do while
 		conVarCommands(1, queueEnter); //1 is wait -> wait for signal from the clerk
 		p_myInfo->whereIn_queue--; //decrease their number in the queue

 		if(p_myInfo->whereIn_queue == 0){
 			if(c1 == p_myInfo->which_queue && c1_busy == 2){ //need to add boolean to check if c1 is busy
 				p_myInfo->server = 1;
	 			c1_busy = 1;
 			}else if(c2 == p_myInfo->which_queue && c2_busy == 2){
 				p_myInfo->server = 2;
	 			c2_busy = 1;
 			}else{
 				printf("error in customer entry with server establishment.\n"); //for testing. should never get called
 			}
 		}
 		queue_countDown[p_myInfo->which_queue]--;
 		if(queue_countDown[p_myInfo->which_queue] == 0){
 			conVarCommands(3, p_myInfo->which_queue);
 		}
 	//lockUnlockMutex(1, queueEnter);//unlock the mutex again to allow the rest of the values to decrement. Only 1 should be 0
 	}while(p_myInfo->whereIn_queue != 0);
 	lockUnlockMutex(1, queueEnter);

 	//this is where stuff prints
 	//know that the server is in p_myInfo->server
 	//NEED TO GET CURRENT TIME AND UPDATE THE TOTAL TIME

	double cur_secs, init_secs, final_secs, waitInQueue_time;
	init_secs = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	double currTime = cur_secs - init_secs;
	waitInQueue_time = cur_secs - enterQueue_time;

 	fprintf(stdout, "A clerk starts serving a customer: time in queue %.2f, service start time %.2f, the customer ID %2d, the clerk ID %1d. \n", waitInQueue_time, currTime, p_myInfo->user_id, p_myInfo->server); //currTime is a double
 	usleep((p_myInfo->service_time)*100000);

 	gettimeofday(&cur_time, NULL);
	final_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	double endTime = final_secs - init_secs; //might be final_secs - cur_secs
 	
 	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", endTime, p_myInfo->user_id, p_myInfo->server);


 	overall_waiting_time = overall_waiting_time + waitInQueue_time;

 	if(c1 == p_myInfo->which_queue && c1_busy == 1){
 		c1_busy = 0;
		test = pthread_cond_signal(&clerk1_convar);
		if(test != 0){
		printf("error signalling mutex");
		exit(0);
	}
	}else if(c2 == p_myInfo->which_queue && c2_busy == 1){
		c2_busy = 0;
		test = pthread_cond_signal(&clerk2_convar);
		if(test != 0){
		printf("error signalling mutex");
		exit(0);
	}
	}else{
		printf("\nerror in customer signalling the clerk\n\n"); //for testing. should never get called
		exit(0);
	}


	pthread_exit(NULL);


 	//when finished everything, signel the specific clerk using their specific convar, using signal, not broadcast
 	//make sure to remember to terminate the thread.
	
	
	return NULL;
}


int main(int argc, char* argv[]){

	srand(time(NULL)); //get random time to use incase queues are all equal length
	char * input;
	while(1) {
		char *prompt = "ACS: > ";
		input = readline(prompt);
		
		if(input == NULL || strcmp(input, "") == 0) {
			continue;
		}
		if(strcmp(input, "end") ==0){ //if end is entered into the prompt we end the run. This was implemented to allow for bash script automated testing
			break;
		}
		gettimeofday(&init_time, NULL); //for when the program starts.
		overall_waiting_time = 0;

	    FILE* file = fopen(input, "r");
	    if(file == NULL){
	    	printf("file %s had an error\n", input);
	    	continue;
	    }
	    char line[256];
	    fgets(line, sizeof(line), file);
	    int token = atoi(strtok(line,""));
	    int numCustomers = token;
	    numCustomersGlob = numCustomers;
	    printf("size: %d\n", token); 
	    struct customer_info custID[token];
	   	int  count = 0;
	    printf("numCustomers %d\n", numCustomers);
	    while (fgets(line, sizeof(line), file)) {
	        token = atoi(strtok(line,":"));
	        if(token < 0){
	        	numCustomers--;
	        	continue;
	        }
	        custID[count].user_id = token;
	        token = atoi(strtok(NULL, ","));
	        if(token < 0){
	        	numCustomers--;
	        	continue;
	        }
	        custID[count].arrival_time = token;
	        token = atoi(strtok(NULL, ""));
	        if(token < 0){
	        	numCustomers--;
	        	continue;
	        }
	        custID[count].service_time = token;

	        count++;
	    }
	    printf("numCustomers %d\n", numCustomers);
	    numCustomersGlob = numCustomers;

	    fclose(file);
	    pthread_t threadClerkId[2];
	    struct clerk_info clerkID[2];
	    pthread_t threadCustId[numCustomers];
	    int i =0;
	    int test = 1;

	    for(i = 0; i<2; i++){	
	    	clerkID[i].clerk_id = i+1; //defining the ids of the clerks, if there are more details to add can remove and do outside of this loop
			test = pthread_create(&threadClerkId[i], NULL, clerk_entry, (void *) &clerkID[i]);
			if(test != 0){
				printf("error creating thread %d\n", i);
				exit(0);
			}
	    }

	    for(i = 0; i < numCustomers; i++){ // number of customers
			//printf("IDS:: %2d\n", custID[i].user_id);
			test = pthread_create(&threadCustId[i], NULL, customer_entry, (void *) &custID[i]);
			if(test != 0){
				printf("error creating thread %d\n", i);
				exit(0);
			}
		}

		for(i = 0; i < numCustomers; i++){
			test = pthread_join(threadCustId[i], NULL);
			if(test != 0){
				printf("error joining thread %d\n", i);
				exit(0);
			}
		}

		for(i = 0;  i < 2; i++){
			test = pthread_join(threadClerkId[i], NULL);
			if(test != 0){
				printf("error joining thread %d\n", i);
				exit(0);
			}
		}

		double final = (double)overall_waiting_time/numCustomers;
		printf("The average waiting time for all customers in the system is: %.2f seconds\n", final);

	} //end while

}

