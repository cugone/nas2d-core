
#include "AnimationSet.h"

#include "ResourceCache.h"
#include "../Utility.h"
#include "../Filesystem.h"
#include "../ContainerUtils.h"
#include "../StringUtils.h"
#include "../ParserHelper.h"
#include "../Version.h"
#include "../Xml/Xml.h"


using namespace NAS2D;


namespace
{
	constexpr std::string_view SPRITE_VERSION{"0.99"};

	using ImageCache = ResourceCache<Image, std::string>;
	ImageCache animationImageCache;


	// Adds a row tag to the end of messages.
	std::string endTag(int row)
	{
		return " (Row: " + std::to_string(row) + ")";
	}

	AnimationSet processXml(std::string filePath, ImageCache& imageCache);
	std::map<std::string, std::string> processImageSheets(const std::string& basePath, const Xml::XmlElement* element, ImageCache& imageCache);
	std::map<std::string, std::vector<AnimationSet::Frame>> processActions(const std::map<std::string, std::string>& imageSheetMap, const Xml::XmlElement* element, ImageCache& imageCache);
	std::vector<AnimationSet::Frame> processFrames(const std::map<std::string, std::string>& imageSheetMap, const std::string& action, const Xml::XmlElement* element, ImageCache& imageCache);
}


AnimationSet::AnimationSet(std::string fileName) : AnimationSet{processXml(std::move(fileName), animationImageCache)}
{
}


AnimationSet::AnimationSet(std::string fileName, std::map<std::string, std::string> imageSheetMap, std::map<std::string, std::vector<Frame>> actions) :
	mFileName{std::move(fileName)},
	mImageSheetMap{std::move(imageSheetMap)},
	mActions{std::move(actions)}
{
}


std::vector<std::string> AnimationSet::actionNames() const
{
	return getKeys(mActions);
}


const std::vector<AnimationSet::Frame>& AnimationSet::frames(const std::string& actionName) const
{
	if (mActions.find(actionName) == mActions.end())
	{
		throw std::runtime_error("Sprite::play called on undefined action: " + actionName + "  (" + mFileName + ")");
	}

	return mActions.at(actionName);
}


namespace
{

