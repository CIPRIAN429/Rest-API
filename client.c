#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include "stdbool.h"

struct User{
    char username[100];
    char password[100];
};

struct Book{
    char title[250];
    char author[250];
    char genre[250];
    int page_count;
    char publisher[250];
    
};

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char *cookie = NULL;
    char buffer[250];
    bool ret = false;
    char token[1000] = "";
    memset(token, 0, sizeof(token));
    memset(buffer, 0, sizeof(buffer));
    while(1){
        sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
        fgets(buffer,250, stdin);
        // caz register
        if(strncmp(buffer, "register", 8) == 0){
            struct User user;
            
            printf("username=");
            size_t sizee;
            fgets(user.username, 100, stdin);
            sizee = strlen(user.username);
            user.username[sizee - 1] = '\0';
            printf("password=");
            fgets(user.password, 100, stdin);
            sizee = strlen(user.password);
            user.password[sizee - 1] = '\0';
    
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
             
            json_object_set_string(object, "username", user.username);
            json_object_set_string(object, "password", user.password);
             
            message = json_serialize_to_string_pretty(value);
            
            char *post = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json", message, NULL, NULL);
            send_to_server(sockfd, post);
            char *from_server = receive_from_server(sockfd);
            
            char *new = strchr(from_server, ' ');
            // primul caracter din status
            if(new[1] != '2')
                printf("Utilizatorul exista deja!\n");
            else 
                printf("Utilizator inregistrat cu succes!\n");
            
            json_value_free(value);
            json_free_serialized_string(message);
            close(sockfd);

        }
        // caz login
        else if(strncmp(buffer, "login", 5) == 0){
            struct User user;
            
            printf("username=");
            size_t sizee;
            fgets(user.username, 100, stdin);
            sizee = strlen(user.username);
            user.username[sizee - 1] = '\0';
            printf("password=");
            fgets(user.password, 100, stdin);
            sizee = strlen(user.password);
            user.password[sizee - 1] = '\0';
            
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
             
            json_object_set_string(object, "username", user.username);
            json_object_set_string(object, "password", user.password);
             
            message = json_serialize_to_string_pretty(value);
        
            char *post = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json", message, NULL, NULL);
          
            send_to_server(sockfd, post);
            char *from_server = receive_from_server(sockfd);
            char *ver = strstr(from_server, "connect.sid");
            ret = false;
            if(ver){
                printf("Logare cu succes!\n");
                ret = true;
            }
            else printf("Logare esuata!\n");

            if(ret){
                cookie = strtok(ver, ";");
            }
            json_value_free(value);
            json_free_serialized_string(message);
            close(sockfd);

        }
        // caz enter_library    
        else if(strncmp(buffer, "enter_library", 13) == 0){
            if(!ret){
                printf("Utilizatorul nu este logat!\n");
                continue;
            }
            char* get = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", cookie, NULL);
            send_to_server(sockfd, get);
            response = receive_from_server(sockfd);
            // folosesc o copie pentru a nu afecta raspunsul original
            char *p, copy[1500];
            strcpy(copy, response);
            p = strtok(copy, " ");
            p = strtok(NULL, " ");
            // extrag token
            if(p[0] == '2'){
                char *aux = strstr(response, "token");
                aux += 8;
                aux[strlen(aux) - 2] = '\0';
                strcpy(token, aux);
                printf("Acces la biblioteca permis!\n");
            }
            else
                printf("Acces la biblioteca refuzat!\n");
            close(sockfd);
            
        }
        // caz get_books
        else if(strncmp(buffer, "get_books", 9) == 0){
            if(strlen(token) == 0 || ret == false){
                printf("Lipsa acces la biblioteca sau lipsa login!\n");
                continue;
            }
            char* get = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", cookie, token);
            
            send_to_server(sockfd, get); 

            response = receive_from_server(sockfd);
           
            char* p = strchr(response, '[');
            JSON_Value* value = json_parse_string(p);
            char* nice = json_serialize_to_string_pretty(value);
            printf("%s\n", nice);

            json_value_free(value);
            json_free_serialized_string(nice);
            close(sockfd);
        }
        // caz get_book
        else if(strncmp(buffer, "get_book", 8) == 0){
            if(strlen(token) == 0 || ret == false){
                printf("Lipsa acces la biblioteca sau lipsa login!\n");
                continue;
            }
            printf("id=");
            int id;
            int ver = (scanf("%d", &id));
            // verific sa am un numar
            if(!ver){
                printf("Id nepotrivit\n");
                continue;
            }

            char for_id[50];
            sprintf(for_id, "/api/v1/tema/library/books/%d", id);
            char* get = compute_get_request("34.241.4.235", for_id, cookie, token);
            send_to_server(sockfd, get);
            char *book;
            response = receive_from_server(sockfd);
            char copy[1500];
            strcpy(copy, response);
            char *var = strtok(copy, " ");
            var = strtok(NULL, " ");
            if(var[0] == '2'){
                char *aux = strstr(response, "{");
                size_t sizee = strlen(aux);
                aux[sizee - 1] = '\0';
                JSON_Value *value = json_parse_string(aux);
                book = json_serialize_to_string_pretty(value);
                printf("%s\n", book);
            }
            else
                printf("Cartea cu id-ul %d nu exista!\n", id);
            close(sockfd);

        }
        // caz add_book
        else if(strncmp(buffer, "add_book", 8) == 0){
            if(strlen(token) == 0 || ret == false){
                printf("Lipsa acces la biblioteca sau lipsa login!\n");
                continue;
            }

            char *info;
            struct Book book;
            size_t sizee;
            memset(&book, 0, sizeof(book));

            printf("title=");
            
            fgets(book.title, 250, stdin);
            sizee = strlen(book.title);
            book.title[sizee - 1] = '\0';

            printf("author=");
            fgets(book.author, 250, stdin);
            sizee = strlen(book.author);
            book.author[sizee - 1] = '\0';

            printf("genre=");
            fgets(book.genre, 250, stdin);
            sizee = strlen(book.genre);
            book.genre[sizee - 1] = '\0';

            printf("page_count=");
            int ver = (scanf("%d", &(book.page_count)));
            if(!ver){
                printf("Nr de pagini nepotrivit!\n");
                continue;
            }

            
            printf("publisher=");
            getchar();
            fgets(book.publisher, 250, stdin);
            sizee = strlen(book.publisher);
            book.publisher[sizee - 1] = '\0';
            

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);

            json_object_set_string(object, "title", book.title);
            json_object_set_string(object, "author", book.author);
            json_object_set_string(object, "genre", book.genre);
            json_object_set_number(object, "page_count", book.page_count);
            json_object_set_string(object, "publisher", book.publisher);
            
            info = json_serialize_to_string_pretty(value);
           

            char *post = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", "application/json", info, cookie, token);
            send_to_server(sockfd, post);
            response = receive_from_server(sockfd);
            char *var = strtok(response, " ");
            var = strtok(NULL, " ");
            if(var[0] == '2')
                printf("Cartea %s a fost adaugata!\n", book.title);
            else
                printf("Cartea nu a fost adaugata!\n");
            
            close(sockfd);

        }
        // caz delete_book
        else if(strncmp(buffer, "delete_book", 11) == 0){
             if(strlen(token) == 0 || ret == false){
                printf("Lipsa acces la biblioteca sau lipsa login!\n");
                continue;
            }
            printf("id=");
            int id;
            int ver = (scanf("%d", &id));

            if(!ver){
                printf("Id nepotrivit\n");
                continue;
            }

            char for_id[50];
            sprintf(for_id, "/api/v1/tema/library/books/%d", id);
            char* get = compute_delete_request("34.241.4.235", for_id, cookie, token);
            send_to_server(sockfd, get);
            response = receive_from_server(sockfd);
            char *var = strtok(response, " ");
            var = strtok(NULL, " ");
            if(var[0] == '2')
                printf("Cartea %d a fost eliminata!\n", id);
            else 
                printf("Cartea %d nu a fost eliminata!\n", id);
        }
        // caz logout
        else if(strncmp(buffer, "logout", 6) == 0){
            if(!ret){
                printf("Nu sunteti logat!\n");
                continue;
            }

            char* get = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", cookie, token);
            send_to_server(sockfd, get);
            response = receive_from_server(sockfd);
            char *var = strtok(response, " ");
            var = strtok(NULL, " ");
            if(var[0] == '2'){
                printf("Delogare reusita!\n");
                ret = false;
            }
            else
                printf("Delogare esuata!\n");
            close(sockfd);

        }
        // caz exit
        else if(strncmp(buffer, "exit", 4) == 0)
            break;
    }

    return 0;
}
