#include "mmEM.h"

mmEM::mmEM(void)
{
}

mmEM::~mmEM(void)
{
}

void mmEM::readInitFile(param myParam)
{
	// if there is an initial mapping file //
	// read the initFile, fill up the limit set //
	if (myParam.initFile != "")
	{
		cout << "Reading the initial file: " << myParam.initFile << endl;
		ifstream INITFILE;
		INITFILE.open(myParam.initFile.c_str());
		if (! INITFILE)
		{
			cerr << "error: unable to open file " << myParam.initFile << endl;
			exit(-1);
		}

		vector_initTable initCount;
		while (! INITFILE.eof() )
		{
			string line;
			vector<string> lineList;

			getline(INITFILE, line);

			initTable initTmp;

			if (line == "")
			{
				continue;
			}
			//string t0,t1,t3;
			//if (myParam.inFormat == "l2p")
			//{
			// should read it as the model format //

			lineList = splitBySpace(line);
			
			initTmp.xstring = lineList[0];
			initTmp.ystring = lineList[1];

			limitSet.insert(initTmp.xstring + "|" + initTmp.ystring);
			
			if (lineList.size() > 2)
			{
				initTmp.prob = (long double)atof(lineList[2].c_str());
			}
			else
			{
				initTmp.prob = 1;
			}
			initCount.push_back(initTmp);
		}
		INITFILE.close();
	}
}

void mmEM::initialization(param myParam, vector_2str stringX, vector_2str stringY)
{

	if (myParam.limitPair)
	{
		readInitFile(myParam);
	}

	if (stringX.size() != stringY.size())
	{
		cerr << "error: data are not in pairs of x and y " << endl;
		cerr << "# of x instances : " << stringX.size() << endl;
		cerr << "# of y instances : " << stringY.size() << endl;
		exit(-1);
	}

	// initialization with uniform distribution all possible alignments //
	
	long double totalCount = 0; // keep track how many observations

	// for each x and y pair //
	for (int i=0; i < stringX.size(); i++)
	{
		// over lengths of x and y
		for (int xl = 0; xl < stringX[i].size(); xl++)
		{
			for (int yl = 0; yl < stringY[i].size(); yl++)
			{
				if (myParam.delX)
				{
					for (int j=0; (j < myParam.maxX) && (xl-j >= 0); j++)
					{
						//string ssX = stringX[i].substr(xl-j,j+1);
						string ssX = join(stringX[i], xl-j , j+1);
						counts[ssX][myParam.nullChar] = 1;
					}
				}

				if (myParam.delY)
				{
					for (int k=0; (k < myParam.maxY) && (yl-k >=0); k++)
					{
						// string ssY = stringY[i].substr(yl-k,k+1);
						string ssY = join(stringY[i], yl-k, k+1);
						counts[myParam.nullChar][ssY] = 1;
					}
				}

				for (int j = 0; (j < myParam.maxX) && (xl-j >= 0); j++)
				{
					for (int k=0; (k < myParam.maxY) && (yl-k >=0); k++)
					{
						//string ssX = stringX[i].substr(xl-j,j+1);
						//string ssY = stringY[i].substr(yl-k,k+1);
						string ssX = join(stringX[i], xl-j, j+1);
						string ssY = join(stringY[i], yl-k, k+1);
						
						// if it defines a limit set //
						if (myParam.limitPair)
						{
							if (limitSet.find(ssX + myParam.sepChar + ssY) == limitSet.end())
							{
								continue;
							}
						}
						counts[ssX][ssY] = 1;
					}
				}
			}
		}
	}

	// if there is an initial mapping file //
	// while reading the initFile, fill up the limit set //
	if (myParam.initFile != "")
	{
		cout << "Reading the initial file: " << myParam.initFile << endl;
		ifstream INITFILE;
		INITFILE.open(myParam.initFile.c_str());
		if (! INITFILE)
		{
			cerr << "error: unable to open file " << myParam.initFile << endl;
			exit(-1);
		}

		vector_initTable initCount;
		while (! INITFILE.eof() )
		{
			string line;
			vector<string> lineList;

			getline(INITFILE, line);

			initTable initTmp;

			if (line == "")
			{
				continue;
			}
			//string t0,t1,t3;
			//if (myParam.inFormat == "l2p")
			//{
			// should read it as the model format //

			lineList = splitBySpace(line);
			
			initTmp.xstring = lineList[0];
			initTmp.ystring = lineList[1];
			
			if (lineList.size() > 2)
			{
				initTmp.prob = (long double)atof(lineList[2].c_str());
			}
			else
			{
				initTmp.prob = 1;
			}
			initCount.push_back(initTmp);
		}
		INITFILE.close();

		// sort initCount //
		cout << "Sorting the initial count table" << endl;
		sort(initCount.begin(), initCount.end(), initTableSortedFn);
		
		long double total_add_prob = 0;
		for (vector_initTable::iterator pos = initCount.begin(); pos != initCount.end(); pos++)
		{
			if ((total_add_prob < myParam.initProbCut) || (pos->prob ==	1))
			{
				counts[pos->xstring][pos->ystring] += ((counts[pos->xstring].size() * 10) + stringX.size()) * pos->prob ;
			}
			else
			{
				break;
			}

			if (pos->prob != 1)
			{
				total_add_prob += pos->prob;
			}
		}
	}

}

