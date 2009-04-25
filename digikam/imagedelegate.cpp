/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "imagedelegate.h"
#include "imagedelegate.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QCache>
#include <QPainter>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>

// Local includes

#include "albummanager.h"
#include "albumsettings.h"
#include "constants.h"
#include "imagecategorydrawer.h"
#include "imagemodel.h"
#include "imagefiltermodel.h"
#include "themeengine.h"
#include "thumbbar.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class ImageDelegatePriv
{
public:

    ImageDelegatePriv()
    {
        categoryDrawer = 0;
        // Pre-computed star polygon for a 15x15 pixmap.
        starPolygon << QPoint(0,  6);
        starPolygon << QPoint(5,  5);
        starPolygon << QPoint(7,  0);
        starPolygon << QPoint(9,  5);
        starPolygon << QPoint(14, 6);
        starPolygon << QPoint(10, 9);
        starPolygon << QPoint(11, 14);
        starPolygon << QPoint(7,  11);
        starPolygon << QPoint(3,  14);
        starPolygon << QPoint(4,  9);

        starPolygonSize = QSize(15, 15);

        ratingPixmaps   = QVector<QPixmap>(10);

        actualPixmapRectCache.setMaxCost(250);
        thumbnailBorderCache.setMaxCost(10);

        editingRating = false;
    }

    QRect                            rect;
    QRect                            tightPixmapRect;
    QRect                            ratingRect;
    QRect                            dateRect;
    QRect                            modDateRect;
    QRect                            pixmapRect;
    QRect                            nameRect;
    QRect                            commentsRect;
    QRect                            resolutionRect;
    QRect                            sizeRect;
    QRect                            tagRect;

    QPixmap                          regPixmap;
    QPixmap                          selPixmap;
    QVector<QPixmap>                 ratingPixmaps;

    QFont                            font;
    QFont                            fontReg;
    QFont                            fontCom;
    QFont                            fontXtra;

    QPolygon                         starPolygon;
    QSize                            starPolygonSize;

    ThumbnailSize                    thumbSize;

    bool                             editingRating;

    QCache<qlonglong, QRect>         actualPixmapRectCache;
    QCache<QString, QPixmap>         thumbnailBorderCache;

    ImageCategoryDrawer             *categoryDrawer;
};

ImageDelegate::ImageDelegate(QObject *parent)
    : QAbstractItemDelegate(parent), d(new ImageDelegatePriv)
{
    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    d->categoryDrawer = new ImageCategoryDrawer;
}

ImageDelegate::~ImageDelegate()
{
    delete d->categoryDrawer;
    delete d;
}

void ImageDelegate::setThumbnailSize(const ThumbnailSize &thumbSize)
{
    if ( d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;

        updateRectsAndPixmaps();
    }
}

ImageCategoryDrawer *ImageDelegate::categoryDrawer() const
{
    return d->categoryDrawer;
}

