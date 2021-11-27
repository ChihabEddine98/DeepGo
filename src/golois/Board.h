const int White = 0;
const int Black = 1;
const int Empty = 2;
const int Exterieur = 3;

const int Ami = 0;
const int Other = 1;

const int Dx = 19;
const int Dy = 19;

const int MaxLegalMoves = Dx * Dy + 1; // pour passe

const int MaxPlayers = 2;
const int MaxSize = (Dx + 2) * (Dy + 2);
const int MaxIntersections = Dx * Dy;
const int MaxMovesPerGame = 4 * MaxSize;
const int MaxPlayoutLength = MaxMovesPerGame;

#include "Rzone.h"
const int Haut = 0;
const int Bas = 1;
const int Gauche = 2;
const int Droite = 3;

int interMove [MaxSize], moveInter [MaxSize];

unsigned long long HashArray [MaxPlayers] [MaxSize];
unsigned long long HashKo [MaxPlayers] [MaxSize];
unsigned long long HashTurn;

int ownership [MaxSize] [MaxPlayers];

int nbTooLong = 0;

int lengthMax = 0;

int MaxGroups = 0;

unsigned long long nbPlay = 0;

timeval stop, start;
unsigned long long previousTime = 0;

bool initH = true;

void initHash () {
  for (int player = 0; player < MaxPlayers; player++)
    for (int inter = 0; inter < MaxSize; inter++) {
      HashArray [player] [inter] = 0;
      for (int j = 0; j < 64; j++)
	if ((rand () / (RAND_MAX + 1.0)) > 0.5)
	  HashArray [player] [inter] |= (1ULL << j);
      HashKo [player] [inter] = 0;
      for (int j = 0; j < 64; j++)
	if ((rand () / (RAND_MAX + 1.0)) > 0.5)
	  HashKo [player] [inter] |= (1ULL << j);
    }
  HashTurn = 0;
  for (int j = 0; j < 64; j++)
    if ((rand () / (RAND_MAX + 1.0)) > 0.5)
      HashTurn |= (1ULL << j);
}

static bool ajoute (int * stack, int elt) {
  for (int i = 1; i <= stack [0]; i++)
    if (stack [i] == elt)
      return false;
  stack [0]++;
  stack [stack [0]] = elt;
  return true;
}

void printVertex (FILE *fp, int m) {
  int x = m % Dx;
  if (x > 7)
    x++;
  char c = 'A' + x;
  fprintf (fp, "%c%d", c, 19 - m / Dx);
}

void printMove (FILE *fp, int m) {
  printVertex (fp, m);
}

std::string getVertex (int m) {
  int x = m % Dx;
  if (x > 7)
    x++;
  char c = 'A' + x;
  char s [1000];
  sprintf (s, "%c%d", c, 19 - m / Dx);
  return std::string (s);
}

const int MaxMoveNumber = 2 * (Dx * Dy + 1) + 1;

class Move {
 public :
  short int inter;
  char color;
  //short int code;
  float val, points;

  int number () {
    int c = 0;
    if (color == White)
      return c * 2 * (Dx * Dy + 1) + inter;
    else
      return c * 2 * (Dx * Dy + 1) + Dx * Dy + 1 + inter;
  }
};

class Board {
 public:
  int start, end, size, delta [8];
  int dxBoard, dyBoard;
  char board [MaxSize];
  
  int NbMovesPlayed;
  int Moves [MaxMovesPerGame];
  int colorMove [MaxMovesPerGame];
  int signature [MaxMovesPerGame];
  int signature8 [MaxMovesPerGame];
  unsigned long long HashHistory [MaxMovesPerGame];
  char winner;

  float komi [MaxPlayers], scorePlayer [MaxPlayers];
  bool handicap;
  bool superKo;
  int ko, nbCaptured;
  int passe, nbPasse;
  unsigned long long hash;

  int nbVides, vides [MaxSize], indiceVide [MaxSize];
  int nbChaines, chaines [MaxSize], indiceChaine [MaxSize];

  int nbPierres [MaxSize];
  int premierePierre [MaxSize];
  int pierreSuivante [MaxSize];

  int nbPseudoLibertes [MaxSize];
  int premierePseudoLiberte [MaxSize];
  int pseudoLiberteSuivante [4 * MaxSize];
  int pseudoLibertePrecedente [4 * MaxSize];

  int turn;

  Move rollout [MaxPlayoutLength];
  int classRollout [MaxPlayoutLength];
  float val [MaxPlayoutLength];
  float points [MaxPlayoutLength];
  int startGame;
  int length;

  Board () {
    if (initH) {
      initH = false;
      initHash ();
      init ();
    }
  }

  void init () {
    dxBoard = Dx + 2;
    dyBoard = Dy + 2;
    start = dxBoard + 1;
    end = dxBoard * dyBoard - dxBoard - 1;
    size = Dx * Dy;
    passe = end;
    superKo = false;
    handicap = false;
    ko = 0;
    nbPasse = 0;
    NbMovesPlayed = 0;
    nbVides = 0;
    nbChaines = 0;
    hash = 0;
    startGame = 0;
    for (int i = 0; i < dxBoard * dyBoard; i++) {
      if ((i < start) || (i % dxBoard == 0) || ((i + 1) % dxBoard == 0) || (i >= end)) 
	board [i] = Exterieur;
      else {
	board [i] = Empty;
	vides [nbVides] = i;
	indiceVide [i] = nbVides;
	interMove [nbVides] = i;
	moveInter [i] = nbVides;
	nbVides++;
      }
    }
    interMove [size] = passe;
    moveInter [passe] = size;   
    komi [Black] = 0;
    scorePlayer [Black] = 0;
    komi [White] = 7.5;
    scorePlayer [White] = 0;
    winner = 'U';
    delta [0] = -dxBoard;
    delta [1] = -1;
    delta [2] = 1;
    delta [3] = dxBoard;
    delta [4] = -dxBoard - 1;
    delta [5] = -dxBoard + 1;
    delta [6] = dxBoard - 1;
    delta [7] = dxBoard + 1;
    turn = Black;
    length = 0;
    nbChanges = 0;
    val [0] = 0.0;
  }
  
  void initSimple () {
    for (int i = 0; i < dxBoard * dyBoard; i++) {
      if ((i < start) || (i % dxBoard == 0) || ((i + 1) % dxBoard == 0) || (i >= end)) 
	board [i] = Exterieur;
      else {
	board [i] = Empty;
      }
    }
    superKo = false;
    handicap = false;
    ko = 0;
    nbPasse = 0;
    NbMovesPlayed = 0;
    hash = 0;
    komi [Black] = 0;
    scorePlayer [Black] = 0;
    komi [White] = 7.5;
    scorePlayer [White] = 0;
    winner = 'U';
    turn = Black;
    length = 0;
    nbChanges = 0;
    val [0] = 0.0;
  }
  
  int legalMoves (int joueur, Move l [MaxLegalMoves]) {
    Move coup;
    int nb = 0;
    coup.color = joueur;
    if (NbMovesPlayed < MaxMovesPerGame - 4) // avoid to long games in the tree
      for (coup.inter = 0; coup.inter < size; coup.inter++)
	if (board [interMove [coup.inter]] == Empty) 
	  if (legalMove (interMove [coup.inter], joueur))
	    if (!oeil (interMove [coup.inter], joueur))
	      if (legalSuperKo (interMove [coup.inter], joueur)) {
		//coup.code = board [interMove [coup.inter] - dxBoard] + 
		//4 * board [interMove [coup.inter] - 1] +
		//16 * board [interMove [coup.inter] + 1] +
		//64 * board [interMove [coup.inter] + dxBoard];
		l [nb] = coup;
		nb++;
	      }
    if (nb == 0) {
      coup.inter = size;
      //coup.code = 0;
      l [nb] = coup;
      nb++;
    }
    return nb;
  }
  
  bool voisineAtari (int inter, int color) {
    int other = opponent (color);
    if (board [inter - 1] == other) 
      if (atari (premierePierre [inter - 1]))
	return true;
    if (board [inter + 1] == other)
      if (atari (premierePierre [inter + 1]))
	return true;
    if (board [inter - dxBoard] == other)
      if (atari (premierePierre [inter - dxBoard]))
	return true;
    if (board [inter + dxBoard] == other)
      if (atari (premierePierre [inter + dxBoard]))
	return true;
    return false;
  }
  
  bool canBeKo (int inter, int color) {
    if (board [inter - 1] == Empty) 
	return false;
    if (board [inter + 1] == Empty) 
	return false;
    if (board [inter - dxBoard] == Empty) 
	return false;
    if (board [inter + dxBoard] == Empty) 
	return false;
    if (board [inter - 1] == color) 
	return false;
    if (board [inter + 1] == color) 
	return false;
    if (board [inter - dxBoard] == color) 
	return false;
    if (board [inter + dxBoard] == color) 
	return false;
    return true;
  }
  
  bool legalSuperKo (int inter, int Color) {
    if (voisineAtari (inter, Color)) {
      unsigned long long h = hashIfPlay (inter, Color);
      for (int i = NbMovesPlayed - 1; ((i >= 0) && (i > NbMovesPlayed - 30)); i--)
	if (HashHistory [i] == h)
	  return false;
    }
    return true;
  }

  bool previousHash () {
    for (int i = 0; i < NbMovesPlayed; i++)
      if (hash == HashHistory [i])
	return true;
    return false;
  }

  bool legalMoveSuperKo (int inter, int Color) {
    if (inter >= passe)
      return true;

    /*   if (board [inter] != Empty) { */
    /*     fprintf (stderr, "bug legal\n"); */
    /*     return false; */
    /*   } */
    
    unsigned long long h = hashIfPlay (inter, Color);
    for (int i = NbMovesPlayed - 1; ((i >= 0) && (i > NbMovesPlayed - 30)); i--)
      if (HashHistory [i] == h)
        return false;
    
    if (board [inter - 1] == Empty) return true;
    if (board [inter + 1] == Empty) return true;
    if (board [inter - dxBoard] == Empty) return true;
    if (board [inter + dxBoard] == Empty) return true;
    
    int nb = 0;
    
    if (board [inter - 1] == Color) {
      nb += nbPseudoLibertes [premierePierre [inter - 1]] - 1;
      if (board [inter + 1] == Color) {
	if (premierePierre [inter - 1] != premierePierre [inter + 1])
	  nb += nbPseudoLibertes [premierePierre [inter + 1]] - 1;
	else
	  nb--;
	if (board [inter - dxBoard] == Color) {
	  if ((premierePierre [inter - 1] != premierePierre [inter - dxBoard]) &&
	      (premierePierre [inter + 1] != premierePierre [inter - dxBoard])
	      )
	    nb += nbPseudoLibertes [premierePierre [inter - dxBoard]] - 1;
	  else
	    nb--;
	  if (board [inter + dxBoard] == Color) {
	    if ((premierePierre [inter - 1] != premierePierre [inter + dxBoard]) &&
		(premierePierre [inter + 1] != premierePierre [inter + dxBoard]) &&
		(premierePierre [inter - dxBoard] != premierePierre [inter + dxBoard])
		)
	      nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	    else
	      nb--;
	  }
	}
	else if (board [inter + dxBoard] == Color) {
	  if ((premierePierre [inter - 1] != premierePierre [inter + dxBoard]) &&
	      (premierePierre [inter + 1] != premierePierre [inter + dxBoard]) 
	      )
	    nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	  else
	    nb--;
	} 
      }
      else {
	if (board [inter - dxBoard] == Color) {
	  if ((premierePierre [inter - 1] != premierePierre [inter - dxBoard])
	      )
	    nb += nbPseudoLibertes [premierePierre [inter - dxBoard]] - 1;
	  else
	    nb--;
	  if (board [inter + dxBoard] == Color) {
	    if ((premierePierre [inter - 1] != premierePierre [inter + dxBoard]) &&
		(premierePierre [inter - dxBoard] != premierePierre [inter + dxBoard])
		)
	      nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	    else
	      nb--;
	  }
	}
	else if (board [inter + dxBoard] == Color) {
	  if ((premierePierre [inter - 1] != premierePierre [inter + dxBoard])
	      )
	    nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	  else
	    nb--;
	} 
      }
    }
    else {
      if (board [inter + 1] == Color) {
	nb += nbPseudoLibertes [premierePierre [inter + 1]] - 1;
	if (board [inter - dxBoard] == Color) {
	  if ((premierePierre [inter + 1] != premierePierre [inter - dxBoard])
	      )
	    nb += nbPseudoLibertes [premierePierre [inter - dxBoard]] - 1;
	  else
	    nb--;
	  if (board [inter + dxBoard] == Color) {
	    if ((premierePierre [inter + 1] != premierePierre [inter + dxBoard]) &&
		(premierePierre [inter - dxBoard] != premierePierre [inter + dxBoard])
		)
	      nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	    else
	      nb--;
	  }
	}
	else if (board [inter + dxBoard] == Color) {
	  if ((premierePierre [inter + 1] != premierePierre [inter + dxBoard]) 
	      )
	    nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	  else
	    nb--;
	} 
      }
      else {
	if (board [inter - dxBoard] == Color) {
	  nb += nbPseudoLibertes [premierePierre [inter - dxBoard]] - 1;
	  if (board [inter + dxBoard] == Color) {
	    if ((premierePierre [inter - dxBoard] != premierePierre [inter + dxBoard])
		)
	      nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	    else
	      nb--;
	  }
	}
	else if (board [inter + dxBoard] == Color) {
	  nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	} 
      }
    }
    
    if (nb > 0)
      return true;
    
    if (inter == ko)
      return false;
    
    if ((board [inter - 1] != Color) && (board [inter - 1] != Exterieur)) 
      if (atari (premierePierre [inter - 1]))
	return true;


    if ((board [inter + 1] != Color) && (board [inter + 1] != Exterieur))
      if (atari (premierePierre [inter + 1]))
	return true;
    if ((board [inter - dxBoard] != Color) && (board [inter - dxBoard] != Exterieur))
      if (atari (premierePierre [inter - dxBoard]))
	return true;
    if ((board [inter + dxBoard] != Color) && (board [inter + dxBoard] != Exterieur))
      if (atari (premierePierre [inter + dxBoard]))
	return true;
    
    return false;
  }
  
