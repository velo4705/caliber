#include "signalprocessing_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QDoubleValidator>
#include <cmath>

static const double PI = M_PI;

static QPushButton* mkBtn(const QString& t, QWidget* p) {
    auto* b = new QPushButton(t, p); b->setProperty("class","actionButton");
    b->setMinimumHeight(36); b->setFocusPolicy(Qt::NoFocus); return b;
}
static QLineEdit* numEdit(const QString& def, const QString& ph, QWidget* p) {
    auto* e = new QLineEdit(def); e->setPlaceholderText(ph);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,p));
    e->setFixedWidth(110); return e;
}
static QLabel* resultLbl(QWidget* p) {
    auto* l = new QLabel("—", p); l->setWordWrap(true);
    l->setStyleSheet("font-size:13px;font-weight:bold;padding:8px;"); return l;
}
static QTextEdit* stepsBox(QWidget* p) {
    auto* t = new QTextEdit(p); t->setReadOnly(true);
    t->setMinimumHeight(100); t->setMaximumHeight(200);
    t->setStyleSheet("font-family:monospace;font-size:12px;"); t->hide(); return t;
}
static QLabel* descLbl(const QString& text, QWidget* p) {
    auto* l = new QLabel(text, p); l->setWordWrap(true);
    l->setStyleSheet("color:gray;font-size:12px;padding:4px 0;"); return l;
}

void SignalProcessingWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

SignalProcessingWidget::SignalProcessingWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Signal Processing", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildDftTab(),         "DFT");
    tabs->addTab(buildFilterTab(),      "Filter Design");
    tabs->addTab(buildSamplingTab(),    "Sampling");
    tabs->addTab(buildConvolutionTab(), "Convolution");
    tabs->addTab(buildTransferTab(),    "Transfer Function");
    root->addWidget(tabs);
    setLayout(root);
}

