#include "date_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QDateEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QDate>
#include <QFrame>

// ── shared helpers ────────────────────────────────────────────────────────────

static QLabel* resultLabel(QWidget* parent) {
    auto* lbl = new QLabel("—", parent);
    lbl->setObjectName("resultLabel");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setWordWrap(true);
    lbl->setStyleSheet("font-size:18px; font-weight:bold; padding:10px;");
    return lbl;
}

static QDateEdit* dateEdit(QWidget* parent, QDate initial = QDate::currentDate()) {
    auto* de = new QDateEdit(initial, parent);
    de->setCalendarPopup(true);
    de->setDisplayFormat("dd MMM yyyy");
    de->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return de;
}

static QPushButton* calcBtn(const QString& text, QWidget* parent) {
    auto* b = new QPushButton(text, parent);
    b->setProperty("class", "actionButton");
    b->setMinimumHeight(40);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

static QFrame* card(QWidget* parent) {
    auto* f = new QFrame(parent);
    f->setObjectName("displayWidget");
    f->setFrameShape(QFrame::StyledPanel);
    return f;
}

// ── constructor ───────────────────────────────────────────────────────────────

DateWidget::DateWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto* title = new QLabel("Date Calculation", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    layout->addWidget(title);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(buildDiffTab(),       "Date Difference");
    tabs->addTab(buildAddSubTab(),     "Add / Subtract");
    tabs->addTab(buildAgeTab(),        "Age Calculator");
    tabs->addTab(buildDayOfWeekTab(),  "Day of Week");
    layout->addWidget(tabs);
    setLayout(layout);
}

// ── Tab 1: Date Difference ────────────────────────────────────────────────────

QWidget* DateWidget::buildDiffTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);

    auto* c = card(w);
    auto* form = new QFormLayout(c);
    form->setContentsMargins(12, 12, 12, 12);
    form->setSpacing(8);

    m_diffFrom = dateEdit(w, QDate::currentDate().addYears(-1));
    m_diffTo   = dateEdit(w);
    form->addRow("From:", m_diffFrom);
    form->addRow("To:",   m_diffTo);
    v->addWidget(c);

    auto* btn = calcBtn("Calculate Difference", w);
    v->addWidget(btn);

    m_diffResult = resultLabel(w);
    v->addWidget(m_diffResult);
    v->addStretch();

    connect(btn, &QPushButton::clicked, this, &DateWidget::calcDiff);
    connect(m_diffFrom, &QDateEdit::dateChanged, this, &DateWidget::calcDiff);
    connect(m_diffTo,   &QDateEdit::dateChanged, this, &DateWidget::calcDiff);

    calcDiff();
    return w;
}

void DateWidget::calcDiff() {
    QDate from = m_diffFrom->date();
    QDate to   = m_diffTo->date();

    if (from > to) {
        m_diffResult->setText("⚠ 'From' date is after 'To' date");
        return;
    }

    int totalDays = from.daysTo(to);
    int weeks     = totalDays / 7;
    int remDays   = totalDays % 7;

    // Calculate years/months/days properly
    int years  = 0, months = 0, days = 0;
    QDate cursor = from;
    years = cursor.year();
    // count full years
    while (cursor.addYears(1) <= to) { cursor = cursor.addYears(1); years++; }
    years = from.year(); // reset — compute properly
    cursor = from;
    while (true) {
        QDate next = cursor.addYears(1);
        if (next > to) break;
        cursor = next; years++;
    }
    years = cursor.year() - from.year();
    // Adjust if we haven't reached the anniversary month/day yet
    if (QDate(cursor.year(), to.month(), to.day()) < cursor)
        years--;
    cursor = from.addYears(years);

    while (cursor.addMonths(1) <= to) { cursor = cursor.addMonths(1); months++; }
    days = cursor.daysTo(to);

    m_diffResult->setText(
        QString("%1 years, %2 months, %3 days\n(%4 total days · %5 weeks + %6 days)")
            .arg(years).arg(months).arg(days)
            .arg(totalDays).arg(weeks).arg(remDays)
    );
}

// ── Tab 2: Add / Subtract ─────────────────────────────────────────────────────

QWidget* DateWidget::buildAddSubTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);

    auto* c = card(w);
    auto* form = new QFormLayout(c);
    form->setContentsMargins(12, 12, 12, 12);
    form->setSpacing(8);

    m_addBaseDate = dateEdit(w);
    m_addYears    = new QSpinBox(w); m_addYears->setRange(0, 9999);
    m_addMonths   = new QSpinBox(w); m_addMonths->setRange(0, 999);
    m_addDays     = new QSpinBox(w); m_addDays->setRange(0, 99999);

    form->addRow("Base date:", m_addBaseDate);
    form->addRow("Years:",     m_addYears);
    form->addRow("Months:",    m_addMonths);
    form->addRow("Days:",      m_addDays);
    v->addWidget(c);

    auto* btnRow = new QHBoxLayout();
    auto* addBtn = calcBtn("+ Add", w);
    auto* subBtn = calcBtn("− Subtract", w);
    subBtn->setProperty("class", "clearButton");
    btnRow->addWidget(addBtn);
    btnRow->addWidget(subBtn);
    v->addLayout(btnRow);

    m_addResult = resultLabel(w);
    v->addWidget(m_addResult);
    v->addStretch();

    connect(addBtn, &QPushButton::clicked, this, [this]{ calcAddSub(+1); });
    connect(subBtn, &QPushButton::clicked, this, [this]{ calcAddSub(-1); });

    return w;
}

