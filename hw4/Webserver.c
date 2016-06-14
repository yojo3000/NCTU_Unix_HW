//
//  main.c
//  Webserver
//
//  Created by Mars Huang on 6/4/16.
//  Copyright © 2016 Mars Huang. All rights reserved.
//
#include "all_include.h"

int number_of_argument;
char executing_binary[200];
int server_port;
char document_root[20];
struct sockaddr_in server_sockaddr_in;
struct sockaddr_in client_sockaddr_in;
int server_socket_fd;
int client_socket_fd;

struct {
	char *extensions;
	char *content_type;
} extension_list [] = {
  {"mp4", "application/octet-stream"},
  {"mp3", "application/octet-stream"},
	{"ogg", "application/octet-stream"},
  {"jpg", "image/jpeg"},
  {"jpeg","image/jpeg"},
  {"png", "image/png" },
	{"htm", "text/html" },
  {"html","text/html" },
  {"txt", "text/plain"},

	{0,0} };

int main(int argc, const char * argv[]) {

    number_of_argument  = argc;
    server_port         = atoi(argv[1]);
    strcpy(executing_binary, argv[0]);
    strcpy(document_root, argv[2]);

    printf("%s \nnow running...\n\n", executing_binary);
    printf("Server port: %d\n", server_port);
    printf("Document root: %s\n", document_root);
    printf("\n");

    chdir(document_root); // set wwwroot

    // set server address info
    server_sockaddr_in.sin_family      = AF_INET;              // ipv4
    server_sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY); // all network interface
    server_sockaddr_in.sin_port        = htons(server_port);   //

    server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // (domain, type, protocol)
    perror("socket");

    // let local port can be reuse when quickly re-execute web server binary
    int x = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x));
    perror("setsockopt");

    // bind server address info to server socket fd
    socklen_t server_sockaddr_length   = sizeof(server_sockaddr_in);
    struct sockaddr* server_sockaddr   = (struct sockaddr *)&server_sockaddr_in; // type casting to sockaddr*
    bind(server_socket_fd, server_sockaddr, server_sockaddr_length);
    perror("bind");

    listen(server_socket_fd, 64); // mark that server fd is ready to accept client connection, max connection count is 64 clients
    perror("listen");

    while(TRUE){
        socklen_t client_sockaddr_length   = sizeof(client_sockaddr_in);
        struct sockaddr* client_sockaddr   = (struct sockaddr*)&client_sockaddr_in;
				printf("\n");
        client_socket_fd = accept(server_socket_fd, client_sockaddr, &client_sockaddr_length);
        perror("accept");

        // when accept a client connect, then fork a child to deal with it
        pid_t pid = fork();

        if (pid < 0)
        {
            return 0; // fork error
        }else if (pid > 0)
        {
            // in parent
        }else
        {
            // in child
            int read_buffer_size = 4096;
            char read_buffer[read_buffer_size]; // store data that read from client fd

            long read_return = read(client_socket_fd, read_buffer, read_buffer_size); // read from client fd
            perror("read");

            if( read_return<0 )
            {
                // read error
                printf("read return < 0\n");
            }else if( read_return==0 )
            {
              printf("read return < 0\n");
                // no data
            }else
            {
                if( read_return>read_buffer_size )
                {
                  printf("read return > 0 && over size\n");
                    // over size
                }else
                {
                    printf("read return > 0 && buffer size ok\n");


                    read_buffer[read_buffer_size] = 0;
                    printf("read return: %ld\n", read_return);

                    //printf("read buffer:\n%s\n", read_buffer);

/**************************************************************************************/
										/*
											0123456
											GET /_
											GET /a
											012345678
										*/
										int blank_index=5;

                    while(TRUE){
                      if(read_buffer[blank_index]==' '){
                        break;
                      }
                      if(read_buffer[blank_index]==NULL){
                        break;
                      }
                      blank_index++;
                    }

                    // get the GET method's path/file (GET /index.html)
										/*
											0123456
											GET /
											GET /aab
											012345678
										*/
										printf("blank index: %d\n", blank_index);
                    char folder_file_origin[30];
                    memset(folder_file_origin, 0, strlen(folder_file_origin));

										if(read_buffer[5] !=' '){
                    	strcpy(folder_file_origin, get_substring2(read_buffer, 4, blank_index));
										}else
										{
											strcpy(folder_file_origin, "/");
										}
                    // get_substring(&folder_file, read_buffer, 5, blank_index);
                    printf("folder file origin:%s, len: %d\n", folder_file_origin, strlen(folder_file_origin));

										//=======================
                    // remove all chars after "?"
                    int q_index;
                    char folder_file[30];
                    strcpy(folder_file, folder_file_origin); // set initial value

                    for(q_index=0; q_index<strlen(folder_file_origin); q_index++)
                    {
                      if(folder_file_origin[q_index] == '?')
                      {
                        strncpy(folder_file, folder_file_origin, q_index);
                        break;
                      }
                    }
                    printf("folder file:%s, len: %d\n", folder_file, strlen(folder_file));
                    //==========================
										//--------------------------
                    // get the restrict path of folder file
                    char folder_file_restrict_path[1024];
                    memset(folder_file_restrict_path, 0, 1024);

                    strcpy(folder_file_restrict_path, get_folder_file_restrict_path(folder_file));
										printf("folder_file_restrict_path: %s\n", folder_file_restrict_path);

                    if(is_directory(folder_file_restrict_path)){ // directory

                      printf("\n%s is a directory\n", folder_file_restrict_path);

                      char last_char[1];
                      strcpy(last_char, folder_file + (strlen(folder_file)-1));
                      printf("last char: %s\n\n", last_char);

                      char folder_file_temp[1024];
                      strcpy(folder_file_temp, folder_file);
                      if(strcmp(last_char, "/") != 0)
                      {
                        // last char is not "/"

                        strcat(folder_file_temp, "/"); // add "/" at the end
                        folder_end_not_slash(client_socket_fd, folder_file_temp);
                      }else
                      {
                        // last char is "/" !!
												printf("last is slash (/)\n");

                        //======================

                        // check folder permission
												char tmpstring[20];
												get_wwwroot_restrict_path(tmpstring);
												strcat(tmpstring, folder_file_temp);;

                        if( open(tmpstring, O_RDONLY)<0 )
                        {
													// folder permission deny

                          printf("OPEN ERROR: %s %d\n", strerror(errno), errno);
													folder_error_response(client_socket_fd, NO_SUCH_FILE);

                        }else
                        {
													// folder is readable
													printf("readable folder\n");
                          // check if there is a "index.html" file
													char path_add_index_html[50];
													strcpy(path_add_index_html, folder_file_restrict_path);
                          strcat(path_add_index_html, "index.html");

                          int have_index_html = open(path_add_index_html, O_RDONLY);

                          if( have_index_html < 0)
                          {
														printf("not readable index.html\n");
                            // no index.html or permision deny
                            printf("errno: %d\n", errno);
                            DIR* dir;
                            struct dirent* ptr;

                            switch (errno) {
                              case 2: //folder readable && index.html not exist
                                // list all file/folder in this folder

                                dir =opendir(folder_file_restrict_path);
																send_header(client_socket_fd, "text/html");

                                while((ptr = readdir(dir))!=NULL) {
                                  send_file_list(client_socket_fd, ptr->d_name, folder_file);
                                }

                                closedir(dir);
                                break;

                              case 13: // folder readable && index.html permission deny
                                // just like static file method

                                  file_error_response(client_socket_fd, FILE_PER_DENY, 1);
                                break;
                            }
                          }else // index.html is readable
                          {
                            // show index.html
														printf("readable index.html \n");
                            send_header(client_socket_fd, "text/html");
                            send_data(client_socket_fd, have_index_html);
                          }

                        }
                      }

                    }else{ // static file

                        printf("\n%s is not a directory\n", folder_file_restrict_path);
                        // int open(const char *pathname, int flags, mode_t mode);

                        int open_return_fd = open(folder_file_restrict_path, O_RDONLY);
                        perror("open");

                        // errorno &&　error string compare
                        // http://www.virtsync.com/c-error-codes-include-errno
                        // printf("error string: %s %d\n", strerror(errno), errno);
                        if( open_return_fd<0 ) // permission deny or not exist
                        {
                          printf("error string: %s %d\n", strerror(errno), errno);
                          switch (errno) {
                            case 2: //No such file or directory

                              file_error_response(client_socket_fd, NO_SUCH_FILE, 0);
                              break;
                            case 13: // Permission deny

                              file_error_response(client_socket_fd, FILE_PER_DENY, 0);
                              break;
                          }
                        }else
                        {
                          //==========================
                          // split string temp.mp3 => temp && mp3

                          char string_parts[2][30];
                          char* str[100];
                          strcpy(str, folder_file);

                          char* delim = ".";
                          char buf[30] = {0};
                          int tmp = 0;
                          char *substr = NULL;

                          strcpy(buf, str);
                          int count = 0;
                          substr = strtok(buf, delim);
                          do {
                            strcpy(string_parts[tmp], substr);
                            tmp++;
                            substr = strtok(NULL, delim);
                          } while (substr);

													printf("request_file_name: %s\n", string_parts[0]);
                          printf("request_file_type: %s\n", string_parts[1]);
                          //=====================================

                          char request_file_name[strlen(string_parts[0])];
                          char request_file_type[strlen(string_parts[1])];
                          strcpy(request_file_name, string_parts[0]);
                          strcpy(request_file_type, string_parts[1]);

                          //printf("request_file_name: %s\n", request_file_name);
                          //printf("request_file_type: %s\n", request_file_type);

													// check the corresponding content-type
                          char* header_content_type;
                          int len;
													int i;
                          header_content_type = (char *)0;
                          for(i=0;extension_list[i].extensions != 0;i++) {
                            len = strlen(extension_list[i].extensions);

                            // check request file's ext, compare it with the content of extension struct
                            if( !strncmp(request_file_type, extension_list[i].extensions, len)) {
                              header_content_type =extension_list[i].content_type;
                              break;
                            }
                          }
                          printf("file_type: %s\n\n", header_content_type);

                          //=========================================

                          FILE* file = NULL;
                          file = fopen(folder_file, "r");
                          printf("before send: %s\n", folder_file);
													printf("test content type: %s\n", header_content_type);
													printf("test return fd: %d\n", open_return_fd);
                          send_header(client_socket_fd, header_content_type);
                          send_data(client_socket_fd, open_return_fd);

                        }
                    }
                    printf("\n%s\n", read_buffer);

                }
            }
        }close(client_socket_fd);
    }
    close(server_socket_fd);
    return 0;
}
