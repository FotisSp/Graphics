#define _CRT_SECURE_NO_DEPRECATE
#include <cstdlib>
#include <glut.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "SOIL.h"
using namespace std;

#define GL_CLAMP_TO_EDGE 0x812F
#define RED 0
#define GREEN 1
#define BLUE 2
#define EMPTY 3
#define T1 4
#define T2 5
#define T3 6
#define W 7
#define REDCOLOR {1.0f, 0.0f, 0.0f, alpha}
#define GREENCOLOR {0.0f, 1.0f, 0.0f, alpha}
#define BLUECOLOR {0.0f, 0.0f, 1.0f, alpha}
#define BLACKCOLOR {0.0f, 0.0f, 0.0f, alpha}
#define WHITECOLOR {1.0f, 1.0f, 1.0f, alpha}

typedef struct tagTeleporters
{
	int x, y, z;
} Teleporter;

typedef struct tagVertex
{
	int x, y, z;
	float minX, maxX, minZ, maxZ;
} Vertex;

typedef struct tagSquares
{
	int type;
	Vertex vertex;
} Squares;

typedef struct tagLevel
{
	int numOfSquares;
	Squares* square;
	Teleporter teleporter[2];
} Level;

typedef struct camPos
{
	GLdouble x, y, z;
} Position;

typedef struct camFront
{
	GLdouble x, y, z;
} Front;

typedef struct camUp
{
	GLdouble x, y, z;
} Up;

typedef struct Color
{
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
} squareColor;

int mainWindow, scoreboardWindow, gameOverWindow, infoWindow, rotationSpeedWindow;
GLsizei winWidth = 1024, winHeight = 800;
GLfloat lastX = 400, lastY = 300;
GLfloat yaw = 90.0f;
GLfloat pitch = -45.0f;
GLuint image[5], skybox[6];
Level* level;
Position cameraPos, tempPos, previousPos;
Front cameraFront;
Up cameraUp;
bool keyState[256];
bool firstMouse = true;
int lev = 0, n = 0, hammers = 0, topCamera = 0, firstPersonCam = 0, usedHammers = 0, numOfMoves = 0;
float angle = 0, score = 0, alpha = 0.5, rotationSpeed = 0.5;
int enabled = 0, endGame = 0, border = 20, enableBonus = 1;

void lights()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT1);

	GLfloat dirVector0[] = { (float)cameraFront.x, (float)cameraFront.y, (float)cameraFront.z, 1.0 };
	GLfloat lightPos0[] = { (float)cameraPos.x, (float)cameraPos.y, (float)cameraPos.z, 1.0 };
	GLfloat lightPos1[] = { 0.0f, 45.0f, 0.0f, 1.0 };

	GLfloat diffuseLight0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat speculartLight0[] = { 0.5f, 0.5f, 0.5f, 1.0f };

	GLfloat ambientLight1[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuseLight1[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat speculartLight1[] = { 0.5f, 0.5f, 0.5f, 1.0f };

	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, speculartLight0);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, speculartLight1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);

	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 50.0);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dirVector0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 20);   // light intensity
}

void drawCharacter()
{
	glPushMatrix();
	glTranslatef(tempPos.x, tempPos.y - 0.5, tempPos.z);
	glRotatef(250, 1.0, 1.0, 1.0);
	GLUquadricObj* cylinder;
	cylinder = gluNewQuadric();
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	gluCylinder(cylinder, 0.1, 0.1, 0.2, 10, 10);
	glPopMatrix();
}

void drawSquare(squareColor color)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(color.r, color.g, color.b, color.a);

	// RIGHT side
	glBegin(GL_POLYGON);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, -0.5, -0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.5, -0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, 0.5, 0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5, -0.5, 0.5);
	glEnd();

	// LEFT side
	glBegin(GL_POLYGON);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5, -0.5, 0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5, 0.5, 0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, 0.5, -0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, -0.5, -0.5);
	glEnd();

	// TOP side
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, 0.5, 0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.5, -0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, 0.5, -0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, 0.5, 0.5);
	glEnd();

	// BOTTOM side
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, -0.5, -0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, -0.5, 0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, -0.5, 0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, -0.5, -0.5);
	glEnd();

	// BACK side
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, -0.5, -0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.5, -0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, 0.5, -0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, -0.5, -0.5);
	glEnd();

	// FRONT side
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, -0.5, 0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.5, 0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, 0.5, 0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, -0.5, 0.5);
	glEnd();
}

void drawTeleportPortal(squareColor color)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(color.r, color.g, color.b, color.a);

	// TOP side
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, 0.4999, 0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.4999, -0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, 0.4999, -0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, 0.4999, 0.5);
	glEnd();

	// BOTTOM side
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, -0.4999, -0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, -0.4999, 0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, -0.4999, 0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, -0.4999, -0.5);
	glEnd();

	glTranslatef(0.0f, 0.0f, 0.0f);
	glColor4f(1.0, 0.0, 0.0, alpha);
	glutSolidSphere(0.2, 30.0, 30.0);
}

