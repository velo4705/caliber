#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;

class VectorsWidget : public QWidget {
    Q_OBJECT
public:
    explicit VectorsWidget(QWidget* parent = nullptr);

private:
    QWidget*   buildArithTab();
    QWidget*   buildDotCrossTab();
    QWidget*   buildMagUnitTab();
    QWidget*   buildAngleProjTab();
    QWidget*   buildLinePlaneTab();
    QWidget*   buildDistanceTab();

    // Vector A and B components (shared)
    QLineEdit* m_ax, *m_ay, *m_az;
    QLineEdit* m_bx, *m_by, *m_bz;

    QLabel*    m_arithResult;
    QTextEdit* m_arithSteps; QCheckBox* m_arithShow;

    QLabel*    m_dcResult;
    QTextEdit* m_dcSteps; QCheckBox* m_dcShow;

    QLabel*    m_magResult;
    QTextEdit* m_magSteps; QCheckBox* m_magShow;

    QLabel*    m_apResult;
    QTextEdit* m_apSteps; QCheckBox* m_apShow;

    QLineEdit* m_lpPx, *m_lpPy, *m_lpPz; // point P
    QLineEdit* m_lpQx, *m_lpQy, *m_lpQz; // point Q (for plane: 3rd point)
    QLabel*    m_lpResult;
    QTextEdit* m_lpSteps; QCheckBox* m_lpShow;

    QLineEdit* m_distPx, *m_distPy, *m_distPz;
    QComboBox* m_distType;
    QLabel*    m_distResult;
    QTextEdit* m_distSteps; QCheckBox* m_distShow;

    void computeArith();
    void computeDotCross();
    void computeMagUnit();
    void computeAngleProj();
    void computeLinePlane();
    void computeDistance();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);

    // helpers
    struct Vec3 { double x,y,z; };
    static Vec3  add(Vec3 a, Vec3 b);
    static Vec3  sub(Vec3 a, Vec3 b);
    static Vec3  scale(Vec3 a, double s);
    static double dot(Vec3 a, Vec3 b);
    static Vec3  cross(Vec3 a, Vec3 b);
    static double mag(Vec3 a);
    static Vec3  unit(Vec3 a);
    static double angle(Vec3 a, Vec3 b);
    Vec3 readA() const;
    Vec3 readB() const;
    QString fmt(Vec3 v) const;
};
