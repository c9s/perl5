/* win32sck.c
 *
 * (c) 1995 Microsoft Corporation. All rights reserved. 
 * 		Developed by hip communications inc., http://info.hip.com/info/
 * Portions (c) 1993 Intergraph Corporation. All rights reserved.
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 */

#define WIN32IO_IS_STDIO
#define WIN32SCK_IS_STDSCK
#define WIN32_LEAN_AND_MEAN
#ifdef __GNUC__
#define Win32_Winsock
#endif
#include <windows.h>
#include "EXTERN.h"
#include "perl.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <io.h>

/* thanks to Beverly Brown	(beverly@datacube.com) */
#ifdef USE_SOCKETS_AS_HANDLES
#	define OPEN_SOCKET(x)	_open_osfhandle(x,O_RDWR|O_BINARY)
#	define TO_SOCKET(x)	_get_osfhandle(x)
#else
#	define OPEN_SOCKET(x)	(x)
#	define TO_SOCKET(x)	(x)
#endif	/* USE_SOCKETS_AS_HANDLES */

#define StartSockets() \
    STMT_START {					\
	if (!wsock_started)				\
	    start_sockets();				\
    } STMT_END

#define EndSockets() \
    STMT_START {					\
	if (wsock_started)				\
	    WSACleanup();				\
    } STMT_END

#define SOCKET_TEST(x, y) \
    STMT_START {					\
	StartSockets();					\
	if((x) == (y))					\
	    errno = WSAGetLastError();			\
    } STMT_END

#define SOCKET_TEST_ERROR(x) SOCKET_TEST(x, SOCKET_ERROR)

static struct servent* win32_savecopyservent(struct servent*d,
                                             struct servent*s,
                                             const char *proto);

#ifdef USE_THREADS
#ifdef USE_DECLSPEC_THREAD
__declspec(thread) struct servent myservent;
#else
#define myservent (thr->i.Wservent)
#endif
#else
static struct servent myservent;
#endif

static int wsock_started = 0;

void
start_sockets(void) 
{
    unsigned short version;
    WSADATA retdata;
    int ret;
    int iSockOpt = SO_SYNCHRONOUS_NONALERT;

    /*
     * initalize the winsock interface and insure that it is
     * cleaned up at exit.
     */
    version = 0x101;
    if(ret = WSAStartup(version, &retdata))
	croak("Unable to locate winsock library!\n");
    if(retdata.wVersion != version)
	croak("Could not find version 1.1 of winsock dll\n");

    /* atexit((void (*)(void)) EndSockets); */

#ifdef USE_SOCKETS_AS_HANDLES
    /*
     * Enable the use of sockets as filehandles
     */
    setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE,
		(char *)&iSockOpt, sizeof(iSockOpt));
#endif	/* USE_SOCKETS_AS_HANDLES */
    wsock_started = 1;
}


#ifndef USE_SOCKETS_AS_HANDLES
#undef fdopen
FILE *
my_fdopen(int fd, char *mode)
{
    FILE *fp;
    char sockbuf[256];
    int optlen = sizeof(sockbuf);
    int retval;

    if (!wsock_started)
	return(fdopen(fd, mode));

    retval = getsockopt((SOCKET)fd, SOL_SOCKET, SO_TYPE, sockbuf, &optlen);
    if(retval == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) {
	return(fdopen(fd, mode));
    }

    /*
     * If we get here, then fd is actually a socket.
     */
    Newz(1310, fp, 1, FILE);
    if(fp == NULL) {
	errno = ENOMEM;
	return NULL;
    }

    fp->_file = fd;
    if(*mode == 'r')
	fp->_flag = _IOREAD;
    else
	fp->_flag = _IOWRT;
   
    return fp;
}
#endif	/* USE_SOCKETS_AS_HANDLES */


u_long
win32_htonl(u_long hostlong)
{
    StartSockets();
    return htonl(hostlong);
}

u_short
win32_htons(u_short hostshort)
{
    StartSockets();
    return htons(hostshort);
}

u_long
win32_ntohl(u_long netlong)
{
    StartSockets();
    return ntohl(netlong);
}

u_short
win32_ntohs(u_short netshort)
{
    StartSockets();
    return ntohs(netshort);
}



SOCKET
win32_accept(SOCKET s, struct sockaddr *addr, int *addrlen)
{
    SOCKET r;

    SOCKET_TEST((r = accept(TO_SOCKET(s), addr, addrlen)), INVALID_SOCKET);
    return OPEN_SOCKET(r);
}

int
win32_bind(SOCKET s, const struct sockaddr *addr, int addrlen)
{
    int r;

    SOCKET_TEST_ERROR(r = bind(TO_SOCKET(s), addr, addrlen));
    return r;
}

int
win32_connect(SOCKET s, const struct sockaddr *addr, int addrlen)
{
    int r;

    SOCKET_TEST_ERROR(r = connect(TO_SOCKET(s), addr, addrlen));
    return r;
}


