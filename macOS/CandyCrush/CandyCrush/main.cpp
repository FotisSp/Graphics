#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <thread>  // std::this_thread::sleep_for
#include <chrono>  // std::chrono::seconds
#include <iostream>// cout, cerr
#include <fstream> // ifstream
#include <sstream> // stringstream
#include <istream> // stringstream
using namespace std;

#define ROWS 12
#define COLUMNS 15
#define SQUAREPIXEL 40
#define USERINFO 20
#define SCISSORS 0
#define ROCK 1
#define PAPER 2
#define RED 3
#define BLUE 4
#define BOMB 5
#define NUMOFBOMBS 4
#define NUMOFEXPLOSIONS 5

GLsizei winWidth = COLUMNS*SQUAREPIXEL + COLUMNS;
GLsizei winHeight = ROWS*SQUAREPIXEL + USERINFO + ROWS;
GLuint texture[4];
GLuint scissorsImage[40][40];
GLuint rockImage[40][40];
GLuint paperImage[40][40];
GLuint bombImage[40][40];

int started;
int squares[ROWS][COLUMNS];
int firstClick, numberOfMoves, score;
int firstXPos, firstYPos, bombs;

void findTriple();
void initialiseGame();
void renderBitmapString(float x, float y, void *font, char *string);
void restorePerspectiveProjection();
void setOrthographicProjection();
void findMatches();
void highlightSquare(int i, int j);
void displayGrid();

int generateRandNum() {
	int r;
	
	do {
		r = rand() % 6;
	}while ((bombs == 0 || bombs > NUMOFBOMBS) && r == BOMB);
	if(r == BOMB)
	{
		bombs--;
	}
	return r;
}

void initGrid(){
	for (int i = 0; i < ROWS; i++)
		for (int j = 0; j < COLUMNS; j++)
			squares[i][j] = generateRandNum();
}

void init() {
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, COLUMNS, ROWS + 1, 0.0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	firstClick = 1;
	displayGrid();
	srand(time(NULL));
}

void initialiseGame() {
	int count = 0;
	int previousType = 6;
	
	for (int i = 0; i < ROWS; i++)				// Group of tiles on rows
	{
		previousType = squares[i][0];
		count = 1;
		for (int j = 1; j <= COLUMNS; j++)
		{
			if (squares[i][j] == previousType)
			{
				count++;
				if (count >= 3)
				{
					count = 1;
					do {
						squares[i][j] = generateRandNum();
					} while (previousType == squares[i][j]);
				}
				previousType = squares[i][j];
			}
			else
			{
				count = 1;
				previousType = squares[i][j];
			}
		}
	}
	
	for (int i = 0; i < COLUMNS; i++)					// Group of tiles on Columns
	{
		previousType = squares[0][i];
		count = 1;
		for (int j = 1; j <= ROWS; j++)
		{
			if (squares[j][i] == previousType)
			{
				count++;
				if (count >= 3)
				{
					count = 1;
					do {
						squares[j][i] = generateRandNum();
					} while (previousType == squares[j][i]);
				}
				previousType = squares[j][i];
			}
			else
			{
				count = 1;
				previousType = squares[j][i];
			}
		}
	}
}

void squareType(int type) {
	switch (type) {
		case SCISSORS:
			glEnable(GL_TEXTURE_2D);
			glColor3f(1, 1, 1);
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			break;
		case ROCK:
			glEnable(GL_TEXTURE_2D);
			glColor3f(1, 1, 1);
			glBindTexture(GL_TEXTURE_2D, texture[2]);
			break;
		case PAPER:
			glEnable(GL_TEXTURE_2D);
			glColor3f(1, 1, 1);
			glBindTexture(GL_TEXTURE_2D, texture[1]);
			break;
		case RED:
			glDisable(GL_TEXTURE_2D);
			glColor3f(1, 0, 0);
			//glBindTexture(GL_TEXTURE_2D, 0);
			break;
		case BLUE:
			glDisable(GL_TEXTURE_2D);
			glColor3f(0, 0, 1);
			//glBindTexture(GL_TEXTURE_2D, 0);
			break;
		case 6:
			glEnable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(0, 0, 0, 0.5);			// debug 8a figei meta auto
			break;
		case BOMB:
			glEnable(GL_TEXTURE_2D);
			glColor3f(1, 1, 1);
			glBindTexture(GL_TEXTURE_2D, texture[3]);
			break;
		default:
			printf("ERROR TYPE\n");
			exit(0);
	}
}

