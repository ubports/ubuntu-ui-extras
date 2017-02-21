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

#include "backend/backend_cups.h"
#include "backend/backend_pdf.h"
#include "i18n.h"
#include "models/jobmodel.h"
#include "models/printermodel.h"
#include "utils.h"

#include <QDebug>

PrinterModel::PrinterModel(PrinterBackend *backend, QObject *parent)
    : QAbstractListModel(parent)
    , m_backend(backend)
{

    QObject::connect(m_backend, &PrinterBackend::printerAdded,
                     this, &PrinterModel::printerAdded);
    QObject::connect(m_backend, &PrinterBackend::printerModified,
                     &m_signalHandler, &PrinterSignalHandler::onPrinterModified);
    QObject::connect(m_backend, &PrinterBackend::printerStateChanged,
                     &m_signalHandler, &PrinterSignalHandler::onPrinterModified);
    QObject::connect(m_backend, &PrinterBackend::printerDeleted,
                     this, &PrinterModel::printerDeleted);

    connect(&m_signalHandler, SIGNAL(printerModified(const QString&)),
            this, SLOT(printerModified(const QString&)));
    connect(m_backend, SIGNAL(printerLoaded(QSharedPointer<Printer>)),
            this, SLOT(printerLoaded(QSharedPointer<Printer>)));

    // Create printer proxies for every printerName.
    Q_FOREACH(auto printerName, m_backend->availablePrinterNames()) {
        auto p = QSharedPointer<Printer>(new Printer(new PrinterBackend(printerName)));
        addPrinter(p, CountChangeSignal::Defer);
    }

    // Add a PDF printer.
    auto pdfPrinter = QSharedPointer<Printer>(
        new Printer(new PrinterPdfBackend(__("Create PDF")))
    );
    addPrinter(pdfPrinter, CountChangeSignal::Defer);

    Q_EMIT countChanged();
}

PrinterModel::~PrinterModel()
{
}

void PrinterModel::printerLoaded(QSharedPointer<Printer> printer)
{
    // Find and possibly replace an old printer.
    for (int i=0; i < m_printers.count(); i++) {
        auto oldPrinter = m_printers.at(i);
        if (printer->name() == oldPrinter->name()) {
            if (!oldPrinter->deepCompare(printer)) {
                updatePrinter(oldPrinter, printer);
            }

            // We're done.
            return;
        }
    }

    addPrinter(printer, CountChangeSignal::Emit);
}

void PrinterModel::printerModified(const QString &printerName)
{
    // These signals might be emitted of a now deleted printer.
    if (getPrinterByName(printerName))
        m_backend->requestPrinter(printerName);
}

void PrinterModel::printerAdded(
    const QString &text, const QString &printerUri,
    const QString &printerName, uint printerState,
    const QString &printerStateReason, bool acceptingJobs)
{
    Q_UNUSED(text);
    Q_UNUSED(printerUri);
    Q_UNUSED(printerState);
    Q_UNUSED(printerStateReason);
    Q_UNUSED(acceptingJobs);

    m_backend->requestPrinter(printerName);
}

void PrinterModel::printerDeleted(
    const QString &text, const QString &printerUri,
    const QString &printerName, uint printerState,
    const QString &printerStateReason, bool acceptingJobs)
{
    Q_UNUSED(text);
    Q_UNUSED(printerUri);
    Q_UNUSED(printerState);
    Q_UNUSED(printerStateReason);
    Q_UNUSED(acceptingJobs);

    auto printer = getPrinterByName(printerName);
    if (printer) {
        removePrinter(printer, CountChangeSignal::Emit);
    }
}

QSharedPointer<Printer> PrinterModel::getPrinterByName(const QString &printerName)
{
    Q_FOREACH(auto p, m_printers) {
        if (p->name() == printerName)
            return p;
    }
    return QSharedPointer<Printer>(Q_NULLPTR);
}

void PrinterModel::movePrinter(const int &from, const int &to)
{
    int size = m_printers.size();
    if (from < 0 || to < 0 || from >= size || to >= size) {
        qWarning() << Q_FUNC_INFO << "Illegal move operation from"
                   << from << "to" << to << ". Size was" << size;
        return;
    }
    if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(), to)) {
        qWarning() << Q_FUNC_INFO << "failed to move rows.";
        return;
    }
    m_printers.move(from, to);
    endMoveRows();
}

void PrinterModel::removePrinter(QSharedPointer<Printer> printer, const CountChangeSignal &notify)
{
    int idx = m_printers.indexOf(printer);
    beginRemoveRows(QModelIndex(), idx, idx);
    m_printers.removeAt(idx);
    endRemoveRows();

    if (notify == CountChangeSignal::Emit)
        Q_EMIT countChanged();
}

void PrinterModel::updatePrinter(QSharedPointer<Printer> old,
                                  QSharedPointer<Printer> newPrinter)
{
    int i = m_printers.indexOf(old);
    QModelIndex idx = index(i);
    old->updateFrom(newPrinter);
    Q_EMIT dataChanged(idx, idx);
}

