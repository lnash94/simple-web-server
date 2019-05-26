#include <netinet/in.h>    
#include <stdio.h>    
#include <stdlib.h>    
#include <sys/socket.h>
#include <sys/uio.h>   
#include <sys/sendfile.h> 
#include <sys/stat.h>    
#include <sys/types.h>    
#include <unistd.h>    
#include <string.h>
#include <fcntl.h>

#define PORT 3000
#define BUFFERSIZE 1000

/* handle the php request use php_cgi library  */
void php_cgi(char *script_path , int new_socket){
    char buff[100]=  "HTTP/1.1 200 OK\n Content-type: text/html\n Connection: close\n";
    char *msg = buff;
    
    send( new_socket, msg, strlen(msg),0);
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
     
}

/* this  function for genarate response header  */

int send_header(int status ,int new_socket , char *f_name, char *content_type , char *method){
    char	*statstr, buff[256];
    int filepath;
    struct stat stbuf; // hold information about input file 
    
    /* we can add all the satus code here */
    
    switch (status)
    {
    case 200:
        
        statstr = "OK";
        break;

    case 400:
        statstr = "Bad Request";
        break;
    
    default:
        break;
    }

            // char *source = source_content(f_name);
            char buffer_body[BUFFERSIZE];
            if((strcmp(f_name,""))==0){

            }
            else{
            //  check that source file exists and can be opened 
                if((filepath = open(f_name, O_RDONLY))<0){
                    perror("open() read");
                    exit(1);
                }
            }
            // get size and permissions of the source file 
            fstat (filepath ,&stbuf);
            int content_ln = stbuf.st_size;
            char* temp_ptr = buffer_body;

            int num_chars =  sprintf(temp_ptr, "HTTP/1.0 %d %s\r\n", status,statstr);
            temp_ptr += num_chars;

            num_chars = sprintf(temp_ptr, "Content-Type: %s; charset=UTF-8\r\n", content_type);
            temp_ptr += num_chars;

            num_chars = sprintf(temp_ptr, "Content-Length: %d\r\n", content_ln-1);
            temp_ptr += num_chars;


            num_chars = sprintf(temp_ptr, "Accept-Ranges: bytes\r\n");

            num_chars = sprintf(temp_ptr, "Connection: close\r\n\r\n");
            temp_ptr += num_chars;

            int total_message_length = temp_ptr - buffer_body;

            send(new_socket,buffer_body,total_message_length,0);
            sendfile(new_socket,filepath,0,stbuf.st_size);
            
            //clean up and exit
            close(filepath);
        
            shutdown(new_socket,SHUT_RDWR);
            shutdown(filepath,SHUT_RDWR);
            close(new_socket);
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
            /* fork for hadle the multiple processors. pthread library also can be use for handle multi process */

            if (fork()== 0)
            {
                if (new_socket > 0){    
                    printf("The Client is connected...\n");
                }

                
                char *f_name = "/home/sumudu/VScode/webserver/web/index.html";
            
                int reciv = recv(new_socket, buffer, buffersize, 0);    
                
                printf("%s\n", buffer);  

            /*  take the request line in the request */
                char *request_line = strtok(buffer,"\n");
                
            /* split the request line for getting the path ... still to be impelement */

                // sscanf(request_line, "%s %s %s", method, path, vers);
                buffer[reciv] = '\0';
                method = strtok(buffer,  " \t\r\n");
                path    = strtok(NULL, " \t");
                vers = strtok(NULL, " \t\r\n"); 


                if ((strcmp(method, "GET")== 0) && ((strcmp(vers, "HTTP/1.0")==0) || strcmp(vers, "HTTP/1.1")==0)) {
                    
                    if(strcmp(path,"/")==0){
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/index.html";
                        send_header(200,new_socket,f_name , "text/html", "GET");
                    }
                    else if (strcmp(path,"/test")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/test.html";
                        send_header(200,new_socket,f_name ,"text/html","GET");
                    }
                    else if (strcmp(path,"/postform")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/PostForm.html";
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
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/kubernaties.png";
                        send_header(200,new_socket,f_name ,"image/jpg" ,"GET");
                    }
                    else if (strcmp(path,"/testphp")==0)
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/testfilephp.php";
                        php_cgi(f_name,new_socket);
                        sleep(1);
                        shutdown(new_socket,SHUT_RDWR);
                        close(new_socket);   
                        // abort(); 
                        _exit(EXIT_FAILURE);
                                               
                    }
                    else
                    {
                        f_name = "/home/sumudu/VScode/webserver/15000915/web/page_not_found.html";
                        send_header(400,new_socket,f_name, "text/html" ,"GET");
                    }            
                }        
                else if((strcmp(method, "POST")== 0) && ((strcmp(vers, "HTTP/1.0")==0) || strcmp(vers, "HTTP/1.1")==0)) {
                    f_name ="";
                    send_header(200,new_socket,f_name ,"application/x-www-form-urlencoded","POST");
                    
                }    
                
                
                buffer[buffersize] = '\0';
                exit(0);
            
            }
            
        }
        
        
    }

    free(buffer);

    shutdown(create_socket,SHUT_RDWR);
    close(create_socket); 
    return 0;    
}