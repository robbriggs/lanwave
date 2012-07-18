#include "MyGLWindow.h"
#include <FL/Fl.H>
#include <FL/gl.h>

#ifndef Win32
#include <fltk3gl/glu.h>
#else
#ifdef Win32
#include <glu.h>
#endif
#endif

#include <math.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ftm_untar.hpp"

#include "global.hpp"

#ifdef WIN32
	#include "rapid/rapidxml.hpp"
	#include "rapid/rapidxml_print.hpp"
#else
	#include "../lib/rapidxml-1.13/rapidxml.hpp"
	#include "../lib/rapidxml-1.13/rapidxml_print.hpp"
#endif

using namespace std;

void IdleCallback(void* pData)
{
	if(pData != NULL)
	{
		MyGLWindow* pGLWindow = reinterpret_cast<MyGLWindow*>(pData);
		pGLWindow->redraw();
	}
	else
		cout << "MyGLWindow - IdleCallback receiving NULL pointer data.\n";
}

// Constructor
MyGLWindow::MyGLWindow(int x, int y, int width, int height, const char* title) : Fl_Gl_Window(x, y, width, height, title)
{
	mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);

	cam_loaded = false;
	show_static_ID = true;
	screen_ratio = 1;

	static_id = -1;

	infoPackage->rid = 2;
	infoPackage->maxid = 10;
	infoPackage->count = 0;

	for(int i = 0; i < 8; i++) infoPackage->frequency_values[i] = i;
	for(int i = 0; i < 5; i++) infoPackage->slide[i] = 0;
	
	infoPackage->camPos[0] = 0;
	infoPackage->camPos[1] = 0;
	infoPackage->camPos[2] = 3;

	for(int i = 0; i < 3; i++) infoPackage->camLook[i] = 0;

	infoPackage->camRot[0] = 0;
	infoPackage->camRot[1] = 1;
	infoPackage->camRot[2] = 0;

	for(int i = 0; i < 3; i++) infoPackage->camCol[i] = 0;

	model_number[0] = loadModelURL("3d/n_0.h", "n_0");
	model_number[1] = loadModelURL("3d/n_1.h", "n_1");
	model_number[2] = loadModelURL("3d/n_2.h", "n_2");
	model_number[3] = loadModelURL("3d/n_3.h", "n_3");
	model_number[4] = loadModelURL("3d/n_4.h", "n_4");
	model_number[5] = loadModelURL("3d/n_5.h", "n_5");
	model_number[6] = loadModelURL("3d/n_6.h", "n_6");
	model_number[7] = loadModelURL("3d/n_7.h", "n_7");
	model_number[8] = loadModelURL("3d/n_8.h", "n_8");
	model_number[9] = loadModelURL("3d/n_9.h", "n_9");

	Fl::add_idle(IdleCallback, this);
}

// Destructor
MyGLWindow::~MyGLWindow(void)
{}

