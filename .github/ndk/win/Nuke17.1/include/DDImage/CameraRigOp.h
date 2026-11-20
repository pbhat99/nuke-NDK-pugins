//------------------------------------------------------------------------------
// DDImage/CameraRigOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_CAMERARIGOP_H
#define DDIMAGE_CAMERARIGOP_H

#include "DDImage/CameraOp.h"


namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::CameraRigOp
    */
    class DDImage_API CameraRigOp : public CameraOp
    {
    protected:
      /*! Stores per-view derived values updated in _validate().
          Used primarily for OpenGL display.
      */
      struct CamParams
      {
        int          view;            //!< Sub camera view index
        const char*  name;            //!< Name to use for OpenGL display
        uint32_t     glColor;         //!< Color to use for OpenGL display
        fdk::Mat4d   localXform;      //!< Rig-local transform matrix, in world-space
        fdk::Mat4d   apertureXform;   //!< Rig-local aperture transform matrix, in millimeters

        //! Default ctor leaves junk in vars.
        CamParams() : name("") {}
      };


    protected:
      bool   _glShowAllRigCameras;          //!< 

      DD::Image::Hash         _rigHash;     //!< Hash of knob values affecting CamParams
      std::vector<CamParams>  _rigParams;   //!< Derived per-view camera params & local xforms


    protected:
      //! Append controls that affect the rig cameras to a hash at a specific OutputContext.
      virtual void _appendRigValuesAt(const OutputContext& context,
                                      DD::Image::Hash&     hash) = 0;

      /*! Rebuild the CamParams list at a specific OutputContext.

          A subclass needs to add at least one CamParam to _rigParams otherwise
          an assert will be thrown.
      */
      virtual void _rebuildCamParamsAt(const OutputContext& context) = 0;


    public:
      CameraRigOp(Node* node);

      //------------------------------------------------------------

      void append(Hash& hash) override;
      void _validate(bool for_real) override;
      void draw_handle(ViewerContext*) override;

      //! Adds the OpenGL display option controls.
      void addDisplayOptionsKnobs(Knob_Callback) override;

      //! Adds addl front-panel knobs. Calls addRigKnobs() with the rigName().
      void addExtraFrontPanelKnobs(Knob_Callback) override;

      //------------------------------------------------------------

      //! Return the identification name of the rig. Must implement.
      virtual const char* rigName() const = 0;

      //! Add rig-specific knobs. Must implement.
      virtual void addRigKnobs(Knob_Callback f,
                               const char*   rig_name = "CameraRig") = 0;

      //! Append controls that affect the rig cameras to a hash at the current OutputContext.
      void appendRigValues(Hash&);

      //! Rebuild the CamParams list at the current OutputContext.
      void rebuildCamParams();

    };

  }
}

#endif
