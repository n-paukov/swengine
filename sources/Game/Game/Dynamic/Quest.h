#pragma once

#include <string>
#include <vector>
#include "GameLogicConditionsManager.h"

class QuestTask {
 public:
  QuestTask(std::string id, std::string name, std::string description);

  [[nodiscard]] const std::string& getId() const;
  [[nodiscard]] const std::string& getName() const;
  [[nodiscard]] const std::string& getDescription() const;

  void setAutostartCondition(std::shared_ptr<GameLogicCondition> condition);
  [[nodiscard]] GameLogicCondition& getAutostartCondition();

  void setActiveCondition(std::shared_ptr<GameLogicCondition> condition);
  [[nodiscard]] GameLogicCondition& getActiveCondition();

  void setCompleteCondition(std::shared_ptr<GameLogicCondition> condition);
  [[nodiscard]] GameLogicCondition& geCompleteCondition();

 private:
  std::string m_id;
  std::string m_name;
  std::string m_description;

  std::shared_ptr<GameLogicCondition> m_autostartCondition;
  std::shared_ptr<GameLogicCondition> m_activeCondition;
  std::shared_ptr<GameLogicCondition> m_completeCondition;
};

class Quest {
 public:
  Quest(std::string id, std::string name, std::string description);

  [[nodiscard]] const std::string& getId() const;
  [[nodiscard]] const std::string& getName() const;
  [[nodiscard]] const std::string& getDescription() const;

  void addTask(const QuestTask& task);
  [[nodiscard]] const std::vector<QuestTask>& getTasks() const;

 private:
  std::string m_id;
  std::string m_name;
  std::string m_description;

  std::vector<QuestTask> m_tasks;
};
