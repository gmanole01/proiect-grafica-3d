/* Deplasarea observatorului intr-o scena 3D
SURSA:  lighthouse3D:  http://www.lighthouse3d.com/tutorials/glut-tutorial/keyboard-example-moving-around-the-world/
Elemente de retinut:
- folosirea functiilor de desenare pentru a schita obiecte 3D
- schimbarea pozitiei observatorului se face in functia gluLookAt
- folosirea glutSpecialFunc si glutKeyboardFunc pentru interactiunea cu tastatura
*/

#include <gl/freeglut.h>
#include <gl/glut.h>
#include <glfw/glfw3.h>
#include <math.h>
#include "SOIL.h"
#include <stdio.h>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 850

#define TILE_SIZE  3.0f
#define TILE_COUNT 150

#define DISCONTINOUS_STRIP_COUNT    50
#define DISCONTINOUS_STRIP_WIDTH    1
#define DISCONTINOUS_STRIP_HEIGHT   5
#define DISCONTINOUS_STRIP_SPACING  10

#define LANE_COUNT 6
#define LANE_WIDTH 10

constexpr float degreesToRadians(float degrees)
{
	return degrees * static_cast<float>(M_PI) / 180.0f;
}

int screenWidth = WINDOW_WIDTH;
int screenHeight = WINDOW_HEIGHT;
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float cameraDistance = 5.0f;
float cameraPositionX = 0.0f;
float cameraPositionY = 5.0f;
float cameraPositionZ = cameraDistance;
float cameraDirectionX = 0.0f;
float cameraDirectionY = 0.0f;
float cameraDirectionZ = -1.0f;

int fast = 0;

// Poziția curentă a mouse-ului
int mouseX;
int mouseY;

class Strip {
public:
	float x;
	float y;
};

Strip strips[DISCONTINOUS_STRIP_COUNT];

void initStrips() {
	for(int i = 0; i < DISCONTINOUS_STRIP_COUNT; i++) {
		strips[i].x = 0;
		strips[i].y =
			(((float) DISCONTINOUS_STRIP_HEIGHT) / 2.0f) +
			((float) i) * (DISCONTINOUS_STRIP_HEIGHT + DISCONTINOUS_STRIP_SPACING);
	}
}

// angle of rotation for the camera direction
float camera_angle = M_PI;
// actual vector representing the camera's direction
float camera_lx = 0.0f, camera_lz = 1.0f;
// XZ position of the camera
float camera_x = 0.0f, camera_z = 0.0f;

typedef struct {
	float r;
	float g;
	float b;
} Color;

Color parseRGB(int r, int g, int b) {
	Color c;
	c.r = (float) r / 255.0f;
	c.g = (float) g / 255.0f;
	c.b = (float) b / 255.0f;
	return c;
}

GLuint textureID[6]; //the array of texture IDs
GLuint grass;

GLuint loadTexture(const char* filepath) {
	GLuint texture = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
		(
			filepath,
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);
	
	// check for an error during the load process
	if(texture == 0) {
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
	}
	
	// Typical Texture Generation
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return texture;
}

