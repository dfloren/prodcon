#include <stdlib.h> // atoi
#include <stdio.h> // fopen
#include <queue> // std::queue
#include <string.h> // strdup
#include <pthread.h>
#include <semaphore.h>
#include "util.h" // MutexIO, Util

#define MIN_ARGC 1
#define MAX_ARGC 3

#define CMD_SIZE 6 // 'T or S' + int:1-100 +'\n'+'\0'
#define CMD_TYPE 0
#define CMD_PARAM 1

#define LOGNAME_LENGTH 100

// defined in tands.cpp
void Trans( int n );
void Sleep( int n );

///// GLOBAL VARS
std::queue<char*> jobQ;
struct timeval startTime;
FILE *fp;

// synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // ensures only 1P/1C pushing/popping, respectively
sem_t sSlots, sWork; // bounds queue(buffer) size
MutexIO printer; // synchronized file printer

// stats tracking
runStats stats;
////// END GLOBAL VARS


void *consumer_t(void *arg){
	threadData* tData = (threadData*) arg;
	
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	while(1){
		printer.statusReport(fp, &startTime, 'A', tData->nId, &jobQ, NULL);
		stats.nAsk++;
		
		// only cancellation point of the consumer, when waiting for work
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		sem_wait(&sWork);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		// pop from the queue and report (the only mutex'd portion of consumer)
		pthread_mutex_lock(&mutex);
		char *cmd = (char*) malloc(CMD_SIZE);
		cmd = jobQ.front();
		jobQ.pop();
		printer.statusReport(fp, &startTime, 'R', tData->nId, &jobQ, &cmd[CMD_PARAM]);
		pthread_mutex_unlock(&mutex);
		stats.nReceive++;

		sem_post(&sSlots); // tell P the queue is not full
		Trans(atoi(&cmd[CMD_PARAM])); // process the transaction

		// report a finished Trans()
		printer.statusReport(fp, &startTime, 'C', tData->nId, NULL, &cmd[CMD_PARAM]);
		stats.nComplete++;
		tData->nDone++;

		free(cmd);
	}
	pthread_exit(0);
}


int main(int argc, char const *argv[]){
	// start the program timer
	gettimeofday(&startTime, NULL);

	// input checking
	if(argc > MAX_ARGC || argc <= MIN_ARGC){
		printf("Usage: %s nthreads <id>\n", argv[0]);
		return 0;
	}

	int nThreads = atoi(argv[1]);

	// generate log file name
	char const *logId = (argc == 3) ? argv[2] : "0";
	char logName[LOGNAME_LENGTH] = "";
	Util::buildLogName(logName, logId);

	fp = fopen(logName, "w");
	if(fp){}
	else{
		perror(logName);
		return 1;
	}

	// Init counting semaphores
	sem_init(&sSlots, 0, nThreads*2); // available queue slots
	sem_init(&sWork, 0, 0); // work waiting in the queue

	// create nThreads, each with its own threadData struct
	threadData threadsData[nThreads];
	pthread_t tids[nThreads];
	for(int i = 0; i < nThreads; i++){
		threadsData[i].nId = i+1;
		threadsData[i].nDone = 0;
		pthread_create(&tids[i], NULL, consumer_t, &threadsData[i]);
	}

	// producer puts work in the queue
	char *cmd = (char*) malloc(CMD_SIZE);
	while(fgets(cmd,CMD_SIZE, stdin) != NULL){
		if(cmd[CMD_TYPE] == 'T'){
			sem_wait(&sSlots); // wait if queue is full
			jobQ.push(strdup(cmd));
			printer.statusReport(fp, &startTime, 'W', 0, &jobQ, &cmd[CMD_PARAM]);
			sem_post(&sWork); // tell C there is work
			stats.nWork++;
		} else if(cmd[CMD_TYPE] == 'S'){
			printer.statusReport(fp, &startTime,'S', 0, NULL, &cmd[CMD_PARAM]);
			Sleep(atoi(&cmd[CMD_PARAM]));
			stats.nSleep++;
		} else {
			printf("Invalid command: %s", cmd);
		}
	}
	free(cmd);
	printer.statusReport(fp, &startTime, 'E', 0, NULL, NULL);


	// thread joining will not continue until last thread is done working
	while(stats.nWork != stats.nComplete){
		for(int i = 0; i < nThreads; i++){
			pthread_tryjoin_np(tids[i], NULL);
		}
	}

	// for each thread, wait for it to stop at a cancellation point(e.g. sem_wait())
	// and send a cancel request. thread cancelling must be ENABLED
	for(int i = 0; i < nThreads; i++){
		while(pthread_tryjoin_np(tids[i],NULL) != 0){
			pthread_cancel(tids[i]);
		} pthread_join(tids[i], NULL);
	}

	printer.summaryReport(fp, &stats, threadsData, &startTime, nThreads);

	fflush(fp);

	fclose(fp);

	return 0;
}