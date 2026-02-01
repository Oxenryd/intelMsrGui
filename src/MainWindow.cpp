//
// Created by oxenryd on 2025-11-15.
//

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "msr/MsrOps.h"
#include "msr/OcMailbox.h"

#include <cpuid.h>
#include <thread>

#include <QIntValidator>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QMenu>
#include <QMessageBox>

#include "msr/IntelTypes.hpp"
#include "msr/Msr.hpp"
#include "msr/IMT_ErrCode.h"

MainWindow::MainWindow(QWidget* parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(size());
    setMinimumSize(size());
    setMaximumSize(size());
    init();
    checkPresetAutostart();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateTrayMenu(QMenu *menu) {

    menu->clear();
    QStringList names;
    for (int i = 0; i < ui->presetCombo->count(); ++i) {
        names << ui->presetCombo->itemText(i);
    }

    for (const QString &name : names) {
        QAction *act = menu->addAction(name);
        connect(act, &QAction::triggered,
            this, [this, name]() {
                const SetupPackage pkg = m_presetMap[name.toStdString()];
                if (MsrOps::apply(pkg) != IMT_ErrCode::OK) {
                    QMessageBox::critical(nullptr,
                      "IntelMsrGui",
                      "Could load preset!");
                }
        });
    }

}

void MainWindow::init() {

    m_pCoreEdits.push_back(ui->pCoreEdit_1);
    m_pCoreEdits.push_back(ui->pCoreEdit_2);
    m_pCoreEdits.push_back(ui->pCoreEdit_3);
    m_pCoreEdits.push_back(ui->pCoreEdit_4);
    m_pCoreEdits.push_back(ui->pCoreEdit_5);
    m_pCoreEdits.push_back(ui->pCoreEdit_6);
    m_pCoreEdits.push_back(ui->pCoreEdit_7);
    m_pCoreEdits.push_back(ui->pCoreEdit_8);

    m_eCoreEdits.push_back(ui->eCoreEdit_1);
    m_eCoreEdits.push_back(ui->eCoreEdit_2);
    m_eCoreEdits.push_back(ui->eCoreEdit_3);
    m_eCoreEdits.push_back(ui->eCoreEdit_4);
    m_eCoreEdits.push_back(ui->eCoreEdit_5);
    m_eCoreEdits.push_back(ui->eCoreEdit_6);
    m_eCoreEdits.push_back(ui->eCoreEdit_7);
    m_eCoreEdits.push_back(ui->eCoreEdit_8);

    m_vfEdits.emplace_back(ui->vfEdit_1);
    m_vfEdits.emplace_back(ui->vfEdit_2);
    m_vfEdits.emplace_back(ui->vfEdit_3);
    m_vfEdits.emplace_back(ui->vfEdit_4);
    m_vfEdits.emplace_back(ui->vfEdit_5);
    m_vfEdits.emplace_back(ui->vfEdit_6);
    m_vfEdits.emplace_back(ui->vfEdit_7);
    m_vfEdits.emplace_back(ui->vfEdit_8);
    m_vfEdits.emplace_back(ui->vfEdit_9);
    m_vfEdits.emplace_back(ui->vfEdit_10);
    m_vfEdits.emplace_back(ui->vfEdit_11);

    m_vfSliders.push_back(ui->slider_1);
    m_vfSliders.push_back(ui->slider_2);
    m_vfSliders.push_back(ui->slider_3);
    m_vfSliders.push_back(ui->slider_4);
    m_vfSliders.push_back(ui->slider_5);
    m_vfSliders.push_back(ui->slider_6);
    m_vfSliders.push_back(ui->slider_7);
    m_vfSliders.push_back(ui->slider_8);
    m_vfSliders.push_back(ui->slider_9);
    m_vfSliders.push_back(ui->slider_10);
    m_vfSliders.push_back(ui->slider_11);

    m_vfPoints.push_back(ui->freqPointLbl_1);
    m_vfPoints.push_back(ui->freqPointLbl_2);
    m_vfPoints.push_back(ui->freqPointLbl_3);
    m_vfPoints.push_back(ui->freqPointLbl_4);
    m_vfPoints.push_back(ui->freqPointLbl_5);
    m_vfPoints.push_back(ui->freqPointLbl_6);
    m_vfPoints.push_back(ui->freqPointLbl_7);
    m_vfPoints.push_back(ui->freqPointLbl_8);
    m_vfPoints.push_back(ui->freqPointLbl_9);
    m_vfPoints.push_back(ui->freqPointLbl_10);
    m_vfPoints.push_back(ui->freqPointLbl_11);

    m_offsetLabels.push_back(ui->offsetLbl_0);
    m_offsetLabels.push_back(ui->offsetLbl_1);
    m_offsetLabels.push_back(ui->offsetLbl_2);
    m_offsetLabels.push_back(ui->offsetLbl_3);
    m_offsetLabels.push_back(ui->offsetLbl_4);
    m_offsetLabels.push_back(ui->offsetLbl_5);

    m_offsetSliders.push_back(ui->offsetSlider_0);
    m_offsetSliders.push_back(ui->offsetSlider_1);
    m_offsetSliders.push_back(ui->offsetSlider_2);
    m_offsetSliders.push_back(ui->offsetSlider_3);
    m_offsetSliders.push_back(ui->offsetSlider_4);
    m_offsetSliders.push_back(ui->offsetSlider_5);


    for (int i = 0; i < 11; ++i) {

        connect(m_vfSliders[i], &QSlider::valueChanged,
            this, [this, i]() {
                onVfSliderValueChanged(m_vfSliders[i]->value(), i);
            });

        connect(m_vfEdits[i], &QLineEdit::textEdited,
            this, [this, i]() {
                onVfEditTextChanged(i);
            });

        auto *v1 = new QDoubleValidator(-307.2, 102.3, 1, m_vfEdits[i]); // not leaking.
        v1->setNotation(QDoubleValidator::StandardNotation);
        v1->setLocale(QLocale::C);
        m_vfEdits[i]->setValidator(v1);
    }

    ui->ringMaxEdit->setValidator(new QIntValidator(8, 99, ui->ringMaxEdit));
    ui->ringMinEdit->setValidator(new QIntValidator(8, 99, ui->ringMinEdit));

    for (int i = 0; i < 8; ++i) {
        m_pCoreEdits[i]->setValidator(new QIntValidator(8, 128, m_pCoreEdits[i]));
        m_eCoreEdits[i]->setValidator(new QIntValidator(8, 128, m_eCoreEdits[i]));
    }

    ui->eppEdit->setValidator(new QIntValidator(0, 256, ui->eppEdit));
    ui->hwpMinEdit->setValidator(new QIntValidator(0, 256, ui->hwpMinEdit));
    ui->hwpMaxEdit->setValidator(new QIntValidator(0, 256, ui->hwpMaxEdit));

    for (int i = 0; i < 6; ++i) {
        connect(m_offsetSliders[i], &QSlider::valueChanged,
        this, [this, i]() {
            onOffsetSliderValueChanged(m_offsetSliders[i]->value(), i);
        });
    }

    ui->cpuLabel->setText(QString(MSR::getCpuName().c_str()));

    connect(ui->vfResetBtn, &QPushButton::clicked,
        this, [this] {
            resetVF();
        });
    connect(ui->globalResetBtn_2, &QPushButton::clicked,
    this, [this] {
        resetGlobals();
    });

    connect(ui->applyBtn, &QPushButton::clicked,
        this, [this] {
            onApplyPressed();
        });

    auto* timer = new QTimer(this); // not leaked
    connect(timer, &QTimer::timeout,
        this, [this]() {
            updateStatString();
    });
    timer->start(1000);

    connect(ui->addPresetBtn, &QPushButton::clicked,
        this, [this]() {
            onSavePressed();
            readPresets();
            for (int i = 0; i < ui->presetCombo->count(); ++i) {
                QString text = ui->presetCombo->itemText(i);
                if (text == ui->presetNameEdit->text()) {
                    ui->presetCombo->setCurrentIndex(i);
                    ui->presetNameEdit->clear();
                    break;
                }
            }
        });

    connect(ui->delPresetBtn, &QPushButton::clicked,
        this, [this] {
            onDeletePressed();
            readPresets();
            ui->presetCombo->setCurrentIndex(0);
        });

    connect(ui->presetCombo, &QComboBox::activated,
    this, [this](const int index) {
        onPresetComboClicked(index);
    });

    const SetupPackage sPack = MsrOps::readCurrentAsPackage();
    readData(sPack);
    for (int i = 0; i < 6; ++i) {
        m_offsetLabels[i]->setText(QString::number(m_offsetSliders[i]->value() / 10, 'f', 1));
    }

    connect(ui->autostartCheck, &QCheckBox::toggled,
        this, [this](const bool checked) {
            onAutostartCheckClicked(checked);
        });

    updateStatString();
    readPresets();
    updateAutostartCheck();
}

void MainWindow::checkPresetAutostart() {
    QString autostart{};
    if (!getAutostartPreset(&autostart) || autostart.isEmpty())
        return;

    readPresets();
    auto pStr = autostart.toStdString();
    for (auto& pair : m_presetMap) {
        if (pStr == pair.first) {
            MsrOps::apply(pair.second);
            readData(pair.second);
            break;
        }
    }
}

void MainWindow::readData(const SetupPackage& pkg) const {

    const auto pGroup = pkg.getPCoreRatioGroups();
    const auto eGroup = pkg.getECoreRatioGroups();
    for (int i = 0; i < 8; ++i) {
        m_pCoreEdits[i]->setText(QString::number(pGroup[i]));
        m_eCoreEdits[i]->setText(QString::number(eGroup[i]));
    }

    for (int i = 0; i < 11; ++i) {
        const auto result =
            std::bit_cast<OcMailbox::OC_MAILBOX_MSR, uint64_t>(OcMailbox::readVFOffsetRaw(i + 1));
        const double offset = OcMailbox::convertOffsetFromRaw(result.VF.Offset);
        m_vfPoints[i]->setText(QString::number(result.VF.PointFrequency * 100));
        m_vfSliders[i]->setValue(static_cast<int>(offset * 10));
        m_vfEdits[i]->setText(QString::number(offset, 'f', 1));
    }

    ui->ringMaxEdit->setText(QString::number(pkg.RingMax.getValue()));
    ui->ringMinEdit->setText(QString::number(pkg.RingMin.getValue()));

    // globals
    ui->offsetSlider_0->setValue(std::lround(pkg.V_Offset_VCore.getValue() * 10));
    ui->offsetSlider_1->setValue(std::lround(pkg.V_Offset_IGpu.getValue() * 10));
    ui->offsetSlider_2->setValue(std::lround(pkg.V_Offset_Cache.getValue() * 10));
    ui->offsetSlider_3->setValue(std::lround(pkg.V_Offset_SystemAgent.getValue() * 10));
    ui->offsetSlider_4->setValue(std::lround(pkg.V_Offset_AnalogIO.getValue() * 10));
    ui->offsetSlider_5->setValue(std::lround(pkg.V_Offset_DigitalIO.getValue() * 10));

    ui->hwpCheck->setCheckState(pkg.HWP_IsSet ? Qt::Checked : Qt::Unchecked);
    ui->eppEdit->setText(QString::number(pkg.HWP_EPP.getValue()));
    ui->c1eCheck->setCheckState(pkg.EnhancedHalt_C1E.getValue() ? Qt::Checked : Qt::Unchecked);
    ui->eeoCheck->setCheckState(pkg.Disable_EE_Optimization.getValue() ? Qt::Unchecked : Qt::Checked );
    ui->hwpMinEdit->setText(QString::number(pkg.HWP_Minimum.getValue()));
    ui->hwpMaxEdit->setText(QString::number(pkg.HWP_Maximum.getValue()));
}

void MainWindow::updateStatString() {

    // Find physical cpus
    unsigned int logicalCores = std::thread::hardware_concurrency();
    if (logicalCores == 0) {
        QMessageBox::critical(nullptr,
                      "IntelMsrGui",
                      "System reports 0 logical cores.\n"
                      "This is not supposed to happen.");
        return;
    }

    std::vector<int> physicalCores;
    for (int i = 0; i < logicalCores; i++) {
        std::filesystem::path dir = std::format("/dev/cpu/{}", i);
        if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
            physicalCores.push_back(i);
        }
    }

    // Find which are P-Cores / E-Cores
    if (m_pCores.empty()) {
        m_pCores.clear();
        m_eCores.clear();
        for (auto i : physicalCores) {
            cpu_set_t set{};
            CPU_ZERO(&set); CPU_SET(i, &set);
            if (sched_setaffinity(0, sizeof(set), &set) != 0) { perror("affinity"); std::exit(1); }

            unsigned eax, ebx, ecx, edx;
            if (!__get_cpuid_count(0x1A, 0, &eax, &ebx, &ecx, &edx)) {
                m_pCores.push_back(i);
                continue;
            }

            auto core_type = static_cast<uint8_t>(eax >> 24);
            if (core_type == 0x20)
                m_eCores.push_back(i);
            else
                m_pCores.push_back(i);
        }
    }

    const auto pUnits{MSR::readAndReturn<MSR_RAPL_POWER_UNIT>(0, MSR_RAPL_POWER_UNIT_ADDR)};
    const auto power{MSR::readAndReturn<MSR_PKG_ENERGY_STATUS>(m_pCores[0], MSR_PKG_ENERGY_STATUS_ADDR)};
    const auto statsP0{MSR::readAndReturn<IA32_PERF_STATUS>(m_pCores[0], IA32_PERF_STATUS_ADDR)};
    const auto statsE0{MSR::readAndReturn<IA32_PERF_STATUS>(m_eCores[0], IA32_PERF_STATUS_ADDR)};
    const auto tempTarget{MSR::readAndReturn<MSR_TEMPERATURE_TARGET>(m_pCores[0], MSR_TEMPERATURE_TARGET_ADDR)};
    const auto pkgTherm{MSR::readAndReturn<IA32_PACKAGE_THERM_STATUS>(m_pCores[0], IA32_PACKAGE_THERM_STATUS_ADDR)};

    const std::string vid = std::format("VID: {:.3f}V", readVID(statsP0));
    const std::string pow = std::format("Power: {:.2f}W", getPkgPowerW(pUnits, power, 1));
    const std::string temp = std::format("Pkg Temp: {}°C", getCurPkgTemp(tempTarget, pkgTherm));
    const std::string pRatio = std::format("P-Core 0 Ratio: {}x", statsP0.CurrentFID);
    const std::string eRatio = std::format("E-Core 0 Ratio: {}x", statsE0.CurrentFID);
    ui->vidLabel->setText(vid.c_str());
    ui->powLabel->setText(pow.c_str());
    ui->tempLabel->setText(temp.c_str());
    ui->pCoreRatio->setText(pRatio.c_str());
    ui->eCoreRatio->setText(eRatio.c_str());
}

