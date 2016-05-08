#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "ysh.h"
#include "ascii_art.c"

void signalHandler_child_dead(int p){
  // 所有 sigchld (when child process dead) signal 均由此 handler 處理
  // wnohang (with no hang up) 指不會 block parent 等待 child 結束
  // 透過 while loop 持續偵測 child dead signal 且不會使得 parent block
	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
	printf("\n");
}

void signalHandler_interupt(int p){
	// 當 sigint (signal interupt) (ctrl + c) signal 發生在 child 時
	if (kill(pid,SIGTERM) == 0){
		printf("\nProcess %d received a SIGINT signal\n",pid);
		no_reprint_prmpt = 1;
	}else{
		printf("\n");
	}
}

int changeDirectory(char* args[]){
	// cd 之後如果沒指定路徑, 則預設回到家目錄
	if (args[1] == NULL) {
		chdir(getenv("HOME")); // 從環境變數中取得 HOME 的值 (即為家目錄路徑)
		return 1;
	}else
  { // 若有提供路徑給 cd 當參數, 則執行 library chdir 至指定路徑
		if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
            return -1;
		}
	}
	return 0;
}

void executeProgram(char **args, int background)
{
	 int err = -1;

   // fork 回傳 -1 表示 fork 失敗
	 if((pid=fork())==-1){
		 printf("Child process could not be created\n");
		 return;
	 }

	if(pid==0){ // pid = 0, 在 child process 中

    // (we want the parent process to handle them with signalHandler_interupt)
    // 讓 child process 忽略 interupt 的 signal (sigint), sig_ign = signal ignore
		signal(SIGINT, SIG_IGN);

		//  執行指令
		if (execvp(args[0],args)==err){
			printf("Command not found");
			kill(getpid(),SIGTERM); // sigterm = signal terminate 結束 process
		}
	 }

	 // pid != -1 且 pid != 0
   // => parent process

   // 若該指令沒有指定為背景執行, 則 parent 等待 child 結束回傳 sinal
	 if (background == 0){
		 waitpid(pid,NULL,0);
	 }else{
     // 若指令有指定背景執行, 則 parent 不用等待 child 結束回傳 signal
     // 由 signalHandler_child_dead 負責處理 child 回傳事宜
		 printf("Process created with PID: %d\n",pid);
	 }
}

void redirectHandler(char * args[], char* inputFile, char* outputFile, int option){

	int err = -1;

	int fileDescriptor; // between 0 and 19, describing the output or input file

	if((pid=fork())==-1){ // fork 回傳 -1 表示 fork 失敗
		printf("Child process could not be created\n");
		return;
	}

	if(pid==0){
    // pid = 0 表示在 child process 中

		// Option 0: >
		if (option == 0){
      // 新增一個檔案名為 outputFile 的值, 並且將 fileDescriptor 指向該檔案 (write only)
			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
      // 將 fileDescriptor 複製到 process stdout
			dup2(fileDescriptor, STDOUT_FILENO);
			close(fileDescriptor); // 複製完畢, 用完即丟, 此時 process 的 stdout 已經導向 fileDescriptor 指向的檔案

		// Option 1: < and >
		}else if (option == 1){
			// 新增一個檔案名為 outputFile 的值, 並且將 fileDescriptor 指向該檔案 (read only)
			fileDescriptor = open(inputFile, O_RDONLY, 0600);
      // 將 fileDescriptor 複製到 process stdin
			dup2(fileDescriptor, STDIN_FILENO);
			close(fileDescriptor); // 複製完畢, 用完即丟, 此時 process 的 stdin 已經導向 fileDescriptor 指向的檔案

			// 和 option 0 的動作相同
			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fileDescriptor, STDOUT_FILENO);
			close(fileDescriptor);
		}else if (option == 2)
    {
      // 新增一個檔案名為 outputFile 的值, 並且將 fileDescriptor 指向該檔案 (read only)
			fileDescriptor = open(inputFile, O_RDONLY, 0600);
      // 將 fileDescriptor 複製到 process stdin
			dup2(fileDescriptor, STDIN_FILENO);
			close(fileDescriptor); // 複製完畢, 用完即丟, 此時 process 的 stdin 已經導向 fileDescriptor 指向的檔案
    }

    // 執行 user input 指令 (e.g.: cat test > temp)
    // cat test 的 stdout 會導向新產生的 temp file
		if (execvp(args[0],args)==err){
			printf("err");
			kill(getpid(),SIGTERM);
		}
	}
	waitpid(pid,NULL,0);
}

