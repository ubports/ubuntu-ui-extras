/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"

#include "backend/backend_cups.h"

#include "models/jobmodel.h"

#include <QDebug>

JobModel::JobModel(QObject *parent) : QAbstractListModel(parent)
{
}

JobModel::JobModel(PrinterBackend *backend,
                   QObject *parent)
    : QAbstractListModel(parent)
    , m_backend(backend)
{
    update();

    QObject::connect(m_backend, &PrinterBackend::jobCreated,
                     this, &JobModel::jobSignalCatchAll);
    QObject::connect(m_backend, &PrinterBackend::jobState,
                     this, &JobModel::jobSignalCatchAll);
    QObject::connect(m_backend, &PrinterBackend::jobCompleted,
                     this, &JobModel::jobSignalCatchAll);
}

JobModel::~JobModel()
{
}

void JobModel::jobSignalCatchAll(
        const QString &text, const QString &printer_uri,
        const QString &printer_name, uint printer_state,
        const QString &printer_state_reasons, bool printer_is_accepting_jobs,
        uint job_id, uint job_state, const QString &job_state_reasons,
        const QString &job_name, uint job_impressions_completed)
{
    Q_UNUSED(text);
    Q_UNUSED(printer_uri);
    Q_UNUSED(printer_name);
    Q_UNUSED(printer_state);
    Q_UNUSED(printer_state_reasons);
    Q_UNUSED(printer_is_accepting_jobs);
    Q_UNUSED(job_id);
    Q_UNUSED(job_state);
    Q_UNUSED(job_state_reasons);
    Q_UNUSED(job_name);
    Q_UNUSED(job_impressions_completed);

    auto job = getJobById(job_id);
    if (job)
        job->setImpressionsCompleted(job_impressions_completed);

    update();
}

void JobModel::update()
{
    // Store the old count and get the new printers
    int oldCount = m_jobs.size();
    QList<QSharedPointer<PrinterJob>> newJobs = m_backend->printerGetJobs();

    // Go through the old model
    for (int i=0; i < m_jobs.count(); i++) {
        // Determine if the old printer exists in the new model
        bool exists = false;

        Q_FOREACH(QSharedPointer<PrinterJob> p, newJobs) {
            if (p->jobId() == m_jobs.at(i)->jobId()) {
                exists = true;

                // Ensure the other properties of the job are up to date
                if (!m_jobs.at(i)->deepCompare(p)) {
                    m_jobs.at(i)->updateFrom(p);

                    Q_EMIT dataChanged(index(i), index(i));
                }

                break;
            }
        }

        // If it doesn't exist then remove it from the old model
        if (!exists) {
            beginRemoveRows(QModelIndex(), i, i);
            QSharedPointer<PrinterJob> p = m_jobs.takeAt(i);
            endRemoveRows();

            i--;  // as we have removed an item decrement
        }
    }

    // Go through the new model
    for (int i=0; i < newJobs.count(); i++) {
        // Determine if the new printer exists in the old model
        bool exists = false;
        int j;

        for (j=0; j < m_jobs.count(); j++) {
            if (m_jobs.at(j)->jobId() == newJobs.at(i)->jobId()) {
                exists = true;
                break;
            }
        }

        if (exists) {
            if (j == i) {  // New printer exists and in correct position
                continue;
            } else {
                // New printer does exist but needs to be moved in old model
                beginMoveRows(QModelIndex(), j, 1, QModelIndex(), i);
                m_jobs.move(j, i);
                endMoveRows();
            }
        } else {
            // New printer does not exist insert into model
            beginInsertRows(QModelIndex(), i, i);
            m_jobs.insert(i, newJobs.at(i));
            endInsertRows();
        }
    }

    if (oldCount != m_jobs.size()) {
        Q_EMIT countChanged();
    }
}

int JobModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_jobs.size();
}

int JobModel::count() const
{
    return rowCount();
}

