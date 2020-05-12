// 作者: Qt君
// QQ交流群: 732271126
// 扫码关注微信公众号: [Qt君] 第一时间获取最新推送.
// ██████████████    ██      ██████████████    ██████████████
// ██          ██  ████  ████    ██  ██        ██          ██
// ██  ██████  ██    ██████              ██    ██  ██████  ██
// ██  ██████  ██  ████  ██    ████████    ██  ██  ██████  ██
// ██  ██████  ██      ██  ██      ██    ████  ██  ██████  ██
// ██          ██  ██  ██      ████    ██  ██  ██          ██
// ██████████████  ██  ██  ██  ██  ██  ██  ██  ██████████████
//                         ██  ██████  ██████
// ██████████  ██████████  ██  ████████████  ██  ██  ██  ██
//       ██        ████        ██  ██    ██  ████████      ██
//   ██  ██  ████  ████  ████████████  ██  ██  ██████
//     ██████        ██████        ██  ██  ██████        ██
//       ██████████  ██  ██  ██  ██  ██  ██  ██      ████
//                 ██  ██  ██████  ████  ████████████  ██  ██
// ████  ██████████    ██        ████  ██  ██  ██  ██  ██
// ████    ████      ████  ██  ██████  ██████████        ██
//   ██  ████  ██    ████  ██████    ██  ██      ██    ██
// ████████      ██  ██      ████  ██    ████  ██████████  ██
// ██    ████  ████  ██  ████    ████      ████  ████████
// ██  ████  ██  ██      ██      ████    ██              ██
// ██  ██████  ████    ████  ██████████    ██████████  ██████
//                 ████    ████  ████  ██  ██      ██████████
// ██████████████  ████        ██████    ████  ██  ██████
// ██          ██    ████  ██  ██████  ██████      ████    ██
// ██  ██████  ██  ████    ████  ██  ██    ██████████████████
// ██  ██████  ██  ████        ██████████  ██        ██  ████
// ██  ██████  ██  ██  ██████    ██  ████████  ████████████
// ██          ██  ██    ██    ████    ██  ████  ██████  ██
// ██████████████  ██████████      ██            ████  ██

#include <QApplication>
#include <QHeaderView>
#include <QTableWidget>
#include <QLabel>
#include <QRegularExpression>

#include <QDropEvent>
#include <QMimeData>
#include <QTime>
#include <QDebug>

/* 歌词结构体 */
struct LyricParser
{
    QString title;  /* 歌名 */
    QString artist; /* 歌手 */
    QString album;  /* 专辑 */
    QString lyricsEditor; /* LRC歌词制作者 */
    int     offset;       /* 进度补偿时值，500=0.5秒 */

    /**
     * 歌词正文内容(列表)
     * QTime类型为歌词时间
     * QString类型为对应的歌词
     */
    QList<QPair<QTime, QString> > contents;

    /* 传入歌词字符串解析出歌词信息 */
    void parse(const QString &string)
    {
        title        = _parse("^\\[ti:(.*)\\]$", string);
        artist       = _parse("^\\[ar:(.*)\\]$", string);
        album        = _parse("^\\[al:(.*)\\]$", string);
        lyricsEditor = _parse("^\\[by:(.*)\\]$", string);
        offset       = _parse("^\\[offset:(.*)\\]$", string).toInt();
        contents     = parseContents(string);
    }

private:
    /* 主要为解析歌词信息如：歌名，歌手，专辑等信息 */
    QString _parse(const QString &regexp, const QString &string)
    {
        /* 创建一个正则表达式对象 */
        QRegularExpression re(regexp);
        /* 设置为多行匹配模式 */
        re.setPatternOptions(QRegularExpression::MultilineOption);

        /* 匹配操作,返回值为匹配结果 */
        QRegularExpressionMatch match = re.match(string);
        if (match.hasMatch() && re.captureCount() == 1) {
            /* 返回组捕获内容 */
            return match.captured(1);
        }

        return QString("");
    }

