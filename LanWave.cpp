/*************************************************************************************************
 *		LanWave
 *
 ************************************************************************************************/
/*
#ifndef Win32
	#define Win32
#endif
*/
#ifndef Win64
	#include "../lib/rapidxml-1.13/rapidxml.hpp"
	#include "../lib/rapidxml-1.13/rapidxml_print.hpp"
#else
	#include "rapid/rapidxml.hpp"
	#include "rapid/rapidxml_print.hpp"
	#include <WinSock2.h>
#endif

#include "MyGLWindow.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_message.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Widget.H>
#include <FL/gl.h>
//#include <fltk/Threads.h>

#include "Capture.hpp"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "LanwaveNetwork.hpp"
#include "ftm_untar.hpp"

#include "global.hpp"

#include <fstream>
#include <algorithm>
#include <vector>

using namespace std;

MyGLWindow *glwind;
MyGLWindow *glwind_fs;

boost::thread* capture_thread;
string device_choose;
LanwaveNetwork *network = new LanwaveNetwork();
bool fullScreen = false;
bool master = false;
bool decided = false;

bool update_vis_for_me = false;
std::string vis_me = "";

boost::thread* master_check_thread;
bool master_check_stop = false;

Fl_Menu_Item *vis_menu;
ftm_untar *tar_obj;
std::vector<ftm_untar::vis_details*> vis_deets;

Fl_Menu_Item *device_menu;
Fl_Menu_Item *scripts;

Fl_Window *wiz_win = 0;
Fl_Wizard *wiz = 0;

// Loading computer images into ram
Fl_PNG_Image png_active("./images/computer_active.png");
Fl_PNG_Image png_inactive("./images/computer_inactive.png");
Fl_PNG_Image png_master("./images/computer_master.png");

void run_capture();

//using namespace std;

struct name_and_length {
	string name;
	int length;
	rapidxml::xml_node<>* manifest;
};

struct vis_details {
	string name;
	string url;
	rapidxml::xml_node<>* manifest;
	//int* manifest;
};

struct struct_pcs_ordered {
	int sorted;
	Fl_Box** pcs;
};

struct_pcs_ordered *str_pc_ord;

string cstr_to_str(const char * str)
{
	string tmp_str(str);
	return tmp_str;
}

int MyGLWindow::handle(int event)
{

	switch(event) 
	{
		case FL_PUSH:
			if(fullScreen)
			{
				if(master)
				{
					cout << "Exiting fullscreen, master." << endl;
					wiz_win->fullscreen_off(10, 10, 800, 600);
			 		wiz->resize(0,0,800,600);
			 		wiz->prev();
		 		}
				else
				{
					cout << "Exiting fullscreen, slave." << endl;
					wiz_win->fullscreen_off(10 , 0, 405, 600);
			 		wiz->resize(0,0,800,600);
			 		wiz->prev();
				}

				fullScreen = false;
			 	
			}
			return 1;
	}
	return Fl_Gl_Window::handle(event);
}

void bt_network_finish_cb(Fl_Widget *w, void* pData)
{
	Fl_Window *networkWindow = reinterpret_cast<Fl_Window*>(pData);
	
	// DELETE ALL THE SHIT
	
	cout << "Finish network map\n";

	// SEND EVERYONE THEIR NEW ID
	for(int i = 0; i < 5; i++)
		network->assign_relative_ids();
/*
	for every node, send their new ID
		const bool setNewRelativePosition( const unsigned short id, const unsigned short newPos );

	then just call this at the end
	void assign_relative_ids( void );
*/	
	networkWindow->hide();
}

//void b_showID_cb(Fl_Widget *w, void* pData);

void broadcast_command(char cmd)
{
	for ( std::map<int, LWNode*>::iterator iter = network->network_map->map.begin(); iter != network->network_map->map.end(); iter++ )
	{
		std::cout << "on node: " << *(iter->second->endpoint) << "\n";

		try
			{
				ip::tcp::endpoint tmp( iter->second->endpoint->address(), DEFAULT_TCP_PORT );
				tcpSession* session = network->tcp->start_session( tmp );

				session->write_command(cmd);

				std::cout << "this is what they say: " << (int)session->read_command() << endl;

				//delete session;
			}
		catch ( std::exception& e )
			{
			}
	}
}

