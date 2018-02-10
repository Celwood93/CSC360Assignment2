
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
pthread_mutex_t q1_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
pthread_mutex_t q2_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
pthread_mutex_t q3_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
pthread_mutex_t q4_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;


//new
pthread_mutex_t clerk_q1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk_q2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk_q3_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk_q4_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t clerk_q1_mutex_broadcast_wait = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk_q2_mutex_broadcast_wait = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk_q3_mutex_broadcast_wait = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk_q4_mutex_broadcast_wait = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t q1_cus_moving = PTHREAD_COND_INITIALIZER;
pthread_cond_t q2_cus_moving = PTHREAD_COND_INITIALIZER;
pthread_cond_t q3_cus_moving = PTHREAD_COND_INITIALIZER;
pthread_cond_t q4_cus_moving = PTHREAD_COND_INITIALIZER;
//endnew

pthread_cond_t q1_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q2_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q3_convar = PTHREAD_COND_INITIALIZER;
pthread_cond_t q4_convar = PTHREAD_COND_INITIALIZER;

 
static struct timeval init_time;
int numCustomersGlob; 
double overall_waiting_time; 
int queue_length[4]; 

//add 2 more variables, c1 and c2, which will be integers. these will store values from 0-3 representing which queue they are working in.
int c1 = -1; //clerk1
int c2 = -1; //clerk2

int c1_busy = 0;// 0 not busy, 1 busy, 2 requesting customer
int c2_busy = 0;

int conVarCommands(int whichCommand, int currentQueue){
	if(whichCommand == 1){ //wait for a queue is 1
		if(currentQueue == 0){
			pthread_cond_wait(&q1_convar, &q1_mutex);
		}else if(currentQueue == 1){
			pthread_cond_wait(&q2_convar, &q2_mutex);
		}else if(currentQueue == 2){
			pthread_cond_wait(&q3_convar, &q3_mutex);
		}else if(currentQueue == 3){
			pthread_cond_wait(&q4_convar, &q4_mutex);
		}
	}else if(whichCommand == 2){ //convar broadcase to queues is 2 (for a specfic queued customers, from clerks)
		if(currentQueue == 0){
			pthread_cond_broadcast(&q1_convar);
		}else if(currentQueue == 1){
			pthread_cond_broadcast(&q2_convar);
		}else if(currentQueue == 2){
			pthread_cond_broadcast(&q3_convar);
		}else if(currentQueue == 3){
			pthread_cond_broadcast(&q4_convar);
		}
	}else if(whichCommand == 3){ //mutex for clerk exclusion is 3. to keep clerks from signalling the same queue at the same time
		if(currentQueue == 0){
			pthread_mutex_lock(&clerk_q1_mutex);
		}else if(currentQueue == 1){
			pthread_mutex_lock(&clerk_q2_mutex);
		}else if(currentQueue == 2){
			pthread_mutex_lock(&clerk_q3_mutex);
		}else if(currentQueue == 3){
			pthread_mutex_lock(&clerk_q4_mutex);
		}
	}else if(whichCommand == 4){ //mutex lock for after broadcast from clerk is 4, this is so the clerk knows then the queue is done moving forward
		if(currentQueue == 0){
			pthread_mutex_lock(&clerk_q1_mutex_broadcast_wait);
		}else if(currentQueue == 1){
			pthread_mutex_lock(&clerk_q2_mutex_broadcast_wait);
		}else if(currentQueue == 2){
			pthread_mutex_lock(&clerk_q3_mutex_broadcast_wait);
		}else if(currentQueue == 3){
			pthread_mutex_lock(&clerk_q4_mutex_broadcast_wait);
		}
	}else if(whichCommand == 5){ //convar for wait is 5, which happens while the clerk is waiting for the customers to all finish moving in the respected queue
		if(currentQueue == 0){
			pthread_cond_wait(&q1_cus_moving, &clerk_q1_mutex_broadcast_wait);
		}else if(currentQueue == 1){
			pthread_cond_wait(&q2_cus_moving, &clerk_q2_mutex_broadcast_wait);
		}else if(currentQueue == 2){
			pthread_cond_wait(&q3_cus_moving, &clerk_q3_mutex_broadcast_wait);
		}else if(currentQueue == 3){
			pthread_cond_wait(&q4_cus_moving, &clerk_q4_mutex_broadcast_wait);
		}
	}else if(whichCommand == 6){ //convar for wait is 5, which happens while the clerk is waiting for the customers to all finish moving in the respected queue
		if(currentQueue == 0){
			pthread_cond_signal(&q1_cus_moving);
		}else if(currentQueue == 1){
			pthread_cond_signal(&q2_cus_moving);
		}else if(currentQueue == 2){
			pthread_cond_signal(&q3_cus_moving);
		}else if(currentQueue == 3){
			pthread_cond_signal(&q4_cus_moving);
		}
	}else if(whichCommand == 7){ //mutex unlock is 7 for when clerk has been signalled after customers are done moving forward
		if(currentQueue == 0){
			pthread_mutex_unlock(&clerk_q1_mutex_broadcast_wait);
		}else if(currentQueue == 1){
			pthread_mutex_unlock(&clerk_q2_mutex_broadcast_wait);
		}else if(currentQueue == 2){
			pthread_mutex_unlock(&clerk_q3_mutex_broadcast_wait);
		}else if(currentQueue == 3){
			pthread_mutex_unlock(&clerk_q4_mutex_broadcast_wait);
		}
	}else if(whichCommand == 8){ //command 8 is for unlocking the mutex once the clerk has finished with the current queue
		if(currentQueue == 0){
			pthread_mutex_unlock(&clerk_q1_mutex);
		}else if(currentQueue == 1){
			pthread_mutex_unlock(&clerk_q2_mutex);
		}else if(currentQueue == 2){
			pthread_mutex_unlock(&clerk_q3_mutex);
		}else if(currentQueue == 3){
			pthread_mutex_unlock(&clerk_q4_mutex);
		}
	}
}


