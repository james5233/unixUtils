/* Sandip Nath Tiwari - 20162032 */
#include <unistd.h>
#include <fcntl.h>		/* For open() */
#include <stdio.h>		/* For formatted IO : printf() and scanf() only */
#include <stdlib.h> 	/* For exit() only */
#include <errno.h>		/* For error codes */
#include <string.h>		

#define TRUE 1
#define FALSE 0
#define OP_FMT_STR " %*ld"
#define BUFSZ 1024
#define TABWIDTH 4

#define IS_WHITE(c) (((c) == ' ') || ((c) == '\t') || ((c) == '\n') || ((c) == '\v') || ((c) == '\f'))
#define IS_NL(c) ((c) == '\n')
#define IS_HTAB(c) ((c) == '\t')

/* Yeah, again. Because it's better than handle_error() */
extern int printf2stderr(const char *fmt, ...);

typedef struct
{
	long nc, nm, nl, L, nw;
	int err;
} sv_t;

char msg_invalid_opt[] = "mywc: invalid option -- \'%c\'\n";
char buf[BUFSZ];

int ndig(unsigned long n);
void get_options(int argc, char **argv, int *opt_c, int *opt_m, int *opt_l, int *opt_L, int *opt_w, int *ct_opt, int *ct_fl);
int wc_file(char *flname, long *nc, long *nm, long *nl, long *L, long *nw, int *err);
int handle_error(char *s);

int main(int argc, char *argv[])
{
	int i;
	int opt_c, opt_m, opt_l, opt_L, opt_w;	/* Option flags */
	int ct_opt; 														/* Count of option flags set */
	int ct_fl;															/* Count of files on cmdline */
	int err;																/* Error variable */
	int fl_i;																/* File index */

	long tot_c, tot_m, tot_l, max_L, tot_w; /* Total stats */
	long nc, nm, nl, L, nw;									/* Stats per file */

	int fw_c, fw_m, fw_l, fw_L, fw_w;				/* Field widths for printing */
	sv_t *sv_ptr;														/* Pointer to array for stat saving */

	/* Initialize option flags to default values */
	opt_c = opt_w = opt_l = TRUE;
	opt_L = opt_m = FALSE;

	/* Initialize counters to default values */
	tot_c = tot_m = tot_l = max_L = tot_w = 0;
	ct_opt = 3;
	ct_fl = 0;
	
	/* Initialise save ptr to NULL */
	sv_ptr = NULL;

	/* Initialise field widths to defaults */
	fw_c = fw_m = fw_l = fw_L = fw_w = 7;

	/* Set the option flags and option and file counters */
	get_options(argc, argv, &opt_c, &opt_m, &opt_l, &opt_L, &opt_w, &ct_opt, &ct_fl);

	/* Allocate space for saving the stats */
	sv_ptr = (sv_t *)malloc(ct_fl * sizeof(sv_t));
	
	/* Cycle through the cmdline arguments */
	fl_i = 0;
	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] != '-')			/* Process only filenames */
		{
			char *flname = argv[i];
			err = 0;
			int res = wc_file(flname, &nc, &nm, &nl, &L, &nw, &err); /* Count stats for file */
			if(res == 0)       /* If no error was encountered */
			{
				tot_c += nc;		/* Calculate running totals for c, m, l, w */
				tot_m += nm;
				tot_l += nl;
				if (L > max_L)  /* For L calculate running max */
					max_L = L;
				tot_w += nw;

				/* Store the stats */
				sv_ptr[fl_i].nc = nc;
				sv_ptr[fl_i].nm = nm;
				sv_ptr[fl_i].nl = nl;
				sv_ptr[fl_i].L  = L;
				sv_ptr[fl_i].nw = nw;
				sv_ptr[fl_i].err = err;
			}
			else
				sv_ptr[fl_i].err = err;

			fl_i++;
		}
	}

	fw_c = ndig(tot_c);
	fw_m = ndig(tot_m);
	fw_l = ndig(tot_l);
	fw_L = ndig(max_L);
	fw_w = ndig(tot_w);

	fl_i = 0;
	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] != '-')			/* Process only filenames */
		{
			char *flname = argv[i];
			if(sv_ptr[fl_i].err != 0)
				handle_error(flname);
			else
			{
				if(opt_l == TRUE)
					printf(OP_FMT_STR, fw_l, sv_ptr[fl_i].nl);
				if(opt_w == TRUE)
					printf(OP_FMT_STR, fw_w, sv_ptr[fl_i].nw);
				if(opt_m == TRUE)
					printf(OP_FMT_STR, fw_m, sv_ptr[fl_i].nm);
				if(opt_c == TRUE)
					printf(OP_FMT_STR, fw_c, sv_ptr[fl_i].nc); /* Print max for L */
				if(opt_L == TRUE)
					printf(OP_FMT_STR, fw_L, sv_ptr[fl_i].L);
				printf(" %s\n", flname);
			}
			fl_i++;
		}
	}

	if(ct_fl > 1) /* If there were more than one files on the cmdline */
	{
		/* Print Totals for c, m, l, and w */
		if(opt_l == TRUE)
			printf(OP_FMT_STR, fw_l, tot_l);
		if(opt_w == TRUE)
			printf(OP_FMT_STR, fw_w, tot_w);
		if(opt_m == TRUE)
			printf(OP_FMT_STR, fw_m, tot_m);
		if(opt_c == TRUE)
			printf(OP_FMT_STR, fw_c, tot_c); /* Print max for L */
		if(opt_L == TRUE)
			printf(OP_FMT_STR, fw_L, max_L);

		/* Mark these as totals */
		printf(" total\n");
	}

	return 0;
}


