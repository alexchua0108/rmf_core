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

#ifndef SRC__RMF_FLEET_ADAPTER__TASKMANAGER_HPP
#define SRC__RMF_FLEET_ADAPTER__TASKMANAGER_HPP

#include "Task.hpp"
#include "agv/RobotContext.hpp"

#include <rmf_traffic/agv/Planner.hpp>

#include <rmf_task/agv/TaskPlanner.hpp>

#include <mutex>

namespace rmf_fleet_adapter {

//==============================================================================
/// This task manager is a first attempt at managing multiple tasks per fleet.
/// This is a simple implementation that only makes a modest attempt at being
/// optimal. A better task manager would queue tasks across the whole fleet
/// instead of queuing tasks for each robot individually. We will attempt that
/// in a later implementation.
class TaskManager : public std::enable_shared_from_this<TaskManager>
{
public:

  static std::shared_ptr<TaskManager> make(agv::RobotContextPtr context);

  using Start = rmf_traffic::agv::Plan::Start;
  using StartSet = rmf_traffic::agv::Plan::StartSet;
  using Assignment = rmf_task::agv::TaskPlanner::Assignment;
  using State = rmf_task::agv::State;

  /// Add a task to the queue of this manager.
  void queue_task(std::shared_ptr<Task> task);

  const agv::RobotContextPtr& context();

  agv::ConstRobotContextPtr context() const;

  const Task* current_task() const;

  // /// Set the queue for this task manager with assignments generated from the
  // /// task planner
  // void set_queue(const std::vector<Assignment>& assignments);

  /// set a vector of tasks
  void set_queue(const std::vector<std::shared_ptr<Task>>& tasks);

  // get tasks in the queue
  const std::vector<std::shared_ptr<Task>> task_queue() const;

  /// Get the non-charging requests among pending tasks
  const std::vector<rmf_task::ConstRequestPtr> requests() const;

  /// The finish state of the current task.
  State expected_finish_state() const;

  /// Callback for the retreat timer. Appends a charging task to the task queue
  /// when robot is idle and battery level drops below a retreat threshold.
  void retreat_to_charger();

  /// TODO(YL) not needed, should be removed?
  /// Get the list of task ids for tasks that have started execution. 
  /// The list will contain upto 100 latest task ids only.
  const std::vector<std::string>& get_executed_tasks() const;  

private:

  TaskManager(agv::RobotContextPtr context);

  agv::RobotContextPtr _context;
  std::shared_ptr<Task> _active_task;
  std::vector<std::shared_ptr<Task>> _queue;
  rxcpp::subscription _task_sub;
  rxcpp::subscription _emergency_sub;

  // TODO: Eliminate the need for a mutex by redesigning the use of the task
  // manager so that modifications of shared data only happen on designated
  // rxcpp worker
  std::mutex _mutex;
  rclcpp::TimerBase::SharedPtr _task_timer;
  rclcpp::TimerBase::SharedPtr _retreat_timer;

  // Container to keep track of tasks that have been started by this TaskManager
  // Use the _register_executed_task() to populate this container.
  std::vector<std::string> _executed_task_registry;

  /// Callback for task timer which begins next task if its deployment time has passed
  void _begin_next_task();

  /// Function to register the task id of a task that has begun execution
  /// The input task id will be inserted into the registry such that the max
  /// size of the registry is 100.
  void _register_executed_task(const std::string& id);
};

using TaskManagerPtr = std::shared_ptr<TaskManager>;

} // namespace rmf_fleet_adapter

#endif // SRC__RMF_FLEET_ADAPTER__TASKMANAGER_HPP
