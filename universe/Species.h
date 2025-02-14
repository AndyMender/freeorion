#ifndef _Species_h_
#define _Species_h_


#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/optional/optional.hpp>
#include "ConstantsFwd.h"
#include "EnumsFwd.h"
#include "../util/Enum.h"
#include "../util/Export.h"
#include "../util/Pending.h"


namespace Condition {
    struct Condition;
}
namespace Effect {
    class EffectsGroup;
}
class ObjectMap;

//! Environmental suitability of planets for a particular Species
FO_ENUM(
    (PlanetEnvironment),
    ((INVALID_PLANET_ENVIRONMENT, -1))
    ((PE_UNINHABITABLE))
    ((PE_HOSTILE))
    ((PE_POOR))
    ((PE_ADEQUATE))
    ((PE_GOOD))
    ((NUM_PLANET_ENVIRONMENTS))
)


/** A setting that a ResourceCenter can be assigned to influence what it
  * produces.  Doesn't directly affect the ResourceCenter, but effectsgroups
  * can use activation or scope conditions that check whether a potential
  * target has a particular focus.  By this method, techs or buildings or
  * species can act on planets or other ResourceCenters depending what their
  * focus setting is. */
class FO_COMMON_API FocusType {
public:
    FocusType() = default;
    FocusType(std::string& name, std::string& description,
              std::unique_ptr<Condition::Condition>&& location,
              std::string& graphic);
    FocusType(std::string&& name, std::string&& description,
              std::unique_ptr<Condition::Condition>&& location,
              std::string&& graphic);
    ~FocusType(); // needed due to forward-declared Condition held in unique_ptr

    bool operator==(const FocusType& rhs) const;
    bool operator!=(const FocusType& rhs) const
    { return !(*this == rhs); }

    [[nodiscard]] const std::string&          Name() const        { return m_name; }          ///< returns the name for this focus type
    [[nodiscard]] const std::string&          Description() const { return m_description; }   ///< returns a text description of this focus type
    [[nodiscard]] const Condition::Condition* Location() const    { return m_location.get(); }///< returns the condition that determines whether an UniverseObject can use this FocusType
    [[nodiscard]] const std::string&          Graphic() const     { return m_graphic; }       ///< returns the name of the grapic file for this focus type
    [[nodiscard]] std::string                 Dump(uint8_t ntabs = 0) const;           ///< returns a data file format representation of this object

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] unsigned int GetCheckSum() const;

private:
    std::string                                 m_name;
    std::string                                 m_description;
    std::shared_ptr<const Condition::Condition> m_location; // TODO: make unique_ptr - requires tweaking SpeciesParser to not require copies
    std::string                                 m_graphic;
};


/** A predefined type of population that can exist on a PopulationCenter.
  * Species have associated sets of EffectsGroups, and various other
  * properties that affect how the object on which they reside functions.
  * Each kind of Species must have a \a unique name string, by which it can be
  * looked up using GetSpecies(). */
class FO_COMMON_API Species {
public:
    Species(std::string&& name, std::string&& desc,
            std::string&& gameplay_desc, std::vector<FocusType>&& foci,
            std::string&& default_focus,
            std::map<PlanetType, PlanetEnvironment>&& planet_environments,
            std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
            std::unique_ptr<Condition::Condition>&& combat_targets,
            bool playable, bool native, bool can_colonize, bool can_produce_ships,
            const std::set<std::string>& tags,
            std::set<std::string>&& likes, std::set<std::string>&& dislikes,
            std::string&& graphic,
            double spawn_rate = 1.0, int spawn_limit = 99999);

    ~Species();

    bool operator==(const Species& rhs) const;
    bool operator!=(const Species& rhs) const
    { return !(*this == rhs); }

