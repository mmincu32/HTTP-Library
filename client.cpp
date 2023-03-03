#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include "helpers.h"
#include "requests.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

/* function to get the cookies or the tokens from the http response */ 
void get_them(char *response, char **info, int &count, const char *type, const char sep)
{
    size_t found = 0;
    string str_response = string(response);

    do {
        found = str_response.find(type, found + strlen(type));

        if (found != string::npos) {
            info[count] = (char *)calloc(LINELEN, sizeof(char));

            int info_len;
            for (info_len = 0; response[info_len + found + strlen(type)] != sep; ++info_len);

            strncpy(info[count++], response + found + strlen(type), info_len);
        }

    } while (found != string::npos);
}

int isNumber(string &str)
{
    for (char &c : str) {
        if (isdigit(c) == 0)
            return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    int cookies_count = 0;
    int tokens_count = 0;

    char **cookies;
    char **tokens;
    
     
    while (1) {
        char command[LINELEN];
        fgets(command, LINELEN, stdin);
        command[strlen(command) - 1] = '\0';
        
        if (strcmp(command, "register") == 0) {
            char username[LINELEN];
            char password[LINELEN];
            json payload;
            
            /* set username */
            printf("username=");
            fgets(username, LINELEN, stdin);
            username[strlen(username) - 1] = '\0';
            payload["username"] = username;

            /* set password */
            printf("password=");
            fgets(password, LINELEN, stdin);
            password[strlen(password) - 1] = '\0';
            payload["password"] = password;
            //printf("Password is: %s\n", password);


            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", 
                        "application/json", payload, NULL, 0);
            printf("%s\n", message);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            printf("%s\n", response);
            close_connection(sockfd);


        }

        else if (strcmp(command, "login") == 0) {
            char username[LINELEN];
            char password[LINELEN];
            json payload;

            /* if there are cookies obviously there is someone logged in*/
            if (cookies != NULL) {
                printf("Someone is already logged in!\n");
                continue;
            }

            /* enter username */
            printf("username=");
            fgets(username, LINELEN, stdin);
            username[strlen(username) - 1] = '\0';
            payload["username"] = username;

            /* enter password */
            printf("password=");
            fgets(password, LINELEN, stdin);
            password[strlen(password) - 1] = '\0';
            payload["password"] = password;

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", 
                        "application/json", payload, NULL, 0);
            printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            /* the resources for the user are prepared only if there are no errors */
            string str_response = string(response);
            if (str_response.find("\"error\"") == string::npos)                     
                cookies = (char **)calloc(1, sizeof(char *)); {           
                /* get the cookies, usually just one, the session cookie */
                get_them(response, cookies, cookies_count, "Set-Cookie: ", ';');
            }

            close_connection(sockfd);

        }

        else if (strcmp(command, "enter_library") == 0) {

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            /* we use the cookies to prove we are logged in */
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", 
                        NULL, cookies, cookies_count, NULL, 0);
            printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            /* the resources for the user are prepared only if there are no errors */
            string str_response = string(response);
            if (str_response.find("\"error\"") == string::npos) {                  
                tokens = (char **)calloc(1, sizeof(char *));

                /* get the tokens, usually just one, the access token */
                get_them(response, tokens, tokens_count, "\"token\":\"", '"');
            }

            close_connection(sockfd);
          

        }

        else if (strcmp(command, "get_books") == 0) {

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            /* we use the tokens to prove we have access to the library */
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", 
                        NULL, NULL, 0, tokens, tokens_count);
            printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            close_connection(sockfd);
        }

         else if (strcmp(command, "get_book") == 0) {
            
            /* enter the id */
            char id[LINELEN];
            printf("id=");
            fgets(id, LINELEN, stdin);
            id[strlen(id) - 1] = '\0';

            char *route = (char *)calloc(LINELEN, sizeof(char *));
            sprintf(route, "/api/v1/tema/library/books/%s", id);

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            /* we use the tokens to prove we have access to the library */
            message = compute_get_request("34.241.4.235", route, NULL, NULL, 0, tokens, tokens_count);
            printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            close_connection(sockfd);
        }

        else if (strcmp(command, "add_book") == 0) {
            char line[LINELEN];
            json payload;

            /* don't bother with entering the book's details if we don't even
            have permission to add it */
            if (tokens == NULL) {   /* no access to the library */
                printf("You don't have permission to add a book!\n");
                continue;
            }
            
            /* enter the title */
            printf("title=");
            memset(line, 0, LINELEN);
            fgets(line, LINELEN, stdin);
            line[strlen(line) - 1] = '\0';
            payload["title"] = line;

            /* enter the author */
            printf("author=");
            memset(line, 0, LINELEN);
            fgets(line, LINELEN, stdin);
            line[strlen(line) - 1] = '\0';
            payload["author"] = line;

            /* enter the genre */
            printf("genre=");
            memset(line, 0, LINELEN);
            fgets(line, LINELEN, stdin);
            line[strlen(line) - 1] = '\0';
            payload["genre"] = line;

            /* enter the page count until it is a number */
            string str;
            do {
                printf("page_count=");
                memset(line, 0, LINELEN);
                fgets(line, LINELEN, stdin);
                line[strlen(line) - 1] = '\0';
                str = string(line);
                if (!isNumber(str)) {
                    printf("Page count is not a number!\n");
                    continue;
                }
            } while (!isNumber(str));
            payload["page_count"] = atoi(line);

            /* enter the publisher */
            printf("publisher=");
            memset(line, 0, LINELEN);
            fgets(line, LINELEN, stdin);
            line[strlen(line) - 1] = '\0';
            payload["publisher"] = line;

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            /* we use the tokens to prove we have access to the library */
            message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", 
                        "application/json", payload, tokens, tokens_count);
            printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            close_connection(sockfd);
            

        }

        else if (strcmp(command, "delete_book") == 0) {

            /* enter the id */
            char id[LINELEN];
            printf("id=");
            fgets(id, LINELEN, stdin);
            id[strlen(id) - 1] = '\0';

            char *route = (char *)calloc(LINELEN, sizeof(char *));
            sprintf(route, "/api/v1/tema/library/books/%s", id);

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            /* we use the tokens to prove we have access to the library */
            message = compute_delete_request("34.341.4.235", route, tokens, tokens_count);
            printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            close_connection(sockfd);
        }

        else if (strcmp(command, "logout") == 0) {

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            /* we use the cookies to prove we are logged in */
            message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", 
                        NULL, cookies, cookies_count, NULL, 0);

            for (int i = 0; i < cookies_count; ++i) {
                free(cookies[i]);
                cookies[i] = NULL;
          }
	    if (cookies)
            	free(cookies);
            cookies = NULL;
            cookies_count = 0;

            for (int i = 0; i < tokens_count; ++i) {
                free(tokens[i]);
                tokens[i] = NULL;
            }
	    if (tokens)
            	free(tokens);
            tokens = NULL;
            tokens_count = 0;

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            close_connection(sockfd);
        }
        else if (strcmp(command, "exit") == 0)
            break;

        else printf("Command %s not found!\n", command);


    }

    return 0;
}
