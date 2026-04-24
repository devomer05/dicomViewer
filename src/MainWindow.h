#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>
#include <QLabel>

#include <QVTKOpenGLNativeWidget.h>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::MainWindow ui;

    void setupVTK();
    void loadDicom(const QString& path);
    void showSlice(int index);
    void startNewMeasurement();
    void updateSliceLabel();

    // callbacks
    void onMeasurementFinished(vtkObject*, unsigned long, void*);
    void onLeftClick(vtkObject*, unsigned long, void*);

    // UI
    QVTKOpenGLNativeWidget* vtkWidget;
    QLabel* sliceLabel;

    // VTK
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkImageActor> imageActor;
    vtkSmartPointer<vtkImageData> image;
    vtkSmartPointer<vtkDistanceWidget> distanceWidget;

    // data
    std::vector<char> buffer;
    unsigned int dims[3];
    int samples;
    int bytesPerPixel;

    int currentSlice = 0;

    bool measurementDone = false;
};