void pipeHandler(char * args[]){

	int filedes[2];
	int filedes2[2];
  // 兩個 file descriptor, 一個給奇數 command 的 pipe, 一個給偶數 command 的 pipe
  // cat a | b | c | d
  //  0  1   2   3   4

	int num_cmds = 0; // 計算 command 數量

	char *command[256]; // 字串的陣列, 用來儲存由 ｜  分割的 command

	pid_t pid;

	int err = -1;
	int end = 0;

  // 各個 loop 的 index 宣告
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;

	// 計算用 | 區隔的 command 的總數
	while (args[l] != NULL){
		if (strcmp(args[l],"|") == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;

  //
	while (args[j] != NULL && end != 1){

    // 將所有 command 依據 | 分割存入 command 字串陣列中
    k = 0;
		while (strcmp(args[j],"|") != 0){
      // cat test | grep a | grep b | grep c, command 取到 cat test
      // 下次: grep a | grep b | grep c, command 取到 grep a
      // 再下次: grep b | grep c, command 取到 grep b
      // 再下次: grep c, command 取到 grep c
			command[k] = args[j];
			j++;
			if (args[j] == NULL){
        // 當沒有 command 了, 就結束吧, 離開分離 command 迴圈
				end = 1;
				k++;
				break;
			}
			k++;
		}
		command[k] = NULL;// 儲存分割 command 的最後補上 NULL 以便之後判斷最後一個 command
		j++;

		if (i % 2 != 0){
			pipe(filedes); // 奇數的 command 使用 filedes 以免連續兩個 pipe 重複使用 file descriptor
		}else{
			pipe(filedes2); // 偶數的 command 使用 filedes2
		}


    /*
      stdin: fd0, stdout: fd1, stderr: fd3
    */
		pid=fork();

		if(pid==-1){ // fork 失敗了
			if (i != num_cmds - 1)
      { // 不是最後一個 command
				if (i % 2 != 0)
        {
					close(filedes[1]); // 此時為奇數的 command 因此關閉其 filedes 的 write end
				}else{
					close(filedes2[1]); // 此時為偶數的 command 因此關閉其 filedes2 的 write end
				}
			}
      // fork 失敗的錯誤訊息
			printf("Child process could not be created\n");
			return;
		}


		if(pid==0){ // fork 成功, 目前在 child process 中

			if (i == 0)
      {
        // 第 0 個 command, 使用屬於偶數的 filedes2 的 write end 取代 stdout
        // stdin 不必更動, 透過使用者輸入當做輸入
				dup2(filedes2[1], STDOUT_FILENO);
			}

			else if (i == num_cmds - 1)
      {
        // 最後一個 command, 因此將 file descriptor 的 read end 取代 stdin
        // stdout 不必更動，直接輸出到螢幕
				if (num_cmds % 2 != 0)
        { // 若最後一個 command 為奇數, 則使用 filedes
					dup2(filedes[0],STDIN_FILENO);
				}else
        { // 若最後一個 command 為偶數, 則使用 filedes2
					dup2(filedes2[0],STDIN_FILENO);
				}

			// If we are in a command that is in the middle, we will
			// have to use two pipes, one for input and another for
			// output. The position is also important in order to choose
			// which file descriptor corresponds to each input/output
			}else
      {
        // 若不是第一個也不是最後一個 command, 則 stdin 和 stdout 都要被取代

				if (i % 2 != 0)
        { // 若為奇數 command

					dup2(filedes2[0],STDIN_FILENO); // 用 filedes2 的 read end 取代自己的 stdin
					dup2(filedes[1],STDOUT_FILENO); // 用 filedes 的 write end 取代自己的 stdout
          // => 前一個 process 的 output (偶數 command) 會接到目前 command 的 input
          // => 自己的 output 會在下次迴圈接到下一個 command 的 input
				}else
        {
          // 若為偶數 command
          // 和上面的奇數動作相反
					dup2(filedes[0],STDIN_FILENO);
					dup2(filedes2[1],STDOUT_FILENO);
				}
			}

      // 執行迴圈中輪到的介於 | 之間的 command, e.g.: execvp("cat", "cat txt")
			if (execvp(command[0],command)==err){
				kill(getpid(),SIGTERM);
        // 若執行有問題, 則透過 kill library 來送 terminate signal 給執行的 process (getpid())
        // kill 只是拿來送 signal 而已, 不見是要砍 process, 應該改成 send_sig 之類的 function name 比較好吧
        // 真的不知道寫 gun c library 的人在想什麼
			}
		}

    // pid != -1, pid != 0 => pid > 0 (parent)

    // file descriptor 已經透過 dup2 複製到指定位置, 因此已經不需要的 descriptor 可以關掉了
		if (i == 0){ // 第 0 個 command 要關 屬於偶數的 filedes2[1] (write end)
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1){ // 最後一個 command 依據奇數偶數關掉 fd[0] (read end)
			if (num_cmds % 2 != 0){
				close(filedes[0]);
			}else{
				close(filedes2[0]);
			}
		}else{ // 中間的 command 前後都要關掉
			if (i % 2 != 0){
				close(filedes2[0]);
				close(filedes[1]);
			}else{
				close(filedes[0]);
				close(filedes2[1]);
			}
		}
    // 等待 第一個 child
		waitpid(pid,NULL,0);

		i++;
	}
}


int commandHandler(char * args[]){
	int i = 0;
	int j = 0;

	int fileDescriptor;
	int standardOut;

	int aux;
	int background = 0;

	char *args_aux[256]={ NULL };

  // 偵測 user command 是否存在 >, <, & 等等需要特殊處理者
  // 並且用另一個 buffer 區分左右兩邊
	while ( args[j] != NULL){
		if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0)){
			break;
		}
		args_aux[j] = args[j]; // 儲存指令到 <, >, & 為止
		j++;
	}

	// 若指令為 exit 則結束程式
	if(strcmp(args[0],"exit") == 0) exit(0);

	// 若指令為 pwd
 	else if (strcmp(args[0],"pwd") == 0){
		if (args[j] != NULL){ // 若有 <, >, & 存在 pwd 的後面

			// 若 pwd 指令被 redirect (>), 且有提供要 redirect 的檔案名稱 (args[j+1])
			if ( (strcmp(args[j],">") == 0) && (args[j+1] != NULL) )
      {
        // 產生一個名稱為 args[j+1] 的值的檔案, 並且用 fileDescritor 來指向他
				fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600);

        // 複製一份 stdout 至 standardOUT, 以便將來回復 stdout 可以使用
				standardOut = dup(STDOUT_FILENO);

        // dup2(source, destination)
				dup2(fileDescriptor, STDOUT_FILENO);
        // 將 fileDescriptor 複製到 stdout (若成功則 stdout 會關閉)
        // 因此 process 的 stdout 會變成寫入 fileDescriptor 指定的檔案

				close(fileDescriptor);
        // fd 被複製完了 就關掉吧
        // 現在 stdout 已經變成 fileDescriptor 的複製品惹 他不需要存在了

        // stdout 的 printf 會被導向至 fileDescriptor 指向的檔案
				printf("%s\n", getcwd(currentDirectory, 1024));

				dup2(standardOut, STDOUT_FILENO);
        // 將剛剛儲存的 standardOut fd 複製到 process stdout fd
        // 因此 process 回復預設狀態, stdout 會噴到螢幕上
			}
		}else{
      // 印出目前路徑
			printf("%s\n", getcwd(currentDirectory, 1024));
		}
	}

 	// 若指令為 clear 則執行 system library clear (clear the terminal screen)
	else if (strcmp(args[0],"clear") == 0) system("clear");

	// 若指令為 cd 則執行 changeDirectory function call
	else if (strcmp(args[0],"cd") == 0) changeDirectory(args);

	// 'setenv' command to set environment variables
	else if (strcmp(args[0],"export") == 0) manageEnviron(args,1);
	// 'unsetenv' command to undefine environment variables
	else if (strcmp(args[0],"unset") == 0) manageEnviron(args,2);

	else{
		// 除了上述之外的指令則執行 binary

    // 持續循環分割的指令們 (index i)
		while (args[i] != NULL && background == 0)
    {
      // 若發現指令中有 & 存在, 則設定 background = 1, 離開 while 迴圈
			if (strcmp(args[i],"&") == 0)
      {
				background = 1;
			}else if (strcmp(args[i],"|") == 0)
      {
        // 若發現指令中有 | 存在, 則透過 pipeHandler 處理
				pipeHandler(args);
				return 1;

			}else if (strcmp(args[i],"<") == 0)
      {
        // 若指令中有 < 存在

				aux = i+1; // 指令 < 的下一個分割指令 (index i+1)

        // 檢查指令格式, 若指令 < 的下一個, 下兩個, 下三個分割指令為空
        // e.g.: cat < text1 > text2
        // 取得 text1 的內容當做 stdin 給 cat, 再把 text2 當做 cat 的 stdout
				if (args[aux] == NULL)
        {
					printf("Not enough input arguments\n");
					return -1;
				}else
        {
          if(args[aux+1] == NULL) // for something like e.g.: cat < text
          {
            redirectHandler(args_aux, args[aux], NULL, 2);
            return 1;
          }

          // 若指令 < 的下兩個指令 不為 > (非 cat < a > b 格式)
					if (strcmp(args[aux+1],">") != 0)
          {
						printf("Usage: Expected '>' and found %s\n",args[aux+1]);
						return -2;
					}
				}

        // 格式正確, 執行 redirectHandler function 處理
        // args_aux 為 <, >, |, & 這些指令左邊的指令分割集合 (e.g.: cat test | less 中的 cat test)
        // args[i] 為 <, >, |, & (暫稱特殊指令)
        // args[i+1] 為特殊指令右側一個分割指令 (e.g.: cat test | less 中的 less)
				redirectHandler(args_aux,args[i+1],args[i+3],1);
        // void redirectHandler(char * args[], char* inputFile, char* outputFile, int option)
				return 1;

			}else if (strcmp(args[i],">") == 0){
        // 若指令中有 > 存在

        // 檢查指令格式, 若指令 > 右側沒東西則格式錯誤
				if (args[i+1] == NULL){
					printf("Not enough input arguments\n");
					return -1;
				}

				printf("%s\n", args[i+1]);

        // 格式正確, 執行 redirectHandler function 處理
        // args_aux 為 <, >, |, & 這些指令左邊的指令分割集合 (e.g.: cat test | less 中的 cat test)
        // args[i] 為 <, >, |, & (暫稱特殊指令)
        // args[i+1] 為特殊指令右側一個分割指令 (e.g.: cat test | less 中的 less)
        // 在此為 stdout 被取代, 因此只需要 output file, 不需要 input file
				redirectHandler(args_aux,NULL,args[i+1],0);
        // void redirectHandler(char * args[], char* inputFile, char* outputFile, int option)
				return 1;
			}
			i++; // while loop
		}

    // 若非 cd, clear 等等 library call, 也沒有 <, >, |, & 等特殊指令則透過 executeProgram 執行
    // background 表示是否為背景執行, background = 1: 背景執行, background = 0: 前景執行
		executeProgram(args_aux,background);
	}
