#include "codeeditor.h"

#include <QPainter>
#include <QTextEdit>
#include <QTextBlock>
#include <QTextFormat>

namespace {

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor)
        : QWidget(editor)
        , m_editor(editor)
    {
    }

    QSize sizeHint() const override
    {
        return QSize(m_editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_editor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *m_editor;
};

} // namespace

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
{
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

bool CodeEditor::lineNumberVisible() const
{
    return m_lineNumberVisible;
}

int CodeEditor::lineNumberAreaWidth() const
{
    if (!m_lineNumberVisible)
        return 0;

    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    const int space = 14 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::setLineNumberVisible(bool visible)
{
    if (m_lineNumberVisible == visible)
        return;

    m_lineNumberVisible = visible;
    m_lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void CodeEditor::refreshLineNumberArea()
{
    m_lineNumberArea->setFont(font());
    updateLineNumberAreaWidth(0);
    m_lineNumberArea->update();
}

void CodeEditor::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);

    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        const QColor base = palette().color(QPalette::Base);
        selection.format.setBackground(base.darker(104));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    const QColor base = palette().color(QPalette::Base);
    painter.fillRect(event->rect(), base.darker(105));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    const QColor normalColor(0x8a, 0x94, 0x9e);
    const QColor currentColor(0x2f, 0x6f, 0xb0);

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const bool current = blockNumber == textCursor().blockNumber();
            QFont numberFont = font();
            numberFont.setBold(current);
            painter.setFont(numberFont);
            painter.setPen(current ? currentColor : normalColor);
            painter.drawText(0, top, m_lineNumberArea->width() - 9, fontMetrics().height(),
                             Qt::AlignRight, QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

    painter.setPen(base.darker(112));
    painter.drawLine(m_lineNumberArea->width() - 1, event->rect().top(),
                     m_lineNumberArea->width() - 1, event->rect().bottom());
}
