#ifndef COUNTCODE_H
#define COUNTCODE_H

#include <QWidget>

namespace Ui {
    class CountCode;
}

class CountCode : public QWidget {
    Q_OBJECT

public:
    explicit CountCode(QWidget *parent = 0);

    ~CountCode();

private:
    void initForm() const;

    bool checkFile(const QString &fileName) const;

    bool checkDir(const QString &dirName) const;

    void countCode(const QString &dirPath);

    void countCode(const QStringList &files);

    void countCode(const QString &fileName, int &lineCode, int &lineBlank, int &lineNotes);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onBtnOpenFileClicked();

    void onBtnOpenPathClicked();

    void onBtnClearClicked();

    void onEditingFinished();

private:
    Ui::CountCode *ui;
    QStringList listFile;
};

#endif // OUNTCODE_H
