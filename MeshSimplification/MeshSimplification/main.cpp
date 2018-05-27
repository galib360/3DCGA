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
unsigned int r = 5;

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
void simplifyMesh(unsigned int r){
 //Create a grid that covers the bounding box of the mesh. 
 //Be thorough and check all functions, as some of the calls below might NOT directly work and need to be written by you.
 //It should be considered a guideline, NOT the solution.
 //Also, use your graphics knowledge to draw for debugging! (e.g., draw the bounding box, the grid etc.)  
 double offset = 0.01;	
 Vec3Df vecOffset = Vec3Df(offset, offset, offset);
 grid = Grid(mesh.bbOrigin-vecOffset, mesh.bbEdgeSize + 2*offset, r );
 
 //work with a local reference on the vertices and triangles
 const vector<Vertex> & vertices = mesh.vertices;
 const vector<Triangle> & triangles = mesh.triangles;

 //   //Put all the vertices in the grid
 grid.putVertices(vertices);

 //	  //calculate a representative vertex for each grid cell
 grid.computeRepresentatives();

 // //Create a new list of vertices for the simplified model
 // //What is the effect of the code below?
 std::map<unsigned int, unsigned int > newIndexRemapping;
 vector<Vertex> simplifiedVertices;

    int count = 0;
    for(RepresentativeList::iterator it = grid.representatives.begin() ; it != grid.representatives.end (); it++, count++){
        newIndexRemapping[(*it).first] = count;
        simplifiedVertices.push_back((*it).second);
    }


 // //Create a new list of triangles
 // //This is NOT COMPLETE and you need to add code here
 // //Think about what simplifiedVertices and newIndexRemapping contain
 vector<Triangle> simplifiedTriangles;
 for (int i = 0; i < triangles.size(); i++) {
	 Triangle tr = triangles[i];
	 int indice1 = grid.isContainedAt(vertices[tr.v[0]].p);
	 int indice2 = grid.isContainedAt(vertices[tr.v[1]].p);
	 int indice3 = grid.isContainedAt(vertices[tr.v[2]].p);

	 if (indice1 == indice2 && indice1 == indice3) {
		 continue;
	 }

	 Triangle trSimple = Triangle(newIndexRemapping[indice1],
		 newIndexRemapping[indice2],
		 newIndexRemapping[indice3]);
	 simplifiedTriangles.push_back(trSimple);
 }

 // //Build the simplified mesh from the CORRECT lists
 simplified = Mesh(simplifiedVertices , simplifiedTriangles);

 // //recalculate the normals.
 simplified.centerAndScaleToUnit();
 simplified.computeVertexNormals();
 simplified.computeBoundingCube();
}


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

    if (displayGrid)
        grid.drawGrid();

	switch (type)
	{
	case TRIANGLE:
		//drawAxis(1);
		simplified.draw();
		
		break;
	default:
		drawAxis(1);
		break;
	}
	return;

    
}
void init(const char * fileName) {
	mesh.loadMesh(fileName);
	//attention! while loadMesh calls compute bounding box, it is not yet implemented!

}


void animate()
{

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

    init(argc == 2 ? argv[1] : "dodge.obj");

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
		simplifyMesh(r);
		grid.drawGrid();
		break;
	case 'h':
		simplifyMesh(r);
		
		simplified.draw();

		break;
    case 27:     // ESC
        exit(0);
    }
    display();
}

