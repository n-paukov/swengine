#include "Dialogue.h"

#include <Engine/Utility/containers.h>

#include <utility>

DialoguePhrase::DialoguePhrase(std::string id, std::string content)
  : m_id(std::move(id)),
    m_content(std::move(content))
{

}

void DialoguePhrase::setId(const std::string& id)
{
  m_id = id;
}

const std::string& DialoguePhrase::getId() const
{
  return m_id;
}

void DialoguePhrase::setContent(const std::string& content)
{
  m_content = content;
}

const std::string& DialoguePhrase::getContent() const
{
  return m_content;
}

void DialoguePhrase::addResponse(const std::string& response)
{
  /* NOTE: it is allowed to add the same response several times */
  m_responses.push_back(response);
}

const std::vector<std::string>& DialoguePhrase::getResponses() const
{
  return m_responses;
}

void DialoguePhrase::setPrecondition(std::shared_ptr<GameLogicCondition> condition)
{
  m_precondition = std::move(condition);
}

std::shared_ptr<GameLogicCondition> DialoguePhrase::getPrecondition() const
{
  return m_precondition;
}

void DialoguePhrase::setActions(const GameLogicActionsList& actions)
{
  m_logicActions = actions;
}

const GameLogicActionsList& DialoguePhrase::getActions() const
{
  return m_logicActions;
}

Dialogue::Dialogue(std::string id)
  : m_id(std::move(id))
{

}

void Dialogue::setId(const std::string& id)
{
  m_id = id;
}

const std::string& Dialogue::getId() const
{
  return m_id;
}

void Dialogue::addPhrase(const DialoguePhrase& phrase)
{
  SW_ASSERT(!m_phrases.contains(phrase.getId()));

  m_phrases.insert({phrase.getId(), phrase});
}

const DialoguePhrase& Dialogue::getStartPhrase() const
{
  return getPhrase("0");
}

const DialoguePhrase& Dialogue::getPhrase(const std::string& id) const
{
  return m_phrases.at(id);
}

bool Dialogue::hasPhrase(const std::string& id) const
{
  return m_phrases.contains(id);
}
