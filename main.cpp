#include <QApplication>
#include <QHeaderView>
#include <QTableWidget>
#include <QRegularExpression>

#include <QTime>
#include <QTimer>
#include <QDebug>

static const QString s_lyric = QStringLiteral(R"(
[ti:告白气球]
[ar:周杰伦]
[al:周杰伦的床边故事]
[by:]
[offset:0]
[00:00.00]告白气球 - 周杰伦 (Jay Chou)
[00:07.86]词：方文山
[00:15.72]曲：周杰伦
[00:23.59]塞纳河畔 左岸的咖啡
[00:26.16]我手一杯 品尝你的美
[00:28.78]
[00:29.33]留下唇印的嘴
[00:31.83]
[00:34.27]花店玫瑰 名字写错谁
[00:36.90]告白气球 风吹到对街
[00:39.29]
[00:40.01]微笑在天上飞
[00:42.10]
[00:44.01]你说你有点难追
[00:46.57]想让我知难而退
[00:49.22]礼物不需挑最贵
[00:51.89]只要香榭的落叶
[00:54.56]喔 营造浪漫的约会
[00:57.26]不害怕搞砸一切
[00:59.93]拥有你就拥有全世界
[01:04.10]
[01:05.01]亲爱的 爱上你
[01:08.17]从那天起
[01:10.61]
[01:11.33]甜蜜的很轻易
[01:14.43]
[01:15.69]亲爱的 别任性
[01:18.85]你的眼睛
[01:21.24]
[01:21.94]在说我愿意
[01:25.23]
[01:48.90]塞纳河畔 左岸的咖啡
[01:51.46]我手一杯 品尝你的美
[01:54.43]留下唇印的嘴
[01:56.63]
[01:59.56]花店玫瑰 名字写错谁
[02:02.14]告白气球 风吹到对街
[02:04.37]
[02:05.23]微笑在天上飞
[02:07.49]
[02:09.29]你说你有点难追
[02:11.90]想让我知难而退
[02:14.60]礼物不需挑最贵
[02:17.26]只要香榭的落叶
[02:19.93]喔 营造浪漫的约会
[02:22.65]不害怕搞砸一切
[02:25.27]拥有你就拥有 全世界
[02:29.23]
[02:30.31]亲爱的 爱上你
[02:33.58]从那天起
[02:36.03]
[02:36.60]甜蜜的很轻易
[02:39.65]
[02:40.94]亲爱的 别任性
[02:44.20]你的眼睛
[02:46.70]
[02:47.26]在说我愿意
[02:50.81]
[02:51.76]亲爱的 爱上你
[02:54.52]
[02:55.05]恋爱日记
[02:57.30]
[02:57.93]飘香水的回忆
[03:00.72]
[03:02.33]一整瓶 的梦境
[03:05.42]全都有你
[03:07.91]
[03:08.64]搅拌在一起
[03:11.39]
[03:13.02]亲爱的别任性
[03:16.23]你的眼睛
[03:19.99]
[03:21.31]在说我愿意
)");

/* 歌词结构体 */
struct Lyric
{
    QString artist; /* 艺人名 */
    QString title; /* 曲名(标题) */
    QString album;  /* 专辑 */
    QString lyricsEditor; /* LRC歌词制作者 */
    int offset; /* 进度补偿时值，500=0.5秒 */

    /**
     * 歌词正文内容(列表)
     * QTime类型为歌词时间
     * QString类型为对应的歌词
     */
    QList<QPair<QTime, QString> > contents;

    void parse(const QString &string)
    {
        artist = parseArtist(string);
        title = parseTitle(string);
        album = parseAlbum(string);
        lyricsEditor = parseLyricsEditor(string);
        offset = parseOffset(string);
        contents = parseContents(string);
    }

private:
    QString _parse(const QString &regexp, const QString &string)
    {
        QRegularExpression re(regexp);
        re.setPatternOptions(QRegularExpression::MultilineOption);
        QRegularExpressionMatch match = re.match(string);
        if (match.hasMatch() && re.captureCount() == 1) {
            return match.captured(1);
        }

        return "";

    }
    QString parseArtist(const QString &string)
    {
        return _parse("^\\[ar:(\\S*)\\]$", string);
    }

