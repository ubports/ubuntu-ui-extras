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
    QObject::connect(m_backend, &PrinterBackend::jobCreated,
                     this, &JobModel::jobCreated);
    QObject::connect(m_backend, &PrinterBackend::jobState,
                     this, &JobModel::jobState);
    QObject::connect(m_backend, &PrinterBackend::jobCompleted,
                     this, &JobModel::jobCompleted);

    connect(m_backend, SIGNAL(jobLoaded(QSharedPointer<PrinterJob>, QSharedPointer<PrinterJob>)),
            this, SLOT(updateJob(QSharedPointer<PrinterJob>, QSharedPointer<PrinterJob>)));

    // Add already existing jobs
    // FIXME: even this should probably be in a background thread?
    // so that it doesn't block startup?
    Q_FOREACH(auto job, m_backend->printerGetJobs()) {
        addJob(job);
    }
}

JobModel::~JobModel()
{
}

void JobModel::jobCreated(
        const QString &text, const QString &printer_uri,
        const QString &printer_name, uint printer_state,
        const QString &printer_state_reasons, bool printer_is_accepting_jobs,
        uint job_id, uint job_state, const QString &job_state_reasons,
        const QString &job_name, uint job_impressions_completed)
{
    Q_UNUSED(text);  // "Job Created"
    Q_UNUSED(printer_uri);
    Q_UNUSED(printer_state);
    Q_UNUSED(printer_state_reasons);
    Q_UNUSED(printer_is_accepting_jobs);
    Q_UNUSED(job_state_reasons);

    QSharedPointer<PrinterJob> job = QSharedPointer<PrinterJob>(
        new PrinterJob(printer_name, m_backend, job_id)
    );
    job->setImpressionsCompleted(job_impressions_completed);
    job->setState(static_cast<PrinterEnum::JobState>(job_state));
    job->setTitle(job_name);

    // Printers listens to rowInserted and spawns a JobLoader to set the
    // printer of the job, which triggers the extended attributes to be loaded
    // once complete this triggers JobModel::updateJob
    addJob(job);
}

void JobModel::jobState(
        const QString &text, const QString &printer_uri,
        const QString &printer_name, uint printer_state,
        const QString &printer_state_reasons, bool printer_is_accepting_jobs,
        uint job_id, uint job_state, const QString &job_state_reasons,
        const QString &job_name, uint job_impressions_completed)
{
    Q_UNUSED(text);
    Q_UNUSED(printer_uri);
    Q_UNUSED(printer_state);
    Q_UNUSED(printer_state_reasons);
    Q_UNUSED(printer_is_accepting_jobs);
    Q_UNUSED(job_state_reasons);
    Q_UNUSED(job_name);

    QSharedPointer<PrinterJob> job = getJob(printer_name, job_id);

    if (job) {
        job->setImpressionsCompleted(job_impressions_completed);
        job->setState(static_cast<PrinterEnum::JobState>(job_state));

        updateJob(job);
    } else {
        qWarning() << "JobModel::jobState for unknown job: " << job_name << " ("
                   << job_id << ") for " << printer_name;
    }
}

void JobModel::jobCompleted(
        const QString &text, const QString &printer_uri,
        const QString &printer_name, uint printer_state,
        const QString &printer_state_reasons, bool printer_is_accepting_jobs,
        uint job_id, uint job_state, const QString &job_state_reasons,
        const QString &job_name, uint job_impressions_completed)
{
    Q_UNUSED(text);
    Q_UNUSED(printer_uri);
    Q_UNUSED(printer_state);
    Q_UNUSED(printer_state_reasons);
    Q_UNUSED(printer_is_accepting_jobs);
    Q_UNUSED(job_state);
    Q_UNUSED(job_state_reasons);
    Q_UNUSED(job_name);
    Q_UNUSED(job_impressions_completed);

    auto job = getJob(printer_name, job_id);
    if (job) {
        removeJob(job);
    } else {
        qWarning() << "JobModel::jobCompleted for unknown job: " << job_name << " ("
                   << job_id << ") for " << printer_name;
    }
}

void JobModel::addJob(QSharedPointer<PrinterJob> job)
{
    int i = m_jobs.size();

    beginInsertRows(QModelIndex(), i, i);
    m_jobs.append(job);
    endInsertRows();

    Q_EMIT countChanged();
}

void JobModel::removeJob(QSharedPointer<PrinterJob> job)
{
    int i = m_jobs.indexOf(job);

    beginRemoveRows(QModelIndex(), i, i);
    m_jobs.removeAt(i);
    endRemoveRows();

    Q_EMIT countChanged();
}

