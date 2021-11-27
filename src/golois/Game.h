
const int MaxGames = 1000000;
const int MaxMoveGame = 1000;
const int MaxExamples = 300000000;
int nbGames = 0;
int startGame [MaxGames];
int nbMovesSGFGame [MaxGames];
//Move proGame [MaxGames] [MaxMoveGame];
Move *proGame [MaxGames];
char winner [MaxGames];
float komi [MaxGames];

bool proGameAllocated = false;

class PositionSGF {
 public:
  int game;
  short int move;
};

int nbPositionsSGF = 0;
PositionSGF * positionSGF = NULL;
int startShuffle = 0;

void shuffle () {
  fprintf (stderr, "nbPositionsSGF = %d\n", nbPositionsSGF);
  for (int i = startShuffle; i < nbPositionsSGF; i++) {
    int other = startShuffle + rand () % (nbPositionsSGF - startShuffle);
    PositionSGF tmp = positionSGF [i];
    positionSGF [i] = positionSGF [other];
    positionSGF [other] = tmp;
  }
}

void loadGames (char * name) {
  if (!proGameAllocated) {
    proGameAllocated = true;
    for (int i = 0; i < MaxGames; i++)
      proGame [i] = new Move [MaxMoveGame];
  }

  FILE * fp = fopen (name, "r");
  nbPositionsSGF = 0;
  nbGames = 0;
  if (positionSGF == NULL)
    positionSGF = new PositionSGF [MaxGames * MaxMoveGame];
  if (fp != NULL) {
    char game [1000];
    int res = 0;
    while ((res != -1) && (nbGames < MaxGames)) {
      res = fscanf (fp, "%s", game);
      if (res != -1) {
        FILE * sgf = fopen (game, "r");
        if (sgf != NULL) {
          Board b = board;
	  b.loadSGF (sgf);
	  //if (nbGames == 180)
	  //b.print (stderr);
	  fprintf (stderr, "%s ", game);
	  winner [nbGames] = b.winner;
	  nbMovesSGFGame [nbGames] = 0;
	  for (int i = 0; i < b.NbMovesPlayed; i++) {
	    //if (b.Moves [i] != b.passe) {
	    Move m;
	    m.inter = moveInter [b.Moves [i]];
	    m.color = b.colorMove [i];
	    if (i == 0)
	      m.val = 0.0;
	    else
	      m.val = b.val [i - 1];
	    //fprintf (stderr, "m.val = %2.3f ", m.val);
	    if (nbMovesSGFGame [nbGames] < MaxMoveGame - 1) {
	      proGame [nbGames] [nbMovesSGFGame [nbGames]] = m;
	      positionSGF [nbPositionsSGF].game = nbGames;
	      positionSGF [nbPositionsSGF].move = nbMovesSGFGame [nbGames];
	      nbPositionsSGF++;
	      nbMovesSGFGame [nbGames]++;
	    }
	    //}
	  }
	  nbGames++;
          fclose (sgf);
        }
      }
    }
    fclose (fp);
  }
  //shuffle ();
}