return 1;
}

int manageEnviron(char * args[], int option){
	char **env_aux;
	switch(option){
		// Case 'setenv': we set an environment variable to a value
		case 1:
			if((args[1] == NULL) && args[2] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}

			// We use different output for new and overwritten variables
			if(getenv(args[1]) != NULL){
				printf("%s", "The environment variable has been overwritten\n");
			}else{
				printf("%s", "The environment variable has been created\n");
			}

			// If we specify no value for the variable, we set it to ""
			if (args[2] == NULL){
				setenv(args[1], "", 1);
			// We set the variable to the given value
			}else{
				setenv(args[1], args[2], 1);
			}
			break;
		// Case 'unset': we delete an environment variable
		case 2:
			if(args[1] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}
			if(getenv(args[1]) != NULL){
				unsetenv(args[1]);
				printf("%s", "The environment variable has been erased\n");
			}else{
				printf("%s", "The environment variable does not exist\n");
			}
		break;


	}
	return 0;
}

void initialization(){
        // 取得呼叫該 function 的 process 的 process id
        YSH_PID = getpid();
        // 查看目前的 file descriptor 是否為 terminal(tty)
        YSH_IS_INTERACTIVE = isatty(STDIN_FILENO);

    // if = 1: fd 為 terminal 沒錯, if = 0: 不是 terminal(tty)
		if (YSH_IS_INTERACTIVE) {

			// tcgetpgrp: 透過 file descriptor 來取得 foreground process group id
      // (c: current, grp: group, p: process)
      // getpgrp: 取得呼叫該 function 的 process 的 process group id
			while (tcgetpgrp(STDIN_FILENO) != (YSH_PGID = getpgrp())){
        kill(YSH_PID, SIGTTIN);
      }

	    // 透過進階的 sigaction libray 定義 signal handler for SIGCHILD and SIGINT
      // 取代單純的 signal library
			signal_child_dead.sa_handler = signalHandler_child_dead;
			signal_interupt.sa_handler = signalHandler_interupt;
			sigaction(SIGCHLD, &signal_child_dead, 0);
			sigaction(SIGINT, &signal_interupt, 0);

      // 設定 shell process 的 process group id (新的 process group)
      // 因為是第一個 => process group leader
			setpgid(YSH_PID, YSH_PID);
			YSH_PGID = getpgrp(); // 取得設定的新 process group id

			if (YSH_PID != YSH_PGID) {
					printf("Error, the shell is not process group leader");
          // 一開始沒有 process group id, 則 setpgid 預設 pid 為新的 process group id
					exit(EXIT_FAILURE);
			}

      // 讓參數中的 process group id 成為 foreground process group
      // 使得該 process 在 terminal console 顯示
			tcsetpgrp(STDIN_FILENO, YSH_PGID); // stdin 為 terminal 的 file descriptor

      // 取得 file descriptor 指向的檔案(stdin: shell)的參數, 屬性等資料, 並存到後面的 termios structure 中
			tcgetattr(STDIN_FILENO, &YSH_TMODES);

        }else{
                printf("Could not make the shell interactive.\n");
                exit(EXIT_FAILURE);
        }
}

