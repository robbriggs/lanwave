#include <fstream>
#include <iostream>
#include "ftm_log.hpp"

ftm_log logg("LanWave");

// **************
// **** Code ****
// **************

// **** Logging ****

// This is the constructor for the logging class, it takes the name
// of the project, so it can be used in the log file
ftm_log::ftm_log( string project_name )
{
	this->name = project_name;
	this->filename = project_name + ".log";
	this->count = -2;
	time(&(this->start_time));
}

// The destructor prints the end time if errors have occured
ftm_log::~ftm_log()
{
	if ( this->count != -2	)
	{
		stringstream str;
		str << this->name << " has shutdown with " << this->count << " error";
	
		if ( this->count > 1 )
			str << "s";
		
		str << "\n";
	
		this->write_log(str.str());
	}
}

// This adds the start time to the log file
void ftm_log::print_start()
{
	stringstream str;
	const char* time_str = time_to_string( &(this->start_time) );
	this->count = -1;
	
	ofstream a_file ( "log_file.log", ios::app );
	str << "\n" << "[" << time_str << "] " << this->name << " has opened.";
	
	a_file << str.str();
	delete time_str;
	this->count++;
}

// This adds a message to the log file
void ftm_log::write_log( string msg )
{
	const char* time_str = cur_time();
	time_t rawtime;
	
	if ( this->count == -2)
		print_start();

	time ( &rawtime );

	ofstream a_file ( (this->filename).c_str(), ios::app );
		
	stringstream str;
	str << "\n[" << time_str << "] " << msg;
	
	a_file << str.str();
	delete time_str;
	this->count++;
}

// **** Logging Operands ****

void ftm_log::operator<< ( string msg )
{
	this->write_log( msg );
}

void ftm_log::operator<< ( int msg )
{
	stringstream ss;
	ss << msg;
	this->write_log( ss.str() );
}

void ftm_log::operator<< ( double msg )
{
	stringstream ss;
	ss << msg;
	this->write_log( ss.str() );
}

void ftm_log::operator<< ( short msg )
{
	stringstream ss;
	ss << msg;
	this->write_log( ss.str() );
}

// **** Logging Stream ****

// Adds a string to the file without a new timestamp or new line
void ftm_log::stream_log( string msg )
{
	ofstream a_file ( this->filename.c_str(), ios::app );
	a_file << msg;
}

// **** Logging Stream Operands ****
// **** Logging Stream Operands ****

// **** Time Related ****
char* ftm_log::cur_time()
{
	time_t timer;
	time(&timer);
	return time_to_string(&timer);
}

char* ftm_log::time_to_string( time_t* t )
{
	
	char* buffer = (char*)malloc( 25 );
	struct tm* tm_info;

	
	tm_info = localtime(t);
	strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm_info);
	return buffer;
}
