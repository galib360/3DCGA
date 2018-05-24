/** \file traqueboule.h
 //DO NOT LOOK at this code... It is here to steer the mouse movement, but you do not need to understand it!
*/
#ifndef TRAQUEBOULE
#define TRAQUEBOULE
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <math.h>
#include "matrix.h"
#include "stdio.h"
static const float speedfact = 0.2;

void display();

GLdouble tb_matrix[16] =   { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0,
                             0,0,0,1  };
GLdouble tb_inverse[16] =  { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0,
                             0,0,0,1  };

int tb_ancienX, tb_ancienY, tb_tournerXY=0, tb_translaterXY=0, tb_bougerZ=0;


void tbInitTransform()
{
    glGetDoublev( GL_MODELVIEW_MATRIX, tb_matrix );
    inverse( tb_matrix, tb_inverse );
}

void tbVisuTransform()
{
    glMultMatrixd( tb_matrix );
};

void tbHelp()
{
    printf("\nLeft Button = Turn,\n");
    printf("Right Button = Translate,\n");
    printf("Middle Button = Turn/advance along Z.\n");
}

void tbMouseFunc( int button, int state, int x, int y )
{
    if( button==GLUT_LEFT_BUTTON && state==GLUT_DOWN )
    {
        tb_tournerXY = 1;
        tb_ancienX = x;
        tb_ancienY = y;
    }
    else if( button==GLUT_LEFT_BUTTON && state==GLUT_UP )
    {
        tb_tournerXY = 0;
    }
    if( button==GLUT_MIDDLE_BUTTON && state==GLUT_DOWN )
    {
        tb_bougerZ = 1;
        tb_ancienX = x;
        tb_ancienY = y;
    }
    else if( button==GLUT_MIDDLE_BUTTON && state==GLUT_UP )
    {
        tb_bougerZ = 0;
    }
    else if( button==GLUT_RIGHT_BUTTON && state==GLUT_DOWN )
    {
        tb_translaterXY = 1;
        tb_ancienX = x;
        tb_ancienY = y;
    }
    else if( button==GLUT_RIGHT_BUTTON && state==GLUT_UP )
    {
        tb_translaterXY = 0;
    }
}


void tbMotionFunc( int x, int y )
{
    double dx,dy,nrm, tx,ty,tz;

    if( tb_tournerXY || tb_translaterXY || tb_bougerZ )
    {
        dx = x - tb_ancienX;
        dy = tb_ancienY - y; 

        if( tb_tournerXY )
        {
            tx = tb_matrix[12];
            tb_matrix[12]=0;
            ty = tb_matrix[13];
            tb_matrix[13]=0;
            tz = tb_matrix[14];
            tb_matrix[14]=0;

            nrm = sqrt( dx*dx+dy*dy+dx*dx+dy*dy )*speedfact;
            glLoadIdentity();
            glRotatef( nrm, -dy, dx, 0 );
            glMultMatrixd( tb_matrix );
            glGetDoublev( GL_MODELVIEW_MATRIX, tb_matrix );

            tb_matrix[12] = tx;
            tb_matrix[13] = ty;
            tb_matrix[14] = tz;
        }
        else if( tb_translaterXY )
        {
            tb_matrix[12] += dx/100.0*speedfact;
            tb_matrix[13] += dy/100.0*speedfact;
        }
        else if( fabs(dx)>fabs(dy) )
        { // rotation z
            tx = tb_matrix[12];
            tb_matrix[12]=0;
            ty = tb_matrix[13];
            tb_matrix[13]=0;
            tz = tb_matrix[14];
            tb_matrix[14]=0;

            glLoadIdentity();
            glRotatef( dx, 0,0,-1 );
            glMultMatrixd( tb_matrix );
            glGetDoublev( GL_MODELVIEW_MATRIX, tb_matrix );

            tb_matrix[12] = tx;
            tb_matrix[13] = ty;
            tb_matrix[14] = tz;
        }
        else if( fabs(dy)>fabs(dx) )
        {
            tb_matrix[14] -= dy/100.0*speedfact;
        }
        tb_ancienX = x;
        tb_ancienY = y;
        inverse( tb_matrix, tb_inverse );
        glutPostRedisplay();
    }
}

void tbRotate( double angle, double x, double y, double z )
{
    double tx,ty,tz;

    tx = tb_matrix[12];
    tb_matrix[12]=0;
    ty = tb_matrix[13];
    tb_matrix[13]=0;
    tz = tb_matrix[14];
    tb_matrix[14]=0;

    glLoadIdentity();
    glRotatef( angle, x, y, z );
    glMultMatrixd( tb_matrix );
    glGetDoublev( GL_MODELVIEW_MATRIX, tb_matrix );

    tb_matrix[12] = tx;
    tb_matrix[13] = ty;
    tb_matrix[14] = tz;

    inverse( tb_matrix, tb_inverse );
    glutPostRedisplay();
}

void tbProject( const GLdouble *m, const GLdouble* p, GLdouble* q )
{
    double pp[4];
    project( m, p, pp );
    project( tb_inverse, pp, q );
}
#endif
