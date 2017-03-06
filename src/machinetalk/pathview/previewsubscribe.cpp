/****************************************************************************
**
** This file was generated by a code generator based on imatix/gsl
** Any changes in this file will be lost.
**
****************************************************************************/
#include "previewsubscribe.h"
#include <google/protobuf/text_format.h>
#include "debughelper.h"

#if defined(Q_OS_IOS)
namespace gpb = google_public::protobuf;
#else
namespace gpb = google::protobuf;
#endif

using namespace nzmqt;

namespace machinetalk {
namespace pathview {

/** Generic Preview Subscribe implementation */
PreviewSubscribe::PreviewSubscribe(QObject *parent) :
    QObject(parent),
    m_ready(false),
    m_debugName("Preview Subscribe"),
    m_socketUri(""),
    m_context(nullptr),
    m_socket(nullptr),
    m_state(Down),
    m_previousState(Down),
    m_errorString("")
{
    // state machine
    connect(this, &PreviewSubscribe::fsmDownConnect,
            this, &PreviewSubscribe::fsmDownConnectEvent);
    connect(this, &PreviewSubscribe::fsmTryingConnected,
            this, &PreviewSubscribe::fsmTryingConnectedEvent);
    connect(this, &PreviewSubscribe::fsmTryingDisconnect,
            this, &PreviewSubscribe::fsmTryingDisconnectEvent);
    connect(this, &PreviewSubscribe::fsmUpMessageReceived,
            this, &PreviewSubscribe::fsmUpMessageReceivedEvent);
    connect(this, &PreviewSubscribe::fsmUpDisconnect,
            this, &PreviewSubscribe::fsmUpDisconnectEvent);

    m_context = new PollingZMQContext(this, 1);
    connect(m_context, &PollingZMQContext::pollError,
            this, &PreviewSubscribe::socketError);
    m_context->start();
}

PreviewSubscribe::~PreviewSubscribe()
{
    stopSocket();

    if (m_context != nullptr)
    {
        m_context->stop();
        m_context->deleteLater();
        m_context = nullptr;
    }
}

/** Add a topic that should be subscribed **/
void PreviewSubscribe::addSocketTopic(const QString &name)
{
    m_socketTopics.insert(name);
}

/** Removes a topic from the list of topics that should be subscribed **/
void PreviewSubscribe::removeSocketTopic(const QString &name)
{
    m_socketTopics.remove(name);
}

/** Clears the the topics that should be subscribed **/
void PreviewSubscribe::clearSocketTopics()
{
    m_socketTopics.clear();
}

/** Connects the 0MQ sockets */
bool PreviewSubscribe::startSocket()
{
    m_socket = m_context->createSocket(ZMQSocket::TYP_SUB, this);
    m_socket->setLinger(0);

    try {
        m_socket->connectTo(m_socketUri);
    }
    catch (const zmq::error_t &e) {
        QString errorString;
        errorString = QString("Error %1: ").arg(e.num()) + QString(e.what());
        //updateState(SocketError, errorString); TODO
        return false;
    }

    connect(m_socket, &ZMQSocket::messageReceived,
            this, &PreviewSubscribe::processSocketMessage);


    for (QString topic: m_socketTopics)
    {
        m_socket->subscribeTo(topic.toLocal8Bit());
    }

#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "sockets connected" << m_socketUri);
#endif

    return true;
}

/** Disconnects the 0MQ sockets */
void PreviewSubscribe::stopSocket()
{
    if (m_socket != nullptr)
    {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

/** Processes all message received on socket */
void PreviewSubscribe::processSocketMessage(const QList<QByteArray> &messageList)
{
    Container &rx = m_socketRx;
    QByteArray topic;

    if (messageList.length() < 2)  // in case we received insufficient data
    {
        return;
    }

    // we only handle the first two messges
    topic = messageList.at(0);
    rx.ParseFromArray(messageList.at(1).data(), messageList.at(1).size());

#ifdef QT_DEBUG
    std::string s;
    gpb::TextFormat::PrintToString(rx, &s);
    DEBUG_TAG(3, m_debugName, "server message" << QString::fromStdString(s));
#endif

    // react to any incoming message

    if (m_state == Up)
    {
        emit fsmUpMessageReceived(QPrivateSignal());
    }

    emit socketMessageReceived(topic, rx);
}

void PreviewSubscribe::socketError(int errorNum, const QString &errorMsg)
{
    QString errorString;
    errorString = QString("Error %1: ").arg(errorNum) + errorMsg;
}

void PreviewSubscribe::fsmDown()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State DOWN");
#endif
    m_state = Down;
    emit stateChanged(m_state);
}

void PreviewSubscribe::fsmDownConnectEvent()
{
    if (m_state == Down)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONNECT");
#endif
        // handle state change
        emit fsmDownExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
        startSocket();
        connected();
     }
}

void PreviewSubscribe::fsmTrying()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State TRYING");
#endif
    m_state = Trying;
    emit stateChanged(m_state);
}

void PreviewSubscribe::fsmTryingConnectedEvent()
{
    if (m_state == Trying)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONNECTED");
#endif
        // handle state change
        emit fsmTryingExited(QPrivateSignal());
        fsmUp();
        emit fsmUpEntered(QPrivateSignal());
        // execute actions
     }
}

void PreviewSubscribe::fsmTryingDisconnectEvent()
{
    if (m_state == Trying)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif
        // handle state change
        emit fsmTryingExited(QPrivateSignal());
        fsmDown();
        emit fsmDownEntered(QPrivateSignal());
        // execute actions
        stopSocket();
     }
}

void PreviewSubscribe::fsmUp()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State UP");
#endif
    m_state = Up;
    emit stateChanged(m_state);
}

void PreviewSubscribe::fsmUpMessageReceivedEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event MESSAGE RECEIVED");
#endif
        // execute actions
     }
}

void PreviewSubscribe::fsmUpDisconnectEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif
        // handle state change
        emit fsmUpExited(QPrivateSignal());
        fsmDown();
        emit fsmDownEntered(QPrivateSignal());
        // execute actions
        stopSocket();
     }
}

/** start trigger function */
void PreviewSubscribe::start()
{
    if (m_state == Down) {
        emit fsmDownConnect(QPrivateSignal());
    }
}

/** stop trigger function */
void PreviewSubscribe::stop()
{
    if (m_state == Up) {
        emit fsmUpDisconnect(QPrivateSignal());
    }
}

/** connected trigger function */
void PreviewSubscribe::connected()
{
    if (m_state == Trying) {
        emit fsmTryingConnected(QPrivateSignal());
    }
}
} // namespace pathview
} // namespace machinetalk
