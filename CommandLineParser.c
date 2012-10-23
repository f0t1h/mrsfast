/*
 * Copyright (c) <2008 - 2020>, University of Washington, Simon Fraser University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *   
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or other
 *   materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without specific
 *   prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Author: 
 *        Faraz Hach (fhach AT cs DOT sfu DOT ca)
 *        Iman Sarrafi (isarrafi AT cs DOT sfu DOT ca)
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "Common.h"
#include "CommandLineParser.h"

int						uniqueMode=1;
int						indexingMode;
int						searchingMode;
int						pairedEndMode;
int						pairedEndDiscordantMode;
int						pairedEndProfilingMode = 0;
int						bestMappingMode = 0;
int						snipMode = 0;
int						seqCompressed;
int						outCompressed;
int						cropSize = 0;
int						progressRep = 0;
int						minPairEndedDistance=-1;
int						maxPairEndedDistance=-1;
int						minPairEndedDiscordantDistance=-1;
int						maxPairEndedDiscordantDistance=-1;
int						errThreshold = -1;
char					*seqFile1;
char					*seqFile2;
char					*mappingOutput = "output";
char					*mappingOutputPath = "";
char					*unmappedOutput = "";
char					fileName[3][FILE_NAME_LENGTH];
unsigned char			maxHits=0;
unsigned char			WINDOW_SIZE = 12;
unsigned int			CONTIG_SIZE;
unsigned int			CONTIG_MAX_SIZE;
unsigned int			THREAD_COUNT = 1;
double					MAX_MEMORY = 4;// GB
int						THREAD_ID[255];
extern char 			_binary_HELP_start;
extern char				_binary_HELP_end;
int						defaultOutputName = 1;


void printHelp()
{
	char *c;
	for (c = &_binary_HELP_start; c != &_binary_HELP_end; c++)
		putchar(*c);
	exit(0);
}

int parseCommandLine (int argc, char *argv[])
{

	int o;
	int index;
	char *fastaFile = NULL;
	char *fastaOutputFile = NULL;
	char *indexFile = NULL;
	char *snipFile = NULL;
	static struct option longOptions[] = 
	{

		{"pe",				no_argument,		&pairedEndMode,		1},
		{"discordant-vh",	no_argument,		&pairedEndDiscordantMode,	1},
//		{"profile",			no_argument, 		&pairedEndProfilingMode,	1},
		{"seqcomp",			no_argument,		&seqCompressed,		1},
		{"outcomp",			no_argument,		&outCompressed,		1},
		{"progress",		no_argument,		&progressRep,		1},
		{"best",			no_argument,		&bestMappingMode,	1},
		{"index",			required_argument,	0, 					'i'},
		{"search",			required_argument,	0,					's'},
		{"help",			no_argument,		0,					'h'},
		{"version",			no_argument,		0,					'v'},
		{"seq",				required_argument,	0,					'x'},
		{"seq1",			required_argument,	0,					'x'},
		{"seq2",			required_argument,	0,					'y'},
		{"ws",				required_argument,  0,					'w'},
		{"min",				required_argument,  0,					'l'},
		{"max",				required_argument,  0,					'm'},
		{"crop",			required_argument,  0,					'c'},
		{"threads",			required_argument,  0,					't'},
		{"mem",				required_argument,  0,					'z'},
		{"snp",				required_argument,  0,					'p'},
		{0,0,0,0}

	};



	while ( (o = getopt_long ( argc, argv, "f:i:u:o:s:e:n:bhv", longOptions, &index))!= -1 )
	{
		switch (o)
		{
			case 'i':
				indexingMode = 1;
				fastaFile = optarg;
				break;
			case 's':
				searchingMode = 1;
				fastaFile = optarg;
				break;
			case 'c': 
				cropSize = atoi(optarg);
				break;
			case 'w':
				WINDOW_SIZE = atoi(optarg);
				break;
			case 'x':
				seqFile1 = optarg;
				break;
			case 'y':
				seqFile2 = optarg;
				break;
			case 'u':
				unmappedOutput = optarg;
				break;
			case 'o':
				mappingOutput = getMem(FILE_NAME_LENGTH);
				mappingOutputPath = getMem(FILE_NAME_LENGTH);
				stripPath (optarg, &mappingOutputPath, &mappingOutput);
				defaultOutputName = 0;
				break;
			case 'n':
				maxHits = atoi(optarg);
				break;
			case 'e':
				errThreshold = atoi(optarg);
				break;
			case 'l':
				minPairEndedDistance = atoi(optarg);
				break;
			case 'm':
				maxPairEndedDistance = atoi(optarg);
				break;					
			case 'h':
				printHelp();
				return 0;
				break;
			case 'v':
				fprintf(stdout, "%s.%s\n", versionNumber, versionNumberF);
				return 0;
				break;
			case 't':
				THREAD_COUNT = atoi(optarg);
				if (THREAD_COUNT == 0 || THREAD_COUNT > sysconf( _SC_NPROCESSORS_ONLN ))
					THREAD_COUNT = sysconf( _SC_NPROCESSORS_ONLN );
				break;
			case 'z':
				MAX_MEMORY = atoi(optarg);
				break;
			case 'p':
				snipMode = 1;
				snipFile = optarg;
				break;
		}

	}

#ifndef MRSFAST_SSE4
	if (searchingMode)
		fprintf(stdout, "==> This version is compiled without SSE4 instructions set. To obtain better performance, please upgrade your GCC version to >4.4 <==\n");
#endif

	if (indexingMode + searchingMode != 1)
	{
		fprintf(stdout, "ERROR: Indexing / Searching mode should be selected\n");
		return 0;
	}

	if (WINDOW_SIZE > 14 || WINDOW_SIZE < 8)
	{
		fprintf(stdout, "ERROR: Window size should be in [8..14]\n");
		return 0;
	}

	if (MAX_MEMORY < 2)
		fprintf(stdout, "ERROR: At least 2 GB of memory is required for running mrsFAST\n");


	if ( indexingMode )
	{
		CONTIG_SIZE		= 80000000;
		CONTIG_MAX_SIZE	= 120000000;

		if (fastaFile == NULL)
		{
			fprintf(stdout, "ERROR: Reference(s) should be indicated for indexing\n");
			return 0;
		}
	}


	if ( searchingMode )
	{
		CONTIG_SIZE		= 300000000;
		CONTIG_MAX_SIZE	= 300000000;

		if (pairedEndDiscordantMode)
		{
			pairedEndDiscordantMode = pairedEndMode = 1;
		}
		
		if (fastaFile == NULL)
		{
			fprintf(stdout, "ERROR: Index File(s) should be indiciated for searching\n");
			return 0;
		}

		if (seqFile1 == NULL && seqFile2 == NULL)
		{
			fprintf(stdout, "ERROR: Please indicate a sequence file for searching.\n");
			return 0;
		}
		
		if (!pairedEndMode && seqFile2 != NULL)
		{
			fprintf(stdout, "ERROR: Second File can be indicated in pairedend mode\n");
			return 0;
		}

		if (pairedEndMode)
		{
			if (minPairEndedDistance < 0 && maxPairEndedDistance < 0)
			{
				pairedEndProfilingMode = 1;
			}
			else if ( minPairEndedDistance <0 || maxPairEndedDistance < 0 || minPairEndedDistance > maxPairEndedDistance )
			{
				fprintf(stdout, "ERROR: Please enter a valid range for pairedend sequences.\n");
				return 0;
			}

			if (seqFile1 == NULL)
			{
				fprintf(stdout, "ERROR: Please indicate the first file for pairedend search.\n");
				return 0;
			}
		}
	}

	int i = 0;

	sprintf(fileName[0], "%s", fastaFile);
	sprintf(fileName[1], "%s.index", fileName[0]);
	if (snipMode)
		sprintf(fileName[2], "%s", snipFile);

/*	if (pairedEndProfilingMode)
	{
		minPairEndedDistance = 0;
		maxPairEndedDistance = 300000000;
	}	
*/
	if (pairedEndDiscordantMode)
	{
		minPairEndedDiscordantDistance = minPairEndedDistance;
		maxPairEndedDiscordantDistance = maxPairEndedDistance;

		minPairEndedDistance = 0;
		maxPairEndedDistance = 300000000;		// precise value is set in initLoadingHashTable()
	}
	
	if (!indexingMode)
	{
		fprintf(stdout, "# Threads: %d\n", THREAD_COUNT);
		for (i = 0; i < 255; i++)
			THREAD_ID[i] = i;
	}

	char fname1[FILE_NAME_LENGTH];
	char fname2[FILE_NAME_LENGTH];
	char fname3[FILE_NAME_LENGTH];
	char fname4[FILE_NAME_LENGTH];
	char fname5[FILE_NAME_LENGTH];

	// Why is this one here?
	if (pairedEndMode)
	{

		sprintf(fname1, "%s__%s__1", mappingOutputPath, mappingOutput);
		sprintf(fname2, "%s__%s__2", mappingOutputPath, mappingOutput);
		sprintf(fname3, "%s__%s__disc", mappingOutputPath, mappingOutput);
		sprintf(fname4, "%s__%s__oea1", mappingOutputPath, mappingOutput);
		sprintf(fname5, "%s__%s__oea2", mappingOutputPath, mappingOutput);
		unlink(fname1);
		unlink(fname2);
		unlink(fname3);
		unlink(fname4);
		unlink(fname5);
	}

	return 1;
}
/**********************************************/
void finalizeCommandParser()
{
	if ( !defaultOutputName )
	{
		freeMem(mappingOutput, FILE_NAME_LENGTH);
		freeMem(mappingOutputPath, FILE_NAME_LENGTH);
	}
}
