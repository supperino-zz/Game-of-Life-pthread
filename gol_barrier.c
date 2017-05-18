/*
* The Game of Life
*
* a cell is born, if it has exactly three neighbours
* a cell dies of loneliness, if it has less than two neighbours
* a cell dies of overcrowding, if it has more than three neighbours
* a cell survives to the next generation, if it does not die of loneliness
* or overcrowding
*
* In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
* The game plays a number of steps (given by the input), printing to the screen each time.  'x' printed
* means on, space means off.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#define THREADS_NUMBER 2

typedef unsigned char cell_t;

int size;
int steps, k;
int num_threads, n;

cell_t ** prev, ** next, ** tmp;
pthread_barrier_t barrier;

cell_t ** allocate_board (int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  int i;
  for (i=0; i<size; i++)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board (cell_t ** board, int size) {
  int     i;
  for (i=0; i<size; i++)
  free(board[i]);
  free(board);
}

/* print the life board */
void print (cell_t ** board, int size) {
  int i, j;
  /* for each row */
  for (j=0; j<size; j++) {
  /* print each column position... */
    for (i=0; i<size; i++)
      printf ("%c", board[i][j] ? 'x' : ' ');
    /* followed by a carriage return */
    printf ("\n");
  }
}

/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t ** board, int size, int i, int j) {
  int k, l, count=0;

  int sk = (i>0) ? i-1 : i;
  int ek = (i+1 < size) ? i+1 : i;
  int sl = (j>0) ? j-1 : j;
  int el = (j+1 < size) ? j+1 : j;

  for (k=sk; k<=ek; k++)
    for (l=sl; l<=el; l++)
      count+=board[k][l];
  count-=board[i][j];

  return count;
}

void *play (void* arg) {
  /* for each cell, apply the rules of Life */
  int thread_id = *((int *) arg);
  int a;

  while (k < steps) {

    for (int i=0; i<n; i++)
        for (int j=0; j<n; j++) {
        a = adjacent_to (prev, size, i+(thread_id*n), j+(thread_id*n));
        if (a == 2) next[i+(thread_id*n)][j+(thread_id*n)] = prev[i+(thread_id*n)][j+(thread_id*n)];
        if (a == 3) next[i+(thread_id*n)][j+(thread_id*n)] = 1;
        if (a < 2) next[i+(thread_id*n)][j+(thread_id*n)] = 0;
        if (a > 3) next[i+(thread_id*n)][j+(thread_id*n)] = 0;
      }

    // Barreira pra todas as threads terminarem de processar
    pthread_barrier_wait(&barrier);

    // Uma única thread executa o final do step
    if(thread_id == 0) {

      tmp = next;
      next = prev;
      prev = tmp;
      k++;

      #ifdef DEBUG
      printf("%d ----------\n", k + 1);
      print (next,size);
      #endif
    }

    // Barreira para esperarem o final do step
    pthread_barrier_wait(&barrier);
    
  }

  pthread_exit(NULL);
}


/* read a file into the life board */
void read_file (FILE * f, cell_t ** board, int size) {
  int i, j;
  char  *s = (char *) malloc(size+10);

  /* read the first new line (it will be ignored) */
  fgets (s, size+10,f);

  /* read the life board */
  for (j=0; j<size; j++) {
    /* get a string */
    fgets (s, size+10,f);
    /* copy the string to the life board */
    for (i=0; i<size; i++)
    board[i][j] = s[i] == 'x';
  }
}

int main (int argc, char ** argv) {

  // Ler informações do arquivo input
  FILE    *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);
  prev = allocate_board (size);
  read_file (f, prev,size);
  fclose(f);
  next = allocate_board (size);

  // Debug print
  #ifdef DEBUG
  printf("Initial:\n");
  print(prev,size);
  #endif  

  // Número de threads pelo argumento
  if (argc!=2) {
    num_threads = THREADS_NUMBER;
  } else {
    num_threads = atoi(argv[1]);
  }

  // Inicializar threads
  pthread_barrier_init(&barrier, NULL, num_threads);
  pthread_t threads[num_threads];
  n = size/num_threads;

  for (int i = 0; i < num_threads; ++i)
  { 
    int *arg = malloc(sizeof(*arg));
    *arg = i;
    pthread_create(&threads[i], NULL, play, arg);
  }

  for (int i = 0; i < num_threads; ++i)
  {
    pthread_join(threads[i], NULL);
  }

  // Result print
  #ifdef RESULT
  printf("Final:\n");
  print (prev,size);
  #endif

  free_board(prev,size);
  free_board(next,size);
}