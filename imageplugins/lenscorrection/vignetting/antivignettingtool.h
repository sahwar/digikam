/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : a digiKam image plugin to reduce
 *               vignetting on an image.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ANTIVIGNETTINGTOOL_H
#define ANTIVIGNETTINGTOOL_H

// Local includes

#include "editortool.h"

namespace DigikamAntiVignettingImagesPlugin
{

class AntiVignettingToolPriv;

class AntiVignettingTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    AntiVignettingTool(QObject *parent);
    ~AntiVignettingTool();

private Q_SLOTS:

    void slotResetSettings();

private:

    void writeSettings();
    void readSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();
    void blockWidgetSignals(bool b);

private:

    AntiVignettingToolPriv* const d;
};

}  // namespace DigikamAntiVignettingImagesPlugin

#endif /* ANTIVIGNETTINGTOOL_H */
