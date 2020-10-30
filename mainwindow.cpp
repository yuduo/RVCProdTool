#include <QRegExp>
#include <QRegExpValidator>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QFile file(":/ui.qss");
    file.open(QFile::ReadOnly);
    QString styleSh = tr(file.readAll());
    setStyleSheet(styleSh);

    initData();
    checkCSVFile();
    initUI();
    changeBtnState(m_opMode);
    updateTotal();
    gotoResetState();
    deleleMACDir("C:\\AWS_Certificate_Generate");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_csvFile.isOpen())
    {
        m_csvFile.flush();
        m_csvFile.close();
    }
}

void MainWindow::initData()
{
    m_opMode = OP_MODE_MANUAL;
    m_currStep = OP_IDLE;

    m_totalSuccess = 0;
    m_totalFailed = 0;

    QSettings setting("config.ini", QSettings::IniFormat);
    m_com = setting.value("COMM/com").toString();
    QString key_success = getTodayKeySuccess();
    m_totalSuccess = setting.value("STATISTIC/" + key_success).toInt();

    QString key_failed = getTodayKeyFailed();
    m_totalFailed = setting.value("STATISTIC/" + key_failed).toInt();

    m_factoryName = setting.value("FACTORY/name").toString();
    m_FWDev = setting.value("FIRMWARE/developer").toString();
    m_FWVersion = setting.value("FIRMWARE/version").toString();
    m_certVersion = setting.value("FIRMWARE/cert").toString();
    if (m_certVersion != "prod" && m_certVersion != "test" && m_certVersion != "dev")
    {
        changeCertVersion("prod");
    }
}

void MainWindow::checkCSVFile()
{
    QDir dbDir("c://APCSV");
    if (false == dbDir.exists())
    {
        dbDir.mkdir("c://APCSV");
    }

    QString csv_path = "c://APCSV/" + QDate::currentDate().toString("yyyyMMdd") + "_" + m_factoryName + "_APPLIANCELIST.csv";
    m_csvFile.setFileName(csv_path);
    if (false == m_csvFile.exists())
    {
        m_csvFile.open(QIODevice::ReadWrite);
        m_csvFile.close();
    }
    m_csvFile.open(QIODevice::ReadWrite);
}

void MainWindow:: deleleMACDir(QString path)
{
    QDir dir(path);
    if (dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        dir.setSorting(QDir::Name);
        QStringList dirList = dir.entryList();
        for (int i = 0; i < dirList.length(); i++)
        {
            if (dirList[i].length() > 0)
            {
                QString dirPath = path + QDir::separator() + dirList[i];
                QDir dirMac(dirPath);
                dirMac.removeRecursively();
            }
        }
    }
}