void bt_pcs_unord_cb(Fl_Widget *w, void* pData)
{
	struct_pcs_ordered* str_pc_ord = reinterpret_cast<struct_pcs_ordered*>(pData);

//	cout << "Label is " << ((Fl_Button*)w)->label() << endl; fflush(stdout);
	((Fl_Button*)w)->image(png_inactive);
 	((Fl_Button*)w)->deactivate();
	
	int temp = str_pc_ord->sorted;
//	cout << "Sorted: " << temp << endl;
	str_pc_ord->pcs[temp]->image(png_master);
	str_pc_ord->pcs[temp]->label(((Fl_Button*)w)->label());
	str_pc_ord->pcs[temp]->redraw();
	str_pc_ord->sorted++;

	if(network->get_static_id() == atoi(((Fl_Button*)w)->label()))
	{
		cout << "Static ID is " << 	((Fl_Button*)w)->label() << ", (that's me!) assign relative ID " << temp+1 << endl;
		network->relative_id(temp+1);
	}
	else
	{
		cout << "Static ID is " << 	((Fl_Button*)w)->label() << ", assign relative ID " << temp+1 << endl;
		network->network_map->setNewRelativePosition( atoi(((Fl_Button*)w)->label()), temp+1 );
	}
}

void b_showNetwork_cb(Fl_Widget *w, void* pData)
{
	//broadcast_command(TSCMD_SHOW_SID);
	network->broadcast_command( TSCMD_SHOW_SID );

	int noPCs = (int)network->map->getNumberOfNodes();
//	int noPCs = 12;

	char **templabel = new char*[noPCs];
	
	for(int i = 0; i < noPCs; i++)
	  templabel[i] = new char[3];

	Fl_Window *networkWindow = new Fl_Window(10, 10, 600, 230, "Network Map");

		Fl_Scroll *scroll = new Fl_Scroll(0,0,600,170);
			Fl_Widget *canvas = new Fl_Widget(0,0,52*(noPCs)+40,150);     // purposely make drawing area larger than scroll

			// Draw list of ordered pcs on the screen
			struct_pcs_ordered *str_pc_ord = (struct_pcs_ordered*)malloc(sizeof(struct_pcs_ordered));
			str_pc_ord->sorted = 0;
			str_pc_ord->pcs = (Fl_Box**)malloc(noPCs * sizeof(Fl_Box));
			for(int i = 0; i < noPCs; i++)
			{
				//cout << "Adding PC " << i << " of " << noPCs << " ordered" << endl;
				str_pc_ord->pcs[i] = new Fl_Box(i*52 + 20, 20,32,50);
//				(*(str_pc_ord->pcs[i])).image(png_inactive);
			}

			// Draw list of unordered pcs on the screen
			Fl_Button* pcs_unordered[noPCs];
			for(int i = 0; i < noPCs; i++)
			{
//				char *templabel = new char[3];
				sprintf(templabel[i], "%d", i+1);
				//cout << "Sprintf -> " << templabel[i] << endl;
			  
				pcs_unordered[i] = new Fl_Button(i*52 + 20, 90,32,50, "PC");
				pcs_unordered[i]->label(templabel[i]);
				pcs_unordered[i]->image(png_active);
				pcs_unordered[i]->callback(bt_pcs_unord_cb, str_pc_ord);
				//cout << "Adding PC " << i << " of " << noPCs << " unordered" << endl;
				//cout << "(label is " << pcs_unordered[i]->label() << ")\n";
			}

		scroll->end();
		
		Fl_Button *bt_network_finish = new Fl_Button(300, 185, 160, 25, "Finish");
		bt_network_finish->callback(bt_network_finish_cb, networkWindow);

//   		cout << "sorted is " << (*str_pc_ord->sorted) << endl;
     	networkWindow->end();
//	networkWindow->resizable(canvas);
	networkWindow->show();
}

void setVis(const char* name);

