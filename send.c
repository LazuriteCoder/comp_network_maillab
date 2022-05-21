#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095

char buf[MAX_SIZE+1];
char* tmpfile_path = "templeate_file_for_comp_network";

void print_response(int s_fd) {
    int r_size;
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
}

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.**.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "*******"; // TODO: Specify the user
    const char* pass = "*******"; // TODO: Specify the password
    const char* from = "*******"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
    s_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(dest_ip);

    if (connect(s_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        printf("Socket connect error...\n");
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

    // Send EHLO command and print server response
    const char* EHLO = "EHLO **.com\r\n"; // TODO: Enter EHLO command here
    send(s_fd, EHLO, strlen(EHLO), 0);

    // TODO: Print server response to EHLO command
    print_response(s_fd);

    // TODO: Authentication. Server response should be printed out.
    char* AUTH = "AUTH login\r\n";
    send(s_fd, AUTH, strlen(AUTH), 0);
    print_response(s_fd);

    int len = 0;
    // Input mail name.
    char* account = encode_str(user);
    len = strlen(account);
    account[len - 1] = '\r';
    account[len] = '\n';
    account[len + 1] = '\0';
    send(s_fd, account, strlen(account), 0);
    free(account);

    // Print response.
    print_response(s_fd);

    // Input pass.
    char* passwd = encode_str(pass);
    len = strlen(passwd);
    passwd[len - 1] = '\r';
    passwd[len] = '\n';
    passwd[len + 1] = '\0';
    send(s_fd, passwd, strlen(passwd), 0);
    free(passwd);

    print_response(s_fd);


    // TODO: Send MAIL FROM command and print server response
    sprintf(buf, "MAIL FROM:<%s>\r\n", from);
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    
    // TODO: Send RCPT TO command and print server response
    sprintf(buf, "RCPT TO:<%s>\r\n", receiver);
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    // TODO: Send DATA command and print server response
    sprintf(buf, "data\r\n");
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    // TODO: Send message data
    sprintf(buf, "Subject:%s\r\n", subject);
    send(s_fd, buf, strlen(buf), 0);

    sprintf(buf, "From:%s\r\n", from);
    send(s_fd, buf, strlen(buf), 0);

    sprintf(buf, "To:%s\r\n", receiver);
    send(s_fd, buf, strlen(buf), 0);

    sprintf(buf, "MIME-Version: 1.0\r\n");
    send(s_fd, buf, strlen(buf), 0);

    sprintf(buf, "Content-Type: multipart/mixed; boundary=qwertyuiopasdfghjklzxcvbnm\r\n");
    send(s_fd, buf, strlen(buf), 0);

    sprintf(buf, "\r\n--qwertyuiopasdfghjklzxcvbnm\r\n");
    send(s_fd, buf, strlen(buf), 0);

    // text
    if (msg != NULL) {
        // header
        sprintf(buf, "Content-Type: text/plain\r\n");
        send(s_fd, buf, strlen(buf), 0);

        // Encode the message with base64, 
        // for the reason that if message contains MIME header, the message would go wrong.
        sprintf(buf, "Content-Transfer-Encoding: base64\r\n\r\n");
        send(s_fd, buf, strlen(buf), 0);

        FILE *msg_file = fopen(msg, "r+");
        // Check if the msg is path or not.
        if (msg_file == NULL) {
            char* msg_base64 = encode_str(msg);
            send(s_fd, msg_base64, strlen(msg_base64), 0);
            free(msg_base64);
        } else {
            FILE *outmsg = fopen(tmpfile_path, "w+");
            encode_file(msg_file, outmsg);
            fclose(outmsg);
            outmsg = fopen(tmpfile_path, "r+");
            memset(buf, 0, MAX_SIZE);
            i = 0;
            while (!feof(outmsg)) {
                fscanf(outmsg, "%c", &buf[i]);
                i++;
            }
            buf[i] = '\0';
            if (remove(tmpfile_path) == -1) {
                perror("REMOVE file error.\n");
            }
            fclose(outmsg);
            send(s_fd, buf, strlen(buf), 0);
            fclose(msg_file);
        }
        sprintf(buf, "\r\n--qwertyuiopasdfghjklzxcvbnm\r\n");
        send(s_fd, buf, strlen(buf), 0);
    }

    // file
    if (att_path != NULL) {
        FILE *att_file = fopen(att_path, "r");
        if (att_file == NULL) {
            perror("No attachment.");
        } else {
            // add header only when file is correct.
            sprintf(buf, "Content-Type: application/octet-stream\r\n");
            send(s_fd, buf, strlen(buf), 0);

            sprintf(buf, "Content-Disposition: attachment; name=\"%s\"\r\n", att_path);
            send(s_fd, buf, strlen(buf), 0);

            sprintf(buf, "Content-Transfer-Encoding: base64\r\n\r\n");
            send(s_fd, buf, strlen(buf), 0);


            FILE *outfile = fopen(tmpfile_path, "w+");
            encode_file(att_file, outfile);
            fclose(outfile);
            fclose(att_file);
            outfile = fopen(tmpfile_path, "r+");

            memset(buf, 0, MAX_SIZE);
            i = 0;
            while (!feof(outfile)) {
                fscanf(outfile, "%c", &buf[i]);
                i++;
            }
            buf[i] = '\0';
            fclose(outfile);

            send(s_fd, buf, strlen(buf), 0);

            if (remove(tmpfile_path) == -1) {
                perror("REMOVE file error.\n");
            }
        }
        sprintf(buf, "\r\n--qwertyuiopasdfghjklzxcvbnm\r\n");
        send(s_fd, buf, strlen(buf), 0);
    }

    // TODO: Message ends with a single period
    send(s_fd, end_msg, strlen(end_msg), 0);
    print_response(s_fd);

    // TODO: Send QUIT command and print server response
    sprintf(buf, "quit\r\n");
    send(s_fd, buf, strlen(buf), 0);
    print_response(s_fd);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
