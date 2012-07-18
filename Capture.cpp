#include "Capture.hpp" 

/*
Initialize function returns the names of all 
the available capture devices on your computer.
*/
char** Capture::Initialize() {
	int i = 0 ;
	pContext = alcGetCurrentContext();
	pDevice = alcGetContextsDevice(pContext);
	pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	chooseDevice = (char**) calloc( 50 , sizeof(char*) ) ; 

	if (pDeviceList)
	{
		//std::cout << endl << "Available Capture Devices are:" << endl;
		while (*pDeviceList)
		{
			//std::cout << endl << i << ".  " << pDeviceList;
			chooseDevice[i] = (char*) calloc( strlen(pDeviceList) + 1 , sizeof(char)) ;
			strncpy( chooseDevice[i] , pDeviceList, strlen(pDeviceList) + 1  ) ;
			pDeviceList += strlen(pDeviceList) + 1; 
			i++;
		}
	}
	return chooseDevice  ;
}

/*
Set up then start the capture device taken as an input;
Stream in audio signal data from the capture device into the buffer;
Analyze of the signal data and format the results for the visalisation;
Send the formated data over the network to the slaves.
*/
void Capture::Run(LanwaveNetwork* network, char* device)
	{
	// Open the capture device selected by the user.
	szDefaultCaptureDevice = (const ALchar*) device ;

	// Set up the network.
	this->the_network = network;

	// Open and configures the capture device with the predefined frequency , format and buffersize.
	pCaptureDevice = alcCaptureOpenDevice(szDefaultCaptureDevice, FS, AL_FORMAT_MONO16, B);
	
	// The truth value of begin handles the windowing for the first signal buffer.
	bool begin = true;

	// Check if a valid capture device is selected.
	if (pCaptureDevice)
	{
		//std::cout<< "Opened" << alcGetString(pCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER) << " Capture Device" << endl; 

		// Configure the FFT buffers.
		kiss_fft_cpx	fout[NF];
		kiss_fft_scalar newfin[NF];
		kiss_fftr_cfg mycfg = kiss_fftr_alloc(NF,0,newfin,NULL);

		// Start streaming audio signal data from the capture device.
		alcCaptureStart(pCaptureDevice);
		
		int avg[9];
		avg[8] = 0;

		// While the program is still running.
		while ( capture_running )
		{
			
			// Start receiving signal samples.
			alcGetIntegerv(pCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &iSamplesAvailable);

			// If enough samples are received.
			if (iSamplesAvailable > ( B / 2 ))
			{

				// Capture the samples in the buffer.
				alcCaptureSamples( pCaptureDevice, Buffer, B / 2);
				short* newbuf = (short*)Buffer;

				// initialize the buffers for signal analysis.
				float	abs[NF/2];
				float	newb[NF/2];
				float	spectrogram[ NF/2]; 

				// handles the windowing for the first buffer of signals.
				if( begin ) 
				{
					for ( int i = 0; i < NF/2 ; i++ ) 
					{
						newb[i] = (float) (newbuf[i]);
					}
					begin = false;
				}
				else 
				{
					/*
					 * prepare the buffer for analysis;
					 * this handles the windowing : by windowing, some of the signal 
					 * data will be analised twice for a more accured output.
					 */
					for ( int i = 0; i < NF/2 ; i++ ) 
					{
						newfin[i]   =  newb[i] ;
						newfin[i+O] = (float) (newbuf[i]);
						newb[i] = newfin[i+O];
					}

					// apply hamming on the windowed signal data buffer.
					for ( int i = 0; i < NF ; i++ ) 
						newfin[i] = newfin[i] *( 0.54 - 0.46* cos( ( 2 * MY_PI * i ) / ( NF - 1 ) ) );

					// apply fourier trnsform 
					kiss_fftr(  mycfg, newfin, fout ) ;

					// get the actual frequency values in time.
					int frequencies[NF/2] ;
					for ( int i = 0 ; i < NF/2 ; i++ )
						frequencies[i] = i * FS / NF ;

					/*
					 * get the absotulute amplitude values;
					 * create the spectogram of amplidute values at the different frequencies in time;
					 * the frequency goes from 0 to 22050;
					 * the time interval depends on the buffersize therefore on the amount of samples.
					 */
					for ( int i = 0 ; i < NF/2 ; i++ ) {
						abs[i] = ( sqrt ( fout[i].r * fout[i].r + fout[i].i * fout[i].i ) ) ;
						if(abs[i] == 0)
							spectrogram[i] = 0;
						else
						//spectrogram[i] = 20*log10( abs[i] );
						spectrogram[i] =  abs[i]; 
					}
					
					/*
					 * start the spectrum analysis;
					 * prepare the results of the analysis for visualisation;
					 * the algorythm creates averages of the amplitudes for different
					 * frequency intervals that represent for each moment in time when 
					 * a sample was captured, and scales it for a smoother visualisation.
					 */
					int pos = 0 ;
					
					int limit[8] =  { 2, 4, 6, 35, 46, 46, 368, 5 } ;
					//float scale[8] = { 20000, 20000, 10000, 10000, 5000, 5000,5000,5000};
					float scale[8] = { 20000, 19000, 13000, 8000, 4000, 3000,2800,2600};
					for ( int i = 0 ; i < 8 ; i++ )
					{
						avg[i] = 0;
						int sum = 0 ;
						for ( int j = pos ; j < pos+limit[i] ; j++ )
							sum += spectrogram[j];
						avg[i] = (int)(((float)sum / (float)limit[i]) / scale[i]);
						// print the formatted results
						//cout << avg[i] << " " << ((float)sum / (float)limit[i]) / scale[i];
						pos += limit[i];
					}
					//std::cout <<endl;
					
					avg[8]++;

					/*
					 * send formatted data over the network to the slaves for visualisation;
					 * the data has information about frequency ranges : sub-bas ,bass, 
					 * upper-base,low-mid ,mid ,upper-mid, brightness and ultra-high.
					 */
					this->the_network->send_int_array(avg,9);
					
				}
			}
		}

		// stop the streaming
		alcCaptureStop(pCaptureDevice);
		// capture the possible lost samples if the program is not closed properly
		alcGetIntegerv(pCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &iSamplesAvailable);
		// close the capture device
		alcCaptureCloseDevice(pCaptureDevice);
	}
	// free the fourier transform buffers
	kiss_fftr_free;
}

Capture::~Capture() {
	// free the main buffer
	free(Buffer);
}

// old main for individual class testing purposes before the capture was added in the main program
/*
void main() 
{
	Cap *C = new Capture ;
	C->Initialize() ;
	C->Run();
}
*/
