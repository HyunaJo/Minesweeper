#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// 색상 정의
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.

#define UP  0x48
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

//#define ENTER 0x0d 
#define SPACE 0x20 // 칸 선택
#define SET_FLAG1 'f' // 깃발 설치
#define SET_FLAG2 'F'
#define QUIT1 'q' // 게임 종료
#define QUIT2 'Q'
#define RESTART1 'r' // 게임 재시작
#define RESTART2 'R'
#define MENU1 'm' // 메뉴로 돌아가기
#define MENU2 'M'

#define MINE 1 // 지뢰 O
#define NO_MINE 0 // 지뢰 X

#define FLAG 1 // 깃발 O
#define NO_FLAG 0 // 깃발 X

#define SELECTED 1 // 칸 선택 O
#define NO_SELECTED 0 // 칸 선택 X

#define WIN 1 // 게임 승
#define LOSE 0 // 게임 패

#define RANKING_1 "ranking_beginner.txt"
#define RANKING_2 "ranking_intermediate.txt"
#define RANKING_3 "ranking_superior.txt"
#define DELETED -1

typedef struct Point {
	int mine_exist; // 지뢰 유무
	int aroundMine; // 주변 지뢰 개수
	int flag_exist; // 깃발 유무
	int selected; // 선택 유무
} Point;

typedef struct RankInfo {
	int play_time;
	int year;
	int month;
	int day;
	int rank;
} RankInfo;

int rows, cols; // 지뢰판 행,열
int total_mines; // 지뢰 총 개수
int selected_spaces; // 선택된 칸 수
int remain_mines; // 남은 지뢰 개수
int startX, startY; // 지뢰판 시작하는 좌표값
Point** map;

//내가 원하는 위치로 커서 이동
void gotoxy(int x, int y)
{
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}

void removeCursor(void) { // 커서를 안보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void showCursor(void) { // 커서를 보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void cls(int text_color, int bg_color) // 화면 지우기
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);
}

// box 그리기 함수
void draw_box()
{
	int x, y;
	int x1 = 0;
	int x2 = 79;
	int y1 = 0;
	int y2 = 23;

	for (x = x1; x <= x2; x++) {
		gotoxy(x, y1);
		printf("━");
		gotoxy(x, y2);
		printf("━");
	}

	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		printf("┃");
		gotoxy(x2, y);
		printf("┃");
	}

	gotoxy(x1, y1);
	printf("┏");
	gotoxy(x1, y2);
	printf("┗");
	gotoxy(x2, y1);
	printf("┓");
	gotoxy(x2, y2);
	printf("┛");
}

// 초기 메뉴창
int show_Menu()
{
	unsigned char ch;

	cls(WHITE, BLACK);
	draw_box();

	textcolor(BLACK, WHITE);
	gotoxy(32, 8);
	printf("               ");
	gotoxy(32, 9);
	printf("  지 뢰 찾 기  ");
	gotoxy(32, 10);
	printf("               ");

	gotoxy(34, 13);
	printf(" 1. START ");

	gotoxy(34, 15);
	printf(" 2. QUIT  ");

	gotoxy(34, 17);
	printf(" 3. RANK  ");

	textcolor(WHITE, BLACK);
	gotoxy(28, 20);
	printf("** HIT NUMBER YOU WANT **");

	while (1) {
		if (kbhit() == 1) {
			ch = getch();
			switch (ch) {
			case '1':
				return 1;
			case '2':
				return 2;
			case '3':
				return 3;
			}
		}
	}
}

// 문자 포함 유무 확인
int check_integer(char type[])
{
	int i;
	for (i = 0; type[i]; i++)
		if (type[i] < '0' || type[i] > '9')
			return FALSE; // 문자 포함한 경우 FALSE 리턴
	return TRUE; // 모두 정수인 경우 TRUE 리턴
}

