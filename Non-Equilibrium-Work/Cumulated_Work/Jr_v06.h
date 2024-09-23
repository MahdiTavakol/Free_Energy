#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstdlib> // For system()
using namespace std;

struct dataFormat {
	int numDirs;
	int numFolders;
	int numSims;
	int numData;
};

template<typename T>
void zeros(T*& array, const int& size)
{
	array = nullptr;
	array = new T[size]();
}

template<typename T>
void zeros(T**& array, const int& nrows, const int& ncols)
{
	array = nullptr;
	T* temp = new T[nrows * ncols]();
	array = new T*[nrows];
	for (int i = 0; i < nrows; i++)
		array[i] = &temp[i * ncols];		
}

template<typename T>
void deallocate(T*& array)
{
	delete[] array;
}

template<typename T>
void deallocate(T**& array)
{
	delete[] array[0];
	delete[] array;
}

void readSims(const dataFormat& df, string* folders, int * exceptions, 
	int* step, double** z1, double** z2, double** z3, double** f)
{
	for (int z = 0; z < df.numFolders; z++)
		for (int i = 0; i < df.numSims; i++)
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

					while (getline(file, line) && j < df.numData)
					{
						int stepi;
						double z1i;
						double z2i;
						double z3i;
						double fi;
						int indxi, indxj;
						ss << line;
						ss >> stepi;
						ss >> z1i;
						ss >> z2i;
						ss >> z3i;
						ss >> fi;
						ss.clear();
						ss.str("");

						indxi = i + z * df.numSims;
						indxj = j + k * df.numData;

						if (i == 0 && z == 0) step[indxj] = stepi;

						z1[indxj][indxi] = z1i;
						z2[indxj][indxi] = z2i;
						z3[indxj][indxi] = z3i;
						f[indxj][indxi] = fi;
						j++;
					}
				}

				if (j < df.numData - 1)
				{
					cout << "Warning: Only " << j << " data lines instead of were read from " << fileNames[k] << endl;
					exceptions[i + z * df.numSims] = 1;
				}

				file.close();
			}


		}
}

void writeData(const dataFormat& df,
	int *step, double** data, string fileName, const string dataName, 
	int *exceptions)
{
	ofstream file;
	file.open(fileName);
	if (!file.is_open())
		cerr << "Error opening file: " << fileName;

	// Writing the header
	file << "step,";
	for (int z = 0; z < df.numFolders; z++)
		for (int i = 0; i < df.numSims; i++)
			file << dataName << "-" << i << "-folder-" << z << ",";
	file << "<" << dataName << ">,";
	file << "<" << dataName << "^2>,";
	file << "std,";
	file << "ErrorOfMean" << endl;

	
	for (int j = 0; j < df.numDirs * df.numData; j++)
	{
		int stepj;
		double datai, std, err;
		double dataAvg = 0.0;
		double data2Avg = 0.0;
		stepj = step[j];
		file << stepj;

		for (int i = 0; i < df.numFolders * df.numSims; i++)
		{
			datai = data[j][i];
			file << "," << datai;
			if (exceptions[i] == 1) continue;
			dataAvg += datai;
			data2Avg += datai * datai;

		}

		int totExcept = 0;
		for (int k = 0; k < df.numFolders * df.numSims; k++) totExcept += exceptions[k];

		dataAvg /= double(df.numFolders * df.numSims - totExcept);
		data2Avg /= double(df.numFolders * df.numSims - totExcept);
		std = data2Avg - dataAvg * dataAvg;
		err = std / sqrt(double(df.numFolders * df.numSims - totExcept));
		file << "," << dataAvg;
		file << "," << data2Avg;
		file << "," << std;
		file << "," << err << endl;
	}
	file.close();
}

void calcWork(const dataFormat& df,
	int* exceptions, double** z1, double** f, double** w)
{
	for (int i = 0; i < df.numFolders * df.numSims; i++)
	{
		double wj = 0.0;
		w[0][i] = wj;

		for (int j = 1; j < df.numDirs * df.numData; j++)
		{
			/*if (i == numData/2)
			{
				cout << "Calculating the reverse work for sim " << j << endl;
				wi = 0.0;
			}*/

			//int indx1 =  k * 2 * numSims * numData + (j-1) * 2 * numSims + i + numSims * z;
			//int indx2 =  k * 2 * numSims * numData + (j) * 2 * numSims + i + numSims * z;
			if (exceptions[i] == 0) {
				if (j < df.numData)
					wj += (f[j-1][i] + f[j][i]) * (z1[j][i]-z1[j-1][i])/2.0;
				if (j == df.numData)
				{
					// Do nothing since j == df.numData and j == df.numData - 1 are both the same point. 
				}
				if (j > df.numData)
					wj -= (f[j-1][i] + f[j][i]) * (z1[j][i]-z1[j-1][i])/2.0;
			}
			w[j][i] = wj;
		}
	}

	int len = 2 * df.numData * 5 / 100;
	double* wavg = new double[df.numFolders * df.numSims];
	for (int i = 0; i < df.numFolders * df.numSims; i++)
	{
		wavg[i] = 0.0;
		for (int j = df.numData - len / 2; j < df.numData + len / 2; j++)
			wavg[i] += w[j][i];
		wavg[i] /= len;
	}
	for (int i = 0; i < df.numFolders * df.numSims; i++)
		for (int j = 0; j < df.numDirs * df.numData; j++)
			w[j][i] -= wavg[i];

	delete[] wavg;
}

