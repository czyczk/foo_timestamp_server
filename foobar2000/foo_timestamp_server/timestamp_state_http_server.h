#pragma once
#include "server_http.hpp"
#include "stdafx.h"

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#ifdef HAVE_OPENSSL
#include "crypto.hpp"
#endif

using namespace std;
using namespace boost::property_tree;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class ResponseTemplate {
private:
	static const std::string access_control_allow_base;
	static const std::string ok_base;
	static const std::string no_content_base;
	static const std::string not_found_base;
	static const std::string options_base;
	
public:
	static std::string ok(std::unique_ptr<std::string> message = nullptr, bool inJson = true) {
		std::string base = ok_base + access_control_allow_base;
		if (message == nullptr) {
			return base + "\r\n";
		}
		else {
			return (boost::format("%sContent-Length: %d\r\n\r\n%s") % base % message->length() % *message)
				.str();
		}
	}

	static std::string ok(double num) {
		std::string base = ok_base + access_control_allow_base;
		std::string num_str = (boost::format("%g") % num).str();
		return (boost::format("%sContent-Length: %d\r\n\r\n%s") % base % num_str.length() % num_str)
			.str();
	}

	static std::string no_content() {
		return no_content_base + access_control_allow_base + "\r\n";
	}

	static std::string not_found() {
		std::string base = not_found_base + access_control_allow_base;
		std::string reason = "Unknown command.";
		auto length = reason.length();
		return (boost::format("%sContent-Length: %d\r\n\r\n%s") % base % length % reason)
				.str();
	}

	static std::string options() {
		return ok_base + options_base + "\r\n";
	}
};

const std::string ResponseTemplate::access_control_allow_base = "Access-Control-Allow-Origin: *\r\n";
const std::string ResponseTemplate::ok_base = "HTTP/1.1 200 OK\r\n";
const std::string ResponseTemplate::no_content_base = "HTTP/1.1 204 No Content\r\n";
const std::string ResponseTemplate::not_found_base = "HTTP/1.1 404 Not Found\r\n";
const std::string ResponseTemplate::options_base = ResponseTemplate::access_control_allow_base +
"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n\
Access-Control-Allow-Headers: Content-Type\r\n\
Access-Control-Max-Age: 86400\r\n";

// Callbacks for playback
namespace PlaybackCallback {
	class PlaybackControlCallback {
	protected:
		static_api_ptr_t<playback_control> m_pc;
	};

	template <typename T>
	class PromisedCallback {
	protected:
		std::promise<T> m_promise;
	public:
		virtual void initialize() {
			m_promise = { };
		}

		virtual std::future<T> get_future() {
			return m_promise.get_future();
		}
	};

	// Callback for "Get timestamp"
	class GetTimestampCallback : public main_thread_callback, PlaybackControlCallback, public PromisedCallback<double> {
	public:
		virtual void callback_run() {
			m_promise.set_value(m_pc->playback_get_position());
		}
	};

	// Callback for "Get playback state"
	class GetPlaybackStateCallback : public main_thread_callback, PlaybackControlCallback, public PromisedCallback<std::string> {
	public:
		virtual void callback_run() {
			std::string result;
			if (m_pc->is_paused()) {
				result = "paused";
			}
			else if (m_pc->is_playing()) {
				result = "playing";
			}
			else {
				result = "idle";
			}
			m_promise.set_value(result);
		}
	};

	// Callback for "Play previous track".
	class PlayPreviousTrackCallback : public main_thread_callback, PlaybackControlCallback {
	public:
		virtual void callback_run() {
			m_pc->previous();
		}
	};

	// Callback for "Play or pause"
	class PlayOrPauseCallback : public main_thread_callback, PlaybackControlCallback {
	public:
		virtual void callback_run() {
			m_pc->play_or_pause();
		}
	};

	// Callback for "Stop"
	class StopCallback : public main_thread_callback, PlaybackControlCallback {
	public:
		virtual void callback_run() {
			m_pc->stop();
		}
	};

