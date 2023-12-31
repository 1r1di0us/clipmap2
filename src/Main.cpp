//
// Geometry Clipmaps Project
// By Jesse Dirks and Eric Ross
// 
/////////////////////////////////////////////
//
// Based on Geometry Clip-Maps Tutorial
//
// (C) by Sven Forstmann in 2014
//
// License : MIT
// http://opensource.org/licenses/MIT
/////////////////////////////////////////////
// Mathlib included from 
// http://sourceforge.net/projects/nebuladevice/
/////////////////////////////////////////////
#include <iostream> 
#include <vector> 
#include <string> 
#include <stdio.h>
#include <glew.h>
#include <wglew.h>
#include <windows.h>
#include <mmsystem.h>
#include <GL/glut.h>

#define MOVESPEED 15 // forward/backward/left/right movement
#define FLYSPEED 0.00025 // up/down movement
#define MOUSESENSITIVITY 1 / 3.0 // pixels to degrees of rotation
#define VERTICALEXAGGERATION -0.40 // scales the verticality of the heightmap. 

using namespace std;

#include "glsl.h"
#include "Bmp.h"
#include "ogl.h"

#pragma comment(lib,"winmm.lib")

int grid= 64;				// patch resolution
int levels=5;				// LOD levels
int width,height;			// heightmap dimensions I think height is actually length.
double	viewangle = 225;	// nice initial value so we are looking in the right direction
double	viewoffsetx = 0;	// not sure how to get view to work without making these global variables.
double	initialView = 0;
bool	initializedView = false;
vec3f	viewpos(0, -0.25, 0);	// initial camera position
bool debug = true;	// debug toggle (true for debugging mode, false for execution mode)
POINT cursor; // point object corresponding to mouse position

double degreesToRadians(double degrees) {
	return degrees * (M_PI / 180);
}

vec3f normalize(vec3f inputvec) {
	vec3f normvec;
	double length = sqrt(inputvec.x * inputvec.x + inputvec.y * inputvec.y + inputvec.z * inputvec.z);
	normvec.x = inputvec.x / length;
	normvec.y = inputvec.y / length;
	normvec.z = inputvec.z / length;
	return normvec;
}

// Controls movement throughout the scene. uses the arrow keys to control the x-z plane movmements and the space 
// and left alt keys to control the y-axis movements.
// Click and drag with the left mouse button to change the view angle along the xz plane.

void move(vec3f &viewpos, double &viewangle, POINT &cursor) {

	// bools controlling movements
	bool	moveforward = GetAsyncKeyState(VK_UP) || GetAsyncKeyState(0x57); // move forward in the scene using the UP arrow key or W
	bool	moveback = GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(0x53); // move backward in the scene using the DOWN arrow key or S
	bool	moveright = GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState(0x44); // move right in the scene using  the RIGHT arrow key or A
	bool	moveleft = GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(0x41); // move left in the scene using the LEFT arrow key or D
	bool	moveup = GetAsyncKeyState(VK_SPACE); // move upwards in the scene using the SPACE key
	bool	movedown = GetAsyncKeyState(VK_LMENU); // move downwards in the scene using the SHIFT key
	bool	changeview = GetAsyncKeyState(VK_LBUTTON); //while left mouse button is pressed, you can change the view.

	if (moveforward && moveback) moveforward = moveback = false;
	if (moveleft && moveright) moveleft = moveright = false;
	if (moveup && movedown) moveup = movedown = false; // moving in opposite directions stops movement

	// apply mouse movements
	if (changeview) {
		if (!initializedView) {
			viewoffsetx = cursor.x; //map the current view direction to the current mouse pos.
			initialView = viewangle;
			initializedView = true;
		}
		viewangle = fmod(initialView - ((double(cursor.x) - viewoffsetx) * MOUSESENSITIVITY), 360);
	}
	if (!changeview) initializedView = false; //next time the user presses left mouse, we map the current view direction to the current mouse pos

	// create direction vector:
	vec3f forevec = normalize(vec3f(sin(degreesToRadians(viewangle)), 0, cos(degreesToRadians(viewangle)))); //forward movement
	vec3f leftvec = normalize(vec3f(forevec.z, 0, -forevec.x)); //left movement

	vec3f movevec = vec3f(0.0, 0.0, 0.0);

	// apply keyboard movements
	if (moveforward) { // made it more complicated to see if I could get movement to be the same speed in all directions
		if (moveleft) {
			movevec = normalize(forevec + leftvec);
			moveleft = false;
		}
		else if (moveright) {
			movevec = normalize(forevec - leftvec); // -leftvec == rightvec
			moveright = false;
		}
		else {
			movevec = forevec; // already normalized
		}
		moveforward = false;
	}
	if (moveback) {
		if (moveleft) {
			movevec = normalize(-forevec + leftvec); // -forevec == backvec
			moveleft = false;
		}
		else if (moveright) {
			movevec = normalize(-forevec - leftvec); // -leftvec == rightvec
			moveright = false;
		}
		else {
			movevec = -forevec; // already normalized
		}
		moveback = false;
	}
	if (moveleft && !moveforward && !moveback) { // if we are only moving left
		movevec = leftvec; // already normalized
		moveleft = false;
	}
	if (moveright && !moveforward && !moveback) { // if we are only moving right
		movevec = -leftvec; // already normalized, -leftvec == rightvec
		moveright = false;
	}
	// flight up and down will be its own thing
	if (movedown && viewpos.y < -0.005) { viewpos.y += FLYSPEED; moveup = false; } // postive y is downwards, can't move too far down																		   // camera floor at y = -.03
	if (moveup) { viewpos.y -= FLYSPEED; moveup = false; }	// negative y is upwards (no ceiling height)

	// actually move the camera in x and z
	viewpos += movevec * MOVESPEED;
}
 
