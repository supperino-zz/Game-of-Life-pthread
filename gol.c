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

#define THREADS_NUMBER 4

typedef unsigned char cell_t;

int size;
int steps, k;
int num_threads, lines, reminder;

cell_t ** prev, ** next, ** tmp;
pthread_barrier_t barrier;

cell_t ** allocate_board () {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  int i;
  for (i=0; i<size; i++)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board (cell_t ** board) {
  int     i;
  for (i=0; i<size; i++)
  free(board[i]);
  free(board);
}

/* print the life board */
void print (cell_t ** board) {
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
int adjacent_to (cell_t ** board, int i, int j) {
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

/* read a file into the life board */
void read_file (FILE * f, cell_t ** board) {
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

void play (int this_start, int this_end, int thread_id) {
  /* for each cell, apply the rules of Life */
  int a;

  while (k < steps) {

    for (int i=this_start; i<this_end; i++) {
        for (int j=0; j<size; j++) {
          a = adjacent_to (prev, i, j);
          if (a == 2) next[i][j] = prev[i][j];
          if (a == 3) next[i][j] = 1;
          if (a < 2) next[i][j] = 0;
          if (a > 3) next[i][j] = 0;
      }
    }

    // Barreira pra todas as threads terminarem de processar
    pthread_barrier_wait(&barrier);

    // Uma única thread executa o final do step
    if(thread_id == 0) {
      #ifdef DEBUG
      printf("%d ----------\n", k + 1);
      print (next,size);
      #endif
      tmp = next;
      next = prev;
      prev = tmp;
      k++;
    }

    // Barreira para esperarem o final do step
    pthread_barrier_wait(&barrier);
    
  }

  pthread_exit(NULL);
}

void* defineWork(void* arg) {
  int thread_id = *((int *) arg);
  int this_start = thread_id * lines;
  int this_end = this_start + lines;

  // A última thread pega o resto
  if (thread_id == num_threads - 1) {
    this_end+= reminder;
  }

  play(this_start, this_end, thread_id);

}

int main (int argc, char ** argv) {

  // Ler informações do arquivo input
  FILE    *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);
  prev = allocate_board ();
  read_file (f, prev);
  fclose(f);
  next = allocate_board ();

  // Debug print
  #ifdef DEBUG
  printf("Initial:\n");
  print(prev);
  #endif  

  // Número de threads pelo argumento
  if (argc!=2) {
    printf("Não foi definido um valor único de threads, utilizando o padrão: %d.\n", THREADS_NUMBER);
    num_threads = THREADS_NUMBER;
  } else if (atoi(argv[1]) > size){
    printf("Número de threads maior que número de linhas da matriz; criando apenas %d threads.\n", size);
    num_threads = size;
  } else {
    num_threads = atoi(argv[1]);
  }

  // Dividindo trabalho
  lines = size/num_threads; 
  reminder = size%num_threads;

  // Inicializar threads
  pthread_barrier_init(&barrier, NULL, num_threads);
  pthread_t threads[num_threads];

  int k = 0;

  for (int i = 0; i < num_threads; ++i)
  { 
    int *arg = malloc(sizeof(int));
    *arg = i;
    pthread_create(&threads[i], NULL, defineWork, arg);
  }

  for (int i = 0; i < num_threads; ++i)
  {
    pthread_join(threads[i], NULL);
  }

  // Result print
  #ifdef RESULT
  printf("Final:\n");
  print (prev);
  #endif

  pthread_barrier_destroy(&barrier);
  free_board(prev);
  free_board(next);
}
