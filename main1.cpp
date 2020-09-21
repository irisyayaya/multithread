#include <windows.h> 
#include <process.h>
#include <stdio.h>
#include <stdlib.h> 
#include "baggie.h" 

/****
 This program obtains two parameters from the command-line:  N and W.  The program
 then launches N worker threads.  Each worker will then attempt to run the computationally heavy
 section (two inner loops) of the "comp" function.   However, at most W threads are allowed
 to do so simultaneously.  This condition will be regulated with a mutex.  Let's use the terminology
 "busy" to describe a worker thread that is employed in the computationally heavy section.  Busy threads
 will either be done with the computation, or be told by the master to terminate according some iteration
 limit that is checked by the master.  As each "busy" worker terminates, any of the remaining "unbusy"
 workers try to become "busy".  One of them will succeed.  Eventually all workers terminate and the program
 stops.
***/

unsigned _stdcall comp_wrapper(void *foo);

//void comp(double, double, double, double *); 

int main(int argc, char *argv[])
{
	HANDLE *pThread;
	unsigned *pthreadID;
	HANDLE consolemutex;
	HANDLE *mastermutexes;
	int retcode = 0;
	int J, W;
	
	int N, T;

	double alpha, pi;


	if (argc != 7) {

		printf("use: main1 J W alpha pi N T\n"); retcode = 1; return retcode;

	}

	
	J = atoi(argv[1]);
	W = atoi(argv[2]);
	
	alpha = atof(argv[3]);

	pi = atof(argv[4]);

	N = atoi(argv[5]);

	T = atoi(argv[6]);



	printf("J = %d W = %d alpha = %g pi = %g N = %d T =%d\n", J, W, alpha, pi, N, T);

	//baggie b(alpha, pi, N, T, 0);
	//b.solve();
	//printf("%4.9g",b.getresult());

	baggie **ppbaggies;


	if ((J <= 0) || (W <= 0)) {
		printf("bad value of J or W: %d %d\n", J, W);
		retcode = 1; return retcode;
	}




	ppbaggies = (baggie **)calloc(J, sizeof(baggie *));
	/** ppbaggies is an array, each of whose members is the address of a baggie, and so the type of ppbaggies is baggie ** **/
	if (ppbaggies == NULL) {
		cout << "cannot allocate" << J << "baggies\n";
		retcode = 1; return retcode;
	}
	pThread = (HANDLE *)calloc(J, sizeof(HANDLE));
	pthreadID = (unsigned *)calloc(J, sizeof(unsigned));
	mastermutexes = (HANDLE *)calloc(J, sizeof(HANDLE));
	if ((pThread == NULL) || (pthreadID == NULL) || (mastermutexes == NULL)) {
		cout << "cannot allocate" << J << "handles and threadids\n";
		retcode = 1; return retcode;
	}

	for (int j = 0; j < J; j++) {
		ppbaggies[j] = new baggie(alpha, pi, N, T, j);  // fake "jobs": normally we would get a list of jobs from e.g. a file
	}


	consolemutex = CreateMutex(NULL, 0, NULL);

	for (int j = 0; j < J; j++) {
		ppbaggies[j]->setconsolemutex(consolemutex); // consolemutex shared across workers plus master
	}

	HANDLE heavymutex;
	heavymutex = CreateMutex(NULL, 0, NULL);

	int nowinheavy = 0;

	for (int j = 0; j < J; j++) {
		mastermutexes[j] = CreateMutex(NULL, 0, NULL);
		ppbaggies[j]->setmastermutex(mastermutexes[j]);

		ppbaggies[j]->setmaxworkersinheavysection(W);
		ppbaggies[j]->setheavysectionmutex(heavymutex);

		ppbaggies[j]->setnowinheavyaddress(&nowinheavy);


	}

	for (int j = 0; j < J; j++) {
		pThread[j] = (HANDLE)_beginthreadex(NULL, 0, &comp_wrapper, (void *)ppbaggies[j],
			0, &pthreadID[j]);
	}


	int numberrunning = J;

	for (; numberrunning > 0;) {
		Sleep(5000);
		printf("master will now check on workers, jobs remaining %d\n", numberrunning); fflush(stdout);
		numberrunning = J;

		for (int j = 0; j < J; j++) {
			double jiterations;
			char jstatus = RUNNING;

			WaitForSingleObject(mastermutexes[j], INFINITE);

			jstatus = ppbaggies[j]->getstatus();

			ReleaseMutex(mastermutexes[j]);



			if (jstatus == FINISHED) {


				//WaitForSingleObject(mastermutexes[j], INFINITE);

				//jiterations = ppbaggies[j]->getmeits();

				//double limit = 10000; // (N + 10 - j) * 1000;  // fake termination criterion dictated by the master
				//if (jiterations > limit) {
				//	ppbaggies[j]->setstatustofinished();
				//WaitForSingleObject(consolemutex, INFINITE);
				//printf("master: worker %d has been told to quit", j);
				--numberrunning;
				//ReleaseMutex(consolemutex);
				//}


				//ReleaseMutex(mastermutexes[j]);

				//if (jiterations > 0) {
				//	WaitForSingleObject(consolemutex, INFINITE);
				//	printf("master: worker %d has done %g iterations, limit: %g\n", j,
				//		jiterations, limit);
				//	ReleaseMutex(consolemutex);
				//}

			}

		}
	}

	for (int j = 0; j < J; j++) {
		WaitForSingleObject(pThread[j], INFINITE);
		WaitForSingleObject(consolemutex, INFINITE);

		printf("--> thread %d done\n", j); fflush(stdout);
		delete ppbaggies[j]; // calls destructor
		ReleaseMutex(pThread[j]);
		ReleaseMutex(consolemutex);

	}
	free(ppbaggies);
BACK:
	return retcode;
}



unsigned _stdcall comp_wrapper(void *genericaddress)
{
	baggie *pbaggie = (baggie *)genericaddress;

	//	comp(pbag->v1, pbag->v2, pbag->v3, &(pbag->result));
		//comp(pbag);

	pbaggie->baggiecomp();

	return 0;
}