    [[nodiscard]] const std::string&            Name() const          { return m_name; }                  ///< returns the unique name for this type of species
    [[nodiscard]] const std::string&            Description() const   { return m_description; }           ///< returns a text description of this type of species
    /** returns a text description of this type of species */
    [[nodiscard]] std::string                   GameplayDescription() const;

    [[nodiscard]] const Condition::Condition*   Location() const      { return m_location.get(); }        ///< returns the condition determining what planets on which this species may spawn
    [[nodiscard]] const Condition::Condition*   CombatTargets() const { return m_combat_targets.get(); }  ///< returns the condition for possible targets. may be nullptr if no condition was specified.

    [[nodiscard]] std::string                   Dump(uint8_t ntabs = 0) const;                                   ///< returns a data file format representation of this object
    [[nodiscard]] const std::vector<FocusType>& Foci() const          { return m_foci; }                  ///< returns the focus types this species can use
    [[nodiscard]] const std::string&            DefaultFocus() const  { return m_default_focus; }         ///< returns the name of the planetary focus this species defaults to. Used for new colonies and uninvaded natives.
    [[nodiscard]] const std::map<PlanetType, PlanetEnvironment>& PlanetEnvironments() const { return m_planet_environments; } ///< returns a map from PlanetType to the PlanetEnvironment this Species has on that PlanetType
    [[nodiscard]] PlanetEnvironment             GetPlanetEnvironment(PlanetType planet_type) const;                     ///< returns the PlanetEnvironment this species has on PlanetType \a planet_type
    [[nodiscard]] PlanetType                    NextBestPlanetType(PlanetType initial_planet_type) const;             ///< returns a best PlanetType for this species from the \a initial_planet_type specified which needs the few steps to reach
    [[nodiscard]] PlanetType                    NextBetterPlanetType(PlanetType initial_planet_type) const;             ///< returns a PlanetType for this species which is a step closer to the best PlanetType than the specified \a initial_planet_type (if such exists)

    /** Returns the EffectsGroups that encapsulate the effects that species of
        this type have. */
    [[nodiscard]] const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    [[nodiscard]] float                           SpawnRate() const       { return m_spawn_rate; }
    [[nodiscard]] int                             SpawnLimit() const      { return m_spawn_limit; }
    [[nodiscard]] bool                            Playable() const        { return m_playable; }          ///< returns whether this species is a suitable starting species for players
    [[nodiscard]] bool                            Native() const          { return m_native; }            ///< returns whether this species is a suitable native species (for non player-controlled planets)
    [[nodiscard]] bool                            CanColonize() const     { return m_can_colonize; }      ///< returns whether this species can colonize planets
    [[nodiscard]] bool                            CanProduceShips() const { return m_can_produce_ships; } ///< returns whether this species can produce ships

    [[nodiscard]] const auto&           Tags() const noexcept { return m_tags; }
    [[nodiscard]] const auto&           PediaTags() const noexcept { return m_pedia_tags; }
    [[nodiscard]] bool                  HasTag(std::string_view tag) const
    { return std::any_of(m_tags.begin(), m_tags.end(), [tag](const auto& t) { return t == tag; }); }
    [[nodiscard]] const auto&           Likes() const    { return m_likes; }
    [[nodiscard]] const auto&           Dislikes() const { return m_dislikes; }
    [[nodiscard]] const std::string&    Graphic() const  { return m_graphic; }       ///< returns the name of the grapic file for this species

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] unsigned int GetCheckSum() const;

