#include "include/TtsController.h"

#ifdef HAVE_QT_TTS
#include <QTextToSpeech>
#include <QVoice>
#endif
#include <QtGlobal>

#ifdef HAVE_QT_TTS
namespace {
QString voiceKeyFor(const QVoice &voice) {
  return voice.name() + "|" + voice.locale().name();
}

QString voiceLabelFor(const QVoice &voice) {
  return voice.name() + " (" + voice.locale().name() + ")";
}
} // namespace
#endif

TtsController::TtsController(QObject *parent) : QObject(parent) {
#ifdef HAVE_QT_TTS
  m_tts = new QTextToSpeech(this);
  m_available = m_tts->state() != QTextToSpeech::Error;
  m_rate = m_tts->rate();
  m_pitch = m_tts->pitch();
  m_volume = m_tts->volume();
  refreshVoices();
  connect(m_tts, &QTextToSpeech::stateChanged, this, [this](QTextToSpeech::State state) {
    const bool nowSpeaking = state == QTextToSpeech::Speaking;
    if (m_speaking != nowSpeaking) {
      m_speaking = nowSpeaking;
      emit speakingChanged();
    }
    const bool nowAvailable = state != QTextToSpeech::Error;
    if (m_available != nowAvailable) {
      m_available = nowAvailable;
      emit availabilityChanged();
    }
    if (state == QTextToSpeech::Ready && !m_queue.isEmpty()) {
      sayNext();
    }
  });
  connect(m_tts, &QTextToSpeech::voicesChanged, this, [this]() {
    refreshVoices();
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
  clearQueue();
  m_tts->stop();
  m_tts->say(text);
  return true;
#else
  Q_UNUSED(text)
  return false;
#endif
}

void TtsController::enqueue(const QString &text) {
#ifdef HAVE_QT_TTS
  if (!m_tts || text.trimmed().isEmpty()) {
    return;
  }
  m_queue.append(text);
  emit queueLengthChanged();
  if (m_tts->state() == QTextToSpeech::Ready && !m_speaking) {
    sayNext();
  }
#else
  Q_UNUSED(text)
#endif
}

void TtsController::speakQueue(const QStringList &texts) {
#ifdef HAVE_QT_TTS
  if (!m_tts) {
    return;
  }
  clearQueue();
  for (const auto &text : texts) {
    if (!text.trimmed().isEmpty()) {
      m_queue.append(text);
    }
  }
  emit queueLengthChanged();
  if (m_tts->state() == QTextToSpeech::Ready && !m_queue.isEmpty()) {
    sayNext();
  }
#else
  Q_UNUSED(texts)
#endif
}

void TtsController::stop() {
#ifdef HAVE_QT_TTS
  if (m_tts) {
    m_tts->stop();
  }
#endif
  clearQueue();
}

bool TtsController::available() const { return m_available; }

bool TtsController::speaking() const { return m_speaking; }

double TtsController::rate() const { return m_rate; }

double TtsController::pitch() const { return m_pitch; }

double TtsController::volume() const { return m_volume; }

QString TtsController::voiceKey() const { return m_voiceKey; }

QStringList TtsController::voiceKeys() const { return m_voiceKeys; }

QStringList TtsController::voiceLabels() const { return m_voiceLabels; }

int TtsController::queueLength() const { return m_queue.size(); }

void TtsController::setRate(double rate) {
#ifdef HAVE_QT_TTS
  const double clamped = qBound(-1.0, rate, 1.0);
  if (m_rate == clamped) {
    return;
  }
  m_rate = clamped;
  if (m_tts) {
    m_tts->setRate(m_rate);
  }
  emit rateChanged();
#else
  Q_UNUSED(rate)
#endif
}

void TtsController::setPitch(double pitch) {
#ifdef HAVE_QT_TTS
  const double clamped = qBound(-1.0, pitch, 1.0);
  if (m_pitch == clamped) {
    return;
  }
  m_pitch = clamped;
  if (m_tts) {
    m_tts->setPitch(m_pitch);
  }
  emit pitchChanged();
#else
  Q_UNUSED(pitch)
#endif
}

void TtsController::setVolume(double volume) {
#ifdef HAVE_QT_TTS
  const double clamped = qBound(0.0, volume, 1.0);
  if (m_volume == clamped) {
    return;
  }
  m_volume = clamped;
  if (m_tts) {
    m_tts->setVolume(m_volume);
  }
  emit volumeChanged();
#else
  Q_UNUSED(volume)
#endif
}

void TtsController::setVoiceKey(const QString &key) {
#ifdef HAVE_QT_TTS
  if (!m_tts || key.isEmpty() || m_voiceKey == key) {
    return;
  }
  const auto voices = m_tts->availableVoices();
  for (const auto &voice : voices) {
    if (voiceKeyFor(voice) == key) {
      m_tts->setVoice(voice);
      m_voiceKey = key;
      emit voiceKeyChanged();
      return;
    }
  }
#else
  Q_UNUSED(key)
#endif
}

void TtsController::clearQueue() {
  if (m_queue.isEmpty()) {
    return;
  }
  m_queue.clear();
  emit queueLengthChanged();
}

void TtsController::refreshVoices() {
#ifdef HAVE_QT_TTS
  if (!m_tts) {
    return;
  }
  const auto voices = m_tts->availableVoices();
  QStringList keys;
  QStringList labels;
  keys.reserve(voices.size());
  labels.reserve(voices.size());
  for (const auto &voice : voices) {
    keys.append(voiceKeyFor(voice));
    labels.append(voiceLabelFor(voice));
  }
  m_voiceKeys = keys;
  m_voiceLabels = labels;

  const QString currentKey = voiceKeyFor(m_tts->voice());
  if (m_voiceKey != currentKey) {
    m_voiceKey = currentKey;
    emit voiceKeyChanged();
  }
  emit voicesChanged();
#endif
}

void TtsController::sayNext() {
#ifdef HAVE_QT_TTS
  if (!m_tts || m_queue.isEmpty()) {
    return;
  }
  const QString next = m_queue.takeFirst();
  emit queueLengthChanged();
  if (!next.trimmed().isEmpty()) {
    m_tts->say(next);
  }
#endif
}
