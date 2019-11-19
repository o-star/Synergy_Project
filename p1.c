#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <sys/time.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define LEFTEDGE 20
#define RIGHTEDGE 80
#define TIMEVAL 40
#define TOWERBOTTOM 20     //화면에서의 제일 아래 블럭 y축위치
#define MAXVIEWEDBLOCKS 4  //게임중 화면에 보여질 블럭의 개수

int dir = 1;
int pos = LEFTEDGE;
int FLOOR = TOWERBOTTOM;
char blank[] = "        ";
double arrCenterX[100];     //블럭의 무게중심 x좌표들의 배열(인덱스는몇번쨰 블럭인지)
int arrBlockPosition[100];  //블럭의 왼쪽끝 x좌표들의 배열(인덱스는 몇번째 블럭인지)
int numStackedBlocks = 0;

void sig_handler();
int set_ticker(int n_msecs);
void set_cr_noecho_mode();
void stack_tower();	// 탑이 밑으로 쌓이는 과정
int can_stack(double);          //탑이 무너지지않게 블럭을 쌓을 수 있는지 체크하는 함수(쌓을 수 있으면 T, 없으면 F 반환)
void view_stack_cnt();          
void move_tower_down();         //화면에 일정 개수의 블럭이 쌓이면 탑을 아래로 내려줌 

int main()
{
	char c;
	char collapsed[] = "Tower is collapsed!!!!!!!!";
    char collapsed2[30];
	initscr();
	set_cr_noecho_mode();
	clear();


	signal(SIGALRM, sig_handler);
	srand(time(NULL));

	if (set_ticker(TIMEVAL) == -1)
		perror("set_ticker");


	while (1) {
		flushinp();
		c = getchar();
		switch (c) {
		case ' ': // spaec bar를 눌렸을때, 실행

			set_ticker(2000); // 탑이 다 떨어질때 까지의 시간 멈춰두는것
			stack_tower();
			if (!can_stack((double)pos))
			{
				signal(SIGALRM, SIG_IGN); // 무시되면, 더이상 블럭이 움직이지 않게 된다.
				clear(); // 화면 없애기
                sprintf(collapsed2,"Stacked block : %d",numStackedBlocks);
				mvaddstr(LINES / 2, (COLS - strlen(collapsed)) / 2, collapsed);
                mvaddstr(LINES/2+7,(COLS-strlen(collapsed))/2,collapsed2);
                refresh();
				sleep(2);
                endwin();
                return 0;
			}
            arrBlockPosition[numStackedBlocks] = pos;
            if(numStackedBlocks > MAXVIEWEDBLOCKS)
                move_tower_down();
            else
			    FLOOR -= 1; // 한층이 쌓였으니깐, FLOOR -1을 시킨다.

			pos = rand() % (RIGHTEDGE - LEFTEDGE) + LEFTEDGE;
			set_ticker(TIMEVAL);
			break;

		case 'q':
			endwin();
			return 0;
		}
	}
}

void move_tower_down(void)
{
    int row, idx;
    
    standend();
    for(row = FLOOR, idx = numStackedBlocks; row <= TOWERBOTTOM; row++, idx--)
        mvaddstr(row, arrBlockPosition[idx], blank);
    
    standout();
    for(row = FLOOR+1, idx = numStackedBlocks; row <= TOWERBOTTOM; row++, idx--)
        mvaddstr(row, arrBlockPosition[idx], blank);
    
    refresh();
}

int can_stack(double leftX)
{
	double curCenterX, centerX;

	curCenterX = leftX + (double)strlen(blank) / 2;
	centerX = 0;

	for (int i = numStackedBlocks; i > 0; i--)
	{
		double tempCenterX;
		if (i == numStackedBlocks)
			tempCenterX = ((numStackedBlocks - i) * centerX + curCenterX) / (numStackedBlocks - i + 1);
		else
			tempCenterX = ((numStackedBlocks - i) * centerX + arrCenterX[i + 1]) / (numStackedBlocks - i + 1);

		if (fabs(tempCenterX - arrCenterX[i]) > strlen(blank) / 2)
			return FALSE;

		centerX = tempCenterX;
	}
	numStackedBlocks++;
	arrCenterX[numStackedBlocks] = curCenterX;
	return TRUE;
}

void sig_handler() // 블럭이 좌우로 움직이는 구간
{
	move(0, pos);
	standend();
	addstr(blank);
	pos += dir;
	if (pos >= RIGHTEDGE)
		dir = -1;
	if (pos <= LEFTEDGE)
		dir = +1;
	move(0, pos);
	standout();
	addstr(blank);
	
	view_stack_cnt();
	curs_set(0);
	refresh();
}

void view_stack_cnt() {

	char stack_cnt_string[100];
	sprintf(stack_cnt_string, "Stacked block : %d ", numStackedBlocks);
	mvaddstr(30, RIGHTEDGE+10, stack_cnt_string);

}

int set_ticker(int n_msecs)
{
	struct itimerval new_timeset;
	long n_sec, n_usecs;

	n_sec = n_msecs / 1000;
	n_usecs = (n_msecs % 1000) * 1000L;

	new_timeset.it_interval.tv_sec = n_sec;
	new_timeset.it_interval.tv_usec = n_usecs;
	new_timeset.it_value.tv_sec = n_sec;
	new_timeset.it_value.tv_usec = n_usecs;

	return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

void set_cr_noecho_mode()
{
	struct termios ttystate;

	tcgetattr(0, &ttystate);
	ttystate.c_lflag &= ~ICANON;
	ttystate.c_lflag &= ~ECHO;
	ttystate.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &ttystate);
}

void stack_tower()
{
	int row_pos = 0;


	while (1) {
		move(row_pos, pos);
		standend();

		addstr(blank);

		row_pos += 1;

		move(row_pos, pos);
		standout();
		addstr(blank);
		curs_set(0);
		refresh();
		usleep(50000);		// 1초 미만 쉬어줄때 사용

		if (row_pos == FLOOR)
			break;
	}
}
