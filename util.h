#ifndef UTIL_H
#define UTIL_H

#include <sys/time.h>
#include <pthread.h>
#include <queue>
#include <stdio.h>

using std::queue;

struct runStats{
	int nWork, nAsk, nReceive, nComplete, nSleep;
};

struct threadData{
	int nId, nDone;
};


class Util{
public:
	static void buildLogName(char* buf, char const *logId);
	static float getRunTime(struct timeval* start);
};


class MutexIO{
public:
	void statusReport(FILE *fp, struct timeval* start, char status, int id, queue<char*>* jobQ, char* nParam);
	void summaryReport(FILE* fp, struct runStats* stats, struct threadData tData[], struct timeval* start, int threadCount);

	MutexIO();

private:
	pthread_mutex_t iomutex;
};

#endif