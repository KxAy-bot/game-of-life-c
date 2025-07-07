#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h> 
#endif /* ifdef _WIN32 */

#if defined(__linux__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#define RADIUS 4
#define DEFAULT_SPEED 150000

int ROWS, COLS;

typedef enum State { DEAD, ALIVE } State;

char *buffer;
char *tempBuffer;
unsigned int useconds;
unsigned short fastMode = 0;
unsigned int generationCounter = 0;
unsigned int populationCounter = 0;
unsigned int seed;
unsigned short status = 0;
unsigned short clockStatus = 0;

void handleExit(int sig);
unsigned int convertStringToSeed(char* input);
void showSeedForm();
void checkPopulation();
void handleKeyInput();
void restoreTerminal();
void moveCursor(int row, int col);
int getTerminalSize(int *rows, int *cols);
void setNoEchoInput();
char getKeyPress();
void putCell(int x, int y, State state, char *buffer);
int checkNeighbours(int x, int y);
State getCell(int x, int y);
void initgrid();
void renderGrid();
void clearScreen();
void updateState();
void cleanMemory();
void setSleep(int useconds);
void runSimulation();
void restartSimulation();
void writeSeedToFile();
unsigned int getSeedFromUser();

int main() {
  runSimulation();
  return 0;
}

void putCell(int x, int y, State state, char *buffer) {
  buffer[y * COLS + x] = (char)state;
}

State getCell(int x, int y) {
  x = (x + COLS) % COLS;
  y = (y + ROWS) % ROWS;
  return (State)buffer[y * COLS + x];
}

void initgrid() {
  getTerminalSize(&ROWS, &COLS);
  if (ROWS == -1 || COLS == -1) {
    fprintf(stderr,
            "Errore durante allocazione buffer, probabile errore terminale\n");
    exit(1);
  }
  buffer = (char *)malloc(ROWS * COLS * sizeof(char));
  tempBuffer = (char *)malloc(ROWS * COLS * sizeof(char));
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      putCell(j, i, DEAD, buffer);
      putCell(j, i, DEAD, tempBuffer);
    }
  }
}

void renderGrid() {
  printf("\033[H");
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      if (getCell(j, i) == DEAD)
        putc(' ', stdout);
      else
        printf("\033[32m█\033[0m");
    }
    putc('\n', stdout);
  }
  printf("Seed: %u\n", seed);
  printf("Generazione %d\n", generationCounter);
  printf("Popolazione %d\n", populationCounter);

  if(status){
    printf("Stable");
  }

  fflush(stdout);
}

void setSleep(int useconds) {
  if (useconds == 0)
    return;
#ifdef _WIN32
  Sleep(useconds / 1000);
#endif /* ifdef __windows__ */

#if defined(__APPLE__) || defined(__linux__)
  usleep(useconds);
#endif
}

int checkNeighbours(int x, int y) {
  int counter = 0;

  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (i == 0 && j == 0)
        continue;
      if (getCell(x + j, y + i) == ALIVE)
        counter++;
    }
  }

  return counter;
}

void updateState() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      int neighbours = checkNeighbours(j, i);
      switch (getCell(j, i)) {
      case ALIVE:
        if (neighbours < 2 || neighbours > 3)
          putCell(j, i, DEAD, tempBuffer);
        else
          putCell(j, i, ALIVE, tempBuffer);
        break;
      case DEAD:
        if (neighbours == 3)
          putCell(j, i, ALIVE, tempBuffer);
        else
          putCell(j, i, DEAD, tempBuffer);
        break;
      }
    }
  }

  for (int i = 0; i < ROWS * COLS; i++) {
    buffer[i] = tempBuffer[i];
  }

  generationCounter++;
  checkPopulation();
}

void clearScreen() {
  printf("\033[2J\033[H");
  fflush(stdout);
}

int getTerminalSize(int *rows, int *cols) {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return -1; // errore
  }
  *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  return 0;
#else
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
    return -1; // errore
  }
  *cols = ws.ws_col;
  *rows = ws.ws_row;
  return 0;
#endif
}

void cleanMemory() {
  if (buffer != NULL)
    free(buffer);
  if (tempBuffer != NULL)
    free(tempBuffer);
  printf("\nPulendo le stronzate che mi hai fatto fare....\n");
  // exit(0);
}