void DateWidget::calcAddSub(int sign) {
    QDate base = m_addBaseDate->date();
    QDate result = base
        .addYears (sign * m_addYears->value())
        .addMonths(sign * m_addMonths->value())
        .addDays  (sign * m_addDays->value());

    QString op = (sign > 0) ? "+" : "−";
    m_addResult->setText(
        QString("%1 %2 %3y %4m %5d  =  %6")
            .arg(base.toString("dd MMM yyyy"))
            .arg(op)
            .arg(m_addYears->value())
            .arg(m_addMonths->value())
            .arg(m_addDays->value())
            .arg(result.toString("dddd, dd MMM yyyy"))
    );
}

// ── Tab 3: Age Calculator ─────────────────────────────────────────────────────

QWidget* DateWidget::buildAgeTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);

    auto* c = card(w);
    auto* form = new QFormLayout(c);
    form->setContentsMargins(12, 12, 12, 12);
    form->setSpacing(8);

    m_ageBirth = dateEdit(w, QDate::currentDate().addYears(-25));
    m_ageAsOf  = dateEdit(w);
    form->addRow("Date of birth:", m_ageBirth);
    form->addRow("Age as of:",     m_ageAsOf);
    v->addWidget(c);

    auto* btn = calcBtn("Calculate Age", w);
    v->addWidget(btn);

    m_ageResult = resultLabel(w);
    v->addWidget(m_ageResult);
    v->addStretch();

    connect(btn,         &QPushButton::clicked,    this, &DateWidget::calcAge);
    connect(m_ageBirth,  &QDateEdit::dateChanged,  this, &DateWidget::calcAge);
    connect(m_ageAsOf,   &QDateEdit::dateChanged,  this, &DateWidget::calcAge);

    calcAge();
    return w;
}

void DateWidget::calcAge() {
    QDate birth = m_ageBirth->date();
    QDate asOf  = m_ageAsOf->date();

    if (birth > asOf) {
        m_ageResult->setText("⚠ Birth date is in the future");
        return;
    }

    int years  = asOf.year()  - birth.year();
    int months = asOf.month() - birth.month();
    int days   = asOf.day()   - birth.day();

    if (days < 0) {
        months--;
        days += QDate(asOf.year(), asOf.month(), 1).addDays(-1).day();
    }
    if (months < 0) { years--; months += 12; }

    int totalDays = birth.daysTo(asOf);

    // Next birthday
    QDate nextBday(asOf.year(), birth.month(), birth.day());
    if (nextBday <= asOf) nextBday = nextBday.addYears(1);
    int daysToNext = asOf.daysTo(nextBday);

    m_ageResult->setText(
        QString("%1 years, %2 months, %3 days\n"
                "(%4 total days)\n"
                "Next birthday in %5 days  (%6)")
            .arg(years).arg(months).arg(days)
            .arg(totalDays)
            .arg(daysToNext)
            .arg(nextBday.toString("dd MMM yyyy"))
    );
}

// ── Tab 4: Day of Week ────────────────────────────────────────────────────────

QWidget* DateWidget::buildDayOfWeekTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);

    auto* c = card(w);
    auto* form = new QFormLayout(c);
    form->setContentsMargins(12, 12, 12, 12);
    form->setSpacing(8);

    m_dowDate = dateEdit(w);
    form->addRow("Date:", m_dowDate);
    v->addWidget(c);

    m_dowResult = resultLabel(w);
    m_dowResult->setStyleSheet("font-size:22px; font-weight:bold; padding:16px;");
    v->addWidget(m_dowResult);
    v->addStretch();

    connect(m_dowDate, &QDateEdit::dateChanged, this, &DateWidget::calcDayOfWeek);
    calcDayOfWeek();
    return w;
}

void DateWidget::calcDayOfWeek() {
    QDate d = m_dowDate->date();
    QString dayName = d.toString("dddd");
    QString info;

    // Is it a weekend?
    int dow = d.dayOfWeek(); // 1=Mon … 7=Sun
    bool weekend = (dow == 6 || dow == 7);

    // Week number
    int week = d.weekNumber();

    // Days until next Monday
    int daysToMonday = (8 - dow) % 7;
    if (daysToMonday == 0) daysToMonday = 7;

    info = QString("%1\n%2\nWeek %3 of %4\n%5")
        .arg(dayName)
        .arg(d.toString("dd MMMM yyyy"))
        .arg(week)
        .arg(d.year())
        .arg(weekend ? "Weekend 🎉" : QString("Weekday · %1 days to next Monday").arg(daysToMonday));

    m_dowResult->setText(info);
}
