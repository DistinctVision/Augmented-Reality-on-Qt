#ifndef ARSCENE_H
#define ARSCENE_H

#include <climits>
#include <limits>
#include <QObject>
#include <QQuickItem>
#include <QMatrix3x3>
#include "QScrollEngine/QScene.h"
#include "ARTracker.h"

class ARScene: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(ARTracker* arTracker READ arTracker WRITE setArTracker NOTIFY arTrackerChanged)

public:
    ARScene();

    void beginUpdate();
    void endUpdate();

    virtual void setFrameTexture(GLuint textureId, const QMatrix3x3& textureMatrix, bool egl);

    QScrollEngine::QScene* scene();
    const QScrollEngine::QScene* scene() const;

    QScrollEngine::QScene* createScene(QScrollEngine::QScrollEngineContext* context);
    void freeScene();

    bool enabled() const;
    void setEnabled(bool enabled);

    ARTracker* arTracker();
    const ARTracker* arTracker() const;
    void setArTracker(ARTracker* arTracker);

    virtual void onPressed(int id, float x, float y);
    virtual void onReleased(int id);

signals:
    void enabledChanged();
    void arTrackerChanged();

protected:
    virtual void initScene();
    virtual void _beginUpdate();
    virtual void _endUpdate();

private:
    bool m_enabled;
    ARTracker* m_arTracker;
    QScrollEngine::QScene* m_scene;
};

#endif // ARSCENE_H