void drawSkybox(float size) {
	float halfSize = size / 2.0f;
	
	float yOffset = 10.0f;
	
	// Turn off depth writing
	glDepthMask(GL_FALSE);
	
	// +x face
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(halfSize, -halfSize + yOffset, -halfSize);
	glTexCoord2f(1, 0); glVertex3f(halfSize, -halfSize + yOffset, halfSize);
	glTexCoord2f(1, 1); glVertex3f(halfSize, halfSize + yOffset, halfSize);
	glTexCoord2f(0, 1); glVertex3f(halfSize, halfSize + yOffset, -halfSize);
	glEnd();
	
	// Continue this for each face of the cube, binding the correct texture and setting the correct vertices.
	// -x face
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-halfSize, -halfSize + yOffset, halfSize);
	glTexCoord2f(1, 0); glVertex3f(-halfSize, -halfSize + yOffset, -halfSize);
	glTexCoord2f(1, 1); glVertex3f(-halfSize, halfSize + yOffset, -halfSize);
	glTexCoord2f(0, 1); glVertex3f(-halfSize, halfSize + yOffset, halfSize);
	glEnd();
	
	// +y face
	glBindTexture(GL_TEXTURE_2D, textureID[2]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-halfSize, halfSize + yOffset, -halfSize);
	glTexCoord2f(1, 0); glVertex3f(halfSize, halfSize + yOffset, -halfSize);
	glTexCoord2f(1, 1); glVertex3f(halfSize, halfSize + yOffset, halfSize);
	glTexCoord2f(0, 1); glVertex3f(-halfSize, halfSize + yOffset, halfSize);
	glEnd();
	
	// -y face
	glBindTexture(GL_TEXTURE_2D, textureID[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-halfSize, -halfSize + yOffset, halfSize);
	glTexCoord2f(1, 0); glVertex3f(halfSize, -halfSize + yOffset, halfSize);
	glTexCoord2f(1, 1); glVertex3f(halfSize, -halfSize + yOffset, -halfSize);
	glTexCoord2f(0, 1); glVertex3f(-halfSize, -halfSize + yOffset, -halfSize);
	glEnd();
	
	// +z face
	glBindTexture(GL_TEXTURE_2D, textureID[4]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-halfSize, -halfSize + yOffset, halfSize);
	glTexCoord2f(1, 0); glVertex3f(halfSize, -halfSize + yOffset, halfSize);
	glTexCoord2f(1, 1); glVertex3f(halfSize, halfSize + yOffset, halfSize);
	glTexCoord2f(0, 1); glVertex3f(-halfSize, halfSize + yOffset, halfSize);
	glEnd();
	
	// -z face
	glBindTexture(GL_TEXTURE_2D, textureID[5]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(halfSize, -halfSize + yOffset, -halfSize);
	glTexCoord2f(1, 0); glVertex3f(-halfSize, -halfSize + yOffset, -halfSize);
	glTexCoord2f(1, 1); glVertex3f(-halfSize, halfSize + yOffset, -halfSize);
	glTexCoord2f(0, 1); glVertex3f(halfSize, halfSize + yOffset, -halfSize);
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Turn depth writing back on
	glDepthMask(GL_TRUE);
}

// draw a cuboid with the center in (x, y, z)
void drawCuboid(float x, float y, float z, float dx, float dy, float dz) {
	// internal define:
	// x: l-r
	// y: u-d
	// z: f-b
	
	const float dx2 = dx / 2;
	const float dy2 = dy / 2;
	const float dz2 = dz / 2;
	
	const float l = x - dx2;
	const float r = x + dx2;
	const float d = y - dy2;
	const float u = y + dy2;
	const float f = z - dz2;
	const float b = z + dz2;
	
	glBegin(GL_QUADS);
	
	// Top face
	glVertex3f(r, u, f);
	glVertex3f(l, u, f);
	glVertex3f(l, u, b);
	glVertex3f(r, u, b);
	
	// Bottom face
	glVertex3f(r, d, f);
	glVertex3f(l, d, f);
	glVertex3f(l, d, b);
	glVertex3f(r, d, b);
	
	// Front face
	glVertex3f(r, u, f);
	glVertex3f(l, u, f);
	glVertex3f(l, d, f);
	glVertex3f(r, d, f);
	
	// Back face
	glVertex3f(l, u, b);
	glVertex3f(r, u, b);
	glVertex3f(r, d, b);
	glVertex3f(l, d, b);
	
	// Left face
	glVertex3f(l, u, f);
	glVertex3f(l, u, b);
	glVertex3f(l, d, b);
	glVertex3f(l, d, f);
	
	// Right face
	glVertex3f(r, u, b);
	glVertex3f(r, u, f);
	glVertex3f(r, d, f);
	glVertex3f(r, d, b);
	
	glEnd();
}

struct Point {
	float x;
	float y;
	float z;
};

struct LightParams {
	// light source coords (top left)
	Point p;
	
	// true if dir is x, false if dir is z
	bool onX = false;
	
	// light source size
	float w = 2.0;
	float h = 1.5;
	
	// true if light goes to inf, false if goes to -inf
	bool positive = true;
	
	// light spread
	float spread = 2;
	
	// floor height
	float floor = -3;
	
	// angle of the light
	float angle = 1.4;
};

void vertex(Point p) {
	glVertex3f(p.x, p.y, p.z);
}

// call looking through the bottom
void drawCubicPipe(Point t[], Point b[]) {
	glBegin(GL_QUADS);
	vertex(t[0]);
	vertex(t[1]);
	vertex(b[1]);
	vertex(b[0]);
	
	vertex(t[1]);
	vertex(t[2]);
	vertex(b[2]);
	vertex(b[1]);
	
	vertex(t[2]);
	vertex(t[3]);
	vertex(b[3]);
	vertex(b[2]);
	
	vertex(t[3]);
	vertex(t[0]);
	vertex(b[0]);
	vertex(b[3]);
	glEnd();
}

void drawLight(LightParams p) {
	float sx = p.p.x;
	float sz = p.p.z;
	if (p.onX) {
		sz = p.p.z + p.w;
	}
	else {
		sx = p.p.x + p.w;
	}
	
	// [0] => top-left
	Point t[] = {
		p.p,
		{p.p.x, p.p.y - p.h, p.p.z},
		{sx, p.p.y - p.h, sz},
		{sx, p.p.y, sz},
	};
	
	float dist = (p.p.y - p.floor) / tan(p.angle);
	
	float dw = p.w * p.spread;
	float dh = p.h * p.spread;
	
	// coords of top-left projected on the floor
	float dx = p.p.x;
	float dz = p.p.z;
	
	if (!p.positive) {
		dist = -dist;
	}
	
	if (p.onX) {
		dx += dist;
	}
	else {
		dz += dist;
	}
	
	if (p.positive) {
		dx += dh / 4;
		dz += dh / 4;
	}
	else {
		dh = -dh;
		dx += dh / 4;
		dz += dh / 4;
	}
	
	Point b2 = { dx, p.floor, dz };
	Point b3 = { dx, p.floor, dz };
	Point b4 = { dx, p.floor, dz };
	
	if (p.onX) {
		b2.x -= dh;
		b3.x -= dh;
		b3.z += dw;
		b4.z += dw;
	}
	else {
		b2.z -= dh;
		b3.z -= dh;
		b3.x += dw;
		b4.x += dw;
	}
	
	Point b[] = {
		{ dx, p.floor, dz },
		b2, b3, b4
	};
	
	drawCubicPipe(t, b);
}

void drawWindows() {
	glBegin(GL_QUADS);
	
	// front
	glVertex3f(5.1, 7.0, 4.5);
	glVertex3f(5.1, 7.0, -4.5);
	glVertex3f(5.1, 3.0, -4.5);
	glVertex3f(5.1, 3.0, 4.5);
	
	// back
	glVertex3f(-5.1, 6.5, 4.0);
	glVertex3f(-5.1, 6.5, -4.0);
	glVertex3f(-5.1, 3.5, -4.0);
	glVertex3f(-5.1, 3.5, 4.0);
	
	// front left
	glVertex3f(4.5, 7.0, -5.1);
	glVertex3f(4.5, 3.0, -5.1);
	glVertex3f(0.5, 3.0, -5.1);
	glVertex3f(0.5, 7.0, -5.1);
	
	// back left
	glVertex3f(-0.5, 7.0, -5.1);
	glVertex3f(-0.5, 3.0, -5.1);
	glVertex3f(-4.5, 3.0, -5.1);
	glVertex3f(-4.5, 7.0, -5.1);
	
	// front right
	glVertex3f(4.5, 7.0, 5.1);
	glVertex3f(0.5, 7.0, 5.1);
	glVertex3f(0.5, 3.0, 5.1);
	glVertex3f(4.5, 3.0, 5.1);
	
	// back right
	glVertex3f(-0.5, 7.0, 5.1);
	glVertex3f(-4.5, 7.0, 5.1);
	glVertex3f(-4.5, 3.0, 5.1);
	glVertex3f(-0.5, 3.0, 5.1);
	
	glEnd();
}

void drawWheels() {
	glPushMatrix();
	glTranslatef(7.0, -2, 3.5);
	glutSolidCylinder(2, 2, 20, 20);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(-7.0, -2, 3.5);
	glutSolidCylinder(2, 2, 20, 20);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(7.0, -2, -5.5);
	glutSolidCylinder(2, 2, 20, 20);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(-7.0, -2, -5.5);
	glutSolidCylinder(2, 2, 20, 20);
	glPopMatrix();
}

void drawHeadlights() {
	glBegin(GL_QUADS);
	glVertex3f(10.01, 2.4, 4.9);
	glVertex3f(10.01, 2.4, 2.9);
	glVertex3f(10.01, 0.9, 2.9);
	glVertex3f(10.01, 0.9, 4.9);
	
	glVertex3f(10.01, 2.4, -4.9);
	glVertex3f(10.01, 2.4, -2.9);
	glVertex3f(10.01, 0.9, -2.9);
	glVertex3f(10.01, 0.9, -4.9);
	glEnd();
	
	/*GLfloat black[] = { 0.0, 0.0, 0.0, 0.4 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
	drawLight({ { 10.01, 2.4, 4.9 } });
	drawLight({ { 10.01, 2.4, -2.9 } });*/
}

void drawStoplights() {
	glBegin(GL_QUADS);
	glVertex3f(-10.01, 2.4, 4.9);
	glVertex3f(-10.01, 2.4, 2.9);
	glVertex3f(-10.01, 0.9, 2.9);
	glVertex3f(-10.01, 0.9, 4.9);
	
	glVertex3f(-10.01, 2.4, -4.9);
	glVertex3f(-10.01, 2.4, -2.9);
	glVertex3f(-10.01, 0.9, -2.9);
	glVertex3f(-10.01, 0.9, -4.9);
	glEnd();
}

// -3
void drawCar() {
	GLfloat green[] = { 0.09, 0.56, 0.56, 1.0 };
	GLfloat orange[] = { 0.87, 0.47, 0.10, 1.0 };
	GLfloat glass[] = { 0.31, 0.76, 0.90, 1.0 };
	GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat headlights[] = { 0.99, 0.96, 0.69, 1.0 };
	GLfloat stoplights[] = { 0.74, 0.16, 0.24, 1.0 };
	
	// body
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, green);
	drawCuboid(0, 0, 0, 20, 5, 10);
	
	// upper
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, orange);
	drawCuboid(0, 5, 0, 10, 5, 10);
	
	// windows
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glass);
	drawWindows();
	
	// wheels
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
	drawWheels();
	
	// lights
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, headlights);
	drawHeadlights();
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, stoplights);
	drawStoplights();
}

