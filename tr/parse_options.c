/* Sandip Nath Tiwari - 20162032 */
#include "tr.h" 

/* Character classes used :
 * upper, lower, punct, space, digit */
const char *char_class[] = {
	NULL,
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
  "abcdefghijklmnopqrstuvwxyz",
  "!\"#$%&\'()*+,-./:;<=>\?@[\\]^_`{|}~",
  " \t\n\v\f\r",
  "0123456789" };

/* Some lovely errors and warnings that tr likes to throw */
const char msg_dangling_escape[] = "tr: warning: an unescaped backslash at end of string is not portable\n";
const char msg_octal[] = "tr: warning: the ambiguous octal escape \\%c%c%c is being\n\tinterpreted as the 2-byte sequence \\%c%c%c, %c\n";
const char msg_reverse_range[] = "tr: range-endpoints of \'%c-%c\' are in reverse collating sequence order\n";
const char msg_empty_str2[] = "tr: when not truncating set1, string2 must be non-empty\n";
const char msg_map_to_one[] = "tr: when translating with complemented character classes,\nstring2 must map all characters in the domain to one\n";
const char msg_invalid_class_in_str2[] = "tr: when translating, the only character classes that may appear in\nstring2 are \'upper\' and \'lower\'\n";
const char msg_misaligned[] = "tr: misaligned [:upper:] and/or [:lower:] construct\n";

/* Strip escape sequences from string */
void escaped2ascii(char *str, long *len)
{
	long j, k;
	char c, c1, c2, c3;

	for(j = 0, k = 0; (c = str[j]) != '\0'; j++)
	{
		if( c == '\\')
		{
			c1 = str[j+1];
			if(c1 == '\0')
			{
				printf2stderr(msg_dangling_escape);
				str[k++] = '\\';
				break;
			}
			
			switch(c1)
			{
				case '\'':
				case '\"':
				case '\?':
				case '\\': str[k++] = c1;
										j++; break;
				case 'a': str[k++] = '\a'; j++; break;
				case 'b': str[k++] = '\b'; j++; break;
				case 'f':	str[k++] = '\f'; j++; break;
				case 'n': str[k++] = '\n'; j++; break;
				case 'r': str[k++] = '\r'; j++; break;
				case 't': str[k++] = '\t'; j++; break;
				case 'v': str[k++] = '\v'; j++; break;
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					c2 = str[j+2];
					if( c2 < '0' || c2 > '7')
					{
						str[k++] = c1 - '0';
						j++; break;
					}
					c3 = str[j+3];
					if( c3 < '0' || c3 > '7')
					{
						str[k++] = (c1-'0')*8 + (c2-'0');
						j += 2; break;
					}
					if(c1 >= '4')
					{
						printf2stderr(msg_octal, c1, c2, c3, '0', c1, c2, c3);
						str[k++] = (c1-'0')*8 + (c2-'0');
						str[k++] = c3;
						j += 3; break;
					}
					str[k++] = (c1-'0')*64 + (c2-'0')*8 + (c3-'0');
					j += 3; break;
				default: str[k++] = c1;
									j++; break;
			}
		}
		else
			str[k++] = c;
	}

	*len = k;
	str[*len] = '\0';
}

/* Return the next token from the escape-sequence stripped string */
unsigned int next_token_ds(tokenizer_state_t *T)
{
	unsigned int c, c1, c2;

	if(T->c_ind != CCLASS_NULL)
	{
		c = (unsigned char)char_class[T->c_ind][T->c_nexti++];
		if (c =='\0')
		{
			T->c_ind = CCLASS_NULL;
			T->c_nexti = 0;
		}
		else
			return c;
	}
	else if(T->r != NO_RANGE)
	{
		c = T->r_next++;
		if(c > T->r_end)
			T->r = NO_RANGE;
		else
			return c;
	}

	c = (unsigned char)T->s[T->nexti];
	if(T->nexti >= T->len)
		return INVALID_TOKEN;
	
	if(c == '[')
	{
		int t = 0;
		if(	(t = 1 * !strncmp(T->s + T->nexti, "[:upper:]", 9)) ||
				(t = 2 * !strncmp(T->s + T->nexti, "[:lower:]", 9)) ||
				(t = 3 * !strncmp(T->s + T->nexti, "[:punct:]", 9)) ||
				(t = 4 * !strncmp(T->s + T->nexti, "[:space:]", 9)) ||
				(t = 5 * !strncmp(T->s + T->nexti, "[:digit:]", 9)) )
		{
			T->c_ind = t;
			T->c_nexti = 1;
			T->nexti += 9;
			return (c =  (unsigned char)char_class[T->c_ind][0]);
		}
	}

	c1 = (unsigned char)T->s[T->nexti + 1];
	if(c1 == '\0')
	{
		T->nexti++;
		return c;
	}

	if(c1 == '-')
	{
		c2 = (unsigned char)T->s[T->nexti + 2];
		if( c2 == '\0' )
		{
			T->nexti++;
			return c;
		}
		
		if (c2 < c)
		{
			printf2stderr(msg_reverse_range, (char)c, (char)c2);
			exit(1);
		}

		T->r = !NO_RANGE;
		T->r_st = c;
		T->r_end = c2;
		T->r_next = c + 1;
		T->nexti += 3;

		return c;
	}

	T->nexti++;
	return c;
}

