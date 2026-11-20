// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.
#ifndef TRANSFORMJACKKNOBI_H
#define TRANSFORMJACKKNOBI_H

#include "DDImage/Vector2.h"
#include "DDImage/Matrix4.h"
#include "DDImage/ViewerContext.h"


#include "DDImage/DDImage_API.h"


namespace DD
{
  namespace Image
  {
    class TransformJack_KnobI;
    class Op;

    /**
     * A class that can be inherited from an object interested in using the transform jack
     * making it a transform jack client which can receive update reports
     */
    class DDImage_API TransformJack_ClientI
    {
    public:

      /**
       * the transform event types
       */
      enum TransformEvent
      {
        EVENT_TRANSLATION,
        EVENT_ROTATE,
        EVENT_SKEW,
        EVENT_SCALE,
        EVENT_CORNERPIN,

        EVENT_PUSH,
        EVENT_RELEASE
      };

      virtual ~TransformJack_ClientI();

      /**
       * a function that signals the client of an event that is about to happen
       * @param ctx the current viewer context
       * @param event the current event
       * @return indicates whether the event was handled or not, if the event is not accepted then the transform wont be performed on the bounding box
       */
      virtual bool transformEvent( ViewerContext* ctx, const TransformJack_KnobI* transformJack, TransformEvent event ) = 0;

      /**
       * indicates that a change is about to happen
       * @param ctx the current viewer context
       * @param source the source of the change about to happen
       */
      virtual void aboutToChange( ViewerContext* ctx, const TransformJack_KnobI* transformJack, int sourceFlags ) = 0;

      /**
       * indicates that a change has just finished
       * @param ctx the viewer context for the change
       * @param source the source of the change
       */
      virtual void finishedChange( ViewerContext* ctx, const TransformJack_KnobI* transformJack, int sourceFlags ) = 0;

      /**
       * indicates that this client requires an individual transform box. Defaults to false, meaning that all clients share
       * a single transform box. Override if individual transform boxes are needed for each client
       */
      virtual bool needsIndividualHandles() const
      {
        return false;
      }

      /**
       * an overrideable function that can be used to signal if the jack is visible or not under certain circumstances
       * @return the state of the jack
       */
      virtual bool jackIsVisible()
      {
        return true;
      }
    };

    class DDImage_API TransformJack_KnobI
    {
      public:
        
        typedef bool (EventCallback)(DD::Image::ViewerContext* ctx, 
                                     DD::Image::Op* owner, 
                                     TransformJack_KnobI* transformJack, 
                                     TransformJack_ClientI* client, 
                                     TransformJack_ClientI::TransformEvent event);

        virtual ~TransformJack_KnobI();

        /**
         * collection of flags indicating the functionality of the transform jack
         * default flag means all of the functions will be active
         */
        enum TransformJackFlags
        {
          /**
           * Transform Action flags
           */
          DEFAULT = 0,
          ROTATION = 1 << 0,
          TRANSLATION = 1 << 1,
          SCALE = 1 << 2,
          SKEW = 1 << 3,
          CORNER_PIN = 1 << 4,

          /**
           * Transform Source Flags
           */
          TOP = 1 << 5,
          BOTTOM = 1 << 6,
          LEFT = 1 << 7,
          RIGHT = 1 << 8,
          CENTER = 1 << 9,

          // The following is not a flag, so shouldn't be bit-shited.
          // It just represents the index of the next available bit.
          // If you add a new flag above, make sure you change this value!
          FIRST_AVAILABLE_TRANSFORM_FLAG_BIT =  10,

          /**
           * Transform Draw Flags
           */
          DRAW_BOUNDING_BOX = 1 << 10,
          DRAW_SCALE_CIRCLE = 1 << 11,
          DRAW_PIVOT_POINT = 1 << 12,
          DRAW_NON_UNIFORM_SCALE = 1 << 13,
          DRAW_SKEW_HANDLE = 1 << 14,
          DRAW_SIDE_ROTATION_HANDLE = 1 << 15,

          DISABLE_AUTO_BUILD_HANDLES = 1 << 16
        };

        /**
         * starts the transform jack processing, this function resets the current state of the transform jack
         * @param topLeft the top left position
         * @param bottomRight the bottom right position
         * @param client the client to be registered as listener , if the client is already registered its current bounding box is overwritten
         */
        virtual void startTransformJackProcessing( const Vector2& topLeft, const Vector2& bottomRight, TransformJack_ClientI* client ) = 0;

        /**
         * starts the transform jack processing, this function resets the current state of the transform jack
         * @param topLeft the top left position
         * @param bottomRight the bottom right position
         * @param transform matrix to transform the bounding box by before rendering
         * @param client the client to be registered as listener , if the client is already registered its current bounding box is overwritten
         */
        virtual void startTransformJackProcessing( const Vector2& topLeft, const Vector2& bottomRight, const DD::Image::Matrix4& transform, TransformJack_ClientI* client) = 0;

        /**
         * function that can query whether or not the transform jack is in an active or inactive state
         * @return a bool flag indicating if the transform jack is active
         */
        virtual bool isActive(TransformJack_ClientI* client) const  = 0;

        /**
         * sets the flags for the transformjack knob, this will reset and replace 
         * the current set flags
         * @param flags the flags to be set
         */
        virtual void setFlags( unsigned int flags ) = 0;

        /**
         * gets the current set of flags 
         * @return the current set flags
         */
        virtual unsigned int getFlags() const = 0;

