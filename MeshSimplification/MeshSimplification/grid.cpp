#include "grid.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>


int Grid::isContainedAt(const Vec3Df & pos){
    //returns index that contains the position
	
	float cubeLength = size / r;

	Vec3Df v = pos - origin;
	int x = v[0] / cubeLength;
	int y = v[1] / cubeLength;
	int z = v[2] / cubeLength;

	int indice = x + r * y + r * r*z;
	return indice;
}

void Grid::addToCell(const Vec3Df & vertexPos) {
	int nr = isContainedAt(vertexPos);
	std::vector<Vec3Df> list = verticesInCell[nr];
	list.push_back(vertexPos);
	verticesInCell[nr] = list;
}

void Grid::putVertices(const std::vector<Vertex> & vertices){
    //put vertices in the corresponding voxels.
	for (int i = 0; i < vertices.size(); i++) {
		addToCell(vertices[i].p);
	}
}

void Grid::computeRepresentatives() {
	for (int i = 0; i < r*r*r; i++) {
		std::vector<Vec3Df> list = verticesInCell[i];
		if (list.size() > 0) {
			Vec3Df p = Vec3Df(0, 0, 0);
			Vec3Df n = Vec3Df(0, 0, 0);
			for (std::vector<Vec3Df>::iterator it = list.begin(); it != list.end(); ++it) {
				p = p + *it;
			}
			p = p / list.size();
			representatives[i] = Vertex(p, n);
		}
	}
}

void Grid::drawCell(const Vec3Df & Min,const Vec3Df& Max) {

    const Vec3Df corners[8] =
    {
        Vec3Df(Min[0],Min[1],Min[2]),
        Vec3Df(Max[0],Min[1],Min[2]),
        Vec3Df(Min[0],Max[1],Min[2]),
        Vec3Df(Max[0],Max[1],Min[2]),
        Vec3Df(Min[0],Min[1],Max[2]),
        Vec3Df(Max[0],Min[1],Max[2]),
        Vec3Df(Min[0],Max[1],Max[2]),
        Vec3Df(Max[0],Max[1],Max[2])
    };


    static const unsigned short faceCorners[6][4] =
    {
        { 0,4,6,2 },
        { 5,1,3,7 },
        { 0,1,5,4 },
        { 3,2,6,7 },
        { 0,2,3,1 },
        { 6,4,5,7 }
    };

    glBegin(GL_QUADS);
    for (unsigned short f=0;f<6;++f)
    {
        const unsigned short* face = faceCorners[f];
		for (unsigned int v = 0; v < 4; v++) {
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(corners[face[v]][0], corners[face[v]][1], corners[face[v]][2]);
		}
    }
    glEnd();

}

void Grid::drawGrid(){
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glLineWidth(1.0f);
    glColor3f(1.0f,0.0f,0.0f);
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    //Complete this function by calling draw cell several times.
	float cubeLength = size / r;

	for (int z = 0; z < r; z++) {
		for (int y = 0; y < r; y++) {
			for (int x = 0; x < r; x++) {
				int nr = x + r * y + r * r*z;
				if (verticesInCell[nr].size() > 0) {
					float xmin = origin[0] + x * cubeLength;
					float ymin = origin[1] + y * cubeLength;
					float zmin = origin[2] + z * cubeLength;

					float xmax = origin[0] + (x + 1) * cubeLength;
					float ymax = origin[1] + (y + 1) * cubeLength;
					float zmax = origin[2] + (z + 1) * cubeLength;

					Vec3Df min = Vec3Df();
					min[0] = xmin;
					min[1] = ymin;
					min[2] = zmin;

					Vec3Df max = Vec3Df();
					max[0] = xmax;
					max[1] = ymax;
					max[2] = zmax;

					drawCell(min, max);
				}
			}
		}
	}

    glPopAttrib();
}



