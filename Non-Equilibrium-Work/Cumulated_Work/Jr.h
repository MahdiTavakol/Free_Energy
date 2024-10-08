#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstdlib> // For system()
using namespace std;



template<typename T>
void zeros(T*& array, const int& size)
{
	array = new T[size];
	for (int i = 0; i < size; i++)
		array[i] = T();
}

void readSims(const int& numFolders, const int& numSims, const int& numData,
	string* folders, int* exceptions,
	int* step, double* z1, double* z2, double* z3, double* f)
{
	for (int z = 0; z < numFolders; z++)
		for (int i = 0; i < numSims; i++)
		{
			int addedExcepts = 0;
			ifstream file;
			stringstream ss;
			string fileNames[2];
			string line;


			fileNames[0] = folders[z] + "/run-" + to_string((i + 1) * 1000000) + "/forward/" + "smd.out";
			fileNames[1] = folders[z] + "/run-" + to_string((i + 1) * 1000000) + "/reverse/" + "smd.out";

			for (int k = 0; k < 2; k++)
			{
				int j = 0;
				if (true)
				{
					file.open(fileNames[k]);
					if (!file.is_open())
						cerr << "Error opening file: " << fileNames[k];

					getline(file, line); // Skipping the empty line

					while (getline(file, line) && j < numData)
					{
						int stepi;
						double z1i;
						double z2i;
						double z3i;
						double fi;
						int indx;
						ss << line;
						ss >> stepi;
						ss >> z1i;
						ss >> z2i;
						ss >> z3i;
						ss >> fi;
						ss.clear();
						ss.str("");

						indx = i + z * numSims + numFolders * numSims * (j + k * numData);  // (i+ z * numSims, j + k * numData ) 

						if (i == 0 && z == 0) step[j + k * numData] = stepi;

						z1[indx] = z1i;
						z2[indx] = z2i;
						z3[indx] = z3i;
						f[indx] = fi;
						j++;
					}
				}

				if (j < numData - 1)
				{
					cout << "Warning: Only " << j << " data lines instead of were read from " << fileNames[k] << endl;
					exceptions[i + z * numSims] = 1;
				}

				file.close();
			}


		}
}

void writeData(const int& numFolders, const int& numSims, const int& numData,
	int* step, double* data,
	const string fileName, const string dataName,
	int* exceptions)
{
	ofstream file;
	file.open(fileName);
	if (!file.is_open())
		cerr << "Error opening file: " << fileName;

	// Writing the header
	file << "step,";
	for (int z = 0; z < numFolders; z++)
		for (int i = 0; i < numSims; i++)
			file << dataName << "-" << i << "-folder-" << z << ",";
	file << "<" << dataName << ">,";
	file << "<" << dataName << "^2>,";
	file << "std,";
	file << "ErrorOfMean" << endl;

	// forward reverse so j < 2 * numData
	for (int z = 0; z < 1; z++)
		for (int j = 0; j < 2 * numData; j++)
		{
			float stepj, datai, std, err;
			float dataAvg = 0.0;
			float data2Avg = 0.0;
			stepj = step[j];
			file << stepj;
			// Two folders so i < 2*numSims
			for (int i = 0; i < numFolders * numSims; i++)
			{
				int  indx = i + numFolders * numSims * j;  // (i+ z * numSims, j + k * numData ) 
				datai = data[indx];
				file << "," << datai;
				if (exceptions[i] == 1) continue;
				dataAvg += datai;
				data2Avg += datai * datai;

			}

			int totExcept = 0;
			for (int k = 0; k < numFolders * numSims; k++) totExcept += exceptions[k];

			dataAvg /= double(numFolders * numSims - totExcept);
			data2Avg /= double(numFolders * numSims - totExcept);
			std = data2Avg - dataAvg * dataAvg;
			err = std / sqrt(double(numFolders * numSims - totExcept));
			file << "," << dataAvg;
			file << "," << data2Avg;
			file << "," << std;
			file << "," << err << endl;
		}
	file.close();
}