// 사용자가 직접 지뢰판 행,열 설정
void custom_MineBoard()
{
	cls(WHITE, BLACK);
	draw_box();

	textcolor(BLACK, WHITE);
	gotoxy(33, 6);
	printf("             ");
	gotoxy(33, 7);
	printf(" 지뢰판 설정 ");
	gotoxy(33, 8);
	printf("             ");
	gotoxy(27, 20);
	printf("** 숫자 입력 후 ENTER **");

	char type_row[32];
	while (1) {
		textcolor(WHITE, BLACK);
		gotoxy(29, 11);
		printf("행 개수 (1~17) >> ");
		scanf("%s", type_row);
		if (check_integer(type_row) == FALSE) { // 문자 포함 유무 확인
			gotoxy(47, 11);
			printf("                                ");
			gotoxy(19, 12);
			textcolor(YELLOW1, BLACK);
			printf("** 1~17 사이 숫자로 다시 입력해주세요 **");
			continue;
		}
		else rows = atoi(type_row); // 정수만 입력했을 경우 정수로 변환 후 rows에 대입

		if (1 <= rows && rows <= 17) {
			gotoxy(19, 12);
			printf("                                        ");
			break;
		}
		else {
			gotoxy(19, 12);
			textcolor(YELLOW1, BLACK);
			printf("** 1~17 사이 숫자로 다시 입력해주세요 **");
			gotoxy(47, 11);
			printf("                                ");
		}
	}

	char type_col[32];
	while (1) {
		textcolor(WHITE, BLACK);
		gotoxy(29, 12);
		printf("열 개수 (1~30) >> ");
		scanf("%s", type_col);
		if (check_integer(type_col) == FALSE) { // 문자 포함 유무 확인
			gotoxy(47, 12);
			printf("                                ");
			gotoxy(19, 13);
			textcolor(YELLOW1, BLACK);
			printf("** 1~30 사이 숫자로 다시 입력해주세요 **");
			continue;
		}
		else cols = atoi(type_col); // 정수만 입력했을 경우 정수로 변환 후 cols에 대입

		if (1 <= cols && cols <= 30) {
			gotoxy(19, 13);
			printf("                                        ");
			break;
		}
		else {
			gotoxy(19, 13);
			textcolor(YELLOW1, BLACK);
			printf("** 1~30 사이 숫자로 다시 입력해주세요 **");
			gotoxy(47, 12);
			printf("                                ");
		}
	}

	int max_mines = cols * rows;
	char type_mines[32];
	while (1) {
		textcolor(WHITE, BLACK);
		gotoxy(29, 13);
		printf("지뢰 개수 (1~%d) >> ", max_mines);
		scanf("%s", type_mines);
		if (check_integer(type_mines) == FALSE) { // 문자 포함 유무 확인
			gotoxy(47, 13);
			printf("                                ");
			gotoxy(19, 14);
			textcolor(YELLOW1, BLACK);
			printf("** 1~%d 사이 숫자로 다시 입력해주세요 **", max_mines);
			continue;
		}
		else total_mines = atoi(type_mines); // 정수만 입력했을 경우 정수로 변환 후 total_mines에 대입

		if (1 <= total_mines && total_mines <= max_mines)
			break;
		else {
			gotoxy(19, 14);
			textcolor(YELLOW1, BLACK);
			printf("** 1~%d 사이 숫자로 다시 입력해주세요 **", max_mines);
			gotoxy(47, 13);
			printf("                                ");
		}
	}
}

void show_Level()
{
	cls(WHITE, BLACK);
	draw_box();

	textcolor(BLACK, WHITE);
	gotoxy(32, 4);
	printf("               ");
	gotoxy(32, 5);
	printf("  난이도 선택  ");
	gotoxy(32, 6);
	printf("               ");

	gotoxy(34, 9);
	printf("  1. 초급  ");

	gotoxy(34, 11);
	printf("  2. 중급  ");

	gotoxy(34, 13);
	printf("  3. 고급  ");

	gotoxy(31, 15);
	printf("  4. 사용자정의  ");

	gotoxy(32, 17);
	printf("  5. 뒤로가기  ");

	textcolor(WHITE, BLACK);
	gotoxy(28, 20);
	printf("** HIT NUMBER YOU WANT **");
}

// 난이도 선택창
int selectLevel()
{
	unsigned char ch;

	while (1) {
		if (kbhit() == 1) {
			ch = getch();
			switch (ch) {
			case '1':
				rows = 9;
				cols = 9;
				total_mines = 10;
				return 1;
			case '2':
				rows = 16;
				cols = 16;
				total_mines = 40;
				return 2;
			case '3':
				rows = 16;
				cols = 30;
				total_mines = 99;
				return 3;
			case '4':
				custom_MineBoard();
				return 4;
			case '5':
				return FALSE;
			}
		}
	}
}