private:
    void Init();

    /** This does the heavy lifting for finding the next better or next best planet type.
      * the callback apply_for_best_forward_backward takes three arguments:
      * 1) the best PlanetType easiest to terraform to for this species for the initial_planet_type
      * 2) the number of terraforming steps necessary to reach a best environment clockwise
      * 3) the number of terraforming steps necessary to reach a best environment counter clockwise
      * note: this supports multiple best PlanetTypes - the function ensures there is no
      *       PlanetType with a better environment available which is reachable in less steps
      *       than the returned PlanetType
      *
      * Based on the parameters, the callback needs to return a fitting PlanetType
      * (i.e. either the best PlanetType or the next step into the direction of it).
      *
      * If terraforming is not possible or is not able to give a better result in the end, the
      * current planet type needs to be returned.
      */
    template <typename Func>
    [[nodiscard]] PlanetType TheNextBestPlanetTypeApply(PlanetType initial_planet_type, Func apply_for_best_forward_backward) const;

    std::string m_name;
    std::string m_description;
    std::string m_gameplay_description;

    std::vector<FocusType>                  m_foci;
    std::string                             m_default_focus;
    std::map<PlanetType, PlanetEnvironment> m_planet_environments;

    std::vector<std::shared_ptr<Effect::EffectsGroup>> m_effects;
    std::unique_ptr<Condition::Condition>              m_location;
    std::unique_ptr<Condition::Condition>              m_combat_targets;

    bool  m_playable = true;
    bool  m_native = true;
    bool  m_can_colonize = true;
    bool  m_can_produce_ships = true;
    float m_spawn_rate = 1.0;
    int   m_spawn_limit = 99999;

    const std::string                   m_tags_concatenated;
    const std::vector<std::string_view> m_tags;
    const std::vector<std::string_view> m_pedia_tags;
    std::vector<std::string_view>       m_likes;
    std::vector<std::string_view>       m_dislikes;
    std::string                         m_graphic;
};


/** Holds all FreeOrion species.  Types may be looked up by name. */
class FO_COMMON_API SpeciesManager {
private:
    struct FO_COMMON_API PlayableSpecies
    { bool operator()(const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const; };
    struct FO_COMMON_API NativeSpecies
    { bool operator()(const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const; };

public:
    using SpeciesTypeMap = std::map<std::string, std::unique_ptr<Species>, std::less<>>;
    using CensusOrder = std::vector<std::string>;
    using iterator = SpeciesTypeMap::const_iterator;
    typedef boost::filter_iterator<PlayableSpecies, iterator> playable_iterator;
    typedef boost::filter_iterator<NativeSpecies, iterator>   native_iterator;

    SpeciesManager() = default;

    /** returns the species with the name \a name; you should use the
      * free function GetSpecies() instead, mainly to save some typing. */
    [[nodiscard]] const Species*      GetSpecies(std::string_view name) const;

    /** returns the species with name \a without guarding access to
      * shared state. */
    [[nodiscard]] const Species*      GetSpeciesUnchecked(std::string_view name) const;

    /** iterators for all species */
    [[nodiscard]] iterator            begin() const;
    [[nodiscard]] iterator            end() const;

    /** iterators for playble species. */
    [[nodiscard]] playable_iterator   playable_begin() const;
    [[nodiscard]] playable_iterator   playable_end() const;

    /** iterators for native species. */
    [[nodiscard]] native_iterator     native_begin() const;
    [[nodiscard]] native_iterator     native_end() const;

    /** returns an ordered list of tags that should be considered for census listings. */
    [[nodiscard]] const CensusOrder&  census_order() const;

    /** returns true iff this SpeciesManager is empty. */
    [[nodiscard]] bool                empty() const;

    /** returns the number of species stored in this manager. */
    [[nodiscard]] int                 NumSpecies() const;
    [[nodiscard]] int                 NumPlayableSpecies() const;
    [[nodiscard]] int                 NumNativeSpecies() const;

    /** returns the name of a species in this manager, or an empty string if
      * this manager is empty. */
    [[nodiscard]] const std::string&  RandomSpeciesName() const;

    /** returns the name of a playable species in this manager, or an empty
      * string if there are no playable species. */
    [[nodiscard]] const std::string&  RandomPlayableSpeciesName() const;
    [[nodiscard]] const std::string&  SequentialPlayableSpeciesName(int id) const;

