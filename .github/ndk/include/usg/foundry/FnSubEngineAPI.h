// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_FNUSDSHIM_FNSUBENGINEAPI_H
#define USG_FNUSDSHIM_FNSUBENGINEAPI_H

#include <map>
#include <string>
#include <vector>

#include "usg/api.h"

#include "usg/geom/Prim.h"

namespace usg {

  // class FnSubEngineAPI
  //
  // Attributes:
  //     opaque fnSubEngine:__INSTANCE_NAME__:
  //     uniform opaque fnSubEngine:__INSTANCE_NAME__:engine:args
  //     uniform string fnSubEngine:__INSTANCE_NAME__:engine:EngineName
  //
  // Example SubEngine schema should look like:
  //
  // token fnSubEngine:__INSTANCE_NAME__:engine:EngineName = "TransformSetEngine"
  // opaque fnSubEngine:__INSTANCE_NAME__:engine:args
  // double3 fnSubEngine:__INSTANCE_NAME__:engine:args:translate = (1, 2, 3)
  // double3 fnSubEngine:__INSTANCE_NAME__:engine:args:rotate = (45, 0, 0)

  class USG_API FnSubEngineAPI
  {
  public:
    using SampledValues = std::unordered_map<fdk::TimeValue, usg::Value>;
    // Args must be an ordered map, since we hash the args, and the hash will be different even if
    // the values are the same if they're appended to the hash in a different order.
    using Args = std::map<usg::Token, SampledValues>;
    FnSubEngineAPI(const Prim& prim, const Token& name);
    ~FnSubEngineAPI();

    bool validate();
    explicit operator bool() const { return _prim.isValid(); }
    Token name() const { return _name; }
    Prim prim() const { return _prim; }

    /** @brief Applies the API schema to the current prim.
     */
    void apply();

    /** @brief Removes the API schema to the current prim.
     */
    void remove();

    Attribute createEngineNameAttr(const Value& defaultValue = Value());
    Attribute getEngineNameAttr() const;
    void setEngineName(const usg::Token& value);
    usg::Token getEngineName() const;
    void removeEngineName();

    /** @brief Returns all the Attributes relating to the Args for the current FnSubEngineAPI.
     *
     * Returns all of the attributes which are nested under the namespace:
     * ``fnSubEngine:__INSTANCE_NAME__:engine:args:``. Where the __INSTANCE_NAME__ matches the
     * current object instances ``name()``.
     *
     * @return A vector of `usg::Attribute` containing the matching attributes, empty if none are
     * found.
     */
    std::vector<Attribute> getArgsAttributes() const;

    /** @brief Returns all the Attribute Names for Args for the current FnSubEngineAPI.
     *
     * Returns all of the attribute names which are nested under the namespace:
     * ``fnSubEngine:__INSTANCE_NAME__:engine:args:``. Where the __INSTANCE_NAME__ matches the
     * current object instances ``name()``.
     *
     * @return A vector of `usg::Token` containing all the attribute names, empty if none are found.
     */
    std::vector<Token> getArgsAttributeNames() const;

    /** @brief Returns a map of attribute names to attributes relating to the args of the the
     * current FnSubEngineAPI.
     *
     * Returns all of the attribute names and Attributes which are nested under the namespace:
     * ``fnSubEngine:__INSTANCE_NAME__:engine:args:``. Where the __INSTANCE_NAME__ matches the
     * current object instances ``name()``.
     *
     * @param stripArgPrefix Determines whether to strip the
     *  ``fnSubEngine:__INSTANCE_NAME__:engine:args:``, this is true by default.
     * @return A map of attribute names to attributes, empty if none are found.
     */
    std::unordered_map<Token, Attribute>
    getArgsAttributeMap(const bool stripArgPrefix = true) const;

    /** @brief Creates or Updates the FnSubEngineAPI attributes and schemas for the current prim.
     *
     * Creates or update the FnSubEngineAPI Attributes. Will also apply or remove the API schema
     * based on whether there's enough valid arguments provided. It will remove any attributes for
     * arguments for this instance if the args are not found in the new args map.
     *
     * @param engineName The name of the engine to store within the ``:engine:name`` suffixed
     *  attribute.
     * @param args The map of args to store as attributes under ``:engine:args:`` suffix utilises
     *  the token as the final suffix of the attribute name. It is expected the sample time is 0.
     * @param usdSampleTime The time sample at which to store the args at.
     */
    void createOrUpdateSubEngineAttrs(const usg::Token& engineName, const Args args,
                                      fdk::TimeValue usdSampleTime = fdk::defaultTimeValue());

    /** @brief Returns the EngineArgs in a pair keyed by the Engine name for this FnSubengineAPI.
     *
     * @param timeSample The time sample at which to query the stored attributes.
     * @param removeThisSchema If ``true``, we also remove this current FnSubEngineAPI from the prim
     *  as we loop through reading the attributes. This is ``false`` by default.
     *
     * @return A pair containing the Engine name as the key, and the Args as the value.
     */
    std::pair<Token, Args> getEngineArgs(const fdk::TimeValue timeSample,
                                         bool removeThisSchema = false);

    /** @brief Returns all of the EngineArgs in a map keyed by the FnSubEngineAPI name from the
     * provided prim.
     *
     * @param prim The prim to find all the applied schemas on.
     * @param timeSample The time sample at which to query the stored attributes.
     * @param removeThisSchema If ``true``, we also remove this current FnSubEngineAPI from the prim
     *  as we loop through reading the attributes. This is ``false`` by default.
     *
     * @return A map keyed by the FnSubEngineAPI instance name with the value being the pair
     *   returned by the ``getEngineArgs`` method, where the key is the Engine name and its Args are
     *   the value
     */
    static std::unordered_map<Token, std::pair<Token, Args>>
    getAllEngineArgs(const Prim& prim, const fdk::TimeValue timeSample,
                     bool removeThisSchema = false);

    /** @brief Returns all the names of the FnSubEngineAPI appliedSchemas. Strips the
     * ``FnSubEngineAPI:`` prefix.
     *
     * @param prim The prim to search for all the FnSubEngineAPI schemas.
     *
     * @return A vector of tokens with the names of the FnSubEngineAPI applied schemas with the
     *  prefix stripped from the applied schema.
     */
    static std::vector<Token> getAllNames(const Prim& prim);

    /** @brief Returns all the the FnSubEngineAPI objects for all the multi-apply schemas on this
     *  prim.
     *
     * @param prim The prim to search for all the FnSubEngineAPI schemas.
     *
     * @return A vector of tokens with the names of the FnSubEngineAPI applied schemas with the
     *  prefix stripped from the applied schema.
     */
    static std::vector<FnSubEngineAPI> getAll(const Prim& prim);

  protected:
    Prim _prim;
    Token _name;
    // A cached property prefix.
    std::string _argPropertyPrefix;

  private:
    static constexpr std::string_view API_PREFIX{ "FnSubEngineAPI:" };
  };

} // namespace usg

#endif
