#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "defines.h"
#include "assembler.h"


int main(int argc,char *argv[]){
	FILE *fp; /*FILE pointer*/
	int ind;
	/*char nameOfFiles[MAX_FILE_NAME];*/
	char line[MAXLINESIZE];
	char *res;
	if(argc < 2){/*check the number values in input is at list 2 - program name and at list one file name*/
		printf("There is no file to read from");
		exit(0);
	}
	for(ind = 1; ind < argc; ind ++){
		char nameOfFiles[MAX_FILE_NAME] = ""; /*declare and initialize an array for the file name*/
		strncpy(nameOfFiles, argv[ind], (strlen(argv[ind]))); /*copy the name of the running file*/
		strcat(nameOfFiles, ".as");/*add to the name of the running file .as in end*/
		fp = fopen(nameOfFiles , "r");
		if(fp == NULL){
			printf("The file %s is not exist or could not be opened \n", nameOfFiles);
			continue;
		}
		counterSetup();/*setup all the counters*/
		res = fgets(line , MAXLINESIZE , fp);   /*get line*/
		while(res != NULL){
			firstPass(line);/*first pass algoritem*/
			res = fgets(line , MAXLINESIZE , fp);	/*get line*/		
		}
		updateAddOfSymbols();
		
		if(isError() == YES){/* if we found an error no need to do the secound pass close the file and go to the next one */
			fclose(fp);
			continue;
		}
			
		rewind(fp);
		countZero();/*empty the counters for pass two*/
		res = fgets(line , MAXLINESIZE , fp);
		while(res != NULL){
			secondPass(line);/*second pass algoritem*/
			res = fgets(line , MAXLINESIZE, fp);
		}
		fclose(fp);
		if(isError() == YES){/* if we found error we dont need to make the files go to next file */
			continue;
		}
		makeFiles(argv[ind]);/* make the output files */
		clearGlobalStructs();/* clean from data the globel structs as prepare to read the next file */

		
	}
	return 0;
}