void drawPoliceCar() {
	GLfloat green[] = { 0.09, 0.09, 0.09, 1.0 };
	GLfloat orange[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat glass[] = { 0.31, 0.76, 0.90, 1.0 };
	GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat headlights[] = { 0.99, 0.96, 0.69, 1.0 };
	GLfloat stoplights[] = { 0.74, 0.16, 0.24, 1.0 };
	GLfloat bluelights[] = { 0.24, 0.16, 0.74, 1.0 };
	
	// body
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, green);
	drawCuboid(0, 0, 0, 20, 5, 10);
	
	// upper
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, orange);
	drawCuboid(0, 5, 0, 10, 5, 10);
	
	// windows
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glass);
	drawWindows();
	
	// wheels
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
	drawWheels();
	
	// lights
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, headlights);
	drawHeadlights();
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, stoplights);
	drawStoplights();
	
	// police lights
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, orange);
	// center
	glBegin(GL_QUADS);
	
	// front
	glVertex3f(1.0, 8.5, 1.0);
	glVertex3f(1.0, 7.5, 1.0);
	glVertex3f(1.0, 7.5, -1.0);
	glVertex3f(1.0, 8.5, -1.0);
	
	// top
	glVertex3f(1.0, 8.5, 1.0);
	glVertex3f(1.0, 8.5, -1.0);
	glVertex3f(-1.0, 8.5, -1.0);
	glVertex3f(-1.0, 8.5, 1.0);
	
	// back
	glVertex3f(-1.0, 8.5, 1.0);
	glVertex3f(-1.0, 8.5, -1.0);
	glVertex3f(-1.0, 7.5, -1.0);
	glVertex3f(-1.0, 7.5, 1.0);
	
	glEnd();
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, stoplights);
	drawCuboid(0, 8, 3, 2, 1, 4);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, bluelights);
	drawCuboid(0, 8, -3, 2, 1, 4);
}

