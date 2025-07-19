#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QObject>
#include <QPixmap>
#include <QVector>
#include <QMap>
#include <QString>
#include <QDir>

class ResourceLoader : public QObject
{
    Q_OBJECT

public:
    explicit ResourceLoader(QObject *parent = nullptr);
    
    // Load images from folder
    static QVector<QPixmap> importFolder(const QString& path);
    
    // Load images from folder as dictionary
    static QMap<QString, QPixmap> importFolderDict(const QString& path);
    
    // Load single image
    static QPixmap loadImage(const QString& path);
    
    // Check if file exists
    static bool fileExists(const QString& path);
    
    // Get all files in directory
    static QStringList getFilesInDirectory(const QString& path, const QStringList& filters = QStringList());
    
public:
    static QString getResourcePath(const QString& relativePath);

private:
    static QString getBasePath();
};

#endif // RESOURCELOADER_H