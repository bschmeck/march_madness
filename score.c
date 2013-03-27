#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESZ 256

struct _team; typedef struct _team TEAM;
typedef char SEED;

int parse(FILE *, TEAM **);
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

/*
 * Given an open file stream, parse the input and create the corresponding team
 * data structures.
 */
int
parse(fp, teampp)
     FILE *fp;
     TEAM **teampp;
{
  SEED *seedp;
  TEAM *teamp;
  char *chunk, *comma, *name, *row;
  int i, nteams, offset;

  offset = -1;
  nteams = 10;
  teamp = (TEAM *)malloc(nteams * sizeof(TEAM));
  row = (char *)malloc(LINESZ * sizeof(char));

  while (fgets(row, LINESZ, fp) != NULL) {
    chunk = row;
    
    /* The row starts with a name. */
    comma = strchr(chunk, ',');
    if (comma == NULL)
      return -1;
    *comma = '\0';
    name = (char *)malloc((strlen(chunk) + 1) * sizeof(char));
    strcpy(name, chunk);
    /* If this is a new name, or the first name, set the team's name. */
    if (offset < 0 || strcmp(name, teamp[offset].name) != 0) {
      offset++;
      /* Dynamically grow the team array, if needed. */
      if (offset >= nteams) {
        nteams *= 2;
        teamp = realloc(teamp, nteams * sizeof(TEAM));
      }
      teamp[offset].name = name;
    }
  }

  for (i = 0; i <= offset; i++)
    printf("TEAM: %s\n", teamp[i].name);
  
  return 0;
}

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

int main(argc, argv)
     char *argv;
     int argc;
{
  FILE *fp;
  TEAM *teamp;
  int ret;

  fp = fopen("picks", "r");
  ret = parse(fp, &teamp);
}

