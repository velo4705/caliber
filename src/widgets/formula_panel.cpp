#include "formula_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFrame>
#include <QPropertyAnimation>
#include <QEasingCurve>

FormulaPanel::FormulaPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("historyPanel"); // reuse same QSS styling as history panel
    setFixedWidth(PANEL_WIDTH);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 14, 10, 10);
    layout->setSpacing(6);

    // Header
    auto* title = new QLabel("Formula Book", this);
    title->setObjectName("historyTitle");
    layout->addWidget(title);

    // Search bar
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Search formulas...");
    m_search->setClearButtonEnabled(true);
    layout->addWidget(m_search);

    // Divider
    auto* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    // Formula list
    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setWordWrap(true);
    m_list->setSpacing(2);
    layout->addWidget(m_list, 1);

    setLayout(layout);

    m_anim = new QPropertyAnimation(this, "drawerX", this);
    m_anim->setDuration(220);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);

    hide();

    populateFormulas();
    filterFormulas("");

    connect(m_search, &QLineEdit::textChanged, this, &FormulaPanel::filterFormulas);
}

int  FormulaPanel::drawerX() const { return x(); }
void FormulaPanel::setDrawerX(int x) { move(x, 0); }

void FormulaPanel::repositionToParent() {
    if (!parentWidget()) return;
    int ph = parentWidget()->height();
    int pw = parentWidget()->width();
    setFixedHeight(ph);
    setFixedWidth(qMin(PANEL_WIDTH, pw));
    int openX = pw - qMin(PANEL_WIDTH, pw);
    move(m_open ? openX : pw, 0);
}

void FormulaPanel::toggleDrawer() {
    if (!parentWidget()) return;
    int pw = parentWidget()->width();
    int panelW = qMin(PANEL_WIDTH, pw);
    setFixedHeight(parentWidget()->height());
    setFixedWidth(panelW);
    int openX = pw - panelW, closedX = pw;
    m_anim->stop();
    if (!m_open) {
        move(closedX, 0); show(); raise();
        m_anim->setStartValue(closedX); m_anim->setEndValue(openX);
        m_open = true;
    } else {
        m_anim->setStartValue(openX); m_anim->setEndValue(closedX);
        m_open = false;
        connect(m_anim, &QPropertyAnimation::finished, this, [this]{
            if (!m_open) hide();
            disconnect(m_anim, &QPropertyAnimation::finished, this, nullptr);
        });
    }
    m_anim->start();
}

void FormulaPanel::filterFormulas(const QString& query) {
    m_list->clear();
    QString q = query.trimmed().toLower();
    for (const FormulaEntry& e : m_formulas) {
        // Mode filter
        if (!m_modeFilter.isEmpty() && e.mode.toLower() != m_modeFilter.toLower())
            continue;
        // Text filter
        bool match = q.isEmpty()
            || e.topic.toLower().contains(q)
            || e.formula.toLower().contains(q)
            || e.mode.toLower().contains(q)
            || e.desc.toLower().contains(q);
        if (!match) continue;

        auto* item = new QListWidgetItem(m_list);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        QString text = QString("【%1】 %2\n%3\n%4")
            .arg(e.mode).arg(e.topic).arg(e.formula).arg(e.desc);
        item->setText(text);
        item->setToolTip(e.formula);
        item->setForeground(QColor(0x42, 0x9e, 0xf5));
        m_list->addItem(item);
    }
}

void FormulaPanel::setFilterMode(const QString& mode) {
    m_modeFilter = mode;
    filterFormulas(m_search->text());
}

