#include "DialoguesUI.h"

#include <utility>
#include <Engine/Modules/Graphics/GUI/GUIText.h>

#include "Game/PlayerComponent.h"
#include "ActorComponent.h"

DialoguesUIHistoryItem::DialoguesUIHistoryItem(std::string initiatorName, std::string phraseText)
  : m_initiatorName(std::move(initiatorName)),
    m_phraseText(std::move(phraseText))
{

}

const std::string& DialoguesUIHistoryItem::getInitiatorName() const
{
  return m_initiatorName;
}

const std::string& DialoguesUIHistoryItem::getPhraseText() const
{
  return m_phraseText;
}

DialoguesUI::DialoguesUI(
  std::shared_ptr<GameWorld> gameWorld,
  std::shared_ptr<DialoguesManager> dialoguesManager,
  std::shared_ptr<QuestsStorage> questsStorage)
  : GUILayout("dialogues_ui"),
    m_gameWorld(std::move(gameWorld)),
    m_dialoguesManager(std::move(dialoguesManager)),
    m_questsStorage(std::move(questsStorage))
{

}

void DialoguesUI::startDialogue(GameObject initiator, GameObject target)
{
  m_initiator = initiator;
  m_target = target;

  m_dialogueHistory.clear();
  m_phrasesLayout->removeChildren();

  auto initiatorResponses = m_dialoguesManager->startAnyDialogue(
    m_initiator,
    m_target,
    m_dialogueState);

  updateUILayout(initiatorResponses);
}

void DialoguesUI::stopDialogue()
{
  m_dialoguesManager->stopDialogue(m_dialogueState);
}

void DialoguesUI::onShow()
{
  if (m_phrasesLayout == nullptr || m_responsesLayout == nullptr) {
    m_phrasesLayout = std::dynamic_pointer_cast<GUILayout>(
      findChildByName("game_ui_dialogues_layout_phrases"));

    SW_ASSERT(m_phrasesLayout != nullptr &&
      "Specify phrases layout in the dialogues UI description");

    m_responsesLayout = std::dynamic_pointer_cast<GUILayout>(
      findChildByName("game_ui_dialogues_layout_responses"));

    SW_ASSERT(m_responsesLayout != nullptr &&
      "Specify responses layout in the dialogues UI description");
  }

  m_gameWorld->subscribeEventsListener<QuestStartedEvent>(this);
  m_gameWorld->subscribeEventsListener<QuestCompletedEvent>(this);
  m_gameWorld->subscribeEventsListener<QuestFailedEvent>(this);
  m_gameWorld->subscribeEventsListener<QuestTaskStartedEvent>(this);
  m_gameWorld->subscribeEventsListener<QuestTaskCompletedEvent>(this);
  m_gameWorld->subscribeEventsListener<QuestTaskFailedEvent>(this);
  m_gameWorld->subscribeEventsListener<InventoryItemTransferEvent>(this);

}

void DialoguesUI::onHide()
{
  stopDialogue();

  m_gameWorld->cancelEventsListening(this);
}

void DialoguesUI::triggerResponsePhrase(const DialogueResponse& response)
{
  // Add player response phrase to UI layout
  const Dialogue& dialogue = m_dialoguesManager->getDialogue(response.getDialogueId());
  addPhrase(m_initiator, m_target, dialogue.getPhrase(response.getPhraseId()));

  auto initiatorResponses = m_dialoguesManager->continueDialogue(response, m_dialogueState);
  updateUILayout(initiatorResponses);
}

void DialoguesUI::updateResponsesLayout(const std::vector<DialogueResponse>& dialogueResponses)
{
  // Detach responses layout and attach it again after layout formation
  // to trigger stylesheet updating for its children
  removeChildWidget(m_responsesLayout);

  m_responsesLayout->removeChildren();

  for (const DialogueResponse& response : dialogueResponses) {
    LOCAL_VALUE_UNUSED(response);

    auto responseText = std::make_shared<GUIText>();
    m_responsesLayout->addChildWidget(responseText);
  }

  addChildWidget(m_responsesLayout);

  glm::ivec2 offset = m_phrasesMargin;

  const auto& childrenResponseWidgets = m_responsesLayout->getChildrenWidgets();

  for (size_t widgetIndex = 0; const DialogueResponse& response : dialogueResponses) {
    std::shared_ptr<GUIText> responseTextWidget = std::dynamic_pointer_cast<GUIText>(
      childrenResponseWidgets[widgetIndex]);

    const Dialogue& dialogue = m_dialoguesManager->getDialogue(response.getDialogueId());
    responseTextWidget->setText(dialogue.getPhrase(response.getPhraseId()).getContent());

    responseTextWidget->setOrigin(offset);
    offset.y += responseTextWidget->getSize().y;

    responseTextWidget->setMouseButtonCallback([this, response](const GUIMouseButtonEvent& event) {
      if (event.type == MouseButtonEventType::ButtonDown) {
        if (event.button == SDL_BUTTON_LEFT) {
          m_pendingResponse = response;
        }
      }
    });

    widgetIndex++;
  }
}

void DialoguesUI::addPhrase(GameObject initiator, GameObject target, const DialoguePhrase& phrase)
{
  ARG_UNUSED(target);

  m_dialogueHistory.emplace_back(
    initiator.getComponent<ActorComponent>()->getName(),
    phrase.getContent());

  std::string initiatorName = initiator.getComponent<ActorComponent>()->getName();

  if (initiator.hasComponent<PlayerComponent>()) {
    initiatorName = "You";
  }

  std::string phraseText = initiatorName + ": " + phrase.getContent();

  addTextMessage(phraseText);
}

