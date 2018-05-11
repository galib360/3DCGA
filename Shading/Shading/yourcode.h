#include <vector>
#include "mesh.h"
#include <math.h>

//THIS IS THE ONLY FILE YOU NEED TO MODIFY!!! NOTHING ELSE!!!
//Good Luck!


//Global array to store mesh material properties and algorithmic parameters
std::vector<Vec3Df> Kd;//diffuse coefficient per vertex
std::vector<Vec3Df> Ks;//specularity coefficient per vertex
std::vector<float> Shininess;//exponent for phong and blinn-phong specularities
int ToonDiscretize=4;//number of levels in toon shading
float ToonSpecularThreshold=0.49;//threshold for specularity

float kdr = 0.5;
float kdg = 0.5;
float kdb = 0.5;


//Mesh - will be filled and loaded outside.
Mesh MyMesh;


//Helper function that you can ignore!
void initStudentVariables()
{
	//this is needed so that your table has the same size as the number of vertices.
	//later, the vertex indices received in the functions correspond to the same location in your vector.
	//in other words, you can store some information per vertex here.

	Kd.resize(MyMesh.vertices.size(), Vec3Df(kdr,kdg,kdb));
	Ks.resize(MyMesh.vertices.size(), Vec3Df(0.5,0.5,0.5));
	Shininess.resize(MyMesh.vertices.size(), 3);
}

void update(float r, float g, float b) {
	Kd.resize(MyMesh.vertices.size(), Vec3Df(r, g, b));
}

//for debugging purposes or variable changes (e.g., modify the toon threshold as done below)
//please notice that some keys are already in use!
void yourKeyboardFunction(unsigned char key)
{
	cout<<"Key not used so far, so executing your code!"<<endl;
	
	//recolor the mesh 
	switch(key)
	{
		case 't': 
			ToonSpecularThreshold-=0.001;
		break;
		case 'T': 
			ToonSpecularThreshold+=0.001;
		break;
		case 'd': 
			ToonDiscretize-=1;
			if (ToonDiscretize<=0)
				ToonDiscretize=1;
		break;
		case 'D': 
			ToonDiscretize+=1;
		break;
		
		//in case you want to add colors! - Not mandatory

		case 'r': //decrase diffuse Kd coefficient in the red channel by 0.01
			kdr -= 0.05;
			if (kdr < 0) kdr = 0;
			initStudentVariables();

		break;
		case 'R': //increase diffuse Kd coefficient in the red channel by 0.01
			kdr += 0.05;
			if (kdr > 1) kdr = 1;
			initStudentVariables();
		break;
		case 'g'://same for green
			kdg -= 0.05;
			if (kdg < 0) kdg = 0;
			initStudentVariables();
		break;
		case 'G':
			kdg += 0.05;
			if (kdg > 1) kdg = 1;
			initStudentVariables();
		break;
		case 'b'://same for blue
			kdb -= 0.05;
			if (kdb < 0) kdb = 0;
			initStudentVariables();
		break;
		case 'B':
			kdb += 0.05;
			if (kdb > 1) kdb = 1;
			initStudentVariables();
		break;
	}
	
	cout<<"ToonSpecular"<<ToonSpecularThreshold<<endl;
	cout<<"Toon Discretization levels"<<ToonDiscretize<<endl;

}


//Debug function
Vec3Df debugColor(unsigned int index)
{	//this function you can use in any way you like!
	//e.g., for debugging purposes!
	return MyMesh.vertices[index].n;
	//or 
	//return Kd[index];
}


///////////////
///Shading
///////////////
//standard lambertian shading: Kd * dot(N,L), clamped to zero when negative. Where L is the light vector
//
Vec3Df diffuseOnly(const Vec3Df & vertexPos, Vec3Df & normal, const Vec3Df & lightPos, unsigned int index)
{	
	Vec3Df result =Kd[index];
	
	float cosTheta = Vec3Df::dotProduct(lightPos, normal);
	if (cosTheta < 0) {
		cosTheta = 0;
	}
	else if(cosTheta>1) {
		cosTheta = 1;
	}
	result.operator*=(cosTheta);
	
	return result;
}


//Phong (!) Shading Specularity (http://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model)
//Follow the course, only calculate Ks pow(dot(V,R),shininess), where V is the view vector and R is the Reflection vector of the light (like in pool billard computed from the LightPos, vertexPos and normal).
//When computing specularities like this, verify that the light is on the right side of the surface, with respect to the normal
//E.g., for a plane, the light source below the plane cannot cast light on the top, hence, there can also not be any specularity. 
Vec3Df phongSpecularOnly(const Vec3Df & vertexPos, Vec3Df & normal, const Vec3Df & lightPos, const Vec3Df & cameraPos, unsigned int index)
{
	Vec3Df check= lightPos;
	check.operator-=(normal);
	float checkDot = Vec3Df::dotProduct(check, normal);
	if (checkDot > 0) {
		Vec3Df result = Ks[index];
		float cosFi = Vec3Df::dotProduct(cameraPos, normal);
		if (cosFi < 0) {
			cosFi = 0;
		}
		else if (cosFi > 1) {
			cosFi = 1;
		}
		cosFi = pow(cosFi, Shininess[index]);
		result.operator*=(cosFi);

		return result;
	}
	else
	{
		return Vec3Df(0, 0, 0);
	}
}