	// Callback for "Play previous track"
	class PlayNextTrackCallback : public main_thread_callback, PlaybackControlCallback {
	public:
		virtual void callback_run() {
			m_pc->next();
		}
	};
}

std::shared_ptr<HttpServer> start_server(unsigned short port) {
	using namespace PlaybackCallback;

	std::shared_ptr<HttpServer> server = std::make_shared<HttpServer>();
	server->config.port = port;

	static_api_ptr_t<main_thread_callback_manager> callback_manager;
	service_ptr_t<GetTimestampCallback> get_timestamp_callback = fb2k::service_new<GetTimestampCallback>();
	service_ptr_t<GetPlaybackStateCallback> get_playback_state_callback = fb2k::service_new<GetPlaybackStateCallback>();
	service_ptr_t<PlayPreviousTrackCallback> play_previous_track_callback = fb2k::service_new<PlayPreviousTrackCallback>();
	service_ptr_t<PlayOrPauseCallback> play_or_pause_callback = fb2k::service_new<PlayOrPauseCallback>();
	service_ptr_t<StopCallback> stop_callback = fb2k::service_new<StopCallback>();
	service_ptr_t<PlayNextTrackCallback> play_next_track_callback = fb2k::service_new<PlayNextTrackCallback>();

	// Add resources using path-regex and method-string, and an anonymous function
	// [Example] GET - /hello
	server->resource["^/api/v1/hello$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		// Retrieve string:
		//auto content = request->content.string();
		// request->content.string() is a convenience function for:
		// stringstream ss;
		// ss << request->content.rdbuf();
		// auto content=ss.str();
		
		string message = "Hello! I received your GET request.";
		ptree root;
		root.put("msg", message);
		std::ostringstream oss;
		write_json(oss, root);
		auto jsonStr = std::make_unique<std::string>(oss.str());

		*response << ResponseTemplate::ok(std::move(jsonStr));
	};

	// [Example] POST - /hello
	server->resource["^/api/v1/hello$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		// Retrieve string:
		//auto content = request->content.string();
		// request->content.string() is a convenience function for:
		// stringstream ss;
		// ss << request->content.rdbuf();
		// auto content=ss.str();

		string message = "Hello! I received your POST request.";
		ptree root;
		root.put("msg", message);
		std::ostringstream oss;
		write_json(oss, root);
		auto jsonStr = std::make_unique<std::string>(oss.str());
		
		*response << ResponseTemplate::ok(std::move(jsonStr));
	};

	// [Example] GET - /no-content
	server->resource["^/api/v1/no-content$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		*response << ResponseTemplate::no_content();
	};

	// ! When using main_thread_callback, the variables must be captured by value (not by reference) or the program will crash.

	// OPTIONS - handle preflights (allowing CORS)
	server->resource["^/api/v1.*"]["OPTIONS"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		*response << ResponseTemplate::options();
	};

	// GET - /playback/timestamp
	// Return: Playback position in double
	server->resource["^/api/v1/playback/timestamp$"]["GET"] = [callback_manager, get_timestamp_callback](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		get_timestamp_callback->initialize();
		auto future = get_timestamp_callback->get_future();
		callback_manager->add_callback(get_timestamp_callback);

		double position = future.get();
		ptree root;
		root.put("msg", position);
		std::ostringstream oss;
		write_json(oss, root);
		auto jsonStr = std::make_unique<std::string>(oss.str());

		*response << ResponseTemplate::ok(std::move(jsonStr));
	};

	// GET - /playback/state
	// Return: "idle" | "paused" | "playing"
	server->resource["^/api/v1/playback/state$"]["GET"] = [callback_manager, get_playback_state_callback](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		get_playback_state_callback->initialize();
		auto future = get_playback_state_callback->get_future();
		callback_manager->add_callback(get_playback_state_callback);

		string state = future.get();
		ptree root;
		root.put("msg", state);
		std::ostringstream oss;
		write_json(oss, root);
		auto jsonStr = std::make_unique<std::string>(oss.str());

		*response << ResponseTemplate::ok(std::move(jsonStr));
	};

	// POST - /playback/previous
	server->resource["^/api/v1/playback/previous$"]["POST"] = [callback_manager, play_previous_track_callback](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		callback_manager->add_callback(play_previous_track_callback);
		*response << ResponseTemplate::no_content();
	};

	// POST - /playback/play-pause
	server->resource["^/api/v1/playback/play-pause$"]["POST"] = [callback_manager, play_or_pause_callback](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		callback_manager->add_callback(play_or_pause_callback);
		*response << ResponseTemplate::no_content();
	};

	// POST - /playback/stop
	server->resource["^/api/v1/playback/stop$"]["POST"] = [callback_manager, stop_callback](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		callback_manager->add_callback(stop_callback);
		*response << ResponseTemplate::no_content();
	};

	// POST - /playback/next
	server->resource["^/api/v1/playback/next"]["POST"] = [callback_manager, play_next_track_callback](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		callback_manager->add_callback(play_next_track_callback);
		*response << ResponseTemplate::no_content();
	};



	// POST-example for the path /json, responds firstName+" "+lastName from the posted json
	// Responds with an appropriate error message if the posted json is not valid, or if firstName or lastName is missing
	// Example posted json:
	// {
	//   "firstName": "John",
	//   "lastName": "Smith",
	//   "age": 25
	// }
	server->resource["^/json$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		try {
			ptree pt;
			read_json(request->content, pt);

			auto name = pt.get<string>("firstName") + " " + pt.get<string>("lastName");

			*response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Length: " << name.length() << "\r\n\r\n"
				<< name;
		}
		catch (const exception &e) {
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
				<< e.what();
		}


		// Alternatively, using a convenience function:
		// try {
		//     ptree pt;
		//     read_json(request->content, pt);

		//     auto name=pt.get<string>("firstName")+" "+pt.get<string>("lastName");
		//     response->write(name);
		// }
		// catch(const exception &e) {
		//     response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
		// }
	};

	// GET-example for the path /match/[number], responds with the matched string in path (number)
	// For instance a request GET /match/123 will receive: 123
	server->resource["^/match/([0-9]+)$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		response->write(request->path_match[1].str());
	};

	// GET-example simulating heavy work in a separate thread
	server->resource["^/work$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> /*request*/) {
		thread work_thread([response] {
			this_thread::sleep_for(chrono::seconds(5));
			response->write("Work done");
		});
		work_thread.detach();
	};

	// Default GET-example. If no other matches, this anonymous function will be called.
	// Will respond with content in the web/-directory, and its subdirectories.
	// Default file: index.html
	// Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
	server->default_resource["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {

#ifdef HAVE_OPENSSL
//    Uncomment the following lines to enable ETag
//    {
//      ifstream ifs(path.string(), ifstream::in | ios::binary);
//      if(ifs) {
//        auto hash = SimpleWeb::Crypto::to_hex_string(SimpleWeb::Crypto::md5(ifs));
//        header.emplace("ETag", "\"" + hash + "\"");
//        auto it = request->header.find("If-None-Match");
//        if(it != request->header.end()) {
//          if(!it->second.empty() && it->second.compare(1, hash.size(), hash) == 0) {
//            response->write(SimpleWeb::StatusCode::redirection_not_modified, header);
//            return;
//          }
//        }
//      }
//      else
//        throw invalid_argument("could not read file");
//    }
#endif
		*response << "HTTP/1.1 404 Page Not Found\r\n\r\n"
			<< "Unknown command.";
	};

	server->on_error = [](shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
		// Handle errors here
		// Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
	};

	thread server_thread([&server]() {
		// Start server
		server->start();
	});

	server_thread.detach();
	return server;
}

void stop_server(std::shared_ptr<HttpServer> server) {
	server->stop();
}
