#include "MapPointsDetector.h"
#include "LocationOptimizer.h"
#include "MapProjector.h"
#include "MapResourceLocker.h"
#include "TMath/TMath.h"
#include "ZMSSD.h"
#include "KeyFrame.h"

namespace AR {

CandidatesReader::CandidatesReader(MapPointsDetector * mapPointsDetector):
    m_locker(mapPointsDetector->m_mutex_finalCandidates),
    m_mapPointsDetector(mapPointsDetector)
{}

CandidateMapPointsList * CandidatesReader::candidateMapPointsList()
{
    return m_mapPointsDetector->m_finalCandidates;
}

CandidateMapPointsList::CandidateMapPointsList()
{
    m_head = nullptr;
    m_size = 0;
}

CandidateMapPointsList::~CandidateMapPointsList()
{
    clear();
}

bool CandidateMapPointsList::isEmpty() const
{
    return (m_head == nullptr);
}

void CandidateMapPointsList::clear()
{
    CandidateMapPoint * p = m_head;
    CandidateMapPoint * p_next;
    while (p != nullptr) {
        p_next = p->next();
        delete p;
        p = p_next;
    }
    m_head = nullptr;
    m_size = 0;
}

CandidateMapPoint * CandidateMapPointsList::head()
{
    return m_head;
}

int CandidateMapPointsList::size() const
{
    return m_size;
}

void CandidateMapPointsList::splice(CandidateMapPointsList * list)
{
    CandidateMapPoint * e = list->m_head;
    if (e == nullptr)
        return;
    while (e->next() != nullptr) {
        e->m_list = this;
        e = e->next();
    }
    e->m_list = this;
    e->m_next = m_head;
    if (m_head != nullptr)
        m_head->m_prev = e;
    m_size += list->m_size;
    m_head = list->m_head;
    list->m_head = nullptr;
    list->m_size = 0;
}

CandidateMapPoint::CandidateMapPoint(CandidateMapPointsList * list)
{
    m_list = list;
    m_prev = nullptr;
    m_next = m_list->m_head;
    m_list->m_head = this;
    ++m_list->m_size;
    if (m_next != nullptr)
        m_next->m_prev = this;
}

CandidateMapPoint::CandidateMapPoint(CandidateMapPointsList * list,
                                     const std::shared_ptr<KeyFrame> & keyFrame,
                                     const Point2f & projection,
                                     int imageLevel,
                                     const TMath::TVectord & position):
    CandidateMapPoint(list)
{
    this->keyFrame = keyFrame;
    this->projection = projection;
    this->imageLevel = imageLevel;
    this->position = position;
}

CandidateMapPoint::~CandidateMapPoint()
{
    if (m_prev != nullptr)
        m_prev->m_next = m_next;
    else
        m_list->m_head = m_next;
    if (m_next != nullptr)
        m_next->m_prev = m_prev;
    --m_list->m_size;
}

CandidateMapPointsList * CandidateMapPoint::list()
{
    return m_list;
}

CandidateMapPoint * CandidateMapPoint::next()
{
    return m_next;
}

CandidateMapPoint * CandidateMapPoint::prev()
{
    return m_prev;
}

MapPointsDetector::MapPointsDetector(Map * map)
{
    TMath_assert(map != nullptr);
    m_map = map;
    m_thread_is_running = false;
    m_isNewKeyFrame = false;
    m_needToStopSeedUpdating = false;
    m_thread = nullptr;
    m_maxNumberOfUsedFrames = 3;
    m_maxNumberOfSearchSteps = 100;
    m_seedConvergenceSquaredSigmaThresh = 100.0f;
    m_sizeOfCommitMapPoints = 10;
    m_maxCountCandidatePoints = 40;
    m_pixelNoise = 1.5f;
    m_cellsLock = nullptr;
    m_preparedCandidates = new CandidateMapPointsList();
    m_finalCandidates = new CandidateMapPointsList();
}

MapPointsDetector::~MapPointsDetector()
{
    stopThread();
    if (m_cellsLock != nullptr)
        delete [] m_cellsLock;
    delete m_preparedCandidates;
    delete m_finalCandidates;
}

MapPointsDetectorConfiguration MapPointsDetector::configuration() const
{
    std::lock_guard<std::mutex> lock_frames(m_framesQueueMutex); (void)lock_frames;
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    MapPointsDetectorConfiguration configuration;

    configuration.maxNumberOfUsedFrames = m_maxNumberOfUsedFrames;
    configuration.maxNumberOfSearchSteps = m_maxNumberOfSearchSteps;
    configuration.seedConvergenceSquaredSigmaThresh = m_seedConvergenceSquaredSigmaThresh;
    configuration.sizeOfCommitMapPoints = m_sizeOfCommitMapPoints;
    configuration.maxCountCandidatePoints = m_maxCountCandidatePoints;
    configuration.frameGridSize = m_featureDetector.gridSize();
    configuration.featureCornerBarier = m_featureDetector.barrier();
    configuration.featureDetectionThreshold = m_featureDetector.detectionThreshold();
    configuration.minImageLevelForFeature = m_featureDetector.minLevelForFeature();
    configuration.maxImageLevelForFeature = m_featureDetector.maxLevelForFeature();
    configuration.featureCursorSize = m_matcher.cursorSize();
    configuration.pixelEps = m_matcher.pixelEps();
    configuration.maxNumberIterationsForOpticalFlow = m_matcher.numberIterations();

    return configuration;
}

void MapPointsDetector::setConfiguration(const MapPointsDetectorConfiguration & configuration)
{
    std::lock_guard<std::mutex> lock_frames(m_framesQueueMutex); (void)lock_frames;
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;

    m_maxNumberOfUsedFrames = configuration.maxNumberOfUsedFrames;
    m_maxNumberOfSearchSteps = configuration.maxNumberOfSearchSteps;
    m_seedConvergenceSquaredSigmaThresh = configuration.seedConvergenceSquaredSigmaThresh;
    m_sizeOfCommitMapPoints = configuration.sizeOfCommitMapPoints;
    m_maxCountCandidatePoints = configuration.maxCountCandidatePoints;
    m_featureDetector.setGridSize(configuration.frameGridSize);
    if (m_cellsLock != nullptr)
        delete[] m_cellsLock;
    m_cellsLock = new bool[configuration.frameGridSize.y * configuration.frameGridSize.x];
    m_featureDetector.setBarrier(configuration.featureCornerBarier);
    m_featureDetector.setDetectionThreshold(configuration.featureDetectionThreshold);
    m_featureDetector.setLevelForFeature(configuration.minImageLevelForFeature,
                                         configuration.maxImageLevelForFeature);
    m_matcher.setCursorSize(configuration.featureCursorSize);
    m_matcher.setPixelEps(configuration.pixelEps);
    m_matcher.setNumberIterations(configuration.maxNumberIterationsForOpticalFlow);
}

int MapPointsDetector::maxNumberOfUsedFrames() const
{
    std::lock_guard<std::mutex> lock_frames(m_framesQueueMutex); (void)lock_frames;
    return m_maxNumberOfUsedFrames;
}

void MapPointsDetector::setMaxNumberOfUsedFrames(int maxNumberOfUsedFrames)
{
    std::lock_guard<std::mutex> lock_frames(m_framesQueueMutex); (void)lock_frames;
    m_maxNumberOfUsedFrames = maxNumberOfUsedFrames;
}

int MapPointsDetector::maxNumberOfSearchSteps() const
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    return m_maxNumberOfSearchSteps;
}