  bool legalMove (int inter, int Color) {
    if (inter >= passe)
      return true;

    if (board [inter] != Empty)
      return false;
    
    if (superKo) {
      unsigned long long h = hashIfPlay (inter, Color);
      for (int i = NbMovesPlayed - 1; ((i >= 0) && (i > NbMovesPlayed - 10)); i--)
	if (HashHistory [i] == h)
	  return false;
    }
    
    
    if (board [inter - 1] == Empty) return true;
    if (board [inter + 1] == Empty) return true;
    if (board [inter - dxBoard] == Empty) return true;
    if (board [inter + dxBoard] == Empty) return true;
    
    int nb = 0;
    
    if (board [inter - 1] == Color) {
      nb += nbPseudoLibertes [premierePierre [inter - 1]] - 1;
      if (board [inter + 1] == Color) {
	if (premierePierre [inter - 1] != premierePierre [inter + 1])
	  nb += nbPseudoLibertes [premierePierre [inter + 1]] - 1;
	else
	  nb--;
	if (board [inter - dxBoard] == Color) {
	  if ((premierePierre [inter - 1] != premierePierre [inter - dxBoard]) &&
	      (premierePierre [inter + 1] != premierePierre [inter - dxBoard])
	      )
	    nb += nbPseudoLibertes [premierePierre [inter - dxBoard]] - 1;
	  else
	    nb--;
	  if (board [inter + dxBoard] == Color) {
	    if ((premierePierre [inter - 1] != premierePierre [inter + dxBoard]) &&
		(premierePierre [inter + 1] != premierePierre [inter + dxBoard]) &&
		(premierePierre [inter - dxBoard] != premierePierre [inter + dxBoard])
		)
	      nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	    else
	      nb--;
	  }
	}
	else if (board [inter + dxBoard] == Color) {
	  if ((premierePierre [inter - 1] != premierePierre [inter + dxBoard]) &&
	      (premierePierre [inter + 1] != premierePierre [inter + dxBoard]) 
	      )
	    nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	  else
	    nb--;
	} 
      }
      else {
	if (board [inter - dxBoard] == Color) {
	  if ((premierePierre [inter - 1] != premierePierre [inter - dxBoard])
	      )
	    nb += nbPseudoLibertes [premierePierre [inter - dxBoard]] - 1;
	  else
	    nb--;
	  if (board [inter + dxBoard] == Color) {
	    if ((premierePierre [inter - 1] != premierePierre [inter + dxBoard]) &&
		(premierePierre [inter - dxBoard] != premierePierre [inter + dxBoard])
		)
	      nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	    else
	      nb--;
	  }
	}
	else if (board [inter + dxBoard] == Color) {
	  if ((premierePierre [inter - 1] != premierePierre [inter + dxBoard])
	      )
	    nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	  else
	    nb--;
	} 
      }
    }
    else {
      if (board [inter + 1] == Color) {
	nb += nbPseudoLibertes [premierePierre [inter + 1]] - 1;
	if (board [inter - dxBoard] == Color) {
	  if ((premierePierre [inter + 1] != premierePierre [inter - dxBoard])
	      )
	    nb += nbPseudoLibertes [premierePierre [inter - dxBoard]] - 1;
	  else
	    nb--;
	  if (board [inter + dxBoard] == Color) {
	    if ((premierePierre [inter + 1] != premierePierre [inter + dxBoard]) &&
		(premierePierre [inter - dxBoard] != premierePierre [inter + dxBoard])
		)
	      nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	    else
	      nb--;
	  }
	}
	else if (board [inter + dxBoard] == Color) {
	  if ((premierePierre [inter + 1] != premierePierre [inter + dxBoard]) 
	      )
	    nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	  else
	    nb--;
	} 
      }
      else {
	if (board [inter - dxBoard] == Color) {
	  nb += nbPseudoLibertes [premierePierre [inter - dxBoard]] - 1;
	  if (board [inter + dxBoard] == Color) {
	    if ((premierePierre [inter - dxBoard] != premierePierre [inter + dxBoard])
		)
	      nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	    else
	      nb--;
	  }
	}
	else if (board [inter + dxBoard] == Color) {
	  nb += nbPseudoLibertes [premierePierre [inter + dxBoard]] - 1;
	} 
      }
    }
    
    if (nb > 0)
      return true;
    
    if (inter == ko)
      return false;
    
    if ((board [inter - 1] != Color) && (board [inter - 1] != Exterieur)) 
      if (atari (premierePierre [inter - 1]))
	return true;


    if ((board [inter + 1] != Color) && (board [inter + 1] != Exterieur))
      if (atari (premierePierre [inter + 1]))
	return true;
    if ((board [inter - dxBoard] != Color) && (board [inter - dxBoard] != Exterieur))
      if (atari (premierePierre [inter - dxBoard]))
	return true;
    if ((board [inter + dxBoard] != Color) && (board [inter + dxBoard] != Exterieur))
      if (atari (premierePierre [inter + dxBoard]))
	return true;
    
    return false;
  }
  
  void replay (int lg) {
    init ();
    for (int i = 0; i < lg; i++)
      joue (Moves [i], colorMove [i]);
  }
  
  void joue (int inter, int couleur) {
    nbPlay++;
    if (NbMovesPlayed < MaxMovesPerGame - 4) { // avoid to long games in the tree
      HashHistory [NbMovesPlayed] = hash;
      Moves [NbMovesPlayed] = inter;
      colorMove [NbMovesPlayed] = couleur;
    }
    nbCaptured = 0;
    ko = 0;
    
    // switch turn
    // pb handicap, black can play multiple moves in a row
    // comment since positional superko
    //hash ^= HashArray [Black] [0];

    //print (stderr);
    //fprintf (stderr, "joue (%d, %d)\n", inter, couleur);
    //check ();
    
    //if (inter == 67)
    //fprintf (stderr, "bug GoBoardMC::joue\n");
    
    // move = size is pass
    if (inter < passe) {
      nbPasse = 0;
      int pile [5];
      posePierre (inter, couleur, pile);
      if (pile [0] > 0) {
	enleveChaines (pile);
	if (nbCaptured == 1)
	  if (nbPierres [premierePierre [inter]] == 1)
	    ko = vides [nbVides - 1];
      }
    }
    else
      nbPasse++;

    //if (couleur == Black)
    //turn = White;
    //else
    //turn = Black;
    if (nbChaines > MaxGroups)
      MaxGroups = nbChaines;
    
    NbMovesPlayed++;
  }
  
  void enleveChaines (int pile [5]) {
    for (int i = 1; i <= pile [0]; i++) 
      if (board [pile [i]] != Empty)
	enleveChaine (pile [i]);
  }
  
  void ajouteChaine (int chaine1, int chaine2) {
    int pierre = chaine1;
    
    do {
      premierePierre [pierre] = chaine2;
      pierre = pierreSuivante [pierre];
    } 
    while (pierre != chaine1);
    
    int suivante = pierreSuivante [chaine1];
    pierreSuivante [chaine1] = pierreSuivante [chaine2];
    pierreSuivante [chaine2] = suivante;
    
    nbPierres [chaine2] += nbPierres [chaine1];
    
    chaines [indiceChaine [chaine1]] = chaines [nbChaines - 1];
    indiceChaine [chaines [nbChaines - 1]] = indiceChaine [chaine1];
    nbChaines--;
    
    if (nbPseudoLibertes [chaine1] > 0) {
      if (nbPseudoLibertes [chaine2] == 0) {
	premierePseudoLiberte [chaine2] = premierePseudoLiberte [chaine1];
      }
      else {
	int premiereChaine2 = premierePseudoLiberte [chaine2];
	int suivanteChaine2 = pseudoLiberteSuivante [premiereChaine2];
	int premiereChaine1 = premierePseudoLiberte [chaine1];
	int derniereChaine1 = pseudoLibertePrecedente [premiereChaine1];
	pseudoLiberteSuivante [premiereChaine2] = premiereChaine1;
	pseudoLibertePrecedente [premiereChaine1] = premiereChaine2;
	pseudoLiberteSuivante [derniereChaine1] = suivanteChaine2;
	pseudoLibertePrecedente [suivanteChaine2] = derniereChaine1;
      }
    }
    nbPseudoLibertes [chaine2] += nbPseudoLibertes [chaine1];
  }
  
  void otePseudoLiberte (int chaine, int lib) {
    nbPseudoLibertes [chaine]--;
    if (nbPseudoLibertes [chaine] > 0) {
      int precedente = pseudoLibertePrecedente [lib];
      int suivante = pseudoLiberteSuivante [lib];
      pseudoLiberteSuivante [precedente] = suivante;
      pseudoLibertePrecedente [suivante] = precedente;
      if (premierePseudoLiberte [chaine] == lib)
	premierePseudoLiberte [chaine] = suivante;
    }
  }
  
  void ajoutePseudoLiberte (int chaine, int lib) {
    if (nbPseudoLibertes [chaine] == 0) {
      premierePseudoLiberte [chaine] = lib;
      pseudoLiberteSuivante [lib] = lib;
      pseudoLibertePrecedente [lib] = lib;
    }
    else {
      int premiere = premierePseudoLiberte [chaine];
      int suivante = pseudoLiberteSuivante [premiere];
      pseudoLiberteSuivante [premiere] = lib;
      pseudoLibertePrecedente [lib] = premiere;
      pseudoLiberteSuivante [lib] = suivante;
      pseudoLibertePrecedente [suivante] = lib;
    }
    nbPseudoLibertes [chaine]++;
  }
  
  int ligne (int inter) {
    int x = inter % dxBoard;
    int y = inter / dxBoard;
    if (x < dxBoard - 1 - x)
      if (y < dyBoard - 1 - y)
        if (x < y)
          return x;
        else
          return y;
      else
        if (x < dyBoard - 1 - y)
          return x;
        else
          return dyBoard - 1 - y;
    else
      if (y < dyBoard - 1 - y)
        if (dxBoard - 1 - x < y)
          return dxBoard - 1 - x;
        else
          return y;
      else
        if (dxBoard -1 - x < dyBoard -1 - y)
          return dxBoard -1 - x;
        else
          return dyBoard -1 - y;
  }

  int secondeLigne (int inter) {
    int x = inter % dxBoard;
    int y = inter / dxBoard;
    if (x < dxBoard - x)
      if (y < dyBoard - y)
        if (x < y)
          return y;
        else
          return x;
      else
        if (x < dyBoard - y)
          return dyBoard - y;
        else
          return x;
    else
      if (y < dyBoard - y)
        if (dxBoard - x < y)
          return y;
        else
          return dxBoard - x;
      else
        if (dxBoard - x < dyBoard - y)
          return dyBoard - y;
        else
          return dxBoard - x;
  }

  bool symmetric () {
    for (int x = 0; x < 19; x++)
      for (int y = 0; y < 19; y++)
	if ((x != 9) || (y != 9)) {
	  if (board [interMove [x + 19 * y]] == Black)
	    if (board [interMove [18 - x + 19 * (18 - y)]] != White)
	      return false;
	  if (board [interMove [x + 19 * y]] == White)
	    if (board [interMove [18 - x + 19 * (18 - y)]] != Black)
	      return false;
	}
    return true;
  }

  void posePierre (int inter, int couleur, int pile [5]) {
    board [inter] = couleur;
    hash ^= HashArray [couleur] [inter];
    pile [0] = 0;
        
    nbVides--;
    indiceVide [vides [nbVides]] = indiceVide [inter];
    vides [indiceVide [inter]] = vides [nbVides];
    
    nbPierres [inter] = 1;
    premierePierre [inter] = inter;
    pierreSuivante [inter] = inter;
    nbPseudoLibertes [inter] = 0;
    
    indiceChaine [inter] = nbChaines;
    chaines [nbChaines] = inter;
    nbChaines++;
    
    if (board [inter - 1] == couleur) {
      if (premierePierre [inter] != premierePierre [inter - 1])
	ajouteChaine (premierePierre [inter], premierePierre [inter - 1]);
      otePseudoLiberte (premierePierre [inter], (inter << 2) | Gauche);
    }
    else if (board [inter - 1] == Empty) {
      ajoutePseudoLiberte (premierePierre [inter], ((inter - 1) << 2) | Droite);
    }
    else if (board [inter - 1] != Exterieur) {
      otePseudoLiberte (premierePierre [inter - 1], (inter << 2) | Gauche);
      if (nbPseudoLibertes [premierePierre [inter - 1]] == 0) {
	pile [0]++;
	pile [pile [0]] = premierePierre [inter - 1];
      }
    }
    
    if (board [inter + 1] == couleur) {
      if (premierePierre [inter] != premierePierre [inter + 1])
	ajouteChaine (premierePierre [inter], premierePierre [inter + 1]);
      otePseudoLiberte (premierePierre [inter], (inter << 2) | Droite);
    }
    else if (board [inter + 1] == Empty) {
      ajoutePseudoLiberte (premierePierre [inter], ((inter + 1)<< 2) | Gauche);
    }
    else if (board [inter + 1] != Exterieur) {
      otePseudoLiberte (premierePierre [inter + 1], (inter << 2) | Droite);
      if (nbPseudoLibertes [premierePierre [inter + 1]] == 0) {
	pile [0]++;
	pile [pile [0]] = premierePierre [inter + 1];
      }
    }
    
    if (board [inter - dxBoard] == couleur) {
      if (premierePierre [inter] != premierePierre [inter - dxBoard])
	ajouteChaine (premierePierre [inter], premierePierre [inter - dxBoard]);
      otePseudoLiberte (premierePierre [inter], (inter << 2) | Haut);
    }
    else if (board [inter - dxBoard] == Empty) {
      ajoutePseudoLiberte (premierePierre [inter], ((inter - dxBoard) << 2) | Bas);
    }
    else if (board [inter - dxBoard] != Exterieur) {
      otePseudoLiberte (premierePierre [inter - dxBoard], (inter << 2) | Haut);
      if (nbPseudoLibertes [premierePierre [inter - dxBoard]] == 0) {
	pile [0]++;
	pile [pile [0]] = premierePierre [inter - dxBoard];
      }
    }
    
    if (board [inter + dxBoard] == couleur) {
      if (premierePierre [inter] != premierePierre [inter + dxBoard])
	ajouteChaine (premierePierre [inter], premierePierre [inter + dxBoard]);
      otePseudoLiberte (premierePierre [inter], (inter << 2) | Bas);
    }
    else if (board [inter + dxBoard] == Empty) {
      ajoutePseudoLiberte (premierePierre [inter], ((inter + dxBoard) << 2) | Haut);
    }
    else if (board [inter + dxBoard] != Exterieur) {
      otePseudoLiberte (premierePierre [inter + dxBoard], (inter << 2) | Bas);
      if (nbPseudoLibertes [premierePierre [inter + dxBoard]] == 0) {
	pile [0]++;
	pile [pile [0]] = premierePierre [inter + dxBoard];
      }
    }
    
  }
  
  void enleveChaine (int chaine) {
    int inter = chaine, couleur = board [chaine];
    
    chaines [indiceChaine [chaine]] = chaines [nbChaines - 1];
    indiceChaine [chaines [nbChaines - 1]] = indiceChaine [chaine];
    nbChaines--;
  
    do {
      board [inter] = Empty;
      indiceVide [inter] = nbVides;
      vides [nbVides] = inter;
      nbVides++;
      nbCaptured++;
      
      hash ^= HashArray [couleur] [inter];
      
      if (board [inter - 1] != Empty)
	if (board [inter - 1] != Exterieur)
	  if (board [inter - 1] != couleur)
	    ajoutePseudoLiberte (premierePierre [inter - 1], (inter << 2) | Gauche);
      if (board [inter + 1] != Empty)
	if (board [inter + 1] != Exterieur)
	  if (board [inter + 1] != couleur)
	    ajoutePseudoLiberte (premierePierre [inter + 1], (inter << 2) | Droite);
      if (board [inter - dxBoard] != Empty)
	if (board [inter - dxBoard] != Exterieur)
	  if (board [inter - dxBoard] != couleur)
	    ajoutePseudoLiberte (premierePierre [inter - dxBoard], (inter << 2) | Haut);
      if (board [inter + dxBoard] != Empty)
	if (board [inter + dxBoard] != Exterieur)
	  if (board [inter + dxBoard] != couleur)
	    ajoutePseudoLiberte (premierePierre [inter + dxBoard], (inter << 2) | Bas);
      
      inter = pierreSuivante [inter];
    } 
    while (inter != chaine);
  }
  
  unsigned long long hashIfPlay (int inter, int color) {
    Board b = *this;
    b.joue (inter, color);
    return b.hash;
    /* unsigned long long h = hash; */
    
    /* h ^= HashArray [color] [inter]; */
    
    /* int stack [5]; */
    /* stack [0] = 0; */
    /* int other = opponent (color); */
    /* if (board [inter - 1] == other) */
    /*   ajoute (stack, premierePierre [inter - 1]); */
    /* if (board [inter - dxBoard] == other) */
    /*   ajoute (stack, premierePierre [inter - dxBoard]); */
    /* if (board [inter + 1] == other) */
    /*   ajoute (stack, premierePierre [inter + 1]); */
    /* if (board [inter + dxBoard] == other) */
    /*   ajoute (stack, premierePierre [inter + dxBoard]); */
    
    /* for (int i = 1; i <= stack [0]; i++) { */
    /*   int inter = stack [i]; */
    /*   do { */
    /* 	h ^= HashArray [other] [inter]; */
    /* 	inter = pierreSuivante [inter]; */
    /*   }  */
    /*   while (inter != stack [i]); */
    /* } */
    
    /* return h; */
  }
  