// ── DFT ───────────────────────────────────────────────────────────────────────
QWidget* SignalProcessingWidget::buildDftTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes the Discrete Fourier Transform (DFT) of a signal. Enter comma-separated sample values. Shows magnitude and phase for each frequency bin.", w));
    v->addWidget(new QLabel("Signal samples (comma-separated):", w));
    m_dftInput=new QTextEdit(w); m_dftInput->setPlaceholderText("e.g. 1, 2, 3, 4, 3, 2, 1, 0");
    m_dftInput->setMaximumHeight(60);
    v->addWidget(m_dftInput);
    m_dftResult=resultLbl(w); m_dftShow=new QCheckBox("Show Steps",w); m_dftSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute DFT",w)); v->addWidget(m_dftResult);
    v->addWidget(m_dftShow); v->addWidget(m_dftSteps); v->addStretch();
    connect(m_dftShow,&QCheckBox::toggled,this,[this](bool on){m_dftSteps->setVisible(on&&!m_dftSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&SignalProcessingWidget::computeDft);
    return w;
}

void SignalProcessingWidget::computeDft() {
    QVector<double> x;
    for(const QString& p:m_dftInput->toPlainText().split(',',Qt::SkipEmptyParts)){
        bool ok; double v=p.trimmed().toDouble(&ok); if(ok) x<<v;
    }
    if(x.isEmpty()){m_dftResult->setText("Enter signal samples.");return;}
    int N=x.size();
    QStringList steps;
    steps<<QString("N=%1 samples").arg(N);
    steps<<"DFT: X[k] = Σ x[n]·e^(−j2πkn/N)";
    QString out=QString("DFT of %1 samples:\n\n").arg(N);
    out+=QString("%-6s  %-12s  %-12s  %-10s\n").arg("k").arg("Real").arg("Imag").arg("|X[k]|");
    out+=QString(50,'-')+"\n";
    for(int k=0;k<N;k++){
        double re=0,im=0;
        for(int n=0;n<N;n++){
            double angle=2*PI*k*n/N;
            re+=x[n]*std::cos(angle);
            im-=x[n]*std::sin(angle);
        }
        double mag=std::sqrt(re*re+im*im);
        double phase=std::atan2(im,re)*180/PI;
        out+=QString("%-6s  %-12s  %-12s  %-10s\n")
            .arg(k).arg(re,0,'f',4).arg(im,0,'f',4).arg(mag,0,'f',4);
        if(k<4) steps<<QString("X[%1]: re=%2, im=%3, |X|=%4, φ=%5°").arg(k).arg(re,0,'g',4).arg(im,0,'g',4).arg(mag,0,'g',4).arg(phase,0,'f',2);
    }
    m_dftResult->setText(out.trimmed());
    showSteps(m_dftSteps,m_dftShow,steps);
}

// ── Filter Design ─────────────────────────────────────────────────────────────
QWidget* SignalProcessingWidget::buildFilterTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes cutoff frequency, normalized cutoff, and RC values for first-order filters. For band-pass/stop, enter both fc1 and fc2.", w));
    auto* row=new QHBoxLayout();
    m_filtType=new QComboBox(w);
    m_filtType->addItems({"Low-pass","High-pass","Band-pass","Band-stop"});
    row->addWidget(new QLabel("Type:",w)); row->addWidget(m_filtType); row->addStretch();
    v->addLayout(row);
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_filtFs=numEdit("44100","Fs (Hz)",w); m_filtFc=numEdit("1000","fc (Hz)",w); m_filtFc2=numEdit("5000","fc2 (Hz)",w);
    g->addWidget(new QLabel("Sample rate Fs:",w),0,0); g->addWidget(m_filtFs,0,1);
    g->addWidget(new QLabel("Cutoff fc (Hz):",w),1,0); g->addWidget(m_filtFc,1,1);
    g->addWidget(new QLabel("Cutoff fc2 (Hz):",w),2,0); g->addWidget(m_filtFc2,2,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_filtResult=resultLbl(w); m_filtShow=new QCheckBox("Show Steps",w); m_filtSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_filtResult);
    v->addWidget(m_filtShow); v->addWidget(m_filtSteps); v->addStretch();
    connect(m_filtShow,&QCheckBox::toggled,this,[this](bool on){m_filtSteps->setVisible(on&&!m_filtSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&SignalProcessingWidget::computeFilter);
    return w;
}

void SignalProcessingWidget::computeFilter() {
    double Fs=m_filtFs->text().toDouble(), fc=m_filtFc->text().toDouble(), fc2=m_filtFc2->text().toDouble();
    QStringList steps;
    double wc=2*PI*fc, wc2=2*PI*fc2;
    double Wn=fc/(Fs/2); // normalized 0-1
    steps<<QString("Fs=%1 Hz, fc=%2 Hz, fc2=%3 Hz").arg(Fs).arg(fc).arg(fc2);
    steps<<QString("Angular cutoff ωc = 2πfc = %1 rad/s").arg(wc,0,'g',6);
    steps<<QString("Normalized cutoff Wn = fc/(Fs/2) = %1").arg(Wn,0,'g',4);
    // RC for first-order
    double RC=1.0/(wc), C=1e-6, R=RC/C;
    steps<<QString("RC = 1/ωc = %1 s").arg(RC,0,'g',6);
    steps<<QString("For C=1μF: R = RC/C = %1 Ω").arg(R,0,'g',6);
    QString out=QString("Filter: %1\nfc = %2 Hz\nωc = %3 rad/s\nNormalized Wn = %4\nRC = %5 s\nFor C=1μF: R = %6 Ω")
        .arg(m_filtType->currentText()).arg(fc,0,'g',8).arg(wc,0,'g',8).arg(Wn,0,'g',4).arg(RC,0,'g',6).arg(R,0,'g',6);
    if(m_filtType->currentIndex()>=2){
        double BW=fc2-fc, fc_center=std::sqrt(fc*fc2);
        steps<<QString("Bandwidth BW = fc2−fc = %1 Hz").arg(BW,0,'g',6);
        steps<<QString("Center frequency = √(fc×fc2) = %1 Hz").arg(fc_center,0,'g',6);
        out+=QString("\nBandwidth = %1 Hz\nCenter freq = %2 Hz").arg(BW,0,'g',6).arg(fc_center,0,'g',6);
    }
    m_filtResult->setText(out);
    showSteps(m_filtSteps,m_filtShow,steps);
}

// ── Sampling ──────────────────────────────────────────────────────────────────
QWidget* SignalProcessingWidget::buildSamplingTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Nyquist-Shannon sampling theorem: Fs ≥ 2×fmax to avoid aliasing. Enter the maximum signal frequency and sampling rate to check for aliasing.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_sampFmax=numEdit("1000","fmax (Hz)",w); m_sampFs=numEdit("8000","Fs (Hz)",w);
    g->addWidget(new QLabel("Max freq fmax:",w),0,0); g->addWidget(m_sampFmax,0,1);
    g->addWidget(new QLabel("Sample rate Fs:",w),1,0); g->addWidget(m_sampFs,1,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_sampResult=resultLbl(w); m_sampShow=new QCheckBox("Show Steps",w); m_sampSteps=stepsBox(w);
    v->addWidget(mkBtn("Check",w)); v->addWidget(m_sampResult);
    v->addWidget(m_sampShow); v->addWidget(m_sampSteps); v->addStretch();
    connect(m_sampShow,&QCheckBox::toggled,this,[this](bool on){m_sampSteps->setVisible(on&&!m_sampSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&SignalProcessingWidget::computeSampling);
    return w;
}

void SignalProcessingWidget::computeSampling() {
    double fmax=m_sampFmax->text().toDouble(), Fs=m_sampFs->text().toDouble();
    QStringList steps;
    double nyquist=2*fmax;
    steps<<QString("Nyquist rate = 2×fmax = 2×%1 = %2 Hz").arg(fmax).arg(nyquist);
    steps<<QString("Sampling rate Fs = %1 Hz").arg(Fs);
    bool aliasing=Fs<nyquist;
    steps<<(aliasing?QString("Fs < Nyquist rate → ALIASING will occur!"):QString("Fs ≥ Nyquist rate → No aliasing"));
    double fAlias=aliasing?std::abs(fmax-Fs):0;
    if(aliasing) steps<<QString("Alias frequency = |fmax − Fs| = %1 Hz").arg(fAlias,0,'g',6);
    double Ts=1.0/Fs;
    steps<<QString("Sampling period Ts = 1/Fs = %1 s").arg(Ts,0,'g',6);
    m_sampResult->setText(QString("Nyquist rate = %1 Hz\nFs = %2 Hz\n%3%4\nTs = %5 s")
        .arg(nyquist,0,'g',8).arg(Fs,0,'g',8)
        .arg(aliasing?"⚠ ALIASING DETECTED":"✓ No aliasing")
        .arg(aliasing?QString("\nAlias freq = %1 Hz").arg(fAlias,0,'g',6):"")
        .arg(Ts,0,'g',8));
    showSteps(m_sampSteps,m_sampShow,steps);
}

// ── Convolution ───────────────────────────────────────────────────────────────
QWidget* SignalProcessingWidget::buildConvolutionTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Discrete linear convolution of two sequences x[n] and h[n]. Output length = len(x)+len(h)−1. Enter comma-separated values.", w));
    v->addWidget(new QLabel("x[n]:", w));
    m_convX=new QTextEdit(w); m_convX->setPlaceholderText("e.g. 1, 2, 3"); m_convX->setMaximumHeight(50);
    v->addWidget(m_convX);
    v->addWidget(new QLabel("h[n]:", w));
    m_convH=new QTextEdit(w); m_convH->setPlaceholderText("e.g. 1, 1, 1"); m_convH->setMaximumHeight(50);
    v->addWidget(m_convH);
    m_convResult=resultLbl(w); m_convShow=new QCheckBox("Show Steps",w); m_convSteps=stepsBox(w);
    v->addWidget(mkBtn("Convolve",w)); v->addWidget(m_convResult);
    v->addWidget(m_convShow); v->addWidget(m_convSteps); v->addStretch();
    connect(m_convShow,&QCheckBox::toggled,this,[this](bool on){m_convSteps->setVisible(on&&!m_convSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&SignalProcessingWidget::computeConvolution);
    return w;
}

void SignalProcessingWidget::computeConvolution() {
    auto parse=[](const QString& s)->QVector<double>{
        QVector<double> v;
        for(const QString& p:s.split(',',Qt::SkipEmptyParts)){bool ok;double d=p.trimmed().toDouble(&ok);if(ok)v<<d;}
        return v;
    };
    QVector<double> x=parse(m_convX->toPlainText()), h=parse(m_convH->toPlainText());
    if(x.isEmpty()||h.isEmpty()){m_convResult->setText("Enter both sequences.");return;}
    int Nx=x.size(), Nh=h.size(), Ny=Nx+Nh-1;
    QVector<double> y(Ny,0);
    QStringList steps;
    steps<<QString("x[n]: length=%1, h[n]: length=%2").arg(Nx).arg(Nh);
    steps<<QString("Output y[n] = x[n]*h[n], length=%1").arg(Ny);
    steps<<"y[n] = Σ x[k]·h[n−k]";
    for(int n=0;n<Ny;n++){
        for(int k=0;k<Nx;k++) if(n-k>=0&&n-k<Nh) y[n]+=x[k]*h[n-k];
        if(n<6) steps<<QString("y[%1] = %2").arg(n).arg(y[n],0,'g',6);
    }
    QStringList ys; for(double v:y) ys<<QString::number(v,'g',6);
    m_convResult->setText(QString("y[n] = [%1]").arg(ys.join(", ")));
    showSteps(m_convSteps,m_convShow,steps);
}

// ── Transfer Function ─────────────────────────────────────────────────────────
QWidget* SignalProcessingWidget::buildTransferTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Analyzes a transfer function H(s) = N(s)/D(s). Enter numerator and denominator polynomial coefficients (highest power first). Finds poles, zeros, DC gain, and stability.", w));
    v->addWidget(new QLabel("Numerator coefficients (highest power first):", w));
    m_tfNum=new QTextEdit(w); m_tfNum->setPlaceholderText("e.g. 1, 2 → s+2"); m_tfNum->setMaximumHeight(50);
    v->addWidget(m_tfNum);
    v->addWidget(new QLabel("Denominator coefficients:", w));
    m_tfDen=new QTextEdit(w); m_tfDen->setPlaceholderText("e.g. 1, 3, 2 → s²+3s+2"); m_tfDen->setMaximumHeight(50);
    v->addWidget(m_tfDen);
    m_tfResult=resultLbl(w); m_tfShow=new QCheckBox("Show Steps",w); m_tfSteps=stepsBox(w);
    v->addWidget(mkBtn("Analyze",w)); v->addWidget(m_tfResult);
    v->addWidget(m_tfShow); v->addWidget(m_tfSteps); v->addStretch();
    connect(m_tfShow,&QCheckBox::toggled,this,[this](bool on){m_tfSteps->setVisible(on&&!m_tfSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&SignalProcessingWidget::computeTransfer);
    return w;
}

void SignalProcessingWidget::computeTransfer() {
    auto parse=[](const QString& s)->QVector<double>{
        QVector<double> v;
        for(const QString& p:s.split(',',Qt::SkipEmptyParts)){bool ok;double d=p.trimmed().toDouble(&ok);if(ok)v<<d;}
        return v;
    };
    QVector<double> num=parse(m_tfNum->toPlainText()), den=parse(m_tfDen->toPlainText());
    if(num.isEmpty()||den.isEmpty()){m_tfResult->setText("Enter coefficients.");return;}
    QStringList steps;
    // DC gain: H(0) = num(0)/den(0) = last coeff / last coeff
    double dcGain=den.last()!=0?num.last()/den.last():0;
    steps<<QString("DC gain H(0) = N(0)/D(0) = %1/%2 = %3").arg(num.last()).arg(den.last()).arg(dcGain,0,'g',6);
    // Zeros: roots of numerator (for 1st/2nd order)
    QString zeros="", poles="";
    if(num.size()==2){ double z=-num.last()/num.first(); zeros=QString("s = %1").arg(z,0,'g',6); steps<<QString("Zero: s=%1").arg(z,0,'g',6); }
    else if(num.size()==3){
        double a=num[0],b=num[1],c=num[2],disc=b*b-4*a*c;
        if(disc>=0){ zeros=QString("s=%1, s=%2").arg((-b+std::sqrt(disc))/(2*a),0,'g',4).arg((-b-std::sqrt(disc))/(2*a),0,'g',4); }
        else{ zeros=QString("s=%1±%2j").arg(-b/(2*a),0,'g',4).arg(std::sqrt(-disc)/(2*a),0,'g',4); }
    } else zeros="(higher order)";
    if(den.size()==2){ double p=-den.last()/den.first(); poles=QString("s = %1").arg(p,0,'g',6); steps<<QString("Pole: s=%1").arg(p,0,'g',6); }
    else if(den.size()==3){
        double a=den[0],b=den[1],c=den[2],disc=b*b-4*a*c;
        if(disc>=0){ poles=QString("s=%1, s=%2").arg((-b+std::sqrt(disc))/(2*a),0,'g',4).arg((-b-std::sqrt(disc))/(2*a),0,'g',4); }
        else{ poles=QString("s=%1±%2j").arg(-b/(2*a),0,'g',4).arg(std::sqrt(-disc)/(2*a),0,'g',4); }
    } else poles="(higher order)";
    // Stability: all poles must have negative real part
    bool stable=true;
    if(den.size()==2){ if(-den.last()/den.first()>=0) stable=false; }
    else if(den.size()==3){ double a=den[0],b=den[1],c=den[2]; if(a*b<=0||b*c<=0||a*c<=0) stable=false; }
    steps<<(stable?"All poles in left half-plane → BIBO stable":"Pole(s) in right half-plane → UNSTABLE");
    m_tfResult->setText(QString("DC Gain = %1\nZeros: %2\nPoles: %3\nStability: %4")
        .arg(dcGain,0,'g',6).arg(zeros).arg(poles).arg(stable?"Stable (BIBO)":"Unstable"));
    showSteps(m_tfSteps,m_tfShow,steps);
}
