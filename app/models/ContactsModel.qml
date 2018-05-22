/* Copyright (C) 2015, Jaguar Land Rover. All Rights Reserved.
 * Copyright (C) 2015, The Qt Company. All Rights Reserved.
 * Copyright (C) 2018, Konsulko Group
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQml.Models 2.2
import NumberType 1.0

ListModel {
    function findName(number) {
        for (var i = 0; i < count; i++) {
            var entry = get(i)
            for (var j = 0; j < entry.numbers.length; j++) {
                if (entry.numbers[j].number) {
                    return entry.name
                }
            }
        }
        return "Unknown number"
    }

    ListElement {
        name: "Art McGee"
        numbers: [
            ListElement {number: "503-715-6618"; type: NumberType.Mobile},
            ListElement {number: "972-256-9732"; type: NumberType.Work},
            ListElement {number: "503-616-4940"; type: NumberType.Home}
        ]
    }

    ListElement {
        name: "Dana Jonty"
        numbers: [
            ListElement {number: "503-209-3254"; type: NumberType.Mobile},
            ListElement {number: "972-000-0000"; type: NumberType.Work},
            ListElement {number: "503-000-0000"; type: NumberType.Other}
        ]
    }

    ListElement {
        name: "Jojo Derick"
        numbers: [
            ListElement {number: "503-209-3254"; type: NumberType.Mobile},
            ListElement {number: "972-000-0000"; type: NumberType.Work},
            ListElement {number: "503-000-0000"; type: NumberType.Other}
        ]
    }

    ListElement {
        name: "Kelly Johnson"
        numbers: [
            ListElement {number: "503-000-0000"; type: NumberType.Mobile},
            ListElement {number: "972-000-0000"; type: NumberType.Work},
            ListElement {number: "503-000-0000"; type: NumberType.Home}
        ]
    }

    ListElement {
        name: "Marco Morales"
        numbers: [
            ListElement {number: "503-209-3254"; type: NumberType.Mobile},
            ListElement {number: "972-000-0000"; type: NumberType.Work},
            ListElement {number: "503-000-0000"; type: NumberType.Home}
        ]
    }

    ListElement {
        name: "Rob Brad"
        numbers: [
            ListElement {number: "503-209-3254"; type: NumberType.Mobile},
            ListElement {number: "972-000-0000"; type: NumberType.Work},
            ListElement {number: "503-000-0000"; type: NumberType.Other}
        ]
    }

    ListElement {
        name: "Ted Gilbert"
        numbers: [
            ListElement {number: "503-209-3254"; type: NumberType.Mobile},
            ListElement {number: "972-000-0000"; type: NumberType.Work},
            ListElement {number: "503-101-1001"; type: NumberType.Home},
            ListElement {number: "503-000-0000"; type: NumberType.Other}
        ]
    }
}
