#include <stdlib.h>
//#include <iostream>
//#include <stdio.h>
#include <curses.h>
//#include <time.h>
#include <unistd.h>

typedef int_fast8_t s8;
typedef uint_fast8_t u8;
typedef int_fast8_t s0;
typedef uint_fast8_t u0;
//typedef int_fast16_t s16;
typedef uint_fast16_t u16;
//typedef int_fast32_t s32;
//typedef uint_fast32_t u32;
//typedef int_fast64_t s64;
typedef uint_fast64_t u64;  //fast, at least 64bits; for time -PR-


#define WHEIGHT 23
#define WWIDTH 80
#define SIZE 18 //wysokość okna planszy -PR-
#define MAX_SIZE ((u0) -1) // -PR-
#define WIDTH 68
#define MARGIN (WWIDTH-WIDTH)/2



#define frogcolropucha 6 //ropucha
#define frogcolrechotka 2 //rzechotka drzewna
#define frogcolkumak 7 // kumak nizinny

typedef struct {
	u64 seed;
	char frog = '@';
	char bocian = '%';
	u8 ups = 20; // updates / s
	u8 fspeed = 2; // updates / frog speed
	u8 bspeed = 10; // updates / bocian speed
	s8 health = 20;
	u0 length = 100;
	u8 frogcol = frogcolrechotka;
} config_t;

typedef struct {
	s0 x;
	s0 y;
	s0 coutdown;
	s0 maxcoutdown;
} car_t;


#define gameroad 0
#define gamegrass 1
#define gametrash 2
#define gamewater 3
#define gametypes 4

typedef struct {
	u0 y = 0;
	u0 x = WIDTH/2;
	u0 last_pos_y = 0;
	u8 frogtime = 0;
	// „game” + „0-null 1-road 2-grass 3-water”
	u8 road[MAX_SIZE][WIDTH] = {};
	u16 score;
	s8 health;
	
} game_t;




u8 bocian(game_t);
u8 update(game_t);
void key(game_t & game, int ch);



// setup functions
void setcolors(void) {
	curs_set(0);
	start_color();
	init_pair(1, COLOR_RED, COLOR_WHITE);
	init_pair(2, COLOR_BLACK, COLOR_GREEN); // rzekotka + trawa!!!
	init_pair(3, COLOR_WHITE, COLOR_BLACK);
	init_pair(4, COLOR_BLACK, COLOR_BLUE);
	init_pair(5, COLOR_GREEN, COLOR_RED);
	init_pair(6, COLOR_GREEN, COLOR_YELLOW); // ropucha
	init_pair(7, COLOR_RED, COLOR_GREEN); // kumak
}

void setmap(game_t & game, u0 L) {
	for (u0 y = 0; y < (SIZE/2)+1; y++) {
		for (u0 x = 0; x < WIDTH; x++) {
			game.road[y][x] = gamewater;
		}
	}
	for (u0 y = (SIZE/2)+1; y < L+(SIZE/2)+1; y++) {
		u8 V = (rand() % 3 == 0) ? gamegrass : gameroad;
		for (u0 x = 0; x < WIDTH; x++) {
			game.road[y][x] = V;
		}
		if (V == gamegrass) {
			game.road[y][rand()%WIDTH] = gametrash;
			game.road[y][rand()%WIDTH] = gametrash;
			game.road[y][rand()%WIDTH] = gametrash;
		}
	}
	for (u0 y = L+(SIZE/2)+1; y < MAX_SIZE; y++) {
		for (u0 x = 0; x < WIDTH; x++) {
			game.road[y][x] = gamewater;
		}
	}
}

void setup(game_t & game, config_t & config) {
    initscr();

    if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1); //ERR_CODE_1
    }
    if (resizeterm(WHEIGHT, WWIDTH) != 0) {
    endwin();
    printf("Your terminal does not support resizing\n");
    exit(2); //ERR_CODE_2
    }

    setcolors();

	//noecho();
	//flushinp();
	nodelay(stdscr, TRUE);
    cbreak();
    keypad(stdscr, TRUE);

	config.length = (config.length > MAX_SIZE - SIZE) ? MAX_SIZE - SIZE : config.length;
    setmap(game, config.length);
    game.score = 0;
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


