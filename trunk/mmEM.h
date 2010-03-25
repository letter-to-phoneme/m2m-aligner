#pragma once

#include "param.h"
#include "util.h"
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <climits>
#include <set>

// require STLport //
#ifdef USESTLPORT
#include <hash_map>
#endif

using namespace std;

typedef vector<long double> vector_Double;
typedef vector<vector_Double> vector_2Double;

typedef vector<int> vector_int;
typedef vector<vector_int> vector_2int;

typedef vector<string> vector_str;
typedef vector<vector_str> vector_2str;

typedef struct QTABLE
{
	long double score;
	int backX;
	int backY;
	int backR;
} qtable;

typedef vector<qtable> vector_qtable;
typedef vector<vector_qtable> vector_2qtable;
typedef vector<vector_2qtable> vector_3qtable;

 struct eqstr
{
  bool operator()(const string s1, const string s2) const
  {
          return s1.compare(s2) == 0;
  }
};

 inline bool DqSortedFn (qtable i, qtable j)
{
	return (i.score > j.score);
}

 typedef struct INITTABLE
 {
	 string xstring;
	 string ystring;
	 long double prob;
 } initTable;

 inline bool initTableSortedFn(initTable i, initTable j)
 {
	 return (i.prob > j.prob);
 }

 typedef vector<initTable> vector_initTable;

 // The program is a lot faster when using STLPORT //

// use standard STL//
#ifndef USESTLPORT
typedef map<string, long double> hash_StrDouble;
typedef map<string, hash_StrDouble> hash_2StrDouble;
#endif

// use STLport //
#ifdef USESTLPORT
typedef hash_map<string, long double, hash<string>, eqstr> hash_StrDouble;
typedef hash_map<string, hash_StrDouble, hash<string>, eqstr> hash_2StrDouble;
#endif


#define LOWLOGPROB -1e12

class mmEM
{
	hash_2StrDouble probs;
	hash_2StrDouble counts;
	set<string> limitSet;
public:
	mmEM(void);
	~mmEM(void);

	void training(param myParam);
	void initialization(param myParam, vector_2str stringX, vector_2str stringY);
	long double maximization(param myParam);
	bool expectation(param myParam, vector_str x, vector_str y);
	vector_2Double forwardEval(param myParam, vector_str x, vector_str y);
	vector_2Double backwardEval(param myParam, vector_str x, vector_str y);

	void createAlignments(param myParam);
	long double viterbi_align(param myParam, vector_str x, vector_str y, vector<string> *alignX, vector<string> *alignY);
	vector<long double> nViterbi_align(param myParam, vector_str x, vector_str y, vector_2str &alignX, vector_2str &alignY);

	void printAlphaBeta(vector_2Double alpha);
	void writeAlingerToFile(param myParam);
	void readAlignerFromFile(param myParam);

	void readFileXY(param myParam, string filename, vector_2str *wordX, vector_2str *wordY);

	void readInitFile(param myParam);
};


