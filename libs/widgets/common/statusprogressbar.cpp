/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-24
 * Description : a progress bar used to display file access
 *               progress or a text in status bar.
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

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QProgressBar>

// KDE includes.

#include <ksqueezedtextlabel.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcursor.h>

// Local includes.

#include "statusprogressbar.h"
#include "statusprogressbar.moc"

namespace Digikam
{

class StatusProgressBarPriv
{

public:

    enum WidgetStackEnum
    {
        TextLabel=0,
        ProgressBar
    };

    StatusProgressBarPriv()
    {
        textLabel      = 0;
        progressBar    = 0;
        progressWidget = 0;
        cancelButton   = 0;
    }

    QWidget            *progressWidget;

    QPushButton        *cancelButton;

    QProgressBar       *progressBar;

    KSqueezedTextLabel *textLabel;
};

StatusProgressBar::StatusProgressBar(QWidget *parent)
                 : QStackedWidget(parent)
{
    d = new StatusProgressBarPriv;
    setAttribute(Qt::WA_DeleteOnClose);

    d->textLabel      = new KSqueezedTextLabel(this);
    //TODO: KDE4PORT: Progress widget is not visible, text label is. Dont know why.
    d->progressWidget = new QWidget(this);
    QHBoxLayout *hBox = new QHBoxLayout(d->progressWidget);
    d->progressWidget->setLayout(hBox);
    d->progressBar    = new QProgressBar(d->progressWidget);
    d->progressBar->setMaximum(100);
    d->cancelButton   = new QPushButton(d->progressWidget);
    d->cancelButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    d->cancelButton->setIcon(SmallIcon("cancel"));

    // Parent widget will probably have the wait cursor set.
    // Set arrow cursor to indicate the button can be clicked
    d->cancelButton->setCursor(Qt::ArrowCursor);

    hBox->addWidget(d->progressBar);
    hBox->addWidget(d->cancelButton);

    insertWidget(StatusProgressBarPriv::TextLabel,   d->textLabel);
    insertWidget(StatusProgressBarPriv::ProgressBar, d->progressWidget);

    connect( d->cancelButton, SIGNAL( clicked() ),
             this, SIGNAL( signalCancelButtonPressed() ) );

    progressBarMode(TextMode);
}

StatusProgressBar::~StatusProgressBar()
{
    delete d;
}

void StatusProgressBar::setText( const QString& text )
{
    d->textLabel->setText(text);
}

void StatusProgressBar::setAlignment(Qt::Alignment a)
{
    d->textLabel->setAlignment(a);
}

void StatusProgressBar::setProgressValue( int v )
{
    d->progressBar->setValue(v);
}

void StatusProgressBar::setProgressText( const QString& text )
{
    d->progressBar->setFormat( text + QString ("%p%") );
}

void StatusProgressBar::progressBarMode( int mode, const QString& text )
{
    if ( mode == TextMode)
    {
        setCurrentIndex(StatusProgressBarPriv::TextLabel);
        setProgressValue(0);
        setText( text );
    }
    else if ( mode == ProgressBarMode )
    {
        d->cancelButton->hide();
        setCurrentIndex(StatusProgressBarPriv::ProgressBar);
        setProgressText( text );
    }
    else  // CancelProgressBarMode
    {
        d->cancelButton->show();
        setCurrentIndex(StatusProgressBarPriv::ProgressBar);
        setProgressText( text );
    }
}

}  // namespace Digikam
