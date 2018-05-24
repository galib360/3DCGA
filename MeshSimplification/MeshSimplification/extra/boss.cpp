#include "boss.h"

#if defined(_WIN32)
#include <GL/glut.h>
#elif defined (__APPLE__)
#include <GLUT/glut.h>
#endif


template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}


void Leg::drawPaw()
{
	glPushMatrix();
	glTranslatef(0, height / 4.0, 0);
	glBegin(GL_TRIANGLES);
	glVertex3f(0, 0, 0);
	glVertex3f(-pawWidth, -pawHeight, 0);
	glVertex3f(0, -pawHeight, pawDepth);

	glVertex3f(0, 0, 0);
	glVertex3f(0, -pawHeight, pawDepth);
	glVertex3f(pawWidth, -pawHeight, 0);

	glVertex3f(0, 0, 0);
	glVertex3f(pawWidth, -pawHeight, 0);
	glVertex3f(-pawWidth, -pawHeight, 0);

	glVertex3f(0, -2 * pawHeight, 0);
	glVertex3f(0, -pawHeight, pawDepth);
	glVertex3f(-pawWidth, -pawHeight, 0);

	glVertex3f(0, -2 * pawHeight, 0);
	glVertex3f(pawWidth, -pawHeight, 0);
	glVertex3f(0, -pawHeight, pawDepth);

	glVertex3f(0, -2 * pawHeight, 0);
	glVertex3f(-pawWidth, -pawHeight, 0);
	glVertex3f(pawWidth, -pawHeight, 0);

	glEnd();
	glPopMatrix();
}

Leg::Leg(float height, Vec3Df hip, Vec3Df toe)
{
	this->height = height;
	this->hip = hip;
	toe_base = toe;
	this->moveToe(toe);
	pawHeight = height / 2.0f;
	pawWidth = height / 4.0f;
	pawDepth = height / 4.0f;
}

Vec3Df Leg::getToeBase()
{
	return toe_base;
}

void Leg::init()
{
	//The vector from the toe to the hip
	Vec3Df delta = toe - hip;
	//The angle for the direction of the leg
	angle_y = atan2f(delta[0], delta[2]) * 180.0f / M_PI;
	//The distance between the toe and the hip
	float length_l = Vec3Df::distance(toe, hip);
	//The angle between delta (line from hip to toe) and the leg, set to 0 if the toe is further away than the length of the leg
	angle_alpha = 0;
	if (length_l < 1.5*height)
		angle_alpha = acos(0.5f*length_l / (0.75f*height)) * 180.0f / M_PI;
	//The angle between delta and the x,z plane
	float angle_beta = atan2(delta[1], sqrtf(delta[0] * delta[0] + delta[2] * delta[2])) * 180.0f / M_PI;
	//The angle of upwards rotation for the leg
	angle_x = -(angle_alpha + angle_beta);
}

void Leg::drawLeg()
{
	//Draw the toe sphere
	/*
	glPushMatrix();
		glColor3f(0.0, 1.0, 0.0);
		glTranslatef(toe[0], toe[1], toe[2]);
		glutSolidSphere(0.1, 16, 16);
	glPopMatrix();
	*/
	//Draw the hip joint
	glPushMatrix();
		glColor3f(1.0, 0.0, 0.0);
		glTranslatef(hip[0], hip[1], hip[2]);
		glutSolidSphere(0.1, 16, 16);

		//Draw the leg
		glPushMatrix();
			glColor3f(1, 0.5, 0);
			glRotatef(angle_y, 0, 1, 0);
			glRotatef(angle_x, 1, 0, 0);
			gluCylinder(gluNewQuadric(), height / 16.0, height / 16.0, 0.75*height, 8, 1);
			glTranslatef(0, 0, 0.75*height);
			//Draw the Paw
			glRotatef(2 * angle_alpha - 90, 1, 0, 0);
			drawPaw();
		glPopMatrix();
	glPopMatrix();
}





Boss::Boss(Vec3Df pos, float speed, float scale)
{
	position = pos;
	destination = pos;
	slices = 6.0;
	body_height = 0.375;
	body_radius = 0.5;
	center = Vec3Df(0, 0.625, 0);
	this->scale = scale;

	leg_size = 1.0;
	//the length of the stretched leg is leg_size * toe_distance_factor
	toe_distance_factor = 1.5;
	//part of the leg's reach used for the walking cycle
	step_size_factor = 0.5;
	base_speed = speed;
	walking_speed = base_speed;

	printf("Loading Mesh\n");
	Mesh mesh = Mesh();
	mesh.loadMesh("./Models/hoofdLowRes2.obj");
	meshes.push_back(mesh);
	printf("Creating Grid, 24\n");
	meshes.push_back(Grid::getReduxMesh(mesh, 24));
	printf("Creating Grid, 20\n");
	meshes.push_back(Grid::getReduxMesh(mesh, 20));
	printf("Creating Grid, 16\n");
	meshes.push_back(Grid::getReduxMesh(mesh, 16));
	printf("Creating Grid, 12\n");
	meshes.push_back(Grid::getReduxMesh(mesh, 12));
	printf("Creating Grid, 8\n");
	meshes.push_back(Grid::getReduxMesh(mesh, 8));
	printf("Creating Grid, 4\n");
	meshes.push_back(Grid::getReduxMesh(mesh, 4));

	translation = Vec3Df(0, body_height + meshes[meshIndex].bbEdgeSize, 0);

	init();
	nextMove(move);
}

