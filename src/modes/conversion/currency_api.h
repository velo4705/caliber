#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QDateTime>

class QNetworkAccessManager;
class QNetworkReply;

// Fetches live exchange rates from open.er-api.com (free, no key needed)
// Falls back to cached rates if offline
class CurrencyApi : public QObject {
    Q_OBJECT
public:
    explicit CurrencyApi(QObject* parent = nullptr);

    void fetchRates(const QString& baseCurrency = "USD");

    // Convert amount from -> to using cached rates
    // Returns -1.0 if rates not available
    double convert(const QString& from, const QString& to, double amount) const;

    QStringList availableCurrencies() const;
    bool        hasRates()            const { return !m_rates.isEmpty(); }
    QDateTime   lastUpdated()         const { return m_lastUpdated; }
    QString     baseCurrency()        const { return m_base; }

signals:
    void ratesUpdated();
    void fetchError(const QString& message);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_nam;
    QMap<QString, double>  m_rates;   // currency -> rate relative to base
    QString                m_base;
    QDateTime              m_lastUpdated;

    void loadCachedRates();
    void saveCachedRates();
    QString cacheFilePath() const;
};
