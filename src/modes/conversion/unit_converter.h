#pragma once
#include <QString>
#include <QStringList>
#include <QMap>

struct ConversionUnit {
    QString name;
    double  toBase; // multiply by this to convert to base unit
};

struct ConversionCategory {
    QString                  name;
    QList<ConversionUnit>    units;
};

// Offline unit conversion engine
// All conversions go through a base unit (SI where possible)
class UnitConverter {
public:
    UnitConverter();

    QStringList categoryNames() const;
    QStringList unitNames(const QString& category) const;

    // Convert value from fromUnit to toUnit within category
    // Returns converted value, throws std::runtime_error on unknown unit
    double convert(const QString& category,
                   const QString& fromUnit,
                   const QString& toUnit,
                   double value) const;

private:
    QList<ConversionCategory> m_categories;
    void buildCategories();
};
