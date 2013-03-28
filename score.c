#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESZ 256

struct _team; typedef struct _team TEAM;
typedef char SEED;

int parse(FILE *, TEAM **);
int process(TEAM *, SEED *, int, int);
int score_outcome(TEAM *, SEED *);
int score_team(TEAM *, SEED *);

struct _team
{
	char *name;
	SEED region_ul[14];
	SEED region_ll[14];
	SEED region_ur[14];
	SEED region_lr[14];
	SEED final_four[7];
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
  int end_of_line, final_four, i, j, multiplier, nteams, offset;

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
  
    /* Next comes the region identifier. */
    chunk = comma + 1;
    comma = strchr(chunk, ',');
    *comma = '\0';
    if (strncmp(chunk, "UL", 2) == 0) {
      seedp = teamp[offset].region_ul;
      multiplier = 0;
    } else if (strncmp(chunk, "LL", 2) == 0) {
      seedp = teamp[offset].region_ll;
      multiplier = 1;
    } else if (strncmp(chunk, "UR", 2) == 0) {
      seedp = teamp[offset].region_ur;
      multiplier = 2;
    } else if (strncmp(chunk, "LR", 2) == 0) {
      seedp = teamp[offset].region_lr;
      multiplier = 3;
    } else if (strncmp(chunk, "FF", 2) == 0) {
      seedp = teamp[offset].final_four;
    } else
      printf("Unknown region: %s\n", chunk);

    final_four = seedp == teamp[offset].final_four;
    end_of_line = 0;
    while (!end_of_line) {
      chunk = comma + 1;
      comma = strchr(chunk, ',');
      if (comma == NULL) {
        comma = strchr(chunk, '\n');
        if (comma == NULL)
          return -1;
        end_of_line = 1;
      }
      *comma = '\0';
      /* The final four seeds are prefixed with the team's region. */
      if (final_four) {
        if (strncmp(chunk, "UL", 2) == 0)
          multiplier = 0;
        else if (strncmp(chunk, "LL", 2) == 0)
          multiplier = 1;
        else if (strncmp(chunk, "UR", 2) == 0)
          multiplier = 2;
        else if (strncmp(chunk, "LR", 2) == 0)
          multiplier = 3;
        else
          printf("Unknown region: %s\n", chunk);
        chunk += 2;
      }
      *seedp++ = (SEED)atoi(chunk) + 16*multiplier;
    }
  }

  return 0;
}

int process(teams, gamestate, matchup, index)
	TEAM *teams;
    SEED *gamestate;
    int index, matchup;
{
  int offset, ret;
  
  if (index > 126)
    return score_outcome(teams, gamestate);

  gamestate[index] = gamestate[matchup];
  if ((ret = process(teams, gamestate, matchup + 2, index + 1)) != 0)
    return ret;
  gamestate[index] = gamestate[matchup + 1];
  if ((ret = process(teams, gamestate, matchup + 2, index + 1)) != 0)
    return ret;
  return 0;
}

int
score_outcome(teams, outcome)
     TEAM *teams;
     SEED *outcome;
{
  TEAM *team;
  int hi_score, i, nwinners, score, *tids, tid_size;

  tid_size = 100;
  tids = (int *)malloc(tid_size * sizeof(int));
  nwinners = 0;
  hi_score = 0;

  for (i = 0; teams[i] != NULL; i++) {
    score = score_team(teams[i], outcome);
    if (score > hi_score) {
      tids[0] = i;
      nwinners = 1;
    } else if (score == hi_score) {
      tids[nwinners++] = i;
      if (nwinners > tid_size) {
        tid_size *= 2;
        tids = (int *)realloc(tid_size * sizeof(int));
      }
    }
  }
  printf("%d", outcome[0]);
  for (i = 1; i < 63; i++)
    printf(",%d", outcome[i]);
  for (i = 0; i < nwinners; i++)
    printf(",%s", team[i].name);
  printf("\n");

  return 0;
}

int
score_team(team, outcome)
     TEAM *team;
     SEED *outcome;
{
  int i, score;

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

