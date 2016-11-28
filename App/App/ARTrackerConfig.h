#ifndef ARTRACKERCONFIG_H
#define ARTRACKERCONFIG_H

#include <QObject>
#include <QSize>
#include <QJsonObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>
#include "AR/Point2.h"
#include "AR/Configurations.h"

class ARTrackerConfig:
        public QObject
{
    Q_OBJECT

    Q_PROPERTY(int countImageLevels READ countImageLevels WRITE setCountImageLevels NOTIFY configChanged)
    Q_PROPERTY(int mapPoint_goodSuccessLimit READ mapPoint_goodSuccessLimit WRITE setMapPoint_goodSuccessLimit
               NOTIFY configChanged)
    Q_PROPERTY(int mapPoint_failedLimit READ mapPoint_failedLimit WRITE setMapPoint_failedLimit
               NOTIFY configChanged)
    Q_PROPERTY(int candidate_failedLimit READ candidate_failedLimit
               WRITE setCandidate_failedLimit NOTIFY configChanged)
    Q_PROPERTY(int maxNumberOfUsedKeyFrames READ maxNumberOfUsedKeyFrames
               WRITE setMaxNumberOfUsedKeyFrames NOTIFY configChanged)
    Q_PROPERTY(int frameBorder READ frameBorder WRITE setFrameBorder NOTIFY configChanged)
    Q_PROPERTY(int maxNumberOfFeaturesOnFrame READ maxNumberOfFeaturesOnFrame
               WRITE setMaxNumberOfFeaturesOnFrame NOTIFY configChanged)
    Q_PROPERTY(QSize frameGridSize READ frameGridSize WRITE setFrameGridSize NOTIFY configChanged)
    Q_PROPERTY(QSize featureCursorSize READ featureCursorSize WRITE setFeatureCursorSize NOTIFY configChanged)
    Q_PROPERTY(float pixelEps READ pixelEps WRITE setPixelEps NOTIFY configChanged)
    Q_PROPERTY(double locationEps READ locationEps WRITE setLocationEps NOTIFY configChanged)
    Q_PROPERTY(double locationMaxPixelError READ locationMaxPixelError WRITE setLocationMaxPixelError NOTIFY configChanged)
    Q_PROPERTY(int locationNumberIterations READ locationNumberIterations WRITE setLocationNumberIterations NOTIFY configChanged)
    Q_PROPERTY(int numberPointsForSructureOptimization READ numberPointsForSructureOptimization
               WRITE setNumberPointsForSructureOptimization NOTIFY configChanged)
    Q_PROPERTY(int numberIterationsForStructureOptimization READ numberIterationsForStructureOptimization
               WRITE setNumberIterationsForStructureOptimization NOTIFY configChanged)
    Q_PROPERTY(double toleranceOfCreatingFrames READ toleranceOfCreatingFrames
               WRITE setToleranceOfCreatingFrames NOTIFY configChanged)
    Q_PROPERTY(int minNumberTrackingPoints READ minNumberTrackingPoints
               WRITE setMinNumberTrackingPoints NOTIFY configChanged)
    Q_PROPERTY(int preferredNumberTrackingPoints READ preferredNumberTrackingPoints
               WRITE setPreferredNumberTrackingPoints NOTIFY configChanged)
    Q_PROPERTY(int sizeOfSmallImage READ sizeOfSmallImage WRITE setSizeOfSmallImage NOTIFY configChanged)
    Q_PROPERTY(int maxCountKeyFrames READ maxCountKeyFrames WRITE setMaxCountKeyFrames NOTIFY configChanged)
    Q_PROPERTY(int featureMaxNumberIterations READ featureMaxNumberIterations
               WRITE setFeatureMaxNumberIterations NOTIFY configChanged)
    Q_PROPERTY(double tracker_eps READ tracker_eps WRITE setTracker_eps NOTIFY configChanged)
    Q_PROPERTY(double tracker_numberIterations READ tracker_numberIterations
               WRITE setTracker_numberIterations NOTIFY configChanged)
    Q_PROPERTY(int tracker_minImageLevel READ tracker_minImageLevel
               WRITE setTracker_minImageLevel NOTIFY configChanged)
    Q_PROPERTY(int tracker_maxImageLevel READ tracker_maxImageLevel
               WRITE setTracker_maxImageLevel NOTIFY configChanged)
    Q_PROPERTY(QSize tracker_cursorSize READ tracker_cursorSize
               WRITE setTracker_cursorSize NOTIFY configChanged)

public:
    AR::TrackingConfiguration get() const
    {
        return m_config;
    }
    void set(const AR::TrackingConfiguration& config)
    {
        m_config = config;
        emit configChanged();
    }

    Q_INVOKABLE void copy(ARTrackerConfig* config)
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

    int countImageLevels() const
    {
        return m_config.countImageLevels;
    }
    void setCountImageLevels(int value)
    {
        m_config.countImageLevels = value;
        emit configChanged();
    }

    int mapPoint_goodSuccessLimit() const
    {
        return m_config.mapPoint_goodSuccessLimit;
    }
    void setMapPoint_goodSuccessLimit(int value)
    {
        m_config.mapPoint_goodSuccessLimit = value;
        emit configChanged();
    }

    int mapPoint_failedLimit() const
    {
        return m_config.mapPoint_failedLimit;
    }
    void setMapPoint_failedLimit(int value)
    {
        m_config.mapPoint_failedLimit = value;
        emit configChanged();
    }

    int candidate_failedLimit() const
    {
        return m_config.candidate_failedLimit;
    }
    void setCandidate_failedLimit(int value)
    {
        m_config.candidate_failedLimit = value;
        emit configChanged();
    }

    int maxNumberOfUsedKeyFrames() const
    {
        return m_config.maxNumberOfUsedKeyFrames;
    }
    void setMaxNumberOfUsedKeyFrames(int value)
    {
        m_config.maxNumberOfUsedKeyFrames = value;
        emit configChanged();
    }

    int frameBorder() const
    {
        return m_config.frameBorder;
    }
    void setFrameBorder(int value)
    {
        m_config.frameBorder = value;
        emit configChanged();
    }

    int maxNumberOfFeaturesOnFrame() const
    {
        return m_config.maxNumberOfFeaturesOnFrame;
    }
    void setMaxNumberOfFeaturesOnFrame(int value)
    {
        m_config.maxNumberOfFeaturesOnFrame = value;
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

    double locationEps() const
    {
        return m_config.locationEps;
    }
    void setLocationEps(double value)
    {
        m_config.locationEps = value;
        emit configChanged();
    }

    double locationMaxPixelError() const
    {
        return m_config.locationMaxPixelError;
    }
    void setLocationMaxPixelError(double value)
    {
        m_config.locationMaxPixelError = value;
        emit configChanged();
    }

    int locationNumberIterations() const
    {
        return m_config.locationNumberIterations;
    }
    void setLocationNumberIterations(int value)
    {
        m_config.locationNumberIterations = value;
        emit configChanged();
    }

    int numberPointsForSructureOptimization() const
    {
        return m_config.numberPointsForSructureOptimization;
    }
    void setNumberPointsForSructureOptimization(int value)
    {
        m_config.numberPointsForSructureOptimization = value;
        emit configChanged();
    }

    int numberIterationsForStructureOptimization() const
    {
        return m_config.numberIterationsForStructureOptimization;
    }
    void setNumberIterationsForStructureOptimization(int value)
    {
        m_config.numberIterationsForStructureOptimization = value;
        emit configChanged();
    }

    double toleranceOfCreatingFrames() const
    {
        return m_config.toleranceOfCreatingFrames;
    }
    void setToleranceOfCreatingFrames(double value)
    {
        m_config.toleranceOfCreatingFrames = value;
        emit configChanged();
    }

    int minNumberTrackingPoints() const
    {
        return m_config.minNumberTrackingPoints;
    }
    void setMinNumberTrackingPoints(int value)
    {
        m_config.minNumberTrackingPoints = value;
        emit configChanged();
    }

    int preferredNumberTrackingPoints() const
    {
        return m_config.preferredNumberTrackingPoints;
    }
    void setPreferredNumberTrackingPoints(int value)
    {
        m_config.preferredNumberTrackingPoints = value;
        emit configChanged();
    }

    int sizeOfSmallImage() const
    {
        return m_config.sizeOfSmallImage;
    }
    void setSizeOfSmallImage(int size)
    {
        m_config.sizeOfSmallImage = size;
        emit configChanged();
    }

    int maxCountKeyFrames() const
    {
        return m_config.maxCountKeyFrames;
    }
    void setMaxCountKeyFrames(int count)
    {
        m_config.maxCountKeyFrames = count;
        emit configChanged();
    }

    int featureMaxNumberIterations() const
    {
        return m_config.featureMaxNumberIterations;
    }
    void setFeatureMaxNumberIterations(int value)
    {
        m_config.featureMaxNumberIterations = value;
        emit configChanged();
    }

    double tracker_eps() const
    {
        return m_config.tracker_eps;
    }
    void setTracker_eps(double eps)
    {
        m_config.tracker_eps = eps;
        emit configChanged();
    }

    int tracker_numberIterations() const
    {
        return m_config.tracker_numberIterations;
    }
    void setTracker_numberIterations(int value)
    {
        m_config.tracker_numberIterations = value;
        emit configChanged();
    }

    int tracker_minImageLevel() const
    {
        return m_config.tracker_minImageLevel;
    }
    void setTracker_minImageLevel(int value)
    {
        m_config.tracker_minImageLevel = value;
        emit configChanged();
    }

    int tracker_maxImageLevel() const
    {
        return m_config.tracker_maxImageLevel;
    }
    void setTracker_maxImageLevel(int value)
    {
        m_config.tracker_maxImageLevel = value;
        emit configChanged();
    }

    QSize tracker_cursorSize() const
    {
        return QSize(m_config.tracker_cursorSize.x, m_config.tracker_cursorSize.y);
    }
    void setTracker_cursorSize(const QSize& size)
    {
        m_config.tracker_cursorSize.set(size.width(), size.height());
        emit configChanged();
    }

signals:
    void configChanged();

private:
    AR::TrackingConfiguration m_config;
};

#endif // ARTRACKERCONFIG_H
