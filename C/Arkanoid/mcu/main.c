/*******************************************************************************
 * Projekt: IMP - Arkanoid na maticovem displeji
 * 
 * Autor: Jan Bednarik (xbedna45)
 * Datum: 15.12.2012
 * Kod: ORIGINAL
 * Popis:
 *      Implememntace rizeni hry Arkanoid na maticovem displeji
 * 
*******************************************************************************/

#include <fitkitlib.h>

#include <keyboard/keyboard.h>
#include <lcd/display.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define ROWS 7
#define COLS 10
#define COLS_HALF COLS/2
#define BRICKS_ROWS 4

// left display columns
#define P2OUT_MASK 0xF8       // bits used: 11111000
#define P2OUT_OFFSET 3        // first bit used

// right display columns
#define P1OUT_MASK 0x7C       // bits used: 01111100
#define P1OUT_OFFSET 2        // first bit used

// rows
#define ROW_REG P6OUT         // register for row bits
#define ROW_REG_MASK 0x7F     // bits used: 01111111

// move delay (msec)
#define MOVE_DELAY 200
// show matrix before play delay
#define SHOW_MATRIX_DELAY 1500

/* Ball's possible directions */
#define LEFT -1
#define RIGHT +1
#define UP -1
#define DOWN +1

/* Game difficulty */
#define LIVES 3

#define LEVELS 5

/* Globals */
/*-----------------*/

/* game states */
enum {
    MENU,
    GAMEPLAY,
    GAMEOVER,
    WINNER
};

typedef struct Game {
    int state;
    int lives;
    bool life_lost;
} TGame;

TGame game = {MENU, LIVES, false};

/* ball position */
typedef struct Ball {
    int x;
    int y;
} Tball;

Tball ball = {0, 0};

/* paddle position and width */
typedef struct Paddle {
    int y;
    int width;
} Tpaddle;

Tpaddle paddle = {0, 0};

/* Ball's actual direction */
typedef struct Direction {
    int x;      // UP | DOWN
    int y;      // LEFT | RIGHT
} Tdirection;

Tdirection direction = {0, 0};

/**
 * Gameplay matrix
*/
int mat[ROWS][COLS];

/* Bricks placement variations */
int brick_maps[][BRICKS_ROWS][COLS] = {
/* map 1 */
    {
        {0,0,1,1,1,1,1,1,0,0},
        {0,1,1,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,1,1,0,0},
        {0,0,0,1,1,1,1,0,0,0}
    },
    
/* map 2 */
    {
        {1,1,1,1,1,0,0,0,1,0},
        {1,0,1,0,1,1,1,1,1,0},
        {1,0,1,1,1,1,0,1,1,0},
        {1,1,0,0,0,1,1,1,1,1}
    },
    
/* map 3 */
    {
        {1,0,0,1,0,0,1,0,0,1},
        {0,1,1,0,1,1,0,1,1,0},
        {0,1,1,0,1,1,0,1,1,0},
        {1,0,0,1,0,0,1,0,0,1}
    },

/* map 4 */
    {
        {0,0,1,1,0,0,1,1,0,0},
        {0,0,1,1,0,0,1,1,0,0},
        {1,0,0,0,0,0,0,0,0,1},
        {0,1,1,1,1,1,1,1,1,0}
    },
    
/* map 5 */
    {
        {1,0,1,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,0,1,0,1}
    }
};  

int bricks_count = 0;
char last_ch; // last read character


/* Functions declaration */
/*------------------------*/
void print_user_help();
int keyboard_idle();
void fpga_initialized();
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd);
void fpga_interrupt_handler(unsigned char bits);

void init_game();
void bricks(int rows);
void draw_bricks(int map);
void draw_ball();
void redraw_ball();
void draw_paddle();
void redraw_paddle(int old_y);
void move_paddle(int dir);
void next_row(int *r);
void set_row(int r);
void set_columns(int r);
void putout_columns();
void flip_y();
void flip_x();
void remove_brick(int x, int y);
void move();


/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
}


/*******************************************************************************
 * Obsluha klavesnice
*******************************************************************************/
int keyboard_idle()
{
  char ch;
  ch = key_decode(read_word_keyboard_4x4());
  if (ch != last_ch) 
  {
    last_ch = ch;
    if (ch != 0) 
    {
      
      switch (game.state) {
          case MENU:
              if(ch == '*') {
                  game.state = GAMEPLAY;                  
              }
              break;

          case GAMEPLAY:
            /* left */
            if (ch == 'B') {
                move_paddle(LEFT);
            }

            /* right */
            if (ch == 'A') {
                move_paddle(RIGHT);
            }            
            break;
      }            
    }
  }
  return 0;
}



/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
  LCD_init();
  LCD_clear();
  LCD_append_string("Arkanoid");
}

