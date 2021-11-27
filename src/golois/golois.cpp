#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <list>
#include <float.h>
#include <vector>
#include <set>
#include <algorithm>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

using namespace std;

#include "Board.h"
#include "Game.h"

bool loaded = false;

int nbExamples = 128;

int nbCallsGetBatch = 0;

int * indexValidation;
bool * validationGame;
int * nbTimesGameIsUsed;

int * numberGame, * numberMove;

float soft = 1.0;

static Board b;

void setSoft (float s) {
  soft = s;
}

PYBIND11_MODULE(golois, m) {
  m.def("setSoft", &setSoft, "setSoft");
  m.def("getBatch", [](py::array_t<float> x, py::array_t<float> policy,
		       py::array_t<float> value, py::array_t<float> end, py::array_t<float> groups, int index) {
	nbCallsGetBatch++;
	// if (!loaded) {
	//   memcpy (historyBoard [0], board.board, MaxSize);
	//   init_all_moves ();
	//   int seed = time (NULL);
	//   srand (seed); // if we run again the same model examples should be different
        //   fprintf (stderr, "load games.data, seed = %d, RAND_MAX = %d, rand () = %d\n", seed, RAND_MAX, rand ());
	//   loadGamesData ("games.data");
        //   fprintf (stderr, "MaxGroups = %d\n", MaxGroups);
	//   loaded = true;
	//   indexValidation = new int [nbExamples];
	//   validationGame = new bool [MaxGames];
	//   nbTimesGameIsUsed = new int [MaxGames];
	//   for (int i = 0; i < nbExamples; i++) 
	//     indexValidation [i] = (int) (((float)rand () / (RAND_MAX + 1.0)) * nbPositionsSGF);
	//   for (int i = 0; i < MaxGames; i++) {
	//     validationGame [i] = false;
	//     nbTimesGameIsUsed [i] = 0;
	//   }
	// }
	auto r = x.mutable_unchecked<4>();
	auto pi = policy.mutable_unchecked<2>();
	auto v = value.mutable_unchecked<1>();
 	auto e = end.mutable_unchecked<4>();
 	auto g = groups.mutable_unchecked<4>();
	nbExamples = r.shape (0);
	fprintf (stderr, "r.shape = (%d, %d, %d, %d)\n", r.shape (0), r.shape (1), r.shape (2), r.shape (3));
	fprintf (stderr, "nbExamples = %d\n", nbExamples);
	for (ssize_t i = 0; i < nbExamples; i++) {
	  //fprintf (stderr, "i = %d, ", i);
	  // choose a random state
	  int pos = (index + i) % nbPositionsSGF;
	  int game = positionSGF [pos].game;
	  //nbTimesExampleIsUsed [pos]++;
	  //nbTimesGameIsUsed [game]++;
	  //fprintf (stderr, "pos = %d, positionSGF [pos].game = %d, positionSGF [pos].move = %d\n",
	  //	   pos, positionSGF [pos].game, positionSGF [pos].move);
	  b = board;
	  for (int j = 0; j < positionSGF [pos].move; j++)
	    playColor (&b, proGame [positionSGF [pos].game] [j].inter, proGame [positionSGF [pos].game] [j].color);
	  b.komi [White] = komi [positionSGF [pos].game];
	  //fprintf (stderr, "fill the input\n");
	  // fill the input
	  int turn = b.turn;
	  int other = Black;
	  if (b.turn == Black)
	    other = White;
	  int randomSym = 0;
	  randomSym = rand () % 8;
	  //encodeSym (&b, randomSym);
	  encodeSymKomi (&b, randomSym);
	  for (ssize_t j = 0; j < r.shape(1); j++)
	    for (ssize_t k = 0; k < r.shape(2); k++)
	      for (ssize_t l = 0; l < r.shape(3); l++) {
		r (i, j, k, l) = input [j] [k] [l];
	      }
	  for (ssize_t j = 0; j < g.shape(1); j++)
	    for (ssize_t k = 0; k < g.shape(2); k++)
	      for (ssize_t l = 0; l < g.shape(3); l++) {
		g (i, j, k, l) = group [j] [k] [l];
	      }
	  // fill the policy
 	  for (ssize_t j = 0; j < pi.shape(1); j++)
	    pi (i, j) = 0.0;
	  int move = proGame [positionSGF [pos].game] [positionSGF [pos].move].inter;
	  move = sym [move] [randomSym];
	  pi (i, move) = 1.0;
	  // fill the value
	  v (i) = proGame [positionSGF [pos].game] [positionSGF [pos].move].val;
	  //if (winner [positionSGF [pos].game] == 'W') {
	  //v (i) = 1.0;
	    /*
	    if (soft < 1.0) {
	      if (positionSGF [pos].move > 200)
		v (i) = 1.0;
	      else
		v (i) = soft + (1.0 - soft) * positionSGF [pos].move / 200.0;
	    }
	    */
	  //}
	  //else {
	  //v (i) = 0.0;
	    /*
	    if (soft < 1.0) {
	      if (positionSGF [pos].move > 200)
		v (i) = 0.0;
	      else
		v (i) = 1.0 - soft - (1.0 - soft) * positionSGF [pos].move / 200.0;
	    }
	    */
	  //}
	  //fprintf (stderr, "fill the endgame\n");
	  // fill the endgame
	  for (int j = positionSGF [pos].move; j < nbMovesSGFGame [positionSGF [pos].game]; j++) {
	    //fprintf (stderr, "positionSGF [pos].game = %d\n", positionSGF [pos].game);
	    //fprintf (stderr, "nbMovesSGFGame [positionSGF [pos].game] = %d\n", nbMovesSGFGame [positionSGF [pos].game]);
	    //fprintf (stderr, "j = %d\n", j);
	    playColor (&b, proGame [positionSGF [pos].game] [j].inter, proGame [positionSGF [pos].game] [j].color);
	  }
	  for (int j = 0; j < 19; j++)
	    for (int k = 0; k < 19; k++) {
	      // - todo randomSym
	      if (b.board [interMove [19 * j + k]] == turn) {
		e (i, j, k, 0) = 1.0;
		e (i, j, k, 1) = 0.0;
	      }
	      else if (b.board [interMove [19 * j + k]] == other) {
		e (i, j, k, 0) = 0.0;
		e (i, j, k, 1) = 1.0;
	      }
	      else {
		e (i, j, k, 0) = 0.0;
		e (i, j, k, 1) = 0.0;
	      }
	    }
	}
    });
  m.def("getValidation", [](py::array_t<float> x, py::array_t<float> policy,
		       py::array_t<float> value, py::array_t<float> end) {
	auto r = x.mutable_unchecked<4>();
	auto pi = policy.mutable_unchecked<2>();
	auto v = value.mutable_unchecked<1>();
 	auto e = end.mutable_unchecked<4>();
	nbExamples = r.shape (0);
	fprintf (stderr, "r.shape = (%d, %d, %d, %d)\n", r.shape (0), r.shape (1), r.shape (2), r.shape (3));
	fprintf (stderr, "nbExamples = %d\n", nbExamples);
	if (!loaded) {
	  memcpy (historyBoard [0], board.board, MaxSize);
	  init_all_moves ();
	  //int seed = time (NULL);
	  //srand (seed); // no, deterministic for getBatch (same seed = 0, same validation set)
          //fprintf (stderr, "load games.data, seed = %d, RAND_MAX = %d, rand () = %d\n", seed, RAND_MAX, rand ());
	  srand (0);
	  loadGamesData ("games.data");
	  loaded = true;
	  shuffle ();
	  indexValidation = new int [nbExamples];
	  validationGame = new bool [MaxGames];
	  nbTimesGameIsUsed = new int [MaxGames];
	  for (int i = 0; i < nbExamples; i++) 
	    indexValidation [i] = (int) (((float)rand () / (RAND_MAX + 1.0)) * nbPositionsSGF);
	  for (int i = 0; i < MaxGames; i++) {
	    validationGame [i] = false;
	    nbTimesGameIsUsed [i] = 0;
	  }
	  FILE * fp = fopen ("validation.data", "r");
	  if (fp == NULL) {
	    fprintf (stderr, "generating validation.data\n");
	    for (ssize_t i = 0; i < nbExamples; i++) {
	      // choose a random state
	      int pos = (int) (((float)rand () / (RAND_MAX + 1.0)) * nbPositionsSGF);
	      //int pos = rand () % nbPositionsSGF;
	      int game = positionSGF [pos].game;
	      while ((proGame [positionSGF [pos].game] [positionSGF [pos].move].inter == 361) || validationGame [game] || (pos >= nbPositionsSGF - nbExamples)) {
		pos = (int) (((float)rand () / (RAND_MAX + 1.0)) * nbPositionsSGF);
		//pos = rand () % nbPositionsSGF;
		game = positionSGF [pos].game;
	      }
	      indexValidation [i] = pos;
	      validationGame [game] = true;
	    }
	    fp = fopen ("validation.data", "w");
	    fprintf (fp, "%d ", nbExamples);
	    for (ssize_t i = 0; i < nbExamples; i++)
	      fprintf (fp, "%d ", indexValidation [i]);
	    fclose (fp);
	  }
	  else {
	    fprintf (stderr, "loading validation.data\n");
	    fscanf (fp, "%d", &nbExamples);
	    for (ssize_t i = 0; i < nbExamples; i++) {
	      fscanf (fp, "%d", &indexValidation [i]);
	      int game = positionSGF [indexValidation [i]].game;
	      validationGame [game] = true;
	    }
	    fclose (fp);
	  }
	  for (ssize_t i = 0; i < nbExamples; i++) {
	    int pos = indexValidation [i];
	    PositionSGF p = positionSGF [pos];
	    positionSGF [pos] = positionSGF [nbPositionsSGF - 1];
	    positionSGF [nbPositionsSGF - 1] = p;
	    indexValidation [i] = nbPositionsSGF - 1;
	    nbPositionsSGF--;
	  }
	  // remove pass and validation from positionSGF
	  // bug : remove validation games
	  /**/
	  int i = 0;
	  while (i < nbPositionsSGF) {
	    int game = positionSGF [i].game;
	    if ((proGame [positionSGF [i].game] [positionSGF [i].move].inter == 361) 
	      || validationGame [game]) {
	      PositionSGF p = positionSGF [i];
	      positionSGF [i] = positionSGF [nbPositionsSGF - 1];
	      positionSGF [nbPositionsSGF - 1] = p;
	      nbPositionsSGF--;
	    }
	    else {
	      i++;
	    }
	  }
	  /**/
	}
	for (ssize_t i = 0; i < nbExamples; i++) {
	  int pos = indexValidation [i];
	  //fprintf (stderr, "(%d,%d) ", i, pos);
	  b = board;
	  for (int j = 0; j < positionSGF [pos].move; j++)
	    playColor (&b, proGame [positionSGF [pos].game] [j].inter, proGame [positionSGF [pos].game] [j].color);
	  b.komi [White] = komi [positionSGF [pos].game];
	  //fprintf (stderr, "fill the input\n");
	  // fill the input
	  int turn = b.turn;
	  int other = Black;
	  if (b.turn == Black)
	    other = White;
	  int randomSym = 0;
	  randomSym = pos % 8; 
	  //encodeSym (&b, randomSym);
	  encodeSymKomi (&b, randomSym);
	  for (ssize_t j = 0; j < r.shape(1); j++)
	    for (ssize_t k = 0; k < r.shape(2); k++)
	      for (ssize_t l = 0; l < r.shape(3); l++) {
		r (i, j, k, l) = input [j] [k] [l];
	      }
	  // fill the policy
 	  for (ssize_t j = 0; j < pi.shape(1); j++)
	    pi (i, j) = 0.0;
	  int move = proGame [positionSGF [pos].game] [positionSGF [pos].move].inter;
	  move = sym [move] [randomSym];
	  pi (i, move) = 1.0;
	  // fill the value
	  v (i) = proGame [positionSGF [pos].game] [positionSGF [pos].move].val;
	  /*
	  if (winner [positionSGF [pos].game] == 'W')
	    v (i) = 1.0;
	  else
	    v (i) = 0.0;
	  */
	  //fprintf (stderr, "fill the endgame\n");
	  // fill the endgame
	  for (int j = positionSGF [pos].move; j < nbMovesSGFGame [positionSGF [pos].game]; j++) {
	    //fprintf (stderr, "positionSGF [pos].game = %d\n", positionSGF [pos].game);
	    //fprintf (stderr, "nbMovesSGFGame [positionSGF [pos].game] = %d\n", nbMovesSGFGame [positionSGF [pos].game]);
	    //fprintf (stderr, "j = %d\n", j);
	    playColor (&b, proGame [positionSGF [pos].game] [j].inter, proGame [positionSGF [pos].game] [j].color);
	  }
	  for (int j = 0; j < 19; j++)
	    for (int k = 0; k < 19; k++) {
	      if (b.board [interMove [19 * j + k]] == turn) {
		e (i, j, k, 0) = 1.0;
		e (i, j, k, 1) = 0.0;
	      }
	      else if (b.board [interMove [19 * j + k]] == other) {
		e (i, j, k, 0) = 0.0;
		e (i, j, k, 1) = 1.0;
	      }
	      else {
		e (i, j, k, 0) = 0.0;
		e (i, j, k, 1) = 0.0;
	      }
	    }
	}
    });
  m.def("getBatchVal", [](py::array_t<float> x, py::array_t<float> policy,
		       py::array_t<float> value, py::array_t<float> end) {
	if (!loaded) {
	  memcpy (historyBoard [0], board.board, MaxSize);
	  fprintf (stderr, "load games.val.data\n");
	  loadGamesDataVal ("games.val.data");
	  loaded = true;
	}
	auto r = x.mutable_unchecked<4>();
	auto pi = policy.mutable_unchecked<2>();
	auto v = value.mutable_unchecked<1>();
 	auto e = end.mutable_unchecked<4>();
	nbExamples = r.shape (0);
	fprintf (stderr, "r.shape = (%d, %d, %d, %d)\n", r.shape (0), r.shape (1), r.shape (2), r.shape (3));
	fprintf (stderr, "nbExamples = %d\n", nbExamples);
	for (ssize_t i = 0; i < nbExamples; i++) {
	  //fprintf (stderr, "i = %d, ", i);
	  // choose a random state
	  int pos = rand () % nbPositionsSGF;
	  while (proGame [positionSGF [pos].game] [positionSGF [pos].move].inter == 361)
	    pos = rand () % nbPositionsSGF;
	  //fprintf (stderr, "pos = %d, positionSGF [pos].game = %d, positionSGF [pos].move = %d\n",
	  //	   pos, positionSGF [pos].game, positionSGF [pos].move);
	  b = board;
	  for (int j = 0; j < positionSGF [pos].move; j++)
	    playColor (&b, proGame [positionSGF [pos].game] [j].inter, proGame [positionSGF [pos].game] [j].color);
	  b.komi [White] = komi [positionSGF [pos].game];
	  //fprintf (stderr, "fill the input\n");
	  // fill the input
	  int turn = b.turn;
	  int other = Black;
	  if (b.turn == Black)
	    other = White;
	  int randomSym = 0;
	  randomSym = rand () % 8;
	  //encodeSym (&b, randomSym);
	  encodeSymKomi (&b, randomSym);
	  float val = proGame [positionSGF [pos].game] [positionSGF [pos].move].val;
	  if (rand () % 2 == 0)
	    val = 0.0;
	  for (int x = 0; x < 19; x++)
	    for (int y = 0; y < 19; y++)
	      input [x] [y] [2 * Planes + 2] = val;
	  for (ssize_t j = 0; j < r.shape(1); j++)
	    for (ssize_t k = 0; k < r.shape(2); k++)
	      for (ssize_t l = 0; l < r.shape(3); l++) {
		r (i, j, k, l) = input [j] [k] [l];
	      }
	  // fill the policy
 	  for (ssize_t j = 0; j < pi.shape(1); j++)
	    pi (i, j) = 0.0;
	  int move = proGame [positionSGF [pos].game] [positionSGF [pos].move].inter;
	  move = sym [move] [randomSym];
	  pi (i, move) = 1.0;
	  // fill the value
	  v (i) = proGame [positionSGF [pos].game] [positionSGF [pos].move].val;
	  /*
	  if (winner [positionSGF [pos].game] == 'W')
	    v (i) = 1.0;
	  else
	    v (i) = 0.0;
	  */
	  //fprintf (stderr, "fill the endgame\n");
	  // fill the endgame
	  for (int j = positionSGF [pos].move; j < nbMovesSGFGame [positionSGF [pos].game]; j++) {
	    //fprintf (stderr, "positionSGF [pos].game = %d\n", positionSGF [pos].game);
	    //fprintf (stderr, "nbMovesSGFGame [positionSGF [pos].game] = %d\n", nbMovesSGFGame [positionSGF [pos].game]);
	    //fprintf (stderr, "j = %d\n", j);
	    playColor (&b, proGame [positionSGF [pos].game] [j].inter, proGame [positionSGF [pos].game] [j].color);
	  }
	  for (int j = 0; j < 19; j++)
	    for (int k = 0; k < 19; k++) {
	      if (b.board [interMove [19 * j + k]] == turn) {
		e (i, j, k, 0) = 1.0;
		e (i, j, k, 1) = 0.0;
	      }
	      else if (b.board [interMove [19 * j + k]] == other) {
		e (i, j, k, 0) = 0.0;
		e (i, j, k, 1) = 1.0;
	      }
	      else {
		e (i, j, k, 0) = 0.0;
		e (i, j, k, 1) = 0.0;
	      }
	    }
	}
    });
}
