/* Program to generate term-biased snippets for paragraphs of text.

   Skeleton program written by Alistair Moffat, ammoffat@unimelb.edu.au,
   August 2022, with the intention that it be modified by students
   to add functionality, as required by the assignment specification.

   Student Authorship Declaration:

   (1) I certify that except for the code provided in the initial skeleton
   file, the  program contained in this submission is completely my own
   individual work, except where explicitly noted by further comments that
   provide details otherwise.  I understand that work that has been developed
   by another student, or by me in collaboration with other students, or by
   non-students as a result of request, solicitation, or payment, may not be
   submitted for assessment in this subject.  I understand that submitting for
   assessment work developed by or in collaboration with other students or
   non-students constitutes Academic Misconduct, and may be penalized by mark
   deductions, or by other penalties determined via the University of
   Melbourne Academic Honesty Policy, as described at
   https://academicintegrity.unimelb.edu.au.

   (2) I also certify that I have not provided a copy of this work in either
   softcopy or hardcopy or any other form to any other student, and nor will I
   do so until after the marks are released. I understand that providing my
   work to other students, regardless of my intention or any undertakings made
   to me by that other student, is also Academic Misconduct.

   (3) I further understand that providing a copy of the assignment
   specification to any form of code authoring or assignment tutoring service,
   or drawing the attention of others to such services and code that may have
   been made available via such a service, may be regarded as Student General
   Misconduct (interfering with the teaching activities of the University
   and/or inciting others to commit Academic Misconduct).  I understand that
   an allegation of Student General Misconduct may arise regardless of whether
   or not I personally make use of such solutions or sought benefit from such
   actions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

	/* you may need to delete this next line on some platforms
	   in order to get it to compile; if you end up doing that
	   make sure you put it back before you submit to the
	   Gradescope system, because it is required there */
#include <strings.h>

	/* maximum number of characters per word */
#define MAX_WORD_LEN 23
	/* maximum number of words per paragraph */
#define MAX_PARA_LEN 10000

	/* return code from get_word if end of paragraph found */
#define PARA_END 1
	/* return code from get_word if a word successfully found */
#define WORD_FND 2

	/* terminating punctuation that may follow a word */
#define TERM_PUNCT ".,;:!?"
	/* terminating punctuation that needs dots added */
#define NEEDS_DOTS ",;:"
	/* the string the separates paragraphs */
#define PARA_SEPARATOR "\n\n"
	/* insertion characters to indicate "bold" */
#define BBOLD "**"
	/* length of those characters when wrapped around a word */
#define BBOLD_LEN (2*strlen(BBOLD))
	/* insertion characters to indicate omitted text */
#define DDOTS "..."

	/* maximum words in any output snippet */
#define MAX_SNIPPET_LEN 30
	/* minimum words in any output snippet */
#define MIN_SNIPPET_LEN 20
	/* maximum length of output lines */
#define MAX_OUTPUT_LINE 72

	/* maximum terms allowed on command line */
#define MAX_TERMS 50
	/* signal for no match between word and query term */
#define NO_MATCH (-1)

typedef char word_t[MAX_WORD_LEN+1];
int     get_word(word_t w, int limit);

/* If you wish to add further #defines, put them below this comment,
   then prototypes for the functions that you add

   The only thing you should alter above this line is to complete the
   Authorship Declaration
*/

/****************************************************************/

/* contsants for calculating snippet score from stage 3 */
#define START_MULTIPLIER 15.0
#define START_ADDITIVE 10.0
#define NEW_WORD_MULTIPLIER 0.5
#define REPEAT_WORD_SCORE 1.0
#define BEFORE_PUNCT_SCORE 0.6
#define END_PUNCT_SCORE 0.3
#define LENGTH_MULTIPLIER 0.1

typedef word_t paragraph_t[MAX_PARA_LEN+1];
typedef word_t snippet_t[MAX_SNIPPET_LEN+1];
int	get_paragraph(paragraph_t p, int para_num, int argc, char *argv[]);
int	print_paragraph(paragraph_t p, int para_num, int para_size,
			int argc, char *argv[]);
