#ifndef ARMAPPOINTSDETECTORCONFIG_H
#define ARMAPPOINTSDETECTORCONFIG_H

#include <QObject>
#include <QSize>
#include <QJsonObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>
#include "AR/Point2.h"
#include "AR/Configurations.h"

class ARMapPointsDetectorConfig:
        public QObject
{
    Q_OBJECT

    Q_PROPERTY(int maxNumberOfUsedFrames READ maxNumberOfUsedFrames WRITE setMaxNumberOfUsedFrames
               NOTIFY configChanged)
    Q_PROPERTY(int maxNumberOfSearchSteps READ maxNumberOfSearchSteps WRITE setMaxNumberOfSearchSteps
               NOTIFY configChanged)
    Q_PROPERTY(float seedConvergenceSquaredSigmaThresh READ seedConvergenceSquaredSigmaThresh
               WRITE setSeedConvergenceSquaredSigmaThresh NOTIFY configChanged)
    Q_PROPERTY(int sizeOfCommitMapPoints READ sizeOfCommitMapPoints WRITE setSizeOfCommitMapPoints NOTIFY configChanged)
    Q_PROPERTY(QSize frameGridSize READ frameGridSize WRITE setFrameGridSize NOTIFY configChanged)
    Q_PROPERTY(int featureCornerBarier READ featureCornerBarier WRITE setFeatureCornerBarier NOTIFY configChanged)
    Q_PROPERTY(float featureDetectionThreshold READ featureDetectionThreshold
               WRITE setFeatureDetectionThreshold NOTIFY configChanged)
    Q_PROPERTY(int minImageLevelForFeature READ minImageLevelForFeature
               WRITE setMinImageLevelForFeature NOTIFY configChanged)
    Q_PROPERTY(int maxImageLevelForFeature READ maxImageLevelForFeature
               WRITE setMaxImageLevelForFeature NOTIFY configChanged)
    Q_PROPERTY(QSize featureCursorSize READ featureCursorSize WRITE setFeatureCursorSize NOTIFY configChanged)
    Q_PROPERTY(float pixelEps READ pixelEps WRITE setPixelEps NOTIFY configChanged)
    Q_PROPERTY(int maxNumberIterationsForOpticalFlow READ maxNumberIterationsForOpticalFlow
               WRITE setMaxNumberIterationsForOpticalFlow NOTIFY configChanged)
    Q_PROPERTY(int maxCountCandidatePoints READ maxCountCandidatePoints
               WRITE setMaxCountCandidatePoints NOTIFY configChanged)
public:
    AR::MapPointsDetectorConfiguration get() const
    {
        return m_config;
    }
    void set(const AR::MapPointsDetectorConfiguration& config)
    {
        m_config = config;
        emit configChanged();
    }

    Q_INVOKABLE void copy(ARMapPointsDetectorConfig* config)
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

    int maxNumberOfUsedFrames() const
    {
        return m_config.maxNumberOfUsedFrames;
    }
    void setMaxNumberOfUsedFrames(int maxNumberOfUsedFrames)
    {
        m_config.maxNumberOfUsedFrames = maxNumberOfUsedFrames;
        emit configChanged();
    }

    int maxNumberOfSearchSteps() const
    {
        return m_config.maxNumberOfSearchSteps;
    }
    void setMaxNumberOfSearchSteps(int maxNumberOfSearchSteps)
    {
        m_config.maxNumberOfSearchSteps = maxNumberOfSearchSteps;
        emit configChanged();
    }

    int maxCountCandidatePoints() const
    {
        return m_config.maxCountCandidatePoints;
    }
    void setMaxCountCandidatePoints(int value)
    {
        m_config.maxCountCandidatePoints = value;
        emit configChanged();
    }

    float seedConvergenceSquaredSigmaThresh() const
    {
        return m_config.seedConvergenceSquaredSigmaThresh;
    }
    void setSeedConvergenceSquaredSigmaThresh(float seedConvergenceSquaredSigmaThresh)
    {
        m_config.seedConvergenceSquaredSigmaThresh = seedConvergenceSquaredSigmaThresh;
        emit configChanged();
    }

    int sizeOfCommitMapPoints() const
    {
        return m_config.sizeOfCommitMapPoints;
    }
    void setSizeOfCommitMapPoints(int sizeOfCommitMapPoints)
    {
        m_config.sizeOfCommitMapPoints = sizeOfCommitMapPoints;
        emit configChanged();
    }

    QSize frameGridSize() const
    {
        return QSize(m_config.frameGridSize.x, m_config.frameGridSize.y);
    }
    void setFrameGridSize(const QSize& frameGridSize)
    {
        m_config.frameGridSize.set(frameGridSize.width(), frameGridSize.height());
        emit configChanged();
    }

    int featureCornerBarier() const
    {
        return m_config.featureCornerBarier;
    }
    void setFeatureCornerBarier(int featureCornerBarier)
    {
        m_config.featureCornerBarier = featureCornerBarier;
        emit configChanged();
    }

    float featureDetectionThreshold() const
    {
        return m_config.featureDetectionThreshold;
    }
    void setFeatureDetectionThreshold(float featureDetectionThreshold)
    {
        m_config.featureDetectionThreshold = featureDetectionThreshold;
        emit configChanged();
    }

    int minImageLevelForFeature() const
    {
        return m_config.minImageLevelForFeature;
    }
    void setMinImageLevelForFeature(int minImageLevelForFeature)
    {
        m_config.minImageLevelForFeature = minImageLevelForFeature;
        emit configChanged();
    }

    int maxImageLevelForFeature() const
    {
        return m_config.maxImageLevelForFeature;
    }
    void setMaxImageLevelForFeature(int maxImageLevelForFeature)
    {
        m_config.maxImageLevelForFeature = maxImageLevelForFeature;
        emit configChanged();
    }

    QSize featureCursorSize() const
    {
        return QSize(m_config.featureCursorSize.x, m_config.featureCursorSize.y);
    }
    void setFeatureCursorSize(const QSize& featureCursorSize)
    {
        m_config.featureCursorSize.set(featureCursorSize.width(), featureCursorSize.height());
        emit configChanged();
    }

    float pixelEps() const
    {
        return m_config.pixelEps;
    }
    void setPixelEps(float pixelEps)
    {
        m_config.pixelEps = pixelEps;
        emit configChanged();
    }

    int maxNumberIterationsForOpticalFlow() const
    {
        return m_config.maxNumberIterationsForOpticalFlow;
    }
    void setMaxNumberIterationsForOpticalFlow(int maxNumberIterationsForOpticalFlow)
    {
        m_config.maxNumberIterationsForOpticalFlow = maxNumberIterationsForOpticalFlow;
        emit configChanged();
    }

signals:
    void configChanged();

private:
    AR::MapPointsDetectorConfiguration m_config;
};

#endif // ARMAPPOINTSDETECTORCONFIG_H
