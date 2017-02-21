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

#ifndef USC_PRINTERS_PRINTERJOB_H
#define USC_PRINTERS_PRINTERJOB_H

#include "printers_global.h"

#include "enums.h"
#include "structs.h"

#include "backend/backend.h"
#include "printer/printer.h"

#include <QSharedPointer>
#include <QtCore/QDateTime>
#include <QtCore/QObject>

class Printer;
class PrinterBackend;

class PRINTERS_DECL_EXPORT PrinterJob : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool collate READ collate WRITE setCollate NOTIFY collateChanged)
    Q_PROPERTY(int colorModel READ colorModel WRITE setColorModel NOTIFY colorModelChanged)
    Q_PROPERTY(PrinterEnum::ColorModelType colorModelType READ colorModelType NOTIFY colorModelTypeChanged)
    Q_PROPERTY(QDateTime completedTime READ completedTime NOTIFY completedTimeChanged)
    Q_PROPERTY(int copies READ copies WRITE setCopies NOTIFY copiesChanged)
    Q_PROPERTY(QDateTime creationTime READ creationTime NOTIFY creationTimeChanged)
    Q_PROPERTY(int duplexMode READ duplexMode WRITE setDuplexMode NOTIFY duplexModeChanged)
    Q_PROPERTY(int impressionsCompleted READ impressionsCompleted NOTIFY impressionsCompletedChanged)
    Q_PROPERTY(bool isTwoSided READ isTwoSided NOTIFY isTwoSidedChanged)
    Q_PROPERTY(bool landscape READ landscape WRITE setLandscape NOTIFY landscapeChanged)
    Q_PROPERTY(QStringList messages READ messages NOTIFY messagesChanged)
    Q_PROPERTY(QSharedPointer<Printer> printer READ printer WRITE setPrinter NOTIFY printerChanged)
    Q_PROPERTY(QString printerName READ printerName NOTIFY printerNameChanged)
    Q_PROPERTY(QString printRange READ printRange WRITE setPrintRange NOTIFY printRangeChanged)
    Q_PROPERTY(PrinterEnum::PrintRange printRangeMode READ printRangeMode WRITE setPrintRangeMode NOTIFY printRangeModeChanged)
    Q_PROPERTY(QDateTime processingTime READ processingTime NOTIFY processingTimeChanged)
    Q_PROPERTY(int quality READ quality WRITE setQuality NOTIFY qualityChanged)
    Q_PROPERTY(bool reverse READ reverse WRITE setReverse NOTIFY reverseChanged)
    Q_PROPERTY(int size READ size NOTIFY sizeChanged)
    Q_PROPERTY(PrinterEnum::JobState state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString user READ user NOTIFY userChanged)

    friend class PrinterCupsBackend;
public:
    explicit PrinterJob(QString dest,
                        PrinterBackend *backend,
                        QObject *parent=Q_NULLPTR);
    explicit PrinterJob(QString dest, PrinterBackend *backend, int jobId,
                        QObject *parent=Q_NULLPTR);
    ~PrinterJob();

    bool collate() const;
    int colorModel() const;
    PrinterEnum::ColorModelType colorModelType() const;
    QDateTime completedTime() const;
    int copies() const;
    QDateTime creationTime() const;
    int duplexMode() const;
    int impressionsCompleted() const;
    bool isTwoSided() const;
    int jobId() const;
    bool landscape() const;
    QStringList messages() const;
    QSharedPointer<Printer> printer() const;
    QString printerName() const;
    QString printRange() const;
    PrinterEnum::PrintRange printRangeMode() const;
    QDateTime processingTime() const;
    int quality() const;
    bool reverse() const;
    int size() const;
    PrinterEnum::JobState state() const;
    QString title() const;
    QString user() const;

    bool deepCompare(QSharedPointer<PrinterJob> other) const;
public Q_SLOTS:
    PrinterEnum::DuplexMode getDuplexMode() const;
    ColorModel getColorModel() const;
    PrintQuality getPrintQuality() const;
    Q_INVOKABLE void printFile(const QUrl &url);
    void setCollate(const bool collate);
    void setColorModel(const int colorModel);
    void setCopies(const int copies);
    void setDuplexMode(const int duplexMode);
    void setImpressionsCompleted(const int &impressionsCompleted);
    void setLandscape(const bool landscape);
    void setPrinter(QSharedPointer<Printer> printer);
    void setPrintRange(const QString &printRange);
    void setPrintRangeMode(const PrinterEnum::PrintRange printRangeMode);
    void setQuality(const int quality);
    void setReverse(const bool reverse);
    void setTitle(const QString &title);

    void updateFrom(QSharedPointer<PrinterJob> other);
private Q_SLOTS:
    void loadDefaults();
    void setCompletedTime(const QDateTime &completedTime);
    void setCreationTime(const QDateTime &creationTime);
    void setIsTwoSided(const bool isTwoSided);
    void setMessages(const QStringList &messages);
    void setProcessingTime(const QDateTime &processingTime);
    void setSize(const int size);
    void setState(const PrinterEnum::JobState &state);
    void setUser(const QString &user);
Q_SIGNALS:
    void collateChanged();
    void colorModelChanged();
    void colorModelTypeChanged();
    void completedTimeChanged();
    void copiesChanged();
    void creationTimeChanged();
    void duplexModeChanged();
    void impressionsCompletedChanged();
    void isTwoSidedChanged();
    void landscapeChanged();
    void messagesChanged();
    void printerChanged();
    void printerNameChanged();
    void printRangeChanged();
    void printRangeModeChanged();
    void processingTimeChanged();
    void qualityChanged();
    void reverseChanged();
    void sizeChanged();
    void stateChanged();
    void titleChanged();
    void userChanged();
private:
    bool m_collate;
    int m_color_model;
    QDateTime m_completed_time;
    int m_copies;
    QDateTime m_creation_time;
    PrinterBackend *m_backend; // TODO: Maybe use the printer's backend?
    QString m_printerName;
    int m_duplex_mode;
    int m_impressions_completed;
    bool m_is_two_sided;
    int m_job_id;
    bool m_landscape;
    QStringList m_messages;
    QSharedPointer<Printer> m_printer;
    QString m_print_range;
    PrinterEnum::PrintRange m_print_range_mode;
    QDateTime m_processing_time;
    int m_quality;
    bool m_reverse;
    int m_size;
    PrinterEnum::JobState m_state;
    QString m_title;
    QString m_user;
};

#endif // USC_PRINTERS_PRINTERJOB_H
