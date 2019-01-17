/*
 * Copyright (C) 2018 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fileplayer.h"
#include <string>
#include <cstring>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <json-c/json.h>

#undef DEBUG

struct set_role_data
{
	bool state;
	std::string output;
	std::condition_variable cv;
};

static void on_hangup(void *closure, struct afb_wsj1 *wsj)
{
}

static void on_call(void *closure, const char *api, const char *verb, struct afb_wsj1_msg *msg)
{
}

static void on_event(void* closure, const char* event, struct afb_wsj1_msg *msg)
{
}

static void on_reply(void *closure, struct afb_wsj1_msg *msg)
{
	struct set_role_data *data = (struct set_role_data*) closure;
	struct json_object* reply;

	if(!(data && data->state))
		goto reply_done;

	// We opened the role, return the output
	reply = afb_wsj1_msg_object_j(msg);
	if(reply) {
#ifdef DEBUG
		std::cerr << __FUNCTION__ << ": reply = " << \
			json_object_to_json_string_ext(reply, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY) << \
			std::endl;
#endif
		struct json_object* response;
		int rc = json_object_object_get_ex(reply, "response", &response);
		if(rc) {
			struct json_object* val;
			rc = json_object_object_get_ex(response, "device_uri", &val);
			if (rc && json_object_get_string_len(val)) {
				const char* jres_pcm = json_object_get_string(val);
				data->output = jres_pcm;
#ifdef DEBUG
				std::cerr << __FUNCTION__ << ": output = " << jres_pcm << std::endl;
#endif
			}
		}
	}
reply_done:
	// Signal reply is done
	data->cv.notify_one();
}

static void *afb_loop_thread(struct sd_event* loop)
{
	for(;;)
		sd_event_run(loop, 30000000);
}

static void *gst_loop_thread(GMainLoop *loop)
{
	if(loop)
		g_main_loop_run(loop);
}

FilePlayer::FilePlayer(const int port, const std::string &token, const std::string &path, const std::string &role) :
	m_path(path),
	m_role(role)
{
	std::string uri;

	if(sd_event_new(&m_afb_loop) < 0) {
		std::cerr << __FUNCTION__ << ": Failed to create event loop" << std::endl;
		return;
	}

	// Initialize interface for websocket
	m_itf.on_hangup = on_hangup;
	m_itf.on_call = on_call;
	m_itf.on_event = on_event;

	uri = "ws://localhost:" + std::to_string(port) + "/api?token=" + token;
#ifdef DEBUG
	std::cerr << "Using URI: " << uri << std::endl;
#endif
	m_ws = afb_ws_client_connect_wsj1(m_afb_loop, uri.c_str(), &m_itf, NULL);
	if(m_ws) {
		m_afb_thread = std::thread(afb_loop_thread, m_afb_loop);
	} else {
		std::cerr << __FUNCTION__ << ": Failed to create websocket connection" << std::endl;
		goto error;
	}

        // Initialize GStreamer
        gst_init(NULL, NULL);

	// Create elements we need
	m_playbin = gst_element_factory_make("playbin", "play");
	m_alsa_sink = gst_element_factory_make("alsasink", NULL);
	if(!(m_playbin && m_alsa_sink))
		goto error;

	// Set up bus callback
	m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
	if(!m_bus)
		goto error;
	m_gst_loop = g_main_loop_new(NULL, FALSE);
	if(!m_gst_loop)
		goto error;
	gst_bus_add_watch(m_bus, gstreamer_bus_callback, this);

	// Start thread to run glib main loop for gstreamer bus
	m_gst_thread = std::thread(gst_loop_thread, m_gst_loop);

	m_valid = true;
	return;
error:
	gst_object_unref(m_playbin);
	m_playbin = nullptr;
	gst_object_unref(m_alsa_sink);
	m_alsa_sink = nullptr;
	gst_object_unref(m_bus);
	m_bus = nullptr;

	if(m_gst_loop) {
		g_main_loop_quit(m_gst_loop);
		m_gst_loop = nullptr;
	}
	if(m_afb_loop) {
		sd_event_unref(m_afb_loop);
		m_afb_loop = nullptr;
	}
	return;
}

FilePlayer::~FilePlayer(void)
{
	gst_element_set_state(m_playbin, GST_STATE_NULL);
	gst_object_unref(m_playbin);
	gst_object_unref(m_alsa_sink);
	gst_object_unref(m_bus);
	sd_event_unref(m_afb_loop);
}

void FilePlayer::play(bool loop)
{
	std::string output;

	if(!m_valid || m_playing)
		return;

	if(set_role_state(true, &output) != 0)
		return;

	if(output.empty())
		return;

	m_playing = true;
	m_looping = loop;

	g_object_set(m_alsa_sink, "device", output.c_str(), NULL);
	g_object_set(m_playbin, "audio-sink", m_alsa_sink, NULL);
	std::string uri = "file://" + m_path;
	g_object_set(m_playbin, "uri", uri.c_str(), NULL);

	// Start playback
	gst_element_set_state(m_playbin, GST_STATE_PLAYING);

	return;
}

void FilePlayer::stop(void)
{
	if(!(m_valid && m_playing))
		return;

	// Stop playback
	gst_element_set_state(m_playbin, GST_STATE_PAUSED);
}

gboolean FilePlayer::gstreamer_bus_callback(GstBus *bus, GstMessage *msg, gpointer data)
{
	if(!data)
		return TRUE;

	return static_cast<FilePlayer*>(data)->bus_callback(bus, msg);
}

int FilePlayer::set_role_state(bool state, std::string *output)
{
	if(!m_valid)
		return -1;

	set_role_data data;
	data.state = state;
	json_object *jsonData = json_object_new_object();
	json_object_object_add(jsonData, "action", json_object_new_string(state ? "open" : "close"));
	int rc = afb_wsj1_call_j(m_ws, "ahl-4a", m_role.c_str(), jsonData, on_reply, (void*) &data);
	if(rc >= 0) {
		// Wait for response
		std::mutex m;
		std::unique_lock<std::mutex> lk(m);
		data.cv.wait(lk);
		if(state && output)
			*output = data.output;
	} else {
		std::cerr << __FUNCTION__ <<  ": Failed to call ahl-4a/" << m_role.c_str() << std::endl;
	}
	return rc;
}

gboolean FilePlayer::bus_callback(GstBus *bus, GstMessage *msg)
{
	switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
#ifdef DEBUG
		std::cerr << __FUNCTION__ << ": GST_MESSAGE_EOS" << std::endl;
#endif
		if(m_looping) {
			// restart playback if at end
			if (!gst_element_seek(m_playbin, 
					      1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
					      GST_SEEK_TYPE_SET, 0,
					      GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
				std::cerr << "Seek failed!" << std::endl;
			}
		} else {
			// Move stream to paused state since we're done
			gst_element_set_state(m_playbin, GST_STATE_PAUSED);
		}
		break;
	case GST_MESSAGE_STATE_CHANGED: {
		if((GstElement*) GST_MESSAGE_SRC(msg) != m_playbin)
			break;

		GstState old_state, new_state;
		gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
#ifdef DEBUG
		std::cerr << __FUNCTION__ << ": GST_MESSAGE_STATE_CHANGE: " << (int) old_state << " to " << (int) new_state << std::endl;
#endif
		if(old_state == GST_STATE_PLAYING && new_state == GST_STATE_PAUSED) {
			set_role_state(false);

			// Seek back to beginning so any subsequent play starts there
			if (!gst_element_seek(m_playbin, 
					      1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
					      GST_SEEK_TYPE_SET, 0,
					      GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
				std::cerr << "Seek failed!" << std::endl;
			}

			m_playing = false;
		} else if(old_state == GST_STATE_READY && new_state == GST_STATE_NULL) {
			// clean up
			g_main_loop_quit(m_gst_loop);
		}
		break;
	}
        default:
		break;
	}
	return TRUE;
}