long double mmEM::maximization(param myParam)
{
	long double totalChange = 0;
	long double updateProb;

	hash_StrDouble countX, countY;
	long double totalCount=0;

	hash_2StrDouble newProbs;

	for(hash_2StrDouble::iterator pos = counts.begin(); pos != counts.end(); pos++)
	{
		for (hash_StrDouble::iterator pos2 = (pos->second).begin(); pos2 != (pos->second).end(); pos2++)
		{
			countX[pos->first] += pos2->second;
			countY[pos2->first] += pos2->second;
			totalCount += pos2->second;
		}
	}
	
	for(hash_2StrDouble::iterator pos = counts.begin(); pos != counts.end(); pos++)
	{
		for (hash_StrDouble::iterator pos2 = (pos->second).begin(); pos2 != (pos->second).end(); pos2++)
		{
			if (countY[pos2->first] == 0)
			{
				cerr << "Error : zero probability problem with y= " << pos2->first << endl;
				exit(-1);
			}

			if (countX[pos->first] == 0)
			{
				cerr << "Error : zero probability problem with x= " << pos->first << endl;
				exit(-1);
			}
			
			if (myParam.maxFn == "conXY") // p(x|y) //
			{
				updateProb = pos2->second / countY[pos2->first];
			}
			else if (myParam.maxFn == "conYX") // p(y|x) //
			{
				updateProb = pos2->second / countX[pos->first];
			}
			else if (myParam.maxFn == "joint") // p(x,y) //
			{
				updateProb = pos2->second / totalCount;
			}
			else
			{
				cerr << "Error : can't find maximization function used " << myParam.maxFn << endl;
				exit(-1);
			}
			totalChange += fabs(probs[pos->first][pos2->first] - updateProb);
			newProbs[pos->first][pos2->first] = updateProb;
		}
	}

	probs = newProbs;

	if (myParam.maxFn == "conXY")
	{
		totalChange = totalChange / countY.size();
	}
	else if (myParam.maxFn == "conYX")
	{
		totalChange = totalChange / countX.size();
	}
	
	// clean counts //
	counts.clear();

	// return total change in probability values //
	return totalChange;
}

