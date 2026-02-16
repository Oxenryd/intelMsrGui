//
// Created by oxenryd on 2025-11-15.
//

#ifndef INTEL_MSR_GUI_MAINWINDOW_H
#define INTEL_MSR_GUI_MAINWINDOW_H

#include <QMainWindow>
#include <vector>

#include "msr/Parameters.hpp"
#include "msr/IMT_ErrCode.h"
#include "msr/IntelTypes.hpp"


class QLineEdit;
class QLabel;
class QSlider;
class QTimer;

namespace Ui {
    class MainWindow;
}

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void updateTrayMenu(QMenu* menu);
    void onShow();
    void onHide();

private:
    Ui::MainWindow* ui;
    std::vector<QLineEdit*> m_vfEdits;
    std::vector<QSlider*> m_vfSliders;
    std::vector<QLabel*> m_vfPoints;
    std::vector<QSlider*> m_offsetSliders;
    std::vector<QLabel*> m_offsetLabels;

    std::vector<int> m_pCores;
    std::vector<int> m_eCores;
    std::vector<QLineEdit*> m_pCoreEdits;
    std::vector<QLineEdit*> m_eCoreEdits;

    QTimer* m_statsTimer{nullptr};

    std::unordered_map<std::string, SetupPackage> m_presetMap;

    void init();
    void checkPresetAutostart();
    bool validateEntries(SetupPackage* outPkg) const;
    void setUiElementsFromData(const SetupPackage& pkg, bool updateVfFreqs = true) const;
    void updateStatString();
    void readPresets();
    void updateAutostartCheck() const;
    static bool getAutostartPreset(QString* outStr);
    static bool setAutostartPreset(const QString& presetName);

    void getPackSetUI() const;

    IMT_ErrCode sendSettingsPackage(const SetupPackage& pkg);
    static bool getSettingsPackage(SetupPackage* outPkg);
    static bool getStatusPackage(StatusPackage* outPkg);

    static std::string getCpuName();

private Q_SLOTS:
    void onVfSliderValueChanged(int value, int vfPoint) const;
    void onVfEditTextChanged(int vfPoint) const;
    void onOffsetSliderValueChanged(int value, int domain) const;
    void resetGlobals() const;
    void resetVF() const;
    void onApplyPressed();
    void onSavePressed() const;
    void onDeletePressed() const;
    void onPresetComboClicked(int index) const;
    void onAutostartCheckClicked(bool checked) const;

};

#endif //INTEL_MSR_GUI_MAINWINDOW_H