void MapPointsDetector::setMaxNumberOfSearchSteps(int maxNumberOfSearchSteps)
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    m_maxNumberOfSearchSteps = maxNumberOfSearchSteps;
}

float MapPointsDetector::seedConvergenceSquaredSigmaThresh() const
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    return m_seedConvergenceSquaredSigmaThresh;
}

void MapPointsDetector::setSeedConvergenceSquaredSigmaThresh(float seedConvergenceSquaredSigmaThresh)
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    m_seedConvergenceSquaredSigmaThresh = seedConvergenceSquaredSigmaThresh;
}

float MapPointsDetector::pixelNoise() const
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    return m_pixelNoise;
}

void MapPointsDetector::setPixelNoise(float pixelNoise)
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    m_pixelNoise = pixelNoise;
}

int MapPointsDetector::sizeOfCommitMapPoints() const
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    return m_sizeOfCommitMapPoints;
}

int MapPointsDetector::countImageLevels() const
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    return m_featureDetector.countLevels();
}

void MapPointsDetector::setCountImageLevels(int count)
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    m_featureDetector.setCountLevels(count);
}

void MapPointsDetector::setSizeOfCommitMapPoints(int sizeOfCommitMapPoints)
{
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    m_sizeOfCommitMapPoints = sizeOfCommitMapPoints;
}

