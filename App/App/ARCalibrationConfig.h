#ifndef ARCALIBRATIONCONFIG_H
#define ARCALIBRATIONCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>
#include <QSize>
#include "AR/Point2.h"
#include "AR/Configurations.h"

class ARCalibrationConfig:
        public QObject
{
    Q_OBJECT

    Q_PROPERTY(int countImageLevels READ countImageLevels WRITE setCountImageLevels NOTIFY configChanged)
    Q_PROPERTY(int minImageLevelForCorner READ minImageLevelForCorner WRITE setMinImageLevelForCorner NOTIFY configChanged)
    Q_PROPERTY(int maxImageLevelForCorner READ maxImageLevelForCorner WRITE setMaxImageLevelForCorner NOTIFY configChanged)
    Q_PROPERTY(double minDistanceForCorner READ minDistanceForCorner WRITE setMinDistanceForCorner NOTIFY configChanged)
    Q_PROPERTY(int cornerPatchPixelSize READ cornerPatchPixelSize WRITE setCornerPatchPixelSize NOTIFY configChanged)
    Q_PROPERTY(double cornerPathEps READ cornerPathEps WRITE setCornerPathEps NOTIFY configChanged)
    Q_PROPERTY(int maxCountIterations READ maxCountIterations WRITE setMaxCountIterations NOTIFY configChanged)
    Q_PROPERTY(double maxError READ maxError WRITE setMaxError NOTIFY configChanged)
    Q_PROPERTY(int minCornersForGrabbedFrame READ minCornersForGrabbedFrame WRITE setMinCornersForGrabbedFrame NOTIFY configChanged)
    Q_PROPERTY(double expandByStepMaxDistFrac READ expandByStepMaxDistFrac WRITE setExpandByStepMaxDistFrac NOTIFY configChanged)
    Q_PROPERTY(int meanGate READ meanGate WRITE setMeanGate NOTIFY configChanged)
    Q_PROPERTY(double minGain READ minGain WRITE setMinGain NOTIFY configChanged)
    Q_PROPERTY(int counOfUsedCalibrationFrames READ counOfUsedCalibrationFrames
               WRITE setCounOfUsedCalibrationFrames NOTIFY configChanged)

public:
    AR::CalibrationConfiguration get() const
    {
        return m_config;
    }
    void set(const AR::CalibrationConfiguration& config)
    {
        m_config = config;
        emit configChanged();
    }

    Q_INVOKABLE void copy(ARCalibrationConfig* config)
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
    void setCountImageLevels(int countImageLevels)
    {
        m_config.countImageLevels = countImageLevels;
        emit configChanged();
    }

    int minImageLevelForCorner() const
    {
        return m_config.minImageLevelForCorner;
    }
    void setMinImageLevelForCorner(int imageLevelForCorner)
    {
        m_config.minImageLevelForCorner = imageLevelForCorner;
        emit configChanged();
    }

    int maxImageLevelForCorner() const
    {
        return m_config.maxImageLevelForCorner;
    }
    void setMaxImageLevelForCorner(int imageLevelForCorner)
    {
        m_config.maxImageLevelForCorner = imageLevelForCorner;
        emit configChanged();
    }

    double minDistanceForCorner() const
    {
        return m_config.minDistanceForCorner;
    }
    void setMinDistanceForCorner(double minDistanceForCorner)
    {
        m_config.minDistanceForCorner = minDistanceForCorner;
        emit configChanged();
    }

    int cornerPatchPixelSize() const
    {
        return m_config.cornerPatchPixelSize;
    }
    void setCornerPatchPixelSize(int cornerPatchPixelSize)
    {
        m_config.cornerPatchPixelSize = cornerPatchPixelSize;
        emit configChanged();
    }

    double cornerPathEps() const
    {
        return m_config.cornerPathEps;
    }
    void setCornerPathEps(double cornerPathEps)
    {
        m_config.cornerPathEps = cornerPathEps;
        emit configChanged();
    }

    int maxCountIterations() const
    {
        return m_config.maxCountIterations;
    }
    void setMaxCountIterations(int maxCountIterations)
    {
        m_config.maxCountIterations = maxCountIterations;
        emit configChanged();
    }

    double maxError() const
    {
        return m_config.maxError;
    }
    void setMaxError(double maxError)
    {
        m_config.maxError = maxError;
        emit configChanged();
    }

    int minCornersForGrabbedFrame() const
    {
        return m_config.minCornersForGrabbedFrame;
    }
    void setMinCornersForGrabbedFrame(int minCornersForGrabbedFrame)
    {
        m_config.minCornersForGrabbedFrame = minCornersForGrabbedFrame;
        emit configChanged();
    }

    double expandByStepMaxDistFrac() const
    {
        return m_config.expandByStepMaxDistFrac;
    }
    void setExpandByStepMaxDistFrac(double expandByStepMaxDistFrac)
    {
        m_config.expandByStepMaxDistFrac = expandByStepMaxDistFrac;
        emit configChanged();
    }

    int meanGate() const
    {
        return m_config.meanGate;
    }
    void setMeanGate(int meanGate)
    {
        m_config.meanGate = meanGate;
        emit configChanged();
    }

    double minGain() const
    {
        return m_config.minGain;
    }
    void setMinGain(double minGain)
    {
        m_config.minGain = minGain;
        emit configChanged();
    }

    int counOfUsedCalibrationFrames() const
    {
        return m_config.counOfUsedCalibrationFrames;
    }
    void setCounOfUsedCalibrationFrames(int counOfUsedCalibrationFrames)
    {
        m_config.counOfUsedCalibrationFrames = counOfUsedCalibrationFrames;
        emit configChanged();
    }

signals:
    void configChanged();

private:
    AR::CalibrationConfiguration m_config;
};

#endif // ARCALIBRATIONCONFIG_H
