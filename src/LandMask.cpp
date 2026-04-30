#include "LandMask.h"
#include <algorithm>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>

bool LandMask::loadGeoJson(const QString& path, QString* errorMessage)
{
    polygons.clear();

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage)
            *errorMessage = "Could not open land GeoJSON file.";
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage)
            *errorMessage = "GeoJSON parse error: " + parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        if (errorMessage)
            *errorMessage = "GeoJSON root is not an object.";
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray features = root.value("features").toArray();

    if (features.isEmpty()) {
        if (errorMessage)
            *errorMessage = "GeoJSON has no features.";
        return false;
    }

    for (const QJsonValue& featureValue : features) {
        QJsonObject feature = featureValue.toObject();
        QJsonObject geometry = feature.value("geometry").toObject();

        QString type = geometry.value("type").toString();
        QJsonArray coordinates = geometry.value("coordinates").toArray();

        if (type == "Polygon") {
            Polygon polygon = parsePolygon(coordinates);

            if (!polygon.rings.isEmpty())
                polygons.append(polygon);
        }
        else if (type == "MultiPolygon") {
            for (const QJsonValue& polygonValue : coordinates) {
                Polygon polygon = parsePolygon(polygonValue.toArray());

                if (!polygon.rings.isEmpty())
                    polygons.append(polygon);
            }
        }
    }

    if (polygons.isEmpty()) {
        if (errorMessage)
            *errorMessage = "No usable land polygons were loaded.";
        return false;
    }

    return true;
}

LandMask::Ring LandMask::parseRing(const QJsonArray& ringArray)
{
    Ring ring;

    ring.minLon = std::numeric_limits<double>::max();
    ring.maxLon = std::numeric_limits<double>::lowest();
    ring.minLat = std::numeric_limits<double>::max();
    ring.maxLat = std::numeric_limits<double>::lowest();

    for (const QJsonValue& pointValue : ringArray) {
        QJsonArray point = pointValue.toArray();

        if (point.size() < 2)
            continue;

        double lon = point.at(0).toDouble();
        double lat = point.at(1).toDouble();

        ring.points.append(QPointF(lon, lat));

        ring.minLon = std::min(ring.minLon, lon);
        ring.maxLon = std::max(ring.maxLon, lon);
        ring.minLat = std::min(ring.minLat, lat);
        ring.maxLat = std::max(ring.maxLat, lat);
    }

    return ring;
}

LandMask::Polygon LandMask::parsePolygon(const QJsonArray& polygonArray)
{
    Polygon polygon;

    polygon.minLon = std::numeric_limits<double>::max();
    polygon.maxLon = std::numeric_limits<double>::lowest();
    polygon.minLat = std::numeric_limits<double>::max();
    polygon.maxLat = std::numeric_limits<double>::lowest();

    for (const QJsonValue& ringValue : polygonArray) {
        Ring ring = parseRing(ringValue.toArray());

        if (ring.points.size() < 3)
            continue;

        polygon.rings.append(ring);

        polygon.minLon = std::min(polygon.minLon, ring.minLon);
        polygon.maxLon = std::max(polygon.maxLon, ring.maxLon);
        polygon.minLat = std::min(polygon.minLat, ring.minLat);
        polygon.maxLat = std::max(polygon.maxLat, ring.maxLat);
    }

    return polygon;
}

bool LandMask::pointInRing(const Ring& ring, double latitude, double longitude)
{
    if (longitude < ring.minLon || longitude > ring.maxLon ||
        latitude < ring.minLat || latitude > ring.maxLat) {
        return false;
    }

    bool inside = false;
    int count = ring.points.size();

    for (int i = 0, j = count - 1; i < count; j = i++) {
        double xi = ring.points[i].x();
        double yi = ring.points[i].y();
        double xj = ring.points[j].x();
        double yj = ring.points[j].y();

        bool intersects =
            ((yi > latitude) != (yj > latitude)) &&
            (longitude < (xj - xi) * (latitude - yi) / ((yj - yi) + 1e-15) + xi);

        if (intersects)
            inside = !inside;
    }

    return inside;
}

bool LandMask::isLand(double latitude, double longitude) const
{
    for (const Polygon& polygon : polygons) {
        if (longitude < polygon.minLon || longitude > polygon.maxLon ||
            latitude < polygon.minLat || latitude > polygon.maxLat) {
            continue;
        }

        if (polygon.rings.isEmpty())
            continue;

        bool insideOuter = pointInRing(polygon.rings.first(), latitude, longitude);

        if (!insideOuter)
            continue;

        // Holes inside land polygons count as water.
        for (int i = 1; i < polygon.rings.size(); i++) {
            if (pointInRing(polygon.rings[i], latitude, longitude))
                return false;
        }

        return true;
    }

    return false;
}