// 지뢰판 그리기
void draw_MineBoard()
{
	cls(WHITE, BLACK);
	draw_box();

	int i, j;
	if (rows == 9 && cols == 9) {
		startX = 26;
		startY = 7;
	}
	else if (rows == 16 && cols == 16) {
		startX = 16;
		startY = 5;
	}
	else {
		startX = 2;
		startY = 5;
	}
	textcolor(GRAY2, BLACK);
	for (i = 0; i < cols; i++) {
		gotoxy(startX + 2 * i, startY);
		for (j = 0; j < rows; j++) {
			gotoxy(startX + 2 * i, startY + j);
			printf("■");
		}
	}
	textcolor(WHITE, BLACK);
}

// 시간 보여주기
void show_time(int run_time)
{
	textcolor(RED1, BLACK);
	gotoxy(74, 2);
	printf("%003d", run_time);
}

// 남은 지뢰개수 보여주기
void show_remainMines()
{
	textcolor(RED1, BLACK);
	gotoxy(4, 2);
	printf("%003d", remain_mines);
}

// 지뢰판 옆에 작동법 나타내기
void show_operation()
{
	textcolor(WHITE, BLACK);
	gotoxy(65, 11);
	printf("이동 ←↑↓→");
	gotoxy(65, 13);
	printf("선택 SPACE");
	gotoxy(65, 15);
	printf("깃발 F");
	gotoxy(65, 17);
	printf("메뉴 M");
	gotoxy(65, 19);
	printf("게임 초기화 R");
	gotoxy(65, 21);
	printf("게임 종료 Q");
}

// 주변 지뢰 개수 세기
void count_AroundMines()
{
	int count;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			count = 0;
			if (i - 1 >= 0) {
				if (j - 1 >= 0) {
					if (map[i - 1][j - 1].mine_exist == MINE)
						count++;
				}
				if (map[i - 1][j].mine_exist == MINE)
					count++;
				if (j + 1 < cols) {
					if (map[i - 1][j + 1].mine_exist == MINE)
						count++;
				}
			}
			if (i + 1 < rows) {
				if (map[i + 1][j].mine_exist == MINE)
					count++;
			}
			if (j - 1 >= 0) {
				if (map[i][j - 1].mine_exist == MINE)
					count++;
				if (i + 1 < rows) {
					if (map[i + 1][j - 1].mine_exist == MINE)
						count++;
				}
			}
			if (j + 1 < cols) {
				if (map[i][j + 1].mine_exist == MINE)
					count++;
				if (i + 1 < rows) {
					if (map[i + 1][j + 1].mine_exist == MINE)
						count++;
				}
			}
			map[i][j].aroundMine = count;
		}
	}
}

// 총개수만큼 지뢰 설치
void setMines()
{
	int i, j;

	// 지뢰 관련 정보 담고 있는 2차원 배열 생성
	map = (Point**)malloc(sizeof(Point*) * rows);
	for (int i = 0; i < rows; i++) {
		map[i] = (Point*)malloc(sizeof(Point) * cols);
	}
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			map[i][j].mine_exist = NO_MINE;
			map[i][j].aroundMine = 0;
			map[i][j].flag_exist = NO_FLAG;
			map[i][j].selected = NO_SELECTED;
		}
	}

	srand(time(NULL));
	int set_mines = total_mines; // 설치해야하는 지뢰 개수
	while (set_mines--) {
		do {
			i = rand() % rows;
			j = rand() % cols;
		} while (map[i][j].mine_exist == MINE);
		map[i][j].mine_exist = MINE;
	}

	count_AroundMines();
}

// 방향키로 움직이기
int moving(unsigned char ch, int oldx, int oldy, int* newx, int* newy)
{
	switch (ch) {
	case UP:
		if (oldx > 0) {
			*newx = oldx - 1;
			return 1;
		}
		break;
	case DOWN:
		if (oldx < rows - 1) {
			*newx = oldx + 1;
			return 1;
		}
		break;
	case RIGHT:
		if (oldy < cols - 1) {
			*newy = oldy + 1;
			return 1;
		}
		break;
	case LEFT:
		if (oldy > 0) {
			*newy = oldy - 1;
			return 1;
		}
		break;
	}
	return 0;
}

