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

#ifndef FILEPLAYER_H
#define FILEPLAYER_H

#include <string>
#include <thread>
#include <gst/gst.h>

extern "C"
{
#include <afb/afb-wsj1.h>
#include <afb/afb-ws-client.h>
#include <systemd/sd-event.h>
}

class FilePlayer
{
public:
	FilePlayer(int port, const std::string &token, const std::string &path, const std::string &role);
	~FilePlayer();

	void play(bool loop = false);
	void stop(void);

	static gboolean gstreamer_bus_callback(GstBus *bus, GstMessage *msg, gpointer data);

private:
	std::string m_path;
	std::string m_role;
	struct afb_wsj1 *m_ws = nullptr;
	struct afb_wsj1_itf m_itf;
	std::thread m_afb_thread;
	sd_event *m_afb_loop = nullptr;
	std::thread m_gst_thread;
	GMainLoop *m_gst_loop = nullptr;
	GstElement *m_playbin = nullptr;
	GstElement *m_alsa_sink = nullptr;
	GstBus *m_bus = nullptr;
	bool m_valid = false;
	bool m_playing = false;
	bool m_looping = false;

	int set_role_state(bool state, std::string *output = nullptr);
	gboolean bus_callback(GstBus *bus, GstMessage *msg);
};

#endif // FILEPLAYER_H
