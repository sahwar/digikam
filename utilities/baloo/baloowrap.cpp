/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-08-05
 * Description : Find Duplicates tree-view search album.
 *
 * Copyright (C) 2014 by Veaeceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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
// Self
#include "baloowrap.h"


// Qt


// KDE
#include <kdebug.h>
#include <kurl.h>

// Baloo
#include <baloo/file.h>
#include <baloo/filefetchjob.h>
#include <baloo/filemodifyjob.h>
#include <baloo/taglistjob.h>


#include "tagscache.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databaseparameters.h"
#include "databasetransaction.h"
#include "databasewatch.h"
#include "imagecomments.h"
#include "imageinfo.h"
#include "imagelister.h"
#include "tagscache.h"
namespace Digikam
{

class BalooWrap::Private
{
public:
    Private()
    {
        syncFromBalooToDigikam = false;
        syncFromDigikamToBaloo = false;
    }
    bool syncFromDigikamToBaloo;
    bool syncFromBalooToDigikam;
};

QPointer<BalooWrap> BalooWrap::internalPtr = QPointer<BalooWrap>();

BalooWrap::BalooWrap(QObject* parent)
: QObject(parent), d(new BalooWrap::Private)
{
}

BalooWrap::~BalooWrap()
{
    delete d;
}

BalooWrap* BalooWrap::instance()
{
    if (BalooWrap::internalPtr.isNull())
    {
        BalooWrap::internalPtr = new BalooWrap();
    }

    return BalooWrap::internalPtr;
}


void BalooWrap::setTags(KUrl &url, QStringList *tags)
{
    setAllData(url,tags, NULL, -1);
}

void BalooWrap::setComment(KUrl &url, QString *comment)
{
    setAllData(url, NULL, comment, -1);
}

void BalooWrap::setRating(KUrl &url, int rating)
{
    setAllData(url, NULL, NULL, rating);
}

void BalooWrap::setAllData(KUrl &url, QStringList* tags, QString* comment, int rating)
{
    if(!d->syncFromDigikamToBaloo)
    {
        return;
    }

    bool write = false;
    Baloo::File file(url.toLocalFile());

    if(tags != NULL)
    {
        file.setTags(*tags);
        write = true;
    }
    if(comment != NULL)
    {
        file.setUserComment(*comment);
        write = true;
    }
    if(rating != -1)
    {
        // digiKam store rating as value form 0 to 5
        // while baloo store it as value from 0 to 10
        file.setRating(rating*2);
        write = true;
    }

    if(write)
    {
        Baloo::FileModifyJob* job = new Baloo::FileModifyJob(file);
        job->start();
    }
}



BalooInfo BalooWrap::getSemanticInfo(const KUrl& url)
{
    if(!d->syncFromBalooToDigikam)
    {
        return BalooInfo();
    }
    Baloo::FileFetchJob* job = new Baloo::FileFetchJob(url.toLocalFile());
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFetchFinished(KJob*)));

    job->exec();

    kDebug() << "Job started";
    Baloo::File file = job->file();

    BalooInfo bInfo;
    // Baloo have rating from 0 to 10, while digikam have only from 0 to 5
    bInfo.rating = file.rating()/2;
    bInfo.comment = file.userComment();
    bInfo.tags = file.tags().toSet().toList();

    return bInfo;
}

void BalooWrap::slotFetchFinished(KJob* job)
{
    kDebug() << "Job finished";
    Baloo::FileFetchJob* fjob = static_cast<Baloo::FileFetchJob*>(job);
    Baloo::File file = fjob->file();

    BalooInfo bInfo;
    bInfo.rating = file.rating();
    bInfo.comment = file.userComment();
    bInfo.tags = file.tags().toSet().toList();


    KUrl url = KUrl::fromLocalFile(file.url());
    addInfoToDigikam(bInfo, url);
}


int BalooWrap::bestDigikamTagForTagName(const ImageInfo& info, const QString& tagname) const
{
    if (tagname.isEmpty())
    {
        return 0;
    }

    QList<int> candidates = TagsCache::instance()->tagsForName(tagname);

    if (candidates.isEmpty())
    {
        // add top-level tag
        return DatabaseAccess().db()->addTag(0, tagname, QString(), 0);
    }
    else if (candidates.size() == 1)
    {
        return candidates.first();
    }
    else
    {
        int currentCandidate    = 0;
        int currentMinimumScore = 0;
        QList<int> assignedTags = info.tagIds();

        foreach(int tagId, candidates)
        {
            // already assigned one of the candidates?
            if (assignedTags.contains(tagId))
            {
                return 0;
            }

            int id = tagId;
            int score = 0;

            do
            {
                id = TagsCache::instance()->parentTag(id);
                score++;
            }
            while (id);

            if (!currentMinimumScore || score < currentMinimumScore)
            {
                currentCandidate = tagId;
            }
        }

        return currentCandidate;
    }
}

void BalooWrap::addInfoToDigikam(BalooInfo &bInfo, const KUrl &fileUrl)
{
    QStringList tags = bInfo.tags;
    QList<int> tagIdsForInfo;
    ImageInfo info(fileUrl);

    // If the path is not in digikam collections, info will be null.
    // It does the same check first that we would be doing here

    if(info.isNull())
    {
        return;
    }

    const int size = tags.size();

    for (int i = 0; i < size; ++i)
    {
        int tagId = bestDigikamTagForTagName(info, tags.at(i));

        if (tagId)
        {
            tagIdsForInfo << tagId;
        }
    }

    if (!tagIdsForInfo.isEmpty())
    {
        DatabaseAccess access;
        DatabaseTransaction transaction(&access);
        const int infosSize = tagIdsForInfo.size();

        for (int i = 0; i < infosSize; ++i)
        {
            info.setTag(tagIdsForInfo.at(i));
        }
    }
}

void BalooWrap::setSyncToBaloo(bool value)
{
    kDebug() << "Setting Sync to Baloo to ++++++++" << value;
    d->syncFromDigikamToBaloo = value;
}

bool BalooWrap::getSyncToBaloo()
{
    return d->syncFromDigikamToBaloo;
}

bool BalooWrap::getSyncToDigikam()
{
    return d->syncFromBalooToDigikam;
}

void BalooWrap::setSyncToDigikam(bool value)
{
    kDebug() << "Setting Sync to Digikam to ++++++++++++++" << value;
    d->syncFromBalooToDigikam = value;
}

// NOTE: useful code to extend functionality in the future
//TagSet BalooWrap::allTags() const
//{
//    if (d->mAllTags.empty()) {
//        const_cast<BalooWrap*>(this)->refreshAllTags();
//    }
//    return d->mAllTags;
//}

//void BalooWrap::refreshAllTags()
//{
//    Baloo::TagListJob* job = new Baloo::TagListJob();
//    job->exec();

//    d->mAllTags.clear();
//    Q_FOREACH(const QString& tag, job->tags()) {
//        d->mAllTags << tag;
//    }
//}

} // namespace
