/* Sandip Nath Tiwari - 20162032 */
#include <stdio.h> 
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

/* cmdline arguments parsing data */
typedef struct
{
	int c, d, s;
	int I, O;
	int iarg_I, iarg_O;
	int iset[4];
	int ct_set, ct_opt;
	int valid;
} tropt_t;

/* tokenization of sets - state info */
typedef struct
{
	char *s;
	long len;
	long nexti;
	int r;
	unsigned int r_st, r_end, r_next;
	int c_ind;
	int c_nexti;
} tokenizer_state_t;


#define NO_RANGE 0 
#define NO_CLASS 0

#define CCLASS_NULL  0
#define CCLASS_UPPER 1
#define CCLASS_LOWER 2
#define CCLASS_PUNCT 3
#define CCLASS_SPACE 4
#define CCLASS_DIGIT 5

#define MAXARR 256
#define INVALID_TOKEN 400

extern char tr_map[];
extern char del_filter[];
extern char sq_filter[];
extern char tr_filter[];

/* get_options.c */
extern void get_options(int argc, char **argv, tropt_t *opts);

/* parse_options.c */
extern void parse_sets(char **argv, tropt_t *opts);
void escaped2ascii(char *str, long *len);
unsigned int next_token_ds(tokenizer_state_t *T);
void set_filter(char *set, int len, char *filt, int comp);
void set_translate(char *set1, int len1, char *set2, int len2, char *trfilt, char *trmap,  int comp);

/* file_handle.c */
int open_in_file(char *str);
int mkdir_recursive(char *str);
int open_out_file(char *str);

/* printf2stderr.c */
int printf2stderr(const char *fmt, ...);
