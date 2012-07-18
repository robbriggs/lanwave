##################################################################################################
#		LanWave
#	Network make file
##################################################################################################

#		Variables
#--------------------------------------------------------------
#	Top level source directory.
SRC_DIR			= ..
#	Drop Box "Source Code" location on your computer
DB_SOURCE_DIR	= ~/Dropbox/Shared\ -\ French\ Toast\ Mafia/Source\ Code/

SOURCE_DIR_TOP	= /home/user-pool/users/rb0262/FrenchToastMafia/lib/OpenAl
#	the "unstable" folder in the drop box folder for your module
DB_UNSTABLE		= network/

#CC = perl /home/user-pool/users/rb0262/FrenchToastMafia/Etc/colorgcc.pl
CC = g++

PREOPS			= -std=c++98 -g -O0 -I $(SRC_DIR)/lib/fltk -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT -DHAVE_GETTIMEOFDAY -D__LINUX_ALSA__ -D__LINUX_ALSASEQ__-D__LITTLE_ENDIAN__ -L../lib/lua-5.2.0/src -I../lib/lua-5.2.0/src/
PATHS			= -I ../../ -I $(SOURCE_DIR_TOP) -L $(SOURCE_DIR_TOP) -I $(SRC_DIR)/lib -L $(SRC_DIR)/lib/boost-lib $(SRC_DIR)/lib/fltk-lib/libfltk3gl.a $(SRC_DIR)/lib/fltk-lib/libfltk3images.a -lGLU -lGL $(SRC_DIR)/lib/fltk-lib/libfltk3.a -lXext -lXft -lfontconfig -lXinerama -lpthread -ldl -lm -lX11 -llua -ldl
LIBS			= -lboost_system -lboost_thread -lpthread -lboost_filesystem -lasound -lm -lpng -lz -ljpeg -lopenal
LINKER_PATHS	= -Wl,-R $(SRC_DIR)/lib/boost-lib -Wl,-R /home/user-pool/users/rb0262/FrenchToastMafia/lib/OpenAl

#		Build Instructions
#--------------------------------------------------------------
#	link the .o files to create the executable
default:
	clear
	make all

gui:
	rm LanWave.o
	make all

all: LanWave.o ftm_log.o MyGLWindow.o global.o Capture.o ftm_untar.o LanWaveRPN.o LanWaveRPNToken.o LanWaveObj.o kiss_fft.o kiss_fftr.o LWNode.o networkMap.o networkMapInterface.o LanwaveNetwork.o udpInterface.o tcpInterface.o tcpSession.o 
	$(CC) $(PREOPS) LWNode.o networkMap.o networkMapInterface.o ftm_log.o MyGLWindow.o global.o kiss_fft.o Capture.o ftm_untar.o LanWaveRPN.o LanWaveRPNToken.o LanWaveObj.o kiss_fftr.o LanwaveNetwork.o udpInterface.o tcpInterface.o tcpSession.o LanWave.o -o LanWave $(PATHS) $(LINKER_PATHS) $(LIBS)

.cpp.o:
	$(CC) $(PREOPS) -c $< -o $@ $(PATHS)
        
.c.o:
	$(CC) $(PREOPS) -c $< -o $@ $(PATHS)

#LanWave.o: LanWave.cpp
#	$(cc) $(PREOPS) -c LanWave.cpp $(PATHS)

# networking
#LanwaveNetwork.o: LanwaveNetwork.cpp
#	$(cc) $(PREOPS) -c LanwaveNetwork.cpp $(PATHS)

#udpInterface.o: udpInterface.cpp
#	$(cc) $(PREOPS) -c udpInterface.cpp $(PATHS)

#ftm_log.o: ftm_log.cpp
#	$(cc) $(PREOPS) -c ftm_log.cpp $(PATHS)

#LWNode.o: LWNode.cpp
#	$(cc) $(PREOPS) -c LWNode.cpp $(PATHS) $(LINKER_PATHS) $(LIBS)

#networkMap.o: networkMap.cpp
#	$(cc) $(PREOPS) -c networkMap.cpp $(PATHS) $(LINKER_PATHS) $(LIBS)

#MyGLWindow.o: MyGLWindow.cpp
#	g++ $(PREOPS) -c MyGLWindow.cpp $(PATHS)

#Rt.o : Rt.cpp 
#	$(cc) $(PREOPS) -c Rt.cpp $(PATHS)
	
#chuck_fft.o : chuck_fft.c 
#	$(cc) $(PREOPS) -c chuck_fft.c $(PATHS)


#		Tools for the make file process
#--------------------------------------------------------------
#	Clean all binaries and intemediary files
clean:
	rm -f *.o logs/*.log *~

permission-fix:
	chown rb0262:frenchtm -R --silent ./