vector_2Double mmEM::backwardEval(param myParam, vector_str x, vector_str y)
{
	vector_2Double beta;

	//resize vector//
	beta.resize(x.size()+1);
	for (int i = 0 ; i < beta.size(); i++)
	{
		beta[i].resize(y.size()+1);
	}

	beta[x.size()][y.size()] = 1.0;

	for (int xl = x.size() ; xl >= 0; xl--)
	{
		for (int yl = y.size(); yl >= 0; yl--)
		{
			if ((xl < x.size()) || (yl < y.size()))
			{
				beta[xl][yl] = 0;
			}

			if ((xl < x.size()) && (myParam.delX))
			{
				for (int i = 1; (i <= myParam.maxX) && (xl+i <= x.size()); i++)
				{
					//string ssX = x.substr(xl, i);
					string ssX = join(x, xl, i);
					beta[xl][yl] += probs[ssX][myParam.nullChar] * beta[xl+i][yl];
				}
			}

			if ((yl < y.size()) && (myParam.delY))
			{
				for (int j = 1; (j <= myParam.maxY) && (yl+j <= y.size()); j++)
				{
					//string ssY = y.substr(yl,j);
					string ssY = join(y, yl, j);
					beta[xl][yl] += probs[myParam.nullChar][ssY] * beta[xl][yl+j];
				}
			}

			if ((xl < x.size()) && (yl < y.size()))
			{
				for (int i = 1; (i <= myParam.maxX) && (xl+i <= x.size()); i++)
				{
					for (int j = 1; (j <= myParam.maxY) && (yl+j <= y.size()); j++)
					{
						//string ssX = x.substr(xl,i);
						//string ssY = y.substr(yl,j);

						string ssX = join(x, xl, i);
						string ssY = join(y, yl, j);

						// if it defines a limit set //
						if (myParam.limitPair)
						{
							if (limitSet.find(ssX + myParam.sepChar + ssY) == limitSet.end())
							{
								continue;
							}
						}
						beta[xl][yl] += probs[ssX][ssY] * beta[xl+i][yl+j];
					}
				}
			}
		}
	}

	return (beta);
}

vector_2Double mmEM::forwardEval(param myParam, vector_str x, vector_str y)
{
	vector_2Double alpha;

	// resize vector //
	alpha.resize(x.size()+1);
	for(int i = 0; i < alpha.size(); i++)
	{
		alpha[i].resize(y.size()+1);
	}

	alpha[0][0] = 1.0;
	for (int xl = 0; xl <= x.size(); xl++)
	{
		for (int yl = 0; yl <= y.size(); yl++)
		{
			if ((xl > 0) || (yl > 0))
			{
				alpha[xl][yl] = 0;
			}

			if ( (xl > 0) && (myParam.delX) )
			{
				for (int i = 1; (i <= myParam.maxX) && (xl-i >= 0); i++)
				{
					//string ssX = x.substr(xl-i, i);
					string ssX = join(x, xl-i,i);
					alpha[xl][yl] += probs[ssX][myParam.nullChar] * alpha[xl-i][yl];
				}
			}

			if ( (yl > 0) && (myParam.delY) )
			{
				for (int j = 1; (j <= myParam.maxY) && (yl-j >= 0); j++)
				{
					//string ssY = y.substr(yl-j, j);
					string ssY = join(y, yl-j, j);
					alpha[xl][yl] += probs[myParam.nullChar][ssY] * alpha[xl][yl-j];
				}
			}

			if ( (yl > 0) && (xl > 0) )
			{
				for (int i = 1; (i <= myParam.maxX) && (xl-i >= 0); i++)
				{
					for (int j = 1; (j <= myParam.maxY) && (yl-j >= 0); j++)
					{
						//string ssX = x.substr(xl-i,i);
						//string ssY = y.substr(yl-j,j);

						string ssX = join(x, xl-i, i);
						string ssY = join(y, yl-j, j);

						// if it defines a limit set //
						if (myParam.limitPair)
						{
							if (limitSet.find(ssX + myParam.sepChar + ssY) == limitSet.end())
							{
								continue;
							}
						}

						alpha[xl][yl] += probs[ssX][ssY] * alpha[xl-i][yl-j];
					}
				}
			}
		}
	}

	return (alpha);
}

void mmEM::printAlphaBeta(vector_2Double alpha)
{
	cout << endl;
	for (int i = 0; i < alpha.size() ; i++)
	{
		for (int j = 0; j < alpha[i].size(); j++)
		{
			cout << alpha[i][j] << " ";
		}
		cout << endl;
	}
}

