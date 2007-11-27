/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to filter album contents
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

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// Local includes.

#include "ddebug.h"
#include "searchtextbar.h"
#include "ratingfilter.h"
#include "mimefilter.h"
#include "albumiconviewfilter.h"
#include "albumiconviewfilter.moc"

namespace Digikam
{

class AlbumIconViewFilterPriv
{
public:

    AlbumIconViewFilterPriv()
    {
        textFilter   = 0;
        mimeFilter   = 0;
        ratingFilter = 0;
    }

    SearchTextBar *textFilter;

    MimeFilter    *mimeFilter;

    RatingFilter  *ratingFilter;
};

AlbumIconViewFilter::AlbumIconViewFilter(QWidget* parent)
                   : KHBox(parent)
{
    d = new AlbumIconViewFilterPriv;

    d->textFilter = new SearchTextBar(this);
    d->textFilter->setToolTip(i18n("Text quick filter (search)"));
    d->textFilter->setWhatsThis(i18n("Here you can enter search patterns to quickly "
                                     "filter this view on file names, captions "
                                     "(comments), and tags"));

    d->mimeFilter   = new MimeFilter(this);
    d->ratingFilter = new RatingFilter(this);

    setSpacing(KDialog::spacingHint());
    setMargin(0);

    connect(d->ratingFilter, SIGNAL(signalRatingFilterChanged(int, AlbumLister::RatingCondition)),
            this, SLOT(slotRatingFilterChanged(int, AlbumLister::RatingCondition)));

    connect(d->mimeFilter, SIGNAL(activated(int)),
            this, SLOT(slotMimeTypeFilterChanged(int)));

    connect(d->textFilter, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotTextFilterChanged(const QString&)));

    connect(AlbumLister::instance(), SIGNAL(signalItemsTextFilterMatch(bool)),
            d->textFilter, SLOT(slotSearchResult(bool)));
}

AlbumIconViewFilter::~AlbumIconViewFilter()
{
    delete d;
}

void AlbumIconViewFilter::slotRatingFilterChanged(int rating, AlbumLister::RatingCondition cond)
{
    AlbumLister::instance()->setRatingFilter(rating, cond);
}

void AlbumIconViewFilter::slotMimeTypeFilterChanged(int mimeTypeFilter)
{
    AlbumLister::instance()->setMimeTypeFilter(mimeTypeFilter);
}

void AlbumIconViewFilter::slotTextFilterChanged(const QString& text)
{
    AlbumLister::instance()->setTextFilter(text);
}

void AlbumIconViewFilter::setRatingFilterCondition(AlbumLister::RatingCondition cond)
{
    d->ratingFilter->setRatingFilterCondition(cond);
}

AlbumLister::RatingCondition AlbumIconViewFilter::ratingFilterCondition()
{
    return d->ratingFilter->ratingFilterCondition();
}

}  // namespace Digikam