void MainWindow::initUI()
{
    setWindowTitle(tr("SweepRobot 生产工具"));
    resize(1400, 800);
    setMinimumSize(1400, 800);

    QMenuBar *mb = menuBar();
    QMenu *menuCfg =  mb->addMenu(tr("配置"));
    QMenu *menuFile =  mb->addMenu(tr("文件"));
    QMenu *menuElse =  mb->addMenu(tr("其他"));

    QAction *actionCom = new QAction(QIcon(":/img/image/iconCom.png"), tr("串口"), this);
    actionCom->setStatusTip(tr("配置串口参数"));
    connect(actionCom, SIGNAL(triggered()), this, SLOT(actionComClicked()));
    menuCfg->addAction(actionCom);

    QAction *actionFactory = new QAction(QIcon(":/img/image/factory.png"), tr("生产商"), this);
    actionFactory->setStatusTip(tr("配置生产厂家名称"));
    connect(actionFactory, SIGNAL(triggered()), this, SLOT(actionFactoryClicked()));
    menuCfg->addAction(actionFactory);

    QAction *actionFWDev = new QAction(QIcon(":/img/image/fw_dev.png"), tr("固件开发者"), this);
    actionFWDev->setStatusTip(tr("配置固件开发者名称"));
    connect(actionFWDev, SIGNAL(triggered()), this, SLOT(actionFWDevClicked()));
    menuCfg->addAction(actionFWDev);

    QAction *actionFWVersion = new QAction(QIcon(":/img/image/fw_version.png"), tr("固件版本"), this);
    actionFWVersion->setStatusTip(tr("配置固件版本号"));
    connect(actionFWVersion, SIGNAL(triggered()), this, SLOT(actionFWVersionClicked()));
    menuCfg->addAction(actionFWVersion);

    QAction *actionCertVer = new QAction(QIcon(":/img/image/iconCert.png"), tr("证书版本"), this);
    actionCertVer->setStatusTip(tr("配置设备证书版本号"));
    connect(actionCertVer, SIGNAL(triggered()), this, SLOT(actionCertVersionClicked()));
    menuCfg->addAction(actionCertVer);

    QAction *actionFiels = new QAction(QIcon(":/img/image/iconFiles.png"), tr("查看"), this);
    actionFiels->setStatusTip(tr("查看生产文件"));
    connect(actionFiels, SIGNAL(triggered()), this, SLOT(actionFilesClicked()));
    menuFile->addAction(actionFiels);

    QAction *actionAbout = new QAction(QIcon(":/img/image/iconAbout.png"), tr("关于"), this);
    actionAbout->setStatusTip(tr("软件信息"));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(actionAboutClicked()));
    menuElse->addAction(actionAbout);

    QToolBar *tbCfg = addToolBar(tr("配置"));
    tbCfg->addAction(actionCom);
    tbCfg->addAction(actionFactory);
    tbCfg->addAction(actionFWDev);
    tbCfg->addAction(actionFWVersion);

    QToolBar *tbFile = addToolBar(tr("文件"));
    tbFile->addAction(actionFiels);

    QToolBar *tbElse = addToolBar(tr("帮助"));
    tbElse->addAction(actionAbout);

    m_statusBar = statusBar();
    m_labelStatus = new QLabel(this);
    m_statusBar->addWidget(m_labelStatus);

    m_modeBtnGroup = new QButtonGroup(this);
    m_modeBtnGroup->setExclusive(true);
    m_rbManual = new QRadioButton(tr("手动"), this);
    m_rbAuto = new QRadioButton(tr("自动"), this);
    if (OP_MODE_MANUAL == m_opMode)
    {
        m_rbManual->setChecked(true);
        m_rbAuto->setChecked(false);
    }
    else
    {
        m_rbManual->setChecked(false);
        m_rbAuto->setChecked(true);
    }

    m_modeBtnGroup->addButton(m_rbManual);
    m_modeBtnGroup->addButton(m_rbAuto);
    m_modeBtnGroup->setId(m_rbManual, OP_MODE_MANUAL);
    m_modeBtnGroup->setId(m_rbAuto, OP_MODE_AUTO);
    connect(m_modeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onModeButtonClicked(QAbstractButton*)));

    QHBoxLayout *opModeLayout = new QHBoxLayout;
    opModeLayout->setContentsMargins(30, 5, 30, 5);
    opModeLayout->setSpacing(30);
    opModeLayout->addWidget(m_rbManual);
    opModeLayout->addWidget(m_rbAuto);

    m_autoBtn = new QPushButton(tr("自动执行"), this);
    opModeLayout->addWidget(m_autoBtn);
    opModeLayout->addStretch(1);
    connect(m_autoBtn, &QPushButton::clicked, this, &MainWindow::btnAutoCliecked);

    m_certShow = new QLabel(m_certVersion, this);
    m_certShow->setStyleSheet("font-size: 30pt; color: blue");
    opModeLayout->addWidget(m_certShow);

    m_step0Btn = new QPushButton(tr("执行"), this);
    m_step1Btn = new QPushButton(tr("执行"), this);
    m_step2Btn = new QPushButton(tr("执行"), this);
    m_step3Btn = new QPushButton(tr("执行"), this);
    m_step4Btn = new QPushButton(tr("执行"), this);

    connect(m_step0Btn, &QPushButton::clicked, this, &MainWindow::btnStep0Cliecked);
    connect(m_step1Btn, &QPushButton::clicked, this, &MainWindow::btnStep1Cliecked);
    connect(m_step2Btn, &QPushButton::clicked, this, &MainWindow::btnStep2Cliecked);
    connect(m_step3Btn, &QPushButton::clicked, this, &MainWindow::btnStep3Cliecked);
    connect(m_step4Btn, &QPushButton::clicked, this, &MainWindow::btnStep4Cliecked);

    m_step0PromptLabel = new QLabel(tr("步骤一：烧写bin文件"), this);
    m_step1PromptLabel = new QLabel(tr("步骤二：扫描设备的序列号"), this);
    m_step2PromptLabel = new QLabel(tr("步骤三：获取设备MAC地址"), this);
    m_step3PromptLabel = new QLabel(tr("步骤四：创建设备证书文件"), this);
    m_step4PromptLabel = new QLabel(tr("步骤五：烧写设备证书文件"), this);

    m_step0ResultLabel = new QLabel(tr("未执行"), this);
    m_step1ResultLabel = new QLabel(tr("未执行"), this);
    m_step2ResultLabel = new QLabel(tr("未执行"), this);
    m_step3ResultLabel = new QLabel(tr("未执行"), this);
    m_step4ResultLabel = new QLabel(tr("未执行"), this);

    m_step0DataEdit = new QLineEdit(this);
    m_step0DataEdit->setPlaceholderText("d:/candy-vx.y.z.bin");
    m_step1DataEdit = new QLineEdit(this);
    m_step2DataEdit = new QLineEdit(this);
    m_step3DataEdit = new QLineEdit(this);
    m_step4DataEdit = new QLineEdit(this);

    m_step1DataEdit->setMaxLength(18);
    connect(m_step1DataEdit, &QLineEdit::textChanged, this, &MainWindow::onStep1LineEditTextChanged);

    QHBoxLayout *step0Layout = new QHBoxLayout;
    step0Layout->setContentsMargins(30, 5, 30, 5);
    step0Layout->setSpacing(30);

    step0Layout->addWidget(m_step0PromptLabel);
    step0Layout->addWidget(m_step0Btn);
    step0Layout->addWidget(m_step0ResultLabel);
    step0Layout->addWidget(m_step0DataEdit);

    step0Layout->setStretchFactor(m_step0PromptLabel, 3);
    step0Layout->setStretchFactor(m_step0Btn, 1);
    step0Layout->setStretchFactor(m_step0ResultLabel, 3);
    step0Layout->setStretchFactor(m_step0ResultLabel, 3);

    QHBoxLayout *step1Layout = new QHBoxLayout;
    step1Layout->setContentsMargins(30, 5, 30, 5);
    step1Layout->setSpacing(30);

    step1Layout->addWidget(m_step1PromptLabel);
    step1Layout->addWidget(m_step1Btn);
    step1Layout->addWidget(m_step1ResultLabel);
    step1Layout->addWidget(m_step1DataEdit);

    step1Layout->setStretchFactor(m_step1PromptLabel, 3);
    step1Layout->setStretchFactor(m_step1Btn, 1);
    step1Layout->setStretchFactor(m_step1ResultLabel, 3);
    step1Layout->setStretchFactor(m_step1ResultLabel, 3);

    QHBoxLayout *step2Layout = new QHBoxLayout;
    step2Layout->setContentsMargins(30, 5, 30, 5);
    step2Layout->setSpacing(30);

    step2Layout->addWidget(m_step2PromptLabel);
    step2Layout->addWidget(m_step2Btn);
    step2Layout->addWidget(m_step2ResultLabel);
    step2Layout->addWidget(m_step2DataEdit);

    step2Layout->setStretchFactor(m_step2PromptLabel, 3);
    step2Layout->setStretchFactor(m_step2Btn, 1);
    step2Layout->setStretchFactor(m_step2ResultLabel, 3);
    step2Layout->setStretchFactor(m_step2ResultLabel, 3);

    QHBoxLayout *step3Layout = new QHBoxLayout;
    step3Layout->setContentsMargins(30, 5, 30, 5);
    step3Layout->setSpacing(30);

    step3Layout->addWidget(m_step3PromptLabel);
    step3Layout->addWidget(m_step3Btn);
    step3Layout->addWidget(m_step3ResultLabel);
    step3Layout->addWidget(m_step3DataEdit);

    step3Layout->setStretchFactor(m_step3PromptLabel, 3);
    step3Layout->setStretchFactor(m_step3Btn, 1);
    step3Layout->setStretchFactor(m_step3ResultLabel, 3);
    step3Layout->setStretchFactor(m_step3ResultLabel, 3);

    QHBoxLayout *step4Layout = new QHBoxLayout;
    step4Layout->setContentsMargins(30, 5, 30, 5);
    step4Layout->setSpacing(30);

    step4Layout->addWidget(m_step4PromptLabel);
    step4Layout->addWidget(m_step4Btn);
    step4Layout->addWidget(m_step4ResultLabel);
    step4Layout->addWidget(m_step4DataEdit);

    step4Layout->setStretchFactor(m_step4PromptLabel, 3);
    step4Layout->setStretchFactor(m_step4Btn, 1);
    step4Layout->setStretchFactor(m_step4ResultLabel, 3);
    step4Layout->setStretchFactor(m_step4ResultLabel, 3);

    m_resetBtn = new QPushButton(tr("复位"), this);
    connect(m_resetBtn, &QPushButton::clicked, this, &MainWindow::btnResetClicked);

    QFont font("Microsoft YaHei", 15, 75);
    m_resetBtn->setFont(font);
    m_resetBtn->setContentsMargins(20, 5, 20, 5);

    m_finalResultLabel = new QLabel(tr("未执行"), this);
    m_finalResultLabel->setFont(font);
    m_finalResultLabel->setMargin(10);

    QHBoxLayout *finalResultLayout = new QHBoxLayout;
    finalResultLayout->addStretch(1);
    finalResultLayout->addWidget(m_finalResultLabel);
    finalResultLayout->addStretch(1);

    QHBoxLayout *finalResetLayout = new QHBoxLayout;
    finalResetLayout->setContentsMargins(5, 5, 5, 5);

    finalResetLayout->addWidget(m_resetBtn);

    m_totalSuccessLabel = new QLabel(tr("0"), this);
    QLabel *totalSuccess = new QLabel(tr("成功: "));
    QHBoxLayout *totalSuccessLayout = new QHBoxLayout;
    totalSuccessLayout->setContentsMargins(30, 5, 30, 5);
    totalSuccessLayout->addStretch(1);
    totalSuccessLayout->addWidget(totalSuccess);
    totalSuccessLayout->addWidget(m_totalSuccessLabel);

    m_totalFailureLabel = new QLabel(tr("0"), this);
    QLabel *totalFailure = new QLabel(tr("失败: "));
    QHBoxLayout *totalFailureLayout = new QHBoxLayout;
    totalFailureLayout->setContentsMargins(30, 5, 30, 5);
    totalFailureLayout->addStretch(1);
    totalFailureLayout->addWidget(totalFailure);
    totalFailureLayout->addWidget(m_totalFailureLabel);

    m_totalSuccessLabel->setStyleSheet("font-size: 20pt; color: green");
    m_totalFailureLabel->setStyleSheet("font-size: 20pt; color: red");

    QVBoxLayout *vLayoutSteps = new QVBoxLayout;
    vLayoutSteps->addStretch();
    vLayoutSteps->addLayout(opModeLayout);
    vLayoutSteps->addStretch();
    vLayoutSteps->addLayout(step0Layout);
    vLayoutSteps->addStretch();
    vLayoutSteps->addLayout(step1Layout);
    vLayoutSteps->addStretch();
    vLayoutSteps->addLayout(step2Layout);
    vLayoutSteps->addStretch();
    vLayoutSteps->addLayout(step3Layout);
    vLayoutSteps->addStretch();
    vLayoutSteps->addLayout(step4Layout);
    vLayoutSteps->addStretch();

    vLayoutSteps->addLayout(finalResultLayout);
    vLayoutSteps->addLayout(finalResetLayout);

    vLayoutSteps->addLayout(totalSuccessLayout);
    vLayoutSteps->addLayout(totalFailureLayout);


    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(vLayoutSteps);
    this->setCentralWidget(mainWidget);
}

