/* Sandip Nath Tiwari - 20162032 */
#include "tr.h"

/* I/O buffer size */
#define BUFSZ 1024

/* Input and output buffers */
char inbuf[BUFSZ+1], outbuf[BUFSZ+1];

/* Filters for -d and -s
 * A Map for translation and a helper filter
 * for complementing before translating */
char del_filter[MAXARR], sq_filter[MAXARR];
char tr_filter[MAXARR], tr_map[MAXARR];

int main(int argc, char *argv[])
{
	tropt_t opts;
	char *sets[3];
	long len[3];
	int last_invalid, last, c, i;
	int infldes = 0, outfldes = 1;
	ssize_t nr;

	get_options(argc, argv, &opts);
	for(i = 1; i <= opts.ct_set; i++)
	{
		sets[i] = argv[opts.iset[i]];
		escaped2ascii(sets[i], len + i);
	}

	if(opts.d == TRUE)
		set_filter(sets[1], len[1], del_filter, opts.c == TRUE);
	else if(opts.ct_set == 2)
		set_translate(sets[1], len[1], sets[2], len[2], tr_filter, tr_map, (opts.c == TRUE));

	if(opts.s == TRUE)
		set_filter(sets[opts.ct_set], len[opts.ct_set], sq_filter, (opts.c == TRUE) && (opts.ct_set == 1));
	
	if(opts.I == TRUE)
		infldes = open_in_file(argv[opts.iarg_I]);

	if(opts.O == TRUE)
		outfldes = open_out_file(argv[opts.iarg_O]);

	last_invalid = 1;
	last = 0;
	while((nr = read(infldes, inbuf, (size_t)BUFSZ)) > 0)
	{
		int ni, no, nw, nwc;
		for(ni = 0, no = 0; ni < nr; ni++)
		{
			c = (unsigned char)inbuf[ni];

			if(opts.d == TRUE)
			{
				if(del_filter[c] == 1)
					continue;
			}
			else if(opts.ct_set == 2)
				c = tr_map[c];

			if(opts.s == TRUE)
			{
				if(!last_invalid && sq_filter[c] == 1 && last == c)
					continue;

				last = c;
				last_invalid = 0;
			}

			outbuf[no++] = c;
		}
		
		nw = 0; nwc = 0;
		while((nw = write(outfldes, outbuf + nwc, (size_t)(no - nwc))) > 0)
			nwc += nw;
		if(nw < 0)
		{
			printf2stderr("I/O Error: Input file cannot be read\n");
			exit(1);
		}
	}

	if(nr < 0)
	{
		printf2stderr("I/O Error: Output file cannot be written\n");
		exit(1);
	}
	
	close(infldes);
	close(outfldes);

	return 0;
}