void changeSize(int w, int h) {
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio = w * 1.0 / h;
	
	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);
	
	// Reset Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
	
	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1f, 1000.0f);
	
	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void drawTree(float r, float g, float b) {
	
	glTranslated(0, 0.0f, 1.5f);
	glRotated(-90.0f, 1.0f, 0.0f, 0.0f);
	
	GLfloat coneColor[4] = {r, g, b, 0.85f};
	
	GLfloat body[4] = {0.4627f, 0.3067f, 0.2823f, 0.85f};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, body);
	glutSolidCylinder(0.75, 3, 20, 20);
	
	glPushMatrix();
	glTranslated(0.0f, 0.0f, 3.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, coneColor);
	glutSolidCone(3.0f, 6.0f, 20, 20);
	
	glTranslated(0.0f, 0.0f, 2.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, coneColor);
	glutSolidCone(3.0f, 6.0f, 20, 20);
	
	glTranslated(0.0f, 0.0f, 2.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, coneColor);
	glutSolidCone(3.0f, 6.0f, 20, 20);
	glPopMatrix();
}

void drawTerrainTile(float x, float y) {
	float half = TILE_SIZE / 2.0f;
	
	glBegin(GL_QUADS);
	
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x - half, 0, y + half);
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x + half, 0, y + half);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x + half, 0, y - half);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x - half, 0, y - half);
	
	glEnd();
}