void ImageDelegate::paint(QPainter * p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ImageInfo info = ImageModel::retrieveImageInfo(index);

    if (info.isNull())
        return;

    // state of painter must not be changed
    p->save();
    p->translate(option.rect.topLeft());

    QRect r;
    const AlbumSettings *settings = AlbumSettings::instance();
    ThemeEngine* te               = ThemeEngine::instance();

    bool isSelected = (option.state & QStyle::State_Selected);

    QPixmap pix;
    if (isSelected)
        pix = d->selPixmap;
    else
        pix = d->regPixmap;

    p->setPen(isSelected ? te->textSelColor() : te->textRegColor());

    QVariant thumbData = index.data(ImageModel::ThumbnailRole);
    if (!thumbData.isNull())
    {
        QPixmap thumbnail = thumbData.value<QPixmap>();
        r = d->pixmapRect;
        p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                      r.y() + (r.height()-thumbnail.height())/2,
                      thumbnail);

        QRect actualPixmapRect(r.x() + (r.width()-thumbnail.width())/2,
                               r.y() + (r.height()-thumbnail.height())/2,
                               thumbnail.width(), thumbnail.height());
        const_cast<ImageDelegate*>(this)->updateActualPixmapRect(info.id(), actualPixmapRect);

        p->save();
        QRegion pixmapClipRegion = QRegion(d->rect) - QRegion(actualPixmapRect);
        p->setClipRegion(pixmapClipRegion);
        p->drawPixmap(0, 0, pix);

        QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size());
        p->drawPixmap(actualPixmapRect.x()-3, actualPixmapRect.y()-3, borderPix);

        p->restore();
    }
    else
    {
        // simplified
        p->drawPixmap(0, 0, pix);
    }

    if (settings->getIconShowRating())
    {
        r = d->ratingRect;

        if (!d->editingRating)
            p->drawPixmap(r, ratingPixmap(info.rating(), isSelected));
        else
            p->drawPixmap(r, ratingPixmap(-1, isSelected));
    }

    if (settings->getIconShowName())
    {
        r = d->nameRect;
        p->setFont(d->fontReg);
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), info.name()));
    }

    p->setFont(d->fontCom);

    if (settings->getIconShowComments())
    {
        QString comments = info.comment();

        r = d->commentsRect;
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), comments));
    }

    p->setFont(d->fontXtra);

    if (settings->getIconShowDate())
    {
        QDateTime date(info.dateTime());

        r = d->dateRect;
        p->setFont(d->fontXtra);
        QString str = dateToString(date);
        str = i18nc("date of image creation", "created : %1",str);
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), str));
    }

    if (settings->getIconShowModDate())
    {
        QDateTime date(info.modDateTime());

        r = d->modDateRect;
        p->setFont(d->fontXtra);
        QString str = dateToString(date);
        str = i18nc("date of last image modification", "modified : %1",str);
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), str));
    }

    if (settings->getIconShowResolution())
    {
        QSize dims = info.dimensions();
        if (dims.isValid())
        {
            QString mpixels, resolution;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            resolution = (!dims.isValid()) ? i18nc("unknown image resolution", "Unknown")
                                           : i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                                                   dims.width(),dims.height(),mpixels);
            r = d->resolutionRect;
            p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), resolution));
        }
    }

    if (settings->getIconShowSize())
    {
        r = d->sizeRect;
        p->drawText(r, Qt::AlignCenter,
                    squeezedText(p, r.width(), KIO::convertSize(info.fileSize())));
    }

    p->setFont(d->fontCom);
    p->setPen(isSelected ? te->textSpecialSelColor() : te->textSpecialRegColor());

    if (settings->getIconShowTags())
    {
        QString tags = AlbumManager::instance()->tagNames(info.tagIds()).join(", ");

        r = d->tagRect;
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), tags));
    }

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        r = d->rect;
        p->setPen(QPen(isSelected ? te->textSelColor() : te->textRegColor(), 1, Qt::DotLine));
        p->drawRect(1, 1, r.width()-3, r.height()-3);
    }

    if (option.state & QStyle::State_MouseOver)
    {
        //paintToggleSelectButton(p);

        r = d->rect;
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, r.width()-3, r.height()-3);
    }

    p->restore();
}

QSize ImageDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    return d->rect.size();
}

void ImageDelegate::setDefaultViewOptions(const QStyleOptionViewItem &option)
{
    d->font = option.font;
    d->categoryDrawer->setDefaultViewOptions(option);
}

void ImageDelegate::slotThemeChanged()
{
    updateRectsAndPixmaps();
    d->categoryDrawer->updateRectsAndPixmaps();
}

void ImageDelegate::slotSetupChanged()
{
    updateRectsAndPixmaps();
    d->categoryDrawer->updateRectsAndPixmaps();
    // it's pretty much nonsense to include any special index here.
    emit sizeHintChanged(QModelIndex());
}

