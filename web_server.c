
#include <netinet/in.h>    
#include <stdio.h>    
#include <stdlib.h>    
#include <sys/socket.h>    
#include <sys/stat.h>    
#include <sys/types.h>    
#include <unistd.h>    
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/sendfile.h>




#define PORT 3000
#define BUFFERSIZE 1000



/* this function for read the all content to the buffer */

char * source_content(char *f_name){
    
    char *source = NULL;
    
    //read the file and add all to the buffer 
        FILE *fd = fopen(f_name,"r");
        if (fd != NULL) {
        /* Go to the end of the file. */
            if (fseek(fd, 0L, SEEK_END) == 0) {
                /* Get the size of the file. */
                long bufsize = ftell(fd);
                if (bufsize == -1) { /* Error */ }

                /* Allocate our buffer to that size. */
                source = malloc(sizeof(char) * (bufsize + 1));

                /* Go back to the start of the file. */
                if (fseek(fd, 0L, SEEK_SET) != 0) { /* Error */ }

                /* Read the entire file into memory. */
                size_t newLen = fread(source, sizeof(char), bufsize, fd);
                if ( ferror( fd ) != 0 ) {
                    fputs("Error reading file", stderr);
                } else {
                    source[newLen++] = '\0'; /* Just to be safe. */
                }
            }
            fclose(fd);
        }
    return source;
}

/* handle the php request use php_cgi library  */
void php_cgi(char *script_path , int new_socket){
    char *msg = "HTTP/1.1 200 OK\n Content-type: text/html\n Connection: close\n";
    // header(msg);
    int x = send( new_socket, msg, strlen(msg),0 );
    // char* script = source_content(script_path);
    char script[1024];
    dup2(new_socket, STDOUT_FILENO);
    strcpy(script, "SCRIPT_FILENAME=");
    strcat(script, script_path);
    putenv("GATEWAY_INTERFACE=CGI/1.1");
    putenv(script);
    putenv("QUERY_STRING=");
    putenv("REQUEST_METHOD=GET");
    putenv("REDIRECT_STATUS=true");
    putenv("SERVER_PROTOCOL=HTTP/1.1");
    putenv("REMOTE_HOST=127.0.0.1");
    execl("/usr/bin/php-cgi", "php-cgi7.0", NULL);

    shutdown(x,SHUT_RDWR);
    close(x);
                        
}


/* this  function for genarate response header  */

int send_header(int status ,int new_socket , char *f_name, char *content_type , char *method){
    char	*statstr, buff[256];

    /* we can add all the satus code here */
    switch (status)
    {
    case 200:
        statstr = "OK";
        break;

    case 400:
        statstr = "Bad Request";
        break;
    
    case 500:
        statstr = "Server Error";
        break;
    

    default:
        break;
    }
    if(strcmp(method, "GET")== 0) {
        char* source = source_content(f_name);
        char buffer_body[BUFFERSIZE];

        // char html_body[] = "<html><head></head><body>Hello World!</body></html>\r\n";
        // int content_ln = sizeof(*source);
        int content_ln = strlen(source);

        // printf("%d\n",content_ln);

        char* temp_ptr = buffer_body;
        int num_chars =  sprintf(temp_ptr, "HTTP/1.1 %d %s\r\n", status,statstr);
        temp_ptr += num_chars;

        num_chars = sprintf(temp_ptr, "Content-Type: %s; charset=UTF-8\r\n", content_type);
        temp_ptr += num_chars;

        num_chars = sprintf(temp_ptr, "Content-Length: %d\r\n", content_ln-1);
        temp_ptr += num_chars;
        
        num_chars = sprintf(temp_ptr, "Accept-Ranges: bytes\r\n");
        num_chars = sprintf(temp_ptr, "Connection: close\r\n\r\n");
        temp_ptr += num_chars;

        num_chars = sprintf(temp_ptr,"%s", source);
        temp_ptr += num_chars;

        int total_message_length = temp_ptr - buffer_body;

        // printf("%s\n",buffer_body);
        write(new_socket, buffer_body, total_message_length);
                
        source[total_message_length]='\0';
        free(source); /* Don't forget to call free() later! */

        shutdown(new_socket,SHUT_RDWR);        
        close(new_socket);
        
    }else
    {
        // char *source = source_content(f_name);
        char buffer_body[BUFFERSIZE];

        char* temp_ptr = buffer_body;

        int num_chars =  sprintf(temp_ptr, "HTTP/1.1 %d %s\r\n", status,statstr);
        temp_ptr += num_chars;

        num_chars = sprintf(temp_ptr, "Content-Type: %s;\r\n", content_type);
        temp_ptr += num_chars;

        num_chars = sprintf(temp_ptr, "Accept-Ranges: bytes\r\n");

        num_chars = sprintf(temp_ptr, "Connection: close\r\n\r\n");
        temp_ptr += num_chars;

        int total_message_length = temp_ptr - buffer_body;

        write(new_socket, buffer_body, total_message_length);
                    
        shutdown(new_socket,SHUT_RDWR);
                                
        close(new_socket);    
                  
    }
    
    return 0;
}

