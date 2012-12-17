/*
 * Implementation of the The Sudoku puzzle module.
 *
 * In keeping with standard Sudoku nomenclature, we use
 * 1-based indexing. Thus row indices, column indices,
 * and digits being placed must all be in the range 1 .. 9.
 */

#include <stdlib.h>
#include <stdio.h>

#include "puzzle.h"
#include "bool.h"

/*
 * The tables are 10x10 so that we can index from 1 .. 9 (the 0th row
 * and column are ignored). This wastes a tad of space, but is much
 * less likely to cause "off by 1" indexing errors.
 *
 * puzzle[i][j] is the number (0 .. 9) at row i, column j.
 *   0 represents a free (blank) puzzle location.
 *   Anything else represents the value at that location, and
 *     must be consistent with the rules of Sudoku.
 *
 * fixed[i][j] is true <=> row i, column j is fixed.
 *   That is, the value was part of the initial puzzle layout and
 *   cannot be erased.
 */

static short puzzle[10][10] ;
static bool  fixed[10][10] ;

/*
 * Functions to determine whether a row, a column, or the "region"
 * containing a row and column, contains the specified digit.
 */
static bool row_contains(int row, int digit) ;
static bool col_contains(int col, int digit) ;
static bool region_contains(int row, int col, int digit) ;

/*
 * Function to determine whether an integer - not the character for
 * a digit - is in the range 0 .. 9.
 */
static bool in_range(int value) ;

/*
 * Print support functions.
 *   print a line of dashes.
 *   print a row of values (with blanks for 0's).
 */
static void print_dashes() ;
static void print_row(int row) ;

/*
 * Helper function to search puzzle
 */
static bool  region_search(int region, int digit);

/*
 * Initialize the puzzle so that (a) all values are zero (free)
 * and (b) none of the values are 'fixed' (uneraseable).
 */
extern void init_puzzle()  {
	int i, j;
	for (i = 1; i < 10; i++)
		for (j = 1; j <10; j++)
			puzzle[i][j] = 0;
			fixed[i][j] = FALSE;
}

/*
 * Read in the initial puzzle configuration.
 * Each line is 4 characters long:
 *   Row    as a character '0' .. '9'
 *   Column as character '0' .. '9'
 *   Digit  as character '0' .. '9'
 *   Terminating newline.
 * Exits with an error message if there are syntactic
 * or semantic errors with any configuration line.
 */
void configure(FILE *puzzle_file) {
	int row, col, digit;
	int line_num, i = 0; 
	char line_in[5];   // String that serves as an array of chars being input

	while (fgets(line_in, 5, puzzle_file) != NULL)
	{
		line_num++;
		
		row = line_in[0] - 48;
		col = line_in[1] - '0';
		digit = line_in[2] - '0';
		
		if (line_in[3] != '\n') // Checks to see if there is more than 3 digits
		{
			printf("Illegal format in configuration file at line %d", line_num); 
			exit(1);
		}
		else if (!in_range(row) || !in_range(col) || !in_range(digit)) // Checks to see if all vales are 1..9
		{
			
			printf("Illegal format in configuration file at line %d", line_num); 
			exit(1);
		}
		else if (puzzle[row][col] != 0) // Checks to see if the cell has a value in it
		{
			printf("Illegal placement in configuration file at line %d", line_num); 
			exit(1);
		}
		else if (row_contains(row, digit) || col_contains(col, digit)) // Checks to see if the row, or the col contains the value
		{
			printf("Illegal placement in configuration file at line %d", line_num); 
			exit(1);
		}
		else if (region_contains(row, col, digit)) // Checks to see if the region contains the value
		{
			printf("Illegal placement in configuration file at line %d", line_num);
			exit(1);
		}
		else
		{
			puzzle[row][col] = digit;
			fixed[row][col] = TRUE;		
		}
	}
}

/*
 * Print the puzzle to standard output using the
 * specified output format.
 * 	- Print an initial row of dashes.
 * 	- Print each row.
 * 	- Print a row of dashes after each 3rd row.
 */
void print_puzzle() {
	int i;
	print_dashes();	
	for (i = 1; i < 10; i++)
	{
		print_row(i);
		if (i % 3 == 0)
		{
			print_dashes();
		}
	}
	
}

/*
 * (Attempt to) add the specified digit at the given row and
 * column location on the puzzle.
 * The digit, row, and column must all be integers in
 * the range 1 .. 9.
 *
 * Return value is the status of the attempt (OP_OK is the only
* status where the puzzle is actually changed).
 */
op_result add_digit(int row, int col, int digit) {
	if (!in_range(row) || !in_range(col) || !in_range(digit))
	{
		return OP_BADARGS;
	}
	else if (puzzle[row][col] != 0)
	{
		return OP_OCCUPIED;
	}
	else if (row_contains(row, digit) || col_contains(col, digit))
	{
		return OP_ILLEGAL;
	}
	else if (region_contains(row, col, digit))
	{
		return OP_ILLEGAL;
	}
	else
	{
		puzzle[row][col] = digit;	
		return OP_OK ;
	}
}

/*
 * (Attempt to) delete the digit at the given row and
 * column location on the puzzle the row and column
 * must both be integers in the range 1 .. 9
 *
 * Return value is the status of the attempt (OP_OK is the only
 * status where the puzzle actually changed).
 */