//Blinn-Phong Shading Specularity (http://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model)
//Be careful!!! The pseudo code does some additional modifications to the formula seen in the course
//Follow the course version and calculate ONLY Ks * pow(dot(N,H), shininess). The definition of H is given on the page above and in the course.
//The same test as before should be used
Vec3Df blinnPhongSpecularOnly(const Vec3Df & vertexPos, Vec3Df & normal, const Vec3Df & lightPos, const Vec3Df & cameraPos, unsigned int index)
{
	Vec3Df check = lightPos;
	check.operator-=(normal);
	float checkDot = Vec3Df::dotProduct(check, normal);

	if (checkDot > 0) {
		Vec3Df halfVector = lightPos;
		halfVector.operator+=(cameraPos);
		halfVector.normalize();
		Vec3Df result = Ks[index];
		float cosFi = Vec3Df::dotProduct(normal, halfVector);
		if (cosFi < 0) {
			cosFi = 0;
		}
		else if (cosFi > 1) {
			cosFi = 1;
		}
		cosFi = pow(cosFi, Shininess[index]);
		result.operator*=(cosFi);

		return result;
	}
	else
	{
		return Vec3Df(0, 0, 0);
	}
	
}


///////////////
//Toon Shading
///////////////
//use the variable ToonDiscretize.
//Normal diffuse shading has values between 0 and Kd (the diffuse coefficient).
//In toon shading, these values are discretized.
//This means, there are N (ToonDiscretize) uniform intervals between 0 and Kd - in this example, we assume a single color channel, you should use the values from the vector Kd 
//Let c(0)=0, c(1) ...c(N), c(N+1)=Kd be the boundary values of these intervals.
//For a value v in [c(i), c(i+1)), the function should return (c(i)+c(i+1))/2.
//For v=Kd, return (c(N)+c(N+1))/2, else 0.
Vec3Df toonShadingNoSpecular(const Vec3Df & vertexPos, Vec3Df & normal, const Vec3Df & lightPos, unsigned int index)
{
	// get
	float f = Vec3Df::dotProduct(normal, (lightPos - vertexPos).unit());
	// normalise
	if (f < 0) f = 0;
	// interval
	float n = floorf(f * ToonDiscretize);
	float interval = 1.0 / ToonDiscretize;
	f = interval * n + (interval * 0.5f);
	return Kd[index] * f;
}

//Toon shading specularity
//The goal is to add white highlights.
//If a channel of Blinn-Phong Specularity has a value bigger or equal to ToonSpecularThreshold, then set it to 1, else to 0.
Vec3Df toonShadingOnlySpecular(const Vec3Df & vertexPos, Vec3Df & normal, const Vec3Df & lightPos, const Vec3Df & cameraPos, unsigned int index)
{
	//normalized light direction vector
	Vec3Df nld = (lightPos - vertexPos).unit();
	//make sure normal is unit vector just in case..
	Vec3Df nn = normal.unit();
	float nnDotNld = Vec3Df::dotProduct(nld, nn);
	//check whether or not the light is on the right side of the surface.
	if (nnDotNld < 0) {
		return Vec3Df(0, 0, 0);
	}
	Vec3Df h = (cameraPos - (lightPos - vertexPos)).unit();
	float dot = Vec3Df::dotProduct(h, normal.unit());

	if (dot > ToonSpecularThreshold)
		dot = 1;
	else
		dot = 0;
	return Ks[index] * dot;
}


///////////////
///INTERACTION
///////////////
Vec3Df userInteractionSphere(const Vec3Df & selectedPos, const Vec3Df & camPos)
{
	//RETURN the new light position, defined as follows.
	//selectedPos is a location on the mesh. Use this location to place the light source to cover the location as seen from camPos.
	//Further, the light should be at a distance of 1.5 from the origin of the scene - in other words, located on a sphere of radius 1.5 around the origin.
	//return Vec3Df(1,1,1);

	float r = 1.5;
	Vec3Df directionV = (selectedPos - camPos).unit();

	float a = sqrt(pow(Vec3Df::dotProduct(directionV, camPos), 2) - camPos.getSquaredLength() + r * r);
	float b = -(Vec3Df::dotProduct(directionV, camPos));
	float d1 = b + a;
	float d2 = b - a;

	float distance = (d1 > d2) ? d2 : d1;
	return (camPos + (directionV * distance));
}

Vec3Df userInteractionShadow(const Vec3Df & selectedPos, const Vec3Df & selectedNormal, const Vec3Df & lightPos)
{
	//RETURN the new light position such that the light towards the selectedPos is orthgonal to the normal at that location
	//--- in this way, the shading boundary will be exactly at this location.
	//there are several ways to do this, choose one you deem appropriate given the current light position
	//no panic, I will not judge what solution you chose, as long as the above condition is met.
	//return Vec3Df(1,0,1);

	// calculate difference
	Vec3Df v = lightPos - selectedPos;
	// calculate scalar of the normal
	float scalar = Vec3Df::dotProduct(v, selectedNormal);
	// subtract scalar times normal from the lightpos
	// to obtain a point projected on the plane orthogonal
	// to the scalar
	Vec3Df proj = lightPos - scalar * selectedNormal;

	return proj;

}

Vec3Df userInteractionSpecular(const Vec3Df & selectedPos, const Vec3Df & selectedNormal, const Vec3Df & lightPos, const Vec3Df & cameraPos)
{
	//RETURN the new light position such that a specularity (highlight) will be located at selectedPos, when viewed from cameraPos and lit from ligthPos.
	//please ensure also that the light is at a distance of 1 from selectedpos! If the camera is on the wrong side of the surface (normal pointing the other way),
	//then just return the original light position.
	//There is only ONE way of doing this!
	//return Vec3Df(0,1,1);
	Vec3Df g;
	Vec3Df n = selectedNormal;
	Vec3Df d = -1 * selectedPos;
	Vec3Df r = d - 2 * Vec3Df::dotProduct(d, n) * n;
	r.normalize();
	return r;

}