void MainWindow::updateLabelStatus(QString params)
{
    m_labelStatus->setText(params);
}

void MainWindow::updateTotal()
{
   m_totalSuccessLabel->setText(QString::number(m_totalSuccess));
    m_totalFailureLabel->setText(QString::number(m_totalFailed));
}

void MainWindow::disableAllSteButtons()
{
    m_step0Btn->setEnabled(false);
    m_step1Btn->setEnabled(false);
    m_step2Btn->setEnabled(false);
    m_step3Btn->setEnabled(false);
    m_step4Btn->setEnabled(false);
}

void MainWindow::disableAllEdit()
{
    m_step1DataEdit->setEnabled(false);
    m_step2DataEdit->setEnabled(false);
    m_step3DataEdit->setEnabled(false);
    m_step4DataEdit->setEnabled(false);
}

void MainWindow::changeBtnState(OP_MODE mode)
{
    if (OP_MODE_MANUAL == mode)
    {
        m_autoBtn->setEnabled(false);
        disableAllSteButtons();
        m_step0Btn->setEnabled(true);
        m_step0Btn->setFocus();
        disableAllEdit();
    }
    else
    {
        m_autoBtn->setEnabled(true);
        m_autoBtn->setFocus();
        disableAllSteButtons();
        disableAllEdit();
    }
}