void vis_check_loop( void* )
	{
		if ( update_vis_for_me )
		{
			setVis(vis_me.c_str());
			update_vis_for_me = false;
		}
	}

void setVis(const char* name)
{
	// Convert c string to string
	std::string label(name);

	// Search for vis file with same name
	for(int i = 0; i < vis_deets.size(); i++)
	{
		if(label.compare(vis_deets[i]->name) == 0)
		{
			// Sending (pointer to) this vis deet to glwindow

			glwind->setVis(vis_deets[i], tar_obj, i);
			glwind_fs->setVis(vis_deets[i], tar_obj, i);

			break;
		}
			
	}
}

void distribute_vis(ftm_untar::vis_details* deets, ftm_untar *tar_obj, int selector)
{
	// Send this to all slaves. Each slave will then call setVis with these parameters.
	//fl_message("Distributing vis to slaves...");
}

void ch_vis_cb(Fl_Widget *w, void* pData)
{
	Fl_Menu_ *mw = (Fl_Menu_ *)w;
	const Fl_Menu_Item *m = mw->mvalue();

	// Convert c string to string
	std::string label(m->label());

	// Search for vis file with same name
	for(int i = 0; i < vis_deets.size(); i++)
	{
		if(label.compare(vis_deets[i]->name) == 0)
		{
			// Sending (pointer to) this vis deet to glwindow

			glwind->setVis(vis_deets[i], tar_obj, i);
			glwind_fs->setVis(vis_deets[i], tar_obj, i);

//			setVis(vis_deets[i], tar_obj, i);

			//cout << "Send visname: " << vis_deets[i]->name << endl;

			if (master)
				network->set_vis(vis_deets[i]->name);

			// SEND 'UPDATE VIS'
			// SEND 'Vis Name'

			break;
		}
			
	}
}

void ch_dev_cb(Fl_Widget *w, void* pData)
{
	MyGLWindow* pglwind = reinterpret_cast<MyGLWindow*>(pData);

	Fl_Menu_ *mw = (Fl_Menu_ *)w;
	const Fl_Menu_Item *m = mw->mvalue();

	if(capture_running == true)
	{
		capture_running = false;
		capture_thread->join();
		delete capture_thread;
	}

	device_choose = m->label();

	capture_running = true;
	capture_thread = new boost::thread( boost::bind( run_capture) );

}

void sld_0_cb(Fl_Widget *w, void* pData)
{
	Fl_Value_Slider *x = (Fl_Value_Slider*)w;
	MyGLWindow* pglwind = reinterpret_cast<MyGLWindow*>(pData);
	infoPackage->slide[0] = (int)x->value();
}

void sld_1_cb(Fl_Widget *w, void* pData)
{
	Fl_Value_Slider *x = (Fl_Value_Slider*)w;
	MyGLWindow* pglwind = reinterpret_cast<MyGLWindow*>(pData);
	infoPackage->slide[1] = (int)x->value();
}

void sld_2_cb(Fl_Widget *w, void* pData)
{
	Fl_Value_Slider *x = (Fl_Value_Slider*)w;
	MyGLWindow* pglwind = reinterpret_cast<MyGLWindow*>(pData);
	infoPackage->slide[2] = (int)x->value();
}

void sld_3_cb(Fl_Widget *w, void* pData)
{
	Fl_Value_Slider *x = (Fl_Value_Slider*)w;
	MyGLWindow* pglwind = reinterpret_cast<MyGLWindow*>(pData);
	infoPackage->slide[3] = (int)x->value();
}

void sld_4_cb(Fl_Widget *w, void* pData)
{
	Fl_Value_Slider *x = (Fl_Value_Slider*)w;
	MyGLWindow* pglwind = reinterpret_cast<MyGLWindow*>(pData);
	infoPackage->slide[4] = (int)x->value();
}

Fl_Menu_Item ntwk_optns[] = {
  {"Make alive"},
  {"Change ID", 0, 0, 0, FL_MENU_INACTIVE},
//  {"Ping", 0, 0, 0, FL_MENU_INACTIVE},
  {"Ping"},
  {"Ignore"},
  {0}
};

void wiz_slave_cb(Fl_Widget*, void*);