void MainWindow::readPresets() {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (!QDir().mkpath(dir)) {
        auto errStr = std::format("Could not create directory: {}", dir.toStdString());
        auto qErr = QString{errStr.c_str()};
        QMessageBox::critical(nullptr,
              "IntelMsrGui",
              qErr);
        return;
    }
    QString filePath = dir + "/preset.conf";
    m_presetMap.clear();
    if (MsrOps::readPresets(filePath.toStdString(), &m_presetMap)== IMT_ErrCode::OK) {
        ui->presetCombo->clear();
        for (const auto &str: m_presetMap | std::views::keys) {
            const QString itemStr = QString::fromStdString(str);
            const auto data = QVariant::fromValue(nullptr);
            ui->presetCombo->addItem(itemStr, data);
        }
    } else
        QMessageBox::critical(nullptr,
            "IntelMsrGui",
            "Could not read preset!");
}

void MainWindow::updateAutostartCheck() const {

    if (ui->presetCombo->currentIndex() < 0)
        return;

    QString autostartPreset{};
    if (getAutostartPreset(&autostartPreset) && autostartPreset == ui->presetCombo->currentText()) {
        ui->autostartCheck->setChecked(true);
    } else
        ui->autostartCheck->setChecked(false);
}

bool MainWindow::getAutostartPreset(QString* outStr) {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (!QDir().mkpath(dir)) {
        QMessageBox::critical(nullptr,
            "IntelMsrGui",
            "Could not read preset directory!");
        return false;
    }
    QString filePath = dir + "/autostartPreset.conf";

    std::filesystem::path presetPath = filePath.toStdString();
    std::ifstream inFile{presetPath};
    if (!inFile) {
        QMessageBox::critical(nullptr,
            "IntelMsrGui",
            "Could not read preset file!");
        return false;
    }

    std::string line;
    if (!std::getline(inFile, line)) {
        return false;
    }
    outStr->append(line);

    return true;
}

