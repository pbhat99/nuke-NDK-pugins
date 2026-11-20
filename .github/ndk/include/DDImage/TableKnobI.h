// TableKnobI.h
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef TABLE_KNOBI_H
#define TABLE_KNOBI_H

#include <functional>
#include <vector>
#include <string>

#include "DDImage/Knob.h"

namespace DD
{
  namespace Image
  {
    class Table_KnobI
    {
    protected:
      virtual ~Table_KnobI() { }

    public:

      enum ColumnType
      {
        Unknown,
        FloatColumn,
        AnimCurveColumn,
        StringColumn,
        BoolColumn,
        AnimBoolColumn,
        ColorPickerColumn,
        ExclusiveBoolColumn,
      };

      enum EditingWidgets
      {
        NoWidgets         = 0x0,
        DeleteRowsWidget  = 0x1,
        AddRowWidget      = 0x2,
      };

      enum AnimCurveColumnInterpolationType
      {
        Default,
        Smooth,
        Constant,
        Linear,
        Catmull_Rom
      };

      //! Column names must be unique per Table_KnobI and should not contain spaces, as they are used for lookups
      //! and serialisation

      //! this function is currently only designed to add columns initially before rows have been added to the table
      virtual void addColumn(const std::string& name, const std::string& title, ColumnType type, bool editable, int width = 70, bool visible = true) = 0;
      virtual void addStringColumn(const std::string& name, const std::string& title, bool editable, int width = 70, bool uniqueStringContents = false, bool visible = true) = 0;
      virtual void addAnimCurveColumn(const std::string& name, const std::string& title, bool editable, int width = 70, AnimCurveColumnInterpolationType interpType = Default, bool visible = true) = 0;
      //! find out the index position of a column, based on the given name
      virtual int getColumnIndex(const std::string& name) = 0;

      enum ModificationType
      {
        eNone, eAddRow, eEditRow, eDeleteRow, eRefresh, eSelectionChanged
      };

      virtual DD::Image::Knob* realKnob() = 0;
      virtual Knob& knob() = 0;

      virtual int getColumnCount() const = 0;
      virtual int getRowCount() const = 0;

      //! Delete all data in the table, keeping the column definitions intact
      virtual void deleteAllItems() = 0;
      virtual void deleteAllItemsNoChanged() = 0;

      //! Note - if you have a Button knob and want it to perform one of the following operations and have
      //! each item within consecutive identical operations (e.g. 3 presses of a delete button) have a discrete
      //! undo point, you need to set the flags for that button knob to include GRANULAR_UNDO, otherwise all the undo operations
      //! will be merged into one.

      virtual void deleteRow(int row) = 0;
      virtual void deleteRows(std::vector<int>& rows) = 0;
      virtual int addRow(int position = -1) = 0;

      virtual void clearSelection() = 0;
      //! Change the selection to the single specified row.
      virtual void selectRow(int row) = 0;
      //! Change the selection to the specified rows.
      virtual void selectRows(std::vector<int>& rows) = 0;

      //! Returns the row index of a single selected row.
      virtual int getSelectedRow() = 0;
      //! Returns the row indexes of all selected rows.
      virtual std::vector<int> getSelectedRows() = 0;
      //! Delete all currently selected rows.
      virtual void deleteSelectedRows() = 0;

      //! Setter functions for table cell values
      //! These setter functions return false and do nothing if the column specified
      //! isn't of the correct type. The exception here is AnimCurve columns on which
      //! setCellFloat can be used. The bits of the unsigned "colour" parameter
      //! supplied to setCellColor should be in 0xRRGGBB format, with alpha omitted.
      virtual bool setCellFloat(int row, int column, float value) = 0;
      virtual bool setCellString(int row, int column, const std::string& value) = 0;
      virtual bool setCellTextColor(const int row, const int column, const unsigned int rgba) = 0;
      virtual bool setCellBool(int row, int column, bool value) = 0;
      virtual bool setCellColor(int row, int column, const unsigned colour) = 0;
      virtual bool setAnimated(int row, int col) = 0;
      //! set a value in an AnimationCurve. Will set the cell to be animated if it isn't
      virtual bool setValueAt(int row, int col, double time, double value) = 0;
      virtual bool setValue(int row, int col, double value) = 0;

      //! get / set editable state
      virtual bool getEnabled(int col) const = 0;
      virtual void setEnabled(int col, bool enable) = 0;

      virtual bool getEnabled(int row, int col) const = 0;
      virtual void setEnabled(int row, int col, bool enable) = 0;

      //! get / set column visibility
      virtual bool getColumnVisibility(int col) const = 0;
      virtual void setColumnVisibility(int col, bool visible) = 0;


      //! Getter functions for table cell values
      virtual float getCellFloat(int row, int column) const = 0;
      virtual std::string getCellString(int row, int column) const = 0;
      virtual bool getCellBool(int row, int column) const = 0;
      virtual unsigned getCellColor(int row, int column) const = 0;

