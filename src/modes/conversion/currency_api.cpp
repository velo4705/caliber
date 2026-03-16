#include "currency_api.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QUrl>

static const QString API_URL = "https://open.er-api.com/v6/latest/%1";

CurrencyApi::CurrencyApi(QObject* parent)
    : QObject(parent)
    , m_base("USD")
{
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, &QNetworkAccessManager::finished,
            this,  &CurrencyApi::onReplyFinished);

    loadCachedRates();
}

void CurrencyApi::fetchRates(const QString& baseCurrency) {
    m_base = baseCurrency.toUpper();
    QUrl url(API_URL.arg(m_base));
    m_nam->get(QNetworkRequest(url));
}

void CurrencyApi::onReplyFinished(QNetworkReply* reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit fetchError(reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull() || !doc.isObject()) {
        emit fetchError("Invalid JSON response");
        return;
    }

    QJsonObject root = doc.object();
    if (root.value("result").toString() != "success") {
        emit fetchError("API returned non-success result");
        return;
    }

    m_rates.clear();
    QJsonObject rates = root.value("rates").toObject();
    for (auto it = rates.begin(); it != rates.end(); ++it)
        m_rates[it.key()] = it.value().toDouble();

    m_base        = root.value("base_code").toString("USD");
    m_lastUpdated = QDateTime::currentDateTime();

    saveCachedRates();
    emit ratesUpdated();
}

double CurrencyApi::convert(const QString& from, const QString& to, double amount) const {
    if (m_rates.isEmpty()) return -1.0;

    QString f = from.toUpper();
    QString t = to.toUpper();

    // Convert from -> base -> to
    double fromRate = (f == m_base) ? 1.0 : m_rates.value(f, 0.0);
    double toRate   = (t == m_base) ? 1.0 : m_rates.value(t, 0.0);

    if (fromRate == 0.0 || toRate == 0.0) return -1.0;

    return amount / fromRate * toRate;
}

QStringList CurrencyApi::availableCurrencies() const {
    QStringList list = m_rates.keys();
    if (!list.contains(m_base)) list.prepend(m_base);
    list.sort();
    return list;
}

// ── Cache (JSON file in app data dir) ────────────────────────────────────────

QString CurrencyApi::cacheFilePath() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/currency_cache.json";
}

void CurrencyApi::saveCachedRates() {
    QJsonObject obj;
    obj["base"]    = m_base;
    obj["updated"] = m_lastUpdated.toString(Qt::ISODate);
    QJsonObject rates;
    for (auto it = m_rates.begin(); it != m_rates.end(); ++it)
        rates[it.key()] = it.value();
    obj["rates"] = rates;

    QFile f(cacheFilePath());
    if (f.open(QFile::WriteOnly))
        f.write(QJsonDocument(obj).toJson());
}

void CurrencyApi::loadCachedRates() {
    QFile f(cacheFilePath());
    if (!f.open(QFile::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (doc.isNull()) return;

    QJsonObject root = doc.object();
    m_base        = root.value("base").toString("USD");
    m_lastUpdated = QDateTime::fromString(root.value("updated").toString(), Qt::ISODate);

    m_rates.clear();
    QJsonObject rates = root.value("rates").toObject();
    for (auto it = rates.begin(); it != rates.end(); ++it)
        m_rates[it.key()] = it.value().toDouble();
}
