#define LIMIT 256 // 定義一次 user input 最多的 command 數量 (e.g.: cat 為一個 command)
#define MAXLINE 1024 // 定義一次 user input command 的最大字數
#define TRUE 1
#define FALSE !TRUE

// ysh pid, pgid, terminal modes (for shell initialization)
static pid_t YSH_PID;
static pid_t YSH_PGID;
static int YSH_IS_INTERACTIVE;
static struct termios YSH_TMODES;

static char* currentDirectory;

struct sigaction signal_child_dead;
struct sigaction signal_interupt;

// for signal repeat prompt shell prifix
int no_reprint_prmpt;
pid_t pid;
extern char** environ;

void signalHandler_child_dead(int p);
void signalHandler_interupt(int p);

int changeDirectory(char * args[]);
void initialization();
void shellPrompt();
void executeProgram(char **args, int background);
void redirectHandler(char * args[], char* inputFile, char* outputFile, int option);
void pipeHandler(char * args[]);
int commandHandler(char * args[]);
int manageEnviron(char * args[], int option);
