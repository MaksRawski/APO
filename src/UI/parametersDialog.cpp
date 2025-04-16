#include "parametersDialog.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <opencv2/core/base.hpp>

namespace {
QLineEdit *createValidatedLineEdit(QWidget *parent, int min, int max, int initialValue) {
  QLineEdit *lineEdit = new QLineEdit(parent);
  QIntValidator *validator = new QIntValidator(min, max, parent);
  lineEdit->setValidator(validator);
  lineEdit->setText(QString::number(initialValue));
  return lineEdit;
}

QDialogButtonBox *createDialogButtons(QDialog *dialog) {
  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
  return buttonBox;
}
const std::vector<uchar> sizes{1, 3, 5, 7};
QComboBox *createKernelComboBox(QDialog *parent) {
  QComboBox *cbKernelSize = new QComboBox(parent);
  for (uchar size : sizes)
    cbKernelSize->addItem(QString("%1x%1").arg(size), size);
  cbKernelSize->setCurrentIndex(1);
  return cbKernelSize;
}
struct BorderInfo {
  int value;
  const char *description;
};

const std::vector<BorderInfo> allBorders = {
    {cv::BORDER_CONSTANT, "Constant (000000|abcdefgh|0000000)"},
    {cv::BORDER_REPLICATE, "Replicate (aaaaaa|abcdefgh|hhhhhhh)"},
    {cv::BORDER_REFLECT, "Reflect (fedcba|abcdefgh|hgfedcb)"},
    {cv::BORDER_REFLECT_101, "Reflect 101 (gfedcb|abcdefgh|gfedcba)"},
    {cv::BORDER_ISOLATED, "Isolated"}};

QComboBox *createBorderTypeComboBox(QDialog *parent) {
  QComboBox *cbBorderType = new QComboBox(parent);
  for (BorderInfo border : allBorders)
    cbBorderType->addItem(border.description, border.value);
  cbBorderType->setCurrentIndex(3);
  return cbBorderType;
}

const std::vector<Mask3x3> laplacianMasks{{{{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}}},
                                          {{{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}}},
                                          {{{1, -2, 1}, {-2, 5, -2}, {1, -2, 1}}}};

std::tuple<QGridLayout *, std::array<std::array<QLineEdit *, 3>, 3>>
createMaskEditor(QDialog *parent) {
  QGridLayout *gridLayout = new QGridLayout();
  std::array<std::array<QLineEdit *, 3>, 3> lineEdits;
  QDoubleValidator *validator = new QDoubleValidator(-100.0, 100.0, 2, parent);

  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      QLineEdit *edit = new QLineEdit("0.0", parent);
      edit->setFixedWidth(50);
      edit->setValidator(validator);
      gridLayout->addWidget(edit, row, col);
      lineEdits[row][col] = edit;
    }
  }
  return std::make_tuple(gridLayout, lineEdits);
}
void setMask(std::array<std::array<QLineEdit *, 3>, 3> lineEdits, const Mask3x3 &mask) {
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      lineEdits[row][col]->setText(QString::number(mask[row][col]));
    }
  }
}
std::optional<Mask3x3> getMask(std::array<std::array<QLineEdit *, 3>, 3> lineEdits) {
  Mask3x3 kernel;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      bool conversionOk;
      double val = lineEdits[i][j]->text().toDouble(&conversionOk);
      if (!conversionOk)
        return std::nullopt;
      kernel[i][j] = val;
    }
  }
  return kernel;
}

const std::vector<Mask3x3> prewittMasks = {
    // N
    {{{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}}},
    // NE
    {{{0, -1, -1}, {1, 0, -1}, {1, 1, 0}}},
    // E
    {{{1, 0, -1}, {1, 0, -1}, {1, 0, -1}}},
    // SE
    {{{1, 1, 0}, {1, 0, -1}, {0, -1, -1}}},
    // S
    {{{1, 1, 1}, {0, 0, 0}, {-1, -1, -1}}},
    // SW
    {{{0, 1, 1}, {-1, 0, 1}, {-1, -1, 0}}},
    // W
    {{{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}}},
    // NW
    {{{-1, -1, 0}, {-1, 0, 1}, {0, 1, 1}}}};
} // namespace

std::optional<std::tuple<uchar, uchar, uchar, uchar>>
rangeStretchDialog(QWidget *parent, uchar initialP1, uchar initialP2, uchar initialQ3,
                   uchar initialQ4) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select ranges");

  QFormLayout formLayout(&dialog);
  auto *p1Edit = createValidatedLineEdit(&dialog, 0, 255, initialP1);
  auto *p2Edit = createValidatedLineEdit(&dialog, 0, 255, initialP2);
  auto *q3Edit = createValidatedLineEdit(&dialog, 0, 255, initialQ3);
  auto *q4Edit = createValidatedLineEdit(&dialog, 0, 255, initialQ4);

  formLayout.addRow("p1 (0-255)", p1Edit);
  formLayout.addRow("p2 (0-255)", p2Edit);
  formLayout.addRow("q3 (0-255)", q3Edit);
  formLayout.addRow("q4 (0-255)", q4Edit);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    bool ok1, ok2, ok3, ok4;
    uchar p1 = static_cast<uchar>(p1Edit->text().toInt(&ok1));
    uchar p2 = static_cast<uchar>(p2Edit->text().toInt(&ok2));
    uchar q3 = static_cast<uchar>(q3Edit->text().toInt(&ok3));
    uchar q4 = static_cast<uchar>(q4Edit->text().toInt(&ok4));
    if (ok1 && ok2 && ok3 && ok4)
      return std::make_tuple(p1, p2, q3, q4);
    QMessageBox::critical(nullptr, "Error",
                          "Invalid input. Please enter integers between 0 and 255.");
  }

  return std::nullopt;
}