bool mmEM::expectation(param myParam, vector_str x, vector_str y)
{
	vector_2Double alpha, beta;
	long double alpha_x_y;

	alpha = forwardEval(myParam, x, y);
	beta = backwardEval(myParam, x, y);

/*	
	cout << "x: " << x << endl;
	cout << "y: " << y << endl;
	cout << "alpha: " << endl;
	printAlphaBeta(alpha);
	cout << "beta: " << endl;
	printAlphaBeta(beta);
*/	

	// zero forward probability //
	if (alpha[x.size()][y.size()] == 0)
	{
		return false;
	}
	else
	{
		alpha_x_y = alpha[x.size()][y.size()];
	}

	for (int xl = 0 ; xl <= x.size(); xl++)
	{
		for (int yl = 0; yl <= y.size(); yl++)
		{
			if ( (xl > 0) && (myParam.delX) )
			{
				for (int i = 1; (i <= myParam.maxX) && (xl-i >= 0); i++)
				{
					//string ssX = x.substr(xl-i, i);
					string ssX = join(x, xl-i,i);
					long double updateCount;
					updateCount = (alpha[xl-i][yl] * probs[ssX][myParam.nullChar] * beta[xl][yl]) / alpha_x_y;

					if (updateCount != 0)
					{
						counts[ssX][myParam.nullChar] += updateCount;					
					}
				}
			}

			if ( (yl > 0) && (myParam.delY) )
			{
				for (int j = 1; (j <= myParam.maxY) && (yl-j >= 0); j++)
				{
					//string ssY = y.substr(yl-j, j);
					string ssY = join(y, yl-j, j);
					long double updateCount;
					updateCount = (alpha[xl][yl-j] * probs[myParam.nullChar][ssY] * beta[xl][yl]) / alpha_x_y;

					if (updateCount != 0)
					{
						counts[myParam.nullChar][ssY] += updateCount;
					}
				}
			}

			if ( (yl > 0) && (xl > 0) )
			{
				for (int i = 1; (i <= myParam.maxX) && (xl-i >= 0); i++)
				{
					for (int j = 1; (j <= myParam.maxY) && (yl-j >= 0); j++)
					{
						//string ssX = x.substr(xl-i,i);
						//string ssY = y.substr(yl-j,j);

						string ssX = join(x, xl-i,i);
						string ssY = join(y, yl-j,j);

						long double updateCount;
						updateCount = (alpha[xl-i][yl-j] * probs[ssX][ssY] * beta[xl][yl]) / alpha_x_y;

						if (updateCount != 0)
						{
							counts[ssX][ssY] += updateCount;
						}
					}
				}
			}
		}
	}
	return true;
}

void mmEM::readFileXY(param myParam, string filename, vector_2str *wordX, vector_2str *wordY)
{
	cout << "Reading file: " << filename << endl;
	ifstream INPUTFILE;
	INPUTFILE.open(filename.c_str());
	if (! INPUTFILE)
	{
		cerr << "error: unable to open file " << filename << endl;
		exit(-1);
	}
	while (! INPUTFILE.eof() )
	{
		string line;
		vector<string> lineList;
		
		// read each line and split column by space //
		getline(INPUTFILE, line);
		// ignore empty line
		if (line == "")
		{
			continue;
		}
		// split by tab to get source and target //
		if (myParam.inFormat == "l2p")
		{
			lineList = splitBySpace(line);
			vector<string> t0,t1;
			Tokenize(lineList[0], t0, "");
			Tokenize(lineList[1], t1, "");

			wordX->push_back(t0);
			wordY->push_back(t1);
		}
		else if (myParam.inFormat == "news")
		{
			Tokenize(line, lineList, "\t");
			wordX->push_back(splitBySpace(lineList[0]));
			wordY->push_back(splitBySpace(lineList[1]));
		}
		else
		{
			cerr << "ERROR: can't find input format type, plz. check --inFormat" << endl << endl;
		}

		if (lineList.size() != 2)
		{
			cerr << "Warning : missing either x or y word here, so skipped:" << endl << line << endl;
		}
	}
	// close file //
	INPUTFILE.close();
}

