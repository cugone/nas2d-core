// ==================================================================================
// = NAS2D
// = Copyright � 2008 - 2017 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// = 
// = Acknowledgement of your use of NAS2D is appriciated but is not required.
// ==================================================================================
// = Originally based on TinyXML. See Xml.h for additional details.
// ==================================================================================
#pragma once

#include "NAS2D/Xml/XmlNode.h"

namespace NAS2D {
namespace Xml {

/**
 * XML text.
 * A text node has two ways to output text: "normal" output and CDATA. It will default
 * to the mode it was parsed from the XML file.
 */
class TiXmlText : public TiXmlNode
{
	friend class TiXmlElement;
public:
	/**
	 * Constructor for text element. By default, it is treated as normal, encoded text.
	 * If you want it be output as a CDATA text	element, call \c CDATA(true).
	 */
	TiXmlText(const std::string& initValue);
	virtual ~TiXmlText() {}

	TiXmlText(const TiXmlText& copy);
	TiXmlText& operator=(const TiXmlText& base);

	virtual void Print(std::string& buf, int depth) const;

	/**
	 * Queries whether this represents text using a CDATA section.
	 */
	bool CDATA() const { return cdata; }

	/**
	 * Turns on or off a CDATA representation of text.
	 */
	void CDATA(bool _cdata) { cdata = _cdata; }

	virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

	virtual const TiXmlText* ToText() const { return this; }
	virtual TiXmlText* ToText() { return this; }

	/**
	 * Walk the XML tree visiting this node and all of its children.
	 */
	virtual bool Accept(XmlVisitor* visitor) const { return visitor->Visit(*this); }

protected:
	///  [internal use] Creates a new Element and returns it.
	virtual TiXmlNode* Clone() const;
	void CopyTo(TiXmlText* target) const;

	bool Blank() const;	// returns true if all white space and new lines
	virtual void StreamIn(std::istream& in, std::string& tag);

private:
	bool cdata;			// true if this should be input and output as a CDATA style text element
};

} // namespace Xml
} // namespace NAS2D