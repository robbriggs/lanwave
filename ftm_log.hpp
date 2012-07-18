#ifndef FTM_LOG
#define FTM_LOG

#include <typeinfo>
#include <time.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#define ftm_log_func ftm_log <<  __PRETTY_FUNCTION__ << " : "
#define ftm_log_place ftm_log << __FILE__<< ":" << __LINE__ << " "

using namespace std;

// The logging class
class ftm_log
{
	public:
		ftm_log( string project_name );
		~ftm_log();
		void operator << ( string msg );
		void operator << ( int msg );
		void operator << ( double msg );
		void operator << ( short msg );
		void write_log( string msg );
		void stream_log( string msg );
		
	
	private:
		int count;
		string name;
		string filename;
		time_t start_time;

		char* cur_time( void );
		char* time_to_string( time_t* t );
		void print_start();
};

#endif
