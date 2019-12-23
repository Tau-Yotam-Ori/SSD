#include "ssd.h"
#include "common.h"
#include <assert.h>

#define SEQUENTIAL 0
#define RANDOM 1
#define WORKLOAD_LENGTH 600000
// #define USAGE_VOLUME 0.7
#define PRINT_EVERY 100000
// #define MAX_LBA ((int)(SECTOR_NB * 1-OVP/100))

#define RUN_TEST(SETUP, FUNC)   if(SETUP) popen("rm data/*.dat", "r"); \
						        SSD_INIT(); \
 								FUNC; \
								if(SETUP) SSD_TERM()

extern int32_t* mapping_table;

/**
 * simple test that writes all sectors in the device sequentially
 */
int test_access()
{
	int ret, i, lba;

	// write entire device 
	for(i=0;i<SECTOR_NB;i+=SECTORS_PER_PAGE){
		if ((i/SECTORS_PER_PAGE) % 1024*10==0){
			LOG("wrote %.3lf of device", (double)i  / (double)SECTOR_NB);
		}

		lba = i % SECTOR_NB;
		SSD_WRITE(SECTORS_PER_PAGE, lba);
	}

	printf("wrote seq\n");

	return 0;
}

/**
 * simple test that writes all sectors in the device sequentially
 */
int sequential_workload(int units)
{
	int ret, i, lba;
	int num_of_requests = WORKLOAD_LENGTH;
	int max_lba = SECTOR_NB;
	printf("Sequential workload of length %d\n",num_of_requests);
	printf("over-provisioning: %d percent\n",OVP);

	// write entire device 
	for(i=0; i<num_of_requests; i+=(SECTORS_PER_PAGE*units)){
		lba = i % max_lba;
		SSD_WRITE((SECTORS_PER_PAGE*units), lba);

		if(i%PRINT_EVERY==0){
			printf("%d write requests\n",i);
		}
	}

	printf("wrote seq\n");

	return 0;
}

int random_workload(int units){
	int num_of_requests = WORKLOAD_LENGTH;

	printf("Random workload of length %d\n",num_of_requests);
	printf("over-provisioning: %d percent\n",OVP);

	for(int i=0;i<num_of_requests; i++){
		int address = rand() % SECTOR_NB;
		address = address - address % SECTORS_PER_PAGE;
		int length = units * SECTORS_PER_PAGE;

		SSD_WRITE(length, address);
		
		if(i%PRINT_EVERY==0){
			printf("%d write requests\n",i);
			float waf = (float)(get_gc_write_count()+WORKLOAD_LENGTH) / WORKLOAD_LENGTH;
			printf("WAF: %f\n",waf);
		}
	}
	return 0;
}

int main(int argc, char *argv[]){
	int units,mode;
	int setup = 1;
	//RUN_TEST(setup, test_access());
	//return 0;
	if(argc < 3){
		printf("Usage: ./tests [request size: 1,2,4,8] [workload: r / s]\n");
		return 1;
	}

	units = atoi(argv[1]);
	if(!(units == 1 || units == 2 || units == 4 || units == 8)){
		printf("invalid request size\n");
		return 1;
	}

	if(!strcmp(argv[2], "s")){
		RUN_TEST(setup, sequential_workload(units));
	}
	else if(!strcmp(argv[2], "r")){
		RUN_TEST(setup, random_workload(units));
	}
	else{
		printf("invalid workload\n");
		return 1;
	}

	float waf = (float)(get_gc_write_count()+WORKLOAD_LENGTH) / WORKLOAD_LENGTH;
	printf("WAF: %f\n",waf);
	return 0;
}