void PrinterModel::addPrinter(QSharedPointer<Printer> printer, const CountChangeSignal &notify)
{
    int i = m_printers.size();
    beginInsertRows(QModelIndex(), i, i);
    m_printers.append(printer);
    endInsertRows();

    if (notify == CountChangeSignal::Emit)
        Q_EMIT countChanged();
}

int PrinterModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_printers.size();
}

int PrinterModel::count() const
{
    return rowCount();
}

QVariant PrinterModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;

    if ((0<=index.row()) && (index.row()<m_printers.size())) {

        auto printer = m_printers[index.row()];


        /* If printer is a proxy (not loaded), determine if the requested role
        is something async and that we need to request the data. */
        if (printer->type() == PrinterEnum::PrinterType::ProxyType) {
            switch (role) {
            case Qt::DisplayRole:
            case NameRole:
            case DefaultPrinterRole:
            case PrinterRole:
            case IsPdfRole:
            case IsLoadedRole:
                break; // All of these can be inferred from the name (lazily).
            default:
                m_backend->requestPrinter(printer->name());
            }
        }

        switch (role) {
        case NameRole:
        case Qt::DisplayRole:
            ret = printer->name();
            break;
        case ColorModelRole:
            ret = printer->supportedColorModels().indexOf(printer->defaultColorModel());
            break;
        case SupportedColorModelsRole: {
                QStringList models;
                Q_FOREACH(const ColorModel &m, printer->supportedColorModels()) {
                    models.append(m.text.isEmpty() ? m.name : m.text);
                }
                ret = models;
            }
            break;
        // case CopiesRole:
        //     ret = printer->copies();
        //     break;
        case DefaultPrinterRole:
            ret = printer->name() == m_backend->defaultPrinterName();
            break;
        case DuplexRole:
            ret = printer->supportedDuplexModes().indexOf(printer->defaultDuplexMode());
            break;
        case SupportedDuplexModesRole:
            ret = printer->supportedDuplexStrings();
            break;
        // case PrintRangeRole:
        //     ret = printer->printRange();
        //     break;
        // case PrintRangeModeRole:
        //     ret = printer->printRangeMode();
        //     break;
        case PrintQualityRole:
            ret = printer->supportedPrintQualities().indexOf(printer->defaultPrintQuality());
            break;
        case SupportedPrintQualitiesRole: {
                QStringList qualities;
                Q_FOREACH(const PrintQuality &q, printer->supportedPrintQualities()) {
                    qualities.append(q.text.isEmpty() ? q.name : q.text);
                }
                ret = qualities;
            }
            break;
        case DescriptionRole:
            ret = printer->description();
            break;
        case PageSizeRole:
            ret = printer->defaultPageSize().name();
            break;
        case SupportedPageSizesRole: {
                QStringList sizes;
                Q_FOREACH(const QPageSize &s, printer->supportedPageSizes()) {
                    sizes << s.name();
                }
                ret = sizes;
            }
            break;

        // case AccessControlRole:
        //     ret = printer->accessControl();
        //     break;
        // case ErrorPolicyRole:
        //     ret = printer->errorPolicy();
        //     break;
        // case UsersRole:
        //     ret = printer->users();
        //     break;
        // case StateRole:
        //     ret = printer->state();
        //     break;
        case PrinterRole:
            ret = QVariant::fromValue(printer);
            break;
        case IsPdfRole:
            ret = printer->type() == PrinterEnum::PrinterType::PdfType;
            break;
        case IsLoadedRole:
            ret = printer->type() != PrinterEnum::PrinterType::ProxyType;
            break;
        case IsRawRole:
            ret = !printer->holdsDefinition();
            break;
        case JobRole:
            ret = QVariant::fromValue(printer->jobs());
            break;
        case EnabledRole:
            ret = printer->enabled();
            break;
        case AcceptJobsRole:
            ret = printer->acceptJobs();
            break;

        // case LastStateMessageRole:
        //     ret = printer->lastStateMessage();
        //     break;
        }
    }

    return ret;
}

bool PrinterModel::setData(const QModelIndex &index,
                           const QVariant &value, int role)
{
    if ((0<=index.row()) && (index.row()<m_printers.size())) {

        auto printer = m_printers[index.row()];

        switch (role) {
        case ColorModelRole: {
                int index = value.toInt();
                auto modes = printer->supportedColorModels();
                if (index >= 0 && modes.size() > index) {
                    printer->setDefaultColorModel(modes.at(index));
                }
            }
            break;
        case DescriptionRole:
            printer->setDescription(value.toString());
            break;
        case DuplexRole: {
                int index = value.toInt();
                auto modes = printer->supportedDuplexModes();
                if (index >= 0 && modes.size() > index) {
                    printer->setDefaultDuplexMode(modes.at(index));
                }
            }
            break;
        case PageSizeRole: {
                int index = value.toInt();
                QList<QPageSize> sizes = printer->supportedPageSizes();
                if (index >= 0 && sizes.size() > index) {
                    printer->setDefaultPageSize(sizes.at(index));
                }
            }
            break;
        case PrintQualityRole: {
                int index = value.toInt();
                QList<PrintQuality> quals = printer->supportedPrintQualities();
                if (index >= 0 && quals.size() > index) {
                    printer->setDefaultPrintQuality(quals.at(index));
                }
            }
            break;
        case EnabledRole:
            printer->setEnabled(value.toBool());
            break;
        case AcceptJobsRole:
            printer->setAcceptJobs(value.toBool());
            break;
        }
    }

    return true;
}