void MainWindow::runStep0()
{
    m_currStep = OP_STEP0;

    QString binPath = m_step0DataEdit->text().trimmed();
    if (0 == binPath.length())
    {
        QMessageBox::warning(this, tr("错误"),
                                   tr("请输入bin文件路径"),
                                   tr("确定"));
        gotoResetState();
        return;
    }

    QFileInfo binFileInfo(binPath);
    if (false == binFileInfo.exists())
    {
        QMessageBox::warning(this, tr("错误"),
                                   tr("输入的bin文件不存在!"),
                                   tr("确定"));
        gotoResetState();
        return;
    }

    QString cmd_write = "c://AWS_Certificate_Generate/esptool.py --chip esp32 -p " + m_com
                + " -b 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x0 "
                + binPath;

    bool write_ok = false;

    do
    {
        m_step0ResultLabel->setText(tr("正在烧写bin文件"));
        repaint();
        QProcess process;
        process.start("cmd.exe", QStringList() <<"/c" << cmd_write);
        if (process.waitForStarted(5000))
        {
            process.waitForFinished(40000);
            QByteArray bytes = process.readAllStandardOutput();
            QString msg = QString::fromLocal8Bit(bytes);

            if (msg.length() > 0 && msg.indexOf("Hash of data verified") > 0)
            {
                write_ok = true;
                break;
            }
        }
        else
        {
            m_step0ResultLabel->setText(tr("烧写bin文件失败！"));
            repaint();
        }
    }while(0);

    if (true == write_ok)
    {
        m_step0ResultLabel->setText(tr("执行成功"));
        setLabelBGSuccess(m_step0ResultLabel);
        repaint();

        m_step1Btn->setEnabled(true);
        runStep1();
    }
    else
    {
        m_step0ResultLabel->setText(tr("执行失败"));
        setLabelBGFailed(m_step0ResultLabel);
        repaint();

        gotoResultFailed();
    }
}

void MainWindow::runStep1()
{
    m_serialno.clear();
    m_mac.clear();

    m_finalResultLabel->setText(tr("等待执行结果"));

    m_currStep = OP_STEP1;
    m_step1ResultLabel->setText(tr("请扫描序列号"));
    m_step1DataEdit->setEnabled(true);
    m_step1DataEdit->setFocus();
}

