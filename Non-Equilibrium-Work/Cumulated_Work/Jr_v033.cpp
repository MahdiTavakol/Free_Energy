#include "Jr_v033.h"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <memory>
#include <random>
#include <cstdlib> // For system()
#include <omp.h>

using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using std::stoi;
using std::to_string;
using std::getline;
using std::stringstream;
using std::cerr;
using std::to_string;
using std::endl;
using std::array;
using std::unique_ptr;
using std::stod;


JarCalculator::JarCalculator(int argc, char** argv)
{
	int iarg;
    logName = std::string(argv[0]) + ".log";
    logFile.open(logName);
	print_message("Usage: ");
    string message = std::string(argv[0]) +  " numFolders numSims numDataPerSim folder1 folder2 ...." ;
    print_message(message);


	numFolders     = stoi(argv[1]);
	numSims        = stoi(argv[2]);
	numData        = stoi(argv[3]);
    
    message = "numFolders == " + to_string(numFolders);
    print_message(message);
    message = "numSims == " + to_string(numSims);
    print_message(message);
    message = "numData == " + to_string(numData);
    print_message(message);
	
	iarg = 4;
	folders.reserve(numFolders);
	for (int i = 0; i < numFolders; i++)
    {
		folders.push_back(argv[iarg]);
        message = "folder" + to_string(i+1) + " == " + argv[iarg];
        print_message(message);
        iarg++;
    }

	mode = modeVal::NORMAL;
	if (argc > iarg) {
		if (argc < iarg+3)
			cerr << "Not enough input arguments" << endl;
		if (argv[iarg] == string("number_analysis"))
        {
			mode = modeVal::NUMBER_ANALYSIS;
            print_message(argv[iarg]);
            iarg++;
        }
		else
			cerr << "Unknown input argument!" << endl;
		numShuffles = stoi(argv[iarg++]);
        message = "numShuffles == " + to_string(numShuffles);
        print_message(message);
		gBarShuffle_filename = argv[iarg++];
        message = "gBarShuffle_filename == " + gBarShuffle_filename;
        print_message(message);
	}

}

template<typename... Args>
JarCalculator::JarCalculator(const string& logName_, const int& numFolders_, const int& numSims_, const int& numData_, Args&&... args):
    logName(logName_), numFolders(numFolders_), numSims(numSims_), numData(numData_)
{
    static_assert((std::is_convertible_v<Args, std::string> && ...), "All arguments must be convertible to std::string");

    vector<string> foldersTmp{ std::forward<Args>(args)... };
    folders = foldersTmp;

    logFile.open(logName);
        
    if (folders.size() != numFolders) 
    {
        std::cerr << "Wrong input arguments " << std::endl;
        logFile << "Wrong input arguments" << std::endl;
    }
}

