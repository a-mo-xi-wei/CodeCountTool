#pragma execution_character_set("utf-8")

#include "CountCode.h"
#include "ui_CountCode.h"

#include<QFileDialog>
#include<QTextStream>
#include<QFile>
#include<QPainter>

CountCode::CountCode(QWidget *parent) : QWidget(parent), ui(new Ui::CountCode)
{
    ui->setupUi(this);
    {
        QFile file(":/Res/style.css");
        if (file.open(QIODevice::ReadOnly)) {
            this->setStyleSheet(file.readAll());
        } else {
            qDebug() << "样式表打开失败QAQ";
            return;
        }
    }
    this->initForm();
    connect(ui->btnOpenFile,&QPushButton::clicked,this,&CountCode::onBtnOpenFileClicked);
    connect(ui->btnOpenPath,&QPushButton::clicked,this,&CountCode::onBtnOpenPathClicked);
    connect(ui->btnClear,&QPushButton::clicked,this,&CountCode::onBtnClearClicked);
    connect(ui->txtFilter, &QLineEdit::editingFinished, this, &CountCode::onEditingFinished);
    connect(ui->dirFilter, &QLineEdit::editingFinished, this, &CountCode::onEditingFinished);
    ui->btnClear->clicked();
}

CountCode::~CountCode()
{
    delete ui;
}

void CountCode::initForm() const {
    QStringList headText;
    headText << "文件名" << "类型" << "大小" << "总行数" << "代码行数" << "注释行数" << "空白行数" << "路径";
    QList<int> columnWidth;
    columnWidth << 130 << 50 << 70 << 80 << 70 << 70 << 70 << 150;

    int columnCount = static_cast<int>(headText.count());
    ui->tableWidget->setColumnCount(columnCount);
    ui->tableWidget->setHorizontalHeaderLabels(headText);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setHighlightSections(false);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(20);
    ui->tableWidget->verticalHeader()->setHighlightSections(false);

    for (int i = 0; i < columnCount; i++) {
        ui->tableWidget->setColumnWidth(i, columnWidth.at(i));
    }

    //设置前景色
    ui->txtCount->setStyleSheet(QStringLiteral("color:#17A086;"));
    ui->txtSize->setStyleSheet(QStringLiteral("color:#CA5AA6;"));
    ui->txtRow->setStyleSheet(QStringLiteral("color:#CD1B19;"));
    ui->txtCode->setStyleSheet(QStringLiteral("color:#22A3A9;"));
    ui->txtNote->setStyleSheet(QStringLiteral("color:#D64D54;"));
    ui->txtBlank->setStyleSheet(QStringLiteral("color:#A279C5;"));

    //设置字体加粗
    QFont font;
    font.setBold(true);
    if (font.pointSize() > 0) {
        font.setPointSize(font.pointSize() + 1);
    } else {
        font.setPixelSize(font.pixelSize() + 2);
    }

    ui->txtCount->setFont(font);
    ui->txtSize->setFont(font);
    ui->txtRow->setFont(font);
    ui->txtCode->setFont(font);
    ui->txtNote->setFont(font);
    ui->txtBlank->setFont(font);

#if (QT_VERSION > QT_VERSION_CHECK(4,7,0))
    ui->txtFilter->setPlaceholderText("中间空格隔开,例如 *.h *.cpp *.c");
#endif
}

bool CountCode::checkFile(const QString &fileName) const {
    if (fileName.startsWith("moc_") || fileName.startsWith("mocs_") || fileName.startsWith("ui_")
        || fileName.startsWith("qrc_")) {
        return false;
    }

    QFileInfo file(fileName);
    QString suffix = "*." + file.suffix();
    QString filter = ui->txtFilter->text().trimmed();
    QStringList filters = filter.split(" ");
    return filters.contains(suffix);
}

bool CountCode::checkDir(const QString &dirName) const{//排除文件夹
    // 如果目录名以 "build" 或 "bin" 开头，直接返回 false，表示不检查这些目录
    if (dirName.startsWith("build") || dirName.startsWith("bin")
        || dirName.startsWith("debug") || dirName.startsWith("release")) {
        return true;
    }

    // 获取用户输入的目录过滤规则
    QString filter = ui->dirFilter->text().trimmed();
    QStringList filters = filter.split(" ");

    // 遍历目录过滤列表，检查目录是否符合过滤条件
    foreach (const QString &dirFilter, filters) {
        if(dirFilter.isEmpty())continue;
        if (dirName.contains(dirFilter)) {
            //qDebug()<<"当前目录："<<dirName<<"符合过滤规则："<<dirFilter;
            return true;  // 如果匹配过滤规则，则返回 true
        }
    }

    return false;  // 否则返回 false，表示该目录无需排除，可以继续检查
}