void MyGLWindow::InitializeGL()
{
	glClearColor(.1f, .1f, .1f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
}

void MyGLWindow::draw()
{
    //static bool firstTime = true;	// static means this will only be initalised once and kept in memory
    //if (firstTime) {
            InitializeGL();
    //        firstTime = false;
    //}

	glClearColor(infoPackage->camCol[0], infoPackage->camCol[1], infoPackage->camCol[2], 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the color and depth buffer

	// view transformations
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-screen_ratio, screen_ratio, -1, 1, 1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(show_static_ID)
	{
		float lightColour[4] = {1, 1, 1, 1};
		float lightPosition[4] = {0, 1, -1, 1};

		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColour);
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		drawNumber(static_id);



		gluLookAt(	0, 0, 3,
					0, 0, 0,
					0, 1, 0 );
	}
	else
	{
		gluLookAt(	infoPackage->camPos[0],
			infoPackage->camPos[1],
			infoPackage->camPos[2],
			infoPackage->camLook[0],
			infoPackage->camLook[1],
			infoPackage->camLook[2],
			infoPackage->camRot[0],
			infoPackage->camRot[1],
			infoPackage->camRot[2]);

		lightCount = 0;

		for(int i = 0; i < objects.size(); i++)
			drawObject(objects[i]);
	}
}

void MyGLWindow::setVis(ftm_untar::vis_details* deets, ftm_untar *tar_obj, int selector)
{
	// objects is vector of "LanWaveObj"s
	show_static_ID = false;
	objects = deets->objects;
	lightCountMax= 0;

	//cout << "glwindow setvis 1." << endl;

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHT4);
	glDisable(GL_LIGHT5);
	glDisable(GL_LIGHT6);
	glDisable(GL_LIGHT7);

	//cout << "setVis got " << objects.size() << " objects." << endl;

	infoPackage->camPos[0] = objects[0]->getPosX();
	infoPackage->camPos[1] = objects[0]->getPosY();
	infoPackage->camPos[2] = objects[0]->getPosZ();

	infoPackage->camLook[0] = objects[0]->getSclX();
	infoPackage->camLook[1] = objects[0]->getSclY();
	infoPackage->camLook[2] = objects[0]->getSclZ();

	infoPackage->camRot[0] = objects[0]->getRotX();
	infoPackage->camRot[1] = objects[0]->getRotY();
	infoPackage->camRot[2] = objects[0]->getRotZ();

	infoPackage->camCol[0] = objects[0]->getColR();
	infoPackage->camCol[1] = objects[0]->getColG();
	infoPackage->camCol[2] = objects[0]->getColB();

	cam_loaded = true;

	//cout << "glwindow setvis 2." << endl;

	for(int i = 1; i < objects.size(); i++)
	{
		//cout << "MyGLWindow::setVis on iteration " << i << endl;
		// Set each object to point to infoPackage, for evaluating dynamic values
		//cout << "MyGLWindow creating object and passing infoPackage address: " << infoPackage << endl;
		//objects[i]->setInfoPackage(infoPackage);

		//cout << "Object " << i << "has name " << endl;
		//cout << objects[i]->getModelName() << endl;

		// If light, set modelID to -1		
		if(objects[i]->getModelName().compare("light") == 0)
		{
			//cout << "if" << endl;
			//cout << "Light set" << endl;
			objects[i]->setModelID(-1);
			lightCountMax++;
		}
		else
		{
			//cout << "else" << endl;
			// Add new models, and assign IDs to prevent duplicates
			bool exists = false;
			int j = 0;
			//cout << "getting models.size()" << endl;
			//cout << "is " << models.size() << endl;
			for(; j < models.size(); j++)
			{
				//cout << "Comparing " << j << " with " << i << endl;
				//cout << "Comparing model name " << models[j]->name << " with object name " << objects[i]->getModelName() << endl;
				if(models[j]->name.compare(objects[i]->getModelName())==0)
				{
					exists = true;
					break;
				}
			}

			if(exists)
				objects[i]->setModelID(j);
			else
			{
				//models.push_back(loadModel(objects[i]->getModelName()));
				// CALL LOADMODEL WITH 3DMODEL-STRING (full thing), OPENED WITH FTM_SUBFILE(objects[i]->getModelName()) (?)
				//cout << "tar_obj->set_file(" << selector << ")" << endl;
				tar_obj->set_file(selector);//
				
				//cout << "GLWindow: Loading model " << objects[i]->getModelName() << endl;	// getModelName() is a string
				//cout << "GLWindow: loadModelString: " << endl;
				//cout << tar_obj->extract_sub_file(objects[i]->getModelName()) << endl;
				models.push_back(loadModelString(tar_obj->extract_sub_file(objects[i]->getModelName()), objects[i]->getModelName())); //!!!

				//cout << "setting modelID" << endl;
				objects[i]->setModelID(models.size()-1);
			}
		}
	}
}