// 깃발 설치 및 해제
void set_flag(int new_m, int new_n)
{
	gotoxy(startX + new_n * 2, startY + new_m);
	if (map[new_m][new_n].flag_exist == FLAG) { // 이미 깃발 존재할 경우
		map[new_m][new_n].flag_exist = NO_FLAG;
		remain_mines++;
		textcolor(GRAY2, BLACK);
		printf("■");
	}
	else { // 깃발 없을 경우
		map[new_m][new_n].flag_exist = FLAG;
		remain_mines--;
		textcolor(BLUE1, BLACK);
		printf("▶");
	}
}

void print_aroundMines(int map_x, int map_y)
{
	gotoxy(startX + map_y * 2, startY + map_x);
	if (map[map_x][map_y].aroundMine == 0) {
		textcolor(GRAY2, BLACK);
		printf("□");
	}
	else {
		textcolor(GREEN1, BLACK);
		printf("%2d", map[map_x][map_y].aroundMine);
	}
}

// 주변 지뢰 수가 0인 칸 선택 시
void select_zero(int new_m, int new_n)
{
	if (new_m - 1 >= 0 && new_n - 1 >= 0) { // map[new_m - 1][new_n - 1]
		if (map[new_m - 1][new_n - 1].flag_exist == NO_FLAG) {
			if (map[new_m - 1][new_n - 1].selected == NO_SELECTED && map[new_m - 1][new_n - 1].mine_exist == NO_MINE) {
				map[new_m - 1][new_n - 1].selected = SELECTED;
				selected_spaces++;
				if (map[new_m - 1][new_n - 1].aroundMine == 0)
					select_zero(new_m - 1, new_n - 1);
			}
			print_aroundMines(new_m - 1, new_n - 1);
		}
	}

	if (new_m - 1 >= 0) { // map[new_m - 1][new_n]
		if (map[new_m - 1][new_n].flag_exist == NO_FLAG) {
			if (map[new_m - 1][new_n].selected == NO_SELECTED && map[new_m - 1][new_n].mine_exist == NO_MINE) {
				map[new_m - 1][new_n].selected = SELECTED;
				selected_spaces++;
				if (map[new_m - 1][new_n].aroundMine == 0)
					select_zero(new_m - 1, new_n);
			}
			print_aroundMines(new_m - 1, new_n);
		}
	}

	if (new_m - 1 >= 0 && new_n + 1 < cols) { // map[new_m - 1][new_n + 1]
		if (map[new_m - 1][new_n + 1].flag_exist == NO_FLAG) {
			if (map[new_m - 1][new_n + 1].selected == NO_SELECTED && map[new_m - 1][new_n + 1].mine_exist == NO_MINE) {
				map[new_m - 1][new_n + 1].selected = SELECTED;
				selected_spaces++;
				if (map[new_m - 1][new_n + 1].aroundMine == 0)
					select_zero(new_m - 1, new_n + 1);
			}
			print_aroundMines(new_m - 1, new_n + 1);
		}
	}

	if (new_n - 1 >= 0) { // map[new_m][new_n - 1]
		if (map[new_m][new_n - 1].flag_exist == NO_FLAG) {
			if (map[new_m][new_n - 1].selected == NO_SELECTED && map[new_m][new_n - 1].mine_exist == NO_MINE) {
				map[new_m][new_n - 1].selected = SELECTED;
				selected_spaces++;
				if (map[new_m][new_n - 1].aroundMine == 0)
					select_zero(new_m, new_n - 1);
			}
			print_aroundMines(new_m, new_n - 1);
		}
	}

	if (new_n + 1 < cols) { // map[new_m][new_n + 1]
		if (map[new_m][new_n + 1].flag_exist == NO_FLAG) {
			if (map[new_m][new_n + 1].selected == NO_SELECTED && map[new_m][new_n + 1].mine_exist == NO_MINE) {
				map[new_m][new_n + 1].selected = SELECTED;
				selected_spaces++;
				if (map[new_m][new_n + 1].aroundMine == 0)
					select_zero(new_m, new_n + 1);
			}
			print_aroundMines(new_m, new_n + 1);
		}
	}

	if (new_m + 1 < rows && new_n - 1 >= 0) { // map[new_m + 1][new_n - 1]
		if (map[new_m + 1][new_n - 1].flag_exist == NO_FLAG) {
			if (map[new_m + 1][new_n - 1].selected == NO_SELECTED && map[new_m + 1][new_n - 1].mine_exist == NO_MINE) {
				map[new_m + 1][new_n - 1].selected = SELECTED;
				selected_spaces++;
				if (map[new_m + 1][new_n - 1].aroundMine == 0)
					select_zero(new_m + 1, new_n - 1);
			}
			print_aroundMines(new_m + 1, new_n - 1);
		}
	}

	if (new_m + 1 < rows) { // map[new_m + 1][new_n]
		if (map[new_m + 1][new_n].flag_exist == NO_FLAG) {
			if (map[new_m + 1][new_n].selected == NO_SELECTED && map[new_m + 1][new_n].mine_exist == NO_MINE) {
				map[new_m + 1][new_n].selected = SELECTED;
				selected_spaces++;
				if (map[new_m + 1][new_n].aroundMine == 0)
					select_zero(new_m + 1, new_n);
			}
			print_aroundMines(new_m + 1, new_n);
		}
	}

	if (new_m + 1 < rows && new_n + 1 < cols) { // map[new_m + 1][new_n + 1]
		if (map[new_m + 1][new_n + 1].flag_exist == NO_FLAG) {
			if (map[new_m + 1][new_n + 1].selected == NO_SELECTED && map[new_m + 1][new_n + 1].mine_exist == NO_MINE) {
				map[new_m + 1][new_n + 1].selected = SELECTED;
				selected_spaces++;
				if (map[new_m + 1][new_n + 1].aroundMine == 0)
					select_zero(new_m + 1, new_n + 1);
			}
			print_aroundMines(new_m + 1, new_n + 1);
		}
	}
}