/**
 * Prints message msg to LCd display 
 */
void print_LCD(char *msg) {
    LCD_clear();
    LCD_append_string(msg);    
}

/**
 * Initializaes position of paddle and ball
 */
void init_paddle_and_ball() {
    int r;
    int c;
    
    /* clear 2 bottom lines */
    for(r = ROWS-2; r < ROWS; r++) {
        for(c = 0; c < COLS; c++) {
            mat[r][c] = 0;
        }
    }
    
    ball.x = 5;
    ball.y = 4;
    
    direction.x = UP;
    direction.y = RIGHT;
    
    paddle.y = 3;
    paddle.width = 3;
    
    draw_ball();
    draw_paddle();
}

/**
 * Initializes the game state
 */
void init_game_state() {
    game.state = MENU;
    game.lives = 3;
    game.life_lost = false;
}

/**
 * Initializes the game
 */
void init_game(int map) {
    init_game_state();
    init_paddle_and_ball();
    draw_bricks(map);    
    bricks(BRICKS_ROWS);
}

/**
 * Shows gampeplay matrix
 * @param delay how long should matrix bw shown (ms)
 */
void show_gameplay_matrix(int delay) {
    int r = 0;
    int cnt = 0;
    
    while(cnt++ < delay) {
        putout_columns();
        set_row(r);
        set_columns(r);
        next_row(&r);
        delay_ms(1);
    }
}

/**
 * Counts initial count of bricks
 * @param rows brick field rows count 
 * @return bricks count
 */
void bricks(int rows) {
    int r;
    int c;    
    
    bricks_count = 0;
    
    for(r = 0; r < rows; r++) {
        for(c = 0; c < COLS; c++) {
            if(mat[r][c] == 1) bricks_count++;
        }
    }    
}

/**
 * Inserts bricks into gameplay matrix
 * @param map bricks placement variation
 */
void draw_bricks(int map) {
    int r;
    int c;
    
    for(r = 0; r < BRICKS_ROWS; r++) {
        for(c = 0; c < COLS; c++) {
            mat[r][c] = brick_maps[map][r][c];
        }
    }
}

/**
 * Inserts ball into gameplay matrix
 */
void draw_ball() {
    mat[ball.x][ball.y] = 1;    
}

/**
 * Redraws ball
 */
void redraw_ball() {
    mat[ball.x - direction.x][ball.y - direction.y] = 0;
    draw_ball();
}

/**
 * Inserts paddle into gampeplay matrix
 */
void draw_paddle() {
    int i;
    
    for(i = 0; i < paddle.width; i++) {
        mat[ROWS-1][paddle.y+i] = 1;
    }
}

/**
 * Redraws paddle position
 */
void redraw_paddle(int dir) {       
    if(dir == LEFT) 
        mat[ROWS-1][paddle.y+paddle.width] = 0;
    else            
        mat[ROWS-1][paddle.y-1] = 0;    
    
    draw_paddle();    
}

/**
 * Moves paddle one step to left or right
 * @param dir LEFT | RIGHT
 */
void move_paddle(int dir) {
    int new_y = paddle.y + dir;
    
    // move is possible
    if((new_y >= 0) && (new_y <= COLS-paddle.width)) {
        paddle.y = new_y;
        redraw_paddle(dir);
    }
}

/**
 * Increments row index in modulo fashion.
 * @param r row index
 */
void next_row(int *r) {
    //*r = ((*r) == (ROWS-1)) ? (0) : (*r + 1);
    *r = (*r + 1) % ROWS;
}

/**
 * Sets r-th bit in row register P6 to 1  
 * @param r row index
 */
void set_row(int r) {
    ROW_REG |= ROW_REG_MASK;       // 01111111
    ROW_REG &= (~(0x01 << r));
}

/**
 * Sets columns according to r-th row in gameplay amtrix
 * @param r row index
 */
void set_columns(int r) {
    uint8_t bits;
    int bit_offset;
    int skip_cols;
    int i;        
        
    /* left display cols - registr P2 [3-7] */
    
    bits = P2OUT;
    bits &= ~P2OUT_MASK;        // preserve bits we dont own
    bit_offset = P2OUT_OFFSET;        
    skip_cols = 0;
    for(i = 0; i < COLS_HALF; i++) {
        bits |= (mat[r][i+skip_cols] << (bit_offset+i));
    }
    P2OUT = bits;
    
    /* right display cols - registr P1 [2-6] */
    
    bits  = P1OUT;
    bits &= ~P1OUT_MASK;        // preserve bits we dont own
    bit_offset = P1OUT_OFFSET;        
    skip_cols = 5;
    for(i = 0; i < COLS_HALF; i++) {
        bits |= (mat[r][i+skip_cols] << (bit_offset+i));
    }
    P1OUT = bits;
}

