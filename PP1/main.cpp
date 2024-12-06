//#include <stdlib.h>
//#include <iostream>
//#include <stdio.h>
#include <curses.h>
//#include <time.h>
#include <unistd.h>
#include <fstream>

using namespace std;

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

#define carnormal 0
#define cartransport 1
#define carstop 2

#define gameroad 0
#define gamegrass 1
#define gametrash 2
#define gamewater 3
#define gametypes 4

struct config_t {
	u64 seed;
	char frog = '@';
	char bocian = '%';
	char car = '#';
	u8 ups = 20; // updates / s
	u8 fspeed = 4; // updates / frog speed
	u8 bspeed = 16; // updates / bocian speed
	s8 health = 20;
	u0 length = 100;
	u8 frogcol = frogcolrechotka;
	u8 tiles[gametypes] = {'-', '.', '=', '%'};
	u8 cartimemod = 3;
	u8 cartimedefault = 1;
	u64 timedown = 100;
	u8 darktime = 37;
};

struct car_t {
	s0 x;
	s0 y;
	u8 type = false;
	s0 coutdown;
	s0 maxcoutdown;
	s8 dir = 0;
};

struct game_t {
	u0 y = 0;
	u0 x = WIDTH/2;
	u0 by = 10;
	u0 bx = WIDTH/2;
	u0 last_pos_y = 0;
	u8 frogtime = 0;
	u8 bociantime = 0;
	// „game” + „0-null 1-road 2-grass 3-water”
	u8 road[MAX_SIZE][WIDTH] = {};
	u16 score;
	s8 health;
	car_t cars[SIZE] = {}; // NULL's
	u64 time = 0;
	u64 timedown = 0;
	u8 level = 0;
	u8 darktime = 0;
};



void printint(u64 a);
u8 bocian(game_t & game, config_t & config);
void key(game_t & game, config_t & config, int ch);



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
		u8 V = (rand() % 4 == 0) ? gamegrass : gameroad;
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

void setup() {
	initscr();

	if (has_colors() == FALSE) {
	endwin();
	printf("Your terminal does not support color\n");
	exit(1); //ERR_CODE_1
	}
	//if (resizeterm(WHEIGHT, WWIDTH) != 0) {
	//endwin();
	//printf("Your terminal does not support resizing\n");
	//exit(2); //ERR_CODE_2
	//}

	setcolors();

	noecho();
	//flushinp();
	nodelay(stdscr, TRUE);
	cbreak();
	keypad(stdscr, TRUE);
}

void load(game_t & game, config_t & config) {

	char T[10];
	u8 t, l = 0;
	int m;
	FILE * F;
	F = fopen("conf.txt", "r");

	if(F){
		fgets(T, 7, F);
		fgets(T, 2, F);
		config.frogcol = T[0] - '0';
		fgets(T, 7, F);
		fscanf(F, "%d", & m);
		config.length = m;
		fgets(T, 7, F);
		fscanf(F, "%d", & config.timedown);

		fclose(F);
	}

	config.length = (config.length > MAX_SIZE - SIZE) ? MAX_SIZE - SIZE : config.length;
	setmap(game, config.length);
	game.score = 0;

	for (u0 i = 0; i < SIZE; i++) {
		game.cars[i].maxcoutdown = (rand() % config.cartimemod) + config.cartimedefault;
		game.cars[i].coutdown = rand() % game.cars[i].maxcoutdown;
		game.cars[i].type = (rand() % 2 == 0) ? 0 : rand() % 3; // 3 jest na razie na „chama” -PR-
		game.cars[i].x=rand() % WIDTH;
		game.cars[i].dir = (2 * (rand() % 2))-1;
	}
}

void selectlevel(game_t & game) {
	game.level = 0;
	attron(COLOR_PAIR(5));
	int inp;
	while (game.level == 0) {
		clear();
		move(5, 10);
		printw("Level Type:");
		move(6, 10);
		printw("(1) Easy Level");
		move(7, 10);
		printw("(2) Hard Level");
		move(8, 10);
		printw("(3) Frog Master Level");
		inp = getch();
		if (inp == '1')
			game.level = 1;
		else if (inp == '2')
			game.level = 2;
		else if (inp == '3')
			game.level = 3;
		refresh();
		usleep(1e6/10);
	}
}



