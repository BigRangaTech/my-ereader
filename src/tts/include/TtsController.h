#pragma once

#include <QObject>
#include <QString>

class TtsController : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool available READ available NOTIFY availabilityChanged)
  Q_PROPERTY(bool speaking READ speaking NOTIFY speakingChanged)

public:
  explicit TtsController(QObject *parent = nullptr);

  Q_INVOKABLE bool speak(const QString &text);
  Q_INVOKABLE void stop();

  bool available() const;
  bool speaking() const;

signals:
  void availabilityChanged();
  void speakingChanged();

private:
  bool m_available = false;
  bool m_speaking = false;

#ifdef HAVE_QT_TTS
  class QTextToSpeech;
  QTextToSpeech *m_tts = nullptr;
#endif
};