int	make_bold(word_t w);
double 	score_snippet(snippet_t s, int start,
			int before_punct, int snippet_len);
int 	word_found(word_t w, word_t found[], int i);
double	find_best_snippet(paragraph_t p, snippet_t s, int para_size,
			int *start, int *len);
int 	has_punct(word_t w);
int	print_snippet(snippet_t s);

/* main program controls all the action
*/
int
main(int argc, char *argv[]) {
	paragraph_t p;
	int para_size = 0;
	int para_num = 1;

	printf("\n");

	/* Goes through all the paragraphs separately */
	while ((para_size=get_paragraph(p, para_num, argc, argv))!=EOF) {
		/* The get_paragraph func above completes stages 1 and 2
		stage 3 is done below
		*/
		snippet_t s;
		int start, len;
		double best_score = find_best_snippet(p, s, para_size,
							&start, &len);

		printf(
		"======= Stage 3 [para %d; start %d; length %d; score %.2f]\n",
		para_num,start,len,best_score);
		print_snippet(s);

		para_num++;
	}
	printf("ta daa!\n");
	return 0;
}

/****************************************************************/


/* extract a single word out of the standard input, but not
   more than "limit" characters in total. One character of
   sensible trailing punctuation is retained.
   argument array W must be limit+1 characters or bigger
*/
int
get_word(word_t W, int limit) {
	int c;
	char end_para[] = PARA_SEPARATOR;
	char *start = W;
	/* first, skip over any non alphanumerics */
	while ((c=getchar())!=EOF && !isalnum(c)) {
		*W = c;
		W += 1;
		*W = '\0';
		/* next, check for the para_separator string */
		if (!strcmp(end_para, start)) {
			return PARA_END;
		}
		/* allow PARA_SEPARATOR to be found when it's after
		another non alnum character */
		if (strchr(end_para, c)==NULL) {
			W = start;
		}
	}
	/* Ensure that the stages are executed for a paragraph even if it's at
	the end of the file. It won't have a PARA_SEPARATOR flag in that case */
	if (c==EOF) {
		if (strchr(end_para,*start)) {
			return PARA_END;
		}
		return EOF;
	}

	W = start;
	/* ok, first character of next word has been found */
	*W = c;
	W += 1;
	limit -= 1;
	/* keep looking through characters until the end of the word is found */
	while (limit>0 && (c=getchar())!=EOF && isalnum(c)) {
		*W = c;
		W += 1;
		limit -= 1;
	}
	/* take a look at that next character, is it a sensible trailing
	   punctuation? */
	if (strchr(TERM_PUNCT, c) && (limit>0)) {
		/* yes, it is */
		*W = c;
		W += 1;
		limit -= 1;
	}

	/* now close off the string */
	*W = '\0';
	return WORD_FND;
}

/****************************************************************/


/* reads in words from the get_word function and stores them in a paragraph
data type for later use until a PARA_SEPARATOR flag is reached. It then counts
the number of words in the paragraph and displays this info as per stage 1
requirements.
*/
int
get_paragraph(paragraph_t P, int para_num, int argc, char *argv[]){
	word_t w;
	word_t *start_p = P;
	int r;
	while ((r=get_word(w, MAX_WORD_LEN))!=PARA_END && r!=EOF) {
		strcpy(*P,w);
		P++;
	}
	if (r==EOF) {
		return EOF;
	}
	int para_size = P-start_p;
	printf("======= Stage 1 [para %d; %d words]\n\n",para_num,para_size);

	print_paragraph(start_p, para_num, para_size, argc, argv);
	return (P-start_p);
}

/****************************************************************/