bool MapPointsDetector::threadIsRunning() const
{
    std::lock_guard<std::mutex> lock(m_thread_mutex); (void)lock;
    return m_thread_is_running;
}

void MapPointsDetector::startThread()
{
    std::lock_guard<std::mutex> lock(m_thread_mutex); (void)lock;
    if (m_thread_is_running)
        return;
    m_thread_is_running = true;
    m_thread = new std::thread(&MapPointsDetector::loop, this);
}

void MapPointsDetector::stopThread()
{
    {
        std::lock_guard<std::mutex> lock(m_thread_mutex); (void)lock;
        if (!m_thread_is_running)
            return;
        m_thread_is_running = false;
    }
    m_frameQueue_condition.notify_one();
    if (m_thread != nullptr) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
}

void MapPointsDetector::addKeyFrame(const std::shared_ptr<KeyFrame> & keyFrame)
{
    TMath_assert(keyFrame->map() == m_map);
    /*{
        std::lock_guard<std::mutex> lockSeeds(m_seedMutex); (void)lockSeeds;
        m_needToStopSeedUpdating = true;
    }*/
    {
        std::lock_guard<std::mutex> lockFrames(m_framesQueueMutex); (void)lockFrames;
        m_keyFrame = keyFrame;
        m_isNewKeyFrame = true;
        if ((int)m_framesQueue.size() >= m_maxNumberOfUsedFrames) {
            m_framesQueue.erase(m_framesQueue.begin(),
                m_framesQueue.begin() + ((m_maxNumberOfUsedFrames + 1) - (int)(m_framesQueue.size())));
        }
        m_framesQueue.push_back(std::make_shared<Frame>(*keyFrame));
        m_frameQueue_condition.notify_one();
    }
}

void MapPointsDetector::addFrame(Frame && frame)
{
    /*{
        std::lock_guard<std::mutex> lockSeeds(m_seedMutex); (void)lockSeeds;
        m_needToStopSeedUpdating = true;
    }*/
    {
        std::lock_guard<std::mutex> lockFrames(m_framesQueueMutex); (void)lockFrames;
        if ((int)m_framesQueue.size() >= m_maxNumberOfUsedFrames) {
            m_framesQueue.erase(m_framesQueue.begin(),
                m_framesQueue.begin() + ((m_maxNumberOfUsedFrames + 1) - (int)(m_framesQueue.size())));
        }
        m_framesQueue.push_back(std::make_shared<Frame>(frame));
        m_frameQueue_condition.notify_one();
    }
}