std::optional<uchar> posterizeDialog(QWidget *parent, uchar N) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Enter number of levels");

  QFormLayout formLayout(&dialog);
  auto *edit = createValidatedLineEdit(&dialog, 1, 255, N);
  formLayout.addRow("N", edit);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    bool ok;
    uchar n = static_cast<uchar>(edit->text().toInt(&ok));
    if (!ok) {
      QMessageBox::critical(parent, "Error",
                            "Invalid input. Please enter a number between 1 and 255.");
      return std::nullopt;
    }
    return n;
  }

  return std::nullopt;
}

// returns kernel size and border type
std::optional<std::tuple<uchar, int>> kernelSizeDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select kernel size");

  QFormLayout formLayout(&dialog);
  auto *cbKernelSize = createKernelComboBox(&dialog);
  auto *cbBorderType = createBorderTypeComboBox(&dialog);

  formLayout.addRow("Kernel size", cbKernelSize);
  formLayout.addRow("Border type", cbBorderType);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted && cbKernelSize->currentIndex() > -1 &&
      cbBorderType->currentIndex() > -1) {
    return std::make_tuple(sizes[cbKernelSize->currentIndex()],
                           allBorders[cbBorderType->currentIndex()].value);
  }

  QMessageBox::critical(parent, "Error", "Invalid input. Please select a kernel size.");
  return std::nullopt;
}

std::optional<std::tuple<uchar, double, int>> gaussianBlurDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select kernel size and sigma");

  QFormLayout formLayout(&dialog);
  auto *cbKernelSize = createKernelComboBox(&dialog);
  auto *cbBorderType = createBorderTypeComboBox(&dialog);

  QLineEdit sigmaEdit;
  QDoubleValidator doubleValidator(0.0, 100.0, 2, &dialog);
  sigmaEdit.setValidator(&doubleValidator);
  sigmaEdit.setText("1.0");

  formLayout.addRow("Kernel size", cbKernelSize);
  formLayout.addRow("Border type", cbBorderType);
  formLayout.addRow("σ (std dev)", &sigmaEdit);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    bool ok;
    double sigma = sigmaEdit.text().toDouble(&ok);
    if (cbKernelSize->currentIndex() > -1 && cbBorderType->currentIndex() > -1 && ok) {
      return std::make_tuple(sizes[cbKernelSize->currentIndex()], sigma,
                             allBorders[cbBorderType->currentIndex()].value);
    }
    QMessageBox::critical(nullptr, "Error", "Invalid input. Please enter a valid sigma.");
  }

  return std::nullopt;
}

std::optional<std::tuple<uchar, Direction, int>> sobelDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select kernel size and direction");

  QFormLayout formLayout(&dialog);
  auto *cbKernelSize = createKernelComboBox(&dialog);
  auto *cbBorderType = createBorderTypeComboBox(&dialog);

  QComboBox cbDir;
  cbDir.addItem(QString("Horizontal"));
  cbDir.addItem(QString("Vertical"));
  cbDir.setCurrentIndex(0);

  formLayout.addRow("Kernel size", cbKernelSize);
  formLayout.addRow("Border type", cbBorderType);
  formLayout.addRow("Direction", &cbDir);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    if (cbKernelSize->currentIndex() > -1 && cbDir.currentIndex() > -1 &&
        cbBorderType->currentIndex() > -1) {
      Direction dir = cbDir.currentIndex() == 0 ? Direction::Horizontal : Direction::Vertical;
      return std::make_tuple(sizes[cbKernelSize->currentIndex()], dir,
                             allBorders[cbBorderType->currentIndex()].value);
    }
    QMessageBox::critical(nullptr, "Error", "Invalid input.");
  }

  return std::nullopt;
}

