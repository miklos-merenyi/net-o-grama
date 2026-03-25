#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#define PORT 5555
#define BS 256
#define DELIM '\0'
int vsendf(int socket, char* fmt, va_list ap);
int sendf(int socket, char* fmt, ...);
int connect2server(char *srv_name, uint16_t port);
char* getAline(char **tostr, char *from, int *end);
extern int srv;