bool MainWindow::setAutostartPreset(const QString &presetName) {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (!QDir().mkpath(dir)) {
        QMessageBox::critical(nullptr,
            "IntelMsrGui",
            "Could not write to preset file directory!");
        return false;
    }

    QString filePath = dir + "/autostartPreset.conf";
    std::filesystem::path presetPath = filePath.toStdString();
    std::ofstream outFile{presetPath};
    if (!outFile) {
        QMessageBox::critical(nullptr,
            "IntelMsrGui",
            "Could not write preset file!");
        return false;
    }

    outFile.clear();

    outFile << presetName.toStdString();

    return true;
}

bool MainWindow::validateEntries(SetupPackage* outPkg) const {
    auto vfOffsets = std::vector<double>(11);
    for (int i = 0; i < 11; ++i) {
        bool ok = false;
        const auto dbl = m_vfEdits[i]->text().toDouble(&ok);
        if (!ok || dbl < -300.0 || dbl > 100.0)
            return false;

        vfOffsets[i] = dbl;
    }
    outPkg->setVfCoreOffsets(vfOffsets);

    auto globalOffsets = std::vector<double>(6);
    for (int i = 0; i < 6; ++i) {
        bool ok = false;
        const auto dbl = m_offsetLabels[i]->text().toDouble(&ok);
        if (!ok || dbl < -300.0 || dbl > 100.0)
            return false;

        globalOffsets[i] = dbl;
    }
    outPkg->setGlobalOffsets(globalOffsets);

    auto pRatios = std::vector<uint64_t>(8);
    auto eRatios = std::vector<uint64_t>(8);
    for (int i = 0; i < 8; ++i) {
        bool ok = false;
        const auto pValue = m_pCoreEdits[i]->text().toInt(&ok);
        if (!ok || pValue> 99)
            return false;

        pRatios[i] = pValue;

        ok = false;
        const auto eValue = m_eCoreEdits[i]->text().toInt(&ok);
        if (!ok || eValue> 99)
            return false;

        eRatios[i] = eValue;
    }
    outPkg->setPCoreRatioGroups(pRatios);
    outPkg->setECoreRatioGroups(eRatios);

    bool eppOk = false;
    const auto epp= ui->eppEdit->text().toInt(&eppOk);
    if (!eppOk || epp < 0 || epp > 255)
        return false;
    outPkg->HWP_EPP.set(epp);
    outPkg->EnhancedHalt_C1E.set(ui->c1eCheck->checkState() == Qt::Checked);
    outPkg->Disable_EE_Optimization.set(ui->eeoCheck->checkState() == Qt::Unchecked);

    return true;
}

