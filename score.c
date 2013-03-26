#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESZ 256

struct _team; typedef struct _team TEAM;
typedef char SEED;

int process(TEAM *);
int score_outcome(SEED *, TEAM *);

struct _team
{
	char *name;
	SEED region_ul[15];
	SEED region_ll[15];
	SEED region_ur[15];
	SEED region_lr[15];
	SEED final_four[3];
};

int process(teams)
	TEAM *teams;
{
    SEED region_seed[16] = {1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15};
    SEED outcome[15];
    SEED *outcomep;
    int i;
    
    outcomep = outcome;
    for (i = 0; i < 15; i++) {
        if (i < 8)
            *outcomep++ = region_seed[2*i];
        else
            *outcomep++ = outcome[2*(i-8)];
    }
    for (i = 0; i < 15; i++)
        printf("%d ", outcome[i]);
    printf("\n");
    return 0;
}

void main() {
    process(NULL);
}

