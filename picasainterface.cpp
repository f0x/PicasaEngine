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

PicasaInterface::PicasaInterface(QObject *parent) : QObject(parent)
{
}

PicasaInterface::~PicasaInterface()
{}

void PicasaInterface::queryAlbum(const QString &searchTerm)
{
    if (searchTerm.isEmpty()) {
        return;
    }

    QString searchString = searchTerm;
    searchString.replace(' ', '+');
    // TODO: maybe reduce maximum results number allowed
    const QString url = "http://picasaweb.google.com/data/feed/api/user/"
               + searchString
               + "?kind=album";
    KUrl query(url);

    KIO::TransferJob *job = KIO::get(query, KIO::NoReload, KIO::HideProgressInfo);
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
        QString byteUsed = entries.at(i).namedItem("gphoto:bytesUsed").toElement().text();
        QDomNode mediaNode = entries.at(i).namedItem("media:group");
        QString thumb = mediaNode.namedItem("media:description").toElement().text();
        QDomElement mediaNode = entries.at(i).firstChildElement("media:thumbnail");
        QString thumb = mediaNode.attribute("url");

        Plasma::DataEngine::Data album;
        album["published"] = published;
        album["updated"] = updated;
        album["title"] = title;
        album["summary"] = summary;
        album["link"] = link;
        album["number of photos"] = numPhotos;
        album["byte used"] = byteUsed;
        album["thumbnail"] = thumb;
        emit result(m_queries[static_cast<KIO::Job*>(job)], id, album);
    }
    m_queries.remove(static_cast<KIO::Job*>(job));
    m_datas.remove(static_cast<KIO::Job*>(job));

}
