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
#include "picasa.h"
#include "picasainterface.h"

// KDE
#include <kio/job.h>


PicasaEngine::PicasaEngine(QObject *parent, const QVariantList &args) : Plasma::DataEngine(parent, args),
m_interface(new PicasaInterface(this))
{
    connect (m_interface, SIGNAL(result(QString,QString,Plasma::DataEngine::Data)), this, SLOT(slotSourceFromResult(QString,QString,Plasma::DataEngine::Data)));
}

PicasaEngine::~PicasaEngine()
{
}

bool PicasaEngine::sourceRequestEvent(const QString &name)
{
    if (!name.startsWith("album/")) {
        return false;
    }
    QString queryString = name;
    queryString.remove("album/");
    m_interface->queryAlbum(queryString);

    return true;
}

void PicasaEngine::slotSourceFromResult(const QString &searchTerm, const QString &id, const Plasma::DataEngine::Data &album)
{
    setData(searchTerm, id, album);
}

K_EXPORT_PLASMA_DATAENGINE(picasa, PicasaEngine)