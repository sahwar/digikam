/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright (C) 2004-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef SHOWFOTO_P_H
#define SHOWFOTO_P_H

// Qt includes

#include <QDir>

// Local includes

#include "showfoto.h"
#include "showfotoiteminfo.h"
#include "showfotothumbnailbar.h"
#include "splashscreen.h"
#include "imagepropertiessidebar.h"
#include "showfotodelegate.h"
#include "showfotosettings.h"
#include "showfotodragdrophandler.h"
#include "kiowrapper.h"

namespace ShowFoto
{

class ShowFoto::Private
{
public:

    Private() :
        deleteItem2Trash(true),
        validIccPath(true),
        droppedUrls(false),
        imagePluginsLoaded(false),
        itemsNb(0),
        vSplitter(0),
        fileOpenAction(0),
        openFilesInFolderAction(0),
        first(0),
        model(0),
        dDHandler(0),
        filterModel(0),
        thumbLoadThread(0),
        kioWrapper(0),
        thumbBar(0),
        thumbBarDock(0),
        normalDelegate(0),
        rightSideBar(0),
        splash(0),
        settings(0)
    {
    }

    bool                             deleteItem2Trash;
    bool                             validIccPath;
    bool                             droppedUrls;
    bool                             imagePluginsLoaded;

    int                              itemsNb;

    QSplitter*                       vSplitter;
    QAction*                         fileOpenAction;
    QUrl                             lastOpenedDirectory;
    QString                          tempFilePath;
    QAction*                         openFilesInFolderAction;
    QAction*                         first;
    QDir                             dir;

    ShowfotoItemInfoList             infoList;
    ShowfotoThumbnailModel*          model;
    ShowfotoDragDropHandler*         dDHandler;
    ShowfotoFilterModel*             filterModel;
    Digikam::ThumbnailLoadThread*    thumbLoadThread;
    Digikam::KIOWrapper*             kioWrapper;
    ShowfotoThumbnailBar*            thumbBar;
    Digikam::ThumbBarDock*           thumbBarDock;
    ShowfotoNormalDelegate*          normalDelegate;
    Digikam::ImagePropertiesSideBar* rightSideBar;
    Digikam::SplashScreen*           splash;
    ShowfotoSettings*                settings;
};

} // namespace Showfoto

#endif // SHOWFOTO_P_H
