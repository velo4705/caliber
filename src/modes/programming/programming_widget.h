#pragma once
#include <QWidget>
#include <QString>
#include <QVector>
#include <QPushButton>
#include <QButtonGroup>

class DisplayWidget;
class HistoryManager;
class HistoryPanel;
class QLabel;
class QButtonGroup;

enum class BitWidth { Bit8 = 8, Bit16 = 16, Bit32 = 32, Bit64 = 64 };
enum class BaseMode { Hex = 16, Dec = 10, Oct = 8, Bin = 2 };

// Programming calculator: integer arithmetic, base conversion, bitwise ops
class ProgrammingWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProgrammingWidget(HistoryManager* history,
                               HistoryPanel* historyPanel,
                               QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onDigitClicked(const QString& digit);
    void onOperatorClicked(const QString& op);
    void onEquals();
    void onClear();
    void onBackspace();
    void onBaseChanged(int base);
    void onBitWidthChanged(int bits);

private:
    void buildUI();
    void updateDisplays();
    void updateButtonStates();
    void applyBitWidth();
    long long applyOperation(long long a, const QString& op, long long b);
    long long parseInput(const QString& text, int base);
    QString  formatInBase(long long value, int base) const;

    // Display labels for each base
    QLabel* m_hexLabel;
    QLabel* m_decLabel;
    QLabel* m_octLabel;
    QLabel* m_binLabel;

    // Bit visualizer (64 toggle-like labels)
    QVector<QLabel*> m_bitLabels;

    QButtonGroup* m_baseGroup;
    QButtonGroup* m_bitWidthGroup;

    QString   m_inputBuffer;   // current input string in active base
    long long m_currentValue = 0;
    long long m_storedValue  = 0;
    QString   m_pendingOp;
    bool      m_resultShown  = false;

    BaseMode  m_base     = BaseMode::Dec;
    BitWidth  m_bitWidth = BitWidth::Bit32;

    HistoryManager* m_history;
    HistoryPanel*   m_historyPanel;

    QVector<QPushButton*> m_digitButtons; // A-F + 0-9 for enabling/disabling
};