void MainWindow::onVfSliderValueChanged(const int value, const int vfPoint) const {

    auto* lEdit = m_vfEdits[vfPoint];
    const double dVal = OcMailbox::convertToVidOffset(value / 10);
    lEdit->setText(QString::number(dVal, 'f', 1));
}

void MainWindow::onVfEditTextChanged(const int vfPoint) const {
    const auto* edit = m_vfEdits[vfPoint];
    auto* slider = m_vfSliders[vfPoint];

    bool ok = false;
    int value = static_cast<int>(edit->text().toDouble(&ok));
    if (!ok) {
        value = 0;
    }
    slider->setValue(value * 10);
}

void MainWindow::onOffsetSliderValueChanged(const int value, const int domain) const {
    auto* label = m_offsetLabels[domain];
    const double dVal = value / 10.0;
    label->setText(QString::number(dVal, 'f', 1));
}

void MainWindow::resetGlobals() const {
    for (int i = 0; i < 6; ++i) {
        m_offsetSliders[i]->setValue(0);
        m_offsetLabels[i]->setText(QString::number(0.0, 'f', 1));
    }
}

void MainWindow::resetVF() const {
    for (int i = 0; i < 11; ++i) {
        m_vfSliders[i]->setValue(0);
        m_vfEdits[i]->setText(QString::number(0.0, 'f', 1));
    }
}