void renderTerrain() {
	float half = (float) TILE_SIZE / 2;
	
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, grass);
	
	for(int i = 0; i < 4; i++) {
		// i = 0 -> sus stanga
		// i = 1 -> sus dreapta
		// i = 2 -> jos dreapta
		// i = 3 -> jos stanga
		int x_multiplier, y_multiplier;
		if(i == 0) {
			x_multiplier = -1;
			y_multiplier = 1;
		} else if(i == 1) {
			x_multiplier = 1;
			y_multiplier = 1;
		} else if(i == 2) {
			x_multiplier = 1;
			y_multiplier = -1;
		} else {
			x_multiplier = -1;
			y_multiplier = -1;
		}
		for(int j = 0; j < TILE_COUNT; j++) {
			for(int k = 0; k < TILE_COUNT; k++) {
				float x = half + (float) x_multiplier * (float) j * TILE_SIZE;
				float y = half + (float) y_multiplier * (float) k * TILE_SIZE;
				
				drawTerrainTile(x, y);
			}
		}
	}
}

void renderRoad() {
	GLfloat asphalt[4] = {0.2, 0.2, 0.2, 1.0};
	GLfloat white[4] = {1, 1, 1, 1.0};
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, asphalt);
	drawCuboid(
		0, 0, 0,
		((LANE_COUNT + 2) * (DISCONTINOUS_STRIP_WIDTH + LANE_WIDTH)), 0.5, 1000
	);
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, white);
	
	for(int p = 0; p < 2; p++) {
		int orientation_multiplier;
		if(p == 0) {
			orientation_multiplier = 1;
		} else {
			orientation_multiplier = -1;
		}
		
		for(int i = 0; i < DISCONTINOUS_STRIP_COUNT; i++) {
			drawCuboid(
				strips[i].x, 0, (orientation_multiplier * strips[i].y),
				DISCONTINOUS_STRIP_WIDTH, 1, DISCONTINOUS_STRIP_HEIGHT
			);
		}
		
		for(int k = 0; k < 2; k++) {
			int multiplier;
			if(k == 0) {
				multiplier = -1;
			} else {
				multiplier = 1;
			}
			
			for(int i = 0; i < ((LANE_COUNT / 2) - 1); i++) {
				for(int j = 0; j < DISCONTINOUS_STRIP_COUNT; j++) {
					drawCuboid(
						strips[j].x + multiplier * ((i + 1) * (DISCONTINOUS_STRIP_WIDTH + LANE_WIDTH)),
						0,
						(orientation_multiplier * strips[j].y),
						DISCONTINOUS_STRIP_WIDTH, 1, DISCONTINOUS_STRIP_HEIGHT
					);
				}
			}
		}
	}
	
	// Left margin
	drawCuboid(
		(LANE_COUNT / 2) * (DISCONTINOUS_STRIP_WIDTH + LANE_WIDTH), 0, 0,
		DISCONTINOUS_STRIP_WIDTH, 1, 10000
	);
	
	// Right margin
	drawCuboid(
		-1 * (LANE_COUNT / 2) * (DISCONTINOUS_STRIP_WIDTH + LANE_WIDTH), 0, 0,
		DISCONTINOUS_STRIP_WIDTH, 1, 10000
	);
}