    /** returns a map from species name to a set of object IDs that are the
      * homeworld(s) of that species in the current game. */
    [[nodiscard]] const std::map<std::string, std::set<int>>&
        GetSpeciesHomeworldsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns a map from species name to a map from empire id to each the
      * species' opinion of the empire */
    [[nodiscard]] const std::map<std::string, std::map<int, float>>&
        GetSpeciesEmpireOpinionsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns opinion of species with name \a species_name about empire with
      * id \a empire_id or 0.0 if there is no such opinion yet recorded. */
    [[nodiscard]] float SpeciesEmpireOpinion(const std::string& species_name, int empire_id) const;

    /** returns a map from species name to a map from other species names to the
      * opinion of the first species about the other species. */
    [[nodiscard]] const std::map<std::string, std::map<std::string, float>>&
        GetSpeciesSpeciesOpinionsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns opinion of species with name \a opinionated_species_name about
      * other species with name \a rated_species_name or 0.0 if there is no
      * such opinion yet recorded. */
    [[nodiscard]] float SpeciesSpeciesOpinion(const std::string& opinionated_species_name,
                                              const std::string& rated_species_name) const;

    [[nodiscard]] std::vector<std::string_view> SpeciesThatLike(std::string_view content_name) const;
    [[nodiscard]] std::vector<std::string_view> SpeciesThatDislike(std::string_view content_name) const;

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] unsigned int GetCheckSum() const;

    /** sets the opinions of species (indexed by name string) of empires (indexed
      * by id) as a double-valued number. */
    void SetSpeciesEmpireOpinions(std::map<std::string, std::map<int, float>>&& species_empire_opinions);
    void SetSpeciesEmpireOpinion(const std::string& species_name, int empire_id, float opinion);

    /** sets the opinions of species (indexed by name string) of other species
      * (indexed by name string) as a double-valued number. */
    void SetSpeciesSpeciesOpinions(std::map<std::string,
                                   std::map<std::string, float>>&& species_species_opinions);
    void SetSpeciesSpeciesOpinion(const std::string& opinionated_species,
                                  const std::string& rated_species, float opinion);
    void ClearSpeciesOpinions();

    void AddSpeciesHomeworld(std::string species, int homeworld_id);
    void RemoveSpeciesHomeworld(const std::string& species, int homeworld_id);
    void ClearSpeciesHomeworlds();

    void UpdatePopulationCounter(const ObjectMap& objects);

    [[nodiscard]] const std::map<std::string, std::map<int, float>>&       SpeciesObjectPopulations(int encoding_empire = ALL_EMPIRES) const;
    [[nodiscard]] const std::map<std::string, std::map<std::string, int>>& SpeciesShipsDestroyed(int encoding_empire = ALL_EMPIRES) const;
    void SetSpeciesObjectPopulations(std::map<std::string, std::map<int, float>> sop);
    void SetSpeciesShipsDestroyed(std::map<std::string, std::map<std::string, int>> ssd);

    /** Sets species types to the value of \p future. */
    void SetSpeciesTypes(Pending::Pending<std::pair<SpeciesTypeMap, CensusOrder>>&& future);

private:
    /** sets the homeworld ids of species in this SpeciesManager to those
      * specified in \a species_homeworld_ids */
    void SetSpeciesHomeworlds(std::map<std::string, std::set<int>>&& species_homeworld_ids);

    /** Assigns any m_pending_types to m_species. */
    static void CheckPendingSpeciesTypes();

    std::map<std::string, std::set<int>>                m_species_homeworlds;
    std::map<std::string, std::map<int, float>>         m_species_empire_opinions;
    std::map<std::string, std::map<std::string, float>> m_species_species_opinions;
    std::map<std::string, std::map<int, float>>         m_species_object_populations;
    std::map<std::string, std::map<std::string, int>>   m_species_species_ships_destroyed;

    template <typename Archive>
    friend void serialize(Archive&, SpeciesManager&, unsigned int const);
};

#endif
