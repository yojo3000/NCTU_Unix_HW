#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0
#define NO_SUCH_FILE 2
#define FILE_PER_DENY 13
//   setvbuf(stdout, 0, 2, 0);
void get_substring(char** dest_string, char* src_string, int start_position, int end_position);
void get_wwwroot_restrict_path(char* buffer);
char* get_folder_file_restrict_path(char* folder_file);
int is_directory(char* restrict_path);
char* get_substring2(char* src_string, int start_position, int end_position);
void send_header(int client_socket_fd, char* content_type);
void send_data(int client_socket_fd, int file_fd);
void file_error_response(int client_socket_fd, int error_type, int normal_or_not);
void folder_error_response(int client_socket_fd, int error_type);
void folder_end_not_slash(int client_socket_fd, char* folder_path);
void send_file_list(int client_socket_fd, char* str, char* directory_path);