op_result erase_digit(int row, int col) {
	if (!in_range(row) || !in_range(col))
	{
		return OP_BADARGS;
	}
	else if (puzzle[row][col] == 0)
	{
		return OP_EMPTY;
	}
	else if (fixed[row][col] == TRUE)
	{
		return OP_FIXED;
	}
	else 
	{
		puzzle[row][col] = 0;
		return OP_OK ;
	}
}

/*
 * Returns TRUE iff the given 'row' has the given 'digit' in it.
 */
static bool row_contains(int row, int digit) {
	int i;
	for (i = 1; i < 10; i++)
	{
		if (puzzle[row][i] == digit)
			return TRUE;
		else
			return FALSE;
	}
}

/*
 * Returns TRUE iff the given 'col' has the given 'digit' in it.
 */
static bool col_contains(int col, int digit) {
	int i;
	for (i = 1; i < 10; i++)
		if (puzzle[i][col] == digit)
			return TRUE;
		else
			return FALSE;
}

/*
 * Returns TRUE iff the region containing the cell at 'row' & 'col'
 * has the given digit in it.
 * NOTE: Finding the first row and column for the 'row'
 *       and 'col' is simple if you (a) remember that integer
 *       division discards the remainder and (b) translating
 *       the indices to 0 rather than 1 base makes things easier.
 * NOTE: A region is a 3 x 3 square.
 */
static bool region_contains(int row, int col, int digit) {
	
	if (row >= 1 && row <= 3 && (col >= 1 && col <= 3)) // Region 1
	{
		region_search(1, digit);	
	}
	else if ((row >= 1 && row <= 3) && (col >= 4 && col <= 6)) // Region 2
	{
		region_search(2, digit);
	}
	else if ((row >= 1 && row <= 3) && (col >= 7 && col <= 9)) // Region 3
	{
		region_search(3, digit);
	}
	else if ((row >= 4 && row <= 6) && (col >= 1 && col <= 3)) // Region 4
	{
		region_search(4, digit);
	}
	else if ((row >= 4 && row <= 6) && (col >= 4 && col <= 6)) // Region 5
	{
		region_search(5, digit);

	}
	else if ((row >= 4 && row <= 6) && (col >= 7 && col <= 9)) // Region 6
	{
		region_search(6, digit);
	}	
	else if ((row >= 7 && row <= 9) && (col >= 1 && col <= 3)) // Region 7
	{
		region_search(7, digit);
	}
	else if ((row >= 7 && row <= 9) && (col >= 4 && col <= 6)) // Region 8
	{
		region_search(8, digit);
	}
	else if ((row >= 7 && row <= 9) && (col >= 7 && col >= 9)) // Region 9
	{
		region_search(9, digit);
	}
	else
	{
		return FALSE;
	}
}

static bool region_search(int region, int digit){
	int i,j;
	if (region == 1)
		for (i = 1; i <= 3; i++)	
			for (j = 1; j <= 3; j++)
				if (puzzle[i][j] == digit)
					return TRUE;

		if (region == 2)
		for (i = 1; i <= 3; i++)	
			for (j = 4; j <= 6; j++)
				if (puzzle[i][j] == digit)
					return TRUE;


		if (region == 3)
		for (i = 1; i <= 3; i++)	
			for (j = 7; j <= 9; j++)
				if (puzzle[i][j] == digit)
					return TRUE;

	if (region == 4)
		for (i = 4; i <= 6; i++)	
			for (j = 1; j <= 3; j++)
				if (puzzle[i][j] == digit)
					return TRUE;

	if (region == 5)
		for (i = 4; i <= 6; i++)	
			for (j = 4; j <= 6; j++)
				if (puzzle[i][j] == digit)
					return TRUE;


	if (region == 6)
		for (i = 4; i <= 6; i++)	
			for (j = 7; j <= 9; j++)
				if (puzzle[i][j] == digit)
					return TRUE;

	if (region == 7) 
		for (i = 7; i <= 9; i++)	
			for (j = 1; j <= 3; j++)
				if (puzzle[i][j] == digit)
					return TRUE;

	if (region == 8)
		for (i = 7; i <= 9; i++)	
			for (j = 4; j <= 6; j++)
				if (puzzle[i][j] == digit)
					return TRUE;

	if (region == 9)
		for (i = 7; i <= 9; i++)	
			for (j = 7; j <= 9; j++)
				if (puzzle[i][j] == digit)
					return TRUE;
}

/*
 * Return TRUE iff the value is in the Sudoku range (1 .. 9)
 */
static bool in_range(int value) {
	if (value >= 1 && value <= 9)
		return TRUE;
	else
		return FALSE;
}

/*
 * Print a row of 25 dashes.
 * This will line up correctly with the rows with data.
 */
static void print_dashes() {
	printf("-------------------------\n");
}

/*
 *   Perint a row from the puzzle.
 *   Print an initial bar ('|').
 *   For each value in the row, print a space and either:
 *     - a space if the value is 0.
 *     - the CHARACTER code for the digit if non-zero.
 *   After the 3rd, 6th and 9th columns, print " |"
 */
static void print_row(int row) {
	int i;
	printf("|");
	for (i = 1; i < 10; i++)
	{
			printf(" ");
		if (puzzle[row][i] == 0)
			printf(" ");
		else
			printf("%d", puzzle[row][i]);

		if (i % 3 == 0)
			printf(" |");

	}
		printf("\n");
}