void select_space(int new_m, int new_n)
{
	map[new_m][new_n].selected = SELECTED;
	selected_spaces++;
	if (map[new_m][new_n].aroundMine == 0) { // 선택한 칸의 주변 지뢰 개수가 0인 경우
		select_zero(new_m, new_n);
	}
	else { // 선택한 칸의 주변 지뢰 개수가 1이상인 경우
		//map[new_m][new_n].selected = SELECTED;
		textcolor(GREEN1, BLACK);
		gotoxy(startX + new_n * 2, startY + new_m);
		printf("%2d", map[new_m][new_n].aroundMine);
	}
}

// 설치된 지뢰 표시
void print_Mines()
{
	textcolor(RED1, BLACK);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (map[i][j].mine_exist == MINE) {
				gotoxy(startX + j * 2, startY + i);
				printf("※");
			}
		}
	}
}

void print_flag()
{
	textcolor(BLUE1, BLACK);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (map[i][j].mine_exist == MINE) {
				gotoxy(startX + j * 2, startY + i);
				printf("▶");
			}
		}
	}
}

void win_Game()
{
	gotoxy(24, 1);
	printf("                                 ");
	gotoxy(24, 2);
	printf("  C O N G R A T U L A T I O N S  ");
	gotoxy(24, 3);
	printf("     지뢰를 모두 찾았습니다!     ");
	gotoxy(24, 4);
	printf("                                 ");
}

void lose_Game()
{
	gotoxy(29, 1);
	printf("                      ");
	gotoxy(29, 2);
	printf("  G A M E    O V E R  ");
	gotoxy(29, 3);
	printf("  지뢰가 터졌습니다!  ");
	gotoxy(29, 4);
	printf("                      ");
}

void end_Game(int game_result)
{
	int c1, c2;
	do {
		c1 = rand() % 16;
		c2 = rand() % 16;
	} while (c1 == c2);

	if (game_result == WIN) {
		print_flag();
		textcolor(c1, c2);
		win_Game();
	}
	else {
		print_Mines();
		textcolor(c1, c2);
		lose_Game();
	}

	textcolor(BLACK, WHITE);
	gotoxy(12, 22);
	printf("** Hit (R) TO RESTART (Q) TO QUIT (M) TO MENU **");

	Sleep(500);
}

void deleteMalloc()
{
	for (int i = 0; i < rows; i++)
		free(map[i]);
	free(map);
}

// 랭킹 데이터 개수
int getnumberofdata(FILE* fp)
{
	int filesize;
	int n;
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	n = filesize / sizeof(RankInfo);
	return n;
}