QVariant JobModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;

    if ((0<=index.row()) && (index.row()<m_jobs.size())) {

        auto job = m_jobs[index.row()];

        switch (role) {
        case CollateRole:
            ret = job->collate();
            break;
        case ColorModelRole: {
            if (job->printer()) {
                ret = job->printer()->supportedColorModels().at(job->colorModel()).text;
            } else {
                qWarning() << "Printer is undefined, no colorModel";
                ret = "";
            }
            break;
        }
        case CompletedTimeRole:
            ret = job->completedTime().toString(QLocale::system().dateTimeFormat());
            break;
        case CopiesRole:
            ret = job->copies();
            break;
        case CreationTimeRole:
            ret = job->creationTime().toString(QLocale::system().dateTimeFormat());
            break;
        case DuplexRole: {
            if (job->printer()) {
                ret = job->printer()->supportedDuplexStrings().at(job->duplexMode());
            } else {
                qWarning() << "Printer is undefined, no duplexMode";
                ret = "";
            }
            break;
        }
        case IdRole:
            ret = job->jobId();
            break;
        case ImpressionsCompletedRole:
            ret = job->impressionsCompleted();
            break;
        case LandscapeRole:
            ret = job->landscape();
            break;
        case MessagesRole:
            ret = job->messages();
            break;
        case PrinterNameRole:
            ret = job->printerName();
            break;
        case PrintRangeRole:
            ret = job->printRange();
            break;
        case PrintRangeModeRole:
            ret = QVariant::fromValue<PrinterEnum::PrintRange>(job->printRangeMode());
            break;
        case ProcessingTimeRole:
            ret = job->processingTime().toString(QLocale::system().dateTimeFormat());
            break;
        case QualityRole: {
            if (job->printer()) {
                ret = job->printer()->supportedPrintQualities().at(job->quality()).text;
            } else {
                qWarning() << "Printer is undefined, no quality";
                ret = "";
            }
            break;
        }
        case ReverseRole:
            ret = job->reverse();
            break;
        case SizeRole:
            ret = job->size();
            break;
        case StateRole:
            // TODO: improve, for now have a switch
            switch (job->state()) {
            case PrinterEnum::JobState::Aborted:
                ret = "Aborted";
                break;
            case PrinterEnum::JobState::Canceled:
                ret = "Canceled";
                break;
            case PrinterEnum::JobState::Complete:
                ret = "Compelete";
                break;
            case PrinterEnum::JobState::Held:
                ret = "Held";
                break;
            case PrinterEnum::JobState::Pending:
                ret = "Pending";
                break;
            case PrinterEnum::JobState::Processing:
                ret = "Processing";
                break;
            case PrinterEnum::JobState::Stopped:
                ret = "Stopped";
                break;
            }
            break;
        case Qt::DisplayRole:
        case TitleRole:
            ret = job->title();
            break;
        case UserRole:
            ret = job->user();
            break;
        }
    }

    return ret;
}

QHash<int, QByteArray> JobModel::roleNames() const
{
    static QHash<int,QByteArray> names;

    if (Q_UNLIKELY(names.empty())) {
        names[Qt::DisplayRole] = "displayName";
        names[IdRole] = "id";
        names[CollateRole] = "collate";
        names[ColorModelRole] = "colorModel";
        names[CompletedTimeRole] = "completedTime";
        names[CopiesRole] = "copies";
        names[CreationTimeRole] = "creationTime";
        names[DuplexRole] = "duplexMode";
        names[ImpressionsCompletedRole] = "impressionsCompleted";
        names[LandscapeRole] = "landscape";
        names[MessagesRole] = "messages";
        names[PrinterNameRole] = "printerName";
        names[PrintRangeRole] = "printRange";
        names[PrintRangeModeRole] = "printRangeMode";
        names[ProcessingTimeRole] = "processingTime";
        names[QualityRole] = "quality";
        names[ReverseRole] = "reverse";
        names[SizeRole] = "size";
        names[StateRole] = "state";
        names[TitleRole] = "title";
        names[UserRole] = "user";
        names[LastStateMessageRole] = "lastStateMessage";
    }

    return names;
}

QVariantMap JobModel::get(const int row) const
{
    QHashIterator<int, QByteArray> iterator(roleNames());
    QVariantMap result;
    QModelIndex modelIndex = index(row, 0);

    while (iterator.hasNext()) {
        iterator.next();
        result[iterator.value()] = modelIndex.data(iterator.key());
    }

    return result;
}

QSharedPointer<PrinterJob> JobModel::getJobById(const int &id)
{
    Q_FOREACH(auto job, m_jobs) {
        if (job->jobId() == id) {
            return job;
        }
    }
    return QSharedPointer<PrinterJob>(Q_NULLPTR);
}


JobFilter::JobFilter(QObject *parent) : QSortFilterProxyModel(parent)
{
    connect(this, SIGNAL(sourceModelChanged()), SLOT(onSourceModelChanged()));
}

JobFilter::~JobFilter()
{
}

QVariantMap JobFilter::get(const int row) const
{
    QHashIterator<int, QByteArray> iterator(roleNames());
    QVariantMap result;
    QModelIndex modelIndex = index(row, 0);

    while (iterator.hasNext()) {
        iterator.next();
        result[iterator.value()] = modelIndex.data(iterator.key());
    }

    return result;
}

void JobFilter::onSourceModelChanged()
{
    connect((JobModel*) sourceModel(),
            SIGNAL(countChanged()),
            this,
            SIGNAL(countChanged()));
}

void JobFilter::onSourceModelCountChanged()
{
    Q_EMIT countChanged();
}

int JobFilter::count() const
{
    return rowCount();
}

void JobFilter::filterOnPrinterName(const QString &name)
{
    m_printerName = name;
    m_printerNameFilterEnabled = true;
    invalidate();
}

bool JobFilter::filterAcceptsRow(int sourceRow,
                                 const QModelIndex &sourceParent) const
{
    bool accepts = true;
    QModelIndex childIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    if (accepts && m_printerNameFilterEnabled) {
        QString printerName = childIndex.model()->data(
            childIndex, JobModel::PrinterNameRole).toString();
        accepts = m_printerName == printerName;
    }

    return accepts;
}
