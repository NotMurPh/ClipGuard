#include "Selection.h" /* For class declarations */

#include <X11/Xatom.h> /* For XA_ATOM */

#include <stdexcept> /* For runtime_error exception */

#include <unistd.h> /* For adding delay */

using namespace std;

// Initialise static/class variables
Display* Selection::display = XOpenDisplay( nullptr );
int Selection::screen = DefaultScreen( Selection::display );
Window Selection::root = RootWindow( Selection::display , Selection::screen );
Window Selection::window = XCreateSimpleWindow( Selection::display , Selection::root , -100 , -100 , 1 , 1 , 0 , 0 , 0 );
Atom Selection::property = XInternAtom( Selection::display , "INPUT" , false );

Atom Selection::targets_atom = XInternAtom( display, "TARGETS" , false );
Atom* Selection::default_targets = new Atom[] { XInternAtom( display , "TARGETS" , false ) , XInternAtom( display , "UTF8_STRING" , false ) };

Atom Selection::incr_atom = XInternAtom( display , "INCR" , false );

XEvent Selection::event;
XEvent Selection::notification_event;

Atom Selection::property_type;
int Selection::property_format;
unsigned long Selection::number_of_items , Selection::remaining_bytes;
vector< unsigned long > Selection::numbers_of_items;
unsigned char* Selection::property_content;
vector< unsigned char* > Selection::property_contents;

// Constructor - Initialise instances
Selection::Selection ( const char* name ) {

	if ( !display ) throw runtime_error( "Failed to connect to X server!" );

	this -> selection = XInternAtom( display , name , false );
	this -> name = name;

}

// Waits 3 seconds for specified notification event to be recived, then returns the event
XEvent Selection::WaitForNotification ( int type , int state = PropertyNewValue , Window window = Selection::window ) {

	// Get the property events for the specified window if the user wants them
	if ( type == PropertyNotify ) {

		XSelectInput( display , window , PropertyChangeMask );

	}

	event = XEvent();

	// Wait 3 seconds for a new event
	for ( int elapsed_miliseconds = 0 ; elapsed_miliseconds != 3000 ; elapsed_miliseconds+= 1 ) {
	
		// Check for new events in 1 milisecond intervals
		if ( !XPending( display ) ) {

			// This function is in microseconds so 1 miliseconds times 1000
			usleep( 1 * 1000 );
			continue;

		}

		// Get the event if it's available
		XNextEvent( display , &event );

		// If the event isn't what the user wants we wait for another event in the remaining time
		if ( event.type != type ) continue;

		switch ( event.type ) {

			// Return the event if the type is SelectionNotify
			case SelectionNotify:

				return event;

			break;

			// If the event type is PropertyNotify check for the specified state before returning
			case PropertyNotify:

				if ( event.xproperty.state == state ) return event;

			// Otherwise wait for another event in the remaining time
			break;

		}

	}

	// If we didn't recive the requested event in time return an empty event with type None
	event = XEvent();
	event.type = None;
	return event;

}

// Gets the available targets of a selection from it's current owner
string Selection::GetTargets () {

	// Request the targets and wait for an awnser
	XConvertSelection( display , selection , targets_atom , property , window , CurrentTime );
	notification_event = WaitForNotification( SelectionNotify );

	// If the owner refused to send the targets in time use default targets
	if ( notification_event.xselection.property == None || notification_event.type == None ) {

		this -> targets = default_targets;
		this -> targets_count = 2;
		return "Owner refused to send TARGETS in time! defaulted to UTF8_STRING";

	}

	// Otherwise read the data and store it
	XGetWindowProperty( display , window , property , 0 , 1024 * sizeof( Atom ) , true , XA_ATOM , &property_type , &property_format , &number_of_items , &remaining_bytes , &property_content );
	this -> targets = ( Atom* )property_content;
	this -> targets_count = number_of_items;
	return "";

}

// Gets the selection content related to the specified target from the current selection owner
string Selection::GetContent ( Atom target ) {
	
	// Refusing if the user wants content for target TARGETS as it has its own method
	if ( target == targets_atom || target == XA_ATOM ) {

		this -> content = ( unsigned char* )"";
		this -> content_size = ( unsigned long )0;
		this -> content_format = 8;
		return "Cowerdly refused to get content for target TARGETS, use GetTargets instead!";

	}

	// Request the content and wait for an awnser
	XConvertSelection( display , selection , target , property , window , CurrentTime );
	notification_event = WaitForNotification( SelectionNotify );

	// If the owner refused to send the content in time store nothing
	if ( notification_event.xselection.property == None || notification_event.type == None ) {

		this -> content = ( unsigned char* )"";
		this -> content_size = ( unsigned long )0;
		this -> content_format = 8;
		return "Owner refused to send the content in time!";

	}

	// Read the content size and type/target
	XGetWindowProperty( display , window , property , 0 , 0 , false , target , &property_type , &property_format , &number_of_items , &remaining_bytes , &property_content );

	// If the content type was INCR do: ( INCR means that the content is large and needs to be broken down to pieces and recived that way )
	if ( property_type ==  incr_atom ) {

		// Make sure that our vecotrs aren't filled with previous content and are empty
		property_contents.clear();
		numbers_of_items.clear();

		// Tell the owner that "we know pls send the pieces" by desperately deleting our window property
		XDeleteProperty( display , window , property );

		// Recive the content in pieces and store it that way
		while ( true ) {

			// Wait for the owner to send the piece 
			notification_event = WaitForNotification( PropertyNotify , PropertyNewValue );

			// Return if the owner didn't send the content piece in time
			if ( notification_event.type == None ) {

				this -> content = ( unsigned char* )"";
				this -> content_size = ( unsigned long )0;
				this -> content_format = 8;
				return "Owner refused to send the content piece in time!";

			}

			// Otherwise read the piece size
			XGetWindowProperty( display , window , property , 0 , 0 , false , target , &property_type , &property_format , &number_of_items , &remaining_bytes , &property_content );

			// Stop reciving pieces if the piece was empty
			if ( remaining_bytes == 0 ) break;

			// Otherwise read it and store it and then ask for a new piece
			XGetWindowProperty( display , window , property , 0 , remaining_bytes , true , target , &property_type , &property_format , &number_of_items , &remaining_bytes , &property_content );
			property_contents.push_back( property_content );
			numbers_of_items.push_back( number_of_items );

		}

		// Signal that we finished reciving, thank you!
		XDeleteProperty( display , window , property );

		// Storing the content as one large fractured piece
		this -> content = property_contents;
		this -> content_size = numbers_of_items;
		this -> content_format = property_format;

	}

	// If the content type/target wasn't INCR
	else {

		// Read and then store the content in one piece
		XGetWindowProperty( display , window , property , 0 , remaining_bytes , true , target , &property_type , &property_format , &number_of_items , &remaining_bytes , &property_content );
		this -> content = property_content;
		this -> content_size = number_of_items;
		this -> content_format = property_format;

	}

	return "";

}

