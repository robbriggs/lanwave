#include "ftm_untar.hpp"
#include "global.hpp"

using namespace std;
using namespace boost::filesystem;

ftm_untar::ftm_untar( string pathstr, string extension )
{
	this->ext = extension;
	this->path = pathstr;
	
	this->stored_vis_details = this->parse_all_vis(this->path);
}

vector<ftm_untar::vis_details*> ftm_untar::get_vis_details()
{
	return this->stored_vis_details;
}

void ftm_untar::delete_vis_details( std::vector<ftm_untar::vis_details*> &vis_details )
{
	for ( int i = 0; i < vis_details.size(); i++ )
		delete vis_details[i];
}

void ftm_untar::set_file( int file_num )
{
	this->cur_file_num = file_num;
}

string ftm_untar::extract_sub_file( string sub_file )
{
	return this->extract_tarred_file( (char*)this->stored_vis_details[this->cur_file_num]->url.c_str(), sub_file );
}

void ftm_untar::skip_chars( ifstream* file, int num )
{
	char letter;
	for ( int i = 0; i < num; i++ )
	{
		if ( file->eof())
			error( "End of file reached. Unable to parse .vis file.\n");

		*file >> letter;
	}
}

string ftm_untar::extract_substring( ifstream* file, int start, int length )
{
	char letter;
	char* str = new char[length+1];

	skip_chars(file, start);
	for ( int i = 0; i < length; i++)
	{
		if ( file->eof())
			error( "End of file reached. Unable to parse .vis file.\n");

		*file >> letter;
		str[i] = letter;

	}
	str[length] = '\0';
	return cstr_to_str(str);
}

ftm_untar::name_and_length ftm_untar::extract_name_and_lenght( ifstream* file )
{
	name_and_length output;
	output.name = extract_substring(file, 0, 100).c_str();
	string lenght_str = extract_substring(file, 24, 12);
	//char * pEnd;
	output.length = (int)strtol( lenght_str.c_str(), NULL, 8 );
	//output.length = octal_str_to_int(lenght_str);
	//delete lenght_str;
	skip_chars( file, 376 );
	return output;
}

string ftm_untar::extract_tarred_file( string url, string file_name )
{
	cout << "FTMUntar: extracting tarred file" << endl;
	name_and_length details;
	cout << "Declared name_and_length details" << endl;
	ifstream file (url.c_str());
	cout << "Declared ifstream" << endl;
	file.unsetf(ios_base::skipws);
	cout << "file.unsetf(ios_base::skipws)" << endl;

	cout << "FTMUntar: before while" << endl;
	while ( true )
	{
		details = extract_name_and_lenght( &file );

		if ( details.name == file_name )
			break;

		skip_chars( &file, details.length );
		skip_chars( &file, 512-(details.length%512) );
	}
	cout << "FTMUntar: after while" << endl;

	string output = extract_substring(&file, 0, details.length);

	output += '\0';
	file.close();

	return output;
}