int lockUnlockMutex(int lockOrUnlock, int currentQueue){

	if(lockOrUnlock == 0){
			if(currentQueue == 0){
			pthread_mutex_lock(&q1_mutex);
		}else if(currentQueue == 1){
			pthread_mutex_lock(&q2_mutex);
		}else if(currentQueue == 2){
			pthread_mutex_lock(&q3_mutex);
		}else if(currentQueue == 3){
			pthread_mutex_lock(&q4_mutex);
		}
	}else{
		if(currentQueue == 0){
			pthread_mutex_unlock(&q1_mutex);
		}else if(currentQueue == 1){
			pthread_mutex_unlock(&q2_mutex);
		}else if(currentQueue == 2){
			pthread_mutex_unlock(&q3_mutex);
		}else if(currentQueue == 3){
			pthread_mutex_unlock(&q4_mutex);
		}
	}
	return 0;
}

//0 is clerk, 1 is customer
int checkQueue(int cuOrCl){
	if(queue_length[0] == 0 && queue_length[1]==0 && queue_length[2]==0 && queue_length[3]==0 && cuOrCl == 0){ //when the queues are all empty and the clerk is requesting return -1
	//	printf("-1\n");
		return -1;
	}
	if((queue_length[0] == queue_length[1] && queue_length[1]==queue_length[2] && queue_length[2] == queue_length[3])){
		int r = rand();
		r = r % 4;
	//	printf("random: %d\n", r);
		return r;
	}

	//printf("queue info: Queue1: %d, Queue2: %d, Queue3: %d, Queue4: %d, CuOrCl: %d\n", queue_length[0],queue_length[1],queue_length[2],queue_length[3], cuOrCl);
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
		printf("smallest: %d\n", smallestSpot);
		return smallestSpot;
	}else{

		printf("queue info: Queue1: %d, Queue2: %d, Queue3: %d, Queue4: %d, CuOrCl: %d\n", queue_length[0],queue_length[1],queue_length[2],queue_length[3], cuOrCl);
		printf("largest: %d\n", largestSpot);
		return largestSpot;
	}

	
}


