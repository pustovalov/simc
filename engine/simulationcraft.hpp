// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================
#ifndef SIMULATIONCRAFT_H
#define SIMULATIONCRAFT_H

#define SC_MAJOR_VERSION "820"
#define SC_MINOR_VERSION "02"
#define SC_VERSION ( SC_MAJOR_VERSION "-" SC_MINOR_VERSION )
#define SC_BETA 0
#if SC_BETA
#define SC_BETA_STR "bfa"
#endif
#define SC_USE_STAT_CACHE

// Platform, compiler and general configuration
#include "config.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <numeric>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <typeinfo>
#include <vector>
#include <bitset>
#include <array>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <atomic>
#include <random>
#if defined( SC_OSX )
#include <Availability.h>
#endif

// Needed for usleep in engine/interface/sc_bcp_api.cpp when default apikey builds are done
#if ! defined( SC_WINDOWS )
#include <unistd.h>
#endif

// Forward Declarations =====================================================

struct absorb_buff_t;
struct action_callback_t;
struct action_priority_t;
struct action_priority_list_t;
struct action_state_t;
struct action_t;
struct actor_t;
struct alias_t;
struct attack_t;
struct benefit_t;
struct buff_t;
struct callback_t;
struct cooldown_t;
struct cost_reduction_buff_t;
class dbc_t;
struct dot_t;
struct event_t;
struct expr_t;
struct gain_t;
struct heal_t;
struct item_t;
struct instant_absorb_t;
struct module_t;
struct pet_t;
struct player_t;
struct plot_t;
struct proc_t;
struct reforge_plot_t;
struct scaling_t;
struct sim_t;
struct special_effect_t;
struct spell_data_t;
struct spell_id_t;
struct spelleffect_data_t;
struct spell_t;
struct stats_t;
struct stat_buff_t;
struct stat_pair_t;
struct travel_event_t;
struct xml_node_t;
class xml_writer_t;
struct real_ppm_t;
struct shuffled_rng_t;
struct ground_aoe_event_t;
namespace spawner {
class base_actor_spawner_t;
}
namespace highchart {
  struct chart_t;
}

#include "dbc/data_enums.hh"
#include "dbc/data_definitions.hh"
#include "util/utf8.h"

// string formatting library
#include "util/fmt/format.h"
#include "util/fmt/ostream.h"
#include "util/fmt/printf.h"

// GSL-Lite: Guideline Support Library, light version
// Lib to assist with CPP Core Guidelines.
#include "util/gsl-lite/gsl-lite.hpp"

// Time class representing ingame time
#include "sc_timespan.hpp"

// Generic programming tools
#include "util/generic.hpp"

// Sample Data
#include "util/sample_data.hpp"

// Timeline
#include "util/timeline.hpp"

// Random Number Generators
#include "util/rng.hpp"

// mutex, thread
#include "util/concurrency.hpp"

#include "sc_enums.hpp"

// Cache Control ============================================================
#include "util/cache.hpp"

#include "sim/sc_profileset.hpp"

#include "player/artifact_data.hpp"
#include "player/azerite_data.hpp"

// Legion-specific "pantheon trinket" system
#include "sim/x6_pantheon.hpp"

// Talent Translation =======================================================

constexpr int MAX_TALENT_ROWS = 7;
constexpr int MAX_TALENT_COLS = 3;
constexpr int MAX_TALENT_SLOTS = MAX_TALENT_ROWS * MAX_TALENT_COLS;


// Utilities ================================================================

#include "sc_util.hpp"

#include "util/stopwatch.hpp"
#include "sim/sc_option.hpp"

// Data Access ==============================================================
const int MAX_LEVEL = 120;
const int MAX_SCALING_LEVEL = 120;
const int MAX_ILEVEL = 1300;

// Include DBC Module
#include "dbc/dbc.hpp"

// Include IO Module
#include "util/io.hpp"

// Report ===================================================================

#include "report/sc_report.hpp"

struct artifact_power_t
{
  artifact_power_t(unsigned rank, const spell_data_t* spell, const artifact_power_data_t* power,
      const artifact_power_rank_t* rank_data) :
      rank_( rank ), spell_( spell ), rank_data_( rank_data ), power_( power )
  { }

  artifact_power_t() :
    artifact_power_t(0, spell_data_t::not_found(), artifact_power_data_t::nil(), artifact_power_rank_t::nil() )
  { }

  unsigned rank_;
  const spell_data_t* spell_;
  const artifact_power_rank_t* rank_data_;
  const artifact_power_data_t* power_;

  operator const spell_data_t*() const
  { return spell_; }

  const artifact_power_data_t* power() const
  { return power_; }

  double value( size_t idx = 1 ) const
  {
    if ( ( rank() == 1 && rank_data_ -> value() == 0.0 ) || rank_data_ -> value() == 0.0 )
    {
      return spell_ -> effectN( idx ).base_value();
    }
    else
    {
      return rank_data_ -> value();
    }
  }

  timespan_t time_value( size_t idx = 1 ) const
  {
    if ( rank() == 1 )
    {
      return spell_ -> effectN( idx ).time_value();
    }
    else
    {
      return timespan_t::from_millis( rank_data_ -> value() );
    }
  }

  double percent( size_t idx = 1 ) const
  { return value( idx ) * .01; }

  const spell_data_t& data() const
  { return *spell_; }

  unsigned rank() const
  { return rank_; }
};


/* Luxurious sample data container with automatic merge/analyze,
 * intended to be used in class modules for custom reporting.
 * Iteration based sampling
 */
struct luxurious_sample_data_t : public extended_sample_data_t, private noncopyable
{
  luxurious_sample_data_t( player_t& p, std::string n );

  void add( double x )
  { buffer_value += x; }

  void datacollection_begin()
  {
    reset_buffer();
  }
  void datacollection_end()
  {
    write_buffer_as_sample();
  }
  player_t& player;
private:
  double buffer_value;
  void write_buffer_as_sample()
  {
    extended_sample_data_t::add( buffer_value );
    reset_buffer();
  }
  void reset_buffer()
  {
    buffer_value = 0.0;
  }
};

// Raid Event

struct raid_event_t : private noncopyable
{
public:
  sim_t* sim;
  std::string name;
  std::string type;
  int64_t num_starts;
  timespan_t first, last;
  double first_pct, last_pct;
  timespan_t cooldown;
  timespan_t cooldown_stddev;
  timespan_t cooldown_min;
  timespan_t cooldown_max;
  timespan_t duration;
  timespan_t duration_stddev;
  timespan_t duration_min;
  timespan_t duration_max;

  // Player filter options
  double     distance_min; // Minimal player distance
  double     distance_max; // Maximal player distance
  bool players_only; // Don't affect pets
  bool force_stop; // Stop immediately at last/last_pct
  double player_chance; // Chance for individual player to be affected by raid event

  std::string affected_role_str;
  role_e     affected_role;
  std::string player_if_expr_str;

  timespan_t saved_duration;
  std::vector<player_t*> affected_players;
  std::unordered_map<size_t, std::unique_ptr<expr_t>> player_expressions;
  std::vector<std::unique_ptr<option_t>> options;

  raid_event_t( sim_t*, const std::string& );

  virtual ~raid_event_t() {}

  virtual bool filter_player( const player_t* );

  void add_option( std::unique_ptr<option_t> new_option )
  { options.insert( options.begin(), std::move(new_option) ); }
  timespan_t cooldown_time();
  timespan_t duration_time();
  timespan_t next_time() const;
  timespan_t until_next() const;
  timespan_t remains() const;
  bool up() const;
  double distance() { return distance_max; }
  double min_distance() { return distance_min; }
  double max_distance() { return distance_max; }
  void schedule();
  virtual void reset();
  void parse_options( const std::string& options_str );
  static std::unique_ptr<raid_event_t> create( sim_t* sim, const std::string& name, const std::string& options_str );
  static void init( sim_t* );
  static void reset( sim_t* );
  static void combat_begin( sim_t* );
  static void combat_end( sim_t* ) {}
  static double evaluate_raid_event_expression(sim_t* s, std::string& type, std::string& filter,
      bool test_filter = false);

  static bool has_raid_event( sim_t*, const std::string& type );
private:
  virtual void _start() = 0;
  virtual void _finish() = 0;
  void activate();
  void deactivate();
  void combat_begin();
  void start();
  void finish();

  bool is_up;
  enum class activation_status_e
  {
    // three different states so we can detect when raid event is deactivated before it is activated.
    not_yet_activated,
    activated,
    deactivated
  } activation_status;
  event_t* cooldown_event;
  event_t* duration_event;
  event_t* start_event;
  event_t* end_event;
};
std::ostream& operator<<(std::ostream&, const raid_event_t&);

// Gear Stats ===============================================================

struct gear_stats_t
{
  double default_value;

  std::array<double, ATTRIBUTE_MAX> attribute;
  std::array<double, RESOURCE_MAX> resource;
  double spell_power;
  double attack_power;
  double expertise_rating;
  double expertise_rating2;
  double hit_rating;
  double hit_rating2;
  double crit_rating;
  double haste_rating;
  double weapon_dps;
  double weapon_speed;
  double weapon_offhand_dps;
  double weapon_offhand_speed;
  double armor;
  double bonus_armor;
  double dodge_rating;
  double parry_rating;
  double block_rating;
  double mastery_rating;
  double resilience_rating;
  double pvp_power;
  double versatility_rating;
  double leech_rating;
  double speed_rating;
  double avoidance_rating;

  gear_stats_t() :
    default_value( 0.0 ), attribute(), resource(),
    spell_power( 0.0 ), attack_power( 0.0 ), expertise_rating( 0.0 ), expertise_rating2( 0.0 ),
    hit_rating( 0.0 ), hit_rating2( 0.0 ), crit_rating( 0.0 ), haste_rating( 0.0 ), weapon_dps( 0.0 ), weapon_speed( 0.0 ),
    weapon_offhand_dps( 0.0 ), weapon_offhand_speed( 0.0 ), armor( 0.0 ), bonus_armor( 0.0 ), dodge_rating( 0.0 ),
    parry_rating( 0.0 ), block_rating( 0.0 ), mastery_rating( 0.0 ), resilience_rating( 0.0 ), pvp_power( 0.0 ),
    versatility_rating( 0.0 ), leech_rating( 0.0 ), speed_rating( 0.0 ),
    avoidance_rating( 0.0 )
  { }

  void initialize( double initializer )
  {
    default_value = initializer;

    range::fill( attribute, initializer );
    range::fill( resource, initializer );

    spell_power = initializer;
    attack_power = initializer;
    expertise_rating = initializer;
    expertise_rating2 = initializer;
    hit_rating = initializer;
    hit_rating2 = initializer;
    crit_rating = initializer;
    haste_rating = initializer;
    weapon_dps = initializer;
    weapon_speed = initializer;
    weapon_offhand_dps = initializer;
    weapon_offhand_speed = initializer;
    armor = initializer;
    bonus_armor = initializer;
    dodge_rating = initializer;
    parry_rating = initializer;
    block_rating = initializer;
    mastery_rating = initializer;
    resilience_rating = initializer;
    pvp_power = initializer;
    versatility_rating = initializer;
    leech_rating = initializer;
    speed_rating = initializer;
    avoidance_rating = initializer;
  }

  friend gear_stats_t operator+( const gear_stats_t& left, const gear_stats_t& right )
  {
    gear_stats_t a = gear_stats_t( left );
    a += right;
    return a;
  }

  gear_stats_t& operator+=( const gear_stats_t& right )
  {
    spell_power += right.spell_power;
    attack_power += right.attack_power;
    expertise_rating += right.expertise_rating;
    expertise_rating2 += right.expertise_rating2;
    hit_rating += right.hit_rating;
    hit_rating2 += right.hit_rating2;
    crit_rating += right.crit_rating;
    haste_rating += right.haste_rating;
    weapon_dps += right.weapon_dps;
    weapon_speed += right.weapon_speed;
    weapon_offhand_dps += right.weapon_offhand_dps;
    weapon_offhand_speed += right.weapon_offhand_speed;
    armor += right.armor;
    bonus_armor += right.bonus_armor;
    dodge_rating += right.dodge_rating;
    parry_rating += right.parry_rating;
    block_rating += right.block_rating;
    mastery_rating += right.mastery_rating;
    resilience_rating += right.resilience_rating;
    pvp_power += right.pvp_power;
    versatility_rating += right.versatility_rating;
    leech_rating += right.leech_rating;
    speed_rating += right.speed_rating;
    avoidance_rating += right.avoidance_rating;
    range::transform ( attribute, right.attribute, attribute.begin(), std::plus<double>() );
    range::transform ( resource, right.resource, resource.begin(), std::plus<double>() );
    return *this;
  }

  void   add_stat( stat_e stat, double value );
  void   set_stat( stat_e stat, double value );
  double get_stat( stat_e stat ) const;
  std::string to_string();
  static double stat_mod( stat_e stat );
};

#include "player/sc_actor_pair.hpp"

struct actor_target_data_t : public actor_pair_t, private noncopyable
{
  struct atd_debuff_t
  {
    buff_t* mark_of_doom;
    buff_t* poisoned_dreams;
    buff_t* fel_burn;
    buff_t* flame_wreath;
    buff_t* thunder_ritual;
    buff_t* brutal_haymaker;
    buff_t* taint_of_the_sea;
    buff_t* solar_collapse;
    buff_t* volatile_magic;
    buff_t* maddening_whispers;
    buff_t* shadow_blades;
    // BFA - Azerite
    buff_t* azerite_globules;
    buff_t* dead_ahead;
    buff_t* battlefield_debuff;
    // BFA - Trinkets
    buff_t* wasting_infection;
    buff_t* everchill;
    buff_t* choking_brine;
    buff_t* razor_coral;
    buff_t* conductive_ink;
    buff_t* luminous_algae;
    // BFA - Essences
    buff_t* blood_of_the_enemy;
    buff_t* condensed_lifeforce;
    buff_t* focused_resolve;
  } debuff;

  struct atd_dot_t
  {
  } dot;

  actor_target_data_t( player_t* target, player_t* source );
};

#include "util/sc_uptime.hpp"
#include "buff/sc_buff.hpp"

#include "sim/sc_expressions.hpp"

// Spell query expression types =============================================

enum expr_data_e
{
  DATA_SPELL = 0,
  DATA_TALENT,
  DATA_EFFECT,
  DATA_TALENT_SPELL,
  DATA_CLASS_SPELL,
  DATA_RACIAL_SPELL,
  DATA_MASTERY_SPELL,
  DATA_SPECIALIZATION_SPELL,
  DATA_ARTIFACT_SPELL,
  DATA_AZERITE_SPELL,
};

struct spell_data_expr_t
{
  std::string name_str;
  sim_t* sim;
  expr_data_e data_type;
  bool effect_query;

  expression::token_e result_tok;
  double result_num;
  std::vector<uint32_t> result_spell_list;
  std::string result_str;

  spell_data_expr_t( sim_t* sim, const std::string& n,
                     expr_data_e dt = DATA_SPELL, bool eq = false,
                     expression::token_e t = expression::TOK_UNKNOWN )
    : name_str( n ),
      sim( sim ),
      data_type( dt ),
      effect_query( eq ),
      result_tok( t ),
      result_num( 0 ),
      result_spell_list(),
      result_str( "" )
  {
  }
  spell_data_expr_t( sim_t* sim, const std::string& n, double constant_value )
    : name_str( n ),
      sim( sim ),
      data_type( DATA_SPELL ),
      effect_query( false ),
      result_tok( expression::TOK_NUM ),
      result_num( constant_value ),
      result_spell_list(),
      result_str( "" )
  {
  }
  spell_data_expr_t( sim_t* sim, const std::string& n,
                     const std::string& constant_value )
    : name_str( n ),
      sim( sim ),
      data_type( DATA_SPELL ),
      effect_query( false ),
      result_tok( expression::TOK_STR ),
      result_num( 0.0 ),
      result_spell_list(),
      result_str( constant_value )
  {
  }
  spell_data_expr_t( sim_t* sim, const std::string& n,
                     const std::vector<uint32_t>& constant_value )
    : name_str( n ),
      sim( sim ),
      data_type( DATA_SPELL ),
      effect_query( false ),
      result_tok( expression::TOK_SPELL_LIST ),
      result_num( 0.0 ),
      result_spell_list( constant_value ),
      result_str( "" )
  {
  }
  virtual ~spell_data_expr_t()
  {
  }
  virtual int evaluate()
  {
    return result_tok;
  }
  const char* name()
  {
    return name_str.c_str();
  }