void MainWindow::runStep2()
{
    m_currStep = OP_STEP2;

    QString cmd_mac = "c://AWS_Certificate_Generate/esptool.py -p " + m_com + " read_mac";
    bool mac_ok = false;

    do
    {
        m_step2ResultLabel->setText(tr("正在获取MAC地址"));
        repaint();
        QProcess process;
        process.start("cmd.exe", QStringList() <<"/c" << cmd_mac);
        if (process.waitForStarted(5000))
        {
            process.waitForFinished(5000);
            QByteArray bytes = process.readAllStandardOutput();
            QString msg = QString::fromLocal8Bit(bytes);

            QStringList list = msg.split("\r\n");
            for (int i = 0;i <list.count(); i++)
            {
                QString line = list.at(i);
                if (line.startsWith("MAC: "))
                {
                    m_mac = line.mid(5).replace(":", "-");
                    m_step2DataEdit->setText(m_mac);
                    mac_ok = true;
                    break;
                }
            }

            if (true == mac_ok)
                break;
        }
        else
        {
            m_step2ResultLabel->setText(tr("读取ESP32 MAC地址失败！"));
            repaint();
        }
    }while(0);

    if (true == mac_ok)
    {
        m_step2ResultLabel->setText(tr("执行成功"));
        setLabelBGSuccess(m_step2ResultLabel);
        m_step2DataEdit->setEnabled(false);
        repaint();

        if (OP_MODE_AUTO == m_opMode)
        {
            runStep3();
        }
        else
        {
            m_step3Btn->setEnabled(true);
            m_step3Btn->setFocus();
        }
    }
    else
    {
        m_step2ResultLabel->setText(tr("执行失败"));
        setLabelBGFailed(m_step2ResultLabel);
        repaint();

        gotoResultFailed();
    }
}

void MainWindow::runStep3()
{
    m_currStep = OP_STEP3;
    bool crt_ok = false;

    QString cmd_crt = "c://AWS_Certificate_Generate/CERT_GEN.bat " + m_com + " " + m_serialno + " " + m_mac;
    do
    {
        m_step3ResultLabel->setText(tr("正在创建设备安全证书"));
        repaint();

        if (m_mac.length() > 0)
        {
            QDir dir;
            dir.setPath("c://AWS_Certificate_Generate/" + m_mac);
            dir.removeRecursively();
        }

        QProcess process;
        process.start("cmd.exe", QStringList() <<"/c" << cmd_crt);
        if (process.waitForStarted(5000))
        {
            process.waitForFinished(30000);
            QByteArray bytes = process.readAllStandardOutput();
            QString msg = QString::fromLocal8Bit(bytes);

            QDir dir("c://AWS_Certificate_Generate/" + m_mac);
            if (dir.exists())
            {
                QString path = "c://AWS_Certificate_Generate/" + m_mac + "/" + m_mac + "_nvs_ext.bin";
                QFileInfo fi(path);
                if (fi.isFile())
                {
                    m_step3DataEdit->setText("nvs_ext.bin");
                    crt_ok = true;
                    break;
                }
            }

        }
        else
        {
            m_step3ResultLabel->setText(tr("创建设备安全证书失败！"));
            repaint();
        }
    }while(0);

    if (true == crt_ok)
    {
        m_step3ResultLabel->setText(tr("执行成功"));
        setLabelBGSuccess(m_step3ResultLabel);
        m_step3DataEdit->setEnabled(false);

        if (OP_MODE_AUTO == m_opMode)
        {
            runStep4();
        }
        else
        {
            m_step4Btn->setEnabled(true);
            m_step4Btn->setFocus();
        }
    }
    else
    {
        m_step3ResultLabel->setText(tr("执行失败"));
        setLabelBGFailed(m_step3ResultLabel);
        repaint();

        gotoResultFailed();
    }
}

void MainWindow::runStep4()
{
    m_currStep = OP_STEP4;
    QString cmd_write = "c://AWS_Certificate_Generate/" + m_mac +"/esptool.py -p " + m_com
                        + " -b 921600 write_flash 0x1B000 "
                        + "c://AWS_Certificate_Generate/" + m_mac + "/" + m_mac + "_nvs_ext.bin";
    bool write_ok = false;

    do
    {
        m_step4ResultLabel->setText(tr("正在烧写设备安全证书"));
        m_step4DataEdit->clear();
        repaint();
        QProcess process;
        process.start("cmd.exe", QStringList() <<"/c" << cmd_write);
        if (process.waitForStarted(5000))
        {
            process.waitForFinished(30000);
            QByteArray bytes = process.readAllStandardOutput();
            QString msg = QString::fromLocal8Bit(bytes);

            if (msg.length() > 0 && msg.indexOf("Hash of data verified") > 0)
                write_ok = true;

            if (true == write_ok)
            {
                m_step4DataEdit->setText(tr("write success"));
                repaint();
                break;
            }
        }
        else
        {
            m_step4ResultLabel->setText(tr("烧写设备证书失败！"));
            m_step4DataEdit->setText(tr("write failed"));
            repaint();
        }
    }while(0);

    if (true == write_ok)
    {
        m_step4ResultLabel->setText(tr("执行成功"));
        setLabelBGSuccess(m_step4ResultLabel);
        m_step4DataEdit->setEnabled(false);
        repaint();

        gotoResultSuccess();
    }
    else
    {
        m_step4ResultLabel->setText(tr("执行失败"));
        setLabelBGFailed(m_step4ResultLabel);
        repaint();

        gotoResultFailed();
    }
}