/**
 * Puts out LEDs in all columns to avoid "ghosts".
 */
void putout_columns() {
    P2OUT &= ~P2OUT_MASK;
    P1OUT &= ~P1OUT_MASK;
}

/**
 * Flips ball's y direction
 */
void flip_y() {
    direction.y = ((direction.y == LEFT) ? RIGHT : LEFT);
}

/**
 * Flips ball's x direction
 */
void flip_x() {
    direction.x = ((direction.x == UP) ? DOWN : UP);
}

/**
 * Removes one brick from gameplay matrix.
 * @param x brick's x pos
 * @param y brick's y pos
 */
void remove_brick(int x, int y) {
    mat[x][y] = 0;
    bricks_count--;
}

/**
 * Checks state of the game
 */
void check_game_state() {
    char message[20];
    
    /* Player lost life */    
    if(game.life_lost == true) {
        game.life_lost = false;
        game.lives -= 1;                
        
        /* game over */
        if(game.lives == 0) {
            game.state = GAMEOVER;            
        }        
        /* just lost life */
        else {            
            init_paddle_and_ball();           
            sprintf(message, "Lives: %d", game.lives);
            print_LCD(message);
            show_gameplay_matrix(SHOW_MATRIX_DELAY);                                           
        }
    }
    
    /* Player won */
    if(bricks_count == 0) {
        game.state = WINNER;
    }
}

/**
 * One game step
 */
void move() {
    bool bounced;       // ball already bounced (changed direction)
    
    int new_y = ball.y + direction.y;
    int new_x = ball.x + direction.x;    
    
    while(bounced) {    
        bounced = false;
        // vertical wall
        if((new_y == COLS) || (new_y < 0)) {
            bounced = true;
            flip_y();
        } 
        // vertical brick
        else if (mat[ball.x][new_y] == 1) {
            bounced = true;
            remove_brick(ball.x, new_y);
            flip_y();
        }       

        // horizontal wall    
        if((new_x == ROWS) || (new_x < 0)) {
            bounced = true;
            flip_x();
        }
        // paddle
        else if((mat[new_x][ball.y] == 1) && (new_x == ROWS-1)) {
            bounced = true;
            flip_x();
        }
        // horizontal brick
        else if(mat[new_x][ball.y] == 1) {
            bounced = true;
            remove_brick(new_x, ball.y);
            flip_x();
        }

        // there is just one obstacle in diagonal direction or nothing
        if(!bounced) {
            // paddle
            if((mat[new_x][new_y] == 1) && (new_x == ROWS-1)) {
                flip_x();
                flip_y();
                bounced = true;
            }
            // brick
            else if(mat[new_x][new_y] == 1) {
                remove_brick(new_x, new_y);
                flip_x();
                flip_y();
                bounced = true;
            }              
        }
        new_y = ball.y + direction.y;
        new_x = ball.x + direction.x;
    }
    
    /* move */
    ball.y += direction.y;
    ball.x += direction.x;
    
    // life lost
    if(ball.x == ROWS-1) {
        game.life_lost = true;
    }       
    
    redraw_ball();
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  unsigned int cnt = 0;  
  last_ch = 0;    
  int level = 0;
  char message[20] = {'\0'};
  
  initialize_hardware();
  keyboard_init();    
  
  P6DIR |= 0x7F;  // set rows register to behave as OUTPUT
  P2DIR |= 0xF8;  // set left matrix columns register to behave as OUTPUT
  P1DIR |= 0x7C;  // set right matrix columns register to behave as OUTPUT

  //set_led_d6(1);                       // rozsviceni D6  

  int r = 0;            // matrix row index
  int row_delay = 1;        // one row delay - ms
  int move_delay = 0;       // time since last move  
  
  while (1) {
      
      /* main menu */      
                   
      init_game(level);   
      print_LCD("Press '*' to start");
      
      while(game.state == MENU) {
        terminal_idle();
        keyboard_idle();
      }      
      
      /* gameplay */
               
      show_gameplay_matrix(SHOW_MATRIX_DELAY);
      sprintf(message, "Lives: %d", game.lives);
      print_LCD(message);      
      
      while(game.state == GAMEPLAY) {          
        delay_ms(row_delay); // time to light-up one row

        putout_columns();
        set_row(r);
        set_columns(r);
        next_row(&r); // to next row          

        if ((move_delay += row_delay) >= MOVE_DELAY) {
            move();
            check_game_state();
            move_delay = 0;            
        }                

        terminal_idle();                   // obsluha terminalu
        keyboard_idle();                   // obsluha klavesnice		         
      }            
                  
      sprintf(message, (game.state == GAMEOVER) ? "Game Over" : "You won!");                
      print_LCD(message); 
      delay_ms(3000);
      
      level = (game.state == GAMEOVER) ? (0) : ((level+1) % LEVELS);
    }
}
