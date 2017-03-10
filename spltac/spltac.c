/* Sandip Nath Tiwari - 20162032 */
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAXNL 1000000		/* Maximum number of lines per output CHUNK file */ 
#define MAXSTR 4096			/* Maximum size of path string */
#define BUFSZ 4					/* Size of read/write buffer */

char outfile[MAXSTR];		/* Path of output CHUNK file */
char infile[MAXSTR];		/* PatH of input file */

off_t *stk;							/* Stack of File Offset values of newlines in input file */
off_t top, max;					/* Top of stack, Max limit of top(top grows towards max) */

/* Push off_t n onto stack */
#define PUSH(n) do{ if(top < max) stk[top++] = n; else exit(1); }while(0)   

/* Pop stack and compute line start and length (excluding newline) from stack (Pop one, compute 2 values) */
#define POP(st, num) do{ if(top > 1){ st = stk[top-2] + 1; num = stk[--top] - st; } else exit(1); }while(0) 

/* Initiate stack: set Top to zero and max to n */
#define INIT(n) do{ max = n; top = 0; }while(0)

/* Error strings for handle_error() to print */
const char msg_EACCES[] = ": Access Denied\n";
const char msg_ENOENT[] = ": No such file or directory\n";
const char msg_ENOTDIR[] = ": Not a directory\n";
const char msg_EPERM[] = ": Operation not permitted\n";
const char msg_default[] = ": ERROR\n";

const char msg_nosrc[] = "spltac: Source directory not specified.\n";
const char msg_nodest[] = "spltac: Destination directory not specified.\n";
const char msg_nolines[] = "spltac: Number of lines per CHUNK file not specified.\n";

/* Macros to help constrict switch/case from err number MACROS */
#define EXP_ERR_EX(x) case x : write(2, msg_ ## x , strlen( msg_ ## x )); exit(1)
#define EXP_ERR(x) case x : write(2, msg_ ## x , strlen( msg_ ## x ))

int mkdir_recursive(char *str);				/* recursively create all parents in path i.e. str */ 
int handle_error(char *s);						/* Print Error and exit(1) if serious */
void mknw_file(int fi, char *base_dir, char *flname, long ct);	/* Make output CHUNK file */
/* - Can I borrow this from the tr program? Can I? Can I?
 * - Yes, you can't. Just don't mess with handle_error(), that's all I'm saying. */
extern int printf2stderr(const char *fmt, ...); 

char buf[BUFSZ+1], buf2[BUFSZ+1];		/* I/O buffers for reading/writing i/p and o/p files */

