#ifndef GLOBAL_H
#define GLOBAL_H

struct visualiser_information {
	int frequency_values[8];
	int rid;
	int maxid;
	int count;
	int slide[5];
	float camPos[3];
	float camLook[3];
	float camRot[3];
	float camCol[3];

};

extern bool capture_running;

extern visualiser_information *infoPackage;

#endif