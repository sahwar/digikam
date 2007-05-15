/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 * 
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */


#ifndef IMAGEINFOCACHE_H
#define IMAGEINFOCACHE_H

// Qt includes

#include <qobject.h>
#include <qmap.h>

// Local includes

#include "databaseattributeswatch.h"

namespace Digikam
{

class ImageInfoData;


class ImageInfoCache : public QObject
{
    Q_OBJECT

public:

    ImageInfoCache();
    ~ImageInfoCache();

    /**
     * Return an ImageInfoData object for the given image id.
     * A new object is created, or an existing object is returned.
     * If a new object is created, the id field will be initialized.
     */
    ImageInfoData *infoForId(Q_LLONG id);
    /**
     * Returns whether an ImageInfoObject for the given image id
     * is contained in the cache.
     */
    bool hasInfoForId(Q_LLONG id) const;

    /**
     * Call this when the reference count is dropped to 1.
     * This method is called under mutex lock, and will check
     * again the reference count. A count of 1 means the info is only
     * left here in the cache, all ImageInfo containers are gone.
     * The cache will delete this object when it wants.
     */
    void dropInfo(ImageInfoData *infodata);

private slots:

    void slotImageFieldChanged(Q_LLONG imageId, int field);

private:

    QMap<Q_LLONG, ImageInfoData *> m_map;
};


}

#endif

