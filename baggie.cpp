

#include <windows.h> 
#include <process.h>
#include "baggie.h"

double mytimecheck(void);

// implementation file for class baggie

baggie::baggie(double alpha_in, double pi_in, int N_in, int T_in, int name_in)
{
	alpha = alpha_in; pi = pi_in; 
	N = N_in; T = T_in;
	name = name_in;
	result = 0;
	status = WAITING;
	iterationsdone = -1;

}

void baggie::setconsolemutex(HANDLE consolemutexinput)
{
	consolemutex = consolemutexinput;
}
void baggie::setmastermutex(HANDLE mastermutexinput)
{
	mastermutex = mastermutexinput;
}

void baggie::letmein(void)
{
	char icangoin;
	int localinheavysection;

	icangoin = 0;
	while (icangoin == 0) {
		Sleep(1000);
		WaitForSingleObject(heavysectionmutex, INFINITE);

		if ((*address_of_nowinheavysection) < maxworkersinheavysection) {
			/** key logic: it checks to see if the number of workers in the heavy section is less than the
			number we want to allow **/
			icangoin = 1;
			++*address_of_nowinheavysection; //** increase the count
			localinheavysection = *address_of_nowinheavysection;
			// so localinheavysection will have the count of busy workers
		}

		ReleaseMutex(heavysectionmutex);
	}
	WaitForSingleObject(consolemutex, INFINITE);
	cout << "******worker" << name << ": I'm in and there are " << localinheavysection << " total busy workers\n";  fflush(stdout);
	// we can use localinheavysection without protecting it with a mutex, because it is a local variable to this function, i.e.
	// it is not shared with other mutexes
	ReleaseMutex(consolemutex);
}

void baggie::seeya(void)
{

	WaitForSingleObject(heavysectionmutex, INFINITE);
	--*address_of_nowinheavysection;
	ReleaseMutex(heavysectionmutex);

}
void baggie::baggiecomp(void)
{
	int i, j, k, M, iteration;
	double outeriterations;
	int othercounter;
	double value;

	M = 100000;
	value = 0;
	iteration = 0;
	othercounter = 0;
	outeriterations = 0;
	status = RUNNING;
	for (i = 0; i < M; i++) {


		letmein(); // check to see if we can become busy

		double t1 = mytimecheck();  // see the comments below.  mytimecheck() returns the time of day in milliseconds
									// it is defined in mytimer.cpp

		this->solve();

		//for (j = 0; j < 10 * M; j++) {
		//	for (k = 0; k < M; k++) {
		//		value += i * v1 + j * j*v2 + k * k*k*v3;
		//		++iteration;
		//		if (iteration == 1000000000) {
		//			// grab the mutex
		//			WaitForSingleObject(consolemutex, INFINITE);
		//			printf("worker %d: value %g, total iterations %g\n", name, value,
		//				outeriterations);
		//			// release the mutex
		//			ReleaseMutex(consolemutex);
		//			iteration = 0;
		//		}
		//		++othercounter;

		//		if (othercounter == 50000000) {
		//			char letmeout = 0;

		//			WaitForSingleObject(mastermutex, INFINITE);

		//			iterationsdone = outeriterations;

		//			if (status == FINISHED) { // this is a status that would be set by the master
		//				letmeout = 1;
		//			}

		//			ReleaseMutex(mastermutex);
		//			othercounter = 0;

		//			if (letmeout) {
		//				//quit!!!
		//				WaitForSingleObject(consolemutex, INFINITE);
		//				printf("-->worker %d: I have been told to quit\n", name);
		//				ReleaseMutex(consolemutex);
		//				itsdone = 1;  // variable "itsdone" is used to record the fact that we have to quit
		//							  // "itsdone" was initialized to 0 above
		//			}
		//		}

		//		if (outeriterations == 5000000000000) {
		//			itsdone = 1; //quit
		//		}

		//		if (itsdone) break;

		//	}

		//	if (itsdone) break;
		//	++outeriterations;
		//}

		double t2 = mytimecheck();  // check out to see how this function works, it's in mytimer.cpp
									// mytimecheck simply returns the time of day in milliseconds
		double tdiff;

		tdiff = t2 - t1;  // t1 was set above 

		WaitForSingleObject(consolemutex, INFINITE);
		printf(" >> worker %d:  I have completed the job in time %g, the result is: %4.9g\n", name, tdiff, result); fflush(stdout);
		ReleaseMutex(consolemutex);

		seeya();

		//WaitForSingleObject(consolemutex, INFINITE);
		//printf(" >> worker %d:  I am out\n", name); fflush(stdout);
		//ReleaseMutex(consolemutex);

		if (status == FINISHED) break;
	}

	//result = value;
}

