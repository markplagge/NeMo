#include <stdio.h>
#include<inttypes.h>
#include<assert.h>
#include<unistd.h>
#include <pthread.h>
#include<sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#define _idType int_fast16_t //  unsigned int
//neuron specific type redefs - for potentially integrating weird bit length variable sizes or what not:
#define _neVoltType uint_fast32_t
#define _neStatType int_fast32_t
#define regid_t uint32_t
#define gid_t uint64_t


#define LOC(a) ((regid_t)a)
#define CORE(a) ((regid_t)(((gid_t)(a) >> 32) & 0xFFFFFFFF))



void getLocalIDs(gid_t global, regid_t * core, regid_t *local ){
	(*core) = CORE(global);
	(*local)= LOC(global);
}
gid_t globalID(regid_t core, regid_t local){
	gid_t returnVal = 0;
	returnVal = ((uint64_t)core << 32) | local;
	return returnVal;
}
int numth = 256;
int cur = 0;
int max = 16777215;
int perTh;

void * testVal(void * str){
	long starti =(*(long *)  str ) * max;
	long startj = startj;
	long endj= starti + max;
	long endi = endj;
	//printf("Thread checking in with start at %i and end at %i.\n", starti, endi);
	for(long i = starti; i < endi; i ++)
	{
		for(long j = startj; j <endj; j++){
			regid_t core = i;
			regid_t local = j;
			gid_t global = globalID(core,local);

			core = CORE(global);
			local = LOC(local);
			if(core != i)
			{
				//ec 1:
				printf("\n*******************************Manual Dump CORE **************************\n "
						       "Core was %u, should have been %li - first check fail, line 57 -- \n"
						       "Local was %u should have been %li\n"
						       "\t-------*****************---\t", core, i,local,j);
			}
			if(local != j)
			{
				//ec 2:
				printf("\n*******************************Manual Dump LOCAL**************************\n "
						       "Core was %u, should have been %li - first check fail, line 63. -- \n"
						       "Local was %u, should have been %li\n"
						       "\t-------*****************---\t", core, i,local,j);
			}
			//assert(core == i);
			//assert(local == j);

			getLocalIDs(global, &core, &local);
			if(core != i)
			{
				//ec 1:
				printf("\n*******************************Manual Dump CORE**************************\n "
						       "Core was %u, should have been %li - second check fail, line 73 -- \n"
						       "Local was %u, should have been %li\n"
						       "\t-------*****************---\t", core, i,local,j);
			}
			if(local != j)
			{
				//ec 2:
				printf("\n*******************************Manual Dump LOCAL**************************\n "
						       "Core was %u, should have been %li - first check fail, line 63. -- \n"
						       "Local was %u, should have been %li\n"
						       "\t-------*****************---\t", core, i,local,j);

			}
			assert(core == i);
			assert(local ==j);

		}
	}

	return NULL;
}

int main() {
	printf("Enter.");
	printf("running tests.\n");



	gid_t global;
	regid_t core = 72000;

	regid_t local = 0;
	global = globalID(core,local);
	gid_t g2 = global;
	printf("Core is :%u\n\n\n",CORE(global));







	dup2(STDOUT_FILENO, STDERR_FILENO);
	int status;
	//pid_t p = fork();

	perTh = max/numth;
	pthread_t threads[numth];
	long * thread_args[numth];
	int result_code;
	int currentSearchVal = 0;
	for(int i = 0; i  < numth; i ++){

		thread_args[i] = (long *) malloc (sizeof(long));
		*thread_args[i] = i;
		  printf("main - creating thread %i with max search index of %i\n",i,*thread_args[i]);
		result_code = pthread_create(&threads[i], NULL, testVal, (void*) thread_args[i]);
	}


	/*
	for(int i = 0; i < 100000; i++)
	{
		for(int j = 0; j < 100000; j++)
		{
			regid_t core = i;
			regid_t local = j;
			gid_t global = globalID(core,local);
			assert(i == core);
			assert(j == local);

			core = CORE(global);
			local = LOC(local);
			assert(core == i);
			assert(local == j);

			getLocalIDs(global, &core, &local);
			assert(core == i);
			assert(local ==j);

		}
	}*/
	for (int index = 0; index < numth; ++index) {
		// block until thread 'index' completes
		result_code = pthread_join(threads[index], NULL);
		printf("In main: thread %d has completed\n", index);
		assert(0 == result_code);
	}

	dup2(STDERR_FILENO,STDOUT_FILENO);
	int w;
	//if(p != 0 )
	//	w = waitpid(p, &status, WUNTRACED | WCONTINUED);
	printf("Testing complete.\n");

	}

