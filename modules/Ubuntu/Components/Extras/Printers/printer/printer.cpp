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

#include "i18n.h"
#include "utils.h"

#include "printer.h"

#include <QDebug>
#include <QLocale>
#include <QQmlEngine>

Printer::Printer(PrinterBackend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
{
    loadAttributes();

    m_jobs.filterOnPrinterName(name());

    QObject::connect(m_backend, &PrinterBackend::printerStateChanged,
                     this, &Printer::onPrinterStateChanged);
}

Printer::~Printer()
{
    m_backend->deleteLater();
}

void Printer::setJobModel(JobModel* jobModel)
{
    if (!m_jobs.sourceModel()) {
        m_jobs.setSourceModel(jobModel);
    }
}

void Printer::updateAcceptJobs(const QMap<QString, QVariant> &serverAttrs)
{
    m_acceptJobs = serverAttrs.value(QStringLiteral("AcceptJobs")).toBool();
}

void Printer::updateColorModel(const QMap<QString, QVariant> &serverAttrs)
{
    auto defModel = QStringLiteral("DefaultColorModel");
    auto models = QStringLiteral("SupportedColorModels");

    m_defaultColorModel = serverAttrs.value(defModel).value<ColorModel>();
    m_supportedColorModels = serverAttrs.value(models).value<QList<ColorModel>>();

    if (m_supportedColorModels.size() == 0) {
        m_supportedColorModels.append(m_defaultColorModel);
    }
}

void Printer::updatePrintQualities(const QMap<QString, QVariant> &serverAttrs)
{
    auto defQuality = QStringLiteral("DefaultPrintQuality");
    auto qualities = QStringLiteral("SupportedPrintQualities");

    m_supportedPrintQualities = serverAttrs.value(qualities).value<QList<PrintQuality>>();
    m_defaultPrintQuality = serverAttrs.value(defQuality).value<PrintQuality>();

    if (m_supportedPrintQualities.size() == 0) {
        m_supportedPrintQualities.append(m_defaultPrintQuality);
    }
}

void Printer::updateLastMessage(const QMap<QString, QVariant> &serverAttrs)
{
    m_stateMessage = serverAttrs.value(QStringLiteral("StateMessage")).toString();
}


void Printer::updateDeviceUri(const QMap<QString, QVariant> &serverAttrs)
{
    m_deviceUri = serverAttrs.value(QStringLiteral("DeviceUri")).toString();
}

void Printer::updateCopies(const QMap<QString, QVariant> &serverAttrs)
{
    m_copies = serverAttrs.value(QStringLiteral("Copies")).toInt();
    if (m_copies <= 0)
        m_copies = 1;
}

void Printer::updateShared(const QMap<QString, QVariant> &serverAttrs)
{
    m_shared = serverAttrs.value(QStringLiteral("Shared")).toBool();
}

void Printer::loadAttributes()
{
    auto opts = QStringList({
        QStringLiteral("AcceptJobs"),
        QStringLiteral("DefaultColorModel"),
        QStringLiteral("SupportedColorModels"),
        QStringLiteral("DefaultPrintQuality"),
        QStringLiteral("SupportedPrintQualities"),
        QStringLiteral("StateMessage"),
        QStringLiteral("DeviceUri"),
        QStringLiteral("Copies"),
        QStringLiteral("Shared"),
    });
    auto result = m_backend->printerGetOptions(name(), opts);

    updateAcceptJobs(result);
    updateColorModel(result);
    updatePrintQualities(result);
    updateLastMessage(result);
    updateDeviceUri(result);
    updateCopies(result);
    updateShared(result);
}

ColorModel Printer::defaultColorModel() const
{
    return m_defaultColorModel;
}

QList<ColorModel> Printer::supportedColorModels() const
{
    return m_supportedColorModels;
}

PrintQuality Printer::defaultPrintQuality() const
{
    return m_defaultPrintQuality;
}

QList<PrintQuality> Printer::supportedPrintQualities() const
{
    return m_supportedPrintQualities;
}

QList<PrinterEnum::DuplexMode> Printer::supportedDuplexModes() const
{
    return m_backend->supportedDuplexModes();
}

QStringList Printer::supportedDuplexStrings() const
{
    QStringList list;
    Q_FOREACH(const PrinterEnum::DuplexMode &mode, supportedDuplexModes()) {
        list << Utils::duplexModeToUIString(mode);
    }
    return list;
}

PrinterEnum::DuplexMode Printer::defaultDuplexMode() const
{
    return m_backend->defaultDuplexMode();
}

int Printer::printFile(const QString &filepath, const PrinterJob *options)
{
    auto dest = m_backend->makeDest(name(), options);  // options could be QMap<QString, QString> ?

    qDebug() << "Going to print:" << filepath << options->title();
    return m_backend->printFileToDest(filepath, options->title(), dest);
}

QString Printer::name() const
{
    return m_backend->printerName();
}

QString Printer::deviceUri() const
{
    return m_deviceUri;
}

QString Printer::make() const
{
    return m_backend->makeAndModel();
}

QString Printer::description() const
{
    return m_backend->description();
}

QString Printer::location() const
{
    return m_backend->location();
}

QPageSize Printer::defaultPageSize() const
{
    return m_backend->defaultPageSize();
}

QList<QPageSize> Printer::supportedPageSizes() const
{
    return m_backend->supportedPageSizes();
}

PrinterEnum::AccessControl Printer::accessControl() const
{
    return PrinterEnum::AccessControl::AccessAllow;
}

PrinterEnum::ErrorPolicy Printer::errorPolicy() const
{
    return PrinterEnum::ErrorPolicy::RetryOnError;
}

bool Printer::enabled() const
{
    return state() != PrinterEnum::State::ErrorState;
}

PrinterEnum::State Printer::state() const
{
    return m_backend->state();
}

bool Printer::shared() const
{
    return m_shared;
}

bool Printer::acceptJobs() const
{
    return m_acceptJobs;
}

bool Printer::holdsDefinition() const
{
    return m_backend->holdsDefinition();
}

bool Printer::isRemote() const
{
    return m_backend->isRemote();
}

PrinterEnum::PrinterType Printer::type() const
{
    return m_backend->type();
}

int Printer::copies() const
{
    return m_copies;
}

void Printer::setDefaultColorModel(const ColorModel &colorModel)
{
    if (defaultColorModel() == colorModel) {
        return;
    }

    if (!supportedColorModels().contains(colorModel)) {
        qWarning() << Q_FUNC_INFO << "color model not supported";
        return;
    }

    QStringList vals({colorModel.name});
    m_backend->printerAddOption(name(), "ColorModel", vals);
}

void Printer::setDescription(const QString &description)
{
    if (this->description() != description) {
        m_backend->printerSetInfo(name(), description);
    }
}

void Printer::setDefaultDuplexMode(const PrinterEnum::DuplexMode &duplexMode)
{
    if (defaultDuplexMode() == duplexMode) {
        return;
    }

    if (!m_backend->supportedDuplexModes().contains(duplexMode)) {
        qWarning() << Q_FUNC_INFO << "duplex mode not supported" << duplexMode;
        return;
    }

    QStringList vals({Utils::duplexModeToPpdChoice(duplexMode)});
    m_backend->printerAddOption(name(), "Duplex", vals);
}

void Printer::setEnabled(const bool enabled)
{
    if (this->enabled() != enabled) {
        QString reply = m_backend->printerSetEnabled(name(), enabled);
        if (!reply.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "failed to set enabled:" << reply;
        }
    }
}

void Printer::setAcceptJobs(const bool accepting)
{
    if (this->acceptJobs() != accepting) {
        QString reply = m_backend->printerSetAcceptJobs(name(), accepting);
        if (!reply.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "failed to set accepting:" << reply;
        }
    }
}

void Printer::setShared(const bool shared)
{
    if (this->shared() != shared) {
        QString reply = m_backend->printerSetShared(name(), shared);
        if (!reply.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "failed to set shared:" << reply;
        }
    }
}

void Printer::setDefaultPrintQuality(const PrintQuality &quality)
{
    if (defaultPrintQuality() == quality) {
        return;
    }

    if (!supportedPrintQualities().contains(quality)) {
        qWarning() << Q_FUNC_INFO << "quality not supported.";
        return;
    }

    QStringList vals({quality.name});
    m_backend->printerAddOption(name(), quality.originalOption, vals);
}

void Printer::setDefaultPageSize(const QPageSize &pageSize)
{
    if (defaultPageSize() == pageSize) {
        return;
    }

    if (!m_backend->supportedPageSizes().contains(pageSize)) {
        qWarning() << Q_FUNC_INFO << "pagesize not supported.";
        return;
    }

    if (pageSize.key().isEmpty()) {
        qWarning() << Q_FUNC_INFO << "pagesize does not expose a ppd key.";
        return;
    }

    QStringList vals({pageSize.key()});
    m_backend->printerAddOption(name(), "PageSize", vals);
    m_backend->refresh();
}

void Printer::setCopies(const int &copies)
{
    if (this->copies() == copies) {
        return;
    }

    m_backend->printerSetCopies(name(), copies);
}

QString Printer::lastMessage() const
{
    return m_stateMessage;
}

QAbstractItemModel* Printer::jobs()
{
    auto ret = &m_jobs;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

bool Printer::deepCompare(QSharedPointer<Printer> other) const
{
    // Return true if they are the same
    return defaultColorModel() == other->defaultColorModel()
            && defaultPrintQuality() == other->defaultPrintQuality()
            && description() == other->description()
            && defaultDuplexMode() == other->defaultDuplexMode()
            && defaultPageSize() == other->defaultPageSize()
            && type() == other->type()
            && acceptJobs() == other->acceptJobs()
            && enabled() == other->enabled()
            && state() == other->state()
            && lastMessage() == other->lastMessage()
            && deviceUri() == other->deviceUri()
            && shared() == other->shared()
            && copies() == other->copies()
            && isRemote() == other->isRemote();
}

void Printer::updateFrom(QSharedPointer<Printer> other)
{
    PrinterBackend *tmp = m_backend;

    // Copy values from other printer which has been loaded in another thread
    // Note: do not use loadAttributes otherwise can cause UI block
    m_acceptJobs = other->m_acceptJobs;
    m_backend = other->m_backend;
    m_defaultColorModel = other->m_defaultColorModel;
    m_defaultPrintQuality = other->m_defaultPrintQuality;
    m_deviceUri = other->m_deviceUri;
    m_shared = other->m_shared;
    m_stateMessage = other->m_stateMessage;
    m_supportedColorModels = other->m_supportedColorModels;
    m_supportedPrintQualities = other->m_supportedPrintQualities;

    other->m_backend = tmp;
}

void Printer::onPrinterStateChanged(
        const QString &text, const QString &printerUri,
        const QString &printerName, uint printerState,
        const QString &printerStateReason, bool acceptingJobs)
{
    Q_UNUSED(printerUri);
    Q_UNUSED(printerState);
    Q_UNUSED(printerStateReason);
    Q_UNUSED(acceptingJobs);

    if (name() == printerName) {
        m_stateMessage = text;
    }
}
