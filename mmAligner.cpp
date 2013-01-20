/********************************************************************
*
* file: mmAligner.cpp
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
* Author: Sittichai Jiampojamarn
* Email: sj@cs.ualberta.ca
*
* Many-to-many alignment : 
* History : 
*
* Sept 9, 2009 : implemented n-best generation model to output
* n-best alignments. It's based on a variation of n-best viterbi algorithm.
*   : added an initialization feature, so we can change 
*   : EM starting point and also inject lingustic knowledge.
*
* Mar 2, 2009 : re-formatting input/output for 
*  NEWS: Shared Task on Transliteration
*
* Dec 10, 2008 : re-implemented the algorithm based on 
*	Sittichai Jiampojamarn, Grzegorz Kondrak and Tarek Sherif. 
*  "Applying Many-to-Many Alignments and Hidden Markov Models 
*	to Letter-to-Phoneme Conversion". Proceedings of the Annual 
*	Conference of the North American Chapter of the Association 
*	for Computational Linguistics (NAACL-HLT 2007), Rochester, 
*	NY, April 2007, pp.372-379
*
* Credits : Tarek Sherif originally proposed this algorithm 
*  based on the Ristad and Yianilos (1997) stochastic transducer
*  as a part of his Master thesis graduated in 2007, U. of Alberta.
*
* Citation : If you use the code for research or commercial purposes,
*  please acknowledge its use with a citation:
*
*  @InProceedings{jiampojamarn2007:main,
*  author    = {Jiampojamarn, Sittichai  and  Kondrak, Grzegorz  and  Sherif, Tarek},
*  title     = {Applying Many-to-Many Alignments and Hidden Markov Models to Letter-to-Phoneme Conversion},
*  booktitle = {Human Language Technologies 2007: The Conference of the North American Chapter of the Association for Computational Linguistics; Proceedings of the Main Conference},
*  month     = {April},
*  year      = {2007},
*  address   = {Rochester, New York},
*  publisher = {Association for Computational Linguistics},
*  pages     = {372--379},
*  url       = {http://www.aclweb.org/anthology/N/N07/N07-1047}
*  }
*
**********************************************************************/

#include <iostream>
#include <list>
#include <ctime>
#include <cstdio>

#include <tclap/CmdLine.h>

#include "param.h"
#include "mmEM.h"
#include "util.h"

using namespace std;
using namespace TCLAP;

namespace TCLAP
{
        /**
         * long doubles have value-like semantics.
         */
        template<>
        struct ArgTraits<long double> {
		typedef ValueLike ValueCategory;
        };
}

inline void printTimeStamp(time_t startT)
{
	time_t stopT, diffT;

	time(&stopT);
	diffT = difftime(stopT, startT);
	cout << endl << "Time stamp: " << ctime(&stopT);
	cout << diffT << " seconds after started\n\n";
}