// 랭킹 초기화
void init_ranking(FILE* fp)
{
	RankInfo data;
	int n, ndata;

	// 초기화된 데이터
	data.play_time = DELETED;
	data.year = DELETED;
	data.month = DELETED;
	data.day = DELETED;
	data.rank = DELETED;

	// 모든 데이터 초기화
	ndata = getnumberofdata(fp);
	fseek(fp, 0, SEEK_SET);
	for (n = 0; n < ndata; n++)
		fwrite(&data, sizeof(data), 1, fp);
}

// 빈 공간 개수 세기
int getblank(FILE* fp, int ndata)
{
	RankInfo oldData;
	int n;

	fseek(fp, 0, SEEK_SET);
	for (n = 0; n < ndata; n++) {
		fread(&oldData, sizeof(oldData), 1, fp);
		if (oldData.play_time == DELETED) // 삭제된 데이터 찾은 경우
			return n;
	}
	return -1; // 삭제된 데이터 없는 경우
}

// 랭킹 10등인 데이터랑 비교
int get_lastRank(FILE* fp, int now_playtime)
{
	RankInfo oldData;
	int n, ndata;
	ndata = getnumberofdata(fp);

	fseek(fp, 0, SEEK_SET);
	for (n = 0; n < ndata; n++) {
		fread(&oldData, sizeof(oldData), 1, fp);
		if (oldData.play_time > now_playtime && oldData.rank == 10)
			return n;
	}
	return -1;
}

// 게임 승리 시 랭킹 텍스트 파일에 기록
void add_Rank(int select_Level, int run_time)
{
	if (select_Level == 4)
		return; // 사용자 정의는 랭킹 지원 안함

	FILE* fp = NULL;
	RankInfo newData;

	// data 내용
	time_t now = time(NULL);
	struct tm* tp;
	tp = localtime(&now);
	newData.year = tp->tm_year + 1900;
	newData.month = tp->tm_mon + 1;
	newData.day = tp->tm_mday;
	newData.play_time = run_time;
	newData.rank = 0; // 기본 랭킹 = 0

	switch (select_Level) {
	case 1:
		if ((fp = fopen(RANKING_1, "r+b")) == NULL) {
			fp = fopen(RANKING_1, "w+b");
		}
		break;
	case 2:
		if ((fp = fopen(RANKING_2, "r+b")) == NULL) {
			fp = fopen(RANKING_2, "w+b");
		}
		break;
	case 3:
		if ((fp = fopen(RANKING_3, "r+b")) == NULL) {
			fp = fopen(RANKING_3, "w+b");
		}
		break;
	}
	if (fp == NULL) {
		gotoxy(31, 13);
		textcolor(YELLOW1, BLACK);
		printf("파일을 열 수 없습니다.");
		exit(0);
	}

	int ndata, blank_n, last_n;
	ndata = getnumberofdata(fp);
	blank_n = getblank(fp, ndata); // 삭제된 데이터 개수
	if (ndata == 0) // 데이터 존재 X
		fseek(fp, 0, SEEK_END);
	else if (blank_n == -1) { // 삭제된 데이터 존재 X
		if (ndata < 10) // 총 데이터 수 10개 미만인 경우 맨 뒤에 기록
			fseek(fp, 0, SEEK_END);
		else {
			last_n = get_lastRank(fp, newData.play_time); // 랭킹 10등인 데이터 찾기
			if (last_n >= 0) // 만약 랭킹 10등인 데이터 기록보다 현재 기록이 좋으면
				fseek(fp, last_n * sizeof(RankInfo), SEEK_SET); // 랭킹 10등인 데이터 위치로 이동
			else {
				fclose(fp);
				return;
			}
		}
	}
	else { // 삭제된 데이터 위치로 이동
		fseek(fp, blank_n * sizeof(RankInfo), SEEK_SET);
	}
	fwrite(&newData, sizeof(newData), 1, fp);
	fclose(fp);
}

// 랭킹 변경
void set_Rank(FILE* fp, RankInfo record_Data[], int size)
{
	int i, j;
	init_ranking(fp);
	fseek(fp, 0, SEEK_SET);
	for (i = 0; i < size; i++) {
		record_Data[i].rank = 1;
		for (j = 0; j < size; j++) {
			if (record_Data[i].play_time > record_Data[j].play_time)
				record_Data[i].rank++;
		}
		fwrite(&record_Data[i], sizeof(RankInfo), 1, fp);
	}
}