void CountCode::countCode(const QString &dirPath)
{
    QDir dir(dirPath);
    if(checkDir(dir.dirName())){
        return;
    }
    QFileInfoList fileInfos = dir.entryInfoList();
    foreach (QFileInfo fileInfo, fileInfos) {
        QString fileName = fileInfo.fileName();
        if (fileInfo.isFile()) {
            if (checkFile(fileName)) {
                listFile << fileInfo.filePath();
            }
        } else {
            if (fileName == "." || fileName == "..") {
                continue;
            }

            //递归找出文件
            countCode(fileInfo.absoluteFilePath());
        }
    }
}

void CountCode::countCode(const QStringList &files)
{
    int lineCode;
    int lineBlank;
    int lineNotes;
    int count = static_cast<int>(files.count());
    ui->btnClear->clicked();
    ui->tableWidget->setRowCount(count);

    quint32 totalLines = 0;
    quint32 totalBytes = 0;
    quint32 totalCodes = 0;
    quint32 totalNotes = 0;
    quint32 totalBlanks = 0;

    auto cnt = 0;
    for (int i = 0; i < count; i++) {
        QFileInfo fileInfo(files.at(i));
        if (!checkFile(fileInfo.filePath()))continue;
        countCode(fileInfo.filePath(), lineCode, lineBlank, lineNotes);
        int lineAll = lineCode + lineBlank + lineNotes;

        const auto itemName = new QTableWidgetItem;
        itemName->setText(fileInfo.fileName());

        const auto itemSuffix = new QTableWidgetItem;
        itemSuffix->setText(fileInfo.suffix());

        const auto itemSize = new QTableWidgetItem;
        itemSize->setText(QString::number(fileInfo.size()));

        const auto itemLine = new QTableWidgetItem;
        itemLine->setText(QString::number(lineAll));

        const auto itemCode = new QTableWidgetItem;
        itemCode->setText(QString::number(lineCode));

        const auto itemNote = new QTableWidgetItem;
        itemNote->setText(QString::number(lineNotes));

        const auto itemBlank = new QTableWidgetItem;
        itemBlank->setText(QString::number(lineBlank));

        const auto itemPath = new QTableWidgetItem;
        itemPath->setText(fileInfo.filePath());

        itemSuffix->setTextAlignment(Qt::AlignCenter);
        itemSize->setTextAlignment(Qt::AlignCenter);
        itemLine->setTextAlignment(Qt::AlignCenter);
        itemCode->setTextAlignment(Qt::AlignCenter);
        itemNote->setTextAlignment(Qt::AlignCenter);
        itemBlank->setTextAlignment(Qt::AlignCenter);

        ui->tableWidget->setItem(cnt, 0, itemName);
        ui->tableWidget->setItem(cnt, 1, itemSuffix);
        ui->tableWidget->setItem(cnt, 2, itemSize);
        ui->tableWidget->setItem(cnt, 3, itemLine);
        ui->tableWidget->setItem(cnt, 4, itemCode);
        ui->tableWidget->setItem(cnt, 5, itemNote);
        ui->tableWidget->setItem(cnt, 6, itemBlank);
        ui->tableWidget->setItem(cnt, 7, itemPath);
        cnt++;

        totalBytes  += fileInfo.size();
        totalLines  += lineAll;
        totalCodes  += lineCode;
        totalNotes  += lineNotes;
        totalBlanks += lineBlank;

        if (i % 100 == 0) {
            qApp->processEvents();
        }
    }
    ui->tableWidget->setRowCount(cnt);

    //显示统计结果
    listFile.clear();
    ui->txtCount->setText(QString::number(count));
    ui->txtSize->setText(QString::number(totalBytes));
    ui->txtRow->setText(QString::number(totalLines));
    ui->txtCode->setText(QString::number(totalCodes));
    ui->txtNote->setText(QString::number(totalNotes));
    ui->txtBlank->setText(QString::number(totalBlanks));

    //计算百分比
    double percent = 0.0;
    //代码行所占百分比
    percent = static_cast<double>(totalCodes) / totalLines * 100;
    ui->code_progressBar->setFormat(QString("%1%").arg(percent, 5, 'f', 2, QChar(' ')));
    ui->code_progressBar->setValue(static_cast<int>(percent));
    //ui->labPercentCode->setText(QString("%1%").arg(percent, 5, 'f', 2, QChar(' ')));
    //注释行所占百分比
    percent = static_cast<double>(totalNotes) / totalLines * 100;
    ui->note_progressBar->setFormat(QString("%1%").arg(percent, 5, 'f', 2, QChar(' ')));
    ui->note_progressBar->setValue(static_cast<int>(percent));
    //ui->labPercentNote->setText(QString("%1%").arg(percent, 5, 'f', 2, QChar(' ')));
    //空行所占百分比
    percent = static_cast<double>(totalBlanks) / totalLines * 100;
    ui->blank_progressBar->setFormat(QString("%1%").arg(percent, 5, 'f', 2, QChar(' ')));
    ui->blank_progressBar->setValue(static_cast<int>(percent));
    //ui->labPercentBlank->setText(QString("%1%").arg(percent, 5, 'f', 2, QChar(' ')));
}

