#include <QApplication>
#include <QClipboard>
#include <QKeySequence>
#include <QKeyEvent>
#include "ExtendedTableWidget.h"
#include "sqlitetablemodel.h"
#include <QScrollBar>
#include <QHeaderView>

ExtendedTableWidget::ExtendedTableWidget(QWidget* parent) :
    QTableView(parent)
{
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(vscrollbarChanged(int)));
}

void ExtendedTableWidget::copy()
{
    // Get list of selected items
    QItemSelectionModel* selection = selectionModel();
    QModelIndexList indices = selection->selectedIndexes();

    // Abort if there's nothing to copy
    if(indices.size() == 0)
        return;

    // Sort the items by row, then by column
    qSort(indices);

    // Go through all the items...
    QString result;
    QModelIndex prev = indices.front();
    indices.removeFirst();
    foreach(QModelIndex index, indices)
    {
        // Add the content of this cell to the clipboard string
        result.append(QString("\"%1\"").arg(prev.data().toString()));

        // If this is a new row add a line break, if not add a tab for cell separation
        if(index.row() != prev.row())
            result.append("\r\n");
        else
            result.append("\t");

        prev = index;
    }
    result.append(QString("\"%1\"\r\n").arg(indices.last().data().toString()));      // And the last cell

    // And finally add it to the clipboard
    qApp->clipboard()->setText(result);
}

void ExtendedTableWidget::keyPressEvent(QKeyEvent* event)
{
    // Call a custom copy method when Ctrl-C is pressed
    if(event->matches(QKeySequence::Copy))
        copy();
    else
        QTableView::keyPressEvent(event);
}

void ExtendedTableWidget::updateGeometries()
{
    // Call the parent implementation first - it does most of the actual logic
    QTableView::updateGeometries();

    // Check if a model has already been set yet
    if(model())
    {
        // If so and if it is a SqliteTableModel and if the parent implementation of this method decided that a scrollbar is needed, update its maximum value
        SqliteTableModel* m = qobject_cast<SqliteTableModel*>(model());
        if(m && verticalScrollBar()->maximum())
            verticalScrollBar()->setMaximum(m->totalRowCount());
    }
}

void ExtendedTableWidget::vscrollbarChanged(int value)
{
    // Cancel if there is no model set yet - this shouldn't happen (because without a model there should be no scrollbar) but just to be sure...
    if(!model())
        return;

    // How many rows are visible right now?
    int row_top = rowAt(0) == -1 ? 0 : rowAt(0);
    int row_bottom = rowAt(height()) == -1 ? model()->rowCount() : rowAt(height());
    int num_visible_rows = row_bottom - row_top;

    // Fetch more data from the DB if necessary
    if((value + num_visible_rows) >= model()->rowCount() && model()->canFetchMore(QModelIndex()))
        model()->fetchMore(QModelIndex());
}
