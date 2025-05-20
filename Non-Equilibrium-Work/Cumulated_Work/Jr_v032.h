#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdlib> // For system()


#define outputFreq 10000

using std::vector;
using std::string;
using std::ofstream;

class JarCalculator
{

public:
	JarCalculator(int argc, char** argv);
	template<typename... Args>
	JarCalculator(const string& logName_, const int& numFolders_, const int& numSims_, const int& numData_, Args&&... args);
	JarCalculator(const JarCalculator& input) = delete;
	JarCalculator(const JarCalculator&& input) = delete;
	JarCalculator& operator=(const JarCalculator& rhs) = delete;
	JarCalculator& operator=(const JarCalculator&& rhs) = delete;
	~JarCalculator()
	{
		print_message("All done!");
	}
	
	void initialize();
	void calculate();
	void write();



private:
	int numFolders, numSims, numData, numDirs;

	vector<string> folders;

	vector<int> step;
	vector<double> z1, z2, z3;
	vector<double> f, w;
	
	vector<long double> gExp;
	vector<double> gSec, gBar, wAvg, z1avg;
	
	vector<int> exceptions;

	string outputFile, exceptionsFileName;
	string logName;

	ofstream logFile;
	
	void print_message(const string& message);
	void readSims();
	void writeData(const vector<double>& data, const string& fileName, const string& dataName);
	void average(const vector<double>& input, vector<double>& output);
	void calcWork();
	void calcJar();
	void calcBar();
	void shiftJar();
	void writeOutput();
	void writeExceptions();

};
