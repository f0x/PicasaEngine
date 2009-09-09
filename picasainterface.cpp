/***************************************************************************
 *   Copyright 2009 by Francesco Grieco <fgrieco@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#include "picasainterface.h"

// Qt
#include <QDomDocument>
#include <QDomNodeList>

// KDE
#include <kio/job.h>
#include <KUrl>
#include <KDebug>
#include <KMessageBox>

PicasaInterface::PicasaInterface(QObject *parent) : QObject(parent)
{
}

PicasaInterface::~PicasaInterface()
{}

void PicasaInterface::queryAlbum(const QString &searchTerm, const QString &password)
{
    if (searchTerm.isEmpty()) {
        return;
    }

    if (!password.isEmpty()) {
        handlePassword(searchTerm, password);
    }

    QString searchString = searchTerm;
    // searchString.replace(' ', '+');

    const QString url = "http://picasaweb.google.com/data/feed/api/user/"
               + searchString
               + "?kind=album";
    KUrl query(url);

    QString auth_string = "GoogleLogin auth=" + m_token;

    KIO::TransferJob *job = KIO::get(query, KIO::NoReload, KIO::HideProgressInfo);

    KMessageBox::error(0, m_token);
    if (!m_token.isEmpty()) {
        job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
        job->addMetaData("customHTTPHeader", "Authorization: " + auth_string);
    }
    m_queries[job] = searchTerm;
    connect (job, SIGNAL(data(KIO::Job*, const QByteArray &)), this, SLOT(picasaDataReady(KIO::Job*, const QByteArray &)));
    connect (job, SIGNAL(result(KJob *)), this, SLOT(parseResults(KJob*)));
}

void PicasaInterface::picasaDataReady(KIO::Job *job, const QByteArray &data)
{
    // could this ever happen?
    if (!m_queries.contains(job)) {
        return;
    }

    m_datas[job].append(data);
}

void PicasaInterface::parseResults(KJob *job)
{
    if (!m_datas.contains(static_cast<KIO::Job*>(job))) {
        return;
    }

    QDomDocument document;
    document.setContent(m_datas[static_cast<KIO::Job*>(job)]);

    QDomNodeList entries = document.elementsByTagName("entry");
    for (int i = 0; i < entries.count(); i++) {

        QString id = entries.at(i).namedItem("id").toElement().text().split("/").last();
        QString published = entries.at(i).namedItem("published").toElement().text();
        QString updated = entries.at(i).namedItem("updated").toElement().text();

        // link is the direct link to the album
        QDomElement domElement = entries.at(i).firstChildElement("link");
        while (!domElement.attribute("type").contains("text/html") && !domElement.isNull()) {
            //value = domElement.attribute("type");
            domElement = domElement.nextSiblingElement("link");
        }
        QString link = domElement.attribute("href");

        QString title = entries.at(i).namedItem("title").toElement().text();
        QString summary = entries.at(i).namedItem("title").toElement().text();
        QString numPhotos = entries.at(i).namedItem("gphoto:numphotos").toElement().text();
        // byteUsed only if the user is logged
        QString byteUsed = entries.at(i).namedItem("gphoto:bytesUsed").toElement().text();
        //QDomElement mediaElement = entries.at(i).firstChildElement("media:thumbnail");
        //QString thumb = mediaElement.attribute("url");
        QDomNode mediaNode = entries.at(i).namedItem("media:group");
        QDomElement mediaElement = mediaNode.firstChildElement("media:thumbnail");
        QString thumb = mediaElement.attribute("url");

        Plasma::DataEngine::Data album;
        album["published"] = published;
        album["updated"] = updated;
        album["title"] = title;
        album["summary"] = summary;
        album["link"] = link;
        album["number of photos"] = numPhotos;
        album["thumbnail"] = thumb;

        if (!byteUsed.isEmpty()) {
            album["byte used"] = byteUsed;
        }

        emit result(m_queries[static_cast<KIO::Job*>(job)], id, album);
    }
    m_queries.remove(static_cast<KIO::Job*>(job));
    m_datas.remove(static_cast<KIO::Job*>(job));

}

void PicasaInterface::handlePassword(const QString &username, const QString &password)
{
    KUrl url("https://www.google.com/accounts/ClientLogin");
    QString accountType = "GOOGLE";
    QStringList qsl;
    qsl.append("Email="+username);
    qsl.append("Passwd="+password);
    qsl.append("accountType="+accountType);
    qsl.append("service=lh2");
    qsl.append("source=kde-picasaengine");
    QString dataParameters = qsl.join("&");
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    KIO::TransferJob *job = KIO::http_post(url, buffer, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded" );
    connect (job, SIGNAL(data(KIO::Job*, const QByteArray &)), this, SLOT(data(KIO::Job*, const QByteArray &)));

}

void PicasaInterface::data(KIO::Job *job, const QByteArray &data)
{
    if (data.isEmpty())
        return;

    QString output(data);

    if (output.contains("Auth=")) {
        QStringList strList = output.split("Auth=");
        if (strList.count() > 0) {
            m_token = strList[1].trimmed();
        }
    }

    KMessageBox::error(0, m_token);
}
