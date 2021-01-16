#include "util.h"
#include <string.h> // strcat
#include <stdlib.h> // atoi

static char const *logFilePrefix = "prodcon.";
static char const *logFileSuffix = ".log";

void Util::buildLogName(char* buf, char const *logId){
	strcat(buf, logFilePrefix);
	strcat(buf, logId);
	strcat(buf, logFileSuffix);
	return;
}


float Util::getRunTime(struct timeval* start){
	struct timeval end;
	gettimeofday(&end, NULL);
	return (float) (end.tv_sec - start->tv_sec) +
           (float) (end.tv_usec - start->tv_usec) / 1000000;
}


MutexIO::MutexIO(){
	iomutex = PTHREAD_MUTEX_INITIALIZER; // ensures I/O is not garbled
}


void MutexIO::statusReport(FILE *fp, struct timeval* start, char status, int id, queue<char*>* jobQ, char* nParam){
	pthread_mutex_lock(&iomutex);

	float time = Util::getRunTime(start);
	int param = 0;
	int remWork = 0;
	if(nParam != NULL){
		param = atoi(nParam);
	}
	if(jobQ != NULL){
		remWork = (int)jobQ->size();
	}


	if(status == 'A'){
		fprintf(fp, "%.3f ID= %d      Ask\n", time, id);
	} else if (status == 'R'){
		fprintf(fp, "%.3f ID= %d Q= %d Receive  %d\n", time, 
			id, remWork, param);
	} else if (status == 'C'){
		fprintf(fp, "%.3f ID= %d      Complete %d\n", time, 
			id, param);
	} else if (status == 'W'){
		fprintf(fp, "%.3f ID= %d Q= %d Work     %d\n", time, 
				id, remWork, param);
	} else if (status == 'S'){
		fprintf(fp, "%.3f ID= %d      Sleep    %d\n", time, 
				id, param);
	} else if (status == 'E'){
		fprintf(fp, "%.3f ID= %d      End\n", time, id);
	} else {
		return;
	}

	pthread_mutex_unlock(&iomutex);
}


void MutexIO::summaryReport(FILE* fp, struct runStats* stats, struct threadData tData[], struct timeval* start, int threadCount){
	pthread_mutex_lock(&iomutex);

	fprintf(fp, "Summary:\n");
	fprintf(fp, "    Work            %d\n", stats->nWork);
	fprintf(fp, "    Ask             %d\n", stats->nAsk);
	fprintf(fp, "    Receive         %d\n", stats->nReceive);
	fprintf(fp, "    Complete        %d\n", stats->nComplete);
	fprintf(fp, "    Sleep           %d\n", stats->nSleep);
	for(int i = 0; i < threadCount; i++){
		fprintf(fp, "    Thread   %d      %d\n", i+1, tData[i].nDone);
	}
	fprintf(fp, "Transactions per second: %.2f\n", (float)stats->nComplete/Util::getRunTime(start));

	pthread_mutex_unlock(&iomutex);
}