FTModel *MyGLWindow::loadModelURL(string model_URL, string model_name)
{
	size_t found;

	int numFaces = 0;
	int numVerts = 0;
	
	// Create file-in stream and open the file
	ifstream model_stream;
	model_stream.open(model_URL.c_str());
	
	string line;
	getline(model_stream, line);

	// Find the line telling number of faces
	while((found = line.find("faces")) == string::npos)
		getline(model_stream, line);

	// Extract int numFaces
	sscanf(line.c_str(), "faces          : %d", &numFaces);

	//cout << "numFaces: " << numFaces << endl;
	float *verts = new float [numFaces*9];
	float *normals = new float [numFaces*9];
	float *texCoords = new float [numFaces*6];

	// Find the line telling numVerts (necessary for OpenGL drawArrays() function)
	while((found = line.find("unsigned int numVerts")) == string::npos)
		getline(model_stream, line);

	// Extract int numVerts
	sscanf(line.c_str(), "unsigned int numVerts = %d;", &numVerts);

	//~ cout << "numVerts: " << numVerts << endl;

	/*-------------*/
	/* VERTS ARRAY */
	/*-------------*/

	// Search through file until verts array is found
	while((found = line.find("float verts []")) == string::npos)
		getline(model_stream, line);

	// Read values from file into vert array
	for(int i = 0; i < numFaces*9; i+=9)
	{
		float v0, v1, v2, v3, v4, v5, v6, v7, v8;

		// Skip description line
		getline(model_stream, line);
		//~ cout << line << endl;

		// Read first row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v0, &v1, &v2);
		//~ cout << v0 << ", " << v1 << ", " << v2 << endl;
		verts[i  ] = v0; verts[i+1] = v1; verts[i+2] = v2; 

		// Read second row into vars then array		
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v3, &v4, &v5);
		//~ cout << v3 << ", " << v4 << ", " << v5 << endl;
		verts[i+3] = v3; verts[i+4] = v4; verts[i+5] = v5;

		// Read third row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v6, &v7, &v8);
		//~ cout << v6 << ", " << v7 << ", " << v8 << endl;
		verts[i+6] = v6; verts[i+7] = v7; verts[i+8] = v8;
	}
	
	//cout << "float verts array read" << endl;

	/*---------------*/
	/* NORMALS ARRAY */
	/*---------------*/

	//~ cout << "float normals array of size " << numFaces*9 << " created\n";

	// Search through file until normals array is found
	while((found = line.find("float normals []")) == string::npos)
		getline(model_stream, line);

	// Read values from file into normals array
	for(int i = 0; i < numFaces*9; i+=9)
	{
		float v0, v1, v2, v3, v4, v5, v6, v7, v8;

		// Skip description line
		getline(model_stream, line);
		//~ cout << line << endl;

		// Read first row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v0, &v1, &v2);
		//~ cout << v0 << ", " << v1 << ", " << v2 << endl;
		normals[i  ] = v0; normals[i+1] = v1; normals[i+2] = v2; 

		// Read second row into vars then array		
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v3, &v4, &v5);
		//~ cout << v3 << ", " << v4 << ", " << v5 << endl;
		normals[i+3] = v3; normals[i+4] = v4; normals[i+5] = v5;

		// Read third row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v6, &v7, &v8);
		//~ cout << v6 << ", " << v7 << ", " << v8 << endl;
		normals[i+6] = v6; normals[i+7] = v7; normals[i+8] = v8;
	}
	
	//cout << "float normals array read" << endl;

	/*-----------------*/
	/* TEXCOORDS ARRAY */
	/*-----------------*/

	//~ cout << "float texCoords array of size " << numFaces*6 << " created\n";

	// Search through file until texCoords array is found
	while((found = line.find("float texCoords []")) == string::npos)
		getline(model_stream, line);

	// Read values from file into texCoords array
	for(int i = 0; i < numFaces*6; i+=6)
	{
		float v0, v1, v2, v3, v4, v5;

		// Skip description line
		getline(model_stream, line);
		//~ cout << line << endl;

		// Read first row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f", &v0, &v1);
		//~ cout << v0 << ", " << v1 << endl;
		texCoords[i  ] = v0; texCoords[i+1] = v1;

		// Read second row into vars then array		
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f", &v2, &v3);
		//~ cout << v2 << ", " << v3 << endl;
		texCoords[i+2] = v2; texCoords[i+3] = v3;

		// Read third row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f", &v4, &v5);
		//~ cout << v4 << ", " << v5 << endl;
		texCoords[i+4] = v4; texCoords[i+5] = v5;
	}
	
	//cout << "float texCoords array read" << endl;

	// CLOSE FILE
	//model_file.close();

	FTModel *return_model = new FTModel;

	return_model->name = model_name;
	return_model->numVerts = numVerts;
	return_model->verts = verts;
	return_model->normals = normals;
	return_model->texCoords = texCoords;

	//cout << "loadModel(" << filename << ") returning" << endl;
	return return_model;
}

