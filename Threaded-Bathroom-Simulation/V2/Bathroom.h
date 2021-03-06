/* Bathroom.h
 * Andrew Morrison
 * Peter Christakos
 * Enter(gender G) 
 *		- check for gender flag
 *		- enter if vacant or same gender
 *		- increment people in bathroom
 *		- wait if not
 *	Leave(void)
 *		- decrement the people in bathroom
 *		- set flag to vacant if last person
 *	Global bathroom object (PROTECT FROM RACE CONDITIONS)
 *		- counter for people in bathroom (both men and women)
 *		- gender flag or vacant
 *		- use synchronization primitives
 *	 	- holds poeple in bathroom
 * 		- holds bathroom flag
 *		- # of usages of bathroom
 *		- total time bathroom vacant
 *		- total time occupied
 *		- average queue length
 * 		- Average # of people in the bathroom at the same time
 *	Initialize(...)
 *		- called by master thread only
 *	Finalize(...)
 *		- after all threads have been called
 *		- print out statistics gathered by global bathroom object */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#ifndef BATHROOM_SEEN
#define BATHROOM_SEEN

struct br
{
	int gender; // -1 for vacant, 0 for female, and 1 for male
	int mCount;
	int fCount;
	int totalUsages;
	struct timeval vacantStartTime;
	struct timeval vacantEndTime;
	long vacantTime;
	long occupiedTime;
	struct timeval occupiedStartTime;
	struct timeval occupiedEndTime;
	//int averageQL;
	//int avePeople;
	pthread_mutex_t lock;
	pthread_mutex_t vacant;
	pthread_cond_t fVacant;
	pthread_cond_t mVacant;
};

//enum gender {male = 1, female = 0};

//struct br * brGlobal;
long enter(int g);
void leave();
void initialize();
void finalize();
void printStats(int gender, int threadNum, int lCount, long minTime, long aveTime, long maxTime);

#endif /* BATHROOM_SEEN */