void Boss::animate(float delta)
{
	cycle += delta/(1000.0f*cycle_duration);
	float hor_speed = 0;
	//Move the boss
	if (position != destination)
	{
		if (Vec3Df::distance(position, destination) > (delta*movement_vec / 1000.0).getLength())
		{
			position += delta*movement_vec / 1000.0;
		}
		else
		{
			position = destination;
			setWalkingSpeed(base_speed);
			nextMove(move);
		}
	}
	//Increment the walking cycle
	if (cycle > 1)
	{
		odd_cycle = !odd_cycle;
		cycle -= 1;
	}
	//Update the legs
	if (walking_speed != 0.0)
	{
		for (int i = 0; i < 6; i++)
		{
			int sign = sgn(walking_speed);
			Vec3Df toe = legs[i].getToeBase();
			//Decide if the leg is on the ground or in the air
			if ((i + odd_cycle) % 2)
				toe[0] += sign*(-step_size / 2.0f + step_size * cycle);
			else
			{
				float delta_x = sign*(step_size / 2.0f - step_size * cycle);
				toe[0] += delta_x;

				float arc_angle = 135.0f * M_PI / 180.0f;
				float step_circle_radius = ((step_size / 2.0f) / sin(arc_angle / 2.0f));
				toe[1] += sqrt(step_circle_radius*step_circle_radius - delta_x*delta_x) - step_circle_radius*sin(M_PI_2 - arc_angle / 2.0f);
			}
			legs[i].moveToe(toe);
		}
	}
	//Update the linger cycle
	if (linger_time > 0)
    {
		linger_time -= delta;
        if (linger_time == 0)
            linger_time = -1; //dirty, but works
    }
	if (linger_time < 0)
	{
		linger_time = 0;
		nextMove(move);
	}

}

void Boss::drawBody()
{
	//Draw a cylinder for the body
	glPushMatrix();
		//Move body to center
		glTranslatef(center[0], center[1], center[2]);
		glPushMatrix();
			glColor3f(1.0, 1.0, 0.5);
			//gluCylinder draws along the Z-axis, so we rotate to make Z point up
			glRotatef(-90, 1, 0, 0);
			//gluCylinder puts a corner at the axis, we want the middle of an edge
			glRotatef(180 / slices, 0, 0, 1);
			//Move to the bottom of the body and draw it
			glTranslatef(0, 0, -body_height / 2.0);
			gluCylinder(gluNewQuadric(), 0.5, 0, 0, slices, 1);
			//Move to the top of the straight part and draw it
			gluCylinder(gluNewQuadric(), 0.5, 0.5, body_height, slices, 1);
			//Move to the top of the sloped part and draw it plus the top
			glTranslatef(0, 0, body_height);
			gluCylinder(gluNewQuadric(), 0.5, 0.25, body_height/2.0, slices, 1);
			glTranslatef(0, 0, body_height / 2.0);
			gluCylinder(gluNewQuadric(), 0.25, 0, 0, slices, 1);
		glPopMatrix();
	glPopMatrix();
}

void Boss::drawHead()
{
	glPushMatrix();
		glTranslatef(0, body_height + meshes[meshIndex].bbEdgeSize, 0);
		//float head_radius = 1.5*body_radius;
		////Draw the eye
		//glTranslatef(center[0], 1.5*body_height + head_radius + center[1], center[2]);
		if (target != NULL)
		{
			//The vector from the player to the boss head
			Vec3Df delta = *target - (position + scale*(center + Vec3Df(0, body_height / 2.0f + meshes[meshIndex].bbEdgeSize, 0)));
			//The angles to aim at the player
			float angle_y = -atan2f(delta[2], delta[0]) * 180.0f / M_PI;
			float angle_z = atan2(delta[1], sqrtf(delta[0] * delta[0] + delta[2] * delta[2])) * 180.0f / M_PI;
			angleHeadY = angle_y +90.0f;
			angleHeadZ = -angle_z;
			
			glRotatef(angleHeadY, 0, 1, 0);
			glRotatef(angleHeadZ, 1, 0, 0);

			
		}
		//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		meshes[meshIndex].drawSmooth();
		//glColor3f(1, 1, 1);
		//gluSphere(gluNewQuadric(), head_radius, 16, 16);
		////Draw the iris
		//glTranslatef(0.5*head_radius, 0, 0);
		//glColor3f(0, 0.2, 1);
		//gluSphere(gluNewQuadric(), head_radius*0.6, 16, 16);
		////Draw the pupil
		//glTranslatef(0.5*head_radius, 0, 0);
		//glColor3f(0, 0, 0);
		//gluSphere(gluNewQuadric(), head_radius*0.2, 16, 16);
	glPopMatrix();
}

