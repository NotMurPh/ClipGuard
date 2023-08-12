// This file contains the class declarations so that the other files notice the class upon including this file
#pragma once

#include <X11/Xlib.h> /* X library to handle X selections */

#include <vector> /* For better arrays */
#include <variant> /* For having a variable with more than one type */
#include <string> /* For return type of some methods */

using namespace std;

class Selection {

	public:

		static Display* display;
		static int screen;
		static Window root;
		static Window window;
		static Atom property;

		static Atom targets_atom;
		static Atom* default_targets;

		static Atom incr_atom;

		Atom selection;
		const char* name;

		Atom* targets;
		unsigned long targets_count;

		using content_type = variant< unsigned char* , vector<unsigned char*> >;
		content_type content;
		vector< content_type > contents;

		using content_size_type = variant< unsigned long , vector<unsigned long> >;
		content_size_type content_size;
		vector< content_size_type > contents_sizes;

		int content_format;
		vector< int > content_formats;

		Selection ( const char* selection );

		static XEvent WaitForNotification ( int type , int state , Window window );

		string GetTargets ();

		string GetContent ( Atom target );

		static void SendNotification ( XSelectionRequestEvent* request_event , bool approval );

		void SendTargets ( XSelectionRequestEvent* request_event );

		string SendContent ( content_type content , content_size_type content_size , int content_format , Atom target , XSelectionRequestEvent* request_event );

		void Own ();

		static void Shutdown ();

	private:

		static XEvent event;
		static XEvent notification_event;

		static Atom property_type;
		static int property_format;
		static unsigned long number_of_items , remaining_bytes;
		static vector< unsigned long > numbers_of_items;
		static unsigned char* property_content;
		static vector< unsigned char* > property_contents;

};

