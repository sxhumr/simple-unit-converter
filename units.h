#ifndef UNITS_H
#define UNITS_H

#include <QString>
#include <QComboBox>
#include <unordered_map>
#include <memory>

enum class UnitCategory { Length, Weight, Temperature, Volume, Speed, Currency };

class Units
{
public:
    static Units& getInstance();

    double convert(const QString &from, const QString &to, double value);
    void populateUnits(QComboBox* combo, UnitCategory category);
    UnitCategory getCategory(const QString& unit) const;

    // --------  Currency rate management --------
    void setCurrencyRate(const QString& from, const QString& to, double rate);
    bool getCurrencyRate(const QString& from, const QString& to, double& outRate) const;

private:
    Units();
    Units(const Units&) = delete;
    Units& operator=(const Units&) = delete;

    void initConversionFactors();

    std::unordered_map<QString, double> lengthFactors;
    std::unordered_map<QString, double> weightFactors;
    std::unordered_map<QString, double> volumeFactors;
    std::unordered_map<QString, double> speedFactors;

    // -------- currency rates --------
    std::unordered_map<QString, std::unordered_map<QString, double>> currencyRates;

    static std::unique_ptr<Units> instance;
};

#endif // UNITS_H