void printline(u8 A[WIDTH], u8 c[gametypes]) {
	for (u0 x = {}; x < WIDTH; x += 1){
		u8 a = A[x];
		if (a == gameroad) {
			attron(COLOR_PAIR(3));
		} else if (a == gamewater) {
			attron(COLOR_PAIR(4));
		} else if (a == gametrash) {
			attron(COLOR_PAIR(5));
		} else {
			attron(COLOR_PAIR(2));
		}
		addch(c[a]);
	}
}

u8 printroad(game_t & game, const config_t & config){
	static u8 c[gametypes] = {};              // do konfig
	c[gameroad] = '-';
	c[gamegrass] = '.';
	c[gamewater] = '=';
	c[gametrash] = '%';
	
	if (game.last_pos_y < game.y) {
		game.last_pos_y += 1;
	}
	
	for (int y = 0; y < SIZE; y++) {
		move(WHEIGHT -y -1, MARGIN);
		printline(game.road[game.last_pos_y + y], c);
	}
	//if (Y >= fy) return 2; //lose
	//if (fy >= SIZE+14) return 3; //win
	return 0;//printcar(road, MY, x, y);
}



// main function
int main() {

	s8 status = 0;

    config_t config;
    game_t game;
    setup(game, config);
    game.health = config.health;
    
    srand(config.seed);


/*  Loop until user hits 'q' to quit  */

	int      ch = 0;
	while ( ch != 'q' and game.health > 0 and game.y < config.length) {//and status < 2) { // {-1, 0, 1} car is moving a frog

	if (game.frogtime >= config.fspeed) {
		ch = getch();
		key(game, ch);
		game.frogtime = 0;
	}	game.frogtime += 1;


		clear();

        status = printroad(game, config);

		attron(COLOR_PAIR(1));
		move(0, 0);
        printint(config.frogcol);

		attron(COLOR_PAIR(2));
		move(2, 0);
        printint(game.y);

		attron(COLOR_PAIR(3));
		move(4, 0);
        printint(game.score);

		attron(COLOR_PAIR(4));
		move(7, 0);
        printint(game.health);

		//attron(COLOR_PAIR(4));
		//move(21, 0);
        //printint(bociany);

		//attron(COLOR_PAIR(1));
		//move(20, MARGIN+x);
		//addch(config.bocian);

		attron(COLOR_PAIR(config.frogcol));
		move(((WHEIGHT-(SIZE)/2))+game.last_pos_y-game.y -1, MARGIN+game.x);
		addch(config.frog);

		//t += 1;
    	refresh();
    	usleep(1e6/config.ups);

		//bociany += bocian();
    }


    /*  Clean up after ourselves  */

    delwin(stdscr);
    endwin();
    refresh();
 
    return EXIT_SUCCESS;
}




// update functions
u0 bocian(game_t & A) {
	static u8 countdown = 0;
	countdown += 1;
	if (countdown > 10) return countdown = 1; //10
	return 0;
}


u0 update(game_t & game) {
	static u64 gametime = 0;
	gametime += 1;
	return 0;
}


void key(game_t & game, int ch) {
	switch ( ch ) {

		case KEY_DOWN:
			if (game.y > 0 and game.y+(SIZE/2) > game.last_pos_y) {
				if (game.road[SIZE/2 + game.y-1][game.x] == gametrash)
					game.health -= 5;
				else
					game.y--;
			}
			break;

		case KEY_UP:
			if (game.y < MAX_SIZE) {
				if (game.road[SIZE/2 + game.y+1][game.x] == gametrash)
					game.health -= 5;
				else
					game.y++;
			}
			break;

		case KEY_LEFT:
			if ( game.x > 0) {
				if (game.road[SIZE/2 + game.y][game.x-1] == gametrash)
					game.health -= 5;
				else
					game.x--;
			}
			break;

		case KEY_RIGHT:
			if ( game.x < (WIDTH-1)) {
				if (game.road[SIZE/2 + game.y][game.x+1] == gametrash)
					game.health -= 5;
				else
					game.x++;
			}
			break;

		//case KEY_HOME:
		//	game.x = 0;
		//	game.y = 0;
		//	break;

		//case KEY_END:
		//    game.x = (cols - width);
		//    game.y = (rows - height);
		//    break;
	}
	flushinp(); // clear input bufer -PR-
}


















/*
u8 srand(u64 R) {
	static u64 seed;
	seed = seed *7 + R; // and it is cut to 64 bit -PR-
	return seed; // cut to 8 bits
	
}
u8 nrand(void) {
	
}
u8 rand(void);
*/