// This is used by JobModel::jobState as it has modified an existing job
void JobModel::updateJob(QSharedPointer<PrinterJob> job)
{
    int i = m_jobs.indexOf(job);
    QModelIndex idx = index(i);
    Q_EMIT dataChanged(idx, idx);
}

// This is used by JobLoader as it creates a new job to prevent threading issues
void JobModel::updateJob(QSharedPointer<PrinterJob> oldJob,
                         QSharedPointer<PrinterJob> newJob)
{
    int i = m_jobs.indexOf(oldJob);
    QModelIndex idx = index(i);

    if (i > -1) {
        oldJob->updateFrom(newJob);
        Q_EMIT dataChanged(idx, idx);
    } else {
        qWarning() << "Tried to updateJob which doesn't exist:" << newJob->printerName() << newJob->jobId();
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
                ColorModel m = job->printer()->supportedColorModels().at(job->colorModel());
                ret = m.text.isEmpty() ? m.name : m.text;
            } else {
                qWarning() << "Printer is undefined, no colorModel";
                ret = "";
            }
            break;
        }
        case CompletedTimeRole:
            ret = job->completedTime();
            break;
        case CopiesRole:
            ret = job->copies();
            break;
        case CreationTimeRole:
            ret = job->creationTime();
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
        case HeldRole:
            ret = job->state() == PrinterEnum::JobState::Held;
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
            ret = job->processingTime();
            break;
        case QualityRole: {
            if (job->printer()) {
                PrintQuality q = job->printer()->supportedPrintQualities().at(job->quality());
                ret = q.text.isEmpty() ? q.name : q.text;
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
            ret = QVariant::fromValue<PrinterEnum::JobState>(job->state());
            break;
        case Qt::DisplayRole:
        case TitleRole:
            ret = job->title();
            break;
        case UserRole:
            ret = job->user();
            break;
        case LastStateMessageRole: {
            if (job->printer()) {
                ret = job->printer()->lastMessage();
            } else {
                ret = "";
            }
            break;
        }

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
        names[HeldRole] = "held";
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

QSharedPointer<PrinterJob> JobModel::getJob(const QString &printerName, const int &id)
{
    Q_FOREACH(auto job, m_jobs) {
        if (job->printerName() == printerName && job->jobId() == id) {
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

void JobFilter::filterOnActive()
{
    m_activeStates = QSet<PrinterEnum::JobState>{
        PrinterEnum::JobState::Processing,
    };
    m_activeFilterEnabled = true;
    invalidate();
}

void JobFilter::filterOnPaused()
{
    m_pausedStates = QSet<PrinterEnum::JobState>{
        PrinterEnum::JobState::Held,
    };
    m_pausedFilterEnabled = true;
    invalidate();
}

void JobFilter::filterOnQueued()
{
    m_queuedStates = QSet<PrinterEnum::JobState>{
        PrinterEnum::JobState::Pending,
    };
    m_queuedFilterEnabled = true;
    invalidate();
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

    if (accepts && m_activeFilterEnabled) {
        PrinterEnum::JobState state = childIndex.model()->data(
            childIndex, JobModel::StateRole
        ).value<PrinterEnum::JobState>();

        accepts = m_activeStates.contains(state);
    }

    if (accepts && m_pausedFilterEnabled) {
        PrinterEnum::JobState state = childIndex.model()->data(
            childIndex, JobModel::StateRole
        ).value<PrinterEnum::JobState>();

        accepts = m_pausedStates.contains(state);
    }

    if (accepts && m_queuedFilterEnabled) {
        PrinterEnum::JobState state = childIndex.model()->data(
            childIndex, JobModel::StateRole
        ).value<PrinterEnum::JobState>();

        accepts = m_queuedStates.contains(state);
    }

    return accepts;
}

bool JobFilter::lessThan(const QModelIndex &source_left,
                         const QModelIndex &source_right) const
{
    QVariant leftData = sourceModel()->data(source_left, sortRole());
    QVariant rightData = sourceModel()->data(source_right, sortRole());

    if (sortRole() == (int) JobModel::CreationTimeRole) {
        // If creationDateTime is the same, fallback to Id
        if (leftData.toDateTime() == rightData.toDateTime()) {
            int leftId = sourceModel()->data(source_left, JobModel::IdRole).toInt();
            int rightId = sourceModel()->data(source_right, JobModel::IdRole).toInt();

            return leftId < rightId;
        } else {
            return leftData.toDateTime() < rightData.toDateTime();
        }
    } else {
        return leftData < rightData;
    }
}