int main(int argc, char *argv[])
{
	char *src_path, *dest_path;	
	long nlines;
	int inlen, outlen;

	/* Initialize I/P directory scanning structures */
	DIR *dp = NULL;
	struct dirent *dirp = NULL;
	
	/* Check cmdline arguments */
	if(argc <= 1)
	{
		printf2stderr(msg_nosrc);
		exit(1);
	}
	else if(argc <= 2)
	{
		printf2stderr(msg_nodest);
		exit(1);
	}
	else if(argc <= 3)
	{
		printf2stderr(msg_nolines);
		exit(1);
	}

	/* Get input from cmdline */
	src_path = argv[1];
	dest_path = argv[2];
	nlines = atol(argv[3]); /* We do not catch errors here: (maybe use strtol(...) ? )*/

	/* Acquire space for stack */
	stk = (off_t *) malloc(MAXNL * sizeof(off_t));


	/* Open source directory */
	dp = opendir(src_path);

	if(dp == NULL)
		handle_error(src_path);

	/* Make the destination directory (make parents as well) */
	mkdir_recursive(dest_path);

	/* Add a / at the end of input directory and stuff */
	strcpy(infile, src_path);
	inlen = strlen(infile);
	if(infile[inlen - 1] != '/')
	{
		infile[inlen++] = '/';
		infile[inlen] = '\0';
	}

	/* Add a / at the end of output directory and the same stuff */
	strcpy(outfile, dest_path);
	outlen = strlen(outfile);
	if(outfile[outlen - 1] != '/')
	{
		outfile[outlen++] = '/';
		outfile[outlen] = '\0';
	}
	
	while((dirp = readdir(dp)) != NULL)
	{
		/* Skip over current dir and parent dir dot files as well as other non-regular files */
		if(!(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..") || dirp->d_type != DT_REG))
		{
			int infldes;
			long ct_nl, llen, nr, ct_fl;
			off_t nc, cur;
			
			infile[inlen] = '\0';
			strcat(infile, dirp->d_name);  /* Set infile to "input_dir_path/input_file" */

			infldes = open(infile, O_RDONLY); /* Open input file. handle errors, if any */
			if(infldes < 0)
				handle_error(infile);

			/* Initialize Stack and Push (-1) at start (calc in POP demands -1 rather than 0) */
	    INIT(MAXNL);
			PUSH(-1);
			ct_nl = 0;		/* Count of newlines encountered so far */
			ct_fl = 0;		/* Count of output file CHUNK  written so far (for this Input file) */
			llen = 0;			/* Length of the last line seen */
			nc = 0;				/* offset of current scan position from start of i/p file*/

			while((nr = read(infldes, buf, (size_t)BUFSZ)) > 0) /* Read input file */
			{
				int i;

				for(i = 0; i < nr; i++)
				{
					if(buf[i] == '\n')
					{
						cur = lseek(infldes, 0, SEEK_CUR); /* Record current read posn. We'd need to restore read posn using this later */
						PUSH(nc);
						ct_nl++;

						if(ct_nl >= nlines)
						{
							off_t bs_next;
							ct_fl++;
							bs_next = stk[top-1];
							mknw_file(infldes, outfile, dirp->d_name, ct_fl);
							stk[0] = bs_next;
							outfile[outlen] = '\0';
							lseek(infldes, cur, SEEK_SET);
							ct_nl = 0;
						}
						llen = -1;
					}
					llen++;
					nc++;
				}
			}
			if(llen != 0)
			{
				PUSH(nc);
				ct_nl++;
			}
				
			if(ct_nl > 0)
			{
				ct_fl++;
				mknw_file(infldes, outfile, dirp->d_name, ct_fl);
				outfile[outlen] = '\0';
				llen = 0;
			}
			close(infldes);
		}
	}

	return 0;
}

/* Make new CHUNK file, number it ct, and tac lines to it:
 * Use stk for finding out which lines */
void mknw_file(int fi, char *base_dir, char *flname, long ct)
{
	char str[40];
	int fo;

	strcat(base_dir, flname);
	sprintf(str, "_%ld", ct);
	strcat(base_dir, str);

	fo = open(base_dir, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

	while(top > 1)
	{
		off_t st, num;
		char c;
		POP(st, num);
		
		lseek(fi, st, SEEK_SET);
		while((c = read(fi, buf2, (size_t)((num < BUFSZ) ? num : BUFSZ))) > 0)
		{
			write(fo, buf2, c);
			num -= c;
		}
		write(fo, "\n", 1);
	}

	close(fo);
}

/* Yeah, this function is everywhere! */
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
					case EACCES: printf("Error: Access Denied. Cannot create directory\n");
											 exit(1);
					case EPERM: printf("Error: Permission Denied. Cannot create directory\n");
											exit(1);
					case EEXIST: 
											break;
					default:	printf("Error: Cannot Create Directory\n");
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
			case EACCES: printf("Error: Access Denied. Cannot create directory\n");
									 exit(1);
			case EPERM: printf("Error: Permission Denied. Cannot create directory\n");
									exit(1);
			case EEXIST: 
									break;
			default:	printf("Error: Cannot Create Directory\n");
								exit(1);
		}
	}

	return 0;
}

/* Doesn't "handle" much:
 * Print a message and exits */
int handle_error(char *s)
{
	int sav = errno;
	
	write(2, "spltac: ", 8);
	write(2, s, strlen(s));

	switch(sav)
	{
		EXP_ERR_EX(EACCES);
		EXP_ERR_EX(EPERM);
		EXP_ERR_EX(ENOTDIR);
		EXP_ERR_EX(ENOENT);
		default:
			write(2, msg_default, strlen(msg_default));
			exit(1);
	}

	return 0;
}