void calcWork(const int& numFolders, const int& numSims, const int& numData, int* exceptions, double* z1, double* f, double* w)
{
	// Two folders so i < 2 * numSims
	for (int i = 0; i < numFolders * numSims; i++)
	{
		double wj = 0.0;
		w[i] = wj;
		// Forward-Reverse so j < 2*numData  
		for (int j = 1; j < 2 * numData; j++)
		{
			/*if (i == numData/2)
			{
				cout << "Calculating the reverse work for sim " << j << endl;
				wi = 0.0;
			}*/

			int indx1 = i + numFolders * numSims * (j - 1);
			int indx2 = i + numFolders * numSims * j;
			//int indx1 =  k * 2 * numSims * numData + (j-1) * 2 * numSims + i + numSims * z;
			//int indx2 =  k * 2 * numSims * numData + (j) * 2 * numSims + i + numSims * z;
			if (exceptions[i] == 0) {
				if (j < numData)
					wj = wj + (f[indx1] + f[indx2]) * (z1[indx2] - z1[indx1]) / 2.0;
				if (j >= numData)
					wj = wj - (f[indx1] + f[indx2]) * (z1[indx2] - z1[indx1]) / 2.0;
			}
			w[indx2] = wj;
		}
	}

	int len = 2 * numData * 5 / 100;
	double* wavg = new double[numFolders * numSims];
	for (int i = 0; i < numFolders * numSims; i++)
	{
		wavg[i] = 0.0;
		for (int j = numData - len / 2; j < numData + len / 2; j++)
			wavg[i] += w[j * numFolders * numSims + i];
		wavg[i] /= len;
	}
	for (int i = 0; i < numFolders * numSims; i++)
		for (int j = 0; j < 2 * numData; j++)
			w[j * numFolders * numSims + i] -= wavg[i];

	delete[] wavg;
}

void calcJar(const int& numFolders, const int& numSims, const int& numData, double* w, int* exceptions,
	long double* gExp, double* gSec, double* wAvg)
{
	double beta = 1 / 0.592; // 1/kT = 1/0.529 (mol/kCal)
	for (int z = 0; z < 1; z++)
		for (int i = 0; i < 2 * numData; i++)
		{
			long double gexp = 0.0;
			double gsec = 0.0;
			double wavg = 0.0;
			double w2avg = 0.0;
			for (int j = 0; j < numFolders * numSims; j++)
			{
				bool except = false;
				if (exceptions[j] == 1) except = true;
				if (!except)
				{
					int indx = i * numFolders * numSims + j;
					double wi = w[indx];
					gexp += exp((long double)(-beta * wi));
					wavg += wi;
					w2avg += wi * wi;
				}

			}

			int totExcept = 0;
			for (int k = 0; k < numFolders * numSims; k++) totExcept += exceptions[k];

			gexp /= (long double)(numFolders * numSims - totExcept);
			gexp = -(1.0 / beta) * log(gexp);
			wavg /= (double)(numFolders * numSims - totExcept);
			w2avg /= (double)(numFolders * numSims - totExcept);
			gsec = wavg - (beta / 2) * (w2avg - wavg * wavg);

			gExp[i] = gexp;
			gSec[i] = gsec;
			wAvg[i] = wavg;
		}
}

void calcBar(const int& numFolders, const int& numSims, const int& numData, double* gBar)
{
	fstream BAR_file;
	string line;
	stringstream iss;
	std::string command = "./BAR.py " + std::to_string(numFolders) + " " + std::to_string(numSims) + " " + std::to_string(numData);

	// Execute the command using system()
	int result = system(command.c_str());

	

	// Check the result
	if (result == 0) {
		
	}
	else {
		std::cout << "BAR.py execution failed." << std::endl;
	}

	BAR_file.open("BAR.dat");

	int i = -2;
	while (std::getline(BAR_file, line))
	{
		i++;
		double double_str;
		double g;
		if (i == -1) continue;
		iss.clear();
		iss.str("");
		iss << line;
		iss >> double_str;
		iss >> g;
		iss >> double_str;
		gBar[i] = g;
	}
	BAR_file.close();
	
	//for (int i = 0; i < numData; i++) gBar[numData-i-1] = gBar[i];
}

