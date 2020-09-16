#pragma once

#include <memory>
#include <glm/vec3.hpp>

#include "AudioSource.h"

class AudioSourceComponent {
 public:
  explicit AudioSourceComponent(std::shared_ptr<AudioClip> clip);
  ~AudioSourceComponent();

  [[nodiscard]] const AudioSource& getSource() const;
  [[nodiscard]] AudioSource& getSource();
  [[nodiscard]] AudioSource* getSourcePtr() const;

 private:
  std::shared_ptr<AudioSource> m_source;
};
