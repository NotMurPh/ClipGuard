#include "Selection.h" /* For creating xlib seleciton instnaces and handling them easily */

#include <csignal> /* For handeling exit signals */
#include <cstring> /* For getting the signal names */
#include <stdexcept> /* For runtime_error exception */

#include <unistd.h> /* For adding delays */

// Cleans up then exits
void CleanExit ( int signal ) {

	throw runtime_error( ( string )"Either killed or crashed! Reason : " + strsignal( signal ) );

}

// Prints the given warning in stdout
void Log ( string warning ) {

	if ( warning != "" ) printf( "%s\n" , warning.c_str() );

}

// Gets the data ( e.g. targets , contents ) and ownership of the specified selection 
void GetData ( Selection& selection ) {

	// Get the selection's available targets
	string warning = selection.GetTargets();
	Log( warning );

	// Clear the old content before getting the new one
	selection.contents.clear();
	selection.contents_sizes.clear();
	selection.content_formats.clear();

	for ( int i = 0 ; i < selection.targets_count ; i++ ) {

		// Dont try to get content for target TARGETS as that results in a warning
		if ( selection.targets[ i ] == Selection::targets_atom ) {

			selection.contents.push_back( ( unsigned char* )"" );
			selection.contents_sizes.push_back( ( unsigned long )0 );
			selection.content_formats.push_back( 8 );
			continue;

		}

		// Get the content of each target and store it
		warning = selection.GetContent( selection.targets[ i ] );
		Log( warning );

		selection.contents.push_back( selection.content );
		selection.contents_sizes.push_back( selection.content_size );
		selection.content_formats.push_back( selection.content_format );

	}

	// Become the owner of the seleciton so people ask us for the content
	selection.Own();

}

// Main entry
int main ( int arguments_count , char** arguments ) {

	bool using_default_arguments = false;

	try {

		// Making sure to exit cleanly upon reciving these signals
		signal( SIGINT , CleanExit );
		signal( SIGTERM , CleanExit );
		signal( SIGHUP , CleanExit );

		// ( Crash signals )
		signal( SIGSEGV , CleanExit );
		signal( SIGABRT , CleanExit );
		signal( SIGILL , CleanExit );
		signal( SIGFPE , CleanExit );

		// Use the default selections if none is provided by the user
		if ( arguments_count == 1 ) {

			printf( "You didn't provide any Selection names as arguments! Defaulting to PRIMARY and CLIPBOARD\n" );
			arguments = new char*[] { ( char* )"" , ( char* )"PRIMARY" , ( char* )"CLIPBOARD" };
			arguments_count += 2;
			using_default_arguments = true;

		}

		// Initilise selections then get their data and become their owner
		vector<Selection> selections;

		for ( int i = 1 ; i < arguments_count ; i++ ) {

			selections.push_back( Selection( arguments[ i ] ) );

			GetData( selections[ i - 1 ] );

		}

		XEvent event;
		string warning;
		bool found_requested_target;

		// Wait for events to handle
		while ( true ) {

			// Checking for events every 1 millisecond so the program doesn't get blocked by XNextEvent
			if ( !XPending( Selection::display ) ) {

				// This function is in microseconds so 1 miliseconds times 1000
				usleep( 1 * 1000 );
				continue;

			}

			// Read the next event in the queue
			XNextEvent( Selection::display , &event );

			// For each selection in selections do :
			for ( Selection& selection : selections ) {

				// Check the event to see if it's related to this selection
				if ( event.xselectionrequest.selection == selection.selection || event.xselection.selection == selection.selection ) {

					switch ( event.type ) {

						// If we lost ownership of a seleciton get the new data and become the owner again
						case SelectionClear:

							GetData( selection );

						break;

						// Handle requests
						case SelectionRequest:

							XSelectionRequestEvent request_event = event.xselectionrequest;
							found_requested_target = false;

							// If the requestor wants the targets send the targets
							if ( request_event.target == Selection::targets_atom ) selection.SendTargets( &request_event );

							else {

								// Otherwise if we have the target that the requestor wants the content of, send the requested content
								for ( int i = 0 ; i < selection.targets_count ; i++ ) {

									if ( request_event.target == selection.targets[ i ] && request_event.property != None ) {

										warning = selection.SendContent( selection.contents[ i ] , selection.contents_sizes[ i ] , selection.content_formats[ i ] , selection.targets[ i ] , &request_event );
										Log( warning );
										found_requested_target = true;

									}

								}

								// Otherwise deny the request because we dont have what the requestor wants
								if ( !found_requested_target ) selection.SendNotification( &request_event , false );

							}

						break;

					}

					break;

				}

			}

		}

	}

	// Exit cleanly if an handled exception happened
	catch ( exception& exception ) {
		
		if ( using_default_arguments ) delete[] arguments;
		Selection::Shutdown();
		fprintf( stderr , "\n%s\n" , exception.what() );
		return 1;

	}

}
