#include "stdafx.h"
#include "resource.h"
#include "timestamp_state_http_server.h"
#include <boost/format.hpp>

void run_timestamp_state_dialog();

class CTimestampStateDialog : public CDialogImpl<CTimestampStateDialog>, private play_callback_impl_base {
public:
	enum { IDD = IDD_TIMESTAMP_STATE };

	BEGIN_MSG_MAP(CTimestampStateDialog)
		MSG_WM_INITDIALOG(on_init_dialog)
		COMMAND_HANDLER_EX(IDCLOSE, BN_CLICKED, on_close)

		COMMAND_HANDLER_EX(IDC_PATTERN, EN_CHANGE, on_pattern_changed)

		COMMAND_HANDLER_EX(IDC_TOGGLE_SERVER, BN_CLICKED, on_toggle_server_clicked)

		COMMAND_HANDLER_EX(IDC_TOGGLE_PAUSE, BN_CLICKED, on_toggle_pause_clicked)
		COMMAND_HANDLER_EX(IDC_STOP, BN_CLICKED, on_stop_clicked)
		COMMAND_HANDLER_EX(IDC_SEEK_BACKWARD, BN_CLICKED, on_seek_backward_clicked)
		COMMAND_HANDLER_EX(IDC_SEEK_FORWARD, BN_CLICKED, on_seek_forward_clicked)
		COMMAND_HANDLER_EX(IDC_SHOW_TIMESTAMP, BN_CLICKED, on_show_timestamp_clicked)
		//MSG_WM_CONTEXTMENU(OnContextMenu)
	END_MSG_MAP()
private:

	// Playback callback methods.
	void on_playback_starting(play_control::t_track_command p_command, bool p_paused) { update(); }
	void on_playback_new_track(metadb_handle_ptr p_track) { update(); }
	void on_playback_stop(play_control::t_stop_reason p_reason) { update(); }
	void on_playback_seek(double p_time) { update(); }
	void on_playback_pause(bool p_state) { update(); }
	void on_playback_edited(metadb_handle_ptr p_track) { update(); }
	void on_playback_dynamic_info(const file_info & p_info) { update(); }
	void on_playback_dynamic_info_track(const file_info & p_info) { update(); }
	void on_playback_time(double p_time) { update(); }
	void on_volume_change(float p_new_val) {}

	void update();

	BOOL on_init_dialog(CWindow, LPARAM);
	void on_close(UINT, int, CWindow);

	void on_pattern_changed(UINT, int, CWindow);

	void on_toggle_server_clicked(UINT, int, CWindow);

	void on_toggle_pause_clicked(UINT, int, CWindow) { m_playback_control->play_or_pause(); }
	void on_stop_clicked(UINT, int, CWindow) { m_playback_control->stop(); }
	void on_seek_backward_clicked(UINT, int, CWindow);
	void on_seek_forward_clicked(UINT, int, CWindow);
	void on_show_timestamp_clicked(UINT, int, CWindow);

	UINT get_seek_step() { return uGetDlgItemInt(IDC_SEEK_STEP); }

	//void OnContextMenu(CWindow wnd, CPoint point);

	bool m_is_server_open = false;
	std::thread::native_handle_type server_thread_handle;
	std::shared_ptr<HttpServer> server;

	titleformat_object::ptr m_script;

	static_api_ptr_t<playback_control> m_playback_control;
};

void run_timestamp_state_dialog() {
	try {
		// ImplementModelessTracking registers our dialog to receive dialog messages thru main app loop's IsDialogMessage().
		// CWindowAutoLifetime creates the window in the constructor (taking the parent window as a parameter) and deletes the object when the window has been destroyed (through WTL's OnFinalMessage).
		new CWindowAutoLifetime<ImplementModelessTracking<CTimestampStateDialog>>(core_api::get_main_window());
	}
	catch (std::exception const & e) {
		popup_message::g_complain("Failed creating dialog.", e);
	}
}

void CTimestampStateDialog::update() {
	if (m_script.is_empty()) {
		pfc::string8 pattern;
		uGetDlgItemText(*this, IDC_PATTERN, pattern);
		static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(m_script, pattern);
	}
	pfc::string_formatter state;
	if (m_playback_control->playback_format_title(NULL, state, m_script, NULL, playback_control::display_level_all)) {
		//Succeeded already.
	}
	else if (m_playback_control->is_playing()) {
		//Starting playback but not done opening the first track yet.
		state = "Opening...";
	}
	else {
		state = "Stopped.";
	}
	uSetDlgItemText(*this, IDC_RESULT, state);
}

BOOL CTimestampStateDialog::on_init_dialog(CWindow, LPARAM) {
	update();
	// Set a default title pattern
	SetDlgItemText(IDC_PATTERN, _T("%codec% | %bitrate% kbps | %samplerate% Hz | %channels% | %playback_time%[ / %length%]$if(%ispaused%, | paused,)"));
	// Set a default seek step
	SetDlgItemInt(IDC_SEEK_STEP, 5);
	// Set a random default port
	SetDlgItemInt(IDC_PORT, 27636);
	::ShowWindowCentered(*this, GetParent()); // Function declared in SDK helpers.
	return TRUE;
}

void CTimestampStateDialog::on_close(UINT, int, CWindow) {
	DestroyWindow();
}

void CTimestampStateDialog::on_pattern_changed(UINT, int, CWindow) {
	m_script.release(); // pattern has changed, force script recompilation
	update();
}

void CTimestampStateDialog::on_toggle_server_clicked(UINT, int, CWindow) {
	// If the server is not open, open the server
	if (!m_is_server_open) {
		SetDlgItemText(IDC_RESULT, _T("Starting server..."));
		// Fetch the port specified
		UINT port = uGetDlgItemInt(IDC_PORT);
		// Start the server
		server = start_server(port);
		uSetDlgItemText(*this, IDC_RESULT, (boost::format("Server started on port %d.") % port).str().c_str());
		uSetDlgItemText(*this, IDC_TOGGLE_SERVER, "Close server");
		m_is_server_open = true;
	}
	// If the server is open, close it
	else {
		stop_server(server);
		/*int termination_result = TerminateThread(server_thread_handle, 0);
		if (!termination_result) {*/
		//uSetDlgItemText(*this, IDC_RESULT, (boost::format("Server closed with code: %d.") % stop_result).str().c_str());
		uSetDlgItemText(*this, IDC_RESULT, "Server closed.");
		uSetDlgItemText(*this, IDC_TOGGLE_SERVER, "Start server");
		m_is_server_open = false;
		//}
	}
}

void CTimestampStateDialog::on_seek_backward_clicked(UINT, int, CWindow) {
	UINT step = get_seek_step();
	m_playback_control->playback_seek_delta(- (int) step);
}

void CTimestampStateDialog::on_seek_forward_clicked(UINT, int, CWindow) {
	UINT step = get_seek_step();
	m_playback_control->playback_seek_delta(step);
}

void CTimestampStateDialog::on_show_timestamp_clicked(UINT, int, CWindow) {
	CString temp;
	temp.Format(_T("%g"), m_playback_control->playback_get_position());
	SetDlgItemText(IDC_RESULT, temp);
}