void get_options(int argc, char **argv, int *opt_c, int *opt_m, int *opt_l, int *opt_L, int *opt_w, int *ct_opt, int *ct_fl)
{
	int i;
	int invalid_opt;			/* Invalid option encountered flag */
	char invalid_optopt; 	/* value of invalid option */

	/* Set flags to default values and counters to 0 */
	*opt_c = *opt_m = *opt_l = *opt_L = *opt_w = FALSE;
	invalid_opt = FALSE;
	*ct_opt = *ct_fl = 0;

	/* Scan through all the args and setting flags for appropriate options */
	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-') 						/* Options start with a '-' */
		{
			int j = 1;
			char c;
			while((c = argv[i][j]) != '\0') /* Options may be grouped together, Process all*/
			{
				switch(c)
				{
					case 'c': *opt_c = TRUE;
										break;
					case 'm': *opt_m = TRUE;
										break;
					case 'l': *opt_l = TRUE;
										break;
					case 'L': *opt_L = TRUE;
										break;
					case 'w': *opt_w = TRUE;
										break;
					default : invalid_opt = TRUE; /* Invalid Option Flag */ 
										invalid_optopt = c; /* Actual invalid option captured for error output */
										break;
				}

				/* Invalid Option encountered: Print message to stderr and terminate */
				if(invalid_opt == TRUE)
				{
					printf2stderr(msg_invalid_opt, invalid_optopt);
					exit(1);
				}
				j++;
			}
		}
		else
			(*ct_fl)++;
	}

	/* Set default options flag if no option is specified */
	*ct_opt = (	*opt_c == TRUE ) + 
		(	*opt_m == TRUE ) +
		(	*opt_l == TRUE ) +
		(	*opt_L == TRUE ) +
		(	*opt_w == TRUE );
	
	/* Count the number of options and set default options is none are specified */
	if(*ct_opt == 0)
	{
		*opt_l = *opt_w = *opt_c = TRUE;
		*ct_opt = 3;
	}
}


#define INW 0
#define OUTW 1

int wc_file(char *flname, long *nc, long *nm, long *nl, long *L, long *nw, int *err)
{
	int fldes;		/* File descriptor for reading */
	int state;		/* marks whether marker is in a word or outside */
	int i;
	long curL;		/* Running length of current line */

	/* Initialise counters and states */
	state = OUTW;
	curL = 0;
	*nc = *nm = *nl = *L = *nw = 0;
	
	/* open the file */
	fldes = open(flname, O_RDONLY);

	/* Error in file opening : return failure, caller prints message*/
	if(fldes < 0)
	{
		*err = errno;
		return -1;
	}
	else								/* File opened successfully */
	{
		ssize_t nr;
		
		while((nr = read(fldes, buf, (size_t)BUFSZ)) > 0) /* Characters read into buffer*/
		{
			for(i = 0; i < nr; i++)				/* Process character in buffer */
			{
				char c = buf[i];
				if(IS_NL(c))								/* Encountered a newline */
				{
					(*nl)++;									/* Increment the number of newlines */
					if (curL > *L)						/* Update the running max line-length variable */
						*L = curL;
					curL = 0;									/* Reset current line-length counter (A new lines begins) */
				}
				else if(IS_HTAB(c))
					curL = ((curL + TABWIDTH)/TABWIDTH)*TABWIDTH;
				else
					curL++;

				if(IS_WHITE(c)) 						/* Encountered white space */
				{
					if(state == INW)					/* If a word was going on, it just ended */
					{	
						(*nw)++;								/* Increment no. of words*/
						state = OUTW;						/* Now, out of word */
					}
				}
				else
					state = INW;							/* still in a word */

				(*nc)++;										/* Characters and bytes incremented */
				(*nm)++;
			}
		}

		if(nr < 0)											/* Error in read() : Print error and return failure */
		{
			*err = errno;
			close(fldes);
			return -1;
		}
	}

	close(fldes);
	return 0;													/* Successful return */
}

/* This thing again! */
int handle_error(char *s)
{
	int sav = errno;									/* Save error variable to return it to caller */
	
	write(2, "wc: ", 4);
	write(2, s, strlen(s));
	switch(sav)										/* Print error string to teh console */
	{
		case EACCES: write(2, ": Access Denied\n", 16);
									break;
		case EPERM: write(2, ": Permission Denied\n", 20);
									break;
		case ENOENT: write(2, ": No such file or directory\n", 28);
									break;
		default: write(2, ": Error\n", 8);
	}
						
	return sav;
}

/* Count the number of digits in positive long number */
int ndig(unsigned long n)
{
	int nd = 0;
	do
	{
		nd++;
		n /= 10;
	}while(n);

	return nd;
}