void drawImage(int i, int j) {
	if(started) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(i, j);
		glTexCoord2f(0.0, 1.0); glVertex2f(i + 1, j);
		glTexCoord2f(1.0, 1.0); glVertex2f(i + 1, j + 1);
		glTexCoord2f(1.0, 0.0); glVertex2f(i, j + 1);
		glEnd();
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex2f(i, j);
	glVertex2f(i + 1, j);
	glVertex2f(i + 1, j + 1);
	glVertex2f(i, j + 1);
	glEnd();
}

void displayGrid() {
	char text[200];
	glClear(GL_COLOR_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++) {
			squareType(squares[i][j]);
			drawImage(j, i);
			if(score == 0 && started == 0)
			{
				glutSwapBuffers();
				std::this_thread::sleep_for (std::chrono::microseconds(10000));
			}
		}
	}
	if (!numberOfMoves && started)
	{
		glColor3f(1.0, 0.0, 0.0);					// text color
		setOrthographicProjection();
		glPushMatrix();
		glLoadIdentity();
		sprintf(text, "Game Over ! Press <b> to Restart game or ESC to exit.");
		renderBitmapString(5, 500, GLUT_BITMAP_HELVETICA_18, text);
		sprintf(text, "Score : %d", score);
		renderBitmapString(490, 500, GLUT_BITMAP_HELVETICA_18, text);
		glPopMatrix();
		restorePerspectiveProjection();
		started = 0;
	}
	else if(started) {
		glColor3f(0.0, 0.0, 1.0);					// text color
		setOrthographicProjection();
		glPushMatrix();
		glLoadIdentity();
		sprintf(text, "To exit game press ESC.");
		renderBitmapString(5, 500, GLUT_BITMAP_HELVETICA_18, text);
		sprintf(text, "|Score %d|", score);
		renderBitmapString(270, 500, GLUT_BITMAP_HELVETICA_18, text);
		sprintf(text, "Remaining Moves (%d)", numberOfMoves);
		renderBitmapString(425, 500, GLUT_BITMAP_HELVETICA_18, text);
		glPopMatrix();
		restorePerspectiveProjection();
	}
	else {
		glColor3f(0.0, 0.0, 1.0);					// text color
		setOrthographicProjection();
		glPushMatrix();
		glLoadIdentity();
		sprintf(text, "Press 'b' to Start Game");
		renderBitmapString(230, 500, GLUT_BITMAP_HELVETICA_18, text);
		glPopMatrix();
		restorePerspectiveProjection();
	}
	glutSwapBuffers();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
	glLoadIdentity(); //Reset the drawing perspective
}