void tellStaticID(int num)
{
	glwind->tellStaticID(num);
	glwind_fs->tellStaticID(num);
}

void gui_idle(void)
{
	//boost::asio::deadline_timer t( *(network->service), boost::posix_time::milliseconds(20));
	//t.wait();
	// If a master exists
	if(network->is_there_a_master())
	{
		std::cout << "LOOP: there is a master" << std::endl;
		// And I'm not already in slave mode		
		if(decided == false)
		{
			std::cout << "LOOP: going to slave" << std::endl;
			wiz_slave_cb(NULL, NULL);
			decided = true;
			//Fl::remove_idle(gui_idle, NULL);
		}
	}
	// If no master exists, and I'm a slave go back to first screen and await instructions..
	else if(decided == true)
	{
		cout << "Whoa, master yielded!" << endl;
		if(fullScreen)
		{
			cout << "and I'm in fullscreen! Better kick my bum into gear!" << endl;
			wiz_win->fullscreen_off(10, 10, 800, 600);
	 		wiz->resize(0,0,800,600);
	 		wiz->prev();
	 		fullScreen = false;
	 	}

	 	cout << "Now going back to initial screen :)" << endl;
		wiz_win->resize(0, 0, 400, 300);
		wiz->prev();
		decided = false;
	}
}

void yield_master_cb(Fl_Widget*, void*)
{
	//Fl::remove_idle(gui_idle_check_master, NULL);
	delete capture_thread;
	
	wiz->prev();
	wiz_win->resize(0, 0, 400, 300);
	network->rank("slave");
	master = false;

	//master_check_stop = false;
	//master_check_thread = new boost::thread( gui_idle );
}

void wiz_master_cb(Fl_Widget*, void*)
{
	//master_check_stop = true;
	//master_check_thread->join();
	//Fl::remove_idle(gui_idle, NULL);
	master = true;
	decided = true;
	tellStaticID(1);
	wiz->next();
	wiz_win->resize(0, 0, 800, 600);
	network->rank("master");
	//Fl::add_idle(gui_idle, NULL);
}

void wiz_slave_cb(Fl_Widget*, void*)
{
	//cout << "wiz_slave_cb" << endl;
	//Fl::remove_idle(gui_idle_check_master, NULL);
	//cout << "Removing idle.." << endl;
	//Fl::remove_idle(gui_idle, NULL);
	//cout << "Removed" << endl;
	master = false;
	decided = true;
	wiz->next();
	wiz_win->resize(0, 0, 405, 600);
	network->rank("slave");
}

void wiz_fullscreen_cb(Fl_Widget*, void*)
{
	wiz->next();
	wiz_win->fullscreen();
	wiz->resize(0,0, Fl::w(), Fl::h());
	fullScreen = true;
}

void network_message_cb( char cmd, tcpSession* session )
{
	//std::cout << "this is what we got: " << (int)cmd << std::endl;

	if ( cmd == TSCMD_SHOW_SID )
	{
		glwind->showStaticID();
		glwind_fs->showStaticID();
	}

	if ( cmd == TSCMD_UPDATE_SID )
	{
		cout << "network_message_cb got TSCMD_UPDATE_SID, static ID " <<  network->get_static_id() << endl;
		tellStaticID(network->get_static_id());
		session->close();
	}
	else if ( cmd == TSCMD_UPDATE_RID )
	{
		cout << "Newly given RID is " << network->get_relative_id() << endl;
		session->close();
	}
	else if ( cmd == TSCMD_SELECT_VIS )
	{
		try
		{
			int length = session->read_command();
			char* str = session->read( length );
			str[length-1] = 0;
			cout << "Slave received TSCMD_SELECT_VIS " << str << endl;
			
			// setVis(str);
			vis_me = std::string(str);
			update_vis_for_me = true;

			cout << "Slave successfully set vis " << str << endl;
			session->close();
			cout << "Slave closed session" << endl;
		}
		catch ( std::exception& e )
		{
			std::cout << "problem receiving the new vis file to select." << std::endl;
		}
	}
}

