#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_PAGES 50
#define NUM_ADDRESSES 4000

#define NUM_ALGORITHMS 3
#define FIFO 0
#define LRU 1
#define CLOCK 2

#define FRAME_MIN 2
#define FRAME_MAX 20

#define NUM_SIMULATIONS 1000

#define DEBUG 0

// struct page{
// 	int id;
// 	void *data;
// } typedef page_t;

struct frame{
	int page_id;
	double time_last_used; //LRU
	int used; //Clock
	//page_t *page;
} typedef frame_t;

void cleanFrameArray(frame_t* arr, int length){
	int i;
	for(i =0;i<length;i++){
		arr[i].page_id=-1; //-1 means empty frame
		arr[i].time_last_used = 0;
		arr[i].used = 0;
		//arr[i].page = NULL;
	}
}

frame_t* generateFrameArray(int length){
	frame_t* arr = calloc(sizeof(frame_t),length);
	cleanFrameArray(arr,length);
	return arr;
}

int* generateAddressArray(int min, int max, int length){
	int* arr = malloc(sizeof(int)*length);
	int i;
	for(i=0;i<length;i++){
		arr[i]=(rand() % max)+min; //generate rand min-max
	}
	return arr;
}

void printFrameArray(frame_t* frames, int frame_len, int ptr, int algorithm){
	int i;
	printf("|%5s|%4s|%4s|%15s|\n","Index", "Page", "Used", "Time Last Used");
	for(i=0;i<frame_len;i++){
		printf("|%5d|%4d|%4d|%10.9f|%s\n",i,
			frames[i].page_id,frames[i].used,
			frames[i].time_last_used,(ptr==i)?"<-":"");
	}
	printf("\n");
}

frame_t* searchFrameArrayForPage(frame_t* frames, int frame_len, int page_id){
	int i;
	for(i=0;i<frame_len;i++){
		if(frames[i].page_id == -1) break; //reached the end. No match
		if(frames[i].page_id == page_id) return &(frames[i]); //match
	}
	return NULL;
}

int simulate(int* addresses, int addr_len, frame_t *frames, int frame_len, int algorithm){
	int page_faults = 0;
	int i;
	//Variable initializations
	//FIFO
	int ptr = 0;//Starts by pointing to the first frame
	//LRU
	frame_t* found_frame;
	struct timespec temp_time;
	//CLOCK


	for(i=0;i<addr_len;i++){
		#if DEBUG
		printFrameArray(frames,frame_len,ptr,algorithm);
		printf("Next Page Number = %d\n",addresses[i]);
		#endif
		found_frame = searchFrameArrayForPage(frames, frame_len,addresses[i]);
		if(found_frame){
			//No fault. Page Exists in memory
			if(algorithm == LRU){
				clock_gettime(CLOCK_MONOTONIC, &temp_time);
				found_frame->time_last_used = temp_time.tv_sec + (temp_time.tv_nsec/10e9);
			}
			if(algorithm == CLOCK){
				found_frame->used = 1;
			}
			continue;
		}
		else{
			//Fault
			page_faults++;
			//Replacement Policy
			switch (algorithm) {
				case FIFO:{
					frames[ptr].page_id = addresses[i];
					ptr = (ptr + 1) % frame_len;
					break;
				}
				case LRU:{
					ptr = 0;
					int j;
					for(j=1;j<frame_len;j++){
						if(frames[j].time_last_used <= frames[ptr].time_last_used){
							ptr = j;
						}
					}
					frames[ptr].page_id = addresses[i];
					clock_gettime(CLOCK_MONOTONIC, &temp_time);
					frames[ptr].time_last_used = temp_time.tv_sec + (temp_time.tv_nsec/10e9);
					break;
				}
				case CLOCK:{
					while(frames[ptr].used != 0){
						frames[ptr].used = 0;
						ptr = (ptr + 1) % frame_len;
					}
					frames[ptr].page_id = addresses[i];
					frames[ptr].used = 1;
					ptr = (ptr + 1) % frame_len;
					break;
				}
			}
		}
	}
	return page_faults;
}

int main(int argc, char **argv){
	//Setup Output file
	FILE *fp;
	if(argc > 1){
		printf("Ouptut file selected: %s\n",argv[1]);
		fp = fopen(argv[1], "w+");
		if(!fp){
			printf("Error: Cannot open file.\n");
			return 0;
		}
	}
	else{
		printf("No Ouput file given as command line argument.\nWriting to Stdout.\n\n");
		fp = stdout;
	}

	//Init
	srand(time(0));

	//Header
	fprintf(fp,"%6s\t%5s\t%5s\t%5s\n","Frames","FIFO","LRU","CLOCK");
	int sim_num; //For each simulation
	for(sim_num=0; sim_num < NUM_SIMULATIONS; sim_num++){
		//Generate Addresses
		int* addresses = generateAddressArray(1, NUM_PAGES, NUM_ADDRESSES);

		int frame_len; //For Each Frame length
		for(frame_len = FRAME_MIN; frame_len <= FRAME_MAX; frame_len++){
			//Generate Frames
			frame_t* frames = generateFrameArray(frame_len);
			fprintf(fp,"%6d", frame_len);
			// For Each Algorithm
			int algo;
			for(algo = FIFO; algo < NUM_ALGORITHMS; algo++){
				int faults =  simulate(addresses, NUM_ADDRESSES, frames, frame_len, algo);
				cleanFrameArray(frames, frame_len);
				fprintf(fp,"\t%5d",faults);
			}
			fprintf(fp,"\n");
			//Cleanup Frames
			free(frames);
		}
		//Cleanup addresses
		free(addresses);
	}

	//Cleanup File
	if(argc > 1) fclose(fp);
	return 0;
}
