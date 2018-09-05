#include "stdafx.h"

// I am foo_first_test and these are *my* GUIDs
// Make your own when reusing code or else
static const GUID g_mainmenu_group_id = { 0x37eeb41d, 0x8ddd, 0x4bd7, { 0x8a, 0x7c , 0x70, 0x72, 0xb0, 0xa1, 0x3f, 0x17 } };

static mainmenu_group_popup_factory g_mainmenu_group(g_mainmenu_group_id, mainmenu_groups::view, mainmenu_commands::sort_priority_dontcare, "Timestamp Control Server");

void run_timestamp_state_dialog(); // timestamp_state.cpp

class mainmenu_commands_sample : public mainmenu_commands {
public:
	enum {
		cmd_test = 0,
		cmd_timestampstate,
		cmd_total
	};
	t_uint32 get_command_count() {
		return cmd_total;
	}
	GUID get_command(t_uint32 p_index) {
		static const GUID guid_test = { 0x7c4726df, 0x3b2d, 0x4c7c, { 0xad, 0xe8, 0x43, 0xd8, 0x46, 0xbe, 0xce, 0xa8 } };
		static const GUID guid_playbackstate = { 0xbd880c51, 0xf0cc, 0x473f, { 0x9d, 0x14, 0xa6, 0x6e, 0x8c, 0xed, 0x25, 0xae } };
		switch(p_index) {
			case cmd_test: return guid_test;
			case cmd_timestampstate: return guid_playbackstate;
			default: uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
	void get_name(t_uint32 p_index,pfc::string_base & p_out) {
		switch(p_index) {
			case cmd_test: p_out = "Test command"; break;
			case cmd_timestampstate: p_out = "Timestamp state"; break;
			default: uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
	bool get_description(t_uint32 p_index,pfc::string_base & p_out) {
		switch(p_index) {
			case cmd_test: p_out = "This is a sample menu command."; return true;
			case cmd_timestampstate: p_out = "Opens the timestamp state dialog."; return true;
			default: uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
	GUID get_parent() {
		return g_mainmenu_group_id;
	}
	void execute(t_uint32 p_index,service_ptr_t<service_base> p_callback) {
		switch(p_index) {
			case cmd_test:
				popup_message::g_show("This is a sample menu command.", "Blah");
				break;
			case cmd_timestampstate:
				run_timestamp_state_dialog();
				break;
			default:
				uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
};

static mainmenu_commands_factory_t<mainmenu_commands_sample> g_mainmenu_commands_sample_factory;
