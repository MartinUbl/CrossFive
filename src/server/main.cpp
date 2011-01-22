#include <global.h>

int running=1;
Client *clients=NULL;
int num_clients=0;
TCPsocket server;

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

char *getMsg(TCPsocket sock, char **buf)
{
	Uint32 len,result;
	static char *_buf;

	if(!buf)
		buf = &_buf;

	if(*buf)
		free(*buf);

	*buf = NULL;

	result = SDLNet_TCP_Recv(sock,&len,sizeof(len));
	if(result < sizeof(len))
	{
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
		return NULL;
	}

	len = SDL_SwapBE32(len);

	if(!len)
		return NULL;

	*buf = (char*)malloc(len);
	if(!(*buf))
		return NULL;

	result = SDLNet_TCP_Recv(sock,*buf,len);
	if(result < len)
	{
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
		free(*buf);
		buf = NULL;
	}

	return (*buf);
}

int putMsg(TCPsocket sock, char *buf)
{
	Uint32 len,result;

	if(!buf || !strlen(buf))
		return 1;

	len = strlen(buf)+1;

	len = SDL_SwapBE32(len);

	result = SDLNet_TCP_Send(sock,&len,sizeof(len));
	if(result < sizeof(len))
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

	len = SDL_SwapBE32(len);

	result = SDLNet_TCP_Send(sock,buf,len);
	if(result < len)
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

	return result;
}

char *strsep(char **stringp, const char *delim)
{
	char *p;
	
	if(!stringp)
		return(NULL);
	p = *stringp;
	while(**stringp && !strchr(delim,**stringp))
		(*stringp)++;
	if(**stringp)
	{
		**stringp = '\0';
		(*stringp)++;
	}
	else
		*stringp = NULL;
	return p;
}

void send_all(char *buf);
int find_client_name(char *name);

char *mformat(char *format,...)
{
	va_list ap;
	Uint32 len = 0;
	static char *str = NULL;
	char *p, *s;
	char c;
	int d;
	unsigned int u;

	if(str)
	{
		free(str);
		str = NULL;
	}

	if(!format)
		return NULL;

	va_start(ap,format);
	for(p = format; *p; p++)
	{
		switch(*p)
		{
			case 's': /* string */
				s = va_arg(ap, char*);
				str = (char*)realloc(str,((len+strlen(s)+4)/4)*4);
				sprintf(str+len,"%s",s);
				break;
			case 'c': /* char */
				c = (char)va_arg(ap, int);
				str = (char*)realloc(str,len+4);
				sprintf(str+len,"%c",c);
				break;
			case 'd': /* int */
				d = va_arg(ap, int);
				str = (char*)realloc(str,((len+64)/4)*4);
				sprintf(str+len,"%d",d);
				break;
			case 'u': /* unsigned int */
				u = va_arg(ap, unsigned int);
				str = (char*)realloc(str,((len+64)/4)*4);
				sprintf(str+len,"%u",u);
				break;
		}
		if(str)
			len = strlen(str);
		else
			len = 0;
	}
	va_end(ap);
	return str;
}

void fix_nick(char *s)
{
	unsigned int i;

	if((i = strspn(s,"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ|_=+.,:;/\\?!@#$%^&*()~`"))!=strlen(s))
		s[i] = '\0';
}

int unique_nick(char *s)
{
	return (find_client_name(s) == -1);
}

Client *add_client(TCPsocket sock, char *name)
{
	fix_nick(name);
	if(!strlen(name))
	{
		putMsg(sock,"Invalid Nickname...bye bye!");
		SDLNet_TCP_Close(sock);
		return(NULL);
	}
	if(!unique_nick(name))
	{
		putMsg(sock,"Duplicate Nickname...bye bye!");
		SDLNet_TCP_Close(sock);
		return(NULL);
	}
	clients = (Client*)realloc(clients, (num_clients+1)*sizeof(Client));
	clients[num_clients].name = name;
	clients[num_clients].sock = sock;
	num_clients++;

	return (&clients[num_clients-1]);
}

int find_client(TCPsocket sock)
{
	for(int i = 0; i < num_clients; i++)
		if(clients[i].sock == sock)
			return i;

	return -1;
}