  bool oeil (int inter, int couleur) {
    if (!entoure (inter, couleur))
      return false;
    
    int other = opponent (couleur);
    int nbDiagOther = 0;
    for (int i = 4; i < 8; i++)
      if (board [inter + delta [i]] == other)
	nbDiagOther++;
    
    bool bord = false;
    if ((board [inter - dxBoard] == Exterieur) ||
	(board [inter + dxBoard] == Exterieur) ||
	(board [inter - 1] == Exterieur) ||
	(board [inter + 1] == Exterieur))
      bord = true;
    
    if (bord && (nbDiagOther > 0))
      return false;
    if (!bord && (nbDiagOther > 1))
      return false;
    
    int nbDiagNotProtected = 0;
    for (int i = 4; i < 8; i++)
      if (board [inter + delta [i]] == Empty)
	if (!protegee (inter + delta [i], couleur)) {
	  nbDiagNotProtected++;
	  if (bord) {
	    if (nbDiagOther + nbDiagNotProtected > 0)
	      return false;
	  }
	  else {
	    if (nbDiagOther + (1 + nbDiagNotProtected) / 2 > 1)
	      return false;
	  }
	}
    
    return true;
  }
  
  bool entoure (int inter, int couleur) {
    // test if all neighbors are of the same color
    if ((board [inter - 1] != couleur) && (board [inter - 1] != Exterieur)) 
      return false;
    if ((board [inter + 1] != couleur) && (board [inter + 1] != Exterieur))
      return false;
    if ((board [inter - dxBoard] != couleur) && (board [inter - dxBoard] != Exterieur))
      return false;
    if ((board [inter + dxBoard] != couleur) && (board [inter + dxBoard] != Exterieur))
      return false;
    if (board [inter - 1] == couleur)
      if (atari (inter - 1))
	return false;
    if (board [inter + 1] == couleur)
      if (atari (inter + 1))
	return false;
    if (board [inter - dxBoard] == couleur)
      if (atari (inter - dxBoard))
	return false;
    if (board [inter + dxBoard] == couleur)
      if (atari (inter + dxBoard))
	return false;
    return true;
  }
  
  bool isCapture (int inter, int couleur) {
    int other = opponent (couleur);
    if (board [inter - 1] == other) 
      if (atari (inter - 1))
	return true;
    if (board [inter - dxBoard] == other) 
      if (atari (inter - dxBoard))
	return true;
    if (board [inter + 1] == other) 
      if (atari (inter + 1))
	return true;
    if (board [inter + dxBoard] == other) 
      if (atari (inter + dxBoard))
	return true;
    return false;
  }
  
  bool capturePlusieursPierres (int inter, int couleur) {
    int nb = 0;
    int other = opponent (couleur);
    if (board [inter - 1] == other) 
      if (atari (inter - 1))
	nb += nbPierres [premierePierre [inter - 1]];
    if (board [inter - dxBoard] == other) 
      if (atari (inter - dxBoard))
	nb += nbPierres [premierePierre [inter - dxBoard]];
    if (board [inter + 1] == other) 
      if (atari (inter + 1))
	nb += nbPierres [premierePierre [inter + 1]];
    if (board [inter + dxBoard] == other) 
      if (atari (inter + dxBoard))
	nb += nbPierres [premierePierre [inter + dxBoard]];
    return nb > 1;
  }
  
  int nbStonesCaptured (int inter, int couleur) {
    int nb = 0;
    int other = opponent (couleur);
    if (board [inter - 1] == other) 
      if (atari (inter - 1))
	nb += nbPierres [premierePierre [inter - 1]];
    if (board [inter - dxBoard] == other) 
      if (atari (inter - dxBoard))
	nb += nbPierres [premierePierre [inter - dxBoard]];
    if (board [inter + 1] == other) 
      if (atari (inter + 1))
	nb += nbPierres [premierePierre [inter + 1]];
    if (board [inter + dxBoard] == other) 
      if (atari (inter + dxBoard))
	nb += nbPierres [premierePierre [inter + dxBoard]];
    return nb;
  }
  
  int nbPierresSiJoue (int inter, int couleur) {
    int nb = 1, stack [5];
    stack [0] = 0;
    if (board [inter - 1] == couleur)
      ajoute (stack, premierePierre [inter - 1]);
    if (board [inter - dxBoard] == couleur)
      ajoute (stack, premierePierre [inter - dxBoard]);
    if (board [inter + 1] == couleur)
      ajoute (stack, premierePierre [inter + 1]);
    if (board [inter + dxBoard] == couleur)
      ajoute (stack, premierePierre [inter + dxBoard]);
    
    for (int i = 1; i <= stack [0]; i++)
      nb += nbPierres [stack [i]];
    return nb;
  }
  
  bool save (int inter, int couleur) {
    if (board [inter - 1] == couleur) 
      if (atari (inter - 1))
	return true;
    if (board [inter - dxBoard] == couleur) 
      if (atari (inter - dxBoard))
	return true;
    if (board [inter + 1] == couleur)
      if (atari (inter + 1))
	return true;
    if (board [inter + dxBoard] == couleur) 
      if (atari (inter + dxBoard))
	return true;
    return false;
  }
  
  bool atariVoisine (int inter, int couleur) {
    int other = opponent (couleur);
    if (board [inter - 1] == other) 
      if (deuxLibertes (inter - 1))
	return true;
    if (board [inter - dxBoard] == other) 
      if (deuxLibertes (inter - dxBoard))
	return true;
    if (board [inter + 1] == other) 
      if (deuxLibertes (inter + 1))
	return true;
    if (board [inter + dxBoard] == other) 
      if (deuxLibertes (inter + dxBoard))
	return true;
    return false;
  }
  
  bool deuxLibertes (int p) {
    int premiere = premierePseudoLiberte [premierePierre [p]];
    int lib = premiere >> 2, lib1 = -1;
    int suivante = pseudoLiberteSuivante [premiere];
    
    while (suivante != premiere) {
      if ((suivante >> 2) != lib) {
	if (lib1 == -1)
	  lib1 = suivante >> 2;
	else if ((suivante >> 2) != lib1)
	  return false;
      }
      suivante = pseudoLiberteSuivante [suivante];
    }
    
    return true;
  }
  
  bool deuxLibertes (int p, int & lib, int & lib1) {
    int premiere = premierePseudoLiberte [premierePierre [p]];
    int suivante = pseudoLiberteSuivante [premiere];
    
    lib = premiere >> 2;
    lib1 = -1;
    while (suivante != premiere) {
      if ((suivante >> 2) != lib) {
	if (lib1 == -1)
	  lib1 = suivante >> 2;
	else if ((suivante >> 2) != lib1)
	  return false;
      }
      suivante = pseudoLiberteSuivante [suivante];
    }
    
    return true;
  }
  
  int nbEmpty (int inter) {
    int nb = 0;
    
    if (board [inter - 1] == Empty)
      nb++;
    if (board [inter + 1] == Empty)
      nb++;
    if (board [inter - dxBoard] == Empty)
      nb++;
    if (board [inter + dxBoard] == Empty)
      nb++;
    return nb;
  }
    
  int libSiJoue (int inter, int couleur, int * stack) {
    stack [0] = 0;
    
    if (board [inter - 1] == Empty) {
      stack [0]++;
      stack [stack [0]] = inter - 1;
    }
    if (board [inter + 1] == Empty) {
      stack [0]++;
      stack [stack [0]] = inter + 1;
    }
    if (board [inter - dxBoard] == Empty) {
      stack [0]++;
      stack [stack [0]] = inter - dxBoard;
    }
    if (board [inter + dxBoard] == Empty)  {
      stack [0]++;
      stack [stack [0]] = inter + dxBoard;
    }
    
    if (board [inter - 1] == couleur) {
      int premiere = premierePseudoLiberte [premierePierre [inter - 1]];
      if ((premiere >> 2) != inter)
	ajoute (stack, premiere >> 2);
      int suivante = pseudoLiberteSuivante [premiere];
      while (suivante != premiere) {
	if ((suivante >> 2) != inter)
	  ajoute (stack, suivante >> 2);
	suivante = pseudoLiberteSuivante [suivante];
      }
    }
    if (board [inter + 1] == couleur) {
      int premiere = premierePseudoLiberte [premierePierre [inter + 1]];
      if ((premiere >> 2) != inter)
	ajoute (stack, premiere >> 2);
      int suivante = pseudoLiberteSuivante [premiere];
      while (suivante != premiere) {
	if ((suivante >> 2) != inter)
	  ajoute (stack, suivante >> 2);
	suivante = pseudoLiberteSuivante [suivante];
      }
    }
    if (board [inter - dxBoard] == couleur) {
      int premiere = premierePseudoLiberte [premierePierre [inter - dxBoard]];
      if ((premiere >> 2) != inter)
	ajoute (stack, premiere >> 2);
      int suivante = pseudoLiberteSuivante [premiere];
      while (suivante != premiere) {
	if ((suivante >> 2) != inter)
	  ajoute (stack, suivante >> 2);
	suivante = pseudoLiberteSuivante [suivante];
      }
    }
    if (board [inter + dxBoard] == couleur)  {
      int premiere = premierePseudoLiberte [premierePierre [inter + dxBoard]];
      if ((premiere >> 2) != inter)
	ajoute (stack, premiere >> 2);
      int suivante = pseudoLiberteSuivante [premiere];
      while (suivante != premiere) {
	if ((suivante >> 2) != inter)
	  ajoute (stack, suivante >> 2);
	suivante = pseudoLiberteSuivante [suivante];
      }
    }
    
    int other = opponent (couleur);
    if (board [inter - 1] == other) 
      if (atari (inter - 1)) {
	ajoute (stack, inter - 1);
      }
    if (board [inter + 1] == other)
      if (atari (inter + 1)) {
	ajoute (stack, inter + 1);
      }
    if (board [inter - dxBoard] == other)
      if (atari (inter - dxBoard)) {
	ajoute (stack, inter - dxBoard);
      }
    if (board [inter + dxBoard] == other)
      if (atari (inter + dxBoard)) {
	ajoute (stack, inter + dxBoard);
      }
    return stack [0];
  }
  
  int nbLibertes (int inter, int * stack) {
    stack [0] = 0;
    int premiere = premierePseudoLiberte [premierePierre [inter]];
    ajoute (stack, premiere >> 2);
    int suivante = pseudoLiberteSuivante [premiere];
    while (suivante != premiere) {
      ajoute (stack, suivante >> 2);
      suivante = pseudoLiberteSuivante [suivante];
    }
    return stack [0];
  }
  
  int nbLiberties (int inter, Rzone & liberties, Rzone & stones, int maxi = 361) {
    stones.add (inter);
    if (board [inter - dxBoard] == Empty) {
      if (!liberties.element (inter - dxBoard)) {
	liberties.add (inter - dxBoard);
	if (liberties.size () >= maxi)
	  return liberties.size ();
      }
    }
    if (board [inter - 1] == Empty) {
      if (!liberties.element (inter - 1)) {
	liberties.add (inter - 1);
	if (liberties.size () >= maxi)
	  return liberties.size ();
      }
    }
    if (board [inter + 1] == Empty) {
      if (!liberties.element (inter + 1)) {
	liberties.add (inter + 1);
	if (liberties.size () >= maxi)
	  return liberties.size ();
      }
    }
    if (board [inter + dxBoard] == Empty) {
      if (!liberties.element (inter + dxBoard)) {
	liberties.add (inter + dxBoard);
	if (liberties.size () >= maxi)
	  return liberties.size ();
      }
    }
    if (board [inter - dxBoard] == board [inter]) {
      if (!stones.element (inter - dxBoard)) {
	nbLiberties (inter - dxBoard, liberties, stones, maxi);
	if (liberties.size () >= maxi)
	  return liberties.size ();
      }
    }
    if (board [inter - 1] == board [inter]) {
      if (!stones.element (inter - 1)) {
	nbLiberties (inter - 1, liberties, stones, maxi);
	if (liberties.size () >= maxi)
	  return liberties.size ();
      }
    }
    if (board [inter + 1] == board [inter]) {
      if (!stones.element (inter + 1)) {
	nbLiberties (inter + 1, liberties, stones, maxi);
	if (liberties.size () >= maxi)
	  return liberties.size ();
      }
    }
    if (board [inter + dxBoard] == board [inter]) {
      if (!stones.element (inter + dxBoard)) {
	nbLiberties (inter + dxBoard, liberties, stones, maxi);
      }
    }
    return liberties.size ();
  }
    
  bool zeroLiberties (int inter, Rzone & stones) {
    if (board [inter - dxBoard] == Empty)
      return false;
    if (board [inter - 1] == Empty)
      return false;
    if (board [inter + 1] == Empty)
      return false;
    if (board [inter + dxBoard] == Empty)
      return false;
    stones.add (inter);
    if (board [inter - dxBoard] == board [inter])
      if (!stones.element (inter - dxBoard))
	if (!zeroLiberties (inter - dxBoard, stones))
	  return false;
    if (board [inter - 1] == board [inter])
      if (!stones.element (inter - 1))
	if (!zeroLiberties (inter - 1, stones))
	  return false;
    if (board [inter + 1] == board [inter])
      if (!stones.element (inter + 1))
	if (!zeroLiberties (inter + 1, stones))
	  return false;
    if (board [inter + dxBoard] == board [inter])
      if (!stones.element (inter + dxBoard))
	if (!zeroLiberties (inter + dxBoard, stones))
	  return false;
    return true;
  }
    
  bool zeroLiberties (int inter) {
    if (board [inter - dxBoard] == Empty)
      return false;
    if (board [inter - 1] == Empty)
      return false;
    if (board [inter + 1] == Empty)
      return false;
    if (board [inter + dxBoard] == Empty)
      return false;
    Rzone stones;
    stones.add (inter);
    if (board [inter - dxBoard] == board [inter])
      if (!stones.element (inter - dxBoard))
	if (!zeroLiberties (inter - dxBoard, stones))
	  return false;
    if (board [inter - 1] == board [inter])
      if (!stones.element (inter - 1))
	if (!zeroLiberties (inter - 1, stones))
	  return false;
    if (board [inter + 1] == board [inter])
      if (!stones.element (inter + 1))
	if (!zeroLiberties (inter + 1, stones))
	  return false;
    if (board [inter + dxBoard] == board [inter])
      if (!stones.element (inter + dxBoard))
	if (!zeroLiberties (inter + dxBoard, stones))
	  return false;
    return true;
  }
    
  int nbLibertiesIfPlay (int inter, int color) {
    if (board [inter] != Empty)
      return -1;
    if (legal (inter, color)) {
      play (inter, color);
      Rzone liberties, stones;
      int n = nbLiberties (inter, liberties, stones);
      undo ();
      return n;
    }
    return 0;
  }

  void markStones (int inter, Rzone & stones) {
    stones.add (inter);
    if (board [inter - dxBoard] == board [inter])
      if (!stones.element (inter - dxBoard))
	markStones (inter - dxBoard, stones);
    if (board [inter - 1] == board [inter])
      if (!stones.element (inter - 1))
	markStones (inter - 1, stones);
    if (board [inter + 1] == board [inter])
      if (!stones.element (inter + 1))
	markStones (inter + 1, stones);
    if (board [inter + dxBoard] == board [inter])
      if (!stones.element (inter + dxBoard))
	markStones (inter + dxBoard, stones);
  }

  void orderTwoLiberties (Rzone & liberties, Rzone & order2) {
    allRzone (i, liberties, l) {
      for (int j = 0; j < 4; j++)
      if (board [l + delta [j]] == Empty)
	if (!liberties.element (l + delta [j]))
	  order2.add (l + delta [j]);
    }
  }

  int nbChanges;
  int changes [10000];

  bool isAlone (int inter) {
    if (board [inter - dxBoard] == board [inter])
      return false;
    if (board [inter - 1] == board [inter])
      return false;
    if (board [inter + dxBoard] == board [inter])
      return false;
    if (board [inter + 1] == board [inter])
      return false;
    return true;
  }