void renderScene() {
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, (float)229 / 255, 1, 0.1f);
	
	// Reset transformations
	glLoadIdentity();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	
	// sursa de lumina 0
	glEnable(GL_LIGHT0);
	GLfloat pozitial0[] = { 1.0, 5.0, 3.0, 0.0 };
	GLfloat alb[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat negru[] = { 0.0, 0.0, 0.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, pozitial0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, alb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, negru);
	//glLightfv (GL_LIGHT0, GL_DIFFUSE, alb);
	glLightfv(GL_LIGHT0, GL_SPECULAR, negru);
	//glLightfv (GL_LIGHT0, GL_SPECULAR, rosu);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.1);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.2);
	
	gluLookAt(cameraPositionX, cameraPositionY, cameraPositionZ,
			  cameraPositionX + cameraDirectionX, cameraPositionY + cameraDirectionY, cameraPositionZ + cameraDirectionZ,
			  0.0f, 1.0f, 0.0f);
	
	// Draw the skybox
	GLfloat white[4] = {1, 1, 1, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, white);
	glEnable(GL_TEXTURE_2D);
	glDepthFunc(GL_LEQUAL);
	drawSkybox(500.0f);
	glDepthFunc(GL_LESS);
	glDisable(GL_TEXTURE_2D);
	
	//efect de ceata
	GLfloat fogColor[] = { 0.9f, 0.9f, 0.9f, 1.0f }; // culoarea ceții
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogi(GL_FOG_MODE, GL_LINEAR); // modul de interpolare liniară a ceții
	glFogf(GL_FOG_START, 5.0f); // distanța de început a ceții
	glFogf(GL_FOG_END, 600.0f); // distanța de sfârșit a ceții
	glEnable(GL_FOG); // activarea ceții
	
	//transparenta
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_TEXTURE_2D);
	renderTerrain();
	glDisable(GL_TEXTURE_2D);
	
	renderRoad();
	
	// Masina de politie
	glPushMatrix();
	glTranslated(-((DISCONTINOUS_STRIP_WIDTH / 2) + (LANE_WIDTH / 2)), 2.15, 20);
	glScaled(0.5f, 0.5f, 0.5f);
	glRotated(270, 0, 1, 0);
	drawPoliceCar();
	glPopMatrix();
	
	// Masina sens opus
	glPushMatrix();
	glTranslated(-((DISCONTINOUS_STRIP_WIDTH / 2) + (LANE_WIDTH / 2)), 2.15, 50);
	glScaled(0.5f, 0.5f, 0.5f);
	glRotated(90, 0, 1, 0);
	drawCar();
	glPopMatrix();
	
	// Masina sens opus
	glPushMatrix();
	glTranslated(-((DISCONTINOUS_STRIP_WIDTH / 2) + (LANE_WIDTH / 2)), 2.15, 70);
	glScaled(0.5f, 0.5f, 0.5f);
	glRotated(90, 0, 1, 0);
	drawCar();
	glPopMatrix();
	
	// Masina sens opus
	glPushMatrix();
	glTranslated(-((DISCONTINOUS_STRIP_WIDTH / 2) + (LANE_WIDTH / 2) + (1 * (LANE_WIDTH + DISCONTINOUS_STRIP_WIDTH))), 2.15, -70);
	glScaled(0.5f, 0.5f, 0.5f);
	glRotated(90, 0, 1, 0);
	drawCar();
	glPopMatrix();
	
	// Masina sens opus
	glPushMatrix();
	glTranslated(-((DISCONTINOUS_STRIP_WIDTH / 2) + (LANE_WIDTH / 2) + (2 * (LANE_WIDTH + DISCONTINOUS_STRIP_WIDTH))), 2.15, 100);
	glScaled(0.5f, 0.5f, 0.5f);
	glRotated(90, 0, 1, 0);
	drawCar();
	glPopMatrix();
	
	// Masina sens opus
	glPushMatrix();
	glTranslated(-((DISCONTINOUS_STRIP_WIDTH / 2) + (LANE_WIDTH / 2) - (3 * (LANE_WIDTH + DISCONTINOUS_STRIP_WIDTH))), 2.15, 80);
	glScaled(0.5f, 0.5f, 0.5f);
	glRotated(90, 0, 1, 0);
	drawCar();
	glPopMatrix();
	
	// Masina sens opus (BUSITA)
	glPushMatrix();
	glTranslated(-((DISCONTINOUS_STRIP_WIDTH / 2) + (LANE_WIDTH / 2) + (4 * (LANE_WIDTH + DISCONTINOUS_STRIP_WIDTH))), 2.15, 100);
	glScaled(0.5f, 0.5f, 0.5f);
	glRotated(255, 0, 1, 0);
	drawCar();
	glPopMatrix();
	
	int offset = (LANE_COUNT / 2) * (DISCONTINOUS_STRIP_WIDTH + LANE_WIDTH) + (2 * LANE_WIDTH);

	Color treeColor = parseRGB(34, 139, 34);

	// Padure dreapta
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < 20; j++) {
			glPushMatrix();
			glTranslated(-offset - i * 30, 0, 15.0f + ((double) j) * 30.0f);
			drawTree(treeColor.r, treeColor.g, treeColor.b);
			glPopMatrix();
		}
	}
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < 20; j++) {
			glPushMatrix();
			glTranslated(-offset - i * 30, 0, -(15.0f + ((double) j) * 30.0f));
			drawTree(treeColor.r, treeColor.g, treeColor.b);
			glPopMatrix();
		}
	}

	// Padure stanga
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < 20; j++) {
			glPushMatrix();
			glTranslated(offset + i * 30, 0, 15.0f + ((double) j) * 30.0f);
			drawTree(treeColor.r, treeColor.g, treeColor.b);
			glPopMatrix();
		}
	}
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < 20; j++) {
			glPushMatrix();
			glTranslated(offset + i * 30, 0, -(15.0f + ((double) j) * 30.0f));
			drawTree(treeColor.r, treeColor.g, treeColor.b);
			glPopMatrix();
		}
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glutSwapBuffers();
}

