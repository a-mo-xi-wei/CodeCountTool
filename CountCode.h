#ifndef COUNTCODE_H
#define COUNTCODE_H

#include <QWidget>

namespace Ui {
class CountCode;
}

class CountCode : public QWidget
{
    Q_OBJECT

public:
    explicit CountCode(QWidget *parent = 0);
    ~CountCode();

private:
    Ui::CountCode *ui;
    QStringList listFile;

private:
    void initForm();
    bool checkFile(const QString &fileName);
    bool checkDir(const QString& dirName);
    void countCode(const QString &filePath);
    void countCode(const QStringList &files);
    void countCode(const QString &fileName, int &lineCode, int &lineBlank, int &lineNotes);

private slots:
    void on_btnOpenFile_clicked();
    void on_btnOpenPath_clicked();
    void on_btnClear_clicked();
};

#endif // OUNTCODE_H