void MainWindow::btnAutoCliecked()
{
    int res = isSettingValid();
    if (0 != res)
    {
        QString msg = tr("请完善配置信息");
        switch(res)
        {
            case -1: msg = tr("请输入配置参数：串口号"); break;
            case -2: msg = tr("请输入配置参数：服务器地址"); break;
            case -3: msg = tr("请输入配置参数：生产商名称"); break;
            case -4: msg = tr("请输入配置参数：固件开发者"); break;
            case -5: msg = tr("请输入配置参数：固件版本号"); break;
        }

        QMessageBox::warning(this, tr("错误"),
                                   msg,
                                   tr("确定"));
        return;
    }

    m_rbManual->setEnabled(false);
    m_rbAuto->setEnabled(false);
    disableAllSteButtons();
    m_autoBtn->setEnabled(false);
    runStep0();
}

void MainWindow::btnStep0Cliecked()
{
    int res = isSettingValid();
    if (0 != res)
    {
        QString msg = tr("请完善配置信息");
        switch(res)
        {
            case -1: msg = tr("请输入配置参数：串口号"); break;
            case -2: msg = tr("请输入配置参数：服务器地址"); break;
            case -3: msg = tr("请输入配置参数：生产商名称"); break;
            case -4: msg = tr("请输入配置参数：固件开发者"); break;
            case -5: msg = tr("请输入配置参数：固件版本号"); break;
        }

        QMessageBox::warning(this, tr("错误"),
                                   msg,
                                   tr("确定"));
        return;
    }

    m_rbManual->setEnabled(false);
    m_rbAuto->setEnabled(false);

    m_step0Btn->setEnabled(false);
    runStep0();
}

void MainWindow::btnStep1Cliecked()
{
    m_step1Btn->setEnabled(false);
    runStep1();
}

void MainWindow::btnStep2Cliecked()
{
    m_step2Btn->setEnabled(false);
    runStep2();
}

void MainWindow::btnStep3Cliecked()
{
    m_step3Btn->setEnabled(false);
    runStep3();
}

void MainWindow::btnStep4Cliecked()
{
    m_step4Btn->setEnabled(false);
    runStep4();
}

void MainWindow::btnResetClicked()
{
    if (OP_IDLE == m_currStep)
    {
        gotoResetState();
    }
    else
    {
        if (1 == QMessageBox::warning(this, tr("警告"),
                                   tr("当前生产流程没有完成，请确定执行复位操作？"),
                                   tr("取消"), tr("确定")))
        {
            gotoResetState();
        }
    }
}

void MainWindow::actionComClicked()
{
    QInputDialog dlg(this);
    dlg.setWindowTitle(tr("设置"));
    dlg.setLabelText(tr("输入端口号"));
    dlg.setTextValue(m_com);
    dlg.setOkButtonText(tr("确定"));
    dlg.setCancelButtonText(tr("取消"));
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.resize(200, 200);
    if (QInputDialog::Accepted == dlg.exec())
    {
        QString input = dlg.textValue();
        QRegExp rx1("com\\d");
        QRegExp rx2("COM\\d");
        if (rx1.exactMatch(input) || rx2.exactMatch(input))
        {
            m_com = input;
            QSettings setting("config.ini", QSettings::IniFormat);
            setting.beginGroup("COMM");
            setting.setValue("com", input);
            setting.endGroup();
            setting.sync();
        }
        else
        {
            QMessageBox::warning(this, tr("错误"),
                                       tr("串口输入格式无效"),
                                       tr("确定"));
        }
    }
}

void MainWindow::actionFactoryClicked()
{
    QInputDialog dlg(this);
    dlg.setWindowTitle(tr("设置"));
    dlg.setLabelText(tr("输入生产商名称"));
    dlg.setTextValue(m_factoryName);
    dlg.setOkButtonText(tr("确定"));
    dlg.setCancelButtonText(tr("取消"));
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.resize(200, 200);
    if (QInputDialog::Accepted == dlg.exec())
    {
        QString input = dlg.textValue();
        if (input.length() >0)
        {
            m_factoryName = input;
            QSettings setting("config.ini", QSettings::IniFormat);
            setting.beginGroup("FACTORY");
            setting.setValue("name", input);
            setting.endGroup();
            setting.sync();

            checkCSVFile();
        }
        else
        {
            QMessageBox::warning(this, tr("错误"),
                                       tr("输入不可以为空"),
                                       tr("确定"));
        }
    }
}

void MainWindow::actionFWDevClicked()
{
    QInputDialog dlg(this);
    dlg.setWindowTitle(tr("设置"));
    dlg.setLabelText(tr("输入固件开发者名称"));
    dlg.setTextValue(m_FWDev);
    dlg.setOkButtonText(tr("确定"));
    dlg.setCancelButtonText(tr("取消"));
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.resize(200, 200);
    if (QInputDialog::Accepted == dlg.exec())
    {
        QString input = dlg.textValue();
        if (input.length() > 0)
        {
            m_FWDev = input;
            QSettings setting("config.ini", QSettings::IniFormat);
            setting.beginGroup("FIRMWARE");
            setting.setValue("developer", input);
            setting.endGroup();
            setting.sync();
        }
        else
        {
            QMessageBox::warning(this, tr("错误"),
                                       tr("输入不可以为空"),
                                       tr("确定"));
        }
    }
}

