/*
 * Wrapper - A word wrap filter.
 *
 * Adds a custum string to each line and wraps the words that get outside
 * the wrap column.
 *
 * Reads from stdin, writes output to stdout.
 *
 * Author: Daniel Stenberg
 * Date: 941012
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define TRUE  1
#define FALSE 0

#define WRAP_STRING "> "
#define WRAP_COLUMN 79
#define WRAP_FILEBUFFER_SIZE 1024

int Wrap(void);
int Pipe(char *);
int Puts(char *);
int WhiteCount(char *);
int WordCount(char *);
int PutNchar(char *, int);
int Putchar(int);

static int column; /* right border column */
static int currcol; /* current column */
static char *leftside; /* left side string */

static char shutoff = FALSE; /* shutoff the 'fancy' option ? */

int main(int argc, char *argv[])
{
	int ret;
	column = WRAP_COLUMN;
	leftside = WRAP_STRING;
	if(argc>1) {
		int n=1;
		while(n<argc) {
			if(argv[n][0] == '-') {
				switch(argv[n][1]) {
				  case 's':
					if(n+1<argc)
						leftside = argv[n+1];
					n+=2;
					break;
				  case 'c':
					if(n+1<argc)
						column = atoi(argv[n+1]);
					n+=2;
					break;
				  case 'q':
					shutoff = TRUE;
					n++;
					break;
				  default:
					printf("Usage: wrap [-scq] <input >output\n\n"
						   "-s [string]   Left side string\n"
						   "-c [column]   Word wrap column\n"
						   "-q            Shut off the 'fancy' wrapping\n");
					return 0;
				}
			}
		}
	}

	if(column < strlen(leftside) + 10) {
		printf("*** ABORTED!\nStupid wrap column/string argument!\n");
		return 0;
	}

	ret = Wrap();

	return ret;
}

int Wrap(void)
{
	int ret=0;
	char buffer[WRAP_FILEBUFFER_SIZE];
	int len;
	int prevlen=0;
	int linenum=0;
	int newlines=0;
	buffer[0]=0;
	while(fgets(buffer, WRAP_FILEBUFFER_SIZE, stdin)) {
		linenum++;
		len = strlen(buffer);
		if(buffer[len-1] == '\n') {
			/* remove trailing newline character */
			buffer[len-1] = 0;
		}
		if(isspace(buffer[0]) || len==1) {
			if(linenum>1) {
				/* This is the beginning of a new paragraph. */
				Putchar('\n');
				newlines++;
			}
		}
		else {
			if(prevlen == 1 || shutoff) {
				Putchar('\n');
				newlines=0;
			} else
				Putchar(' ');
		}
		prevlen=len;
		Pipe(buffer);
	}
	return ret;
}

int Pipe(char *string)
{
	int white;
	int word;
	int left;
	while(*string) {
		if(!currcol) {
			/* hmmm, pretend we have the leftside already written! */
			left = strlen(leftside);
		}
		else
			left = 0;

		white = WhiteCount(string);
		word = WordCount(string + white);
		if(word) {
			/*
			 * This is only for white + word
			 */
			if(currcol + left + white + word <column) {
				PutNchar(string, white+word);
			}
			else {
				Putchar('\n');
				PutNchar(string + white, word);
			}
		}
		string += white + word;
	}
	return 0;
}

int WhiteCount(char *string)
{
	int number=0;
	while(*string && isspace(*string)) {
		string++;
		number++;
	}
	return number;
}

int WordCount(char *string)
{
	int number=0;
	while(*string && !isspace(*string)) {
		string++;
		number++;
	}
	return number;
}

int Puts(char *string)
{
	while(*string) {
		Putchar(*string);
		string++;
	}
	return 0;
}

int Putchar(int letter)
{
	if(letter != '\n') {
		if(!currcol) {
			currcol++;
			Puts(leftside);
		} else if(currcol >= column) {
			putchar('\n');
			currcol=1;
			Puts(leftside);
		} else
			currcol++;
	}
	else {
		currcol = 0;
	}
	if(letter) {
		putchar(letter);
	}
	else
		currcol--;
	return 0;
}

int PutNchar(char *string, int N)
{
	while(N--) {
		Putchar(*string++);
	}
	return 0;
}
