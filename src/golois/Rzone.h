
const int MaxRZone = (Dx + 2) * (Dy + 2) + 1;

class Rzone {
 public:
  
  short elt [MaxRZone];
  unsigned int present [1 + MaxRZone / 32];
  
  Rzone () { 
    Init ();
  }
/*   Rzone (const Rzone & z) { */
/*     sizeMax = z.sizeMax; */
/*     elt = new short [sizeMax + 1]; */
/*     present = new unsigned int [1 + sizeMax / 32];  */
/*     memcpy (present, z.present, 1 + sizeMax / 8); */
/*     for (int i = 0; i <= z.elt[0]; i++) */
/*       elt [i] = z.elt [i]; */
/*   } */
/*   ~Rzone () { */
/*     delete [] elt; */
/*     delete [] present; */
/*   } */
  // Initialise the relevancy zone, making it empty
  void Init () {
    elt [0] = 0;
    memset (present, 0, sizeof present);
  }  
  void init () { Init (); }  
  short & operator [] (const int i) {
    return elt [i];
  }  
  Rzone operator + (const Rzone & z) {
    Rzone temp (*this);
    for (int i = 1; i <= z.elt[0]; i++)
      temp.add (z.elt[i]);
    return temp;
  }
  Rzone & operator += (const Rzone & z) {
    for (int i = 1; i <= z.elt[0]; i++)
      add (z.elt[i]);
    return *this;
  }  
  Rzone & operator |= (const Rzone & z) {
    for (int i = 1; i <= z.elt[0]; i++)
      add (z.elt[i]);
    return *this;
  }  
  Rzone & operator += (const int * z2) {
    for (int i = 1; i <= z2 [0]; i++)
      add (z2[i]);
    return *this;
  }  
  Rzone & operator += (const short unsigned int * z2) {
    for (int i = 1; i <= z2 [0]; i++)
      add (z2[i]);
    return *this;
  }  
  Rzone & operator += (const short * z) {
    for (int i = 1; i <= z [0]; i++)
      add (z [i]);
    return * this;
  }  
  Rzone & operator += (const int elt) {
    add (elt);
    return *this;
  }  
  Rzone & operator -= (const Rzone & z) {
    for (int i = 1; i <= z.elt [0]; i++)
      remove (z.elt [i]);
    return *this;
  }  
  Rzone & operator -= (const int elt) {
    remove (elt);
    return *this;
  }  

