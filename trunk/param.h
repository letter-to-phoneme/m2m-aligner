/********************************************************************
*
* file: param.h
*
* Copyright (c) 2007, Sittichai Jiampojamarn
* All rights reserved.
* 
* See the file COPYING in the top directory of this distribution
* for more information.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
* OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

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


