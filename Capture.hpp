#pragma once
#ifndef FTM_CAPTURE
#define FTM_CAPTURE


#include "al.h" 
#include "alc.h" 
#include "kiss_fftr.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include "global.hpp"
#include "LanwaveNetwork.hpp"


using namespace std ;

// capture device buffer size
#define B		2048 	
// window size
#define NF		1024
#define O		512
// frequency 
#define FS		44100 
// PI
#define MY_PI   3.14159265358979323846
// main buffersize
#define BUFFERSIZE				4096

class Capture
{
private:

	ALchar			Buffer[B];
	ALCcontext		*pContext;
	const ALchar	*pDeviceList;
	char			**chooseDevice;
	ALint			iSamplesAvailable;
	const ALCchar	*szDefaultCaptureDevice;
	ALCdevice		*pDevice, *pCaptureDevice;
	LanwaveNetwork  *the_network;

public:

	char** Initialize();
	void Run(LanwaveNetwork *network, char* device );
	~Capture();

};

#endif