/* Take an post escape-conversion array and set filter */
void set_filter(char *set, int len, char *filt, int comp)
{
	int i;
	unsigned int c;
	tokenizer_state_t T = {0};
	
	T.s = set;
	T.len = len;

	for(i = 0; i < MAXARR; i++)
		filt[i] = 0;
	
	while((c = next_token_ds(&T)) != INVALID_TOKEN)
		filt[c] = 1;

	if(comp)
	{
		for(i = 0; i < MAXARR; i++)
			filt[i] = !filt[i];
	}
}


/* Set the translation map used by tr */
void set_translate(char *set1, int len1, char *set2, int len2, char *trfilt, char *trmap,  int comp)
{
	int i;
	tokenizer_state_t T1 = {0}, T2 = {0};

	T1.s = set1;
	T1.len = len1;

	T2.s = set2;
	T2.len = len2;

	for(i = 0; i < MAXARR; i++)
	{
		trfilt[i] = 0;
		trmap[i] = i;
	}

	if(comp != 1)
	{
		unsigned int c1, c2;
		int last_token = INVALID_TOKEN;

		while((c1 = next_token_ds(&T1)) != INVALID_TOKEN)
		{
			c2 = next_token_ds(&T2);
			if(c2 == INVALID_TOKEN)
			{
				if(last_token == INVALID_TOKEN)
				{
					printf2stderr(msg_empty_str2);
					exit(1);
				}

				c2 = last_token;
			}
			else
				last_token = c2;

			if(T2.c_ind != CCLASS_NULL)
			{
				if(T2.c_ind != CCLASS_UPPER && T2.c_ind != CCLASS_LOWER)
				{
					printf2stderr(msg_invalid_class_in_str2);
					exit(1);
				}

				if(T1.c_ind != CCLASS_UPPER && T1.c_ind != CCLASS_LOWER)
				{
					printf2stderr(msg_misaligned);
					exit(0);
				}
			}

			trmap[c1] = c2;
		}
	}
	else
	{
		unsigned int c1, c2;
		unsigned int saved_token;

		saved_token = INVALID_TOKEN;
		
		while((c1 = next_token_ds(&T1)) != INVALID_TOKEN)
		{
			if(T1.c_ind != CCLASS_NULL)
			{
				while((c2 = next_token_ds(&T2)) != INVALID_TOKEN)
				{
					if(T2.c_ind != CCLASS_NULL)
					{
						if(T2.c_ind == CCLASS_UPPER || T2.c_ind == CCLASS_LOWER)
						{
							printf2stderr(msg_misaligned);
							exit(1);
						}

						printf2stderr(msg_invalid_class_in_str2);
						exit(1);
					}

					if(saved_token == INVALID_TOKEN)
						saved_token = c2;

					if(c2 != saved_token)
					{
						printf2stderr(msg_map_to_one);
						exit(1);
					}
				}

				if(c2 == INVALID_TOKEN && saved_token == INVALID_TOKEN)
				{
					printf2stderr(msg_empty_str2);
					exit(1);
				}
			}

			trfilt[c1] = 1;
		}

		if(saved_token != INVALID_TOKEN)
		{
			for(i = 0; i < MAXARR; i++)
				if(trfilt[i] != 1)
					trmap[i] = saved_token;
		}
		else
		{
			unsigned int last_token = INVALID_TOKEN;
			for(i = 0; i < MAXARR; i++)
			{
				if(trfilt[i] != 1)
				{
					c2 = next_token_ds(&T2);
					if(c2 == INVALID_TOKEN)
					{
						if(last_token == INVALID_TOKEN)
						{
							printf2stderr(msg_empty_str2);
							exit(1);
						}
						c2 = last_token;
					}
					else
						last_token = c2;

					trmap[i] = c2;
				}
			}
		}
	}
}
					

#ifdef TESTING
int main(int argc, char *argv[])
{
	long len, i;
	unsigned int c;
	tokenizer_state_t T = {0};

	printf("[%s]\n", argv[1]);
	escaped2ascii(argv[1], &len);

	printf("{");
	for(i = 0; i < len; i++)
		printf(" %d,", (int)argv[1][i]);
	printf("}\n");
	
	T.s = argv[1];
	T.len = len;

	printf("Tokens:");
	while((c = next_token_ds(&T)) != INVALID_TOKEN )
		printf(" %u,", c);
	printf("\n");

	return 0;
}
#endif
