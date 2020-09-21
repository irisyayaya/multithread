#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>

using namespace std;

#define RUNNING 0
#define WAITING 1
#define FINISHED 2


class baggie {
private:
	double alpha, pi;
	int N, T;
	int name;
	double result = 0;
	char status;
	int iterationsdone;
	HANDLE consolemutex;
	HANDLE mastermutex;
	HANDLE heavysectionmutex;
	int* address_of_nowinheavysection;
	int maxworkersinheavysection;
	//int itsdone = 0;
public:
	baggie(double, double, int, int, int);
	void setconsolemutex(HANDLE consolemutexinput);
	void setmastermutex(HANDLE mastermutexinput);
	void letmein(void);
	void seeya(void);
	void baggiecomp(void);
	~baggie() {
		/*if (address_of_nowinheavysection){
			delete address_of_nowinheavysection;
			address_of_nowinheavysection = NULL;
		}*/
	}

	void setmaxworkersinheavysection(int W) { maxworkersinheavysection=W; }
	void setheavysectionmutex(HANDLE heavymutex) { heavysectionmutex = heavymutex; }
	void setnowinheavyaddress(int *address_nowinheavy) { address_of_nowinheavysection = address_nowinheavy; }

	char getstatus() { return status; }
	int getmeits() { return iterationsdone; }
	double getresult() { return result; }
	void setstatustofinished() { status = FINISHED; }

	int _stdcall solve();
};