vector<ftm_untar::vis_details*> ftm_untar::parse_all_vis(string path)
{
	vector<vis_details*> output;

	//path += "*." + this->ext;

	string manifest;
	rapidxml::xml_document<> doc;
	vis_details* singleVis;
	directory_iterator end_itr;

	for ( directory_iterator itr( path ); itr != end_itr; ++itr )
	{
		if ( !is_directory(itr->status()) && boost::algorithm::ends_with(itr->path().string(), ".vis") )
		{
			cout << "Found file "<< itr->path().string() << endl;
			singleVis = new vis_details;
			singleVis->url = itr->path().string();

			manifest = extract_tarred_file(itr->path().string(), "manifest");

			doc.parse<0>((char*)manifest.c_str());

			singleVis->manifest = doc.first_node();
			singleVis->name = singleVis->manifest->first_node("name")->value();

			rapidxml::xml_node<> *vis_node = singleVis->manifest->first_node("visualisation");
			rapidxml::xml_attribute<> *attr;
			
			singleVis->objects.push_back(new LanWaveObj("camera"));
			// index of camera in objects vector
			//cout << "Should be 0: " << singleVis->objects.size() -1 << endl;
			//int c = singleVis->objects.size() -1;

			singleVis->objects[0]->setModelID(-2);

			if(attr = vis_node->first_attribute("cpx")) singleVis->objects[0]->givePosX(attr->value());	else singleVis->objects[0]->givePosX("-1");
			if(attr = vis_node->first_attribute("cpy")) singleVis->objects[0]->givePosY(attr->value());	else singleVis->objects[0]->givePosY("2");
			if(attr = vis_node->first_attribute("cpz")) singleVis->objects[0]->givePosZ(attr->value());	else singleVis->objects[0]->givePosZ("10");

			//cout << "Given camera positions..." << endl;

			if(attr = vis_node->first_attribute("clx")) singleVis->objects[0]->giveSclX(attr->value());	else singleVis->objects[0]->giveSclX("0");
			if(attr = vis_node->first_attribute("cly")) singleVis->objects[0]->giveSclY(attr->value());	else singleVis->objects[0]->giveSclY("0");
			if(attr = vis_node->first_attribute("clz")) singleVis->objects[0]->giveSclZ(attr->value());	else singleVis->objects[0]->giveSclZ("0");

			//cout << "Given camera target position..." << endl;

			if(attr = vis_node->first_attribute("crx")) singleVis->objects[0]->giveRotX(attr->value());	else singleVis->objects[0]->giveRotX("0");
			if(attr = vis_node->first_attribute("cry")) singleVis->objects[0]->giveRotY(attr->value());	else singleVis->objects[0]->giveRotY("1");
			if(attr = vis_node->first_attribute("crz")) singleVis->objects[0]->giveRotZ(attr->value());	else singleVis->objects[0]->giveRotZ("0");

			//cout << "Given camera rotation..." << endl;

			if(attr = vis_node->first_attribute("r")) singleVis->objects[0]->giveColR(attr->value());	else singleVis->objects[0]->giveColR("0");
			if(attr = vis_node->first_attribute("g")) singleVis->objects[0]->giveColG(attr->value());	else singleVis->objects[0]->giveColG("0");
			if(attr = vis_node->first_attribute("b")) singleVis->objects[0]->giveColB(attr->value());	else singleVis->objects[0]->giveColB("0");

			//cout << "Given camera colours..." << endl;

			//cout << "FTMUntar: Camera done." << endl;

			rapidxml::xml_node<> *obj_node = singleVis->manifest->first_node("visualisation")->first_node("object");

			int o_cnt = 1;
			for (rapidxml::xml_node<> *curr_obj = obj_node;
			     curr_obj; curr_obj = curr_obj->next_sibling(), o_cnt++)
			{
				string name(curr_obj->name());
				if(name.compare("light")==0)
				{
					singleVis->objects.push_back(new LanWaveObj("light"));

					if(attr = curr_obj->first_attribute("px")) singleVis->objects[o_cnt]->givePosX(attr->value());	else singleVis->objects[o_cnt]->givePosX("5");
					if(attr = curr_obj->first_attribute("py")) singleVis->objects[o_cnt]->givePosY(attr->value());	else singleVis->objects[o_cnt]->givePosY("5");
					if(attr = curr_obj->first_attribute("pz")) singleVis->objects[o_cnt]->givePosZ(attr->value());	else singleVis->objects[o_cnt]->givePosZ("5");

					if(attr = curr_obj->first_attribute("r")) singleVis->objects[o_cnt]->giveColR(attr->value());	else singleVis->objects[o_cnt]->giveColR("1");
					if(attr = curr_obj->first_attribute("g")) singleVis->objects[o_cnt]->giveColG(attr->value());	else singleVis->objects[o_cnt]->giveColG("1");
					if(attr = curr_obj->first_attribute("b")) singleVis->objects[o_cnt]->giveColB(attr->value());	else singleVis->objects[o_cnt]->giveColB("1");

					//cout << "FTMUntar: Light done." << endl;
				}
				else
				{
					//cout << "Creating an object of type " << obj_node->value() << endl;
					singleVis->objects.push_back(new LanWaveObj(curr_obj->value()));

					if(attr = curr_obj->first_attribute("px")) singleVis->objects[o_cnt]->givePosX(attr->value());	else singleVis->objects[o_cnt]->givePosX("0");
					if(attr = curr_obj->first_attribute("py")) singleVis->objects[o_cnt]->givePosY(attr->value());	else singleVis->objects[o_cnt]->givePosY("0");
					if(attr = curr_obj->first_attribute("pz")) singleVis->objects[o_cnt]->givePosZ(attr->value());	else singleVis->objects[o_cnt]->givePosZ("0");

					//cout << "Given all positions..." << endl;

					if(attr = curr_obj->first_attribute("sx")) singleVis->objects[o_cnt]->giveSclX(attr->value());	else singleVis->objects[o_cnt]->giveSclX("1");
					if(attr = curr_obj->first_attribute("sy")) singleVis->objects[o_cnt]->giveSclY(attr->value());	else singleVis->objects[o_cnt]->giveSclY("1");
					if(attr = curr_obj->first_attribute("sz")) singleVis->objects[o_cnt]->giveSclZ(attr->value());	else singleVis->objects[o_cnt]->giveSclZ("1");

					//cout << "Given all scales..." << endl;

					if(attr = curr_obj->first_attribute("rx")) singleVis->objects[o_cnt]->giveRotX(attr->value());	else singleVis->objects[o_cnt]->giveRotX("0");
					if(attr = curr_obj->first_attribute("ry")) singleVis->objects[o_cnt]->giveRotY(attr->value());	else singleVis->objects[o_cnt]->giveRotY("0");
					if(attr = curr_obj->first_attribute("rz")) singleVis->objects[o_cnt]->giveRotZ(attr->value());	else singleVis->objects[o_cnt]->giveRotZ("0");

					//cout << "Given all rotations..." << endl;

					if(attr = curr_obj->first_attribute("r")) singleVis->objects[o_cnt]->giveColR(attr->value());	else singleVis->objects[o_cnt]->giveColR("1");
					if(attr = curr_obj->first_attribute("g")) singleVis->objects[o_cnt]->giveColG(attr->value());	else singleVis->objects[o_cnt]->giveColG("1");
					if(attr = curr_obj->first_attribute("b")) singleVis->objects[o_cnt]->giveColB(attr->value());	else singleVis->objects[o_cnt]->giveColB("1");

					//cout << "Given all colours..." << endl;

					if(attr = curr_obj->first_attribute("wf"))
					{
						string wf_val = curr_obj->first_attribute("wf")->value();
						if((wf_val).compare("0") != 0) singleVis->objects[o_cnt]->setWireframe(true);
					}
					else
						singleVis->objects[o_cnt]->setWireframe(false);

					//cout << "FTMUntar: Object done." << endl;

					//cout << "Finished loop count " << o_cnt << endl;
				}

			}
			// LOOP UNTIL

			output.push_back(singleVis);
		}
	}
	
	return output;
}

void ftm_untar::error( string msg )
{
	cout << "Error: " << msg;
	exit(0);
}

std::string ftm_untar::cstr_to_str(const char * str)
{
	string tmp_str(str);
	return tmp_str;
}
/*
int main()
{
	string path = "";
	string ext = ".vis";
	ftm_untar* tar = new ftm_untar( path.c_str(), ext.c_str() );

	vector<ftm_untar::vis_details*> vis = tar->get_vis_details();
	tar->set_file( 0 );
	
	//cout << tar->extract_sub_file( "main.lua" );

	tar->delete_vis_details(vis);

	delete tar;
}*/