#include "stdafx.h"

// Declaration of your component's version information
// Since foobar2000 v1.0 having at least one of these in your DLL is mandatory to let the troubleshooter tell different versions of your component apart.
// Note that it is possible to declare multiple components within one DLL, but it's strongly recommended to keep only one declaration per DLL.
// As for 1.1, the version numbers are used by the component update finder to find updates; for that to work, you must have ONLY ONE declaration per DLL. If there are multiple declarations, the component is assumed to be outdated and a version number of "0" is assumed, to overwrite the component with whatever is currently on the site assuming that it comes with proper version numbers.
DECLARE_COMPONENT_VERSION("Timestamp Server", "0.6", "about message goes here");


// This will prevent users from renaming your component around (important for proper troubleshooter behaviors) or loading multiple instances of it.
VALIDATE_COMPONENT_FILENAME("foo_timestamp_server.dll");

class initquit_handler : public initquit {
	virtual void on_init() {
		/*static_api_ptr_t<playback_control_v3> pc;
		console::formatter() << "Position: " << pfc::format_float(pc->playback_get_position(), 0, 2) << " dB";
		popup_message::g_complain("Hello world!");*/
	}
	virtual void on_quit() {}
};
static initquit_factory_t<initquit_handler> foo_initquit;