    /* 解析歌词正文内容 */
    QList<QPair<QTime, QString> > parseContents(const QString &string)
    {
        /* 歌词正文列表容器 */
        QList<QPair<QTime, QString> > out;

        /* 正则匹配歌词正文内容，匹配格式："[00:00.00]内容" */
        QRegularExpression re("^\\[(?<time>\\d+:\\d+(?:\\.\\d+)?)\\](?: *(?<content>.*))?$");
        re.setPatternOptions(QRegularExpression::MultilineOption);

        /* 使用全局匹配 */
        QRegularExpressionMatchIterator iter = re.globalMatch(string);
        /* 遍历全局匹配到的内容 */
        while (iter.hasNext()) {
            QRegularExpressionMatch match = iter.next();
            /* 获取组捕获分组名为"time"的内容 */
            QTime t = QTime::fromString(match.captured("time"), "mm:ss.z");
            /* 获取组捕获分组名为"content"的内容 */
            out << qMakePair(t, match.captured("content"));
        }

        return out;
    }
};

class Widget : public QWidget {
    Q_OBJECT
public:
    Widget(QWidget *parent = NULL) :
        QWidget(parent)
    {
        this->setAcceptDrops(true);
        this->resize(640, 480);
        QLabel *tips = new QLabel(QStringLiteral("将歌词文件拖拽到这里自动解析"), this);
        tips->adjustSize();
        tips->move((width() - tips->width())/2, (height() - tips->height())/2);
    }

    void refresh()
    {
        /* 将解析到的歌词显示在QTableWidget上 */
        QTableWidget *tableWidget = new QTableWidget(this);
        tableWidget->resize(size());

        /* 启用内容自动调节表格宽度的功能 */
        tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        /* 禁用水平与垂直的表头 */
        tableWidget->horizontalHeader()->setVisible(false);
        tableWidget->verticalHeader()->setVisible(false);

        tableWidget->setRowCount(m_lyricParser.contents.count() + 5);
        tableWidget->setColumnCount(2);

        tableWidget->setItem(0, 0, new QTableWidgetItem(QStringLiteral("歌名")));
        tableWidget->setItem(0, 1, new QTableWidgetItem(m_lyricParser.title));

        tableWidget->setItem(1, 0, new QTableWidgetItem(QStringLiteral("歌手")));
        tableWidget->setItem(1, 1, new QTableWidgetItem(m_lyricParser.artist.isEmpty() ? QString("--") : m_lyricParser.artist));

        tableWidget->setItem(2, 0, new QTableWidgetItem(QStringLiteral("专辑")));
        tableWidget->setItem(2, 1, new QTableWidgetItem(m_lyricParser.album.isEmpty() ? QString("--") : m_lyricParser.album));

        tableWidget->setItem(3, 0, new QTableWidgetItem(QStringLiteral("歌词制作者")));
        tableWidget->setItem(3, 1, new QTableWidgetItem(m_lyricParser.lyricsEditor.isEmpty() ? QString("--") : m_lyricParser.lyricsEditor));

        tableWidget->setItem(4, 0, new QTableWidgetItem(QStringLiteral("进度补偿值")));
        tableWidget->setItem(4, 1, new QTableWidgetItem(QString::number(m_lyricParser.offset)));

        /* 遍历歌词正文内容，并逐一写入到表格中 */
        for (int i = 0; i < m_lyricParser.contents.count(); i++) {
            const QPair<QTime, QString> &each = m_lyricParser.contents.at(i);
            QTableWidgetItem *item1 = new QTableWidgetItem(each.first.toString("mm:ss.zzz"));
            QTableWidgetItem *item2 = new QTableWidgetItem(each.second);
            tableWidget->setItem(i + 5, 0, item1);
            tableWidget->setItem(i + 5, 1, item2);
        }

        tableWidget->show();
    }

protected:
    /* 拖动事件的释放操作事件 */
    void dropEvent(QDropEvent *event)
    {
        /* 获取拖放的第一个目标文件 */
        QString targetFile = event->mimeData()->urls().first().toLocalFile();

        QFile file(targetFile);
        file.open(QFile::ReadOnly);
        /* 解析歌词内容 */
        m_lyricParser.parse(file.readAll());
        /* 刷新歌词内容到表格中 */
        refresh();
    }

    /* 拖动事件的进入操作事件 */
    void dragEnterEvent(QDragEnterEvent* event)
    {
        if (event->mimeData()->hasUrls()) {
            /* 表示可以在这个部件上拖放对象 */
            event->acceptProposedAction();
        }
        else {
            event->ignore();
        }
    }

private:
   LyricParser m_lyricParser;
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.setWindowTitle(QStringLiteral("歌词解析器 @Qt君"));
    w.show();

    return a.exec();
}
