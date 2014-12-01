#include	<stdio.h>

#ifdef	_WIN32
    #include	<fcntl.h>
    #include	<io.h>
#else
    #include	<unistd.h>
    #include	<sys/types.h>
    #include	<sys/stat.h>
    #include	<fcntl.h>
    #define	O_BINARY	0
#endif

#include	<malloc.h>
#include	<memory.h>
#include	<string.h>
#include	"Data.h"

#pragma warning (disable:4996)

Data::Data(void)
{
}

Data::~Data(void)
{
	if (bits)
	{
		free (bits);
	}
}

Data :: Data ( const char * fileName )
{
// make a fix for windows to replace '/' in file path to Windows style '\\' if under windoze
	char * name = strdup ( fileName );

#ifdef	_WIN32
	char * ptr;

	while ( ( ptr = (char*)strchr ( fileName, '/' ) ) != NULL )
		*ptr = '\\';
#endif

	bits      = NULL;
	length    = 0;
	pos       = 0;

	int	file = open ( name, O_RDONLY | O_BINARY );

	free ( name );

	if ( file == -1 )
		return;

#ifndef _WIN32
	struct	stat statBuf;
	
	fstat ( file, &statBuf );
	
	long	len = statBuf.st_size; 
#else	
	long	len = filelength ( file );
#endif

	if ( len == -1 )
	{
		close ( file );

		return;
	}

	bits = (byte *) malloc ( len );

	if ( bits == NULL )
	{
		close ( file );

		return;
	}

	length = read ( file, bits, len );

	close ( file );
}

bool	Data :: isOk () const
{
	return bits != NULL;
}

void * Data :: getPtr ( int offs ) const
{
	if ( offs < 0 || offs >= length )
		return NULL;

	return bits + offs;
}

int	Data :: getBytes ( void * ptr, int len )
{
	if ( pos >= length )
		return -1;

	if ( pos + len > length )
		len = length - pos;

	memcpy ( ptr, bits + pos, len );

	pos += len;

	return len;
}