void MainWindow::actionFWVersionClicked()
{
    QInputDialog dlg(this);
    dlg.setWindowTitle(tr("设置"));
    dlg.setLabelText(tr("输入固件版本号"));
    dlg.setTextValue(m_FWVersion);
    dlg.setOkButtonText(tr("确定"));
    dlg.setCancelButtonText(tr("取消"));
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.resize(200, 200);
    if (QInputDialog::Accepted == dlg.exec())
    {
        QString input = dlg.textValue();
        QRegExp rx("V.*");
        if (rx.exactMatch(input))
        {
            m_FWVersion = input;
            QSettings setting("config.ini", QSettings::IniFormat);
            setting.beginGroup("FIRMWARE");
            setting.setValue("version", input);
            setting.endGroup();
            setting.sync();
        }
        else
        {
            QMessageBox::warning(this, tr("错误"),
                                       tr("固件版本号输入格式无效"),
                                       tr("确定"));
        }
    }
}

void MainWindow::actionCertVersionClicked()
{
    QStringList items;
    items << "prod" << "test" << "dev";

    QInputDialog dlg(this);
    dlg.setWindowTitle(tr("设置"));
    dlg.setLabelText(tr("选择证书类型"));
    dlg.setOkButtonText(tr("确定"));
    dlg.setCancelButtonText(tr("取消"));
    dlg.setInputMode(QInputDialog::TextInput);

    dlg.setComboBoxItems(items);
    dlg.setComboBoxEditable(false);
    dlg.setTextValue(m_certVersion);

    dlg.resize(200, 200);
    if (QInputDialog::Accepted == dlg.exec())
    {
        QString input = dlg.textValue();

        if (m_certVersion != input)
        {
            changeCertVersion(input);
        }
    }
}

void MainWindow::actionFilesClicked()
{
    QDesktopServices::openUrl(QUrl("C://APCSV", QUrl::TolerantMode));
}

void MainWindow::actionAboutClicked()
{
    QMessageBox::information(this, tr("关于"),
                                    tr("V0.1.5\n技术支持：无锡桓海智能科技"),
                                    tr("确定"));
}

int MainWindow::isSettingValid()
{
    int res = 0;
    do {
        if (m_com.length() == 0)
        {
            res = -1;
            break;
        }

        if (m_factoryName.length() == 0)
        {
            res = -3;
            break;
        }

        if (m_FWDev.length() == 0)
        {
            res = -4;
            break;
        }

        if (m_FWVersion.length() == 0)
        {
            res = -5;
            break;
        }
    }while(false);

    return res;
}

bool MainWindow::isDigitStr(QString src)
{
    QByteArray ba = src.toLatin1();
    const char *s = ba.data();
    while(*s && *s>='0' && *s<='9') s++;
    if (*s)
        return false;
    else
        return true;
}

bool MainWindow::isSweepRobotSn(QString src)
{
    QRegExp rx("^\\d{8} \\d{4} \\d{4}$");
    QRegExpValidator v(rx, 0);
    int pos;
    if (QValidator::Acceptable == v.validate(src, pos))
        return true;
    return false;
}

void MainWindow::onStep1LineEditTextChanged()
{
    m_serialno = m_step1DataEdit->text();
    if (18 == m_serialno.length())
    {
        if (isSweepRobotSn(m_serialno))
        {
            m_serialno.replace(QString(" "), QString(""));
            m_step1ResultLabel->setText(tr("执行成功"));
            setLabelBGSuccess(m_step1ResultLabel);
            m_step1DataEdit->clearFocus();
            m_step1DataEdit->setEnabled(false);

            if (OP_MODE_AUTO == m_opMode)
            {
                runStep2();
            }
            else
            {
                m_step2Btn->setEnabled(true);
                m_step2Btn->setFocus();
            }
        }
        else
        {
            m_step1ResultLabel->setText(tr("检测到非法字符"));
            setLabelBGFailed(m_step1ResultLabel);
        }
    }
}

void MainWindow::onModeButtonClicked(QAbstractButton *btn)
{
    if (OP_MODE_MANUAL == m_modeBtnGroup->checkedId())
    {
        m_opMode = OP_MODE_MANUAL;
        changeBtnState(OP_MODE_MANUAL);
    }
    else
    {
        m_opMode = OP_MODE_AUTO;
        changeBtnState(OP_MODE_AUTO);
    }
}

void MainWindow::gotoResetState()
{
    m_serialno.clear();
    m_mac.clear();

    m_step0ResultLabel->setText(tr("未执行"));
    m_step1ResultLabel->setText(tr("未执行"));
    m_step2ResultLabel->setText(tr("未执行"));
    m_step3ResultLabel->setText(tr("未执行"));
    m_step4ResultLabel->setText(tr("未执行"));

    m_step1DataEdit->clear();
    m_step2DataEdit->clear();
    m_step3DataEdit->clear();
    m_step4DataEdit->clear();

    setLabelBGDefault(m_step0ResultLabel);
    setLabelBGDefault(m_step1ResultLabel);
    setLabelBGDefault(m_step2ResultLabel);
    setLabelBGDefault(m_step3ResultLabel);
    setLabelBGDefault(m_step4ResultLabel);

    m_finalResultLabel->setText(tr("未执行"));
    setLabelBGDefault(m_finalResultLabel);

    m_rbManual->setEnabled(true);
    m_rbAuto->setEnabled(true);

    m_currStep = OP_IDLE;
    changeBtnState(m_opMode);
}

