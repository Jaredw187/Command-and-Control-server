//#include <iostream>
//using namespace std;

// List of includes
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>



#define MAXBUF 1024
#define MAXHOSTNAME 200
#define MAXACTION  10
#define MAXPORT 6

struct eventQ{
    char Cname[MAXHOSTNAME];
    int Cport;
    char Caction[MAXACTION];
    time_t joined;
    struct eventQ *next;
};
struct logQ{
    char message[100];
    struct logQ *next;
};

void logger(char lbuffer[100], struct logQ *Head);

int main(int argc,char* argv[]){
    struct timeval tv;
    struct eventQ *Head;
    Head = (struct eventQ *) malloc (sizeof(struct eventQ));
    Head -> next = NULL;
    struct logQ *lHead;
    lHead = (struct logQ *) malloc (sizeof(struct logQ));
    lHead -> next = NULL;
    char timeArr[30];
    time_t millitime = 0;
    
    int cont = 1, numBits;
    int counter = 0, fd = 0, numConnected = 0;
    socklen_t client;
    char suppliedPort[MAXPORT], currClient[MAXHOSTNAME], buffer[MAXBUF], Action[MAXACTION], lbuffer[100], message[MAXBUF], milliray[MAXBUF];
    sprintf(suppliedPort,"%s",argv[1]);
    int port = atoi(argv[1]);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0){ printf("ERROR establishing socket \n"); return 0;}
    
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    struct sockaddr_in clientInfo;
    client = sizeof(clientInfo);
    if (bind(fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {printf("Binding error (45)\n"); return 0;}
    if (listen(fd, MAXACTION) < 0) {printf("ERROR lsitening \n"); return 0;}
    
    while (cont == 1) {//infinite loop so we can take in requests
        int currConnfd = accept(fd, (struct sockaddr*) &clientInfo, &client);
        if (currConnfd < 0) {printf("accepting error\n"); return 0;}
        numBits = read(currConnfd, buffer, MAXBUF);
        if (numBits < 0) {printf("reading error\n"); return 0;}
        //get client ip and store in currClient
        inet_ntop(AF_INET, &clientInfo.sin_addr.s_addr, currClient, sizeof(currClient));
        
        
        strcpy(Action, buffer);
        
        //creating log string
        memset(lbuffer, 0, sizeof(lbuffer));
        //getting cur time
        gettimeofday(&tv, NULL);
        millitime = tv.tv_sec;
        strftime(timeArr, 30, "%m-%d-%Y  %T.", (const void *)localtime(&millitime));
        memset(milliray, 0, sizeof(milliray));
        sprintf(milliray, "%d", tv.tv_usec);
        
        strcpy(lbuffer, "\"");
        strcat(lbuffer, timeArr);
        strcat(lbuffer, ".");
        strcat(lbuffer, milliray);
        strcat(lbuffer,"\": Recieved a \"#");
        strcat(lbuffer, Action);
        strcat(lbuffer, "\" action from agent \"");
        strcat(lbuffer, currClient);
        strcat(lbuffer, "\"\n");
        
        logger(lbuffer, lHead);
        
        if (strcmp(Action, "JOIN") == 0) {
            int isHere = 0;
            struct eventQ *search;
            search = (struct eventQ *) malloc (sizeof(struct eventQ));
            search = Head;
            
            while(search -> next != NULL && isHere == 0) {
                search = search -> next;
                if (strcmp(search -> Cname, currClient) == 0) {
                    strcpy(message, "$");
                    strcat(message, "ERROR: ALREADY MEMBER");
                    printf("%s\n", message);
                    write(currConnfd, message, strlen(message));//works
                    memset(message, 0, sizeof(message));
                    memset(lbuffer, 0, sizeof(lbuffer));
                    //getting cur time
                    gettimeofday(&tv, NULL);
                    millitime = tv.tv_sec;
                    strftime(timeArr, 30, "%m-%d-%Y  %T.", localtime(&millitime));
                    memset(milliray, 0, sizeof(milliray));
                    sprintf(milliray, "%d", tv.tv_usec);
                    
                    strcpy(lbuffer, "\"");
                    strcat(lbuffer, timeArr);
                    strcat(lbuffer, ".");
                    strcat(lbuffer, milliray);
                    strcat(lbuffer, "\": Responded to agent \"");
                    strcat(lbuffer, currClient);
                    strcat(lbuffer, "\" with \"$ERROR: ALREADY MEMBER\"\n");
                    logger(lbuffer, lHead);
                    isHere = 1;
                    
                }
                
            }
            
            if (isHere == 0) {
                struct eventQ *new;
                new = (struct eventQ *) malloc (sizeof(struct eventQ));
                struct eventQ *search;
                search =(struct eventQ *) malloc (sizeof(struct eventQ));
                search = Head;
                while (search -> next != NULL) {
                    search = search -> next;
                }
                search -> next = new;
                new -> next = NULL;
                int i = 0;
                for(i = 0; i < sizeof(currClient); i++){
                    new -> Cname[i] = currClient[i];
                }
                for (i = 0; i < sizeof(Action); i++) {
                    new -> Caction[i] = Action[i];
                }
                new -> joined = time(0);
                printf("%cOK\n",36);
                strcpy(message, "$");
                strcat(message, "OK");
                write(currConnfd, message, strlen(message)); // works
                memset(lbuffer, 0, sizeof(lbuffer));
                //getting cur time
                gettimeofday(&tv, NULL);
                millitime = tv.tv_sec;
                strftime(timeArr, 30, "%m-%d-%Y  %T.", localtime(&millitime));
                memset(milliray, 0, sizeof(milliray));
                sprintf(milliray, "%d", tv.tv_usec);
                
                strcpy(lbuffer, "\"");
                strcat(lbuffer, timeArr);
                strcat(lbuffer, ".");
                strcat(lbuffer, milliray);
                strcat(lbuffer, "\": Responded to agent \"");
                strcat(lbuffer, currClient);
                strcat(lbuffer, "\" with \"$OK\"\n");
                logger(lbuffer, lHead);
                
            }
        }
        if (strcmp(Action, "LEAVE") == 0) {
            int isHere = 0;
            struct eventQ *search;
            search = (struct eventQ *) malloc (sizeof(struct eventQ));
            struct eventQ *trailer;
            trailer = (struct eventQ *) malloc (sizeof(struct eventQ));
            search = Head;
            trailer = Head;
            while(search -> next != NULL && isHere == 0) {
                search = search -> next;
                if (strcmp(search -> Cname, currClient) == 0) {
                    isHere = 1;
                    trailer -> next = search -> next;
                    if(search != Head) free(search);
                    printf("%cOK\n",36);
                    memset(message, 0, sizeof(message));
                    strcpy(message, "$");
                    strcat(message, "OK");
                    write(currConnfd, message, 3);
                    memset(lbuffer, 0, sizeof(lbuffer));
                    //getting cur time
                    gettimeofday(&tv, NULL);
                    millitime = tv.tv_sec;
                    strftime(timeArr, 30, "%m-%d-%Y  %T.", localtime(&millitime));
                    memset(milliray, 0, sizeof(milliray));
                    sprintf(milliray, "%d", tv.tv_usec);
                    
                    strcpy(lbuffer, "\"");
                    strcat(lbuffer, timeArr);
                    strcat(lbuffer, ".");
                    strcat(lbuffer, milliray);
                    strcat(lbuffer, "\": Responded to agent \"");
                    strcat(lbuffer, currClient);
                    strcat(lbuffer, "\" with \"$OK\"\n");
                    logger(lbuffer, lHead);
                }
                search = trailer;
                trailer = trailer -> next;
                
            }
            if (isHere == 0){
                printf("$ERROR: NOT MEMBER\n");
                memset(message, 0, sizeof(message));
                strcpy(message, "$");
                strcat(message, "ERROR: NOT MEMBER");
                write(currConnfd, message, strlen(message));
                memset(lbuffer, 0, sizeof(lbuffer));
                memset(timeArr, 0, sizeof(timeArr));
                //getting cur time
                gettimeofday(&tv, NULL);
                millitime = tv.tv_sec;
                strftime(timeArr, 30, "%m-%d-%Y  %T.", localtime(&millitime));
                memset(milliray, 0, sizeof(milliray));
                sprintf(milliray, "%d", tv.tv_usec);
                
                strcpy(lbuffer, "\"");
                strcat(lbuffer, timeArr);
                strcat(lbuffer, ".");
                strcat(lbuffer, milliray);
                strcat(lbuffer, "\": Responded to agent \"");
                strcat(lbuffer, currClient);
                strcat(lbuffer, "\" with \"$ERROR: NOT MEMBER\"\n");
                logger(lbuffer, lHead);
            }
        }
        if (strcmp(Action, "LIST") == 0) {
            struct eventQ *search;
            search = (struct eventQ *) malloc (sizeof(struct eventQ));
            search = Head;
            int found = 0;
            while (search -> next != NULL) {
                search = search -> next;
                if (strcmp(search -> Cname, currClient) == 0) found = 1;
            }
            if (found == 0) {
                memset(lbuffer, 0, sizeof(lbuffer));
                //getting cur time
                gettimeofday(&tv, NULL);
                millitime = tv.tv_sec;
                strftime(timeArr, 30, "%m-%d-%Y  %T.", localtime(&millitime));
                memset(milliray, 0, sizeof(milliray));
                sprintf(milliray, "%d", tv.tv_usec);
                
                strcpy(lbuffer, "\"");
                strcat(lbuffer, timeArr);
                strcat(lbuffer, ".");
                strcat(lbuffer, milliray);
                strcat(lbuffer, "\": no response is supplied to agent \"");
                strcat(lbuffer, currClient);
                strcat(lbuffer, "\" (agent not active)\n");
                logger(lbuffer, lHead);
            }
            if (found == 1){
                printf("$List of active agents\n");
                search = Head;
                while (search -> next != NULL){
                    search = search -> next;
                    printf("$   <%s, %ld>\n", search -> Cname, (time(0) - search -> joined));
                    memset(message, 0, sizeof(message));
                    strcpy(message, "$ <");
                    strcat(message, search -> Cname);
                    strcat(message, ", ");
                    char numarry[MAXBUF];
                    sprintf(numarry, "%ld", time(0) - search -> joined);
                    strcat(message, numarry);
                    strcat(message, ">\n");
                    write(currConnfd, message, strlen(message));
                }
                memset(lbuffer, 0, sizeof(lbuffer));
                //getting cur time
                gettimeofday(&tv, NULL);
                millitime = tv.tv_sec;
                strftime(timeArr, 30, "%m-%d-%Y  %T.", localtime(&millitime));
                memset(milliray, 0, sizeof(milliray));
                sprintf(milliray, "%d", tv.tv_usec);
                
                strcpy(lbuffer, "\"");
                strcat(lbuffer, timeArr);
                strcat(lbuffer, ".");
                strcat(lbuffer, milliray);
                strcat(lbuffer, "\": Responded to agent \"");
                strcat(lbuffer, currClient);
                strcat(lbuffer, "\" with \"$List of active agents\n\"\n");
                logger(lbuffer, lHead);
            }
        }
        if (strcmp(Action, "LOG") == 0) {
            int isHere = 0;
            struct logQ * search;
            search = (struct logQ *) malloc (sizeof(struct logQ));
            struct eventQ *find;
            find = (struct eventQ *) malloc (sizeof(struct eventQ));
            find = Head;
            while(find -> next != NULL && isHere == 0) {
                find = find -> next;
                if (strcmp(find -> Cname, currClient) == 0) {
                    isHere = 1; // isfound
                }
            }
            if(isHere == 0){
                memset(lbuffer, 0, sizeof(lbuffer));
                //getting cur time
                gettimeofday(&tv, NULL);
                millitime = tv.tv_sec;
                strftime(timeArr, 30, "%m-%d-%Y  %T.", localtime(&millitime));
                memset(milliray, 0, sizeof(milliray));
                sprintf(milliray, "%d", tv.tv_usec);
                
                strcpy(lbuffer, "\"");
                strcat(lbuffer, timeArr);
                strcat(lbuffer, ".");
                strcat(lbuffer, milliray);
                strcat(lbuffer, "\": no response is supplied to agent \"");
                strcat(lbuffer, currClient);
                strcat(lbuffer, "\" (agent not active)\n");
                logger(lbuffer, lHead);
            }
            if (isHere == 1){
                FILE *myfile = fopen("log.txt", "w");
                printf("Printing Log File\n");
                //getting cur time
                gettimeofday(&tv, NULL);
                millitime = tv.tv_sec;
                strftime(timeArr, 30, "%m-%d-%Y  %T.", localtime(&millitime));
                memset(milliray, 0, sizeof(milliray));
                sprintf(milliray, "%d", tv.tv_usec);
                
                strcpy(lbuffer, "\"");
                strcat(lbuffer, timeArr);
                strcat(lbuffer, ".");
                strcat(lbuffer, milliray);
                strcat(lbuffer, "\": Responded to agent \"");
                strcat(lbuffer, currClient);
                strcat(lbuffer, "\" with \"$Log File\"\n");
                logger(lbuffer, lHead);
                search = lHead;
                while(search -> next != NULL){
                    search = search -> next;
                    printf("   %s\n", search -> message);
                    fprintf(myfile, "%s\n", search -> message);
                    memset(message, 0, sizeof(message));
                    strcpy(message, "$");
                    
                    strcat(message, search -> message);
                    write(currConnfd, message, strlen(message));
                }
                fclose(myfile);
            }
        }
        
        memset(currClient, 0, sizeof(currClient)); // clearing these three for next iteration
        memset(Action, 0, sizeof(Action));
        memset(buffer, 0, sizeof(buffer));
        close(currConnfd);
    }
}

void logger(char lbuffer[100], struct logQ *lHead){
    //printf("The contents of the buffer in logger: %s", lbuffer);
    struct logQ *newEntry;
    newEntry = (struct logQ *) malloc (sizeof(struct logQ));
    struct logQ *search;
    search = (struct logQ *) malloc (sizeof(struct logQ));
    search = lHead;
    while (search -> next != NULL) {
        search = search -> next;
    }
    search -> next = newEntry;
    newEntry -> next = NULL;
    strcpy(newEntry -> message, lbuffer);
    
}


//CONNECT
//   establishes a connection with a server
//    int connect(int sockfd, struct sockaddr * name, socklen_t namelen);
//BIND
//   binds a socket (sockfd) to an address and a port
//    int bind(int sockfd, struct sockaddr * my_addr, socklen_t addrlen);
//SOCKET
//   returns a file descriptor to be used for subsequent calls & reutrns a -1 on error
//    int socket (int domain, int type, int protocol);
//ACCEPT
//   blocks server until client connects and returns new file descriptor to read a write with client
//    int accept (int sockfd, struct sockaddr * addr, socklen_t *addrlen)
//READ
//   try to read count into fifer from fd, retunrs bytes written or -1 on error (0 dentoes end of file)
//    int read(int fd, void * buf, int count);
//WRITE
//   writes up to count into fd from buffer and returns num bites written
//    int write( int fd, const void * buffer, int count);
//LISTEN
//   creates a queue and allows the process to listen
//    int listen(int fd, int queue size);



