void JarCalculator::initialize()
{

    string z1file = "z1s.txt";
	string z2file = "z2s.txt";
	string z3file = "z3s.txt";
	string ffile = "fs.txt";
	

	string z1name = "z1";
	string z2name = "z2";
	string z3name = "z3";
	string fname = "f";

	outputFile = "FreeEnergy.dat";
	exceptionsFileName = "Exceptions.txt";

    numDirs = 2;


    /* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */

    print_message("Initializing the vectors");

    // I used resize on purpose so the initial values are set to zero!
    step.resize(numDirs*numData); // forward-reverse
	z1.resize(numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	z2.resize(numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	z3.resize(numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	f.resize(numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	w.resize(numSims * numData * numFolders * numDirs); // forward-reverse + two folders
	gExp.resize(numData * numDirs); // forward-reverse
	gBar.resize(numData * numDirs); // forward-reverse
	gSec.resize(numData * numDirs); // forward-reverse
	wAvg.resize(numData * numDirs); // forward-reverse
	z1avg.resize(numData * numDirs); // forward-reverse
	exceptions.resize(numFolders*numSims);

    print_message("Finished initializing the vectors");


    /* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */

    print_message("Reading the simulation data");
    readSims();
       
       
    print_message("Writing the simulation data");
    writeData(z1, z1file, z1name);
    writeData(z2, z2file, z2name);
    writeData(z3, z3file, z3name);
    writeData(f,  ffile,  fname);


    if (mode == modeVal::NUMBER_ANALYSIS) {
        print_message("Creating the shuffle indexes");
        shuffleIndexes();
    } 
}

void JarCalculator::calculate()
{
    string wfile = "ws.txt";
    string wname = "w";

    print_message("Calculating the z1 average");
	average(z1, z1avg);
	
	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
	
	print_message("Calculating the work");
	calcWork();
	print_message("Writing the work");
	writeData(w,  wfile,  wname);
	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
    if (mode == modeVal::NORMAL) {

        print_message("Calculating the free energy");
        calcJar();
        calcBar();
        /* ----------------------------------------------------------------
        
           ---------------------------------------------------------------- */
        print_message("Shifting the free energy");
        shiftJar();

    } else if (mode == modeVal::NUMBER_ANALYSIS) {

        print_message("Calculating the BAR shuffles");
        calcBarShuffled();
    } else
        cerr << "Unkown error!" << endl;
}

void JarCalculator::write()
{
    if (mode == modeVal::NORMAL) {
        print_message("Writing the free energy");
        writeOutput();
    } else if (mode == modeVal::NUMBER_ANALYSIS) {
        print_message("Writing the BAR shuffles");
        writeBarShuffled();
    } else
        cerr << "Unkown error!" << endl;
	/* ----------------------------------------------------------------

	   ---------------------------------------------------------------- */
	print_message("Writing the exceptions");
	writeExceptions();
}

void JarCalculator::print_message(const string& message)
{
    logFile << message << std::endl;
    std::cout << message << std::endl;
}

void JarCalculator::readSims()
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
				using std::vector;
				vector<double> z1Tmp, z2Tmp, z3Tmp, fTmp;
				z1Tmp.reserve(numData);
				z2Tmp.reserve(numData);
				z3Tmp.reserve(numData);
				fTmp.reserve(numData);
				
				int j = 0;
				if (true)
				{
					file.open(fileNames[k]);
					if (!file.is_open())
						cerr << "Error opening file1: " << fileNames[k];

					getline(file, line); // Skipping the empty line

					while (getline(file, line))
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

						if (i == 0 && z == 0 && j < numData) 
						{
							int stp = (k == 0)?outputFreq*j:outputFreq*(j+k*numData-1);
							step[j + k * numData] = stp;
						}

						
						z1Tmp.push_back(z1i);
						z2Tmp.push_back(z2i);
						z3Tmp.push_back(z3i);
						fTmp.push_back(fi);
						j++;
					}
				}

				if (j < numData )
				{
					string message = "Warning: Only " + std::to_string(j) + " data lines instead of were read from " + fileNames[k];
                    print_message(message);
					exceptions[i + z * numSims] = 1;
					file.close();
					continue;
				}
				

				file.close();
				
				if (j > numData)
				{
					string message = "Warning: Discarding " + std::to_string(j-numData) + " from the end of forward and begining of reserve for " + fileNames[k];
                    print_message(message);
				}
				
				int simIndex = i + z * numSims;
				for (int l = 0; l < numData; l++)
				{
					int timeIndex = l + k * numData; 
					int indx = simIndex + numFolders * numSims * timeIndex;
					int TmpIndx = (k == 0)?l:l+j-numData; 
					z1[indx] = z1Tmp[TmpIndx];
					z2[indx] = z2Tmp[TmpIndx];
					z3[indx] = z3Tmp[TmpIndx];
					f[indx] = fTmp[TmpIndx];
				}
			}
		}
}