// 전체 랭킹 출력
void list_Ranking(FILE* fp)
{
	int y, n, ndata;
	RankInfo data;

	y = 9; // y축 값
	ndata = getnumberofdata(fp);
	if (ndata == 0) {
		gotoxy(31, 13);
		textcolor(YELLOW1, BLACK);
		printf("** 랭킹 기록 X **");
		textcolor(WHITE, BLACK);
	}
	else {
		RankInfo record_Data[10];
		int size = 0;
		fseek(fp, 0, SEEK_SET);
		for (n = 0; n < ndata; n++) {
			fread(&data, sizeof(data), 1, fp);
			if (data.play_time != -1) {
				record_Data[size].play_time = data.play_time;
				record_Data[size].year = data.year;
				record_Data[size].month = data.month;
				record_Data[size].day = data.day;
				record_Data[size].rank = data.rank;
				size++;
			}
		}
		if (size == 0) {
			gotoxy(31, 13);
			textcolor(YELLOW1, BLACK);
			printf("** 랭킹 기록 X **");
			textcolor(WHITE, BLACK);
		}
		else {
			set_Rank(fp, record_Data, size);

			int i;
			int rank = 1;
			int print_data = 0; // 화면에 기록한 데이터 수
			while (1) {
				for (i = 0; i < size; i++) {
					if (record_Data[i].rank == rank) {
						// clear time 기록
						gotoxy(30, y);
						printf("%5d", record_Data[i].play_time);
						// date 기록
						gotoxy(45, y);
						printf("%d-%d-%d", record_Data[i].year, record_Data[i].month, record_Data[i].day);
						print_data++;
						y++;
						if (print_data == 10) // 화면에 데이터 10개 기록했을 시 기록 종료
							break;
					}
				}
				if (print_data == 10 || print_data == size) // 화면에 데이터 10개 기록했을 시 기록 종료
					break;
				rank++;
			}
		}
	}
}

// 랭킹 화면
int show_Ranking(FILE* fp)
{
	cls(WHITE, BLACK);
	draw_box();

	textcolor(BLACK, WHITE);
	gotoxy(32, 3);
	printf("               ");
	gotoxy(32, 4);
	printf(" R A N K I N G ");
	gotoxy(32, 5);
	printf("               ");

	textcolor(WHITE, BLACK);
	gotoxy(28, 7);
	printf("CLEAR TIME");
	gotoxy(47, 7);
	printf("DATE");


	unsigned char ch;

	list_Ranking(fp);

	while (1) {
		gotoxy(24, 20);
		printf("** 초기화 : D / 뒤로가기 : Q **");

		if (kbhit() == 1) {
			ch = getch();
			if (ch == 'd' || ch == 'D') {
				init_ranking(fp);
				textcolor(YELLOW1, BLACK);
				gotoxy(34, 22);
				printf("초기화 완료");
				textcolor(WHITE, BLACK);
				continue;
			}
			else if (ch == 'q' || ch == 'Q')
				break;
		}
	}
}

// 랭킹 확인
FILE* check_Ranking()
{
	unsigned char ch;
	FILE* fp;

	while (1) {
		if (kbhit() == 1) {
			ch = getch();
			switch (ch) {
			case '1':
				fp = fopen(RANKING_1, "r+b");
				if (fp == NULL) {
					gotoxy(50, 9);
					textcolor(YELLOW1, BLACK);
					printf("** 랭킹 파일 X **");
					continue;
				}
				return fp;
			case '2':
				fp = fopen(RANKING_2, "r+b");
				if (fp == NULL) {
					gotoxy(50, 11);
					textcolor(YELLOW1, BLACK);
					printf("** 랭킹 파일 X **");
					continue;
				}
				return fp;
			case '3':
				fp = fopen(RANKING_3, "r+b");
				if (fp == NULL) {
					gotoxy(50, 13);
					textcolor(YELLOW1, BLACK);
					printf("** 랭킹 파일 X **");
					continue;
				}
				return fp;
			case '4':
				gotoxy(50, 15);
				textcolor(YELLOW1, BLACK);
				printf("** 랭킹 지원 X **");
				continue;
			case '5':
				fp = NULL;
				return fp;
			}
		}
	}
}

