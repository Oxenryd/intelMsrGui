//
// Created by oxenryd on 2025-11-15.
//

#ifndef INTEL_MSR_GUI_MAINWINDOW_H
#define INTEL_MSR_GUI_MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "msr/Parameters.hpp"


class QLineEdit;
class QLabel;
class QSlider;

namespace Ui {
    class MainWindow;
}

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

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

    std::unordered_map<std::string, SetupPackage> m_presetMap;

    void init();
    bool validateEntries(SetupPackage* outPkg) const;
    void readData(const SetupPackage& pkg) const;
    void updateStatString();
    void readPresets();

private Q_SLOTS:
    void onVfSliderValueChanged(int value, int vfPoint) const;
    void onVfEditTextChanged(int vfPoint) const;
    void onOffsetSliderValueChanged(int value, int domain) const;
    void resetGlobals() const;
    void resetVF() const;
    void onApplyPressed() const;
    void onSavePressed() const;
    void onDeletePressed() const;
    void onPresetComboClicked(int index) const;
};

#endif //INTEL_MSR_GUI_MAINWINDOW_H