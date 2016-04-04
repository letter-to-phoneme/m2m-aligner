# Many-to-Many alignment model (m2m-aligner) #

m2m-aligner is implemented by [Sittichai Jiampojamarn](http://www.cs.ualberta.ca/~sj) during the PhD's years at [Department of Computing Science](http://www.cs.ualberta.ca/), [University of Alberta](http://www.ualberta.ca).
This algorithm has been applied in letter-to-phoneme conversion, name transliteration and other tasks;
for example, please see the below list of known publications that utilized the m2m-aligner. Please also check out [DirecTL+](http://code.google.com/p/directl-p/) for the generation model.
In general, this algorithm creates lexicon alignments without requiring annotated data nor linguistic knowledge.
Its principle algorithm is based on the Ristad and Yianilos (1998) stochastic transducer described in:

```
@Article{RYsed98,
  author  =  {Eric Sven Ristad and Peter N. Yianilos},
  title   =  {Learning String Edit Distance},
  journal =  {IEEE Transactions on Pattern Recognition and Machine Intelligence},
  year    =  1998,
  volume  =  20,
  number  =  5,
  pages   =  {522--532},
  month   =  {May}
}
```

Tarek Sherif originally proposed this algorithm as a part of his Master thesis graduated in 2007 from University of Alberta.
I later reimplemented this algorithm to the first version of m2m-aligner based on the paper we published together at
NAACL 2007. Since then, many refinements, improvements, and features have been included for later tasks.

You are welcome to use the code for research, commercial, and other purposes; however, please acknowledge its use with a citation to:

```
@InProceedings{jiampojamarn2007:,
  author    = {Jiampojamarn, Sittichai  and  Kondrak, Grzegorz  and  Sherif, Tarek},
  title     = {Applying Many-to-Many Alignments and Hidden Markov Models to Letter-to-Phoneme Conversion},
  booktitle = {Human Language Technologies 2007: The Conference of the North American Chapter of the Association for Computational Linguistics;
Proceedings of the Main Conference},
  month     = {April},
  year      = {2007},
  address   = {Rochester, New York},
  publisher = {Association for Computational Linguistics},
  pages     = {372--379},
  url       = {http://www.aclweb.org/anthology/N/N07/N07-1047}
}
```

## VERSIONS: ##
```
 1.0 : The first version of m2m-aligner released to public. All previous versions were for in-house users available upon request in the past.
```
## INSTALL: ##

m2m-aligner has been tested on Linux systems with gcc version 4.1.2. It should be compatible with other versions, and c++ compilers.
By default, the makefile is "makefile.default". It should be compiled by the "make" command:

$ make

For a faster m2m-aligner, you can change "makefile" to "makefile.stlport" which uses the stlport library instead of the default one.
You can obtain the stlport library from http://www.stlport.org/
Then, specify the stlport path in the makefile file.
The major difference is in the use of `<map>` and `<hash_map>` data structures.
You may also use `<hash_map>` implemented as in gcc and others. The interface should be the same but I haven't tested on them yet.


## USAGE: ##

```
   ./m2m-aligner  [--limit] [--errorInFile] [--initProb <long double>]
                  [--init <string>] [--nBest <int>] [--inFormat <l2p|news>]
                  [--sepInChar <string>] [--sepChar <string>] [--nullChar
                  <string>] [--pProcess <string>] [--pScore] [--cutoff
                  <double>] [--maxFn <conXY|conYX|joint>] [--eqMap]
                  [--delY] [--delX] [--maxY <int>] [--maxX <int>]
                  [--alignerIn <string>] [--alignerOut <string>] [-o
                  <string>] -i <string> [--] [--version] [-h]


Where:

   --limit
     Limit the alignment pair to used only from the initFile only (default false)

   --errorInFile
     Keep unaligned item in the output file (default false)

   --initProb <long double>
     Cut-off sum prior probability (default 0.5)

   --init <string>
     Initial mapping (model) filename (default null)

   --nBest <int>
     Generate n-best alignments (default n=1)

   --inFormat <l2p|news>
     Input file format [l2p, news] (default news)

   --sepInChar <string>
     Separated in-character used (default :)

   --sepChar <string>
     Separated character used (default |)

   --nullChar <string>
     Null character used (default _)

   --pProcess <string>
     Specify prefix output files

   --pScore
     Report score of each alignment (default false)

   --cutoff <double>
     Training threshold (default 0.01)

   --maxFn <conXY|conYX|joint>
     Maximization function [conXY, conYX, joint] (default conYX)

   --eqMap
     Allow mapping of |x| == |y| > 1 (default false)

   --delY
     Allow deletion of substring y (default false)

   --delX
     Allow deletion of substring x (default false)

   --maxY <int>
     Maximum length of substring y (default 2)

   --maxX <int>
     Maximum length of substring x (default 2)

   --alignerIn <string>
     Aligner model input filename

   --alignerOut <string>
     Aligner model output filename

   -o <string>,  --outputFile <string>
     Output filename

   -i <string>,  --inputFile <string>
     (required)  Input filename

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.
```

## File formats: ##
m2m-aligner takes two input formats so called "l2p" and "news".

> news format: each token separated by a space, a tab (\t) separates between source x and target y, one line per (x,y) pair.

> l2p format : each character byte is a toke, white space(s) separates between source x and target y, one line per (x,y) pair.

Please see an example file "toAlignEx". This example file is a small part of randomly taken around 1k words from the [CMU Pronouncing Dictionary](http://www.speech.cs.cmu.edu/cgi-bin/cmudict)

## Example run: ##
```
	$ ./m2m-aligner --delX --maxX 2 --maxY 2 -i toAlignEx
	
	--delX : allow deletion in the source side.
	--maxX <value> : the maximum size of sub-alignments in the source side.
	--maxY <value> : the maximum size of sub-alignments in the target side.
	-i <inputfile> : unaligned lexical file to train a model
```

## Example outputs: ##

> "toAlignEx.m-mAlign.2-2.1-best.conYX.align" is the alignment output file of "toAlignEx": each token's separated by ":", each sub-alignment's separated by "|", a tab (\t) separates between aligned x and y.

> "toAlignEx.m-mAlign.2-2.1-best.conYX.align.err" contains those examples from "toAlignEx" that can't be aligned with the current model.

> "toAlignEx.m-mAlign.2-2.1-best.conYX.align.model" is the aligner's model file.


## Acknowledgments: ##

This work was supported by the Alberta Ingenuity and Informatics
Circle of Research Excellence (iCORE) throughout
the Alberta Ingenuity Graduate Student Scholarship and
iCORE ICT Graduate Student Scholarship.

## The list of known publications that utilized the m2m-aligner: ##
_(Please contact me to include your usage of the m2m-aligner in this list)_

Yoh Okuno Introduction to Machine Transliteration, http://www.slideshare.net/nokuno/introduction-to-machine-transliteration
accessed March 2012

Sittichai Jiampojamarn, Colin Cherry and Grzegorz Kondrak Integrating Joint n-gram Features into a Discriminative Training Framework In Proceeding of
The 11th Annual Conference of the North American Chapter of the Association for Computational Linguistics (NAACL-HLT 2010), June 2010

Sittichai Jiampojamarn and Grzegorz Kondrak Online Discriminative Training for Grapheme-to-Phoneme Conversion In Proceeding of the 10th Annual
Conference of the International Speech Communication Association (INTERSPEECH), Brighton, U.K., September 2009, pp.1303-1306.

Sittichai Jiampojamarn, Aditya Bhargava, Qing Dou, Kenneth Dwyer and Grzegorz Kondrak "DIRECTL: a Language-Independent Approach to Transliteration".
In Proceedings of the 2009 Named Entities Workshop: Shared Task on Transliteration (NEWS 2009), Singapore, August 2009, pp.28-31.

Qing Dou, Shane Bergsma, Sittichai Jiampojamarn and Grzegorz Kondrak "A Ranking Approach to Stress Prediction for Letter-to-Phoneme Conversion".
Proceedings of the Joint Conference of the 47th Annual Meeting of the ACL and the 4th International Joint Conference on Natural Language Processing
of the AFNLP, Singapore, August 2009, pp.118-126.

Cook, P. and Stevenson, S. 2009. An unsupervised model for text message normalization. In Proceedings of the Workshop on Computational Approaches To
Linguistic Creativity (Boulder, Colorado, June 04 - 04, 2009). ACL Workshops. Association for Computational Linguistics, Morristown, NJ, 71-78.

Sittichai Jiampojamarn, Colin Cherry and Grzegorz Kondrak. "Joint Processing and Discriminative Training for Letter-to-Phoneme Conversion". In
Proceeding of the Annual Meeting of the Association for Computational Linguistics: Human Language Technologies (ACL-08: HLT), Columbus, OH, June
2008, pp.905-913.

Sittichai Jiampojamarn, Grzegorz Kondrak and Tarek Sherif. "Applying Many-to-Many Alignments and Hidden Markov Models to Letter-to-Phoneme
Conversion". Proceedings of the Annual Conference of the North American Chapter of the Association for Computational Linguistics (NAACL-HLT 2007),
Rochester, NY, April 2007, pp.372-379.


**Author: Sittichai Jiampojamarn**

**Date  : March 25th, 2010**

http://code.google.com/p/m2m-aligner