void loadGamesOneFile (char * name) {
  if (!proGameAllocated) {
    proGameAllocated = true;
    for (int i = 0; i < MaxGames; i++)
      proGame [i] = new Move [MaxMoveGame];
  }

  FILE * fp = fopen (name, "r");
  if (positionSGF == NULL)
    positionSGF = new PositionSGF [MaxGames * MaxMoveGame];
  if (fp != NULL) {
    int lines = 0;
    while (true) {
      Board b = board;
      int res = b.loadSGF (fp);
      lines++;
      fprintf (stderr, "%d %d ", res, lines);
      if (res == 0)
	break;
      bool use = true;
      if (res == -1)
	use = false;
      if ((b.winner != 'B') && (b.winner != 'W'))
	use = false;
      //if ((b.winner == 'W') && (b.komi [White] > 7.5))
      //use = false;
      //if ((b.winner == 'B') && (b.komi [White] < 7.5))
      //use = false;
      //if (b.komi [White] != 7.5)

      if ((b.komi [White] > 9.5) || (b.komi [White] < 0.5))
	use = false;
      if ((int)(2 * b.komi [White]) == 2 * (int)(b.komi [White]))
	use = false;

      //if ((res != -1) && ((b.winner == 'B') || (b.winner == 'W'))) {
      if (use) {
	//if (nbGames == 180)
	//b.print (stderr);
	winner [nbGames] = b.winner;
	komi [nbGames] = b.komi [White];
	nbMovesSGFGame [nbGames] = 0;
	//startGame [nbGames] = b.startGame;
	//fprintf (stderr, " start = %d ", b.startGame);
	for (int i = 0; i < b.NbMovesPlayed; i++) {
	  //if (b.Moves [i] != b.passe) {
	  Move m;
	  m.inter = moveInter [b.Moves [i]];
	  m.color = b.colorMove [i];
	  //if (i == 0)
	  //m.val = 0.0;
	  //else
	  //m.val = b.val [i - 1];
	  m.val = b.val [i];
	  m.points = b.points [i];
	  //fprintf (stderr, "m.val = %2.3f ", m.val);
	  if (nbMovesSGFGame [nbGames] < MaxMoveGame - 1) {
	    proGame [nbGames] [nbMovesSGFGame [nbGames]] = m;
	    positionSGF [nbPositionsSGF].game = nbGames;
	    positionSGF [nbPositionsSGF].move = nbMovesSGFGame [nbGames];
	    nbPositionsSGF++;
	    nbMovesSGFGame [nbGames]++;
	  }
	  //}
	}
	nbGames++;
	if (nbGames == MaxGames)
	  break;
      }
    }
    fclose (fp);
  }
}

void writeGamesData (char * name) {
  FILE * fp = fopen (name, "w");
  if (fp != NULL) {
    int nb = 0;
    for (int g = 0; g < nbGames; g++)
      if ((winner [g] == 'W') || (winner [g] == 'B'))
        nb++;
    fprintf (fp, "%d\n", nb);
    for (int g = 0; g < nbGames; g++)
      if ((winner [g] == 'W') || (winner [g] == 'B')) {
        fprintf (fp, "%2.1f ", komi [g]);
        fprintf (fp, "%c ", winner [g]);
	fprintf (fp, "%d ", nbMovesSGFGame [g]);
	//fprintf (fp, "%d ", startGame [g]);
        for (int i = 0; i < nbMovesSGFGame [g]; i++) {
          fprintf (fp, "%d %d %2.2f %2.2f ", proGame [g] [i].inter, proGame [g] [i].color, proGame [g] [i].val, proGame [g] [i].points);
        }
        fprintf (fp, "\n");
      }
    fclose (fp);
  }
}

void loadGamesData (char * name) {
  //if (!proGameAllocated) {
  //proGameAllocated = true;
  //for (int i = 0; i < MaxGames; i++)
  //  proGame [i] = new Move [MaxMoveGame];
  //}

  FILE * fp = fopen (name, "r");
  if (fp == NULL) {
    fprintf (stderr, "Error loading %s\n", name);
    return;
  }
  char s [1000];
  nbPositionsSGF = 0;
  if (positionSGF == NULL)
    //positionSGF = new PositionSGF [MaxGames * MaxMoveGame];
    positionSGF = new PositionSGF [MaxExamples];
  if (fp != NULL) {
    fscanf (fp, "%d", &nbGames);
    for (int g = 0; g < nbGames; g++) {
      fscanf (fp, "%f", &komi [g]);
      fscanf (fp, "%s ", s);
      winner [g] = s [0];
      //fprintf (stderr, "%c", winner [g]);
      fscanf (fp, "%d", &nbMovesSGFGame [g]);
      proGame [g] = new Move [nbMovesSGFGame [g] + 1];
      //fscanf (fp, "%d", &startGame [g]);
      Board b = board;
      bool illegal = false;
      int prevnb = nbPositionsSGF;
      for (int i = 0; i < nbMovesSGFGame [g]; i++) {
        fscanf (fp, "%d %d %f %f", &proGame [g] [i].inter, &proGame [g] [i].color, &proGame [g] [i].val, &proGame [g] [i].points);
	if (!illegal) {
          int move = proGame [g] [i].inter;
          if (move != 361) {
	    if (b.board [interMove [move]] != Empty) {
	      illegal = true;
	      fprintf (stderr, "not empty ");
	    }
	    if (!b.legalMove (interMove [move], proGame [g] [i].color)) {
	      illegal = true;
	      fprintf (stderr, "not legal ");
	    }
	    if (illegal) {
	      fprintf (stderr, "illegal ");
	      b.print (stderr);
	      fprintf (stderr, "Bug play, move = %d (%d,%d), color = %d\n", move, move / 19, move % 19, proGame [g] [i].color);
	      nbPositionsSGF = prevnb;
	    }
	    /*
              else {
                b.print (stderr);
                fprintf (stderr, "b.length = %d\n", b.length);
                fprintf (stderr, "g = %d, nbPositionsSGF = %d, nbMoves [%d] = %d, winner [%d] = %c\n",
                         g, nbPositionsSGF, g, nbMovesSGFGame [g], g, winner [g]);
                fprintf (stderr, "Bug play, move = %d (%d,%d)\n", move, move / 19, move % 19);
                exit (1);
              }
	    */
	  }
          if (!illegal) {
            b.joue (interMove [move], proGame [g] [i].color);
	    //if ((move != 361) && (i >= startGame [g]))
	    if (proGame [g] [i].val > -1.0) {
	      positionSGF [nbPositionsSGF].game = g;
	      positionSGF [nbPositionsSGF].move = i;
	      if (move != 361)
		nbPositionsSGF++;
	    }
          }
        }
      }
    }
    fprintf (stderr, "nbPositionsSGF = %d\n", nbPositionsSGF);
    //shuffle ();
    fclose (fp);
  }
}