    QString parseTitle(const QString &string)
    {
        return _parse("^\\[ti:(\\S*)\\]$", string);
    }

    QString parseAlbum(const QString &string)
    {
        return _parse("^\\[al:(\\S*)\\]$", string);
    }

    QString parseLyricsEditor(const QString &string)
    {
        return _parse("^\\[by:(\\S*)\\]$", string);
    }

    int parseOffset(const QString &string)
    {
        return _parse("^\\[offset:(\\S*)\\]$", string).toInt();
    }

    QList<QPair<QTime, QString> > parseContents(const QString &string)
    {
        QList<QPair<QTime, QString> > out;
        QRegularExpression re("^\\[(?<time>\\d+:\\d+(?:\\.\\d+)?)\\](?: *(?<content>.*))?$");
        re.setPatternOptions(QRegularExpression::MultilineOption);

        QRegularExpressionMatchIterator iter = re.globalMatch(string);
        while (iter.hasNext()) {
            QRegularExpressionMatch match = iter.next();
            QTime t = QTime::fromString(match.captured("time"), "mm:ss.z");
            out << qMakePair(t, match.captured("content"));
        }

        return out;
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* 歌词解析结构体 */
    Lyric lyric;
    lyric.parse(s_lyric);

    /* 将解析到的歌词显示在QTableWidget上 */
    QTableWidget *tableWidget = new QTableWidget();
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setVisible(false);

    tableWidget->resize(640, 480);
    tableWidget->setRowCount(lyric.contents.count() + 5);
    tableWidget->setColumnCount(2);


    tableWidget->setItem(0, 0, new QTableWidgetItem(QStringLiteral("歌曲")));
    tableWidget->setItem(0, 1, new QTableWidgetItem(lyric.title));

    tableWidget->setItem(1, 0, new QTableWidgetItem(QStringLiteral("歌手")));
    tableWidget->setItem(1, 1, new QTableWidgetItem(lyric.artist.isEmpty() ? QString("--") : lyric.artist));

    tableWidget->setItem(2, 0, new QTableWidgetItem(QStringLiteral("专辑")));
    tableWidget->setItem(2, 1, new QTableWidgetItem(lyric.album.isEmpty() ? QString("--") : lyric.album));

    tableWidget->setItem(3, 0, new QTableWidgetItem(QStringLiteral("歌词制作者")));
    tableWidget->setItem(3, 1, new QTableWidgetItem(lyric.lyricsEditor.isEmpty() ? QString("--") : lyric.lyricsEditor));

    tableWidget->setItem(4, 0, new QTableWidgetItem(QStringLiteral("进度补偿值")));
    tableWidget->setItem(4, 1, new QTableWidgetItem(QString::number(lyric.offset)));

    for (int i = 0; i < lyric.contents.count(); i++) {
        const QPair<QTime, QString> &each = lyric.contents.at(i);
        QTableWidgetItem *item1 = new QTableWidgetItem(each.first.toString("mm:ss.zzz"));
        QTableWidgetItem *item2 = new QTableWidgetItem(each.second);
        tableWidget->setItem(i + 5, 0, item1);
        tableWidget->setItem(i + 5, 1, item2);
    }

#if 0
    QTimer timer;
    QTime  time(0, 0, 0, 0);
    QObject::connect(&timer, &QTimer::timeout, [&](){
        int i = 0;
        time = time.addMSecs(timer.interval());
        tableWidget->takeHorizontalHeaderItem(0);
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(time.toString("mm:ss.z"));
        tableWidget->setHorizontalHeaderItem(0, item);

        for (const QPair<QTime, QString> &each: lyric.contents) {
            if (each.first > time) {
                int offset = (tableWidget->height() / tableWidget->rowHeight(0));
                tableWidget->scrollToItem(tableWidget->item(i + offset, 0));
                return;
            }
            i++;
         }
    });
    timer.start(500);
#endif

    tableWidget->show();
//    qDebug() << lyric;


    return a.exec();
}
