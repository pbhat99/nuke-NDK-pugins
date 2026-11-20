// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef MULTIARRAY_KNOBI_H
#define MULTIARRAY_KNOBI_H

#include <vector>

namespace DD
{
  namespace Image
  {

    class Knob;
    class ViewerContext;
    class Matrix4;

    /**
     * the MultiArray_Knob is a 3-dimensional array of XYZ-knobs.
     *
     * this is intended for the use for the extended bicubics.
     *
     * first, comes a 2-D array with dimensions x and y.
     * presently x and y are 2.  TODO: add resizing support.
     *
     * inside each of these elements, are then 5 XYZ-knobs.
     * the first of these is the 'main' one, and then the
     * remaining four are tangents.
     *
     * there is no load/save/undo/copy/paste support yet,
     * nor support for setting values on many knobs within
     * this at the same time, nor support for accessing
     * the subknobs from internal expressions.
     */
    class MultiArray_KnobI
    {

    public:
      /* get the x-dimension: returns 2 */
      virtual int xsize() const = 0;

      /* get the y-dimension: returns 2 */
      virtual int ysize() const = 0;

      /* gets the number of knobs in each element. returns 5 */
      virtual int cellsize() const = 0;

      /* gets the number of knobs total */
      virtual int cells() const = 0;

      /* get particular knob */
      virtual Knob* cell(unsigned index) = 0;

      /* get the actual knob for the given y/x/cell. */
      virtual Knob* getKnob(int y, int x, int cell) = 0;

      /* whether or not the given knob is selected in the viewer */
      virtual bool knobSelected(DD::Image::ViewerContext* ctx, int y, int x, int cell) const = 0;

      /* get the actual knob for the given y/x/cell. */
      virtual const Knob* getKnob(int y, int x, int cell) const = 0;

      /* sets flag to indicate whether the tangents (ie cells with cell>0)
         should be visible.  redraws.  at some point this will be a built-in
         checkbox */
      virtual void setTangentsVisible(bool) = 0;

      /* gets above flag */
      virtual bool tangentsVisible() const = 0;

      virtual void setKnobVisible(int y, int x, int cell, bool) = 0;

      struct KnobVisible
      {
        int y, x, cell;
        bool visible;

        KnobVisible(int y, int x, int cell, bool visible) : y(y), x(x), cell(cell), visible(visible)
        {
        }
      };

      virtual void setKnobsVisible(const std::vector<KnobVisible>& visibles) = 0;

      /* narrows down the view so it only displays the knobs in the y,x element. */
      virtual void showWidgetsFor(int y, int x) = 0;

      /* shows all knobs */
      virtual void showAllWidgets() = 0;

      virtual ~MultiArray_KnobI() { }

      /* resize the multi-array-knob. all previous data is thrown away. */
      virtual void resize(int y, int x) = 0;

      /* insert column at specified position.  non-destructive */
      virtual void insertColumn(int x) = 0;

      /* insert row at specified position.  non-destructive */
      virtual void insertRow(int y) = 0;

      virtual void deleteColumn(int x) = 0;
      virtual void deleteRow(int y) = 0;
    };
  }
}

#endif