  int getKo () {
    if (nbChanges > 4)
      if (changes [nbChanges - 1] == 4) // prise d'une seule pierre
	if (board [changes [nbChanges - 3]] == Empty) // pierre prise = inter
	  if (isAlone (changes [nbChanges - 5])) { // prise avec une seule pierre
	    //fprintf (stderr, "ko : %d %d\n", changes [nbChanges - 3], changes [nbChanges - 5]);
	    return changes [nbChanges - 3];
	  }
    return 0;
  }

  bool legal (int inter, int color) {
    if (board [inter] != Empty)
      return false;

    /**/
    if (board [inter - dxBoard] == Empty)
      return true;
    if (board [inter - 1] == Empty)
      return true;
    if (board [inter + 1] == Empty)
      return true;
    if (board [inter + dxBoard] == Empty)
      return true;
    /**/

    board [inter] = color;
    Rzone stones;
    bool zero = zeroLiberties (inter, stones);
    board [inter] = Empty;
    if (!zero)
      return true;

    // + tester aussi si prise avec une seule pierre
    if (nbChanges > 4)
      if (changes [nbChanges - 1] == 4) // prise d'une seule pierre
	if (changes [nbChanges - 3] == inter) // pierre prise = inter
	  if (isAlone (changes [nbChanges - 5])) { // prise avec une seule pierre
	    //fprintf (stderr, "ko : %d %d\n", changes [nbChanges - 3], changes [nbChanges - 5]);
	    return false;
	  }

    int other = opponent (color);
    if (board [inter - dxBoard] == other) {
      Rzone liberties, stones;
      int n = nbLiberties (inter - dxBoard, liberties, stones, 2);
      if (n == 1)
	return true;
    }
    if (board [inter - 1] == other) {
      Rzone liberties, stones;
      int n = nbLiberties (inter - 1, liberties, stones, 2);
      if (n == 1)
	return true;
    }
    if (board [inter + 1] == other) {
      Rzone liberties, stones;
      int n = nbLiberties (inter + 1, liberties, stones, 2);
      if (n == 1)
	return true;
    }
    if (board [inter + dxBoard] == other) {
      Rzone liberties, stones;
      int n = nbLiberties (inter + dxBoard, liberties, stones, 2);
      if (n == 1)
	return true;
    }
    return false;
  }

  Rzone stonesLadder, libertiesLadder;
  int nLadder;
  void initLadder (int inter) {
    libertiesLadder.init ();
    stonesLadder.init ();
    nLadder = nbLiberties (inter, libertiesLadder, stonesLadder, 3);
  }

  void play (int inter, int color) {
    nbPlay++;
    if (NbMovesPlayed < MaxMovesPerGame - 4) { // avoid to long games in the tree
      Moves [NbMovesPlayed] = inter;
      colorMove [NbMovesPlayed] = color;
    }
    NbMovesPlayed++;
    int nb = nbChanges;
    int other = opponent (color);
    if (inter != passe) {
      board [inter] = color;
      hash ^= HashArray [color] [inter];
      changes [nbChanges] = inter;
      nbChanges++;
      changes [nbChanges] = Empty;
      nbChanges++;
      if (board [inter - dxBoard] == other) {
	Rzone stones;
	if (zeroLiberties (inter - dxBoard, stones)) 
	  allRzone (i, stones, s) {
	    board [s] = Empty;
	    hash ^= HashArray [other] [s];
	    changes [nbChanges] = s;
	    nbChanges++;
	    changes [nbChanges] = other;
	    nbChanges++;
	  }
      }
      if (board [inter - 1] == other) {
	Rzone stones;
	if (zeroLiberties (inter - 1, stones)) 
	  allRzone (i, stones, s) {
	    board [s] = Empty;
	    hash ^= HashArray [other] [s];
	    changes [nbChanges] = s;
	    nbChanges++;
	    changes [nbChanges] = other;
	    nbChanges++;
	  }
      }
      if (board [inter + 1] == other) {
	Rzone stones;
	if (zeroLiberties (inter + 1, stones)) 
	  allRzone (i, stones, s) {
	    board [s] = Empty;
	    hash ^= HashArray [other] [s];
	    changes [nbChanges] = s;
	    nbChanges++;
	    changes [nbChanges] = other;
	    nbChanges++;
	  }
      }
      if (board [inter + dxBoard] == other) {
	Rzone stones;
	if (zeroLiberties (inter + dxBoard, stones)) 
	  allRzone (i, stones, s) {
	    board [s] = Empty;
	    hash ^= HashArray [other] [s];
	    changes [nbChanges] = s;
	    nbChanges++;
	    changes [nbChanges] = other;
	    nbChanges++;
	  }
      }
    }
    changes [nbChanges] = nbChanges - nb;
    nbChanges++;
    ko = 0;
    if (nbChanges > 4)
      if (changes [nbChanges - 1] == 4) { // prise d'une seule pierre
	int k = changes [nbChanges - 3]; // pierre prise = inter
	if (isAlone (changes [nbChanges - 5])) { // prise avec une seule pierre
	  //fprintf (stderr, "ko : %d %d\n", changes [nbChanges - 3], changes [nbChanges - 5]);
	  ko = k;
	}
      }
  }
  
  void undo () {
    nbChanges--;
    int nb = changes [nbChanges];
    for (int i = 0; i < nb; i += 2) {
      board [changes [nbChanges - i - 2]] = changes [nbChanges - i - 1];
      hash ^= HashArray [changes [nbChanges - i - 1]] [changes [nbChanges - i - 2]];
      //if (debugUndo)
      //fprintf (stderr, "debug undo nb = %d, nbChanges = %d, changes [nbChanges - i - 2]] = %d changes [nbChanges - i - 1] = %d\n", nb, nbChanges, changes [nbChanges - i - 2], changes [nbChanges - i - 1]);
    }
    nbChanges -= nb;
    NbMovesPlayed--;
    ko = 0;
    if (nbChanges > 4)
      if (changes [nbChanges - 1] == 4) { // prise d'une seule pierre
	int k = changes [nbChanges - 3]; // pierre prise = inter
	if (isAlone (changes [nbChanges - 5])) { // prise avec une seule pierre
	  //fprintf (stderr, "ko : %d %d\n", changes [nbChanges - 3], changes [nbChanges - 5]);
	  ko = k;
	}
      }
  }

  bool captureLadder (int inter, Rzone & moves) {
    Rzone liberties, stones;
    int n = nbLiberties (inter, liberties, stones, 3);
    //fprintf (stderr, "liberties %d\n", n);
    if (n > 2)
      return false;
    int other = opponent (board [inter]);
    if (n == 1) {
      allRzone (i, liberties, m) 
	if (legal (m, other)) {
	  moves += liberties;
	  return true;
	}
    }
    bool res = false;
    if (n == 2) {
      allRzone (i, liberties, m)
	if (legal (m, other)) {
	  play (m, other);
	  if (isCapturedLadder (inter, 0)) {
	    moves.add (m);
	    res = true;
	  }
	  undo ();
	}
    }
    return res;
  }

  bool captureLadder (int inter, int depth, Rzone & liberties, Rzone * trace = NULL) {
    if (depth > 100)
      return false;
    nbNodesLadder++;
    if (nbNodesLadder > 1000)
      return false;
    //print (stderr);
    if (board [inter] == Empty)
      return true;
    //if (debug)
    //fprintf (stderr, "captureLadder (%d,%d) board [1] = %d\n", inter, depth, board [1]);
    int n = liberties.size ();
    if (trace != NULL)
      trace->add (liberties);
    //fprintf (stderr, "%d lib,", n);
    if (n > 2)
      return false;
    int other = opponent (board [inter]);
    if (n <= 1) {
      allRzone (i, liberties, m) 
	if (legal (m, other))
	  return true;
      //else
      //  fprintf (stderr, "illegal %d\n", m);
    }
    bool res = false;
    if (n == 2) {
      /**/
      int lib [2];
      allRzone (i, liberties, m)
	lib [i - 1] = m;
      if (nbEmpty (lib [1]) > nbEmpty (lib [0])) {
	int tmp = lib [0];
	lib [0] = lib [1];
	lib [1] = tmp;
      }
      for (int i = 0; i < 2; i++) {
	//int nbMoves = 1;
	//if (nbEmpty (lib [0]) <= 2)
	//nbMoves = 2;
	//nbMoves = 2;
	//for (int i = 0; i < nbMoves; i++) {
	int m = lib [i];
	if (legal (m, other)) {
	  play (m, other);
	  if (isCapturedLadder (inter, depth + 1, trace)) {
	    res = true;
	  }
	  //if (debug) {
	  //fprintf (stderr, "captureLadder after (%d,%d) board [1] = %d\n", inter, depth, board [1]);
	  //if ((inter == 414) && (depth == 4))
	  //  debugUndo = true;
	  //}
	  undo ();
	  //debugUndo = false;
	  //if (debug)
	  //fprintf (stderr, "captureLadder after undo (%d,%d) board [1] = %d\n", inter, depth, board [1]);
	  if (res)
	    return true;
	}
      }
      // capture adjacent adjacent strings
      /*
      adjacents (chaines [i], stones, adjacent);
      allRzone (j, adjacent, s) {
        Rzone libertiesAdjacent, stonesAdjacent;
        int nadj = nbLiberties (s, libertiesAdjacent, stonesAdjacent);
        if (nadj == 1) {
          Rzone libertiesAdjacentAdjacent, stonesAdjacentAdjacent;
          int nadjadj = nbLiberties (s, libertiesAdjacentAdjacent, stonesAdjacentAdjacent);
          if (nadjadj == 1) {
            allRzone (k, libertiesAdjacentAdjacent, m) {
              if (legal (m, other)) {
                play (m, other);
                if (isCapturedLadder (inter, depth + 1)) {
                  res = true;
                }
                undo ();
                if (res)
                  return true;
              }
            }
          }
        }
      }
      */
    }
    return res;
  }
  
  bool isCapturedLadder (int inter, int depth, Rzone * trace = NULL) {
    if (depth > 100)
      return false;
    nbNodesLadder++;
    if (nbNodesLadder > 1000)
      return false;
    //print (stderr);
    if (board [inter] == Empty)
      return true;
    Rzone liberties, stones, adjacent;
    int n = nbLiberties (inter, liberties, stones, 2);
    if (trace != NULL)
      trace->add (liberties);
    if (n == 0)
      return true;
    if (n > 1)
      return false;
    //if (debug)
    //fprintf (stderr, "isCapturedLadder (%d,%d) board [1] = %d\n", inter, depth, board [1]);
    bool res = true;
    if (n == 1) {
      allRzone (i, liberties, m) 
	if (legal (m, board [inter])) {
	  play (m, board [inter]);
	  Rzone liberties1, stones1;
	  int n1 = nbLiberties (inter, liberties1, stones1, 3);
	  if (!captureLadder (m, depth + 1, liberties1, trace)) {
	    res = false;
	  }
	  undo ();
	  //if (debug)
	  //fprintf (stderr, "isCapturedLadder after n==1 (%d,%d) board [1] = %d\n", inter, depth, board [1]);
	  if (res == false)
	    return false;
	}
      adjacents (inter, stones, adjacent);
      allRzone (i, adjacent, s) {
	Rzone libertiesAdjacent, stonesAdjacent;
	int nadj = nbLiberties (s, libertiesAdjacent, stonesAdjacent, 2);
	if (nadj == 1) {
	  allRzone (j, libertiesAdjacent, l) 
	    if (legal (l, board [inter])) {
	      play (l, board [inter]);
	      if (trace != NULL)
		trace->add (l);
	      Rzone liberties1, stones1;
	      int n1 = nbLiberties (inter, liberties1, stones1, 3);
	      if (!captureLadder (inter, depth + 1, liberties1, trace)) {
		res = false;
	      }
	      undo ();
	      //if (debug)
	      //fprintf (stderr, "isCapturedLadder after adj (%d,%d) board [1] = %d\n", inter, depth, board [1]);
	      if (res == false)
		return false;
	    }
	}
      }
    }
    return res;
  }
  
  int isInLadder [MaxSize];
  int adjIsInLadder [MaxSize];
  int opponentIsInLadder [MaxSize];
  int captureInLadder [MaxSize];
  int escapeLadder [MaxSize];
  int threatCaptureInLadder [MaxSize];
  int opponentThreatCaptureInLadder [MaxSize];
  int isLostLadder [MaxSize];
  int adjIsLostLadder [MaxSize];
  int isUnsettledLadder [MaxSize];

  int nbNodesLadder;

  void computeLadders (int color) {
    //print (stderr);
    int other = opponent (color);
    for (int i = 0; i < MaxSize; i++) {
      isInLadder [i] = 0;
      opponentIsInLadder [i] = 0;
      captureInLadder [i] = 0;
      escapeLadder [i] = 0;
      threatCaptureInLadder [i] = 0;
      opponentThreatCaptureInLadder [i] = 0;
    }
    //print (stderr);
    //int b [MaxSize];
    //for (int i = 0; i < MaxSize; i++)
    //b [i] = board [i];
    for (int i = 0; i < nbChaines; i++) {
      //fprintf (stderr, "%d, ", chaines [i]);
      //if ((NbMovesPlayed == 140) && (i == 54))
      //fprintf (stderr, "debug");
      /*
      if (board [chaines [i]] == color) {
	nbNodesLadder = 0;
	//initLadder (chaines [i]);
	if (isCapturedLadder (chaines [i], 0)) {
	  Rzone liberties, stones, adjacent;
	  int n = nbLiberties (chaines [i], liberties, stones);
	  allRzone (j, stones, m) {
	    isInLadder [m] = 1;
	  }
	}
	nbNodesLadder = 0;
	if (captureLadder (chaines [i], 0)) {
	  // - fusionner les zones pour eviter les doublons
	  Rzone liberties, stones, adjacent, order2, escapeMoves;
	  int n = nbLiberties (chaines [i], liberties, stones);
	  allRzone (j, liberties, m) {
	    play (m, color);
	    nbNodesLadder = 0;
	    if (!captureLadder (chaines [i], 0)) {
	      escapeMoves.add (m);
	    }
	    undo ();
	  }
	  // order 2 liberties
	  orderTwoLiberties (liberties, order2);
	  allRzone (j, order2, m) {
	    play (m, color);
	    nbNodesLadder = 0;
	    if (!captureLadder (chaines [i], 0)){
	      escapeMoves.add (m);
	    }
	    undo ();
	  }
	  // adjacent strings
	  adjacents (chaines [i], stones, adjacent);
	  allRzone (j, adjacent, s) {
	    Rzone libertiesAdjacent, stonesAdjacent;
	    int nadj = nbLiberties (s, libertiesAdjacent, stonesAdjacent);
	    if (nadj <= 2) {
	      allRzone (k, libertiesAdjacent, m) {
		play (m, color);
                Rzone libertiesAdjacentAdjacent, stonesAdjacentAdjacent;
                int nadjadj = nbLiberties (m, libertiesAdjacentAdjacent, stonesAdjacentAdjacent);
                if (nadjadj == 1) {
                  allRzone (k1, libertiesAdjacentAdjacent, m1) {
                    play (m1, other);
                    undo ();
                  }
                }
		else {
                  nbNodesLadder = 0;
                  if (!captureLadder (chaines [i], 0)){
                    escapeMoves.add (m);
                  }
                }
                undo ();
	      }
	    }
	  }
	  allRzone (j, escapeMoves, m) {
	    int nb = nbPierres [chaines [i]];
	    escapeLadder [m] += nb;
	    //fprintf (stderr, "el (%d,%d)\n", m, nb);
	  }
	}
	//for (int j = 0; j < MaxSize; j++)
	//if (b [j] != board [j]) {
	//  print (stderr);
	//  fprintf (stderr, "i = %d, NbMovesPlayed = %d, ", i, NbMovesPlayed);
	//}
      }
      else if (board [chaines [i]] == other) {
	Rzone moves;
	nbNodesLadder = 0;
	//initLadder (chaines [i]);
	if (captureLadder (chaines [i], moves)) {
	  if (!isCapturedLadder (chaines [i], 0)) {
	    allRzone (j, moves, m) {
	      int nb = nbPierres [chaines [i]];
	      captureInLadder [m] += nb;
	    }
	  }
	  else {
	    Rzone liberties, stones, adjacent;
	    int n = nbLiberties (chaines [i], liberties, stones);
	    allRzone (j, stones, m) {
	      isInLadder [m] = 1;
	    }
	  }
	}
	else {
	  Rzone liberties, stones, adjacent, order2;
	  int n = nbLiberties (chaines [i], liberties, stones);
	  if (n < 4)
	    allRzone (j, liberties, m) {
	      play (m, color);
	      nbNodesLadder = 0;
	      if (captureLadder (chaines [i], 0))
		threatCaptureInLadder [m] = 1;
	      undo ();
	    }
	  // order 2 liberties
	  if (n < 3) {
	    orderTwoLiberties (liberties, order2);
	    allRzone (j, order2, m) {
	      play (m, color);
	      nbNodesLadder = 0;
	      if (captureLadder (chaines [i], 0))
		threatCaptureInLadder [m] = 1;
	      undo ();
	    }
	  }
	  // adjacent strings
	  if (n < 3) {
	    adjacents (chaines [i], stones, adjacent);
	    allRzone (j, adjacent, s) {
	      Rzone libertiesAdjacent, stonesAdjacent;
	      int nadj = nbLiberties (s, libertiesAdjacent, stonesAdjacent);
	      if (nadj <= 2) {
		allRzone (k, libertiesAdjacent, m) {
		  play (m, color);
		  nbNodesLadder = 0;
		  if (captureLadder (chaines [i], 0))
		    threatCaptureInLadder [m] = 1;
		  undo ();
		}
	      }
	    }
	  }
	}
	//for (int j = 0; j < MaxSize; j++)
	//if (b [j] != board [j]) {
	//  print (stderr);
	//  fprintf (stderr, "i = %d, NbMovesPlayed = %d, ", i, NbMovesPlayed);
	//}
      }
    }
    for (int i = 0; i < MaxSize; i++)
      if (board [i] == Empty) {
	if (legal (i, color)) {
	  //if (n > 5)
          //fprintf (stderr, "i(%d,%d)\n", i, n);
	  play (i, color);
	  int n = nbPierresSiJoue (i, color);
	  nbNodesLadder = 0;
	  //initLadder (i);
	  if (captureLadder (i, 0)) {
	    isInLadder [i] = n;
	    //fprintf (stderr, "iil(%d,%d),", i, n);
	  } 
	  undo ();
	}
	
	//if (legal (i, other)) {
	  //play (i, other);
	  //nbNodesLadder = 0;
	  //if (captureLadder (i, 0))
	    //opponentIsInLadder [i] = true;
	  //undo ();
	//}
	*/
    }
  }
  