      virtual bool isAnimated(int row, int col) const = 0;
      virtual int getNumKeys(int row) const = 0;
      virtual int getNumKeys(int row, int col) const = 0;
      virtual double getKeyTime(int row, int keyNum) const = 0;
      virtual double getKeyTime(int row, int col, int keyNum) const = 0;
      virtual int getKeyIndex(int row, int col, double time) const = 0;
      virtual double getValueAt(double time, int row, int col) const = 0;
      virtual double getValueAtKey(int keyNum, int row, int col) const = 0;
      virtual double getValue(int row, int col) const = 0;
      virtual bool isKeyAt(double time, int row, int col) const = 0;
      //! uses the current frame
      virtual bool isKey(int row, int col) const = 0;
      virtual void removeKey(int row, int col) = 0;
      virtual void removeKeyAt(int row, int col, double time) = 0;

      virtual bool setExpression(int row, int col, const std::string& value) = 0;
      virtual bool hasExpression(int row, int col) const = 0;
      virtual void removeExpression(int row, int col) = 0;

      virtual void setAnimCurveColumnInterpolationType(int col, AnimCurveColumnInterpolationType type) = 0;
      virtual AnimCurveColumnInterpolationType getAnimCurveColumnInterpolationType(int col) = 0;

      //! StoreCallbackType specifies how data in a particular column should be processed according
      //! to Nuke's current proxy mode. For example, if two columns in your table are (x,y) positions,
      //! adding store callbacks with StoreCallbackTypes of eXPositionValue, eYPositionValue resp. will
      //! convert the location pushed into the std::vector to proxy co-ordinates first. A value of
      //! eDefault will not apply any proxy scaling.
      enum StoreCallbackType {
        eDefault, eXPositionValue, eYPositionValue, eWidthValue, eHeightValue
      };

      //! Register member variables for cells to update in Knob::store(). The optional type parameter
      //! will instruct the value stored to be changed according to the current proxy setting.
      virtual bool addStoreCallback(int col, std::vector<double>* p, StoreCallbackType type = eDefault) = 0;
      virtual bool removeStoreCallback(int col) = 0;
      //! In order to actually
      virtual void beforeStore() = 0;
      virtual void doStore() = 0;

      //! will remove any animation and set the float value to be what the value
      //! of the current curve is at the current frame
      virtual void removeAnimation(int row, int col) = 0;

      //! Returns a list of the rows which were modified by the last table change.
      virtual std::set<int> getModifiedRows() const = 0;
      //! Returns the set of all cells which were modified by the last table change.
      virtual std::set<std::pair<int, int> > getModifiedCells() const = 0;
      //! Returns a value indicating what the last modification was (adding, editing or deleting rows).
      virtual ModificationType getModificationType() const = 0;

      virtual void beginUndoBatchOperation(ModificationType type) = 0;
      virtual void endUndoBatchOperation() = 0;

      //! These functions can be used when updating multiple rows, and only want a changed() call
      //! to be called when events are resumed
      virtual void suspendKnobChangedEvents() = 0;
      virtual void resumeKnobChangedEvents(bool sendChanged = true) = 0;
      virtual std::set<int> getSuspendedChangedEventRows() const = 0;

      virtual void setColumnsForExpressionLinks(std::vector<int> indices) = 0;
      virtual bool areColumnsForExpressionLinksSet() const = 0;
      virtual void resetColumnsForExpressionLinks() = 0;

      virtual void reset() = 0;

      //! The following functions allow you to create default +/-, "add row" and "delete rows" buttons
      //! under the TableKnob. You can pass in Table_KnobI::EditingWidgets flags such as AddRowWidget
      //! and/or DeleteRowsWidget flags to setEditingWidgets to enable / disable specific buttons.
      //! To maintain backwards compatibility, the default behaviour
      //! of passing "true" into setEditingWidgets(bool) creates only the delete rows button.
      virtual bool editingWidgets() const = 0;
      virtual int editingWidgetFlags() const = 0;
      virtual void setEditingWidgets(bool editing) = 0;
      virtual void setEditingWidgetFlags(int editWidgetFlags) = 0;

      //! sets whether to display all animation curves in the table,
      //! or just the ones in selected rows
      virtual bool displayAllAnimationCurves() const = 0;
      virtual void setDisplayAllAnimationCurves(bool displayAll) = 0;

      struct MenuItem {
        std::string itemText;  //!< The text of the menu item. Am empty string will make a separator line.
        int itemId;            //!< An integer ID that you can use for any purpose, such as associating a command
      };
      using MenuCreateCallback = std::function<std::vector<MenuItem> (Table_KnobI*)>;
      using MenuNotifyCallback = std::function<void (Table_KnobI*, const MenuItem& item)>;

      //! Add a custom right-click menu to the knob. You supply two functions, one to create the menu and one which
      //! is called to notify when an item is invoked. The create function is called each time the menu is displayed allowing
      //! the menu to be customized according to the cuurent selection.
      //! To clear an existing menu, call this again with default-constructed callbacks.
      virtual void setContextMenu(const MenuCreateCallback& createCallback, const MenuNotifyCallback& notifyCallback) = 0;

      //! Set the height of the table.
      virtual void setFixedHeight(int height) = 0;

      //! Set the resize behaviour of a column.
      virtual void setColumnResizeMode(int column, bool stretch) = 0;

      //! Resize a column to fit its contents.
      virtual void resizeColumnToContents(int column) = 0;
    };
  }
}

#endif // TABLE_KNOBI_H