void writeOutput(const string& outputFile, const int& numData,
	int* step, double* z1, long double* gExp, double* gSec, double* gBar, double* wAvg)
{
	ofstream file;
	file.open(outputFile);
	if (!file.is_open())
		cerr << "Error opening file: " << outputFile;

	// Writing the header
	file << "step,";
	file << "z1,";
	file << "gExp,";
	file << "gSec,";
	file << "gBar,";
	file << "wAvg" << endl;

	for (int i = 0; i < 2 * numData; i++) {
		file << step[i] << "," << z1[i] << "," << gExp[i] << "," << gSec[i] << "," << gBar[i] << "," << wAvg[i] << endl;
	}
	file.close();
}

void average(const int& numFolders, const int& numSims, const int& numData,
	double* z1, double* z1avg,
	int* exceptions)
{
	for (int z = 0; z < 1; z++)
		for (int i = 0; i < 2 * numData; i++)
		{
			double avg = 0.0;
			for (int j = 0; j < numFolders * numSims; j++)
			{
				if (exceptions[j] == 1) continue;
				avg += z1[i * numFolders * numSims + j]; //[i,j+z*numSims]
			}
			int totExcept = 0;
			for (int k = 0; k < numFolders * numSims; k++) totExcept += exceptions[k];
			avg /= double(numFolders * numSims - totExcept);
			z1avg[i] = avg;
		}
}


void shiftJar(const int& numData, long double* gExp, double* gSec, double * gBar, double* wAvg)
{
	int len = numData * 5 / 100;
	double gExpNeg, gExpPos, gSecNeg, gSecPos, gBarPos, gBarNeg, wAvgNeg, wAvgPos;
	gExpNeg = 0.0;
	gExpPos = 0.0;
	gSecNeg = 0.0;
	gSecPos = 0.0;
	gBarNeg = 0.0;
	gBarPos = 0.0;
	wAvgNeg = 0.0;
	wAvgPos = 0.0;

	for (int i = numData - len; i < numData; i++)
	{
		gExpNeg += gExp[i];
		gSecNeg += gSec[i];
		gBarNeg += gBar[i];
		wAvgNeg += wAvg[i];
	}
	for (int i = numData; i < numData + len; i++)
	{
		gExpPos += gExp[i];
		gSecPos += gSec[i];
		gBarPos += gBar[i];
		wAvgPos += wAvg[i];
	}
	gExpNeg /= len;
	gExpPos /= len;
	gSecNeg /= len;
	gSecPos /= len;
	gBarNeg /= len;
	gBarPos /= len;
	wAvgNeg /= len;
	wAvgPos /= len;

	for (int i = 0; i < numData; i++)
	{
		gExp[i] -= gExpNeg;
		gSec[i] -= gSecNeg;
		gBar[i] -= gBarNeg;
		wAvg[i] -= wAvgNeg;
	}
	for (int i = numData; i < 2 * numData; i++)
	{
		gExp[i] -= gExpPos;
		gSec[i] -= gSecPos;
		gBar[i] -= gBarPos;
		wAvg[i] -= wAvgPos;
	}
}

void writeExceptions(const string& exceptionFileName, const int* exceptions, const int& numSims, const int& numFolders)
{
	ofstream file;
	file.open(exceptionFileName);
	if (!file.is_open())
		cerr << "Error opening file: " << exceptionFileName;

	for (int j = 0; j < numFolders; j++)
		for (int i = 0; i < numSims; i++)
		{
			int indx = j * numSims + i;
			if (exceptions[indx] == 1)
				file << (j + 1) << "-Series-" << (j + 1) << "/run-" << (i + 1) * 1000000 << endl;
		}
	file.close();
}

void writelog(const string& logFileName, const string& command)
{
	ofstream file;
	file.open(logFileName);
	if (!file.is_open())
		cerr << "Error opening file: " << logFileName;
	file << command << endl;
	file.close();
}