void MainWindow::onApplyPressed() const {
    SetupPackage pkg{};
    pkg.NumECores = m_eCores.size();
    pkg.NumPCores = m_pCores.size();
    pkg.ApplyClocksFirst = ui->clocksFirstCheck->checkState();
    pkg.HWP_IsSet = ui->hwpCheck->checkState();
    pkg.HWP_Desired.set(0);
    pkg.HWP_Minimum.set(1);
    pkg.HWP_Maximum.set(255);
    pkg.RingMin.set(ui->ringMinEdit->text().toInt());
    pkg.RingMax.set(ui->ringMaxEdit->text().toInt());
    if (validateEntries(&pkg)) {
        if (MsrOps::apply(pkg) != IMT_ErrCode::OK) {
            QMessageBox::critical(nullptr,
            "IntelMsrGui",
            "MSRs not written correctly.");
        }
        const SetupPackage sPack = MsrOps::readCurrentAsPackage();
        readData(sPack);
    } else
        QMessageBox::critical(nullptr,
        "IntelMsrGui",
        "Entries failed validation.");
}

void MainWindow::onSavePressed() const {

    if (ui->presetNameEdit->text().isEmpty()) {
        QMessageBox::critical(nullptr,
        "IntelMsrGui",
        "Must enter Preset name.");
        return;
    }

    SetupPackage pkg{};
    pkg.NumECores = m_eCores.size();
    pkg.NumPCores = m_pCores.size();
    pkg.ApplyClocksFirst = ui->clocksFirstCheck->checkState();
    pkg.HWP_IsSet = ui->hwpCheck->checkState();
    pkg.HWP_Desired.set(0);
    pkg.HWP_Minimum.set(1);
    pkg.HWP_Maximum.set(255);
    pkg.RingMin.set(ui->ringMinEdit->text().toInt());
    pkg.RingMax.set(ui->ringMaxEdit->text().toInt());
    if (!validateEntries(&pkg)) {
        QMessageBox::critical(nullptr,
            "IntelMsrGui",
            "Entries failed validation.");
        return;
    }

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (!QDir().mkpath(dir)) {
        QMessageBox::critical(nullptr,
             "IntelMsrGui",
             "Could not write preset directory.");
        return;
    }
    QString filePath = dir + "/preset.conf";
    if (MsrOps::saveCurrentSetting(
        filePath.toStdString(),
        ui->presetNameEdit->text().toStdString(),
        pkg) != IMT_ErrCode::OK)
    {
        QMessageBox::critical(nullptr,
            "IntelMsrGui",
            "Could not write preset.");
    }

}