// printing functions
void printint_(u64 a) {
	int t;
	if (a > 0){
		t = a % 10;
		a /= 10;
		printint_(a);
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
void printint(u64 a) {
	if (a == 0) {
		addch('0');
	} else {
		printint_(a);
	}
} /// zostały wymagania = 12i, 13i; olej = 14i, 11


void printline(u8 A[WIDTH], const u8 c[gametypes]) {
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

void ruch(game_t & game){
	car_t T = game.cars[SIZE-1];
	for (u0 i = SIZE-1; i > 0; i--) {
		game.cars[i] = game.cars[i-1];
	}
	game.cars[0] = T;
}

bool f_stop(u8 y1, u8 x1, u8 y2, u8 x2) {
	s8 dy = (y1 - y2) * 4;
	float dx = x1 - x2 - 0.5;
	return (dy*dy+dx*dx > 35);
}

s8 printcar(u0 Y, const config_t & config, car_t car[SIZE], u0 fy, s0 dy, u0 fx, bool active) {
	if (active != 1) return 0;

	s8 moved = 0;
	u8 y = WHEIGHT-(SIZE/2)+dy-1;
	car_t & C = car[Y%SIZE];

	if ((C.type != carstop) or f_stop(y, fx, Y, C.x)) {
		if (C.maxcoutdown <= C.coutdown) {
			if (fx + C.dir > 0 and fx + C.dir < WIDTH - 1)
				moved = C.dir;
			C.x += C.dir;
			C.coutdown = 0;

			if (C.x < 0 or C.x > WIDTH -2) {
				if (car[(Y+1)%SIZE].coutdown % 2 == 0) {
					if (C.x < 0)
						C.x = WIDTH -1;
					else
						C.x = -1;
					C.coutdown += 0;
				}
				else if (C.x < 0) C.dir = 1;
				else if (C.x > WIDTH -2) C.dir = -1;
			}
		} C.coutdown += 1;
	}

	move(Y, C.x + MARGIN);
	attron(COLOR_PAIR(4+C.type));
	addch(config.car);
	addch(config.car);

	
	if (C.type != cartransport) {
		if ((C.x == fx) or (C.x+1 == fx))
			return 5;
	} else {
		if ((C.x-moved == fx) or (C.x-moved+1 == fx))
			return moved;
	}
	return 0;
}

s8 printroad(game_t & game, const config_t & config){
	u8 r = 0;
	
	if (game.last_pos_y < game.y) {
		game.last_pos_y += 1;
		ruch(game);
	}
	
	for (int y = 0; y < SIZE; y++) {
		move(WHEIGHT -y -1, MARGIN);
		printline(game.road[game.last_pos_y + y], config.tiles);
		s8 info = printcar(WHEIGHT -y -1, config, game.cars, game.y, game.last_pos_y-game.y, game.x, game.road[game.last_pos_y + y][0] == gameroad);
		if (y == ((SIZE)/2)-game.last_pos_y+game.y) r = info;
	}
	return r; //-1 ,  0 , 1 or 5
}



// main function
int main() {

	s8 status = 0;

	config_t config;
	game_t game;
	setup();
	load(game, config);
	game.health = config.health;
	game.timedown = config.timedown * config.ups -1;
	selectlevel(game);

	srand(config.seed);

	int	  ch = 0;
	while ( ch != 'q' and game.health > 0 and game.y < config.length+1) {

	if (game.frogtime >= config.fspeed) {
		ch = getch();
		key(game, config, ch);
		game.frogtime = 0;
	}	game.frogtime += 1;
		game.time += 1;
		game.timedown -= 1;
		if (game.timedown == 0) game.health = 0;

		clear();

		status = printroad(game, config);
		if (status == 5) game.health = 0;
		else game.x += status;
		status = bocian(game, config);
		if (status == 5) game.health = 0;

		attron(COLOR_PAIR(3));
		move(1, MARGIN);
		printw("HEALTH: ");
		printint(game.health);
		move(2, MARGIN);
		printw("TIME (s): ");
		printint(game.time/config.ups);
		move(3, MARGIN);
		printw("LEFT (s): ");
		printint(game.timedown/config.ups);

		int t = game.last_pos_y-game.y -1+game.y-game.by;
		if (game.level > 1 and t >= -(SIZE)/2 and t <= (SIZE)/2) {
			attron(COLOR_PAIR(1));
			move(((WHEIGHT-(SIZE)/2))+t, MARGIN+game.bx);
			addch(config.bocian);
		}

		attron(COLOR_PAIR(config.frogcol));
		move(((WHEIGHT-(SIZE)/2))+game.last_pos_y-game.y -1, MARGIN+game.x);
		addch(config.frog);

		if (game.darktime >= config.darktime) {
			game.darktime = 0;
		}	game.darktime += 1;
		if (game.level > 2 and (game.darktime == 6 or game.darktime == 7 or game.darktime == 8 or game.darktime == 9 or game.darktime == 13 or game.darktime == 14)) clear();
		refresh();
		usleep(1e6/config.ups);
	}


	attron(COLOR_PAIR(1));
	move(((WHEIGHT-(SIZE)/2))+game.last_pos_y-game.y -1, MARGIN+game.x);
	addch(config.frog);
	refresh();
	usleep(1e6);
	flushinp();
	getch();

	/*  Clean up after ourselves  */

	delwin(stdscr);
	endwin();
	refresh();
 
	return EXIT_SUCCESS;
}




// update functions
u0 bocian(game_t & game, config_t & config) {
	if (game.level < 2) return 0;
	if (game.bociantime >= config.bspeed) {

		if (game.bx > game.x)
			game.bx -= 1;
		else if (game.bx < game.x)
			game.bx += 1;

		if (game.by > game.y)
			game.by -= 1;
		else if (game.by < game.y)
			game.by += 1;

		game.bociantime = 0;
	}	game.bociantime += 1;
	if (game.x == game.bx and game.y == game.by) return 5;
	return 0;
}

/*void pref(game_t & game, config_t & config) {
	int inp = '\0';
	u8 menu = 0;
	while (inp != 'q' or menu != 0) {
		if (inp == 'q') menu = 0;
		clear();
		attron(COLOR_PAIR(3));
			move(4, 4);
		//printint(config.frogcol);
		if (menu == 0) {
			printw("Preferences:");
			move(5, 8);
			printw("(1) frog color");
			move(6, 8);
			printw("(2) seed");
			inp = getch();
			if (inp == '1')
				menu = 1;
			else if (inp == '2')
				menu = 2;
		} else if (menu == 1) {
			printw("Frog color:");
			move(5, 10);
			printw("(1) rekotka");
			move(6, 10);
			printw("(2) ropucha");
			move(7, 10);
			printw("(3) kumak");
			move(5, 8);
		attron(COLOR_PAIR(frogcolrechotka));
			printw("@");
			move(6, 8);
		attron(COLOR_PAIR(frogcolropucha));
			printw("@");
			move(7, 8);
		attron(COLOR_PAIR(frogcolkumak));
			printw("@");
			inp = getch();
			if (inp == '1')
				config.frogcol = frogcolrechotka;
			else if (inp == '2')
				config.frogcol = frogcolropucha;
			else if (inp == '3')
				config.frogcol = frogcolkumak;
		} else { //menu == 2
			printw("seed = ");
			printint(config.seed);
			move(5, 10);
			printw("(+) +10");
			move(6, 10);
			printw("(-) -1");
			move(7, 10);
			printw("(0) =0");
			inp = getch();
			if (inp == '=')
				config.seed += +10;
			else if (inp == '-')
				config.seed += -1;
			else if (inp == '0')
				config.seed = 0;
		}
		refresh();
		usleep(1e6/10);
	}
}*/

void key(game_t & game, config_t & config, int ch) {
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
	u8 bspeed = 10; // updates / bocian speed
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

//		case 'p':
//			pref(game, config);
//			break;


		//case KEY_HOME:
		//	game.x = 0;
		//	game.y = 0;
		//	break;

		//case KEY_END:
		//	game.x = (cols - width);
		//	game.y = (rows - height);
		//	break;
	}
	flushinp(); // clear input bufer -PR-
}
