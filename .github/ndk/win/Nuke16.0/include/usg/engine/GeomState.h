// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_GEOMSTATE_H
#define USG_ENGINE_GEOMSTATE_H

#include "usg/engine/api.h"
#include "fdk/base/Hash.h"

namespace usg
{

  struct USG_ENGINE_API GeomState
  {
    fdk::Hash defineGeometryState;  //!< Edit Layer global prim/attribute topology - causes Stage recompose!
    fdk::Hash modifyValuesState;    //!< Edit Layer global attribute contents modifications (no Stage recompose)
    fdk::Hash editStageState;       //!< Edit Stage's current state (no Stage recompose)

    //! Default ctor leaves all hashes at their default value (~0xLL)
    GeomState() {}
    //! Set all hashes to a specific value.
    GeomState(const fdk::Hash& init);

    //! Return a single hash consisting of all non-default hashes appended together.
    fdk::Hash  hash() const;
    operator fdk::Hash() const { return hash(); }

    bool operator == (const GeomState& b) const;
    bool operator != (const GeomState& b) const { return !(*this == b); }

    bool  isDefineGeometryDefault() const { return defineGeometryState.isDefault(); }
    bool  isModifyValuesDefault()   const { return modifyValuesState.isDefault(); }
    bool  isEditStageDefault()      const { return editStageState.isDefault(); }

    //! Are all hashes at the fdk::Hash default value? (this won't work if hashes set to a different initial value)
    bool  isDefault() const;

    //! Set all hashes to a specific value.
    void  set(const fdk::Hash& h);

    //! Copy all the hashes from \a b GeomState.
    void  set(const GeomState& b);

    //! Reset all the hashes to the fdk::Hash default value (~0xLL)
    void  reset();

    //! Append each of \a b GeomState's hashes.
    void  append(const GeomState& b);

    //! Append each of \a b GeomState's hashes if they're not at the default value (~0xLL)
    void  appendIfNotDefault(const GeomState& b);

    //! If \a a's hashes are not default they initialize this GeomState's hashes, then
    //! \a b's hashes are appended, otherwise if \a a's hashes are default then \a b's
    //! hashes initialize this GeomState's hashes and \a a's are ignored.
    static GeomState  combineIfNotDefault(const GeomState& a, const GeomState& b);


    friend USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomState&);
  };


  USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomState&);


inline
GeomState::GeomState(const fdk::Hash& init) :
  defineGeometryState(init),
  modifyValuesState(init),
  editStageState(init)
{
  //
}

inline bool
GeomState::operator == (const GeomState& b) const
{
  return (defineGeometryState == b.defineGeometryState &&
          modifyValuesState == b.modifyValuesState &&
          editStageState == b.editStageState);
}

inline bool
GeomState::isDefault() const
{
  return (defineGeometryState.isDefault() &&
          modifyValuesState.isDefault() &&
          editStageState.isDefault());
}


inline void
GeomState::set(const fdk::Hash& h)
{
  defineGeometryState = h;
  modifyValuesState = h;
  editStageState = h;
}

inline void
GeomState::set(const GeomState& b)
{
  defineGeometryState = b.defineGeometryState;
  modifyValuesState = b.modifyValuesState;
  editStageState = b.editStageState;
}

inline void
GeomState::reset()
{
  defineGeometryState.reset();
  modifyValuesState.reset();
  editStageState.reset();
}

inline void
GeomState::append(const GeomState& b)
{
  defineGeometryState.append(b.defineGeometryState);
  modifyValuesState.append(b.modifyValuesState);
  editStageState.append(b.editStageState);
}

inline void
GeomState::appendIfNotDefault(const GeomState& b)
{
  defineGeometryState.appendIfNotDefault(b.defineGeometryState);
  modifyValuesState.appendIfNotDefault(b.modifyValuesState);
  editStageState.appendIfNotDefault(b.editStageState);
}

inline GeomState
GeomState::combineIfNotDefault(const GeomState& a, const GeomState& b)
{
  GeomState s;
  s.defineGeometryState = fdk::Hash::combineIfNotDefault(a.defineGeometryState, b.defineGeometryState);
  s.modifyValuesState = fdk::Hash::combineIfNotDefault(a.modifyValuesState, b.modifyValuesState);
  s.editStageState = fdk::Hash::combineIfNotDefault(a.editStageState, b.editStageState);
  return s;
}

inline fdk::Hash
GeomState::hash() const
{
  fdk::Hash hash;
  hash.appendIfNotDefault(defineGeometryState);
  hash.appendIfNotDefault(modifyValuesState);
  hash.appendIfNotDefault(editStageState);
  return hash;
}


}

#endif
