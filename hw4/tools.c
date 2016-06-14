#include "all_include.h"

void get_substring(char** dest_string, char* src_string, int start_position, int end_position) {
  // char *strncpy(char *dest, const char *src, size_t n);
  strncpy(*dest_string, src_string+start_position, end_position-start_position+1);
  perror("strncpy");
  //*dest_string[end_position-start_position] = 0;
}

char* get_substring2(char* src_string, int start_position, int end_position) {
  // char *strncpy(char *dest, const char *src, size_t n);
  char* dest_string = (char*) malloc(sizeof(char)*(end_position-start_position+1) );
  strncpy(dest_string, src_string+start_position, end_position-start_position+1);
  perror("strncpy");

  dest_string[end_position-start_position] = 0;
  return dest_string;
}

void get_wwwroot_restrict_path(char* buffer){
  char temp[100];
  getcwd(temp, sizeof(temp));
  perror("getcwd");

  strcpy(buffer, temp);
}

char* get_folder_file_restrict_path(char* folder_file)
{
  char wwwroot_restrict_path[100];
  get_wwwroot_restrict_path(wwwroot_restrict_path);

  int length1 = strlen(wwwroot_restrict_path);
  int length2 = strlen(folder_file);
  char* buffer = (char*)malloc(sizeof(char)*(length1 + length2));

  char folder_file_restrict_path[30];
  strcpy(folder_file_restrict_path, wwwroot_restrict_path);

  strcat(folder_file_restrict_path, folder_file);

  strcpy(buffer, folder_file_restrict_path);
  return buffer;
}

int is_directory(char* restrict_path)
{
  struct stat statbuf;
  lstat(restrict_path, &statbuf);

  if (S_ISDIR(statbuf.st_mode)){
    return 1;
  }
  else{
    return 0;
  }
}

void send_header(int client_socket_fd, char* content_type)
{
  char buffer[1024];
  sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);
  send(client_socket_fd, buffer, strlen(buffer), 0);

}

void send_data(int client_socket_fd, int file_fd)
{
   char buffer2[1024];
   int read_return_size;
   int count = 0;
   int x;
   while ((read_return_size = read(file_fd, buffer2, 1024)) > 0 ) {
     x = send(client_socket_fd, buffer2, read_return_size, 0);
     count++;
     //write(client_socket_fd, buffer2, read_return_size);
	}
  printf("total size: %d\n", 1024*(count-1)+x);
}

void send_file_list(int client_socket_fd, char* str, char* directory_path)
{

  char buffer2[1024];
  char temp[50];
  strcpy(temp, directory_path);
  strcat(temp, str);

  sprintf(buffer2, "<a style='font-size: 10px;' href='%s'>%s</a><br>", temp, str);
  send(client_socket_fd, buffer2, strlen(buffer2), 0);

}

void file_error_response(int client_socket_fd, int error_type, int normal_or_not)
{
  /*
    normal: no such file: 404, permission deny: 403
    abnormal: permission deny: 404, no such file: 403
  */

  char buffer[1024];
  if(normal_or_not==0)
  {
    switch (error_type) {
      case NO_SUCH_FILE:
        sprintf(buffer, "HTTP/1.1 404 Not Found\r\n\r\n<h1>403 Forbidden</h1>");
        send(client_socket_fd, buffer, strlen(buffer), 0);
        sprintf(buffer, "<br><h3>actually it's 404 file not found, just for homework</h3>");
        send(client_socket_fd, buffer, strlen(buffer), 0);
        break;

      case FILE_PER_DENY:
        sprintf(buffer, "HTTP/1.1 403 Forbidden\r\n\r\n<h1>404 Not Found</h1>");
        send(client_socket_fd, buffer, strlen(buffer), 0);
        sprintf(buffer, "<br><h3>actually it's 403 forbidden, just for homework</h3>");
        send(client_socket_fd, buffer, strlen(buffer), 0);
        break;
    }
  }else
  {
    switch (error_type) {
      case FILE_PER_DENY:
        sprintf(buffer, "HTTP/1.1 403 Forbidden\r\n\r\n<h1>403 Forbidden</h1>");
        send(client_socket_fd, buffer, strlen(buffer), 0);
        break;
    }
  }
}

void folder_error_response(int client_socket_fd, int error_type)
{
  char buffer[1024];
  switch (error_type) {
    case NO_SUCH_FILE:
      sprintf(buffer, "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Not Found</h1>");
      send(client_socket_fd, buffer, strlen(buffer), 0);
      break;

    case FILE_PER_DENY:
      sprintf(buffer, "HTTP/1.1 403 Forbidden\r\n\r\n<h1>403 Forbidden</h1>");
      send(client_socket_fd, buffer, strlen(buffer), 0);
      break;
  }
}

void folder_end_not_slash(int client_socket_fd, char* folder_path)
{
  printf("301301301301301301301301\n");
  char buffer[1024];
  sprintf(buffer, "HTTP/1.1 301 Moved Permanently\r\nLocation: %s\r\n\r\n", folder_path);
  send(client_socket_fd, buffer, strlen(buffer), 0);

}
