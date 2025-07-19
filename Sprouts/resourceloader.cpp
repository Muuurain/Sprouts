#include "resourceloader.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

ResourceLoader::ResourceLoader(QObject *parent)
    : QObject{parent}
{
}

QVector<QPixmap> ResourceLoader::importFolder(const QString& path)
{
    QVector<QPixmap> surfaceList;
    
    // Convert relative path to absolute path from source directory
    QString absolutePath = getResourcePath(path);
    QDir dir(absolutePath);
    
    if (!dir.exists()) {
        qDebug() << "Directory does not exist:" << absolutePath;
        return surfaceList;
    }
    
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Name);
    
    QStringList imageFiles = dir.entryList(QDir::Files);
    
    for (const QString& imageFile : imageFiles) {
        QString fullPath = dir.absoluteFilePath(imageFile);
        QPixmap pixmap(fullPath);
        
        if (!pixmap.isNull()) {
            surfaceList.append(pixmap);
        } else {
            qDebug() << "Failed to load image:" << fullPath;
        }
    }
    
    return surfaceList;
}

QMap<QString, QPixmap> ResourceLoader::importFolderDict(const QString& path)
{
    QMap<QString, QPixmap> surfaceDict;
    
    // Convert relative path to absolute path from source directory
    QString absolutePath = getResourcePath(path);
    QDir dir(absolutePath);
    
    if (!dir.exists()) {
        qDebug() << "Directory does not exist:" << absolutePath;
        return surfaceDict;
    }
    
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    dir.setNameFilters(filters);
    
    QStringList imageFiles = dir.entryList(QDir::Files);
    
    for (const QString& imageFile : imageFiles) {
        QString fullPath = dir.absoluteFilePath(imageFile);
        QPixmap pixmap(fullPath);
        
        if (!pixmap.isNull()) {
            QString baseName = QFileInfo(imageFile).baseName();
            surfaceDict[baseName] = pixmap;
        } else {
            qDebug() << "Failed to load image:" << fullPath;
        }
    }
    
    return surfaceDict;
}

QPixmap ResourceLoader::loadImage(const QString& path)
{
    // Convert relative path to absolute path from source directory
    QString absolutePath = getResourcePath(path);
    QPixmap pixmap(absolutePath);
    if (pixmap.isNull()) {
        qDebug() << "Failed to load image:" << absolutePath;
    }
    return pixmap;
}

bool ResourceLoader::fileExists(const QString& path)
{
    return QFileInfo::exists(path);
}

QStringList ResourceLoader::getFilesInDirectory(const QString& path, const QStringList& filters)
{
    QDir dir(path);
    if (!dir.exists()) {
        return QStringList();
    }
    
    if (!filters.isEmpty()) {
        dir.setNameFilters(filters);
    }
    
    return dir.entryList(QDir::Files);
}

QString ResourceLoader::getBasePath()
{
    return QCoreApplication::applicationDirPath();
}

QString ResourceLoader::getResourcePath(const QString& relativePath)
{
    QString appDir = QCoreApplication::applicationDirPath();
    
    // First, try to find the resource in the application directory (for deployed apps)
    QString deployedPath = QDir(appDir).absoluteFilePath(relativePath);
    if (QFileInfo::exists(deployedPath)) {
        return deployedPath;
    }
    
    // If not found in app directory, try development environment paths
    QDir dir(appDir);
    
    // Navigate up from build directory to source directory
    // Typical structure: source/build/Desktop_Qt_6_5_3_MinGW_64_bit-Debug/debug/
    while (dir.dirName().contains("build") || dir.dirName().contains("debug") || 
           dir.dirName().contains("release") || dir.dirName().contains("Desktop")) {
        if (!dir.cdUp()) break;
    }
    
    // If we're still in a build-related directory, try to find the source
    if (!QFileInfo(dir.absoluteFilePath("Sprouts.pro")).exists()) {
        // Try going up one more level and look for Sprouts folder
        dir.cdUp();
        if (dir.exists("Sprouts")) {
            dir.cd("Sprouts");
        }
    }
    
    QString sourcePath = dir.absoluteFilePath(relativePath);
    
    // If source path exists, return it; otherwise return the deployed path anyway
    if (QFileInfo::exists(sourcePath)) {
        return sourcePath;
    }
    
    return deployedPath;
}