void drawScreen()
{
	Fl::scheme("plastic");

	//std::cout << "Screen size: " << Fl::w() << " x " << Fl::h() << std::endl;
	
	Fl_PNG_Image *pc_icon = new Fl_PNG_Image("./images/computer.png");

	wiz_win = new Fl_Window(400,300,"FTM LanWave v1.0");
	wiz = new Fl_Wizard(0,0,800,600);

	//Wizrd: page 1 - Master or slave selection
	{
		Fl_Group *master_selection_page = new Fl_Group(0, 0, 400, 300, "selection");

		Fl_Button *master_selection = new Fl_Button(150, 150, 100, 30,"Master");
		master_selection->callback(wiz_master_cb);

		Fl_Button *slave_selection = new Fl_Button(150, 190, 100, 30,"Slave");
		slave_selection->callback(wiz_slave_cb);

		master_selection_page->end();
	}
	// Wizard: page 3 - Main Page
	{
		cout << "start drawing page 1" << endl;
		Fl_Group *gr_vis_page = new Fl_Group(0 , 0, 800, 600, "Main Page?");

		glwind = new MyGLWindow(5, 5, 395, 555, "Preview");
		glwind->setRatio((float)395/555);

				// Drop-down box for visualisations
				//if(master)
				//{
					cout << "Master. Loading vis files and adding drop-down menu." << endl;

					tar_obj = new ftm_untar("vis", "vis");
					vis_deets = tar_obj->get_vis_details();

					//distribute_vis(vis_deets[i], tar_obj, i);

					vis_menu = (Fl_Menu_Item*)calloc(vis_deets.size()+2, sizeof(Fl_Menu_Item));

					vis_menu[0].label("Visualisations");
					vis_menu[0].shortcut(0);
					vis_menu[0].deactivate();

					for(int i = 1; i <= vis_deets.size(); i++)
					{
						std::string tempname = (vis_deets[i-1]->name);
						vis_menu->add(tempname.c_str(), 0, 0, 0, 0);
					}

					Fl_Choice *ch_vis = new Fl_Choice(270, 565, 125, 25, "&Visualisation:");
	 				ch_vis->menu(vis_menu);
	 				ch_vis->callback(ch_vis_cb, glwind);
				//}
				//else
				//{
					cout << "Slave. Get vis files from master... RIGHT NOW" << endl;
				//}
				

				// Drop-down bow for sound devices
				Capture *C = new Capture ;

				char** devices = C->Initialize();
				int device_count = 0 ;
				while ( devices[device_count] != NULL )
					device_count++;
				
				device_menu = (Fl_Menu_Item*)calloc(device_count+2, sizeof(Fl_Menu_Item));
				device_menu[0].label("Sound Devices");
				device_menu[0].shortcut(0);
				device_menu[0].deactivate();

				for(int i = 1; i <= device_count; i++)
				{
					std::string device_name = (devices[i-1]);
					device_menu->add(device_name.c_str(), 0, 0, 0, 0);
				}

				Fl_Choice *ch_dev = new Fl_Choice(550, 130, 190, 25, "&Sound Devices:");
				ch_dev->menu(device_menu);
				ch_dev->callback(ch_dev_cb, glwind);

				// Slider 0
				Fl_Value_Slider *sld_0 = new Fl_Value_Slider(550, 180, 190, 25, "Slider 0");
				sld_0->align(FL_ALIGN_LEFT);
				sld_0->type(FL_HORIZONTAL);
				sld_0->range(-5,5);
				sld_0->step(1);
				sld_0->callback(sld_0_cb, glwind);

				// Slider 1
				Fl_Value_Slider *sld_1 = new Fl_Value_Slider(550, 210, 190, 25, "Slider 1");
				sld_1->align(FL_ALIGN_LEFT);
				sld_1->type(FL_HORIZONTAL);
				sld_1->range(-5,5);
				sld_1->step(1);
				sld_1->callback(sld_1_cb, glwind);

				// Slider 2
				Fl_Value_Slider *sld_2 = new Fl_Value_Slider(550, 240, 190, 25, "Slider 2");
				sld_2->align(FL_ALIGN_LEFT);
				sld_2->type(FL_HORIZONTAL);
				sld_2->range(-5,5);
				sld_2->step(1);
				sld_2->callback(sld_2_cb, glwind);

				// Slider 3
				Fl_Value_Slider *sld_3 = new Fl_Value_Slider(550, 270, 190, 25, "Slider 3");
				sld_3->align(FL_ALIGN_LEFT);
				sld_3->type(FL_HORIZONTAL);
				sld_3->range(-5,5);
				sld_3->step(1);
				sld_3->callback(sld_3_cb, glwind);

				// Slider 4
				Fl_Value_Slider *sld_4 = new Fl_Value_Slider(550, 300, 190, 25, "Slider 4");
				sld_4->align(FL_ALIGN_LEFT);
				sld_4->type(FL_HORIZONTAL);
				sld_4->range(-5,5);
				sld_4->step(1);
				sld_4->callback(sld_4_cb, glwind);

			//gr_vis->end();

		Fl_Button *back = new Fl_Button(550,350,130,25,"Network"); back->callback(b_showNetwork_cb);
		Fl_Button *yield = new Fl_Button(550,380,130,25,"Yield Master"); yield->callback(yield_master_cb);
		Fl_Button *bt_fs = new Fl_Button(10,565,100,25,"Fullscreen"); bt_fs->callback(wiz_fullscreen_cb);

		gr_vis_page->end();
		cout << "Finish building page 1" << endl;
	}
	// Wizard: page 4 - Full screen
	{
		Fl_Group *gr_fs = new Fl_Group(0, 0, Fl::w(), Fl::h(), "Fulscreen vis");

		glwind_fs = new MyGLWindow(0, -Fl::h()/3, Fl::w(), Fl::h()+100, "Preview");
		glwind_fs->setRatio((float)Fl::w()/Fl::h());

		gr_fs->end();
	}

	wiz->end();
	wiz_win->end();
	wiz_win->show();
}

