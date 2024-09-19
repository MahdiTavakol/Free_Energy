#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

#include "Jr.h"
using namespace std;

string version = "5.02";


int main(int argc, char** argv)
{
	cout << "Usage: " << endl;
	cout << "./Jar_v" << version <<  " numFolders numSims numDataPerSim folder1 folder2 ...." << endl;
	string logName = "Jar_v" + version + ".log";
	string command = "";
	string space = " ";
	for (int i = 0; i < argc; i++) {
		char * str = argv[i];
		command +=  space + str;
		}
	writelog(logName,command);
	int numFolders     = stoi(argv[1]);
	int numSims        = stoi(argv[2]);
	int numData        = stoi(argv[3]);
	
	
	string * folders = new string[numFolders];
	for (int i = 0; i < numFolders; i++)
	{
		folders[i] = argv[3+1+i];
	}


	int numDirs = 2;
	
	

	string z1file = "z1s.txt";
	string z2file = "z2s.txt";
	string z3file = "z3s.txt";
	string ffile = "fs.txt";
	string wfile = "ws.txt";

	string z1name = "z1";
	string z2name = "z2";
	string z3name = "z3";
	string fname = "f";
	string wname = "w";

	string outputFile = "FreeEnergy.dat";
	
	string exceptionsFileName = "Exceptions.txt";

	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
	cout << "Initializing the arrays" << endl;
	int* step = nullptr;
	double* z1 = nullptr;
	double* z2 = nullptr;
	double* z3 = nullptr;
	double* f = nullptr;
	double* w = nullptr;
	long double* gExp = nullptr;
	double* gSec = nullptr;
	double* gBar = nullptr;
	double* wAvg = nullptr;
	int* exceptions  = nullptr;
	double* z1avg = nullptr;

	zeros(step, numDirs*numData); // forward-reverse
	zeros(z1, numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	zeros(z2, numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	zeros(z3, numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	zeros(f,  numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	zeros(w,  numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	zeros(gExp,numData * numDirs); // forward-reverse
	zeros(gBar,numData * numDirs); // forward-reverse
	zeros(gSec,numData * numDirs); // forward-reverse
	zeros(wAvg,numData * numDirs); // forward-reverse
	zeros(z1avg, numData * numDirs); // forward-reverse
	zeros(exceptions,numFolders*numSims);

	cout << "Finished initializing the arrays" << endl;
	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
	cout << "Reading the simulation data" << endl;
	readSims(numFolders,numSims, numData, folders, exceptions ,step, z1, z2, z3, f);
	
	
	cout << "Writing the simulation data" << endl;
	writeData(numFolders,numSims, numData, step, z1, z1file, z1name, exceptions);
	writeData(numFolders,numSims, numData, step, z2, z2file, z2name, exceptions);
	writeData(numFolders,numSims, numData, step, z3, z3file, z3name, exceptions);
	writeData(numFolders,numSims, numData, step, f,  ffile,  fname, exceptions);
	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
	cout << "Calculating the z1 average" << endl;
	average(numFolders,numSims, numData, z1, z1avg, exceptions);
	
	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
	
	cout << "Calculating the work" << endl;
	calcWork(numFolders,numSims, numData, exceptions, z1, f, w);
	cout << "Writing the work" << endl;
	writeData(numFolders,numSims, numData, step, w,  wfile,  fname,  exceptions);
	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
	cout << "Calculating the free energy" << endl;
	calcJar(numFolders, numSims, numData, w, exceptions, gExp, gSec, wAvg);
	calcBar(numFolders, numSims, numData,gBar);
	/* ----------------------------------------------------------------
	
	   ---------------------------------------------------------------- */
	cout << "Shifting the free energy" << endl;
	shiftJar(numData, gExp, gSec, gBar, wAvg);
	/* ----------------------------------------------------------------
	
	   ---------------------------------------------------------------- */
	cout << "Writing the free energy" << endl;
	writeOutput(outputFile, numData, step, z1avg, gExp, gSec, gBar, wAvg);
	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
	cout << "Writing the exceptions" << endl;
	writeExceptions(exceptionsFileName, exceptions, numSims , numFolders);

	cout << "Cleaning up" << endl;

	delete[] step;
	delete[] z1;
	delete[] z2;
	delete[] z3;
	delete[] f;
	delete[] w;
	delete[] gExp;
	delete[] gSec;
	delete[] wAvg;
	delete[] exceptions;

	cout << "All done!" << endl;
	return 0;
}