int
win32_getpeername(SOCKET s, struct sockaddr *addr, int *addrlen)
{
    int r;

    SOCKET_TEST_ERROR(r = getpeername(TO_SOCKET(s), addr, addrlen));
    return r;
}

int
win32_getsockname(SOCKET s, struct sockaddr *addr, int *addrlen)
{
    int r;

    SOCKET_TEST_ERROR(r = getsockname(TO_SOCKET(s), addr, addrlen));
    return r;
}

int
win32_getsockopt(SOCKET s, int level, int optname, char *optval, int *optlen)
{
    int r;

    SOCKET_TEST_ERROR(r = getsockopt(TO_SOCKET(s), level, optname, optval, optlen));
    return r;
}

int
win32_ioctlsocket(SOCKET s, long cmd, u_long *argp)
{
    int r;

    SOCKET_TEST_ERROR(r = ioctlsocket(TO_SOCKET(s), cmd, argp));
    return r;
}

int
win32_listen(SOCKET s, int backlog)
{
    int r;

    SOCKET_TEST_ERROR(r = listen(TO_SOCKET(s), backlog));
    return r;
}

int
win32_recv(SOCKET s, char *buf, int len, int flags)
{
    int r;

    SOCKET_TEST_ERROR(r = recv(TO_SOCKET(s), buf, len, flags));
    return r;
}

int
win32_recvfrom(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
    int r;

    SOCKET_TEST_ERROR(r = recvfrom(TO_SOCKET(s), buf, len, flags, from, fromlen));
    return r;
}

/* select contributed by Vincent R. Slyngstad (vrs@ibeam.intel.com) */
int
win32_select(int nfds, Perl_fd_set* rd, Perl_fd_set* wr, Perl_fd_set* ex, const struct timeval* timeout)
{
    int r;
#ifdef USE_SOCKETS_AS_HANDLES
    Perl_fd_set dummy;
    int i, fd, bit, offset;
    FD_SET nrd, nwr, nex, *prd, *pwr, *pex;

    PERL_FD_ZERO(&dummy);
    if (!rd)
	rd = &dummy, prd = NULL;
    else
	prd = &nrd;
    if (!wr)
	wr = &dummy, pwr = NULL;
    else
	pwr = &nwr;
    if (!ex)
	ex = &dummy, pex = NULL;
    else
	pex = &nex;

    FD_ZERO(&nrd);
    FD_ZERO(&nwr);
    FD_ZERO(&nex);
    for (i = 0; i < nfds; i++) {
	fd = TO_SOCKET(i);
	if (PERL_FD_ISSET(i,rd))
	    FD_SET(fd, &nrd);
	if (PERL_FD_ISSET(i,wr))
	    FD_SET(fd, &nwr);
	if (PERL_FD_ISSET(i,ex))
	    FD_SET(fd, &nex);
    }

    SOCKET_TEST_ERROR(r = select(nfds, prd, pwr, pex, timeout));

    for (i = 0; i < nfds; i++) {
	fd = TO_SOCKET(i);
	if (PERL_FD_ISSET(i,rd) && !FD_ISSET(fd, &nrd))
	    PERL_FD_CLR(i,rd);
	if (PERL_FD_ISSET(i,wr) && !FD_ISSET(fd, &nwr))
	    PERL_FD_CLR(i,wr);
	if (PERL_FD_ISSET(i,ex) && !FD_ISSET(fd, &nex))
	    PERL_FD_CLR(i,ex);
    }
#else
    SOCKET_TEST_ERROR(r = select(nfds, rd, wr, ex, timeout));
#endif
    return r;
}

int
win32_send(SOCKET s, const char *buf, int len, int flags)
{
    int r;

    SOCKET_TEST_ERROR(r = send(TO_SOCKET(s), buf, len, flags));
    return r;
}

int
win32_sendto(SOCKET s, const char *buf, int len, int flags,
	     const struct sockaddr *to, int tolen)
{
    int r;

    SOCKET_TEST_ERROR(r = sendto(TO_SOCKET(s), buf, len, flags, to, tolen));
    return r;
}

int
win32_setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen)
{
    int r;

    SOCKET_TEST_ERROR(r = setsockopt(TO_SOCKET(s), level, optname, optval, optlen));
    return r;
}
    
int
win32_shutdown(SOCKET s, int how)
{
    int r;

    SOCKET_TEST_ERROR(r = shutdown(TO_SOCKET(s), how));
    return r;
}

int
win32_closesocket(SOCKET s)
{
    int r;

    SOCKET_TEST_ERROR(r = closesocket(TO_SOCKET(s)));
    return r;
}

SOCKET
win32_socket(int af, int type, int protocol)
{
    SOCKET s;

#ifndef USE_SOCKETS_AS_HANDLES
    SOCKET_TEST(s = socket(af, type, protocol), INVALID_SOCKET);
#else
    StartSockets();
    if((s = socket(af, type, protocol)) == INVALID_SOCKET)
	errno = WSAGetLastError();
    else
	s = OPEN_SOCKET(s);
#endif	/* USE_SOCKETS_AS_HANDLES */

    return s;
}

