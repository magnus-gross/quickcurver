#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <QObject>
#include <QtQuick>
#include <memory>
#include <QTimer>
#include <QPointF>

#include "util.h"

#define PARTICLECOUNT 64
#define PARTICLESIZE 4
#define PARTICLERANGE 256
#define PARTICLELIFETIME 512

/**
 * @brief A class representing a Curver explosion
 *
 * This class is used, when a Curver died
 */
class Explosion : public QObject
{
	Q_OBJECT
public:
	explicit Explosion(QPointF location, QSGNode *parentNode, QSGFlatColorMaterial *material, QObject *parent = nullptr);
	~Explosion();
private slots:
	void progress();
private:
	/**
	 * @brief The location of the explosion's origin
	 */
	QPointF location;
	/**
	 * @brief The parent node in the scene graph
	 */
	QSGNode *parentNode;
	/**
	 * @brief The node responsible for fading the explosion out
	 */
	std::unique_ptr<QSGOpacityNode> opacityNode;
	/**
	 * @brief The node containg the actual explosion
	 */
	std::unique_ptr<QSGGeometryNode> geoNode;
	/**
	 * @brief The geometry of the explosion
	 */
	QSGGeometry geometry = QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
	/**
	 * @brief The timer responsible for updating the explosion
	 */
	QTimer timer;
	/**
	 * @brief The time of the explosion start
	 */
	QTime initialTime = QTime::currentTime();
	/**
	 * @brief A pointer to the actual geometry data
	 */
	QSGGeometry::Point2D *vertices;
	/**
	 * @brief The directions of each individual particle
	 */
	QPointF particleDirections[PARTICLECOUNT];
};

#endif // EXPLOSION_H