FTModel *MyGLWindow::loadModelString(string model_string, string model_name)
{
	size_t found;

	int numFaces = 0;
	int numVerts = 0;
	
	// Create file-in stream and open the file
	stringstream model_stream (stringstream::in | stringstream::out);
	model_stream << model_string;
	
	string line;
	getline(model_stream, line);

	// Find the line telling number of faces
	while((found = line.find("faces")) == string::npos)
		getline(model_stream, line);

	// Extract int numFaces
	sscanf(line.c_str(), "faces          : %d", &numFaces);

	//cout << "numFaces: " << numFaces << endl;
	float *verts = new float [numFaces*9];
	float *normals = new float [numFaces*9];
	float *texCoords = new float [numFaces*6];

	// Find the line telling numVerts (necessary for OpenGL drawArrays() function)
	while((found = line.find("unsigned int numVerts")) == string::npos)
		getline(model_stream, line);

	// Extract int numVerts
	sscanf(line.c_str(), "unsigned int numVerts = %d;", &numVerts);

	//~ cout << "numVerts: " << numVerts << endl;

	/*-------------*/
	/* VERTS ARRAY */
	/*-------------*/

	// Search through file until verts array is found
	while((found = line.find("float verts []")) == string::npos)
		getline(model_stream, line);

	// Read values from file into vert array
	for(int i = 0; i < numFaces*9; i+=9)
	{
		float v0, v1, v2, v3, v4, v5, v6, v7, v8;

		// Skip description line
		getline(model_stream, line);
		//~ cout << line << endl;

		// Read first row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v0, &v1, &v2);
		//~ cout << v0 << ", " << v1 << ", " << v2 << endl;
		verts[i  ] = v0; verts[i+1] = v1; verts[i+2] = v2; 

		// Read second row into vars then array		
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v3, &v4, &v5);
		//~ cout << v3 << ", " << v4 << ", " << v5 << endl;
		verts[i+3] = v3; verts[i+4] = v4; verts[i+5] = v5;

		// Read third row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v6, &v7, &v8);
		//~ cout << v6 << ", " << v7 << ", " << v8 << endl;
		verts[i+6] = v6; verts[i+7] = v7; verts[i+8] = v8;
	}
	
	//cout << "float verts array read" << endl;

	/*---------------*/
	/* NORMALS ARRAY */
	/*---------------*/

	//~ cout << "float normals array of size " << numFaces*9 << " created\n";

	// Search through file until normals array is found
	while((found = line.find("float normals []")) == string::npos)
		getline(model_stream, line);

	// Read values from file into normals array
	for(int i = 0; i < numFaces*9; i+=9)
	{
		float v0, v1, v2, v3, v4, v5, v6, v7, v8;

		// Skip description line
		getline(model_stream, line);
		//~ cout << line << endl;

		// Read first row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v0, &v1, &v2);
		//~ cout << v0 << ", " << v1 << ", " << v2 << endl;
		normals[i  ] = v0; normals[i+1] = v1; normals[i+2] = v2; 

		// Read second row into vars then array		
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v3, &v4, &v5);
		//~ cout << v3 << ", " << v4 << ", " << v5 << endl;
		normals[i+3] = v3; normals[i+4] = v4; normals[i+5] = v5;

		// Read third row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f, %f", &v6, &v7, &v8);
		//~ cout << v6 << ", " << v7 << ", " << v8 << endl;
		normals[i+6] = v6; normals[i+7] = v7; normals[i+8] = v8;
	}
	
	//cout << "float normals array read" << endl;

	/*-----------------*/
	/* TEXCOORDS ARRAY */
	/*-----------------*/

	//~ cout << "float texCoords array of size " << numFaces*6 << " created\n";

	// Search through file until texCoords array is found
	while((found = line.find("float texCoords []")) == string::npos)
		getline(model_stream, line);

	// Read values from file into texCoords array
	for(int i = 0; i < numFaces*6; i+=6)
	{
		float v0, v1, v2, v3, v4, v5;

		// Skip description line
		getline(model_stream, line);
		//~ cout << line << endl;

		// Read first row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f", &v0, &v1);
		//~ cout << v0 << ", " << v1 << endl;
		texCoords[i  ] = v0; texCoords[i+1] = v1;

		// Read second row into vars then array		
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f", &v2, &v3);
		//~ cout << v2 << ", " << v3 << endl;
		texCoords[i+2] = v2; texCoords[i+3] = v3;

		// Read third row into vars then array
		getline(model_stream, line);
		sscanf(line.c_str(), "%f, %f", &v4, &v5);
		//~ cout << v4 << ", " << v5 << endl;
		texCoords[i+4] = v4; texCoords[i+5] = v5;
	}
	
	//cout << "float texCoords array read" << endl;

	// CLOSE FILE
	//model_file.close();

	FTModel *return_model = new FTModel;

	return_model->name = model_name;
	return_model->numVerts = numVerts;
	return_model->verts = verts;
	return_model->normals = normals;
	return_model->texCoords = texCoords;

	//cout << "loadModel(" << filename << ") returning" << endl;
	return return_model;
}