void mmEM::training(param myParam)
{
	vector_2str wordX;
	vector_2str wordY;

	bool stillTrain = true;
	
	// reading file //
	readFileXY(myParam, myParam.inputFilename, &wordX, &wordY);

	// initialization //
	cout << "Initialization ... ";
	initialization(myParam, wordX, wordY);

	// maximization //
	cout << "Maximization ... " << endl;
	maximization(myParam);
	cout << endl;

	int atIter = 0;
	// still training //
	while (stillTrain)
	{
		atIter++;
		cout << "Iteration " << atIter << endl;
		// for each x and y pair //
		cout << "Expectation ... " ;
		for (int i = 0; i < wordX.size(); i++)
		{
			// expectation //
			expectation(myParam, wordX[i], wordY[i]);	
		}

		cout << "Maximization ... ";
		long double totalChange = maximization(myParam);
		cout << "Total probability change = " << totalChange << endl;

		// stop by the probability change condition //
		if ((totalChange <= myParam.cutOff) && (myParam.cutOff < 1))
		{
			stillTrain = false;
		}

		// stop by the number of iteration condition //
		if ((myParam.cutOff >= 1) && (atIter >= myParam.cutOff))
		{
			stillTrain = false;
		}
	}
	cout << endl;
}

// write the model to file 
void mmEM::writeAlingerToFile(param myParam)
{
	cout << "Writing aligner model to file : " << myParam.alignerOut << endl;

	ofstream ALIGNEROUT;
	ALIGNEROUT.precision(15);
	ALIGNEROUT.open(myParam.alignerOut.c_str());
	if (! ALIGNEROUT)
	{
		cerr << "Error : can't write to file " << myParam.alignerOut << endl;
		exit(-1);
	}
	
	for (hash_2StrDouble::iterator pos = probs.begin(); pos != probs.end(); pos++)
	{
		for (hash_StrDouble::iterator pos2 = (pos->second).begin(); pos2 != (pos->second).end(); pos2++)
		{
			ALIGNEROUT << pos->first << "\t" << pos2->first << "\t";
			ALIGNEROUT << pos2->second << endl;
		}
	}

	ALIGNEROUT.close();
}

void mmEM::readAlignerFromFile(param myParam)
{
	cout << "Reading aligner model from file : " << myParam.alignerIn << endl;

	//clear model parameters 
	probs.clear();
	counts.clear();

	ifstream ALIGNERIN;
	ALIGNERIN.open(myParam.alignerIn.c_str());
	if (! ALIGNERIN)
	{
		cerr << "Error : can't read aligner model from file " << myParam.alignerIn << endl;
		exit(-1);
	}

	while (! ALIGNERIN.eof())
	{
		string line;
		vector<string> lineList;

		getline(ALIGNERIN, line);
		if (line == "")
		{
			continue;
		}

		lineList = splitBySpace(line);

		if (lineList.size() != 3)
		{
			cerr << "Error : aligner model is in the wrong format " << endl << line << endl;
			exit(-1);
		}

		// problem with long double when reading from string // 
		//probs[lineList[0]][lineList[1]] = convertTo<long double>(lineList[2]);
		probs[lineList[0]][lineList[1]] = (long double)atof(lineList[2].c_str());
	}
	ALIGNERIN.close();
}

