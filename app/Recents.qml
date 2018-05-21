/*
 * Copyright (C) 2018 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import AGL.Demo.Controls 1.0
import CallType 1.0
import 'models'

Item {
    id: root

    function log_icon(type) {
        if (type == CallType.Missed) {
            return './images/ic_call_missed_48px.svg'
        } else if (type == CallType.Incoming) {
            return './images/ic_call_received_48px.svg'
        } else if (type == CallType.Outgoing) {
            return './images/ic_call_made_48px.svg'
        }
    }

    signal call(var cname, var cnumber)

    ListView {
        anchors.fill: parent
        anchors.margins: 100

        model: CallHistoryModel { id: history }

        delegate: MouseArea {
            width: ListView.view.width
            height: width / 5
            RowLayout {
                anchors.fill: parent
                spacing: 50
                Image {
                    source: log_icon(model.type)
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: 150
                }
                Image {
                    source: './images/HMI_Phone_Contact_BlankPhoto.svg'
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: 150
                }
                ColumnLayout {
                    Label {
                        Layout.fillWidth: true
                        color: '#59FF7F'
                        font.pixelSize: 50
                        text: model.name
                    }

                    Label {
                        Layout.fillWidth: true
                        font.pixelSize: 50
                        text: new Date(model.time).toLocaleString(Qt.locale(), "ddd MMM d 'at' HH':'mm")
                    }
                }
            }
            onClicked: root.call(model.name, model.number)
        }
    }
}
