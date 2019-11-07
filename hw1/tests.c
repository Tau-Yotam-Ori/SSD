#include "ssd.h"
#include "common.h"
#include <assert.h>

#define MEMORY_USAGE 0.7
#define RUN_TEST(SETUP, FUNC)   if(SETUP) popen("rm data/*.dat", "r"); \
						        SSD_INIT(); \
 								FUNC; \
								if(SETUP) SSD_TERM()

extern int32_t* mapping_table;


/**
 * simple test that writes all sectors in the device sequentially
 */
int test_access(int size, char type)
{
	int ret, i, lba, loop_size;

	if (type == 's' || type == ' '){
		if (type == ' '){
			loop_size = SECTOR_NB;
		}
		else {
			loop_size = size*SECTORS_PER_PAGE;
		}
		for(i = 0; i < loop_size; i += SECTORS_PER_PAGE){
			if ((i/SECTORS_PER_PAGE) % 1024*10==0){
				LOG("wrote %.3lf of device", (double)i  / (double)SECTOR_NB);
			}
			printf("i: %d\n", i);

			lba = i % (int)(MEMORY_USAGE*SECTOR_NB); // get valid lba

			SSD_WRITE(SECTORS_PER_PAGE, lba);
		}

		printf("wrote seq\n");
	}
	else {
		for(i=0;i<size*SECTORS_PER_PAGE;i+=SECTORS_PER_PAGE){
			if ((i/SECTORS_PER_PAGE) % 1024*10==0){
				LOG("wrote %.3lf of device", (double)i  / (double)SECTOR_NB);
			}
			lba = rand() % (int)(0.7*SECTOR_NB); // get valid lba
			

			lba = lba - (lba % SECTORS_PER_PAGE); // align lba

			SSD_WRITE(SECTORS_PER_PAGE, lba);
		}

		printf("wrote random\n");
	}

	return 0;
}



int main(int argc, char *argv[]){
	int setup = 1;
	int size=-1;
	char type = ' ';
	if (argc == 3){
		size = (int)strtol(argv[1], NULL, 10);
		type = argv[2][0];
		if (type != 'r' && type != 's'){
			LOG("Invalid second argument!");
			return 1;
		}
	}
	else if (argc != 1) {
		LOG("Invalid amount of arguments!");
		return 1;
	}
	RUN_TEST(setup, test_access(size, type));
	return 0;
}