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

#include "printer.h"

#include <QDebug>
#include <QQmlEngine>

Printer::Printer(PrinterBackend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
{
    loadColorModel();
    loadPrintQualities();
    loadAcceptJobs();

    m_jobs.filterOnPrinterName(name());
}

Printer::~Printer()
{
    m_backend->deleteLater();
}

void Printer::setJobModel(JobModel* jobModel)
{
    if (!m_jobs.sourceModel()) {
        m_jobs.setSourceModel(jobModel);
        m_jobs.sort(JobModel::Roles::IdRole);
    }
}

void Printer::loadAcceptJobs()
{
    auto opt = QStringLiteral("AcceptJobs");
    m_acceptJobs = m_backend->printerGetOption(name(), opt).toBool();
}

void Printer::loadColorModel()
{
    auto defModel = QStringLiteral("DefaultColorModel");
    auto models = QStringLiteral("SupportedColorModels");
    auto result = m_backend->printerGetOptions(
        name(), QStringList({defModel, models})
    );

    m_defaultColorModel = result.value(defModel).value<ColorModel>();
    m_supportedColorModels = result.value(models).value<QList<ColorModel>>();

    if (m_supportedColorModels.size() == 0) {
        m_supportedColorModels.append(m_defaultColorModel);
    }
}

void Printer::loadPrintQualities()
{
    auto defQuality = QStringLiteral("DefaultPrintQuality");
    auto qualities = QStringLiteral("SupportedPrintQualities");
    auto result = m_backend->printerGetOptions(
        name(), QStringList({defQuality, qualities})
    );

    m_supportedPrintQualities = result.value(qualities).value<QList<PrintQuality>>();
    m_defaultPrintQuality = result.value(defQuality).value<PrintQuality>();

    if (m_supportedPrintQualities.size() == 0) {
        m_supportedPrintQualities.append(m_defaultPrintQuality);
    }
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

QString Printer::description() const
{
    return m_backend->description();
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
    // TODO: implement
    return PrinterEnum::AccessControl::AccessAllow;
}

PrinterEnum::ErrorPolicy Printer::errorPolicy() const
{
    // TODO: implement
    return PrinterEnum::ErrorPolicy::RetryOnError;
}

bool Printer::enabled() const
{
    return state() != PrinterEnum::State::ErrorState;
}

QStringList Printer::users() const
{
    // TODO: implement
    return QStringList();
}

PrinterEnum::State Printer::state() const
{
    return m_backend->state();
}

QString Printer::lastStateMessage() const
{
    // TODO: implement
    return QString();
}

bool Printer::acceptJobs() const
{
    return m_acceptJobs;
}

bool Printer::holdsDefinition() const
{
    return m_backend->holdsDefinition();
}

PrinterEnum::PrinterType Printer::type() const
{
    return m_backend->type();
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
    QString reply = m_backend->printerAddOption(name(), "ColorModel", vals);
    Q_UNUSED(reply);
}

void Printer::setAccessControl(const PrinterEnum::AccessControl &accessControl)
{
    // TODO: implement
    Q_UNUSED(accessControl);
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
    QString reply = m_backend->printerAddOption(name(), "Duplex", vals);
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

void Printer::setErrorPolicy(const PrinterEnum::ErrorPolicy &errorPolicy)
{
    // TODO: implement
    Q_UNUSED(errorPolicy);
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
    QString reply = m_backend->printerAddOption(name(), quality.originalOption, vals);
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
    QString reply = m_backend->printerAddOption(name(), "PageSize", vals);

    m_backend->refresh();
}

void Printer::addUser(const QString &username)
{
    // TODO: implement
    Q_UNUSED(username);
}

void Printer::removeUser(const QString &username)
{
    // TODO: implement
    Q_UNUSED(username);
}

QAbstractItemModel* Printer::jobs()
{
    auto ret = &m_jobs;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

bool Printer::deepCompare(QSharedPointer<Printer> other) const
{
    bool changed = false;

    changed |= defaultColorModel() != other->defaultColorModel();
    changed |= defaultPrintQuality() != other->defaultPrintQuality();
    changed |= description() != other->description();
    changed |= defaultDuplexMode() != other->defaultDuplexMode();
    changed |= defaultPageSize() != other->defaultPageSize();
    changed |= type() != other->type();
    changed |= acceptJobs() != other->acceptJobs();
    changed |= enabled() != other->enabled();
    changed |= state() != other->state();

    // TODO: accessControl
    // TODO: errorPolicy

    // Return true if they are the same, so no change
    return changed == false;
}

void Printer::updateFrom(QSharedPointer<Printer> other)
{
    PrinterBackend *tmp = m_backend;
    m_backend = other->m_backend;
    other->m_backend = tmp;

    loadColorModel();
    loadPrintQualities();
    loadAcceptJobs();
}