#undef fclose
int
my_fclose (FILE *pf)
{
    int osf, retval;
    if (!wsock_started)		/* No WinSock? */
	return(fclose(pf));	/* Then not a socket. */
    osf = TO_SOCKET(fileno(pf));/* Get it now before it's gone! */
    retval = fclose(pf);	/* Must fclose() before closesocket() */
    if (osf != -1
	&& closesocket(osf) == SOCKET_ERROR
	&& WSAGetLastError() != WSAENOTSOCK)
    {
	return EOF;
    }
    return retval;
}

struct hostent *
win32_gethostbyaddr(const char *addr, int len, int type)
{
    struct hostent *r;

    SOCKET_TEST(r = gethostbyaddr(addr, len, type), NULL);
    return r;
}

struct hostent *
win32_gethostbyname(const char *name)
{
    struct hostent *r;

    SOCKET_TEST(r = gethostbyname(name), NULL);
    return r;
}

int
win32_gethostname(char *name, int len)
{
    int r;

    SOCKET_TEST_ERROR(r = gethostname(name, len));
    return r;
}

struct protoent *
win32_getprotobyname(const char *name)
{
    struct protoent *r;

    SOCKET_TEST(r = getprotobyname(name), NULL);
    return r;
}

struct protoent *
win32_getprotobynumber(int num)
{
    struct protoent *r;

    SOCKET_TEST(r = getprotobynumber(num), NULL);
    return r;
}

struct servent *
win32_getservbyname(const char *name, const char *proto)
{
    struct servent *r;
    dTHR;    

    SOCKET_TEST(r = getservbyname(name, proto), NULL);
    if (r) {
	r = win32_savecopyservent(&myservent, r, proto);
    }
    return r;
}

struct servent *
win32_getservbyport(int port, const char *proto)
{
    struct servent *r;
    dTHR; 

    SOCKET_TEST(r = getservbyport(port, proto), NULL);
    if (r) {
	r = win32_savecopyservent(&myservent, r, proto);
    }
    return r;
}

int
win32_ioctl(int i, unsigned int u, char *data)
{
    u_long argp = (u_long)data;
    int retval;

    if (!wsock_started) {
	croak("ioctl implemented only on sockets");
	/* NOTREACHED */
    }

    retval = ioctlsocket(TO_SOCKET(i), (long)u, &argp);
    if (retval == SOCKET_ERROR) {
	if (WSAGetLastError() == WSAENOTSOCK) {
	    croak("ioctl implemented only on sockets");
	    /* NOTREACHED */
	}
	errno = WSAGetLastError();
    }
    return retval;
}

char FAR *
win32_inet_ntoa(struct in_addr in)
{
    StartSockets();
    return inet_ntoa(in);
}

unsigned long
win32_inet_addr(const char FAR *cp)
{
    StartSockets();
    return inet_addr(cp);
}

/*
 * Networking stubs
 */

void
win32_endhostent() 
{
    croak("endhostent not implemented!\n");
}

void
win32_endnetent()
{
    croak("endnetent not implemented!\n");
}

void
win32_endprotoent()
{
    croak("endprotoent not implemented!\n");
}

void
win32_endservent()
{
    croak("endservent not implemented!\n");
}


struct netent *
win32_getnetent(void) 
{
    croak("getnetent not implemented!\n");
    return (struct netent *) NULL;
}

struct netent *
win32_getnetbyname(char *name) 
{
    croak("getnetbyname not implemented!\n");
    return (struct netent *)NULL;
}

struct netent *
win32_getnetbyaddr(long net, int type) 
{
    croak("getnetbyaddr not implemented!\n");
    return (struct netent *)NULL;
}

struct protoent *
win32_getprotoent(void) 
{
    croak("getprotoent not implemented!\n");
    return (struct protoent *) NULL;
}

struct servent *
win32_getservent(void) 
{
    croak("getservent not implemented!\n");
    return (struct servent *) NULL;
}

void
win32_sethostent(int stayopen)
{
    croak("sethostent not implemented!\n");
}


void
win32_setnetent(int stayopen)
{
    croak("setnetent not implemented!\n");
}


void
win32_setprotoent(int stayopen)
{
    croak("setprotoent not implemented!\n");
}


void
win32_setservent(int stayopen)
{
    croak("setservent not implemented!\n");
}

static struct servent*
win32_savecopyservent(struct servent*d, struct servent*s, const char *proto)
{
    d->s_name = s->s_name;
    d->s_aliases = s->s_aliases;
    d->s_port = s->s_port;
#ifndef __BORLANDC__	/* Buggy on Win95 and WinNT-with-Borland-WSOCK */
    if (!IsWin95() && s->s_proto && strlen(s->s_proto))
	d->s_proto = s->s_proto;
    else
#endif
	if (proto && strlen(proto))
	d->s_proto = (char *)proto;
    else
	d->s_proto = "tcp";
   
    return d;
}


