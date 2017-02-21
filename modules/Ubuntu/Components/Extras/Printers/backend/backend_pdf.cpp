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
#include "backend/backend_pdf.h"

PrinterPdfBackend::PrinterPdfBackend(const QString &printerName,
                                     QObject *parent)
    : PrinterBackend(printerName, parent)
{
    m_type = PrinterEnum::PrinterType::PdfType;
}

QVariant PrinterPdfBackend::printerGetOption(const QString &name,
                                             const QString &option) const
{
    auto res = printerGetOptions(name, QStringList({option}));
    return res[option];
}

QMap<QString, QVariant> PrinterPdfBackend::printerGetOptions(
    const QString &name, const QStringList &options) const
{
    Q_UNUSED(name);

    QMap<QString, QVariant> ret;

    ColorModel rgb;
    rgb.colorType = PrinterEnum::ColorModelType::ColorType;
    rgb.name = "RGB";
    rgb.text = __("Color");

    PrintQuality quality;
    quality.name = __("Normal");

    Q_FOREACH(const QString &option, options) {
        if (option == QLatin1String("DefaultColorModel")) {
            ret[option] = QVariant::fromValue(rgb);
        } else if (option == QLatin1String("DefaultPrintQuality")) {
            ret[option] = QVariant::fromValue(quality);
        } else if (option == QLatin1String("SupportedPrintQualities")) {
            auto qualities = QList<PrintQuality>({quality});
            ret[option] = QVariant::fromValue(qualities);
        } else if (option == QLatin1String("SupportedColorModels")) {
            auto models = QList<ColorModel>{rgb};
            ret[option] = QVariant::fromValue(models);
        } else if (option == QLatin1String("AcceptJobs")) {
            ret[option] = true;
        } else {
            throw std::invalid_argument("Invalid value for PDF printer: " + option.toStdString());
        }
    }

    return ret;
}

QString PrinterPdfBackend::printerName() const
{
    return m_printerName;
}

PrinterEnum::State PrinterPdfBackend::state() const
{
    return PrinterEnum::State::IdleState;
}

QList<QPageSize> PrinterPdfBackend::supportedPageSizes() const
{
    return QList<QPageSize>{QPageSize(QPageSize::A4)};
}

QPageSize PrinterPdfBackend::defaultPageSize() const
{
    return QPageSize(QPageSize::A4);
}

bool PrinterPdfBackend::supportsCustomPageSizes() const
{
    return false;
}

QPageSize PrinterPdfBackend::minimumPhysicalPageSize() const
{
    return QPageSize(QPageSize::A4);
}

QPageSize PrinterPdfBackend::maximumPhysicalPageSize() const
{
    return QPageSize(QPageSize::A4);
}

QList<int> PrinterPdfBackend::supportedResolutions() const
{
    return QList<int>{};
}

PrinterEnum::DuplexMode PrinterPdfBackend::defaultDuplexMode() const
{
    return PrinterEnum::DuplexMode::DuplexNone;
}

QList<PrinterEnum::DuplexMode> PrinterPdfBackend::supportedDuplexModes() const
{
    return QList<PrinterEnum::DuplexMode>{PrinterEnum::DuplexMode::DuplexNone};
}

