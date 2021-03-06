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
  
  free(row);
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
  if (gamestate[index] > 0)
    return process(teams, nteams, gamestate, matchup + 2, index + 1);

  gamestate[index] = gamestate[matchup];
  if ((ret = process(teams, nteams, gamestate, matchup + 2, index + 1)) != 0)
    return ret;
  gamestate[index] = gamestate[matchup + 1];
  if ((ret = process(teams, nteams, gamestate, matchup + 2, index + 1)) != 0)
    return ret;
  gamestate[index] = 0;
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
  free(tids);
  return 0;
}

int
score_team(team, outcome)
     TEAM *team;
     SEED *outcome;
{
  int i, j, *round_score, score;
  int rounds[6] = { 2, 3, 5, 8, 13, 21 };
  
  score = 0;
  round_score = rounds;
  for (i = 0; i < 63; i++) {
    if (i == 32 || i == 48 || i == 56 || i == 60 || i == 62)
      round_score++;
  
    if (team->prediction[i] == outcome[i]) {
      score += *round_score;
      
      if (i < 32) {
        /* In the first round, it's an upset if the winner's seed is > 8. */
        if ((outcome[i] - 1) % 16 >= 8)
          score++;        
      } else if (i < 48) {
        /*
         * In the second round, you have to look at the first round matchup.  j
         * is the index of the first round matchup.
         */
        j = 2 * (i - 32);
        if (outcome[i] >= outcome[j] && outcome[i] >= outcome[j+1])
          score++;
      }
    }
  }
  
  return score;
}

int main(argc, argv)
     char *argv;
     int argc;
{
  FILE *fp;
  TEAM *teamp;
  int i, j, nteams, ret;
  SEED initial_bracket[16] = {1,16,8,9,5,12,4,13,3,14,6,11,7,10,2,15};
  SEED early_rounds[56] = {1,8,12,4,6,3,7,2,17,25,28,29,22,30,26,18,33,40,37,36,43,35,39,47,49,57,60,52,54,51,55,50,1,12,3,2,25,29,22,18,33,36,35,47,49,52,51,50,1,0,25,18,36,0,52,51};
  SEED *outcome;
  
  fp = fopen("picks", "r");
  ret = parse(fp, &teamp, &nteams);

  outcome = (SEED *)calloc(127, sizeof(SEED));
  for (i = 0; i < 4; i++)
    memcpy(outcome + i*16, initial_bracket, 16*sizeof(SEED));
  memcpy(outcome + 64, early_rounds, 56*sizeof(SEED));
  
  process(teamp, nteams, outcome, 96, 112);
  
  /* for (i = 0; i < nteams; i++) { */
  /*   printf("%s: %d\n", teamp[i].name, score_team(&teamp[i], outcome + 64)); */
  /* } */
}

