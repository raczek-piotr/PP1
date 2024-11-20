#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
//#include <time.h>
#include <unistd.h>

typedef int_fast8_t s8;
typedef uint_fast8_t u8;
typedef int_fast8_t sq;
typedef uint_fast8_t uq;
//typedef int_fast16_t s16;
//typedef uint_fast16_t u16;
//typedef int_fast32_t s32;
//typedef uint_fast32_t u32;
//typedef int_fast64_t s64;
//typedef uint_fast64_t u64;


#define SIZE 25
#define MAX_SIZE (uq) -1
#define WIDTH 68
#define MARGIN (80-WIDTH)/2


typedef struct {
	uq frog = '@';
	uq bocian = '%';
	uq ups = 20; // updates / s
} config_t;

typedef struct {
	sq x;
	sq y;
	sq coutdown;
	sq maxcoutdown;
} car_t;


#define gamenull 0
#define gameroad 1
#define gamegrass 2
#define gamewater 3

typedef struct {
	uq y = 0;
	uq x = WIDTH/2;
	// „game” + „0-null 1-road 2-grass 3-water”
	uq road[MAX_SIZE] = {};
	
} game_t;


pos_t bocian(const pos_t);



// setup functions
void setcolors(void) {
	curs_set(0);
	start_color();
	init_pair(1, COLOR_RED, COLOR_WHITE);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_WHITE, COLOR_BLACK);
	init_pair(4, COLOR_BLACK, COLOR_BLUE);
}

game_t setup(void) {
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

    game_t game;
    return game;
}



// printing functions
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

void printroad(game_t game){ // Y and MaxY
	const int size = 15;
	static int last_pos_y = 0;
	static char road[SIZE] = {}; //default 0
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
	//return 0;//printcar(road, MY, x, y);
}




// main function
int main() {

	int      width = 23, height = 7, rows = 25, cols = 80;
    int      x = WIDTH/2, y = 3, bociany = 0, t = 0, status = 0;
    config_t config;
    
    game_t g = setup();

    /*  Loop until user hits 'q' to quit  */

    int      ch = 0;
    while ( (ch = getch()) != 'q' and status < 2) { // {-1, 0, 1} car is moving a frog
    
    //key(game, ch);

	switch ( ch ) {

	case KEY_DOWN:
	    if (g.y > bociany+1) {
			--g.y;}
	    break;

	case KEY_UP:
	    if (g.y < bociany+14) {
			++g.y;
		}
	    break;

	case KEY_LEFT:
	    if ( g.x > 0 )
		--g.x;
	    break;

	case KEY_RIGHT:
	    if ( g.x < (WIDTH-1) )
		++g.x;
	    break;

	case KEY_HOME:
	    g.x = 0;
	    g.y = 0;
	    break;

	case KEY_END:
	    g.x = (cols - width);
	    g.y = (rows - height);
	    break;
	}
		clear();

        status = printroad(bociany, SIZE, g.x, g.y);

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

		//bociany += bocian();
    }


    /*  Clean up after ourselves  */

    delwin(stdscr);
    endwin();
    refresh();
 
    return EXIT_SUCCESS;
}




// update functions
void bocian(game_t A) {
	static char countdown = 0;
	countdown += 1;
	if (countdown > 10) return countdown = 1; //10
	return 0;
}


void update(game_t A) {
	static int gametime = 0;
	gametime += 1;
	return 0;
}




