void DialoguesUI::applyStylesheetRule(const GUIWidgetStylesheetRule& stylesheetRule)
{
  GUILayout::applyStylesheetRule(stylesheetRule);

  stylesheetRule.visit([this](auto propertyName, auto property, GUIWidgetVisualState visualState) {
    if (propertyName == "phrases-margin") {
      SW_ASSERT(std::holds_alternative<glm::ivec2>(property.getValue()));
      SW_ASSERT(visualState == GUIWidgetVisualState::Default &&
        "Phrases margin is supported only for default state");

      this->setPhrasesMargin(std::get<glm::ivec2>(property.getValue()));
    }
    else if (propertyName == "background") {
      // Do nothing as property should be already processed by GUILayout
    }
    else {
      SW_ASSERT(false);
    }

  });
}

void DialoguesUI::setPhrasesMargin(const glm::ivec2& margin)
{
  m_phrasesMargin = margin;
}

const glm::ivec2& DialoguesUI::getPhrasesMargin() const
{
  return m_phrasesMargin;
}

void DialoguesUI::updateUILayout(const std::vector<DialogueResponse>& dialogueResponses)
{
  if (m_dialogueState.hasLastPhrase()) {
    // Add NPC phrase to UI layout
    const Dialogue& dialogue = m_dialoguesManager->getDialogue(m_dialogueState.getDialogueId());

    GameObject phraseInitiator = m_target;
    GameObject phraseTarget = m_initiator;

    addPhrase(phraseInitiator, phraseTarget, dialogue.getPhrase(m_dialogueState.getLastPhraseId()));
  }

  updateResponsesLayout(dialogueResponses);
}

void DialoguesUI::updatePendingResponses()
{
  if (m_pendingResponse) {
    triggerResponsePhrase(m_pendingResponse.value());
    m_pendingResponse.reset();
  }
}

void DialoguesUI::addNotification(const std::string& notification)
{
  addTextMessage("          " + notification);
}

void DialoguesUI::addTextMessage(const std::string& message)
{
  // Detach phrases layout and attach it again after layout formation
  // to trigger stylesheet updating for its children
  removeChildWidget(m_phrasesLayout);

  auto phraseTextWidget = std::make_shared<GUIText>();
  m_phrasesLayout->addChildWidget(phraseTextWidget);

  addChildWidget(m_phrasesLayout);

  phraseTextWidget->setText(message);

  const auto& phrasesWidgets = m_phrasesLayout->getChildrenWidgets();
  int totalHeight = 0;

  for (const auto& phraseWidget : phrasesWidgets) {
    totalHeight += phraseWidget->getSize().y + m_phrasesMargin.y;
  }

  glm::ivec2 offset = m_phrasesMargin;

  if (totalHeight >= m_phrasesLayout->getSize().y) {
    offset.y = -(totalHeight - m_phrasesLayout->getSize().y);
  }

  for (auto& phraseWidget : phrasesWidgets) {
    phraseWidget->setOrigin(offset);
    offset.y += phraseWidget->getSize().y + m_phrasesMargin.y;
  }
}

EventProcessStatus DialoguesUI::receiveEvent(const QuestStartedEvent& event)
{
  addNotification(fmt::format("Quest started: {}",
    m_questsStorage->getQuest(event.getQuestId()).getName()));

  return EventProcessStatus::Processed;
}

EventProcessStatus DialoguesUI::receiveEvent(const QuestCompletedEvent& event)
{
  addNotification(fmt::format("Quest completed: {}",
    m_questsStorage->getQuest(event.getQuestId()).getName()));

  return EventProcessStatus::Processed;
}

EventProcessStatus DialoguesUI::receiveEvent(const QuestFailedEvent& event)
{
  addNotification(fmt::format("Quest failed: {}",
    m_questsStorage->getQuest(event.getQuestId()).getName()));

  return EventProcessStatus::Processed;
}

EventProcessStatus DialoguesUI::receiveEvent(const QuestTaskStartedEvent& event)
{
  addNotification(fmt::format("Quest task started: {}",
    m_questsStorage->getQuest(event.getQuestId()).getTask(event.getTaskId()).getName()));

  return EventProcessStatus::Processed;
}

EventProcessStatus DialoguesUI::receiveEvent(const QuestTaskCompletedEvent& event)
{
  addNotification(fmt::format("Quest task completed: {}",
    m_questsStorage->getQuest(event.getQuestId()).getTask(event.getTaskId()).getName()));

  return EventProcessStatus::Processed;
}

EventProcessStatus DialoguesUI::receiveEvent(const QuestTaskFailedEvent& event)
{
  addNotification(fmt::format("Quest task failed: {}",
    m_questsStorage->getQuest(event.getQuestId()).getTask(event.getTaskId()).getName()));

  return EventProcessStatus::Processed;
}

EventProcessStatus DialoguesUI::receiveEvent(const InventoryItemTransferEvent& event)
{
  if (event.initiator == m_initiator) {
    addNotification(fmt::format("Inventory item {} is transferred to {}",
      GameObject(event.item).getComponent<InventoryItemComponent>()->getName(),
      GameObject(event.target).getComponent<ActorComponent>()->getName()));
  }
  else {
    addNotification(fmt::format("Inventory item {} is received from {}",
      GameObject(event.item).getComponent<InventoryItemComponent>()->getName(),
      GameObject(event.target).getComponent<ActorComponent>()->getName()));
  }

  return EventProcessStatus::Processed;
}

std::shared_ptr<QuestsStorage> DialoguesUI::getQuestsStorage() const
{
  return m_questsStorage;
}
