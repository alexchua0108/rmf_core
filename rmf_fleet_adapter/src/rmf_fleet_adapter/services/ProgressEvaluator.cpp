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

#include "ProgressEvaluator.hpp"


#include <iostream>

namespace rmf_fleet_adapter {
namespace services {

//==============================================================================
ProgressEvaluator::ProgressEvaluator(
    const double compliant_leeway_base_,
    const double compliant_leeway_multiplier_,
    const double estimate_leeway_)
  : compliant_leeway_base(compliant_leeway_base_),
    compliant_leeway_multiplier(compliant_leeway_multiplier_),
    estimate_leeway(estimate_leeway_)
{
  // Do nothing
}

//==============================================================================
bool ProgressEvaluator::initialize(const Result& setup)
{
  if (!setup.cost_estimate())
    return false;

  const double cost = *setup.cost_estimate();
  if (cost < best_estimate.cost)
    best_estimate = Info{cost, &setup};

  return true;
}

//==============================================================================
bool ProgressEvaluator::evaluate(Result& progress)
{
  if (!progress.success() && !progress.cost_estimate())
  {
    ++finished_count;
    return false;
  }

  const double cost = progress.success()?
        progress->get_cost() : *progress.cost_estimate();

  if (progress.success())
  {
    if (cost < best_result.cost)
      best_result = Info{cost, &progress};
  }

  if (cost < second_best_estimate.cost)
    second_best_estimate = Info{cost, &progress};

  if (best_estimate.progress == &progress)
  {
    best_estimate = second_best_estimate;
    second_best_estimate = Info();
  }

  const double dropdead_cost =
      compliant_leeway_multiplier*progress.initial_cost_estimate()
      + compliant_leeway_base;

  const bool giveup = dropdead_cost <= cost;
  if (!progress.success() && !giveup)
  {
    if (!best_result.progress)
    {
      progress.options().maximum_cost_estimate(
            std::min(estimate_leeway * cost, dropdead_cost));

//      std::cout << "(" << &progress << ") Cost: " << cost << " | Next: "
//                << estimate_leeway*cost << " | Drop: " << dropdead_cost << std::endl;
      return true;
    }
    else if (cost < best_result.cost)
    {
//      std::cout << "(" << &progress << ") Cost: " << cost << " | Best: "
//                << estimate_leeway*cost << " | Drop: " << dropdead_cost << std::endl;
      progress.options().maximum_cost_estimate(
            std::min(best_result.cost, dropdead_cost));
      return true;
    }
  }

  ++finished_count;
  return false;
}

//==============================================================================
void ProgressEvaluator::discard(Result& progress)
{
  if (best_estimate.progress == &progress)
  {
    best_estimate = second_best_estimate;
    second_best_estimate = Info();
  }

  const double cost = progress.cost_estimate()?
        *progress.cost_estimate() : std::numeric_limits<double>::infinity();
  if (best_discarded.progress || cost < best_discarded.cost)
    best_discarded = Info{cost, &progress};

  ++finished_count;
}

} // namespace services
} // namespace rmf_fleet_adapter