/* takes a paragraph along with info about its length and the arguments passed
to the command line, and completes stage 2 by finding all words in the
paragraph matching a query term and makes them 'bold' and then prints the
nummber of matches as stage 2 requires and then prints the full paragraph
*/
int
print_paragraph(paragraph_t P, int para_num, int para_size,
		int argc, char *argv[]) {
	char term_punct[] = TERM_PUNCT;
	int num_of_matches = 0;
	int bytes_on_line = 0;
	for (int i=0; i<para_size; i++) {
		word_t word;
		strcpy(word, P[i]);

		if (strchr(term_punct, word[strlen(word)-1])) {
			word[strlen(word)-1] = '\0';
		}

		for (int j=1; j<argc; j++) {
			if (!strcasecmp(word, argv[j])) {
				num_of_matches++;
				make_bold(P[i]);
			}
		}
	}


	printf("======= Stage 2 [para %d; %d words; %d matches]\n",
		para_num,para_size,num_of_matches);

	for (int i=0; i<para_size; i++) {
		if (bytes_on_line + strlen(P[i]) > MAX_OUTPUT_LINE) {
			printf("\n");
			bytes_on_line = 0;
		} else if (bytes_on_line) {
			printf(" ");
		}
		printf("%s",P[i]);
		bytes_on_line += strlen(P[i])+1;
	}
	printf("\n\n");

	return 0;
}

/****************************************************************/

/* Takes a pointer to a word and makes it 'bold' by inserting the BBOLD string
on each side of the word, making sure to keep punctuation on the outside of the
bold string.
*/
int
make_bold(word_t w) {
	word_t new_w;
	char term_punct[] = TERM_PUNCT;
	char bbold[] = BBOLD;
	char removed = '\0';
	/* Put the bold indicator into the start of the new word */
	for (int i=0; bbold[i]; i++) {
		new_w[i] = bbold[i];
	}
	/* Remove trailing punctuation and copy the word to the new word */
	for (int c=0; w[c]; c++) {
		if (strchr(term_punct, w[c])) {
			removed = w[c];
			w[c] = '\0';
		}
		new_w[c+BBOLD_LEN/2] = w[c];
	}
	/* Add trailing bold indicator */
	for (int i=0; i<=strlen(bbold); i++) {
		new_w[i+strlen(w)+BBOLD_LEN/2] = bbold[i];
	}
	/* put trailing punctuation back in */
	int index = strlen(new_w);
	new_w[index] = removed;
	new_w[index+1] = '\0';

	strcpy(w, new_w);

	return 0;
}

/****************************************************************/

/* Calculates a score for a particular snippet of text based on the requirements
of stage 3. It only handles cases where the paragraph is longer than the
required length as shorter ones are handled in the function find_best_snippet
*/
double
score_snippet(snippet_t s, int start, int before_punct, int snippet_len) {
	word_t found[MAX_SNIPPET_LEN];
	double score = 0;
	char bbold[] = BBOLD;

	score += START_MULTIPLIER/(start + START_ADDITIVE);

	/* Looks for matched query words in the snippet */
	for (int w=0; w<snippet_len; w++) {
		strcpy(found[w], "");
		strcpy(found[w+1], "");
		if (!memcmp(s[w], bbold, strlen(bbold))) {
			word_t pre_token;
			strcpy(pre_token, s[w]);
			char *token = strtok(pre_token, bbold);
			/* Scores differently if the word is repeated */
			if (word_found(token, found, w))	{
				score += REPEAT_WORD_SCORE;
			} else {
				strcpy(found[w], token);
				score += strlen(token) * NEW_WORD_MULTIPLIER;
			}
		}
	}
	/* add score if the snippet appears after a punctuation mark */
	if (before_punct) {
		score += BEFORE_PUNCT_SCORE;
	}
	/* add score if it ends in punctuation */
	if (has_punct(s[snippet_len-1])) {
		score += END_PUNCT_SCORE;
	}
	/* subtract score for being longer than required */
	if (snippet_len > MIN_SNIPPET_LEN) {
		score -= ((snippet_len - MIN_SNIPPET_LEN) * LENGTH_MULTIPLIER);
	}
	return score;
}

/****************************************************************/

/* checks to see if a word has previously been found within a snippet so it
can be scored accordingly
*/
int
word_found(word_t w, word_t found[], int i) {
	for (int j=0; j<i; j++) {
		if (*found[j] && !strcasecmp(found[j], w)) {
			return 1;
		}
	}
	return 0;
}