vector<long double> mmEM::nViterbi_align(param myParam, vector_str x, vector_str y, vector_2str &alignX, vector_2str &alignY)
{
	vector_3qtable Q;
	vector<long double> nBestScore;

	qtable qtmp;

	Q.resize(x.size()+1);

	for (int i = 0; i < Q.size() ;i++)
	{
		Q[i].resize(y.size()+1);
	}

	qtmp.score = 0;
	qtmp.backX = -1;
	qtmp.backY = -1;
	qtmp.backR = -1;
	Q[0][0].push_back(qtmp);	

	for (int xl =0 ; xl <= x.size(); xl++)
	{
		for (int yl = 0; yl <= y.size(); yl++)
		{
			/*if ((xl > 0) || (yl > 0))
			{
				qtmp.score = LOWLOGPROB;
				qtmp.backX = 0;
				qtmp.backY = 0;
				qtmp.backR = 0;
				
				Q[xl][yl].push_back(qtmp);
			}*/

			if ( (xl > 0) && (myParam.delX) )
			{
				for (int i = 1; (i <= myParam.maxX) && (xl-i >=0); i++)
				{
					string ssX = join(x, xl-i,i);
	
					long double score = (long double) logl(probs[ssX][myParam.nullChar]) * i;
					qtmp.backX = i;
					qtmp.backY = 0;

					for (int rindex = 0; rindex < Q[xl-i][yl].size(); rindex++)
					{
						qtmp.score = score + Q[xl-i][yl][rindex].score;
						qtmp.backR = rindex;
						Q[xl][yl].push_back(qtmp);
					}					
				}
			}

			if ( (yl > 0) && (myParam.delY) )
			{
				for (int j = 1; (j <= myParam.maxY) && (yl-j >=0); j++)
				{
					string ssY = join(y, yl-j, j);

					long double score = (long double) logl(probs[myParam.nullChar][ssY]) * j;
					qtmp.backX = 0;
					qtmp.backY = j;

					for (int rindex  = 0; rindex < Q[xl][yl-j].size(); rindex++)
					{
						qtmp.score = score + Q[xl][yl-j][rindex].score;
						qtmp.backR = rindex;
						Q[xl][yl].push_back(qtmp);
					}
				}
			}

			if ( (xl > 0) && (yl > 0) )
			{
				for (int i = 1; (i <= myParam.maxX) && (xl-i >=0); i++)
				{
					for (int j = 1; (j <= myParam.maxY) && (yl-j >=0); j++)
					{
						if (! myParam.eqMap)
						{
							if ((i==j) && (i>1))
							{
								continue;
							}
						}
						
						string ssX = join(x, xl-i ,i);
						string ssY = join(y, yl-j, j);

						long double score = (long double) logl(probs[ssX][ssY]) * max(i,j);
						qtmp.backX = i;
						qtmp.backY = j;
						for (int rindex = 0; rindex < Q[xl-i][yl-j].size(); rindex++)
						{
							qtmp.score = score + Q[xl-i][yl-j][rindex].score;
							qtmp.backR = rindex;
							Q[xl][yl].push_back(qtmp);
						}
					}
				}
			}

			// reduce size of n-best //
			if (Q[xl][yl].size() > myParam.nBest)
			{
				vector_qtable qtmpSort(myParam.nBest);
				partial_sort_copy(Q[xl][yl].begin(), Q[xl][yl].end(), qtmpSort.begin(), qtmpSort.end(), DqSortedFn);
				Q[xl][yl] = qtmpSort;
			}
		}
	}

	// sorting
	sort(Q[x.size()][y.size()].begin(), Q[x.size()][y.size()].end(), DqSortedFn);

	//backTracking
	for (int k=0; ( k < myParam.nBest ) && (Q[x.size()][y.size()].size() > 0) ; k++)
	{
		long double score = Q[x.size()][y.size()][0].score;

		// If the score indicates a proper alignment //
		if (score > LOWLOGPROB)
		{
			int xxl = x.size();
			int yyl = y.size();
			int moveX;
			int moveY;
			int backR = 0;
			vector_str alignXtmp, alignYtmp;

			while ((xxl > 0) || (yyl > 0))
			{
				moveX = Q[xxl][yyl][backR].backX;
				moveY = Q[xxl][yyl][backR].backY;
				backR = Q[xxl][yyl][backR].backR;

				if (moveX > 0)
				{
					//alignX->push_back(x.substr(xxl-moveX,moveX));
					//alignX->push_back(join(x, xxl-moveX, moveX, myParam.sepInChar));
					alignXtmp.push_back(join(x,xxl-moveX, moveX, myParam.sepInChar));
				}
				else
				{
					//alignX->push_back(myParam.nullChar);
					alignXtmp.push_back(myParam.nullChar);
				}

				if (moveY > 0)
				{
					//alignY->push_back(y.substr(yyl-moveY,moveY));
					//alignY->push_back(join(y, yyl-moveY, moveY, myParam.sepInChar));
					alignYtmp.push_back(join(y, yyl-moveY, moveY, myParam.sepInChar));
				}
				else
				{
					//alignY->push_back(myParam.nullChar);
					alignYtmp.push_back(myParam.nullChar);
				}

				xxl -= moveX;
				yyl -= moveY;
				
			}

			//reverse(alignX->begin(), alignX->end());
			//reverse(alignY->begin(), alignY->end());
			reverse(alignXtmp.begin(), alignXtmp.end());
			reverse(alignYtmp.begin(), alignYtmp.end());

			alignX.push_back(alignXtmp);
			alignY.push_back(alignYtmp);
			nBestScore.push_back(score);
		}

		// delete top guy // 
		Q[x.size()][y.size()].erase(Q[x.size()][y.size()].begin());
	}

	return nBestScore;
}

