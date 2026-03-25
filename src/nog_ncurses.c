#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <glib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "client.h"
#include "cli_ncurses.h"
#include "network.h"
#include "sound.h"
#define PROGNAME "nog_ncurses"
#define DESC "A terminal based NET-O-GRAMA game client."

int debugLevel=10;
char *logfile="/dev/null";

int main(int argc, char* argv[])
{
    GOptionContext *context;
    void usage(int t)
    {
        printf("%s",g_option_context_get_help (context, FALSE, NULL));
        exit(t);
    }

    GError *error = NULL;
    static GOptionEntry entries[6];

    gchar * port_text;
    gchar * logfile_text;
    gchar * debuglevel_text;
    gchar * usrname_text;
    char * usr_temp=NULL;
    char * srv_temp=NULL;

    entries[0]=(  GOptionEntry)
    {
        "server", 's', 0, G_OPTION_ARG_STRING, &srv_temp, "Server address ( defaults to \"localhost\").", "server.domain"
    } ;

    asprintf(&port_text,"TCP port number (default: %d).",PORT);
    entries[1]=( GOptionEntry)
    {
        "port", 'p', 0, G_OPTION_ARG_INT, &port, port_text, "number"
    };

    strncpy(usrname,(char *)getpwuid(getuid())->pw_name,9);
    asprintf(&usrname_text,"Nickname ( default: your login name \"%s\").",usrname);
    entries[2]=( GOptionEntry)
    {
        "name", 'n', 0, G_OPTION_ARG_STRING, &usr_temp, usrname_text, "nickname"
    };

    asprintf(&debuglevel_text,"Debug level ( 0..10, 10 is the most verbose; default: %d).",debugLevel);
    entries[3]=(  GOptionEntry)
    {
        "debuglevel", 'd', 0, G_OPTION_ARG_INT, &debugLevel, debuglevel_text, "level"
    };

    asprintf(&logfile,"./%s.log",PROGNAME);
    asprintf(&logfile_text,"Logfile (default: %s).",logfile);

    entries[4]=(  GOptionEntry)
    {
        "logfile", 'l', 0, G_OPTION_ARG_FILENAME, &logfile, logfile_text, "file"
    } ;
    entries[5]=( GOptionEntry ) {NULL};
    context = g_option_context_new (NULL);

    g_option_context_add_main_entries(context, entries,PROGNAME);

    if (!g_option_context_parse(context, &argc, &argv,  &error))
    {
        g_warning("Error parsing command line options: %s",
            error->message);
        exit(EXIT_FAILURE);
    }
    if (debugLevel<0 || debugLevel > 10) usage(1);
    if (usr_temp) { strncpy(usrname,usr_temp,8); free(usr_temp); }
    if (srv_temp) { strncpy(srvname,srv_temp,50); free(srv_temp); }
    else
        srv_temp="localhost";
    free(port_text);free(usrname_text);free(logfile_text);
    free(debuglevel_text);

    g_option_context_free(context);
    //exit(1);
    initLogfile(logfile);
    initConnection(srvname,port);
    initScreen();
    initSound();
    Mix_PlayChannel(-1, getSound("foundbig"), 0);
    displayMessage("welcome",usrname);
    gameLoop();
    endSound();
    endScreen();
    close(srv);
    closeLogfile();
    exit(0);
}