void calcJar(const dataFormat& df , double** w, int* exceptions,
	long double* gExp, double* gSec, double* wAvg)
{
	double beta = 1 / 0.592; // 1/kT = 1/0.529 (mol/kCal)

	for (int i = 0; i < df.numDirs * df.numData; i++)
	{
			long double gexp = 0.0;
			double gsec = 0.0;
			double wavg = 0.0;
			double w2avg = 0.0;
			for (int j = 0; j < df.numFolders * df.numSims; j++)
			{
				if (exceptions[j] == 0)
				{
					double wi = w[i][j];
					gexp += exp((long double)(-beta * wi));
					wavg += wi;
					w2avg += wi * wi;
				}
			}

			int totExcept = 0;
			for (int k = 0; k < df.numFolders * df.numSims; k++) totExcept += exceptions[k];

			gexp /= (long double)(df.numFolders * df.numSims - totExcept);
			gexp = -(1.0 / beta) * log(gexp);
			wavg /= (double)(df.numFolders * df.numSims - totExcept);
			w2avg /= (double)(df.numFolders * df.numSims - totExcept);
			gsec = wavg - (beta / 2) * (w2avg - wavg * wavg);

			gExp[i] = gexp;
			gSec[i] = gsec;
			wAvg[i] = wavg;
	}
}

void calcBar(const dataFormat& df, double* gBar)
{
	fstream BAR_file;
	string line;
	stringstream iss;
	std::string command = "./BAR.py " + std::to_string(df.numFolders) + " " + std::to_string(df.numSims) + " " + std::to_string(df.numData);

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

void writeOutput(const dataFormat& df, const string& outputFile,
	int* step, double* z1avg, long double* gExp, double* gSec, double* gBar, double* wAvg)
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
	file << "wAvg";
	file << endl;

	for (int i = 0; i < df.numDirs * df.numData; i++) {
		file << step[i] << "," << z1avg[i] << "," << gExp[i] << "," << gSec[i] << "," << gBar[i] << "," << wAvg[i] << endl;
	}
	file.close();
}

void average(const dataFormat& df,
	double** z1, double* z1avg,
	int* exceptions)
{
	for (int z = 0; z < 1; z++)
		for (int i = 0; i < df.numDirs * df.numData; i++)
		{
			double avg = 0.0;
			for (int j = 0; j < df.numFolders * df.numSims; j++)
			{
				if (exceptions[j] == 1) continue;
				avg += z1[i][j];
			}
			int totExcept = 0;
			for (int k = 0; k < df.numFolders * df.numSims; k++) totExcept += exceptions[k];
			avg /= double(df.numFolders * df.numSims - totExcept);
			z1avg[i] = avg;
		}
}


void shiftJar(const dataFormat& df, long double* gExp, double* gSec, double * gBar, double* wAvg)
{
	int len = df.numData * 5 / 100;
	double gExpNeg, gExpPos, gSecNeg, gSecPos, gBarPos, gBarNeg, wAvgNeg, wAvgPos;
	gExpNeg = 0.0;
	gExpPos = 0.0;
	gSecNeg = 0.0;
	gSecPos = 0.0;
	gBarNeg = 0.0;
	gBarPos = 0.0;
	wAvgNeg = 0.0;
	wAvgPos = 0.0;

	for (int i = df.numData - len; i < df.numData; i++)
	{
		gExpNeg += gExp[i];
		gSecNeg += gSec[i];
		gBarNeg += gBar[i];
		wAvgNeg += wAvg[i];
	}
	for (int i = df.numData; i < df.numData + len; i++)
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

	for (int i = 0; i < df.numData; i++)
	{
		gExp[i] -= gExpNeg;
		gSec[i] -= gSecNeg;
		gBar[i] -= gBarNeg;
		wAvg[i] -= wAvgNeg;
	}
	for (int i = df.numData; i < 2 * df.numData; i++)
	{
		gExp[i] -= gExpPos;
		gSec[i] -= gSecPos;
		gBar[i] -= gBarPos;
		wAvg[i] -= wAvgPos;
	}
}

void writeExceptions(const dataFormat& df, const string& exceptionFileName, const int* exceptions)
{
	ofstream file;
	file.open(exceptionFileName);
	if (!file.is_open())
		cerr << "Error opening file: " << exceptionFileName;

	for (int j = 0; j < df.numFolders; j++)
		for (int i = 0; i < df.numSims; i++)
		{
			int indx = j * df.numSims + i;
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
