/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : an tool bar action object to display logo
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qtooltip.h>
#include <qpixmap.h>

// KDE includes.

#include <kurllabel.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>

// Local includes.

#include "dlogoaction.h"

namespace Digikam 
{

DLogoAction::DLogoAction(QObject* parent, const char* name)
           : KAction(parent, name)
{
}

int DLogoAction::plug(QWidget *widget, int index)
{
    if (kapp && !kapp->authorizeKAction(name()))
        return -1;
    
    if ( widget->inherits( "KToolBar" ) )
    {
        KToolBar *bar = (KToolBar *)widget;
    
        int id = getToolButtonID();

        KURLLabel *pixmapLogo = new KURLLabel("http://www.digikam.org", QString(), bar);
        pixmapLogo->setMargin(0);
        pixmapLogo->setScaledContents(false);
        pixmapLogo->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
        QToolTip::add(pixmapLogo, i18n("Visit digiKam project website"));
        KGlobal::dirs()->addResourceType("logo-digikam", KGlobal::dirs()->kde_default("data") + "digikam/data");
        QString directory = KGlobal::dirs()->findResourceDir("logo-digikam", "logo-digikam.png");
        pixmapLogo->setPixmap(QPixmap( directory + "logo-digikam.png" ));

        bar->insertWidget(id, pixmapLogo->width(), pixmapLogo);
        bar->alignItemRight(id);
    
        addContainer(bar, id);
    
        connect(bar, SIGNAL(destroyed()), 
                this, SLOT(slotDestroyed()));

        connect(pixmapLogo, SIGNAL(leftClickedURL(const QString&)),
                this, SLOT(slotProcessURL(const QString&)));    

        return containerCount() - 1;
    }

    int containerId = KAction::plug( widget, index );

    return containerId;
}
	
void DLogoAction::slotProcessURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}    
    
} // namespace Digikam
