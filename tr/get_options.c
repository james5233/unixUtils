/* Sandip Nath Tiwari - 20162032 */
#include "tr.h"

/* A Few error messages */
const char *es[] = {	
	"tr: missing operand\n",
	"tr: missing operand after \'%s\'\nTwo strings must be given when translating.\n",
	"tr: extra operand '\%s\'\n",
	"tr: extra operand '\%s\'\nOnly one string may be given when deleting without squeezing repeats.\n",
	"tr: missing operand after \'%s\'\nTwo strings must be given when both deleting and squeezing repeats.\n" };

const char msg_invalid_opt[] = "tr: invalid option -- \'%c\'\n";
const char msg_clubbed[] = "tr: options -I and -O cannot be clubbed with other options.\n";
const char msg_no_infile[] = "tr: no input file specified.\n";
const char msg_no_outfile[] = "tr: no output file specified.\n";

/* Get the Options from the cmdline and update corresponding flags and stuff in opts */
void get_options(int argc, char **argv, tropt_t *opts)
{
	int i;
	int invalid_opt;			/* Invalid option encountered flag */
	char invalid_optopt; 	/* value of invalid option */
	
	/* Option -I and -O must not be clubbed with other options */
	int clubbed;
	
	/* Flags mark if input file or output file is not specified */
	int no_infile, no_outfile; 	
	
	/* Comined value of d and c flags */
	int dc_comb, skip_one;

	/* Inilialize the error string and argument lookup tables */
	const char *err_msg[4][4] = { 	
		{ es[0],	es[1],	NULL,		es[2] },
		{ es[0], 	NULL, 	NULL, 	es[2] },
		{ es[0], 	NULL, 	es[3], 	es[2] },
		{ es[0], 	es[4], 	NULL, 	es[2] } };
	const int err_arg[4][4] = { 
		{0, 1, 0, 3},
		{0, 0, 0, 3},
		{0, 0, 2, 2},
		{0, 1, 0, 3} };

	/* Set flags to default values and counters to 0 */
	opts->c = opts->d = opts->s = opts->I = opts->O = FALSE;
	opts->valid = TRUE;
	opts->ct_set = opts->ct_opt = 0;
	opts->iset[1] = opts->iset[2] = opts->iset[3] = -1;

	invalid_opt = FALSE;
	clubbed = FALSE; /* Options I and O need to specified separately and can't be clubbed with others */
	no_infile = no_outfile = FALSE; /* Input or output file missing */
	skip_one = FALSE;

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
					case 'c': opts->c = TRUE;
										break;
					case 'd': opts->d = TRUE;
										break;
					case 's': opts->s = TRUE;
										break;
					case 'I': if(j != 1 || argv[i][j+1] != '\0')
										{
											clubbed = TRUE;
											break;
										}
										else if(i+1 >= argc)
										{
											no_infile = TRUE;
											break;
										}

										opts->I = TRUE;
										opts->iarg_I = ++i;
										skip_one = TRUE;
										break;
					case 'O': if(j != 1 || argv[i][j+1] != '\0')
										{
											clubbed = TRUE;
											break;
										}
										else if(i+1 >= argc)
										{
											no_outfile = TRUE;
											break;
										}

										opts->O = TRUE;
										opts->iarg_O = ++i;
										skip_one = TRUE;
										break;
					default : invalid_opt = TRUE; /* Invalid Option Flag */ 
										invalid_optopt = c; /* Actual invalid option captured for error output */
										break;
				}

				/* Invalid Option encountered: Print message to stderr and terminate */
				if(invalid_opt == TRUE)
				{
					/* I'm sending error to stdin because you don't like fprintf(stderr, ...) and 
					 * I'm too tired to do all the formatting and then do a write(2, ...) <-------+
					 *																																	 		      |
					 * There's a reason we have formatted i/o functions in libc : This is it -----+ 
					 * UPDATE: So... now the error goes to stderr (printf2stderr) */
					printf2stderr(msg_invalid_opt, invalid_optopt);
					exit(1);
				}
				else if(clubbed == TRUE)
				{
					printf2stderr(msg_clubbed);
					exit(1);
				}
				else if(no_infile == TRUE)
				{
					printf2stderr(msg_no_infile);
					exit(1);
				}
				else if(no_outfile == TRUE)
				{
					printf2stderr(msg_no_outfile);
					exit(1);
				}
				j++;

				if(skip_one == TRUE)
				{
					skip_one = FALSE;
					break;
				}
			}
		}
		else
		{
			opts->iset[++(opts->ct_set)] = i;
			if(opts->ct_set >= 3)
				break;
		}
	}

	dc_comb = opts->d * 2 + opts->s;
	if(err_msg[dc_comb][opts->ct_set] != NULL)
	{
		if(err_arg[dc_comb][opts->ct_set] == 0)
			printf2stderr(err_msg[dc_comb][opts->ct_set]);
		else
			printf2stderr(err_msg[dc_comb][opts->ct_set], argv[opts->iset[err_arg[dc_comb][opts->ct_set]]]);
		exit(1);
	}
}

