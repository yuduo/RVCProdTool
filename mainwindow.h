#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QMainWindow>
#include <QCloseEvent>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QLineEdit>
#include <QSpacerItem>
#include <QButtonGroup>
#include <QRadioButton>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QRegExp>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVector>
#include <QLibrary>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef enum _OP_MODE_ {
    OP_MODE_MANUAL,
    OP_MODE_AUTO
}OP_MODE;

typedef enum _OP_STEPS_ {
    OP_IDLE,
    OP_STEP0,
    OP_STEP1,
    OP_STEP2,
    OP_STEP3,
    OP_STEP4,
    OP_END
}OP_STEPS;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);

    void initData();
    void initUI();

signals:

public slots:
    void btnAutoCliecked();
    void btnStep0Cliecked();
    void btnStep1Cliecked();
    void btnStep2Cliecked();
    void btnStep3Cliecked();
    void btnStep4Cliecked();
    void btnResetClicked();

    void actionComClicked();
    void actionFactoryClicked();
    void actionFWDevClicked();
    void actionFWVersionClicked();
    void actionCertVersionClicked();
    void actionFilesClicked();
    void actionAboutClicked();

    void onStep1LineEditTextChanged();
    void onModeButtonClicked(QAbstractButton *btn);
private:
    bool isDigitStr(QString src);
    bool isSweepRobotSn(QString src);
    void updateLabelStatus(QString params);
    void updateTotal();
    void changeBtnState(OP_MODE mode);
    void checkCSVFile();
    void deleleMACDir(QString path);

    void disableAllSteButtons();
    void disableAllEdit();

    void runStep0();
    void runStep1();
    void runStep2();
    void runStep3();
    void runStep4();

    int isSettingValid();
    void gotoResetState();
    void gotoResultSuccess();
    void gotoResultFailed();

    void setLabelBGDefault(QLabel *label);
    void setLabelBGSuccess(QLabel *label);
    void setLabelBGFailed(QLabel *label);

    QString getTodayKeySuccess();
    QString getTodayKeyFailed();
    void changeCertVersion(QString certVer);
private:
    Ui::MainWindow *ui = NULL;
    QButtonGroup *m_modeBtnGroup = NULL;
    QRadioButton *m_rbAuto = NULL;
    QRadioButton *m_rbManual = NULL;

    QPushButton *m_autoBtn = NULL;
    QLabel *m_certShow = NULL;

    QPushButton *m_step0Btn = NULL;
    QPushButton *m_step1Btn = NULL;
    QPushButton *m_step2Btn = NULL;
    QPushButton *m_step3Btn = NULL;
    QPushButton *m_step4Btn = NULL;

    QLabel *m_step0PromptLabel = NULL;
    QLabel *m_step1PromptLabel = NULL;
    QLabel *m_step2PromptLabel = NULL;
    QLabel *m_step3PromptLabel = NULL;
    QLabel *m_step4PromptLabel = NULL;

    QLabel *m_step0ResultLabel = NULL;
    QLabel *m_step1ResultLabel = NULL;
    QLabel *m_step2ResultLabel = NULL;
    QLabel *m_step3ResultLabel = NULL;
    QLabel *m_step4ResultLabel = NULL;

    QLineEdit *m_step0DataEdit = NULL;
    QLineEdit *m_step1DataEdit = NULL;
    QLineEdit *m_step2DataEdit = NULL;
    QLineEdit *m_step3DataEdit = NULL;
    QLineEdit *m_step4DataEdit = NULL;

    QPushButton *m_resetBtn = NULL;
    QLabel *m_finalResultLabel = NULL;

    QLabel *m_totalSuccessLabel = NULL;
    QLabel *m_totalFailureLabel = NULL;

    QStatusBar *m_statusBar = NULL;
    QLabel *m_labelStatus = NULL;

    QString m_com;
    QString m_factoryName;
    QString m_FWDev;
    QString m_FWVersion;
    QString m_certVersion;
    int m_xpos;

    OP_MODE m_opMode;
    OP_STEPS m_currStep;

    ulong m_totalFailed;
    ulong m_totalSuccess;
    QFile m_csvFile;
    QString m_serialno;
    QString m_mac;
};
#endif