long double mmEM::viterbi_align(param myParam, vector_str x, vector_str y, vector<string> *alignX, vector<string> *alignY)
{
	vector_2Double Q;
	vector_2int backX;
	vector_2int backY;

	long double score;

	// allocate vectors //
	Q.resize(x.size()+1);
	backX.resize(x.size()+1);
	backY.resize(x.size()+1);
	
	for (int i=0 ; i < Q.size() ; i++)
	{
		Q[i].resize(y.size()+1);
		backX[i].resize(y.size()+1);
		backY[i].resize(y.size()+1);
	}

	// Make sure we are in the log probability mode.
	// LOWLOGPROB is considered as a very low probability value 
	// used to indicate zero probability.

	Q[0][0] = 0;
	for (int xl = 0; xl <= x.size(); xl++)
	{
		for (int yl = 0; yl <= y.size(); yl++)
		{
			if ((xl > 0) || (yl > 0))
			{
				Q[xl][yl] = LOWLOGPROB;
				backX[xl][yl] = 0;
				backY[xl][yl] = 0;
			}

			if ( (xl > 0) && (myParam.delX) )
			{
				for (int i = 1; (i <= myParam.maxX) && (xl-i >=0); i++)
				{
					//string ssX = x.substr(xl-i,i);
					string ssX = join(x, xl-i,i);

					score = (long double) logl(probs[ssX][myParam.nullChar]) * i;
					if (score + Q[xl-i][yl] > Q[xl][yl])
					{
						Q[xl][yl] = score + Q[xl-i][yl];
						backX[xl][yl] = i;
						backY[xl][yl] = 0;
					}
				}
			}

			if ( (yl > 0) && (myParam.delY) )
			{
				for (int j = 1; (j <= myParam.maxY) && (yl-j >=0); j++)
				{
					//string ssY = y.substr(yl-j,j);
					string ssY = join(y, yl-j, j);

					score = (long double) logl(probs[myParam.nullChar][ssY]) * j;
					if (score + Q[xl][yl-j] > Q[xl][yl])
					{
						Q[xl][yl] = score + Q[xl][yl-j];
						backX[xl][yl] = 0;
						backY[xl][yl] = j;
					}
				}
			}

			if ( (xl > 0) && (yl > 0) )
			{
				for (int i = 1; (i <= myParam.maxX) && (xl-i >=0); i++)
				{
					for (int j = 1; (j <= myParam.maxY) && (yl-j >=0); j++)
					{
						if (! myParam.eqMap)
						{
							if ((i==j) && (i>1))
							{
								continue;
							}
						}
						//string ssX = x.substr(xl-i,i);
						//string ssY = y.substr(yl-j,j);

						string ssX = join(x, xl-i ,i);
						string ssY = join(y, yl-j, j);

						score = (long double) logl(probs[ssX][ssY]) * max(i,j);
						if ( score + Q[xl-i][yl-j] > Q[xl][yl])
						{
							Q[xl][yl] = score + Q[xl-i][yl-j];
							backX[xl][yl] = i;
							backY[xl][yl] = j;
						}
					}
				}
			}
		}
	}

	score = Q[x.size()][y.size()];

	if (score <= LOWLOGPROB)
	{
		return score;
	}

	int xxl = x.size();
	int yyl = y.size();
	int moveX;
	int moveY;

	while ((xxl > 0) || (yyl > 0))
	{
		moveX = backX[xxl][yyl];
		moveY = backY[xxl][yyl];

		if (moveX > 0)
		{
			//alignX->push_back(x.substr(xxl-moveX,moveX));
			alignX->push_back(join(x, xxl-moveX, moveX, myParam.sepInChar));
		}
		else
		{
			alignX->push_back(myParam.nullChar);
		}

		if (moveY > 0)
		{
			//alignY->push_back(y.substr(yyl-moveY,moveY));
			alignY->push_back(join(y, yyl-moveY, moveY, myParam.sepInChar));
		}
		else
		{
			alignY->push_back(myParam.nullChar);
		}

		xxl -= moveX;
		yyl -= moveY;
	}

	reverse(alignX->begin(), alignX->end());
	reverse(alignY->begin(), alignY->end());

	return score;
}