int _stdcall baggie::solve() {

	int retcode = 0;

	int j, t;

	double *optimal0, *optimal1, candidate, bestone0, bestone1, *shift0, *shift1;

	int *execution0, *execution1, *best_executions;

	int bestk0, bestk1;

	double *prices;

	optimal0 = (double *)calloc((N + 1)*T, sizeof(double));

	optimal1 = (double *)calloc((N + 1)*T, sizeof(double));

	execution0 = (int *)calloc((N + 1)*T, sizeof(int));

	execution1 = (int *)calloc((N + 1)*T, sizeof(int));





	if (!optimal0 || !optimal1 || !execution0 || !execution1) {
		WaitForSingleObject(consolemutex, INFINITE);
		printf(" >> worker %d: cannot allocate large matrix\n", name);  fflush(stdout);
		ReleaseMutex(consolemutex);
		retcode = 2; return retcode;

	}





	shift0 = (double *)calloc(N + 1, sizeof(double));

	shift1 = (double *)calloc(N + 1, sizeof(double));



	if (!shift0 || !shift1) {
		WaitForSingleObject(consolemutex, INFINITE);
		printf(">> worker %d: cannot allocate large array\n", name);  fflush(stdout);
		ReleaseMutex(consolemutex);

		retcode = 2; return retcode;

	}



	prices = (double *)calloc(T, sizeof(double));

	best_executions = (int *)calloc(T, sizeof(int));



	if (!prices || !best_executions) {
		WaitForSingleObject(consolemutex, INFINITE);
		printf(">> worker %d: cannot allocate large array\n", name);  fflush(stdout);
		ReleaseMutex(consolemutex);

		retcode = 2; return retcode;

	}



	for (j = 0; j <= N; j++) {

		if (j <= 99) {

			shift0[j] = 1;

			shift1[j] = 1;

		}

		else if (j <= 900) {

			shift0[j] = 1 - alpha * pow((double)log(j), pi);

			shift1[j] = shift0[j];

		}

		else {

			shift0[j] = 1 - alpha * pow((double)log(j), 2 * pi);

			shift1[j] = 1 - alpha * pow((double)log(j), 3 * pi);

		}

	}





	/** do last stage **/

	for (j = 0; j <= N; j++) {

		if (j <= 99) {

			optimal0[(T - 1)*(N + 1) + j] = j;

			optimal1[(T - 1)*(N + 1) + j] = j;

			execution0[(T - 1)*(N + 1) + j] = j;

			execution1[(T - 1)*(N + 1) + j] = j;



		}

		else if (j <= 900) {

			bestone0 = 99;

			bestk0 = 99;

			bestk1 = 99;

			for (int k = 1; k <= min(9, int(floor(j / 100.0))); ++k) {

				candidate = shift0[100 * k] * 100 * k;

				if (candidate > bestone0) {

					bestone0 = candidate;

					bestk0 = 100 * k;

					bestk1 = 100 * k;

				}

			}

			optimal0[(T - 1)*(N + 1) + j] = bestone0;

			optimal1[(T - 1)*(N + 1) + j] = bestone0;

			execution0[(T - 1)*(N + 1) + j] = bestk0;

			execution1[(T - 1)*(N + 1) + j] = bestk1;



		}

		else {

			bestone0 = 99;

			bestone1 = 99;

			bestk0 = 99;

			bestk1 = 99;

			for (int k = 1; k <= 9; ++k) {

				candidate = shift0[100 * k] * 100 * k;

				if (candidate > bestone0) {

					bestone0 = candidate;

					bestk0 = 100 * k;

				}

				if (candidate > bestone1) {

					bestone1 = candidate;

					bestk1 = 100 * k;

				}

			}

			for (int k = 1; k <= int(floor(j / 1000.0)); ++k) {

				candidate = shift0[1000 * k] * 1000 * k;

				if (candidate > bestone0) {

					bestone0 = candidate;

					bestk0 = 1000 * k;

				}

				candidate = shift1[1000 * k] * 1000 * k;

				if (candidate > bestone1) {

					bestone1 = candidate;

					bestk1 = 1000 * k;



				}

			}

			optimal0[(T - 1)*(N + 1) + j] = bestone0;

			optimal1[(T - 1)*(N + 1) + j] = bestone1;

			execution0[(T - 1)*(N + 1) + j] = bestk0;

			execution1[(T - 1)*(N + 1) + j] = bestk1;

		}

	}



	/* in the middle */

	for (t = T - 2; t >= 0; t--) {

		for (j = 0; j <= N; j++) {



			bestone0 = 0;

			bestone1 = 0;

			bestk0 = 0;

			bestk1 = 0;

			/** enumerate possibilities **/

			for (int k = 0; k <= min(99, j); ++k) {

				candidate = shift0[k] * (k + optimal0[(t + 1)*(N + 1) + j - k]);

				if (candidate > bestone0) {

					bestone0 = candidate;

					bestk0 = k;

				}

				if (candidate > bestone1) {

					bestone1 = candidate;

					bestk1 = k;

				}

			}

			for (int k = 1; k <= min(9, int(floor(j / 100.0))); ++k) {

				candidate = shift0[100 * k] * (100 * k + optimal1[(t + 1)*(N + 1) + j - 100 * k]);

				if (candidate > bestone0) {

					bestone0 = candidate;

					bestk0 = 100 * k;

				}

				if (candidate > bestone1) {

					bestone1 = candidate;

					bestk1 = 100 * k;



				}

			}

			for (int k = 1; k <= int(floor(j / 1000.0)); ++k) {

				candidate = shift0[1000 * k] * (1000 * k + optimal1[(t + 1)*(N + 1) + j - 1000 * k]);

				if (candidate > bestone0) {

					bestone0 = candidate;

					bestk0 = 1000 * k;



				}

				candidate = shift1[1000 * k] * (1000 * k + optimal1[(t + 1)*(N + 1) + j - 1000 * k]);

				if (candidate > bestone1) {

					bestone1 = candidate;

					bestk1 = 1000 * k;



				}

			}


			optimal0[t*(N + 1) + j] = bestone0;

			optimal1[t*(N + 1) + j] = bestone1;

			execution0[t*(N + 1) + j] = bestk0;

			execution1[t*(N + 1) + j] = bestk1;

		}

		//printf("done with stage t = %d\n", t);

	}



	//printf("optimal value for trade sequencing = %4.9f\n", optimal0[N]);

	//double P = 1.0;

	//double sum = 0.0;

	//int N_remaining = N;

	//int large = 0;

	//int bestk;

	//for (int t = 0; t < T; ++t) {

	//	if (large == 0) {

	//		bestk = execution0[t*(N + 1) + N_remaining];

	//		P *= shift0[bestk];

	//	}

	//	else {

	//		bestk = execution1[t*(N + 1) + N_remaining];

	//		P *= shift1[bestk];

	//	}

	//	if (bestk >= 100) {

	//		large = 1;

	//	}

	//	else {

	//		large = 0;

	//	}

	//	N_remaining -= bestk;

	//	best_executions[t] = bestk;

	//	prices[t] = P;

	//	//printf("When t = %d, we sell %d, the price now is %g\n", t, bestk, P);

	//	sum += P * bestk;

	//}

	//printf("check by dot product: %4.9f\n", sum);

	this->result = optimal0[N];

	free(optimal0);
	free(optimal1);
	free(shift0);
	free(shift1);
	free(execution0);
	free(execution1);
	free(best_executions);
	free(prices);



	//this->itsdone = 1;
	this->status = FINISHED;



	return retcode;



}