  void computeAllLadders (int color, bool oldLadder = true) {
    //print (stderr);
    //fprintf (stderr, "start board [1] = %d\n", board [1]);
    int other = opponent (color);
    for (int i = 0; i < MaxSize; i++) {
      isInLadder [i] = 0;
      opponentIsInLadder [i] = 0;
      captureInLadder [i] = 0;
      escapeLadder [i] = 0;
      threatCaptureInLadder [i] = 0;
      opponentThreatCaptureInLadder [i] = 0;
      isLostLadder [i] = 0;
      isUnsettledLadder [i] = 0;
    }
    //print (stderr);
    //int b [MaxSize];
    //for (int i = 0; i < MaxSize; i++)
    //b [i] = board [i];
    Rzone seen;
    for (int i = 0; i < MaxSize; i++) 
      if (!seen.element (i)) {
	//fprintf (stderr, "%d, ", i);
	//if ((NbMovesPlayed == 140) && (i == 54))
	//fprintf (stderr, "debug");
	if (board [i] == color) {
	  //fprintf (stderr, "color board [1] = %d\n", board [1]);
	  Rzone liberties, stones;
	  int n = nbLiberties (i, liberties, stones);
	  seen += stones;
	  if (n == 1) {
	    nbNodesLadder = 0;
	    if (isCapturedLadder (i, 0)) {
	      allRzone (j, stones, m) {
		isLostLadder [m] = 1;
		if (oldLadder)
		  isInLadder [m] = 1;
	      }
	    }
	  }
	  if (n <= 2) {
	    nbNodesLadder = 0;
	    Rzone trace;
	    if (captureLadder (i, 0, liberties, &trace)) {
	      Rzone escapeMoves;
	      /*
	      Rzone adjacent;
	      adjacents (i, stones, adjacent);
	      allRzone (j, adjacent, s) {
		Rzone libertiesAdjacent, stonesAdjacent;
		int nadj = nbLiberties (s, libertiesAdjacent, stonesAdjacent, 3);
		if (nadj <= 2) {
		  trace.add (libertiesAdjacent);
		}
	      }
	      allRzone (j, trace, m) 
	        if (legal (m, color)) {
		  play (m, color);
	      */
	      for (int j = 0; j < MaxSize; j++)
		if (board [j] == Empty)
		if (legal (j, color)) {
		  play (j, color);
		  nbNodesLadder = 0;
		  Rzone liberties1, stones1;
		  int n1 = nbLiberties (i, liberties1, stones1);
		  if (!captureLadder (i, 0, liberties1)) {
		    escapeMoves.add (j);
		  }
		  undo ();
		}
	      allRzone (j, escapeMoves, m) {
		int nb = stones.size ();
		escapeLadder [m] += nb;
		//fprintf (stderr, "el (%d,%d)\n", m, nb);
	      }
	      if (escapeMoves.size () == 0) {
		allRzone (j, stones, m) {
		  isLostLadder [m] = 1;
		}
	      }
	      else {
		allRzone (j, stones, m) {
		  isUnsettledLadder [m] = 1;
		}
	      }
	    }
	  }
	  if (n == 3) {
	    //fprintf (stderr, "other else i = %d board [1] = %d\n", i, board [1]);
	    //if (i == 388)
	    //debug = true;
	    allRzone (j, liberties, m) 
	      //for (int j = 0; j < MaxSize; j++)
	      //if (board [j] == Empty) {
	      //if (legal (j, color)) {
	      if (legal (m, opponent (color))) {
		play (m, opponent(color));
		//if (debug)
		//fprintf (stderr, "other else j = %d board [1] = %d\n", j, board [1]);
		Rzone liberties1, stones1;
		int n1 = nbLiberties (i, liberties1, stones1);
		nbNodesLadder = 0;
		if (captureLadder (i, 0, liberties1))
		  opponentThreatCaptureInLadder [m] = 1;
		//if (debug)
		//fprintf (stderr, "other else after j = %d board [1] = %d\n", j, board [1]);
		undo ();
	      }
	    //debug = false;
	  }
	  /**/
	  //for (int j = 0; j < MaxSize; j++)
	//if (b [j] != board [j]) {
	//  print (stderr);
	//  fprintf (stderr, "i = %d, NbMovesPlayed = %d, ", i, NbMovesPlayed);
	//}
	}
	else if (board [i] == other) {
	  //fprintf (stderr, "other board [1] = %d\n", board [1]);
	  Rzone liberties, stones;
	  int n = nbLiberties (i, liberties, stones);
	  seen += stones;
	  Rzone moves;
	  nbNodesLadder = 0;
	  if (captureLadder (i, moves)) {
	    //fprintf (stderr, "other captureLadder board [1] = %d\n", board [1]);
	    bool escaped = false;
	    for (int j = 0; ((j < MaxSize) && (escaped == false)); j++)
	      if (board [j] == Empty)
		if (legal (j, other)) {
		  play (j, other);
		  nbNodesLadder = 0;
		  Rzone liberties1, stones1;
		  int n1 = nbLiberties (i, liberties1, stones1);
		  if (!captureLadder (i, 0, liberties1)) {
		    escaped = true;
		  }
		  undo ();
		}
	    if (escaped) {
	      allRzone (j, moves, m) {
		int nb = stones.size ();
		captureInLadder [m] += nb;
	      }
	      allRzone (j, stones, m) {
		isUnsettledLadder [m] = 1;
	      }
	    }
	    else {
	      allRzone (j, stones, m) {
		isLostLadder [m] = 1;
		if (oldLadder)
		  isInLadder [m] = 1;
	      }
	    }
	  }
	  /**/
	  else if (n == 3) {
	    //fprintf (stderr, "other else i = %d board [1] = %d\n", i, board [1]);
	    //if (i == 388)
	    //debug = true;
	    allRzone (j, liberties, m) 
	      //for (int j = 0; j < MaxSize; j++)
	      //if (board [j] == Empty) {
	      //if (legal (j, color)) {
	      if (legal (m, color)) {
		play (m, color);
		//if (debug)
		//fprintf (stderr, "other else j = %d board [1] = %d\n", j, board [1]);
		Rzone liberties1, stones1;
		int n1 = nbLiberties (i, liberties1, stones1);
		nbNodesLadder = 0;
		if (captureLadder (i, 0, liberties1))
		  threatCaptureInLadder [m] = 1;
		//if (debug)
		//fprintf (stderr, "other else after j = %d board [1] = %d\n", j, board [1]);
		undo ();
	      }
	    //debug = false;
	  }
	  /**/
	}
      }
    /**/
    for (int i = 0; i < MaxSize; i++)
      if (board [i] == Empty) {
	if (legal (i, color)) {
	  play (i, color);
	  //if (i == 22) {
	  //print (stderr);
	  //fprintf (stderr, "board [1] = %d\n", board [1]);
	  //}
	  Rzone liberties, stones;
	  int n = nbLiberties (i, liberties, stones);
	  if (n <= 2) {
	    nbNodesLadder = 0;
	    if (captureLadder (i, 0, liberties)) {
	      isInLadder [i] = stones.size ();
	      //fprintf (stderr, "iil(%d,%d),", i, n);
	    } 
	  }
	  undo ();
	}
	if (legal (i, other)) {
	  play (i, other);
	  Rzone liberties, stones;
	  int n = nbLiberties (i, liberties, stones);
	  if (n <= 2) {
	    nbNodesLadder = 0;
	    if (captureLadder (i, 0, liberties))
	      opponentIsInLadder [i] = stones.size ();
	  }
	  undo ();
	}
      }
  /**/
  }

  void computeIsInLadder () {
    for (int i = 0; i < MaxSize; i++) {
      isInLadder [i] = 0;
      adjIsInLadder [i] = 0;
    }
    Rzone seen;
    for (int i = 0; i < MaxSize; i++) 
      if (!seen.element (i)) {
	if ((board [i] == Black) || (board [i] == White)) {
	  Rzone liberties, stones;
	  int n = nbLiberties (i, liberties, stones);
	  seen += stones;
	  if (n == 2) {
	    nbNodesLadder = 0;
	    Rzone trace;
	    if (captureLadder (i, 0, liberties, &trace)) {
	      allRzone (j, stones, m) {
		isInLadder [m] = 1;
	      }
	      Rzone adjacent;
	      adjacents (i, stones, adjacent);
	      allRzone (j, adjacent, s) {
		Rzone liberties1, stones1;
		int n1 = nbLiberties (s, liberties1, stones1);
		allRzone (k, stones1, a) {
		  adjIsInLadder [a] = 1;
		}
	      }
	    }
	  }
	}
      }
  }
  
  void computeLostLadders () {
    for (int i = 0; i < MaxSize; i++) {
      isLostLadder [i] = 0;
      adjIsLostLadder [i] = 0;
    }
    Rzone seen;
    for (int i = 0; i < MaxSize; i++) 
      if (!seen.element (i)) {
	if ((board [i] == Black) || (board [i] == White)) {
	  Rzone liberties, stones;
	  int n = nbLiberties (i, liberties, stones);
	  seen += stones;
	  if (n == 1) {
	    nbNodesLadder = 0;
	    if (isCapturedLadder (i, 0)) {
	      allRzone (j, stones, m) {
		isLostLadder [m] = 1;
	      }
	      Rzone adjacent;
	      adjacents (i, stones, adjacent);
	      allRzone (j, adjacent, s) {
		Rzone liberties1, stones1;
		int n1 = nbLiberties (s, liberties1, stones1);
		allRzone (k, stones1, a) {
		  adjIsLostLadder [a] = 1;
		}
	      }
	    }
	  }
	}
      }
  }
  
  void printLadders (FILE * fp, int color) {
    int other = opponent (color);
    for (int i = 0; i < MaxSize; i++) {
      if (isInLadder [i] > 0)
	fprintf (fp, "1 ");
      else if (opponentIsInLadder [i])
	fprintf (fp, "2 ");
      else if (captureInLadder [i] > 0)
	fprintf (fp, "3 ");
      else if (escapeLadder [i] > 0)
	fprintf (fp, "4 ");
      else if (threatCaptureInLadder [i] > 0)
	fprintf (fp, "5 ");
      else if (opponentThreatCaptureInLadder [i])
	fprintf (fp, "6 ");
      else if (isLostLadder [i] > 0)
	fprintf (fp, "7 ");
      else if (board [i] == Exterieur)
	fprintf (fp, "- ");
      else if (board [i] == White)
	fprintf (fp, "O ");
      else if (board [i] == Black)
	fprintf (fp, "@ ");
      else
	fprintf (fp, "+ ");
      if ((i + 1) % dxBoard == 0) 
	fprintf (fp, "\n");
    }
  }

  
  void printZone (FILE * fp, Rzone & zone) {
    for (int i = 0; i < MaxSize; i++) {
      if (zone.element (i))
	fprintf (fp, ".  ");
      else if (board [i] == Exterieur)
	fprintf (fp, " - ");
      else if (board [i] == White)
	fprintf (fp, "O  ");
      else if (board [i] == Black)
	fprintf (fp, "@  ");
      else
	fprintf (fp, "+  ");
      if ((i + 1) % dxBoard == 0) 
	fprintf (fp, "\n");
    }
  }

  bool sameString (int inter, int inter1) {
    Rzone liberties, stones;
    int n = nbLiberties (inter, liberties, stones);
    return stones.element (inter1);
  }
  
  bool neighbors (int inter, int inter1) {
    if (inter == inter1 - dxBoard)
      return true;
    if (inter == inter1 - 1)
      return true;
    if (inter == inter1 + 1)
      return true;
    if (inter == inter1 + dxBoard)
      return true;
    return false;
  }

  bool eye (int inter, int couleur) {
    if (!surrounded (inter, couleur))
      return false;
    
    int other = opponent (couleur);
    int nbDiagOther = 0;
    for (int i = 4; i < 8; i++)
      if (board [inter + delta [i]] == other)
	nbDiagOther++;
    
    bool bord = false;
    if ((board [inter - dxBoard] == Exterieur) ||
	(board [inter + dxBoard] == Exterieur) ||
	(board [inter - 1] == Exterieur) ||
	(board [inter + 1] == Exterieur))
      bord = true;
    
    if (bord && (nbDiagOther > 0))
      return false;
    if (!bord && (nbDiagOther > 1))
      return false;
    
    int nbDiagNotProtected = 0;
    for (int i = 4; i < 8; i++)
      if (board [inter + delta [i]] == Empty)
	if (!isProtected (inter + delta [i], couleur)) {
	  nbDiagNotProtected++;
	  if (bord) {
	    if (nbDiagOther + nbDiagNotProtected > 0)
	      return false;
	  }
	  else {
	    if (nbDiagOther + (1 + nbDiagNotProtected) / 2 > 1)
	      return false;
	  }
	}
    
    return true;
  }
  
  bool surrounded (int inter, int couleur) {
    // test if all neighbors are of the same color
    if ((board [inter - 1] != couleur) && (board [inter - 1] != Exterieur)) 
      return false;
    if ((board [inter + 1] != couleur) && (board [inter + 1] != Exterieur))
      return false;
    if ((board [inter - dxBoard] != couleur) && (board [inter - dxBoard] != Exterieur))
      return false;
    if ((board [inter + dxBoard] != couleur) && (board [inter + dxBoard] != Exterieur))
      return false;
    return true;
  }

  bool isProtected (int inter, int color) {
    if (board [inter] != Empty)
      return false;
    int other = opponent (color);
    int n = nbLibertiesIfPlay (inter, other);
    if (n < 2)
      return true;
    return false;
  }
  
