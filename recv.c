#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

char buf[MAX_SIZE+1];

void print_response(int s_fd) {
    int r_size;
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
}


void recv_mail()
{
    const char* host_name = "pop.***.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "*******"; // TODO: Specify the user
    const char* pass = "*******"; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    s_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(dest_ip);

    if (connect(s_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        perror("Socket connect.\n");
        exit(EXIT_FAILURE);
    }

    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send user and password and print server response
    sprintf(buf, "user %s\r\n", user);
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);
    
    sprintf(buf, "pass %s\r\n", pass);
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    // TODO: Send STAT command and print server response
    sprintf(buf, "STAT\r\n");
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    // TODO: Send LIST command and print server response
    sprintf(buf, "LIST\r\n");
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    // TODO: Retrieve the first mail and print its content
    sprintf(buf, "retr 1\r\n");
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    // TODO: Send QUIT command and print server response
    sprintf(buf, "quit\r\n");
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