void processSpecialKeys(int key, int xx, int yy) {
	
	float fraction;
	if(fast) {
		fraction = 10.0f;
	} else {
		fraction = 1.0f;
	}
	
	switch (key) {
		case GLUT_KEY_LEFT:
			cameraPositionX += cameraDirectionZ * fraction;
			cameraPositionZ -= cameraDirectionX * fraction;
			break;
		case GLUT_KEY_RIGHT:
			cameraPositionX -= cameraDirectionZ * fraction;
			cameraPositionZ += cameraDirectionX * fraction;
			break;
		case GLUT_KEY_UP:
			cameraPositionX += cameraDirectionX * fraction;
			cameraPositionZ += cameraDirectionZ * fraction;
			break;
		case GLUT_KEY_DOWN:
			cameraPositionX -= cameraDirectionX * fraction;
			cameraPositionZ -= cameraDirectionZ * fraction;
			break;
	}
	
	glutPostRedisplay();
}

void processNormalKeys(unsigned char key, int x, int y) {
	if (key == 'w') {
		processSpecialKeys(GLUT_KEY_UP, 0, 0);
	} else if (key == 'a') {
		processSpecialKeys(GLUT_KEY_LEFT, 0, 0);
	} else if (key == 's') {
		processSpecialKeys(GLUT_KEY_DOWN, 0, 0);
	} else if (key == 'd') {
		processSpecialKeys(GLUT_KEY_RIGHT, 0, 0);
	} else if (key == 'f') {
		fast = !fast;
	} else if (key == 27) {
		exit(1);
	}
}