void mmEM::createAlignments(param myParam)
{
	vector_2str wordX;
	vector_2str wordY;

	double lessNbest = 0;

	//reading input file//
	readFileXY(myParam, myParam.inputFilename, &wordX, &wordY);

	cout << "There are " << wordX.size() << " pairs to be aligned" << endl;

	cout << "Write aligned data to : " << myParam.outputFilename << endl;
	cout << "Write un-aligned data to : " << myParam.outputFilename << ".err" << endl;

	ofstream ALIGNED, NOALIGNED;

	ALIGNED.open(myParam.outputFilename.c_str());
	NOALIGNED.open(string(myParam.outputFilename + ".err").c_str());

	
	double alignCount = 0;
	double noAlignCount = 0;

	for (unsigned int i = 0; i < wordX.size(); i++)
	{
		//vector<string> alignX, alignY;
		vector_2str nAlignX, nAlignY;
		vector<long double> nScore;

		nScore = nViterbi_align(myParam, wordX[i], wordY[i], nAlignX, nAlignY);
		//score = viterbi_align(myParam, wordX[i], wordY[i], &alignX, &alignY);

		if (nScore.size() > 0)
		{
			// found n-best alignments
			alignCount++;

			// count number of examples that have less than n alignment candidates //
			if (nScore.size() < myParam.nBest)
			{
				lessNbest++;
				cout << join(wordX[i],0, wordX[i].size(), "") << "\t" << join(wordY[i],0,wordY[i].size(), "") << "\t has " << nScore.size() << " alignments" << endl;
			}
			for (int nbest = 0; nbest < nScore.size(); nbest++)
			{
				for (unsigned int k = 0; k < nAlignX[nbest].size(); k++)
				{
					ALIGNED << nAlignX[nbest][k] << myParam.sepChar;
				}
				ALIGNED << "\t";
				
				for (unsigned int k=0; k < nAlignY[nbest].size(); k++)
				{
					ALIGNED << nAlignY[nbest][k] << myParam.sepChar;
				}

				if (myParam.printScore)
				{
					ALIGNED << "\t" << nbest+1;
					ALIGNED << "\t" <<  nScore[nbest];
				}
				ALIGNED << endl;
			}
		}
		else
		{
			// can't be aligned 
			noAlignCount++;
			if (myParam.errorInFile)
			{
				ALIGNED << "NO ALIGNMENT \t" << endl;
			}
			NOALIGNED << join(wordX[i],0,wordX[i].size()," ") << "\t" << join(wordY[i], 0, wordY[i].size(), " ") << endl;
		}
	}
	ALIGNED.close();
	NOALIGNED.close();

	cout << "Aligned " << alignCount << " pairs" << endl;
	if (noAlignCount > 0)
	{
		cout << "No aligned " << noAlignCount << " pairs" << endl;
	}

	cout << "There are " << lessNbest << " example pairs having less than " << myParam.nBest << " alignment candidates" << endl;
}