void main()
{
	unsigned char ch;
	int select_Menu;
	int select_Level;
	int run_time, start_time;
	int old_m, old_n, new_m, new_n; // 2차원 배열에서의 m, n 값
	int game_result; // 게임 승패
	FILE* fp;

	removeCursor();

START_MENU:
	switch (show_Menu()) { // Menu에서 번호 선택
	case 1: // 게임 start
		show_Level();
		select_Level = selectLevel();
		if (select_Level == FALSE) { // Level 선택창에서 5번 선택 시
			goto START_MENU; // Menu 창으로 되돌아감
		}
		break;
	case 2: // 게임 quit
		textcolor(WHITE, BLACK);
		gotoxy(0, 24); // 끝나는 화면 위치
		return;
	case 3: // 랭킹 확인
		while (1) {
			show_Level();
			fp = check_Ranking();
			if (fp == NULL) // 5번 뒤로가기 누른 경우
				goto START_MENU;
			else {
				show_Ranking(fp); // 랭킹 보여주기
				fclose(fp);
			}
		}
	}

START_GAME:
	old_m = 0;
	old_n = 0;
	new_m = 0;
	new_n = 0;
	selected_spaces = 0;
	remain_mines = total_mines;

	setMines(); // 실제 지뢰판 만들기

	draw_MineBoard(); // 화면용 지뢰판 그리기
	/*
	print_Mines();
	*/
	gotoxy(startX + old_n * 2, startY + old_m); // 지뢰판 시작지점으로 커서 이동
	textcolor(BLACK, GRAY2);
	printf("▣");
	start_time = time(NULL);
	while (1) {
		run_time = time(NULL) - start_time;
		show_time(run_time);
		show_remainMines();
		show_operation();
		if (kbhit() == 1) {
			ch = getch();
			if (ch == SPECIAL1) {
				ch = getch();
				if (moving(ch, old_m, old_n, &new_m, &new_n)) { // 방향키 입력했을 경우
					gotoxy(startX + old_n * 2, startY + old_m); // 이전 칸으로 커서 이동
					if (map[old_m][old_n].flag_exist == FLAG) {
						textcolor(BLUE1, BLACK);
						printf("▶");
					}
					else if (map[old_m][old_n].selected == SELECTED) {
						print_aroundMines(old_m, old_n);
					}
					else {
						textcolor(GRAY2, BLACK);
						printf("■");
					}
					gotoxy(startX + new_n * 2, startY + new_m); // 새로운 칸으로 커서 이동
					textcolor(BLACK, GRAY2);
					printf("▣");
				}
				old_m = new_m;
				old_n = new_n;
			}
			else if (ch == SET_FLAG1 || ch == SET_FLAG2) { // 깃발 설치 및 해제
				if (map[new_m][new_n].selected == NO_SELECTED)
					set_flag(new_m, new_n);
			}
			else if (ch == SPACE) { // 칸 선택
				if (map[new_m][new_n].flag_exist == NO_FLAG) { // 깃발 설치된 칸은 선택 불가능
					if (map[new_m][new_n].selected == NO_SELECTED) { // 선택안된 칸
						if (map[new_m][new_n].mine_exist == NO_MINE) { // 지뢰 아닌 경우
							select_space(new_m, new_n);
						}
						else { // 지뢰인 경우
							game_result = LOSE;
							break;
						}
					}
				}
			}
			else if (ch == MENU1 || ch == MENU2) {
				deleteMalloc();
				goto START_MENU;
			}
			else if (ch == RESTART1 || ch == RESTART2) {
				deleteMalloc();
				goto START_GAME;
			}
			else if (ch == QUIT1 || ch == QUIT2) {
				deleteMalloc();
				textcolor(WHITE, BLACK);
				gotoxy(0, 24); // 끝나는 화면 위치
				return;
			}
		}
		if (selected_spaces == (rows * cols - total_mines)) { // 선택된 칸 수 == 지뢰 없는 칸 수 => 게임 승
			remain_mines = 0;
			show_remainMines();
			game_result = WIN;
			add_Rank(select_Level, run_time);
			break;
		}
	}

	while (1) {
		end_Game(game_result);

		if (kbhit()) {
			ch = getch();
			if (ch == MENU1 || ch == MENU2) {
				deleteMalloc();
				goto START_MENU;
			}
			else if (ch == RESTART1 || ch == RESTART2) {
				deleteMalloc();
				goto START_GAME;
			}
			else if (ch == QUIT1 || ch == QUIT2)
				break;
		}
	}

	deleteMalloc();
	textcolor(WHITE, BLACK);
	gotoxy(0, 24); // 끝나는 화면 위치
}