void MapPointsDetector::loop()
{
    for (;;) {
        {
            std::lock_guard<std::mutex> lock(m_thread_mutex); (void)lock;
            if (!m_thread_is_running)
                break;
        }
        std::shared_ptr<Frame> frame(nullptr);
        {
            std::unique_lock<std::mutex> lock(m_framesQueueMutex);
            while (!m_isNewKeyFrame && m_framesQueue.empty() && m_thread_is_running)
                m_frameQueue_condition.wait(lock);
            {
                std::lock_guard<std::mutex> lockSeeds(m_seedMutex); (void)lockSeeds;
                m_needToStopSeedUpdating = false;
            }
            if (!m_framesQueue.empty()) {
                frame = m_framesQueue[0];
                m_framesQueue.erase(m_framesQueue.begin());
            }
        }
        if (frame) {
            _updateSeeds(*frame);
        }
        {
            std::weak_ptr<KeyFrame> keyFrame;
            {
                std::unique_lock<std::mutex> lock(m_framesQueueMutex); (void)lock;
                if (m_isNewKeyFrame) {
                    keyFrame = m_keyFrame;
                    m_isNewKeyFrame = false;
                    m_keyFrame.reset();
                }
            }
            _initializeSeed(keyFrame.lock());
        }
    }
}