int main(int argc, char** argv)
{

	try{
		// program options //
		CmdLine cmd("Command description message ", ' ' , "1.0");

		ValueArg<string> inputFilename("i", "inputFile", "Input filename" , true, "", "string", cmd);
		ValueArg<string> outputFilename("o", "outputFile", "Output filename", false, "", "string", cmd);

		ValueArg<string> alignerOut("","alignerOut", "Aligner model output filename", false, "", "string", cmd);
		ValueArg<string> alignerIn("","alignerIn", "Aligner model input filename", false, "", "string", cmd);

		ValueArg<int> maxX("", "maxX", "Maximum length of substring x (default 2)", false, 2, "int", cmd);
		ValueArg<int> maxY("", "maxY", "Maximum length of substring y (default 2)", false, 2, "int", cmd);

		SwitchArg delX("", "delX", "Allow deletion of substring x (default false)", cmd, false);
		SwitchArg delY("", "delY", "Allow deletion of substring y (default false)", cmd, false);

		SwitchArg eqMap("", "eqMap", "Allow mapping of |x| == |y| > 1 (default false)", cmd, false);

		vector<string> allowFn;
		allowFn.push_back("conXY");
		allowFn.push_back("conYX");
		allowFn.push_back("joint");
		ValuesConstraint<string> allowFnValues(allowFn);
		ValueArg<string> maxFn("", "maxFn", "Maximization function [conXY, conYX, joint] (default conYX)", false, "conYX", &allowFnValues, cmd);

		ValueArg<double> cutOff("", "cutoff", "Training threshold (default 0.01)", false, 0.01, "double", cmd);

		SwitchArg printScore("", "pScore", "Report score of each alignment (default false)", cmd, false);

		ValueArg<string> prefixProcess("","pProcess","Specify prefix output files", false, "", "string", cmd);

		ValueArg<string> nullChar("","nullChar", "Null character used (default _)", false, "_", "string", cmd);
		ValueArg<string> sepChar("","sepChar","Separated character used (default |)", false, "|", "string", cmd);
		ValueArg<string> sepInChar("","sepInChar","Separated in-character used (default :)", false, ":","string", cmd);

		vector<string> allowFormat;
		allowFormat.push_back("l2p");
		allowFormat.push_back("news");
		ValuesConstraint<string> allowFormats(allowFormat);
		ValueArg<string> inFormat("","inFormat","Input file format [l2p, news] (default news)", false, "news", &allowFormats, cmd);

		ValueArg<int> nBest("","nBest","Generate n-best alignments (default n=1)", false, 1, "int", cmd);

		ValueArg<string> initFile("","init","Initial mapping (model) filename (default null)", false, "", "string", cmd);

		ValueArg<long double> initProbCut("","initProb","Cut-off sum prior probability (default 0.5)", false, 0.5, "long double", cmd);

		SwitchArg errorInFile("","errorInFile","Keep unaligned item in the output file (default false)", cmd, false);

		SwitchArg limitPair("","limit","Limit the alignment pair to used only from the initFile only (default false)", cmd, false);

		cmd.parse(argc, argv);

		// define program parameters //
		param myParam;
		myParam.inputFilename = inputFilename.getValue();
		myParam.outputFilename = outputFilename.getValue();

		myParam.alignerIn = alignerIn.getValue();
		myParam.alignerOut = alignerOut.getValue();

		myParam.maxX = maxX.getValue();
		myParam.maxY = maxY.getValue();
		myParam.delX = delX.getValue();
		myParam.delY = delY.getValue();
		myParam.eqMap = eqMap.getValue();

		myParam.maxFn = maxFn.getValue();
		myParam.cutOff = cutOff.getValue();

		myParam.printScore = printScore.getValue();
		myParam.prefixProcess = prefixProcess.getValue();

		myParam.nullChar = nullChar.getValue();
		myParam.sepChar = sepChar.getValue();
		myParam.sepInChar = sepInChar.getValue();

		myParam.inFormat = inFormat.getValue();
		myParam.nBest = nBest.getValue();
		
		myParam.initFile = initFile.getValue();

		myParam.initProbCut = initProbCut.getValue();

		myParam.errorInFile = errorInFile.getValue();

		myParam.limitPair = limitPair.getValue();

		// print program command to stdout//

		cout << "Command line: " << endl;

		for (int i = 0; i < argc; i++)
		{
			cout << argv[i] << endl;
		}
		cout << endl;

		cout << inputFilename.getDescription() << " : " << inputFilename.getValue() << endl;
		cout << outputFilename.getDescription() << " : " << outputFilename.getValue() << endl;
		cout << alignerIn.getDescription() << " : " << alignerIn.getValue() << endl;
		cout << alignerOut.getDescription() << " : " << alignerOut.getValue() << endl;
		cout << maxX.getDescription() << " : " << maxX.getValue() << endl;
		cout << maxY.getDescription() << " : " << maxY.getValue() << endl;
		cout << delX.getDescription() << " : " << delX.getValue() << endl;
		cout << delY.getDescription() << " : " << delY.getValue() << endl;
		cout << eqMap.getDescription() << " : " << eqMap.getValue() << endl;
		cout << maxFn.getDescription() << " : " << maxFn.getValue() << endl;
		cout << cutOff.getDescription() << " : " << cutOff.getValue() << endl;
		cout << printScore.getDescription() << " : " << printScore.getValue() << endl;
		cout << prefixProcess.getDescription() << " : " << prefixProcess.getValue() << endl;
		cout << nullChar.getDescription() << " : " << nullChar.getValue() << endl;
		cout << sepChar.getDescription() << " : " << sepChar.getValue() << endl;
		cout << sepInChar.getDescription() << " : " << sepInChar.getValue() << endl;
		cout << inFormat.getDescription() << " : " << inFormat.getValue() << endl;
		cout << nBest.getDescription() << " : " << nBest.getValue() << endl;
		cout << initFile.getDescription() << " : " << initFile.getValue() << endl;
		cout << initProbCut.getDescription() << " : " << initProbCut.getValue() << endl;
		cout << errorInFile.getDescription() << " : " << errorInFile.getValue() << endl;
		cout << limitPair.getDescription() << " : " << limitPair.getValue() << endl;

		if (myParam.prefixProcess == "")
		{
			myParam.prefixProcess = "m-mAlign." + stringify(myParam.maxX) + "-" + stringify(myParam.maxY);
			if (myParam.delX)
			{
				myParam.prefixProcess += ".delX";
			}
			if (myParam.delY)
			{
				myParam.prefixProcess += ".delY";
			}

			myParam.prefixProcess += "." + stringify(myParam.nBest) + "-best";
			myParam.prefixProcess += "." + myParam.maxFn;

			if (myParam.printScore)
			{
				myParam.prefixProcess += ".pScore";
			}

			if (myParam.initFile != "")
			{
				myParam.prefixProcess += ".withInit";
			}

			if (myParam.errorInFile)
			{
				myParam.prefixProcess += ".errorInFile";
			}

			if (myParam.limitPair)
			{
				myParam.prefixProcess += ".limitPair";
			}
		}

		cout << endl;
		time(&myParam.startT);
		cout << "Started at: " << ctime(&myParam.startT) << endl << endl;
		
		mmEM myAligner;

		if (myParam.alignerIn != "")
		{
			// read aligner model from file
			myAligner.readAlignerFromFile(myParam);
		}
		else
		{
			// training aligner //
			myAligner.training(myParam);
			
			// writing aligner model to file //
			if (myParam.alignerOut == "")
			{
				myParam.alignerOut = myParam.inputFilename + "." + myParam.prefixProcess + ".align.model";
			}
			myAligner.writeAlingerToFile(myParam);

			// read model from file //
			myParam.alignerIn = myParam.alignerOut;
			myAligner.readAlignerFromFile(myParam);
		}
		printTimeStamp(myParam.startT);

		// create alignments //
		if (myParam.outputFilename == "")
		{
			myParam.outputFilename = myParam.inputFilename + "." + myParam.prefixProcess + ".align";
		}
		myAligner.createAlignments(myParam);
		printTimeStamp(myParam.startT);		
	}
	catch (TCLAP::ArgException &e)
	{
		cerr << "error: " << e.error() <<  " for arg " << e.argId() << endl;
	}
	return 0;
}