void drawFrontBack(int x, squareColor color)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image[4]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(color.r, color.g, color.b, color.a);

	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, x*1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, -0.5, x*0.4999);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.5, x*0.4999);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, 0.5, x*0.4999);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, -0.5, x*0.4999);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawFrontBackDoor(int x, squareColor color)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image[3]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(color.r, color.g, color.b, color.a);

	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, x*1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5, -0.5, x*0.499);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.3, x*0.499);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.3, 0.3, x*0.499);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.3, -0.5, x*0.499);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, x*1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.3, x*0.499);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, 0.5, x*0.499);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, 0.5, x*0.499);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5, 0.3, x*0.499);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, x*1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.3, -0.5, x*0.499);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.3, 0.3, x*0.499);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, 0.3, x*0.499);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, -0.5, x*0.499);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawRightLeft(int x, squareColor color)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image[4]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(color.r, color.g, color.b, color.a);

	glBegin(GL_POLYGON);
	glNormal3f(x*1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x*0.4999, -0.5, -x*0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x*0.4999, 0.5, -x*0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x*0.4999, 0.5, x*0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x*0.4999, -0.5, x*0.5);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawRightLeftDoor(int x, squareColor color)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image[3]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(color.r, color.g, color.b, color.a);

	glBegin(GL_POLYGON);
	glNormal3f(x*1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x*0.4999, -0.5, -x*0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x*0.4999, 0.3, -x*0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x*0.4999, 0.3, x*-0.3);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x*0.4999, -0.5, x*-0.3);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(x*1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x*0.4999, 0.3, -x*0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x*0.4999, 0.5, -x*0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x*0.4999, 0.5, x*0.5);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x*0.4999, 0.3, x*0.5);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(x*1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x*0.4999, -0.5, -x*-0.3);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x*0.4999, 0.3, -x*-0.3);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x*0.4999, 0.3, x*0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x*0.4999, -0.5, x*0.5);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawDoorsAndWalls(int y, int xz)
{
	Squares tempSq = level[y].square[xz + n];	// front
	if (tempSq.type != EMPTY && tempSq.type != W)
	{
		drawFrontBack(1, WHITECOLOR);
	}
	else if (y > 0)
	{
		int downFront = level[y - 1].square[xz + n].type;
		if ((xz + n) < n*n && downFront != EMPTY)
		{
			drawFrontBackDoor(1, WHITECOLOR);
		}
	}
	else
	{
		if (xz >= n*n - n && xz <= n*n)
		{
			drawFrontBack(1, WHITECOLOR);
		}
		else {
			drawFrontBackDoor(1, WHITECOLOR);
		}
	}

	tempSq = level[y].square[xz - n];			// back
	if (tempSq.type != EMPTY && tempSq.type != W)
	{
		drawFrontBack(-1, WHITECOLOR);
	}
	else if (y > 0)
	{
		int downBack = level[y - 1].square[xz - n].type;
		if ((xz - n) >= 0 && downBack != EMPTY)
		{
			drawFrontBackDoor(-1, WHITECOLOR);
		}
	}
	else
	{
		if (xz >= 0 && xz <= n - 1)
		{
			drawFrontBack(-1, WHITECOLOR);
		}
		else {
			drawFrontBackDoor(-1, WHITECOLOR);
		}
	}

	tempSq = level[y].square[xz - 1];			// left
	if (tempSq.type != EMPTY && tempSq.type != W)
	{
		drawRightLeft(-1, WHITECOLOR);
	}
	else if (y > 0)
	{
		int downBack = level[y - 1].square[xz - 1].type;
		if ((xz - 1) >= 0 && downBack != EMPTY)
		{
			drawRightLeftDoor(-1, WHITECOLOR);
		}
	}
	else
	{
		if (xz % n == 0)
		{
			drawRightLeft(-1, WHITECOLOR);
		}
		else {
			drawRightLeftDoor(-1, WHITECOLOR);
		}
	}

	tempSq = level[y].square[xz + 1];			// right 
	if (tempSq.type != EMPTY && tempSq.type != W)
	{
		drawRightLeft(1, WHITECOLOR);
	}
	else if (y > 0)
	{
		int downFront = level[y - 1].square[xz + 1].type;
		if ((xz + 1) < n*n && downFront != EMPTY)
		{
			drawRightLeftDoor(1, WHITECOLOR);
		}
	}
	else
	{
		if (xz % n == n - 1)
		{
			drawRightLeft(1, WHITECOLOR);
		}
		else {
			drawRightLeftDoor(1, WHITECOLOR);
		}
	}
}

void drawScene()
{
	glShadeModel(GL_SMOOTH);
	for (int i = 0; i < lev; i++)
	{
		for (int j = 0; j < level[i].numOfSquares; j++)
		{
			int xx = level[i].square[j].vertex.x;
			int yy = level[i].square[j].vertex.y;
			int zz = level[i].square[j].vertex.z;

			glPushMatrix();
			glTranslatef(xx*1.0, yy*1.0, zz*1.0);

			int type = level[i].square[j].type;
			if (type == RED)
			{
				drawSquare(REDCOLOR);
			}
			else if (type == GREEN)
			{
				drawSquare(GREENCOLOR);
			}
			else if (type == BLUE)
			{
				drawSquare(BLUECOLOR);
			}
			else if (type == T1)
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, image[0]);
				drawSquare(WHITECOLOR);
				glDisable(GL_TEXTURE_2D);
			}
			else if (type == T2)
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, image[1]);
				drawSquare(WHITECOLOR);
				glDisable(GL_TEXTURE_2D);
			}
			else if (type == T3)
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, image[2]);
				drawSquare(WHITECOLOR);
				glDisable(GL_TEXTURE_2D);
			}
			else if (type == W)
			{
				glDisable(GL_TEXTURE_2D);
				drawTeleportPortal(BLACKCOLOR);
			}
			else if (type == EMPTY && enableBonus)
			{
				drawDoorsAndWalls(i, j);
			}
			glPopMatrix();
		}
	}
}

