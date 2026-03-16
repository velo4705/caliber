#pragma once
#include <QWidget>

class QComboBox;
class QLineEdit;
class QLabel;
class QTabWidget;
class UnitConverter;
class CurrencyApi;

class ConversionWidget : public QWidget {
    Q_OBJECT
public:
    explicit ConversionWidget(QWidget* parent = nullptr);

private:
    // Unit conversion tab
    QWidget*  buildUnitTab();
    QComboBox* m_categoryCombo;
    QComboBox* m_fromUnitCombo;
    QComboBox* m_toUnitCombo;
    QLineEdit* m_unitInput;
    QLabel*    m_unitResult;

    // Currency tab
    QWidget*  buildCurrencyTab();
    QComboBox* m_fromCurrCombo;
    QComboBox* m_toCurrCombo;
    QLineEdit* m_currInput;
    QLabel*    m_currResult;
    QLabel*    m_currStatus;

    UnitConverter* m_converter;
    CurrencyApi*   m_currencyApi;

    void onCategoryChanged(const QString& category);
    void convertUnits();
    void convertCurrency();
    void onRatesUpdated();
    void populateCurrencies();
    void swapUnits();
    void swapCurrencies();
};