void MapPointsDetector::_initializeSeed(const std::shared_ptr<KeyFrame> & keyFrame)
{
    using namespace TMath;

    if (!keyFrame)
        return;

    m_map->lock();

    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;
    double depth_mean, depth_min;

    MapResourceLocker lockerKeyFrame(&m_resourceManager, keyFrame.get()); (void)lockerKeyFrame;

    if (keyFrame->isDeleted()) {
        m_map->unlock();
        return;
    }

    m_featureDetector.setFirstImage(*keyFrame);
    for (int i = 0; i < keyFrame->countFeatures(); ++i) {
        m_featureDetector.setCellLock(keyFrame->feature(i)->positionOnFrame());
    }
    KeyFrame::getDepth(&m_resourceManager, depth_mean, depth_min, keyFrame);
    if (depth_mean <= std::numeric_limits<float>::epsilon()) {
        m_map->unlock();
        return;
    }
    std::shared_ptr<const Camera> camera = keyFrame->camera();
    std::vector<FeatureDetector::FeatureCorner> features;
    m_featureDetector.detectFeaturesOnFirstImage(features);
    m_featureDetector.freeGrid();
    float mu = (float)(1.0 / depth_mean);
    float z_range = (float)(1.0 / depth_min);
    float sigmaSquared = (z_range * z_range) / 36.0f;
    std::srand((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
    std::random_shuffle(features.begin(), features.end());
    for (auto it = features.begin(); it != features.end(); ++it) {
        std::vector<ProjectionPoint> projections;
        Point2d l = camera->unproject(it->pos);
        TVectord localDir = TVectord::create(l.x, l.y, 1.0).normalized();
        projections.push_back({ keyFrame->camera(),
                                it->pos.cast<float>(),
                                keyFrame->rotation(),
                                keyFrame->translation()
                              });
        m_seeds.push_back({ keyFrame,                               // keyFrame
                            localDir,                               // localDir
                            it->level,                              // imageLevel
                            10.0f,                                  // a
                            10.0f,                                  // b
                            mu,                                     // mu
                            z_range,                                // z_range
                            sigmaSquared,                           // sigmaSquared
                            projections                             // projections
                          });
    }
    if (m_seeds.size() > 1000) {
        auto to = m_seeds.begin();
        std::advance(to, (m_seeds.size() - 1000));
        m_seeds.erase(m_seeds.begin(), to);
    }
    m_map->unlock();
}

void MapPointsDetector::_updateSeeds(const Frame & frame)
{
    using namespace TMath;
    std::lock_guard<std::mutex> lock_config(m_config_mutex); (void)lock_config;

    m_map->lock();

    TMatrixd frameRotation = frame.rotation(), invSeedRotation(3, 3), deltaRotation(3, 3);
    TVectord frameTranslation = frame.translation(), invSeedTranslation(3), deltaTranslation(3);
    TVectord v;
    Point2d focalLength = frame.camera()->pixelFocalLength();
    // law of chord (sehnensatz)
    double px_error_angle = std::atan(m_pixelNoise / (2.0 * std::max(focalLength.x, focalLength.y))) * 2.0;
    //TVectorf result(3);

    Point2i gridSize = m_featureDetector.gridSize();
    int countCells = gridSize.y * gridSize.x;
    for (int i = 0; i < countCells; ++i)
        m_cellsLock[i] = false;

    auto it = m_seeds.begin();
    while (it != m_seeds.end()) {
        {
            std::lock_guard<std::mutex> lockSeeds(m_seedMutex); (void)lockSeeds;
            if (m_needToStopSeedUpdating) {
                m_needToStopSeedUpdating = false;
                m_map->unlock();
                return;
            }
        }
        std::shared_ptr<KeyFrame> keyFrame = it->keyFrame.lock();
        if (!keyFrame) {
            it = m_seeds.erase(it);
            continue;
        }
        MapResourceLocker lockerKeyFrame(&m_resourceManager, keyFrame.get()); (void)lockerKeyFrame;
        if (keyFrame->isDeleted()) {
            it = m_seeds.erase(it);
            continue;
        }
        if (keyFrame->equals(frame)) {
            ++it;
            continue;
        }

        invSeedRotation = keyFrame->rotation();
        if (!TTools::matrix3x3Invert(invSeedRotation)) {
            it = m_seeds.erase(it);
            continue;
        }
        invSeedTranslation = invSeedRotation * (- keyFrame->translation());

        deltaRotation = frameRotation * invSeedRotation;
        deltaTranslation = frameRotation * invSeedTranslation + frameTranslation;

        v = deltaRotation * (it->localDir * (1.0f / it->mu)) + deltaTranslation;
        if (v(2) < std::numeric_limits<float>::epsilon()) {
            ++it;
            continue;
        }

        if(!frame.imagePointInFrame(frame.camera()->project(Point2d(v(0) / v(2), v(1) / v(2))))) {
            ++it; // point does not project in image
            continue;
        }

        // we are using inverse depth coordinates
        float sigma = std::sqrt(it->sigmaSquared);
        float z_inv_min = it->mu + sigma;
        float z_inv_max = std::max(it->mu - sigma, std::numeric_limits<float>::epsilon());
        double z;
        if (!_findEpipolarMatch(z, keyFrame, *it, frame, deltaRotation, deltaTranslation,
                                1.0 / it->mu, 1.0 / z_inv_min, 1.0 / z_inv_max)) {
            it->b += 1.0f; // increase outlier probability when no match was found
            ++it;
            //++n_failed_matches;
            continue;
        }

        int indexOfCell = m_featureDetector.indexOfCell(m_lastProjection);
        if (m_cellsLock[indexOfCell]) {
            ++it;
            continue;
        } else {
            m_cellsLock[indexOfCell] = true;
        }

        //if (m_isNewKeyFrame)
        //    m_featureDetector.setCellLock(m_lastProjection);

        /*TVectord result(3);
        double error = _getPointInWorld(result, it->projections);

        if (error > 0.5) {
            it = m_seeds.erase(it);
            continue;
        } else if (it->projections.size() >= 2) {
            _addPreparedPoint(*it, result);
            it = m_seeds.erase(it);
            continue;
        }*/

        // compute tau
        double tau = _computeTau(TTools::matrix3x3Inverted(deltaRotation) * (- deltaTranslation),
                                 it->localDir, z, px_error_angle);
        double tau_inverse = 0.5 * (1.0 / std::max(0.0000001, z - tau) - 1.0 / (z + tau));

        // update the estimate
        _updateSeed(*it, (float)(1.0 / z), (float)(tau_inverse * tau_inverse));
        //++n_updates;

        // if the seed has converged, we initialize a new candidate point and remove the seed
        if (std::isnan(it->sigmaSquared)) {
            it = m_seeds.erase(it);
            continue;
        } else if (std::sqrt(it->sigmaSquared) < it->z_range / m_seedConvergenceSquaredSigmaThresh) {
            TVectord point = invSeedRotation * (it->localDir * (1.0 / it->mu)) + invSeedTranslation;
            _addPreparedPoint(*it, point);
            it = m_seeds.erase(it);
            continue;
        }
        ++it;
    }
    _commitPreparedPoints();
    m_map->unlock();
}

bool MapPointsDetector::_findEpipolarMatch(double & depth,
                                           const std::shared_ptr<KeyFrame> & keyFrame,
                                           Seed & seed, const Frame & secondFrame,
                                           const TMath::TMatrixd & deltaRotation, const TMath::TVectord & deltaTranslation,
                                           const double d_estimate, const double d_min, const double d_max)
{
    using namespace TMath;

    // Compute start and end of epipolar line in old_kf for match search, on unit plane!
    Point2d A = LocationOptimizer::project2d(deltaRotation * (seed.localDir * d_min) + deltaTranslation);
    Point2d B = LocationOptimizer::project2d(deltaRotation * (seed.localDir * d_max) + deltaTranslation);
    Point2d epi_dir = A - B;
    std::shared_ptr<const Camera> secondCamera = secondFrame.camera();

    // Compute affine warp matrix
    TMatrixf w(2, 2);
    MapProjector::getWarpMatrixAffine(w,
                                      m_matcher.cursorSize().cast<float>(),
                                      keyFrame->camera(), secondCamera,
                                      seed.projections[0].projection, seed.localDir * d_estimate, seed.imageLevel,
                                      deltaRotation, deltaTranslation);

    int search_level = MapProjector::getBestSearchLevel(w, secondFrame.countImageLevels() - 1);

    if (!TTools::matrix2x2Invert(w))
        return false;


    Point2f px_A = secondCamera->project(A).cast<float>();
    Point2f px_B = secondCamera->project(B).cast<float>();
    float search_scale = (float)(1 << search_level);
    float epi_length = (px_A - px_B).length() / search_scale;

    Image<uchar> patch = m_matcher.patch();

    MapProjector::warpAffine(patch, w, keyFrame->imageLevel(seed.imageLevel), seed.projections[0].projection,
                             seed.imageLevel, search_level);

    ConstImage<uchar> searchImage = secondFrame.imageLevel(search_level);

    /*if (epi_length < 2.0) {
        m_lastProjection = (px_A + px_B) * 0.5f;
        m_lastProjection /= search_scale;
        m_matcher.setSecondImage(searchImage);
        if (m_matcher.tracking2d_patch(m_lastProjection) != TrackingResult::Completed)
            return false;
        m_lastProjection *= search_scale;
        Point2d cur = secondCamera->unproject(m_lastProjection);
        TVectord curLocalDir = TVectord::create(cur.x, cur.y, 1.0).normalized();
//        seed.projections.push_back({ secondCamera, m_lastProjection,
//                                     secondFrame.rotation(),
//                                     secondFrame.translation()
//                                   });
//        return true;
          return TTools::depthFromTriangulation(depth,
                                                deltaRotation, deltaTranslation,
                                                seed.localDir, curLocalDir);
    }*/

    int n_steps = (int)(epi_length / 0.7); // one step per pixel
    Point2d step = epi_dir / (double)n_steps;
    ++n_steps;

    if (n_steps > m_maxNumberOfSearchSteps)
        return false;

    // now we sample along the epipolar line
    Point2d uv = B - step, uv_best = uv;
    Point2i last_checked_px_i(0, 0);
    Point2d px;
    Point2i px_i;
    Point2i cursorSize = m_matcher.cursorSize() + Point2i(2, 2);
    int zmssd_best = std::numeric_limits<int>::max();
    for (int i=0; i < n_steps; ++i, uv += step) {
        px = secondCamera->project(uv);
        px_i.set((int)(px.x / search_scale + 0.5), (int)(px.y / search_scale + 0.5)); // +0.5 to round to closest int

        if (px_i == last_checked_px_i)
            continue;
        last_checked_px_i = px_i;

        // check if the patch is full within the new frame
        if ((px_i.x < cursorSize.x) ||
                (px_i.y < cursorSize.y) ||
                (px_i.x >= (searchImage.width() - (cursorSize.x + 1))) ||
                (px_i.y >= (searchImage.height() - (cursorSize.y + 1)))) {
            continue;
        }

        int zmssd = ZMSSD::compare(patch, Point2i(0, 0),
                                   searchImage, px_i - cursorSize,
                                   patch.size());

        if (zmssd < zmssd_best) {
            zmssd_best = zmssd;
            uv_best = uv;
        }
    }

    //if (zmssd_best < (patch.area() * 9000)) {
        //if (m_subpix_refinement) {
            m_lastProjection = secondCamera->project(uv_best).cast<float>();
            m_lastProjection /= search_scale;
            m_matcher.setSecondImage(searchImage);
            if (m_matcher.tracking2d_patch(m_lastProjection) != TrackingResult::Completed)
                return false;
            m_lastProjection *= search_scale;
            Point2d cur = secondCamera->unproject(m_lastProjection);
            TVectord curLocalDir = TVectord::create(cur.x, cur.y, 1.0).normalized();
            /*seed.projections.push_back({ secondCamera, m_lastProjection,
                                         secondFrame.rotation(),
                                         secondFrame.translation()
                                       });
            return true;*/
            return TTools::depthFromTriangulation(depth,
                                                  deltaRotation, deltaTranslation,
                                                  seed.localDir, curLocalDir);
        /*} else {
            TVectord uvLocalDir = TVectord::create(uv_best.x, uv_best.y, 1.0).normalized();
            return TTools::depthFromTriangulation(depth,
                                                  deltaRotation, deltaTranslation,
                                                  seed.localDir, uvLocalDir);
        }*/
    //}
    return false;
}

double MapPointsDetector::_computeTau(const TMath::TVectord & t, const TMath::TVectord & localDir,
                                       double z, double px_error_angle)
{
    using namespace TMath;
    TVectord a = localDir * z - t;
    double t_length = std::sqrt(t(0) * t(0) +
                                t(1) * t(1) +
                                t(2) * t(2));
    double a_length = std::sqrt(a(0) * a(0) + a(1) * a(1) + a(2) * a(2));
    double alpha = std::acos((localDir(0) * t(0) +
                              localDir(1) * t(1) +
                              localDir(2) * t(2)) / t_length); // dot product
    double beta = std::acos(- (a(0) * t(0) +
                               a(1) * t(1) +
                               a(2) * t(2)) / (t_length * a_length)); // dot product
    double beta_plus = beta + px_error_angle;
    double gamma_plus = M_PI - alpha - beta_plus; // triangle angles sum to PI
    double z_plus = t_length * std::sin(beta_plus) / std::sin(gamma_plus); // law of sines
    return (z_plus - z); // tau
}

void MapPointsDetector::_updateSeed(Seed & seed, float x, float tauSquared)
{
    float norm_scale = sqrt(seed.sigmaSquared + tauSquared);
    if (std::isnan(norm_scale))
        return;

    float sSquared = 1.0f / (1.0f / seed.sigmaSquared + 1.0f / tauSquared);
    float m = sSquared * (seed.mu / seed.sigmaSquared + x / tauSquared);
    float C1 = seed.a / (seed.a + seed.b) * _pdf(x, seed.mu, norm_scale);
    float C2 = seed.b / (seed.a + seed.b) * (1.0f / seed.z_range);
    float normalization_constant = C1 + C2;
    C1 /= normalization_constant;
    C2 /= normalization_constant;
    float f = C1 * (seed.a + 1.0f) / (seed.a + seed.b + 1.0f) + C2 * seed.a / (seed.a + seed.b + 1.0f);
    float e = C1 * (seed.a + 1.0f) * (seed.a + 2.0f) / ((seed.a + seed.b + 1.0f) * (seed.a + seed.b + 2.0f))
            + C2 * seed.a * (seed.a + 1.0f) / ((seed.a + seed.b + 1.0f) * (seed.a + seed.b + 2.0f));

    // update parameters
    float mu_new = C1 * m + C2 * seed.mu;
    seed.sigmaSquared = C1 * (sSquared + m * m) + C2 * (seed.sigmaSquared + seed.mu * seed.mu) - mu_new * mu_new;
    seed.mu = mu_new;
    seed.a = (e - f) / (f - e / f);
    seed.b = seed.a * (1.0f - f) / f;
}

float MapPointsDetector::_pdf(float x, float mean, float scale)
{
    float exponent = x - mean;
    exponent *= - exponent;
    exponent /= 2.0f * scale * scale;

    return std::exp(exponent) / (scale * std::sqrt(2.0f * (float)(M_PI)));
}

void MapPointsDetector::_addPreparedPoint(Seed & seed, const TMath::TVectord & worldPoint)
{

    std::shared_ptr<KeyFrame> keyFrame = seed.keyFrame.lock();
    if (!keyFrame)
        return;

    new CandidateMapPoint(m_preparedCandidates,
                          keyFrame,
                          seed.projections[0].projection,
                          seed.imageLevel,
                          worldPoint);
    if (m_preparedCandidates->size() >= m_sizeOfCommitMapPoints) {
        _commitPreparedPoints();
    }
}

void MapPointsDetector::_commitPreparedPoints()
{
    if (!m_preparedCandidates->isEmpty()) {
        /*CandidateMapPoint * p = m_preparedCandidates->head();
        CandidateMapPoint * p_next;
        while (p != nullptr) {
            p_next = p->next();
            KeyFramePtr keyFrame = p->keyFrame;
            if (keyFrame->isDeleted())
                delete p;
            p = p_next;
        }*/
        {
            std::lock_guard<std::mutex> locker(m_mutex_finalCandidates); (void)locker;
            m_finalCandidates->splice(m_preparedCandidates);
            while (m_finalCandidates->size() > m_maxCountCandidatePoints)
                delete m_finalCandidates->head();
        }
    }
}

double MapPointsDetector::_getPointInWorld(TMath::TVectord & result, const std::vector<ProjectionPoint> & projections) const
{
    using namespace TMath;

    TMath_assert(projections.size() >= 2);
    TMath_assert(result.size() == 3);
    std::size_t i;
    TMatrixd M((int)(projections.size() * 2), 4);
    double * dataRow = M.firstDataRow();
    for (i = 0; i < projections.size(); ++i) {
        const ProjectionPoint& p = projections[i];
        Point2d uv = p.camera->unproject(p.projection);
        dataRow[0] = p.rotation(0, 0) - uv.x * p.rotation(2, 0);
        dataRow[1] = p.rotation(0, 1) - uv.x * p.rotation(2, 1);
        dataRow[2] = p.rotation(0, 2) - uv.x * p.rotation(2, 2);
        dataRow[3] = p.translation(0) - uv.x * p.translation(2);
        dataRow = &dataRow[4];

        dataRow[0] = p.rotation(1, 0) - uv.y * p.rotation(2, 0);
        dataRow[1] = p.rotation(1, 1) - uv.y * p.rotation(2, 1);
        dataRow[2] = p.rotation(1, 2) - uv.y * p.rotation(2, 2);
        dataRow[3] = p.translation(1) - uv.y * p.translation(2);
        dataRow = &dataRow[4];
    }
    m_svd.compute(M, false);
    const double * resultData = m_svd.V_transposed().getDataRow(3);
    if (std::fabs(resultData[3]) < 1e-4)
        return std::numeric_limits<double>::max();
    result(0) = resultData[0] / resultData[3];
    result(1) = resultData[1] / resultData[3];
    result(2) = resultData[2] / resultData[3];
    return std::fabs(m_svd.diagonalW()(3));
    /*float maxErrorSquared = 0.0f, errorSquared;
    for (i = 0; i < projections.size(); ++i) {
        const ProjectionPoint & p = projections[i];
        TVectorf v = p.rotation * result + p.translation;
        errorSquared = (p.camera->project(Point2d((double)(v(0) / v(2)), (double)(v(1) / v(2)))).cast<float>() -
                        p.projection).lengthSquared();
        if (errorSquared > maxErrorSquared)
            maxErrorSquared = errorSquared;
    }
    return maxErrorSquared;*/
}

}
