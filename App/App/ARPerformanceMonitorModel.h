#ifndef ARPERFORMANCEMONITOR_H
#define ARPERFORMANCEMONITOR_H

#include <QObject>
#include <QAbstractListModel>
#include <QHash>
#include <QMutex>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QColor>
#include "AR/PerformanceMonitor.h"
#include "ARTracker.h"
#include <string>
#include <memory>

class ARPerformanceMonitorModel:
        public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(ARTracker* arTracker READ arTracker WRITE setArTracker NOTIFY arTrackerChanged)
    Q_PROPERTY(int commonTime READ commonTime NOTIFY commonTimeChanged)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(int countUsedTimes READ countUsedTimes WRITE setCountUsedTimes NOTIFY countUsedTimesChanged)
public:
    enum StepRoles {
        Name = Qt::DisplayRole,
        SizeOfPart = Qt::UserRole + 1
    };

    struct Step {
        std::string name;
        float sizeOfPart;
    };

    explicit ARPerformanceMonitorModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Step getItem(int position) const;
    int findItem(const std::string& name, int startIndex = 0) const;

    ARTracker* arTracker() const;
    void setArTracker(ARTracker* arTracker);

    int commonTime() const;

    int countUsedTimes() const;
    void setCountUsedTimes(int count);

public slots:
    QColor getItemColor(int index) const;
    void updateModel();
    void clear();

signals:
    void arTrackerChanged();
    void commonTimeChanged();
    void rowCountChanged();
    void countUsedTimesChanged();

private:
    ARTracker* m_arTracker;
    QVector<Step> m_data;
    int m_commonTime;
    int m_countUsedTimes;
    std::shared_ptr<AR::PerformanceMonitor> m_performanceMonitor;
    mutable QMutex m_mutex;
};

#endif // ARPERFORMANCEMONITOR_H
