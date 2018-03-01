#include "gui.h"

/**
 * @brief Returns the Gui singleton
 * @return The Gui singleton
 */
const Gui &Gui::getSingleton()
{
	static Gui result;
	return result;
}
