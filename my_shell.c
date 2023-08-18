#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int pggid;
/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void handle_sigstp(int sig){

	kill(pggid, SIGKILL);
}

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	int arr[64]={0};

	while(1) {		
		/* BEGIN: TAKING INPUT */
		struct sigaction sa;
		sa.sa_handler = &handle_sigstp;
		sa.sa_flags = SA_RESTART;
		sigaction(SIGINT, &sa, NULL);
		int k;
   		for(k=0; k<64; k++){
   			if(arr[k]){
   				if(waitpid(arr[k],NULL, WNOHANG)>0){
   				printf("Shell: Background process finished \n");
   				arr[k] = 0;
   				}  				
   			}  				
   		}
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
   
       		//do whatever you want with the commands, here we just print them
		if(tokens[0] == NULL){
			continue;
		}
		//check for total number of tokens
   		int len = 0;
   		while(1){
   		if(tokens[len] == NULL)
   			break;
   		else 
   			len++;
   		}
   		if(strcmp("exit", tokens[0]) == 0){
   			int k;
   			for(k=0; k<64; k++){
   				if(arr[k]){
   				kill(arr[k],SIGKILL); 
   				waitpid(arr[k],NULL, 0);
   				}	
   			} 			
			return 0;
		}		
		//check for builtin 
		if(strcmp("cd", tokens[0]) == 0){
			int err;
			err = chdir(tokens[1]);
			if(err == -1){
			 printf("Shell:Incorrect command \n");
			}
			 continue;
		} else {
			int pid = fork();
			if(pid < 0){
			
			} else if (pid == 0){
				//check for background process			
				if(!strcmp("&",tokens[len-1]))
					tokens[len-1] = NULL;			
				execvp(tokens[0], tokens);
				printf("%s: command not found\n", tokens[0]);
			} else {
				setpgid(pid, pid);				
				if(strcmp("&",tokens[len-1])){
				pggid = pid;
				waitpid(pid, NULL, 0);	
				} else {
				         for(int p=0; p<64; p++){
				        	if(!arr[p]){
						  arr[p] = pid;
						  break;
						}
					}
				}
			}
		}

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
