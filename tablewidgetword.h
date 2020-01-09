#ifndef TABLEWIDGETWORD_H
#define TABLEWIDGETWORD_H

#include <QTableWidget>

#define OUTPUT_PEN_COLOR "black"
#define BAOBIAO_COLOR "blue"
#define INPUT_COLOR "MediumAquamarine"
#define ASSIST_COLOR "LightSeaGreen"
#define REMARK_COLOR "LightGray"
#define HEADER_COLOR "DarkSeaGreen"
#define OUTPUT_COLOR "white"
#define CONFLICT_OUTPUT_COLOR "gray"
#define CONFLICT_OUTPUT_PEN_COLOR "white"

class TableWidgetWord : public QTableWidget
{
    Q_OBJECT

public:
    explicit TableWidgetWord(QWidget *parent = nullptr);


protected:
    bool eventFilter(QObject *o, QEvent *e);
    void contextMenuEvent(QContextMenuEvent *event);


signals:
    void inputCurrentRowInfo(int row);
    void outputCurrentRowInfo(int row);
    void addOneRecord(bool upFlag);
    void addColumn();

private:
    QMenu *menu;
    QMenu *menu_addRow;
    QMenu *menu_operator;
    QAction *action_input;
    QAction *action_output;
    QAction *action_conflictOutput;
    QAction *action_header;
    QAction *action_bb;
    QAction *action_assist_output;
    QAction *action_remark;
    QAction *action_addUpon;
    QAction *action_addBelow;
    QAction *action_addColumn;
    QAction *action_delete;
    QAction *action_deleteColumn;
    QAction *action_greater;
    QAction *action_greater_equal;
    QAction *action_equal;
    QAction *action_less;
    QAction *action_less_equal;
    QAction *action_undo;
    QAction *action_check;

    bool IsNumber(QString &qstrSrc);

private slots:
    void slot_input();          //设为输入
    void slot_output();         //设为输出
    void slot_conflictOutput(); //设为互为因果输出
    void slot_header();         //设为表头
    void slot_bb();             //设为报表
    void slot_assistoutput();   //辅助输出
    void slot_remark();         //设为备注
    void slot_addUpon();        //在上方添加行
    void slot_addBelow();       //在下方添加行
    void slot_addColumn();      //添加列
    void slot_deleteColumn();   //删除列
    void slot_delete();         //删除行
    void slot_greater();        //>
    void slot_greater_equal();  //>=
    void slot_equal();          //==
    void slot_less();           //<
    void slot_less_equal();     //<=
    void slot_undo();           //撤销
    void slot_check();          //检查
};

#endif // TABLEWIDGETWORD_H