        /**
         * gets the current centre point of the boundary box
         * @return Vector2
         */
        virtual const Vector2& getCenterPoint(TransformJack_ClientI* client) const = 0;

        /**
         * gets the current pivot point
         * @return Vector2
         */
        virtual const Vector2& getPivotPoint(TransformJack_ClientI* client) const = 0;

        /**
         * gets the current rotation value of the transform jack
         * @return float in deg
         */
        virtual float getRotation(TransformJack_ClientI* client) const = 0;

        /**
         * sets the current rotation value of the transform jack
         * @param rotation in degrees
         */
        virtual void setRotation(TransformJack_ClientI* client, float rotation) = 0;

        /**
         * gets the last delta rotation value of the transform jack
         * @return float in deg
         */
        virtual float getDeltaRotation(TransformJack_ClientI* client) const = 0;

        /**
         * gets the current delta translation
         * @return a vector2 representing the current delta translation
         */
        virtual const Vector2& getDeltaTranslation(TransformJack_ClientI* client) const = 0;


        /**
         * this should be used to get an up to date center
         */
        virtual const Vector2& getScaleCenter(TransformJack_ClientI* client)  const = 0;

        /**
         * returns the transformation delta matrix for the current stage of the transform jack
         */
        virtual const Matrix4& getTransformMatrix(TransformJack_ClientI* client) const = 0;

        /**
         * gets the corners of the current bounding box in screen space corrdinates.
         * the corners are returned in clockwise order.
         * @param ctx current viewer context
         * @param tl first corner
         * @param tr second corner
         * @param br third corner
         * @param bl fourth corner
         */
        virtual void getBoundingBox( DD::Image::ViewerContext* ctx, TransformJack_ClientI* client, DD::Image::Vector2& tl, DD::Image::Vector2& tr, DD::Image::Vector2& br, DD::Image::Vector2& bl  ) const = 0;

        /**
         * gets the corners for the initial bounding box
         * the corners are returned in clockwise order.
         * @param ctx current viewer context
         * @param tl first corner
         * @param tr second corner
         * @param br third corner
         * @param bl fourth corner
         */
        virtual void getStartingBoundingBox( DD::Image::ViewerContext* ctx, TransformJack_ClientI* client, DD::Image::Vector2& tl, DD::Image::Vector2& tr, DD::Image::Vector2& br, DD::Image::Vector2& bl  ) const = 0;

        /**
         * transforms the point with the corner pin transform
         * @param point the point to be transformed
         * @return flag indicating whether the point was transformed or not
         */
        virtual bool transformCornerPinPoint( TransformJack_ClientI* client, DD::Image::Vector2& point ) const = 0;


        /**
         * gets the current delta scale
         * @return a vector2 pointing to the current delta scale
         */
        virtual const Vector2& getDeltaScale(TransformJack_ClientI* client) const = 0;


        /**
         * gets the current delta skew
         * @return a vector2 pointing to the current delta skew
         */
        virtual const Vector2& getDeltaSkew(TransformJack_ClientI* client) const = 0;

        /**
         * offsets the transform jack, this can be used
         * to offset the bounding box after external changes
         * @param x the x parameter offset
         * @param y the y parameter offset
         */
        virtual void translateCenter( TransformJack_ClientI* client, float x, float y ) = 0;

        /**
         * offsets the transform jack's pivot point, this can be used
         * to offset the pivot point after external changes
         * @param x the x parameter offset
         * @param y the y parameter offset
         */
        virtual void translatePivot( TransformJack_ClientI* client, float x, float y ) = 0;

        /**
         * finishes the selection stage and changes the state to PAUSED, a further call to setBoundingBox
         * will clear all of the cached transform values.
         */
        virtual void finishedSelection( TransformJack_ClientI* client ) = 0;

        /**
         * queries whether there is a client for the transform jack
         * @param client the pointer to the client to check against
         * @return flag indicating whether the client exists
         */
        virtual bool hasClient( TransformJack_ClientI* client ) const  = 0;

        /**
         * retrieves the current client count
         * @return the number of clients
         */
        virtual size_t clientCount() const = 0;

        /**
         * queries whether the transformjack is visible or not
         * @return a flag indicating whether its visible or not
         */
        virtual bool isVisible(TransformJack_ClientI* client) const = 0;

        /**
         * sets the visibility of the transform jack
         * @param visible a flag indicating whether or not this is visible
         */
        virtual void setVisibility( TransformJack_ClientI* client, bool visible ) = 0;

        /**
         * resets the transform jack back to it's initial state, clearing all clients
         */
        virtual void reset()
        {
        }

        /**
         * set an override color for all handles drawn by the transform jack
         */
        virtual void setColorOverride(unsigned int color)
        {
        }

        /**
         * set a scale factor for the size of the pivot point handle
         */
        virtual void setPivotPointHandleScale(float scale)
        {
        }

        /**
         * Set a callback for event handling. If a callback is set, any transform jack events
         * will be sent to it. Setting this callback will mean that transform events will not be 
         * sent to the clients individually, replacing TransformJack_ClientI::transformEvent, 
         * TransformJack_ClientI::aboutToChange, and TransformJack_ClientI::finishedChange.
         */
        virtual void setEventCallback(DD::Image::Op* owner, EventCallback* cb)
        {
        }
    };
   
  }
}

#endif // TRANSFORMJACKKNOBI_H
