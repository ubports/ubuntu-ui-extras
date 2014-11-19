#ifndef PHOTOUTILS_H
#define PHOTOUTILS_H

#include <QObject>

class FileUtils : public QObject
{
    Q_OBJECT
public:
    explicit FileUtils(QObject *parent = 0);

    Q_INVOKABLE bool createDirectory(QString path) const;
    Q_INVOKABLE bool removeDirectory(QString path, bool recursive = false) const;
    Q_INVOKABLE QString createTemporaryDirectory(QString pathTemplate) const;

    Q_INVOKABLE bool remove(QString path) const;
    Q_INVOKABLE bool copy(QString sourceFile, QString destinationFile) const;
    Q_INVOKABLE bool rename(QString sourceFile, QString destinationFile) const;

    Q_INVOKABLE QString parentDirectory(QString path) const;
    Q_INVOKABLE QString nameFromPath(QString path) const;

    Q_INVOKABLE bool exists(QString path) const;
};

#endif // PHOTOUTILS_H
