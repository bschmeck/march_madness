#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESZ 256

struct _team; typedef struct _team TEAM;
typedef char SEED;

int parse(FILE *, TEAM **, int *);
int process(TEAM *, int, SEED *, int, int);
int score_outcome(TEAM *, int, SEED *);
int score_team(TEAM *, SEED *);

struct _team
{
	char *name;
    SEED prediction[63];
};

/*
 * Given an open file stream, parse the input and create the corresponding team
 * data structures.
 */
int
parse(fp, teampp, nteamsp)
     FILE *fp;
     TEAM **teampp;
     int *nteamsp;
{
  SEED *seedp;
  TEAM *teams;
  char *chunk, *comma, *name, *row;
  int end_of_line, final_four, i, j, multiplier, nteams, offset, pick;

  i = -1;
  nteams = 10;
  teams = (TEAM *)malloc(nteams * sizeof(TEAM));
  row = (char *)malloc(LINESZ * sizeof(char));

  while (fgets(row, LINESZ, fp) != NULL) {
    i++;
    if (i >= nteams) {
      nteams *= 2;
      teams = realloc(teams, nteams * sizeof(TEAM));
    }

    chunk = row;
    
    /* The row starts with a name. */
    comma = strchr(chunk, ',');
    if (comma == NULL)
      return -1;
    *comma = '\0';
    teams[i].name = (char *)malloc((strlen(chunk) + 1) * sizeof(char));
    strcpy(teams[i].name, chunk);
    
    seedp = teams[i].prediction;
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
          
      *seedp++ = (SEED)atoi(chunk);
    }
  }
  *nteamsp = i + 1;
  *teampp = teams;
  
  return 0;
}

int process(teams, nteams, gamestate, matchup, index)
	TEAM *teams;
    SEED *gamestate;
    int index, matchup;
{
  int offset, ret;
  
  if (index > 126)
    return score_outcome(teams, nteams, &gamestate[64]);

  gamestate[index] = gamestate[matchup];
  if ((ret = process(teams, nteams, gamestate, matchup + 2, index + 1)) != 0)
    return ret;
  gamestate[index] = gamestate[matchup + 1];
  if ((ret = process(teams, nteams, gamestate, matchup + 2, index + 1)) != 0)
    return ret;
  return 0;
}

int
score_outcome(teams, nteams, outcome)
     TEAM *teams;
     SEED *outcome;
     int nteams;
{
  TEAM *team;
  int hi_score, i, nwinners, score, *tids, tid_size;

  tid_size = 100;
  tids = (int *)malloc(tid_size * sizeof(int));
  nwinners = 0;
  hi_score = 0;

  for (i = 0; i < nteams; i++) {
    score = score_team(&teams[i], outcome);
    if (score > hi_score) {
      hi_score = score;
      tids[0] = i;
      nwinners = 1;
    } else if (score == hi_score) {
      tids[nwinners++] = i;
      if (nwinners > tid_size) {
        tid_size *= 2;
        tids = (int *)realloc(tids, tid_size * sizeof(int));
      }
    }
  }
  printf("\"%d", outcome[0]);
  for (i = 1; i < 63; i++)
    printf(",%d", outcome[i]);
  printf("\":[\"%s\"", teams[tids[0]].name);
  for (i = 1; i < nwinners; i++)
    printf(",\"%s\"", teams[tids[i]].name);
  printf("],\n");

  return 0;
}

int
score_team(team, outcome)
     TEAM *team;
     SEED *outcome;
{
  int i, score;

  score = 0;
  for (i = 0; i < 63; i++)
    if (team->prediction[i] == outcome[i])
      score++;
  
  return score;
}

int main(argc, argv)
     char *argv;
     int argc;
{
  FILE *fp;
  TEAM *teamp;
  int i, j, nteams, ret;
  SEED outcome[63] = {1,9,5,4,6,3,10,2,17,24,21,20,22,19,23,18,33,40,37,36,43,35,39,34,49,56,53,52,54,51,55,50,1,4,3,2,17,20,19,18,33,37,35,34,49,52,54,50,1,3,17,18,33,35,49,50,1,17,35,50,17,50,50};
  
  fp = fopen("picks", "r");
  ret = parse(fp, &teamp, &nteams);

  for (i = 0; i < nteams; i++) {
    printf("%s", teamp[i].name);
    for (j = 0; j < 63; j++) {
      printf(",%d", teamp[i].prediction[j]);
    }
    printf("\n");
  }

  score_outcome(teamp, nteams, outcome);
}