void FormulaPanel::populateFormulas() {
    // ── Basic ─────────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Percentage",        "Basic",      "x% of y = (x/100)×y",                          "Find x percent of y"};
    m_formulas << FormulaEntry{"Percentage Change",  "Basic",      "Δ% = (new−old)/old × 100",                     "Relative change between two values"};
    m_formulas << FormulaEntry{"Order of Operations","Basic",      "BODMAS: Brackets, Orders, ÷×, +−",             "Evaluation precedence"};

    // ── Scientific ────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Quadratic Formula",  "Scientific", "x = (−b ± √(b²−4ac)) / 2a",                   "Roots of ax²+bx+c=0"};
    m_formulas << FormulaEntry{"Pythagorean Theorem","Scientific", "a² + b² = c²",                                 "Right triangle sides"};
    m_formulas << FormulaEntry{"Euler's Number",     "Scientific", "e ≈ 2.71828...",                               "Base of natural logarithm"};
    m_formulas << FormulaEntry{"Log Laws",           "Scientific", "log(ab)=log a+log b, log(a/b)=log a−log b",    "Logarithm product/quotient rules"};
    m_formulas << FormulaEntry{"Change of Base",     "Scientific", "log_b(x) = ln(x)/ln(b)",                       "Convert between log bases"};
    m_formulas << FormulaEntry{"Trig Identities",    "Scientific", "sin²θ+cos²θ=1, tan θ=sin θ/cos θ",            "Fundamental trig identities"};
    m_formulas << FormulaEntry{"Double Angle",       "Scientific", "sin(2θ)=2sinθcosθ, cos(2θ)=cos²θ−sin²θ",      "Double angle formulas"};
    m_formulas << FormulaEntry{"Euler's Formula",    "Scientific", "e^(iθ) = cos θ + i sin θ",                     "Complex exponential"};

    // ── Equations ─────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Linear Equation",    "Equations",  "ax + b = 0  →  x = −b/a",                     "One-variable linear"};
    m_formulas << FormulaEntry{"Cramer's Rule 2×2",  "Equations",  "x=(C·E−B·F)/det, y=(A·F−C·D)/det",            "2-variable system solution"};
    m_formulas << FormulaEntry{"Determinant 2×2",    "Equations",  "det(A) = ad − bc",                             "2×2 matrix determinant"};
    m_formulas << FormulaEntry{"Matrix Inverse 2×2", "Equations",  "A⁻¹ = (1/det)·[[d,−b],[−c,a]]",               "Inverse of 2×2 matrix"};
    m_formulas << FormulaEntry{"Gaussian Elimination","Equations", "Row reduce [A|b] to upper triangular, back-sub","System of equations solver"};

    // ── Calculus ──────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Derivative (central)","Calculus",  "f'(x) ≈ [f(x+h)−f(x−h)] / 2h",               "Numerical differentiation"};
    m_formulas << FormulaEntry{"Simpson's Rule",      "Calculus",  "∫f dx ≈ h/3·[f(a)+4f(x₁)+2f(x₂)+...+f(b)]",  "Numerical integration"};
    m_formulas << FormulaEntry{"Taylor Series",       "Calculus",  "f(x) = Σ f⁽ⁿ⁾(a)/n! · (x−a)ⁿ",               "Function approximation around a"};
    m_formulas << FormulaEntry{"Chain Rule",          "Calculus",  "d/dx[f(g(x))] = f'(g(x))·g'(x)",              "Derivative of composite function"};
    m_formulas << FormulaEntry{"Product Rule",        "Calculus",  "d/dx[uv] = u'v + uv'",                         "Derivative of product"};
    m_formulas << FormulaEntry{"Quotient Rule",       "Calculus",  "d/dx[u/v] = (u'v − uv') / v²",                 "Derivative of quotient"};
    m_formulas << FormulaEntry{"Integration by Parts","Calculus",  "∫u dv = uv − ∫v du",                           "Integration technique"};
    m_formulas << FormulaEntry{"Partial Derivative",  "Calculus",  "∂f/∂x ≈ [f(x+h,y)−f(x−h,y)] / 2h",           "Multivariable differentiation"};
    m_formulas << FormulaEntry{"L'Hôpital's Rule",    "Calculus",  "lim f/g = lim f'/g' (0/0 or ∞/∞ form)",       "Evaluate indeterminate limits"};

    // ── Statistics ────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Mean",               "Statistics", "x̄ = Σxᵢ / n",                                  "Arithmetic average"};
    m_formulas << FormulaEntry{"Variance (sample)",  "Statistics", "s² = Σ(xᵢ−x̄)² / (n−1)",                       "Sample variance"};
    m_formulas << FormulaEntry{"Std Deviation",      "Statistics", "s = √(Σ(xᵢ−x̄)²/(n−1))",                       "Spread of data"};
    m_formulas << FormulaEntry{"Z-score",            "Statistics", "z = (x − μ) / σ",                              "Standardize a value"};
    m_formulas << FormulaEntry{"Normal PDF",         "Statistics", "f(x) = (1/σ√2π)·e^(−(x−μ)²/2σ²)",             "Normal distribution density"};
    m_formulas << FormulaEntry{"Confidence Interval","Statistics", "CI = x̄ ± z*(σ/√n)",                            "Interval estimate for mean"};
    m_formulas << FormulaEntry{"Linear Regression",  "Statistics", "ŷ = bx + a,  b = Σ(x−x̄)(y−ȳ)/Σ(x−x̄)²",       "Best-fit line"};
    m_formulas << FormulaEntry{"Pearson r",          "Statistics", "r = Σ(x−x̄)(y−ȳ) / √[Σ(x−x̄)²·Σ(y−ȳ)²]",      "Linear correlation coefficient"};
    m_formulas << FormulaEntry{"Chi-Square",         "Statistics", "χ² = Σ(O−E)²/E",                               "Goodness-of-fit test"};
    m_formulas << FormulaEntry{"Bayes' Theorem",     "Statistics", "P(A|B) = P(B|A)·P(A) / P(B)",                  "Conditional probability"};

    // ── Vectors ───────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Dot Product",        "Vectors",    "A·B = AxBx+AyBy+AzBz = |A||B|cosθ",            "Scalar product of two vectors"};
    m_formulas << FormulaEntry{"Cross Product",      "Vectors",    "A×B = (AyBz−AzBy, AzBx−AxBz, AxBy−AyBx)",     "Vector perpendicular to A and B"};
    m_formulas << FormulaEntry{"Vector Magnitude",   "Vectors",    "|A| = √(Ax²+Ay²+Az²)",                         "Length of a vector"};
    m_formulas << FormulaEntry{"Unit Vector",        "Vectors",    "Â = A/|A|",                                     "Direction vector, magnitude=1"};
    m_formulas << FormulaEntry{"Projection",         "Vectors",    "proj_B(A) = (A·B/|B|²)·B",                     "Component of A along B"};
    m_formulas << FormulaEntry{"Angle Between",      "Vectors",    "θ = arccos(A·B / |A||B|)",                      "Angle between two vectors"};
    m_formulas << FormulaEntry{"Plane Equation",     "Vectors",    "n·(r−P) = 0  →  ax+by+cz = d",                 "Plane from normal and point"};

    // ── Physics ───────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"SUVAT v=u+at",       "Physics",    "v = u + at",                                    "Final velocity"};
    m_formulas << FormulaEntry{"SUVAT s=ut+½at²",    "Physics",    "s = ut + ½at²",                                 "Displacement"};
    m_formulas << FormulaEntry{"SUVAT v²=u²+2as",    "Physics",    "v² = u² + 2as",                                 "Velocity-displacement"};
    m_formulas << FormulaEntry{"Newton's 2nd Law",   "Physics",    "F = ma",                                        "Force equals mass times acceleration"};
    m_formulas << FormulaEntry{"Kinetic Energy",     "Physics",    "KE = ½mv²",                                     "Energy of motion"};
    m_formulas << FormulaEntry{"Potential Energy",   "Physics",    "PE = mgh",                                      "Gravitational potential energy"};
    m_formulas << FormulaEntry{"Coulomb's Law",      "Physics",    "F = kq₁q₂/r²,  k=8.99×10⁹ N·m²/C²",           "Electrostatic force"};
    m_formulas << FormulaEntry{"Ideal Gas Law",      "Physics",    "PV = nRT,  R=8.314 J/mol·K",                    "Ideal gas equation"};
    m_formulas << FormulaEntry{"Wave Equation",      "Physics",    "v = fλ",                                        "Wave speed, frequency, wavelength"};
    m_formulas << FormulaEntry{"Snell's Law",        "Physics",    "n₁sinθ₁ = n₂sinθ₂",                            "Refraction at interface"};
    m_formulas << FormulaEntry{"Projectile Range",   "Physics",    "R = v₀²sin(2θ)/g",                             "Horizontal range"};
    m_formulas << FormulaEntry{"Centripetal Force",  "Physics",    "Fc = mv²/r",                                    "Force for circular motion"};

    // ── Chemistry ─────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Ideal Gas (Chem)",   "Chemistry",  "PV = nRT",                                      "Pressure, volume, moles, temperature"};
    m_formulas << FormulaEntry{"pH",                 "Chemistry",  "pH = −log[H⁺]",                                 "Acidity measure"};
    m_formulas << FormulaEntry{"pOH",                "Chemistry",  "pOH = −log[OH⁻],  pH+pOH=14",                  "Basicity measure"};
    m_formulas << FormulaEntry{"Dilution",           "Chemistry",  "C₁V₁ = C₂V₂",                                  "Dilution equation"};
    m_formulas << FormulaEntry{"Gibbs Free Energy",  "Chemistry",  "ΔG = ΔH − TΔS",                                 "Spontaneity criterion"};
    m_formulas << FormulaEntry{"Weak Acid [H⁺]",     "Chemistry",  "[H⁺] = √(Ka·C)",                               "Hydrogen ion concentration"};

    // ── Electrical ────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Ohm's Law",          "Electrical", "V = IR,  P = VI = I²R = V²/R",                 "Voltage, current, resistance, power"};
    m_formulas << FormulaEntry{"Series Resistors",   "Electrical", "R_eq = R₁+R₂+R₃+...",                          "Total resistance in series"};
    m_formulas << FormulaEntry{"Parallel Resistors", "Electrical", "1/R_eq = 1/R₁+1/R₂+...",                       "Total resistance in parallel"};
    m_formulas << FormulaEntry{"RC Time Constant",   "Electrical", "τ = RC,  fc = 1/(2πRC)",                        "RC circuit time constant"};
    m_formulas << FormulaEntry{"RL Time Constant",   "Electrical", "τ = L/R,  fc = R/(2πL)",                        "RL circuit time constant"};
    m_formulas << FormulaEntry{"Resonant Frequency", "Electrical", "f₀ = 1/(2π√LC)",                               "RLC resonance"};
    m_formulas << FormulaEntry{"Voltage Divider",    "Electrical", "Vout = Vin × R₂/(R₁+R₂)",                      "Resistive voltage divider"};
    m_formulas << FormulaEntry{"Inverting Op-Amp",   "Electrical", "Gain = −Rf/Rin",                                "Inverting amplifier gain"};
    m_formulas << FormulaEntry{"Non-Inv Op-Amp",     "Electrical", "Gain = 1 + Rf/R₁",                              "Non-inverting amplifier gain"};
    m_formulas << FormulaEntry{"dB Voltage",         "Electrical", "dB = 20·log₁₀(Vout/Vin)",                       "Voltage gain in decibels"};
    m_formulas << FormulaEntry{"Power Factor",       "Electrical", "PF = P/S = cos(φ),  S²=P²+Q²",                 "Real/apparent power ratio"};

    // ── Financial ─────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Compound Interest",  "Financial",  "A = P(1+r/n)^(nt)",                             "Future value with compounding"};
    m_formulas << FormulaEntry{"Continuous Compound","Financial",  "A = Pe^(rt)",                                   "Continuous compounding"};
    m_formulas << FormulaEntry{"Loan Payment",       "Financial",  "M = P·r(1+r)^n / [(1+r)^n−1]",                 "Monthly mortgage/loan payment"};
    m_formulas << FormulaEntry{"NPV",                "Financial",  "NPV = −C₀ + Σ CFₜ/(1+r)^t",                    "Net present value"};
    m_formulas << FormulaEntry{"Rule of 72",         "Financial",  "Years to double ≈ 72/r%",                       "Quick doubling time estimate"};

    // ── Number Theory ─────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"GCD (Euclidean)",    "Number Theory","gcd(a,b) = gcd(b, a mod b)",                  "Greatest common divisor"};
    m_formulas << FormulaEntry{"LCM",                "Number Theory","lcm(a,b) = |a·b| / gcd(a,b)",                 "Least common multiple"};
    m_formulas << FormulaEntry{"Modular Inverse",    "Number Theory","a⁻¹ mod m: extended Euclidean",               "Inverse in modular arithmetic"};
    m_formulas << FormulaEntry{"Fermat's Little",    "Number Theory","a^(p−1) ≡ 1 (mod p), p prime",               "Fermat's little theorem"};

    // ── Advanced Math ─────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Complex Modulus",    "Adv. Math",  "|z| = √(a²+b²)",                               "Magnitude of complex number"};
    m_formulas << FormulaEntry{"Complex Polar",      "Adv. Math",  "z = r·e^(iθ) = r(cosθ+i sinθ)",               "Polar form of complex number"};
    m_formulas << FormulaEntry{"De Moivre's",        "Adv. Math",  "(cosθ+i sinθ)^n = cos(nθ)+i sin(nθ)",          "Power of complex number"};
    m_formulas << FormulaEntry{"Eigenvalue Eq.",     "Adv. Math",  "det(A−λI) = 0",                                 "Characteristic equation"};
    m_formulas << FormulaEntry{"Law of Sines",       "Adv. Math",  "a/sin A = b/sin B = c/sin C",                   "Triangle side-angle relation"};
    m_formulas << FormulaEntry{"Law of Cosines",     "Adv. Math",  "c² = a²+b²−2ab·cos C",                         "Generalized Pythagorean theorem"};
    m_formulas << FormulaEntry{"Geometric Series",   "Adv. Math",  "S∞ = a/(1−r),  |r|<1",                         "Infinite geometric series sum"};

    // ── Discrete Math ─────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Permutations",       "Discrete",   "P(n,r) = n!/(n−r)!",                            "Ordered arrangements"};
    m_formulas << FormulaEntry{"Combinations",       "Discrete",   "C(n,r) = n!/[r!(n−r)!]",                        "Unordered selections"};
    m_formulas << FormulaEntry{"Inclusion-Exclusion","Discrete",   "|A∪B| = |A|+|B|−|A∩B|",                        "Union of two sets"};
    m_formulas << FormulaEntry{"Pigeonhole",         "Discrete",   "n items, k boxes → ≥⌈n/k⌉ in one box",         "Pigeonhole principle"};
    m_formulas << FormulaEntry{"De Morgan's Laws",   "Discrete",   "¬(A∧B)=¬A∨¬B,  ¬(A∨B)=¬A∧¬B",                 "Boolean complement of AND/OR"};

    // ── Civil/Mechanical ──────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Normal Stress",      "Civil/Mech", "σ = F/A",                                       "Stress from axial force"};
    m_formulas << FormulaEntry{"Young's Modulus",    "Civil/Mech", "E = σ/ε = (F/A)/(ΔL/L)",                       "Stiffness of material"};
    m_formulas << FormulaEntry{"Bernoulli's Eq.",    "Civil/Mech", "P+½ρv²+ρgh = const",                           "Energy conservation in fluid"};
    m_formulas << FormulaEntry{"Reynolds Number",    "Civil/Mech", "Re = ρvD/μ",                                    "Laminar vs turbulent flow"};
    m_formulas << FormulaEntry{"Fourier Conduction", "Civil/Mech", "Q = kA(ΔT/L)",                                  "Heat conduction rate"};
    m_formulas << FormulaEntry{"Gear Ratio",         "Civil/Mech", "N₁/N₂ = ω₂/ω₁ = T₂/T₁",                       "Speed and torque ratio"};

    // ── Signal Processing ─────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"DFT",                "Signals",    "X[k] = Σ x[n]·e^(−j2πkn/N)",                   "Discrete Fourier Transform"};
    m_formulas << FormulaEntry{"Nyquist Rate",       "Signals",    "Fs ≥ 2·fmax",                                   "Minimum sampling rate"};
    m_formulas << FormulaEntry{"Convolution",        "Signals",    "y[n] = Σ x[k]·h[n−k]",                         "Discrete linear convolution"};
    m_formulas << FormulaEntry{"RC Low-pass fc",     "Signals",    "fc = 1/(2πRC)",                                  "Cutoff frequency"};

    // ── Control Systems ───────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"PID Controller",     "Control",    "u(t) = Kp·e + Ki·∫e dt + Kd·de/dt",            "PID control law"};
    m_formulas << FormulaEntry{"Closed-Loop TF",     "Control",    "T(s) = G(s)/(1+G(s)H(s))",                     "Closed-loop transfer function"};
    m_formulas << FormulaEntry{"Steady-State Error", "Control",    "e_ss = 1/(1+Kp) for step input",                "Position error constant"};
    m_formulas << FormulaEntry{"2nd Order ωn",       "Control",    "ωn = √(c/a),  ζ = b/(2√(ac))",                 "Natural frequency and damping"};
    m_formulas << FormulaEntry{"Ziegler-Nichols PID","Control",    "Kp=0.6Ku, Ti=Tu/2, Td=Tu/8",                   "PID tuning from oscillation"};

    // ── MCS ───────────────────────────────────────────────────────────────────
    m_formulas << FormulaEntry{"Master Theorem",     "MCS",        "T(n)=aT(n/b)+f(n): compare log_b(a) vs k",     "Recurrence complexity"};
    m_formulas << FormulaEntry{"Big-O Order",        "MCS",        "O(1)<O(log n)<O(n)<O(n log n)<O(n²)<O(2^n)",   "Growth rate hierarchy"};
    m_formulas << FormulaEntry{"Machine Epsilon",    "MCS",        "ε = 2^(−52) ≈ 2.22×10⁻¹⁶ (double)",           "Smallest representable difference"};
}