void setNoEchoInput(){
  struct termios ttystate;
  tcgetattr(STDIN_FILENO, &ttystate);
  ttystate.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

char getKeyPress(){
  #ifdef _WIN32 
    if(_kbhit()){
      return _getch();
    }
  return 0;
  #else
  char ch = 0;
  read(STDIN_FILENO, &ch, 1);
  return ch;
  #endif
}


void restoreTerminal(){
  struct termios ttystate;
  tcgetattr(STDIN_FILENO, &ttystate); // impostazioni
  ttystate.c_lflag |= ICANON | ECHO; // disattivo echo da terminale
  tcsetattr(STDIN_FILENO, TCSANOW, &ttystate); // applico impostazioni
  
  fcntl(STDIN_FILENO, F_SETFL, 0);
}

void handleExit(int sig){
  writeSeedToFile();
  restoreTerminal();
  cleanMemory();
  exit(0);
}

void handleKeyInput(){
  switch(getKeyPress()){
    case ' ':
      fastMode = !fastMode;
      break;
    case 'r':
      restartSimulation();
      break;
  }

  useconds = fastMode ? (DEFAULT_SPEED / 3) : DEFAULT_SPEED;
}

void checkPopulation(){
  unsigned int counter = 0;
  for(int i = 0; i < ROWS; i++){
    for(int j = 0; j < COLS; j++){
      if(getCell(j, i) == ALIVE) counter++;
    }
  }
  if(populationCounter == counter && status == 0) clockStatus++;
  if(populationCounter != counter) clockStatus = 0;
  if(clockStatus > 3) status = 1;
  else status = 0;

  populationCounter = counter;
}

void runSimulation(){
  seed = getSeedFromUser();
  useconds = DEFAULT_SPEED;
  initgrid();
  clearScreen();

  srand(seed);

  for (int i = 0; i < 20; i++) {
    int x = (COLS / 2 + rand() % (2 * RADIUS + 1) - RADIUS + COLS) % COLS;
    int y = (ROWS / 2 + rand() % (2 * RADIUS + 1) - RADIUS + ROWS) % ROWS;
    putCell(x, y, ALIVE, buffer);
  }

  signal(SIGINT, handleExit);
  setNoEchoInput();

  while (1) {
    renderGrid();
    handleKeyInput();
    updateState();
    setSleep(useconds);
  }
}

void restartSimulation(){
  cleanMemory();
  restoreTerminal();
  runSimulation();
}

void writeSeedToFile(){
  unsigned int count = 0;
  FILE* file = fopen("seeds.txt", "r");

  if(file != NULL){
    char line[64];
    while(fgets(line, sizeof(line), file)){
      count++;
    }
    fclose(file);
  }

  const char *mode = (count >= 100) ? "w" : "a";
  FILE* output = fopen("seeds.txt", mode);

  fprintf(output, "%u\n", seed);

  fclose(output);
}

void showSeedForm() {
  clearScreen();

  int termRows = 24, termCols = 80; // default se ioctl fallisce

  #if defined(__linux__) || defined(__APPLE__)
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
      termRows = w.ws_row;
      termCols = w.ws_col;
  }
#endif

  int width = 50;
  int height = 7;
  int top = (termRows - height) / 2;
  int left = (termCols - width) / 2;

  // disegna riquadro
  moveCursor(top, left);
  printf("╭────────────────────────────────────────────────╮");
  moveCursor(top + 1, left);
  printf("│           🌱 Inserisci un seed manuale         │");
  moveCursor(top + 2, left);
  printf("│      oppure premi INVIO per usarne uno random  │");
  moveCursor(top + 3, left);
  printf("╰────────────────────────────────────────────────╯");
  moveCursor(top + 5, left);
  printf("  Seed: ");

  fflush(stdout);
}

void moveCursor(int row, int col){
  printf("\033[%d;%dH", row, col);
}

unsigned int getSeedFromUser(){
  showSeedForm();
  char input[32];
  if(fgets(input, sizeof(input), stdin)){
    if(input[0] != '\n'){
      input[strcspn(input, "\n")] = '\0';
      return convertStringToSeed(input);
    }
  }
  return (unsigned int)(time(NULL)+time(NULL));
}

unsigned int convertStringToSeed(char* input){
  unsigned int result = 0;

  for(int i = 0; input[i] != '\0' ; i++){
    result = result * 10 + abs((input[i] - 48));
  }

  return result;
}
