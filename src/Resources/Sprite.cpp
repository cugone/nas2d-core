// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2019 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgement of your use of NAS2D is appriciated but is not required.
// ==================================================================================

#include "NAS2D/Resources/Sprite.h"
#include "NAS2D/Version.h"
#include "NAS2D/Xml/Xml.h"

#include <iostream>

using namespace std;
using namespace NAS2D;
using namespace NAS2D::Xml;

const string NAS2D::SPRITE_VERSION("0.99");

const int FRAME_PAUSE = -1;


// Adds a row/name tag to the end of messages.
string endTag(int row, const std::string& name)
{
	return " (Row: " + std::to_string(row) + ", " + name + ")";
}


/**
 * Default c'tor.
 *
 * Provided to allow sprites to be used in STL containers.
 *
 * \warning	Generally speaking, Sprites should not be instantiated
 *			with the default c'tor.
 */
Sprite::Sprite()
{
	addDefaultAction();
}


/**
 * C'tor.
 *
 * \param filePath	File path of the Sprite definition file.
 */
Sprite::Sprite(const std::string& filePath) :
	mSpriteName(filePath)
{
	processXml(filePath);
}

/**
 * Plays an action animation.
 *
 * \param	action	Name of the action to use for animation. Action's are not
 *			case sensitive. "Case", "caSe", "CASE", etc. will all be viewed
 *			as identical.
 *
 * \note	If the named action doesn't exist, a warning message will
 *			be written to the log and the default action will be used
 *			instead.
 */
void Sprite::play(const std::string& action)
{
	// Set the current frame list to the defined action. If action
	// isn't found, set to default and reset frame counter.
	ActionList::iterator actionIt = mActions.find(toLowercase(action));
	if (actionIt == mActions.end())
	{
		cout << "Named action '" << action << "' is not defined in '" << name() << "'." << endl;
		mCurrentAction = DEFAULT_ACTION;
		mCurrentFrame = 0;
	}
	else if (mCurrentAction == toLowercase(action))
	{
		// Reset the frame counter.
		mCurrentFrame = 0;
	}
	else
	{
		// Set the current action to the named action.
		mCurrentFrame = 0;
		mCurrentAction = toLowercase(action);
	}

	mTimer.reset();
	resume();
}


/**
 * Pauses animation for this Sprite.
 */
void Sprite::pause()
{
	mPaused = true;
}


/**
 * Resumes the action of the Sprite.
 */
void Sprite::resume()
{
	mPaused = false;
}


/**
 * Skips animation playback frames.
 *
 * \param	frames	Number of frames to skip.
 */
void Sprite::skip(int frames)
{
	if (mActions.find(toLowercase(mCurrentAction)) == mActions.end())
	{
		return;
	}

	mCurrentFrame = frames % mActions[mCurrentAction].size();
}


/**
 * Updates the Sprite and draws it to the screen at specified coordinaes.
 *
 * \param	x	X-Screen Coordinate to render the Sprite.
 * \param	y	X-Screen Coordinate to render the Sprite.
 */
void Sprite::update(float x, float y)
{
	SpriteFrame frame = mActions[mCurrentAction][mCurrentFrame];

	if (!mPaused && (frame.frameDelay() != FRAME_PAUSE))
	{
		while (frame.frameDelay() > 0 && static_cast<int>(mTimer.accumulator()) >= frame.frameDelay())
		{
			mTimer.adjust_accumulator(frame.frameDelay());
			mCurrentFrame++;
		}

		// Check that our frame count is within bounds.
		if (mCurrentFrame >= mActions[mCurrentAction].size())
		{
			mCurrentFrame = 0;
			mFrameCallback();		// Notifiy any frame listeners that the animation sequence has completed.
		}
	}
	else if (frame.frameDelay() == FRAME_PAUSE)
	{
		mFrameCallback();
	}

	Utility<Renderer>::get().drawSubImageRotated(mImageSheets[frame.sheetId()], x - frame.anchorX(), y - frame.anchorY(), static_cast<float>(frame.x()), static_cast<float>(frame.y()), static_cast<float>(frame.width()), static_cast<float>(frame.height()), mRotationAngle, mColor);
}


/**
 * Sets the rotation angle of the Sprite.
 *
 * \param	angle	Angle of rotation in degrees.
 */
void Sprite::rotation(float angle)
{
	mRotationAngle = angle;
}


/**
 * Gets the rotation angle of the Sprite.
 *
 * \return	Angle of rotation in degrees.
 */
float Sprite::rotation() const
{
	return mRotationAngle;
}