void MainWindow::onDeletePressed() const {

    if (ui->presetCombo->currentIndex() < 0 || m_presetMap.size() < 1)
        return;

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (!QDir().mkpath(dir)) {
        return; // todo
    }
    QString filePath = dir + "/preset.conf";
    if (MsrOps::deletePreset(filePath.toStdString(),
        ui->presetCombo->currentText().toStdString()) != IMT_ErrCode::OK) {
        std::cout << "Error deleting.";
    }
}

void MainWindow::onPresetComboClicked(int index) const {
    if (index == -1)
        return;

    ui->presetCombo->setCurrentIndex(index);
    const std::string str = ui->presetCombo->currentText().toStdString();
    const SetupPackage pkg = m_presetMap.at(str);
    readData(pkg);
    updateAutostartCheck();
}

void MainWindow::onAutostartCheckClicked(bool checked) const {

    if (ui->presetCombo->currentIndex() < 0 || m_presetMap.size() < 1)
        ui->autostartCheck->setChecked(false);

    if (checked && ui->presetCombo->currentIndex() >= 0) {
        setAutostartPreset(ui->presetCombo->currentText());
        ui->autostartCheck->setChecked(true);
    } else if (!checked && ui->presetCombo->currentIndex() >= 0) {
        setAutostartPreset("");
    }
}