int main() {    
    int valread, create_socket, new_socket; 
    int opt = 1;
    // int addrlen = sizeof(address);
    socklen_t addrlen;    
    int buffersize = 1024;    
    char *buffer = malloc(buffersize);
    struct sockaddr_in address;
    char symbol;   

    char *method,*path, *vers;
    char *f_name;
    // char *source = NULL;
    
    /* ================================ */ 
    // char *hello = "Hello from server";   
    /* ================================ */ 
    
    /*
        Creating socket file descriptor domain, type, protocol
        create_socket = socket descriptor, an integer (like a file-handle)
        socket type = SOCK_STREAM (TCP)
    */
    create_socket = socket(AF_INET, SOCK_STREAM, 0);
    if ( create_socket > 0){    
        printf("The socket was created\n");
    }
    else if (create_socket == 0 )
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    /* 
        Forcefully attaching socket to the port 3000 
        This is completely optional, but it helps in reuse of address and port
    */
    if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
        
    address.sin_family = AF_INET;    
    address.sin_addr.s_addr = INADDR_ANY;    
    address.sin_port = htons(PORT);

    /* bind function binds the socket to the address and port number specified in address */    
    if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == 0){    
        printf("Binding Socket\n");
    }
        
    /*  Loop infinitely to accept and service connections  */

    while (1) {    
        if (listen(create_socket, 10) < 0) {    
            perror("server: listen");    
            exit(1);    
        } 

        new_socket = accept(create_socket, (struct sockaddr *) &address, &addrlen);
        if ( new_socket < 0) {    
            perror("server: accept");    
            exit(1);    
        }    
        else
        {
            /* fork for hadle the multiple processors ex: we can use pthread library also for this bcs threads have lightweight processes property */

            if (fork()== 0)
            {
                if (new_socket > 0){    
                    printf("The Client is connected...\n");
                }

                /* First version */
                //for any request first version for GET  function

                //recv(new_socket, buffer, bufsize, 0);    
                //printf("%s\n", buffer);    
                //write(new_socket, "hello world\n", 12);    
                //close(new_socket);

                /*=======================*/
                // char buffer[30000] = {0};
                // valread = read( new_socket , buffer, 30000);
                // printf("%s\n",buffer );
                // write(new_socket , hello , 17);
                // printf("------------------Hello message sent-------------------\n");
                // close(new_socket);

                /* ---------------------------------------------------------------------*/

                /* Second version : with Header Request */

                // int size_ln = sizeof("<html><head><title>Simple-Web-Server html-file</title></head><body><H1>This is the content of index.html</H1></body></html>");
                // recv(new_socket, buffer, bufsize, 0);    
                // printf("%s\n", buffer);
                // printf("%d\n", bufsize);
                // write(new_socket, "HTTP/1.1 200 OK\n", 16);
                // write(new_socket, "Content-length: 124\n", 20);
                // write(new_socket, "Content-Type: text/html\n\n", 25);
                // write(new_socket, "<html><head><title>Simple-Web-Server html-file</title></head><body><H1>This is the content of index.html</H1></body></html>",size_ln-1);    
                // close(new_socket);


                /* ---------------------------------------------------------------------*/
                //Third version : Send the local html file to the browser.

                char *f_name = "/home/sumudu/VScode/webserver/15000915/web/index.html";
            
                int reciv = recv(new_socket, buffer, buffersize, 0);    
                
                printf("%s\n", buffer);  

            /*  take the request line in the request */
                char *request_line = strtok(buffer,"\n");
                
            /* split the request line for getting the path ... still to be impelement */
                buffer[reciv] = '\0';
                method = strtok(buffer,  " \t\r\n");
                path    = strtok(NULL, " \t");
                vers = strtok(NULL, " \t\r\n"); 


                if ((strcmp(method, "GET")== 0) && ((strcmp(vers, "HTTP/1.0")==0) || strcmp(vers, "HTTP/1.1")==0)) {
                    
                    if(strcmp(path,"/")==0){
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/index.html";
                        send_header(200,new_socket,f_name , "text/html", "GET");
                    }
                    else if (strcmp(path,"/postform")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/PostForm.html";
                        send_header(200,new_socket,f_name ,"text/html","GET");
                    }
                    else if (strcmp(path,"/test")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/test.html";
                        send_header(200,new_socket,f_name ,"text/html","GET");
                    }
                    else if (strcmp(path,"/text")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/test2.txt";
                        send_header(200,new_socket,f_name ,"text/plain", "GET");
                    }
                    else if (strcmp(path,"/pdf")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/test_pdf.pdf";
                        send_header(200,new_socket,f_name ,"application/pdf", "GET");
                
                    }
                    else if (strcmp(path,"/image1")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/network.jpg";
                        send_header(200,new_socket,f_name ,"image/jpg" ,"GET");
                    }
                    else if (strcmp(path,"/testphp")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/testfilephp.php";
                        php_cgi(f_name,new_socket);
                        sleep(1);
                        shutdown(new_socket,SHUT_RDWR);
                        close(new_socket);    
                        _exit(EXIT_FAILURE);
                                            

                        
                    }
                    
                    else
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/page_not_found.html";
                        send_header(400,new_socket,f_name, "text/html" ,"GET");
                    }            
                }        
                else if((strcmp(method, "POST")== 0) && ((strcmp(vers, "HTTP/1.0")==0) || strcmp(vers, "HTTP/1.1")==0)) {
                    f_name =NULL;
                    send_header(200,new_socket,f_name ,"application/x-www-form-urlencoded","POST");
                    
                }    
                
                
                buffer[buffersize] = '\0';
                exit(0);
            
            }
            
        }
        
        
    }

    free(buffer);

    close(create_socket); 
    return 0;    
}