/**
 * Gets a list of Actions available for the Sprite.
 *
 * \return	StringList of actions.
 */
StringList Sprite::actions()
{
	StringList list;

	for (auto it = mActions.begin(); it != mActions.end(); ++it)
	{
		list.push_back(it->first);
	}

	return list;
}


/**
 * Increments the frame counter.
 */
void Sprite::incrementFrame()
{
	++mCurrentFrame;
	if (mCurrentFrame >= mActions[mCurrentAction].size())
	{
		mCurrentFrame = 0;
	}
}


/**
 * Decrements the frame counter.
 */
void Sprite::decrementFrame()
{
	if (mCurrentFrame == 0)
	{
		mCurrentFrame = mActions[mCurrentAction].size();
	}

	--mCurrentFrame;
}


/**
 * Parses a Sprite XML Definition File.
 *
 * \param filePath	File path of the sprite XML definition file.
 */
void Sprite::processXml(const std::string& filePath)
{
	Filesystem& fs = Utility<Filesystem>::get();

	if (!fs.exists(filePath))
	{
		cout << "Sprite file '" << filePath << "' doesn't exist." << endl;
		addDefaultAction();
		return;
	}

	File xmlFile = fs.open(filePath);

	// Load the file
	if (xmlFile.empty())
	{
		cout << "Sprite file '" << filePath << "' is empty." << endl;
		addDefaultAction();
		return;
	}

	XmlDocument docXml;
	XmlElement *xmlRootElement = nullptr;

	// Load the XML document and handle any errors if occuring
	docXml.parse(xmlFile.raw_bytes());
	if (docXml.error())
	{
		cout << "Malformed XML. Row: " << docXml.errorRow() << " Column: " << docXml.errorCol() << ": " << docXml.errorDesc() << " (" << name() << ")" << endl;
		addDefaultAction();
		return;
	}
	else
	{
		// Find the Sprite node.
		xmlRootElement = docXml.firstChildElement("sprite");
		if (!xmlRootElement)
		{
			cout << "Specified file '" << filePath << "' doesn't contain a <sprite> tag." << endl;
			addDefaultAction();
			return;
		}

		// Get the Sprite version.
		const XmlAttribute* version = xmlRootElement->firstAttribute();
		if (!version || version->value().empty())
		{
			cout << "Root element in sprite file '" << filePath << "' doesn't define a version." << endl;
			addDefaultAction();
			return;
		}
		else if (version->value() != SPRITE_VERSION)
		{
			cout << "Sprite version mismatch (" << versionString() << ") in '" << filePath << "'. Expected (" << SPRITE_VERSION << ")." << endl;
			addDefaultAction();
			return;
		}

		// Note:
		// Here instead of going through each element and calling a processing function to handle
		// it, we just iterate through all nodes to find sprite sheets. This allows us to define
		// image sheets anywhere in the sprite file.
		processImageSheets(xmlRootElement);
		processActions(xmlRootElement);
	}
}


/**
 * Iterates through all elements of a Sprite XML definition looking
 * for 'imagesheet' elements and processes them.
 *
 * \note	Since 'imagesheet' elements are processed before any other
 *			element in a sprite definition, these elements can appear
 *			anywhere in a Sprite XML definition.
 */
void Sprite::processImageSheets(void* root)
{
	XmlElement* e = static_cast<XmlElement*>(root);

	string id, src;
	for (XmlNode* node = e->iterateChildren(nullptr);
		node != nullptr;
		node = e->iterateChildren(node))
	{
		if (node->value() == "imagesheet" && node->toElement())
		{
			const XmlAttribute* attribute = node->toElement()->firstAttribute();
			while (attribute)
			{
				if (toLowercase(attribute->name()) == "id") { id = attribute->value(); }
				else if (toLowercase(attribute->name()) == "src") { src = attribute->value(); }

				attribute = attribute->next();
			}

			if (id.empty())
			{
				cout << "Zero-length 'id' value in Imagesheet definition." << endTag(node->row(), name()) << endl;
				continue;
			}

			if (src.empty())
			{
				cout << "Zero-length 'src' value in Imagesheet definition." << endTag(node->row(), name()) << endl;
				continue;
			}

			addImageSheet(id, src, node);
		}
	}
}


/**
 * Adds an image sheet to the Sprite.
 *
 * \note	Imagesheet ID's are not case sensitive. "Case", "caSe",
 *			"CASE", etc. will all be viewed as identical.
 *
 * \param	id		String ID for the image sheet.
 * \param	src		Image sheet file path.
 * \param	node	XML Node (for error information).
 */