void renderSkybox()
{
	glEnable(GL_TEXTURE_2D);

	if (enabled)
		glDisable(GL_BLEND);

	float x = 0, y = 0, z = 0, width = 90, height = 90, length = 90;
	x = x - width / 2;
	y = y - height / 2;
	z = z - length / 2;

	// Draw Front side
	glBindTexture(GL_TEXTURE_2D, skybox[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z + length);
	glEnd();

	// Draw Back side
	glBindTexture(GL_TEXTURE_2D, skybox[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
	glEnd();

	// Draw Left side
	glBindTexture(GL_TEXTURE_2D, skybox[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
	glEnd();

	// Draw Right side
	glBindTexture(GL_TEXTURE_2D, skybox[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
	glEnd();

	// Draw Up side
	glBindTexture(GL_TEXTURE_2D, skybox[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
	glEnd();

	// Draw Down side
	glBindTexture(GL_TEXTURE_2D, skybox[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	if (enabled)
		glEnable(GL_BLEND);
}

void setProjection(int width, int height)
{
	float ratio = 1.0f * width / height;
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, width, height);

	// Set the clipping volume
	gluPerspective(45, ratio, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void reshapeWindow(int width, int height)
{
	if (height < 580)
	{
		height = 580;
	}
	if (width < 500)
	{
		width = 500;
	}

	winHeight = height;
	winWidth = width;
	glutReshapeWindow(winWidth, winHeight);
	setProjection(winWidth, winHeight);

	if (!endGame)
	{
		glutSetWindow(scoreboardWindow);
		glutPositionWindow(winWidth - 220 - border, border);
		glutReshapeWindow(220, 50);
		setProjection(220, 50);
	}

	if (endGame)
	{
		glutSetWindow(gameOverWindow);
		glutPositionWindow((winWidth / 2) - 260, 55);
		glutReshapeWindow(520, 180);
		setProjection(520, 180);
	}

	if (keyState['r'] && !endGame)
	{
		glutSetWindow(rotationSpeedWindow);
		glutPositionWindow(winWidth - 220 - border, border * 4);
		glutReshapeWindow(220, 25);
		setProjection(220, 25);
	}

	if (keyState['i'] && !endGame)
	{
		glutSetWindow(infoWindow);
		glutPositionWindow(winWidth / 10, winHeight / 8);
		glutReshapeWindow(400, 480);
		setProjection(400, 480);
	}
}

void setOrthographicProjection(int right, int bottom)
{
	// switch to projection mode
	glMatrixMode(GL_PROJECTION);

	// save previous matrix which contains the
	//settings for the perspective projection
	glPushMatrix();

	// reset matrix
	glLoadIdentity();

	// set a 2D orthographic projection
	gluOrtho2D(0, right, bottom, 0);

	// switch back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}

void restorePerspectiveProjection()
{
	glMatrixMode(GL_PROJECTION);
	// restore previous projection matrix
	glPopMatrix();

	// get back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}

void renderBitmapString(float x, float y, float z, void *font, char *string)
{
	char *c;
	glRasterPos3f(x, y, z);
	for (c = string; *c != '\0'; c++)
	{
		glutBitmapCharacter(font, *c);
	}
}

void renderScoreboard()
{
	int noHammers = 0;
	char scoreBuf[50], hammerBuf[50];
	int OldWindow = glutGetWindow();
	glutSetWindow(scoreboardWindow);

	glClearColor(0.85, 0.85, 0.85, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	score = pow(n, 2) - numOfMoves * 10 - usedHammers * 50;
	sprintf(scoreBuf, "Score : %.2f", score);
	if (usedHammers < hammers)
	{
		sprintf(hammerBuf, "  Used hammers (%d/%d)", usedHammers, hammers);
	}
	else {
		noHammers = 1;
		sprintf(hammerBuf, "No more hammers left");
	}

	setOrthographicProjection(220, 50);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.0, 0.0, 0.0);
	renderBitmapString(50, 22, 0, GLUT_BITMAP_HELVETICA_18, scoreBuf);
	if (noHammers)
	{
		glColor3f(1.0, 0.0, 0.0);
	}
	renderBitmapString(22, 42, 0, GLUT_BITMAP_HELVETICA_18, hammerBuf);

	glPopMatrix();
	restorePerspectiveProjection();

	glutSwapBuffers();
	glutSetWindow(OldWindow);
}

void renderGameOver()
{
	int noHammers = 0;
	char scoreBuf[50], hammerBuf[50], message[100], exitMessage[50];
	int OldWindow = glutGetWindow();
	glutSetWindow(gameOverWindow);

	glClearColor(0.65, 0.65, 0.65, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	sprintf(message, "Congratulations for completing the Maze Game!!");
	sprintf(scoreBuf, "**** Score : %.2f ****", score);
	sprintf(hammerBuf, "**** Used hammers (%d/%d) ****", usedHammers, hammers);
	sprintf(exitMessage, "Press \"x/X\" to exit game.");

	setOrthographicProjection(520, 180);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.0, 0.0, 1.0);
	renderBitmapString(18, 50, 0, GLUT_BITMAP_TIMES_ROMAN_24, message);
	renderBitmapString(135, 100, 0, GLUT_BITMAP_TIMES_ROMAN_24, scoreBuf);
	renderBitmapString(102, 140, 0, GLUT_BITMAP_TIMES_ROMAN_24, hammerBuf);
	renderBitmapString(370, 170, 0, GLUT_BITMAP_HELVETICA_12, exitMessage);

	glPopMatrix();
	restorePerspectiveProjection();

	glutSwapBuffers();
	glutSetWindow(OldWindow);
}

void renderInfoWindow()
{
	char titleMovement[100], moveKeys[100], endTitle[100], spaceKey[100], hKey[100];
	char vKey[100], rKey[100], oKey[100], eKey[100], xKey[100], titleKeys[100], title[50];
	char rotationTitle[100], rotAInfo[100], rotDInfo[100], stopRot[100], toggleBonus[100];
	int OldWindow = glutGetWindow();
	glutSetWindow(infoWindow);

	glClearColor(0.65, 0.65, 0.65, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	sprintf(title, "Info Panel");
	sprintf(titleMovement, "----------- * MOVEMENT * -----------");
	sprintf(moveKeys, "Use WASD keys to move around the maze.");
	sprintf(titleKeys, "--------- * Other Useful Keys * ---------");
	sprintf(spaceKey, "* |Space| : Climb in the upper front empty square.");
	sprintf(hKey, "* |H\\h|     : Destroy the square you are looking at.");
	sprintf(vKey, "* |V\\v|     : Toggle between First Person Camera and Top Camera.");
	sprintf(rKey, "* |R\\r|      : Toggle between Rotation and Top Camera.");
	sprintf(oKey, "* |O\\o|     : Toggle Opacity On/Off.");
	sprintf(toggleBonus, "* |B\\b|     : Toggle Bonus Mode On/Off");
	sprintf(eKey, "* |E\\e|     : Check if you completed the game successfully.");
	sprintf(xKey, "* |X\\x|     : Exit game.");
	sprintf(endTitle, "-----------------------------------");
	sprintf(rotationTitle, "------- * During Camera Rotation * -------");
	sprintf(rotAInfo, "* Use wheel up (or left mouse click) to accelerate the rotation.");
	sprintf(rotDInfo, "* Use wheel down (or right mouse click) to decelerate the rotation.");
	sprintf(stopRot, "* Press the middle mouse button to stop the rotation.");
	sprintf(endTitle, "-----------------------------------");

	setOrthographicProjection(400, 500);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.0, 0.0, 1.0);
	renderBitmapString(158, 35, 0, GLUT_BITMAP_HELVETICA_18, title);

	glColor3f(1.0, 0.0, 0.0);		// titles / end titles
	renderBitmapString(7, 80, 0, GLUT_BITMAP_HELVETICA_18, titleMovement);
	renderBitmapString(7, 120, 0, GLUT_BITMAP_HELVETICA_18, endTitle);
	renderBitmapString(7, 160, 0, GLUT_BITMAP_HELVETICA_18, titleKeys);
	renderBitmapString(7, 340, 0, GLUT_BITMAP_HELVETICA_18, endTitle);
	renderBitmapString(7, 380, 0, GLUT_BITMAP_HELVETICA_18, rotationTitle);
	renderBitmapString(7, 460, 0, GLUT_BITMAP_HELVETICA_18, endTitle);

	glColor3f(0.0, 0.0, 0.0);		// text / info
	renderBitmapString(85, 100, 0, GLUT_BITMAP_HELVETICA_12, moveKeys);
	renderBitmapString(17, 180, 0, GLUT_BITMAP_HELVETICA_12, spaceKey);
	renderBitmapString(17, 200, 0, GLUT_BITMAP_HELVETICA_12, hKey);
	renderBitmapString(17, 220, 0, GLUT_BITMAP_HELVETICA_12, vKey);
	renderBitmapString(17, 240, 0, GLUT_BITMAP_HELVETICA_12, rKey);
	renderBitmapString(17, 260, 0, GLUT_BITMAP_HELVETICA_12, oKey);
	renderBitmapString(17, 280, 0, GLUT_BITMAP_HELVETICA_12, toggleBonus);
	renderBitmapString(17, 300, 0, GLUT_BITMAP_HELVETICA_12, eKey);
	renderBitmapString(17, 320, 0, GLUT_BITMAP_HELVETICA_12, xKey);
	renderBitmapString(17, 400, 0, GLUT_BITMAP_HELVETICA_12, rotAInfo);
	renderBitmapString(17, 420, 0, GLUT_BITMAP_HELVETICA_12, rotDInfo);
	renderBitmapString(17, 440, 0, GLUT_BITMAP_HELVETICA_12, stopRot);

	glPopMatrix();
	restorePerspectiveProjection();

	glutSwapBuffers();
	glutSetWindow(OldWindow);
}

void renderRotationSpeed()
{
	char rotSpeed[50];
	int OldWindow = glutGetWindow();
	glutSetWindow(rotationSpeedWindow);

	glClearColor(0.85, 0.85, 0.85, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(1.0, 0.5, 0.0);
	sprintf(rotSpeed, "Rotation Speed : (%.2f)", rotationSpeed);
	setOrthographicProjection(220, 25);

	glPushMatrix();
	glLoadIdentity();

	if (rotationSpeed >= 0.0)
	{
		renderBitmapString(18, 19, 0, GLUT_BITMAP_HELVETICA_18, rotSpeed);
	}
	else
	{
		renderBitmapString(10, 19, 0, GLUT_BITMAP_HELVETICA_18, rotSpeed);
	}

	glPopMatrix();
	restorePerspectiveProjection();

	glutSwapBuffers();
	glutSetWindow(OldWindow);
}

void importMazFile(const char *file)
{
	FILE *infile;
	char buf[100], skip[100], num[5];

	infile = fopen(file, "r");
	if (infile != NULL)
	{
		for (int i = 0; i < 3; i++)
		{
			fgets(buf, 100, infile);
			strcpy(skip, strtok(buf, "= "));
			if (strcmp(skip, "L") == 0)
			{
				strcpy(num, strtok(NULL, "="));
				lev = atoi(num);
			}
			else if (strcmp(skip, "N") == 0)
			{
				strcpy(num, strtok(NULL, "="));
				n = atoi(num);
			}
			else {
				strcpy(num, strtok(NULL, "="));
				hammers = atoi(num);
			}
		}

		level = (Level*)calloc(lev, sizeof(Level));
		if (level == NULL)
		{
			printf("Malloc Failed\n");
			getchar();
			exit(1);
		}

		int i = 0, j = 0, tp = 0, z;
		while (fgets(buf, 100, infile) != NULL)
		{
			if (!strncmp(buf, "LEVEL", 5) || !strncmp(buf, "END OF MAZE", 11))
			{
				if (!strncmp(buf, "END OF MAZE", 11))
				{
					break;
				}
				level[i].numOfSquares = n*n;
				level[i].square = (Squares*)malloc(n*n*sizeof(Squares));
				i++;
				j = 0;
				z = 0;
				tp = 0;
				fgets(buf, 100, infile);
			}

			const char delimeters[] = " \n\t";
			char *temp = strtok(buf, delimeters);
			while (temp)
			{
				Vertex tempV;
				tempV.x = j % n;
				tempV.maxX = (j % n) + 0.5;
				tempV.minX = (j % n) - 0.5;
				tempV.y = i - 1;
				tempV.z = z;
				tempV.maxZ = z + 0.5;
				tempV.minZ = z - 0.5;

				level[i - 1].square[j].vertex = tempV;
				if (j % n == n - 1)
				{
					z++;
				}

				if (strncmp(temp, "R", 1) == 0)
				{
					level[i - 1].square[j].type = RED;
				}
				else if (strncmp(temp, "G", 1) == 0)
				{
					level[i - 1].square[j].type = GREEN;
				}
				else if (strncmp(temp, "B", 1) == 0)
				{
					level[i - 1].square[j].type = BLUE;
				}
				else if (strncmp(temp, "E", 1) == 0)
				{
					level[i - 1].square[j].type = EMPTY;
				}
				else if (strncmp(temp, "T1", 2) == 0)
				{
					level[i - 1].square[j].type = T1;
				}
				else if (strncmp(temp, "T2", 2) == 0)
				{
					level[i - 1].square[j].type = T2;
				}
				else if (strncmp(temp, "T3", 2) == 0)
				{
					level[i - 1].square[j].type = T3;
				}
				else if (strncmp(temp, "W", 2) == 0)
				{
					if (tp > 1)
					{
						printf("ERROR ** Each level must have only two (2) teleporters!\n");
						getchar();
						exit(1);
					}
					level[i - 1].square[j].type = W;
					level[i - 1].teleporter[tp].x = j % n;
					level[i - 1].teleporter[tp].y = i - 1;
					level[i - 1].teleporter[tp].z = z;
					tp++;
				}
				else {
					printf("Fatal Error !!\n");
					printf("** %s **\n", temp);
					getchar();
					exit(1);
				}
				temp = strtok(NULL, delimeters);
				j++;
			}
		}
		if (j != n*n)
		{
			printf("File is indicating that there are less or more entries than i could find\n");
			printf("Please Check your file and try again\n");
			getchar();
			exit(1);
		}
		if (lev != i)
		{
			printf("File is indicating that there are less or more levels than i could find\n");
			printf("Please Check your file and try again\n");
			getchar();
			exit(1);
		}
	}
	else {
		printf("\nCannot open file (%s) !\nFile is missing.\n\n", file);
		getchar();
		exit(0);
	}

	fclose(infile);
}

void loadTexture(const char *file, int i)
{
	image[i] = SOIL_load_OGL_texture(
		file,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
}

void loadSkyBox(const char *file, int i)
{
	skybox[i] = SOIL_load_OGL_texture(
		file,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
}

void findRandomPosition()
{
	srand(time(NULL));

	do
	{
		tempPos.x = rand() % n;
		tempPos.y = 0.0;
		tempPos.z = rand() % n;
	} while (level[0].square[(int)((tempPos.z*n) + tempPos.x)].type != EMPTY);
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

	importMazFile("file.maz");
	loadTexture("Textures/T1.png", 0);
	loadTexture("Textures/T2.png", 1);
	loadTexture("Textures/T3.png", 2);
	loadTexture("Textures/door.png", 3);
	loadTexture("Textures/wall.png", 4);
	loadSkyBox("Textures/skyboxTexture/front.jpg", 0);
	loadSkyBox("Textures/skyboxTexture/back.jpg", 1);
	loadSkyBox("Textures/skyboxTexture/left.jpg", 2);
	loadSkyBox("Textures/skyboxTexture/right.jpg", 3);
	loadSkyBox("Textures/skyboxTexture/up.jpg", 4);
	loadSkyBox("Textures/skyboxTexture/down.jpg", 5);

	findRandomPosition();
	cameraPos = { n / 2.0, 2.0*lev, n / 2.0 };
	cameraFront = { 0.0f, -1.0f, 1.0f };
	cameraUp = { 0.0f, 1.0f, 0.0f };
	glViewport(0, 0, winWidth, winHeight);
}

void jumpCheck()
{
	bool foundSpot = false;
	int x = (int)(cameraPos.x + 0.5);
	int y = (int)(cameraPos.y + 0.5);
	int z = (int)(cameraPos.z + 0.5);
	int frontX = (int)(cameraFront.x + x + 0.5);
	int frontZ = (int)(cameraFront.z + z + 0.5);

	if (y < lev - 1)
	{
		Squares *topSq = level[y + 1].square;
		Squares *base = level[y].square;

		if (topSq[(z*n) + x].type == EMPTY)
		{
			if ((topSq[(frontZ*n) + frontX].type == EMPTY && base[(frontZ*n) + frontX].type != EMPTY) || topSq[(frontZ*n) + frontX].type == W)
			{
				foundSpot = true;
			}
			if (foundSpot)
			{
				cameraPos.y++;
				cameraPos.x = (GLdouble)frontX;
				cameraPos.z = (GLdouble)frontZ;
			}
		}
	}
	else if (y == lev - 1)
	{
		Squares *base = level[y].square;

		if (base[(frontZ*n) + frontX].type != EMPTY)
		{
			foundSpot = true;
		}
		if (foundSpot)
		{
			cameraPos.y++;
			cameraPos.x = (GLdouble)frontX;
			cameraPos.z = (GLdouble)frontZ;
		}
	}
}

void checkEndGame()
{
	if (cameraPos.y == lev)
	{
		endGame = 1;
		glutDestroyWindow(scoreboardWindow);
		gameOverWindow = glutCreateSubWindow(mainWindow, (winWidth / 2) - 260, 55, 520, 180);
		glutDisplayFunc(renderGameOver);
		keyState['v'] = false;
		topCamera = 1;
		rotationSpeed = 0.5;
		keyState['r'] = true;
	}
	if (keyState['x'])
	{
		if (cameraPos.y < lev)
		{
			score = 0.0;
		}
		exit(0);
	}
}

GLfloat convertToRads(GLfloat num)
{
	return num * 3.141592654f / 180;
}

void setRotationSpeed(int button, int state, int x, int y)
{
	if (keyState['r'] && !endGame)
	{
		if (button == 3 || button == 0)
		{
			if (state == GLUT_DOWN)
			{
				rotationSpeed += 0.1;
			}
		}
		if (button == 4 || button == 2)
		{
			if (state == GLUT_DOWN)
			{
				rotationSpeed -= 0.1;
			}
		}
		if (button == 1)
		{
			rotationSpeed = 0.0;
		}
	}
}

void mouseMotion(int xpos, int ypos)
{
	if (!keyState['r'])
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}
		GLfloat xoffset = xpos - lastX;
		GLfloat yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		GLfloat sensitivity = 0.5f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;					// yaw is x
		pitch += yoffset;				// pitch is y

		if (pitch > 89.0)				// Limit up and down camera movement to 90 degrees
			pitch = 89.0;
		if (pitch < -89.0)
			pitch = -89.0;

		// Update camera position and viewing angle
		cameraFront.x = cos(convertToRads(yaw)) * cos(convertToRads(pitch));
		cameraFront.y = sin(convertToRads(pitch));
		cameraFront.z = sin(convertToRads(yaw)) * cos(convertToRads(pitch));
	}
}

static void keyPressed(unsigned char key, int x, int y)
{
	previousPos = cameraPos;
	if (key == 'x' || key == 'X')
	{
		keyState['x'] = true;
		checkEndGame();
	}
	if (key == 'r' || key == 'R')
	{
		if (keyState['r'] && !endGame)
		{
			keyState['r'] = false;
			topCamera = 1;
			glutDestroyWindow(rotationSpeedWindow);
		}
		else if (!keyState['r'] && !keyState['v'])
		{
			keyState['r'] = true;
			rotationSpeed = 0.5;
			rotationSpeedWindow = glutCreateSubWindow(mainWindow, winWidth - 220 - border, border * 4, 220, 25);
			glutDisplayFunc(renderRotationSpeed);
		}
	}
	if (key == 'v' || key == 'V')
	{
		if (keyState['v'])
		{
			keyState['v'] = false;
			glDisable(GL_LIGHT0);

			topCamera = 1;
		}
		else if (!keyState['r'])
		{
			keyState['v'] = true;
			glEnable(GL_LIGHT0);

			firstPersonCam = 1;
		}
	}
	if (key == 'h' || key == 'H')
	{
		keyState['h'] = true;
	}
	if (key == 'w' || key == 'W')
	{
		keyState['w'] = true;
	}
	if (key == 's' || key == 'S')
	{
		keyState['s'] = true;
	}
	if (key == 'a' || key == 'A')
	{
		keyState['a'] = true;
	}
	if (key == 'd' || key == 'D')
	{
		keyState['d'] = true;
	}
	if (key == ' ') // Spacebar
	{
		checkEndGame();
		jumpCheck();
	}
	if (key == 'e' || key == 'E')
	{
		checkEndGame();
	}
	if (key == 'o' || key == 'O')
	{
		if (!enabled)
		{
			enabled = 1;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			enabled = 0;
			glDisable(GL_BLEND);
		}

	}
	if (key == 'i' || key == 'I')
	{
		if (keyState['i'])
		{
			keyState['i'] = false;
			glutDestroyWindow(infoWindow);
		}
		else if (!keyState['i'] && !endGame)
		{
			keyState['i'] = true;
			infoWindow = glutCreateSubWindow(mainWindow, winWidth / 10, winHeight / 8, 400, 500);
			glutDisplayFunc(renderInfoWindow);
			glutKeyboardFunc(keyPressed);
		}
	}
	if (key == 'b' || key == 'B')
	{
		if (enableBonus)
		{
			enableBonus = 0;
			glutPostRedisplay();
		}
		else {
			enableBonus = 1;
			glutPostRedisplay();
		}
	}
}

static void keyReleased(unsigned char key, int x, int y)
{
	if (key == 'h' || key == 'H')
	{
		keyState['h'] = false;
	}
	if (key == 'w' || key == 'W')
	{
		keyState['w'] = false;
	}
	if (key == 's' || key == 'S')
	{
		keyState['s'] = false;
	}
	if (key == 'a' || key == 'A')
	{
		keyState['a'] = false;
	}
	if (key == 'd' || key == 'D')
	{
		keyState['d'] = false;
	}
}

Vertex findPosition(GLdouble x, GLdouble z, GLdouble border)
{
	int y = (int)cameraPos.y;
	Vertex temp = { -1,-1,-1 };

	for (int i = 0; i < n*n; i++)
	{
		if (x < level[y].square[i].vertex.maxX &&
			x > level[y].square[i].vertex.minX &&
			z < level[y].square[i].vertex.maxZ &&
			z > level[y].square[i].vertex.minZ)
		{
			temp = level[y].square[i].vertex;
			temp.maxX -= border;
			temp.minX += border;
			temp.maxZ -= border;
			temp.minZ += border;
			break;
		}
	}
	return temp;
}

Position findEmptyNeighbor(Teleporter tp)
{
	Position temp = { -1.0,-1.0,-1.0 };

	int tempFront = level[tp.y].square[(tp.z*n) + tp.x + 1].type;
	int tempBase = level[tp.y - 1].square[(tp.z*n) + tp.x + 1].type;
	if (tempFront == EMPTY)
	{
		if (tp.y == 0)
		{
			temp.x = (GLdouble)tp.x + 1;
			temp.y = (GLdouble)tp.y;
			temp.z = (GLdouble)tp.z;
		}
		else if (tp.y > 0)//&& tempBase != EMPTY)				// enable gia na mn pefteis se tripious geitones otan kaneis teleport
		{
			temp.x = (GLdouble)tp.x + 1;
			temp.y = (GLdouble)tp.y;
			temp.z = (GLdouble)tp.z;
		}
	}

	tempFront = level[tp.y].square[(tp.z*n) + tp.x - 1].type;
	tempBase = level[tp.y - 1].square[(tp.z*n) + tp.x - 1].type;
	if (tempFront == EMPTY)
	{
		if (tp.y == 0)
		{
			temp.x = (GLdouble)tp.x - 1;
			temp.y = (GLdouble)tp.y;
			temp.z = (GLdouble)tp.z;
		}
		else if (tp.y > 0)//&& tempBase != EMPTY)				// enable gia na mn pefteis se tripious geitones otan kaneis teleport
		{
			temp.x = (GLdouble)tp.x - 1;
			temp.y = (GLdouble)tp.y;
			temp.z = (GLdouble)tp.z;
		}
	}

	tempFront = level[tp.y].square[((tp.z + 1)*n) + tp.x].type;
	tempBase = level[tp.y - 1].square[((tp.z + 1)*n) + tp.x].type;
	if (tempFront == EMPTY)
	{
		if (tp.y == 0)
		{
			temp.x = (GLdouble)tp.x;
			temp.y = (GLdouble)tp.y;
			temp.z = (GLdouble)tp.z + 1;
		}
		else if (tp.y > 0)//&& tempBase != EMPTY)				// enable gia na mn pefteis se tripious geitones otan kaneis teleport
		{
			temp.x = (GLdouble)tp.x;
			temp.y = (GLdouble)tp.y;
			temp.z = (GLdouble)tp.z + 1;
		}
	}

	tempFront = level[tp.y].square[((tp.z - 1)*n) + tp.x].type;
	tempBase = level[tp.y - 1].square[((tp.z - 1)*n) + tp.x].type;
	if (tempFront == EMPTY)
	{
		if (tp.y == 0)
		{
			temp.x = (GLdouble)tp.x;
			temp.y = (GLdouble)tp.y;
			temp.z = (GLdouble)tp.z - 1;
		}
		else if (tp.y > 0)//&& tempBase != EMPTY)				// enable gia na mn pefteis se tripious geitones otan kaneis teleport
		{
			temp.x = (GLdouble)tp.x;
			temp.y = (GLdouble)tp.y;
			temp.z = (GLdouble)tp.z - 1;
		}
	}
	return temp;
}

bool collisionDetection(int direction)
{
	int y = (int)cameraPos.y;

	if (y < lev)
	{
		GLdouble border = 0.15625;
		GLdouble frX, frZ;
		Vertex location = findPosition(cameraPos.x, cameraPos.z, border);

		if (location.x == -1)
		{	
			printf("ERROR ** LOCATION NOT FOUND ** ERROR\n");
			getchar();
			exit(1);
		}

		if (direction == 'w')
		{
			frX = cameraPos.x + (cameraFront.x * border);
			frZ = cameraPos.z + (cameraFront.z * border);
		}
		else if (direction == 's')
		{
			frX = cameraPos.x - (cameraFront.x * border);
			frZ = cameraPos.z - (cameraFront.z * border);
		}
		else if (direction == 'a')
		{
			frX = cameraPos.x - (cameraFront.y * cameraUp.z - cameraFront.z * cameraUp.y) * border;
			frZ = cameraPos.z - (cameraFront.x * cameraUp.y - cameraFront.z * cameraUp.x) * border;
		}
		else if (direction == 'd')
		{
			frX = cameraPos.x + (cameraFront.y * cameraUp.z - cameraFront.z * cameraUp.y) * border;
			frZ = cameraPos.z + (cameraFront.x * cameraUp.y - cameraFront.z * cameraUp.x) * border;
		}

		if (frX > location.maxX || frX < location.minX || frZ > location.maxZ || frZ < location.minZ)
		{
			Vertex front = findPosition(frX, frZ, 0);
			int frontSq = level[y].square[(front.z*n) + front.x].type;

			if (frontSq != EMPTY && frontSq != W)
			{
				return true;
			}

			if (frontSq == W)
			{
				Position emptyBlock;
				if (level[y].teleporter[0].z == front.z)
				{
					emptyBlock = findEmptyNeighbor(level[y].teleporter[1]);
					if (emptyBlock.x != -1.0)
					{
						cameraPos = emptyBlock;
						return false;
					}
				}
				else
				{
					emptyBlock = findEmptyNeighbor(level[y].teleporter[0]);
					if (emptyBlock.x != -1.0)
					{
						cameraPos = emptyBlock;
						return false;
					}
				}
				//return true;		// ti einai kalutero ? otan den exei adikrisma to allo portal ?
			}						// na pernas mesa apo to portal h na kanei collision ?
		}
	}
	return false;
}

void dropCheck()
{
	int x = (int)(cameraPos.x + 0.5);
	int y = (int)cameraPos.y;
	int z = (int)(cameraPos.z + 0.5);

	if ((y < lev && level[y].square[(z*n) + x].type != W) || y == lev)
	{
		while (y > 0 && level[y - 1].square[(z*n) + x].type == EMPTY)
		{
			y--;
		}
	}
	cameraPos.y = (GLfloat)y;
}

void calculateNumOfMoves(Position prev)
{
	int prevX = (int)(prev.x + 0.5);
	int currentX = (int)(cameraPos.x + 0.5);
	int prevZ = (int)(prev.z + 0.5);
	int currentZ = (int)(cameraPos.z + 0.5);

	if (prevX != currentX)
	{
		numOfMoves += abs(currentX - prevX);
	}
	if (prev.y != cameraPos.y)
	{
		numOfMoves += abs((int)(cameraPos.y - prev.y));
	}
	if (prevZ != currentZ)
	{
		numOfMoves += abs(currentZ - prevZ);
	}
}

void movementOperation()
{
	GLfloat cameraSpeed = 0.05f;
	GLfloat bounceBack = 3.0f;

	if (cameraPos.x < -0.5)				// left
	{
		cameraPos.x = -0.5;
	}
	else if (cameraPos.x > n - 0.5)		// right
	{
		cameraPos.x = n - 0.5;
	}
	else if (cameraPos.z < -0.5)		// back
	{
		cameraPos.z = -0.5;
	}
	else if (cameraPos.z > n - 0.5)		// forward
	{
		cameraPos.z = n - 0.5;
	}
	else
	{
		if (keyState['w'])
		{
			cameraPos.x += cameraSpeed * cameraFront.x;
			cameraPos.z += cameraSpeed * cameraFront.z;
			calculateNumOfMoves(previousPos);
			previousPos = cameraPos;
			if (collisionDetection('w'))
			{
				cameraPos.x -= bounceBack * cameraSpeed * cameraFront.x;
				cameraPos.z -= bounceBack * cameraSpeed * cameraFront.z;
			}
		}
		if (keyState['s'])
		{
			cameraPos.x -= cameraSpeed * cameraFront.x;
			cameraPos.z -= cameraSpeed * cameraFront.z;
			calculateNumOfMoves(previousPos);
			previousPos = cameraPos;
			if (collisionDetection('s'))
			{
				cameraPos.x += bounceBack * cameraSpeed * cameraFront.x;
				cameraPos.z += bounceBack * cameraSpeed * cameraFront.z;
			}
		}
		if (keyState['a'])
		{
			cameraPos.x -= cameraSpeed * (cameraFront.y * cameraUp.z - cameraFront.z * cameraUp.y);
			cameraPos.z -= cameraSpeed * (cameraFront.x * cameraUp.y - cameraFront.z * cameraUp.x);
			calculateNumOfMoves(previousPos);
			previousPos = cameraPos;
			if (collisionDetection('a'))
			{
				cameraPos.x += bounceBack * cameraSpeed * (cameraFront.y * cameraUp.z - cameraFront.z * cameraUp.y);
				cameraPos.z += bounceBack * cameraSpeed * (cameraFront.x * cameraUp.y - cameraFront.z * cameraUp.x);
			}
		}
		if (keyState['d'])
		{
			cameraPos.x += cameraSpeed * (cameraFront.y * cameraUp.z - cameraFront.z * cameraUp.y);
			cameraPos.z += cameraSpeed * (cameraFront.x * cameraUp.y - cameraFront.z * cameraUp.x);
			calculateNumOfMoves(previousPos);
			previousPos = cameraPos;
			if (collisionDetection('d'))
			{
				cameraPos.x -= bounceBack * cameraSpeed * (cameraFront.y * cameraUp.z - cameraFront.z * cameraUp.y);
				cameraPos.z -= bounceBack * cameraSpeed * (cameraFront.x * cameraUp.y - cameraFront.z * cameraUp.x);
			}
		}
	}

	if (keyState['h'])
	{
		int x = (int)(cameraPos.x + cameraFront.x + 0.5);
		int y = (int)(cameraPos.y + cameraFront.y + 0.5);
		int z = (int)(cameraPos.z + cameraFront.z + 0.5);

		if (y < lev && x < n && z < n && x >= 0 && z >= 0)
		{
			int frontSq = level[y].square[(z*n) + x].type;
			if (frontSq != EMPTY && hammers > usedHammers && frontSq != W)
			{
				usedHammers++;
				level[y].square[(z*n) + x].type = EMPTY;
				glutPostRedisplay();
			}
		}
	}
}

void cameraStates()
{
	if (topCamera)
	{
		topCamera = 0;
		cameraPos = { n / 2.0, 2.0*lev, n / 2.0 };
		cameraFront = { 0.0f, -1.0f, 1.0f };
	}
	if (keyState['r'])
	{
		float radius = n + 5.0;

		angle += rotationSpeed;

		GLfloat x = radius*sin(convertToRads(angle)) + n / 2.0;
		GLfloat z = radius*cos(convertToRads(angle)) + n / 2.0;
		cameraPos = { x, lev / 2.0f, z };
		cameraFront = { -x + (n / 2.0f), 0.0f, -z + (n / 2.0f) };

		if (angle > 360)
			angle = 0;
	}

	if (keyState['v'] && firstPersonCam)
	{
		firstPersonCam = 0;
		cameraPos = tempPos;
		cameraFront = { cameraPos.x, cameraPos.y, cameraPos.z };
	}
}

void renderScene(void)
{
	glutSetWindow(mainWindow);

	// Clear Color and Depth Buffers
	glClearColor(1.0f, 1.0f, 1.0f, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();

	//Set the camera
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
		cameraPos.x + cameraFront.x, cameraPos.y + cameraFront.y, cameraPos.z + cameraFront.z,
		cameraUp.x, cameraUp.y, cameraUp.z);

	renderSkybox();
	cameraStates();

	drawScene();
	lights();

	// Allow player to move only when in first person
	if (keyState['v'])
	{
		movementOperation();
		dropCheck();
		tempPos = cameraPos;
	}
	if (!keyState['v'])
	{
		drawCharacter();
	}

	if (!endGame)
	{
		renderScoreboard();
		if (keyState['r'])
		{
			renderRotationSpeed();
		}
	}
	else {
		renderGameOver();
	}

	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(800, 200);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(winWidth, winHeight);
	mainWindow = glutCreateWindow("Maze Game (Press (i) for info)");
	init();

	// Register Call Backs
	glutDisplayFunc(renderScene);
	glutReshapeFunc(reshapeWindow);
	glutIdleFunc(renderScene);

	// Keyboard call backs
	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyReleased);

	// Mouse call backs
	glutMouseFunc(setRotationSpeed);
	glutPassiveMotionFunc(mouseMotion);

	scoreboardWindow = glutCreateSubWindow(mainWindow, winWidth - 220 - border, border, 220, 50);
	glutDisplayFunc(renderScoreboard);

	glutMainLoop();
}