#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<signal.h>
#include <time.h>
char directory[200];

void writeInFile()
{
    FILE *pFile;
    char buffer[]="Child process was terminated";
    char location[200+strlen("/log.txt")];
    strcpy(location,directory);//copying the directory to concatenate it with file name
    strcat(location,"/log.txt");//concatenating to get the location of the file
    pFile=fopen(location, "a");//append in the given location
    if(pFile!=NULL)
    {
        time_t t;
        time(&t);
        fprintf(pFile, "LOG: (%.19s) %s\n", ctime(&t),buffer);//.19s is used in printing to remove new line /n
    }
    fclose(pFile);
}

void signalHandler()
{
    waitpid(-1,NULL,WNOHANG); //wait for the process that is done ,WNOHANG flag specifies that waitpid should return immediately instead of waiting
    writeInFile();

}

int checkInput(char* str)
{
    for(int l=0; str[l]!='\n'; l++)//this loop check if user entered anything that isn't spaces or enter
    {
        if(str[l]!=' '  &&    str[l]!='\t')
        {
            return 1;
        }
    }
    return 0;
}

int main()
{
    getcwd(directory,sizeof(directory));//getting directory to write in same file.(without directory the program will write in different directory every cd command)
    signal(SIGCHLD,signalHandler);//when a child process is done, SIGCHLD is sent to the parent process, and signalHandler will be executed
    char str[100];
    while(1)
    {
        printf("Shell > ");
        fgets(str,100,stdin);//scan user input

        if(checkInput(str)==0)//check if user entered correct commands
            continue;

        strtok(str,"\n");//remove \n from str

        char *copy=malloc(sizeof(char)*strlen(str));//Making a copy array to be used instead of original array
        strcpy(copy,str);//copying array
        char *token = strtok(copy, " \t");//tokenizing the input
        if(strcmp(token,"exit")==0)//if the user entered exit then free copy and break the loop
        {
            free(copy);
            break;
        }

        strcpy(copy,str);//copying the original array again to make the argument_list
        token = strtok(copy, " \t");//tokenizing and removing tabs and & and \t
        char* argument_list[200];//will be used in execvp
        int backgroundFlag=0;//flag used to check if the parent will wait for the child or not
        int noOfWords=0;
        while(token)
        {
            if(strcmp(token,"&")==0)//check if user entered &
            {
                backgroundFlag=1;//don't wait
                token=strtok(NULL, " \t");//going to next token without copying &
            }
            else
            {
                if(token[strlen(token)-1]=='&')
                {
                    backgroundFlag=1;//don't wait
                    strtok(token,"&");//remove &
                }
                argument_list[noOfWords]=malloc(sizeof(char)*strlen(token));//allocating space for the string
                strcpy(argument_list[noOfWords],token);//copying it
                token=strtok(NULL, " \t");//going to next token
                noOfWords++;
            }
        }
        free(copy);//copy won't be needed
        argument_list[noOfWords]=NULL;//adding null

        if(strcmp(argument_list[0],"cd")==0)//if the user entered cd
        {
            if(noOfWords==1) // ex : cd
            {
                chdir(getenv("HOME"));//get enviroment variable HOME to return to home directory. if he entered cd only
            }
            else
            {
                chdir(argument_list[1]);//use second string in the array to change directory
            }
        }
        else// user entered a command that isn't cd
        {
            pid_t pid = fork();//creating a process
            if (pid < 0)   /* error occurred */
            {
                printf("Fork failed Unable to create child process.\n");
                return 1;
            }
            else if (pid == 0)   /* child process */
            {
                int x=execvp(argument_list[0],argument_list);//executing
                if(x<0)//user entered a wrong a command
                {
                    printf("Unknown command\n");
                    kill(getpid(),SIGKILL);
                    exit(0);
                }
            }
            else   /* parent process */
            {
                if(backgroundFlag==0)//which means that the user didn't enter & so the parent should wait for the child to be executed
                {
                    waitpid(pid,NULL,0);//wait for the child that has this certain pid till it execute.
                }
            }
        }

    }
    return 0;
}
