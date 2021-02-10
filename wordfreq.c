#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "sort.h"
#include "ADTs/stringADT.h"
#include "ADTs/arraylist.h"
#include "ADTs/hashcskmap.h"

#define UNUSED __attribute__((unused))

int keycmp(void *x1, void *x2){
	MEntry *m1 = (MEntry *)x1;
	MEntry *m2 = (MEntry *)x2;
	return strcmp((char *)m1->key, (char *)m2->key);
}

int invertkeycmp(void *x1, void *x2){
	MEntry *m1 = (MEntry *)x1;
	MEntry *m2 = (MEntry *)x2;
	return strcmp((char *)m2->key, (char *)m1->key);
}

int invertvalcmp(void *x1, void *x2){
	MEntry *m1 = (MEntry *)x1;
	MEntry *m2 = (MEntry *)x2;
	return ((long)m2->value < (long)m1->value);
}

int valcmp(void *x1, void *x2){
	MEntry *m1 = (MEntry *)x1;
	MEntry *m2 = (MEntry *)x2;
	return ((long)m1->value < (long)m2->value);
}

//want to use an arrayList to put words in 
//then loop through that arrayList and print words at the end of the file


//use map for word frequencies
	//keys are c strings (char *)
	//values are the frequencies (void * pointer)

void wordFreq(FILE *file, bool doA, bool doF, bool doI, bool doL, bool doP, bool doPrint, const CSKMap *map){

	char buf[BUFSIZ];
	long size;
	
	if(map == NULL){
		fprintf(stderr, "Cannot create hash map\n");
		return;
	}
	
	while(fgets(buf, BUFSIZ, file) != NULL){

		const String *str = String_create(buf);
		if(str == NULL){
			fprintf(stderr, "Cannot create string\n");
			return;
		}
		
		if(doL){
			//convert to all lower-case
			str->lower(str);
		}

		if(doP){
			//punctuation chars also separate words
			str->translate(str,"[:punct:]",' ');
		}

		//after flag processing create arrayList
		//split strings at white space
		const ArrayList *al = str->split(str,"");
		if(al == NULL){
			str->destroy(str);
			continue;
		}

		//iterate through arrayList and add to map		
		size = al->size(al);
		long i;
		
		for(i = 0L; i<size; i++){
			char *string;
			long count;
			(void) al->get(al,i,(void**)&string);
			
			if(map->get(map, string, (void **)&count))
				count++;
			else
				count = 1;
			map->put(map, string, (void *)count);		}
		
	//destroy containers inside outer while loop	
	str->destroy(str);
	al->destroy(al);
	}
	//end of while loop
	
	MEntry **mapArray;
	mapArray = map->entryArray(map, &size);
	if(mapArray == NULL){
		fprintf(stderr, "Cannot create entryArray\n");
		return;
	}

	if(doA && doF){
		fprintf(stderr, "Illegal flag invocation\n");
		return;
	}
	
	if(doI){ //inverse flag processing
		if(doA && doF){
			fprintf(stderr, "Cannot invoke -a and -f together\n");
			return;
		}
		if(doA){
		//print in inverse order
		sort((void **)mapArray, size, invertkeycmp);
		}
		if(doF){
		sort((void **)mapArray, size, invertvalcmp);
		}
	}
	else{
		if(doA){ //sort alphabetically
			sort((void **)mapArray,size,keycmp);
		}
		if(doF){
			sort((void **)mapArray,size,valcmp);
		}
	}

	if(doPrint){
	
	//run through map and print 
	long i;
	for(i = 0; i< size; i++){
		printf("%8ld %s\n",(long)mapArray[i]->value, (char *)mapArray[i]->key);
	}
	}

	//map->destroy(map); //may need to eliminate this
	//destroy MEntry * pointer when finished with it
	free((void *)mapArray);
	 //destroy mapArray			
}

int main(UNUSED int argc, UNUSED char *argv[]){
	int opt;
	bool doA, doF, doI, doL, doP, doPrint;
	FILE *file;
	int exitStatus = EXIT_FAILURE;
	const CSKMap *m = HashCSKMap(0L, 0.0, NULL);
	if(m == NULL){
		fprintf(stderr, "Cannot create hash map\n");
		return EXIT_FAILURE;
	}
	

	doA = doF = doI = doL = doP = doPrint = false;

	opterr = 0;
	while((opt = getopt(argc,argv,"afilp")) != -1){
		switch(opt){
			case 'a': doA = true;break;
			case 'f': doF = true;break;
			case 'i': doI = true;break;
			case 'l': doL = true;break;
			case 'p': doP = true;break;
			default: fprintf(stderr,"%s: illegal option, '-%c'\n", argv[0], optopt);
		}
	}

	if(optind == argc){
		//0 files given, no non-option arguments provided
		//use standard input
		doPrint = true;
		wordFreq(stdin, doA, doF, doI, doL, doP, doPrint, m);
		exitStatus = EXIT_SUCCESS;
		goto cleanup;
	}

	if(argc == (optind+1)){
		//just one file provided
		doPrint = true;
		file = fopen(argv[optind],"r");
		if(file == NULL){
			printf("open(%s) error\n",argv[optind]);
			exitStatus = EXIT_FAILURE;
			goto cleanup;

	}
		wordFreq(file, doA, doF, doI, doL, doP, doPrint, m);
		fclose(file); //close file
		exitStatus = EXIT_SUCCESS;
		goto cleanup;
	}

	if(argc > (optind+1)){
		int numFiles = argc - optind;
		int filesLeft = 0;
		//for multiple files
		while(argv[optind] != NULL){
			filesLeft++;
			if(filesLeft == numFiles){
				doPrint = true;
			}
			file = fopen(argv[optind],"r");
			if(file == NULL){
				printf("open(%s) error\n", argv[optind]);
				return EXIT_FAILURE;
			}

		wordFreq(file, doA, doF, doI, doL, doP, doPrint, m);
		optind++;
		fclose(file);//close each file	
		}

	exitStatus = EXIT_SUCCESS;
	goto cleanup;
	}


	cleanup:
	//destroy map instance
	//destroys each entry in map
	m->destroy(m);
	return exitStatus;

}
