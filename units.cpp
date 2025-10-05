#include "units.h"

std::unique_ptr<Units> Units::instance = nullptr;

Units& Units::getInstance() {
    if (!instance) {
        instance.reset(new Units());
    }
    return *instance;
}

Units::Units() {
    initConversionFactors();
}

void Units::initConversionFactors() {
    lengthFactors["Meters"] = 1.0;
    lengthFactors["Feet"] = 3.28084;
    lengthFactors["Kilometers"] = 1000.0;
    lengthFactors["Miles"] = 1609.34;

    weightFactors["Kilograms"] = 1.0;
    weightFactors["Pounds"] = 2.20462;

    volumeFactors["Liters"] = 1.0;
    volumeFactors["Milliliters"] = 0.001;
    volumeFactors["Gallons"] = 3.78541;

    speedFactors["m/s"] = 1.0;
    speedFactors["km/h"] = 0.277778;
    speedFactors["mph"] = 0.44704;
}

UnitCategory Units::getCategory(const QString& unit) const {
    if (lengthFactors.find(unit) != lengthFactors.end()) return UnitCategory::Length;
    if (weightFactors.find(unit) != weightFactors.end()) return UnitCategory::Weight;
    if (volumeFactors.find(unit) != volumeFactors.end()) return UnitCategory::Volume;
    if (speedFactors.find(unit) != speedFactors.end()) return UnitCategory::Speed;
    if (unit == "Celsius" || unit == "Fahrenheit") return UnitCategory::Temperature;
    return UnitCategory::Length;
}

double Units::convert(const QString &from, const QString &to, double value) {
    UnitCategory category = getCategory(from);
    if (category != getCategory(to)) return value;

    switch (category) {
    case UnitCategory::Length:
        return value * (lengthFactors[to] / lengthFactors[from]);
    case UnitCategory::Weight:
        return value * (weightFactors[to] / weightFactors[from]);
    case UnitCategory::Volume:
        return value * (volumeFactors[to] / volumeFactors[from]);
    case UnitCategory::Speed:
        return value * (speedFactors[to] / speedFactors[from]);
    case UnitCategory::Temperature:
        if (from == "Celsius" && to == "Fahrenheit") return (value * 9.0/5.0) + 32;
        if (from == "Fahrenheit" && to == "Celsius") return (value - 32) * 5.0/9.0;
        return value;
    }
    return value;
}

void Units::populateUnits(QComboBox *combo, UnitCategory category) {
    if (!combo) return;
    combo->clear();
    switch (category) {
    case UnitCategory::Length:
        combo->addItems({"Meters", "Feet", "Kilometers", "Miles"});
        break;
    case UnitCategory::Weight:
        combo->addItems({"Kilograms", "Pounds"});
        break;
    case UnitCategory::Temperature:
        combo->addItems({"Celsius", "Fahrenheit"});
        break;
    case UnitCategory::Volume:
        combo->addItems({"Liters", "Milliliters", "Gallons"});
        break;
    case UnitCategory::Speed:
        combo->addItems({"m/s", "km/h", "mph"});
        break;
    }
}
