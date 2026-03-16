#pragma once
#include <QWidget>

class QDateEdit;
class QLabel;
class QTabWidget;
class QSpinBox;
class QComboBox;

// Date Calculation mode:
//  Tab 1 — Date Difference (between two dates)
//  Tab 2 — Add / Subtract duration from a date
//  Tab 3 — Age calculator
//  Tab 4 — Day of week finder
class DateWidget : public QWidget {
    Q_OBJECT
public:
    explicit DateWidget(QWidget* parent = nullptr);

private:
    // Tab 1
    QWidget* buildDiffTab();
    QDateEdit* m_diffFrom;
    QDateEdit* m_diffTo;
    QLabel*    m_diffResult;

    // Tab 2
    QWidget* buildAddSubTab();
    QDateEdit* m_addBaseDate;
    QSpinBox*  m_addYears;
    QSpinBox*  m_addMonths;
    QSpinBox*  m_addDays;
    QLabel*    m_addResult;

    // Tab 3
    QWidget* buildAgeTab();
    QDateEdit* m_ageBirth;
    QDateEdit* m_ageAsOf;
    QLabel*    m_ageResult;

    // Tab 4
    QWidget* buildDayOfWeekTab();
    QDateEdit* m_dowDate;
    QLabel*    m_dowResult;

    void calcDiff();
    void calcAddSub(int sign);
    void calcAge();
    void calcDayOfWeek();
};