  bool potentialEye (int inter, int couleur, Rzone & zone) {
    if (board [inter] == couleur)
      return false;
    int other = opponent (couleur);
    if (board [inter] == other)
      return true;
    if (board [inter] == Empty)
      if ((board [inter - dxBoard] == other) ||
	  (board [inter + dxBoard] == other) ||
	  (board [inter - 1] == other) ||
	  (board [inter + 1] == other))
	return false;
    int nbDiagOther = 0;
    for (int i = 4; i < 8; i++)
      if (board [inter + delta [i]] == other)
	if (!zone.element (inter + delta [i]))
	  nbDiagOther++;
    
    bool bord = false;
    if ((board [inter - dxBoard] == Exterieur) ||
	(board [inter + dxBoard] == Exterieur) ||
	(board [inter - 1] == Exterieur) ||
	(board [inter + 1] == Exterieur))
      bord = true;
    
    if (bord && (nbDiagOther > 0))
      return false;
    if (!bord && (nbDiagOther > 1))
      return false;

    return true;
  }
  

  void adjacents (int inter, Rzone & stones, Rzone & adjacent) {
    Rzone seen;
    int other = opponent (board [inter]);
    allRzone (i, stones, s) {
      if (board [s - dxBoard] == other) {
	if (!seen.element (s - dxBoard)) {
	  adjacent.add (s - dxBoard);
	  markStones (s - dxBoard, seen);
	}
      }
      if (board [s - 1] == other) {
	if (!seen.element (s - 1)) {
	  adjacent.add (s - 1);
	  markStones (s - 1, seen);
	}
      }
      if (board [s + 1] == other) {
	if (!seen.element (s + 1)) {
	  adjacent.add (s + 1);
	  markStones (s + 1, seen);
	}
      }
      if (board [s + dxBoard] == other) {
	if (!seen.element (s + dxBoard)) {
	  adjacent.add (s + dxBoard);
	  markStones (s + dxBoard, seen);
	}
      }
    }
  }
    
  bool atari (int p) {
    if (nbPseudoLibertes [premierePierre [p]] > 4)
      return false;
    int premiere = premierePseudoLiberte [premierePierre [p]];
    int lib = premiere >> 2;
    int suivante = pseudoLiberteSuivante [premiere];
    
    while (suivante != premiere) {
      if ((suivante >> 2) != lib)
	return false;
      suivante = pseudoLiberteSuivante [suivante];
    }
    
    return true;
  }
  
  bool protegee (int inter, int couleur) {
    for (int p = 0; p < MaxPlayers; p++)
      if (p != couleur)
	if (!atariSiJoue (inter, p))
	  return false;
    return true;
  }
  
  bool atariSiJoue (int inter, int couleur) {
    int lib = 0;
    
    if (board [inter - 1] == Empty) {
      if (lib == 0) lib = inter - 1;
      else if (lib != inter - 1) return false;
    }
    if (board [inter + 1] == Empty) {
      if (lib == 0) lib = inter + 1;
      else if (lib != inter + 1) return false;
    }
    if (board [inter - dxBoard] == Empty) {
      if (lib == 0) lib = inter - dxBoard;
      else if (lib != inter - dxBoard) return false;
    }
    if (board [inter + dxBoard] == Empty)  {
      if (lib == 0) lib = inter + dxBoard;
      else if (lib != inter + dxBoard) return false;
    }
    
    if (board [inter - 1] == couleur) {
      int premiere = premierePseudoLiberte [premierePierre [inter - 1]];
      if ((premiere >> 2) != inter) {
	if (lib == 0) lib = premiere >> 2;
	else if (lib != (premiere >> 2)) return false;
      }
      int suivante = pseudoLiberteSuivante [premiere];
      while (suivante != premiere) {
	if ((suivante >> 2) != inter) {
	  if (lib == 0) lib = suivante >> 2;
	  else if (lib != (suivante >> 2)) return false;
	}
	suivante = pseudoLiberteSuivante [suivante];
      }
    }
    if (board [inter + 1] == couleur) {
      int premiere = premierePseudoLiberte [premierePierre [inter + 1]];
      if ((premiere >> 2) != inter) {
	if (lib == 0) lib = premiere >> 2;
	else if (lib != (premiere >> 2)) return false;
      }
      int suivante = pseudoLiberteSuivante [premiere];
      while (suivante != premiere) {
	if ((suivante >> 2) != inter) {
	  if (lib == 0) lib = suivante >> 2;
	  else if (lib != (suivante >> 2)) return false;
	}
	suivante = pseudoLiberteSuivante [suivante];
      }
    }
    if (board [inter - dxBoard] == couleur) {
      int premiere = premierePseudoLiberte [premierePierre [inter - dxBoard]];
      if ((premiere >> 2) != inter) {
	if (lib == 0) lib = premiere >> 2;
	else if (lib != (premiere >> 2)) return false;
      }
      int suivante = pseudoLiberteSuivante [premiere];
      while (suivante != premiere) {
	if ((suivante >> 2) != inter) {
	  if (lib == 0) lib = suivante >> 2;
	  else if (lib != (suivante >> 2)) return false;
	}
	suivante = pseudoLiberteSuivante [suivante];
      }
    }
    if (board [inter + dxBoard] == couleur)  {
      int premiere = premierePseudoLiberte [premierePierre [inter + dxBoard]];
      if ((premiere >> 2) != inter) {
	if (lib == 0) lib = premiere >> 2;
	else if (lib != (premiere >> 2)) return false;
      }
      int suivante = pseudoLiberteSuivante [premiere];
      while (suivante != premiere) {
	if ((suivante >> 2) != inter) {
	  if (lib == 0) lib = suivante >> 2;
	  else if (lib != (suivante >> 2)) return false;
	}
	suivante = pseudoLiberteSuivante [suivante];
      }
    }
    
    int other = opponent (couleur);
    if (board [inter - 1] == other) 
      if (atari (inter - 1)) {
	if (lib == 0) lib = inter - 1;
	else if (lib != inter - 1) return false;
      }
    if (board [inter + 1] == other)
      if (atari (inter + 1)) {
	if (lib == 0) lib = inter + 1;
	else if (lib != inter + 1) return false;
      }
    if (board [inter - dxBoard] == other)
      if (atari (inter - dxBoard)) {
	if (lib == 0) lib = inter - dxBoard;
	else if (lib != inter - dxBoard) return false;
      }
    if (board [inter + dxBoard] == other)
      if (atari (inter + dxBoard)) {
	if (lib == 0) lib = inter + dxBoard;
	else if (lib != inter + dxBoard) return false;
      }
    
    
    return true;
  }
  
  bool atariPlusieursPierresSiJoue (int inter, int couleur) {
    if ((board [inter - 1] == couleur) ||
	(board [inter + 1] == couleur) || 
	(board [inter - dxBoard] == couleur) ||
	(board [inter + dxBoard] == couleur))  {
      if (atariSiJoue (inter, couleur)) {
	/* coup @ en A9 capture plusieurs pierres : */
	/*        A  B  C  D  E  F  G  H  J   */
	/*     /  -  -  -  -  -  -  -  -  -  \  */
	/*   9 |  +  O  O  @  @  @  @  @  @  |  9 */
	/*   8 |  @  O  @  +  @  +  @  @  @  |  8 */
	/*   7 |  O  O  @  @  @  @  @  @  @  |  7 */
	/*   6 |  @  @  +  @  @  @  @  +  @  |  6 */
	/*   5 |  @  @  @  @  @  +  @  @  @  |  5 */
	if (capturePlusieursPierres (inter, couleur))
	  return false;
	
	/* coup @ en J4 */
	/*        A  B  C  D  E  F  G  H  J   */
	/*     /  -  -  -  -  -  -  -  -  -  \  */
	/*   9 |  @  +  @  @  @  @  @  @  +  |  9 */
	/*   8 |  +  @  +  @  @  +  @  +  @  |  8 */
	/*   7 |  @  @  @  @  @  @  +  @  @  |  7 */
	/*   6 |  +  @  +  @  @  @  @  @  O  |  6 */
	/*   5 |  @  @  @  @  @  @  @  O  O  |  5 */
	/*   4 |  @  +  @  O  @  +  @  O  +  |  4 */
	/*   3 |  +  @  O  O  @  @  @  O  @  |  3 */
	/*   2 |  @  @  O  O  O  O  @  O  +  |  2 */
	/*   1 |  +  @  @  O  O  O  O  O  O  |  1 */
	/*     \  -  -  -  -  -  -  -  -  -  /  */
	/*        A  B  C  D  E  F  G  H  J   */
	if (nbPierresSiJoue (inter, couleur) == 2)
	  if (atariVoisine (inter, couleur))
	    return false;
	return true;
      }
    }
    return false;
  }
  
  bool atariLent (int p) {
    int pierre = premierePierre [p], lib = 0;
    
    do {
      if (board [pierre - 1] == Empty)
	if (lib == 0) lib = pierre - 1;
	else if (lib != pierre - 1) return false;
      if (board [pierre + 1] == Empty)
	if (lib == 0) lib = pierre + 1;
	else if (lib != pierre + 1) return false;
      if (board [pierre - dxBoard] == Empty)
	if (lib == 0) lib = pierre - dxBoard;
	else if (lib != pierre - dxBoard) return false;
      if (board [pierre + dxBoard] == Empty)
	if (lib == 0) lib = pierre + dxBoard;
	else if (lib != pierre + dxBoard) return false;
      pierre = pierreSuivante [pierre];
    } 
    while (pierre != premierePierre [p]);
    
    return true;
  }
  
  int nbPseudo (int p) {
    int nb = 1;
    int premiere = premierePseudoLiberte [premierePierre [p]];
    int suivante = pseudoLiberteSuivante [premiere];
    
    while (suivante != premiere) {
      nb++;
      suivante = pseudoLiberteSuivante [suivante];
    }
    return nb;
  }
  
  int nbPseudoLent (int chaine) {
    int pierre = chaine, nb = 0;
    
    do {
      if (board [pierre - 1] == Empty)
	nb++;
      if (board [pierre + 1] == Empty)
	nb++;
      if (board [pierre - dxBoard] == Empty)
	nb++;
      if (board [pierre + dxBoard] == Empty)
	nb++;
      pierre = pierreSuivante [pierre];
    } 
    while (pierre != chaine);
    
    return nb;
  }
  
  void check () {
    for (int i = start; i < end; i++)
      if (board [i] != Exterieur)
	if (board [i] != Empty)
	  if (nbPseudo (premierePierre [i]) != nbPseudoLibertes [premierePierre [i]])
	    fprintf (stderr, "Pb check\n");
  }
  
  /* score a la chinoise : nb de pierres sur le goban */
  void calculeScores (bool winlost) {
    /*
    int owner [MaxSize];
    for (int i = 0; i < MaxSize; i++)
      owner [i] = ownership [i] [Black];
    print (stderr, owner);
    */
    for (int p = 0; p < MaxPlayers; p++)
      scorePlayer [p] = komi [p];
    
    for (int i = start; i < end; i++) {
      if (board [i] == Empty) {
	for (int p = 0; p < MaxPlayers; p++)
	  if (oeil (i, p)) {
	    scorePlayer [p] += 1.0;
	    ownership [i] [p]++;
	  }
      }
      else if (board [i] == Black) {
	scorePlayer [Black] += 1.0;
	ownership [i] [Black]++;
      }
      else if (board [i] == White) {
	scorePlayer [White] += 1.0;
	ownership [i] [White]++;
      }
    }
    
    if (winlost) {
      float best = scorePlayer [White];
      int winner = White;
      for (int p = 0; p < MaxPlayers; p++)
	if (scorePlayer [p] > best) {
	  best = scorePlayer [p];
	  winner = p;
	}
      for (int p = 0; p < MaxPlayers; p++)
	if (p == winner)
	  scorePlayer [p] = 1.0;
	else
	  scorePlayer [p] = 0.0;
    }
  }
  
  bool gameOver () {
    if (nbPasse >= MaxPlayers)    
      return true;
    return false;
  }
  
  bool losingMove (Move m) {
    if (oeil (interMove [m.inter], m.color))
      return true;
    return false;
  }

  bool coupAutorise (int inter, int couleur) {
    if (legalMove (inter, couleur))
      if (!oeil (inter, couleur))
	if (!atariPlusieursPierresSiJoue (inter, couleur))
	  return true;
    return false;
  }