void run_capture()
{
	Capture *C = new Capture ;
	C->Run(network,(char*)device_choose.c_str());
	capture_running = false;
	C->~Capture();
}

void fl_master_thread(void*)
{
	//master_check_thread = new boost::thread( gui_idle );
//	Fl::remove_idle(fl_master_thread);

	if(network->is_there_a_master())
	{
		std::cout << "LOOP: there is a master" << std::endl;
		// And I'm not already in slave mode		
		if(decided == false)
		{
			std::cout << "LOOP: going to slave" << std::endl;
			wiz_slave_cb(NULL, NULL);
			decided = true;
			//Fl::remove_idle(gui_idle, NULL);
		}
	}
	// If no master exists, and I'm a slave go back to first screen and await instructions..
	else if(decided == true)
	{

		wiz_win->resize(0, 0, 400, 300);
		wiz->prev();
		decided = false;
	}
	Fl::repeat_timeout(1.0, fl_master_thread);
}

// Run program LanWave ip_address (-m/-s) file_name
int main( int argc, const char* argv[] )
{
	capture_running = false;

	drawScreen();

	network->add_receive_handler( boost::bind( &MyGLWindow::dataReceiveHandler, glwind, _1, _2 ) );
	network->add_receive_handler( boost::bind( &MyGLWindow::dataReceiveHandler, glwind_fs, _1, _2 ) );
	network->set_tcp_rx_handler( boost::bind( network_message_cb, _1, _2 ) );
/*
	cout << "Lanwave main, static id is " << network->get_static_id() << endl;
	cout << "telling glwind" <<  endl;
	glwind->tellStaticID(network->get_static_id());
	cout << "telling glwind_fs" <<  endl;
	glwind_fs->tellStaticID(network->get_static_id());
*/
	

	// Commented out only for master/slave testing.
	// Uncomment to perform actual networked master/auto-slave decision
	//Fl::add_idle(gui_idle, NULL);

	Fl::add_idle( vis_check_loop, NULL );
	//Fl::Thread fltkthread;
	//Fl::create_thread( fltkthread, fl_master_thread, NULL );

	Fl::add_timeout( 1.0, fl_master_thread );

	sleep(1);

	Fl::run();

	return 0;
}
