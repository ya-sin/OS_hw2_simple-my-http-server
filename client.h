#ifndef CLIENT_H
#define CLIENT_H

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

int starts_with(char *s, const char *with)
{
    return strncmp(s, with, strlen(with)) == 0;
}

int ends_with(const char *s, const char *with)
{
    int len_s = strlen(s);
    int len_with = strlen(with);

    if (len_with <= len_s) {
        return strncmp(s + len_s - len_with, with, len_with) == 0;
    } else {
        return 0;
    }
}

int contains(const char *s1, const char *s2)
{
    return strstr(s1, s2) != NULL;
}

char* substr(const char* input, int offset, int len, char* dest)
{
    int input_len = strlen (input);

    if (offset + len > input_len) {
        return NULL;
    }

    strncpy(dest, input + offset, len);
    return dest;
}

int ends_with_extension(const char *inp)
{
    int end_pos = strlen(inp);

    while (--end_pos >= 0) {
        if (inp[end_pos] == '.') return 1;
        if (!isalpha(inp[end_pos])) return 0;
    }

    return 0;
}

char *concat(const char *s1, const char *s2)
{
    char *r = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(r, s1);
    strcat(r, s2);
    return r;
}

char *concat3(const char *s1, const char *s2, const char *s3)
{
    char *r = malloc(strlen(s1) + strlen(s2) + strlen(s3) + 1);
    strcpy(r, s1);
    strcat(r, s2);
    strcat(r, s3);
    return r;
}

char *concat4(const char *s1, const char *s2, const char *s3, const char *s4)
{
    char *r = malloc(strlen(s1) + strlen(s2) + strlen(s3) + strlen(s4) + 1);
    strcpy(r, s1);
    strcat(r, s2);
    strcat(r, s3);
    strcat(r, s4);
    return r;
}

char *strappend(const char *s1, const char *s2)
{
    char *r = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(r, s1);
    strcat(r, s2);
    assert(strlen(r) == strlen(s1) + strlen(s2));
    return r;
}

void error(char *message)
{
    perror(message);
    exit(1);
}

char *read_text_from_socket(int sockfd)
{
    const int BUF_SIZE = 256;
    char *buffer = malloc(BUF_SIZE);

    char *result = malloc(1);
    result[0] = '\0';

    int n;
    while (1) {
        int n = read(sockfd, buffer, BUF_SIZE - 1);
        if (n < 0) {
            error("Error reading from socket");
        }
        buffer[n] = '\0';
        char *last_result = result;
        result = strappend(last_result, buffer);
        free(last_result);
        if (n < BUF_SIZE - 1) {
            break;
        }
    }

    free(buffer);

    return result;
}

void write_to_socket(int sockfd, const char *message)
{
    if (write(sockfd, message, strlen(message)) == -1) {
        error("write_to_socket");
    }
}


#endif
