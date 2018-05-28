#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "traqueboule.h"
#include "mesh.h"
#include "grid.h"

typedef enum {Wireframe, Flat, Gouraud} PolygonMode;
static PolygonMode polygonMode = Gouraud;

using namespace std;

Mesh mesh; //Main mesh
Mesh simplified; //simplified mesh - to be built by you
Grid grid; //voxel grid to be filled by you
//Number of cells along an axis in the grid 
unsigned int r = 30;

bool displayGrid = false;
bool simplifiedDisplay = false;

// Different rendering modes, you can add the ones you need
enum { TRIANGLE=0, SIMPLE=1};
unsigned int type = TRIANGLE;

unsigned int W_fen = 800;  // window width
unsigned int H_fen = 800;  // window height


/************************************************************
 * Initialization function; here we load the mesh and you can add additional precomputations
 ********************* 
/************************************************************
 * Simplification Function !! TO BE COMPLETED
 ************************************************************/



/************************************************************
 * Function calls for drawing operations
 ************************************************************/
void drawAxis(float length)
{
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(length,0,0);

    glColor3f(0,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,length,0);

    glColor3f(0,0,1);
    glVertex3f(0,0,0);
    glVertex3f(0,0,length);
    glEnd();
    glEnable(GL_LIGHTING);

}


void mainDraw( )
{
    /*
    RepresentativeList & representatives = grid.representants;
    glDisable(GL_LIGHTING);
    glPointSize(5.);
    glBegin(GL_POINTS);


    for(RepresentantList::iterator it = representatives.begin() ; it != representatives.end (); it++){
        Vec3Df v = (*it).second.p;
        glVertex3f(v[0], v[1], v[2]);
    }
    glEnd();
    glEnable(GL_LIGHTING);
*/

    if (polygonMode != Gouraud)
        simplifiedDisplay? simplified.draw() : mesh.draw();
    else
        simplifiedDisplay? simplified.drawSmooth() : mesh.drawSmooth();

	if (displayGrid) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clean the screen and the depth buffer
		//glLoadIdentity();
		simplified.drawSmooth();
		grid.drawGrid();
	}

    return;

    switch( type )
    {
    case TRIANGLE:
        drawAxis(1);
        break;
    default:
        drawAxis(1);
        break;
    }
}

void animate()
{

}

void init(const char * fileName) {
	mesh.loadMesh(fileName);
	//attention! while loadMesh calls compute bounding box, it is not yet implemented!

}

void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);

void printUsage () {
    cerr << endl
         << "Practical" << endl
         << "for 3D graphics." << endl
         << "Usage : ./test [<file.obj>]" << endl
         << "Keyboard commands" << endl
         << "------------------" << endl
         << " ?: Print help" << endl
         << " w: Toggle Wireframe/Flat/Gouraud Rendering Mode" << endl
         << " <drag>+<left button>: rotate model" << endl
         << " <drag>+<right button>: move model" << endl
         << " <drag>+<middle button>: zoom" << endl
         << " q, <esc>: Quit" << endl << endl;
}

/************************************************************
 * Main Program
 ************************************************************/
int main(int argc, char** argv)
{

    if (argc > 2) {
        printUsage ();
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);

    init(argc == 2 ? argv[1] : "Boss_high.obj");

    // setup framebuffer
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );

    // define window position and size for the application
    glutInitWindowPosition(200, 100);
    glutInitWindowSize(W_fen,H_fen);
    glutCreateWindow(argv[0]);	

    //init trackball for mouse mouvement
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0,0,-4);
    tbInitTransform();     // Trackball
    tbHelp();              // show help message

    //
    // Let there be LIGHT :) 

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glEnable(GL_COLOR_MATERIAL);
    int LightPos[4] = {0,0,3,1};
    int MatSpec [4] = {1,1,1,1};
    glLightiv(GL_LIGHT0,GL_POSITION,LightPos);
    glMaterialiv(GL_FRONT_AND_BACK,GL_SPECULAR,MatSpec);
    glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,10);
    glEnable(GL_NORMALIZE);
    

    // callbacks
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMouseFunc(tbMouseFunc);    // trackball uses mouse
    glutMotionFunc(tbMotionFunc);  // trackball uses mouse
    glutIdleFunc( animate);

    // main loop
    glutMainLoop();

    return 0;  // never reached
}


/************************************************************
 * Functions for OpenGL no need to touch the code below!
 ************************************************************/
// draw 
void display(void)
{
    // Setup OpengGL for each frame
    glEnable( GL_DEPTH_TEST );            // depth test

    if(polygonMode == Wireframe){
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    }

    glShadeModel(GL_SMOOTH);

    // Effacer tout
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT); //clear color/value
    

    glLoadIdentity();  // init camera

    tbVisuTransform(); // trackball

    mainDraw( );    

    glutSwapBuffers();
}

// Window size changed
void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective (50, (float)w/h, 1, 10);
    glMatrixMode(GL_MODELVIEW);
}

// keyboard input
void keyboard(unsigned char key, int x, int y)
{
    printf("key %d pressed at %d,%d\n",key,x,y);
    fflush(stdout);
    switch (key)
    {
    case 'a':     
        ++type;
        if (type==5)
            type=0;
        break;
    case 'w':
        if (polygonMode == Flat) {
            polygonMode = Wireframe;
        } else if (polygonMode == Wireframe) {
            polygonMode = Gouraud;
        } else {
            polygonMode = Flat;
        }
        break;
	case 'g':
		if (displayGrid)
			displayGrid = FALSE;
		else
		{
			displayGrid = TRUE;
			simplified = grid.simplifyMesh(mesh, r);
			
		}
		break;
    case 27:     // ESC
        exit(0);
    }
    display();
}

