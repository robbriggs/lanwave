#ifndef FTM_UNTAR_H
#define FTM_UNTAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "LanWaveObj.hpp"

#ifdef Win32
	#include "rapid/rapidxml.hpp"
	#include "rapid/rapidxml_print.hpp"
#else
	#include "../lib/rapidxml-1.13/rapidxml.hpp"
	#include "../lib/rapidxml-1.13/rapidxml_print.hpp"
#endif

class ftm_untar
{
	public:
		struct vis_details {
			std::string name;
			std::string url;
			//std::string *entire_manifest;
			rapidxml::xml_node<>* manifest;
			std::vector<LanWaveObj*> objects;
		};

		ftm_untar( std::string pathstr, std::string extention );

		std::vector<ftm_untar::vis_details*> get_vis_details();
		void delete_vis_details( std::vector<ftm_untar::vis_details*> &vis_details );
		void set_file( int file_num );
		std::string extract_sub_file( std::string sub_file );

	private:
		struct name_and_length {
			std::string name;
			int length;
			rapidxml::xml_node<>* manifest;
		};

		struct list_and_length {
			int length;
			char** names;
		};

		std::string ext;
		std::string path;
		std::vector<ftm_untar::vis_details*> stored_vis_details;
		int cur_file_num;
	
		void skip_chars( std::ifstream* file, int num );
		std::string extract_substring( std::ifstream* file, int start, int length );
		name_and_length extract_name_and_lenght( std::ifstream* file );
		std::string extract_tarred_file( std::string url, std::string file_name );
		std::vector<vis_details*> parse_all_vis(std::string path);
		void error( std::string msg );
		std::string cstr_to_str(const char * str);
		int octal_str_to_int( const char* str );
		list_and_length files_in_dir(std::string);
};

#endif
