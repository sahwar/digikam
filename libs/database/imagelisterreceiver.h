/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Simple virtual interface for ImageLister
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef IMAGELISTERRECEIVER_H
#define IMAGELISTERRECEIVER_H

#include <qstring.h>
#include <qvaluelist.h>

#include <kio/slavebase.h>

#include "digikam_export.h"
#include "imagelisterrecord.h"

namespace Digikam
{

//TODO: Docs
class DIGIKAM_EXPORT ImageListerReceiver
{
public:
    virtual void receive(const ImageListerRecord &record) = 0;
    virtual void error(const QString &/*errMsg*/) {};
};

class DIGIKAM_EXPORT ImageListerValueListReceiver
    : public ImageListerReceiver
{
public:
    ImageListerValueListReceiver();
    QValueList<ImageListerRecord> records;
    bool hasError;
    virtual void receive(const ImageListerRecord &record);
    virtual void error(const QString &errMsg);
};

class DIGIKAM_EXPORT ImageListerSlaveBaseReceiver : public ImageListerValueListReceiver
{
public:
    ImageListerSlaveBaseReceiver(KIO::SlaveBase *slave);
    virtual void error(const QString &errMsg);
    void sendData();
protected:
    KIO::SlaveBase *m_slave;
};

class DIGIKAM_EXPORT ImageListerSlaveBasePartsSendingReceiver
    : public ImageListerSlaveBaseReceiver
{
public:
    ImageListerSlaveBasePartsSendingReceiver(KIO::SlaveBase *slave, int limit);
    virtual void receive(const ImageListerRecord &record);
protected:
    int m_limit;
    int m_count;
};

}


#endif

