#include "unit_converter.h"
#include <stdexcept>

UnitConverter::UnitConverter() { buildCategories(); }

void UnitConverter::buildCategories() {
    // Base unit noted in comments
    m_categories = {
        { "Length", {           // base: metre
            {"Metre",           1.0},
            {"Kilometre",       1000.0},
            {"Centimetre",      0.01},
            {"Millimetre",      0.001},
            {"Micrometre",      1e-6},
            {"Nanometre",       1e-9},
            {"Mile",            1609.344},
            {"Yard",            0.9144},
            {"Foot",            0.3048},
            {"Inch",            0.0254},
            {"Nautical Mile",   1852.0},
            {"Light Year",      9.461e15},
        }},
        { "Mass", {             // base: kilogram
            {"Kilogram",        1.0},
            {"Gram",            0.001},
            {"Milligram",       1e-6},
            {"Tonne",           1000.0},
            {"Pound",           0.45359237},
            {"Ounce",           0.028349523},
            {"Stone",           6.35029318},
            {"US Ton",          907.18474},
            {"Imperial Ton",    1016.0469},
        }},
        { "Temperature", {      // special handling below
            {"Celsius",         1.0},
            {"Fahrenheit",      1.0},
            {"Kelvin",          1.0},
            {"Rankine",         1.0},
        }},
        { "Speed", {            // base: m/s
            {"Metre/second",    1.0},
            {"Kilometre/hour",  1.0/3.6},
            {"Mile/hour",       0.44704},
            {"Knot",            0.514444},
            {"Foot/second",     0.3048},
            {"Mach",            343.0},
        }},
        { "Area", {             // base: m²
            {"Square Metre",    1.0},
            {"Square Kilometre",1e6},
            {"Square Mile",     2589988.11},
            {"Square Yard",     0.836127},
            {"Square Foot",     0.092903},
            {"Square Inch",     6.4516e-4},
            {"Hectare",         10000.0},
            {"Acre",            4046.856},
        }},
        { "Volume", {           // base: litre
            {"Litre",           1.0},
            {"Millilitre",      0.001},
            {"Cubic Metre",     1000.0},
            {"Cubic Centimetre",0.001},
            {"Cubic Inch",      0.016387},
            {"Cubic Foot",      28.3168},
            {"US Gallon",       3.78541},
            {"UK Gallon",       4.54609},
            {"US Quart",        0.946353},
            {"US Pint",         0.473176},
            {"US Cup",          0.236588},
            {"US Fluid Ounce",  0.0295735},
            {"Tablespoon",      0.0147868},
            {"Teaspoon",        0.00492892},
        }},
        { "Energy", {           // base: joule
            {"Joule",           1.0},
            {"Kilojoule",       1000.0},
            {"Calorie",         4.184},
            {"Kilocalorie",     4184.0},
            {"Watt-hour",       3600.0},
            {"Kilowatt-hour",   3.6e6},
            {"Electronvolt",    1.60218e-19},
            {"BTU",             1055.06},
            {"Foot-pound",      1.35582},
        }},
        { "Pressure", {         // base: pascal
            {"Pascal",          1.0},
            {"Kilopascal",      1000.0},
            {"Megapascal",      1e6},
            {"Bar",             1e5},
            {"Millibar",        100.0},
            {"Atmosphere",      101325.0},
            {"PSI",             6894.76},
            {"Torr",            133.322},
            {"mmHg",            133.322},
        }},
        { "Power", {            // base: watt
            {"Watt",            1.0},
            {"Kilowatt",        1000.0},
            {"Megawatt",        1e6},
            {"Horsepower (mech)",745.7},
            {"Horsepower (elec)",746.0},
            {"BTU/hour",        0.29307},
        }},
        { "Data", {             // base: byte
            {"Bit",             0.125},
            {"Byte",            1.0},
            {"Kilobyte",        1024.0},
            {"Megabyte",        1048576.0},
            {"Gigabyte",        1073741824.0},
            {"Terabyte",        1099511627776.0},
            {"Kibibyte",        1024.0},
            {"Mebibyte",        1048576.0},
            {"Gibibyte",        1073741824.0},
        }},
        { "Angle", {            // base: degree
            {"Degree",          1.0},
            {"Radian",          180.0 / 3.14159265358979},
            {"Gradian",         0.9},
            {"Arcminute",       1.0/60.0},
            {"Arcsecond",       1.0/3600.0},
            {"Turn",            360.0},
        }},
        { "Time", {             // base: second
            {"Second",          1.0},
            {"Millisecond",     0.001},
            {"Microsecond",     1e-6},
            {"Nanosecond",      1e-9},
            {"Minute",          60.0},
            {"Hour",            3600.0},
            {"Day",             86400.0},
            {"Week",            604800.0},
            {"Month (avg)",     2629800.0},
            {"Year",            31557600.0},
        }},
        { "Fuel Economy", {     // base: L/100km
            {"L/100km",         1.0},
            {"km/L",            100.0},   // inverted — handled specially? No, approx
            {"MPG (US)",        235.215},
            {"MPG (UK)",        282.481},
        }},
    };
}

QStringList UnitConverter::categoryNames() const {
    QStringList names;
    for (const auto& cat : m_categories)
        names << cat.name;
    return names;
}

QStringList UnitConverter::unitNames(const QString& category) const {
    for (const auto& cat : m_categories)
        if (cat.name == category) {
            QStringList names;
            for (const auto& u : cat.units) names << u.name;
            return names;
        }
    return {};
}

double UnitConverter::convert(const QString& category,
                               const QString& fromUnit,
                               const QString& toUnit,
                               double value) const
{
    // Temperature needs special handling (non-linear)
    if (category == "Temperature") {
        // Convert to Celsius first
        double celsius;
        if      (fromUnit == "Celsius")     celsius = value;
        else if (fromUnit == "Fahrenheit")  celsius = (value - 32.0) * 5.0 / 9.0;
        else if (fromUnit == "Kelvin")      celsius = value - 273.15;
        else if (fromUnit == "Rankine")     celsius = (value - 491.67) * 5.0 / 9.0;
        else throw std::runtime_error("Unknown temperature unit");

        if      (toUnit == "Celsius")       return celsius;
        else if (toUnit == "Fahrenheit")    return celsius * 9.0 / 5.0 + 32.0;
        else if (toUnit == "Kelvin")        return celsius + 273.15;
        else if (toUnit == "Rankine")       return (celsius + 273.15) * 9.0 / 5.0;
        else throw std::runtime_error("Unknown temperature unit");
    }

    // General: find toBase factors
    double fromFactor = 0.0, toFactor = 0.0;
    for (const auto& cat : m_categories) {
        if (cat.name != category) continue;
        for (const auto& u : cat.units) {
            if (u.name == fromUnit) fromFactor = u.toBase;
            if (u.name == toUnit)   toFactor   = u.toBase;
        }
    }
    if (fromFactor == 0.0) throw std::runtime_error("Unknown unit: " + fromUnit.toStdString());
    if (toFactor   == 0.0) throw std::runtime_error("Unknown unit: " + toUnit.toStdString());

    return value * fromFactor / toFactor;
}