int find_client_name(char *name)
{
	for(int i = 0; i < num_clients; i++)
		if(!strcasecmp(clients[i].name,name))
			return i;
	return -1;
}

void remove_client(int i)
{
	char *name = clients[i].name;

	if(i < 0 && i >= num_clients)
		return;

	SDLNet_TCP_Close(clients[i].sock);
	
	num_clients--;
	if(num_clients > i)
		memmove(&clients[i], &clients[i+1], (num_clients-i)*sizeof(Client));
	clients = (Client*)realloc(clients, num_clients*sizeof(Client));

    //Disconnect message

	if(name)
		free(name);
}

SDLNet_SocketSet create_sockset()
{
	static SDLNet_SocketSet set = NULL;
	int i;

	if(set)
		SDLNet_FreeSocketSet(set);

	set = SDLNet_AllocSocketSet(num_clients+1);
	if(!set)
    {
		printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
		exit(1);
	}

	SDLNet_TCP_AddSocket(set,server);

	for(i = 0; i < num_clients; i++)
		SDLNet_TCP_AddSocket(set,clients[i].sock);

	return set;
}

void send_all(char *buf)
{
	int cindex;

	if(!buf || !num_clients)
		return;

	cindex = 0;
	while(cindex < num_clients)
	{
		if(putMsg(clients[cindex].sock,buf))
			cindex++;
		else
			remove_client(cindex);
	}
}

int main(int argc, char **argv)
{
	IPaddress ip;
	TCPsocket sock;
	SDLNet_SocketSet set;
	char *message = NULL;
	const char *host = NULL;
	Uint32 ipaddr;
	Uint16 port;

    printf("-------------===========-------------\n");
    printf("-------====== CrossFive ======-------\n");
    printf("-------======  server   ======-------\n");
    printf("-------------===========-------------\n");
    printf("\n");

    printf("Initializing SDL... ");
	if(SDL_Init(0) == -1)
	{
		printf("FAILED: %s\n",SDL_GetError());
		exit(1);
	}
    printf("OK\n");

    printf("Initializing SDLNet... ");
	if(SDLNet_Init() == -1)
	{
		printf("FAILED: %s\n",SDLNet_GetError());
		SDL_Quit();
		exit(2);
	}
    printf("OK\n");

    printf("Resolving host... ");
	port = (Uint16)555;
	if(SDLNet_ResolveHost(&ip,NULL,port) == -1)
	{
		printf("FAILED: %s\n",SDLNet_GetError());
		SDLNet_Quit();
		SDL_Quit();
		exit(3);
	}
    printf("OK\n");

	ipaddr = SDL_SwapBE32(ip.host);
	host = SDLNet_ResolveIP(&ip);

    printf("Opening socket... ");
	server = SDLNet_TCP_Open(&ip);
	if(!server)
	{
		printf("FAILED: %s\n",SDLNet_GetError());
		SDLNet_Quit();
		SDL_Quit();
		exit(4);
	}
    printf("OK\n");
    printf("\n");

    printf("CrossFive server initialized!\n");
    printf("IP: %d.%d.%d.%d:%d\n",
			ipaddr>>24,
			(ipaddr>>16)&0xff,
			(ipaddr>>8)&0xff,
			ipaddr&0xff,
            port);

	while(1)
	{
		int numready,i;
		set = create_sockset();
		numready = SDLNet_CheckSockets(set, (Uint32)-1);
		if(numready == -1)
		{
			printf("SDLNet_CheckSockets: %s\n",SDLNet_GetError());
			break;
		}

		if(!numready)
			continue;

		if(SDLNet_SocketReady(server))
		{
			numready--;
			sock = SDLNet_TCP_Accept(server);
			if(sock)
			{
				char *name = NULL;

				if(getMsg(sock, &name))
				{
					Client *client;
					client = add_client(sock,name);
				}
				else
					SDLNet_TCP_Close(sock);
			}
		}

		for(i = 0; numready && i < num_clients; i++)
		{
			if(SDLNet_SocketReady(clients[i].sock))
			{
				if(getMsg(clients[i].sock, &message))
				{
					numready--;
                    ProcessPacket(message,&clients[i]);

					free(message);
					message = NULL;
				}
				else
					remove_client(i);
			}
		}
	}

    SDLNet_Quit();

    SDL_Quit();

	return 0;
}
