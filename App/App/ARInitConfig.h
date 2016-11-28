#ifndef ARINITCONFIG_H
#define ARINITCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>
#include <QSize>
#include "AR/Point2.h"
#include "AR/Configurations.h"

class ARInitConfig:
        public QObject
{
    Q_OBJECT

    Q_PROPERTY(double maxPixelError READ maxPixelError WRITE setMaxPixelError NOTIFY configChanged)
    Q_PROPERTY(double epsDistanceForPlane READ epsDistanceForPlane WRITE setEpsDistanceForPlane NOTIFY configChanged)
    Q_PROPERTY(int countTimes READ countTimes WRITE setCountTimes NOTIFY configChanged)
    Q_PROPERTY(double mapScale READ mapScale WRITE setMapScale NOTIFY configChanged)
    Q_PROPERTY(int minCountFeatures READ minCountFeatures WRITE setMinCountFeatures NOTIFY configChanged)
    Q_PROPERTY(int maxCountFeatures READ maxCountFeatures WRITE setMaxCountFeatures NOTIFY configChanged)
    Q_PROPERTY(float minDisparity READ minDisparity WRITE setMinDisparity NOTIFY configChanged)
    Q_PROPERTY(int minCountMapPoints READ minCountMapPoints WRITE setMinCountMapPoints NOTIFY configChanged)
    Q_PROPERTY(QSize frameGridSize READ frameGridSize WRITE setFrameGridSize NOTIFY configChanged)
    Q_PROPERTY(int featureCornerBarier READ featureCornerBarier WRITE setFeatureCornerBarier NOTIFY configChanged)
    Q_PROPERTY(float featureDetectionThreshold READ featureDetectionThreshold WRITE setFeatureDetectionThreshold NOTIFY configChanged)
    Q_PROPERTY(int countImageLevels READ countImageLevels WRITE setCountImageLevels NOTIFY configChanged)
    Q_PROPERTY(int minImageLevelForFeature READ minImageLevelForFeature WRITE setMinImageLevelForFeature NOTIFY configChanged)
    Q_PROPERTY(int maxImageLevelForFeature READ maxImageLevelForFeature WRITE setMaxImageLevelForFeature NOTIFY configChanged)
    Q_PROPERTY(QSize featureCursorSize READ featureCursorSize WRITE setFeatureCursorSize NOTIFY configChanged)
    Q_PROPERTY(float pixelEps READ pixelEps WRITE setPixelEps NOTIFY configChanged)
    Q_PROPERTY(int maxNumberIterationsForOpticalFlow READ maxNumberIterationsForOpticalFlow
               WRITE setMaxNumberIterationsForOpticalFlow NOTIFY configChanged)
public:
    AR::InitConfiguration get() const
    {
        return m_config;
    }
    void set(const AR::InitConfiguration& config)
    {
        m_config = config;
        emit configChanged();
    }

    Q_INVOKABLE void copy(ARInitConfig* config)
    {
        set(config->get());
    }

    QJsonObject toJsonObject() const
    {
        QJsonObject result;
        const QMetaObject* metaObject = this->metaObject();
        for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
            QMetaProperty property = metaObject->property(i);
            const char* name = property.name();
            QVariant value = property.read(this);
            if (property.type() == QVariant::Size) {
                QSize size = value.toSize();
                QJsonObject jsonSize;
                jsonSize.insert("x", size.width());
                jsonSize.insert("y", size.height());
                result.insert(name, jsonSize);
            } else if (property.type() == QVariant::SizeF) {
                QSizeF sizeF = value.toSizeF();
                QJsonObject jsonSizeF;
                jsonSizeF.insert("x", sizeF.width());
                jsonSizeF.insert("y", sizeF.height());
                result.insert(name, jsonSizeF);
            } else {
                result.insert(name, QJsonValue::fromVariant(value));
            }
        }
        return result;
    }

    void fromJsonObject(const QJsonObject& object)
    {
        const QMetaObject* metaObject = this->metaObject();
        if (object.isEmpty()) {
            qWarning((QString(metaObject->className()) + ": JSON object is empty").toStdString().c_str());
            return;
        }
        for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
            QMetaProperty property = metaObject->property(i);
            const char* name = property.name();
            if (!object.contains(name)) {
                qWarning((QString(metaObject->className()) +
                          ": could not find property from JSON '" + name + "'").toStdString().c_str());
                continue;
            }
            if (property.type() == QVariant::Size) {
                QSize size = property.read(this).toSize();
                QJsonObject jsonSize = object[name].toObject();
                size.setWidth(jsonSize.value("x").toInt(size.width()));
                size.setHeight(jsonSize.value("y").toInt(size.height()));
                property.write(this, QVariant::fromValue(size));
            } else if (property.type() == QVariant::SizeF) {
                QSizeF sizeF = property.read(this).toSizeF();
                QJsonObject jsonSizeF = object[name].toObject();
                sizeF.setWidth(jsonSizeF.value("x").toInt(sizeF.width()));
                sizeF.setHeight(jsonSizeF.value("y").toInt(sizeF.height()));
                property.write(this, QVariant::fromValue(sizeF));
            } else {
                setProperty(name, object[name]);
            }
        }
    }

    double maxPixelError() const
    {
        return m_config.maxPixelError;
    }
    void setMaxPixelError(double value)
    {
        m_config.maxPixelError = value;
        emit configChanged();
    }

    double epsDistanceForPlane() const
    {
        return m_config.epsDistanceForPlane;
    }
    void setEpsDistanceForPlane(double value)
    {
        m_config.epsDistanceForPlane = value;
        emit configChanged();
    }

    int countTimes() const
    {
        return m_config.countTimes;
    }
    void setCountTimes(int value)
    {
        m_config.countTimes = value;
        emit configChanged();
    }

    double mapScale() const
    {
        return m_config.mapScale;
    }
    void setMapScale(double value)
    {
        m_config.mapScale = value;
        emit configChanged();
    }

    int minCountFeatures() const
    {
        return m_config.minCountFeatures;
    }
    void setMinCountFeatures(int value)
    {
        m_config.minCountFeatures = value;
        emit configChanged();
    }

    int maxCountFeatures() const
    {
        return m_config.maxCountFeatures;
    }
    void setMaxCountFeatures(int value)
    {
        m_config.maxCountFeatures = value;
        emit configChanged();
    }

    float minDisparity() const
    {
        return m_config.minDisparity;
    }
    void setMinDisparity(float value)
    {
        m_config.minDisparity = value;
        emit configChanged();
    }

    int minCountMapPoints() const
    {
        return m_config.minCountMapPoints;
    }
    void setMinCountMapPoints(int value)
    {
        m_config.minCountMapPoints = value;
        emit configChanged();
    }

    QSize frameGridSize() const
    {
        return QSize(m_config.frameGridSize.x, m_config.frameGridSize.y);
    }
    void setFrameGridSize(const QSize& value)
    {
        m_config.frameGridSize.set(value.width(), value.height());
        emit configChanged();
    }

    int featureCornerBarier() const
    {
        return m_config.featureCornerBarier;
    }
    void setFeatureCornerBarier(int value)
    {
        m_config.featureCornerBarier = value;
        emit configChanged();
    }

    float featureDetectionThreshold() const
    {
        return m_config.featureDetectionThreshold;
    }
    void setFeatureDetectionThreshold(float value)
    {
        m_config.featureDetectionThreshold = value;
        emit configChanged();
    }

    int countImageLevels() const
    {
        return m_config.countImageLevels;
    }
    void setCountImageLevels(int value)
    {
        m_config.countImageLevels = value;
        emit configChanged();
    }

    int minImageLevelForFeature() const
    {
        return m_config.minImageLevelForFeature;
    }
    void setMinImageLevelForFeature(int value)
    {
        m_config.minImageLevelForFeature = value;
        emit configChanged();
    }

    int maxImageLevelForFeature() const
    {
        return m_config.maxImageLevelForFeature;
    }
    void setMaxImageLevelForFeature(int value)
    {
        m_config.maxImageLevelForFeature = value;
        emit configChanged();
    }

    QSize featureCursorSize() const
    {
        return QSize(m_config.featureCursorSize.x, m_config.featureCursorSize.y);
    }
    void setFeatureCursorSize(const QSize& value)
    {
        m_config.featureCursorSize.set(value.width(), value.height());
        emit configChanged();
    }

    float pixelEps() const
    {
        return m_config.pixelEps;
    }
    void setPixelEps(float value)
    {
        m_config.pixelEps = value;
        emit configChanged();
    }

    int maxNumberIterationsForOpticalFlow() const
    {
        return m_config.maxNumberIterationsForOpticalFlow;
    }
    void setMaxNumberIterationsForOpticalFlow(int value)
    {
        m_config.maxNumberIterationsForOpticalFlow = value;
        emit configChanged();
    }

signals:
    void configChanged();

private:
    AR::InitConfiguration m_config;
};

#endif // ARINITCONFIG_H