// Sends a SelectionNotify event with the specified approval state
void Selection::SendNotification ( XSelectionRequestEvent* request_event , bool approval ) {

	notification_event.xselection.type = SelectionNotify;
	notification_event.xselection.requestor = request_event -> requestor;
	notification_event.xselection.selection = request_event -> selection;
	notification_event.xselection.target = request_event -> target;
	notification_event.xselection.time = request_event -> time;

	notification_event.xselection.property = None;
	if ( approval == true ) notification_event.xselection.property = request_event -> property;

	XSendEvent( display , request_event -> requestor , true , NoEventMask , &notification_event );

}

// Sends the targets to the requestor
void Selection::SendTargets ( XSelectionRequestEvent* request_event ) {
	
	XChangeProperty( display , request_event -> requestor , request_event -> property , targets_atom , 32 , PropModeReplace , ( unsigned char* )this -> targets , this -> targets_count );
	SendNotification( request_event , true );

}

// Sends the specified content to the requestor
string Selection::SendContent ( content_type content , content_size_type content_size , int content_format , Atom target , XSelectionRequestEvent* request_event ) {

	// Refuse if the user wants to send content with the target TARGETS as it has its own method
	if ( target == targets_atom || target == XA_ATOM ) {

		SendNotification( request_event , false );
		return "Cowerdly refused to send TARGETS, use SendTargets instead!";
	
	}

	// Refuse if the user tries to send mismatched target with the requested target
	if ( target != request_event -> target ) {

		SendNotification( request_event , false );
		return "Cowerdly refused to send the content! Mismatched target with the requested target";

	}

	// If the content isn't INCR, meaning that it isn't large
	if ( holds_alternative< unsigned char* > ( content ) ) {

		// Send the content in one piece and then notify the requestor
		XChangeProperty( display , request_event -> requestor , request_event -> property , target , content_format , PropModeReplace , get<unsigned char*>(content) , get<unsigned long>(content_size) );
		SendNotification( request_event , true );

	}

	// If the content is INCR or large send content as pre made smaller pieces
	else {

		vector< unsigned char* > content_pieces = get< vector<unsigned char*> > ( content );
		vector< unsigned long > content_size_pieces = get< vector<unsigned long> > ( content_size );

		// Telling the requestor that the data is INCR or large
		XChangeProperty( display , request_event -> requestor , request_event -> property , incr_atom , 32 , PropModeReplace , 0 , 0 );
		SendNotification( request_event , true );

		// Wait for the requestor to say yes pls send the pieces
		notification_event = WaitForNotification( PropertyNotify , PropertyDelete , request_event -> requestor );

		// Return if the requestor didn't respond in time
		if ( notification_event.type == None )
			return "Requestor didn't respond in time! most likely becuase he didn't want the INCR contnet";

		// Sending the content in pieces
		for ( int i = 0 ; i < content_pieces.size() ; i++ ) {

			// Send each piece and then wait for the requestor to ask for the next one and repeat until we are out of pieces
			XChangeProperty( display , request_event -> requestor , request_event -> property , target , content_format , PropModeAppend , content_pieces[ i ] , content_size_pieces[ i ] );
			notification_event = WaitForNotification( PropertyNotify , PropertyDelete , request_event -> requestor );

			// Return if the requestor didn't respond in time
			if ( notification_event.type == None )
				return "Requestor didn't respond in time! Either he forgot to respond or he is damn slow";

		}

		// Send an empty content piece to sey "that was the last one have a happy dayyy!"
		XChangeProperty( display , request_event -> requestor , request_event -> property , target , content_format , PropModeReplace , 0 , 0 );

	}

	return "";

}

// Makes you the new owner of this selection
void Selection::Own () {

	XSetSelectionOwner( display , this -> selection , window , CurrentTime );

}

// Shutdowns the class and cleans the memory
void Selection::Shutdown () {

	delete []default_targets;
	XCloseDisplay( display );

}