/****************************************************************/

/* Generates every possible snippet from a given paragraph and scores it. It
keeps the snippet with the best score. It handles things slightly differently
if thr paragraph is less than the min snippet length since the only possible
snippet is the paragraph itself and therefore the best by default.
*/
double
find_best_snippet(paragraph_t p, snippet_t s,
			int para_size, int *start, int *len) {
	double best_score = 0;
	int best_start = 0;
	int best_len = 0;
	snippet_t best_snippet;

	/* If the snippet is shorter than the required length then the only
	possible snippet is the paragraph itself, therefore it is the best */
	if (para_size < MIN_SNIPPET_LEN) {
		for (int i=0; i<para_size; i++) {
			strcpy(s[i],p[i]);
		}
		strcpy(s[para_size],"");
		best_score = score_snippet(s, 0, 1, para_size);
		best_start = 0;
		best_len = para_size;
		*start = best_start;
		*len = best_len;
		return best_score;
	}

	/* For longer snippets, go through all starting positions */
	for (int i=0; i<=para_size-MIN_SNIPPET_LEN; i++) {
		/* Now through all sizes for this starting pos */
		int max_snippet_len = MAX_SNIPPET_LEN;
		if (i > para_size - MAX_SNIPPET_LEN) {
			max_snippet_len = para_size - i;
		}
		for (int j=MIN_SNIPPET_LEN; j<=max_snippet_len; j++) {
			int before_punct = 0;
			double score;
			/* Create snippet with this start and length */
			for (int k=0; k<j; k++) {
				strcpy(s[k],p[i+k]);
			}
			strcpy(s[j], "");

			/* figure out if the snippet comes after punctuation */
			if (i==0 || has_punct(p[i-1])) {
				before_punct = 1;
			}
			score = score_snippet(s, i, before_punct, j);
			/* Keep only the best score and save its corresponding
			snippet
			*/
			if (score>best_score) {
				best_score = score;
				best_start = i;
				best_len = j;
				for (int w = 0; w<j; w++) {
					strcpy(best_snippet[w], s[w]);
				}
				strcpy(best_snippet[j], "");
			}
		}
	}

	/* copy the snippet and its info into the memory that can be accessed
	by the main function
	*/
	for (int w=0; w<MAX_SNIPPET_LEN; w++) {
		strcpy(s[w], best_snippet[w]);
	}
	strcpy(s[MAX_SNIPPET_LEN], "");
	*start = best_start;
	*len = best_len;

	return best_score;
}

/****************************************************************/

/* Figure out if the given word has punctuation in it. returns boolean */
int
has_punct(word_t w) {
	char punct[] = TERM_PUNCT;
	int len = strlen(w);
	if (strchr(punct, w[len-1])) {
		return 1;
	}
	return 0;
}

/****************************************************************/

/* Prints a given snippet and formats it correctly based on stage 3
instructions by correctly appending dots to the end if required
*/
int
print_snippet(snippet_t s) {
	int w=0;
	int bytes_on_line = 0;
	char needs_dots[] = NEEDS_DOTS;
	char ddots[] = DDOTS;
	while (strcmp(s[w], "")) {
		/* make sure the line isn't too long */
		if (bytes_on_line + strlen(s[w]) > MAX_OUTPUT_LINE) {
			printf("\n");
			bytes_on_line = 0;
		} else if (bytes_on_line) {
			printf(" ");
		}
		/* if at the end, decide if dots are needed and print those,
		print just the regular word otherwise
		*/
		if (!strcmp(s[w+1], "")) {
			printf("%s",s[w]);
			if (strchr(needs_dots,s[w][strlen(s[w])-1])
			|| !has_punct(s[w])) {
				printf("%s",ddots);
			}
		} else {
			printf("%s",s[w]);
		}
		bytes_on_line += strlen(s[w]) + 1;
		w++;
	}
	printf("\n\n");
	return 0;
}

/* Algorithms are fun! */
