#pragma once

#include <string>
#include <ctime>

using namespace std;

typedef struct PARAM{

		string inputFilename;
		string outputFilename;

		string prefixProcess;

		string alignerOut;
		string alignerIn;

		int maxX;
		int maxY;

		bool delX;
		bool delY;
		bool eqMap;

		string maxFn;
		double cutOff;

		bool printScore;
		time_t startT;

		string nullChar;
		string sepChar;
		string sepInChar;

		string inFormat;
		int nBest;

		string initFile;
		long double initProbCut;

		bool errorInFile;

		bool limitPair;
} param;