	/**
	 * Parses a Sprite XML Definition File.
	 *
	 * \param filePath	File path of the sprite XML definition file.
	 */
	AnimationSet processXml(std::string filePath, ImageCache& imageCache)
	{
		try
		{
			auto& filesystem = Utility<Filesystem>::get();
			const auto basePath = filesystem.parentPath(filePath);

			Xml::XmlDocument xmlDoc;
			xmlDoc.parse(filesystem.read(filePath).c_str());

			if (xmlDoc.error())
			{
				throw std::runtime_error("Sprite file has malformed XML: Row: " + std::to_string(xmlDoc.errorRow()) + " Column: " + std::to_string(xmlDoc.errorCol()) + " : " + xmlDoc.errorDesc());
			}

			// Find the Sprite node.
			const auto* xmlRootElement = xmlDoc.firstChildElement("sprite");
			if (!xmlRootElement)
			{
				throw std::runtime_error("Sprite file does not contain required <sprite> tag");
			}

			// Get the Sprite version.
			const auto version = xmlRootElement->attribute("version");
			if (version.empty())
			{
				throw std::runtime_error("Sprite file's root element does not specify a version");
			}
			if (version != SPRITE_VERSION)
			{
				throw std::runtime_error("Sprite version mismatch. Expected: " + std::string{SPRITE_VERSION} + " Actual: " + versionString());
			}

			// Note:
			// Here instead of going through each element and calling a processing function to handle
			// it, we just iterate through all nodes to find sprite sheets. This allows us to define
			// image sheets anywhere in the sprite file.
			auto imageSheetMap = processImageSheets(basePath, xmlRootElement, imageCache);
			auto actions = processActions(imageSheetMap, xmlRootElement, imageCache);
			return {std::move(filePath), std::move(imageSheetMap), std::move(actions)};
		}
		catch(const std::runtime_error& error)
		{
			throw std::runtime_error("Error parsing Sprite file: " + filePath + "\nError: " + error.what());
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
	std::map<std::string, std::string> processImageSheets(const std::string& basePath, const Xml::XmlElement* element, ImageCache& imageCache)
	{
		std::map<std::string, std::string> imageSheetMap;

		for (const auto* node = element->firstChildElement(); node; node = node->nextSiblingElement())
		{
			if (node->value() == "imagesheet")
			{
				const auto dictionary = attributesToDictionary(*node);
				const auto id = dictionary.get("id");
				const auto src = dictionary.get("src");

				if (id.empty())
				{
					throw std::runtime_error("Sprite imagesheet definition has `id` of length zero: " + endTag(node->row()));
				}

				if (src.empty())
				{
					throw std::runtime_error("Sprite imagesheet definition has `src` of length zero: " + endTag(node->row()));
				}

				if (imageSheetMap.find(id) != imageSheetMap.end())
				{
					throw std::runtime_error("Sprite image sheet redefinition: id: '" + id + "' " + endTag(node->row()));
				}

				const auto imagePath = basePath + src;
				imageSheetMap.try_emplace(id, imagePath);
				imageCache.load(imagePath);
			}
		}

		return imageSheetMap;
	}


	/**
	 * Iterates through all elements of a Sprite XML definition looking
	 * for 'action' elements and processes them.
	 */
	std::map<std::string, std::vector<AnimationSet::Frame>> processActions(const std::map<std::string, std::string>& imageSheetMap, const Xml::XmlElement* element, ImageCache& imageCache)
	{
		std::map<std::string, std::vector<AnimationSet::Frame>> actions;

		for (const auto* action = element->firstChildElement(); action; action = action->nextSiblingElement())
		{
			if (action->value() == "action")
			{
				const auto dictionary = attributesToDictionary(*action);
				const auto actionName = dictionary.get("name");

				if (actionName.empty())
				{
					throw std::runtime_error("Sprite Action definition has 'name' of length zero: " + endTag(action->row()));
				}
				if (actions.find(actionName) != actions.end())
				{
					throw std::runtime_error("Sprite Action redefinition: '" + actionName + "' " + endTag(action->row()));
				}

				actions[actionName] = processFrames(imageSheetMap, actionName, action, imageCache);
			}
		}

		return actions;
	}


	/**
	 * Parses through all <frame> tags within an <action> tag in a Sprite Definition.
	 */
	std::vector<AnimationSet::Frame> processFrames(const std::map<std::string, std::string>& imageSheetMap, const std::string& action, const Xml::XmlElement* element, ImageCache& imageCache)
	{
		std::vector<AnimationSet::Frame> frameList;

		for (const auto* frame = element->firstChildElement(); frame; frame = frame->nextSiblingElement())
		{
			int currentRow = frame->row();

			if (frame->value() != "frame")
			{
				throw std::runtime_error("Sprite frame tag unexpected: <" + frame->value() + "> : " + endTag(currentRow));
			}

			const auto dictionary = attributesToDictionary(*frame);
			reportMissingOrUnexpected(dictionary.keys(), {"sheetid", "delay", "x", "y", "width", "height", "anchorx", "anchory"}, {});

			const auto sheetId = dictionary.get("sheetid");
			const auto delay = dictionary.get<int>("delay");
			const auto x = dictionary.get<int>("x");
			const auto y = dictionary.get<int>("y");
			const auto width = dictionary.get<int>("width");
			const auto height = dictionary.get<int>("height");
			const auto anchorx = dictionary.get<int>("anchorx");
			const auto anchory = dictionary.get<int>("anchory");

			if (sheetId.empty())
			{
				throw std::runtime_error("Sprite Frame definition has 'sheetid' of length zero: " + endTag(currentRow));
			}
			const auto iterator = imageSheetMap.find(sheetId);
			if (iterator == imageSheetMap.end())
			{
				throw std::runtime_error("Sprite Frame definition references undefined imagesheet: '" + sheetId + "' " + endTag(currentRow));
			}

			const auto& image = imageCache.load(iterator->second);
			// X-Coordinate
			if (x < 0 || x > image.size().x)
			{
				throw std::runtime_error("Sprite frame attribute 'x' is out of bounds: " + endTag(currentRow));
			}
			// Y-Coordinate
			if (y < 0 || y > image.size().y)
			{
				throw std::runtime_error("Sprite frame attribute 'y' is out of bounds: " + endTag(currentRow));
			}
			// Width
			if (width <= 0 || width > image.size().x - x)
			{
				throw std::runtime_error("Sprite frame attribute 'width' is out of bounds: " + endTag(currentRow));
			}
			// Height
			if (height <= 0 || height > image.size().y - y)
			{
				throw std::runtime_error("Sprite frame attribute 'height' is out of bounds: " + endTag(currentRow));
			}

			const auto bounds = Rectangle<int>::Create(Point<int>{x, y}, Vector{width, height});
			const auto anchorOffset = Vector{anchorx, anchory};
			frameList.push_back(AnimationSet::Frame{image, bounds, anchorOffset, static_cast<unsigned int>(delay)});
		}

		if (frameList.empty())
		{
			throw std::runtime_error("Sprite Action contains no valid frames: " + action);
		}

		return frameList;
	}

}
