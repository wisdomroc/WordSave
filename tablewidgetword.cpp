#include "tablewidgetword.h"
#include <QEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>
#include <QStylePainter>
#include <QAbstractButton>
#include <QContextMenuEvent>

TableWidgetWord::TableWidgetWord(QWidget* parent) :
    QTableWidget(parent), menu(nullptr)
{
    QAbstractButton* btn = findChild<QAbstractButton*>();
    if (btn)
    {
        btn->setText("");
        btn->installEventFilter(this);

        // adjust the width of the vertical header to match the preferred corner button width
        // (unfortunately QAbstractButton doesn't implement any size hinting functionality)

        QStyleOptionHeader opt;
        opt.text = btn->text();
        QSize s = (btn->style()->sizeFromContents(QStyle::CT_HeaderSection, &opt, QSize(), btn).
                   expandedTo(QApplication::globalStrut()));
        if (s.isValid())
            verticalHeader()->setMinimumWidth(s.width());
    }
}

bool TableWidgetWord::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Paint)
    {
        QAbstractButton* btn = qobject_cast<QAbstractButton*>(o);
        if (btn)
        {
            // paint by hand (borrowed from QTableCornerButton)

            QStyleOptionHeader opt;
            opt.init(btn);
            QStyle::State state = QStyle::State_None;
            if (btn->isEnabled())
                state |= QStyle::State_Enabled;
            if (btn->isActiveWindow())
                state |= QStyle::State_Active;
            if (btn->isDown())
                state |= QStyle::State_Sunken;
            opt.state = state;
            opt.rect = btn->rect();
            opt.text = btn->text(); // this line is the only difference to QTableCornerButton

            opt.position = QStyleOptionHeader::OnlyOneSection;
            QStylePainter painter(btn);
            painter.drawControl(QStyle::CE_Header, opt);
            return true; // eat event

        }
    }
    return false;
}


void TableWidgetWord::contextMenuEvent(QContextMenuEvent *event)
{
    if(menu != nullptr)
    {
        delete menu;
        menu = nullptr;
    }

    menu = new QMenu();
    menu_addRow = new QMenu(QStringLiteral("添加行"));
    menu_operator = new QMenu(QStringLiteral("符号设置"));
    action_input = new QAction(QStringLiteral("设为条件"), menu);
    action_output = new QAction(QStringLiteral("设为输出"), menu);
    action_conflictOutput = new QAction(QStringLiteral("设为互为因果式输出"), menu);
    action_header = new QAction(QStringLiteral("设为表头"), menu);
    action_bb = new QAction(QStringLiteral("设为报表"), menu);
    action_assist_output = new QAction(QStringLiteral("设为辅助输出"), menu);
    action_remark = new QAction(QStringLiteral("设为备注"), menu);
    action_addUpon = new QAction(QStringLiteral("在上方添加行"), menu_addRow);
    action_addBelow = new QAction(QStringLiteral("在下方添加行"), menu_addRow);
    action_addColumn = new QAction(QStringLiteral("添加列"), menu);
    action_delete = new QAction(QStringLiteral("删除行"), menu);
    action_deleteColumn = new QAction(QStringLiteral("删除列"),menu);
    action_greater = new QAction(QStringLiteral("添加符号>"), menu_operator);
    action_greater_equal = new QAction(QStringLiteral("添加符号>="), menu_operator);
    action_equal = new QAction(QStringLiteral("添加符号=="), menu_operator);
    action_less = new QAction(QStringLiteral("添加符号<"), menu_operator);
    action_less_equal = new QAction(QStringLiteral("添加符号<="), menu_operator);
    action_undo = new QAction(QStringLiteral("撤销所有符号"), menu_operator);
    action_check = new QAction(QStringLiteral("修正符号"), menu);

    menu->addAction(action_bb);
    menu->addAction(action_header);
    menu->addAction(action_input);
    menu->addAction(action_output);
    menu->addAction(action_conflictOutput);
    menu->addAction(action_assist_output);
    menu->addAction(action_remark);
    menu->addSeparator();
    menu->addMenu(menu_addRow);
    menu->addAction(action_addColumn);
    menu->addAction(action_delete);
    menu->addAction(action_deleteColumn);

    if(currentItem() == nullptr)
        return;
    QString currentInfo = currentItem()->text();
    currentInfo.remove(">");
    currentInfo.remove("<");
    currentInfo.remove("=");
    bool ret = IsNumber(currentInfo);
    if(ret)
    {
        menu->addMenu(menu_operator);
    }
    menu_addRow->addAction(action_addUpon);
    menu_addRow->addAction(action_addBelow);
    menu_operator->addAction(action_greater);
    menu_operator->addAction(action_greater_equal);
    menu_operator->addAction(action_equal);
    menu_operator->addAction(action_less);
    menu_operator->addAction(action_less_equal);
    menu_operator->addAction(action_undo);
    menu->addAction(action_check);
    connect(action_input, SIGNAL(triggered()), this, SLOT(slot_input()));
    connect(action_output, SIGNAL(triggered()), this, SLOT(slot_output()));
    connect(action_conflictOutput, SIGNAL(triggered()), this, SLOT(slot_conflictOutput()));
    connect(action_header, SIGNAL(triggered()), this, SLOT(slot_header()));
    connect(action_bb, SIGNAL(triggered()), this, SLOT(slot_bb()));
    connect(action_assist_output, SIGNAL(triggered()), this, SLOT(slot_assistoutput()));
    connect(action_remark, SIGNAL(triggered()), this, SLOT(slot_remark()));
    connect(action_addUpon, SIGNAL(triggered()), this, SLOT(slot_addUpon()));
    connect(action_addBelow, SIGNAL(triggered()), this, SLOT(slot_addBelow()));
    connect(action_addColumn, SIGNAL(triggered()), this, SLOT(slot_addColumn()));
    connect(action_delete, SIGNAL(triggered()), this, SLOT(slot_delete()));
    connect(action_deleteColumn, SIGNAL(triggered()), this, SLOT(slot_deleteColumn()));
    connect(action_greater, SIGNAL(triggered()), this, SLOT(slot_greater()));
    connect(action_greater_equal, SIGNAL(triggered()), this, SLOT(slot_greater_equal()));
    connect(action_equal, SIGNAL(triggered()), this, SLOT(slot_equal()));
    connect(action_less, SIGNAL(triggered()), this, SLOT(slot_less()));
    connect(action_less_equal, SIGNAL(triggered()), this, SLOT(slot_less_equal()));
    connect(action_undo, SIGNAL(triggered()), this, SLOT(slot_undo()));
    connect(action_check, SIGNAL(triggered()), this, SLOT(slot_check()));



    menu->exec(event->globalPos());
}

