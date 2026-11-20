// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef LIST_KNOBI_H
#define LIST_KNOBI_H

#include "DDImage/Knob.h"
#include <vector>
#include <string>

namespace DD
{
  namespace Image
  {

    class List_KnobI
    {
    public:
      // Storage type for list data
      typedef std::vector<std::vector<std::string > > DataType;

    protected:
      virtual ~List_KnobI() { }

    public:

      enum ColumnType {
        String_Column,
        View_Column,
        Colour_Column,
        Choice_Column,
        MetaData_Key_Column,
        Float_Column,
        ExclBool_Column
      };

      struct Column
      {
        ColumnType type;
        std::string name;
        std::vector<std::string> actions;

        Column(ColumnType type, const std::string& name) : type(type), name(name)
        {
        }

        Column(ColumnType type, const std::string& name, const std::vector<std::string>& actions) : type(type), name(name), actions(actions)
        {
        }
      };

      virtual const DataType& getData() = 0;
      virtual ColumnType getColumnType(int i) const = 0;
      virtual const char* getColumnTitle(int i) const = 0;
      virtual size_t getColumnCount() const = 0;
      virtual size_t getRowCount() const = 0;
      virtual size_t getColumnCountForRow(int row) const = 0;

      virtual void deleteAllItems() = 0;
      virtual void deleteAllItemsNoChanged() = 0;
      virtual std::string& cell(int y, int x) = 0;
      virtual const std::string& cell(int y, int x) const = 0;

      virtual std::string cellExpand(int y, int x, const OutputContext& oc) const = 0;

      //! delete all the columns previously added with setColumn()
      virtual void clearColumns() = 0;

      virtual void setColumn(int i, const Column& column) = 0;

      virtual void setMinItems(int items) = 0;

      virtual bool editingWidgets() const = 0;
      virtual void setEditingWidgets(bool editing) = 0;

      virtual Knob& knob() = 0;

      virtual void setDefaultValue(const DataType& defaultList) = 0;
    };

  }
}

#endif