void ImageDelegate::updateRectsAndPixmaps()
{
    const AlbumSettings *albumSettings = AlbumSettings::instance();

    d->rect           = QRect(0, 0, 0, 0);
    d->ratingRect     = QRect(0, 0, 0, 0);
    d->dateRect       = QRect(0, 0, 0, 0);
    d->modDateRect    = QRect(0, 0, 0, 0);
    d->pixmapRect     = QRect(0, 0, 0, 0);
    d->nameRect       = QRect(0, 0, 0, 0);
    d->commentsRect   = QRect(0, 0, 0, 0);
    d->resolutionRect = QRect(0, 0, 0, 0);
    d->sizeRect       = QRect(0, 0, 0, 0);
    d->tagRect        = QRect(0, 0, 0, 0);

    d->fontReg  = d->font;
    d->fontCom  = d->font;
    d->fontXtra = d->font;
    d->fontCom.setItalic(true);

    int fnSz = d->fontReg.pointSize();
    if (fnSz > 0)
    {
        d->fontCom.setPointSize(fnSz-1);
        d->fontXtra.setPointSize(fnSz-2);
    }
    else
    {
        fnSz = d->fontReg.pixelSize();
        d->fontCom.setPixelSize(fnSz-1);
        d->fontXtra.setPixelSize(fnSz-2);
    }

    const int radius = 3;
    const int margin = 5;
    int w            = d->thumbSize.size() + 2*radius;

    QFontMetrics fm(d->fontReg);
    QRect oneRowRegRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fontCom);
    QRect oneRowComRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fontXtra);
    QRect oneRowXtraRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                           Qt::AlignTop | Qt::AlignHCenter,
                                           "XXXXXXXXX");

    QSize starPolygonSize(15, 15);

    int y = margin;

    d->pixmapRect = QRect(margin, y, w, d->thumbSize.size() + 2*radius);
    y = d->pixmapRect.bottom();

    if (albumSettings->getIconShowRating())
    {
        d->ratingRect = QRect(margin, y, w, starPolygonSize.height());
        y = d->ratingRect.bottom();
    }

    if (albumSettings->getIconShowName())
    {
        d->nameRect = QRect(margin, y, w-margin, oneRowRegRect.height());
        y = d->nameRect.bottom();
    }

    if (albumSettings->getIconShowComments())
    {
        d->commentsRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->commentsRect.bottom();
    }

    if (albumSettings->getIconShowDate())
    {
        d->dateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->dateRect.bottom();
    }

    if (albumSettings->getIconShowModDate())
    {
        d->modDateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->modDateRect.bottom();
    }

    if (albumSettings->getIconShowResolution())
    {
        d->resolutionRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->resolutionRect.bottom() ;
    }

    if (albumSettings->getIconShowSize())
    {
        d->sizeRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->sizeRect.bottom();
    }

    if (albumSettings->getIconShowTags())
    {
        d->tagRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->tagRect.bottom();
    }

    d->rect      = QRect(0, 0, w + 2*margin, y+margin+radius);

    d->regPixmap = ThemeEngine::instance()->thumbRegPixmap(d->rect.width(),
                                                               d->rect.height());

    d->selPixmap = ThemeEngine::instance()->thumbSelPixmap(d->rect.width(),
                                                               d->rect.height());

    // -- Generate rating pixmaps ------------------------------------------

    // We use antialiasing and want to pre-render the pixmaps.
    // So we need the background at the time of painting,
    // and the background may be a gradient, and will be different for selected items.
    // This makes 5*2 (small) pixmaps.
    if (albumSettings->getIconShowRating())
    {
        for (int sel=0; sel<2; ++sel)
        {
            QPixmap basePix;

            // do this once for regular, once for selected backgrounds
            if (sel)
                basePix = d->selPixmap.copy(d->ratingRect);
            else
                basePix = d->regPixmap.copy(d->ratingRect);

            for (int rating=1; rating<=5; ++rating)
            {
                // we store first the 5 regular, then the 5 selected pixmaps, for simplicity
                int index = (sel * 5 + rating) - 1;

                // copy background
                d->ratingPixmaps[index] = basePix;
                // open a painter
                QPainter painter(&d->ratingPixmaps[index]);

                // use antialiasing
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setBrush(ThemeEngine::instance()->textSpecialRegColor());
                QPen pen(ThemeEngine::instance()->textRegColor());
                // set a pen which joins the lines at a filled angle
                pen.setJoinStyle(Qt::MiterJoin);
                painter.setPen(pen);

                // move painter while drawing polygons
                painter.translate( lround((d->ratingRect.width() - margin - rating*(starPolygonSize.width()+1))/2.0) + 2, 1 );
                for (int s=0; s<rating; ++s)
                {
                    painter.drawPolygon(d->starPolygon, Qt::WindingFill);
                    painter.translate(starPolygonSize.width() + 1, 0);
                }
            }
        }
    }

    clearThumbnailBorderCache();
}

