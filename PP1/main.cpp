#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
//#include <time.h>
#include <unistd.h>


#define SIZE 40
#define WIDTH 68
#define MARGIN 6
void setup(void);
int bocian(void);


typedef struct {
	char frog = '@';
	char bocian = '%';
	char ups = 20; // updates / s
} config_t;


void setcolors() {
	curs_set(0);
	start_color();
	init_pair(1, COLOR_RED, COLOR_WHITE);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_WHITE, COLOR_BLACK);
	init_pair(4, COLOR_BLACK, COLOR_BLUE);

}

void printint(int a) {
	int t;
	if (a > 0){
		t = a % 10;
		a /= 10;
		printint(a);
switch (t) {

case 0:
	addch('0');
	break;
case 1:
	addch('1');
	break;
case 2:
	addch('2');
	break;
case 3:
	addch('3');
	break;
case 4:
	addch('4');
	break;
case 5:
	addch('5');
	break;
case 6:
	addch('6');
	break;
case 7:
	addch('7');
	break;
case 8:
	addch('8');
	break;
case 9:
	addch('9');
	break;
}
	}
}



void printline(char a, int size) {
	const char g = '.';
	const char r = '-';
	const char c = '=';
	const char *p;
	if (a == 1) {
		attron(COLOR_PAIR(3));
		p = &r;
	} else if (a == 2) {
		attron(COLOR_PAIR(4));
		p = &c;
	} else {
		attron(COLOR_PAIR(2));
		p = &g;
	}
	for (int i = 0; i < size; i++) {
		addch(*p);
	}
}



void printcar(char a, int size) {
	const char g = '.';
	const char r = '-';
	const char c = '=';
	const char *p;
	if (a == 1) {
		attron(COLOR_PAIR(3));
		p = &r;
	} else if (a == 2) {
		attron(COLOR_PAIR(4));
		p = &c;
	} else {
		attron(COLOR_PAIR(2));
		p = &g;
	}
	for (int i = 0; i < size; i++) {
		addch(*p);
	}
}

int printroad(int Y, int MY, int fx, int fy){ // Y and MaxY
	const int size = 15;
	static int last_pos_y = 0;
	static char road[size] = {0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1};
	char t = road[0];
	
	if (last_pos_y < Y) {
		for (int i = 0; i < size-1; i++)
			road[i] = road[i+1];
		if (MY > Y)
			road[size-1] = t;
		else
			road[size-1] = 2;
		last_pos_y += 1;
	}
	
	for (int y = 0; y < size; y++) {
		move(20-y, 6);
		printline(road[y], WIDTH);
	}
	if (Y >= fy) return 2; //lose
	if (fy >= SIZE+14) return 3; //win
	return 0;//printcar(road, MY, x, y);
}



int main() {

	int      width = 23, height = 7, rows = 25, cols = 80;
    int      x = WIDTH/2, y = 3, bociany = 0, t = 0, status = 0;
    config_t config;
    
    setup();

    /*  Loop until user hits 'q' to quit  */

    int      ch = 0;
    while ( (ch = getch()) != 'q' and status < 2) { // {-1, 0, 1} car is moving a frog

	switch ( ch ) {

	case KEY_DOWN:
	    if (y > bociany+1) {
			--y;}
	    break;

	case KEY_UP:
	    if (y < bociany+14) {
			++y;
		}
	    break;

	case KEY_LEFT:
	    if ( x > 0 )
		--x;
	    break;

	case KEY_RIGHT:
	    if ( x < (WIDTH-1) )
		++x;
	    break;

	case KEY_HOME:
	    x = 0;
	    y = 0;
	    break;

	case KEY_END:
	    x = (cols - width);
	    y = (rows - height);
	    break;
	}
		clear();

        status = printroad(bociany, SIZE, x, y);

		attron(COLOR_PAIR(1));
		move(0, 0);
        printint(t);

		attron(COLOR_PAIR(2));
		move(20, 0);
        printint(y);

		attron(COLOR_PAIR(4));
		move(21, 0);
        printint(bociany);

		attron(COLOR_PAIR(1));
		move(20, MARGIN+x);
		addch(config.bocian);

		attron(COLOR_PAIR(2));
		move(20-y+bociany, MARGIN+x);
		addch(config.frog);

		t += 1;
    	refresh();
		flushinp();
    	usleep(1000000/config.ups);

		bociany += bocian();
    }


    /*  Clean up after ourselves  */

    delwin(stdscr);
    endwin();
    refresh();
 
    return EXIT_SUCCESS;
}

int bocian() {
	static char countdown = 0;
	countdown += 1;
	if (countdown > 10) return countdown = 1; //10
	return 0;
}

void setup(void) {
    initscr();

    if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
    }
    setcolors();

	//noecho();
	flushinp();
	nodelay(stdscr, TRUE);
    cbreak();
    keypad(stdscr, TRUE);
}