bool TableWidgetWord::IsNumber(QString &qstrSrc)
{
    QByteArray ba = qstrSrc.toLatin1();
    const char *s = ba.data();
    bool bret = true;
    while(*s)
    {
        if(*s>='0' && *s<='9')
        {

        }
        else
        {
            bret = false;
            break;
        }
        s++;
    }
    if(qstrSrc.contains("0."))
    {
        bret = true;
    }
    return bret;
}

void TableWidgetWord::slot_input()
{
    emit inputCurrentRowInfo(currentRow());
    QList<QTableWidgetItem *>items = selectedItems();
    foreach (QTableWidgetItem *item, items) {
        item->setBackgroundColor(QColor(INPUT_COLOR));
        if(item->foreground().color() == QColor("black"))
        {
            item->setForeground(QBrush(QColor("white")));
        }
    }
}

void TableWidgetWord::slot_output()
{
    emit outputCurrentRowInfo(currentRow());
    QList<QTableWidgetItem *>items = selectedItems();
    foreach (QTableWidgetItem *item, items) {
        item->setBackgroundColor(QColor(OUTPUT_COLOR));
        if(item->foreground().color() == QColor("black"))
        {
            item->setForeground(QBrush(QColor(OUTPUT_PEN_COLOR)));//white
        }
        item->setForeground(QBrush(QColor(OUTPUT_PEN_COLOR)));
    }
}

void TableWidgetWord::slot_conflictOutput()
{
    QList<QTableWidgetItem *>items = selectedItems();
    foreach (QTableWidgetItem *item, items) {
        item->setBackgroundColor(QColor(CONFLICT_OUTPUT_COLOR));
        if(item->foreground().color() == QColor("black"))
        {
            item->setForeground(QBrush(QColor("white")));
        }
    }
}

void TableWidgetWord::slot_header()
{
    QList<int> rowList;
    QList<QTableWidgetItem *>items = selectedItems();
    foreach (QTableWidgetItem *item, items) {
        if(!rowList.contains(item->row()))
        {
            rowList.append(item->row());
        }
    }

    int colCount = columnCount();
    for(int i = 0; i < colCount; i ++)
    {
        for(int j = 0; j < rowList.count(); j ++)
        {
            QTableWidgetItem *item_ = item(rowList.at(j), i);

            item_->setBackgroundColor(QColor(HEADER_COLOR));
            //! 这里晃眼
//            if(item_->foreground().color() == QColor("black"))
//            {
//                item_->setForeground(QBrush(QColor("white")));
//            }
        }
    }
}

void TableWidgetWord::slot_bb()
{
    int minRow = 0;
    int minColumn = 0;
    int maxRow = 0;
    int maxColumn = 0;

    QList<QTableWidgetItem *>items = selectedItems();
    foreach (QTableWidgetItem *item, items) {
        item->setForeground(QBrush(QColor(BAOBIAO_COLOR)));
        int row = item->row();
        int column = item->column();
        if(row > maxRow)
        {
            maxRow = row;
        }
        if(row < minRow)
        {
            minRow = row;
        }
        if(column > maxColumn)
        {
            maxColumn = column;
        }
        if(column < minColumn)
        {
            minColumn = column;
        }
    }
}