void JarCalculator::writeData(const vector<double>& data,
	const string& fileName, const string& dataName)
{
	ofstream file;
	file.open(fileName);
	if (!file.is_open())
		cerr << "Error opening file2: " << fileName;

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

void JarCalculator::average(const vector<double>& in, vector<double>& out)
{
	for (int z = 0; z < 1; z++)
		for (int i = 0; i < 2 * numData; i++)
		{
			double avg = 0.0;
			for (int j = 0; j < numFolders * numSims; j++)
			{
				if (exceptions[j] == 1) continue;
				avg += in[i * numFolders * numSims + j]; //[i,j+z*numSims]
			}
			int totExcept = 0;
			for (int k = 0; k < numFolders * numSims; k++) totExcept += exceptions[k];
			avg /= double(numFolders * numSims - totExcept);
			out[i] = avg;
		}
}


void JarCalculator::calcWork()
{
	// Two folders so i < 2 * numSims
	for (int i = 0; i < numFolders * numSims; i++)
	{
		double wj = 0.0;
		w[i] = wj;
		// Forward-Reverse so j < 2*numData  
		for (int j = 1; j < 2 * numData; j++)
		{
			int indx1 = i + numFolders * numSims * (j - 1); // row based [i,j-1] index with 2*numData rows and numFolders*numSims columns
			int indx2 = i + numFolders * numSims * j; // row based [i,j] index with // // //

			if (exceptions[i] == 0) {
				double favg = (f[indx2]+f[indx1])/2.0;
				double dz = z1[indx2]-z1[indx1];
				double dW = favg*dz;
				if (j < numData)
					wj = wj + dW;
				if (j >= numData)
					wj = wj - dW;
			}
			w[indx2] = wj;
		}
	}
}


void JarCalculator::calcJar()
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

void JarCalculator::calcBar()
{
	ifstream BAR_file;
	string line;
	stringstream iss;
	std::string command = "./BAR-v02.py " + std::to_string(numFolders) + " " + std::to_string(numSims) + " " + std::to_string(numData);

	// Execute the command using system()
	int result = system(command.c_str());

	

	// Check the result
	if (result == 0) {
		
	}
	else {
		print_message("BAR-v02.py execution failed.");
	}

	BAR_file.open("BAR-v02.dat");

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

}

void JarCalculator::calcBarShuffled()
{
	vector<double>& gBar_means = this->gBarShuffle_means;
	vector<double>& gBar_stds = this->gBarShuffle_stds;
    vector<double>& gBar_vars = this->gBarShuffle_vars;

	gBar_means.resize(numFolders*numSims);
	gBar_stds.resize(numFolders*numSims);
    gBar_vars.resize(numFolders*numSims);
    gBarShuffle_vals.resize(numFolders*numSims*numShuffles);


	double etol = 1e-4;


	#pragma omp parallel for
	for (int i = 1; i <= numFolders*numSims; i++)
	{
        double *gBarShuffleI = gBarShuffle_vals.data()+(i-1)*numShuffles;
		for (int j = 0; j < numShuffles; j++)
		{
			ifstream BAR_file;
			string line;
			int result;

			int* shuffleJ = shuffles.data() + numFolders*numSims*j;

			string command = "JAX_ENABLE_X64=True  ./BAR-v03.py " + to_string(numData) + " " + to_string(i);


			for (int k = 0; k < i; k++)
				command += (" " + to_string(shuffleJ[k]));


			array<char,128> buffer;
			string resultString;
			unique_ptr<FILE,decltype(&pclose)> pipe(popen(command.c_str(),"r"),pclose);
			if (!pipe)
			{
				#pragma omp critical
				std::cerr << "Failed to run command: " << command << "\n";
				continue;
			}

			while(fgets(buffer.data(),buffer.size(),pipe.get()))
				resultString += buffer.data();

			try {
				gBarShuffleI[j] = stod(resultString);
			} catch (...) {
				#pragma omp critical
                std::cerr << "Failed to parse BAR output: '" << resultString << "'\n";
				gBarShuffleI[j] = 0.0;
			}
			
		}

		int count = 0;
		double gBar_meanI = std::accumulate(gBarShuffleI,gBarShuffleI + numShuffles,0.0, [&count, etol](double acc, double val) {
			if (std::abs(val) >= etol)
			{
				count++;
				return acc+val;
			}
			else
				return acc;
		});

		double gBar_stdI;
        double gBar_varI;

		if (count >= 2)
		{
			gBar_meanI /= count;

			double sq_sum = std::accumulate(gBarShuffleI,gBarShuffleI + numShuffles,0.0,[gBar_meanI,etol](double acc, double val)
			{
				if (std::abs(val) >= etol)
				{
					double diff = val - gBar_meanI;
					return acc+diff*diff;
				}
				else
					return acc;
			});

            gBar_varI = sq_sum;
			gBar_stdI = std::sqrt(sq_sum/(count-1));
		}
		else
		{
			gBar_meanI = 0.0;
			gBar_stdI = 0.0;
            gBar_varI = 0.0;
		}
		gBar_means[i-1] = gBar_meanI;
		gBar_stds[i-1] = gBar_stdI;
		gBar_vars[i-1] = gBar_varI;
	}

    string message = "\n\n\n\nFinished calculating the shuffled BAR values";
    print_message(message);

}

void JarCalculator::writeBarShuffled()
{
	ofstream output(gBarShuffle_filename);
	if (output.is_open()) cerr << "Cannot open file " << gBarShuffle_filename  << " for writing" << endl;

	int length = numFolders*numSims;

	output << "num-sims,";

    for (int j= 0;j < numShuffles; j++)
        output << "Shuffle-" << j << ",";
    
    
    output << "bar-mean,bar-var,bar-std" << endl;

	for (int i = 0; i < length; i++)
	{
		output << i+1 << ",";
        double* gBarShuffleI = gBarShuffle_vals.data() + i*numShuffles; 
        for (int j = 0; j < numShuffles; j++)
            output << gBarShuffleI[j] << ",";
		output << gBarShuffle_means[i] << ",";
        output << gBarShuffle_vars[i] << ",";
		output << gBarShuffle_stds[i] << endl;
	}
}


void JarCalculator::shiftJar()
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

void JarCalculator::writeOutput()
{
	ofstream file;
	file.open(outputFile);
	if (!file.is_open())
		cerr << "Error opening file3: " << outputFile;

	// Writing the header
	file << "step,";
	file << "z1,";
	file << "gExp,";
	file << "gSec,";
	file << "gBar,";
	file << "wAvg" << endl;

	for (int i = 0; i < 2 * numData; i++) {
		file << step[i] << "," << z1avg[i] << "," << gExp[i] << "," << gSec[i] << "," << gBar[i] << "," << wAvg[i] << endl;
	}
	file.close();
}

void JarCalculator::writeExceptions()
{
	ofstream file;
	file.open(exceptionsFileName);
	if (!file.is_open())
		cerr << "Error opening file4: " << exceptionsFileName;

	for (int j = 0; j < numFolders; j++)
		for (int i = 0; i < numSims; i++)
		{
			int indx = j * numSims + i;
			if (exceptions[indx] == 1)
				file << (j + 1) << "-Series-" << (j + 1) << "/run-" << (i + 1) * 1000000 << endl;
		}
	file.close();
}

void JarCalculator::shuffleIndexes() 
{
	shuffles.resize(numSims*numFolders*numShuffles); // row-based indexing with shuffles in rows and numFolders*numSims in columns so each shuffle in the continguous memory locations.

	std::random_device rd;
	std::mt19937_64 g{rd()};

	// Filling all the shuffles
	for (int i = 0; i < numShuffles; i++) {
		auto firstIndex = shuffles.begin() + i*numFolders*numSims;
		auto lastIndex = firstIndex + numFolders*numSims;

		std::iota(firstIndex,lastIndex,0);
		std::shuffle(firstIndex,lastIndex,g);
	}
}