void writeGamesDataVal (char * name) {
  FILE * fp = fopen (name, "w");
  if (fp != NULL) {
    fprintf (fp, "%d\n", nbGames);
    for (int g = 0; g < nbGames; g++) {
      fprintf (fp, "%c ", winner [g]);
      fprintf (fp, "%d ", nbMovesSGFGame [g]);
      for (int i = 0; i < nbMovesSGFGame [g]; i++) {
	fprintf (fp, "%d %2.3f %d ", proGame [g] [i].inter, proGame [g] [i].val, proGame [g] [i].color);
      }
      fprintf (fp, "\n");
    }
    fclose (fp);
  }
}

void loadGamesDataVal (char * name) {
  FILE * fp = fopen (name, "r");
  if (fp == NULL) {
    fprintf (stderr, "Error loading %s\n", name);
    return;
  }
  char s [1000];
  nbPositionsSGF = 0;
  if (positionSGF == NULL)
    positionSGF = new PositionSGF [MaxGames * MaxMoveGame];
  if (fp != NULL) {
    fscanf (fp, "%d", &nbGames);
    for (int g = 0; g < nbGames; g++) {
      fscanf (fp, "%s ", s);
      winner [g] = s [0];
      fscanf (fp, "%d", &nbMovesSGFGame [g]);
      Board b = board;
      for (int i = 0; i < nbMovesSGFGame [g]; i++) {
	fscanf (fp, "%d %f %d ", &proGame [g] [i].inter, &proGame [g] [i].val, &proGame [g] [i].color);
	positionSGF [nbPositionsSGF].game = g;
	positionSGF [nbPositionsSGF].move = i;
	int move = proGame [g] [i].inter;
	if (move != 361)
	  if ((b.board [interMove [move]] != Empty) || !b.legalMove (interMove [move], b.turn)) {
	    b.print (stderr);
	    fprintf (stderr, "b.length = %d\n", b.length);
	    fprintf (stderr, "g = %d, nbPositionsSGF = %d, nbMoves [%d] = %d, winner [%d] = %c\n",
		     g, nbPositionsSGF, g, nbMovesSGFGame [g], g, winner [g]);
	    fprintf (stderr, "Bug play, move = %d (%d,%d)\n", move, move / 19, move % 19);
	    exit (1);
	  }
	b.play (move);
	nbPositionsSGF++;
      }
    }
    fprintf (stderr, "nbPositionsSGF = %d\n", nbPositionsSGF);
    //shuffle ();
    fclose (fp);
  }
}

const int Planes = 5;
const int MaxLib = 6;

char historyBoard [MaxPlayoutLength] [MaxSize];

