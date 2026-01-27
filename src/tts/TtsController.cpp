#include "include/TtsController.h"

#ifdef HAVE_QT_TTS
#include <QTextToSpeech>
#endif

TtsController::TtsController(QObject *parent) : QObject(parent) {
#ifdef HAVE_QT_TTS
  m_tts = new QTextToSpeech(this);
  m_available = m_tts->state() != QTextToSpeech::Error;
  connect(m_tts, &QTextToSpeech::stateChanged, this, [this](QTextToSpeech::State state) {
    const bool nowSpeaking = state == QTextToSpeech::Speaking;
    if (m_speaking != nowSpeaking) {
      m_speaking = nowSpeaking;
      emit speakingChanged();
    }
  });
#else
  m_available = false;
#endif
}

bool TtsController::speak(const QString &text) {
#ifdef HAVE_QT_TTS
  if (!m_tts || text.trimmed().isEmpty()) {
    return false;
  }
  m_tts->say(text);
  return true;
#else
  Q_UNUSED(text)
  return false;
#endif
}

void TtsController::stop() {
#ifdef HAVE_QT_TTS
  if (m_tts) {
    m_tts->stop();
  }
#endif
}

bool TtsController::available() const { return m_available; }

bool TtsController::speaking() const { return m_speaking; }