FTModel* MyGLWindow::setupModel(FTModel *m)
{
	glEnable(GL_VERTEX_ARRAY);

	// set input data to arrays
	glVertexPointer(3, GL_FLOAT, 0, m->verts);
	glNormalPointer(GL_FLOAT, 0, m->normals);
	glTexCoordPointer(2, GL_FLOAT, 0, m->texCoords);

	return m;
}

void MyGLWindow::setRatio(float ratio)
{
	screen_ratio = ratio;
}

void MyGLWindow::showStaticID()
{
	show_static_ID = true;
}

void MyGLWindow::tellStaticID(int id)
{
	static_id = id;
}

void MyGLWindow::drawNumber(int num)
{
	//cout << "GLWindow: drawing number " << num << endl;

	if(num == -1)		// Not told static ID yet
	{
		//cout << "glwindow not told static id yet" << endl;
	}
	else if(num < 10)		// Single-digit
	{
		drawModel(setupModel(model_number[num]), 0.0f, -0.5f, 1.0f, false, false, 1.4f, 1.4f, 1.0f, 270.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else if(num < 100)	// Double-digit
	{
		int d1 = num/10;
		int d2 = num - d1*10;
		
		drawModel(setupModel(model_number[d1]), -0.5f, -0.5f, 1.0f, false, false, 1.4f, 1.4f, 1.0f, 270.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		drawModel(setupModel(model_number[d2]),  0.5f, -0.5f, 1.0f, false, false, 1.4f, 1.4f, 1.0f, 270.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else if(num < 1000)	// Triple-digit
	{
		int d1 = num/100;
		int d2 = (num - d1*100)/10;
		int d3 = num - d1*100 - d2*10;
		
		drawModel(setupModel(model_number[d1]), -0.75f, -0.5f, 2.0f, false, false, 1.0f, 1.4f, 1.0f, 270.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		drawModel(setupModel(model_number[d2]),  0.00f, -0.5f, 2.0f, false, false, 1.0f, 1.4f, 1.0f, 270.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		drawModel(setupModel(model_number[d3]),  0.75f, -0.5f, 2.0f, false, false, 1.0f, 1.4f, 1.0f, 270.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}

}


void MyGLWindow::drawObject(LanWaveObj *o)
{
	if(o->getModelID() == -2)
	{
		infoPackage->camPos[0] = o->getPosX();
		infoPackage->camPos[1] = o->getPosY();
		infoPackage->camPos[2] = o->getPosZ();

		infoPackage->camLook[0] = o->getSclX();
		infoPackage->camLook[1] = o->getSclY();
		infoPackage->camLook[2] = o->getSclZ();

		infoPackage->camRot[0] = o->getRotX();
		infoPackage->camRot[1] = o->getRotY();
		infoPackage->camRot[2] = o->getRotZ();

		infoPackage->camCol[0] = o->getColR();
		infoPackage->camCol[1] = o->getColG();
		infoPackage->camCol[2] = o->getColB();
	}
	else if(o->getModelID() == -1)//
	{
		//cout << "Drawing light " << lightCount << " of " << lightCountMax << " at (" << o->getPosX() << ", " << o->getPosY() << ", " << o->getPosZ() << " )" << endl;

		float lightColour[4] = {o->getColR(), o->getColG(), o->getColB(), 1};
		float lightPosition[4] = {o->getPosX(), o->getPosY(), o->getPosZ(), 1};

		switch(lightCount)
		{
			case 0:
				glEnable(GL_LIGHT0);
				glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColour);
				glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
				break;
			case 1:
				glEnable(GL_LIGHT1);
				glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColour);
				glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
				break;
			case 2:
				glEnable(GL_LIGHT2);
				glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColour);
				glLightfv(GL_LIGHT2, GL_POSITION, lightPosition);
				break;
			case 3:
				glEnable(GL_LIGHT3);
				glLightfv(GL_LIGHT3, GL_DIFFUSE, lightColour);
				glLightfv(GL_LIGHT3, GL_POSITION, lightPosition);
				break;
			case 4:
				glEnable(GL_LIGHT4);
				glLightfv(GL_LIGHT4, GL_DIFFUSE, lightColour);
				glLightfv(GL_LIGHT4, GL_POSITION, lightPosition);
				break;
			case 5:
				glEnable(GL_LIGHT5);
				glLightfv(GL_LIGHT5, GL_DIFFUSE, lightColour);
				glLightfv(GL_LIGHT5, GL_POSITION, lightPosition);
				break;
			case 6:
				glEnable(GL_LIGHT6);
				glLightfv(GL_LIGHT6, GL_DIFFUSE, lightColour);
				glLightfv(GL_LIGHT6, GL_POSITION, lightPosition);
				break;
			case 7:
				glEnable(GL_LIGHT7);
				glLightfv(GL_LIGHT7, GL_DIFFUSE, lightColour);
				glLightfv(GL_LIGHT7, GL_POSITION, lightPosition);
				break;
		}

		lightCount++;
	}
	else
	{
		//cout << "Drawing model" << endl;
		drawModel(models[o->getModelID()],
					o->getPosX(), o->getPosY(), o->getPosZ(),
					true, o->isWireframe(),
					o->getSclX(), o->getSclY(), o->getSclZ(),
					o->getRotX(), o->getRotY(), o->getRotZ(),
					o->getColR(), o->getColG(), o->getColB());
		//cout << "Finished drawing model" << endl;
	}
}

void MyGLWindow::drawModel(FTModel *m, float posx, float posy, float posz, bool flatshaded, bool wireframe, float scalex, float scaley, float scalez, float rotx, float roty, float rotz, float colr, float colg, float colb)
{
	glEnable(GL_VERTEX_ARRAY);

	// set input data to arrays
	glVertexPointer(3, GL_FLOAT, 0, m->verts);
	glNormalPointer(GL_FLOAT, 0, m->normals);
	glTexCoordPointer(2, GL_FLOAT, 0, m->texCoords);


	// Set flat shading
	if(flatshaded)
	  glShadeModel( GL_FLAT );
	else
	  glShadeModel( GL_SMOOTH );

	glPushMatrix();

	glTranslatef(posx, posy, posz);
	glRotatef(rotx, 1, 0, 0);
	glRotatef(roty, 0, 1, 0);
	glRotatef(rotz, 0, 0, 1);
	glScalef(scalex, scaley, scalez);

	glColor3f ( colr, colg, colb);

	// draw data
	if(wireframe)
	{
		glLineWidth(1);
		glDrawArrays(GL_LINES, 0, m->numVerts);
	}
	else
	{
		//glColor3f ( 1, 0.5 + (float)pulseFraction[0]/2, (float)pulseFraction[0]);
		glDrawArrays(GL_TRIANGLES, 0, m->numVerts);
	}

	glPopMatrix();  
}

void MyGLWindow::drawPoint( float x, float y, float z) 
{ 
	glDisable(GL_TEXTURE_2D); 
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glColor4ub( 255, 255, 255, 150); 

	glPointSize(3); 
	glBegin(GL_POINTS); 
	glVertex3f( x, y, z); 
	glEnd(); 

	glDisable(GL_BLEND); 
	glEnable(GL_TEXTURE_2D); 
}

void MyGLWindow::drawLine( float ax, float ay, float az, float bx, float by, float bz) 
{ 
	glDisable(GL_TEXTURE_2D); 
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glColor4ub( 255, 255, 0, 255); 

	glLineWidth(7); 
	glBegin(GL_LINES); 
	glVertex3f( ax, ay, az); 
	glVertex3f( bx, by, bz); 
	glEnd(); 

	glDisable(GL_BLEND); 
	glEnable(GL_TEXTURE_2D); 
}


void MyGLWindow::dataReceiveHandler(int* data, size_t length)
{
	if(data)
	{
		for(int i = 0; i < 9; i++)
			infoPackage->frequency_values[i] = data[i];

		infoPackage->count = infoPackage->frequency_values[8];

	}
	else
		cout << "dataReceiveHandler: NULL data received" << endl;
}