void TableWidgetWord::slot_assistoutput()
{
    QList<QTableWidgetItem *>items = selectedItems();
    foreach (QTableWidgetItem *item, items) {
        item->setBackgroundColor(QColor(ASSIST_COLOR));
        if(item->foreground().color() == QColor("black"))
        {
            item->setForeground(QBrush(QColor("white")));
        }
    }
}

void TableWidgetWord::slot_remark()
{
    QList<QTableWidgetItem *>items = selectedItems();
    foreach (QTableWidgetItem *item, items) {
        item->setBackgroundColor(QColor(REMARK_COLOR));
        if(item->foreground().color() == QColor("black"))
        {
            item->setForeground(QBrush(QColor("white")));
        }
    }
}

void TableWidgetWord::slot_addUpon()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }
    emit addOneRecord(true);
}

void TableWidgetWord::slot_addBelow()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }
    emit addOneRecord(false);
}

void TableWidgetWord::slot_addColumn()
{
    emit addColumn();
}

void TableWidgetWord::slot_deleteColumn()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    QList<int> columnNumberList;
    foreach (QTableWidgetItem *item, itemList) {
        if(!columnNumberList.contains(item->column())){
            columnNumberList.append(item->column());
        }
    }
    QStringList columnNumberStrList;
    for(int i = 0; i < columnNumberList.count(); ++i)
    {
        columnNumberStrList.append(QString::number(columnNumberList.at(i) + 1));
    }


    QMessageBox msgBox;
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox.setText(QStringLiteral("确定删除第%1列？").arg(columnNumberStrList.join(",")));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Yes:
        break;
    case QMessageBox::No:
        return;
        break;
    }


    for(int i = columnNumberList.count() - 1; i >=0; i --)
    {
        removeColumn(columnNumberList.at(i));
    }
}

void TableWidgetWord::slot_delete()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    QList<int> rowNumberList;
    foreach (QTableWidgetItem *item, itemList) {
        if(!rowNumberList.contains(item->row())){
            rowNumberList.append(item->row());
        }
    }
    QStringList rowNumberStrList;
    for(int i = 0; i < rowNumberList.count(); ++i)
    {
        rowNumberStrList.append(QString::number(rowNumberList.at(i) + 1));
    }


    QMessageBox msgBox;
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox.setText(QStringLiteral("确定删除第%1行？").arg(rowNumberStrList.join(",")));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Yes:
        break;
    case QMessageBox::No:
        return;
        break;
    }


    for(int i = rowNumberList.count() - 1; i >=0; i --)
    {
        removeRow(rowNumberList.at(i));
    }
}

void TableWidgetWord::slot_greater()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    foreach (QTableWidgetItem *item, itemList) {
        QString info = item->text();
        item->setText(QStringLiteral(">%1").arg(info));
    }
}

void TableWidgetWord::slot_greater_equal()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    foreach (QTableWidgetItem *item, itemList) {
        QString info = item->text();
        item->setText(QStringLiteral(">=%1").arg(info));
    }
}

void TableWidgetWord::slot_equal()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    foreach (QTableWidgetItem *item, itemList) {
        QString info = item->text();
        item->setText(QStringLiteral("==%1").arg(info));
    }
}

void TableWidgetWord::slot_less()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    foreach (QTableWidgetItem *item, itemList) {
        QString info = item->text();
        item->setText(QStringLiteral("<%1").arg(info));
    }
}

void TableWidgetWord::slot_less_equal()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    foreach (QTableWidgetItem *item, itemList) {
        QString info = item->text();
        item->setText(QStringLiteral("<=%1").arg(info));
    }
}

void TableWidgetWord::slot_undo()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    foreach (QTableWidgetItem *item, itemList) {
        QString info = item->text();
        info.remove("<=");
        info.remove(">=");
        info.remove("==");
        info.remove("<");
        info.remove(">");
        item->setText(info);
    }
}

void TableWidgetWord::slot_check()
{
    QList<QTableWidgetItem *> itemList = selectedItems();
    if(itemList.count() == 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择一条数据后，再进行当前操作"));
        return;
    }

    foreach (QTableWidgetItem *item, itemList) {
        QString info = item->text();
        if(info.contains("≥"))
        {
            info.replace("≥",">=");
        }
        if(info.contains("≤"))
        {
            info.replace("≤","<=");
        }
        if(info.contains("＜"))
        {
            info.replace("＜","<");
        }
        if(info.contains("＞"))
        {
            info.replace("＞",">");
        }
        item->setText(info);
    }
}
