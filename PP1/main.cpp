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
//typedef uint_fast16_t u16;
//typedef int_fast32_t s32;
//typedef uint_fast32_t u32;
//typedef int_fast64_t s64;
typedef uint_fast64_t u64;  //fast, at least 64bits; for time -PR-


//u8 srand(u64);
//u8 nrand(void);
//u8 rand(void);


#define WHEIGHT 23
#define WWIDTH 80
#define SIZE 18 //wysokość okna planszy -PR-
#define MAX_SIZE ((u0) -1) // -PR-
#define WIDTH 68
#define MARGIN (WWIDTH-WIDTH)/2


typedef struct {
	u64 seed;
	char frog = '@';
	char bocian = '%';
	u8 ups = 10; // updates / s
	u8 fspeed = 2; // updates / frog speed
	u8 bspeed = 10; // updates / bocian speed
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
	
} game_t;




u8 bocian(game_t);
u8 update(game_t);
void key(game_t * game, int ch);



// setup functions
void setcolors(void) {
	curs_set(0);
	start_color();
	init_pair(1, COLOR_RED, COLOR_WHITE);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_WHITE, COLOR_BLACK);
	init_pair(4, COLOR_BLACK, COLOR_BLUE);
}

void setmap(game_t * game) {
	for (u0 y = 0; y < (SIZE/2)+1; y++) {
		for (u0 x = 0; x < MAX_SIZE; x++) {
			game -> road[y][x] = gamewater;
		}
	}
	for (u0 y = (SIZE/2)+1; y < MAX_SIZE; y++) {
		u8 V = (rand() % 3 == 0) ? gamegrass : gameroad;
		for (u0 x = 0; x < MAX_SIZE; x++) {
			game -> road[y][x] = V;
		}
	}
}

game_t setup(void) {
    initscr();

    if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
    }
    if (resizeterm(WHEIGHT, WWIDTH) != 0) {
    endwin();
    printf("Your terminal does not support resizing\n");
    exit(1);
    }

    setcolors();

	//noecho();
	flushinp();
	nodelay(stdscr, TRUE);
    cbreak();
    keypad(stdscr, TRUE);


    game_t game;

    setmap(&game);

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


void printline(u8 A[WIDTH], u8 c[gametypes]) {
	for (u0 x = {}; x < WIDTH; x += 1){
		u8 a = A[x];
		if (a == gameroad) {
			attron(COLOR_PAIR(3));
		} else if (a == gamewater) {
			attron(COLOR_PAIR(4));
		} else {
			attron(COLOR_PAIR(2));
		}
		addch(c[a]);
	}
}

u8 printroad(game_t * game, const config_t config){
	static u8 c[gametypes] = {};              // do konfig
	c[gameroad] = '-';
	c[gamegrass] = '.';
	c[gamewater] = '=';
	c[gametrash] = '%';
	
	if (game->last_pos_y < game->y) {
		game->last_pos_y += 1;
	}
	
	for (int y = 0; y < SIZE; y++) {
		move(WHEIGHT -y -1, MARGIN);
		printline(game->road[game->last_pos_y + y], c);
	}
	//if (Y >= fy) return 2; //lose
	//if (fy >= SIZE+14) return 3; //win
	return 0;//printcar(road, MY, x, y);
}



// main function
int main() {

	s8 status = 0;

    config_t config;
    game_t game = setup();
    
    srand(config.seed);


/*  Loop until user hits 'q' to quit  */

	int      ch = 0;
	while ( (ch = getch()) != 'q' and status < 2) { // {-1, 0, 1} car is moving a frog

	if (game.frogtime >= config.fspeed) {
		key(&game, ch);
		game.frogtime = 0;
	}	game.frogtime += 1;


		clear();

        status = printroad(&game, config);

		attron(COLOR_PAIR(1));
		move(0, 0);
        printint(game.x);

		attron(COLOR_PAIR(2));
		move(20, 0);
        printint(game.y);

		//attron(COLOR_PAIR(4));
		//move(21, 0);
        //printint(bociany);

		//attron(COLOR_PAIR(1));
		//move(20, MARGIN+x);
		//addch(config.bocian);

		attron(COLOR_PAIR(2));
		move(((WHEIGHT-(SIZE)/2))+game.last_pos_y-game.y -1, MARGIN+game.x);
		addch(config.frog);

		//t += 1;
    	refresh();
		flushinp();
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
u0 bocian(game_t A) {
	static u8 countdown = 0;
	countdown += 1;
	if (countdown > 10) return countdown = 1; //10
	return 0;
}


u0 update(game_t game) {
	static u64 gametime = 0;
	gametime += 1;
	return 0;
}



















void key(game_t * game, int ch) {
	switch ( ch ) {

		case KEY_DOWN:
			if (game->y > 0 and game->y+(SIZE/2) > game->last_pos_y) {
				game->y--;
			}
			break;

		case KEY_UP:
			if (game->y < MAX_SIZE) {
				game->y++;
			}
			break;

		case KEY_LEFT:
			if ( game->x > 0 )
			--game->x;
			break;

		case KEY_RIGHT:
			if ( game->x < (WIDTH-1) )
			++game->x;
			break;

		case KEY_HOME:
			game->x = 0;
			game->y = 0;
			break;

		//case KEY_END:
		//    game.x = (cols - width);
		//    game.y = (rows - height);
		//    break;
	}
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
