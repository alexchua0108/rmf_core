/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef RMF_TRAFFIC__RESERVATIONS_HPP
#define RMF_TRAFFIC__RESERVATIONS_HPP

#include <optional>
#include <rmf_traffic/Time.hpp>
#include <rmf_traffic/agv/Graph.hpp>
#include <rmf_traffic/schedule/Participant.hpp>
#include <rmf_utils/impl_ptr.hpp>

namespace rmf_traffic {
namespace reservations {

//==============================================================================
using ReservationId = size_t;

//==============================================================================  
class Reservation
{
public:

  /// A unique ID for reservations
  ReservationId reservation_id() const;

  /// Get the reservation way point
  const rmf_traffic::agv::Graph::Waypoint waypoint() const;

  /// Traffic duration
  const std::optional<rmf_traffic::Duration> duration() const;

  /// Returns the participant for who the reservation is max
  rmf_traffic::schedule::ParticipantId participantId() const;

  class Implementation;

  /// Constructor
  /// Do not use this to construct reservations unless you are deseriallizing/
  /// Serializing resolutions. If you wish to make a reservation see
  /// the \ref{ReservationSystem} class.
  Reservation(
    ReservationId unique_id,
    std::optional<rmf_traffic::Duration> duration,
    rmf_traffic::agv::Graph::Waypoint waypoint,
    rmf_traffic::schedule::ParticipantId participantId);

private:
  rmf_utils::impl_ptr<Implementation> _pimpl;
};

//==============================================================================
/// Manages reservations of waypoints within the class 
class ReservationSystem
{
public:
  /// Reserves an item for a given robot for a fixed duration
  /// \param participantId
  ///   The participant ID whishing to make this reservation
  /// \param time
  ///   The time at which the reservation will take place
  /// \param vertices
  ///   The waypoints in which we are interested
  /// \param duration
  ///   The amount of time for which we would like to make the
  /// \return a nullopt if the reservation is not possible,
  /// otherwise returns a Reservation object.
  std::optional<Reservation> reserve(
    const rmf_traffic::schedule::ParticipantId participantId,
    const rmf_traffic::Time time,
    const std::vector<rmf_traffic::agv::Graph::Waypoint>& vertices,
    std::optional<rmf_traffic::Duration> duration = std::nullopt);
  
  /// Cancels a reservation
  /// \throws runtime error if a reservation with given ID is  not found
  void cancel_reservation(ReservationId reservation_id);

  ReservationSystem();

  class Implementation;
private:
  rmf_utils::unique_impl_ptr<Implementation> _pimpl;
};

} // end namespace reservations
} // end namespace rmf_traffic
#endif