void Boss::draw()
{
	glPushMatrix();
	
	glTranslatef(position[0], position[1], position[2]);
		glScalef(scale, scale, scale);
		glEnable(GL_LIGHTING);
		drawBody();
		glDisable(GL_LIGHTING);

		//Draw the head
		drawHead();

		//Draw the legs
		glEnable(GL_LIGHTING);
		for (int i = 0; i < 6; i++)
			legs[i].drawLeg();

		glDisable(GL_LIGHTING);
		
	glPopMatrix();
}

void Boss::init()
{	
	body_elevation = center[1];
	hip_radius = body_radius*cos(M_PI / slices);
	toe_distance = toe_distance_factor*hip_radius;
	step_size = sqrtf(2.25*leg_size*leg_size - toe_distance*toe_distance - body_elevation*body_elevation) * step_size_factor;
	cycle_duration = step_size / abs(walking_speed);

	//Make the legs
	for (int i = 0; i < 6; i++)
	{
		Vec3Df hip = Vec3Df(center[0] + hip_radius*sin(2.0f * i*M_PI / slices), center[1], center[2] + hip_radius*cos(2.0f * i*M_PI / slices));
		Vec3Df toe = Vec3Df(2 * toe_distance*sin(2.0f * i*M_PI / slices), 0, 2.0f * toe_distance*cos(2.0f * i*M_PI / slices));
		legs[i] = Leg(leg_size, hip, toe);
	}

	//Create the list of moves for the boss to execute during the bossfight
	//Move to the middle of the background
	move_list.push_back(std::bind(&Boss::setDestination, std::placeholders::_1, Vec3Df(0, -1, -2), 1));
	move_list.push_back(std::bind(&Boss::linger, std::placeholders::_1, 1000));
	//Quickly move to the side of the screen
	move_list.push_back(std::bind(&Boss::setDestination, std::placeholders::_1, Vec3Df(2, -1, 0), 2));

	move_list.push_back(std::bind(&Boss::linger, std::placeholders::_1, 6000));
	move_list.push_back(std::bind(&Boss::setDestination, std::placeholders::_1, Vec3Df(-2, -1, 0), 2));
	move_list.push_back(std::bind(&Boss::linger, std::placeholders::_1, 4000));
	move_list.push_back(std::bind(&Boss::setDestination, std::placeholders::_1, Vec3Df(2, -1, 0), 1));
	//glutTimerFunc(1000, this->nextMove, 2);
}

void Boss::moveBody(Vec3Df delta)
{
	center += delta; 
	init(); 
}

void Boss::setDestination(Vec3Df dest, float speed)
{
	//Move the boss to dest by adding the movement_vec every tick
	destination = dest;
	movement_vec = (destination - position);
	movement_vec.normalize();
	movement_vec *= speed;
	setWalkingSpeed(base_speed - movement_vec[0]);
	cycle_duration = step_size / abs(walking_speed);
}

void Boss::setWalkingSpeed(float speed)
{
	//flip the walking cycle if the speed changes direction
	if (sgn(speed) != sgn(walking_speed?walking_speed:1))
		cycle = 1 - cycle;
	walking_speed = speed;
	cycle_duration = step_size / abs(walking_speed);
}

void Boss::setTarget(const Vec3Df* target)
{
	this->target = target;
}

void Boss::nextMove(unsigned int n)
{
	if (n < move_list.size())
	{
		move_list[n](this);
		move++;
	}
	else 
	{
		move_list[n-4](this);
		move -= 3;
	}
}

void Boss::linger(int ms)
{
	linger_time = ms;
}

void Boss::hit() {
	int total = meshes.size();
	meshIndex++;
	if (meshIndex >= total) {
		//kill boss

		meshIndex = 0;
	}

	translation = Vec3Df(0, body_height + meshes[meshIndex].bbEdgeSize, 0);
}

float Boss::getHeadWidth() {
	return meshes[meshIndex].bbEdgeSize;
}

std::vector<Vec3Df> Boss::getBoundingBox() {
	float meshSize = getHeadWidth();
	Vec3Df center = position + Vec3Df(0, (body_height + meshes[meshIndex].bbEdgeSize) * scale, 0);

	float offset = (meshSize / 2.0f) * scale;

	Vec3Df topLeft = Vec3Df(center[0] - offset, center[1] - offset, center[2] - offset);
	Vec3Df bottomRight = Vec3Df(center[0] + offset, center[1] + offset, center[2] + offset);

	std::vector<Vec3Df> list = { topLeft, bottomRight };
	return list;
}

Mesh& Boss::getMesh() {
	return meshes[meshIndex];
}