void mouse(int x, int y) {
	
	static float lastX = (float) screenWidth / 2;
	static float lastY = (float) screenHeight / 2;
	
	// Verificăm dacă mouse-ul a ieșit în afara ferestrei
	if (x < 30 || x >= (screenWidth - 30) || y < 30 || y >= (screenHeight - 30)) {
		// Repozitionăm mouse-ul în centrul ferestrei
		glutWarpPointer(screenWidth / 2, screenHeight / 2);
		lastX = (float) screenWidth / 2;
		lastY = (float) screenHeight / 2;
		return;
	}
	
	float deltaX = (float) x - (float) lastX;
	float deltaY = (float) y - (float) lastY;
	
	lastX = (float) x;
	lastY = (float) y;
	
	float sensitivity = 0.5f;
	deltaX *= sensitivity;
	deltaY *= sensitivity;
	
	cameraYaw += deltaX;
	cameraPitch -= deltaY;
	
	if (cameraPitch > 89.0f)
		cameraPitch = 89.0f;
	if (cameraPitch < -89.0f)
		cameraPitch = -89.0f;
	
	float radYaw = degreesToRadians(cameraYaw);
	float radPitch = degreesToRadians(cameraPitch);
	
	float dirX = cos(radYaw) * cos(radPitch);
	float dirY = sin(radPitch);
	float dirZ = sin(radYaw) * cos(radPitch);
	
	// Calculăm lungimea vectorului
	float length = sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
	
	// Normalizăm vectorul
	if (length != 0.0f)
	{
		dirX /= length;
		dirY /= length;
		dirZ /= length;
	}
	
	cameraDirectionX = dirX;
	cameraDirectionY = dirY;
	cameraDirectionZ = dirZ;
}

int main(int argc, char **argv) {
	
	initStrips();
	
	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Scena 3D cu oameni de zapada");
	
	// Load textures
	textureID[0] = loadTexture("TEXTURES/right.png");
	textureID[1] = loadTexture("TEXTURES/left.png");
	textureID[2] = loadTexture("TEXTURES/top.png");
	textureID[3] = loadTexture("TEXTURES/bottom.png");
	textureID[5] = loadTexture("TEXTURES/front.png");
	textureID[4] = loadTexture("TEXTURES/back.png");
	grass = loadTexture("TEXTURES/grass.png");
	
	glutSetCursor(GLUT_CURSOR_NONE);
	
	// register callbacks
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	
	glutPassiveMotionFunc(mouse);
	
	// OpenGL init
	glEnable(GL_DEPTH_TEST);
	
	// enter GLUT event processing cycle
	glutMainLoop();
	
	return 1;
}