  int choisitUnCoup (int couleur, int pos, int oldpos) {
    int stackMove [362];
    stackMove [0] = 0;
    int other = opponent (couleur);
    
    int urgency [362];
    
    
    // if last move is atari, save string
    for (int i = 0; i < 4; i++)
      if (board [pos + delta [i]] == couleur) {
	int chaine = premierePierre [pos + delta [i]];
	if (atari (chaine)) {
	  int lib = premierePseudoLiberte [chaine] >> 2;
	  if (!atariSiJoue (lib, couleur)) {
	    stackMove [0]++;
	    stackMove [stackMove [0]] = lib;
	    urgency [stackMove [0]] = 10 * nbPierres [chaine];
	  }
	  int pierre = chaine;
	  do {
	    for (int v = 0; v < 4; v++)
	      if (board [pierre + delta [v]] == other)
		if (atari (premierePierre [pierre + delta [v]])) {
		  lib = premierePseudoLiberte [premierePierre [pierre + delta [v]]] >> 2;
		  stackMove [0]++;
		  stackMove [stackMove [0]] = lib;
		  urgency [stackMove [0]] = 100 * nbPierres [premierePierre [pierre + delta [v]]];
		}
	    pierre = pierreSuivante [pierre];
	  } 
	  while (pierre != chaine);
	}
      }
    
    while (stackMove [0] > 0) {
      int indice = 1, best = urgency [1];
      for (int i = 2; i <= stackMove [0]; i++)
	if (urgency [i] > best) {
	  best = urgency [i];
	  indice = i;
	}
      //indice = 1 + rand () % stackMove [0];
      if (coupAutorise (stackMove [indice], couleur))
	return stackMove [indice];
      stackMove [indice] = stackMove [stackMove [0]];
      urgency [indice] = urgency [stackMove [0]];
      stackMove [0]--;
    }
    
    /*   while (stackMove [0] > 0) { */
    /*     int indice = 1 + rand () % stackMove [0]; */
    /*     if (legalMove (stackMove [indice], couleur)) */
    /*       if (!oeil (stackMove [indice], couleur)) */
    /* 	return stackMove [indice]; */
    /*     stackMove [indice] = stackMove [stackMove [0]]; */
    /*     stackMove [0]--; */
    /*   } */
    
    // try pattern 
    /*
    if (pos != passe) {
      int p = couleur - 2;
      if (board [pos - dxBoard - 1] == Empty)
	if (validCode [code3x3 [p] [pos - dxBoard - 1]]) {
	  stackMove [0]++;
	  stackMove [stackMove [0]] = pos - dxBoard - 1;
	}
      if (board [pos - dxBoard] == Empty)
	if (validCode [code3x3 [p] [pos - dxBoard]]) {
	  stackMove [0]++;
	  stackMove [stackMove [0]] = pos - dxBoard;
	}
      if (board [pos - dxBoard + 1] == Empty)
	if (validCode [code3x3 [p] [pos - dxBoard + 1]]) {
	  stackMove [0]++;
	  stackMove [stackMove [0]] = pos - dxBoard + 1;
	}
      if (board [pos - 1] == Empty)
	if (validCode [code3x3 [p] [pos - 1]]) {
	  stackMove [0]++;
	  stackMove [stackMove [0]] = pos - 1;
	}
      if (board [pos + 1] == Empty)
	if (validCode [code3x3 [p] [pos + 1]]) {
	  stackMove [0]++;
	  stackMove [stackMove [0]] = pos + 1;
	}
      if (board [pos + dxBoard - 1] == Empty)
	if (validCode [code3x3 [p] [pos + dxBoard - 1]]) {
	  stackMove [0]++;
	  stackMove [stackMove [0]] = pos + dxBoard - 1;
	}
      if (board [pos + dxBoard] == Empty)
	if (validCode [code3x3 [p] [pos + dxBoard]]) {
	  stackMove [0]++;
	  stackMove [stackMove [0]] = pos + dxBoard;
	}
      if (board [pos + dxBoard + 1] == Empty)
	if (validCode [code3x3 [p] [pos + dxBoard + 1]]) {
	  stackMove [0]++;
	  stackMove [stackMove [0]] = pos + dxBoard + 1;
	}
    }
    
    while (stackMove [0] > 0) {
      int indice = 1 + rand () % stackMove [0];
      if (legalMove (stackMove [indice], couleur))
	if (!oeil (stackMove [indice], couleur))
	  if (coupAutorise (stackMove [indice], couleur))
	    //if (!atariSiJoue (stackMove [indice], couleur))
	    return stackMove [indice];
      stackMove [indice] = stackMove [stackMove [0]];
      stackMove [0]--;
    }
    */
    
    /*
    // try to capture string in atari
    for (int i = 0; i < nbChaines; i++)
      if (board [chaines [i]] != couleur)
	if (nbPierres [chaines [i]] > 1)
	  if (atari (chaines [i])) {
	    int lib = premierePseudoLiberte [chaines [i]] >> 2;
	    stackMove [0]++;
	    stackMove [stackMove [0]] = lib;
	    urgency [stackMove [0]] = 10 * nbPierres [chaines [i]];
	    int pierre = chaines [i];
	    do {
	      for (int v = 0; v < 4; v++)
		if (board [pierre + delta [v]] == couleur)
		  if (atari (premierePierre [pierre + delta [v]])) {
		    if (urgency [stackMove [0]] < 100 * nbPierres [premierePierre [pierre + delta [v]]])
		      urgency [stackMove [0]] = 100 * nbPierres [premierePierre [pierre + delta [v]]];
		  }
	      pierre = pierreSuivante [pierre];
	    } 
	    while (pierre != chaines [i]);
	    //if (board [chaines [i]] != couleur) 
	    // 	  urgency [stackMove [0]] = 10 * nbPierres [chaines [i]];
	    // 	else
	    // 	  urgency [stackMove [0]] = nbPierres [chaines [i]];
	  }
    
    while (stackMove [0] > 0) {
      int indice = 1, best = urgency [1];
      for (int i = 2; i <= stackMove [0]; i++)
	if (urgency [i] > best) {
	  best = urgency [i];
	  indice = i;
	}
      //indice = 1 + rand () % stackMove [0];
      if (coupAutorise (stackMove [indice], couleur))
	return stackMove [indice];
      stackMove [indice] = stackMove [stackMove [0]];
      urgency [indice] = urgency [stackMove [0]];
      stackMove [0]--;
    }
    */
    
    /*   // try to capture first */
    /*   if (oldpos != passe) { */
    /*     for (int i = 0; i < 4; i++) */
    /*       if ((board [oldpos + delta [i]] > Exterieur) && (board [oldpos + delta [i]] != couleur)) */
    /* 	if (atari (oldpos + delta [i])) { */
    /* 	  int lib = premierePseudoLiberte [premierePierre [oldpos + delta [i]]] >> 2; */
    /* 	  if (coupAutorise (lib, couleur)) */
    /* 	    return lib; */
    /* 	} */
    /*   } */
    
    /*   if (pos != passe) { */
    /*     // capture also lonely stones */
    /*     if ((board [pos] == couleur)) */
    /*       if (atari (pos)) { */
    /* 	int lib = premierePseudoLiberte [premierePierre [pos]] >> 2; */
    /* 	if (coupAutorise (lib, couleur)) */
    /* 	  return lib; */
    /*       } */
    /*     // try to save second */
    /*     for (int i = 0; i < 4; i++) */
    /*       if ((board [pos + delta [i]] > Exterieur) && (board [pos + delta [i]] == couleur)) */
    /* 	if (atari (pos + delta [i])) { */
    /* 	  int lib = premierePseudoLiberte [premierePierre [pos + delta [i]]] >> 2; */
    /* 	  if (coupAutorise (lib, couleur)) */
    /* 	    return lib; */
    /* 	} */
    /*   } */
    
    //int debut = rand () % nbVides;
    int debut = (int)(nbVides * ((double)rand() / ((double)(RAND_MAX)+(double)(1))));
    
    for (int i = debut; i < nbVides; i++)
      if (coupAutorise (vides [i], couleur))
	return vides [i];
    
    for (int i = 0; i < debut; i++)
      if (coupAutorise (vides [i], couleur))
	return vides [i];
    
    return passe;
  }
  
  int choisitUnCoupAleatoire (int couleur, int pos, int oldpos) {
    int debut = (int)(nbVides * ((double)rand() / ((double)(RAND_MAX)+(double)(1))));
    
    for (int i = debut; i < nbVides; i++)
      if (coupAutorise (vides [i], couleur))
	return vides [i];
    
    for (int i = 0; i < debut; i++)
      if (coupAutorise (vides [i], couleur))
	return vides [i];
    
    return passe;
  }
  
  int newStringInAtari (int couleur, int stack [361]) {
    stack [0] = 0;
    if (NbMovesPlayed < 1)
      return 0;
    int inter = Moves [NbMovesPlayed - 1];
    for (int v = 0; v < 4; v++)
      if (board [inter + delta [v]] == couleur)
	if (atari (premierePierre [inter + delta [v]])) {
	  stack [0]++;
	  stack [stack [0]] = premierePierre [inter + delta [v]];
	}
    return stack [0];
  }

  bool capture (int inter, int couleur) {
    for (int v = 0; v < 4; v++)
      if (board [inter + delta [v]] == opponent (couleur))
	if (atari (premierePierre [inter + delta [v]]))
	  return true;
    return false;
  }

  bool captureStringContiguousToNewStringInAtari (int inter, int couleur, int stack [361]) {
    if (stack [0] == 0)
      return false;
    for (int v = 0; v < 4; v++)
      if (board [inter + delta [v]] == opponent (couleur))
	if (atari (premierePierre [inter + delta [v]])) {
	  int pierre = premierePierre [inter + delta [v]];
	  do {
	    for (int v1 = 0; v1 < 4; v1++)
	      if (board [pierre + delta [v1]] == couleur)
		if (atari (premierePierre [pierre + delta [v1]])) {
		  for (int i = 1; i <= stack [0]; i++)
		    if (stack [i] == premierePierre [pierre + delta [v1]])
		      return true;
		}
	    pierre = pierreSuivante [pierre];
	  } 
	  while (pierre != premierePierre [inter + delta [v]]);
	}
    return false;
  }

  int playAtariIfCapture (int couleur) {
    int stackMove [362];
    stackMove [0] = 0;
    int other = opponent (couleur);
    for (int i = 0; i < nbChaines; i++)
      if (board [chaines [i]] != couleur)
	if (nbPierres [chaines [i]] > 1) {
	  int lib, lib1;
	  if (deuxLibertes (chaines [i], lib, lib1)) {
	    int pierre = chaines [i];
	    bool adjacentAtari = false;
	    do {
	      for (int v = 0; v < 4; v++)
		if (board [pierre + delta [v]] == couleur)
		  if (atari (premierePierre [pierre + delta [v]])) {
		    adjacentAtari = true;
		  }
	      pierre = pierreSuivante [pierre];
	    } 
	    while (pierre != chaines [i]);
	    if (!adjacentAtari) {
	      int stack [361];
	      int nblib1 = libSiJoue (lib, board [chaines [i]], stack);
	      int nblib2 = libSiJoue (lib1, board [chaines [i]], stack);
	      if (nblib1 <= 2) {
		stackMove [0]++;
		stackMove [stackMove [0]] = lib1;
	      }
	      if (nblib2 <= 2) {
		stackMove [0]++;
		stackMove [stackMove [0]] = lib;
	      }
	    }
	  }
	}
    
    while (stackMove [0] > 0) {
      int indice = 1 + rand () % stackMove [0];
      if (coupAutorise (stackMove [indice], couleur))
	return stackMove [indice];
      stackMove [indice] = stackMove [stackMove [0]];
      stackMove [0]--;
    }
    
    return -1;
  }
  
  bool libertyNewStringInAtari (int inter, int couleur, int stack [361]) {
    for (int i = 1; i <= stack [0]; i++) {
      int lib = premierePseudoLiberte [premierePierre [stack [i]]] >> 2;
      if (lib == inter)
	return true;
    }
    return false;
  }

  int captureAdjacentAtari (int couleur) {
    int stackMove [362];
    stackMove [0] = 0;
    int other = opponent (couleur);
    for (int i = 0; i < nbChaines; i++)
      if (board [chaines [i]] != couleur)
	if (nbPierres [chaines [i]] > 1)
	  if (atari (chaines [i])) {
	    bool adjacentAtari = false;
	    int pierre = chaines [i];
	    do {
	      for (int v = 0; v < 4; v++)
		if (board [pierre + delta [v]] == couleur)
		  if (atari (premierePierre [pierre + delta [v]])) {
		    adjacentAtari = true;
		  }
	      pierre = pierreSuivante [pierre];
	    } 
	    while (pierre != chaines [i]);
	    if (adjacentAtari) {
	      int lib = premierePseudoLiberte [chaines [i]] >> 2;
	      stackMove [0]++;
	      stackMove [stackMove [0]] = lib;
	    }
	  }
    
    while (stackMove [0] > 0) {
      int indice = 1 + rand () % stackMove [0];
      if (coupAutorise (stackMove [indice], couleur))
	return stackMove [indice];
      stackMove [indice] = stackMove [stackMove [0]];
      stackMove [0]--;
    }
    return -1;
  }
  
  int captureAtari (int couleur) {
    int stackMove [362];
    stackMove [0] = 0;
    int other = opponent (couleur);
    for (int i = 0; i < nbChaines; i++)
      if (board [chaines [i]] != couleur)
	if (nbPierres [chaines [i]] > 1)
	  if (atari (chaines [i])) {
	    int lib = premierePseudoLiberte [chaines [i]] >> 2;
	    stackMove [0]++;
	    stackMove [stackMove [0]] = lib;
	  }
    
    while (stackMove [0] > 0) {
      int indice = 1 + rand () % stackMove [0];
      if (coupAutorise (stackMove [indice], couleur))
	return stackMove [indice];
      stackMove [indice] = stackMove [stackMove [0]];
      stackMove [0]--;
    }
    return -1;
  }
  
  int avoidCaptureAtari (int couleur) {
    int stackMove [362];
    stackMove [0] = 0;
    int other = opponent (couleur);
    for (int i = 0; i < nbChaines; i++)
      if (board [chaines [i]] == couleur)
	if (nbPierres [chaines [i]] > 1)
	  if (atari (chaines [i])) {
	    int lib = premierePseudoLiberte [chaines [i]] >> 2;
	    stackMove [0]++;
	    stackMove [stackMove [0]] = lib;
	  }
    
    while (stackMove [0] > 0) {
      int indice = 1 + rand () % stackMove [0];
      if (coupAutorise (stackMove [indice], couleur))
	return stackMove [indice];
      stackMove [indice] = stackMove [stackMove [0]];
      stackMove [0]--;
    }
    return -1;
  }
  
  int avoidAtariIfCapture (int couleur) {
    int stackMove [362];
    stackMove [0] = 0;
    int other = opponent (couleur);
    for (int i = 0; i < nbChaines; i++)
      if (board [chaines [i]] == couleur)
	if (nbPierres [chaines [i]] > 1) {
	  int lib, lib1;
	  if (deuxLibertes (chaines [i], lib, lib1)) {
	    int pierre = chaines [i];
	    bool adjacentAtari = false;
	    do {
	      for (int v = 0; v < 4; v++)
		if (board [pierre + delta [v]] == other)
		  if (atari (premierePierre [pierre + delta [v]])) {
		    adjacentAtari = true;
		  }
	      pierre = pierreSuivante [pierre];
	    } 
	    while (pierre != chaines [i]);
	    if (!adjacentAtari) {
	      int stack [361];
	      int nblib1 = libSiJoue (lib, board [chaines [i]], stack);
	      int nblib2 = libSiJoue (lib1, board [chaines [i]], stack);
	      if ((nblib2 <= 2) && (nblib1 > 2)) {
		stackMove [0]++;
		stackMove [stackMove [0]] = lib;
	      }
	      if ((nblib1 <= 2) && (nblib2 > 2)){
		stackMove [0]++;
		stackMove [stackMove [0]] = lib1;
	      }
	    }
	  }
	}
    
    while (stackMove [0] > 0) {
      int indice = 1 + rand () % stackMove [0];
      if (coupAutorise (stackMove [indice], couleur))
	return stackMove [indice];
      stackMove [indice] = stackMove [stackMove [0]];
      stackMove [0]--;
    }
    
    return -1;
  }
  
  int choisitUnCoupBis (int couleur, int pos, int oldpos) {
    int move;
    
    // k=8000, c=0.3, caa + ca + aca => 58/200
    // k=8000, c=0.3, caa + ca + paic + aca + aaic => 105/200
    // k=2000, c=0.3, caa + ca + paic + aca + aaic => 54/200
    move = captureAdjacentAtari (couleur);
    if (move != -1)
      return move;
    
    move = captureAtari (couleur);
    if (move != -1)
      return move;
    
    move = playAtariIfCapture (couleur);
    if (move != -1)
      return move;
    
    move = avoidCaptureAtari (couleur);
    if (move != -1)
      return move;
    
    move = avoidAtariIfCapture (couleur);
    if (move != -1)
      return move;
    
    int debut = rand () % nbVides;
    
    for (int i = debut; i < nbVides; i++)
      if (coupAutorise (vides [i], couleur))
	return vides [i];
    
    for (int i = 0; i < debut; i++)
      if (coupAutorise (vides [i], couleur))
	return vides [i];
    
    return passe;
  }
  
  int playout (int couleur) {
    /*
    int owner [MaxSize];
    for (int i = 0; i < MaxSize; i++)
      owner [i] = ownership [i] [Black];
    print (stderr, owner);
    */
    int pos = passe, oldpos = passe;
    for (;;) {
      if (NbMovesPlayed >= MaxMovesPerGame - 1) {
	//fprintf(stderr, "W");
	if (false) {
	  char s [256];
	  sprintf (s, "toolong%03d.sgf", nbTooLong);
	  //printSgf (s);
	}
	nbTooLong++;
	return false;
      }
      /*     if (NbMovesPlayed >= 2 * size) */
      /*       superKo = true; */
      
      int mempos = pos;
      pos = choisitUnCoup (couleur, pos, oldpos);
      //pos = choisitUnCoupAleatoire (couleur, pos, oldpos);
      //pos = choisitUnCoupBis (couleur, pos, oldpos);
      //pos = choisitUnCoupAleatoireBestPattern (couleur, pos, oldpos);
      oldpos = mempos;
      
      if (length < MaxPlayoutLength) {
	Move m;
	m.inter = moveInter [pos];
	m.color = couleur;
	//m.code = 0;
      }

      //print (stderr);
      //fprintf (stderr, "%d (%d,%d)\n", pos, pos % dxBoard, pos / dxBoard);
      //fprintf (stderr, "%d,%d,%d,%d,%d\n", bestMovesPattern [White] [0], bestMovesPattern [White] [1], bestMovesPattern [White] [2], bestMovesPattern [White] [3], bestMovesPattern [White] [4]);

      joue (pos, couleur);

      if (length < MaxPlayoutLength) {
	rollout [length].inter = moveInter [pos];
	rollout [length].color = couleur;
	length++;
      }
      
      if (gameOver ()) {
	//if (nb < 0) {
	if (0) {
	  char s [256];
	  //sprintf (s, "playout%03d.sgf", nb);
	  //printSgf (s);
	}
	break;
      }
      /*     if (first [pos] == 0) */
      /*       first [pos] = couleur; */
      couleur = opponent (couleur);
    }
    return score ();
  }
    
  void print () {
    print (stderr);
  }

