#pragma once

#include <QString>
#include <QVector>
#include <QPointF>
#include <QJsonArray>

class LandMask
{
public:
    bool loadGeoJson(const QString& path, QString* errorMessage = nullptr);
    bool isLand(double latitude, double longitude) const;

private:
    struct Ring
    {
        QVector<QPointF> points; // x = longitude, y = latitude
        double minLon = 0;
        double maxLon = 0;
        double minLat = 0;
        double maxLat = 0;
    };

    struct Polygon
    {
        QVector<Ring> rings; // first = outer ring, rest = holes
        double minLon = 0;
        double maxLon = 0;
        double minLat = 0;
        double maxLat = 0;
    };

    QVector<Polygon> polygons;

    static Ring parseRing(const QJsonArray& ringArray);
    static Polygon parsePolygon(const QJsonArray& polygonArray);
    static bool pointInRing(const Ring& ring, double latitude, double longitude);
};