QPixmap ImageDelegate::ratingPixmap(int rating, bool selected) const
{
    if (rating < 1 || rating > 5)
    {
        QPixmap pix;
        if (selected)
            pix = d->selPixmap.copy(d->ratingRect);
        else
            pix = d->regPixmap.copy(d->ratingRect);

        return pix;
    }

    rating--;
    if (selected)
        return d->ratingPixmaps[5 + rating];
    else
        return d->ratingPixmaps[rating];
}

QRect ImageDelegate::actualPixmapRect(qlonglong imageid) const
{
    // We do not recompute if not found. Assumption is cache is always properly updated.
    QRect *rect = d->actualPixmapRectCache.object(imageid);
    if (rect)
        return *rect;
    else
        return d->pixmapRect;
}

void ImageDelegate::updateActualPixmapRect(qlonglong imageid, const QRect &rect)
{
    QRect *old = d->actualPixmapRectCache.object(imageid);
    if (!old || *old != rect)
        d->actualPixmapRectCache.insert(imageid, new QRect(rect));
}

QPixmap ImageDelegate::thumbnailBorderPixmap(const QSize &pixSize) const
{
    const int radius         = 3;
    const QColor borderColor = QColor(0, 0, 0, 128);

    QString cacheKey  = QString::number(pixSize.width()) + "-" + QString::number(pixSize.height());
    QPixmap *cachePix = d->thumbnailBorderCache.object(cacheKey);

    if (!cachePix)
    {
        QPixmap pix = ThumbBarView::generateFuzzyRect(QSize(pixSize.width()  + 2*radius,
                                                            pixSize.height() + 2*radius),
                                                      borderColor, radius);
        const_cast<ImageDelegate*>(this)->d->thumbnailBorderCache.insert(cacheKey, new QPixmap(pix));
        return pix;
    }

    return *cachePix;
}

void ImageDelegate::clearThumbnailBorderCache()
{
    d->thumbnailBorderCache.clear();
}

QString ImageDelegate::dateToString(const QDateTime& datetime) const
{
    return KGlobal::locale()->formatDateTime(datetime, KLocale::ShortDate, false);
}

QString ImageDelegate::squeezedText(QPainter* p, int width, const QString& text) const
{
    QString fullText(text);
    fullText.replace('\n',' ');
    QFontMetrics fm(p->fontMetrics());
    int textWidth = fm.width(fullText);

    if (textWidth > width)
    {
        // start with the dots only
        QString squeezedText = "...";
        int squeezedWidth    = fm.width(squeezedText);

        // estimate how many letters we can add to the dots on both sides
        int letters = fullText.length() * (width - squeezedWidth) / textWidth;
        if (width < squeezedWidth) letters=1;
        squeezedText  = fullText.left(letters) + "...";
        squeezedWidth = fm.width(squeezedText);

        if (squeezedWidth < width)
        {
            // we estimated too short
            // add letters while text < label
            do
            {
                letters++;
                squeezedText  = fullText.left(letters) + "...";
                squeezedWidth = fm.width(squeezedText);
            }
            while (squeezedWidth < width);

            letters--;
            squeezedText = fullText.left(letters) + "...";
        }
        else if (squeezedWidth > width)
        {
            // we estimated too long
            // remove letters while text > label
            do
            {
                letters--;
                squeezedText  = fullText.left(letters) + "...";
                squeezedWidth = fm.width(squeezedText);
            }
            while (letters && squeezedWidth > width);
        }

        if (letters >= 5)
        {
            return squeezedText;
        }
    }

    return fullText;
}
}


