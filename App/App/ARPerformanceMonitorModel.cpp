#include "ARPerformanceMonitorModel.h"
#include <QModelIndex>
#include <QMutexLocker>
#include <chrono>


ARPerformanceMonitorModel::ARPerformanceMonitorModel(QObject* parent):
    QAbstractListModel(parent), m_mutex(QMutex::Recursive)
{
    m_countUsedTimes = 10;
    m_arTracker = nullptr;
    m_commonTime = 1;
}

int ARPerformanceMonitorModel::countUsedTimes() const
{
    return m_countUsedTimes;
}

void ARPerformanceMonitorModel::setCountUsedTimes(int count)
{
    m_countUsedTimes = count;
    if (m_performanceMonitor)
        m_performanceMonitor->setCountUsedTimes(m_countUsedTimes);
    emit countUsedTimesChanged();
}

int ARPerformanceMonitorModel::rowCount(const QModelIndex& parent) const
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    Q_UNUSED(parent);
    return m_data.size();
}

QVariant ARPerformanceMonitorModel::data(const QModelIndex& index, int role) const
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    if (!index.isValid() || (index.row() >= m_data.size()))
        return QVariant();
    const Step& step = m_data[index.row()];
    switch (role) {
    case StepRoles::Name:
        return QVariant::fromValue(QString::fromStdString(step.name));
    case StepRoles::SizeOfPart:
        return QVariant::fromValue(step.sizeOfPart);
    default:
        break;
    }
    return QVariant();
}

QVariant ARPerformanceMonitorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    Q_UNUSED(orientation);
    if (role != Qt::DisplayRole)
        return QVariant();
    return QString("Row %1").arg(section);
}

QHash<int, QByteArray> ARPerformanceMonitorModel::roleNames() const
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    QHash<int, QByteArray> roles;
    roles.insert(StepRoles::Name, "name");
    roles.insert(StepRoles::SizeOfPart, "sizeOfPart");
    return roles;
}

ARPerformanceMonitorModel::Step ARPerformanceMonitorModel::getItem(int position) const
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    return m_data[position];
}

int ARPerformanceMonitorModel::findItem(const std::string& name, int startIndex) const
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    for (int i = startIndex; i < m_data.size(); ++i) {
        if (m_data[i].name == name)
            return i;
    }
    return -1;
}

void ARPerformanceMonitorModel::updateModel()
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    if (!m_performanceMonitor)
        return;
    //m_commonTime = 0;
    m_commonTime = std::max((int)m_performanceMonitor->commonTime().count(), 1);
    emit commonTimeChanged();
    QVector<int> roles;
    roles.push_back(StepRoles::SizeOfPart);
    int indexData = 0;
    for (std::size_t indexTimer = 0; indexTimer < m_performanceMonitor->countTimers(); ++indexTimer) {
        AR::PerformanceMonitor::Timer timer = m_performanceMonitor->timer(indexTimer);
        //m_commonTime += (int)timer.duration;
        if (indexData < m_data.size()) {
            if (timer.name == m_data[indexData].name) {
                m_data[indexData].sizeOfPart = timer.duration / (float)m_commonTime;
                emit dataChanged(index(indexData), index(indexData), roles);
                ++indexData;
            } else {
                int foundedIndex = findItem(timer.name, indexData + 1);
                if (foundedIndex < 0) {
                    beginRemoveRows(QModelIndex(), indexData, m_data.size() - 1);
                    m_data.erase(m_data.begin() + indexData, m_data.end());
                    endRemoveRows();
                    beginInsertRows(QModelIndex(), indexData, indexData);
                    m_data.insert(m_data.end(), { timer.name, timer.duration / (float)m_commonTime });
                    endInsertRows();
                    ++indexData;
                } else {
                    beginRemoveRows(QModelIndex(), indexData, foundedIndex - 1);
                    m_data.erase(m_data.begin() + indexData, m_data.begin() + foundedIndex);
                    endRemoveRows();
                    m_data[indexData].sizeOfPart = timer.duration / (float)m_commonTime;
                    emit dataChanged(index(indexData), index(indexData), roles);
                    ++indexData;
                }
            }
        } else {
            beginInsertRows(QModelIndex(), indexData, indexData);
            m_data.insert(m_data.begin() + indexData, { timer.name, timer.duration / (float)m_commonTime });
            endInsertRows();
            ++indexData;
        }
    }
    if (indexData < m_data.size()) {
        beginRemoveRows(QModelIndex(), indexData, m_data.size() - 1);
        m_data.erase(m_data.begin() + indexData, m_data.end());
        endRemoveRows();
    }
    emit rowCountChanged();
}

void ARPerformanceMonitorModel::clear()
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    beginResetModel();
    m_data.clear();
    endResetModel();
    m_commonTime = 1;
    emit commonTimeChanged();
    emit rowCountChanged();
}

int ARPerformanceMonitorModel::commonTime() const
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    return m_commonTime;
}

QColor ARPerformanceMonitorModel::getItemColor(int index) const
{
    static const QColor colors[] = {
        QColor(250, 120, 20), QColor(120, 250, 20), QColor(20, 120, 250),
        QColor(250, 20, 120), QColor(120, 20, 250), QColor(20, 250, 120) };
    return colors[index % (sizeof colors)];
}

ARTracker* ARPerformanceMonitorModel::arTracker() const
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    return m_arTracker;
}

void ARPerformanceMonitorModel::setArTracker(ARTracker* arTracker)
{
    QMutexLocker ml(&m_mutex);
    Q_UNUSED(ml);
    clear();
    m_arTracker = arTracker;
    if (m_arTracker != nullptr) {
        m_performanceMonitor = m_arTracker->arSystem().performanceMonitor();
        m_performanceMonitor->setCountUsedTimes(m_countUsedTimes);
    } else {
        m_performanceMonitor.reset();
    }
    emit arTrackerChanged();
}