QHash<int, QByteArray> PrinterModel::roleNames() const
{
    static QHash<int,QByteArray> names;

    if (Q_UNLIKELY(names.empty())) {
        names[Qt::DisplayRole] = "displayName";
        names[ColorModelRole] = "colorModel";
        names[SupportedColorModelsRole] = "supportedColorModels";
        names[CopiesRole] = "copies";
        names[DefaultPrinterRole] = "default";
        names[DuplexRole] = "duplexMode";
        names[SupportedDuplexModesRole] = "supportedDuplexModes";
        names[NameRole] = "name";
        names[EnabledRole] = "printerEnabled";
        names[AcceptJobsRole] = "acceptJobs";
        names[PrintRangeRole] = "printRange";
        names[PrintRangeModeRole] = "printRangeMode";
        names[PdfModeRole] = "pdfMode";
        names[PrintQualityRole] = "printQuality";
        names[SupportedPrintQualitiesRole] = "supportedPrintQualities";
        names[DescriptionRole] = "description";
        names[PageSizeRole] = "pageSize";
        names[SupportedPageSizesRole] = "supportedPageSizes";
        names[AccessControlRole] = "accessControl";
        names[ErrorPolicyRole] = "errorPolicy";
        names[UsersRole] = "users";
        names[StateRole] = "state";
        names[PrinterRole] = "printer";
        names[IsPdfRole] = "isPdf";
        names[IsLoadedRole] = "isLoaded";
        names[IsRawRole] = "isRaw";
        names[JobRole] = "jobs";
        names[LastStateMessageRole] = "lastStateMessage";
    }

    return names;
}

QVariantMap PrinterModel::get(const int row) const
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

PrinterFilter::PrinterFilter(QObject *parent) : QSortFilterProxyModel(parent)
{
    connect(this, SIGNAL(sourceModelChanged()), SLOT(onSourceModelChanged()));
}

PrinterFilter::~PrinterFilter()
{

}

QVariantMap PrinterFilter::get(const int row) const
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

void PrinterFilter::onSourceModelChanged()
{
    connect((PrinterModel*) sourceModel(),
            SIGNAL(countChanged()),
            this,
            SIGNAL(countChanged()));
}

void PrinterFilter::onSourceModelCountChanged()
{
    Q_EMIT countChanged();
}

int PrinterFilter::count() const
{
    return rowCount();
}

void PrinterFilter::filterOnState(const PrinterEnum::State &state)
{
    Q_UNUSED(state);
}

void PrinterFilter::filterOnRecent(const bool recent)
{
    Q_UNUSED(recent);
}

void PrinterFilter::filterOnPdf(const bool pdf)
{
    m_pdfEnabled = true;
    m_pdf = pdf;
}

bool PrinterFilter::filterAcceptsRow(int sourceRow,
                                     const QModelIndex &sourceParent) const
{
    bool accepts = true;
    QModelIndex childIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    if (accepts && m_recentEnabled) {
        // TODO: implement recent
    }

    // If pdfEnabled is false we only want real printers
    if (accepts && m_pdfEnabled) {
        bool isPdf = (bool) childIndex.model()->data(
            childIndex, PrinterModel::IsPdfRole).toBool();
        accepts = isPdf == m_pdf;
    }

    if (accepts && m_stateEnabled) {
        const PrinterEnum::State state =
            (PrinterEnum::State) childIndex.model()->data(
                childIndex, PrinterModel::StateRole
            ).toInt();
        accepts = m_state == state;
    }

    return accepts;
}
bool PrinterFilter::lessThan(const QModelIndex &left,
                             const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left, sortRole());
    QVariant rightData = sourceModel()->data(right, sortRole());
    if ((QMetaType::Type) leftData.type() == QMetaType::Bool) {
        // FIXME: hack to put Pdf printer at the bottom
        if (leftData.toInt() == rightData.toInt()) {
            int leftPdf = sourceModel()->data(left, PrinterModel::IsPdfRole).toInt();
            int rightPdf = sourceModel()->data(right, PrinterModel::IsPdfRole).toInt();

            // If Pdf is also same then sort by name
            if (leftPdf == rightPdf) {
                QString leftName = sourceModel()->data(left, PrinterModel::NameRole).toString();
                QString rightName = sourceModel()->data(right, PrinterModel::NameRole).toString();

                return leftName > rightName;
            } else {
                return leftPdf > rightPdf;
            }
        } else {
            return leftData.toInt() < rightData.toInt();
        }
    } else {
        return leftData < rightData;
    }
}