void MainWindow::setLabelBGDefault(QLabel( *label))
{
    label->setStyleSheet("QLabel{background-color:gray}");
}

void MainWindow::setLabelBGSuccess(QLabel *label)
{
    label->setStyleSheet("QLabel{background-color:green}");
}

void MainWindow::setLabelBGFailed(QLabel *label)
{
    label->setStyleSheet("QLabel{background-color:red}");
}

QString MainWindow::getTodayKeySuccess()
{
    return QDate::currentDate().toString("yyyyMMdd") + "_success";
}

QString MainWindow::getTodayKeyFailed()
{
    return QDate::currentDate().toString("yyyyMMdd") + "_failed";
}

void MainWindow::gotoResultSuccess()
{
    m_finalResultLabel->setText(tr("成功"));
    setLabelBGSuccess(m_finalResultLabel);

    m_totalSuccess += 1;

    QString key_success = getTodayKeySuccess();
    QSettings setting("config.ini", QSettings::IniFormat);
    setting.beginGroup("STATISTIC");
    setting.setValue(key_success, QString::number(m_totalSuccess));
    setting.endGroup();
    setting.sync();

    bool find = false;
    QVector<QString> lines;
    QTextStream out(&m_csvFile);
    out.seek(0);
    while (!out.atEnd()) {
        QString line = out.readLine();
        if (-1 == line.indexOf(m_serialno) && -1 == line.indexOf(m_mac))
        {
            lines.push_back(line);
        }
        else
        {
            find = true;
        }
    }

    if (find)
    {
        m_csvFile.resize(0);
        for (QString line : lines)
            m_csvFile.write(line.append("\n").toLatin1());
        m_csvFile.flush();
    }

    QDateTime now = QDateTime::currentDateTime();
    int offset = now.offsetFromUtc();
    now.setOffsetFromUtc(offset);
    QString dt = now.toString(Qt::ISODate);


    QString line = QString("\t%1\t,%2,%3,%4,%5,%6,%7\n")
                    .arg(m_serialno)
                    .arg(m_mac)
                    .arg(dt)
                    .arg(m_serialno.left(8))
                    .arg(m_factoryName)
                    .arg(m_FWVersion)
                    .arg(m_FWDev);
    m_csvFile.write(line.toLatin1());
    m_csvFile.flush();

    m_currStep = OP_IDLE;
    updateTotal();

    if (m_mac.length() > 0)
    {
        QString path = QString("c://AWS_Certificate_Generate/%1").arg(m_mac);
        QDir dir(path);
        dir.removeRecursively();
    }
}

void MainWindow::gotoResultFailed()
{
    m_finalResultLabel->setText(tr("失败"));
    QFont font("Microsoft YaHei", 15, 75);
    m_finalResultLabel->setFont(font);
    setLabelBGFailed(m_finalResultLabel);

    m_totalFailed += 1;

    QString key_failed = getTodayKeyFailed();
    QSettings setting("config.ini", QSettings::IniFormat);
    setting.beginGroup("STATISTIC");
    setting.setValue(key_failed, QString::number(m_totalFailed));
    setting.endGroup();
    setting.sync();

    m_currStep = OP_IDLE;
    updateTotal();

    QString data2 = m_step2DataEdit->text().trimmed();
    if (data2.length() > 0)
    {
        QString path = QString("c://AWS_Certificate_Generate/%1").arg(data2);
        QDir dir(path);
        dir.removeRecursively();
    }
}

void MainWindow::changeCertVersion(QString input)
{
    QString dest("c://AWS_Certificate_Generate/CERT_GEN.bat");
    QString src = QString("c://AWS_Certificate_Generate/CERT_GEN_%1.bat").arg(input);

    QFileInfo fiSrc(src);
    if (false == fiSrc.exists())
    {
        QMessageBox::warning(this, tr("错误"),
                                   tr("该版本证书生成文件不存在!"),
                                   tr("确定"));
        return;
    }

    do
    {
        QFile f(dest);
        if (false == f.exists())
            break;
        if (f.remove())
            break;

        QMessageBox::warning(this, tr("错误"),
                                   tr("无法删除文件CERT_GEN.bat!"),
                                   tr("确定"));
        return;
    }while(false);

    do
    {
        if (QFile::copy(src, dest))
        {
            m_certVersion = input;
            if (m_certShow)
                m_certShow->setText(m_certVersion);

            QSettings setting("config.ini", QSettings::IniFormat);
            setting.beginGroup("FIRMWARE");
            setting.setValue("cert", m_certVersion);
            setting.endGroup();
            setting.sync();

            break;
        }

        QMessageBox::warning(this, tr("错误"),
                                   tr("无法复制文件!"),
                                   tr("确定"));
        return;
    } while(false);
}