void shellPrompt(){
	char hostname[1204] = "";
  char* username = getenv("LOGNAME");
	gethostname(hostname, sizeof(hostname));

  printf("%s\n", getcwd(currentDirectory, 1024));
	printf("%s@%s> ", username, hostname);
}

int main(int argc, char *argv[], char ** envp) {
	char line[MAXLINE]; // 使用者輸入指令字串用
	char* tokens[LIMIT]; // 分割指令字串字串用
	int numTokens; // 分割指令字串後的片段數量

	no_reprint_prmpt = 0;

	pid = -10; // initialize pid

	initialization();
  print_ascii_art();
	environ = envp;

  // 目前所在的路徑
  currentDirectory = (char*) calloc(1024, sizeof(char));

  // 持續提供使用者輸入 while loop
  // TRUE 定義為 1
	while(TRUE){
		// We print the shell prompt if necessary
		if (no_reprint_prmpt == 0) shellPrompt();
		no_reprint_prmpt = 0;

		// 初始化 將存放 user command 的 buffer 初始化為 \0
		memset ( line, '\0', MAXLINE );

		// 讀取使用者輸入 (stdin) 放在 line 中
		fgets(line, MAXLINE, stdin);

		// 若輸入值為空則繼續
		if((tokens[0] = strtok(line," \n\t")) == NULL) continue;

		numTokens = 1;

    // 持續對使用者輸入(line)切割, 將切割後的用空白分割的 command 存入 token, 字串的陣列中
		while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;

		commandHandler(tokens);
	}

	exit(0);
}