void Sprite::addImageSheet(const std::string& id, const std::string& src, void* node)
{
	Filesystem& fs = Utility<Filesystem>::get();

	// Search for an image sheet with 'id'. If not found, add it.
	if (mImageSheets.find(toLowercase(id)) == mImageSheets.end())
	{
		string imagePath = fs.workingPath(mSpriteName);
		imagePath += src;
		if (!fs.exists(imagePath))
		{
			cout << "Couldn't find '" << imagePath << "' defined in sprite file '" << name() << "'." << endl;
			mImageSheets[toLowercase(id)]; // Add a default image
		}
		else
		{
			mImageSheets[id] = Image(imagePath);
		}
	}
	else
	{
		cout << "Image-sheet redefinition '" << id << "'." << endTag(static_cast<XmlNode*>(node)->row(), name()) << ". Imagesheet ignored." << endl;
	}
}


/**
 * Iterates through all elements of a Sprite XML definition looking
 * for 'action' elements and processes them.
 *
 * \note	Action names are not case sensitive. "Case", "caSe",
 *			"CASE", etc. will all be viewed as identical.
 */
void Sprite::processActions(void* root)
{
	XmlElement* element = static_cast<XmlElement*>(root);

	for (XmlNode* node = element->iterateChildren(nullptr);
		node != nullptr;
		node = element->iterateChildren(node))
	{
		if (toLowercase(node->value()) == "action" && node->toElement())
		{

			string action_name;
			const XmlAttribute* attribute = node->toElement()->firstAttribute();
			while (attribute)
			{
				if (toLowercase(attribute->name()) == "name")
				{
					action_name = attribute->value();
				}

				attribute = attribute->next();
			}

			if (action_name.empty())
			{
				cout << "Zero-length 'name' value in Action definition." << endTag(node->row(), name()) << endl;
				continue;
			}

			if (mActions.find(toLowercase(action_name)) == mActions.end())
			{
				processFrames(action_name, node);
			}
			else
			{
				cout << "Redefinition of action '" << action_name << "'. First definition will be used." << endTag(node->row(), name()) << endl;
			}
		}
	}
}


/**
 * Parses through all <frame> tags within an <action> tag in a Sprite Definition.
 */
void Sprite::processFrames(const std::string& action, void* _node)
{
	XmlNode* node = static_cast<XmlNode*>(_node);

	FrameList frameList;

	for (XmlNode* frame = node->iterateChildren(nullptr);
		frame != nullptr;
		frame = node->iterateChildren(frame))
	{
		int currentRow = frame->row();

		if (frame->value() == "frame" && frame->toElement())
		{
			string sheetId;
			int delay = 0;
			int x = 0, y = 0;
			int width = 0, height = 0;
			int anchorx = 0, anchory = 0;

			const XmlAttribute* attribute = frame->toElement()->firstAttribute();
			while (attribute)
			{
				if (toLowercase(attribute->name()) == "sheetid") { sheetId = attribute->value(); }
				else if (toLowercase(attribute->name()) == "delay") { attribute->queryIntValue(delay); }
				else if (toLowercase(attribute->name()) == "x") { attribute->queryIntValue(x); }
				else if (toLowercase(attribute->name()) == "y") { attribute->queryIntValue(y); }
				else if (toLowercase(attribute->name()) == "width") { attribute->queryIntValue(width); }
				else if (toLowercase(attribute->name()) == "height") { attribute->queryIntValue(height); }
				else if (toLowercase(attribute->name()) == "anchorx") { attribute->queryIntValue(anchorx); }
				else if (toLowercase(attribute->name()) == "anchory") { attribute->queryIntValue(anchory); }
				else { std::cout << "Unexpected attribute '" << attribute->name() << "' found on row " << currentRow << std::endl; }

				attribute = attribute->next();
			}

			if (!validateSheetId(sheetId, currentRow))
			{
				continue;
			}

			// X-Coordinate
			if ( x < 0 || x > mImageSheets.find(sheetId)->second.width())
			{
				cout << "Value 'x' is out of bounds." << endTag(currentRow, name()) << endl;
				continue;
			}

			// Y-Coordinate
			if (y < 0 || y > mImageSheets.find(sheetId)->second.height())
			{
				cout << "Value 'y' is out of bounds." << endTag(currentRow, name()) << endl;
				continue;
			}

			// Width
			if (width < 1)
			{
				width = 1;
				cout << "'width' value must be greater than 0." << endTag(currentRow, name()) << endl;
				continue;
			}
			else if (x + width > mImageSheets.find(sheetId)->second.width())
			{
				cout << "'x' + 'width' value exceeds dimensions of specified imagesheet." << endTag(currentRow, name()) << endl;
				continue;
			}

			// Height
			if (height < 1)
			{
				height = 1;
				cout << "'height' value must be greater than 0." << endTag(currentRow, name()) << endl;
				continue;
			}
			else if (y + height > mImageSheets.find(sheetId)->second.height())
			{
				cout << "'y' + 'height' value exceeds dimensions of specified imagesheet." << endTag(currentRow, name()) << endl;
				continue;
			}

			frameList.push_back(SpriteFrame(sheetId, x, y, width, height, anchorx, anchory, delay));
		}
		else
		{
			cout << "Unexpected tag '<" << frame->value() << ">'." << endTag(currentRow, name()) << endl;
		}
	}

	// Add the frame list to the action container.
	if (frameList.size() > 0)
	{
		mActions[toLowercase(action)] = frameList;
	}
	else
	{
		cout << "Action '" << action << "' contains no valid frames. (" << name() << ")" << endl;
	}
}


