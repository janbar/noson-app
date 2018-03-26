#include <unistd.h>
#include <sys/socket.h>

#include "signalhandler.h"
#include <qdebug.h>

int SignalHandler::m_pipe[] = {0,0};

SignalHandler::SignalHandler(QObject *parent)
: QObject(parent)
{
  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_pipe))
     qFatal("Couldn't create signal socketpair.");
  else
  {
    m_notifier = new QSocketNotifier(m_pipe[1], QSocketNotifier::Read, this);
    connect(m_notifier, SIGNAL(activated(int)), this, SLOT(forward()));
    qDebug("Signal handler is enabled.");
  }
}

SignalHandler::~SignalHandler()
{
  if (m_notifier)
  {
    disconnect(m_notifier, SIGNAL(activated(int)), this, SLOT(forward()));
    delete m_notifier;
  }
  qDebug("Signal handler is destroyed.");
}

bool SignalHandler::catchSignal(int signal, int flags)
{
  struct sigaction act;
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_flags |= flags;
  return (sigaction(signal, &act, 0) == 0);
}

void SignalHandler::omitSignal(int signal)
{
  struct sigaction act;
  act.sa_handler = SIG_DFL;
  sigemptyset(&act.sa_mask);
  sigaction(signal, &act, 0);
}

void SignalHandler::forward()
{
  int signal;
  m_notifier->setEnabled(false);
  size_t r = ::read(m_pipe[1], &signal, sizeof(signal));
  (void)r;
  emit catched(signal);
  m_notifier->setEnabled(true);
}

void SignalHandler::handler(int signal)
{
  size_t w = ::write(m_pipe[0], &signal, sizeof(signal));
  (void)w;
}

