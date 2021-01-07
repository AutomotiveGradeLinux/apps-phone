/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2017,2020 Konsulko Group
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

#include <QtCore/QDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QUrlQuery>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>
#include <QQuickWindow>

#include <pbap.h>
#include <telephony.h>
#include "phone.h"
#include "numbertype.h"
#include "fileplayer.h"

int main(int argc, char *argv[])
{
    QString graphic_role = QString("phone");

    QGuiApplication app(argc, argv);
    app.setDesktopFileName(graphic_role);

    QQuickStyle::setStyle("AGL");

    QQmlApplicationEngine engine;

    QCommandLineParser parser;
    parser.addPositionalArgument("port", app.translate("main", "port for binding"));
    parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    QStringList positionalArguments = parser.positionalArguments();

    if (positionalArguments.length() == 2) {
	int port = positionalArguments.takeFirst().toInt();
	QString secret = positionalArguments.takeFirst();
	QUrl bindingAddress;

	bindingAddress.setScheme(QStringLiteral("ws"));
	bindingAddress.setHost(QStringLiteral("localhost"));
	bindingAddress.setPort(port);
	bindingAddress.setPath(QStringLiteral("/api"));
	QUrlQuery query;

	query.addQueryItem(QStringLiteral("token"), secret);
	bindingAddress.setQuery(query);
	QQmlContext *context = engine.rootContext();
	context->setContextProperty(QStringLiteral("bindingAddress"), bindingAddress);
	Telephony *telephony = new Telephony(bindingAddress);
	context->setContextProperty("telephony", telephony);

	std::string token = secret.toStdString();
	std::string install_dir = getenv("AFM_APP_INSTALL_DIR");

	FilePlayer *player = new FilePlayer(install_dir + "/bin/Phone.wav", std::string("phone"));
	Phone *phone = new Phone(telephony, player);
	context->setContextProperty("phone", phone);

	QObject::connect(telephony, &Telephony::callStateChanged, phone, &Phone::onCallStateChanged);
	context->setContextProperty("pbap", new Pbap(bindingAddress, context));

	qmlRegisterUncreatableType<NumberType>("NumberType", 1, 0, "NumberType", "Not creatable as it is an enum type");
        engine.load(QUrl(QStringLiteral("qrc:/Phone.qml")));
    }
    return app.exec();
}