bool Sprite::validateSheetId(const std::string& sheetId, int row)
{
	if (sheetId.empty())
	{
		cout << "Frame definition has a zero-length 'sheetid' value. Frame is being ignored." << endTag(row, name()) << endl;
		return false;
	}
	else if (mImageSheets.find(sheetId) == mImageSheets.end())
	{
		cout << "Frame definition references an undefined imagesheet '" << sheetId << "'." << endTag(row, name()) << endl;
		return false;
	}
	else if (!mImageSheets.find(sheetId)->second.loaded())
	{
		cout << "Frame definition references an imagesheet that failed to load." << endTag(row, name()) << endl;
		return false;
	}

	return true;
}


/**
 * Adds a 'default' action to the action list.
 *
 * \note	This function will not override a 'default' action if one has
 *			has already been specified.
 */
void Sprite::addDefaultAction()
{
	if (mActions.find("default") == mActions.end())
	{
		auto& imageSheet = mImageSheets["default"]; // Adds a default sheet. //-V607

		int width = imageSheet.width();
		int height = imageSheet.height();

		FrameList frameList{SpriteFrame("default", 0, 0, width, height, width / 2, height / 2, -1)};
		mActions["default"] = frameList;
	}
}


/**
 * Gets the width of the Sprite.
 *
 * \note	This gets the width of the current frame. For most
 *			most sprites this will be the same for all frames
 *			but can be surprising if frame sizes vary.
 */
int Sprite::width() const
{
	return mActions.at(mCurrentAction)[mCurrentFrame].width();
}


/**
 * Gets the height of the Sprite.
 *
 * \note	This gets the height of the current frame. For most
 *			most sprites this will be the same for all frames
 *			but can be surprising if frame sizes vary.
 */
int Sprite::height() const
{
	return mActions.at(mCurrentAction)[mCurrentFrame].height();
}


/**
 * Gets the origin X-Coordinate of the Sprite.
 *
 * \note	This gets the origin of the current frame. For most
 *			most sprites this will be the same for all frames
 *			but can be surprising if frame sizes vary.
 */
int Sprite::originX(int x) const
{
	return x - mActions.at(mCurrentAction)[mCurrentFrame].anchorX();
}


/**
 * Gets the origin Y-Coordinate of the Sprite.
 *
 * \note	This gets the origin of the current frame. For most
 *			most sprites this will be the same for all frames
 *			but can be surprising if frame sizes vary.
 */
int Sprite::originY(int y) const
{
	return y - mActions.at(mCurrentAction)[mCurrentFrame].anchorY();
}



// ==================================================================================
// = spriteFrame member function definitions.
// ==================================================================================

/**
 * Constructor
 *
 * \param	sId	Sprite sheet ID.
 * \param	x	X-Coordinte of the area to copy from the source Image object.
 * \param	y	Y-Coordinte of the area to copy from the source Image object.
 * \param	w	Width of the area to copy from the source Image object.
 * \param	h	Height of the area to copy from the source Image object.
 * \param	aX	X-Axis of the Anchor Point for this spriteFrame.
 * \param	aY	Y-Axis of the Anchor Point for this spriteFrame.
 * \param	d	Length of time milliseconds to display this spriteFrame during animation playback.
 */
Sprite::SpriteFrame::SpriteFrame(const std::string& sId, int x, int y, int w, int h, int aX, int aY, int d) :
	mSheetId(sId),
	mFrameDelay(d),
	mAnchorX(aX),
	mAnchorY(aY),
	mRect(x, y, w, h)
{}
