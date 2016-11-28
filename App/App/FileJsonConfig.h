#ifndef FILEJSONCONFIG_H
#define FILEJSONCONFIG_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QSize>
#include <QFile>
#include <QUrl>
#include <QStandardPaths>
#include "ARCalibrationConfig.h"
#include "ARInitConfig.h"
#include "ARMapPointsDetectorConfig.h"
#include "ARTrackerConfig.h"
#include <QtMath>
#include <QCamera>
#include <QCameraInfo>

#if defined(__ANDROID__)
#include "AndroidCamera/AndroidCameraView.h"
#endif

class FileJsonConfig:
        public QObject
{
    Q_OBJECT

    Q_PROPERTY(QSize frameResolution READ frameResolution WRITE setFrameResolution NOTIFY frameResolutionChanged)
public:
    FileJsonConfig() {
        m_frameResolution = QSize(0, 480);
    }

    void add(const QString& name, const QJsonValue& value)
    {
        m_main.insert(name, value);
    }

    void add(const QString& name, const QJsonObject& object)
    {
        m_main.insert(name, object);
    }

    QSize frameResolution() const
    {
        return m_frameResolution;
    }
    void setFrameResolution(const QSize& value)
    {
        m_frameResolution = value;
        emit frameResolutionChanged();
    }

    bool saveCameraInfo(const QUrl& url, const QCameraInfo& cameraInfo) const
    {
        QJsonObject log;
        log.insert("description", cameraInfo.description());
        log.insert("deviceName", cameraInfo.deviceName());
        log.insert("orientation", cameraInfo.orientation());
        QCamera::Position position = cameraInfo.position();
        QString strPosition;
        if (position == QCamera::UnspecifiedPosition)
            strPosition = "UnspecifiedPosition";
        else if (position == QCamera::BackFace)
            strPosition = "BackFace";
        else if (position == QCamera::FrontFace)
            strPosition = "FrontFace";
        else
            strPosition = "null";
        log.insert("position", strPosition);

        QString path = url.path();
#ifdef _WIN32
        if (path.at(0) == '/')
            path = path.right(path.size() - 1);
#endif
        QFile file(path);
        if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
            qWarning(QString("Couldn't open load file: %1").arg(path).toStdString().c_str());
            return false;
        }
        QJsonDocument document(log);
        file.write(document.toJson(QJsonDocument::Indented));
        return true;
    }

#if defined(__ANDROID__)
    bool saveCameraInfo(const QUrl& url, const AndroidJniCamera::AndroidCameraInfo& cameraInfo) const
    {
        QJsonObject log;
        log.insert("description", cameraInfo.description);
        log.insert("deviceName", QString(cameraInfo.name.toStdString().c_str()));
        log.insert("orientation", cameraInfo.orientation);
        QCamera::Position position = cameraInfo.position;
        QString strPosition;
        if (position == QCamera::UnspecifiedPosition)
            strPosition = "UnspecifiedPosition";
        else if (position == QCamera::BackFace)
            strPosition = "BackFace";
        else if (position == QCamera::FrontFace)
            strPosition = "FrontFace";
        else
            strPosition = "null";
        log.insert("position", strPosition);

        QString path = url.path();
#ifdef _WIN32
        if (path.at(0) == '/')
            path = path.right(path.size() - 1);
#endif
        QFile file(path);
        if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
            qWarning(QString("Couldn't open load file: %1").arg(path).toStdString().c_str());
            return false;
        }
        QJsonDocument document(log);
        file.write(document.toJson(QJsonDocument::Indented));
        return true;
    }
#endif

    Q_INVOKABLE bool saveLogInfoOfCamera(const QUrl& url, QObject* camera)
    {
        QCamera* cameraObject = dynamic_cast<QCamera*>(camera);
        if (cameraObject == nullptr) {
            cameraObject = camera->property("mediaObject").value<QCamera*>();
        }
        if (cameraObject) {
            QCameraInfo cameraInfo(*cameraObject);
            return saveCameraInfo(url, cameraInfo);
        }
        return false;
    }

    Q_INVOKABLE bool saveLogInfoOfCameraView(const QUrl& url, QObject* cameraView)
    {
#if defined(__ANDROID__)
        AndroidCameraView* androidCameraView = dynamic_cast<AndroidCameraView*>(cameraView);
        if (androidCameraView)
            return saveCameraInfo(url, androidCameraView->getCameraInfo());
#else
        Q_UNUSED(url);
        Q_UNUSED(cameraView);
#endif
        return false;
    }

public slots:

    void clear()
    {
        m_main = QJsonObject();
    }

    void addCalibrationConfig(ARCalibrationConfig* config)
    {
        add("CalibrationConfiguration", config->toJsonObject());
    }

    void addInitConfig(ARInitConfig* config)
    {
        add("InitializationConfiguration", config->toJsonObject());
    }

    void addMapPointsDetectorConfig(ARMapPointsDetectorConfig* config)
    {
        add("MapPointsDetectorConfig", config->toJsonObject());
    }

    void addTrackerConfig(ARTrackerConfig* config)
    {
        add("TrackerConfiguration", config->toJsonObject());
    }

    bool save(const QUrl& url)
    {
        QString path = url.path();
#ifdef _WIN32
        if (path.at(0) == '/')
            path = path.right(path.size() - 1);
#endif
        QFile file(path);
        if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
            qWarning(QString("Couldn't open save file: %s").arg(path).toStdString().c_str());
            return false;
        }
        QJsonObject jsonSize;
        jsonSize.insert("x", m_frameResolution.width());
        jsonSize.insert("y", m_frameResolution.height());
        add("frameResolution", jsonSize);
        QJsonDocument document(m_main);
        file.write(document.toJson(QJsonDocument::Indented));
        return true;
    }

    bool load(const QUrl& url,
              ARCalibrationConfig* calibrationConfig,
              ARInitConfig* initConfig,
              ARMapPointsDetectorConfig* mapPointsDetectorConfig,
              ARTrackerConfig* trackerConfig)
    {
        QString path = url.path();
#ifdef _WIN32
        if (path.at(0) == '/')
            path = path.right(path.size() - 1);
#endif
        QFile file(path);
        if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
            qWarning(QString("Couldn't open load file: %1").arg(path).toStdString().c_str());
            return false;
        }
        QByteArray saveData = file.readAll();
        QJsonDocument document = QJsonDocument::fromJson(saveData);
        m_main = document.object();
        if (m_main.isEmpty()) {
            qWarning("Json object is empty");
            return false;
        }
        calibrationConfig->fromJsonObject(m_main["CalibrationConfiguration"].toObject());
        initConfig->fromJsonObject(m_main["InitializationConfiguration"].toObject());
        mapPointsDetectorConfig->fromJsonObject(m_main["MapPointsDetectorConfig"].toObject());
        trackerConfig->fromJsonObject(m_main["TrackerConfiguration"].toObject());
        if (m_main.contains("frameResolution")) {
            QJsonObject jsonSize = m_main.value("frameResolution").toObject();
            setFrameResolution(QSize((float)jsonSize.value("x").toDouble(),
                                     (float)jsonSize.value("x").toDouble()));
        }
        return true;
    }

    QString getDefaultDirectory() const
    {
        return QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    }

signals:
    void frameResolutionChanged();

private:
    QSize m_frameResolution;
    QJsonObject m_main;
};

#endif // JSONCONFIG_H
