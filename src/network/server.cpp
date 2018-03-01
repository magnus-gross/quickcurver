#include "server.h"

Server::Server()
{
	connect(&tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));
	connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
	reListen(0);
}

Server::~Server()
{
	clients.clear();
}

/**
 * @brief Broadcasts new Curver data to every Client
 */
void Server::broadcastCurverData()
{
	if (++dataBroadcastIteration % Settings::getSingleton().getNetworkCurverBlock() == 0) {
		Packet::ServerCurverData p;
		p.fill();
		p.start = true;
		// if reset is due, send reset and reset the reset flag
		p.reset = resetDue;
		resetDue = false;
		broadcastPacket(p);
	}
}

/**
 * @brief Broadcasts a chat message to every Client
 * @param username The author of the message
 * @param message The chat message
 */
void Server::broadcastChatMessage(QString username, QString message)
{
	Packet::ServerChatMsg p;
	p.username = username;
	p.message = message;
	ChatModel::getSingleton().appendMessage(username, message);
	broadcastPacket(p);
}

/**
 * @brief Broadcasts an admin chat message to every Client
 * @param msg The chat message to broadcast
 */
void Server::broadcastChatMessage(QString msg)
{
	broadcastChatMessage(ADMIN_NAME, msg);
}

/**
 * @brief Resets the current round
 */
void Server::resetRound()
{
	resetDue = true;
}

/**
 * @brief Reconfigures the Server to listen on another port
 * @param port The new port to listen on
 */
void Server::reListen(quint16 port)
{
	tcpServer.close();
	tcpServer.listen(QHostAddress::Any, port);
	qDebug() << "Running on port " << tcpServer.serverPort();
}

/**
 * @brief Broadcasts the PlayerModel to every Client
 */
void Server::broadcastPlayerModel()
{
	Packet::ServerPlayerModel p;
	p.fill();
	broadcastPacket(p);
}

/**
 * @brief Broadcasts a new Item event to every Client
 * @param spawned Whether the Item spawned or was triggered
 * @param sequenceNumber The unique sequence number of the Item
 * @param which The kind of Item
 * @param pos The location of the Item
 * @param allowedUsers The allowed users for the Item
 * @param collectorIndex If \a spawned is \c false, this value defines which Curver collected the Item
 */
void Server::broadcastItemData(bool spawned, unsigned int sequenceNumber, int which, QPointF pos, Item::AllowedUsers allowedUsers, int collectorIndex)
{
	Packet::ServerItemData p;
	p.spawned = spawned;
	p.sequenceNumber = sequenceNumber;
	p.which = which;
	p.pos = pos;
	p.allowedUsers = allowedUsers;
	p.collectorIndex = collectorIndex;
	broadcastPacket(p);
}

/**
 * @brief This function is called, when an error occurred during accepting an incoming connection
 */
void Server::acceptError(QAbstractSocket::SocketError)
{
	Gui::getSingleton().postInfoBar(tcpServer.errorString());
}

/**
 * @brief This function is called when there is a new connection pending
 */
void Server::newConnection()
{
	QTcpSocket *s = tcpServer.nextPendingConnection();
	// socket must not be NULL
	if (s) {
		auto *curver = PlayerModel::getSingleton().getNewPlayer();
		curver->controller = Curver::Controller::CONTROLLER_REMOTE;
		clients[std::unique_ptr<QTcpSocket>(s)] = curver;
		connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
		connect(s, SIGNAL(disconnected()), this, SLOT(socketDisconnect()));
		connect(s, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
		broadcastChatMessage(s->peerAddress().toString() + " joined");
	}
}

/**
 * @brief This function is called, when there was a socket error
 */
void Server::socketError(QAbstractSocket::SocketError)
{
	QTcpSocket *s = static_cast<QTcpSocket *>(sender());
	Gui::getSingleton().postInfoBar(s->errorString());
	removePlayer(s);
}

/**
 * @brief This function is called, when a socket disconnected
 */
void Server::socketDisconnect()
{
	QTcpSocket *s = static_cast<QTcpSocket *>(sender());
	removePlayer(s);
}

/**
 * @brief This function is called, when there is data available to read from a socket
 */
void Server::socketReadyRead()
{
	QTcpSocket *s = static_cast<QTcpSocket *>(sender());
	QDataStream in(s);
	bool illformedPacket = false;
	while (s->bytesAvailable() && !illformedPacket) {
		in.startTransaction();
		auto packet = Packet::AbstractPacket::receivePacket(in, InstanceType::Client);
		if (in.commitTransaction()) {
			handlePacket(packet, s);
		} else {
			qDebug() << "received ill-formed packet";
			illformedPacket = true;
		}
	}
}

/**
 * @brief Removes a player permanently
 * @param s The socket that defines the Curver to remove
 */
void Server::removePlayer(const QTcpSocket *s)
{
	// TODO: Reconsider, whether Server should remove the Curver from the player model as well
	// TODO: Actually delete something here: Note, deleting here results in a segfault, because after this method was called in socketError()
	// or in socketDisconnect(), the TcpSocket seems to be still in use until after those methods are completely finished.

//	auto it = Util::find_if(clients, [=](const auto &c){ return c.first.get() == s; });
//	if (it != clients.end()) {
//		clients.erase(it);
	//	}
	broadcastChatMessage(s->peerAddress().toString() + " left the game");
}

/**
 * @brief Processes an already received packet
 * @param p The packet to process
 * @param s The socket that the packet was received with
 */
void Server::handlePacket(std::unique_ptr<Packet::AbstractPacket> &p, const QTcpSocket *s)
{
	Curver *curver = curverFromSocket(s);
	// TODO: Deal with flags
	switch (static_cast<Packet::ClientTypes>(p->type)) {
	case Packet::ClientTypes::Chat_Message:
	{
		QString msg = ((Packet::ClientChatMsg *)p.get())->message;
		broadcastChatMessage(curver->userName, msg);
		break;
	}
	case Packet::ClientTypes::PlayerModelEdit:
	{
		auto *playerData = (Packet::ClientPlayerModel *)p.get();
		curver->userName = playerData->username;
		curver->setColor(playerData->color);
		PlayerModel::getSingleton().forceRefresh();
		break;
	}
	case Packet::ClientTypes::CurverRotation:
	{
		if (curver) {
			curver->rotation = ((Packet::ClientCurverRotation *)p.get())->rotation;
		}
		break;
	}
	default:
		qDebug() << "Unsupported package type";
		break;
	}
}

/**
 * @brief Broadcasts a packet to every Client
 * @param p The packet to broadcast
 */
void Server::broadcastPacket(Packet::AbstractPacket &p)
{
	Util::for_each(clients, [&](auto &c){ p.sendPacket(c.first.get()); });
}

/**
 * @brief Returns the Curver connected to a given socket
 * @param s The socket to return the Curver of
 * @return The Curver that belongs to \a s
 */
Curver *Server::curverFromSocket(const QTcpSocket *s) const
{
	auto it = Util::find_if(clients, [&](auto &c){ return c.first.get() == s; });
	if (it != clients.end()) {
		return it->second;
	} else {
		return nullptr;
	}
}
