#include "units.h"

std::unique_ptr<Units> Units::instance = nullptr;

/* ===================== SINGLETON ===================== */

Units& Units::getInstance() {
    if (!instance) {
        instance.reset(new Units());
    }
    return *instance;
}

Units::Units() {
    initConversionFactors();
}

/* ===================== STATIC FACTORS ===================== */

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

/* ===================== UNIT CATEGORY ===================== */

UnitCategory Units::getCategory(const QString& unit) const {

    // Static categories
    if (lengthFactors.find(unit) != lengthFactors.end()) return UnitCategory::Length;
    if (weightFactors.find(unit) != weightFactors.end()) return UnitCategory::Weight;
    if (volumeFactors.find(unit) != volumeFactors.end()) return UnitCategory::Volume;
    if (speedFactors.find(unit) != speedFactors.end()) return UnitCategory::Speed;
    if (unit == "Celsius" || unit == "Fahrenheit") return UnitCategory::Temperature;

    // Currency (always treat these as currency)
    if (unit == "USD" || unit == "ZAR" || unit == "EUR" || unit == "GBP" || unit == "JPY")
        return UnitCategory::Currency;

    return UnitCategory::Length; // fallback
}

/* ===================== CONVERSION ENGINE ===================== */

double Units::convert(const QString &from, const QString &to, double value) {
    UnitCategory category = getCategory(from);
    if (category != getCategory(to))
        return value; // mismatched categories -> do nothing

    switch (category) {

    /* ----- LENGTH ----- */
    case UnitCategory::Length:
        if (lengthFactors.find(from) != lengthFactors.end() &&
            lengthFactors.find(to) != lengthFactors.end())
            return value * (lengthFactors.at(to) / lengthFactors.at(from));
        return value;

    /* ----- WEIGHT ----- */
    case UnitCategory::Weight:
        if (weightFactors.find(from) != weightFactors.end() &&
            weightFactors.find(to) != weightFactors.end())
            return value * (weightFactors.at(to) / weightFactors.at(from));
        return value;

    /* ----- VOLUME ----- */
    case UnitCategory::Volume:
        if (volumeFactors.find(from) != volumeFactors.end() &&
            volumeFactors.find(to) != volumeFactors.end())
            return value * (volumeFactors.at(to) / volumeFactors.at(from));
        return value;

    /* ----- SPEED ----- */
    case UnitCategory::Speed:
        if (speedFactors.find(from) != speedFactors.end() &&
            speedFactors.find(to) != speedFactors.end())
            return value * (speedFactors.at(to) / speedFactors.at(from));
        return value;

    /* ----- TEMPERATURE ----- */
    case UnitCategory::Temperature:
        if (from == "Celsius" && to == "Fahrenheit")
            return (value * 9.0 / 5.0) + 32;
        if (from == "Fahrenheit" && to == "Celsius")
            return (value - 32) * 5.0 / 9.0;
        return value;

    /* ----- CURRENCY (API-fed values) ----- */
    case UnitCategory::Currency: {
        double rate = 0.0;
        if (getCurrencyRate(from, to, rate))
            return value * rate;

        return value; // fallback if API not ready
    }
    }

    return value;
}

/* ===================== UI POPULATION ===================== */

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

    case UnitCategory::Currency:
        // Static list; API will fill values later
        combo->addItems({"USD", "ZAR", "EUR", "GBP", "JPY"});
        break;
    }
}

/* ===================== CURRENCY RATE STORAGE ===================== */

void Units::setCurrencyRate(const QString &from, const QString &to, double rate) {
    currencyRates[from][to] = rate;
}

bool Units::getCurrencyRate(const QString &from, const QString &to, double &outRate) const {
    auto itFrom = currencyRates.find(from);
    if (itFrom == currencyRates.end())
        return false;

    auto itTo = itFrom->second.find(to);
    if (itTo == itFrom->second.end())
        return false;

    outRate = itTo->second;
    return true;
}