// draws the scene. runs every single frame. press the ESCAPE key to end execution.
void DrawScene()
{
	if (GetAsyncKeyState(VK_ESCAPE))  exit(0); // escape key to exit tbe program (ESCAPE)

	POINT cursor; // point object corresponding to mouse position
	GetCursorPos(&cursor); // mouse pointer position
	bool	wireframe = GetAsyncKeyState(VK_LCONTROL);	// render wireframe (HOLD LEFT CONTROL)
	bool	topdown = GetAsyncKeyState(VK_RETURN);	// view top-down (HOLD ENTER)
	
	// apply keyboard and mouse movements
	move(viewpos, viewangle, cursor);

	// print player information to the command prompt every frame.
	//if (debug) cout << "PLAYERPOS | x: " << viewpos.x << ", y: " << viewpos.y << ", z: " << viewpos.z << " | MOUSEPOS | x: " << cursor.x << ", mouse y: " << cursor.y << ", viewangle: " << viewangle << endl;

	//set background to black
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//other openGL stuff
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	static int tex_heightmap = 0; // heightmap value
	static int tex_terrain = 0; // texture value

	static bool init = true;
	static Shader shader("S_1"); // create shader

	static int vbo = 0; // vertex buffer object (stores vertex data ie. triangles)
	static std::vector<float> vert; // vector of vert heights?
	//not done adding comments yet

	if (init)
	{
		// terrain heightmap - generates the heightmap
		//Bmp bmp(width, height, 32);
		//loopj(0, height) loopi(0, width)
		//{
		//	float x = float(i) / float(width); // x = i / width (gradually increasing % of width)
		//	float y = float(j) / float(height); // y = j / height (gradually increasing % of height or rather length)
		//	float h = (sin(4 * M_PI * x) + sin(4 * M_PI * y) + sin(16 * M_PI * x) * sin(16 * M_PI * y)) * 0.125 + 0.5; // calculate height based on x and y
		//	((float*)bmp.data)[i + j * width] = h; // bmp.data is the height map we are adding in.
		//}

		Bmp bmp("../Images/Ridge Through Terrain Height Map BMP.bmp"); // load the height map bmp file
		Bmp texbmp("../Images/Ridge Through Terrain BMP.bmp"); // load the texture bmp file
		width = bmp.width; //set global variables
		height = bmp.height;
		tex_heightmap = ogl_tex_bmp(bmp); //sets the heightmap
		tex_terrain = ogl_tex_bmp(texbmp); //sets the texture

		//bmp.load_float("../Images/test2.bmp"); // <-- use this for loading raw float map from file
		//tex_heightmap = ogl_tex_new(width, height, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_LUMINANCE16F_ARB, GL_LUMINANCE, bmp.data, GL_FLOAT);

		// terrain texture
		//loopj(0, height)	loopi(0, width) loopk(0, 3)
		//{
		//	bmp.data[(i + j * width) * 3 + k] = i ^ j ^ (k * 192);
		//}
		//tex_terrain = ogl_tex_new(width, height, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_RGB, GL_RGB, bmp.data, GL_UNSIGNED_BYTE);

		// driver info
		std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;			//std::cout << "GL_EXTENSIONS: " << glGetString(GL_EXTENSIONS) << std::endl;
		std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
		std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "GLU_VERSION: " << gluGetString(GLU_VERSION) << std::endl;			//std::cout << "GLU_EXTENSIONS: " << gluGetString(GLU_EXTENSIONS) << std::endl;
		std::cout << "GLUT_API_VERSION: " << GLUT_API_VERSION << std::endl;

		// load shaders
		shader.attach(GL_VERTEX_SHADER, "../shader/vs.txt");
		shader.attach(GL_FRAGMENT_SHADER, "../shader/frag.txt");
		shader.link();

		// make vbo quad patch
		loopj(0, grid + 1) // for loop, 0 < j < grid + 1, j++
			loopi(0, grid + 2) // for loop, 0 < i < grid + 1, i++
		{
			loopk(0, ((i == 0) ? 2 : 1)) // for loop, 0 < k < i [if (i==0) then i = 2, otherwise i = 1], k++
			{
				vert.push_back(float(i) / grid); // i / grid (% of grid)
				vert.push_back(float(j) / grid); // j / grid (% of grid)
				vert.push_back(0);
			}
			++j;
			loopk(0, ((i == grid + 1) ? 2 : 1)) // for loop, 0 < k < i [if (i == grid +1) then i = 2, otherwise i = 1], k++
			{
				vert.push_back(float(i) / grid); // i / grid (% of grid)
				vert.push_back(float(j) / grid); // j / grid (% of grid)
				vert.push_back(0);
			}
			--j;
		}

		//openGL stuff
		glGenBuffers(1, (GLuint*)(&vbo));
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert.size(), &vert[0], GL_DYNAMIC_DRAW_ARB);

		init = false; //initialization complete
	}

	//openGL stuff
	glMatrixMode( GL_PROJECTION);
	glLoadIdentity();

	// if top down view has been activated
	if (topdown)
	{
		glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); // aw an orthographic projection of the scene
		glRotatef(180, 1, 0, 0); // rotate the camera by 180 degrees
		wireframe ^= 1; // ??
	}
	else		 
	{
		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		gluPerspective(90.0,float(vp[2])/float(vp[3]) , 0.0001, 1.0);
		glTranslatef(0, viewpos.y, 0);	// set height
		glRotatef(130, 1, 0, 0); // rotate 130 degreessa	
		glRotatef(viewangle, 0, 0, 1); // set rotation
	}

	matrix44 mat;
	glGetFloatv(GL_PROJECTION_MATRIX, &mat.m[0][0]);		CHECK_GL_ERROR();
	
	// Enable VBO
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);				CHECK_GL_ERROR();
	glEnableClientState(GL_VERTEX_ARRAY);					CHECK_GL_ERROR();
	glVertexPointer  ( 3, GL_FLOAT,0, (char *) 0);			CHECK_GL_ERROR();

	//openGL stuff
	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB( GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, tex_heightmap);
	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB( GL_TEXTURE1 );
	glBindTexture(GL_TEXTURE_2D, tex_terrain);

	// Triangle Mesh
	shader.begin();
	shader.setUniform1i("tex_heightmap",0);
	shader.setUniform1i("tex_terrain",1);

	float sxy=2; // scale x/y
	shader.setUniform4f("map_position", 
		-viewpos.x/float(2*512*grid),
		-viewpos.z/float(2*512*grid), 0, 0);

	shader.setUniform1f("heightscale", VERTICALEXAGGERATION); //vertical exaggeration

	loopi(0,levels) //for loop, i = 0; i < levels; ++i
	{
		float ox = (int(viewpos.x * (1 << i)) & 511) / float(512 * grid); // offsetx
		float oy = (int(viewpos.z * (1 << i)) & 511) / float(512 * grid); // offsety

		vec3f scale	(sxy*0.25,sxy*0.25,1);
		shader.setUniform4f("scale" , scale.x,scale.y,1,1);	

		loopk(-2,2) loopj(-2,2) // each level has 4x4 patches
		{
			if(i!=levels-1) if(k==-1||k==0) if(j==-1||j==0) continue;

			vec3f offset(ox+float(j),oy+float(k),0);
			if(k>=0) offset.y-=1.0/float(grid); // adjust offset for proper overlapping
			if(j>=0) offset.x-=1.0/float(grid); // adjust offset for proper overlapping

			//cull
			int xp=0,xm=0,yp=0,ym=0,zp=0;
			looplmn(0,0,0,2,2,2)
			{
				vec3f v = scale*(offset+vec3f(l,m,float(-n)*0.05)); // bbox vector
				vec4f cs = mat * vec4f(v.x,v.y,v.z,1); // clipspace
				if(cs.z< cs.w) zp++;				
				if(cs.x<-cs.w) xm++;	if(cs.x>cs.w) xp++;
				if(cs.y<-cs.w) ym++;	if(cs.y>cs.w) yp++;
			}
			if(zp==0 || xm==8 || xp==8 || ym==8 || yp==8)continue; // skip if invisible
			
			//render
			shader.setUniform4f("offset", offset.x,offset.y,0,0);
			if(wireframe)	glDrawArrays( GL_LINES, 0, vert.size()/3);
			else			glDrawArrays( GL_TRIANGLE_STRIP, 0, vert.size()/3);
		}
		sxy*=0.5;
	}	
	shader.end();

	// Disable VBO
	glDisableClientState(GL_VERTEX_ARRAY);									CHECK_GL_ERROR();
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);								CHECK_GL_ERROR();
	glutSwapBuffers();
}

int main(int argc, char **argv) 
{ 
  glutInit(&argc, argv);  // glutInit() using command-line arguments

  // setting up window
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);  // set display mode
  glutInitWindowSize(1024, 512); // set window size
  glutInitWindowPosition(0, 0);  // set window position
  glutCreateWindow("Geometry Clipmaps Project");

  // displaying scene
  glutDisplayFunc(DrawScene);
  glutIdleFunc(DrawScene);
  glewInit();
  wglSwapIntervalEXT(0);
  glutMainLoop();
  return 0;
}