  virtual std::vector<uint32_t> operator|( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator&( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator-( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  virtual std::vector<uint32_t> operator<( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator>( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator<=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator>=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  virtual std::vector<uint32_t> in( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> not_in( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  static spell_data_expr_t* parse( sim_t* sim, const std::string& expr_str );
  static spell_data_expr_t* create_spell_expression( sim_t* sim, const std::string& name_str );
};


// Iteration data entry for replayability
struct iteration_data_entry_t
{
  double   metric;
  uint64_t seed;
  uint64_t iteration;
  double   iteration_length;
  std::vector <uint64_t> target_health;

  iteration_data_entry_t( double m, double il, uint64_t s, uint64_t h, uint64_t i ) :
    metric( m ), seed( s ), iteration( i ), iteration_length( il )
  { target_health.push_back( h ); }

  iteration_data_entry_t( double m, double il, uint64_t s, uint64_t i ) :
    metric( m ), seed( s ), iteration( i ), iteration_length( il )
  { }

  void add_health( uint64_t h )
  { target_health.push_back( h ); }
};

// Simulation Setup =========================================================

struct player_description_t
{
  // Add just enough to describe a player

  // name, class, talents, gear, professions, actions explicitly stored
  std::string name;
  // etc

  // flesh out API, these functions cannot depend upon sim_t
  // ideally they remain static, but if not then move to sim_control_t
  static void load_bcp    ( player_description_t& /*etc*/ );
  static void load_wowhead( player_description_t& /*etc*/ );
};

struct combat_description_t
{
  std::string name;
  int target_seconds;
  std::string raid_events;
  // etc
};

struct sim_control_t
{
  combat_description_t combat;
  std::vector<player_description_t> players;
  option_db_t options;
};

struct sim_progress_t
{
  int current_iterations;
  int total_iterations;
  double pct() const
  { return std::min( 1.0, current_iterations / static_cast<double>(total_iterations) ); }
};

// Progress Bar =============================================================

struct progress_bar_t
{
  sim_t& sim;
  int steps, updates, interval, update_number;
  double start_time, last_update, max_interval_time;
  std::string status;
  std::string base_str, phase_str;
  size_t work_index, total_work_;
  double elapsed_time;
  size_t time_count;

  progress_bar_t( sim_t& s );
  void init();
  bool update( bool finished = false, int index = -1 );
  void output( bool finished = false );
  void restart();
  void progress();
  void set_base( const std::string& base );
  void set_phase( const std::string& phase );
  void add_simulation_time( double t );
  size_t current_progress() const;
  size_t total_work() const;
  double average_simulation_time() const;

  static std::string format_time( double t );
private:
  size_t compute_total_phases();
  bool update_simple( const sim_progress_t&, bool finished, int index );
  bool update_normal( const sim_progress_t&, bool finished, int index );

  size_t n_stat_scaling_players( const std::string& stat ) const;
  // Compute the number of various option-related phases
  size_t n_plot_phases() const;
  size_t n_reforge_plot_phases() const;
  size_t n_scale_factor_phases() const;
};

/* Encapsulated Vector
 * const read access
 * Modifying the vector triggers registered callbacks
 */
template <typename T>
struct vector_with_callback
{
private:
  std::vector<T> _data;
  std::vector<std::function<void(T)> > _callbacks ;
public:
  /* Register your custom callback, which will be called when the vector is modified
   */
  void register_callback( std::function<void(T)> c )
  {
    if ( c )
      _callbacks.push_back( c );
  }

  typename std::vector<T>::iterator begin()
  { return _data.begin(); }
  typename std::vector<T>::const_iterator begin() const
  { return _data.begin(); }
  typename std::vector<T>::iterator end()
  { return _data.end(); }
  typename std::vector<T>::const_iterator end() const
  { return _data.end(); }

  typedef typename std::vector<T>::iterator iterator;

  void trigger_callbacks(T v) const
  {
    for ( size_t i = 0; i < _callbacks.size(); ++i )
      _callbacks[i](v);
  }

  void push_back( T x )
  { _data.push_back( x ); trigger_callbacks( x ); }

  void find_and_erase( T x )
  {
    typename std::vector<T>::iterator it = range::find( _data, x );
    if ( it != _data.end() )
      erase( it );
  }

  void find_and_erase_unordered( T x )
  {
    typename std::vector<T>::iterator it = range::find( _data, x );
    if ( it != _data.end() )
      erase_unordered( it );
  }

  // Warning: If you directly modify the vector, you need to trigger callbacks manually!
  std::vector<T>& data()
  { return _data; }

  const std::vector<T>& data() const
  { return _data; }

  player_t* operator[]( size_t i ) const
  { return _data[ i ]; }

  size_t size() const
  { return _data.size(); }

  bool empty() const
  { return _data.empty(); }

  void reset_callbacks()
  { _callbacks.clear(); }

private:
  void erase_unordered( typename std::vector<T>::iterator it )
  {
    T _v = *it;
    ::erase_unordered( _data, it );
    trigger_callbacks( _v );
  }

  void erase( typename std::vector<T>::iterator it )
  {
    T _v = *it;
    _data.erase( it );
    trigger_callbacks( _v );
  }
};

/* Unformatted SimC output class.
 */
struct sc_raw_ostream_t {
  template <class T>
  sc_raw_ostream_t & operator<< (T const& rhs)
  { (*_stream) << rhs; return *this; }

  template<typename Format, typename... Args>
  sc_raw_ostream_t& printf(Format&& format, Args&& ... args)
  {
    fmt::fprintf(*get_stream(), std::forward<Format>(format), std::forward<Args>(args)... );
    return *this;
  }

  sc_raw_ostream_t( std::shared_ptr<std::ostream> os ) :
    _stream( os ) {}
  const sc_raw_ostream_t operator=( std::shared_ptr<std::ostream> os )
  { _stream = os; return *this; }
  std::ostream* get_stream()
  { return _stream.get(); }
private:
  std::shared_ptr<std::ostream> _stream;
};

/* Formatted SimC output class.
 */
struct sim_ostream_t
{
  struct no_close {};

  explicit sim_ostream_t( sim_t& s, std::shared_ptr<std::ostream> os ) :
      sim(s),
    _raw( os )
  {
  }
  sim_ostream_t( sim_t& s, std::ostream* os, no_close ) :
      sim(s),
    _raw( std::shared_ptr<std::ostream>( os, dont_close ) )
  {}
  const sim_ostream_t operator=( std::shared_ptr<std::ostream> os )
  { _raw = os; return *this; }

  sc_raw_ostream_t& raw()
  { return _raw; }

  std::ostream* get_stream()
  { return _raw.get_stream(); }
  template <class T>
  sim_ostream_t & operator<< (T const& rhs);

  template<typename Format, typename... Args>
  sim_ostream_t& printf(Format&& format, Args&& ... args);

  /**
   * Print using fmt libraries python-like formatting syntax.
   */
  template<typename Format, typename... Args>
  sim_ostream_t& print(Format&& format, Args&& ... args)
  {
    *this << fmt::format(std::forward<Format>(format), std::forward<Args>(args)... );
    return *this;
  }
private:
  static void dont_close( std::ostream* ) {}
  sim_t& sim;
  sc_raw_ostream_t _raw;
};

#ifndef NDEBUG
#define ACTOR_EVENT_BOOKKEEPING 1
#endif

// Event Manager ============================================================

struct event_manager_t
{
  sim_t* sim;
  timespan_t current_time;
  uint64_t events_remaining;
  uint64_t events_processed;
  uint64_t total_events_processed;
  uint64_t max_events_remaining;
  unsigned timing_slice, global_event_id;
  std::vector<event_t*> timing_wheel;
  event_t* recycled_event_list;
  int    wheel_seconds, wheel_size, wheel_mask, wheel_shift;
  double wheel_granularity;
  timespan_t wheel_time;
  std::vector<event_t*> allocated_events;

  stopwatch_t event_stopwatch;
  bool monitor_cpu;
  bool canceled;
#ifdef EVENT_QUEUE_DEBUG
  unsigned max_queue_depth, n_allocated_events, n_end_insert, n_requested_events;
  uint64_t events_traversed, events_added;
  std::vector<std::pair<unsigned, unsigned> > event_queue_depth_samples;
  std::vector<unsigned> event_requested_size_count;
#endif /* EVENT_QUEUE_DEBUG */

  event_manager_t( sim_t* );
 ~event_manager_t();
  void* allocate_event( std::size_t size );
  void recycle_event( event_t* );
  void add_event( event_t*, timespan_t delta_time );
  void reschedule_event( event_t* );
  event_t* next_event();
  bool execute();
  void cancel();
  void flush();
  void init();
  void reset();
  void merge( event_manager_t& other );
};

// Simulation Engine ========================================================

struct sim_t : private sc_thread_t
{
  event_manager_t event_mgr;

  // Output
  sim_ostream_t out_log;
  sim_ostream_t out_debug;
  bool debug;

  /**
   * Error on unknown options (default=false)
   *
   * By default Simulationcraft will ignore unknown sim, player, item, or action-scope options.
   * Enable this to hard-fail the simulator option parsing if an unknown option name is used for a
   * given scope.
   **/
  bool strict_parsing;

  // Iteration Controls
  timespan_t max_time, expected_iteration_time;
  double vary_combat_length;
  int current_iteration, iterations;
  bool canceled;
  double target_error;
  role_e target_error_role;
  double current_error;
  double current_mean;
  int analyze_error_interval, analyze_number;
  // Clean up memory for threads after iterating (defaults to no in normal operation, some options
  // will force-enable the option)
  bool cleanup_threads;

  sim_control_t* control;
  sim_t*      parent;
  bool initialized;
  player_t*   target;
  player_t*   heal_target;
  vector_with_callback<player_t*> target_list;
  vector_with_callback<player_t*> target_non_sleeping_list;
  vector_with_callback<player_t*> player_list;
  vector_with_callback<player_t*> player_no_pet_list;
  vector_with_callback<player_t*> player_non_sleeping_list;
  vector_with_callback<player_t*> healing_no_pet_list;
  vector_with_callback<player_t*> healing_pet_list;
  player_t*   active_player;
  size_t      current_index; // Current active player
  int         num_players;
  int         num_enemies;
  int         num_tanks;
  int         enemy_targets;
  int         healing; // Creates healing targets. Useful for ferals, I guess.
  int global_spawn_index;
  int         max_player_level;
  timespan_t  queue_lag, queue_lag_stddev;
  timespan_t  gcd_lag, gcd_lag_stddev;
  timespan_t  channel_lag, channel_lag_stddev;
  timespan_t  queue_gcd_reduction;
  timespan_t  default_cooldown_tolerance;
  int         strict_gcd_queue;
  double      confidence, confidence_estimator;
  // Latency
  timespan_t  world_lag, world_lag_stddev;
  double      travel_variance, default_skill;
  timespan_t  reaction_time, regen_periodicity;
  timespan_t  ignite_sampling_delta;
  bool        fixed_time, optimize_expressions;
  int         current_slot;
  int         optimal_raid, log, debug_each;
  std::vector<uint64_t> debug_seed;
  bool        save_profiles;
  bool        save_profile_with_actions; // When saving full profiles, include actions or not
  bool        default_actions;
  stat_e      normalized_stat;
  std::string current_name, default_region_str, default_server_str, save_prefix_str, save_suffix_str;
  int         save_talent_str;
  talent_format_e talent_format;
  auto_dispose< std::vector<player_t*> > actor_list;
  std::string main_target_str;
  int         stat_cache;
  int         max_aoe_enemies;
  bool        show_etmi;
  double      tmi_window_global;
  double      tmi_bin_size;
  bool        requires_regen_event;
  bool        single_actor_batch;
  int         progressbar_type;
  int         armory_retries;

  // Target options
  double      enemy_death_pct;
  int         rel_target_level, target_level;
  std::string target_race;
  int         target_adds;
  std::string sim_progress_base_str, sim_progress_phase_str;
  int         desired_targets; // desired number of targets
  bool        enable_taunts;

  // Disable use-item action verification in the simulator
  bool        use_item_verification;

  // Data access
  dbc_t       dbc;

  // Default stat enchants
  gear_stats_t enchant;

  bool challenge_mode; // if active, players will get scaled down to 620 and set bonuses are deactivated
  int timewalk;
  int scale_to_itemlevel; //itemlevel to scale to. if -1, we don't scale down
  bool scale_itemlevel_down_only; // Items below the value of scale_to_itemlevel will not be scaled up.
  bool disable_set_bonuses; // Disables all set bonuses.
  unsigned int disable_2_set; // Disables all 2 set bonuses for this tier/integer that this is set as
  unsigned int disable_4_set; // Disables all 4 set bonuses for this tier/integer that this is set as
  unsigned int enable_2_set;// Enables all 2 set bonuses for the tier/integer that this is set as
  unsigned int enable_4_set; // Enables all 4 set bonuses for the tier/integer that this is set as
  bool pvp_crit; // Sets critical strike damage to 150% instead of 200%
  bool feast_as_dps = true;
  bool auto_attacks_always_land; /// Allow Auto Attacks (white attacks) to always hit the enemy

  // Actor tracking
  int active_enemies;
  int active_allies;

  std::vector<std::unique_ptr<option_t>> options;
  std::vector<std::string> party_encoding;
  std::vector<std::string> item_db_sources;

  // Random Number Generation
  std::unique_ptr<rng::rng_t> _rng;
  std::string rng_str;
  uint64_t seed;
  int deterministic;
  int strict_work_queue;
  int average_range, average_gauss;

  // Raid Events
  std::vector<std::unique_ptr<raid_event_t>> raid_events;
  std::string raid_events_str;
  std::string fight_style;
  size_t add_waves;

  // Buffs and Debuffs Overrides
  struct overrides_t
  {
    // Buff overrides
    int arcane_intellect;
    int battle_shout;
    int power_word_fortitude;

    // Debuff overrides
    int chaos_brand;
    int mystic_touch;
    int mortal_wounds;
    int bleeding;

    // Misc stuff needs resolving
    int    bloodlust;
    std::vector<uint64_t> target_health;
  } overrides;

  struct auras_t
  {
    buff_t* arcane_intellect;
    buff_t* battle_shout;
    buff_t* power_word_fortitude;
  } auras;

  // Expansion specific custom parameters. Defaults in the constructor.
  struct legion_opt_t
  {
    // Legion
    int                 infernal_cinders_users = 1;
    int                 engine_of_eradication_orbs = 4;
    int                 void_stalkers_contract_targets = -1;
    double              specter_of_betrayal_overlap = 1.0;
    std::vector<double> cradle_of_anguish_resets;
    std::string         pantheon_trinket_users;
    timespan_t          pantheon_trinket_interval = timespan_t::from_seconds( 1.0 );
    double              pantheon_trinket_interval_stddev = 0.0;
    double              archimondes_hatred_reborn_damage = 1.0;
  } legion_opts;

  struct bfa_opt_t
  {
    /// Number of allies affected by Jes' Howler buff
    unsigned            jes_howler_allies = 4;
    /// Chance to spawn the rare droplet
    double              secrets_of_the_deep_chance = 0.1; // TODO: Guessed, needs validation
    /// Chance that the player collects the droplet, defaults to always
    double              secrets_of_the_deep_collect_chance = 1.0;
    /// Gutripper base RPPM when target is above 30%
    double              gutripper_default_rppm = 2.0;
    /// Initial stacks for Archive of the Titans
    int                 initial_archive_of_the_titans_stacks = 0;
    /// Number of Reorigination array stats on the actors in the sim
    int                 reorigination_array_stacks = 0;
    /// Allow Reorigination Array to ignore scale factor stat changes (default false)
    bool                reorigination_array_ignore_scale_factors = false;
    /// Chance to pick up visage spawned by Seductive Power
    double              seductive_power_pickup_chance = 1.0;
    /// Initial stacks for Seductive Power buff
    int                 initial_seductive_power_stacks = 0;
    /// Randomize Variable Intensity Gigavolt Oscillating Reactor start-of-combat oscillation
    bool                randomize_oscillation = true;
    /// Automatically use Oscillating Overload on max stack, true = yes if no use_item, 0 = no
    bool                auto_oscillating_overload = true;
    /// Is the actor in Zuldazar? Relevant for one of the set bonuses.
    bool                zuldazar = false;
    /// Treacherous Covenant update period.
    timespan_t          covenant_period = 1.0_s;
    /// Chance to gain the buff on each Treacherous Covenant update.
    double              covenant_chance = 1.0;
    /// Chance to gain a stack of Incandescent Sliver each time it ticks.
    double              incandescent_sliver_chance = 1.0;
    /// Fight or Flight proc attempt period
    timespan_t          fight_or_flight_period = 1.0_s;
    /// Chance to gain the buff on each Fight or Flight attempt
    double              fight_or_flight_chance = 0.0;
    /// Chance of being silenced by Harbinger's Inscrutable Will projectile
    double              harbingers_inscrutable_will_silence_chance = 0.0;
    /// Chance avoiding Harbinger's Inscrutable Will projectile by moving
    double              harbingers_inscrutable_will_move_chance = 1.0;
    /// Chance player is above 60% HP for Leggings of the Aberrant Tidesage damage proc
    double              aberrant_tidesage_damage_chance = 1.0;
    /// Chance player is above 90% HP for Fa'thuul's Floodguards damage proc
    double              fathuuls_floodguards_damage_chance = 1.0;
    /// Chance player is above 90% HP for Grips of Forgotten Sanity damage proc
    double              grips_of_forsaken_sanity_damage_chance = 1.0;
    /// Chance player takes damage and loses Untouchable from Stormglide Steps
    double              stormglide_steps_take_damage_chance = 0.0;
    /// Duration of the Lurker's Insidious Gift buff, the player can cancel it early to avoid unnecessary damage. 0 = full duration
    timespan_t          lurkers_insidious_gift_duration = 0_ms;
    /// Expected duration (in seconds) of shield from Abyssal Speaker's Gauntlets. 0 = full duration
    timespan_t          abyssal_speakers_gauntlets_shield_duration = 0_ms;
    /// Expected duration of the absorb provided by Trident of Deep Ocean. 0 = full duration
    timespan_t          trident_of_deep_ocean_duration = 0_ms;
    /// Chance that the player has a higher health percentage than the target for Legplates of Unbound Anguish proc
    double              legplates_of_unbound_anguish_chance = 1.0;
    /// Number of allies with the Loyal to the End azerite trait, default = 4 (max)
    int                 loyal_to_the_end_allies = 0;
    /// Period to check for if an ally dies with Loyal to the End
    timespan_t          loyal_to_the_end_ally_death_timer = 60_s;
    /// Chance on every check to see if an ally dies with Loyal to the End
    double              loyal_to_the_end_ally_death_chance = 0.0;
    /// Number of allies also using the Worldvein Resonance minor
    int                 worldvein_allies = 0;
    /// Chance to proc Reality Shift (normally triggers on moving specific distance)
    double              ripple_in_space_proc_chance = 0.0;
    /// Chance to be in range to hit with Blood of the Enemy major power (12 yd PBAoE)
    double              blood_of_the_enemy_in_range = 1.0;
    /// Period to check for if Undulating Tides gets locked out
    timespan_t          undulating_tides_lockout_timer = 60_s;
    /// Chance on every check to see if Undulating Tides gets locked out
    double              undulating_tides_lockout_chance = 0.0;
    /// Base RPPM for Leviathan's Lure
    double              leviathans_lure_base_rppm = 0.75;
    /// Chance to catch returning wave of Aquipotent Nautilus
    double              aquipotent_nautilus_catch_chance = 1.0;
    /// Chance of having to interrupt casting by moving to void tear from Za'qul's Portal Key
    double              zaquls_portal_key_move_chance = 0.0;
    /// Unleash stacked potency from Anu-Azshara, Staff of the Eternal after X seconds
    timespan_t          anuazshara_unleash_time = 0_ms;
    /// Whether the player is in Nazjatar/Eternal Palace for various effects
    bool                nazjatar = true;
    /// Whether the Shiver Venom Crossbow/Lance should assume the target has the Shiver Venom debuff
    bool                shiver_venom = true;
    /// Storm of the Eternal haste and crit stat split ratio.
    double              storm_of_the_eternal_ratio = 0.05;
    /// How long before combat to start channeling Azshara's Font of Power
    timespan_t font_of_power_precombat_channel = 0_ms;
    /// Hps done while using the Azerite Trait Arcane Heart
    unsigned arcane_heart_hps = 0;
  } bfa_opts;

  // Expansion specific data
  struct legion_data_t
  {
    std::unique_ptr<unique_gear::pantheon_state_t> pantheon_proxy;
  } legion_data;

  // Auras and De-Buffs
  auto_dispose<std::vector<buff_t*>> buff_list;

  // Global aura related delay
  timespan_t default_aura_delay;
  timespan_t default_aura_delay_stddev;

  auto_dispose<std::vector<cooldown_t*>> cooldown_list;

  /// Status of azerite-related effects
  azerite_control azerite_status;

  // Reporting
  progress_bar_t progress_bar;
  std::unique_ptr<scaling_t> scaling;
  std::unique_ptr<plot_t> plot;
  std::unique_ptr<reforge_plot_t> reforge_plot;
  double elapsed_cpu;
  double elapsed_time;
  std::vector<size_t> work_per_thread;
  size_t work_done;
  double     iteration_dmg, priority_iteration_dmg,  iteration_heal, iteration_absorb;
  simple_sample_data_t raid_dps, total_dmg, raid_hps, total_heal, total_absorb, raid_aps;
  extended_sample_data_t simulation_length;
  double merge_time, init_time, analyze_time;
  // Deterministic simulation iteration data collectors for specific iteration
  // replayability
  std::vector<iteration_data_entry_t> iteration_data, low_iteration_data, high_iteration_data;
  // Report percent (how many% of lowest/highest iterations reported, default 2.5%)
  double     report_iteration_data;
  // Minimum number of low/high iterations reported (default 5 of each)
  int        min_report_iteration_data;
  int        report_progress;
  int        bloodlust_percent;
  timespan_t bloodlust_time;
  std::string reference_player_str;
  std::vector<player_t*> players_by_dps;
  std::vector<player_t*> players_by_priority_dps;
  std::vector<player_t*> players_by_hps;
  std::vector<player_t*> players_by_hps_plus_aps;
  std::vector<player_t*> players_by_dtps;
  std::vector<player_t*> players_by_tmi;
  std::vector<player_t*> players_by_name;
  std::vector<player_t*> players_by_apm;
  std::vector<player_t*> players_by_variance;
  std::vector<player_t*> targets_by_name;
  std::vector<std::string> id_dictionary;
  std::map<double, std::vector<double> > divisor_timeline_cache;
  std::string output_file_str, html_file_str, json_file_str;
  std::string reforge_plot_output_file_str;
  std::vector<std::string> error_list;
  int report_precision;
  int report_pets_separately;
  int report_targets;
  int report_details;
  int report_raw_abilities;
  int report_rng;
  int hosted_html;
  int save_raid_summary;
  int save_gear_comments;
  int statistics_level;
  int separate_stats_by_actions;
  int report_raid_summary;
  int buff_uptime_timeline;
  int json_full_states;
  int decorated_tooltips;

  int allow_potions;
  int allow_food;
  int allow_flasks;
  int allow_augmentations;
  int solo_raid;
  bool maximize_reporting;
  std::string apikey, user_apitoken;
  bool distance_targeting_enabled;
  bool ignore_invulnerable_targets;
  bool enable_dps_healing;
  double scaling_normalized;

  // Multi-Threading
  mutex_t merge_mutex;
  int threads;
  std::vector<sim_t*> children; // Manual delete!
  int thread_index;
  computer_process::priority_e process_priority;
  struct work_queue_t
  {
    private:
#ifndef SC_NO_THREADING
    std::recursive_mutex m;
    using G = std::lock_guard<std::recursive_mutex>;
#else
    struct no_m {
      void lock() {}
      void unlock() {}
    };
    struct nop {
      nop(no_m i) { (void)i; }
    };
    no_m m;
    using G = nop;
#endif
    public:
    std::vector<int> _total_work, _work, _projected_work;
    size_t index;

    work_queue_t() : index( 0 )
    { _total_work.resize( 1 ); _work.resize( 1 ); _projected_work.resize( 1 ); }

    void init( int w )    { G l(m); range::fill( _total_work, w ); range::fill( _projected_work, w ); }
    // Single actor batch sim init methods. Batches is the number of active actors
    void batches( size_t n ) { G l(m); _total_work.resize( n ); _work.resize( n ); _projected_work.resize( n ); }

    void flush()          { G l(m); _total_work[ index ] = _projected_work[ index ] = _work[ index ]; }
    int  size()           { G l(m); return index < _total_work.size() ? _total_work[ index ] : _total_work.back(); }
    bool more_work()      { G l(m); return index < _total_work.size() && _work[ index ] < _total_work[ index ]; }
    void lock()           { m.lock(); }
    void unlock()         { m.unlock(); }

    void project( int w )
    {
      G l(m);
      _projected_work[ index ] = w;
#ifdef NDEBUG
      if ( w > _work[ index ] )
      {
      }
#endif
    }

    // Single-actor batch pop, uses several indices of work (per active actor), each thread has it's
    // own state on what index it is simulating
    size_t pop()
    {
      G l(m);

      if ( _work[ index ] >= _total_work[ index ] )
      {
        if ( index < _work.size() - 1 )
        {
          ++index;
        }
        return index;
      }

      if ( ++_work[ index ] == _total_work[ index ] )
      {
        _projected_work[ index ] = _work[ index ];
        if ( index < _work.size() - 1 )
        {
          ++index;
        }
        return index;
      }

      return index;
    }

    // Standard progress method, normal mode sims use the single (first) index, single actor batch
    // sims progress with the main thread's current index.
    sim_progress_t progress( int idx = -1 )
    {
      G l(m);
      size_t current_index = idx;
      if ( idx < 0 )
      {
        current_index = index;
      }

      if ( current_index >= _total_work.size() )
      {
        return sim_progress_t{ _work.back(), _projected_work.back() };
      }

      return sim_progress_t{ _work[ current_index ], _projected_work[ current_index ] };
    }
  };
  std::shared_ptr<work_queue_t> work_queue;

  // Related Simulations
  mutex_t relatives_mutex;
  std::vector<sim_t*> relatives;

  // Spell database access
  std::unique_ptr<spell_data_expr_t> spell_query;
  unsigned           spell_query_level;
  std::string        spell_query_xml_output_file_str;

  std::unique_ptr<mutex_t> pause_mutex; // External pause mutex, instantiated an external entity (in our case the GUI).
  bool paused;

  // Highcharts stuff

  // Vector of on-ready charts. These receive individual jQuery handlers in the HTML report (at the
  // end of the report) to load the highcharts into the target div.
  std::vector<std::string> on_ready_chart_data;

  // A map of highcharts data, added as a json object into the HTML report. JQuery installs handlers
  // to correct elements (toggled elements in the HTML report) based on the data.
  std::map<std::string, std::vector<std::string> > chart_data;

  bool chart_show_relative_difference;
  double chart_boxplot_percentile;

  // List of callbacks to call when an actor_target_data_t object is created. Currently used to
  // initialize the generic targetdata debuffs/dots we have.
  std::vector<std::function<void(actor_target_data_t*)> > target_data_initializer;

  bool display_hotfixes, disable_hotfixes;
  bool display_bonus_ids;

  // Profilesets
  opts::map_list_t profileset_map;
  std::vector<scale_metric_e> profileset_metric;
  std::vector<std::string> profileset_output_data;
  bool profileset_enabled;
  int profileset_work_threads, profileset_init_threads;
  profileset::profilesets_t profilesets;


  sim_t();
  sim_t( sim_t* parent, int thread_index = 0 );
  sim_t( sim_t* parent, int thread_index, sim_control_t* control );
  virtual ~sim_t();

  virtual void run() override;
  int       main( const std::vector<std::string>& args );
  double    iteration_time_adjust() const;
  double    expected_max_time() const;
  bool      is_canceled() const;
  void      cancel_iteration();
  void      cancel();
  void      interrupt();
  void      add_relative( sim_t* cousin );
  void      remove_relative( sim_t* cousin );
  sim_progress_t progress(std::string* phase = nullptr, int index = -1 );
  double    progress( std::string& phase, std::string* detailed = nullptr, int index = -1 );
  void      detailed_progress( std::string*, int current_iterations, int total_iterations );
  void      datacollection_begin();
  void      datacollection_end();
  void      reset();
  void      check_actors();
  void      init_fight_style();
  void      init_parties();
  void      init_actors();
  void      init_actor( player_t* );
  void      init_actor_pets();
  void      init();
  void      analyze();
  void      merge( sim_t& other_sim );
  void      merge();
  bool      iterate();
  void      partition();
  bool      execute();
  void      analyze_error();
  void      analyze_iteration_data();
  void      print_options();
  void      add_option( std::unique_ptr<option_t> opt );
  void      create_options();
  bool      parse_option( const std::string& name, const std::string& value );
  void      setup( sim_control_t* );
  bool      time_to_think( timespan_t proc_time );
  player_t* find_player( const std::string& name ) const;
  player_t* find_player( int index ) const;
  cooldown_t* get_cooldown( const std::string& name );
  void      use_optimal_buffs_and_debuffs( int value );
  expr_t*   create_expression( const std::string& name );
  /**
   * Create error with printf formatting.
   */
  template<typename Format, typename... Args>
  void errorf(Format&& format, Args&& ... args)
  {
    if ( thread_index != 0 )
      return;

    auto s = fmt::sprintf(std::forward<Format>(format), std::forward<Args>(args)... );
    util::replace_all( s, "\n", "" );
    std::cerr << s << "\n";

    error_list.push_back( s );
  }

  /**
   * Create error using fmt libraries python-like formatting syntax.
   */
  template<typename Format, typename... Args>
  void error(Format&& format, Args&& ... args)
  {
    if ( thread_index != 0 )
      return;

    auto s = fmt::format(std::forward<Format>(format), std::forward<Args>(args)... );
    util::replace_all( s, "\n", "" );
    std::cerr << s << "\n";

    error_list.push_back( s );
  }
  void abort();
  void combat();
  void combat_begin();
  void combat_end();
  void add_chart_data( const highchart::chart_t& chart );
  bool      has_raid_event( const std::string& name ) const;

  // Activates the necessary actor/actors before iteration begins.
  void activate_actors();

  timespan_t current_time() const
  { return event_mgr.current_time; }
  static double distribution_mean_error( const sim_t& s, const extended_sample_data_t& sd )
  { return s.confidence_estimator * sd.mean_std_dev; }
  void register_target_data_initializer(std::function<void(actor_target_data_t*)> cb)
  { target_data_initializer.push_back( cb ); }
  rng::rng_t& rng() const
  { return *_rng; }
  double averaged_range( double min, double max )
  {
    if ( average_range ) return ( min + max ) / 2.0;
    return rng().range( min, max );
  }

  // Thread id of this sim_t object
#ifndef SC_NO_THREADING
  std::thread::id thread_id() const
  { return sc_thread_t::thread_id(); }
#endif

  /**
   * Convenient stdout print function using python-like formatting.
   *
   * Print to stdout
   * Print using fmt libraries python-like formatting syntax.
   */

  /**
   * Convenient debug function using python-like formatting.
   *
   * Checks if sim debug is enabled.
   * Print using fmt libraries python-like formatting syntax.
   */
  template<typename Format, typename... Args>
  void print_debug(Format&& format, Args&& ... args)
  {
    if ( ! debug )
      return;

    out_debug.print(std::forward<Format>(format), std::forward<Args>(args)... );
  }

  /**
   * Convenient log function using python-like formatting.
   *
   * Checks if sim logging is enabled.
   * Print using fmt libraries python-like formatting syntax.
   */
  template<typename Format, typename... Args>
  void print_log(Format&& format, Args&& ... args)
  {
    if ( ! log )
      return;

    out_log.print(std::forward<Format>(format), std::forward<Args>(args)... );
  }

private:
  void do_pause();
  void print_spell_query();
  void enable_debug_seed();
  void disable_debug_seed();
  bool requires_cleanup() const;
};

// Module ===================================================================

struct module_t
{
  player_e type;

  module_t( player_e t ) :
    type( t ) {}

  virtual ~module_t() {}
  virtual player_t* create_player( sim_t* sim, const std::string& name, race_e r = RACE_NONE ) const = 0;
  virtual bool valid() const = 0;
  virtual void init( player_t* ) const = 0;
  virtual void static_init() const { }
  virtual void register_hotfixes() const { }
  virtual void combat_begin( sim_t* ) const = 0;
  virtual void combat_end( sim_t* ) const = 0;

  static const module_t* death_knight();
  static const module_t* demon_hunter();
  static const module_t* druid();
  static const module_t* hunter();
  static const module_t* mage();
  static const module_t* monk();
  static const module_t* paladin();
  static const module_t* priest();
  static const module_t* rogue();
  static const module_t* shaman();
  static const module_t* warlock();
  static const module_t* warrior();
  static const module_t* enemy();
  static const module_t* tmi_enemy();
  static const module_t* tank_dummy_enemy();
  static const module_t* heal_enemy();

  static const module_t* get( player_e t )
  {
    switch ( t )
    {
      case DEATH_KNIGHT: return death_knight();
      case DEMON_HUNTER: return demon_hunter();
      case DRUID:        return druid();
      case HUNTER:       return hunter();
      case MAGE:         return mage();
      case MONK:         return monk();
      case PALADIN:      return paladin();
      case PRIEST:       return priest();
      case ROGUE:        return rogue();
      case SHAMAN:       return shaman();
      case WARLOCK:      return warlock();
      case WARRIOR:      return warrior();
      case ENEMY:        return enemy();
      case TMI_BOSS:     return tmi_enemy();
      case TANK_DUMMY:   return tank_dummy_enemy();
      default: break;
    }
    return nullptr;
  }
  static const module_t* get( const std::string& n )
  {
    return get( util::parse_player_type( n ) );
  }
  static void init()
  {
    for ( player_e i = PLAYER_NONE; i < PLAYER_MAX; i++ )
    {
      const module_t* m = get( i );
      if ( m )
      {
        m -> static_init();
        m -> register_hotfixes();
      }
    }
  }
};

// Scaling ==================================================================

struct scaling_t
{
  mutex_t mutex;
  sim_t* sim;
  sim_t* baseline_sim;
  sim_t* ref_sim;
  sim_t* delta_sim;
  sim_t* ref_sim2;
  sim_t* delta_sim2;
  stat_e scale_stat;
  double scale_value;
  double scale_delta_multiplier;
  int    calculate_scale_factors;
  int    center_scale_delta;
  int    positive_scale_delta;
  int    scale_lag;
  double scale_factor_noise;
  int    normalize_scale_factors;
  int    debug_scale_factors;
  std::string scale_only_str;
  stat_e current_scaling_stat;
  int num_scaling_stats, remaining_scaling_stats;
  std::string scale_over;
  scale_metric_e scaling_metric;
  std::string scale_over_player;

  // Gear delta for determining scale factors
  gear_stats_t stats;

  scaling_t( sim_t* s );

  void init_deltas();
  void analyze();
  void analyze_stats();
  void analyze_ability_stats( stat_e, double, player_t*, player_t*, player_t* );
  void analyze_lag();
  void normalize();
  double progress( std::string& phase, std::string* detailed = nullptr );
  void create_options();
  bool has_scale_factors();
};

// Plot =====================================================================

struct plot_t
{
public:
  sim_t* sim;
  std::string dps_plot_stat_str;
  double dps_plot_step;
  int    dps_plot_points;
  int    dps_plot_iterations;
  double dps_plot_target_error;
  int    dps_plot_debug;
  stat_e current_plot_stat;
  int    num_plot_stats, remaining_plot_stats, remaining_plot_points;
  bool   dps_plot_positive, dps_plot_negative;

  plot_t( sim_t* s );
  void analyze();
  double progress( std::string& phase, std::string* detailed = nullptr );
private:
  void analyze_stats();
  void write_output_file();
  void create_options();
};

// Reforge Plot =============================================================

struct reforge_plot_t
{
  sim_t* sim;
  sim_t* current_reforge_sim;
  std::string reforge_plot_stat_str;
  std::vector<stat_e> reforge_plot_stat_indices;
  int    reforge_plot_step;
  int    reforge_plot_amount;
  int    reforge_plot_iterations;
  double reforge_plot_target_error;
  int    reforge_plot_debug;
  int    current_stat_combo;
  int    num_stat_combos;

  reforge_plot_t( sim_t* s );

  void generate_stat_mods( std::vector<std::vector<int> > &stat_mods,
                           const std::vector<stat_e> &stat_indices,
                           int cur_mod_stat,
                           std::vector<int> cur_stat_mods );
  void analyze();
  void analyze_stats();
  double progress( std::string& phase, std::string* detailed = nullptr );
private:
  void write_output_file();
  void create_options();
};

struct plot_data_t
{
  double plot_step;
  double value;
  double error;
};

// Event ====================================================================
//
// core_event_t is designed to be a very simple light-weight event transporter and
// as such there are rules of use that must be honored:
//
// (1) The pure virtual execute() method MUST be implemented in the sub-class
// (2) There is 1 * sizeof( event_t ) space available to extend the sub-class
// (3) sim_t is responsible for deleting the memory associated with allocated events

struct event_t : private noncopyable
{
  sim_t& _sim;
  event_t*    next;
  timespan_t  time;
  timespan_t  reschedule_time;
  unsigned    id;
  bool        canceled;
  bool        recycled;
  bool scheduled;
#ifdef ACTOR_EVENT_BOOKKEEPING
  actor_t*    actor;
#endif
  event_t( sim_t& s, actor_t* a = nullptr );
  event_t( actor_t& p );

  // If possible, use one of the two following constructors, which directly
  // schedule the created event
  event_t( sim_t& s, timespan_t delta_time ) : event_t( s )
  {
    schedule( delta_time );
  }
  event_t( actor_t& a, timespan_t delta_time ) : event_t( a )
  {
    schedule( delta_time );
  }

  timespan_t occurs() const  { return ( reschedule_time != timespan_t::zero() ) ? reschedule_time : time; }
  timespan_t remains() const { return occurs() - _sim.event_mgr.current_time; }

  void schedule( timespan_t delta_time );

  void reschedule( timespan_t new_time );
  sim_t& sim()
  { return _sim; }
  const sim_t& sim() const
  { return _sim; }
  rng::rng_t& rng() { return sim().rng(); }
  rng::rng_t& rng() const { return sim().rng(); }

  virtual void execute() = 0; // MUST BE IMPLEMENTED IN SUB-CLASS!
  virtual const char* name() const
  { return "core_event_t"; }

  virtual ~event_t() {}

  template<class T>
  static void cancel( T& e )
  {
    event_t* ref = static_cast<event_t*>(e);
    event_t::cancel( ref );
    e = nullptr;
  }
  static void cancel( event_t*& e );

protected:
  template <typename Event, typename... Args>
  friend Event* make_event( sim_t& sim, Args&&... args );
  /// Placement-new operator for creating events. Do not use in user-code.
  static void* operator new( std::size_t size, sim_t& sim )
  {
    return sim.event_mgr.allocate_event( size );
  }
  static void  operator delete( void*, sim_t& ) { }
  static void  operator delete( void* ) { }
  static void* operator new( std::size_t ) = delete;
};

/**
 * @brief Creates a event
 *
 * This function should be used as the one and only way to create new events. It
 * is used to hide internal placement-new mechanism for efficient event
 * allocation, and also makes sure that any event created is properly added to
 * the event manager (scheduled).
 */
template <typename Event, typename... Args>
inline Event* make_event( sim_t& sim, Args&&... args )
{
  static_assert( std::is_base_of<event_t, Event>::value,
                 "Event must be derived from event_t" );
  auto r = new ( sim ) Event( args... );
  assert( r -> id != 0 && "Event not added to event manager!" );
  return r;
}

template <typename T>
inline event_t* make_event( sim_t& s, const timespan_t& t, const T& f )
{
  class fn_event_t : public event_t
  {
    T fn;

    public:
      fn_event_t( sim_t& s, const timespan_t& t, const T& f ) :
        event_t( s, t ), fn( f )
      { }

      const char* name() const override
      { return "function_event"; }

      void execute() override
      { fn(); }
  };

  return make_event<fn_event_t>( s, s, t, f );
}

template <typename T>
inline event_t* make_repeating_event( sim_t& s, const timespan_t& t, const T& fn, int n = -1 )
{
  class fn_event_repeating_t : public event_t
  {
    T fn;
    timespan_t time;
    int n;

    public:
      fn_event_repeating_t( sim_t& s, const timespan_t& t, const T& f, int n ) :
        event_t( s, t ), fn( f ), time( t ), n( n )
      { }

      const char* name() const override
      { return "repeating_function_event"; }

      void execute() override
      {
        fn();
        if ( n == -1 || --n > 0 )
        {
          make_event<fn_event_repeating_t>( sim(), sim(), time, fn, n );
        }
      }
  };

  return make_event<fn_event_repeating_t>( s, s, t, fn, n );
}

template <typename T>
inline event_t* make_event( sim_t* s, const timespan_t& t, const T& f )
{ return make_event( *s, t, f ); }

template <typename T>
inline event_t* make_event( sim_t* s, const T& f )
{ return make_event( *s, timespan_t::zero(), f ); }

template <typename T>
inline event_t* make_event( sim_t& s, const T& f )
{ return make_event( s, timespan_t::zero(), f ); }

template <typename T>
inline event_t* make_repeating_event( sim_t* s, const timespan_t& t, const T& f, int n = -1 )
{ return make_repeating_event( *s, t, f, n ); }

// Gear Rating Conversions ==================================================

inline cache_e cache_from_rating( rating_e r )
{
  switch ( r )
  {
    case RATING_SPELL_HASTE: return CACHE_SPELL_HASTE;
    case RATING_SPELL_HIT: return CACHE_SPELL_HIT;
    case RATING_SPELL_CRIT: return CACHE_SPELL_CRIT_CHANCE;
    case RATING_MELEE_HASTE: return CACHE_ATTACK_HASTE;
    case RATING_MELEE_HIT: return CACHE_ATTACK_HIT;
    case RATING_MELEE_CRIT: return CACHE_ATTACK_CRIT_CHANCE;
    case RATING_RANGED_HASTE: return CACHE_ATTACK_HASTE;
    case RATING_RANGED_HIT: return CACHE_ATTACK_HIT;
    case RATING_RANGED_CRIT: return CACHE_ATTACK_CRIT_CHANCE;
    case RATING_EXPERTISE: return CACHE_EXP;
    case RATING_DODGE: return CACHE_DODGE;
    case RATING_PARRY: return CACHE_PARRY;
    case RATING_BLOCK: return CACHE_BLOCK;
    case RATING_MASTERY: return CACHE_MASTERY;
    case RATING_PVP_POWER: return CACHE_NONE;
    case RATING_PVP_RESILIENCE: return CACHE_NONE;
    case RATING_DAMAGE_VERSATILITY: return CACHE_DAMAGE_VERSATILITY;
    case RATING_HEAL_VERSATILITY: return CACHE_HEAL_VERSATILITY;
    case RATING_MITIGATION_VERSATILITY: return CACHE_MITIGATION_VERSATILITY;
    case RATING_LEECH: return CACHE_LEECH;
    case RATING_SPEED: return CACHE_RUN_SPEED;
    case RATING_AVOIDANCE: return CACHE_AVOIDANCE;
    default: break;
  }
  assert( false ); return CACHE_NONE;
}

struct rating_t
{
  double  spell_haste,  spell_hit,  spell_crit;
  double attack_haste, attack_hit, attack_crit;
  double ranged_haste, ranged_hit, ranged_crit;
  double expertise;
  double dodge, parry, block;
  double mastery;
  double pvp_resilience, pvp_power;
  double damage_versatility, heal_versatility, mitigation_versatility;
  double leech, speed, avoidance;

  double& get( rating_e r )
  {
    switch ( r )
    {
      case RATING_SPELL_HASTE: return spell_haste;
      case RATING_SPELL_HIT: return spell_hit;
      case RATING_SPELL_CRIT: return spell_crit;
      case RATING_MELEE_HASTE: return attack_haste;
      case RATING_MELEE_HIT: return attack_hit;
      case RATING_MELEE_CRIT: return attack_crit;
      case RATING_RANGED_HASTE: return ranged_haste;
      case RATING_RANGED_HIT: return ranged_hit;
      case RATING_RANGED_CRIT: return ranged_crit;
      case RATING_EXPERTISE: return expertise;
      case RATING_DODGE: return dodge;
      case RATING_PARRY: return parry;
      case RATING_BLOCK: return block;
      case RATING_MASTERY: return mastery;
      case RATING_PVP_POWER: return pvp_power;
      case RATING_PVP_RESILIENCE: return pvp_resilience;
      case RATING_DAMAGE_VERSATILITY: return damage_versatility;
      case RATING_HEAL_VERSATILITY: return heal_versatility;
      case RATING_MITIGATION_VERSATILITY: return mitigation_versatility;
      case RATING_LEECH: return leech;
      case RATING_SPEED: return speed;
      case RATING_AVOIDANCE: return avoidance;
      default: break;
    }
    assert( false ); return mastery;
  }

  rating_t()
  {
    // Initialize all ratings to a very high number
    double max = +1.0E+50;
    for ( rating_e i = static_cast<rating_e>( 0 ); i < RATING_MAX; ++i )
    {
      get( i ) = max;
    }
  }

  void init( dbc_t& dbc, int level )
  {
    // Read ratings from DBC
    for ( rating_e i = static_cast<rating_e>( 0 ); i < RATING_MAX; ++i )
    {
      get( i ) = dbc.combat_rating( i,  level );
      if ( i == RATING_MASTERY )
        get( i ) /= 100.0;
    }
  }

  std::string to_string()
  {
    std::ostringstream s;
    for ( rating_e i = static_cast<rating_e>( 0 ); i < RATING_MAX; ++i )
    {
      if ( i > 0 ) s << " ";
      s << util::cache_type_string( cache_from_rating( i ) ) << "=" << get( i ); // hacky
    }
    return s.str();
  }
};

// Weapon ===================================================================

struct weapon_t
{
  weapon_e type;
  school_e school;
  double damage, dps;
  double min_dmg, max_dmg;
  timespan_t swing_time;
  slot_e slot;
  int    buff_type;
  double buff_value;
  double bonus_dmg;

  weapon_t( weapon_e t    = WEAPON_NONE,
            double d      = 0.0,
            timespan_t st = timespan_t::from_seconds( 2.0 ),
            school_e s    = SCHOOL_PHYSICAL ) :
    type( t ),
    school( s ),
    damage( d ),
    dps( d / st.total_seconds() ),
    min_dmg( d ),
    max_dmg( d ),
    swing_time( st ),
    slot( SLOT_INVALID ),
    buff_type( 0 ),
    buff_value( 0.0 ),
    bonus_dmg( 0.0 )
  {}

  weapon_e group() const
  {
    if ( type <= WEAPON_SMALL )
      return WEAPON_SMALL;

    if ( type <= WEAPON_1H )
      return WEAPON_1H;

    if ( type <= WEAPON_2H )
      return WEAPON_2H;

    if ( type <= WEAPON_RANGED )
      return WEAPON_RANGED;

    return WEAPON_NONE;
  }

  /**
   * Normalized weapon speed for weapon damage calculations
   *
   * * WEAPON_SMALL: 1.7s
   * * WEAPON_1H: 2.4s
   * * WEAPON_2h: 3.3s
   * * WEAPON_RANGED: 2.8s
   */
  timespan_t get_normalized_speed()
  {
    weapon_e g = group();

    if ( g == WEAPON_SMALL  ) return timespan_t::from_seconds( 1.7 );
    if ( g == WEAPON_1H     ) return timespan_t::from_seconds( 2.4 );
    if ( g == WEAPON_2H     ) return timespan_t::from_seconds( 3.3 );
    if ( g == WEAPON_RANGED ) return timespan_t::from_seconds( 2.8 );

    assert( false );
    return timespan_t::zero();
  }

  double proc_chance_on_swing( double PPM, timespan_t adjusted_swing_time = timespan_t::zero() )
  {
    if ( adjusted_swing_time == timespan_t::zero() ) adjusted_swing_time = swing_time;

    timespan_t time_to_proc = timespan_t::from_seconds( 60.0 ) / PPM;
    double proc_chance = adjusted_swing_time / time_to_proc;

    return proc_chance;
  }
};

// Special Effect ===========================================================

// A special effect callback that will be scoped by the pure virtual valid() method of the class.
// If the implemented valid() returns false, the special effect initializer callbacks will not be
// invoked.
//
// Scoped callbacks are prioritized statically.
//
// Special effects will be selected and initialize automatically for the actor from the highest
// priority class.  Currently PRIORITY_CLASS is used by class-scoped special effects callback
// objects that use the class scope (i.e., player_e) validator, and PRIORITY_SPEC is used by
// specialization-scoped special effect callback objects. The separation of the two allows, for
// example, the creation of a class-scoped special effect, and then an additional
// specialization-scoped special effect for a single specialization for the class.
//
// The PRIORITY_DEFAULT value is used for old-style callbacks (i.e., static functions or
// string-based initialization defined in sc_unique_gear.cpp).
struct scoped_callback_t
{
  enum priority
  {
    PRIORITY_DEFAULT = 0U, // Old-style callbacks, and any kind of "last resort" special effect
    PRIORITY_CLASS,        // Class-filtered callbacks
    PRIORITY_SPEC          // Specialization-filtered callbacks
  };

  priority priority;

  scoped_callback_t() : priority( PRIORITY_DEFAULT )
  { }

  scoped_callback_t( enum priority p ) : priority( p )
  { }

  virtual ~scoped_callback_t()
  { }

  // Validate special effect against conditions. Return value of false indicates that the
  // initializer should not be invoked.
  virtual bool valid( const special_effect_t& ) const = 0;

  // Initialize the special effect.
  virtual void initialize( special_effect_t& ) = 0;
};

struct special_effect_t
{
  const item_t* item;
  player_t* player;

  special_effect_e type;
  special_effect_source_e source;
  std::string name_str, trigger_str, encoding_str;
  unsigned proc_flags_; /* Proc-by */
  unsigned proc_flags2_; /* Proc-on (hit/damage/...) */
  stat_e stat;
  school_e school;
  int max_stacks;
  double stat_amount, discharge_amount, discharge_scaling;
  double proc_chance_;
  double ppm_;
  unsigned rppm_scale_;
  double rppm_modifier_;
  int rppm_blp_;
  timespan_t duration_, cooldown_, tick;
  bool cost_reduction;
  int refresh;
  bool chance_to_discharge;
  unsigned int override_result_es_mask;
  unsigned result_es_mask;
  bool reverse;
  int reverse_stack_reduction;
  int aoe;
  bool proc_delay;
  bool unique;
  bool weapon_proc;
  unsigned spell_id, trigger_spell_id;
  action_t* execute_action; // Allows custom action to be executed on use
  buff_t* custom_buff; // Allows custom action

  bool action_disabled, buff_disabled;

  // Old-style function-based custom second phase initializer (callback)
  std::function<void(special_effect_t&)> custom_init;
  // New-style object-based custom second phase initializer
  std::vector<scoped_callback_t*> custom_init_object;
  // Activation callback, if set, called when an actor becomes active
  std::function<void(void)> activation_cb;
  // Link to an SpellItemEnchantment entry, set for various "enchant"-based special effects
  // (Enchants, Gems, Addons)
  const item_enchantment_data_t* enchant_data;

  special_effect_t( player_t* p ) :
    item( nullptr ), player( p ),
    name_str()
  { reset(); }

  special_effect_t( const item_t* item );

  // Uses a custom initialization callback or object
  bool is_custom() const
  { return custom_init || custom_init_object.size() > 0; }

  // Forcefully disable creation of an (autodetected) buff or action. This is necessary in scenarios
  // where the autodetection decides to create an invalid action or buff due to the spell data.
  void disable_action()
  { action_disabled = true; }
  void disable_buff()
  { buff_disabled = true; }

  void reset();
  std::string to_string() const;
  bool active() { return stat != STAT_NONE || school != SCHOOL_NONE || execute_action; }

  const spell_data_t* driver() const;
  const spell_data_t* trigger() const;
  std::string name() const;
  std::string cooldown_name() const;

  // On-use item cooldown handling accessories
  int cooldown_group() const;
  std::string cooldown_group_name() const;
  timespan_t cooldown_group_duration() const;

  // Buff related functionality
  buff_t* create_buff() const;
  special_effect_buff_e buff_type() const;
  int max_stack() const;

  bool is_stat_buff() const;
  stat_e stat_type() const;
  stat_buff_t* initialize_stat_buff() const;

  bool is_absorb_buff() const;
  absorb_buff_t* initialize_absorb_buff() const;

  // Action related functionality
  action_t* create_action() const;
  special_effect_action_e action_type() const;
  bool is_offensive_spell_action() const;
  spell_t* initialize_offensive_spell_action() const;
  bool is_heal_action() const;
  heal_t* initialize_heal_action() const;
  bool is_attack_action() const;
  attack_t* initialize_attack_action() const;
  bool is_resource_action() const;
  spell_t* initialize_resource_action() const;

  /* Accessors for driver specific features of the proc; some are also used for on-use effects */
  unsigned proc_flags() const;
  unsigned proc_flags2() const;
  double ppm() const;
  double rppm() const;
  unsigned rppm_scale() const;
  double rppm_modifier() const;
  double proc_chance() const;
  timespan_t cooldown() const;

  /* Accessors for buff specific features of the proc. */
  timespan_t duration() const;
  timespan_t tick_time() const;
};

inline std::ostream& operator<<(std::ostream &os, const special_effect_t& x);
// Item =====================================================================

struct stat_pair_t
{
  stat_e stat;
  int value;

  stat_pair_t() : stat( STAT_NONE ), value( 0 )
  { }

  stat_pair_t( stat_e s, int v ) : stat( s ), value( v )
  { }
};

struct item_t
{
  sim_t* sim;
  player_t* player;
  slot_e slot, parent_slot;
  bool unique, unique_addon, is_ptr;

  // Structure contains the "parsed form" of this specific item, be the data
  // from user options, or a data source such as the Blizzard API, or Wowhead
  struct parsed_input_t
  {
    int                                              item_level;
    unsigned                                         enchant_id;
    unsigned                                         addon_id;
    int                                              armor;
    unsigned                                         azerite_level;
    std::array<int, MAX_ITEM_STAT>                   stat_val;
    std::array<int, MAX_GEM_SLOTS>                   gem_id;
    std::array<std::vector<unsigned>, MAX_GEM_SLOTS> gem_bonus_id;
    std::array<unsigned, MAX_GEM_SLOTS>              gem_ilevel;
    std::array<int, MAX_GEM_SLOTS>                   gem_actual_ilevel;
    std::array<int, MAX_GEM_SLOTS>                   gem_color;
    std::vector<int>                                 bonus_id;
    std::vector<stat_pair_t>                         gem_stats, meta_gem_stats, socket_bonus_stats;
    std::string                                      encoded_enchant;
    std::vector<stat_pair_t>                         enchant_stats;
    std::string                                      encoded_addon;
    std::vector<stat_pair_t>                         addon_stats;
    item_data_t                                      data;
    auto_dispose< std::vector<special_effect_t*> >   special_effects;
    std::vector<std::string>                         source_list;
    timespan_t                                       initial_cd;
    unsigned                                         drop_level;
    std::vector<unsigned>                            azerite_ids;

    parsed_input_t() :
      item_level( 0 ), enchant_id( 0 ), addon_id( 0 ),
      armor( 0 ), azerite_level( 0 ), data(), initial_cd( timespan_t::zero() ), drop_level( 0 )
    {
      range::fill( data.stat_type_e, -1 );
      range::fill( data.stat_alloc, 0 );
      range::fill( stat_val, 0 );
      range::fill( gem_id, 0 );
      range::fill( bonus_id, 0 );
      range::fill( gem_color, SOCKET_COLOR_NONE );
      range::fill( gem_ilevel, 0 );
      range::fill( gem_actual_ilevel, 0 );
    }
  } parsed;

  std::shared_ptr<xml_node_t> xml;

  std::string name_str;
  std::string icon_str;

  std::string source_str;
  std::string options_str;

  // Option Data
  std::string option_name_str;
  std::string option_stats_str;
  std::string option_gems_str;
  std::string option_enchant_str;
  std::string option_addon_str;
  std::string option_equip_str;
  std::string option_use_str;
  std::string option_weapon_str;
  std::string option_lfr_str;
  std::string option_warforged_str;
  std::string option_heroic_str;
  std::string option_mythic_str;
  std::string option_armor_type_str;
  std::string option_ilevel_str;
  std::string option_quality_str;
  std::string option_data_source_str;
  std::string option_enchant_id_str;
  std::string option_addon_id_str;
  std::string option_gem_id_str;
  std::string option_gem_bonus_id_str;
  std::string option_gem_ilevel_str;
  std::string option_bonus_id_str;
  std::string option_initial_cd_str;
  std::string option_drop_level_str;
  std::string option_azerite_powers_str;
  std::string option_azerite_level_str;
  double option_initial_cd;

  // Extracted data
  gear_stats_t base_stats, stats;

  item_t() : sim( nullptr ), player( nullptr ), slot( SLOT_INVALID ), parent_slot( SLOT_INVALID ),
    unique( false ), unique_addon( false ), is_ptr( false ),
    parsed(), xml(), option_initial_cd(0)
  { }

  item_t( player_t*, const std::string& options_str );

  bool active() const;
  const char* name() const;
  std::string full_name() const;
  const char* slot_name() const;
  weapon_t* weapon() const;
  void init();
  void parse_options();
  bool initialize_data(); // Initializes item data from a data source
  inventory_type inv_type() const;

  bool is_matching_type() const;
  bool is_valid_type() const;
  bool socket_color_match() const;

  unsigned item_level() const;
  unsigned base_item_level() const;
  stat_e stat( size_t idx ) const;
  int stat_value( size_t idx ) const;
  gear_stats_t total_stats() const;
  bool has_item_stat( stat_e stat ) const;

  std::string encoded_item() const;
  std::string encoded_comment();

  std::string encoded_stats() const;
  std::string encoded_weapon() const;
  std::string encoded_gems() const;
  std::string encoded_enchant() const;
  std::string encoded_addon() const;

  void decode_stats();
  void decode_gems();
  void decode_enchant();
  void decode_addon();
  void decode_weapon();
  void decode_warforged();
  void decode_lfr();
  void decode_heroic();
  void decode_mythic();
  void decode_armor_type();
  void decode_ilevel();
  void decode_quality();
  void decode_data_source();
  void decode_equip_effect();
  void decode_use_effect();


  bool verify_slot();

  void init_special_effects();

  static bool download_item( item_t& );

  static std::vector<stat_pair_t> str_to_stat_pair( const std::string& stat_str );
  static std::string stat_pairs_to_str( const std::vector<stat_pair_t>& stat_pairs );

  std::string to_string() const;
  std::string item_stats_str() const;
  std::string weapon_stats_str() const;
  std::string gem_stats_str() const;
  std::string socket_bonus_stats_str() const;
  std::string enchant_stats_str() const;
  bool has_stats() const;
  bool has_special_effect( special_effect_source_e source = SPECIAL_EFFECT_SOURCE_NONE, special_effect_e type = SPECIAL_EFFECT_NONE ) const;
  bool has_use_special_effect() const
  { return has_special_effect( SPECIAL_EFFECT_SOURCE_NONE, SPECIAL_EFFECT_USE ); }
  bool has_scaling_stat_bonus_id() const;

  const special_effect_t* special_effect( special_effect_source_e source = SPECIAL_EFFECT_SOURCE_NONE, special_effect_e type = SPECIAL_EFFECT_NONE ) const;
};
std::ostream& operator<<(std::ostream&, const item_t&);


// Benefit ==================================================================

struct benefit_t : private noncopyable
{
private:
  int up, down;
public:
  simple_sample_data_t ratio;
  const std::string name_str;

  explicit benefit_t( const std::string& n ) :
    up( 0 ), down( 0 ),
    ratio(), name_str( n ) {}

  void update( bool is_up )
  { if ( is_up ) up++; else down++; }
  void datacollection_begin()
  { up = down = 0; }
  void datacollection_end()
  { ratio.add( up != 0 ? 100.0 * up / ( down + up ) : 0.0 ); }
  void merge( const benefit_t& other )
  { ratio.merge( other.ratio ); }

  const char* name() const
  { return name_str.c_str(); }
};

// Proc =====================================================================

struct proc_t : private noncopyable
{
private:
  sim_t& sim;
  size_t iteration_count; // track number of procs during the current iteration
  timespan_t last_proc; // track time of the last proc
public:
  const std::string name_str;
  simple_sample_data_t interval_sum;
  simple_sample_data_t count;

  proc_t( sim_t& s, const std::string& n ) :
    sim( s ),
    iteration_count(),
    last_proc( timespan_t::min() ),
    name_str( n ),
    interval_sum(),
    count()
  {}

  void occur()
  {
    iteration_count++;
    if ( last_proc >= timespan_t::zero() && last_proc < sim.current_time() )
    {
      interval_sum.add( ( sim.current_time() - last_proc ).total_seconds() );
      reset();
    }
    if ( sim.debug )
      sim.out_debug.printf( "[PROC] %s: iteration_count=%u count.sum=%u last_proc=%f",
                  name(),
                  static_cast<unsigned>( iteration_count ),
                  static_cast<unsigned>( count.sum() ),
                  last_proc.total_seconds() );

    last_proc = sim.current_time();
  }

  void reset()
  { last_proc = timespan_t::min(); }

  void merge( const proc_t& other )
  {
    count.merge( other.count );
    interval_sum.merge( other.interval_sum );
  }

  void datacollection_begin()
  { iteration_count = 0; }
  void datacollection_end()
  { count.add( static_cast<double>( iteration_count ) ); }

  const char* name() const
  { return name_str.c_str(); }
};

// Set Bonus ================================================================

struct set_bonus_t
{
  // Some magic constants
  static const unsigned N_BONUSES = 8;       // Number of set bonuses in tier gear

  struct set_bonus_data_t
  {
    const spell_data_t* spell;
    const item_set_bonus_t* bonus;
    int overridden;
    bool enabled;

    set_bonus_data_t() :
      spell( spell_data_t::not_found() ), bonus( nullptr ), overridden( -1 ), enabled( false )
    { }
  };

  // Data structure definitions
  typedef std::vector<set_bonus_data_t> bonus_t;
  typedef std::vector<bonus_t> bonus_type_t;
  typedef std::vector<bonus_type_t> set_bonus_type_t;

  typedef std::vector<unsigned> bonus_count_t;
  typedef std::vector<bonus_count_t> set_bonus_count_t;

  player_t* actor;

  // Set bonus data structure
  set_bonus_type_t set_bonus_spec_data;
  // Set item counts
  set_bonus_count_t set_bonus_spec_count;

  set_bonus_t( player_t* p );

  // Collect item information about set bonuses, fully DBC driven
  void initialize_items();

  // Initialize set bonuses in earnest
  void initialize();

  expr_t* create_expression( const player_t*, const std::string& type );

  std::vector<const item_set_bonus_t*> enabled_set_bonus_data() const;

  // Fast accessor to a set bonus spell, returns the spell, or spell_data_t::not_found()
  const spell_data_t* set( specialization_e spec, set_bonus_type_e set_bonus, set_bonus_e bonus ) const
  {
    if ( specdata::spec_idx( spec ) < 0 )
    {
      return spell_data_t::nil();
    }
#ifndef NDEBUG
    assert(set_bonus_spec_data.size() > (unsigned)set_bonus );
    assert(set_bonus_spec_data[ set_bonus ].size() > (unsigned)specdata::spec_idx( spec ) );
    assert(set_bonus_spec_data[ set_bonus ][ specdata::spec_idx( spec ) ].size() > (unsigned)bonus );
#endif
    return set_bonus_spec_data[ set_bonus ][ specdata::spec_idx( spec ) ][ bonus ].spell;
  }

  // Fast accessor for checking whether a set bonus is enabled
  bool has_set_bonus( specialization_e spec, set_bonus_type_e set_bonus, set_bonus_e bonus ) const
  {
    if ( specdata::spec_idx( spec ) < 0 )
    {
      return false;
    }

    return set_bonus_spec_data[ set_bonus ][ specdata::spec_idx( spec ) ][ bonus ].enabled;
  }

  bool parse_set_bonus_option( const std::string& opt_str, set_bonus_type_e& set_bonus, set_bonus_e& bonus );
  std::string to_string() const;
  std::string to_profile_string( const std::string& = "\n" ) const;
  std::string generate_set_bonus_options() const;
};

std::ostream& operator<<(std::ostream&, const set_bonus_t&);

// "Real" 'Procs per Minute' helper class =====================================

struct real_ppm_t
{
  enum blp : int
  {
    BLP_DISABLED = 0,
    BLP_ENABLED
  };

private:
  player_t*    player;
  std::string  name_str;
  double       freq;
  double       modifier;
  double       rppm;
  timespan_t   last_trigger_attempt;
  timespan_t   last_successful_trigger;
  unsigned     scales_with;
  blp          blp_state;

  real_ppm_t() : player( nullptr ), freq( 0 ), modifier( 0 ), rppm( 0 ), scales_with( 0 ),
    blp_state( BLP_ENABLED )
  { }

  static double max_interval() { return 10.0; }
  static double max_bad_luck_prot() { return 1000.0; }
public:
  static double proc_chance( player_t*         player,
                             double            PPM,
                             const timespan_t& last_trigger,
                             const timespan_t& last_successful_proc,
                             unsigned          scales_with,
                             blp               blp_state );

  real_ppm_t( const std::string& name, player_t* p, double frequency = 0, double mod = 1.0, unsigned s = RPPM_NONE, blp b = BLP_ENABLED ) :
    player( p ),
    name_str( name ),
    freq( frequency ),
    modifier( mod ),
    rppm( freq * mod ),
    last_trigger_attempt( timespan_t::zero() ),
    last_successful_trigger( timespan_t::zero() ),
    scales_with( s ),
    blp_state( b )
  { }

  real_ppm_t( const std::string& name, player_t* p, const spell_data_t* data = spell_data_t::nil(), const item_t* item = nullptr );

  void set_scaling( unsigned s )
  { scales_with = s; }

  void set_modifier( double mod )
  { modifier = mod; rppm = freq * modifier; }

  const std::string& name() const
  { return name_str; }

  void set_frequency( double frequency )
  { freq = frequency; rppm = freq * modifier; }

  void set_blp_state( blp state )
  { blp_state = state; }

  unsigned get_scaling() const
  {
    return scales_with;
  }

  double get_frequency() const
  { return freq; }

  double get_modifier() const
  { return modifier; }

  double get_rppm() const
  { return rppm; }

  blp get_blp_state() const
  { return blp_state; }

  void set_last_trigger_attempt( const timespan_t& ts )
  { last_trigger_attempt = ts; }

  void set_last_trigger_success( const timespan_t& ts )
  { last_successful_trigger = ts; }

  void reset()
  {
    last_trigger_attempt = timespan_t::zero();
    last_successful_trigger = timespan_t::zero();
  }

  bool trigger();
};

// "Deck of Cards" randomizer helper class ====================================
// Described at https://www.reddit.com/r/wow/comments/6j2wwk/wow_class_design_ama_june_2017/djb8z68/

struct shuffled_rng_t
{
private:
  player_t*    player;
  std::string  name_str;
  int          success_entries;
  int          total_entries;
  int          success_entries_remaining;
  int          total_entries_remaining;

  shuffled_rng_t() : player(nullptr), success_entries(0), total_entries(0), success_entries_remaining(0), total_entries_remaining(0)
  { }

public:

  shuffled_rng_t(const std::string& name, player_t* p, int success_entries = 0, int total_entries = 0) :
    player(p),
    name_str(name),
    success_entries(success_entries),
    total_entries(total_entries),
    success_entries_remaining(success_entries),
    total_entries_remaining(total_entries)
  { }

  const std::string& name() const
  {
    return name_str;
  }

  int get_success_entries() const
  {
    return success_entries;
  }

  int get_success_entries_remaining() const
  {
    return success_entries_remaining;
  }

  int get_total_entries() const
  {
    return total_entries;
  }

  int get_total_entries_remaining() const
  {
    return total_entries_remaining;
  }

  double get_remaining_success_chance() const
  {
    return (double)success_entries_remaining / (double)total_entries_remaining;
  }

  void reset()
  {
    success_entries_remaining = success_entries;
    total_entries_remaining = total_entries;
  }

  bool trigger();
};

// Cooldown =================================================================

struct cooldown_t
{
  sim_t& sim;
  player_t* player;
  std::string name_str;
  timespan_t duration;
  timespan_t ready;
  timespan_t reset_react;
  int charges;
  event_t* recharge_event;
  event_t* ready_trigger_event;
  timespan_t last_start, last_charged;
  bool hasted; // Hasted cooldowns will reschedule based on haste state changing (through buffs). TODO: Separate hastes?
  action_t* action; // Dynamic cooldowns will need to know what action triggered the cd

  // Associated execution types amongst all the actions shared by this cooldown. Bitmasks based on
  // the execute_type enum class
  unsigned execute_types_mask;

  // State of the current cooldown progression. Only updated for ongoing cooldowns.
  int current_charge;
  double recharge_multiplier;
  timespan_t base_duration;

  cooldown_t( const std::string& name, player_t& );
  cooldown_t( const std::string& name, sim_t& );

  // Adjust the CD. If "requires_reaction" is true (or not provided), then the CD change is something
  // the user would react to rather than plan ahead for.
  void adjust( timespan_t, bool requires_reaction = true );
  void adjust_recharge_multiplier(); // Reacquire cooldown recharge multiplier from the action to adjust the cooldown time
  void adjust_base_duration(); // Reacquire base cooldown duration from the action to adjust the cooldown time
  // Instalty recharge a cooldown. For multicharge cooldowns, charges_ specifies how many charges to reset.
  // If less than zero, all charges are reset.
  void reset( bool require_reaction, int charges_ = 1 );
  void start( action_t* action, timespan_t override = timespan_t::min(), timespan_t delay = timespan_t::zero() );
  void start( timespan_t override = timespan_t::min(), timespan_t delay = timespan_t::zero() );

  void reset_init();

  timespan_t remains() const
  { return std::max( timespan_t::zero(), ready - sim.current_time() ); }

  timespan_t current_charge_remains() const
  { return recharge_event ? recharge_event -> remains() : timespan_t::zero(); }

  // Return true if the cooldown is ready (has at least one charge).
  bool up() const
  { return ready <= sim.current_time(); }

  // Return true if the cooldown is not ready (has zero charges).
  bool down() const
  { return ready > sim.current_time(); }

  // Return true if the cooldown is in progress.
  bool ongoing() const
  { return down() || recharge_event; }

  // Return true if the action bound to this cooldown is ready. Cooldowns are ready either when
  // their cooldown has elapsed, or a short while before its cooldown is finished. The latter case
  // is in use when the cooldown is associated with a foreground action (i.e., a player button
  // press) to model in-game behavior of queueing soon-to-be-ready abilities. Inlined below.
  bool is_ready() const;

  // Return the queueing delay for cooldowns that are queueable
  timespan_t queue_delay() const
  { return std::max( timespan_t::zero(), ready - sim.current_time() ); }

  // Point in time when the cooldown is queueable
  timespan_t queueable() const;

  // Trigger update of specialized execute thresholds for this cooldown
  void update_ready_thresholds();

  const char* name() const
  { return name_str.c_str(); }

  expr_t* create_expression( const std::string& name_str );

  void add_execute_type( execute_type e )
  { execute_types_mask |= ( 1 << static_cast<unsigned>( e ) ); }

  static timespan_t ready_init()
  { return timespan_t::from_seconds( -60 * 60 ); }

  static timespan_t cooldown_duration( const cooldown_t* cd );

private:
  void adjust_remaining_duration( double delta ); // Modify the remaining duration of an ongoing cooldown.
};

// Player Callbacks
template <typename T_CB>
struct effect_callbacks_t
{
  sim_t* sim;
  std::vector<T_CB*> all_callbacks;
  // New proc system

  // Callbacks (procs) stored in a vector
  typedef std::vector<T_CB*> proc_list_t;
  // .. an array of callbacks, for each proc_type2 enum (procced by hit/crit, etc...)
  typedef std::array<proc_list_t, PROC2_TYPE_MAX> proc_on_array_t;
  // .. an array of procced by arrays, for each proc_type enum (procced on aoe, heal, tick, etc...)
  typedef std::array<proc_on_array_t, PROC1_TYPE_MAX> proc_array_t;

  proc_array_t procs;

  effect_callbacks_t( sim_t* sim ) : sim( sim )
  { }

  bool has_callback( const std::function<bool(const T_CB*)> cmp ) const
  { return range::find_if( all_callbacks, cmp ) != all_callbacks.end(); }

  virtual ~effect_callbacks_t()
  { range::sort( all_callbacks ); dispose( all_callbacks.begin(), range::unique( all_callbacks ) ); }

  void reset();

  void register_callback( unsigned proc_flags, unsigned proc_flags2, T_CB* cb );
private:
  void add_proc_callback( proc_types type, unsigned flags, T_CB* cb );
};

// Stat Cache

/* The Cache system increases simulation performance by moving the calculation point
 * from call-time to modification-time of a stat. Because a stat is called much more
 * often than it is changed, this reduces costly and unnecessary repetition of floating-point
 * operations.
 *
 * When a stat is accessed, its 'valid'-state is checked:
 *   If its true, the cached value is accessed.
 *   If it is false, the stat value is recalculated and written to the cache.
 *
 * To indicate when a stat gets modified, it needs to be 'invalidated'. Every time a stat
 * is invalidated, its 'valid'-state gets set to false.
 */

/* - To invalidate a stat, use player_t::invalidate_cache( cache_e )
 * - using player_t::stat_gain/loss automatically invalidates the corresponding cache
 * - Same goes for stat_buff_t, which works through player_t::stat_gain/loss
 * - Buffs with effects in a composite_ function need invalidates added to their buff_creator
 *
 * To create invalidation chains ( eg. Priest: Spirit invalidates Hit ) override the
 * virtual player_t::invalidate_cache( cache_e ) function.
 *
 * Attention: player_t::invalidate_cache( cache_e ) is recursive and may call itself again.
 */
struct player_stat_cache_t
{
  const player_t* player;
  mutable std::array<bool, CACHE_MAX> valid;
  mutable std::array < bool, SCHOOL_MAX + 1 > spell_power_valid, player_mult_valid, player_heal_mult_valid;
  // 'valid'-states
private:
  // cached values
  mutable double _strength, _agility, _stamina, _intellect, _spirit;
  mutable double _spell_power[SCHOOL_MAX + 1], _attack_power;
  mutable double _attack_expertise;
  mutable double _attack_hit, _spell_hit;
  mutable double _attack_crit_chance, _spell_crit_chance;
  mutable double _attack_haste, _spell_haste;
  mutable double _attack_speed, _spell_speed;
  mutable double _dodge, _parry, _block, _crit_block, _armor, _bonus_armor;
  mutable double _mastery, _mastery_value, _crit_avoidance, _miss;
  mutable double _player_mult[SCHOOL_MAX + 1], _player_heal_mult[SCHOOL_MAX + 1];
  mutable double _damage_versatility, _heal_versatility, _mitigation_versatility;
  mutable double _leech, _run_speed, _avoidance;
  mutable double _rppm_haste_coeff, _rppm_crit_coeff;
public:
  bool active; // runtime active-flag
  void invalidate_all();
  void invalidate( cache_e );
  double get_attribute( attribute_e ) const;
  player_stat_cache_t( const player_t* p ) : player( p ), active( false ) { invalidate_all(); }
#if defined(SC_USE_STAT_CACHE)
  // Cache stat functions
  double strength() const;
  double agility() const;
  double stamina() const;
  double intellect() const;
  double spirit() const;
  double spell_power( school_e ) const;
  double attack_power() const;
  double attack_expertise() const;
  double attack_hit() const;
  double attack_crit_chance() const;
  double attack_haste() const;
  double attack_speed() const;
  double spell_hit() const;
  double spell_crit_chance() const;
  double spell_haste() const;
  double spell_speed() const;
  double dodge() const;
  double parry() const;
  double block() const;
  double crit_block() const;
  double crit_avoidance() const;
  double miss() const;
  double armor() const;
  double mastery() const;
  double mastery_value() const;
  double bonus_armor() const;
  double player_multiplier( school_e ) const;
  double player_heal_multiplier( const action_state_t* ) const;
  double damage_versatility() const;
  double heal_versatility() const;
  double mitigation_versatility() const;
  double leech() const;
  double run_speed() const;
  double avoidance() const;
  double rppm_haste_coeff() const;
  double rppm_crit_coeff() const;
#else
  // Passthrough cache stat functions for inactive cache
  double strength() const  { return _player -> strength();  }
  double agility() const   { return _player -> agility();   }
  double stamina() const   { return _player -> stamina();   }
  double intellect() const { return _player -> intellect(); }
  double spirit() const    { return _player -> spirit();    }
  double spell_power( school_e s ) const { return _player -> composite_spell_power( s ); }
  double attack_power() const            { return _player -> composite_melee_attack_power();   }
  double attack_expertise() const { return _player -> composite_melee_expertise(); }
  double attack_hit() const       { return _player -> composite_melee_hit();       }
  double attack_crit_chance() const      { return _player -> composite_melee_crit_chance();      }
  double attack_haste() const     { return _player -> composite_melee_haste();     }
  double attack_speed() const     { return _player -> composite_melee_speed();     }
  double spell_hit() const        { return _player -> composite_spell_hit();       }
  double spell_crit_chance() const       { return _player -> composite_spell_crit_chance();      }
  double spell_haste() const      { return _player -> composite_spell_haste();     }
  double spell_speed() const      { return _player -> composite_spell_speed();     }
  double dodge() const            { return _player -> composite_dodge();      }
  double parry() const            { return _player -> composite_parry();      }
  double block() const            { return _player -> composite_block();      }
  double crit_block() const       { return _player -> composite_crit_block(); }
  double crit_avoidance() const   { return _player -> composite_crit_avoidance();       }
  double miss() const             { return _player -> composite_miss();       }
  double armor() const            { return _player -> composite_armor();           }
  double mastery() const          { return _player -> composite_mastery();   }
  double mastery_value() const    { return _player -> composite_mastery_value();   }
  double damage_versatility() const { return _player -> composite_damage_versatility(); }
  double heal_versatility() const { return _player -> composite_heal_versatility(); }
  double mitigation_versatility() const { return _player -> composite_mitigation_versatility(); }
  double leech() const { return _player -> composite_leech(); }
  double run_speed() const { return _player -> composite_run_speed(); }
  double avoidance() const { return _player -> composite_avoidance(); }
#endif
};

struct player_processed_report_information_t
{
  bool generated = false;
  bool buff_lists_generated = false;
  std::array<std::string, SCALE_METRIC_MAX> gear_weights_wowhead_std_link, gear_weights_pawn_string, gear_weights_askmrrobot_link;
  std::string save_str;
  std::string save_gear_str;
  std::string save_talents_str;
  std::string save_actions_str;
  std::string comment_str;
  std::string thumbnail_url;
  std::string html_profile_str;
  std::vector<buff_t*> buff_list, dynamic_buffs, constant_buffs;

};

/* Contains any data collected during / at the end of combat
 * Mostly statistical data collection, represented as sample data containers
 */
struct player_collected_data_t
{
  extended_sample_data_t fight_length;
  extended_sample_data_t waiting_time, pooling_time, executed_foreground_actions;

  // DMG
  extended_sample_data_t dmg;
  extended_sample_data_t compound_dmg;
  extended_sample_data_t prioritydps;
  extended_sample_data_t dps;
  extended_sample_data_t dpse;
  extended_sample_data_t dtps;
  extended_sample_data_t dmg_taken;
  sc_timeline_t timeline_dmg;
  sc_timeline_t timeline_dmg_taken;
  // Heal
  extended_sample_data_t heal;
  extended_sample_data_t compound_heal;
  extended_sample_data_t hps;
  extended_sample_data_t hpse;
  extended_sample_data_t htps;
  extended_sample_data_t heal_taken;
  sc_timeline_t timeline_healing_taken;
  // Absorb
  extended_sample_data_t absorb;
  extended_sample_data_t compound_absorb;
  extended_sample_data_t aps;
  extended_sample_data_t atps;
  extended_sample_data_t absorb_taken;
  // Tank
  extended_sample_data_t deaths;
  extended_sample_data_t theck_meloree_index;
  extended_sample_data_t effective_theck_meloree_index;
  extended_sample_data_t max_spike_amount;

  // Metric used to end simulations early
  extended_sample_data_t target_metric;
  mutex_t target_metric_mutex;

  std::vector<simple_sample_data_t> resource_lost, resource_gained;
  struct resource_timeline_t
  {
    resource_e type;
    sc_timeline_t timeline;

    resource_timeline_t( resource_e t = RESOURCE_NONE ) : type( t ) {}
  };
  // Druid requires 4 resource timelines health/mana/energy/rage
  std::vector<resource_timeline_t> resource_timelines;

  std::vector<simple_sample_data_with_min_max_t > combat_end_resource;

  struct stat_timeline_t
  {
    stat_e type;
    sc_timeline_t timeline;

    stat_timeline_t( stat_e t = STAT_NONE ) : type( t ) {}
  };

  std::vector<stat_timeline_t> stat_timelines;

  // hooked up in resource timeline collection event
  struct health_changes_timeline_t
  {
    double previous_loss_level, previous_gain_level;
    sc_timeline_t timeline; // keeps only data per iteration
    sc_timeline_t timeline_normalized; // same as above, but normalized to current player health
    sc_timeline_t merged_timeline;
    bool collect; // whether we collect all this or not.
    health_changes_timeline_t() : previous_loss_level( 0.0 ), previous_gain_level( 0.0 ), collect( false ) {}

    void set_bin_size( double bin )
    {
      timeline.set_bin_size( bin );
      timeline_normalized.set_bin_size( bin );
      merged_timeline.set_bin_size( bin );
    }

    double get_bin_size() const
    {
      if ( timeline.get_bin_size() != timeline_normalized.get_bin_size() || timeline.get_bin_size() != merged_timeline.get_bin_size() )
      {
        assert( false );
        return 0.0;
      }
      else
        return timeline.get_bin_size();
    }
  };

  health_changes_timeline_t health_changes;     //records all health changes
  health_changes_timeline_t health_changes_tmi; //records only health changes due to damage and self-healng/self-absorb

  // Total number of iterations for the actor. Needed for single_actor_batch if target_error is
  // used.
  int total_iterations;

  struct action_sequence_data_t : noncopyable
  {
    const action_t* action;
    const player_t* target;
    const timespan_t time;
    timespan_t wait_time;
    std::vector< std::pair< buff_t*, std::vector<double> > > buff_list;
    std::vector< std::pair< cooldown_t*, std::vector<double> > > cooldown_list;
    std::vector< std::pair<player_t*, std::vector< std::pair< buff_t*, std::vector<double> > > > > target_list;
    std::array<double, RESOURCE_MAX> resource_snapshot;
    std::array<double, RESOURCE_MAX> resource_max_snapshot;

    action_sequence_data_t( const timespan_t& ts, const timespan_t& wait, const player_t* p );
    action_sequence_data_t( const action_t* a, const player_t* t, const timespan_t& ts, const player_t* p );
  };
  std::vector<action_sequence_data_t> action_sequence;
  std::vector<action_sequence_data_t> action_sequence_precombat;

  // Buffed snapshot_stats (for reporting)
  struct buffed_stats_t
  {
    std::array< double, ATTRIBUTE_MAX > attribute;
    std::array< double, RESOURCE_MAX > resource;

    double spell_power, spell_hit, spell_crit_chance, manareg_per_second;
    double attack_power,  attack_hit,  mh_attack_expertise,  oh_attack_expertise, attack_crit_chance;
    double armor, miss, crit, dodge, parry, block, bonus_armor;
    double spell_haste, spell_speed, attack_haste, attack_speed;
    double mastery_value;
    double damage_versatility, heal_versatility, mitigation_versatility;
    double leech, run_speed, avoidance;
  } buffed_stats_snapshot;

  player_collected_data_t( const player_t* player );
  void reserve_memory( const player_t& );
  void merge( const player_t& );
  void analyze( const player_t& );
  void collect_data( const player_t& );
  void print_tmi_debug_csv( const sc_timeline_t* nma, const std::vector<double>& weighted_value, const player_t& p );
  double calculate_tmi( const health_changes_timeline_t& tl, int window, double f_length, const player_t& p );
  double calculate_max_spike_damage( const health_changes_timeline_t& tl, int window );
  std::ostream& data_str( std::ostream& s ) const;

};

struct player_talent_points_t
{
public:
  using validity_fn_t = std::function<bool(const spell_data_t*)>;

  player_talent_points_t() { clear(); }

  int choice( int row ) const
  {
    row_check( row );
    return choices[ row ];
  }

  void clear( int row )
  {
    row_check( row );
    choices[ row ] = -1;
  }

  bool has_row_col( int row, int col ) const
  { return choice( row ) == col; }

  void select_row_col( int row, int col )
  {
    row_col_check( row, col );
    choices[ row ] = col;
  }

  void clear();
  std::string to_string() const;

  bool validate( const spell_data_t* spell, int row, int col ) const
  {
    return has_row_col( row, col ) ||
      range::find_if( validity_fns, [ spell ]( const validity_fn_t& fn ) { return fn( spell ); } ) !=
      validity_fns.end();
  }

  friend std::ostream& operator << ( std::ostream& os, const player_talent_points_t& tp )
  { os << tp.to_string(); return os; }

  void register_validity_fn( const validity_fn_t& fn )
  { validity_fns.push_back( fn ); }

private:
  std::array<int, MAX_TALENT_ROWS> choices;
  std::vector<validity_fn_t> validity_fns;

  static void row_check( int row )
  { assert( row >= 0 && row < MAX_TALENT_ROWS ); ( void )row; }

  static void column_check( int col )
  { assert( col >= 0 && col < MAX_TALENT_COLS ); ( void )col; }

  static void row_col_check( int row, int col )
  { row_check( row ); column_check( col ); }

};

// Actor
/* actor_t is a lightweight representation of an actor belonging to a simulation,
 * having a name and some event-related helper functionality.
 */

struct actor_t : private noncopyable
{
  sim_t* sim; // owner
  spawner::base_actor_spawner_t* spawner; // Creation/spawning of the actor delegated here
  std::string name_str;
  int event_counter; // safety counter. Shall never be less than zero

#ifdef ACTOR_EVENT_BOOKKEEPING
  /// Measure cpu time for actor-specific events.
  stopwatch_t event_stopwatch;
#endif // ACTOR_EVENT_BOOKKEEPING

  actor_t( sim_t* s, const std::string& name ) :
    sim( s ), spawner( nullptr ), name_str( name ),
#ifdef ACTOR_EVENT_BOOKKEEPING
    event_counter( 0 ),
    event_stopwatch( STOPWATCH_THREAD )
#else
    event_counter( 0 )
#endif
  {

  }
  virtual ~ actor_t() { }
  virtual const char* name() const
  { return name_str.c_str(); }
};

/* Player Report Extension
 * Allows class modules to write extension to the report sections
 * based on the dynamic class of the player
 */

struct player_report_extension_t
{
public:
  virtual ~player_report_extension_t()
  {

  }
  virtual void html_customsection( report::sc_html_stream& )
  {

  }
};

// Assessors, currently a state assessor functionality is defined.
namespace assessor
{
  // Assessor commands (continue evaluating, or stop to this assessor)
  enum command { CONTINUE = 0U, STOP };

  // Default assessor priorities
  enum priority_e
  {
    TARGET_MITIGATION = 100U,    // Target assessing (mitigation etc)
    TARGET_DAMAGE     = 200U,    // Do damage to target (and related functionality)
    LOG               = 300U,    // Logging (including debug output)
    CALLBACKS         = 400U,    // Impact callbacks
    LEECH             = 1000U,   // Leech processing
  };

  // State assessor callback type
  using state_assessor_t = std::function<command(dmg_e, action_state_t*)>;

  // A simple entry that defines a state assessor
  struct state_assessor_entry_t
  {
    int priority;
    state_assessor_t assessor;
  };

  // State assessor functionality creates an ascending priority-based list of manipulators for state
  // (action_state_t) objects. Each assessor is run on the state object, until assessor::STOP is
  // encountered, or all the assessors have ran.
  //
  // There are a number of default assessors associated for outgoing damage state (the only place
  // where this code is used for now), defined in player_t::init_assessors. Init_assessors is called
  // late in the actor initialization order, and can take advantage of conditional registration
  // based on talents, items, special effects, specialization and other actor-related state.
  //
  // An assessor function takes two parameters, dmg_e indicating the "damage type", and
  // action_state_t pointer to the state being assessed. The state object can be manipulated by the
  // function, but it may not be freed. The function must return one of priority enum values,
  // typically priority::CONTINUE to continue the pipeline.
  //
  // Assessors are sorted to ascending priority order in player_t::init_finished.
  //
  struct state_assessor_pipeline_t
  {
    std::vector<state_assessor_entry_t> assessors;

    void add( int p, state_assessor_t cb )
    { assessors.push_back( state_assessor_entry_t{p, std::move(cb)} ); }

    void sort()
    {
      range::sort( assessors, []( const state_assessor_entry_t& a, const state_assessor_entry_t& b )
      { return a.priority < b.priority; } );
    }

    void execute( dmg_e type, action_state_t* state )
    {
      for ( const auto& a: assessors )
      {
        if ( a.assessor( type, state ) == STOP )
        {
          break;
        }
      }
    }
  };
} // Namespace assessor ends

// Player ===================================================================

struct action_variable_t
{
  std::string name_;
  double current_value_, default_, constant_value_;
  std::vector<action_t*> variable_actions;

  action_variable_t( const std::string& name, double def = 0 ) :
    name_( name ), current_value_( def ), default_( def ),
    constant_value_( std::numeric_limits<double>::lowest() )
  { }

  double value() const
  { return current_value_; }

  void reset()
  { current_value_ = default_; }

  bool is_constant( double* constant_value ) const
  {
    *constant_value = constant_value_;
    return constant_value_ != std::numeric_limits<double>::lowest();
  }

  // Implemented in sc_player.cpp
  void optimize();
};

struct scaling_metric_data_t {
  std::string name;
  double value, stddev;
  scale_metric_e metric;
  scaling_metric_data_t( scale_metric_e m, const std::string& n, double v, double dev ) :
    name( n ), value( v ), stddev( dev ), metric( m ) {}
  scaling_metric_data_t( scale_metric_e m, const extended_sample_data_t& sd ) :
    name( sd.name_str ), value( sd.mean() ), stddev( sd.mean_std_dev ), metric( m ) {}
  scaling_metric_data_t( scale_metric_e m, const sc_timeline_t& tl, const std::string& name ) :
    name( name ), value( tl.mean() ), stddev( tl.mean_stddev() ), metric( m ) {}
};

struct player_scaling_t
{
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling;
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling_normalized;
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling_error;
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling_delta_dps;
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling_compare_error;
  std::array<double, SCALE_METRIC_MAX> scaling_lag, scaling_lag_error;
  std::array<bool, STAT_MAX> scales_with;
  std::array<double, STAT_MAX> over_cap;
  std::array<std::vector<stat_e>, SCALE_METRIC_MAX> scaling_stats; // sorting vector

  player_scaling_t()
  {
    range::fill( scaling_lag, 0 );
    range::fill( scaling_lag_error, 0 );
    range::fill( scales_with, false );
    range::fill( over_cap, 0 );
  }

  void enable( stat_e stat )
  { scales_with[ stat ] = true; }

  void disable( stat_e stat )
  { scales_with[ stat ] = false; }

  void set( stat_e stat, bool state )
  { scales_with[ stat ] = state; }
};

// Resources
struct player_resources_t
{
  std::array<double, RESOURCE_MAX> base, initial, max, current, temporary,
      base_multiplier, initial_multiplier;
  std::array<int, RESOURCE_MAX> infinite_resource;
  std::array<bool, RESOURCE_MAX> active_resource;
  // Initial user-input resources
  std::array<double, RESOURCE_MAX> initial_opt;
  // Start-of-combat resource level
  std::array<double, RESOURCE_MAX> start_at;
  std::array<double, RESOURCE_MAX> base_regen_per_second;

  player_resources_t() :
    base(),
    initial(),
    max(),
    current(),
    temporary(),
    infinite_resource(),
    start_at(),
    base_regen_per_second()
  {
    base_multiplier.fill( 1.0 );
    initial_multiplier.fill( 1.0 );
    active_resource.fill( true );
    initial_opt.fill( -1.0 );

    // Init some resources to specific values at the beginning of the combat, defaults to 0.
    // The actual start-of-combat resource is min( computed_max, start_at ).
    start_at[ RESOURCE_HEALTH     ] = std::numeric_limits<double>::max();
    start_at[ RESOURCE_MANA       ] = std::numeric_limits<double>::max();
    start_at[ RESOURCE_FOCUS      ] = std::numeric_limits<double>::max();
    start_at[ RESOURCE_ENERGY     ] = std::numeric_limits<double>::max();
    start_at[ RESOURCE_RUNE       ] = std::numeric_limits<double>::max();
    start_at[ RESOURCE_SOUL_SHARD ] = 3.0;
  }

  double pct( resource_e rt ) const
  { return max[ rt ] ? current[ rt ] / max[ rt ] : 0.0; }

  bool is_infinite( resource_e rt ) const
  { return infinite_resource[ rt ] != 0; }

  bool is_active( resource_e rt ) const
  { return active_resource[ rt ] && current[ rt ] >= 0.0; }
};

struct player_t : public actor_t
{
  static const int default_level = 120;

  // static values
  player_e type;
  player_t* parent; // corresponding player in main thread
  int index;
  int creation_iteration; // The iteration when this actor was created, -1 for "init"
  size_t actor_index;
  int actor_spawn_index; // a unique identifier for each arise() of the actor
  // (static) attributes - things which should not change during combat
  race_e       race;
  role_e       role;
  int          true_level; /* The character's true level. If the outcome would change when the character's level is
                           scaled down (such as when timewalking) then use the level() method instead. */
  int          party;
  int          ready_type;
  specialization_e  _spec;
  bool         bugs; // If true, include known InGame mechanics which are probably the cause of a bug and not inteded
  int          disable_hotfixes;
  bool         scale_player;
  double       death_pct; // Player will die if he has equal or less than this value as health-pct
  double       height; // Actor height, only used for enemies. Affects the travel distance calculation for spells.
  double       combat_reach; // AKA hitbox size, for enemies.
  profile_source profile_source_;
  player_t*    default_target;

  // dynamic attributes - things which change during combat
  player_t*   target;
  bool        initialized;
  bool        potion_used;

  std::string talents_str, id_str, target_str;
  std::string region_str, server_str, origin_str;
  std::string race_str, professions_str, position_str;
  enum timeofday_e { NIGHT_TIME, DAY_TIME, } timeofday; // Specify InGame time of day to determine Night Elf racial
  enum zandalari_loa_e {AKUNDA, BWONSAMDI, GONK, KIMBUL, KRAGWA, PAKU} zandalari_loa; //Specify which loa zandalari has chosen to determine racial

  // GCD Related attributes
  timespan_t  gcd_ready, base_gcd, min_gcd; // When is GCD ready, default base and minimum GCD times.
  haste_type_e gcd_haste_type; // If the current GCD is hasted, what haste type is used
  double gcd_current_haste_value; // The currently used haste value for GCD speedup

  timespan_t started_waiting;
  std::vector<pet_t*> pet_list;
  std::vector<pet_t*> active_pets;
  std::vector<absorb_buff_t*> absorb_buff_list;
  std::map<unsigned,instant_absorb_t> instant_absorb_list;

  int         invert_scaling;

  // Reaction
  timespan_t  reaction_offset, reaction_max, reaction_mean, reaction_stddev, reaction_nu;
  // Latency
  timespan_t  world_lag, world_lag_stddev;
  timespan_t  brain_lag, brain_lag_stddev;
  bool        world_lag_override, world_lag_stddev_override;
  timespan_t  cooldown_tolerance_;

  // Data access
  dbc_t       dbc;

  // Option Parsing
  std::vector<std::unique_ptr<option_t>> options;

  // Stat Timelines to Display
  std::vector<stat_e> stat_timelines;

  // Talent Parsing
  player_talent_points_t talent_points;
  std::string talent_overrides_str;

  // Profs
  std::array<int, PROFESSION_MAX> profession;

  // Artifact
  std::unique_ptr<artifact::player_artifact_data_t> artifact;

  /// Azerite state object
  std::unique_ptr<azerite::azerite_state_t> azerite;

  /// Azerite essence state object
  std::unique_ptr<azerite::azerite_essence_state_t> azerite_essence;

  // TODO: FIXME, these stats should not be increased by scale factor deltas
  struct base_initial_current_t
  {
    base_initial_current_t();
    std::string to_string();
    gear_stats_t stats;

    double spell_power_per_intellect, spell_power_per_attack_power, spell_crit_per_intellect;
    double attack_power_per_strength, attack_power_per_agility, attack_crit_per_agility;
    double dodge_per_agility, parry_per_strength;
    double health_per_stamina;
    std::array<double, SCHOOL_MAX> resource_reduction;
    double miss, dodge, parry, block;
    double hit, expertise;
    double spell_crit_chance, attack_crit_chance, block_reduction, mastery;
    double skill, skill_debuff, distance;
    double distance_to_move;
    double moving_away;
    movement_direction_e movement_direction;
    double armor_coeff;
    bool sleeping;
    rating_t rating;

    std::array<double, ATTRIBUTE_MAX> attribute_multiplier;
    double spell_power_multiplier, attack_power_multiplier, base_armor_multiplier, armor_multiplier;
    position_e position;
  }
  base, // Base values, from some database or overridden by user
  initial, // Base + Passive + Gear (overridden or items) + Player Enchants + Global Enchants
  current; // Current values, reset to initial before every iteration

  gear_stats_t passive; // Passive stats from various passive auras (and similar effects)

  timespan_t last_cast;

  // Defense Mechanics
  struct diminishing_returns_constants_t
  {
    double horizontal_shift = 0.0;
    double block_vertical_stretch = 0.0;
    double vertical_stretch = 0.0;
    double dodge_factor = 1.0;
    double parry_factor = 1.0;
    double miss_factor = 1.0;
    double block_factor = 1.0;
  } def_dr;

  // Weapons
  weapon_t main_hand_weapon;
  weapon_t off_hand_weapon;

  // Main, offhand, and ranged attacks
  attack_t* main_hand_attack;
  attack_t*  off_hand_attack;

  // Current attack speed (needed for dynamic attack speed adjustments)
  double current_attack_speed;

  player_resources_t resources;

  // Events
  action_t* executing;
  action_t* queueing;
  action_t* channeling;
  action_t* strict_sequence; // Strict sequence of actions currently being executed
  event_t* readying;
  event_t* off_gcd;
  event_t* cast_while_casting_poll_event; // Periodically check for something to do while casting
  std::vector<const cooldown_t*> off_gcd_cd, off_gcd_icd;
  std::vector<const cooldown_t*> cast_while_casting_cd, cast_while_casting_icd;
  timespan_t off_gcd_ready;
  timespan_t cast_while_casting_ready;
  bool in_combat;
  bool action_queued;
  bool first_cast;
  action_t* last_foreground_action;
  std::vector<action_t*> prev_gcd_actions;
  std::vector<action_t*> off_gcdactions; // Returns all off gcd abilities used since the last gcd.

  // Delay time used by "cast_delay" expression to determine when an action
  // can be used at minimum after a spell cast has finished, including GCD
  timespan_t cast_delay_reaction;
  timespan_t cast_delay_occurred;

  // Callbacks
  effect_callbacks_t<action_callback_t> callbacks;
  auto_dispose< std::vector<special_effect_t*> > special_effects;
  std::vector<std::function<void(player_t*)> > callbacks_on_demise;
  std::vector<std::function<void(void)>> callbacks_on_arise;

  // Action Priority List
  auto_dispose< std::vector<action_t*> > action_list;
  /// Actions that have an action-specific dynamic targeting
  std::set<action_t*> dynamic_target_action_list;
  std::string action_list_str;
  std::string choose_action_list;
  std::string action_list_skip;
  std::string modify_action;
  std::string use_apl;
  bool use_default_action_list;
  auto_dispose< std::vector<dot_t*> > dot_list;
  auto_dispose< std::vector<action_priority_list_t*> > action_priority_list;
  std::vector<action_t*> precombat_action_list;
  action_priority_list_t* active_action_list;
  action_priority_list_t* default_action_list;
  action_priority_list_t* active_off_gcd_list;
  action_priority_list_t* active_cast_while_casting_list;
  action_priority_list_t* restore_action_list;
  std::unordered_map<std::string, std::string> alist_map;
  std::string action_list_information; // comment displayed in profile
  bool no_action_list_provided;

  bool quiet;
  // Reporting
  std::unique_ptr<player_report_extension_t> report_extension;
  timespan_t arise_time;
  timespan_t iteration_fight_length;
  timespan_t iteration_waiting_time, iteration_pooling_time;
  int iteration_executed_foreground_actions;
  std::array< double, RESOURCE_MAX > iteration_resource_lost, iteration_resource_gained;
  double rps_gain, rps_loss;
  std::string tmi_debug_file_str;
  double tmi_window;

  auto_dispose< std::vector<buff_t*> > buff_list;
  auto_dispose< std::vector<proc_t*> > proc_list;
  auto_dispose< std::vector<gain_t*> > gain_list;
  auto_dispose< std::vector<stats_t*> > stats_list;
  auto_dispose< std::vector<benefit_t*> > benefit_list;
  auto_dispose< std::vector<uptime_t*> > uptime_list;
  auto_dispose< std::vector<cooldown_t*> > cooldown_list;
  auto_dispose< std::vector<real_ppm_t*> > rppm_list;
  auto_dispose< std::vector<shuffled_rng_t*> > shuffled_rng_list;
  std::vector<cooldown_t*> dynamic_cooldown_list;
  std::array< std::vector<plot_data_t>, STAT_MAX > dps_plot_data;
  std::vector<std::vector<plot_data_t> > reforge_plot_data;
  auto_dispose< std::vector<luxurious_sample_data_t*> > sample_data_list;

  // All Data collected during / end of combat
  player_collected_data_t collected_data;

  // Damage
  double iteration_dmg, priority_iteration_dmg, iteration_dmg_taken; // temporary accumulators
  double dpr;
  struct incoming_damage_entry_t {
    timespan_t time;
    double amount;
    school_e school;
  };
  std::vector<incoming_damage_entry_t> incoming_damage; // for tank active mitigation conditionals

  // Heal
  double iteration_heal, iteration_heal_taken, iteration_absorb, iteration_absorb_taken; // temporary accumulators
  double hpr;
  std::vector<unsigned> absorb_priority; // for strict sequence absorbs

  player_processed_report_information_t report_information;

  void sequence_add( const action_t* a, const player_t* target, const timespan_t& ts );
  void sequence_add_wait( const timespan_t& amount, const timespan_t& ts );

  // Gear
  std::string items_str, meta_gem_str, potion_str, flask_str, food_str, rune_str;
  std::vector<item_t> items;
  gear_stats_t gear, enchant; // Option based stats
  gear_stats_t total_gear; // composite of gear, enchant and for non-pets sim -> enchant
  std::unique_ptr<set_bonus_t> sets;
  meta_gem_e meta_gem;
  bool matching_gear;
  bool karazhan_trinkets_paired;
  cooldown_t item_cooldown;
  cooldown_t* legendary_tank_cloak_cd; // non-Null if item available


  // Warlord's Unseeing Eye (6.2 Trinket)
  double warlords_unseeing_eye;
  stats_t* warlords_unseeing_eye_stats;

  // Misc Multipliers
  // auto attack multiplier (for Jeweled Signet of Melandrus and similar effects)
  double auto_attack_multiplier;
  // Prepatch Insignia of the Grand Army flat dmg multiplier
  double insignia_of_the_grand_army_multiplier;

  // Scale Factors
  std::unique_ptr<player_scaling_t> scaling;

  // Movement & Position
  double base_movement_speed;
  double passive_modifier; // _PASSIVE_ movement speed modifiers
  double x_position, y_position, default_x_position, default_y_position;

  struct consumables_t {
    buff_t* flask;
    stat_buff_t* guardian_elixir;
    stat_buff_t* battle_elixir;
    buff_t* food;
    buff_t* augmentation;
  } consumables;

  struct buffs_t
  {
    buff_t* angelic_feather;
    buff_t* beacon_of_light;
    buff_t* blood_fury;
    buff_t* body_and_soul;
    buff_t* damage_done;
    buff_t* darkflight;
    buff_t* devotion_aura;
    buff_t* entropic_embrace;
    buff_t* exhaustion;
    buff_t* guardian_spirit;
    buff_t* blessing_of_sacrifice;
    buff_t* mongoose_mh;
    buff_t* mongoose_oh;
    buff_t* nitro_boosts;
    buff_t* pain_supression;
    buff_t* movement;
    buff_t* stampeding_roar;
    buff_t* shadowmeld;
    buff_t* windwalking_movement_aura;
    buff_t* stoneform;
    buff_t* stunned;
    std::array<buff_t*, 4> ancestral_call;
    buff_t* fireblood;
    buff_t* embrace_of_paku;

    buff_t* berserking;
    buff_t* bloodlust;

    buff_t* cooldown_reduction;
    buff_t* amplification;
    buff_t* amplification_2;

    // Legendary meta stuff
    buff_t* courageous_primal_diamond_lucidity;
    buff_t* tempus_repit;
    buff_t* fortitude;

    buff_t* archmages_greater_incandescence_str;
    buff_t* archmages_greater_incandescence_agi;
    buff_t* archmages_greater_incandescence_int;
    buff_t* archmages_incandescence_str;
    buff_t* archmages_incandescence_agi;
    buff_t* archmages_incandescence_int;
    buff_t* legendary_aoe_ring; // Legendary ring buff.
    buff_t* legendary_tank_buff;

    // T17 LFR stuff
    buff_t* surge_of_energy;
    buff_t* natures_fury;
    buff_t* brute_strength;

    // 7.0 trinket proxy buffs
    buff_t* incensed;
    buff_t* taste_of_mana; // Gnawed Thumb Ring buff

    // 7.0 Legendaries
    buff_t* aggramars_stride;

    // 7.1
    buff_t* temptation; // Ring that goes on a 5 minute cd if you use it too much.
    buff_t* norgannons_foresight; //Legendary item that allows movement for 5 seconds if you stand still for 8.
    buff_t* norgannons_foresight_ready;
    buff_t* nefarious_pact; // Whispers in the dark good buff
    buff_t* devils_due; // Whispers in the dark bad buff

    // 6.2 trinket proxy buffs
    buff_t* naarus_discipline; // Priest-Discipline Boss 13 T18 trinket
    buff_t* tyrants_immortality; // Tyrant's Decree trinket proc
    buff_t* tyrants_decree_driver; // Tyrant's Decree trinket driver

    buff_t* fel_winds; // T18 LFR Plate Melee Attack Speed buff
    buff_t* demon_damage_buff; // 6.2.3 Heirloom trinket demon damage buff

    // Darkmoon Faire versatility food
    buff_t* dmf_well_fed;

    // 8.0
    buff_t* galeforce_striking; // Gale-Force Striking weapon enchant
    buff_t* torrent_of_elements; // Torrent of Elements weapon enchant

    // 8.0 - Leyshock's Grand Compendium stat buffs
    buff_t* leyshock_crit;
    buff_t* leyshock_haste;
    buff_t* leyshock_mastery;
    buff_t* leyshock_versa;

    // Azerite power
    buff_t* normalization_increase;

    // Uldir
    buff_t* reorigination_array;

    /// 8.2 Azerite Essences
    stat_buff_t* memory_of_lucid_dreams;
    stat_buff_t* lucid_dreams; // Versatility Buff from Rank 3
    buff_t* reckless_force; // The Unbound Force minor - crit chance
    buff_t* reckless_force_counter; // The Unbound Force minor - max 20 stack counter
    stat_buff_t* lifeblood; // Worldvein Resonance - grant primary stat per shard, max 4
    buff_t* seething_rage; // Blood of the Enemy major - 25% crit dam
    stat_buff_t* reality_shift; // Ripple in Space minor - primary stat on moving 25yds
    buff_t* guardian_of_azeroth; // Condensed Life-Force major - R3 stacking haste on pet cast

    // 8.2 misc
    buff_t* damage_to_aberrations; // Benthic belt special effect
    buff_t* fathom_hunter; // Follower themed Benthic boots special effect
    buff_t* delirious_frenzy; // Dream's End 1H STR axe attack speed buff
    buff_t* bioelectric_charge; // Diver's Folly 1H AGI axe buff to store damage
    buff_t* razor_coral; // Ashvane's Razor Coral trinket crit rating buff

  } buffs;

  struct debuffs_t
  {
    buff_t* bleeding;
    buff_t* casting;
    buff_t* flying;
    buff_t* forbearance;
    buff_t* invulnerable;
    buff_t* vulnerable;
    buff_t* damage_taken;

    // WoD debuffs
    buff_t* mortal_wounds;

    // BfA Raid Damage Modifier Debuffs
    buff_t* chaos_brand;  // Demon Hunter
    buff_t* mystic_touch; // Monk
  } debuffs;

  struct gains_t
  {
    gain_t* arcane_torrent;
    gain_t* endurance_of_niuzao;
    std::array<gain_t*, RESOURCE_MAX> resource_regen;
    gain_t* health;
    gain_t* mana_potion;
    gain_t* restore_mana;
    gain_t* touch_of_the_grave;
    gain_t* vampiric_embrace;
    gain_t* warlords_unseeing_eye;
    gain_t* embrace_of_bwonsamdi;

    gain_t* leech;
  } gains;

  struct spells_t
  {
    action_t* leech;
  } spells;

  struct procs_t
  {
    proc_t* parry_haste;
  } procs;

  struct uptimes_t
  {
    uptime_t* primary_resource_cap;
  } uptimes;

  struct racials_t
  {
    const spell_data_t* quickness;
    const spell_data_t* command;
    const spell_data_t* arcane_acuity;
    const spell_data_t* heroic_presence;
    const spell_data_t* might_of_the_mountain;
    const spell_data_t* expansive_mind;
    const spell_data_t* nimble_fingers;
    const spell_data_t* time_is_money;
    const spell_data_t* the_human_spirit;
    const spell_data_t* touch_of_elune;
    const spell_data_t* brawn;
    const spell_data_t* endurance;
    const spell_data_t* viciousness;
    const spell_data_t* magical_affinity;
    const spell_data_t* mountaineer;
    const spell_data_t* brush_it_off;
  } racials;

  struct passives_t
  {
    double amplification_1;
    double amplification_2;
  } passive_values;

  bool active_during_iteration;
  const spelleffect_data_t* _mastery; // = find_mastery_spell( specialization() ) -> effectN( 1 );
  player_stat_cache_t cache;
  auto_dispose<std::vector<action_variable_t*>> variables;
  std::vector<std::string> action_map;

  regen_type_e regen_type;

  /// Last iteration time regeneration occurred. Set at player_t::arise()
  timespan_t last_regen;

  /// A list of CACHE_x enumerations (stats) that affect the resource regeneration of the actor.
  std::vector<bool> regen_caches;

  /// Flag to indicate if any pets require dynamic regneration. Initialized in player_t::init().
  bool dynamic_regen_pets;

  /// Visited action lists, needed for call_action_list support. Reset by player_t::execute_action().
  uint64_t visited_apls_;

  /// Internal counter for action priority lists, used to set action_priority_list_t::internal_id for lists.
  unsigned action_list_id_;

  /// Current execution type
  execute_type current_execute_type;


  using resource_callback_function_t = std::function<void()>;

private:
  /// Flag to activate/deactive resource callback checks. Motivation: performance.
  bool has_active_resource_callbacks;

  struct resource_callback_entry_t
  {
    resource_e resource;
    double value;
    bool is_pct;
    bool fire_once;
    bool is_consumed;
    resource_callback_function_t callback;
  };
  std::vector<resource_callback_entry_t> resource_callbacks;

public:



  player_t( sim_t* sim, player_e type, const std::string& name, race_e race_e );

  // Static methods
  static player_t* create( sim_t* sim, const player_description_t& );
  static bool _is_enemy( player_e t ) { return t == ENEMY || t == ENEMY_ADD || t == TMI_BOSS || t == TANK_DUMMY; }
  static bool _is_sleeping( const player_t* t ) { return t -> current.sleeping; }


  // Overrides
  const char* name() const override
  { return name_str.c_str(); }


  // Normal methods
  void init_character_properties();
  void collect_resource_timeline_information();
  void stat_gain( stat_e stat, double amount, gain_t* g = nullptr, action_t* a = nullptr, bool temporary = false );
  void stat_loss( stat_e stat, double amount, gain_t* g = nullptr, action_t* a = nullptr, bool temporary = false );
  void modify_current_rating( rating_e stat, double amount );
  void create_talents_numbers();
  void create_talents_armory();
  void create_talents_wowhead();
  void clear_action_priority_lists() const;
  void copy_action_priority_list( const std::string& old_list, const std::string& new_list );
  void change_position( position_e );
  void register_resource_callback(resource_e resource, double value, resource_callback_function_t callback,
      bool use_pct, bool fire_once = true);
  bool add_action( std::string action, std::string options = "", std::string alist = "default" );
  bool add_action( const spell_data_t* s, std::string options = "", std::string alist = "default" );
  void add_option( std::unique_ptr<option_t> o )
  {
    // Push_front so derived classes (eg. enemy_t) can override existing options
    options.insert( options.begin(), std::move( o ) );
  }
  void parse_talents_numbers( const std::string& talent_string );
  bool parse_talents_armory( const std::string& talent_string );
  bool parse_talents_armory2( const std::string& talent_string );
  bool parse_talents_wowhead( const std::string& talent_string );
  expr_t* create_resource_expression( const std::string& name );


  bool is_moving() const
  { return buffs.movement && buffs.movement -> check(); }
  double composite_block_dr( double extra_block ) const;
  bool is_pet() const { return type == PLAYER_PET || type == PLAYER_GUARDIAN || type == ENEMY_ADD; }
  bool is_enemy() const { return _is_enemy( type ); }
  bool is_add() const { return type == ENEMY_ADD; }
  bool is_sleeping() const { return _is_sleeping( this ); }
  bool is_my_pet( const player_t* t ) const;
  bool in_gcd() const { return gcd_ready > sim -> current_time(); }
  bool recent_cast() const;
  bool dual_wield() const
  { return main_hand_weapon.type != WEAPON_NONE && off_hand_weapon.type != WEAPON_NONE; }
  bool has_shield_equipped() const
  { return  items[ SLOT_OFF_HAND ].parsed.data.item_subclass == ITEM_SUBCLASS_ARMOR_SHIELD; }
  /// Figure out if healing should be recorded
  bool record_healing() const
  { return role == ROLE_TANK || role == ROLE_HEAL || sim -> enable_dps_healing; }
  specialization_e specialization() const
  { return _spec; }
  const char* primary_tree_name() const;
  timespan_t total_reaction_time();
  double avg_item_level() const;
  double get_attribute( attribute_e a ) const
  { return util::floor( composite_attribute( a ) * composite_attribute_multiplier( a ) ); }
  double strength() const
  { return get_attribute( ATTR_STRENGTH ); }
  double agility() const
  { return get_attribute( ATTR_AGILITY ); }
  double stamina() const
  { return get_attribute( ATTR_STAMINA ); }
  double intellect() const
  { return get_attribute( ATTR_INTELLECT ); }
  double spirit() const
  { return get_attribute( ATTR_SPIRIT ); }
  double mastery_coefficient() const
  { return _mastery -> mastery_value(); }
  double get_player_distance( const player_t& ) const;
  double get_ground_aoe_distance( const action_state_t& ) const;
  double get_position_distance( double m = 0, double v = 0 ) const;
  double compute_incoming_damage( timespan_t interval) const;
  double compute_incoming_magic_damage( timespan_t interval ) const;
  double calculate_time_to_bloodlust() const;
  slot_e parent_item_slot( const item_t& item ) const;
  slot_e child_item_slot( const item_t& item ) const;
  /// Actor-specific cooldown tolerance for queueable actions
  timespan_t cooldown_tolerance() const
  { return cooldown_tolerance_ < timespan_t::zero() ? sim -> default_cooldown_tolerance : cooldown_tolerance_; }
  position_e position() const
  { return current.position; }


  pet_t* cast_pet() { return debug_cast<pet_t*>( this ); }
  const pet_t* cast_pet() const { return debug_cast<const pet_t*>( this ); }

  artifact_power_t find_artifact_spell( const std::string&, bool = false ) const
  { return {}; }
  artifact_power_t find_artifact_spell( unsigned ) const
  { return {}; }
  azerite_power_t find_azerite_spell( const std::string& name, bool tokenized = false ) const;
  azerite_power_t find_azerite_spell( unsigned power_id ) const;
  azerite_essence_t find_azerite_essence( const std::string& name, bool tokenized = false ) const;
  azerite_essence_t find_azerite_essence( unsigned power_id ) const;
  const spell_data_t* find_racial_spell( const std::string& name, race_e s = RACE_NONE ) const;
  const spell_data_t* find_class_spell( const std::string& name, specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_pet_spell( const std::string& name ) const;
  const spell_data_t* find_talent_spell( const std::string& name, specialization_e s = SPEC_NONE, bool name_tokenized = false, bool check_validity = true ) const;
  const spell_data_t* find_specialization_spell( const std::string& name, specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_specialization_spell( unsigned spell_id, specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_mastery_spell( specialization_e s, uint32_t idx = 0 ) const;
  const spell_data_t* find_spell( const std::string& name, specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_spell( unsigned int id, specialization_e s ) const;
  const spell_data_t* find_spell( unsigned int id ) const;

  pet_t*      find_pet( const std::string& name ) const;
  item_t*     find_item_by_name( const std::string& name );
  item_t*     find_item_by_id( unsigned id );
  item_t*     find_item_by_use_effect_name( const std::string& name );
  action_t*   find_action( const std::string& ) const;
  cooldown_t* find_cooldown( const std::string& name ) const;
  dot_t*      find_dot     ( const std::string& name, player_t* source ) const;
  stats_t*    find_stats   ( const std::string& name ) const;
  gain_t*     find_gain    ( const std::string& name ) const;
  proc_t*     find_proc    ( const std::string& name ) const;
  benefit_t*  find_benefit ( const std::string& name ) const;
  uptime_t*   find_uptime  ( const std::string& name ) const;
  luxurious_sample_data_t* find_sample_data( const std::string& name ) const;
  action_priority_list_t* find_action_priority_list( const std::string& name ) const;
  int find_action_id( const std::string& name ) const;

  cooldown_t* get_cooldown( const std::string& name, action_t* action = nullptr );
  real_ppm_t* get_rppm    ( const std::string& name, const spell_data_t* data = spell_data_t::nil(), const item_t* item = nullptr );
  real_ppm_t* get_rppm    ( const std::string& name, double freq, double mod = 1.0, unsigned s = RPPM_NONE );
  shuffled_rng_t* get_shuffled_rng(const std::string& name, int success_entries = 0, int total_entries = 0);
  dot_t*      get_dot     ( const std::string& name, player_t* source );
  gain_t*     get_gain    ( const std::string& name );
  proc_t*     get_proc    ( const std::string& name );
  stats_t*    get_stats   ( const std::string& name, action_t* action = nullptr );
  benefit_t*  get_benefit ( const std::string& name );
  uptime_t*   get_uptime  ( const std::string& name );
  luxurious_sample_data_t* get_sample_data( const std::string& name );
  action_priority_list_t* get_action_priority_list( const std::string& name, const std::string& comment = std::string() );
  int get_action_id( const std::string& name );


  // Virtual methods
  virtual void invalidate_cache( cache_e c );
  virtual void init();
  virtual void override_talent( std::string& override_str );
  virtual void init_meta_gem();
  virtual void init_resources( bool force = false );
  virtual std::string init_use_item_actions( const std::string& append = std::string() );
  virtual std::string init_use_profession_actions( const std::string& append = std::string() );
  virtual std::string init_use_racial_actions( const std::string& append = std::string() );
  virtual std::vector<std::string> get_item_actions( const std::string& options = std::string() );
  virtual std::vector<std::string> get_profession_actions();
  virtual std::vector<std::string> get_racial_actions();
  virtual void init_target();
  virtual void init_race();
  virtual void init_talents();
  virtual void replace_spells();
  virtual void init_position();
  virtual void init_professions();
  virtual void init_spells();
  virtual void init_items();
  virtual void init_azerite(); /// Initialize azerite-related support structures for the actor
  virtual void init_weapon( weapon_t& );
  virtual void init_base_stats();
  virtual void init_initial_stats();
  virtual void init_defense();
  virtual void create_buffs();
  virtual void create_special_effects();
  virtual void init_special_effects();
  virtual void init_scaling();
  virtual void init_action_list() {}
  virtual void init_gains();
  virtual void init_procs();
  virtual void init_uptimes();
  virtual void init_benefits();
  virtual void init_rng();
  virtual void init_stats();
  virtual void init_distance_targeting();
  virtual void init_absorb_priority();
  virtual void init_assessors();
  virtual void create_actions();
  virtual void init_actions();
  virtual void init_finished();
  virtual bool verify_use_items() const;
  virtual void reset();
  virtual void combat_begin();
  virtual void combat_end();
  virtual void merge( player_t& other );
  virtual void datacollection_begin();
  virtual void datacollection_end();

  /// Single actor batch mode calls this every time the active (player) actor changes for all targets
  virtual void actor_changed() { }
  virtual void activate();
  virtual void deactivate();
  virtual int level() const;

  virtual double resource_regen_per_second( resource_e ) const;

  virtual double composite_melee_haste() const;
  virtual double composite_melee_speed() const;
  virtual double composite_melee_attack_power() const;
  virtual double composite_melee_attack_power( attack_power_e type ) const;
  virtual double composite_melee_hit() const;
  virtual double composite_melee_crit_chance() const;
  virtual double composite_melee_crit_chance_multiplier() const
  { return 1.0; }
  virtual double composite_melee_expertise( const weapon_t* w = nullptr ) const;
  virtual double composite_spell_haste() const;
  virtual double composite_spell_speed() const;
  virtual double composite_spell_power( school_e school ) const;
  virtual double composite_spell_crit_chance() const;
  virtual double composite_spell_crit_chance_multiplier() const
  { return 1.0; }
  virtual double composite_spell_hit() const;
  virtual double composite_mastery() const;
  virtual double composite_mastery_value() const;
  virtual double composite_damage_versatility() const;
  virtual double composite_heal_versatility() const;
  virtual double composite_mitigation_versatility() const;
  virtual double composite_leech() const;
  virtual double composite_run_speed() const;
  virtual double composite_avoidance() const;
  virtual double composite_armor() const;
  virtual double composite_bonus_armor() const;
  virtual double composite_base_armor_multiplier() const; // Modify Base Besistance
  virtual double composite_armor_multiplier() const; // Modify Armor%, affects everything
  virtual double composite_miss() const;
  virtual double composite_dodge() const;
  virtual double composite_parry() const;
  virtual double composite_block() const;
  virtual double composite_block_reduction( action_state_t* s ) const;
  virtual double composite_crit_block() const;
  virtual double composite_crit_avoidance() const;
  virtual double composite_attack_power_multiplier() const;
  virtual double composite_spell_power_multiplier() const;
  virtual double matching_gear_multiplier( attribute_e /* attr */ ) const
  { return 0; }
  virtual double composite_player_multiplier   ( school_e ) const;
  virtual double composite_player_dd_multiplier( school_e,  const action_t* /* a */ = nullptr ) const { return 1; }
  virtual double composite_player_td_multiplier( school_e,  const action_t* a = nullptr ) const;
  /// Persistent multipliers that are snapshot at the beginning of the spell application/execution
  virtual double composite_persistent_multiplier( school_e ) const
  { return 1.0; }
  virtual double composite_player_target_multiplier( player_t* target, school_e school ) const;
  virtual double composite_player_heal_multiplier( const action_state_t* s ) const;
  virtual double composite_player_dh_multiplier( school_e ) const { return 1; }
  virtual double composite_player_th_multiplier( school_e ) const;
  virtual double composite_player_absorb_multiplier( const action_state_t* s ) const;
  virtual double composite_player_pet_damage_multiplier( const action_state_t* ) const;
  virtual double composite_player_critical_damage_multiplier( const action_state_t* s ) const;
  virtual double composite_player_critical_healing_multiplier() const;
  virtual double composite_mitigation_multiplier( school_e ) const;
  virtual double temporary_movement_modifier() const;
  virtual double passive_movement_modifier() const;
  virtual double composite_movement_speed() const;
  virtual double composite_attribute( attribute_e attr ) const;
  virtual double composite_attribute_multiplier( attribute_e attr ) const;
  virtual double composite_rating_multiplier( rating_e /* rating */ ) const;
  virtual double composite_rating( rating_e rating ) const;
  virtual double composite_spell_hit_rating() const
  { return composite_rating( RATING_SPELL_HIT ); }
  virtual double composite_spell_crit_rating() const
  { return composite_rating( RATING_SPELL_CRIT ); }
  virtual double composite_spell_haste_rating() const
  { return composite_rating( RATING_SPELL_HASTE ); }
  virtual double composite_melee_hit_rating() const
  { return composite_rating( RATING_MELEE_HIT ); }
  virtual double composite_melee_crit_rating() const
  { return composite_rating( RATING_MELEE_CRIT ); }
  virtual double composite_melee_haste_rating() const
  { return composite_rating( RATING_MELEE_HASTE ); }
  virtual double composite_ranged_hit_rating() const
  { return composite_rating( RATING_RANGED_HIT ); }
  virtual double composite_ranged_crit_rating() const
  { return composite_rating( RATING_RANGED_CRIT ); }
  virtual double composite_ranged_haste_rating() const
  { return composite_rating( RATING_RANGED_HASTE ); }
  virtual double composite_mastery_rating() const
  { return composite_rating( RATING_MASTERY ); }
  virtual double composite_expertise_rating() const
  { return composite_rating( RATING_EXPERTISE ); }
  virtual double composite_dodge_rating() const
  { return composite_rating( RATING_DODGE ); }
  virtual double composite_parry_rating() const
  { return composite_rating( RATING_PARRY ); }
  virtual double composite_block_rating() const
  { return composite_rating( RATING_BLOCK ); }
  virtual double composite_damage_versatility_rating() const
  { return composite_rating( RATING_DAMAGE_VERSATILITY ); }
  virtual double composite_heal_versatility_rating() const
  { return composite_rating( RATING_HEAL_VERSATILITY ); }
  virtual double composite_mitigation_versatility_rating() const
  { return composite_rating( RATING_MITIGATION_VERSATILITY ); }
  virtual double composite_leech_rating() const
  { return composite_rating( RATING_LEECH ); }
  virtual double composite_speed_rating() const
  { return composite_rating( RATING_SPEED ); }
  virtual double composite_avoidance_rating() const
  { return composite_rating( RATING_AVOIDANCE ); }

  /// Total activity time for this actor during the iteration
  virtual timespan_t composite_active_time() const
  { return iteration_fight_length; }

  virtual void interrupt();
  virtual void halt();
  virtual void moving();
  virtual void finish_moving();
  virtual void stun();
  virtual void clear_debuffs();
  virtual void trigger_ready();
  virtual void schedule_ready( timespan_t delta_time = timespan_t::zero(), bool waiting = false );
  virtual void schedule_off_gcd_ready( timespan_t delta_time = timespan_t::from_millis( 100 ) );
  virtual void schedule_cwc_ready( timespan_t delta_time = timespan_t::from_millis( 100 ) );
  virtual void arise();
  virtual void demise();
  virtual timespan_t available() const
  { return timespan_t::from_seconds( 0.1 ); }
  virtual action_t* select_action( const action_priority_list_t&, execute_type type = execute_type::FOREGROUND, const action_t* context = nullptr );
  virtual action_t* execute_action();

  virtual void   regen( timespan_t periodicity = timespan_t::from_seconds( 0.25 ) );
  virtual double resource_gain( resource_e resource_type, double amount, gain_t* g = nullptr, action_t* a = nullptr );
  virtual double resource_loss( resource_e resource_type, double amount, gain_t* g = nullptr, action_t* a = nullptr );
  virtual void   recalculate_resource_max( resource_e resource_type );
  virtual bool   resource_available( resource_e resource_type, double cost ) const;
  virtual resource_e primary_resource() const
  { return RESOURCE_NONE; }
  virtual role_e   primary_role() const;
  virtual stat_e convert_hybrid_stat( stat_e s ) const
  { return s; }
  virtual stat_e normalize_by() const;
  virtual double health_percentage() const;
  virtual double max_health() const;
  virtual double current_health() const;
  virtual timespan_t time_to_percent( double percent ) const;
  virtual void cost_reduction_gain( school_e school, double amount, gain_t* g = nullptr, action_t* a = nullptr );
  virtual void cost_reduction_loss( school_e school, double amount, action_t* a = nullptr );

  virtual void assess_damage( school_e, dmg_e, action_state_t* );
  virtual void target_mitigation( school_e, dmg_e, action_state_t* );
  virtual void assess_damage_imminent_pre_absorb( school_e, dmg_e, action_state_t* );
  virtual void assess_damage_imminent( school_e, dmg_e, action_state_t* );
  virtual void do_damage( action_state_t* );
  virtual void assess_heal( school_e, dmg_e, action_state_t* );

  virtual bool taunt( player_t* /* source */ ) { return false; }

  virtual void  summon_pet( const std::string& name, timespan_t duration = timespan_t::zero() );
  virtual void dismiss_pet( const std::string& name );

  virtual expr_t* create_expression( const std::string& name );
  virtual expr_t* create_action_expression( action_t&, const std::string& name );

  virtual void create_options();
  void recreate_talent_str( talent_format_e format = TALENT_FORMAT_NUMBERS );
  virtual std::string create_profile( save_e = SAVE_ALL );

  virtual void copy_from( player_t* source );

  virtual action_t* create_action( const std::string& name, const std::string& options );
  virtual void      create_pets() { }
  virtual pet_t*    create_pet( const std::string& /* name*/,  const std::string& /* type */ = std::string() ) { return nullptr; }

  virtual void armory_extensions( const std::string& /* region */, const std::string& /* server */, const std::string& /* character */,
                                  cache::behavior_e /* behavior */ = cache::players() )
  {}

  virtual void do_dynamic_regen();

  /**
   * Returns owner if available, otherwise the player itself.
   */
  virtual const player_t* get_owner_or_self() const
  { return this; }

  player_t* get_owner_or_self()
  { return const_cast<player_t*>(static_cast<const player_t*>(this) -> get_owner_or_self()); }

  // T18 Hellfire Citadel class trinket detection
  virtual bool has_t18_class_trinket() const;

  // Targetdata stuff
  virtual actor_target_data_t* get_target_data( player_t* /* target */ ) const
  { return nullptr; }

  // Opportunity to perform any stat fixups before analysis
  virtual void pre_analyze_hook() {}

  /* New stuff */
  virtual double composite_player_vulnerability( school_e ) const;

  virtual void activate_action_list( action_priority_list_t* a, execute_type type = execute_type::FOREGROUND );

  virtual void analyze( sim_t& );

  scaling_metric_data_t scaling_for_metric( scale_metric_e metric ) const;

  virtual action_t* create_proc_action( const std::string& /* name */, const special_effect_t& /* effect */ )
  { return nullptr; }
  virtual bool requires_data_collection() const
  { return active_during_iteration; }

  rng::rng_t& rng() { return sim -> rng(); }
  rng::rng_t& rng() const { return sim -> rng(); }
  virtual timespan_t time_to_move() const;
  virtual void trigger_movement( double distance, movement_direction_e);
  virtual void update_movement( timespan_t duration );
  virtual void teleport( double yards, timespan_t duration = timespan_t::zero() );
  virtual movement_direction_e movement_direction() const
  { return current.movement_direction; }
  
  virtual void reset_auto_attacks( timespan_t delay = timespan_t::zero() );

  virtual void acquire_target( retarget_event_e /* event */, player_t* /* context */ = nullptr );

  // Various default values for the actor
  virtual std::string default_potion() const
  { return ""; }
  virtual std::string default_flask() const
  { return ""; }
  virtual std::string default_food() const
  { return ""; }
  virtual std::string default_rune() const
  { return ""; }

  /**
   * Default attack power type to use for value computation.
   *
   * Defaults to BfA "new style" base_power + mh_weapon_dps * coefficient, can be overridden in
   * class modules. Used by actions as a "last resort" to determine what attack power type drives
   * the value calculation of the ability.
   */
  virtual attack_power_e default_ap_type() const
  { return AP_DEFAULT; }

  // JSON Report extension. Overridable in class methods. Root element is an object assigned for
  // each JSON player object under "custom" property.
  virtual void output_json_report( js::JsonOutput& /* root */ ) const
  { }

private:
  std::vector<unsigned> active_dots;
public:
  void add_active_dot( unsigned action_id )
  {
    if ( active_dots.size() < action_id + 1 )
      active_dots.resize( action_id + 1 );

    active_dots[ action_id ]++;
    if ( sim -> debug )
      sim -> out_debug.printf( "%s Increasing %s dot count to %u", name(), action_map[ action_id ].c_str(), active_dots[ action_id ] );
  }

  void remove_active_dot( unsigned action_id )
  {
    assert( active_dots.size() > action_id );
    assert( active_dots[ action_id ] > 0 );

    active_dots[ action_id ]--;
    if ( sim -> debug )
      sim -> out_debug.printf( "%s Decreasing %s dot count to %u", name(), action_map[ action_id ].c_str(), active_dots[ action_id ] );
  }

  unsigned get_active_dots( unsigned action_id ) const
  {
    if ( active_dots.size() <= action_id )
      return 0;

    return active_dots[ action_id ];
  }

  virtual void adjust_dynamic_cooldowns()
  { range::for_each( dynamic_cooldown_list, []( cooldown_t* cd ) { cd -> adjust_recharge_multiplier(); } ); }
  virtual void adjust_global_cooldown( haste_type_e haste_type );
  virtual void adjust_auto_attack( haste_type_e haste_type );

  // 8.2 Vision of Perfection essence
  virtual void vision_of_perfection_proc();

private:
  void do_update_movement( double yards );
  void check_resource_callback_deactivation();
  void reset_resource_callbacks();
  void check_resource_change_for_callback(resource_e resource, double previous_amount, double previous_pct_points);
public:


  // Figure out another actor, by name. Prioritizes pets > harmful targets >
  // other players. Used by "actor.<name>" expression currently.
  virtual player_t* actor_by_name_str( const std::string& ) const;

  // Wicked resource threshold trigger-ready stuff .. work in progress
  event_t* resource_threshold_trigger;
  std::vector<double> resource_thresholds;
  void min_threshold_trigger();

  // Assessors

  // Outgoing damage assessors, pipeline is invoked on all objects passing through
  // action_t::assess_damage.
  assessor::state_assessor_pipeline_t assessor_out_damage;

  /// Start-of-combat effects
  using combat_begin_fn_t = std::function<void(player_t*)>;
  std::vector<combat_begin_fn_t> combat_begin_functions;

  /// Register a buff that triggers at the beginning of combat
  void register_combat_begin( buff_t* b );
  /// Register an action that triggers at the beginning of combat
  void register_combat_begin( action_t* a );
  /// Register a custom function that triggers at the beginning of combat
  void register_combat_begin( const combat_begin_fn_t& fn );
  /// Register a resource gain that triggers at the beginning of combat
  void register_combat_begin( double amount, resource_e resource, gain_t* g = nullptr );

  void update_off_gcd_ready();
  void update_cast_while_casting_ready();

  // Stuff, testing
  std::vector<spawner::base_actor_spawner_t*> spawners;

  spawner::base_actor_spawner_t* find_spawner( const std::string& id ) const;
  int nth_iteration() const
  {
    return creation_iteration == -1
           ? sim -> current_iteration
           : sim -> current_iteration - creation_iteration;
  }
};

std::ostream& operator<<(std::ostream &os, const player_t& p);

// Target Specific ==========================================================

template < class T >
struct target_specific_t
{
  bool owner_;
public:
  target_specific_t( bool owner = true ) : owner_( owner )
  { }

  T*& operator[](  const player_t* target ) const
  {
    assert( target );
    if ( data.size() <= target -> actor_index )
    {
      data.resize( target -> actor_index + 1 );
    }
    return data[ target -> actor_index ];
  }
  ~target_specific_t()
  {
    if ( owner_ )
      range::dispose( data );
  }
private:
  mutable std::vector<T*> data;
};

struct player_event_t : public event_t
{
  player_t* _player;
  player_event_t( player_t& p, timespan_t delta_time ) :
    event_t( p, delta_time ),
    _player( &p ){}
  player_t* p()
  { return player(); }
  const player_t* p() const
  { return player(); }
  player_t* player()
  { return _player; }
  const player_t* player() const
  { return _player; }
  virtual const char* name() const override
  { return "event_t"; }
};

/* Event which will demise the player
 * - Reason for it are that we need to finish the current action ( eg. a dot tick ) without
 * killing off target dependent things ( eg. dot state ).
 */
struct player_demise_event_t : public player_event_t
{
  player_demise_event_t( player_t& p, timespan_t delta_time = timespan_t::zero() /* Instantly kill the player */ ) :
     player_event_t( p, delta_time )
  {
    if ( sim().debug )
      sim().out_debug.printf( "New Player-Demise Event: %s", p.name() );
  }
  virtual const char* name() const override
  { return "Player-Demise"; }
  virtual void execute() override
  {
    p() -> demise();
  }
};

// Pet ======================================================================

struct pet_t : public player_t
{
  typedef player_t base_t;

  std::string full_name_str;
  player_t* const owner;
  double stamina_per_owner;
  double intellect_per_owner;
  bool summoned;
  bool dynamic;
  pet_e pet_type;
  event_t* expiration;
  timespan_t duration;
  bool affects_wod_legendary_ring;

  struct owner_coefficients_t
  {
    double armor = 1.0;
    double health = 1.0;
    double ap_from_ap = 0.0;
    double ap_from_sp = 0.0;
    double sp_from_ap = 0.0;
    double sp_from_sp = 0.0;
  } owner_coeff;

public:
  pet_t( sim_t* sim, player_t* owner, const std::string& name, bool guardian = false, bool dynamic = false );
  pet_t( sim_t* sim, player_t* owner, const std::string& name, pet_e pt, bool guardian = false, bool dynamic = false );

  void create_options() override;
  void create_buffs() override;
  void init() override;
  void init_base_stats() override;
  void init_target() override;
  void init_finished() override;
  void reset() override;
  void assess_damage( school_e, dmg_e, action_state_t* s ) override;

  virtual void summon( timespan_t duration = timespan_t::zero() );
  virtual void dismiss( bool expired = false );

  const char* name() const override { return full_name_str.c_str(); }
  const player_t* get_owner_or_self() const override
  { return owner; }

  const spell_data_t* find_pet_spell( const std::string& name );

  double composite_attribute( attribute_e attr ) const override;
  double composite_player_multiplier( school_e ) const override;
  double composite_player_target_multiplier( player_t*, school_e ) const override;

  // new pet scaling by Ghostcrawler, see http://us.battle.net/wow/en/forum/topic/5889309137?page=49#977
  // http://us.battle.net/wow/en/forum/topic/5889309137?page=58#1143

  double hit_exp() const;

  double composite_movement_speed() const override
  { return owner -> composite_movement_speed(); }

  double composite_melee_expertise( const weapon_t* ) const override
  { return hit_exp(); }
  double composite_melee_hit() const override
  { return hit_exp(); }
  double composite_spell_hit() const override
  { return hit_exp() * 2.0; }

  double pet_crit() const;

  double composite_melee_crit_chance() const override
  { return pet_crit(); }
  double composite_spell_crit_chance() const override
  { return pet_crit(); }

  double composite_melee_speed() const override
  { return owner -> cache.attack_speed(); }

  double composite_melee_haste() const override
  { return owner -> cache.attack_haste(); }

  double composite_spell_haste() const override
  { return owner -> cache.spell_haste(); }

  double composite_spell_speed() const override
  { return owner -> cache.spell_speed(); }

  double composite_bonus_armor() const override
  { return owner -> cache.bonus_armor(); }

  double composite_damage_versatility() const override
  { return owner -> cache.damage_versatility(); }

  double composite_heal_versatility() const override
  { return owner -> cache.heal_versatility(); }

  double composite_mitigation_versatility() const override
  { return owner -> cache.mitigation_versatility(); }

  double composite_melee_attack_power() const override;

  double composite_spell_power( school_e school ) const override;

  double composite_player_critical_damage_multiplier( const action_state_t* s ) const override;

  // Assuming diminishing returns are transfered to the pet as well
  double composite_dodge() const override
  { return owner -> cache.dodge(); }

  double composite_parry() const override
  { return owner -> cache.parry(); }

  // Influenced by coefficients [ 0, 1 ]
  double composite_armor() const override
  { return owner -> cache.armor() * owner_coeff.armor; }

  void init_resources( bool force ) override;
  bool requires_data_collection() const override
  { return active_during_iteration || ( dynamic && sim -> report_pets_separately == 1 ); }

  timespan_t composite_active_time() const override;

  void acquire_target( retarget_event_e /* event */, player_t* /* context */ = nullptr ) override;
};


// Gain =====================================================================

struct gain_t : private noncopyable
{
public:
  std::array<double, RESOURCE_MAX> actual, overflow, count;
  const std::string name_str;

  gain_t( const std::string& n ) :
    actual(),
    overflow(),
    count(),
    name_str( n )
  { }
  void add( resource_e rt, double amount, double overflow_ = 0.0 )
  { actual[ rt ] += amount; overflow[ rt ] += overflow_; count[ rt ]++; }
  void merge( const gain_t& other )
  {
    for ( resource_e i = RESOURCE_NONE; i < RESOURCE_MAX; i++ )
    { actual[ i ] += other.actual[ i ]; overflow[ i ] += other.overflow[ i ]; count[ i ] += other.count[ i ]; }
  }
  void analyze( size_t iterations )
  {
    for ( resource_e i = RESOURCE_NONE; i < RESOURCE_MAX; i++ )
    { actual[ i ] /= iterations; overflow[ i ] /= iterations; count[ i ] /= iterations; }
  }
  const char* name() const { return name_str.c_str(); }
};

// Stats ====================================================================

struct stats_t : private noncopyable
{
private:
  sim_t& sim;
public:
  const std::string name_str;
  player_t* player;
  stats_t* parent;
  // We should make school and type const or const-like, and either stricly define when, where and who defines the values,
  // or make sure that it is equal to the value of all it's actions.
  school_e school;
  stats_e type;

  std::vector<action_t*> action_list;
  gain_t resource_gain;
  // Flags
  bool analyzed;
  bool quiet;
  bool background;

  simple_sample_data_t num_executes, num_ticks, num_refreshes, num_direct_results, num_tick_results;
  unsigned int iteration_num_executes, iteration_num_ticks, iteration_num_refreshes;
  // Variables used both during combat and for reporting
  simple_sample_data_t total_execute_time, total_tick_time;
  timespan_t iteration_total_execute_time, iteration_total_tick_time;
  double portion_amount;
  simple_sample_data_t total_intervals;
  timespan_t last_execute;
  extended_sample_data_t actual_amount, total_amount, portion_aps, portion_apse;
  std::vector<stats_t*> children;

  struct stats_results_t
  {
  public:
    simple_sample_data_with_min_max_t actual_amount, avg_actual_amount;
    simple_sample_data_t total_amount, fight_actual_amount, fight_total_amount, overkill_pct;
    simple_sample_data_t count;
    double pct;
  private:
    int iteration_count;
    double iteration_actual_amount, iteration_total_amount;
    friend struct stats_t;
  public:

    stats_results_t();
    void analyze( double num_results );
    void merge( const stats_results_t& other );
    void datacollection_begin();
    void datacollection_end();
  };
  std::array<stats_results_t,FULLTYPE_MAX> direct_results;
  std::array<stats_results_t,RESULT_MAX> tick_results;

  // Reporting only
  std::array<double, RESOURCE_MAX> resource_portion, apr, rpe;
  double rpe_sum, compound_amount, overkill_pct;
  double aps, ape, apet, etpe, ttpt;
  timespan_t total_time;
  std::string aps_distribution_chart;

  std::string timeline_aps_chart;

  struct stats_scaling_t {
    gear_stats_t value;
    gear_stats_t error;
  };
  std::unique_ptr<stats_scaling_t> scaling;
  std::unique_ptr<sc_timeline_t> timeline_amount;

  stats_t( const std::string& name, player_t* );

  void add_child( stats_t* child );
  void consume_resource( resource_e resource_type, double resource_amount );
  full_result_e translate_result( result_e result, block_result_e block_result );
  void add_result( double act_amount, double tot_amount, dmg_e dmg_type, result_e result, block_result_e block_result, player_t* target );
  void add_execute( timespan_t time, player_t* target );
  void add_tick   ( timespan_t time, player_t* target );
  void add_refresh( player_t* target );
  void datacollection_begin();
  void datacollection_end();
  void reset();
  void analyze();
  void merge( const stats_t& other );
  const char* name() const { return name_str.c_str(); }

  bool has_direct_amount_results() const;
  bool has_tick_amount_results() const;
};

struct action_state_t : private noncopyable
{
  action_state_t* next;
  // Source action, target actor
  action_t*       action;
  player_t*       target;
  // Execution attributes
  size_t          n_targets;            // Total number of targets the execution hits.
  int             chain_target;         // The chain target number, 0 == no chain, 1 == first target, etc.
  double original_x;
  double original_y;
  // Execution results
  dmg_e           result_type;
  result_e        result;
  block_result_e  block_result;
  double          result_raw;             // Base result value, without crit/glance etc.
  double          result_total;           // Total unmitigated result, including crit bonus, glance penalty, etc.
  double          result_mitigated;       // Result after mitigation / resist. *NOTENOTENOTE* Only filled after action_t::impact() call
  double          result_absorbed;        // Result after absorption. *NOTENOTENOTE* Only filled after action_t::impact() call
  double          result_amount;          // Final (actual) result
  double          blocked_amount;         // The amount of damage reduced via block or critical block
  double          self_absorb_amount;     // The amount of damage reduced via personal absorbs such as shield_barrier.
  // Snapshotted stats during execution
  double          haste;
  double          crit_chance;
  double          target_crit_chance;
  double          attack_power;
  double          spell_power;
  // Snapshotted multipliers
  double          versatility;
  double          da_multiplier;
  double          ta_multiplier;
  double          persistent_multiplier;
  double          pet_multiplier; // Owner -> pet multiplier
  double          target_da_multiplier;
  double          target_ta_multiplier;
  // Target mitigation multipliers
  double          target_mitigation_da_multiplier;
  double          target_mitigation_ta_multiplier;
  double          target_armor;

  static void release( action_state_t*& s );
  static std::string flags_to_str( unsigned flags );

  action_state_t( action_t*, player_t* );
  virtual ~action_state_t() {}

  virtual void copy_state( const action_state_t* );
  virtual void initialize();

  virtual std::ostringstream& debug_str( std::ostringstream& debug_str );
  virtual void debug();

  virtual double composite_crit_chance() const
  { return crit_chance + target_crit_chance; }

  virtual double composite_attack_power() const
  { return attack_power; }

  virtual double composite_spell_power() const
  { return spell_power; }

  virtual double composite_versatility() const
  { return versatility; }

  virtual double composite_da_multiplier() const
  { return da_multiplier * persistent_multiplier * target_da_multiplier * versatility * pet_multiplier; }

  virtual double composite_ta_multiplier() const
  { return ta_multiplier * persistent_multiplier * target_ta_multiplier * versatility * pet_multiplier; }

  virtual double composite_target_mitigation_da_multiplier() const
  { return target_mitigation_da_multiplier; }

  virtual double composite_target_mitigation_ta_multiplier() const
  { return target_mitigation_ta_multiplier; }

  virtual double composite_target_armor() const
  { return target_armor; }

  // Inlined
  virtual proc_types proc_type() const;
  virtual proc_types2 execute_proc_type2() const;

  // Secondary proc type of the impact event (i.e., assess_damage()). Only
  // triggers the "amount" procs
  virtual proc_types2 impact_proc_type2() const
  {
    // Don't allow impact procs that do not do damage or heal anyone; they
    // should all be handled by execute_proc_type2(). Note that this is based
    // on the _total_ amount done. This is so that fully overhealed heals are
    // still alowed to proc things.
    if ( result_total <= 0 )
      return PROC2_INVALID;

    if ( result == RESULT_HIT )
      return PROC2_HIT;
    else if ( result == RESULT_CRIT )
      return PROC2_CRIT;
    else if ( result == RESULT_GLANCE )
      return PROC2_GLANCE;

    return PROC2_INVALID;
  }

  virtual proc_types2 cast_proc_type2() const;
};

// Action ===================================================================

struct action_t : private noncopyable
{
public:
  const spell_data_t* s_data;
  const spell_data_t* s_data_reporting;
  sim_t* sim;
  const action_e type;
  std::string name_str;
  player_t* const player;
  player_t* target;

  /// Item back-pointer for trinket-sourced actions so we can show proper tooltips in reports
  const item_t* item;

  /// Weapon used for this ability. If set extra weapon damage is calculated.
  weapon_t* weapon;

  /**
   * Default target is needed, otherwise there's a chance that cycle_targets
   * option will _MAJORLY_ mess up the action list for the actor, as there's no
   * guarantee cycle_targets will end up on the "initial target" when an
   * iteration ends.
   */
  player_t* default_target;

  /// What type of damage this spell does.
  school_e school;

  /// What base school components this spell has
  std::vector<school_e> base_schools;

  /// Spell id if available, 0 otherwise
  unsigned id;

  /**
   * @brief player & action-name unique id.
   *
   * Every action -- even actions without spelldata -- is given an internal_id
   */
  int internal_id;

  /// What resource does this ability use.
  resource_e resource_current;

  /// The amount of targets that an ability impacts on. -1 will hit all targets.
  int aoe;

  /// If set to true, this action will not be counted toward total amount of executes in reporting. Useful for abilities with parent/children attacks.
  bool dual;

  /// enables/disables proc callback system on the action, like trinkets, enchants, rppm.
  bool callbacks;

  /// Whether or not the spell uses the yellow attack hit table.
  bool special;

  /// Tells the sim to not perform any other actions, as the ability is channeled.
  bool channeled;

  /// mark this as a sequence_t action
  bool sequence;

  /**
   * @brief Disables/enables reporting of this action.
   *
   * When set to true, action will not show up in raid report or count towards executes.
   */
  bool quiet;

  /**
   * @brief Enables/Disables direct execution of the ability in an action list.
   *
   * Background actions cannot be executed via action list, but can be triggered by other actions.
   * Background actions do not count for executes.
   * Abilities with background = true can still be called upon by other actions,
   * example being deep wounds and how it is activated by devastate.
   */
  bool background;

  /**
   * @brief Check if action is executable between GCD's
   *
   * When set to true, will check every 100 ms to see if this action needs to be used,
   * rather than waiting until the next gcd.
   * Slows simulation down significantly.
   */
  bool use_off_gcd;

  /// True if ability should be used while casting another spell.
  bool use_while_casting;

  /// True if ability is usable while casting another spell
  bool usable_while_casting;

  /// true if channeled action does not reschedule autoattacks, used on abilities such as bladestorm.
  bool interrupt_auto_attack;

  /// Used for actions that will do awful things to the sim when a "false positive" skill roll happens.
  bool ignore_false_positive;

  /// Skill is now done per ability, with the default being set to the player option.
  double action_skill;

  /// Used with DoT Drivers, tells simc that the direct hit is actually a tick.
  bool direct_tick;

  /// Used for abilities that repeat themselves without user interaction, only used on autoattacks.
  bool repeating;

  /**
   * Simplified: Will the ability pull the boss if used.
   * Also determines whether ability can be used precombat without counting towards the 1 harmful spell limit
   */
  bool harmful;

  /**
   * @brief Whether or not this ability is a proc.
   *
   * Procs do not consume resources.
   */
  bool proc;

  /// Is the action initialized? (action_t::init ran successfully)
  bool initialized;

  /// Self explanatory.
  bool may_hit, may_miss, may_dodge, may_parry, may_glance, may_block, may_crit, tick_may_crit;

  /// Whether or not the ability/dot ticks immediately on usage.
  bool tick_zero;

  /// Whether or not the ability/dot ticks when it is first applied, but not on refresh applications
  bool tick_on_application;

  /**
   * @brief Whether or not ticks scale with haste.
   *
   * Generally only used for bleeds that do not scale with haste,
   * or with ability drivers that deal damage every x number of seconds.
   */
  bool hasted_ticks;

  /// Need to consume per tick?
  bool consume_per_tick_;

  /// Split damage evenly between targets
  bool split_aoe_damage;

  /**
   * @brief Normalize weapon speed for weapon damage calculations
   *
   * \sa weapon_t::get_normalized_speed()
   */
  bool normalize_weapon_speed;

  /// This ability leaves a ticking dot on the ground, and doesn't move when the target moves. Used with original_x and original_y
  bool ground_aoe;

  /// Round spell base damage to integer before using
  bool round_base_dmg;

  /// Used with tick_action, tells tick_action to update state on every tick.
  bool dynamic_tick_action;

  /// Type of attack power used by the ability
  attack_power_e ap_type;

  /// Did a channel action have an interrupt_immediate used to cancel it on it
  bool interrupt_immediate_occurred;

  bool hit_any_target;

  /**
   * @brief Behavior of dot.
   *
   * Acceptable inputs are DOT_CLIP, DOT_REFRESH, and DOT_EXTEND.
   */
  dot_behavior_e dot_behavior;

  /// Ability specific extra player ready delay
  timespan_t ability_lag, ability_lag_stddev;

  /// Deathknight specific, how much runic power is gained.
  double rp_gain;

  /// The minimum gcd triggered no matter the haste.
  timespan_t min_gcd;

  /// Hasted GCD stat type. One of HASTE_NONE, HASTE_ATTACK, HASTE_SPELL, SPEED_ATTACK, SPEED_SPELL
  haste_type_e gcd_haste;

  /// Length of unhasted gcd triggered when used.
  timespan_t trigger_gcd;

  /// This is how far away the target can be from the player, and still be hit or targeted.
  double range;

  /**
   * @brief AoE ability area of influence.
   *
   * Typically anything that has a radius, but not a range, is based on the players location.
   */
  double radius;

  /// Weapon AP multiplier.
  double weapon_power_mod;

  /// Attack/Spell power scaling of the ability.
  struct {
  double direct, tick;
  } attack_power_mod, spell_power_mod;

  /**
   * @brief full damage variation in %
   *
   * Amount of variation of the full raw damage (base damage + AP/SP modified).
   * Example: amount_delta = 0.1 means variation between 95% and 105%.
   * Parsed from spell data.
   */
  double amount_delta;

  /// Amount of time the ability uses to execute before modifiers.
  timespan_t base_execute_time;

  /// Amount of time the ability uses between ticks.
  timespan_t base_tick_time;

  /// Default full duration of dot.
  timespan_t dot_duration;

  // Maximum number of DoT stacks.
  int dot_max_stack;

  /// Cost of using the ability.
  std::array< double, RESOURCE_MAX > base_costs, secondary_costs;

  /// Cost of using ability per periodic effect tick.
  std::array< double, RESOURCE_MAX > base_costs_per_tick;

  /// Minimum base direct damage
  double base_dd_min;

  /// Maximum base direct damage
  double base_dd_max;

  /// Base tick damage
  double base_td;

  /// base direct damage multiplier
  double base_dd_multiplier;

  /// base tick damage multiplier
  double base_td_multiplier;

  /// base damage multiplier (direct and tick damage)
  double base_multiplier;

  double base_hit;
  double base_crit;
  double crit_multiplier;
  double crit_bonus_multiplier;
  double crit_bonus;
  double base_dd_adder;
  double base_ta_adder;

  /// Weapon damage for the ability.
  double weapon_multiplier;

  /// Damage modifier for chained/AoE, exponentially increased by number of target hit.
  double chain_multiplier;

  /// Damage modifier for chained/AoE, linearly increasing with each target hit.
  double chain_bonus_damage;

  /// Static reduction of damage for AoE
  double base_aoe_multiplier;

  /// Static action cooldown duration multiplier
  double base_recharge_multiplier;

  /// Maximum distance that the ability can travel. Used on abilities that instantly move you, or nearly instantly move you to a location.
  double base_teleport_distance;

  /// Missile travel speed in yards / second
  double travel_speed;

  // Amount of resource for the energize to grant.
  double energize_amount;

  /**
   * @brief Movement Direction
   * @code
   * movement_directionality = MOVEMENT_OMNI; // Can move in any direction, ex: Heroic Leap, Blink. Generally set movement skills to this.
   * movement_directionality = MOVEMENT_TOWARDS; // Can only be used towards enemy target. ex: Charge
   * movement_directionality = MOVEMENT_AWAY; // Can only be used away from target. Ex: ????
   * @endcode
   */
  movement_direction_e movement_directionality;

  /// This is used to cache/track spells that have a parent driver, such as most channeled/ground aoe abilities.
  dot_t* parent_dot;

  std::vector< action_t* > child_action;

  /// This action will execute every tick
  action_t* tick_action;

  /// This action will execute every execute
  action_t* execute_action;

  /// This action will execute every impact - Useful for AoE debuffs
  action_t* impact_action;

  // Gain object of the same name as the action
  gain_t* gain;

  // Sets the behavior for when this action's resource energize occur.
  action_energize_e energize_type;

  // Resource for the energize to grant.
  resource_e energize_resource;

  /**
   * @brief Used to manipulate cooldown duration and charges.
   *
   * @code
   * cooldown -> duration = timespan_t::from_seconds( 20 ); //Ability has a cooldown of 20 seconds.
   * cooldown -> charges = 3; // Ability has 3 charges.
   * @endcode
   */
  cooldown_t* cooldown, *internal_cooldown;

  /** action statistics, merged by action-name */
  stats_t* stats;

  /** Execute event (e.g., "casting" of the action) */
  event_t* execute_event;

  /** Queue delay event (for queueing cooldowned actions shortly before they execute. */
  event_t* queue_event;

  /** Last available, effectively used execute time */
  timespan_t time_to_execute;

  /** Last available, effectively used travel time */
  timespan_t time_to_travel;

  /** Last available, effectively used resource cost */
  double last_resource_cost;

  /** Last available number of targets effectively hit */
  int num_targets_hit;

  /** Marker for sample action priority list reporting */
  char marker;

  // Options
  struct options_t {
    /**
     * moving (default: -1), when different from -1, will flag the action as usable only when the players are moving
     * (moving=1) or not moving (moving=0). When left to -1, the action will be usable anytime. The players happen to
     * move either because of a "movement" raid event, or because of "start_moving" actions. Note that actions which are
     * not usable while moving do not need to be flagged with "move=0", Simulationcraft is already aware of those
     * restrictions.
     *
     */
    int moving;

    int wait_on_ready;

    int max_cycle_targets;

    int target_number;

    /** Interrupt option. If true, channeled action can get interrupted. */
    bool interrupt;

    /**
     * Chain can be used to re-cast a channeled spell at the beginning of its last tick. This has two advantages over
     * waiting for the channel to complete before re-casting: 1) the gcd finishes sooner, and 2) it avoids the roughly
     * 1/4 second delay between the end of a channel and the beginning of the next cast.
     */
    bool chain;

    int cycle_targets;

    bool cycle_players;

    bool interrupt_immediate;

    std::string if_expr_str;
    std::string target_if_str;
    std::string interrupt_if_expr_str;
    std::string early_chain_if_expr_str;
    std::string cancel_if_expr_str;
    std::string sync_str;
    std::string target_str;
    options_t();
  } option;

  bool interrupt_global;

  expr_t* if_expr;

  enum target_if_mode_e
  {
    TARGET_IF_NONE,
    TARGET_IF_FIRST,
    TARGET_IF_MIN,
    TARGET_IF_MAX
  } target_if_mode;

  expr_t* target_if_expr;
  expr_t* interrupt_if_expr;
  expr_t* early_chain_if_expr;
  expr_t* cancel_if_expr;
  action_t* sync_action;
  std::string signature_str;
  target_specific_t<dot_t> target_specific_dot;
  action_priority_list_t* action_list;

  /**
   * @brief Resource starvation tracking.
   *
   * Tracking proc triggered on resource-starved ready() calls.
   * Can be overridden by class modules for tracking purposes.
   */
  proc_t* starved_proc;
  uint_least64_t total_executions;

  /**
   * @brief Cooldown for specific APL line.
   *
   * Tied to a action_t object, and not shared by action-name,
   * this cooldown helps articifally restricting usage of a specific line
   * in the APL.
   */
  cooldown_t line_cooldown;
  const action_priority_t* signature;


  /// State of the last execute()
  action_state_t* execute_state;

  /// Optional - if defined before execute(), will be copied into execute_state
  action_state_t* pre_execute_state;

  unsigned snapshot_flags;

  unsigned update_flags;

  /**
   * Target Cache System
   * - list: contains the cached target pointers
   * - callback: unique_Ptr to callback
   * - is_valid: gets invalidated by the callback from the target list source.
   *  When the target list is requested in action_t::target_list(), it gets recalculated if
   *  flag is false, otherwise cached version is used
   */
  struct target_cache_t {
    std::vector< player_t* > list;
    bool is_valid;
    target_cache_t() : is_valid( false ) {}
  } mutable target_cache;

private:
  std::vector<std::unique_ptr<option_t>> options;
  action_state_t* state_cache;
  std::vector<travel_event_t*> travel_events;
public:
  action_t( action_e type, const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  virtual ~action_t();

  void add_child( action_t* child );

  void add_option( std::unique_ptr<option_t> new_option )
  { options.insert( options.begin(), std::move(new_option) ); }

  void   check_spec( specialization_e );

  void   check_spell( const spell_data_t* );

  /**
   * @brief Spell Data associated with the action.
   *
   * spell_data_t::nil() if no spell data is available,
   * spell_data_t::not_found if spell given was not found.
   *
   * This means that if no spell data is available/found (eg. talent not available),
   * all spell data fields will be filled with 0 and can thus be used directly
   * without checking specifically for the spell_data_t::ok()
   */
  const spell_data_t& data() const
  { return ( *s_data ); }

  // return s_data_reporting if available, otherwise fallback to s_data
  const spell_data_t& data_reporting() const
  {
    if (s_data_reporting == spell_data_t::nil())
    {
      return ( *s_data );
    }
    else
    {
      return ( *s_data_reporting );
    }
  }

  dot_t* find_dot( player_t* target ) const;

  bool is_aoe() const
  { return n_targets() == -1 || n_targets() > 0; }

  const char* name() const
  { return name_str.c_str(); }

  size_t num_travel_events() const
  { return travel_events.size(); }

  bool has_travel_events() const
  { return ! travel_events.empty(); }

  timespan_t shortest_travel_event() const;

  bool has_travel_events_for( const player_t* target ) const;

  /** Determine if the action can have a resulting damage/heal amount > 0 */
  bool has_amount_result() const
  {
    return attack_power_mod.direct > 0 || attack_power_mod.tick > 0
        || spell_power_mod.direct > 0 || spell_power_mod.tick > 0
        || (weapon && weapon_multiplier > 0);
  }

  void parse_spell_data( const spell_data_t& );

  void parse_target_str();

  void remove_travel_event( travel_event_t* e );

  void reschedule_queue_event();

  rng::rng_t& rng()
  { return sim -> rng(); }

  rng::rng_t& rng() const
  { return sim -> rng(); }

  player_t* select_target_if_target();

  // =======================
  // Const virtual functions
  // =======================

  virtual bool verify_actor_level() const;

  virtual bool verify_actor_spec() const;

  virtual bool verify_actor_weapon() const;

  virtual double cost() const;

  virtual double base_cost() const;

  virtual double cost_per_tick( resource_e ) const;

  virtual timespan_t gcd() const;

  virtual double false_positive_pct() const;

  virtual double false_negative_pct() const;

  virtual timespan_t execute_time() const
  { return base_execute_time; }

  virtual timespan_t tick_time( const action_state_t* state ) const;

  virtual timespan_t travel_time() const;

  virtual timespan_t distance_targeting_travel_time( action_state_t* s ) const;

  virtual result_e calculate_result( action_state_t* /* state */ ) const
  {
    assert( false );
    return RESULT_UNKNOWN;
  }

  virtual block_result_e calculate_block_result( action_state_t* s ) const;

  virtual double calculate_direct_amount( action_state_t* state ) const;

  virtual double calculate_tick_amount( action_state_t* state, double multiplier ) const;

  virtual double calculate_weapon_damage( double attack_power ) const;

  virtual double calculate_crit_damage_bonus( action_state_t* s ) const;

  virtual double target_armor( player_t* t ) const
  { return t -> cache.armor(); }

  virtual double recharge_multiplier( const cooldown_t& ) const
  { return base_recharge_multiplier; }

  /** Cooldown base duration for action based cooldowns. */
  virtual timespan_t cooldown_base_duration( const cooldown_t& cd ) const
  { return cd.duration; }

  virtual resource_e current_resource() const
  { return resource_current; }

  virtual int n_targets() const
  { return aoe; }

  virtual double last_tick_factor( const dot_t* d, const timespan_t& time_to_tick, const timespan_t& duration ) const;

  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const
  { return RESULT_TYPE_NONE; }

  virtual dmg_e report_amount_type( const action_state_t* state ) const
  { return state -> result_type; }

  /// Use when damage schools change during runtime.
  virtual school_e get_school() const
  { return school; }

  virtual double miss_chance( double /* hit */, player_t* /* target */ ) const
  { return 0; }

  virtual double dodge_chance( double /* expertise */, player_t* /* target */ ) const
  { return 0; }

  virtual double parry_chance( double /* expertise */, player_t* /* target */ ) const
  { return 0; }

  virtual double glance_chance( int /* delta_level */ ) const
  { return 0; }

  virtual double block_chance( action_state_t* /* state */ ) const
  { return 0; }

  virtual double crit_block_chance( action_state_t* /* state */  ) const
  { return 0; }

  virtual double total_crit_bonus( action_state_t* /* state */ ) const; // Check if we want to move this into the stateless system.

  virtual int num_targets() const;

  virtual size_t available_targets( std::vector< player_t* >& ) const;

  virtual std::vector< player_t* >& target_list() const;

  virtual player_t* find_target_by_number( int number ) const;

  virtual bool execute_targeting( action_t* action ) const;

  virtual std::vector<player_t*> targets_in_range_list( std::vector< player_t* >& tl ) const;

  virtual std::vector<player_t*>& check_distance_targeting( std::vector< player_t* >& tl ) const;

  virtual double ppm_proc_chance( double PPM ) const;

  virtual bool usable_moving() const;

  virtual timespan_t composite_dot_duration( const action_state_t* ) const;

  virtual double attack_direct_power_coefficient( const action_state_t* ) const
  { return attack_power_mod.direct; }

  virtual double amount_delta_modifier( const action_state_t* ) const
  { return amount_delta; }

  virtual double attack_tick_power_coefficient( const action_state_t* ) const
  { return attack_power_mod.tick; }

  virtual double spell_direct_power_coefficient( const action_state_t* ) const
  { return spell_power_mod.direct; }

  virtual double spell_tick_power_coefficient( const action_state_t* ) const
  { return spell_power_mod.tick; }

  virtual double base_da_min( const action_state_t* ) const
  { return base_dd_min; }

  virtual double base_da_max( const action_state_t* ) const
  { return base_dd_max; }

  virtual double base_ta( const action_state_t* ) const
  { return base_td; }

  virtual double bonus_da( const action_state_t* ) const
  { return base_dd_adder; }

  virtual double bonus_ta( const action_state_t* ) const
  { return base_ta_adder; }

  virtual double range_() const
  { return range; }

  virtual double radius_() const
  { return radius; }

  virtual double action_multiplier() const
  { return base_multiplier; }

  virtual double action_da_multiplier() const
  { return base_dd_multiplier; }

  virtual double action_ta_multiplier() const
  { return base_td_multiplier; }

  virtual double composite_hit() const
  { return base_hit; }

  virtual double composite_crit_chance() const
  { return base_crit; }

  virtual double composite_crit_chance_multiplier() const
  { return 1.0; }

  virtual double composite_crit_damage_bonus_multiplier() const
  { return crit_bonus_multiplier; }

  virtual double composite_haste() const
  { return 1.0; }

  virtual attack_power_e attack_power_type() const
  { return ap_type; }

  virtual double composite_attack_power() const
  { return player -> composite_melee_attack_power( attack_power_type() ); }

  virtual double composite_spell_power() const
  {
    double spell_power = 0;
    double tmp;

    for ( auto base_school : base_schools )
    {
      tmp = player -> cache.spell_power( base_school );
      if ( tmp > spell_power ) spell_power = tmp;
    }

    return spell_power;
  }

  virtual double composite_target_crit_chance( player_t* target ) const
  {
    actor_target_data_t* td = player->get_owner_or_self()->get_target_data( target );

    double c = 0.0;

    if ( td )
    {
      // Essence: Blood of the Enemy Major debuff
      c += td->debuff.blood_of_the_enemy->stack_value();

      // Consumable: Potion of Focused Resolve (TODO: Does this apply to pets as well?)
      if ( !player->is_pet() )
      {
        c += td->debuff.focused_resolve->stack_value();
      }
    }

    return c;
  }

  virtual double composite_target_multiplier( player_t* target ) const
  {
    return player -> composite_player_target_multiplier( target, get_school() );
  }

  virtual double composite_target_damage_vulnerability( player_t* target ) const
  {
    double target_vulnerability = 0.0;
    double tmp;

    for ( auto base_school : base_schools )
    {
      tmp = target -> composite_player_vulnerability( base_school );
      if ( tmp > target_vulnerability ) target_vulnerability = tmp;
    }

    return target_vulnerability;
  }

  virtual double composite_versatility( const action_state_t* ) const
  { return 1.0; }

  virtual double composite_leech( const action_state_t* ) const
  { return player -> cache.leech(); }

  virtual double composite_run_speed() const
  { return player -> cache.run_speed(); }

  virtual double composite_avoidance() const
  { return player -> cache.avoidance(); }

  /// Direct amount multiplier due to debuffs on the target
  virtual double composite_target_da_multiplier( player_t* target ) const
  { return composite_target_multiplier( target ); }

  /// Tick amount multiplier due to debuffs on the target
  virtual double composite_target_ta_multiplier( player_t* target ) const
  { return composite_target_multiplier( target ); }

  virtual double composite_da_multiplier( const action_state_t* /* s */ ) const
  {
    double base_multiplier = action_multiplier();
    double direct_multiplier = action_da_multiplier();
    double player_school_multiplier = 0.0;
    double tmp;

    for ( auto base_school : base_schools )
    {
      tmp = player -> cache.player_multiplier( base_school );
      if ( tmp > player_school_multiplier ) player_school_multiplier = tmp;
    }

    return base_multiplier * direct_multiplier * player_school_multiplier *
           player -> composite_player_dd_multiplier( get_school(), this );
  }

  /// Normal ticking modifiers that are updated every tick
  virtual double composite_ta_multiplier( const action_state_t* /* s */ ) const
  {
    double base_multiplier = action_multiplier();
    double tick_multiplier = action_ta_multiplier();
    double player_school_multiplier = 0.0;
    double tmp;

    for ( auto base_school : base_schools )
    {
      tmp = player -> cache.player_multiplier( base_school );
      if ( tmp > player_school_multiplier ) player_school_multiplier = tmp;
    }

    return base_multiplier * tick_multiplier * player_school_multiplier *
           player -> composite_player_td_multiplier( get_school(), this );
  }

  /// Persistent modifiers that are snapshot at the start of the spell cast
  virtual double composite_persistent_multiplier( const action_state_t* ) const
  { return player -> composite_persistent_multiplier( get_school() ); }

  /**
   * @brief Generic aoe multiplier for the action.
   *
   * Used in action_t::calculate_direct_amount, and applied after
   * other (passive) aoe multipliers are applied.
   */
  virtual double composite_aoe_multiplier( const action_state_t* ) const
  { return 1.0; }

  virtual double composite_target_mitigation( player_t* t, school_e s ) const
  { return t -> composite_mitigation_multiplier( s ); }

  virtual double composite_player_critical_multiplier( const action_state_t* s ) const
  { return player -> composite_player_critical_damage_multiplier( s ); }

  /// Action proc type, needed for dynamic aoe stuff and such.
  virtual proc_types proc_type() const
  { return PROC1_INVALID; }

  virtual bool has_movement_directionality() const;

  virtual double composite_teleport_distance( const action_state_t* ) const
  { return base_teleport_distance; }

  virtual timespan_t calculate_dot_refresh_duration(const dot_t*,
      timespan_t triggered_duration) const;

  // Helper for dot refresh expression, overridable on action level
  virtual bool dot_refreshable( const dot_t* dot, const timespan_t& triggered_duration ) const;

  virtual double composite_energize_amount( const action_state_t* ) const
  { return energize_amount; }

  virtual resource_e energize_resource_() const
  { return energize_resource; }

  virtual action_energize_e energize_type_() const
  { return energize_type; }

  virtual gain_t* energize_gain( const action_state_t* /* state */ ) const
  { return gain; }

  // ==========================
  // mutating virtual functions
  // ==========================

  virtual void parse_effect_data( const spelleffect_data_t& );

  virtual void parse_options( const std::string& options_str );

  virtual void consume_resource();

  virtual void execute();

  virtual void tick(dot_t* d);

  virtual void last_tick(dot_t* d);

  virtual void assess_damage(dmg_e, action_state_t* assess_state);

  virtual void record_data(action_state_t* data);

  virtual void schedule_execute(action_state_t* execute_state = nullptr);

  virtual void queue_execute( execute_type type );

  virtual void reschedule_execute(timespan_t time);

  virtual void start_gcd();

  virtual void update_ready(timespan_t cd_duration = timespan_t::min());

  /// Is the _ability_ ready based on spell characteristics
  virtual bool ready();
  /// Is the action ready, as a combination of ability characteristics and user input? Main
  /// ntry-point when selecting something to do for an actor.
  virtual bool action_ready();
  /// Select a target to execute on
  virtual bool select_target();
  /// Target readiness state checking
  virtual bool target_ready( player_t* target );

  /// Ability usable during an on-going cast
  virtual bool usable_during_current_cast() const;
  /// Ability usable during the on-going global cooldown
  virtual bool usable_during_current_gcd() const;

  virtual void init();

  virtual void init_finished();

  virtual void reset();

  virtual void cancel();

  virtual void interrupt_action();

  // Perform activation duties for the action. This is used to "enable" the action when the actor
  // becomes active.
  virtual void activate();

  virtual expr_t* create_expression(const std::string& name);

  virtual action_state_t* new_state();

  virtual action_state_t* get_state(const action_state_t* = nullptr);
private:
  friend struct action_state_t;
  virtual void release_state( action_state_t* );
public:
  virtual void do_schedule_travel( action_state_t*, const timespan_t& );

  virtual void schedule_travel( action_state_t* );

  virtual void impact( action_state_t* );

  virtual void trigger_dot( action_state_t* );

  virtual void snapshot_internal( action_state_t*, unsigned flags, dmg_e );

  virtual void snapshot_state( action_state_t* s, dmg_e rt )
  { snapshot_internal( s, snapshot_flags, rt ); }

  virtual void update_state( action_state_t* s, dmg_e rt )
  { snapshot_internal( s, update_flags, rt ); }

  event_t* start_action_execute_event( timespan_t time, action_state_t* execute_state = nullptr );

  virtual bool consume_cost_per_tick( const dot_t& dot );

  virtual void do_teleport( action_state_t* );

  virtual dot_t* get_dot( player_t* = nullptr );

  virtual void acquire_target( retarget_event_e /* event */, player_t* /* context */, player_t* /* candidate_target */ );

  virtual void set_target( player_t* target );

  // ================
  // Static functions
  // ================

  static bool result_is_hit( result_e r )
  {
    return( r == RESULT_HIT        ||
            r == RESULT_CRIT       ||
            r == RESULT_GLANCE     ||
            r == RESULT_NONE       );
  }

  static bool result_is_miss( result_e r )
  {
    return( r == RESULT_MISS   ||
            r == RESULT_DODGE  ||
            r == RESULT_PARRY );
  }

  static bool result_is_block( block_result_e r )
  {
    return( r == BLOCK_RESULT_BLOCKED || r == BLOCK_RESULT_CRIT_BLOCKED );
  }
};

std::ostream& operator<<(std::ostream &os, const action_t& p);

struct call_action_list_t : public action_t
{
  action_priority_list_t* alist;

  call_action_list_t( player_t*, const std::string& );
  void execute() override
  { assert( 0 ); }
};

struct swap_action_list_t : public action_t
{
  action_priority_list_t* alist;

  swap_action_list_t( player_t* player, const std::string& options_str,
    const std::string& name = "swap_action_list" );

  void execute() override;
  bool ready() override;
};

struct run_action_list_t : public swap_action_list_t
{
  run_action_list_t( player_t* player, const std::string& options_str );

  void execute() override;
};

// Attack ===================================================================

struct attack_t : public action_t
{
  double base_attack_expertise;

  attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Attack Overrides
  virtual timespan_t execute_time() const override;
  virtual void execute() override;
  virtual result_e calculate_result( action_state_t* ) const override;
  virtual void   init() override;

  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const override;
  virtual dmg_e report_amount_type( const action_state_t* /* state */ ) const override;

  virtual double  miss_chance( double hit, player_t* t ) const override;
  virtual double  dodge_chance( double /* expertise */, player_t* t ) const override;
  virtual double  block_chance( action_state_t* s ) const override;
  virtual double  crit_block_chance( action_state_t* s ) const override;

  virtual double action_multiplier() const override
  {
    double mul = action_t::action_multiplier();

    if ( ! special )
    {
      mul *= player -> auto_attack_multiplier;
    }

    return mul;
  }

  virtual double composite_target_multiplier( player_t* target ) const override
  {
    double mul = action_t::composite_target_multiplier( target );

    mul *= composite_target_damage_vulnerability( target );

    return mul;
  }

  virtual double composite_hit() const override
  { return action_t::composite_hit() + player -> cache.attack_hit(); }

  virtual double composite_crit_chance() const override
  { return action_t::composite_crit_chance() + player -> cache.attack_crit_chance(); }

  virtual double composite_crit_chance_multiplier() const override
  { return action_t::composite_crit_chance_multiplier() * player -> composite_melee_crit_chance_multiplier(); }

  virtual double composite_haste() const override
  { return action_t::composite_haste() * player -> cache.attack_haste(); }

  virtual double composite_expertise() const
  { return base_attack_expertise + player -> cache.attack_expertise(); }

  virtual double composite_versatility( const action_state_t* state ) const override
  { return action_t::composite_versatility( state ) + player -> cache.damage_versatility(); }

  double recharge_multiplier( const cooldown_t& cd ) const override
  {
    double m = action_t::recharge_multiplier( cd );

    if ( cd.hasted )
    {
      m *= player->cache.attack_haste();
    }

    return m;
  }

  virtual void reschedule_auto_attack( double old_swing_haste );

  virtual void reset() override
  {
    attack_table.reset();
    action_t::reset();
  }

private:
  /// attack table generator with caching
  struct attack_table_t{
    std::array<double, RESULT_MAX> chances;
    std::array<result_e, RESULT_MAX> results;
    int num_results;
    double attack_table_sum; // Used to check whether we can use cached values or not.

    attack_table_t()
    {reset(); }

    void reset()
    { attack_table_sum = std::numeric_limits<double>::min(); }

    void build_table( double miss_chance, double dodge_chance,
                      double parry_chance, double glance_chance,
                      double crit_chance, sim_t* );
  };
  mutable attack_table_t attack_table;


};

// Melee Attack ===================================================================

struct melee_attack_t : public attack_t
{
  melee_attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Melee Attack Overrides
  virtual void init() override;
  virtual double parry_chance( double /* expertise */, player_t* t ) const override;
  virtual double glance_chance( int delta_level ) const override;

  virtual proc_types proc_type() const override;
};

// Ranged Attack ===================================================================

struct ranged_attack_t : public attack_t
{
  ranged_attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Ranged Attack Overrides
  virtual double composite_target_multiplier( player_t* ) const override;
  virtual void schedule_execute( action_state_t* execute_state = nullptr ) override;

  virtual proc_types proc_type() const override;
};

// Spell Base ====================================================================

struct spell_base_t : public action_t
{
  // special item flags
  bool procs_courageous_primal_diamond;

  spell_base_t( action_e at, const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Spell Base Overrides
  virtual timespan_t execute_time() const override;
  virtual result_e   calculate_result( action_state_t* ) const override;
  virtual void   execute() override;
  virtual void   schedule_execute( action_state_t* execute_state = nullptr ) override;

  virtual double composite_crit_chance() const override
  { return action_t::composite_crit_chance() + player -> cache.spell_crit_chance(); }

  virtual double composite_haste() const override
  { return action_t::composite_haste() * player -> cache.spell_speed(); }

  virtual double composite_crit_chance_multiplier() const override
  { return action_t::composite_crit_chance_multiplier() * player -> composite_spell_crit_chance_multiplier(); }

  double recharge_multiplier( const cooldown_t& cd ) const override
  {
    double m = action_t::recharge_multiplier( cd );

    if ( cd.hasted )
    {
      m *= player->cache.spell_haste();
    }

    return m;
  }

  virtual proc_types proc_type() const override;
};

// Harmful Spell ====================================================================

struct spell_t : public spell_base_t
{
public:
  spell_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Harmful Spell Overrides
  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const override;
  virtual dmg_e report_amount_type( const action_state_t* /* state */ ) const override;
  virtual double miss_chance( double hit, player_t* t ) const override;
  virtual void   init() override;
  virtual double composite_hit() const override
  { return action_t::composite_hit() + player -> cache.spell_hit(); }
  virtual double composite_versatility( const action_state_t* state ) const override
  { return spell_base_t::composite_versatility( state ) + player -> cache.damage_versatility(); }

  virtual double composite_target_multiplier( player_t* target ) const override
  {
    double mul = action_t::composite_target_multiplier( target );

    mul *= composite_target_damage_vulnerability( target );

    return mul;
  }

};

// Heal =====================================================================

struct heal_t : public spell_base_t
{
public:
  typedef spell_base_t base_t;
  bool group_only;
  double base_pct_heal;
  double tick_pct_heal;
  gain_t* heal_gain;

  heal_t( const std::string& name, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  virtual double composite_pct_heal( const action_state_t* ) const;
  virtual void assess_damage( dmg_e, action_state_t* ) override;
  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const override;
  virtual dmg_e report_amount_type( const action_state_t* /* state */ ) const override;
  virtual size_t available_targets( std::vector< player_t* >& ) const override;
  void activate() override;
  virtual double calculate_direct_amount( action_state_t* state ) const override;
  virtual double calculate_tick_amount( action_state_t* state, double dmg_multiplier ) const override;
  player_t* find_greatest_difference_player();
  player_t* find_lowest_player();
  std::vector < player_t* > find_lowest_players( int num_players ) const;
  player_t* smart_target() const; // Find random injured healing target, preferring non-pets // Might need to move up hierarchy if there are smart absorbs
  virtual int num_targets() const override;
  void   parse_effect_data( const spelleffect_data_t& ) override;

  virtual double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = action_multiplier() * action_da_multiplier() *
           player -> cache.player_heal_multiplier( s ) *
           player -> composite_player_dh_multiplier( get_school() );

    return m;
  }

  virtual double composite_ta_multiplier( const action_state_t* s ) const override
  {
    double m = action_multiplier() * action_ta_multiplier() *
           player -> cache.player_heal_multiplier( s ) *
           player -> composite_player_th_multiplier( get_school() );

    return m;
  }

  virtual double composite_player_critical_multiplier( const action_state_t* /* s */ ) const override
  { return player -> composite_player_critical_healing_multiplier(); }

  virtual double composite_versatility( const action_state_t* state ) const override
  { return spell_base_t::composite_versatility( state ) + player -> cache.heal_versatility(); }

  virtual expr_t* create_expression( const std::string& name ) override;
};

// Absorb ===================================================================

struct absorb_t : public spell_base_t
{
  target_specific_t<absorb_buff_t> target_specific;

  absorb_t( const std::string& name, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Allows customization of the absorb_buff_t that we are creating.
  virtual absorb_buff_t* create_buff( const action_state_t* s )
  {
    buff_t* b = buff_t::find( s -> target, name_str );
    if ( b )
      return debug_cast<absorb_buff_t*>( b );

    std::string stats_obj_name = name_str;
    if ( s -> target != player )
      stats_obj_name += "_" + player -> name_str;
    stats_t* stats_obj = player -> get_stats( stats_obj_name, this );
    if ( stats != stats_obj )
    {
      // Add absorb target stats as a child to the main stats object for reporting
      stats -> add_child( stats_obj );
    }
    auto buff = make_buff<absorb_buff_t>( s -> target, name_str, &data() );
    buff->set_absorb_source( stats_obj );

    return buff;
  }

  virtual void assess_damage( dmg_e, action_state_t* ) override;
  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const override
  { return ABSORB; }
  virtual void impact( action_state_t* ) override;
  virtual void activate() override;
  virtual size_t available_targets( std::vector< player_t* >& ) const override;
  virtual int num_targets() const override;

  virtual double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = action_multiplier() * action_da_multiplier() *
           player -> composite_player_absorb_multiplier( s );

    return m;
  }
  virtual double composite_ta_multiplier( const action_state_t* s ) const override
  {
    double m = action_multiplier() * action_ta_multiplier() *
           player -> composite_player_absorb_multiplier( s );

    return m;
  }
  virtual double composite_versatility( const action_state_t* state ) const override
  { return spell_base_t::composite_versatility( state ) + player -> cache.heal_versatility(); }
};

// Sequence =================================================================

struct sequence_t : public action_t
{
  bool waiting;
  int sequence_wait_on_ready;
  std::vector<action_t*> sub_actions;
  int current_action;
  bool restarted;
  timespan_t last_restart;

  sequence_t( player_t*, const std::string& sub_action_str );

  virtual void schedule_execute( action_state_t* execute_state = nullptr ) override;
  virtual void reset() override;
  virtual bool ready() override;
  void restart() { current_action = 0; restarted = true; last_restart = sim -> current_time(); }
  bool can_restart()
  { return ! restarted && last_restart < sim -> current_time(); }
};

struct strict_sequence_t : public action_t
{
  size_t current_action;
  std::vector<action_t*> sub_actions;
  std::string seq_name_str;

  // Allow strict sequence sub-actions to be skipped if they are not ready. Default = false.
  bool allow_skip;

  strict_sequence_t( player_t*, const std::string& opts );

  bool ready() override;
  void reset() override;
  void cancel() override;
  void interrupt_action() override;
  void schedule_execute( action_state_t* execute_state = nullptr ) override;
};

// Primary proc type of the result (direct (aoe) damage/heal, periodic
// damage/heal)
inline proc_types action_state_t::proc_type() const
{
  if ( result_type == DMG_DIRECT || result_type == HEAL_DIRECT )
    return action -> proc_type();
  else if ( result_type == DMG_OVER_TIME )
    return PROC1_PERIODIC;
  else if ( result_type == HEAL_OVER_TIME )
    return PROC1_PERIODIC_HEAL;

  return PROC1_INVALID;
}

// Secondary proc type of the "finished casting" (i.e., execute()). Only
// triggers the "landing", dodge, parry, and miss procs
inline proc_types2 action_state_t::execute_proc_type2() const
{
  if ( result == RESULT_DODGE )
    return PROC2_DODGE;
  else if ( result == RESULT_PARRY )
    return PROC2_PARRY;
  else if ( result == RESULT_MISS )
    return PROC2_MISS;
  // Bunch up all non-damaging harmful attacks that land into "hit"
  else if ( action -> harmful )
    return PROC2_LANDED;

  return PROC2_INVALID;
}

inline proc_types2 action_state_t::cast_proc_type2() const
{
  // Only foreground actions may trigger the "on cast" procs
  if ( action -> background )
  {
    return PROC2_INVALID;
  }

  if ( action -> attack_direct_power_coefficient( this ) ||
       action -> attack_tick_power_coefficient( this ) ||
       action -> spell_direct_power_coefficient( this ) ||
       action -> spell_tick_power_coefficient( this ) ||
       action -> base_ta( this ) || action -> base_da_min( this ) ||
       action -> bonus_ta( this ) || action -> bonus_da( this ) )
  {
    // This is somewhat naive way to differentiate, better way would be to classify based on the
    // actual proc types, but it will serve our purposes for now.
    return action -> harmful ? PROC2_CAST_DAMAGE : PROC2_CAST_HEAL;
  }

  // Generic fallback "on any cast"
  return PROC2_CAST;
}

// DoT ======================================================================

// DoT Tick Event ===========================================================

struct dot_tick_event_t : public event_t
{
public:
  dot_tick_event_t( dot_t* d, timespan_t time_to_tick );

private:
  virtual void execute() override;
  virtual const char* name() const override
  { return "Dot Tick"; }
  dot_t* dot;
};

// DoT End Event ===========================================================

struct dot_end_event_t : public event_t
{
public:
  dot_end_event_t( dot_t* d, timespan_t time_to_end );

private:
  virtual void execute() override;
  virtual const char* name() const override
  { return "DoT End"; }
  dot_t* dot;
};

struct dot_t : private noncopyable
{
private:
  sim_t& sim;
  bool ticking;
  timespan_t current_duration;
  timespan_t last_start;
  timespan_t prev_tick_time; // Used for rescheduling when refreshing right before last partial tick
  timespan_t extended_time; // Added time per extend_duration for the current dot application
  timespan_t reduced_time; // Removed time per reduce_duration for the current dot application
  int stack;
public:
  event_t* tick_event;
  event_t* end_event;
  double last_tick_factor;

  player_t* const target;
  player_t* const source;
  action_t* current_action;
  action_state_t* state;
  int num_ticks, current_tick;
  int max_stack;
  timespan_t miss_time;
  timespan_t time_to_tick;
  std::string name_str;

  dot_t( const std::string& n, player_t* target, player_t* source );

  void   extend_duration( timespan_t extra_seconds, timespan_t max_total_time = timespan_t::min(), uint32_t state_flags = -1 );
  void   extend_duration( timespan_t extra_seconds, uint32_t state_flags )
  { extend_duration( extra_seconds, timespan_t::min(), state_flags ); }
  void   reduce_duration( timespan_t remove_seconds, uint32_t state_flags = -1 );
  void   refresh_duration( uint32_t state_flags = -1 );
  void   reset();
  void   cancel();
  void   trigger( timespan_t duration );
  void   decrement( int stacks );
  void   increment(int stacks);
  void   copy( player_t* destination, dot_copy_e = DOT_COPY_START ) const;
  void   copy( dot_t* dest_dot ) const;
  // Scale on-going dot remaining time by a coefficient during a tick. Note that this should be
  // accompanied with the correct (time related) scaling information in the action's supporting
  // methods (action_t::tick_time, action_t::composite_dot_ruration), otherwise bad things will
  // happen.
  void   adjust( double coefficient );
  // Alternative to adjust() based on the rogue ability Exsanguinate and how it works with hasted bleeds.
  // It rounds the number of ticks to the nearest integer and reschedules the remaining ticks
  // as full ticks from the end. If one tick would theoretically occur before Exsanguinate, it will
  // happen immediately instead.
  // Note that this should be accompanied with the correct (time related) scaling information in the
  // action's supporting methods (action_t::tick_time, action_t::composite_dot_ruration), otherwise
  // bad things will happen.
  void   adjust_full_ticks( double coefficient );
  static expr_t* create_expression( dot_t* dot, action_t* action, action_t* source_action,
      const std::string& name_str, bool dynamic );

  timespan_t remains() const;
  timespan_t time_to_next_tick() const;
  timespan_t duration() const
  { return ! is_ticking() ? timespan_t::zero() : current_duration; }
  int    ticks_left() const;
  const char* name() const
  { return name_str.c_str(); }
  bool is_ticking() const
  { return ticking; }
  timespan_t get_extended_time() const
  { return extended_time; }
  double get_last_tick_factor() const
  { return last_tick_factor; }
  int current_stack() const
  { return ticking ? stack : 0; }

  void tick();
  void last_tick();
  bool channel_interrupt();

private:
  void tick_zero();
  void schedule_tick();
  void start( timespan_t duration );
  void refresh( timespan_t duration );
  void check_tick_zero( bool start );
  bool is_higher_priority_action_available() const;
  void recalculate_num_ticks();

  friend struct dot_tick_event_t;
  friend struct dot_end_event_t;
};

inline double action_t::last_tick_factor( const dot_t* /* d */, const timespan_t& time_to_tick, const timespan_t& duration ) const
{ return std::min( 1.0, duration / time_to_tick ); }

inline dot_tick_event_t::dot_tick_event_t( dot_t* d, timespan_t time_to_tick ) :
    event_t( *d -> source, time_to_tick ),
  dot( d )
{
  if ( sim().debug )
    sim().out_debug.printf( "New DoT Tick Event: %s %s %d-of-%d %.4f",
                d -> source -> name(), dot -> name(), dot -> current_tick + 1, dot -> num_ticks, time_to_tick.total_seconds() );
}


inline void dot_tick_event_t::execute()
{
  dot -> tick_event = nullptr;
  dot -> current_tick++;

  if ( dot -> current_action -> channeled &&
       dot -> current_action -> action_skill < 1.0 &&
       dot -> remains() >= dot -> current_action -> tick_time( dot -> state ) )
  {
    if ( rng().roll( std::max( 0.0, dot -> current_action -> action_skill - dot -> current_action -> player -> current.skill_debuff ) ) )
    {
      dot -> tick();
    }
  }
  else // No skill-check required
  {
    dot -> tick();
  }

  // Some dots actually cancel themselves mid-tick. If this happens, we presume
  // that the cancel has been "proper", and just stop event execution here, as
  // the dot no longer exists.
  if ( ! dot -> is_ticking() )
    return;

  if ( ! dot -> current_action -> consume_cost_per_tick( *dot ) )
  {
    return;
  }

  if ( dot -> channel_interrupt() )
  {
    return;
  }

  // continue ticking
  dot->schedule_tick();
}

inline dot_end_event_t::dot_end_event_t( dot_t* d, timespan_t time_to_end ) :
    event_t( *d -> source, time_to_end ),
    dot( d )
{
  if ( sim().debug )
    sim().out_debug.printf( "New DoT End Event: %s %s %.3f",
                d -> source -> name(), dot -> name(), time_to_end.total_seconds() );
}

inline void dot_end_event_t::execute()
{
  dot -> end_event = nullptr;
  if ( dot -> current_tick < dot -> num_ticks )
  {
    dot -> current_tick++;
    dot -> tick();
  }
  // If for some reason the last tick has already ticked, ensure that the next tick has not
  // consumed any time yet, i.e., the last tick has occurred on the same timestamp as this end
  // event. This situation may occur in conjunction with extensive dot extension, where the last
  // rescheduling of the dot-end-event occurs between the second to last and last ticks. That will
  // in turn flip the order of the dot-tick-event and dot-end-event.
  else
  {
    assert( ! dot -> tick_event || ( dot -> tick_event && dot -> time_to_tick == dot -> tick_event -> remains() ) );
  }

  // Aand sanity check that the dot has consumed all ticks just in case./
  assert( dot -> current_tick == dot -> num_ticks );
  dot -> last_tick();
}

// Action Callback ==========================================================

struct action_callback_t : private noncopyable
{
  player_t* listener;
  bool active;
  bool allow_self_procs;
  bool allow_procs;

  action_callback_t( player_t* l, bool ap = false, bool asp = false ) :
    listener( l ), active( true ), allow_self_procs( asp ), allow_procs( ap )
  {
    assert( l );
    if ( std::find( l -> callbacks.all_callbacks.begin(), l -> callbacks.all_callbacks.end(), this )
        == l -> callbacks.all_callbacks.end() )
      l -> callbacks.all_callbacks.push_back( this );
  }
  virtual ~action_callback_t() {}
  virtual void trigger( action_t*, void* call_data ) = 0;
  virtual void reset() {}
  virtual void initialize() { }
  virtual void activate() { active = true; }
  virtual void deactivate() { active = false; }

  static void trigger( const std::vector<action_callback_t*>& v, action_t* a, void* call_data = nullptr )
  {
    if ( a && ! a -> player -> in_combat ) return;

    std::size_t size = v.size();
    for ( std::size_t i = 0; i < size; i++ )
    {
      action_callback_t* cb = v[ i ];
      if ( cb -> active )
      {
        if ( ! cb -> allow_procs && a && a -> proc ) return;
        cb -> trigger( a, call_data );
      }
    }
  }

  static void reset( const std::vector<action_callback_t*>& v )
  {
    std::size_t size = v.size();
    for ( std::size_t i = 0; i < size; i++ )
    {
      v[ i ] -> reset();
    }
  }
};

// Generic proc callback ======================================================

/**
 * DBC-driven proc callback. Extensively leans on the concept of "driver"
 * spells that blizzard uses to trigger actual proc spells in most cases. Uses
 * spell data as much as possible (through interaction with special_effect_t).
 * Intentionally vastly simplified compared to our other (older) callback
 * systems. The "complex" logic is offloaded either into special_effect_t
 * (creation of buffs/actions), special effect_t initialization (what kind of
 * special effect to create from DBC data or user given options, or when and
 * why to proc things (new DBC based proc system).
 *
 * The actual triggering logic is also vastly simplified (see execute()), as
 * the majority of procs in WoW are very simple. Custom procs can always be
 * derived off of this struct.
 */
struct dbc_proc_callback_t : public action_callback_t
{
  struct proc_event_t : public event_t
  {
    dbc_proc_callback_t* cb;
    action_t*            source_action;
    action_state_t*      source_state;

    proc_event_t( dbc_proc_callback_t* c, action_t* a, action_state_t* s ) :
      event_t( *a->sim ), cb( c ), source_action( a ),
      // Note, state has to be cloned as it's about to get recycled back into the action state cache
      source_state( s->action->get_state( s ) )
    {
      schedule( timespan_t::zero() );
    }

    ~proc_event_t()
    { action_state_t::release( source_state ); }

    const char* name() const override
    { return "dbc_proc_event"; }

    void execute() override
    { cb->execute( source_action, source_state ); }
  };

  static const item_t default_item_;

  const item_t& item;
  const special_effect_t& effect;
  cooldown_t* cooldown;

  // Proc trigger types, cached/initialized here from special_effect_t to avoid
  // needless spell data lookups in vast majority of cases
  real_ppm_t* rppm;
  double      proc_chance;
  double      ppm;

  buff_t* proc_buff;
  action_t* proc_action;
  weapon_t* weapon;

  dbc_proc_callback_t( const item_t& i, const special_effect_t& e ) :
    action_callback_t( i.player ), item( i ), effect( e ), cooldown( nullptr ),
    rppm( nullptr ), proc_chance( 0 ), ppm( 0 ),
    proc_buff( nullptr ), proc_action( nullptr ), weapon( nullptr )
  {
    assert( e.proc_flags() != 0 );
  }

  dbc_proc_callback_t( const item_t* i, const special_effect_t& e ) :
    action_callback_t( i -> player ), item( *i ), effect( e ), cooldown( nullptr ),
    rppm( nullptr ), proc_chance( 0 ), ppm( 0 ),
    proc_buff( nullptr ), proc_action( nullptr ), weapon( nullptr )
  {
    assert( e.proc_flags() != 0 );
  }

  dbc_proc_callback_t( player_t* p, const special_effect_t& e ) :
    action_callback_t( p ), item( default_item_ ), effect( e ), cooldown( nullptr ),
    rppm( nullptr ), proc_chance( 0 ), ppm( 0 ),
    proc_buff( nullptr ), proc_action( nullptr ), weapon( nullptr )
  {
    assert( e.proc_flags() != 0 );
  }

  virtual void initialize() override;

  void trigger( action_t* a, void* call_data ) override
  {
    if ( cooldown && cooldown -> down() ) return;

    // Weapon-based proc triggering differs from "old" callbacks. When used
    // (weapon_proc == true), dbc_proc_callback_t _REQUIRES_ that the action
    // has the correct weapon specified. Old style procs allowed actions
    // without any weapon to pass through.
    if ( weapon && ( ! a -> weapon || ( a -> weapon && a -> weapon != weapon ) ) )
      return;

    auto state = static_cast<action_state_t*>( call_data );

    // Don't allow procs to proc itself
    if ( proc_action && state->action && state->action->internal_id == proc_action->internal_id )
    {
      return;
    }

    // Don't allow harmful actions to proc on players
    if ( proc_action && proc_action->harmful && !state->target->is_enemy() )
    {
      return;
    }

    bool triggered = roll( a );
    if ( listener -> sim -> debug )
      listener -> sim -> out_debug.printf( "%s attempts to proc %s on %s: %d",
                                 listener -> name(),
                                 effect.to_string().c_str(),
                                 a -> name(), triggered );
    if ( triggered )
    {
      // Detach proc execution from proc triggering
      make_event<proc_event_t>( *listener->sim, this, a, state );

      if ( cooldown )
        cooldown -> start();
    }
  }

  // Determine target for the callback (action).
  virtual player_t* target( const action_state_t* state ) const
  {
    // Incoming event to the callback actor, or outgoing
    bool incoming = state -> target == listener;

    // Outgoing callbacks always target the target of the state object
    if ( ! incoming )
    {
      return state -> target;
    }

    // Incoming callbacks target either the callback actor, or the source of the incoming state.
    // Which is selected depends on the type of the callback proc action.
    //
    // Technically, this information is exposed in the client data, but simc needs a proper
    // targeting system first before we start using it.
    switch ( proc_action -> type )
    {
      // Heals are always targeted to the callback actor on incoming events
      case ACTION_ABSORB:
      case ACTION_HEAL:
        return listener;
      // The rest are targeted to the source of the callback event
      default:
        return state -> action -> player;
    }
  }

  rng::rng_t& rng() const
  { return listener -> rng(); }

private:
  bool roll( action_t* action )
  {
    if ( rppm )
      return rppm -> trigger();
    else if ( ppm > 0 )
      return rng().roll( action -> ppm_proc_chance( ppm ) );
    else if ( proc_chance > 0 )
      return rng().roll( proc_chance );

    assert( false );
    return false;
  }

protected:
  /**
   * Base rules for proc execution.
   * 1) If we proc a buff, trigger it
   * 2a) If the buff triggered and is at max stack, and we have an action,
   *     execute the action on the target of the action that triggered this
   *     proc.
   * 2b) If we have no buff, trigger the action on the target of the action
   *     that triggered this proc.
   *
   * TODO: Ticking buffs, though that'd be better served by fusing tick_buff_t
   * into buff_t.
   * TODO: Proc delay
   * TODO: Buff cooldown hackery for expressions. Is this really needed or can
   * we do it in a smarter way (better expression support?)
   */
  virtual void execute( action_t* /* a */, action_state_t* state )
  {
    if ( state->target->is_sleeping() )
    {
      return;
    }

    bool triggered = proc_buff == nullptr;
    if ( proc_buff )
      triggered = proc_buff -> trigger();

    if ( triggered && proc_action &&
         ( ! proc_buff || proc_buff -> check() == proc_buff -> max_stack() ) )
    {
      proc_action -> target = target( state );
      proc_action -> schedule_execute();

      // Decide whether to expire the buff even with 1 max stack
      if ( proc_buff && proc_buff -> max_stack() > 1 )
      {
        proc_buff -> expire();
      }
    }
  }
};

// Action Priority List =====================================================

struct action_priority_t
{
  std::string action_;
  std::string comment_;

  action_priority_t( const std::string& a, const std::string& c ) :
    action_( a ), comment_( c )
  { }

  action_priority_t* comment( const std::string& c )
  { comment_ = c; return this; }
};

struct action_priority_list_t
{
  // Internal ID of the action list, used in conjunction with the "new"
  // call_action_list action, that allows for potential infinite loops in the
  // APL.
  unsigned internal_id;
  uint64_t internal_id_mask;
  std::string name_str;
  std::string action_list_comment_str;
  std::string action_list_str;
  std::vector<action_priority_t> action_list;
  player_t* player;
  bool used;
  std::vector<action_t*> foreground_action_list;
  std::vector<action_t*> off_gcd_actions;
  std::vector<action_t*> cast_while_casting_actions;
  int random; // Used to determine how faceroll something actually is. :D
  action_priority_list_t( std::string name, player_t* p, const std::string& list_comment = std::string() ) :
    internal_id( 0 ), internal_id_mask( 0 ), name_str( name ), action_list_comment_str( list_comment ), player( p ), used( false ),
    foreground_action_list(), off_gcd_actions(), cast_while_casting_actions(), random( 0 )
  { }

  action_priority_t* add_action( const std::string& action_priority_str, const std::string& comment = std::string() );
  action_priority_t* add_action( const player_t* p, const spell_data_t* s, const std::string& action_name,
                                 const std::string& action_options = std::string(), const std::string& comment = std::string() );
  action_priority_t* add_action( const player_t* p, const std::string& name, const std::string& action_options = std::string(),
                                 const std::string& comment = std::string() );
  action_priority_t* add_talent( const player_t* p, const std::string& name, const std::string& action_options = std::string(),
                                 const std::string& comment = std::string() );
};

struct travel_event_t : public event_t
{
  action_t* action;
  action_state_t* state;
  travel_event_t( action_t* a, action_state_t* state, timespan_t time_to_travel );
  virtual ~travel_event_t() { if ( state && canceled ) action_state_t::release( state ); }
  virtual void execute() override;
  virtual const char* name() const override
  { return "Stateless Action Travel"; }
};

// Item database ============================================================

namespace item_database
{
bool     download_item(      item_t& item );
bool     initialize_item_sources( item_t& item, std::vector<std::string>& source_list );

int      random_suffix_type( item_t& item );
int      random_suffix_type( const item_data_t* );
uint32_t armor_value(        const item_t& item );
uint32_t armor_value(        const item_data_t*, const dbc_t&, unsigned item_level = 0 );
// Uses weapon's own (upgraded) ilevel to calculate the damage
uint32_t weapon_dmg_min(     item_t& item );
// Allows custom ilevel to be specified
uint32_t weapon_dmg_min(     const item_data_t*, const dbc_t&, unsigned item_level = 0 );
uint32_t weapon_dmg_max(     item_t& item );
uint32_t weapon_dmg_max(     const item_data_t*, const dbc_t&, unsigned item_level = 0 );

bool     load_item_from_data( item_t& item );

// Parse anything relating to the use of ItemSpellEnchantment.dbc. This includes
// enchants, and engineering addons.
bool     parse_item_spell_enchant( item_t& item, std::vector<stat_pair_t>& stats, special_effect_t& effect, unsigned enchant_id );

std::string stat_to_str( int stat, int stat_amount );

// Stat scaling methods for items, or item stats
double approx_scale_coefficient( unsigned current_ilevel, unsigned new_ilevel );
int scaled_stat( const item_t& item, const dbc_t& dbc, size_t idx, unsigned new_ilevel );

stat_pair_t item_enchantment_effect_stats( const item_enchantment_data_t& enchantment, int index );
stat_pair_t item_enchantment_effect_stats( player_t* player, const item_enchantment_data_t& enchantment, int index );
double item_budget( const item_t* item, unsigned max_ilevel );
double item_budget( const player_t* player, unsigned ilevel );

inline bool heroic( unsigned f ) { return ( f & RAID_TYPE_HEROIC ) == RAID_TYPE_HEROIC; }
inline bool lfr( unsigned f ) { return ( f & RAID_TYPE_LFR ) == RAID_TYPE_LFR; }
inline bool warforged( unsigned f ) { return ( f & RAID_TYPE_WARFORGED ) == RAID_TYPE_WARFORGED; }
inline bool mythic( unsigned f ) { return ( f & RAID_TYPE_MYTHIC ) == RAID_TYPE_MYTHIC; }

bool apply_item_bonus( item_t& item, const item_bonus_entry_t& entry );

double curve_point_value( dbc_t& dbc, unsigned curve_id, double point_value );
void apply_item_scaling( item_t& item, unsigned scaling_id, unsigned player_level );
double apply_combat_rating_multiplier( const item_t& item, double amount );
double apply_combat_rating_multiplier( const player_t* player, combat_rating_multiplier_type type, unsigned ilevel, double amount );
double apply_stamina_multiplier( const item_t& item, double amount );
double apply_stamina_multiplier( const player_t* player, combat_rating_multiplier_type type, unsigned ilevel, double amount );

/// Convert stat values to stat allocation values based on the item data
void convert_stat_values( item_t& item );

// Return the combat rating multiplier category for item data
combat_rating_multiplier_type item_combat_rating_type( const item_data_t* data );

struct token_t
{
  std::string full;
  std::string name;
  double value;
  std::string value_str;
};
std::vector<token_t> parse_tokens( const std::string& encoded_str );

bool has_item_bonus_type( const item_t& item, item_bonus_type bonus_type );
}

// Procs ====================================================================

namespace special_effect
{
  void parse_special_effect_encoding( special_effect_t& effect, const std::string& str );
  bool usable_proc( const special_effect_t& effect );
}

// Enchants =================================================================

namespace enchant
{
  struct enchant_db_item_t
  {
    const char* enchant_name;
    unsigned    enchant_id;
  };

  unsigned find_enchant_id( const std::string& name );
  std::string find_enchant_name( unsigned enchant_id );
  std::string encoded_enchant_name( const dbc_t&, const item_enchantment_data_t& );

  const item_enchantment_data_t& find_item_enchant( const item_t& item, const std::string& name );
  const item_enchantment_data_t& find_meta_gem( const dbc_t& dbc, const std::string& encoding );
  meta_gem_e meta_gem_type( const dbc_t& dbc, const item_enchantment_data_t& );
  bool passive_enchant( item_t& item, unsigned spell_id );

  void initialize_item_enchant( item_t& item, std::vector< stat_pair_t >& stats, special_effect_source_e source, const item_enchantment_data_t& enchant );
  item_socket_color initialize_gem( item_t& item, size_t gem_idx );
  item_socket_color initialize_relic( item_t& item, size_t relic_idx, const gem_property_data_t& gem_property );
}

// Unique Gear ==============================================================

namespace unique_gear
{
  typedef std::function<void(special_effect_t&)> custom_cb_t;
  typedef void(*custom_fp_t)(special_effect_t&);

  struct wrapper_callback_t : public scoped_callback_t
  {
    custom_cb_t cb;

    wrapper_callback_t( const custom_cb_t& cb_ ) :
      scoped_callback_t(), cb( cb_ )
    { }

    bool valid( const special_effect_t& ) const override
    { return true; }

    void initialize( special_effect_t& effect ) override
    { cb( effect ); }
  };

  // A scoped special effect callback that validates against a player class or specialization.
  struct class_scoped_callback_t : public scoped_callback_t
  {
    std::vector<player_e> class_;
    std::vector<specialization_e> spec_;

    class_scoped_callback_t( player_e c ) :
      scoped_callback_t( PRIORITY_CLASS ), class_( { c } )
    { }

    class_scoped_callback_t( const std::vector<player_e>& c ) :
      scoped_callback_t( PRIORITY_CLASS ), class_( c )
    { }

    class_scoped_callback_t( specialization_e s ) :
      scoped_callback_t( PRIORITY_SPEC ), spec_( { s } )
    { }

    class_scoped_callback_t( const std::vector<specialization_e>& s ) :
      scoped_callback_t( PRIORITY_SPEC ), spec_( s )
    { }

    bool valid( const special_effect_t& effect ) const override
    {
      assert( effect.player );

      if ( class_.size() > 0 && range::find( class_, effect.player -> type ) == class_.end() )
      {
        return false;
      }

      if ( spec_.size() > 0 && range::find( spec_, effect.player -> specialization() ) == spec_.end() )
      {
        return false;
      }

      return true;
    }
  };

  // A templated, class-scoped special effect initializer that automatically generates a single buff
  // on the actor.
  //
  // First template argument is the actor class (e.g., shaman_t), required parameter
  // Second template argument is the buff type (e.g., buff_t, stat_buff_t, ...)
  template <typename T, typename T_BUFF = buff_t>
  struct class_buff_cb_t : public class_scoped_callback_t
  {
    private:
    // Note, the dummy buff is assigned to from multiple threads, but since it is not meant to be
    // accessed, it does not really matter what buff ends up there.
    T_BUFF* __dummy;

    public:
    typedef class_buff_cb_t<T, T_BUFF> super;

    // The buff name. Optional if create_buff and create_fallback are both overridden. If fallback
    // behavior is required and the method is not overridden, must be provided.
    const std::string buff_name;

    class_buff_cb_t( specialization_e spec, const std::string& name = std::string() ) :
      class_scoped_callback_t( spec ), __dummy( nullptr ), buff_name( name )
    { }

    class_buff_cb_t( player_e class_, const std::string& name = std::string() ) :
      class_scoped_callback_t( class_ ), __dummy( nullptr ), buff_name( name )
    { }

    T* actor( const special_effect_t& e ) const
    { return debug_cast<T*>( e.player ); }

    // Generic initializer for the class-scoped special effect buff creator. Override to customize
    // buff creation if a simple binary fallback yes/no switch is not enough.
    void initialize( special_effect_t& effect ) override
    {
      if ( ! is_fallback( effect ) )
      {
        create_buff( effect );
      }
      else
      {
        create_fallback( effect );
      }
    }

    // Determine (from the given special_effect_t) whether to create the real buff, or the fallback
    // buff. All fallback special effects will have their source set to
    // SPECIAL_EFFECT_SOURCE_FALLBACK.
    virtual bool is_fallback( const special_effect_t& e ) const
    { return e.source == SPECIAL_EFFECT_SOURCE_FALLBACK; }

    // Create a correct type buff creator. Derived classes can override this method to fully
    // customize the buff creation.
    virtual T_BUFF* creator( const special_effect_t& e ) const
    { return new T_BUFF( e.player, buff_name ); }

    // An accessor method to return the assignment pointer for the buff (in the actor). Primary use
    // is to automatically assign fallback buffs to correct member variables. If the special effect
    // requires a fallback buff, and create_fallback is not fully overridden, must be implemented.
    virtual T_BUFF*& buff_ptr( const special_effect_t& )
    { return __dummy; }

    // Create the real buff, default implementation calls creator() to create a correct buff
    // creator, that will then instantiate the buff to the assigned member variable pointer returned
    // by buff_ptr.
    virtual void create_buff( const special_effect_t& e )
    { buff_ptr( e ) = creator( e ); }

    // Create a generic fallback buff. If the special effect requires a fallback buff, the developer
    // must also provide the following attributes, if the method is not overridden.
    // 1) A non-empty buff_name in the constructor
    // 2) Overridden buff_ptr method that returns a reference to a pointer where the buff should be
    //    assigned to.
    virtual void create_fallback( const special_effect_t& e )
    {
      assert( ! buff_name.empty() );
      // Assert here, but note that release builds will not crash, rather the buff is assigned
      // to a dummy pointer inside this initializer object
      assert( &buff_ptr( e ) != &__dummy );

      // Proc chance is hardcoded to zero, essentially disabling the buff described by creator()
      // call.
      buff_ptr( e ) = creator( e );
      buff_ptr( e )->set_chance( 0 );
      if ( e.player -> sim -> debug )
      {
        e.player -> sim -> out_debug.printf( "Player %s created fallback buff for %s",
            e.player -> name(), buff_name.c_str() );
      }
    }
  };

  // Manipulate actor somehow (using an overridden manipulate method)
  template<typename T_ACTOR>
  struct scoped_actor_callback_t : public class_scoped_callback_t
  {
    typedef scoped_actor_callback_t<T_ACTOR> super;

    scoped_actor_callback_t( player_e c ) : class_scoped_callback_t( c )
    { }

    scoped_actor_callback_t( specialization_e s ) : class_scoped_callback_t( s )
    { }

    void initialize( special_effect_t& e ) override
    { manipulate( debug_cast<T_ACTOR*>( e.player ), e ); }

    // Overridable method to manipulate the actor. Must be implemented.
    virtual void manipulate( T_ACTOR* actor, const special_effect_t& e ) = 0;
  };

  // Manipulate the action somehow (using an overridden manipulate method)
  template<typename T_ACTION = action_t>
  struct scoped_action_callback_t : public class_scoped_callback_t
  {
    typedef scoped_action_callback_t<T_ACTION> super;

    const std::string name;
    const int spell_id;

    scoped_action_callback_t( player_e c, const std::string& n ) :
      class_scoped_callback_t( c ), name( n ), spell_id( -1 )
    { }

    scoped_action_callback_t( specialization_e s, const std::string& n ) :
      class_scoped_callback_t( s ), name( n ), spell_id( -1 )
    { }

    scoped_action_callback_t( player_e c, unsigned sid ) :
      class_scoped_callback_t( c ), spell_id( sid )
    { }

    scoped_action_callback_t( specialization_e s, unsigned sid ) :
      class_scoped_callback_t( s ), spell_id( sid )
    { }

    // Initialize the callback by manipulating the action(s)
    void initialize( special_effect_t& e ) override
    {
      // "Snapshot" size's value so we don't attempt to manipulate any actions created during the loop.
      size_t size = e.player -> action_list.size();

      for ( size_t i = 0; i < size; i++ )
      {
        action_t* a = e.player -> action_list[ i ];

        if ( ( ! name.empty() && util::str_compare_ci( name, a -> name_str ) ) ||
             ( spell_id > 0 && spell_id == as<int>( a -> id ) ) )
        {
          if ( a -> sim -> debug )
          {
            e.player -> sim -> out_debug.printf( "Player %s manipulating action %s",
                e.player -> name(), a -> name() );
          }
          manipulate( debug_cast<T_ACTION*>( a ), e );
        }
      }
    }

    // Overridable method to manipulate the action. Must be implemented.
    virtual void manipulate( T_ACTION* action, const special_effect_t& e ) = 0;
  };

  // Manipulate the buff somehow (using an overridden manipulate method)
  template<typename T_BUFF = buff_t>
  struct scoped_buff_callback_t : public class_scoped_callback_t
  {
    typedef scoped_buff_callback_t<T_BUFF> super;

    const std::string name;
    const int spell_id;

    scoped_buff_callback_t( player_e c, const std::string& n ) :
      class_scoped_callback_t( c ), name( n ), spell_id( -1 )
    { }

    scoped_buff_callback_t( specialization_e s, const std::string& n ) :
      class_scoped_callback_t( s ), name( n ), spell_id( -1 )
    { }

    scoped_buff_callback_t( player_e c, unsigned sid ) :
      class_scoped_callback_t( c ), spell_id( sid )
    { }

    scoped_buff_callback_t( specialization_e s, unsigned sid ) :
      class_scoped_callback_t( s ), spell_id( sid )
    { }

    // Initialize the callback by manipulating the action(s)
    void initialize( special_effect_t& e ) override
    {
      // "Snapshot" size's value so we don't attempt to manipulate any buffs created during the loop.
      size_t size = e.player -> buff_list.size();

      for ( size_t i = 0; i < size; i++ )
      {
        buff_t* b = e.player -> buff_list[ i ];

        if ( ( ! name.empty() && util::str_compare_ci( name, b -> name_str ) ) ||
              ( spell_id > 0 && spell_id == as<int>( b -> data().id() ) ) )
        {
          if ( b -> sim -> debug )
          {
            e.player -> sim -> out_debug.printf( "Player %s manipulating buff %s",
                e.player -> name(), b -> name() );
          }
          manipulate( debug_cast<T_BUFF*>( b ), e );
        }
      }
    }

    // Overridable method to manipulate the action. Must be implemented.
    virtual void manipulate( T_BUFF* buff, const special_effect_t& e ) = 0;
  };

  struct special_effect_db_item_t
  {
    unsigned    spell_id;
    std::string encoded_options;
    scoped_callback_t* cb_obj;
    bool fallback;

    special_effect_db_item_t() :
      spell_id( 0 ), encoded_options(), cb_obj( nullptr ), fallback( false )
    { }
  };

typedef std::vector<const special_effect_db_item_t*> special_effect_set_t;

void register_hotfixes();
void register_hotfixes_legion();
void register_hotfixes_bfa();
void register_special_effects();
void register_special_effects_legion(); // Legion special effects
void register_special_effects_bfa(); // Battle for Azeroth special effects
void sort_special_effects();
void unregister_special_effects();

void add_effect( const special_effect_db_item_t& );
special_effect_set_t find_special_effect_db_item( unsigned spell_id );

// Old-style special effect registering functions
void register_special_effect( unsigned spell_id, const char* encoded_str );
void register_special_effect( unsigned spell_id, const custom_cb_t& init_cb );
void register_special_effect( unsigned spell_id, const custom_fp_t& init_cb );

// New-style special effect registering function
template <typename T>
void register_special_effect( unsigned spell_id, const T& cb, bool fallback = false )
{
  static_assert( std::is_base_of<scoped_callback_t, T>::value,
      "register_special_effect callback object must be derived from scoped_callback_t" );

  special_effect_db_item_t dbitem;
  dbitem.spell_id = spell_id;
  dbitem.cb_obj = new T( cb );
  dbitem.fallback = fallback;

  add_effect( dbitem );
}

void register_target_data_initializers( sim_t* );
void register_target_data_initializers_legion( sim_t* ); // Legion targetdata initializers
void register_target_data_initializers_bfa( sim_t* ); // Battle for Azeroth targetdata initializers

void init( player_t* );

special_effect_t* find_special_effect( player_t* actor, unsigned spell_id, special_effect_e = SPECIAL_EFFECT_NONE );

// First-phase special effect initializers
void initialize_special_effect( special_effect_t& effect, unsigned spell_id );
void initialize_special_effect_fallbacks( player_t* actor );
// Second-phase special effect initializer
void initialize_special_effect_2( special_effect_t* effect );

// Initialize special effects related to various race spells
void initialize_racial_effects( player_t* );

const item_data_t* find_consumable( const dbc_t& dbc, const std::string& name, item_subclass_consumable type );
const item_data_t* find_item_by_spell( const dbc_t& dbc, unsigned spell_id );

expr_t* create_expression( player_t& player, const std::string& name_str );

// Kludge to automatically apply all player-derived, label based modifiers to unique effects. Will
// be replaced in the future by something else.
void apply_label_modifiers( action_t* a );

// Base template for various "proc actions".
template <typename T_ACTION>
struct proc_action_t : public T_ACTION
{
  using super = T_ACTION;
  using base_action_t = proc_action_t<T_ACTION>;
  const special_effect_t* effect;

  void __initialize()
  {
    this->background = true;
    this->hasted_ticks = false;

    this->callbacks = !this->data().flags( spell_attribute::SX_DISABLE_PLAYER_PROCS );
    this->may_crit = !this->data().flags( spell_attribute::SX_CANNOT_CRIT );
    this->tick_may_crit = this->data().flags( spell_attribute::SX_TICK_MAY_CRIT );
    this->may_miss = !this->data().flags( spell_attribute::SX_ALWAYS_HIT );
    this->may_dodge = this->may_parry =
      this->may_block = !this->data().flags( spell_attribute::SX_NO_D_P_B );

    if ( this -> radius > 0 )
      this -> aoe = -1;

    bool has_dot = false;
    // Reparse effect data for any item-dependent variables.
    for ( size_t i = 1; i <= this -> data().effect_count(); i++ )
    {
      this -> parse_effect_data( this -> data().effectN( i ) );
      if ( this -> data().effectN( i ).subtype() == A_PERIODIC_DAMAGE )
      {
        has_dot = true;
      }
    }

    // Auto-infer dot max stack
    if ( has_dot && this -> data().max_stacks() > 1 )
    {
      this -> dot_max_stack = this -> data().max_stacks();
    }

    this->hasted_ticks = this -> data().flags( spell_attribute::SX_DOT_HASTED );
    this->tick_on_application = this->data().flags( spell_attribute::SX_TICK_ON_APPLICATION );

    unique_gear::apply_label_modifiers( this );
  }

  proc_action_t( const special_effect_t& e ) :
    super( e.name(), e.player, e.trigger() ),
    effect(&e)
  {
    this -> item = e.item;
    this -> cooldown = e.player -> get_cooldown( e.cooldown_name() );

    __initialize();
  }

  void init() override
  {
    super::init();
    if ( effect )
    {

      override_data( *effect );
    }
  }

  proc_action_t( const std::string& token, player_t* p, const spell_data_t* s, const item_t* i = nullptr ) :
    super( token, p, s ),
    effect(nullptr)
  {
    this -> item = i;

    __initialize();
  }

  virtual void override_data( const special_effect_t& e );
};

// Base proc spells used by the generic special effect initialization
struct proc_spell_t : public proc_action_t<spell_t>
{
  using super = proc_action_t<spell_t>;

  proc_spell_t( const special_effect_t& e ) :
    super( e )
  { }

  proc_spell_t( const std::string& token, player_t* p, const spell_data_t* s, const item_t* i = nullptr ) :
    super( token, p, s, i )
  { }
};

struct proc_heal_t : public proc_action_t<heal_t>
{
  using super = proc_action_t<heal_t>;

  proc_heal_t( const std::string& token, player_t* p, const spell_data_t* s, const item_t* i = nullptr ) :
    super( token, p, s, i )
  { }

  proc_heal_t( const special_effect_t& e ) :
    super( e )
  { }
};

struct proc_attack_t : public proc_action_t<attack_t>
{
  using super = proc_action_t<attack_t>;

  proc_attack_t( const special_effect_t& e ) :
    super( e )
  {
  }

  proc_attack_t( const std::string& token, player_t* p, const spell_data_t* s, const item_t* i = nullptr ) :
    super( token, p, s, i )
  { }

  void override_data( const special_effect_t& e ) override;
};

struct proc_resource_t : public proc_action_t<spell_t>
{
  using super = proc_action_t<spell_t>;

  gain_t* gain;
  double gain_da, gain_ta;
  resource_e gain_resource;

  // Note, not called by proc_action_t
  void __initialize()
  {
    may_miss = may_dodge = may_parry = may_block = harmful = false;
    target = player;

    for ( size_t i = 1; i <= data().effect_count(); i++ )
    {
      const spelleffect_data_t& effect = data().effectN( i );
      if ( effect.type() == E_ENERGIZE )
      {
        gain_da = effect.average( item );
        gain_resource = effect.resource_gain_type();
      }
      else if ( effect.type() == E_APPLY_AURA && effect.subtype() == A_PERIODIC_ENERGIZE )
      {
        gain_ta = effect.average( item );
        gain_resource = effect.resource_gain_type();
      }
    }

    gain = player -> get_gain( name() );
  }

  proc_resource_t( const special_effect_t& e ) :
    super( e ), gain_da( 0 ), gain_ta( 0 ), gain_resource( RESOURCE_NONE )
  {
    __initialize();
  }

  proc_resource_t( const std::string& token, player_t* p, const spell_data_t* s, const item_t* item_ = nullptr ) :
    super( token, p, s, item_ ), gain_da( 0 ), gain_ta( 0 ), gain_resource( RESOURCE_NONE )
  {
    __initialize();
  }

  result_e calculate_result( action_state_t* /* state */ ) const override
  { return RESULT_HIT; }

  void init() override
  {
    super::init();

    snapshot_flags = update_flags = 0;
  }

  void execute() override
  {
    super::execute();

    player -> resource_gain( gain_resource, gain_da, gain );
  }

  void tick( dot_t* d ) override
  {
    super::tick( d );

    player -> resource_gain( gain_resource, gain_ta, gain );
  }
};

template <typename CLASS, typename ...ARGS>
action_t* create_proc_action( const std::string& name, const special_effect_t& effect, ARGS&&... args )
{
  auto player = effect.player;
  auto a = player -> find_action( name );

  if ( a == nullptr )
  {
    a = player -> create_proc_action( name, effect );
  }

  if ( a == nullptr )
  {
    a = new CLASS( effect, args... );
  }

  return a;
}

template <typename BUFF, typename ...ARGS>
BUFF* create_buff( player_t* p, const std::string& name, ARGS&&... args )
{
  auto b = buff_t::find( p, name );
  if ( b != nullptr )
  {
    return debug_cast<BUFF*>( b );
  }

  return make_buff<BUFF>( p, name, args... );
}
} // namespace unique_gear ends

// Consumable ===============================================================

namespace consumable
{
action_t* create_action( player_t*, const std::string& name, const std::string& options );
}

// Wowhead  =================================================================

namespace wowhead
{
// 2016-07-20: Wowhead's XML output for item stats produces weird results on certain items that are
// no longer available in game. Skip very high values to let the sim run, but not use completely
// silly values.
enum
{
  WOWHEAD_STAT_MAX = 10000
};

enum wowhead_e
{
  LIVE,
  PTR,
  BETA
};

bool download_item( item_t&, wowhead_e source = LIVE, cache::behavior_e b = cache::items() );
bool download_item_data( item_t&            item,
                         cache::behavior_e  caching,
                         wowhead_e          source );

std::string domain_str( wowhead_e domain );
}


// Blizzard Community Platform API ==========================================

namespace bcp_api
{
bool download_guild( sim_t* sim,
                     const std::string& region,
                     const std::string& server,
                     const std::string& name,
                     const std::vector<int>& ranks,
                     int player_e = PLAYER_NONE,
                     int max_rank = 0,
                     cache::behavior_e b = cache::players() );

player_t* download_player( sim_t*,
                           const std::string& region,
                           const std::string& server,
                           const std::string& name,
                           const std::string& talents = std::string( "active" ),
                           cache::behavior_e b = cache::players(),
                           bool allow_failures = false );

player_t* from_local_json( sim_t*,
                           const std::string&,
                           const std::string&,
                           const std::string&
                         );

bool download_item( item_t&, cache::behavior_e b = cache::items() );
void token_load();
void token_save();

slot_e translate_api_slot( const std::string& slot_str );
}

// HTTP Download  ===========================================================

namespace http
{
struct proxy_t
{
  std::string type;
  std::string host;
  int port;
};
void set_proxy( const std::string& type, const std::string& host, const unsigned port );

void cache_load( const std::string& file_name );
void cache_save( const std::string& file_name );
bool clear_cache( sim_t*, const std::string& name, const std::string& value );

int get( std::string& result, const std::string& url,
          cache::behavior_e caching, const std::string& confirmation = "",
          const std::vector<std::string>& headers = {} );
}

// XML ======================================================================
#include "util/xml.hpp"

// Handy Actions ============================================================

struct wait_action_base_t : public action_t
{
  wait_action_base_t( player_t* player, const std::string& name ):
    action_t( ACTION_OTHER, name, player )
  {
    trigger_gcd = timespan_t::zero();
    interrupt_auto_attack = false;
  }

  virtual void execute() override
  { player -> iteration_waiting_time += time_to_execute; }
};

// Wait For Cooldown Action =================================================

struct wait_for_cooldown_t : public wait_action_base_t
{
  cooldown_t* wait_cd;
  action_t* a;
  wait_for_cooldown_t( player_t* player, const std::string& cd_name );
  virtual bool usable_moving() const override { return a -> usable_moving(); }
  virtual timespan_t execute_time() const override;
};

// Namespace for a ignite like action. Not mandatory, but encouraged to use it.
namespace residual_action
{
// Custom state for ignite-like actions. tick_amount contains the current
// ticking value of the ignite periodic effect, and is adjusted every time
// residual_periodic_action_t (the ignite spell) impacts on the target.
struct residual_periodic_state_t : public action_state_t
{
  double tick_amount;

  residual_periodic_state_t( action_t* a, player_t* t ) :
    action_state_t( a, t ),
    tick_amount( 0 )
  { }

  std::ostringstream& debug_str( std::ostringstream& s ) override
  { action_state_t::debug_str( s ) << " tick_amount=" << tick_amount; return s; }

  void initialize() override
  { action_state_t::initialize(); tick_amount = 0; }

  void copy_state( const action_state_t* o ) override
  {
    action_state_t::copy_state( o );
    const residual_periodic_state_t* dps_t = debug_cast<const residual_periodic_state_t*>( o );
    tick_amount = dps_t -> tick_amount;
  }
};

template <class Base>
struct residual_periodic_action_t : public Base
{
private:
  typedef Base ab; // typedef for the templated action type, spell_t, or heal_t
public:
  typedef residual_periodic_action_t base_t;
  typedef residual_periodic_action_t<Base> residual_action_t;

  template <typename T>
  residual_periodic_action_t( const std::string& n, T& p, const spell_data_t* s ) :
    ab( n, p, s )
  {
    initialize_();
  }

  template <typename T>
  residual_periodic_action_t( const std::string& n, T* p, const spell_data_t* s ) :
    ab( n, p, s )
  {
    initialize_();
  }

  void initialize_()
  {
    ab::background = true;

    ab::tick_may_crit = false;
    ab::hasted_ticks  = false;
    ab::may_crit = false;
    ab::attack_power_mod.tick = 0;
    ab::spell_power_mod.tick = 0;
    ab::dot_behavior  = DOT_REFRESH;
    ab::callbacks = false;
  }

  virtual action_state_t* new_state() override
  { return new residual_periodic_state_t( this, ab::target ); }

  // Residual periodic actions will not be extendeed by the pandemic mechanism,
  // thus the new maximum length of the dot is the ongoing tick plus the
  // duration of the dot.
  virtual timespan_t calculate_dot_refresh_duration( const dot_t* dot, timespan_t triggered_duration ) const override
  { return dot -> time_to_next_tick() + triggered_duration; }

  virtual void impact( action_state_t* s ) override
  {
    // Residual periodic actions + tick_zero does not work
    assert( ! ab::tick_zero );

    dot_t* dot = ab::get_dot( s -> target );
    double current_amount = 0, old_amount = 0;
    int ticks_left = 0;
    residual_periodic_state_t* dot_state = debug_cast<residual_periodic_state_t*>( dot -> state );

    // If dot is ticking get current residual pool before we overwrite it
    if ( dot -> is_ticking() )
    {
      old_amount = dot_state -> tick_amount;
      ticks_left = dot -> ticks_left();
      current_amount = old_amount * dot -> ticks_left();
    }

    // Add new amount to residual pool
    current_amount += s -> result_amount;

    // Trigger the dot, refreshing it's duration or starting it
    ab::trigger_dot( s );

    // If the dot is not ticking, dot_state will be nullptr, so get the
    // residual_periodic_state_t object from the dot again (since it will exist
    // after trigger_dot() is called)
    if ( ! dot_state )
      dot_state = debug_cast<residual_periodic_state_t*>( dot -> state );

    // Compute tick damage after refresh, so we divide by the correct number of
    // ticks
    dot_state -> tick_amount = current_amount / dot -> ticks_left();

    // Spit out debug for what we did
    if ( ab::sim -> debug )
      ab::sim -> out_debug.printf( "%s %s residual_action impact amount=%f old_total=%f old_ticks=%d old_tick=%f current_total=%f current_ticks=%d current_tick=%f",
                  ab::player -> name(), ab::name(), s -> result_amount,
                  old_amount * ticks_left, ticks_left, ticks_left > 0 ? old_amount : 0,
                  current_amount, dot -> ticks_left(), dot_state -> tick_amount );
  }

  // The damage of the tick is simply the tick_amount in the state
  virtual double base_ta( const action_state_t* s ) const override
  {
    auto dot = ab::find_dot( s -> target );
    if ( dot )
    {
      auto rd_state = debug_cast<const residual_periodic_state_t*>( dot -> state );
      return rd_state -> tick_amount;
    }
    return 0.0;
  }

  // Ensure that not travel time exists for the ignite ability. Delay is
  // handled in the trigger via a custom event
  virtual timespan_t travel_time() const override
  { return timespan_t::zero(); }

  // This object is not "executable" normally. Instead, the custom event
  // handles the triggering of ignite
  virtual void execute() override
  { assert( 0 ); }

  // Ensure that the ignite action snapshots nothing
  virtual void init() override
  {
    ab::init();

    ab::update_flags = ab::snapshot_flags = 0;
  }
};

// Trigger a residual periodic action on target t
void trigger( action_t* residual_action, player_t* t, double amount );

}  // namespace residual_action

// Expansion specific methods and helpers
namespace expansion
{
// Legion (WoW 7.0)
namespace legion
{
} // namespace legion

namespace bfa
{
// All Leyshocks-related functionality defined in sc_unique_gear_x7.cpp

// Register a simple spell id -> stat buff type mapping
void register_leyshocks_trigger( unsigned spell_id, stat_e stat_buff );

// Trigger based on a spell id on a predetermined mappings on an actor
void trigger_leyshocks_grand_compilation( unsigned spell_id, player_t* actor );
// Bypass spell mapping to trigger any of the buffs required on an actor
void trigger_leyshocks_grand_compilation( stat_e buff, player_t* actor );
}
} // namespace expansion

// Inlines ==================================================================

inline bool player_t::is_my_pet( const player_t* t ) const
{ return t -> is_pet() && t -> cast_pet() -> owner == this; }


/**
 * Perform dynamic resource regeneration
 */
inline void player_t::do_dynamic_regen()
{
  if ( sim -> current_time() == last_regen )
    return;

  regen( sim -> current_time() - last_regen );
  last_regen = sim -> current_time();

  if ( dynamic_regen_pets )
  {
    for (auto & elem : active_pets)
    {
      if ( elem -> regen_type == REGEN_DYNAMIC )
        elem -> do_dynamic_regen();
    }
  }
}

inline target_wrapper_expr_t::target_wrapper_expr_t( action_t& a, const std::string& name_str, const std::string& expr_str ) :
  expr_t( name_str ), action( a ), suffix_expr_str( expr_str )
{
  proxy_expr.resize( action.sim -> actor_list.size() + 1, nullptr );
}

inline double target_wrapper_expr_t::evaluate()
{
  assert( target() );

  size_t actor_index = target() -> actor_index;

  if ( proxy_expr[ actor_index ] == nullptr )
  {
    proxy_expr[ actor_index ] = target() -> create_expression( suffix_expr_str );
  }

  return proxy_expr[ actor_index ] -> eval();
}

inline player_t* target_wrapper_expr_t::target() const
{ return action.target; }

inline actor_target_data_t::actor_target_data_t( player_t* target, player_t* source ) :
  actor_pair_t( target, source ), debuff(), dot()
{
  for (auto & elem : source -> sim -> target_data_initializer)
  {
    elem( this );
  }
}

// Real PPM inlines

inline real_ppm_t::real_ppm_t( const std::string& name, player_t* p, const spell_data_t* data, const item_t* item ) :
  player( p ),
  name_str( name ),
  freq( data -> real_ppm() ),
  modifier( p -> dbc.real_ppm_modifier( data -> id(), player, item ? item -> item_level() : 0 ) ),
  rppm( freq * modifier ),
  last_trigger_attempt( timespan_t::zero() ),
  last_successful_trigger( timespan_t::zero() ),
  scales_with( p -> dbc.real_ppm_scale( data -> id() ) ),
  blp_state( BLP_ENABLED )
{ }

inline double real_ppm_t::proc_chance( player_t*         player,
                                       double            PPM,
                                       const timespan_t& last_trigger,
                                       const timespan_t& last_successful_proc,
                                       unsigned          scales_with,
                                       blp               blp_state )
{
  auto sim = player->sim;
  double coeff = 1.0;
  double seconds = std::min( ( sim->current_time() - last_trigger ).total_seconds(), max_interval() );

  if ( scales_with & RPPM_HASTE )
    coeff *= player->cache.rppm_haste_coeff();

  // This might technically be two separate crit values, but this should be sufficient for our
  // cases. In any case, the client data does not offer information which crit it is (attack or
  // spell).
  if ( scales_with & RPPM_CRIT )
    coeff *= player->cache.rppm_crit_coeff();

  if ( scales_with & RPPM_ATTACK_SPEED )
    coeff *= 1.0 / player -> cache.attack_speed();

  double real_ppm = PPM * coeff;
  double old_rppm_chance = real_ppm * ( seconds / 60.0 );
  double rppm_chance = 0;

  if ( blp_state == BLP_ENABLED )
  {
    // RPPM Extension added on 12. March 2013: http://us.battle.net/wow/en/blog/8953693?page=44
    // Formula see http://us.battle.net/wow/en/forum/topic/8197741003#1
    double last_success = std::min( ( sim->current_time() - last_successful_proc ).total_seconds(), max_bad_luck_prot() );
    double expected_average_proc_interval = 60.0 / real_ppm;

    rppm_chance = std::max( 1.0, 1 + ( ( last_success / expected_average_proc_interval - 1.5 ) * 3.0 ) )  * old_rppm_chance;
  }
  else
  {
    rppm_chance = old_rppm_chance;
  }

  if ( sim->debug )
  {
    sim->out_debug.print( "base={:.3f} coeff={:.3f} last_trig={:.3f} last_proc={:.3f}"
                          " scales={} blp={} chance={:.5f}%",
        PPM, coeff, last_trigger.total_seconds(), last_successful_proc.total_seconds(),
        scales_with, blp_state == BLP_ENABLED ? "enabled" : "disabled", rppm_chance * 100.0 );
  }

  return rppm_chance;
}

inline bool real_ppm_t::trigger()
{
  if ( freq <= 0 )
  {
    return false;
  }

  if ( last_trigger_attempt == player->sim->current_time() )
    return false;

  double chance = proc_chance( player, rppm, last_trigger_attempt, last_successful_trigger,
      scales_with, blp_state );
  bool success = player->rng().roll( chance );

  last_trigger_attempt = player->sim->current_time();

  if ( success )
    last_successful_trigger = player->sim->current_time();
  return success;
}

// Shuffle Proc inlines

inline bool shuffled_rng_t::trigger()
{
  if (total_entries <= 0 || success_entries <= 0)
  {
    return false;
  }

  if (total_entries_remaining <= 0)
  {
    reset(); // Re-Shuffle the "Deck"
  }

  bool result = false;
  if (success_entries_remaining > 0)
  {
    result = player->rng().roll(get_remaining_success_chance());
    if (result)
    {
      success_entries_remaining--;
    }
  }

  total_entries_remaining--;

  if (total_entries_remaining <= 0)
  {
    reset(); // Re-Shuffle the "Deck"
  }

  return result;
}

// Instant absorbs
struct instant_absorb_t
{
private:
  /* const spell_data_t* spell; */
  std::function<double( const action_state_t* )> absorb_handler;
  stats_t* absorb_stats;
  gain_t*  absorb_gain;
  player_t* player;

public:
  std::string name;

  instant_absorb_t( player_t* p, const spell_data_t* s, const std::string n,
                    std::function<double( const action_state_t* )> handler ) :
    /* spell( s ), */ absorb_handler( handler ), player( p ), name( n )
  {
    util::tokenize( name );

    absorb_stats = p -> get_stats( name );
    absorb_gain  = p -> get_gain( name );
    absorb_stats -> type = STATS_ABSORB;
    absorb_stats -> school = s -> get_school_type();
  }

  double consume( const action_state_t* s )
  {
    double amount = std::min( s -> result_amount, absorb_handler( s ) );

    if ( amount > 0 )
    {
      absorb_stats -> add_result( amount, 0, ABSORB, RESULT_HIT, BLOCK_RESULT_UNBLOCKED, player );
      absorb_stats -> add_execute( timespan_t::zero(), player );
      absorb_gain -> add( RESOURCE_HEALTH, amount, 0 );

      if ( player -> sim -> debug )
        player -> sim -> out_debug.printf( "%s %s absorbs %.2f", player -> name(), name.c_str(), amount );
    }

    return amount;
  }
};

inline timespan_t cooldown_t::queueable() const
{
  return ready - player->cooldown_tolerance();
}

inline bool cooldown_t::is_ready() const
{
  if ( up() )
  {
    return true;
  }

  // Cooldowns that are not bound to specific actions should not be considered as queueable in the
  // simulator, ever, so just return up() here. This limits the actual queueable cooldowns to
  // basically only abilities, where the user must press a button to initiate the execution. Note
  // that off gcd abilities that bypass schedule_execute (i.e., action_t::use_off_gcd is set to
  // true) will for now not use the queueing system.
  if ( ! action || ! player )
  {
    return up();
  }

  // Cooldown is not up, and is action-bound (essentially foreground action), check if it's within
  // the player's (or default) cooldown tolerance for queueing.
  return queueable() <= sim.current_time();
}

template <class T>
sim_ostream_t& sim_ostream_t::operator<< (T const& rhs)
{
  fmt::fprintf(*_raw.get_stream(), "%.3f ", sim.current_time().total_seconds());
  _raw << rhs << "\n";

  return *this;
}

template<typename Format, typename... Args>
sim_ostream_t& sim_ostream_t::printf(Format&& format, Args&& ... args)
{
  fmt::fprintf(*_raw.get_stream(), "%.3f ", sim.current_time().total_seconds());
  fmt::fprintf(*_raw.get_stream(), std::forward<Format>(format), std::forward<Args>(args)... );
  _raw << "\n";
  return *this;
}

struct ground_aoe_params_t
{
  enum hasted_with
  {
    NOTHING = 0,
    SPELL_HASTE,
    SPELL_SPEED,
    ATTACK_HASTE,
    ATTACK_SPEED
  };

  enum expiration_pulse_type
  {
    NO_EXPIRATION_PULSE = 0,
    FULL_EXPIRATION_PULSE,
    PARTIAL_EXPIRATION_PULSE
  };

  enum state_type
  {
    EVENT_STARTED = 0,  // Ground aoe event started
    EVENT_CREATED,      // A new ground_aoe_event_t object created
    EVENT_DESTRUCTED,   // A ground_aoe_Event_t object destructed
    EVENT_STOPPED       // Ground aoe event stopped
  };

  using param_cb_t = std::function<void(void)>;
  using state_cb_t = std::function<void(state_type, ground_aoe_event_t*)>;

  player_t* target_;
  double x_, y_;
  hasted_with hasted_;
  action_t* action_;
  timespan_t pulse_time_, start_time_, duration_;
  expiration_pulse_type expiration_pulse_;
  unsigned n_pulses_;
  param_cb_t expiration_cb_;
  state_cb_t state_cb_;

  ground_aoe_params_t() :
    target_( nullptr ), x_( -1 ), y_( -1 ), hasted_( NOTHING ), action_( nullptr ),
    pulse_time_( timespan_t::from_seconds( 1.0 ) ), start_time_( timespan_t::min() ),
    duration_( timespan_t::zero() ),
    expiration_pulse_( NO_EXPIRATION_PULSE ), n_pulses_( 0 ), expiration_cb_( nullptr )
  { }

  player_t* target() const { return target_; }
  double x() const { return x_; }
  double y() const { return y_; }
  hasted_with hasted() const { return hasted_; }
  action_t* action() const { return action_; }
  const timespan_t& pulse_time() const { return pulse_time_; }
  const timespan_t& start_time() const { return start_time_; }
  const timespan_t& duration() const { return duration_; }
  expiration_pulse_type expiration_pulse() const { return expiration_pulse_; }
  unsigned n_pulses() const { return n_pulses_; }
  const param_cb_t& expiration_callback() const { return expiration_cb_; }
  const state_cb_t& state_callback() const { return state_cb_; }

  ground_aoe_params_t& target( player_t* p )
  {
    target_ = p;
    if ( start_time_ == timespan_t::min() )
    {
      start_time_ = target_ -> sim -> current_time();
    }

    if ( x_ == -1 )
    {
      x_ = target_ -> x_position;
    }

    if ( y_ == -1 )
    {
      y_ = target_ -> y_position;
    }

    return *this;
  }

  ground_aoe_params_t& action( action_t* a )
  {
    action_ = a;
    if ( start_time_ == timespan_t::min() )
    {
      start_time_ = action_ -> sim -> current_time();
    }

    return *this;
  }

  ground_aoe_params_t& x( double x )
  { x_ = x; return *this; }

  ground_aoe_params_t& y( double y )
  { y_ = y; return *this; }

  ground_aoe_params_t& hasted( hasted_with state )
  { hasted_ = state; return *this; }

  ground_aoe_params_t& pulse_time( const timespan_t& t )
  { pulse_time_ = t; return *this; }

  ground_aoe_params_t& start_time( const timespan_t& t )
  { start_time_ = t; return *this; }

  ground_aoe_params_t& duration( const timespan_t& t )
  { duration_ = t; return *this; }

  ground_aoe_params_t& expiration_pulse( expiration_pulse_type state )
  { expiration_pulse_ = state; return *this; }

  ground_aoe_params_t& n_pulses( unsigned n )
  { n_pulses_ = n; return *this; }

  ground_aoe_params_t& expiration_callback( const param_cb_t& cb )
  { expiration_cb_ = cb; return *this; }

  ground_aoe_params_t& state_callback( const state_cb_t& cb )
  { state_cb_ = cb; return *this; }
};

// Delayed expiration callback for groud_aoe_event_t
struct expiration_callback_event_t : public event_t
{
  ground_aoe_params_t::param_cb_t callback;

  expiration_callback_event_t( sim_t& sim, const ground_aoe_params_t* p, const timespan_t& delay ) :
    event_t( sim, delay ), callback( p -> expiration_callback() )
  { }

  void execute() override
  { callback(); }
};

// Fake "ground aoe object" for things. Pulses until duration runs out, does not perform partial
// ticks (fits as many ticks as possible into the duration). Intended to be able to spawn multiple
// ground aoe objects, each will have separate state. Currently does not snapshot stats upon object
// creation. Parametrization through object above (ground_aoe_params_t).
struct ground_aoe_event_t : public player_event_t
{
  // Pointer needed here, as simc event system cannot fit all params into event_t
  const ground_aoe_params_t* params;
  action_state_t* pulse_state;
  unsigned current_pulse;

protected:
  template <typename Event, typename... Args>
  friend Event* make_event( sim_t& sim, Args&&... args );
  // Internal constructor to schedule next pulses, not to be used outside of the struct (or derived
  // structs)
  ground_aoe_event_t( player_t* p, const ground_aoe_params_t* param,
                      action_state_t* ps, bool immediate_pulse = false )
    : player_event_t(
          *p, immediate_pulse ? timespan_t::zero() : _pulse_time( param, p ) ),
      params( param ),
      pulse_state( ps ), current_pulse( 1 )
  {
    // Ensure we have enough information to start pulsing.
    assert( params -> target() != nullptr && "No target defined for ground_aoe_event_t" );
    assert( params -> action() != nullptr && "No action defined for ground_aoe_event_t" );
    assert( params -> pulse_time() > timespan_t::zero() &&
            "Pulse time for ground_aoe_event_t must be a positive value" );
    assert( params -> start_time() >= timespan_t::zero() &&
            "Start time for ground_aoe_event must be defined" );
    assert( ( params -> duration() > timespan_t::zero() || params -> n_pulses() > 0 ) &&
            "Duration or number of pulses for ground_aoe_event_t must be defined" );

    // Make a state object that persists for this ground aoe event throughout its lifetime
    if ( ! pulse_state )
    {
      pulse_state = params -> action() -> get_state();
      action_t* spell_ = params -> action();
      spell_ -> snapshot_state( pulse_state, spell_ -> amount_type( pulse_state ) );
    }

    if ( params -> state_callback() )
    {
      params -> state_callback()( ground_aoe_params_t::EVENT_CREATED, this );
    }
  }
public:
  // Make a copy of the parameters, and use that object until this event expires
  ground_aoe_event_t( player_t* p, const ground_aoe_params_t& param, bool immediate_pulse = false ) :
    ground_aoe_event_t( p, new ground_aoe_params_t( param ), nullptr, immediate_pulse )
  {
    if ( params -> state_callback() )
    {
      params -> state_callback()( ground_aoe_params_t::EVENT_STARTED, this );
    }
  }

  // Cleans up memory for any on-going ground aoe events when the iteration ends, or when the ground
  // aoe finishes during iteration.
  ~ground_aoe_event_t()
  {
    delete params;
    if ( pulse_state )
    {
      action_state_t::release( pulse_state );
    }
  }

  void set_current_pulse( unsigned v )
  { current_pulse = v; }

  static timespan_t _time_left( const ground_aoe_params_t* params, const player_t* p )
  { return params -> duration() - ( p -> sim -> current_time() - params -> start_time() ); }

  static timespan_t _pulse_time( const ground_aoe_params_t* params, const player_t* p, bool clamp = true )
  {
    auto tick = params -> pulse_time();
    auto time_left = _time_left( params, p );

    switch ( params -> hasted() )
    {
      case ground_aoe_params_t::SPELL_SPEED:
        tick *= p -> cache.spell_speed();
        break;
      case ground_aoe_params_t::SPELL_HASTE:
        tick *= p -> cache.spell_haste();
        break;
      case ground_aoe_params_t::ATTACK_SPEED:
        tick *= p -> cache.attack_speed();
        break;
      case ground_aoe_params_t::ATTACK_HASTE:
        tick *= p -> cache.attack_haste();
        break;
      default:
        break;
    }

    // Clamping can only occur on duration-based ground aoe events.
    if ( params -> n_pulses() == 0 && clamp && tick > time_left )
    {
      assert( params -> expiration_pulse() != ground_aoe_params_t::NO_EXPIRATION_PULSE );
      return time_left;
    }

    return tick;
  }

  timespan_t remaining_time() const
  { return _time_left( params, player() ); }

  bool may_pulse() const
  {
    if ( params -> n_pulses() > 0 )
    {
      return current_pulse < params -> n_pulses();
    }
    else
    {
      auto time_left = _time_left( params, player() );

      if ( params -> expiration_pulse() == ground_aoe_params_t::NO_EXPIRATION_PULSE )
      {
        return time_left >= pulse_time( false );
      }
      else
      {
        return time_left > timespan_t::zero();
      }
    }
  }

  virtual timespan_t pulse_time( bool clamp = true ) const
  { return _pulse_time( params, player(), clamp ); }

  virtual void schedule_event()
  {
    auto event = make_event<ground_aoe_event_t>( sim(), _player, params, pulse_state );
    // If the ground-aoe event is a pulse-based one, increase the current pulse of the newly created
    // event.
    if ( params -> n_pulses() > 0 )
    {
      event -> set_current_pulse( current_pulse + 1 );
    }
  }

  const char* name() const override
  { return "ground_aoe_event"; }

  void execute() override
  {
    action_t* spell_ = params -> action();

    if ( sim().debug )
    {
      sim().out_debug.printf( "%s %s pulse start_time=%.3f remaining_time=%.3f tick_time=%.3f",
          player() -> name(), spell_ -> name(), params -> start_time().total_seconds(),
          params -> n_pulses() > 0
            ? ( params -> n_pulses() - current_pulse ) * pulse_time().total_seconds()
            : ( params -> duration() - ( sim().current_time() - params -> start_time() ) ).total_seconds(),
          pulse_time( false ).total_seconds() );
    }

    // Manually snapshot the state so we can adjust the x and y coordinates of the snapshotted
    // object. This is relevant if sim -> distance_targeting_enabled is set, since then we need to
    // use the ground object's x, y coordinates, instead of the source actor's.
    spell_ -> update_state( pulse_state, spell_ -> amount_type( pulse_state ) );
    pulse_state -> target = params -> target();
    pulse_state -> original_x = params -> x();
    pulse_state -> original_y = params -> y();

    // Update state multipliers if expiration_pulse() is PARTIAL_PULSE, and the object is pulsing
    // for the last (partial) time. Note that pulse-based ground aoe events do not have a concept of
    // partial ticks.
    if ( params -> n_pulses() == 0 &&
         params -> expiration_pulse() == ground_aoe_params_t::PARTIAL_EXPIRATION_PULSE )
    {
      // Don't clamp the pulse time here, since we need to figure out the fractional multiplier for
      // the last pulse.
      auto time = pulse_time( false );
      auto time_left = _time_left( params, player() );
      if ( time > time_left )
      {
        double multiplier = time_left / time;
        pulse_state -> da_multiplier *= multiplier;
        pulse_state -> ta_multiplier *= multiplier;
      }
    }

    spell_ -> schedule_execute( spell_ -> get_state( pulse_state ) );

    // This event is about to be destroyed, notify callback of the event if needed
    if ( params -> state_callback() )
    {
      params -> state_callback()( ground_aoe_params_t::EVENT_DESTRUCTED, this );
    }

    // Schedule next tick, if it can fit into the duration
    if ( may_pulse() )
    {
      schedule_event();
      // Ugly hack-ish, but we want to re-use the parmas and pulse state objects while this ground
      // aoe is pulsing, so nullptr the params from this (soon to be recycled) event.
      params = nullptr;
      pulse_state = nullptr;
    }
    else
    {
      handle_expiration();

      // This event is about to be destroyed, notify callback of the event if needed
      if ( params -> state_callback() )
      {
        params -> state_callback()( ground_aoe_params_t::EVENT_STOPPED, this );
      }
    }
  }

  // Figure out how to handle expiration callback if it's defined
  void handle_expiration()
  {
    if ( ! params -> expiration_callback() )
    {
      return;
    }

    auto time_left = _time_left( params, player() );

    // Trigger immediately, since no time left. Can happen for example when ground aoe events are
    // not hasted, or when pulse-based behavior is used (instead of duration-based behavior)
    if ( time_left <= timespan_t::zero() )
    {
      params -> expiration_callback()();
    }
    // Defer until the end of the ground aoe event, even if there are no ticks left
    else
    {
      make_event<expiration_callback_event_t>( sim(), sim(), params, time_left );
    }
  }
};

// Player Callbacks

// effect_callbacks_t::register_callback =====================================

template <typename T>
inline void add_callback( std::vector<T*>& callbacks, T* cb )
{
  if ( range::find( callbacks, cb ) == callbacks.end() )
    callbacks.push_back( cb );
}

template <typename T_CB>
void effect_callbacks_t<T_CB>::add_proc_callback( proc_types type,
                                                  unsigned flags,
                                                  T_CB* cb )
{
  std::stringstream s;
  if ( sim -> debug )
    s << "Registering procs: ";

  // Setup the proc-on-X types for the proc
  for ( proc_types2 pt = PROC2_TYPE_MIN; pt < PROC2_TYPE_MAX; pt++ )
  {
    if ( ! ( flags & ( 1 << pt ) ) )
      continue;

    // Healing and ticking spells all require "an amount" on landing, so
    // automatically convert a "proc on spell landed" type to "proc on
    // hit/crit".
    if ( pt == PROC2_LANDED &&
         ( type == PROC1_PERIODIC || type == PROC1_PERIODIC_TAKEN ||
           type == PROC1_PERIODIC_HEAL || type == PROC1_PERIODIC_HEAL_TAKEN ||
           type == PROC1_NONE_HEAL || type == PROC1_MAGIC_HEAL ) )
    {
      add_callback( procs[ type ][ PROC2_HIT  ], cb );
      if ( cb -> listener -> sim -> debug )
        s << util::proc_type_string( type ) << util::proc_type2_string( PROC2_HIT ) << " ";

      add_callback( procs[ type ][ PROC2_CRIT ], cb );
      if ( cb -> listener -> sim -> debug )
        s << util::proc_type_string( type ) << util::proc_type2_string( PROC2_CRIT ) << " ";
    }
    // Do normal registration based on the existence of the flag
    else
    {
      add_callback( procs[ type ][ pt ], cb );
      if ( cb -> listener -> sim -> debug )
        s << util::proc_type_string( type ) << util::proc_type2_string( pt ) << " ";
    }
  }

  if ( sim -> debug )
    sim -> out_debug.printf( "%s", s.str().c_str() );
}

template <typename T_CB>
void effect_callbacks_t<T_CB>::register_callback( unsigned proc_flags,
                                                  unsigned proc_flags2,
                                                  T_CB* cb )
{
  // We cannot default the "what kind of abilities proc this callback" flags,
  // they need to be non-zero
  assert( proc_flags != 0 && cb != 0 );

  if ( sim -> debug )
    sim -> out_debug.printf( "Registering callback proc_flags=%#.8x proc_flags2=%#.8x",
        proc_flags, proc_flags2 );

  // Default method for proccing is "on spell landing", if no "what type of
  // result procs this callback" is given
  if ( proc_flags2 == 0 )
    proc_flags2 = PF2_LANDED;

  for ( proc_types t = PROC1_TYPE_MIN; t < PROC1_TYPE_BLIZZARD_MAX; t++ )
  {
    // If there's no proc-by-X, we don't need to add anything
    if ( ! ( proc_flags & ( 1 << t ) ) )
      continue;

    // Skip periodic procs, they are handled below as a special case
    if ( t == PROC1_PERIODIC || t == PROC1_PERIODIC_TAKEN )
      continue;

    add_proc_callback( t, proc_flags2, cb );
  }

  // Periodic X done
  if ( proc_flags & PF_PERIODIC )
  {
    // 1) Periodic damage only. This is the default behavior of our system when
    // only PROC1_PERIODIC is defined on a trinket.
    if ( ! ( proc_flags & PF_ALL_HEAL ) &&                                               // No healing ability type flags
         ! ( proc_flags2 & PF2_PERIODIC_HEAL ) )                                         // .. nor periodic healing result type flag
    {
      add_proc_callback( PROC1_PERIODIC, proc_flags2, cb );
    }
    // 2) Periodic heals only. Either inferred by a "proc by direct heals" flag,
    //    or by "proc on periodic heal ticks" flag, but require that there's
    //    no direct / ticked spell damage in flags.
    else if ( ( ( proc_flags & PF_ALL_HEAL ) || ( proc_flags2 & PF2_PERIODIC_HEAL ) ) && // Healing ability
              ! ( proc_flags & PF_ALL_DAMAGE ) &&                                        // .. with no damaging ability type flags
              ! ( proc_flags2 & PF2_PERIODIC_DAMAGE ) )                                  // .. nor periodic damage result type flag
    {
      add_proc_callback( PROC1_PERIODIC_HEAL, proc_flags2, cb );
    }
    // Both
    else
    {
      add_proc_callback( PROC1_PERIODIC, proc_flags2, cb );
      add_proc_callback( PROC1_PERIODIC_HEAL, proc_flags2, cb );
    }
  }

  // Periodic X taken
  if ( proc_flags & PF_PERIODIC_TAKEN )
  {
    // 1) Periodic damage only. This is the default behavior of our system when
    // only PROC1_PERIODIC is defined on a trinket.
    if ( ! ( proc_flags & PF_ALL_HEAL_TAKEN ) &&                                         // No healing ability type flags
         ! ( proc_flags2 & PF2_PERIODIC_HEAL ) )                                         // .. nor periodic healing result type flag
    {
      add_proc_callback( PROC1_PERIODIC_TAKEN, proc_flags2, cb );
    }
    // 2) Periodic heals only. Either inferred by a "proc by direct heals" flag,
    //    or by "proc on periodic heal ticks" flag, but require that there's
    //    no direct / ticked spell damage in flags.
    else if ( ( ( proc_flags & PF_ALL_HEAL_TAKEN ) || ( proc_flags2 & PF2_PERIODIC_HEAL ) ) && // Healing ability
              ! ( proc_flags & PF_DAMAGE_TAKEN ) &&                                        // .. with no damaging ability type flags
              ! ( proc_flags & PF_ANY_DAMAGE_TAKEN ) &&                                    // .. nor Blizzard's own "any damage taken" flag
              ! ( proc_flags2 & PF2_PERIODIC_DAMAGE ) )                                    // .. nor periodic damage result type flag
    {
      add_proc_callback( PROC1_PERIODIC_HEAL_TAKEN, proc_flags2, cb );
    }
    // Both
    else
    {
      add_proc_callback( PROC1_PERIODIC_TAKEN, proc_flags2, cb );
      add_proc_callback( PROC1_PERIODIC_HEAL_TAKEN, proc_flags2, cb );
    }
  }
}

template <typename T_CB>
void effect_callbacks_t<T_CB>::reset()
{
  T_CB::reset( all_callbacks );
}

/**
 * Targetdata initializer for items. When targetdata is constructed (due to a call to
 * player_t::get_target_data failing to find an object for the given target), all targetdata
 * initializers in the sim are invoked. Most class specific targetdata is handled by the derived
 * class-specific targetdata, however there are a couple of trinkets that require "generic"
 * targetdata support. Item targetdata initializers will create the relevant debuffs/buffs needed.
 * Note that the buff/debuff needs to be created always, since expressions for buffs/debuffs presume
 * the buff exists or the whole sim fails to init.
 *
 * See unique_gear::register_target_data_initializers() for currently supported target-based debuffs
 * in generic items.
 */
struct item_targetdata_initializer_t
{
  unsigned item_id;
  std::vector< slot_e > slots_;

  item_targetdata_initializer_t( unsigned iid, const std::vector< slot_e >& s ) :
    item_id( iid ), slots_( s )
  { }

  virtual ~item_targetdata_initializer_t() {}

  // Returns the special effect based on item id and slots to source from. Overridable if more
  // esoteric functionality is needed
  virtual const special_effect_t* find_effect( player_t* player ) const
  {
    // No need to check items on pets/enemies
    if ( player -> is_pet() || player -> is_enemy() || player -> type == HEALING_ENEMY )
    {
      return 0;
    }

    for ( size_t i = 0; i < slots_.size(); ++i )
    {
      if ( player -> items[slots_[ i ] ].parsed.data.id == item_id )
      {
        return player -> items[slots_[ i ] ].parsed.special_effects[ 0 ];
      }
    }

    return 0;
  }

  // Override to initialize the targetdata object.
  virtual void operator()( actor_target_data_t* ) const = 0;
};

namespace report
{
class buff_decorator_t : public spell_decorator_t<buff_t>
{
  using super = spell_decorator_t<buff_t>;

protected:
  std::vector<std::string> parms() const override;

public:
  buff_decorator_t( const buff_t* obj ) :
    super( obj )
  { }

  buff_decorator_t( const buff_t& obj ) :
    buff_decorator_t( &obj )
  { }

  // Buffs have pet names included in them
  std::string url_name_prefix() const override;
};

class action_decorator_t : public spell_decorator_t<action_t>
{
  using super = spell_decorator_t<action_t>;

protected:
  std::vector<std::string> parms() const override;

public:
  action_decorator_t( const action_t* obj ) :
    super( obj )
  { }

  action_decorator_t( const action_t& obj ) :
    action_decorator_t( &obj )
  { }
};

}

namespace spawner
{
void merge( sim_t& parent_sim, sim_t& other_sim );
void create_persistent_actors( player_t& player );

// Minimal base class to store in owner actors automatically, all functionality should be
// implemented in a templated class (pet_spawner_t for example). Methods that need to be invoked
// from the core simulator should be declared here as pure virtual (must be implemented in the
// derived class).
class base_actor_spawner_t
{
protected:
  std::string m_name;
  player_t*   m_owner;

public:
  base_actor_spawner_t( const std::string& id, player_t* o ) :
    m_name( id ), m_owner( o )
  {
    register_object();
  }

  virtual ~base_actor_spawner_t()
  { }

  const std::string& name() const
  { return m_name; }

  virtual void create_persistent_actors() = 0;

  // Data merging
  virtual void merge( base_actor_spawner_t* other ) = 0;

  // Expressions
  virtual expr_t* create_expression( const arv::array_view<std::string>& expr ) = 0;

  // Uptime
  virtual timespan_t iteration_uptime() const = 0;

  // State reset
  virtual void reset() = 0;

  // Data collection
  virtual void datacollection_end() = 0;

private:
  // Register this pet spawner object to owner
  void register_object()
  {
    auto it = range::find_if( m_owner -> spawners,
        [ this ]( const base_actor_spawner_t* obj ) {
          return util::str_compare_ci( obj -> name(), name() );
        } );

    if ( it == m_owner -> spawners.end() )
    {
      m_owner -> spawners.push_back( this );
    }
    else
    {
      m_owner -> sim -> errorf( "%s attempting to create duplicate pet spawner object %s",
        m_owner -> name(), name().c_str() );
    }
  }
};
} // Namespace spawner ends

/**
 * Snapshot players stats during pre-combat to get raid-buffed stats values.
 *
 * This allows us to report "raid-buffed" player stats by collecting the values through this action,
 * which is executed by the player action system.
 */
struct snapshot_stats_t : public action_t
{
  bool completed;
  spell_t* proxy_spell;
  attack_t* proxy_attack;
  role_e role;

  snapshot_stats_t( player_t* player, const std::string& options_str );

  void init_finished() override;
  void execute() override;
  void reset() override;
  bool ready() override;
};
#endif // SIMULATIONCRAFT_H