void resize(int newWidth, int newHeight) {
	/*  Reset viewport and projection parameters  */
	glViewport(0, 0, newWidth, newHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, COLUMNS, ROWS + 1, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	/*  Reset display-window size parameters.  */
	winWidth = newWidth;
	winHeight = newHeight;
}

void importPGM(GLuint imageArray[40][40], string file, int text) {
	std::ifstream infile(file.c_str());
	if (infile.is_open())
	{
		string skipLine;
		getline(infile, skipLine);
		getline(infile, skipLine);
		getline(infile, skipLine);
		
		for (int i = 0; i<40; i++)
		{
			for (int j = 0; j<40; j++)
			{
				infile >> imageArray[i][j];
			}
		}
	}
	else{
		printf("\nCannot open file (%s) !\nFile is missing.\n\n",file.c_str());
		exit(0);
	}
	infile.close();
	
	glGenTextures(1, &texture[text]);
	glBindTexture(GL_TEXTURE_2D, texture[text]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 40, 40, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageArray);
}

bool checkNeighbours(int fposX, int fposY, int posX, int posY) {
	if (fposX == posX && (fposY == posY - 1 || fposY == posY + 1))
	{
		return true;
	}
	else if (fposY == posY && ((fposX == posX - 1 || fposX == posX + 1)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

static void normalHotkeys(unsigned char key, int x, int y) {	// Normal keyboard events processing
	if (key == 27)													// Esc
	{
		exit(0);
	}
	else if ((key == 'b' || key == 'B') && started == 0)			// Start game
	{
		char text[200];
		
		bombs = NUMOFBOMBS;
		score = 0;
		importPGM(scissorsImage, "scissors.pgm", 0);
		importPGM(paperImage, "paper.pgm", 1);
		importPGM(rockImage, "rock.pgm", 2);
		importPGM(bombImage, "bomb.pgm", 3);
		cout << "Please enter number of moves: ";
		cin >> numberOfMoves;
		initGrid();
		initialiseGame();
		started = 1;
		displayGrid();
		glColor3f(0.0, 0.0, 1.0);					// text color
		setOrthographicProjection();
		
		glPushMatrix();
		glLoadIdentity();
		sprintf(text, "To exit game press ESC.");
		renderBitmapString(5, 500, GLUT_BITMAP_HELVETICA_18, text);
		sprintf(text, "|Score %d|", score);
		renderBitmapString(270, 500, GLUT_BITMAP_HELVETICA_18, text);
		sprintf(text, "Remaining Moves (%d)", numberOfMoves);
		renderBitmapString(425, 500, GLUT_BITMAP_HELVETICA_18, text);
		glPopMatrix();
		restorePerspectiveProjection();
	}
}

bool checkForType(int prev, int current, int option) {
	if (prev == SCISSORS && current == PAPER)
	{
		return true;
	}
	else if (prev == ROCK && current == SCISSORS)
	{
		return true;
	}
	else if (prev == PAPER && current == ROCK)
	{
		return true;
	}
	else if ((prev == SCISSORS || prev == ROCK || prev == PAPER) && (current == RED || current == BLUE) && option == 1)
	{
		return true;
	}
	return false;
}

void paintBlack(int i, int j, int count, int type, int option) {
	int x, xx, y, yy, midR, midC;
	
	if (option == ROWS)	{
		if (i < 3) {
			x = 0;
			xx = i + 3;
		}
		else if (i < ROWS - 4) {
			x = i - 3;
			xx = i + 3;
		}
		else {
			x = i - 3;
			xx = (ROWS - i - 1) + i;
		}
		
		if (j - count < 3) {
			y = 0;
			yy = j + 2;
		}
		else if (j < COLUMNS - 4) {
			y = j - 3 - count;
			yy = j + 2;
		}
		else {
			y = j - 3 - count;
			yy = (COLUMNS - j - 1) + j;
		}
		midR = i;
	}
	else if (option == COLUMNS)	{
		if (j < 3) {
			y = 0;
			yy = j + 3;
		}
		else if (j < COLUMNS - 4) {
			y = j - 3;
			yy = j + 3;
		}
		else {
			y = j - 3;
			yy = j + (COLUMNS - j - 1);
		}
		
		if (i - count < 3) {
			x = 0;
			xx = i + 2;
		}
		else if (i < ROWS - 4) {
			x = i - count - 3;
			xx = i + 2;
		}
		else {
			x = i - count - 3;
			xx = (ROWS - i - 2) + i;
		}
		midC = j;
	}
	
	for (int k = x; k <= xx; k++)					// rows
	{
		for (int n = y; n <= yy; n++)			// columns
		{
			if (option == ROWS && k >= midR - 1 && k <= midR + 1)
			{
				if(squares[k][n] == BOMB)
				{
					bombs++;
					squares[k][n] = generateRandNum();
					score -=30;
				}
				else if (checkForType(type, squares[k][n],1)) {
					squares[k][n] = 6;
					score += 2;
				}
				else {
					score -= 1;
				}
			}
			else if (option == COLUMNS && k >= midC - 1 && k <= midC + 1)
			{
				if(squares[k][n] == BOMB)
				{
					bombs++;
					squares[k][n] = generateRandNum();
					score -=30;
				}
				else if (checkForType(type, squares[k][n], 1))
				{
					squares[k][n] = 6;
					score += 2;
				}
				else {
					score -= 1;
				}
			}
			else {
				if (checkForType(type, squares[k][n], 0))
				{
					squares[k][n] = 6;
					score += 3;
				}
			}
		}
	}
	displayGrid();
}

void collapseSquares() {
	int temp, flag = 0;
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			if (squares[i][j] == 6) {
				if (i == 0) {
					squares[i][j] = generateRandNum();
				}
				else {
					temp = squares[i][j];
					squares[i][j] = squares[i - 1][j];
					squares[i - 1][j] = temp;
					flag = 1;
				}
			}
		}
		displayGrid();
	}
	
	if (flag) {
		collapseSquares();
	}
	/*else {				// **if enabled it overides the explosions limit**
	 findMatches();		// O K. Foudos mas eipe oti boroume na
	 }*/						// periorisoume tis ekrixeis an theloume.
}

void findMatches() {
	int count = 0, type, explosions = 0;
	
	for (int i = 0; i < ROWS; i++)
	{
		type = squares[i][0];
		count = 1;
		for (int j = 1; j <= COLUMNS; j++)
		{
			if (squares[i][j] == type)
			{
				count++;
			}
			else if (count >= 3 && squares[i][j] != type && explosions <= NUMOFEXPLOSIONS)
			{
				int k = count;
				score += 10;
				while (k >= 1)
				{
					squares[i][j - k] = 6;
					k--;
				}
				paintBlack(i, j, count, type, ROWS);
				collapseSquares();
				count = 1;
				type = squares[i][j];
				explosions++;
			}
			else
			{
				count = 1;
				type = squares[i][j];
			}
		}
	}
	
	explosions = 0;
	for (int j = 0; j < COLUMNS; j++) {
		type = squares[0][j];
		count = 1;
		for (int i = 1; i <= ROWS; i++) {
			
			if (squares[i][j] == type)
			{
				count++;
			}
			else if (count >= 3 && squares[i][j] != type && explosions <= NUMOFEXPLOSIONS)
			{
				int k = count;
				score += 10;
				while (k >= 1)
				{
					squares[i-k][j] = 6;
					k--;
				}
				paintBlack(i, j, count, type, COLUMNS);
				collapseSquares();
				count = 1;
				type = squares[i][j];
				explosions++;
			}
			else
			{
				count = 1;
				type = squares[i][j];
			}
		}
	}
}

void renderBitmapString(float x, float y, void *font, char *string) {
	char *c;
	glRasterPos2f(x, y);
	for (c = string; *c != '\0'; c++)
	{
		glutBitmapCharacter(font, *c);
	}
}

void restorePerspectiveProjection() {
	glMatrixMode(GL_PROJECTION);
	// restore previous projection matrix
	glPopMatrix();
	
	// get back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}

void setOrthographicProjection() {
	// switch to projection mode
	glMatrixMode(GL_PROJECTION);
	
	// save previous matrix which contains the
	//settings for the perspective projection
	glPushMatrix();
	
	// reset matrix
	glLoadIdentity();
	
	// set a 2D orthographic projection
	gluOrtho2D(0, winWidth, winHeight, 0);
	
	// switch back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}

void swapSquares(int x, int y) {
	if (firstClick && squares[y][x] != BOMB) {
		firstClick = 0;
		firstXPos = x;
		firstYPos = y;
	}
	else if(!firstClick && squares[y][x] != BOMB) {
		firstClick = 1;
		int secondXPos = x;
		int secondYPos = y;
		if (checkNeighbours(firstXPos, firstYPos, secondXPos, secondYPos))
		{
			int temp = squares[firstYPos][firstXPos];
			squares[firstYPos][firstXPos] = squares[secondYPos][secondXPos];
			squares[secondYPos][secondXPos] = temp;
			numberOfMoves--;
			findMatches();
		}
	}
}

void highlightSquare(int i, int j) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(i, j);
	glVertex2f(i + 1, j);
	glVertex2f(i + 1, j + 1);
	glVertex2f(i, j + 1);
	glEnd();
	glutSwapBuffers();
}

void selectSquare(GLint button, GLint action, GLint xMouse, GLint yMouse) {
	if (button == GLUT_LEFT_BUTTON && action == GLUT_UP && started == 1)
	{
		int ClickXPos = (float)xMouse / winWidth*COLUMNS;
		int ClickYPos = (float)yMouse / (winHeight - USERINFO)*ROWS;
		swapSquares(ClickXPos, ClickYPos);
		displayGrid();
		highlightSquare(ClickXPos, ClickYPos);
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowPosition(800, 200);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("Candy Crash (Press Esc to Exit)");
	
	init();
	
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(normalHotkeys);
	
	glutMouseFunc(selectSquare);
	glutMainLoop();
	return 0;
}