void play (Board * board, int move) {
  if (move != 361)
    if (board->board [interMove [move]] != Empty) {
      board->print (stderr);
      fprintf (stderr, "board->length = %d\n", board->length);
      fprintf (stderr, "Bug play, move = %d (%d,%d)\n", move, move / 19, move % 19);
      exit (1);
    }
  board->play (move);
  memcpy (historyBoard [board->length], board->board, MaxSize);
  //fprintf (stderr, "move = %d (%d,%d)\n", move, move / 19, move % 19);
  //board->print (stderr);
}

void playColor (Board * board, int move, int color) {
  if (move != 361)
    if (board->board [interMove [move]] != Empty) {
      board->print (stderr);
      fprintf (stderr, "board->length = %d\n", board->length);
      fprintf (stderr, "Bug play, move = %d (%d,%d)\n", move, move / 19, move % 19);
      exit (1);
    }
  board->playColor (move, color);
  memcpy (historyBoard [board->length], board->board, MaxSize);
  //fprintf (stderr, "move = %d (%d,%d)\n", move, move / 19, move % 19);
  //board->print (stderr);
}

float input [19] [19] [2 * Planes + 5 + MaxLib + 10];
float group [19] [19] [1];

void encode (Board * board) {
  if (board->turn == Black) {
    for (int i = 0; i < 19; i++)
      for (int j = 0; j < 19; j++)
	input [i] [j] [0] = 1.0;
  }
  else {
    for (int i = 0; i < 19; i++)
      for (int j = 0; j < 19; j++)
	input [i] [j] [0] = 0.0;
  }
  board->computeLostLadders ();
  for (int i = 0; i < 19; i++)
    for (int j = 0; j < 19; j++) 
      input [i] [j] [1] = board->isLostLadder [interMove [19 * i + j]];
  int start = 2;
  int other = Black;
  if (board->turn == Black)
    other = White;
  int current = board->length;
  for (int plane = 0; plane < Planes; plane++) {
    if (current <= 0) {
      for (int i = 0; i < 19; i++)
	for (int j = 0; j < 19; j++) {
	  input [i] [j] [start + 2 * plane] = 0.0;
	  input [i] [j] [start + 2 * plane + 1] = 0.0;
	}
    }
    else {
      for (int i = 0; i < 19; i++)
	for (int j = 0; j < 19; j++) {
	  if (historyBoard [current] [interMove [19 * i + j]] == board->turn) {
	    input [i] [j] [start + 2 * plane] = 1.0;
	    input [i] [j] [start + 2 * plane + 1] = 0.0;
	  }
	  else if (historyBoard [current] [interMove [19 * i + j]] == other) {
	    input [i] [j] [start + 2 * plane] = 0.0;
	    input [i] [j] [start + 2 * plane + 1] = 1.0;
	  }
	  else {
	    input [i] [j] [start + 2 * plane] = 0.0;
	    input [i] [j] [start + 2 * plane + 1] = 0.0;
	  }
	}
    }
    current--;
  }   
}

int sym [362] [8];

int moveFromxy (int x, int y) {
  return 19 * x + y;
}

void init_all_moves () {
  for (int row = 0; row < 19; row++)
    for (int col = 0; col < 19; col++) {
      int move = moveFromxy (row, col);
      sym [move] [0] = moveFromxy (row, col);
      sym [move] [1] = moveFromxy (18 - row, col);
      sym [move] [2] = moveFromxy (row, 18 - col);
      sym [move] [3] = moveFromxy (18 - row, 18 - col);
      sym [move] [4] = moveFromxy (col, row);
      sym [move] [5] = moveFromxy (18 - col, row);
      sym [move] [6] = moveFromxy (col, 18 - row);
      sym [move] [7] = moveFromxy (18 - col, 18 - row);
    }
  for (int s = 0; s < 8; s++)
    sym [361] [s] = 361;
}