void *clerk_entry(void * clerkInfo){

	struct clerk_info * p_clerkInfo = clerkInfo;
	//printf("clerk %d created\n", p_clerkInfo->clerk_id);
	while(numCustomersGlob != 0){
		pthread_mutex_lock(&queues_mutex);
		int queueTake = checkQueue(0);
		if(queueTake == -1){
			pthread_mutex_unlock(&queues_mutex);
			if(queueTake == -1 && numCustomersGlob < 1){
				break;
			}
			usleep(100);
			continue;
		}
		
 		
 		
	// 	printf("clerk wanting to get into queue: %d\n", queueTake);
 		lockUnlockMutex(0, queueTake); //lock that queue
 		queue_length[queueTake]--;
	 	numCustomersGlob--; //these two -- guys used to be above the printf statement just above. moved so that queue stuff wouldnt conflict.
	 //	printf("clerk got into the queue: %d\n", queueTake);
	 	printf("stuck here\n");
 		pthread_mutex_unlock(&queues_mutex); //unlock the amount of values in a queue
	 	//check which clerk this is, using its p_clerkInfo->clerk_id
	 	if(p_clerkInfo->clerk_id == 1){
	 		printf("c2: %d, queueTake: %d\n", c2, queueTake);
	 		if(c2 == queueTake){
	 			//release the mutex and wait
	 			lockUnlockMutex(1, queueTake);
	 			printf("clerk %d @unlock\n", p_clerkInfo->clerk_id);
	 			//wait
	 			conVarCommands(3, queueTake);
	 			lockUnlockMutex(0, queueTake);
	 		}
	 		c1 = queueTake;
	 		c1_busy = 2;
	 	}else if(p_clerkInfo->clerk_id == 2){
	 		printf("c1: %d, queueTake: %d\n", c1, queueTake);
	 		if(c1 == queueTake){
	 			//release the queue mutex and wait
	 			lockUnlockMutex(1, queueTake);
	 			printf("clerk %d @unlock\n", p_clerkInfo->clerk_id);
	 			//wait
	 			conVarCommands(3, queueTake);
	 			lockUnlockMutex(0, queueTake);
	 		}
	 		c2 = queueTake;
	 		c2_busy = 2;
	 	}
	 	//update the appropriate global variable to specifiy what value to update (which queue its accessing)
	 	printf("clerk %d @3 lock shared mutex\n", p_clerkInfo->clerk_id);
	 	conVarCommands(3, queueTake); //lock the mutex shared by the clerks
	 	printf("clerk %d @2 broadcast\n", p_clerkInfo->clerk_id);
 		conVarCommands(2, queueTake); //broadcast to processes BROADCAST--BROADCAST
 		printf("clerk %d @1 unlock current queue\n", p_clerkInfo->clerk_id);
	 	lockUnlockMutex(1, queueTake); //unlock the current queue
	 	printf("clerk %d @4 lock wait mutex\n", p_clerkInfo->clerk_id);
 		conVarCommands(4, queueTake); //lock the broadcast wait convar mutex 
	 	printf("clerk %d @5 convar wait\n", p_clerkInfo->clerk_id);
 		conVarCommands(5, queueTake); //convar - wait for the broadcast command
	 	printf("clerk %d @7 unlock wait mutex\n", p_clerkInfo->clerk_id);
 		conVarCommands(7, queueTake); //unlock the broadcast wait convar mutex
	 	printf("clerk %d @8 unlock shared mutex\n", p_clerkInfo->clerk_id);
 		conVarCommands(8, queueTake); //unlock the mutex shared by the clerks
	 //moved
	 //	printf("clerk %d @1 unlock current queue\n", p_clerkInfo->clerk_id);
	 //	lockUnlockMutex(1, queueTake); //unlock the current queue

	 	//lock mutex, wait, unlock
	 	//based on which clerk it is. lock its mutex
	 	if(p_clerkInfo->clerk_id == 1){
	 		pthread_mutex_lock(&clerk1_mutex);
	 		pthread_cond_wait(&clerk1_convar, &clerk1_mutex);
	 		pthread_mutex_unlock(&clerk1_mutex);
	 	}else if(p_clerkInfo->clerk_id == 2){
	 		pthread_mutex_lock(&clerk2_mutex);
	 		pthread_cond_wait(&clerk2_convar, &clerk2_mutex);
	 		pthread_mutex_unlock(&clerk2_mutex);
	 	}

	 	//do convar wait, using the convar specific to the clerk_id
	 	//unlock after

	 	if(p_clerkInfo->clerk_id == 1){
	 		c1 = -1;
	 		c1_busy = 0;
	 	}else if(p_clerkInfo->clerk_id == 2){
	 		c2 = -1;
	 		c2_busy = 0;
	 	}

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

 	pthread_mutex_lock(&queues_mutex);
 	int queueEnter = checkQueue(1);
 	lockUnlockMutex(0, queueEnter);
 	p_myInfo->which_queue = queueEnter; //0-3, after the indexes moved this to be with the other update stuff
 	p_myInfo->whereIn_queue = queue_length[queueEnter]+1;//this will return the number in the queue, so 1 will be the "first" in the queue
 	queue_length[queueEnter]++;
 	pthread_mutex_unlock(&queues_mutex); //moved this to be before the time stuff
 	fprintf(stdout, "Customer %d enters a queue: the queue ID %1d, and queue location %2d. \n",p_myInfo->user_id, queueEnter+1, p_myInfo->whereIn_queue);
 	gettimeofday(&cur_time,NULL); //taking the time of when the customer enters the queue
	enterQueue_time = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
 	//lockUnlockMutex(1, queueEnter);
 	//lockUnlockMutex(0, queueEnter);
 	do{
 		//lockUnlockMutex(0, queueEnter); //lock the mutex while in the do while
 		printf("waiting Im: %d in queue: %d\n", p_myInfo->user_id, p_myInfo->which_queue);
 		conVarCommands(1, queueEnter); //1 is wait -> wait for signal from the clerk
 		p_myInfo->whereIn_queue--; //decrease their number in the queue
 		printf("CusID: %d, Queue: %d, Place: %d\n", p_myInfo->user_id, p_myInfo->which_queue, p_myInfo->whereIn_queue);
 		printf("%d == %d @ cus %d\n",p_myInfo->whereIn_queue, queue_length[queueEnter], p_myInfo->user_id);
 		if(p_myInfo->whereIn_queue == queue_length[queueEnter]){
 			printf("where at 6\n");
 			conVarCommands(6, p_myInfo->which_queue); //send signal to the clerk
 		}

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
 	//	lockUnlockMutex(1, queueEnter);//unlock the mutex again to allow the rest of the values to decrement. Only 1 should be 0
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
		pthread_cond_signal(&clerk1_convar);
	}else if(c2 == p_myInfo->which_queue && c2_busy == 1){
		c2_busy = 0;
		pthread_cond_signal(&clerk2_convar);
	}else{
		printf("\nerror in customer signalling the clerk\n\n"); //for testing. should never get called
	}


	pthread_exit(NULL);


 	//when finished everything, signel the specific clerk using their specific convar, using signal, not broadcast
 	//make sure to remember to terminate the thread.
	
	
	return NULL;
}


int main(int argc, char* argv[]){
	/* Testing the checkQueue function (should probably be done better)
	queue_length[0]=3;
	queue_length[1]=3;
	queue_length[2]=3;
	queue_length[3]=3;


	int b = checkQueue(1);
	printf("as a cust %d\n", b);
	b = checkQueue(0);
	printf("as a clerk %d\n", b);
	return 1;
	*/
	srand(time(NULL));
	int repeat = 0;
	char * input;
	int counter = 0;
	while(1) {
		char *prompt = "ACS: > ";
		if(repeat == 0){
			input = readline(prompt);
		}
		if(input == NULL || strcmp(input, "") == 0) {
			continue;
		}
		if(strcmp(input, "end") ==0){
			break;
		}
		gettimeofday(&init_time, NULL);
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
	    while (fgets(line, sizeof(line), file)) {
	        token = atoi(strtok(line,":"));
	        custID[count].user_id = token;
	        token = atoi(strtok(NULL, ","));
	        custID[count].arrival_time = token;
	        token = atoi(strtok(NULL, ""));
	        custID[count].service_time = token;

	        count++;
	    }
	    if(0){ //change to 1 to run
		    for(int i = 0; i < count; i++){
		    	printf("customer number: %d\n", i);
		    	printf("	user_id: %d\n", custID[i].user_id);
		    	printf("	service_time: %d\n", custID[i].service_time);
		    	printf("	arrival_time: %d\n", custID[i].arrival_time);

		    }
		}

	    fclose(file);

	    pthread_t threadClerkId[2];
	    struct clerk_info clerkID[2];
	    pthread_t threadCustId[numCustomers];
	    int i =0;

	    for(i = 0; i<2; i++){	
	    	clerkID[i].clerk_id = i+1; //defining the ids of the clerks, if there are more details to add can remove and do outside of this loop
			pthread_create(&threadClerkId[i], NULL, clerk_entry, (void *) &clerkID[i]);
	    }

	    for(i = 0; i < numCustomers; i++){ // number of customers
			//printf("IDS:: %2d\n", custID[i].user_id);
			pthread_create(&threadCustId[i], NULL, customer_entry, (void *) &custID[i]);
		}

		for(i = 0; i < numCustomers; i++){
			pthread_join(threadCustId[i], NULL);
		}

		pthread_join(threadClerkId[0], NULL);
		pthread_join(threadClerkId[1], NULL);

		double final = (double)overall_waiting_time/numCustomers;
		printf("The average waiting time for all customers in the system is: %.2f seconds\n", final);

		//for testing
		if((final/.3) < 1.05){
			repeat = 1;
			counter++;
			if(counter == 100){
				repeat = 0;
			}
		}else{
			repeat = 0;
		}

	printf("times:%d\n", counter);

	} //end while
	printf("times:%d\n", counter);

}