  void print (FILE * fp) {
    int i;
    fprintf(fp,"       ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "%-3c", 'A' + i + (i > 7));
    fprintf (fp, "\n");
    fprintf (fp, "    /  ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "-  ");
    fprintf (fp, "\\ \n");
    for (i = start - 1; i <= end; i++) {
      if (((i) % dxBoard == 0)) fprintf (fp, "%3d |  ", dyBoard - 1 - (i / dxBoard));
      else if (((i + 1) % (dxBoard) == 0)) fprintf (fp, "| %2d\n", dyBoard - 1 - (i / dxBoard));
      else if (board [i] == Empty) fprintf (fp, "+  ");
      else if (board [i] == Black) fprintf (fp, "@  ");
      else if (board [i] == White) fprintf (fp, "O  ");
      else fprintf(fp, "%d  ", board [i]);
    }
    fprintf (fp, "    \\  ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "-  ");
    fprintf (fp, "/ \n");
    fprintf(fp,"       ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "%-3c", 'A' + i + (i > 7));
    fprintf (fp, "\n");
    fprintf (fp, "hash = %llu\n", hash);
  }
  
  void print (FILE * fp, double tab [MaxSize]) {
    int i;
    fprintf(fp,"       ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "   %-3c  ", 'A' + i + (i > 7));
    fprintf (fp, "\n");
    fprintf (fp, "    /  ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "   -    ");
    fprintf (fp, "\\ \n");
    for (i = start - 1; i <= end; i++) {
      if (((i) % dxBoard == 0)) fprintf (fp, "%3d |  ", dyBoard - 1 - (i / dxBoard));
      else if (((i + 1) % (dxBoard) == 0)) fprintf (fp, "| %2d\n", dyBoard - 1 - (i / dxBoard));
      else if (board [i] == Empty) fprintf (fp, "%5.2lf  ", tab [i]);
      else if (board [i] == Black) fprintf (fp, "   @   ");
      else if (board [i] == White) fprintf (fp, "   O   ");
      else fprintf(fp, "%d  ", board [i]);
    }
    fprintf (fp, "    \\  ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "   -    ");
    fprintf (fp, "/ \n");
    fprintf(fp,"       ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "   %-3c  ", 'A' + i + (i > 7));
    fprintf (fp, "\n");
    fprintf (fp, "hash = %llu\n", hash);
  }
  
  void print (FILE * fp, int tab [MaxSize]) {
    int i;
    fprintf(fp,"       ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "   %-3c  ", 'A' + i + (i > 7));
    fprintf (fp, "\n");
    fprintf (fp, "    /  ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "   -    ");
    fprintf (fp, "\\ \n");
    for (i = start - 1; i <= end; i++) {
      if (((i) % dxBoard == 0)) fprintf (fp, "%3d |  ", dyBoard - 1 - (i / dxBoard));
      else if (((i + 1) % (dxBoard) == 0)) fprintf (fp, "| %2d\n", dyBoard - 1 - (i / dxBoard));
      else if (board [i] == Empty) fprintf (fp, "%5d  ", tab [i]);
      else if (board [i] == Black) fprintf (fp, "   @   ");
      else if (board [i] == White) fprintf (fp, "   O   ");
      else fprintf(fp, "%d  ", board [i]);
    }
    fprintf (fp, "    \\  ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "   -    ");
    fprintf (fp, "/ \n");
    fprintf(fp,"       ");
    for (i = 0; i < dxBoard - 2; i++) fprintf (fp, "   %-3c  ", 'A' + i + (i > 7));
    fprintf (fp, "\n");
    fprintf (fp, "hash = %llu\n", hash);
  }
  
  void save (FILE * fp) {
    fprintf (fp, "%d\n", NbMovesPlayed);
    for (int i = 0; i < NbMovesPlayed; i++)
      fprintf (fp, "%d %d ", Moves [i], colorMove [i]);
    fprintf (fp, "\n");
  }

  void save (const char * name) {
    FILE * fp = fopen (name, "w");
    save (fp);
    fclose (fp);
  }
  
  void load (FILE * fp) {
    init ();
    int nb = 0;
    fscanf (fp, "%d", &nb);
    for (int i = 0; i < nb; i++) {
      fscanf (fp, "%d", &Moves [i]);
      fscanf (fp, "%d", &colorMove [i]);
      joue (Moves [i], colorMove [i]);
    }
  }

  void load (const char * name) {
    FILE * fp = fopen (name, "r");
    load (fp);
    fclose (fp);
  }
  
  bool terminal () {
    return gameOver ();
  }

  int score () {
    calculeScores (false);
    if (scorePlayer [White] > scorePlayer [Black])
      return 1;
    return 0;
  }

  float scoreWhite () {
    calculeScores (false);
    return scorePlayer [White] - scorePlayer [Black];
  }

  int opponent (int joueur) {
    if (joueur == White)
      return Black;
    return White;
  }

  /*
  int playout (int joueur) {
    return fastPlayout (joueur);
    Move listeCoups [MaxLegalMoves];
    while (true) {
      int nb = legalMoves (joueur, listeCoups);
      if (nb == 0) {
	if (joueur == Black)
	  return 1;
	else
	  return 0;
      }
      int n = rand () % nb;
      play (listeCoups [n]);
      if (length >= MaxPlayoutLength - 20) {
	return 0;
      }
      joueur = opponent (joueur);
    }
  }  
  */

  float discountedPlayout (int joueur, int maxLength = MaxPlayoutLength - 20) {
    Move listeCoups [MaxLegalMoves];
    while (true) {
      if (terminal ())
	return score ();
      int nb = legalMoves (joueur, listeCoups);
      if (nb == 0) {
	if (joueur == Black)
	  return 1.0 / (length + 1);
	else
	  return -1.0 / (length + 1);
      }
      int n = rand () % nb;
      play (listeCoups [n]);
      if (length >= maxLength) {
	return 0;
      }
      joueur = opponent (joueur);
    }
  }

  bool legalMove (Move m) {
    if (interMove [m.inter] == passe)
      return true;
    if (board [interMove [m.inter]] != Empty)
      return false;
    return legalMove (interMove [m.inter], m.color);
  }

  bool legalMoveSuperKo (Move m) {
    if (interMove [m.inter] == passe)
      return true;
    if (board [interMove [m.inter]] != Empty)
      return false;
    return legalMoveSuperKo (interMove [m.inter], m.color);
  }

  void play (Move m) {
    joue (interMove [m.inter], m.color);
    turn = opponent (m.color);
    if (length < MaxPlayoutLength) {
      rollout [length] = m;
      length++;
    }
  }

  void play (int move) {
    // remove HashKo
    // do not modify hash before joue because joue stores hash in HashHistory
    // and get superko wrong in this case
    //if (ko > 0)
    //hash = hash ^ HashKo [0] [ko];
    if (move == 361) {
      //fprintf (stderr, "joue (passe)\n");
      joue (passe, turn);
    }
    else
      joue (interMove [move], turn);
    // add HashKo
    // pb HashHistory do not use
    //if (ko > 0)
    //hash = hash ^ HashKo [0] [ko];
    //fprintf (stderr, "apres joue dans play, ");
    if (length < MaxPlayoutLength) {
      Move m;
      m.inter = move;
      m.color = turn;
      rollout [length] = m;
      length++;
    }
    turn = opponent (turn);
    // no positional superko
    // hash = hash ^ HashTurn;
  }

  void playColor (int move, int color) {
    // remove HashKo
    // do not modify hash before joue because joue stores hash in HashHistory
    // and get superko wrong in this case
    //if (ko > 0)
    //hash = hash ^ HashKo [0] [ko];
    if (move == 361) {
      //fprintf (stderr, "joue (passe)\n");
      joue (passe, turn);
    }
    else {
      turn = color;
      joue (interMove [move], color);
    }
    // add HashKo
    // pb HashHistory do not use
    //if (ko > 0)
    //hash = hash ^ HashKo [0] [ko];
    //fprintf (stderr, "apres joue dans play, ");
    if (length < MaxPlayoutLength) {
      Move m;
      m.inter = move;
      m.color = color;
      rollout [length] = m;
      length++;
    }
    turn = opponent (color);
    // no positional superko
    // hash = hash ^ HashTurn;
  }

  const static int MaxSGFCommandSize = 1000;

  int ReadSgfBracketed (FILE *SgfFile, char *InsideBracket) {
    int i;
    char c; 
    
    if (fscanf (SgfFile,"%c",&c)==-1) return 0;
    i=0;
    while (c!=']') {
      InsideBracket [i]=c;
      i++;
      if (fscanf (SgfFile,"%c",&c)==-1) 
	return 0;
      if (c == '\\') {
	InsideBracket [i]=c;
	if (fscanf (SgfFile,"%c",&c)==-1) 
	  return 0;
	InsideBracket [i]=c;
	if (fscanf (SgfFile,"%c",&c)==-1) 
	  return 0;
      }
      if (i >= MaxSGFCommandSize)
	return 0;
    }
    InsideBracket[i]='\0';
    return 1;
  }
  
  int ReadSgfCommand (FILE *SgfFile, char *Command, char *InsideBracket) {
    int i;
    char c; 
    
    Command[0]='\0';
    if (fscanf (SgfFile,"%c",&c)==-1) return 0;
    // fin du probleme
    if (c==')') { Command[0]=')'; return 0; }
    if (c=='\n') {
      if (fscanf (SgfFile,"%c",&c)==-1) return 0;
      if (c==')') { Command[0]=')'; return 0; }
    }
    i=0;
    while ((c!='[')&&(c!=')')) {
      if ( ((c>='A') && (c<='Z')) || ((c>='a') && (c<='z')) ) {
	Command[i]=c;
	i++;
      }
      if (fscanf (SgfFile,"%c",&c)==-1) return 0;
    }
    Command[i]='\0';
    if (c==')') return 0;
    return ReadSgfBracketed(SgfFile, InsideBracket);
  }
  
  int loadSGF (FILE * SgfFile) {
    char x, y;
    char lastCommand [MaxSGFCommandSize], Command [MaxSGFCommandSize], InsideBracket [MaxSGFCommandSize];

    int res = 0;
    winner = 'U';
    if (SgfFile!=NULL) {
      bool bypass = false;
      komi [White] = 7.5;
      startGame = 0;
      lastCommand [0] = '\0';
      while (ReadSgfCommand(SgfFile, Command, InsideBracket)) {
	if (bypass)
	  continue;
	/* if there is no command, take the last command
	 * else update the last command
	 */
	if (Command [0] == '\0')
	  strcpy (Command, lastCommand);
	else
	  strcpy (lastCommand, Command);

	if (false)
	  fprintf (stderr, "%s %s, ", Command, InsideBracket);
	
	if (!strcmp(Command,"AB")) {
          handicap = true;
	  if (sscanf (InsideBracket, "%c%c", &x, &y) == 2)
	    if ((x<'t')&&(x>='a')&&(y<'t')&&(y>='a')) {
	      joue (interMove [x - 'a' + Dx * (y - 'a')], Black);
	      length++;
	      val [length - 1] = -1.0;
	      points [length - 1] = 0.0;
	    }
	  //startGame = length;
	}
	else if (!strcmp(Command,"AW")) {
	  if (sscanf(InsideBracket,"%c%c",&x,&y)==2)
	    if ((x<'t')&&(x>='a')&&(y<'t')&&(y>='a')) {
	      joue (interMove [x - 'a' + Dx * (y - 'a')], White);
	      length++;
	      val [length - 1] = -1.0;
	      points [length - 1] = 0.0;
	    }
	  //startGame = length;
	}
	else if (!strcmp(Command,"RE")) {
	  if (sscanf(InsideBracket,"%c",&x)==1) {
	    if (x == 'W')
	      winner = 'W';
	    else if (x == 'B')
	      winner = 'B';
	  }
	}
	else if (!strcmp(Command,"KM")) {
	  if (sscanf(InsideBracket,"%f",&komi [White])==1) {
	    //fprintf (stderr, "\nKM[%s]\n", InsideBracket);
	    //fprintf (stderr, "komi = %2.1f\n", komi [White]);
	  }
	}
	else if (!strncmp(Command,"W", 1) && (strlen (Command) == 1)) {
	  if (res > -1) {
	    // no move = pass move
	    if (strlen(InsideBracket) < 2) {
	      joue (passe, White);
	      length++;
	      val [length - 1] = -1.0;
	      points [length - 1] = 0.0;
	    }
	    else if (sscanf(InsideBracket,"%c%c",&x,&y)==2) {
	      if ((x<'t')&&(x>='a')&&(y<'t')&&(y>='a')) {
		int inter = interMove [x - 'a' + Dx * (y - 'a')];
		if (board [inter] != Empty) {
		  fprintf (stderr, "\n\nErreur sgf W[%c%c]\n\n", x, y);
		  print (stderr);
		  res = -1;
		}
		if (!legalMove (inter, White)) {
		  //print (stderr);
		  fprintf (stderr, "\n\nErreur legalMove sgf W[%c%c]\n\n", x, y);
		  res = -1;
		}
		if (res > -1) {
		  joue (inter, White);
		  length++;
		  val [length - 1] = -1.0;
		  points [length - 1] = 0.0;
		}
	      }
	      // pass move
	      else if ((x=='t') && (y=='t')) {
		joue (passe, White);
		length++;
		val [length - 1] = -1.0;
		points [length - 1] = 0.0;
	      }
	    }
	  }
	}
	else if (!strncmp(Command,"B", 1) && (strlen (Command) == 1)) {
	  if (res > -1) {
	    // no move
	    if (strlen(InsideBracket) < 2) {
	      joue (passe, Black);
	      length++;
	      val [length - 1] = -1.0;
	      points [length - 1] = 0.0;
	    }
	    else if (sscanf(InsideBracket,"%c%c",&x,&y)==2) {
	      if ((x<'t')&&(x>='a')&&(y<'t')&&(y>='a')) {
		//if 
		int inter = interMove [x - 'a' + Dx * (y - 'a')];
		if (board [inter] != Empty) {
		  fprintf (stderr, "\n\nErreur sgf B[%c%c]\n\n", x, y);
		  res = -1;
		}
		if (!legalMove (inter, Black)) {
		  //print (stderr);
		  fprintf (stderr, "\n\nErreur legalMove sgf B[%c%c]\n\n", x, y);
		  print (stderr);
		  res = -1;
		}
		if (res > -1) {
		  joue (inter, Black);
		  length++;
		  val [length - 1] = -1.0;
		  points [length - 1] = 0.0;
		}
		//if (length == 15)
		//print (stderr);
	      }
	      // pass move
	      else if ((x=='t') && (y=='t')) {
		joue (passe, Black);
		length++;
		val [length - 1] = -1.0;
		points [length - 1] = 0.0;
	      }
	    }
	  }
	}
	else if (!strncmp(Command,"SZ", 2)) {
	  int sz = 19;
	  int res = sscanf (InsideBracket, "%d", &sz);
	  //fprintf (stderr, "sz = %d\n", sz);
	  if (sz != 19)
	    bypass = true;
	  init ();
	}
	else if (!strncmp(Command,"C", 1)) {
	  int i = 0;
	  float v = 0.0, pts = 0.0, v1, v2;
	  if (length > 0) {
	    int res = sscanf (InsideBracket, "%f %f %f %f", &v, &v1, &v2, &pts);
	    //fprintf (stderr, "%d, %f, ", length, v);
	    if (res == 4) {
	      val [length - 1] = v;
	      points [length - 1] = pts;
	    }
	  }
	}
      }
    }
    if (length > lengthMax) {
      lengthMax = length;
      fprintf (stderr, "Longest sgf game = %d\n", lengthMax);
    }
    if (res == -1)
      return -1;
    // si il peut encore y avoir des problemes ensuite
    if (Command[0]==')') return 1;
    // sinon c'est qu'on est a la fin du fichier
    return 0;
  }

  bool lireSGF (char * filename) {
    FILE * fp = fopen (filename, "r");
    if (fp == NULL)
      return false;
    loadSGF (fp);
  }
};

Board board;