void CountCode::countCode(const QString &fileName, int &lineCode, int &lineBlank, int &lineNotes)
{
    if (!checkFile(fileName))return;
    lineCode = lineBlank = lineNotes = 0;
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        QTextStream out(&file);
        QString line;
        bool isNote = false;
        while (!out.atEnd()) {
            line = out.readLine();

            //移除前面的空行
            if (line.startsWith(" ")) {
                line.remove(" ");
            }

            //判断当前行是否是注释
            if (line.startsWith("/*")) {
                isNote = true;
            }

            //注释部分
            if (isNote) {
                lineNotes++;
            } else {
                if (line.startsWith("//")) {    //注释行
                    lineNotes++;
                } else if (line.isEmpty()) {    //空白行
                    lineBlank++;
                } else {                        //代码行
                    lineCode++;
                }
            }

            //注释结束
            if (line.endsWith("*/")) {
                isNote = false;
            }
        }
    }
}

void CountCode::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CountCode::onBtnOpenFileClicked()
{
    ui->btnClear->clicked();
    ui->txtPath->clear();
    const QString filter = QString("代码文件(%1)").arg(ui->txtFilter->text().trimmed());
    const QStringList files = QFileDialog::getOpenFileNames(this, "选择文件", "./", filter);
    if (!files.empty()) {
        ui->txtFile->setText(files.join("|"));
        countCode(files);
    }
}

void CountCode::onBtnOpenPathClicked()
{
    ui->btnClear->clicked();
    ui->txtFile->clear();
    const QString path = QFileDialog::getExistingDirectory(this, "选择目录", "./",  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty()) {
        ui->txtPath->setText(path);
        listFile.clear();
        countCode(path);
        countCode(listFile);
    }
}

void CountCode::onBtnClearClicked()
{
    ui->txtCount->setText("0");
    ui->txtSize->setText("0");
    ui->txtRow->setText("0");

    ui->txtCode->setText("0");
    ui->txtNote->setText("0");
    ui->txtBlank->setText("0");

    //ui->labPercentCode->setText("0%");
    //ui->labPercentNote->setText("0%");
    //ui->labPercentBlank->setText("0%");
    ui->code_progressBar->setFormat("0%");
    ui->note_progressBar->setFormat("0%");
    ui->blank_progressBar->setFormat("0%");
    ui->code_progressBar->setValue(0);
    ui->note_progressBar->setValue(0);
    ui->blank_progressBar->setValue(0);

    ui->tableWidget->setRowCount(0);
}

void CountCode::onEditingFinished() {
    //qDebug()<<"编辑完成";
    ui->btnClear->clicked();
    if (!ui->txtFile->text().isEmpty()) {
        const QStringList files = ui->txtFile->text().split('|');
        countCode(files);
    }
    else if (!ui->txtPath->text().isEmpty()) {
        auto path = ui->txtPath->text();
        listFile.clear();
        countCode(path);
        countCode(listFile);
    }
}
