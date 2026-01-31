#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#ifdef HAVE_QT_TTS
class QTextToSpeech;
#endif

class TtsController : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool available READ available NOTIFY availabilityChanged)
  Q_PROPERTY(bool speaking READ speaking NOTIFY speakingChanged)
  Q_PROPERTY(double rate READ rate WRITE setRate NOTIFY rateChanged)
  Q_PROPERTY(double pitch READ pitch WRITE setPitch NOTIFY pitchChanged)
  Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(QString voiceKey READ voiceKey WRITE setVoiceKey NOTIFY voiceKeyChanged)
  Q_PROPERTY(QStringList voiceKeys READ voiceKeys NOTIFY voicesChanged)
  Q_PROPERTY(QStringList voiceLabels READ voiceLabels NOTIFY voicesChanged)
  Q_PROPERTY(int queueLength READ queueLength NOTIFY queueLengthChanged)

public:
  explicit TtsController(QObject *parent = nullptr);

  Q_INVOKABLE bool speak(const QString &text);
  Q_INVOKABLE void enqueue(const QString &text);
  Q_INVOKABLE void speakQueue(const QStringList &texts);
  Q_INVOKABLE void stop();
  Q_INVOKABLE void clearQueue();

  bool available() const;
  bool speaking() const;
  double rate() const;
  double pitch() const;
  double volume() const;
  QString voiceKey() const;
  QStringList voiceKeys() const;
  QStringList voiceLabels() const;
  int queueLength() const;

  void setRate(double rate);
  void setPitch(double pitch);
  void setVolume(double volume);
  void setVoiceKey(const QString &key);

signals:
  void availabilityChanged();
  void speakingChanged();
  void rateChanged();
  void pitchChanged();
  void volumeChanged();
  void voiceKeyChanged();
  void voicesChanged();
  void queueLengthChanged();

private:
  void refreshVoices();
  void sayNext();

  bool m_available = false;
  bool m_speaking = false;
  double m_rate = 0.0;
  double m_pitch = 0.0;
  double m_volume = 1.0;
  QString m_voiceKey;
  QStringList m_voiceKeys;
  QStringList m_voiceLabels;
  QStringList m_queue;

#ifdef HAVE_QT_TTS
  QTextToSpeech *m_tts = nullptr;
#endif
};
