#pragma once
#include <QWidget>
#include <QPropertyAnimation>

class QLineEdit;
class QListWidget;
class QListWidgetItem;

struct FormulaEntry {
    QString topic;    // e.g. "Quadratic Formula"
    QString mode;     // e.g. "Equations"
    QString formula;  // e.g. "x = (−b ± √(b²−4ac)) / 2a"
    QString desc;     // short description
};

class FormulaPanel : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int drawerX READ drawerX WRITE setDrawerX)
public:
    explicit FormulaPanel(QWidget* parent = nullptr);

    void repositionToParent();
    void toggleDrawer();
    bool isDrawerOpen() const { return m_open; }

private:
    int  drawerX() const;
    void setDrawerX(int x);
    void populateFormulas();
    void filterFormulas(const QString& query);

    QLineEdit*          m_search;
    QListWidget*        m_list;
    QPropertyAnimation* m_anim;
    bool                m_open = false;

    QList<FormulaEntry> m_formulas;

    static constexpr int PANEL_WIDTH = 340;
};
