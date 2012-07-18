#ifndef MY_WINDOW_H
#define MY_WINDOW_H

#include <string>
#include <iostream>

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_draw.H>

#include "ftm_untar.hpp"

#include "LanWaveRPN.hpp"
#include "LanWaveObj.hpp"

using namespace std;

enum SHAPE {	CUBE,
				BANANA,
				MAN,
				SPHERE,
				TORUS,
				STAR};


struct FTModel{
		string name;
		int numVerts;
		float *verts;
		float *normals;
		float *texCoords;
};

struct vertex{
		float x;
		float y;
		float z;
		};

class MyGLWindow : public Fl_Gl_Window
{
	public:
	//Constructor and destructor
		MyGLWindow(int x, int y, int width, int height, const char* title);
		virtual ~MyGLWindow();

	//int handle(int event);

	void setVis(ftm_untar::vis_details* deets, ftm_untar *tar_obj, int selector);

	void setRatio(float ratio);

	void drawLine( float ax, float ay, float az, float bx, float by, float bz);
	void drawPoint(float x, float y, float z);

	FTModel *loadModelString(string filename, string modelName);
	FTModel *loadModelURL(string model_URL, string model_name);
	FTModel *setupModel(FTModel *m);
	
	void tellStaticID(int id);
	void showStaticID();
	void drawNumber(int num);

	void drawObject(LanWaveObj *o);

	void drawModel(FTModel *m, float posx, float posy, float posz, bool flatshaded, bool wireframe, float scalex, float scaley, float scalez, float rotx, float roty, float rotz, float colr, float colg, float colb);

	void InitializeGL();
	virtual void draw();

	void dataReceiveHandler(int* data, size_t length);
	
//	FTModel model;

	FTModel *model_number[10];
	
	int static_id;

	int *audioData;
	int audioPieces;
	int option_colour;
	enum SHAPE option_shape;
	int option_number_shape;
	float option_speed;

	float screen_ratio;
	
	int lightCountMax;
	int lightCount;
//	bool optional_bars;
	
	bool cam_loaded;
	bool show_static_ID;

	std::vector<LanWaveObj*> objects;
	std::vector<FTModel*> models;

//	virtual void IdleCallback(void*);

	virtual int handle(int event);

	float rotation;
	float rotationIncrement;

	int	pulseCounter;
	int	option_pulseLength;
	int	*pulseFraction;

	float shapeBaseSize;
	float cubeBigger;

	//visualiser_information* infoPackage;

	//private:
		
};

#endif
