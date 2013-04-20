/*
 * Driver for the SUDOKU game system.
 */
#include <stdlib.h>
#include "arguments.h"
#include "bool.h"
#include "puzzle.h"

/*
 * Maximum number of characters we consider on a command line.
 * Indices of command argument characters on the command line.
 */
#define MAX_CHARS       (8)
#define CMD_INDEX		(0)
#define ROW_INDEX		(2)
#define COL_INDEX		(4)
#define DIGIT_INDEX 	(6)

/*
 * Local files to read a line of text and print error
 * messages based on the status from the board functions.
 */
static bool read_line(char buf[], int max) ;
static void print_error(op_result status) ;

int main(int ac, char **av) {
	char cmd_line[MAX_CHARS+1] ;
	op_result status ;

	/*
	 * Parse the command arguments.
	 * If this returns, initialize the puzzle.
	 * Then
	 *   - configure the board from the command
	 *     line puzzle file,
	 *   - close the file,
	 *   - print the initial board
	 */
	parse_args(ac, av) ;
	init_puzzle() ;
	configure( puzzle_file() ) ;
	fclose( puzzle_file() ) ;
	print_puzzle() ;

	/*
	 * Command loop.
	 * Read a line and use the first character to decide
	 * what command to execute (or report an error).
	 */
	printf("command: ");

	while(read_line(cmd_line, MAX_CHARS) != EOF ) {
		if(cmd_line[CMD_INDEX] == 'q') {		// quit
			break ;
		} else if (cmd_line[CMD_INDEX] == 'p') {	// print the board
			print_puzzle() ;
		} else if (cmd_line[CMD_INDEX] == 'a') {	// add a digit
			int r, c, d ;
			r = cmd_line[ROW_INDEX] - '0' ;
			c = cmd_line[COL_INDEX] - '0' ;
			d = cmd_line[DIGIT_INDEX] - '0' ;

			status = add_digit(r, c, d) ;
			if( status != OP_OK ) {
				print_error(status) ;
			}
		} else if (cmd_line[CMD_INDEX] == 'e') {	// erase a digit
			int r, c ;
			r = cmd_line[ROW_INDEX] - '0' ;
			c = cmd_line[COL_INDEX] - '0' ;

			status = erase_digit(r, c) ;
			if( status != OP_OK ) {
				print_error(status) ;
			}
		} else {				// error
			printf("Unknown command %s\n", cmd_line) ;
		}
	
	printf("command: ") ;
	}

	return 0 ;
}

/*
 * Read a line of text from standard input, possibly echoing it.
 * Up to the 1st 'max' characters are transfereed to the 'buf',
 * and properly NUL terminated.
 * Extra characters at the end of the line are simply ignored.
 */
static bool read_line(char buf[], int max) {
	int ch ;
	int i = 0 ;

	for(ch = getchar(); ch != EOF && ch != '\n'; ch = getchar()) {
		if(echo_input()) {
			putchar(ch);
		}
		if(i < max) {
			buf[i++] = ch;
		}
	}
	if(echo_input() && ch == '\n') {
		putchar(ch) ;
	}
	
	buf[i] = '\0'; /* NULL terminating the string */
}

/*
 * Print a message corresponding to the error 'status'
 */
static void print_error(op_result status) {
	switch(status) {
	case OP_BADARGS:
		printf("Bad row index, column index, or digit.\n");
		break;
	case OP_OCCUPIED:
		printf("Selected board space is already occupied.\n");
		break;
	case OP_ILLEGAL:
		printf("Digit placement violates Sudoku rules.\n");
		break;
	case OP_EMPTY:
		printf("Selected board space is already empty.\n");
		break;
	case OP_FIXED:
		printf("Cannot erase an initialization square.\n");
		break;
	}
}
