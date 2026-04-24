#include "MainWindow.h"

#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <QVBoxLayout>

#include <vtkInteractorStyleImage.h>
#include <vtkCommand.h>

#include <gdcmImageReader.h>
#include <gdcmImage.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setupVTK();

    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
}

void MainWindow::setupVTK()
{
    vtkWidget = new QVTKOpenGLNativeWidget(ui.vtkWidget);

    QVBoxLayout* layout = new QVBoxLayout(ui.vtkWidget);
    layout->addWidget(vtkWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    ui.vtkWidget->setLayout(layout);

    auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    auto interactor = vtkWidget->interactor();
    auto style = vtkSmartPointer<vtkInteractorStyleImage>::New();
    interactor->SetInteractorStyle(style);

    sliceLabel = new QLabel(vtkWidget);
    sliceLabel->setStyleSheet(
        "QLabel { color: white; background-color: rgba(0,0,0,150); padding: 4px; }"
    );
    sliceLabel->adjustSize();
    sliceLabel->move(10, 10);
    sliceLabel->raise();
    sliceLabel->setText("Slice: -");

    startNewMeasurement();

    distanceWidget->AddObserver(
        vtkCommand::EndInteractionEvent,
        this,
        &MainWindow::onMeasurementFinished
    );

    interactor->AddObserver(
        vtkCommand::LeftButtonPressEvent,
        this,
        &MainWindow::onLeftClick
    );
}

void MainWindow::startNewMeasurement()
{
    if (distanceWidget)
        distanceWidget->EnabledOff();

    distanceWidget = vtkSmartPointer<vtkDistanceWidget>::New();
    distanceWidget->SetInteractor(vtkWidget->interactor());

    auto rep = vtkSmartPointer<vtkDistanceRepresentation2D>::New();
    distanceWidget->SetRepresentation(rep);

    distanceWidget->CreateDefaultRepresentation();
    distanceWidget->EnabledOn();

    distanceWidget->AddObserver(
        vtkCommand::EndInteractionEvent,
        this,
        &MainWindow::onMeasurementFinished
    );
}

void MainWindow::onMeasurementFinished(vtkObject*, unsigned long, void*)
{
    measurementDone = true;
}

void MainWindow::onLeftClick(vtkObject*, unsigned long, void*)
{
    if (measurementDone)
    {
        startNewMeasurement();
        measurementDone = false;
    }
}

void MainWindow::loadDicom(const QString& path)
{
    gdcm::ImageReader reader;
    reader.SetFileName(path.toStdString().c_str());

    if (!reader.Read())
    {
        qDebug() << "Failed to read DICOM";
        return;
    }

    const gdcm::Image& gdcmImage = reader.GetImage();

    dims[0] = gdcmImage.GetDimension(0);
    dims[1] = gdcmImage.GetDimension(1);
    dims[2] = gdcmImage.GetDimension(2);

    auto pixelFormat = gdcmImage.GetPixelFormat();
    samples = pixelFormat.GetSamplesPerPixel();
    int bits = pixelFormat.GetBitsAllocated();
    bytesPerPixel = bits / 8;

    buffer.resize(gdcmImage.GetBufferLength());
    gdcmImage.GetBuffer(buffer.data());

    image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(dims[0], dims[1], 1);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, samples);

    currentSlice = 0;

    startNewMeasurement();
    measurementDone = false;

    showSlice(currentSlice);
    renderer->ResetCamera();
    vtkWidget->renderWindow()->Render();
}

void MainWindow::showSlice(int index)
{
    if (!image) return;

    size_t frameSize = dims[0] * dims[1] * bytesPerPixel * samples;
    size_t offset = index * frameSize;

    memcpy(
        image->GetScalarPointer(),
        buffer.data() + offset,
        frameSize
    );

    image->Modified();

    if (!imageActor)
    {
        imageActor = vtkSmartPointer<vtkImageActor>::New();
        renderer->AddActor(imageActor);
    }

    imageActor->SetInputData(image);

    startNewMeasurement();
    measurementDone = false;

    updateSliceLabel();

    vtkWidget->renderWindow()->Render();
}

void MainWindow::updateSliceLabel()
{
    if (dims[2] > 0)
    {
        QString text = QString("Slice: %1 / %2")
            .arg(currentSlice + 1)
            .arg(dims[2]);

        sliceLabel->setText(text);
        sliceLabel->adjustSize();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    auto urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;

    QString filePath = urls.first().toLocalFile();
    loadDicom(filePath);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (!image) return;

    if (event->key() == Qt::Key_Up)
        currentSlice++;
    else if (event->key() == Qt::Key_Down)
        currentSlice--;

    if (currentSlice < 0) currentSlice = 0;
    if (currentSlice >= (int)dims[2]) currentSlice = dims[2] - 1;

    showSlice(currentSlice);
}