  // the != operator returns TRUE if Rzone are different
  int operator != ( const Rzone &z ) const {
    if( this->size() != z.size() )
      return 1;
    else{
      for( int i=1; i<=z.elt[0]; i++ )
	if( !this->element( z.elt[i] ) )
	  return 1;
    }
    return 0;
  }
  // the & operator performs the intersection of two Rzones
  Rzone & operator &= (const Rzone & z) {
    for (int i = 1; i <= elt [0]; i++)
      if (!z.element(elt [i])) {
	// put 0 in the bit of the element to remove
	present[(elt[i]) >> 5] &= ~(1 << (((unsigned int)elt[i]) & 31));
	// remove the element which is not in z
	elt [i] = elt [elt [0]];
	elt [0]--;
	i--;
      }
    return *this;
  }  
  /* sends back 1 if the intersection is non void */
  int intersection (const Rzone & z) {
    for (int i = 0; i <= MaxRZone / 32; i++)
      if (present [i] & z.present[i])
	return 1;
    return 0;
  }
  // sends back 0 if the elt is not in the zone
  // something >0 otherwise
  int element (const int e) const { 
    return (((present[(e) >> 5]) >> ((e) & 31)) & 1); }
  // append the element elt to the zone
  // if it is not already present
  int add (int e) {
    if (e >= MaxRZone || e < 0)
      fprintf (stderr, "Pb Rzone::add %d\n", e);
    if (!element (e)) {
      //assert (e != 70);
      elt[0]++;
      elt[elt[0]] = e;
      (present[(e) >> 5] |= (1 << ((e) & 31)));
      return 1;
    }
    return 0; 
  }
  void add (const int * z2) {
    for (int i = 1; i <= z2[0]; i++)
      add (z2 [i]);
  }  
  void add (const Rzone & z) {
    for (int i = 1; i <= z.elt[0]; i++)
      add (z.elt[i]);
  }  
  void remove (int e) {
    if (element (e)) {
      // put 0 in the bit of the element to remove
      present [(e) >> 5] &= ~(1 << (((unsigned int)e) & 31));
      // remove the element which is not in z
      for (int i = 1, cont = 1; ((i <= elt [0]) && cont); i++) {
	if (elt [i] == e) {
	  elt [i] = elt [elt [0]];
	  elt [0]--;
	  cont = 0;
	}
      }
    }
  }
  const int size () const {
    return elt [0];
  }
  const int count() const {return size();}
  const int getSize () const {return size();}
  int First (int & i) const {
    i = 1;
    return elt [1];
  }
  int Continue (int i) const { return (i <= elt [0]); }
  int Next (int & i) const { 
    i++;
    //if ((*Indice) <= elt[0])
    return elt [i];
      //return 0;
  }
  int first (int & i) const {
    i = 1;
    return elt [1];
  }
  int cont (int i) const { return (i <= elt [0]); }
  int next (int & i) const { 
    i++;
    //if ((*Indice) <= elt[0])
    return elt [i];
      //return 0;
  }
  void sortMoves (long long unsigned int * scores) { 
    int i, Sorted = 0, tmp;
    while (!Sorted) {
      Sorted=1;
      for (i = 1; i < elt [0]; i++)
	if (scores [elt [i]] < scores [elt [i + 1]]) {
	  Sorted=0;
	  tmp = elt [i];
	  elt [i] = elt [i + 1];
	  elt [i + 1] = tmp;
	}
    }
  }
  void sortMoves (int * scores) { 
    short i, Sorted = 0, tmp;
    while (!Sorted) {
      Sorted = 1;
      for (i = 1; i < elt [0]; i++)
	if (scores [elt [i]] < scores [elt [i + 1]]) {
	  Sorted=0;
	  tmp = elt [i];
	  elt [i] = elt [i + 1];
	  elt [i + 1] = tmp;
	}
    }
  }
  void sortMoves() {		/* sort by its number in the board */
    short i, Sorted=0, tmp;
    while( !Sorted) {
      Sorted=1;
      for( i=1; i <elt[0]; i++ )
	if( elt[i] < elt[i+1] ) {
	  Sorted = 0;
	  tmp= elt[i]; elt[i]=elt[i+1]; elt[i+1]=tmp;
	}
    }
  }        
  int isDisjunct(const Rzone& z) const {
    for(int i=0;i<1 + MaxRZone / 32;i++)
      if((present[i]&z.present[i])!=0)
			return 0;
    return 1;
  }
  void print(FILE *fp) const {
  		int i=0; int inter;
		for (inter = first(i);cont(i);inter = next(i))
		  fprintf(fp,"%i ",inter);
  }

  void printNice(FILE *fp) const {
    fprintf(fp,"[");
    int nr1=1;
    int i=0; int inter;
    for (inter = first(i);cont(i);inter = next(i)) {
      if(nr1)nr1=0; else fprintf(fp,",");
      fprintf(fp,"%i",inter);
    }
    fprintf(fp,"]\n");
  }

  void printGraph(FILE *fp, int dx, int dy) const {
    for(int i=0;i<dx*dy;i++) {
      if(element(i)) fprintf(fp,"x");
      else fprintf(fp,".");
      if(((i+1)%dx)==0) fprintf(fp,"\n");
    }
  }
  void set(int e) { add(e);}
  void reset(int e) { remove(e);}
};

#define allRzone(i,z,elt) for (int i, elt = (z).first (i);\
                                (z).cont (i);\
                                elt = (z).next (i))

#define allpRzone(i,z,elt) for (int i, elt = z->first (i);\
                                z->cont (i);\
                                elt = z->next (i))
