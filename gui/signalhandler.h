#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <qsocketnotifier.h>
#include <signal.h>

class SignalHandler : public QObject
{
    Q_OBJECT
public:
  SignalHandler(QObject *parent = 0);
  virtual ~SignalHandler();

  bool catchSignal(int signal);
  void omitSignal(int signal);

signals:
  void catched(int signal);

private slots:
  void forward();

private:
  SignalHandler(const SignalHandler&);
  SignalHandler& operator=(const SignalHandler&);

  QSocketNotifier *m_notifier;

  static int m_pipe[2];
  static void handler(int signal, siginfo_t * info, void * data);
};

#endif // SIGNALHANDLER_H
