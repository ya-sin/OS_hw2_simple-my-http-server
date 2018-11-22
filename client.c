#include "utils.h"
#include "string_util.h"
#include "threads.h"
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
typedef struct sendpack {
    int fd;
    char *request;
} sendpack;
void init_sendpack(sendpack* arg,char*request,int fd)
{
    arg->fd = malloc(strlen(fd)*sizeof(int));
    arg->request = malloc(strlen(request)*sizeof(char));
    bzero(arg->request,strlen(request));
    bzero(arg->fd,strlen(fd));
}
// void *send(void* data){
//   // sendpack *msg = (sendpack*) data;
//   // int fd = msg->fd;
//   // char* mes = msg->request;
//   // write(fd, data, strlen(data));
//   // pthread_exit(NULL);
// }
void readsocket(int sockfd,char *result,char* arg,char *query)
{
    char *temp = malloc(14400*sizeof(char));
    strcpy(temp,result);
    char *test[100]= {};
    int i = 1;
    char *requestlist = strtok(result,"\n");

    // get line
    *test = requestlist;
    while(requestlist!=NULL) {
        *(test+i) = strtok(NULL,"\n");
        requestlist = *(test+i);
        i++;
    }
    i = 0;
    // printf("q%s\n",result);
    // printf("%s\n%d\n",*(test+1),strlen(test));
    // dir
    if(contains(*(test+1),"directory")) {
        // get each content in the dir
        char *con[20];
        con[i] = strtok(*(test+4)," ");
        while(con[i]!=NULL) {
            i++;
            con[i] = strtok(NULL," ");
        }
        printf("%s\n", temp);

        for(int j=0; j<i; j++) {
            char *qu = malloc(256*sizeof(char));
            char *tmp = malloc(14400*sizeof(char));
            // struct sendpack *pack = malloc(sizeof(sendpack));
            // init_sendpack(pack,pack->request,pack->fd);
            strcpy(tmp,query);
            //con[j] = concat("/",con[j]);
            char* p = malloc(256*sizeof(char));
            char* t = malloc(256*sizeof(char));
            sprintf(p,"/%s",con[j]);
            sprintf(t,"%s%s",tmp,p);
            free(p);
            //tmp = concat(tmp,con[j]);
            printf("%s %d\n",con[j],j);
            sprintf(qu,"GET %s %s",t,arg);
            // pack->fd = sockfd;
            // pack->request = qu;
            // pthread_t t;
            // pthread_create(&t,NULL,send, (void*)pack);
            // pthread_join(t,NULL);
            // read_text_from_socket(sockfd);
            free(tmp);
            free(t);
            free(qu);
        }
    } else { // file
        printf("%s\n\n", temp);
    }
    free(temp);
    free(result);
}
int main(int argc, char *argv[])
{

    char request[200];
    char *query = argv[2];
    char *ip = argv[4];
    char *port = argv[6];
    int portnum;

    portnum = atoi(port);


    // scanf("%s",ip);
    // printf("query: %s",query);
    // printf("ip: %s",ip);
    // printf("port: %d",portnum);

    if (argc != 7) {
        printf("Syntax: client <url>\n");
        exit(1);
    }


    // char *url = argv[1];
// connecting
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Check create socket or not
    if (sockfd == -1) {
        // error("socket");
        printf("\n Error : Could not create socket\n");
        return 1;
    }


    struct sockaddr_in serv_addr;

    memset(&serv_addr,0,sizeof(serv_addr));
    // setting sockaddr_in
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_port = htons(portnum);
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    // serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // inet_pton convert IPv4 and IPv6 addresses from text to binary form
    // if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0) {
    //     printf("\n inet_pton error occured\n");
    //     return 1;
    // }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) == -1) {
        /* Try running the client without the server started and you'll get this error */
        // error("connect");
        printf("\n Error : Connect Failed \n");
        return 1;
    }
// complete connecting

    char *get_str = malloc(128 + strlen(query));
    char *arg = malloc(128);

    sprintf(get_str,"GET %s HTTP/1.x\r\nHOST: %s:%s\r\n\r\n",query,ip,port);
    sprintf(arg,"HTTP/1.x\r\nHOST: %s:%s\r\n\r\n",ip,port);
    // printf("%s", get_str);
    write_to_socket(sockfd, get_str);

    char *result = read_text_from_socket(sockfd);

    readsocket(sockfd,result,arg,query);

    free(get_str);
    free(arg);
    close(sockfd);

    return 0;
}
