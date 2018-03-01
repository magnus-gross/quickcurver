#include "game.h"

/**
 * @brief Constructs a Game with the given parent.
 *
 * \a parent is used to draw everything onto the GUI window.
 * @param parent The parent to draw on
 */
Game::Game(QQuickItem *parent) : QQuickItem(parent)
{
	/* Create a new root node. No, this does not leak memory, the Qt scene graph automatically deletes this node at the end.
	 * The scene graph knows this root node from updatePaintNode and will destroy it, when the window closes.
	 * Do NOT under any circumstances delete this node, or otherwise Qt will try to double free it,
	 * which will result in a big Qt backtrace when the window closes.
	 * Trust me, you do not want to debug this again.
	*/
	rootNode = new QSGNode();
	itemFactory = std::make_unique<ItemFactory>(rootNode);
	// tell the playermodel, what the root node is, so that it can tell its curvers
	PlayerModel::getSingleton().setRootNode(this->rootNode);
	connect(&PlayerModel::getSingleton(), SIGNAL(curverDied()), this, SLOT(curverDied()));
	connect(&PlayerModel::getSingleton(), SIGNAL(playerModelChanged()), &server, SLOT(broadcastPlayerModel()));
	connect(&ItemModel::getSingleton(), SIGNAL(itemSpawned(bool,uint,int,QPointF,Item::AllowedUsers,int)), &server, SLOT(broadcastItemData(bool,uint,int,QPointF,Item::AllowedUsers,int)));
	wall.setParentNode(rootNode);
	connect(&client, SIGNAL(integrateItem(bool,uint,int,QPointF,Item::AllowedUsers,int)), itemFactory.get(), SLOT(integrateItem(bool,uint,int,QPointF,Item::AllowedUsers,int)));
	connect(&client, SIGNAL(resetRound()), this, SLOT(resetRound()));
	connect(&Settings::getSingleton(), SIGNAL(dimensionChanged()), this, SLOT(dimensionChanged()));
	// GUI signals
	connect(&Gui::getSingleton(), SIGNAL(postInfoBar(QString)), this, SIGNAL(postInfoBar(QString)));
	connect(&Gui::getSingleton(), SIGNAL(startGame()), this, SLOT(tryStartGame()));

	// client signals
	connect(&client, SIGNAL(connectedToServerChanged(bool)), this, SLOT(connectedToServerChanged(bool)));

	connect(&gameTimer, SIGNAL(timeout()), this, SLOT(progress()));
	// tell QtQuick, that this component wants to draw stuff
	setFlag(ItemHasContents);
}

Game::~Game() {
	// we do manual memory management with every child node, so we have to remove every child node to prevent a double free
	rootNode->removeAllChildNodes();
}

/**
 * @brief Starts the game
 */
void Game::startGame()
{
	tryStartGame();
	lastProgressTime = QTime::currentTime();
	Util::for_each(getCurvers(), [](const std::unique_ptr<Curver> &c){ c->start(); });
	itemFactory->resetRound();
	// 60 FPS = 16 ms interval
	gameTimer.start(static_cast<int>(1000.f / Settings::getSingleton().getUpdatesPerSecond()));
}

/**
 * @brief Processes a key
 *
 * Checks if this key was registered to trigger any rotation changes.
 * @param key The key to process
 * @param release Whether the key was pressed or released
 */
void Game::processKey(Qt::Key key, bool release)
{
	Util::for_each(getCurvers(), [&](std::unique_ptr<Curver> &c){ c->processKey(key, release); });
	if (connectedToServer) {
		client.processKey(key, release);
	}
}

/**
 * @brief Connects as a client to the given host
 * @param ip The IP address of the host
 * @param port The port that the host is listening on
 */
void Game::connectToHost(QString ip, int port)
{
	QHostAddress addr = QHostAddress(ip);
	client.connectToHost(addr, port);
}

/**
 * @brief Sends a chat message
 *
 * If this instance is the Server of a game, the chat message will be broadcasted to all clients.
 * @param msg The message to send
 */
void Game::sendChatMessage(QString msg)
{
	if (connectedToServer) {
		client.sendChatMessage(msg);
	} else {
		server.broadcastChatMessage(msg);
	}
}

/**
 * @brief Reconfigures the server to listen on a different port
 * @param port The port to listen on
 */
void Game::serverReListen(quint16 port)
{
	server.reListen(port);
}

/**
 * @brief Resets the entire game
 */
void Game::resetGame()
{
	resetRound();
	Util::for_each(getCurvers(), [](const auto &c){ c->totalScore = 0; });
}

/**
 * @brief Called by the scene graph. This is called before the screen is redrawn.
 * @return Always return Game::rootNode
 */
QSGNode *Game::updatePaintNode(QSGNode *, QQuickItem::UpdatePaintNodeData *)
{
	return rootNode;
}

/**
 * @brief Updates the game's logic respecting how much time actually passed by
 */
void Game::progress()
{
	QTime currentTime = QTime::currentTime();
	int deltat = lastProgressTime.msecsTo(currentTime);
	lastProgressTime = currentTime;
	for (auto &c : getCurvers()) {
		if (c->isAlive()) {
			if (c->controller == Curver::Controller::CONTROLLER_BOT) {
				Bot::makeMove(c.get());
			}
			c->progress(deltat, getCurvers());
			c->checkForWall();
		}
	}
	itemFactory->update();

	update();
	server.broadcastCurverData();
}

/**
 * @brief Called, when a curver died
 *
 * This method taskes care of the score board and checks if a new round is due.
 */
void Game::curverDied()
{
	Util::for_each(getCurvers(), [](const auto &c){ if (c->isAlive()) c->increaseScore(); });
	auto maxScorer = Util::max_element(getCurvers());
	if ((*maxScorer)->totalScore >= Settings::getSingleton().getTargetScore()) {
		// we have a winner
		server.broadcastChatMessage((*maxScorer)->userName + " won!");
	}
	// check if only one player is remaining
	if (Util::count_if(getCurvers(), [](const auto &c){ return c->isAlive(); }) == 1) {
		resetRoundTimer.singleShot(Settings::getSingleton().getRoundTimeOut(), this, SLOT(resetRound()));
	}
}

/**
 * @brief Resets the round
 */
void Game::resetRound()
{
	itemFactory->resetRound();
	Util::for_each(getCurvers(), [](const auto &c){ c->resetRound(); });
	server.resetRound();
}

/**
 * @brief Called, when the game dimension changed
 */
void Game::dimensionChanged()
{
	wall.updateDimension();
}

/**
 * @brief Called, when the connection status changed
 * @param connected Whether the new state is connected
 */
void Game::connectedToServerChanged(bool connected)
{
	this->connectedToServer = connected;
}

/**
 * @brief Tries to start the game.
 *
 * This method does nothing, if the game already started.
 */
void Game::tryStartGame()
{
	if (!started) {
		started = true;
		emit gameStarted();
	}
}

/**
 * @brief A helper method to get a vector of all curvers
 * @return A vector containing all curvers
 */
std::vector<std::unique_ptr<Curver> > &Game::getCurvers()
{
	return PlayerModel::getSingleton().getCurvers();
}
