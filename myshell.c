#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>

bool checkcommand(char *s){
	// check if command is in the list of all possible commands
	char *validlist[] = {"start", "wait", "run", "kill", "continue", "stop"};
	int len = sizeof(validlist)/sizeof(validlist[0]);
	int i;
	bool flag = false;
	for(i=0; i<len; i++){
		if(strcmp(validlist[i], s)==0)
			flag = true;	
	}
	return flag;
}

void startprocess(char *args[]){
	// starting a process without parent waiting
	pid_t pid;
	if ((pid = fork()) < 0){
		printf("Fork failed\n");
		exit(1);
	}
	else if (pid == 0){
		if (execvp(*args, args) < 0){ // execute command
			printf("Could not execute command: %s\n",*args);
			exit(1);
		}
	}
	else{
		printf("myshell: process %d started\n",(int)pid);
	}
}

void waitprocess(){
	// waiting for a child process to finish
	int status;
	pid_t tpid = wait(&status);
	if (tpid == -1 && errno == ECHILD){
		printf("myshell: no processes left\n");
	}
	else if (WIFEXITED(status)) {
		int value = WEXITSTATUS(status);
		printf("myshell: process %d exited normally with status %d\n", (int)tpid, value);
	}
	else if (WIFSIGNALED(status)){ // Child is terminated by a signal
		int sig_no = WTERMSIG(status);
		printf("myshell: process %d exited abnormally with signal %d: %s\n", (int)tpid, sig_no, strsignal(sig_no));
	}
}

void killprocess(int pid){
	int s;
	s = kill(pid, 9);
	if(s==0){
		printf("myshell: process %d killed\n",pid);
	}
	else {
		printf("myshell: kill process failed with: %s\n", strerror(errno));
	}
}

void continueprocess(int pid){
	int s;
	s = kill(pid, SIGCONT);
	if(s==0){
		printf("myshell: process %d continued\n",pid);
	}
	else {
		printf("myshell: continue process failed with: %s\n", strerror(errno));
	}
}


void stopprocess(int pid){
	int s;
	s = kill(pid, SIGSTOP);
	if(s==0){
		printf("myshell: process %d stopped\n",pid);
	}
	else {
		printf("myshell: stop process failed with: %s\n", strerror(errno));
	}
}


void runprocess(char *args[]){
	pid_t pid;
	int status;
	if ((pid = fork()) < 0){
		printf("Fork failed\n");
		exit(1);
	}
	else if (pid == 0){
		if (execvp(*args, args) < 0){ // execute command
			printf("Could not execute command: %s\n",*args);
			exit(1);
		}
	}
	else{
		if (waitpid(pid, &status, 0) > 0){
			printf("myshell: process %d exited normally with status %d\n", (int)pid, status);
		}
		else {
			printf("myshell: process %d exited abnormally with status %d\n", (int)pid, status);
		}
	}
}

int main(int argc, char *argv[]){
	char buffer[BUFSIZ];
	printf("myshell> ");
	while(fgets(buffer, BUFSIZ, stdin) && strcmp(buffer, "exit\n") != 0 && strcmp(buffer, "quit\n") != 0 ){
		if (strcmp(buffer, "\n") == 0) {
			printf("myshell> ");
			continue;
		}
		buffer[strcspn(buffer, "\n")] = 0;
		char *words[100];
		char *word;
		int index = 0;
		word = strtok(buffer," ");
		if (!checkcommand(word)){
			printf("myshell: unknown command: %s\n",word);
			printf("myshell> ");
			continue;
		}
		// parse commands
		words[0] = word;
		while(word){
			index++;
			word = strtok(NULL," ");
			words[index] = word;
		}
		if (strcmp(words[0], "run") == 0){
			runprocess(words+1);
		}
		else if (strcmp(words[0], "start") == 0){
			startprocess(words+1);
		}
		else if (strcmp(words[0], "wait") == 0){
			waitprocess();
		}
		else if (strcmp(words[0], "stop") == 0){
			stopprocess(atoi(words[1]));
		}
		else if (strcmp(words[0], "continue") == 0){
			continueprocess(atoi(words[1]));
		}
		else if (strcmp(words[0], "kill") == 0){
			killprocess(atoi(words[1]));
		}
		fflush(stdout);
		printf("myshell> ");
		fflush(stdout);
	}
	exit(0);
}
