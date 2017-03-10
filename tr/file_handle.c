/* Sandip Nath Tiwari - 20162032 */
#include "tr.h"

/* Open file for reading return file descriptor */
int open_in_file(char *str)
{
	int fd;

	fd = open(str, O_RDONLY);
	if(fd < 0)
	{
		int save = errno;
		switch(save)
		{
			case ENOENT:
				printf2stderr("File/Path doesn't exist\n");
				break;
			case EACCES:
				printf2stderr("Permission denied\n");
				break;
		}
		exit(1);
	}

	return fd;
}

/* Create all directories in the path (.i.e. str) leading upto and
 * including the last */
int mkdir_recursive(char *str)
{
	char *s = str;
	int l, res;

	l = strlen(str);
	if(str[l-1] == '/')
		str[l-1] = '\0';

	while(*s)
	{
		if(*s == '/')
		{
			*s = '\0';
			res = mkdir(str, S_IRUSR | S_IWUSR | S_IXUSR);
			if(res < 0)
			{
				int save = errno;
				switch(save)
				{
					case EACCES: printf2stderr("Error: Access Denied. Cannot create directory\n");
											 exit(1);
					case EPERM: printf2stderr("Error: Permission Denied. Cannot create directory\n");
											exit(1);
					case EEXIST: 
											break;
					default:	printf2stderr("Error: Cannot Create Directory\n");
										exit(1);
				}
			}
			*s = '/';
		}
		s++;
	}

	res = mkdir(str, S_IRUSR | S_IWUSR | S_IXUSR);
	if(res < 0)
	{
		int save = errno;
		switch(save)
		{
			case EACCES: printf2stderr("Error: Access Denied. Cannot create directory\n");
									 exit(1);
			case EPERM: printf2stderr("Error: Permission Denied. Cannot create directory\n");
									exit(1);
			case EEXIST: 
									break;
			default:	printf2stderr("Error: Cannot Create Directory\n");
								exit(1);
		}
	}

	return 0;
}

/* Open File for Writing ; return file descriptor
 * Create if doesn't exist. Truncate to zero */
int open_out_file(char *str)
{
	int l = strlen(str);
	int i, fd;

	if(str[l-1] == '/')
	{
		printf2stderr("Invalid Output File\n");
		exit(1);
	}

	for(i = l-1; i >= 0 && str[i] != '/'; i--);

	if(i >= 0)
	{
		str[i] = '\0';
		mkdir_recursive(str);
		str[i] = '/';
	}

	fd = open(str, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(fd < 0)
	{
		int save = errno;
		switch(save)
		{
			case ENOENT:
				printf2stderr("File/Path doesn't exist\n");
				break;
			case EACCES:
				printf2stderr("Permission denied\n");
				break;
		}
		exit(1);
	}

	return fd;
}

#ifdef TESTING
int main(int argc, char *argv[])
{
	int res;

	res = open_out_file(argv[1]);
	if( write(res, "Done\n", 5) != 5)
		printf("Write Error\n");
	
	close(res);

	return 0;
}
#endif