void encodeSym (Board * board, int randomSym) {
  for (int i = 0; i < 19; i++)
    for (int j = 0; j < 19; j++) {
      int move = moveFromxy (i, j);
      int x = sym [move] [randomSym] / 19;
      int y = sym [move] [randomSym] % 19;
      if (board->board [interMove [move]] == Empty)
        group [x] [y] [0] = 0;
      else
        group [x] [y] [0] = 1 + board->indiceChaine [board->premierePierre[interMove [move]]];
    }
  /*
  board->print (stderr);
  for (int i = 0; i < 19; i++) {
    for (int j = 0; j < 19; j++) {
      fprintf (stderr, "%2.0f ", group [i] [j] [0]);
    }
    fprintf (stderr, "\n");
  }
  fprintf (stderr, "\n");
  */
  if (board->turn == Black) {
    for (int i = 0; i < 19; i++)
      for (int j = 0; j < 19; j++)
	input [i] [j] [0] = 1.0;
  }
  else {
    for (int i = 0; i < 19; i++)
      for (int j = 0; j < 19; j++)
	input [i] [j] [0] = 0.0;
  }
  board->computeLostLadders ();
  for (int i = 0; i < 19; i++)
    for (int j = 0; j < 19; j++) {
      int move = moveFromxy (i, j);
      int x = sym [move] [randomSym] / 19;
      int y = sym [move] [randomSym] % 19;
      input [x] [y] [1] = board->isLostLadder [interMove [move]];
      input [x] [y] [2] = board->adjIsLostLadder [interMove [move]];
    }
  board->computeIsInLadder ();
  for (int i = 0; i < 19; i++)
    for (int j = 0; j < 19; j++) {
      int move = moveFromxy (i, j);
      int x = sym [move] [randomSym] / 19;
      int y = sym [move] [randomSym] % 19;
      input [x] [y] [3] = board->isInLadder [interMove [move]];
      input [x] [y] [4] = board->adjIsInLadder [interMove [move]];
    }
  int minPlane = 5;
  int stack [1000];
  for (int i = 0; i < 19; i++)
    for (int j = 0; j < 19; j++) {
      int move = moveFromxy (i, j);
      int x = sym [move] [randomSym] / 19;
      int y = sym [move] [randomSym] % 19;
      for (int plane = 0; plane < MaxLib; plane++)
        input [x] [y] [minPlane + plane] = 0;
      if (board->board [interMove [move]] != Empty) {
	int n = board->nbLibertes (interMove [move], stack);
	if (n > MaxLib)
	  n = MaxLib;
        input [x] [y] [minPlane + n - 1] = 1;
      }
    }
  int start = minPlane + MaxLib;
  int other = Black;
  if (board->turn == Black)
    other = White;
  int current = board->length;
  for (int plane = 0; plane < Planes; plane++) {
    if (current <= 0) {
      for (int i = 0; i < 19; i++)
	for (int j = 0; j < 19; j++) {
	  input [i] [j] [start + 2 * plane] = 0.0;
	  input [i] [j] [start + 2 * plane + 1] = 0.0;
	}
    }
    else {
      for (int i = 0; i < 19; i++)
	for (int j = 0; j < 19; j++) {
	  int move = moveFromxy (i, j);
	  int x = sym [move] [randomSym] / 19;
	  int y = sym [move] [randomSym] % 19;
	  if (historyBoard [current] [interMove [move]] == board->turn) {
	    input [x] [y] [start + 2 * plane] = 1.0;
	    input [x] [y] [start + 2 * plane + 1] = 0.0;
	  }
	  else if (historyBoard [current] [interMove [move]] == other) {
	    input [x] [y] [start + 2 * plane] = 0.0;
	    input [x] [y] [start + 2 * plane + 1] = 1.0;
	  }
	  else {
	    input [x] [y] [start + 2 * plane] = 0.0;
	    input [x] [y] [start + 2 * plane + 1] = 0.0;
	  }
	}
    }
    current--;
  }   
}

void encodeSymKomi (Board * board, int randomSym) {
  encodeSym (board, randomSym);
  for (int k = 0; k < 10; k++) {
    int p = 2 * Planes + 5 + MaxLib + k;
    if ((int)(board->komi [White]) == k) {
      for (int i = 0; i < 19; i++)
	for (int j = 0; j < 19; j++) {
	  input [i] [j] [p] = 1.0;
	}
    }
    else {
      for (int i = 0; i < 19; i++)
	for (int j = 0; j < 19; j++) {
	  input [i] [j] [p] = 0.0;
	}
    }
  }
}