// returns kernel size, start, end, border type
std::optional<std::tuple<uchar, uchar, uchar, int>> cannyDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select threshold");

  QFormLayout formLayout(&dialog);
  auto *startLineEdit = createValidatedLineEdit(&dialog, 0, 255, 100);
  auto *endLineEdit = createValidatedLineEdit(&dialog, 0, 255, 200);
  auto *cbKernelSize = createKernelComboBox(&dialog);
  auto *cbBorderType = createBorderTypeComboBox(&dialog);

  formLayout.addRow("Kernel size", cbKernelSize);
  formLayout.addRow("Border type", cbBorderType);
  formLayout.addRow("Start (0-255)", startLineEdit);
  formLayout.addRow("End (0-255)", endLineEdit);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    bool ok1, ok2;
    uchar start = static_cast<uchar>(startLineEdit->text().toInt(&ok1));
    uchar end = static_cast<uchar>(endLineEdit->text().toInt(&ok2));
    if (cbKernelSize->currentIndex() > -1 && cbBorderType->currentIndex() > -1 && ok1 && ok2) {
      uchar k = sizes[cbKernelSize->currentIndex()];
      int borderType = allBorders[cbBorderType->currentIndex()].value;
      return std::make_tuple(k, start, end, borderType);
    }
    QMessageBox::critical(parent, "Error",
                          "Invalid input. Please enter integers between 0 and 255.");
  }

  return std::nullopt;
}

std::optional<std::tuple<Mask3x3, int>> laplacianMaskDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select Laplacian mask");

  QFormLayout formLayout(&dialog);
  QComboBox *cbMask = new QComboBox(&dialog);
  cbMask->addItem("1.");
  cbMask->addItem("2.");
  cbMask->addItem("3.");
  cbMask->setCurrentIndex(0);

  auto *cbBorderType = createBorderTypeComboBox(&dialog);

  auto maskEditor = createMaskEditor(&dialog);
  QGridLayout *maskViewerGrid;
  std::array<std::array<QLineEdit *, 3>, 3> maskViewerLineEdits;
  std::tie(maskViewerGrid, maskViewerLineEdits) = maskEditor;
  setMask(maskViewerLineEdits, laplacianMasks[0]);

  QObject::connect(cbMask, &QComboBox::currentIndexChanged, [&maskViewerLineEdits](int index) {
    setMask(maskViewerLineEdits, laplacianMasks[index]);
  });

  formLayout.addRow("Mask", cbMask);
  formLayout.addRow("Border type", cbBorderType);
  formLayout.addItem(maskViewerGrid);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    auto mask = getMask(maskViewerLineEdits);
    if (cbMask->currentIndex() > -1 && cbBorderType->currentIndex() > -1 && mask.has_value()) {
      return std::make_tuple(mask.value(), allBorders[cbBorderType->currentIndex()].value);
    }
    QMessageBox::critical(nullptr, "Error", "Invalid input.");
  }
  return std::nullopt;
}

std::optional<std::tuple<Mask3x3, int>> prewittDirection(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select prewitt direction");

  QFormLayout formLayout(&dialog);
  QComboBox *cbDir = new QComboBox(&dialog);
  cbDir->addItem("N");
  cbDir->addItem("NE");
  cbDir->addItem("E");
  cbDir->addItem("SE");
  cbDir->addItem("S");
  cbDir->addItem("SW");
  cbDir->addItem("W");
  cbDir->addItem("NW");
  cbDir->setCurrentIndex(0);

  auto *cbBorderType = createBorderTypeComboBox(&dialog);

  auto maskEditor = createMaskEditor(&dialog);
  QGridLayout *maskViewerGrid;
  std::array<std::array<QLineEdit *, 3>, 3> maskViewerLineEdits;
  std::tie(maskViewerGrid, maskViewerLineEdits) = maskEditor;
  setMask(maskViewerLineEdits, prewittMasks[0]);

  QObject::connect(cbDir, &QComboBox::currentIndexChanged, [&maskViewerLineEdits](int index) {
    setMask(maskViewerLineEdits, prewittMasks[index]);
  });

  formLayout.addRow("Direction", cbDir);
  formLayout.addRow("Border type", cbBorderType);
  formLayout.addItem(maskViewerGrid);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    auto mask = getMask(maskViewerLineEdits);
    if (cbDir->currentIndex() > -1 && cbBorderType->currentIndex() > -1 && mask.has_value()) {
      return std::make_tuple(mask.value(), allBorders[cbBorderType->currentIndex()].value);
    }
    QMessageBox::critical(parent, "Error", "Invalid input.");
  }
  return std::nullopt;
}


// returns indexes of the chosen windows names
std::optional<std::tuple<uchar, uchar>> windowsPairDialog(QWidget *parent,
                                                          std::vector<QString> names, uchar first) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select two windows");

  QFormLayout formLayout(&dialog);
  QComboBox *cbFirst = new QComboBox(&dialog);
  QComboBox *cbSecond = new QComboBox(&dialog);
  cbFirst->setCurrentIndex(first);

  for (QString name : names) {
    cbFirst->addItem(name);
    cbSecond->addItem(name);
  }

  formLayout.addRow("First image", cbFirst);
  formLayout.addRow("Second image", cbSecond);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    if (cbFirst->currentIndex() > -1 && cbSecond->currentIndex() > -1) {
      return std::make_tuple(cbFirst->currentIndex(), cbSecond->currentIndex());
    }
    QMessageBox::critical(parent, "Error", "Invalid input.");
  }
  return std::nullopt;
}
