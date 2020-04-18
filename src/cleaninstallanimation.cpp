#include "cleaninstallanimation.h"

#define ANIMATION_DURATION 300

/**
 * @brief Creates a CleaninstallAnimation object
 * @param parent The parent object
 */
CleaninstallAnimation::CleaninstallAnimation(QObject *parent) : QObject(parent)
{
	connect(&timer, &QTimer::timeout, this, &CleaninstallAnimation::progress);
}

/**
 * @brief Triggers the cleaninstall animation
 * @param newSegments The current state of segments in the Curver object
 *
 * This will take ownership of all segments from the Curver object.
 * This method automatically removes all segments from the Curver object.
 */
void CleaninstallAnimation::trigger(std::vector<std::unique_ptr<Segment> > &newSegments)
{
	if (newSegments.size() == 0) {
		// do not spawn an animation, if there is nothing to animate
		return;
	}
	timer.stop();
	for (auto it = newSegments.begin(); it != newSegments.end(); ++it) {
		segments.push_back(std::move(*it));
	}
	newSegments.clear();
	// calculate total amount of points
	sizeCache.resize(segments.size());
	pointsDeleted.resize(segments.size());
	for (size_t i = 0; i < segments.size(); ++i) {
		sizeCache[i] = segments[i]->getSegmentSize();
		pointsDeleted[i] = 0;
	}
	totalSize = Util::accumulate(sizeCache, 0);
	initialTime = QTime::currentTime();
	timer.start(16);
}

/**
 * @brief Updates the animation
 */
void CleaninstallAnimation::progress()
{
	const float timeSinceStart = initialTime.msecsTo(QTime::currentTime());
	const float factor = timeSinceStart / ANIMATION_DURATION;
	const size_t pos = totalSize * factor;
	if (factor < 1) {
		size_t pointsToDelete = pos;
		size_t segmentIndex;
		for (segmentIndex = 0; segmentIndex < sizeCache.size() && pointsToDelete >= sizeCache[segmentIndex]; ++segmentIndex) {
			pointsToDelete -= sizeCache[segmentIndex];
			segments[segmentIndex]->clear();
			pointsDeleted[segmentIndex] = sizeCache[segmentIndex];
		}
		segments[segmentIndex]->popPoints(pointsToDelete - pointsDeleted[segmentIndex]);
		pointsDeleted[segmentIndex] = pointsToDelete;
	} else {
		timer.stop();
		segments.clear();
	}
}
