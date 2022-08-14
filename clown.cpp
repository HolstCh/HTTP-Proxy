/*
  Author: Chad Holst
  Course: CPSC441
  Instructor: Carey Williamson
  Date submitted: Jan 27, 2022

  The follwing sources assisted the development of this proxy and were supplied by the instructor:
  https://www.binarytides.com/socket-programming-c-linux-tutorial/
  https://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass0/mypal-server.c
  https://d2l.ucalgary.ca/d2l/le/content/426495/viewContent/5143830/View

*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <time.h>
#include <ctype.h>
#include <unistd.h>	
#include <sys/types.h>

using namespace std;

// 10000 bytes
#define SIZE 10000

// Default port for HTTP interactions with Web servers
#define HTTP_PORT 80

// Web server host
#define HOST "pages.cpsc.ucalgary.ca" // web requests get sent to this web server 

// global variable
int child_fd; // child socket file descriptor

// This is a signal handler (mypal-server.c)
void catcher(int sig)
{
	close(child_fd);
	exit(0);
}

// ensure that send() will send all bytes for each request or response that it recieves from recv()
int send_bytes(int socket_fd, char *buffer, int bytes_left, int flag)
{
  int total_sent_bytes = 0; // total bytes sent
  int sent_bytes = 0; // bytes sent each send() call
  do 
  {
    sent_bytes = send(socket_fd, buffer, bytes_left, flag);
    if(sent_bytes == -1)
    { 
      printf("send() failed, but it has sent %i bytes so far.\n", total_sent_bytes);
      return total_sent_bytes;
    }
    else if(sent_bytes == 0)
    { 
      printf("send() has just sent 0 bytes but it has sent %i bytes so far.\n", total_sent_bytes);
      fflush(stdout);
    }
    else
    {
      total_sent_bytes += sent_bytes; // sum all bytes sent
      bytes_left -= sent_bytes; // calculate remaining bytes
      buffer += total_sent_bytes; // pointer artithmetic to the next position in the buffer
    }
  }
  while(bytes_left > 0);
  
  return total_sent_bytes; // return all bytes sent
}

// Each browser request has its own connection with the proxy server part. This is where the main proxy functionality occurs.
void client_connection(int* socket_fd)
{
  char request[SIZE], response[SIZE]; // buffers for HTTP requests and responses
  int client = *socket_fd; // dereference to get socket fd 
  int server; // server socket fd

  int bytes; // bytes recieved from browser request
  while((bytes = recv(client, request, SIZE, 0))) // recieving requests from web browser (client)
  {
    printf("The proxy has recieved %i bytes from a browser request.\n", bytes);

    // set up address and information for the web server
    struct hostent *host;
    struct sockaddr_in address; 

    int connect_fd; // web server connection fd
    char *ip; // for verifying IP

    // initialize web server address
    ::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = ::htons(HTTP_PORT); 

    // supply hostent structure with host information
    if((host = gethostbyname(HOST)) == NULL)
    {
      herror("gethostbyname");
      exit(1);
    }

    ip = inet_ntoa(*((struct in_addr*)host->h_addr_list[0])); // convert to ASCII format to verify IP
    bcopy((char *) host->h_addr, (char *) &address.sin_addr.s_addr, host->h_length); // copy host ip address to s_addr
    printf("%s resolved to : %s\n" , HOST , ip); // h_addr_list[0] is h_addr which is first host IP address

    // socket creation for interface between proxy and web server
    server = ::socket(AF_INET, SOCK_STREAM, 0); // create TCP socket with IPv4
    if(server == -1)
    {
      printf("Failed to create socket, socket() failed\n"); 
    } 

    // connection to web server 
    connect_fd = ::connect(server, (struct sockaddr*)&address, sizeof(address));
    if(connect_fd == -1)
    {
      printf("Failed to connect, connect() failed\n"); 
    }
    printf("Connected to web server and ready to request HTML.\n");

    // redirect each GET request containing a JPG from the web browser to a PNG clown image
    char *images; 
    if((images = strstr(request, "GET")) != NULL) // pointer to beginning of each GET request
    {
      char *check_jpg = NULL;
      check_jpg = strstr(images, ".jpg"); // identify that the content being requested is JPG file using extension
      if(check_jpg != NULL) // replace the first occurence of JPG in the URL
      { 
        // randomly select from a set of two clowns 
        int random_num = rand() % 2; // generates 0 or 1 for random selection
        printf("\nThis GET request needs a different URL:\n");
        bzero(request, strlen(request)); // clear buffer

        // change the request to a request with the URL containing a random PNG clown image
        if(random_num == 0) // randomly select first clown image
        {
          strcpy(request, "GET http://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass1/clown1.png HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n");
        }
        else if(random_num == 1) // randomly select second clown image
        {
          strcpy(request, "GET http://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass1/clown2.png HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n");
        }
      }
    }

    printf("\n*** Here is the request:\n\n%s", request);
    int total_request_bytes = 0;
    total_request_bytes = send_bytes(server, request, bytes, 0); // send bytes recieved from browser request to web server
    printf("The proxy has sent %i bytes during a request to the web server.\n", total_request_bytes);
    bzero(request, SIZE); // clear buffer
    
    // web server response recieved
    int rec_bytes; // bytes recieved from web server
    while(rec_bytes = ::recv(server, response, SIZE, 0)) 
    {
      char *test2;
      while((test2 = ::strstr(response, "Happy")) != NULL) // substitute "Happy" in web server response
      {
        ::strncpy(test2, "Silly", 5);
      }

      int total_response_bytes = 0;
      total_response_bytes = send_bytes(client, response, rec_bytes, 0); // send bytes recieved from web server to browser
      printf("The proxy has sent %i bytes during a response to the web browser.\n", total_response_bytes);
      printf("\n*** Here is the response:\n\n %s\n", response);
      bzero(response, SIZE); // clear buffer
    }
    if(rec_bytes == 0)
    {
      printf("recv() returned 0 while recieving web server response.\n");
      fflush(stdout);
    }
    else if(rec_bytes == -1)
    {
      printf("recv() failed and returned -1 while recieving web server response.\n");
    }
  }
  if(bytes == 0)
  {
    printf("Client disconnected after recieving browser requests.\n");
    fflush(stdout);
  }
  else if(bytes == -1)
  {
    printf("recv() failed while recieving browser requests.\n");
  }

  // free memory of pointer argument
  free(socket_fd);

  // close client fd and server fd
  close(client);
  close(server);
}

// pass in port from command line to start while loop in main which accepts connections to the proxy part of the server
int main(int argc, char *argv[])
{
  if(argc < 2) // check arg is 1
  {
    printf("Error: please provide a port number.\n");
    return 1;
  }

  int port = atoi(argv[1]); // command line arg: port number 

  static struct sigaction act; // signal action
  int parent_fd; // parent socket file descriptor
  int struct_size, *child_sock; 
  struct sockaddr_in address; // internet socket address
  struct sockaddr_in client; // client socket address for incoming connection

  // set up a signal handler for odd terminations
  act.sa_handler = catcher;
  sigfillset(&(act.sa_mask));
  sigaction(SIGPIPE, &act, NULL);
  
  // initialize client address
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port); // takes port from command line
  address.sin_addr.s_addr = htonl(INADDR_ANY); // server will connect to any IP address

  // create parent socket fd
  parent_fd = socket(PF_INET, SOCK_STREAM, 0);
  if(parent_fd == -1)
  {
    printf("socket() call failed!\n");
    exit(1);
  }

  // bind an address and port to the end point 
  if(bind(parent_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_in)) == -1 )
  {
    printf("bind() call failed!\n");
    exit(1);
  }
  printf("bind() is done\n");

  // start listening for incoming connections from web browser
  if(listen(parent_fd, 10) == -1 )
  {
    printf("listen() call failed!\n");
    exit(1);
  }
  printf("Hello, I am a clown proxy and I'm running on port %d\n\n", port);

  // Accept incoming browser connection from a client on a data socket for transferring data
  printf("Waiting for incoming connections...\n");
  struct_size = sizeof(struct sockaddr_in);
  while((child_fd = accept(parent_fd, (struct sockaddr*)&client, (socklen_t*)& struct_size))) // child_fd is a data socket
  {
    printf("Connection to proxy server is accepted.\n");
    child_sock = (int*)malloc(1); // allocate memory for global data socket pointer
    *child_sock = child_fd; // assign fd value for data transfer
    client_connection(child_sock); // send data socket to function for data transfer
    close(child_fd); // close child socket fd
    printf("Client connection has terminated.\n");
  }
  if(child_fd == -1)
  {
    printf("accept() failed\n");
    return 1;
  }
  close(parent_fd); // close parent socket fd
  return 0;
}
